//===--------------- ResolveTypes.cpp - DTransResolveTypesPass ------------===//
//
// Copyright (C) 2018 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
//
// This file implements the DTrans ResolveTypes pass.
//
// This pass looks for duplicate types that result from the same type being
// defined in multiple modules. When this happens the IR will contain two or
// more types with the same base name but different numeric suffixes that
// have the same layout, possibly distinguished by other duplicate types.
// The types may appear in bitcasts, including function bitcasts, where
// cross-module calls were made.
//
// For example:
//
//   %struct.A = type { i32, i32 }
//   %struct.B = type { %struct.A*, %struct.A* }
//   %struct.A.123 = type { i32, i32 }
//   %struct.B.456 = type { %struct.A.123*, %struct.A.123* }
//
//   define void @f(%struct.A* %a, %struct.B* %b) {
//     < function body >
//   }
//
//   define void @g(%struct.A.123* %a, %struct.B.456* %b) {
//     call void bitcast (void (%struct.A*, %struct.B*)* @f
//                          to void (%struct.A.123*, %struct.B.456*)*) (%a, %b)
//     ret void
//   }
//
// This pass will combine the duplicate types above and remove the bitcast at
// the callsite.
//
// This pass should be run before DTransAnalysis in order to prevent bitcast
// safety conditions resulting from the duplicate types.
//
//===----------------------------------------------------------------------===//

#include "Intel_DTrans/Transforms/ResolveTypes.h"
#include "Intel_DTrans/Analysis/DTrans.h"
#include "Intel_DTrans/Analysis/DTransAnalysis.h"
#include "Intel_DTrans/DTransCommon.h"
#include "Intel_DTrans/Transforms/DTransOptBase.h"
#include "llvm/ADT/EquivalenceClasses.h"
#include "llvm/ADT/SmallBitVector.h"
#include "llvm/Analysis/Intel_WP.h"
#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/IR/InstVisitor.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Operator.h"
#include "llvm/Pass.h"
#include "llvm/Transforms/IPO.h"
using namespace llvm;

#define DEBUG_TYPE "dtrans-resolvetypes"

#define DTRT_VERBOSE "dtrans-resolvetypes-verbose"

#define DTRT_COMPAT "dtrans-resolvetypes-compat"
#define DTRT_COMPAT_VERBOSE "dtrans-resolvetypes-compat-verbose"

namespace {

// Check to see if the specified name ends with a suffix consisting of a '.'
// and one or more digits. If it does, return the name without the suffix.
// If not, return the name as is.
//
// We're looking for types that have the same name except for an appended
// suffix, like this:
//
//   %struct.A = type {...}
//   %struct.A.123 = type {...}
//   %struct.A.456 = type {...}
//   %struct.A.2.789 = type {...}
//
// However, we don't want to attempt to match types like this:
//
//   %struct.CO = type {...}
//   %struct.CO2 = type {...}
//
// So we start by trimming trailing numbers, then look for and trim a
// single '.', and repeat this as long as we trimmed something and found
// a trailing '.'.
StringRef getTypeBaseName(StringRef TyName) {
  StringRef RefName = TyName;
  StringRef BaseName = TyName.rtrim("0123456789");
  while (RefName.size() != BaseName.size()) {
    if (!BaseName.endswith("."))
      return RefName;
    RefName = BaseName.drop_back(1);
    BaseName = RefName.rtrim("0123456789");
  }
  return RefName;
}

bool typesHaveSameBaseName(StructType *StTyA, StructType *StTyB) {
  if (!StTyA->hasName() && !StTyB->hasName())
    return true;
  if (!StTyA->hasName() || !StTyB->hasName())
    return false;
  return getTypeBaseName(StTyA->getName())
      .equals(getTypeBaseName(StTyB->getName()));
}

class DTransResolveTypesWrapper : public ModulePass {
private:
  dtrans::ResolveTypesPass Impl;

public:
  static char ID;

  DTransResolveTypesWrapper() : ModulePass(ID) {
    initializeDTransResolveTypesWrapperPass(*PassRegistry::getPassRegistry());
  }

  bool runOnModule(Module &M) override {
    if (skipModule(M))
      return false;
    DTransAnalysisInfo &DTInfo =
        getAnalysis<DTransAnalysisWrapper>().getDTransInfo();
    const TargetLibraryInfo &TLI =
        getAnalysis<TargetLibraryInfoWrapperPass>().getTLI();
    WholeProgramInfo &WPInfo =
        getAnalysis<WholeProgramWrapperPass>().getResult();
    return Impl.runImpl(M, DTInfo, TLI, WPInfo);
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    // TODO: Mark the actual required and preserved analyses.
    AU.addRequired<DTransAnalysisWrapper>();
    AU.addRequired<TargetLibraryInfoWrapperPass>();
    AU.addRequired<WholeProgramWrapperPass>();
    AU.addPreserved<WholeProgramWrapperPass>();
  }
};

class ResolveTypesImpl : public DTransOptBase {
public:
  ResolveTypesImpl(DTransAnalysisInfo &DTInfo, LLVMContext &Context,
                   const DataLayout &DL, const TargetLibraryInfo &TLI,
                   DTransTypeRemapper *TypeRemapper)
      : DTransOptBase(DTInfo, Context, DL, TLI, "__DTRT_", TypeRemapper) {}

  bool prepareTypes(Module &M) override;
  void populateTypes(Module &M) override;

private:
  // A mapping from the original structure type to the new structure type
  DenseMap<StructType *, StructType *> OrigToNewTypeMapping;

  // A second mapping that lets us distinguish between types that we remapped
  // because another type was mapped to them and types that are being
  // remapped because we are merging them with another type.
  DenseMap<StructType *, StructType *> IdentityDestMapping;

  enum CompareResult {
    Equivalent, // The types have equivalent members in every field
    Compatible, // The types are equivalent except for pointer types of members
    Distinct    // The types are neither equivalent nor compatible
  };

  CompareResult compareTypes(StructType *TyA, StructType *TyB);
  CompareResult
  compareTypeMembers(StructType *TyA, StructType *TyB,
                     DenseSet<std::pair<Type *, Type *>> &TypesSeen);
  bool tryToMapTypes(StructType *TyA, StructType *TyB,
                     EquivalenceClasses<StructType *> &CompatSets);
  void collectDependentTypeMappings(
      StructType *SrcTy, StructType *DestTy,
      DenseSet<std::pair<StructType *, StructType *>> &DependentMappings);
  void addTypeMapping(StructType *SrcTy, StructType *DestTy);
  bool hasBeenRemapped(StructType *Ty);
};

// This class analyzes all globals and functions in the module to see if any
// types in the CompatibleTypes EquivalenceClasses can be remapped cleanly to
// one another. Compatible types can be remapped cleanly if the mismatched
// elements are never accessed in one of the types.
//
// As a rule, pointer types have no semantic meaning in LLVM IR. If we remap
// a structure type to a different structure type that is equivalent except
// for the types of pointer elements, the IR will still be semantically
// correct no matter how the structure is used. If the mismatched pointer
// is accessed, the type mapped will insert a bitcast to correct the
// accessed type. However, the purpose of this pass is to clean up situations
// where types were incorrectly mapped and extra bitcasts have been inserted.
// Therefore, we'd like to avoid adding any incorrect mappings of our own.
// The CompatibleTypeAnalyzer attempts to identify cases where remapping
// will result in clean IR.
//
// For example, consider the following types:
//
//   %A = type { i32, i32, %B*, %C* }
//   %A.1 = type { i32, i32, %B.1*, %C.1* }
//   %A.2 = type { i32, i32, %B.1*, %C* }
//   %B = type { i64, i64 }
//   %B.1 = type { i32, i32 }
//   %C = type { [8 x i8] }
//   %C.1 = type { i32*, i32* }
//
// If the %B.1* and %C.1* members of %A.1 are never accessed through a
// value that has the %A.1 type, then %A.1 can be remapped to %A without
// complications. If the %B.1* member is accessed but the %C.1* member is not,
// then %A.1 should not be mapped to %A but it could still be remapped to %A.2.
class CompatibleTypeAnalyzer : public InstVisitor<CompatibleTypeAnalyzer> {
public:
  CompatibleTypeAnalyzer(Module &M,
                         EquivalenceClasses<StructType *> &CompatibleTypes)
      : M(M), CompatibleTypes(CompatibleTypes) {}

  // This is the main entry point called by the client of this class.
  void run() {
    visitModule(M);
    DEBUG_WITH_TYPE(DTRT_COMPAT, dumpCollectedData());
  }

  // Apply a heuristic to determine what type the specified type should be
  // remapped to and return a pointer to that type. If the heuristic determines
  // that the type should not be remapped, this function will return a pointer
  // to the original type.
  //
  // The heuristic is basically this: if a type was seen to be implicitly
  // bitcast to another structure type via a bitcast at a function call site
  // and there are no conflicting fields between the two types that are
  // accessed by the source type then the source type should be remapped to the
  // destination type.
  //
  // In some cases there may be a bitcast chain, where A is cast to B and B is
  // cast to C. In those cases we will continue to follow the bitcast chain
  // as long as the conditions above are met.
  StructType *getRemapCandidate(StructType *Ty) {
    unsigned NumElements = Ty->getNumElements();
    const TypeUseInfo &CurUseInfo = TypeUseInfoMap[Ty];
    SmallPtrSet<StructType *, 4> VisitedTypes;

    // If the type is accessed by a non-constant GEP, don't remap it.
    if (CurUseInfo.HasNonConstantIndexAccess)
      return Ty;

    // Follow the chain of function bitcasts as long as there is
    // only one destination and the destination doesn't have an access
    // that conflicts with any fields that have been accessed by any
    // type that previously appeared in the chain.
    auto *MapToTy = Ty;
    VisitedTypes.insert(Ty);
    SmallBitVector FieldsUsed(CurUseInfo.NonScalarFieldsUsed); // Copy
    auto *FnBitcastToSet = &(TypeUseInfoMap[MapToTy].FnBitcastToSet);
    while (FnBitcastToSet->size() == 1) {
      auto *MaybeTy = dyn_cast<StructType>(*(FnBitcastToSet->begin()));
      if (!MaybeTy)
        break;

      if (!VisitedTypes.insert(MaybeTy).second) {
        DEBUG_WITH_TYPE(DTRT_COMPAT_VERBOSE,
                        dbgs() << "DTRT-compat: Rejecting mapping of "
                               << Ty->getName()
                               << " because of a circular bitcast chain.\n");
        return Ty;
      }

      const TypeUseInfo &MaybeUseInfo = TypeUseInfoMap[MaybeTy];

      assert((NumElements == MaybeTy->getNumElements()) &&
             "Compatible types found with mismatch element count!");
      SmallBitVector ConflictingFields(NumElements);
      // TODO: We should be checking for remapped equivalent types here.
      for (unsigned Idx = 0; Idx < NumElements; ++Idx)
        if (MaybeTy->getElementType(Idx) != Ty->getElementType(Idx))
          ConflictingFields.set(Idx);

      // If any of the previous types in the chain were used to access any
      // of the fields that are in conflict, we can't use this mapping.
      if (FieldsUsed.anyCommon(ConflictingFields)) {
        DEBUG_WITH_TYPE(DTRT_COMPAT_VERBOSE,
                        dbgs() << "DTRT-compat: Rejecting mapping ("
                               << Ty->getName() << " -> " << MaybeTy->getName()
                               << " because of conflicting field access.\n");
        break;
      }

      // If the type is accessed by a non-constant GEP, don't remap past it.
      if (MaybeUseInfo.HasNonConstantIndexAccess)
        break;

      MapToTy = MaybeTy;
      FieldsUsed |= MaybeUseInfo.NonScalarFieldsUsed;
      FnBitcastToSet = &(TypeUseInfoMap[MapToTy].FnBitcastToSet);
    }

    return MapToTy;
  }

  // Here we look at the global variables and aliases in the module to
  // find an GEPOperators that would otherwise have been missed while
  // visiting instructions. Then we visit each function to return control
  // to the InstVisitor base class which will in turn call our instruction
  // visitors.
  void visitModule(Module &M) {
    for (auto &GV : M.globals())
      visitGlobalValueUsers(&GV);
    for (auto &A : M.aliases())
      visitGlobalValueUsers(&A);

    // Visit each function.
    visit(M.begin(), M.end());
  }

  // Here we defer to a helper function so that GEP operators accessing
  // globals directly as inline operands of other instructions can be
  // handled by the same code that analyzes GEP instructions.
  void visitGetElementPtrInst(GetElementPtrInst &GEP) {
    visitGEPOperator(cast<GEPOperator>(&GEP));
  }

  // Here we defer to a helper function so bitcast operators acting on
  // globals directly as inline operands of other instructions can be
  // handled by the same code that analyzes bitcast instructions.
  void visitBitCastInst(BitCastInst &Cast) {
    visitBitCastOperator(cast<BitCastOperator>(&Cast));
  }

  // Call sites are the primary point of interest for fixing linker mistakes.
  // When the IR linker resolves a type from one input module differently than
  // what should be a matching type in another input module, calls of functions
  // that were defined in one module to functions that we defined in the other
  // module will require a bitcast of the function pointer type.
  //
  // Here we look for bitcasts of the called value and attempt to record any
  // implicit casts between types, either the return type or a parameter type,
  // that results from this function bitcast.
  void visitCallSite(CallSite CS) {
    if (auto *Cast = dyn_cast<BitCastOperator>(CS.getCalledValue())) {
      auto referencesTypeOfInterest = [&](Type *Ty) {
        auto *BaseTy = Ty;
        while (BaseTy->isPointerTy())
          BaseTy = BaseTy->getPointerElementType();
        if (isTypeOfInterest(BaseTy))
          return true;
        return false;
      };

      auto fnTyReferencesTypeOfInterest = [&](FunctionType *FnTy) {
        auto *RetTy = FnTy->getReturnType();
        if (referencesTypeOfInterest(RetTy))
          return true;
        for (auto *ParamTy : FnTy->params())
          if (referencesTypeOfInterest(ParamTy))
            return true;
        return false;
      };

      auto *SrcTy = Cast->getSrcTy()->getPointerElementType();
      auto *DestTy = Cast->getDestTy()->getPointerElementType();

      // It's possible that the source type was not a function type pointer.
      // In that case, we can't really deduce anything from the call site.
      if (auto *SrcFnTy = dyn_cast<FunctionType>(SrcTy)) {
        bool ReferencesTypeOfInterest = false;
        if (fnTyReferencesTypeOfInterest(SrcFnTy))
          ReferencesTypeOfInterest = true;
        auto *DestFnTy = cast<FunctionType>(DestTy);
        if (fnTyReferencesTypeOfInterest(DestFnTy))
          ReferencesTypeOfInterest = true;

        if (ReferencesTypeOfInterest) {
          // If the source was a function pointer type, this effectively
          // bitcasts the return type and every argument type.
          //
          // A typical call site will look like this:
          //
          //   call void bitcast (void (%struct.A*)* @useA
          //                   to void (%struct.A.1*)*)(%struct.A.1* %a)
          //
          // In this case, a function which has a %struct.A* parameter is
          // being called with a %struct.A.1* value. Although the DestTy
          // in the call site bitcast is a pointer to the function type
          // with a %struct.A.1* parameter, this has the effect of casting
          // the %struct.A.1* value to %struct.A*, so the SrcTy and DestTy
          // are mapped here in the opposite way of what their names
          // might suggest.
          recordTypeCasting(DestFnTy->getReturnType(), SrcFnTy->getReturnType(),
                            /*IsCall*/ true);
          // Don't assume that both function types have the same number of
          // parameters.
          unsigned MinArgs =
              std::min(DestFnTy->getNumParams(), SrcFnTy->getNumParams());
          for (unsigned Idx = 0; Idx < MinArgs; ++Idx)
            recordTypeCasting(DestFnTy->getParamType(Idx),
                              SrcFnTy->getParamType(Idx), /*IsCall=*/true);
          DEBUG_WITH_TYPE(DTRT_COMPAT_VERBOSE,
                          dbgs() << "DTRT-compt: visiting call with bitcast"
                                 << *(CS.getInstruction()) << "\n");
        }
      }
    }
  }

private:
  Module &M;
  const EquivalenceClasses<StructType *> &CompatibleTypes;

  // This structure is used to hold information collected about the field
  // accesses and bitcasts involving types in the CompatibleTypes container.
  class TypeUseInfo {
  public:
    // Each bit in this vector, if set, corresponds to a pointer element
    // in the structure that was accessed via a GEP.
    SmallBitVector NonScalarFieldsUsed;

    // Each type in this set is a type that was seen as the destination of
    // a bitcast (or implicit conversion in a bitcast call) from the type
    // tracked by this TypeUseInfo object.
    //
    // i.e. We saw a bitcast from the TypeUseInfoType *to* the set entry type.
    SmallPtrSet<Type *, 4> BitcastToSet;
    SmallPtrSet<Type *, 4> FnBitcastToSet;

    // Each type in this set is a type that was seen as the source of
    // a bitcast (or implicit conversion in a bitcast call) to the type
    // tracked by this TypeUseInfo object.
    //
    // i.e. We saw a bitcast *from* the set entry type to the TypeUseInfo type.
    SmallPtrSet<Type *, 4> BitcastFromSet;
    SmallPtrSet<Type *, 4> FnBitcastFromSet;

    // This value indicates whether or not we saw a GEP access to the type
    // using a non-constant index.
    bool HasNonConstantIndexAccess = false;

    // This value indicates that the return type of some GEP involved the type.
    bool ReturnedByGEP = false;
  };

  // A map from entries in the CompatibleTypes container to a object describing
  // data we recorded for that type.
  DenseMap<Type *, TypeUseInfo> TypeUseInfoMap;

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  // This is a diagnostic function that will only be used for debugging and
  // testing.
  LLVM_DUMP_METHOD
  void dumpCollectedData() {
    dbgs() << "\n========================\n";
    dbgs() << " DTRT-compat: Type data\n";
    dbgs() << "========================\n\n";
    // Iterate over all of the equivalence sets.
    for (EquivalenceClasses<StructType *>::iterator I = CompatibleTypes.begin(),
                                                    E = CompatibleTypes.end();
         I != E; ++I) {
      // A "Leader" in an EquivalenceClasses set is just the first entry
      // in a group of entries that have been marked as belonging together.
      // It serves as the head of a linked list.
      if (!I->isLeader())
        continue; // Skip over non-leader sets.
      dbgs() << "    ===== Group =====\n\n";
      auto MI = CompatibleTypes.member_begin(I);
      auto ME = CompatibleTypes.member_end();
      StructType *LeaderTy = *MI;
      unsigned NumElements = LeaderTy->getNumElements();
      dbgs() << "\n" << LeaderTy->getName() << "\n";
      dbgs() << *LeaderTy << "\n";
      SmallBitVector &LdrNonScalarFieldsUsed =
          TypeUseInfoMap[LeaderTy].NonScalarFieldsUsed;
      dbgs() << "  Leader non-scalar fields accessed:";
      if (LdrNonScalarFieldsUsed.none()) {
        dbgs() << " None\n\n";
      } else {
        for (unsigned BI : LdrNonScalarFieldsUsed.set_bits())
          dbgs() << " " << BI;
        dbgs() << "\n\n";
      }

      // Loop over members in this set.
      SmallBitVector ConflictingFields;
      while (MI != ME) {
        StructType *Ty = *MI;
        dbgs() << Ty->getName() << "\n";

        TypeUseInfo &UseInfo = TypeUseInfoMap[Ty];

        ConflictingFields.clear();
        ConflictingFields.resize(NumElements);

        if (UseInfo.HasNonConstantIndexAccess)
          dbgs() << "  Has non-constant field access\n";

        if (UseInfo.ReturnedByGEP)
          dbgs() << "  Returned by GEP\n";

        if (Ty != LeaderTy) {
          dbgs() << *Ty << "\n";
          assert((Ty->getNumElements() == NumElements) &&
                 "Compatible types found with mismatch element count!");
          // TODO: We should be checking for remapped equivalent types here.
          for (unsigned Idx = 0; Idx < NumElements; ++Idx)
            if (Ty->getElementType(Idx) != LeaderTy->getElementType(Idx))
              ConflictingFields.set(Idx);

          SmallBitVector &FieldsUsed = UseInfo.NonScalarFieldsUsed;

          if (ConflictingFields.none())
            dbgs() << "UNEXPECTED: No conflicting fields in compatible type!\n";
          dbgs() << "  Conflicting fields accessed:";
          if (!ConflictingFields.anyCommon(FieldsUsed)) {
            dbgs() << " None\n";
          } else {
            FieldsUsed.resize(NumElements);
            for (unsigned BI : ConflictingFields.set_bits())
              if (FieldsUsed.test(BI))
                dbgs() << " " << BI;
            dbgs() << "\n";
          }
        }

        dbgs() << "  Bitcast to:";
        SmallPtrSetImpl<Type *> &CastTo = UseInfo.BitcastToSet;
        if (CastTo.empty()) {
          dbgs() << " None\n";
        } else {
          dbgs() << "\n";
          for (auto *DestTy : CastTo) {
            auto *DST = dyn_cast<StructType>(DestTy);
            if (DST && DST->hasName())
              dbgs() << "    " << DST->getName() << "\n";
            else
              dbgs() << "    " << *DestTy << "\n";
          }
        }

        dbgs() << "  Bitcast from:";
        SmallPtrSetImpl<Type *> &CastFrom = UseInfo.BitcastFromSet;
        if (CastFrom.empty()) {
          dbgs() << " None\n";
        } else {
          dbgs() << "\n";
          for (auto *SrcTy : CastFrom) {
            auto *SST = dyn_cast<StructType>(SrcTy);
            if (SST && SST->hasName())
              dbgs() << "    " << SST->getName() << "\n";
            else
              dbgs() << "    " << *SrcTy << "\n";
          }
        }

        dbgs() << "  Fn Bitcast to:";
        SmallPtrSetImpl<Type *> &FnCastTo = UseInfo.FnBitcastToSet;
        if (FnCastTo.empty()) {
          dbgs() << " None\n";
        } else {
          dbgs() << "\n";
          for (auto *DestTy : FnCastTo) {
            auto *DST = dyn_cast<StructType>(DestTy);
            if (DST && DST->hasName())
              dbgs() << "    " << DST->getName() << "\n";
            else
              dbgs() << "    " << *DestTy << "\n";
          }
        }

        dbgs() << "  Fn Bitcast from:";
        SmallPtrSetImpl<Type *> &FnCastFrom = UseInfo.FnBitcastFromSet;
        if (FnCastFrom.empty()) {
          dbgs() << " None\n";
        } else {
          dbgs() << "\n";
          for (auto *SrcTy : FnCastFrom) {
            auto *SST = dyn_cast<StructType>(SrcTy);
            if (SST && SST->hasName())
              dbgs() << "    " << SST->getName() << "\n";
            else
              dbgs() << "    " << *SrcTy << "\n";
          }
        }

        StructType *RemapCandidateTy = getRemapCandidate(Ty);
        if (RemapCandidateTy == Ty)
          dbgs() << "  Type cannot be remapped.\n";
        else
          dbgs() << "  Type can be remapped to " << RemapCandidateTy->getName()
                 << "\n";

        dbgs() << "\n";
        ++MI;
      }
      dbgs() << "\n    =================\n\n";
    }
  }
#endif

  // This function follows the chain of users of global variables and aliases
  // to see if these values are used by GEP or bitcast operators as direct
  // operands of instructions. If these are found, we call helper functions
  // that analyzer these operators in the same way that the corresponding
  // instruction would be used. Because these operators (as well as other
  // operators such as extract value) can be chained together, we continue to
  // follow the uses of any users that are constant expressions. It isn't
  // possible for these uses to form a cycle, so no recursion guard is needed.
  void visitGlobalValueUsers(Constant *C) {
    auto *Ty = C->getType();
    if (!Ty->isPointerTy() || !isTypeOfInterest(Ty->getPointerElementType()))
      return;
    for (auto *U : C->users()) {
      if (auto *GEP = dyn_cast<GEPOperator>(U))
        visitGEPOperator(GEP);
      else if (auto *BC = dyn_cast<BitCastOperator>(U))
        visitBitCastOperator(BC);
      // If we still haven't reached an instruction user, keep following uses.
      if (auto *CE = dyn_cast<ConstantExpr>(U))
        visitGlobalValueUsers(CE);
    }
  }

  // For GEP instructions and operators, we want to know which elements
  // within a structure are being accessed. For the purposes of compatibility
  // analysis, we are only interested in accessed to elements that are in
  // conflict between two types that were judged to be compatible. These
  // conflicting members are necessarily pointers so we can ignore intermediate
  // accesses and any access that isn't referring to a structure pointer member.
  void visitGEPOperator(GEPOperator *GEP) {
    // If the GEP has fewer than 2 index operands, it isn't accessing a
    // structure element. For the purposes of remapping types we don't need
    // to consider byte-flattened GEPs. We'll see those cases as bitcasts.
    if (GEP->getNumIndices() < 2)
      return;

    // If the pointer returned references one of the structures we're
    // working with, that type cannot be remapped without re-working the GEP.
    // Record the fact that it was returned here.
    auto *ResultTy = GEP->getResultElementType();
    if (auto *ResultST = dyn_cast<StructType>(dtrans::unwrapType(ResultTy)))
      if (isTypeOfInterest(ResultST))
        TypeUseInfoMap[ResultST].ReturnedByGEP = true;

    // The result element type is the type of the structure element being
    // accessed (in the case where the GEP is indexing into a structure).
    // If this element's type isn't a pointer type then this element can't be
    // a conflicting element between compatible types.
    if (!ResultTy->isPointerTy() && !ResultTy->isStructTy())
      return;

    // Find the types being indexed by this GEP.
    SmallVector<Value *, 4> Ops(GEP->idx_begin(), GEP->idx_end());
    while (Ops.size() > 1) {
      Value *IdxArg = Ops.back();
      Ops.pop_back();
      Type *IndexedTy =
          GetElementPtrInst::getIndexedType(GEP->getSourceElementType(), Ops);

      // If the GEP isn't indexing a structure, we don't need to track it.
      auto *IndexedStTy = dyn_cast<StructType>(IndexedTy);
      if (!IndexedStTy)
        continue;

      // If the last element isn't a constant int, we can't do anything with it.
      auto *Idx = dyn_cast<ConstantInt>(IdxArg);
      if (!Idx) {
        TypeUseInfoMap[IndexedTy].HasNonConstantIndexAccess = true;
        continue;
      }
      // If we can't get a value for this constant or it exceeeds the capacity
      // of an unsigned value, we can't track it.
      uint64_t IdxVal = Idx->getLimitedValue();
      if (IdxVal > std::numeric_limits<unsigned>::max()) {
        TypeUseInfoMap[IndexedTy].HasNonConstantIndexAccess = true;
        continue;
      }
      // If we get here, this is a potentially interesting element access.
      // Set a bit in the type field use map entry for this type to record it.
      SmallBitVector &Bits = TypeUseInfoMap[IndexedTy].NonScalarFieldsUsed;
      if (Bits.size() <= IdxVal)
        Bits.resize(IdxVal + 1);
      Bits.set((unsigned)IdxVal);
      DEBUG_WITH_TYPE(DTRT_COMPAT_VERBOSE,
                      dbgs() << "DTRT-compat: GEP accesses type:\n    "
                             << *IndexedTy << "\n  At index: " << IdxVal
                             << "\n  GEP:\n    " << *GEP << "\n");
    }
  }

  // For bitcast instructions or operators, we want to examine the source
  // and destination types to see if they involve one of the types in our
  // compatible types container. The types being cast will always be pointers
  // but they may be pointers-to-pointers, pointers-to-arrays, or
  // pointers-to-vectors. We'll peel through array and vector wrappers types
  // when we call recordTypeCasting(), but here we need to look through any
  // pointers to skip over element zero access casts.
  void visitBitCastOperator(BitCastOperator *Cast) {
    auto *SrcTy = Cast->getSrcTy();
    auto *DestTy = Cast->getDestTy();
    while (SrcTy->isPointerTy() && DestTy->isPointerTy()) {
      if (dtrans::isElementZeroAccess(SrcTy, DestTy))
        return;
      if (isVTableCast(SrcTy, DestTy))
        return;
      SrcTy = SrcTy->getPointerElementType();
      DestTy = DestTy->getPointerElementType();
    }
    recordTypeCasting(SrcTy, DestTy, /*IsCall=*/false);
    DEBUG_WITH_TYPE(DTRT_COMPAT_VERBOSE,
                    dbgs() << "DTRT-compt: visiting bitcast" << Cast << "\n");
  }

  // This function records type casts that result either from a bitcast
  // instruction, a bitcast operator acting on a global variable, or a
  // bitcast operator at a call site.
  void recordTypeCasting(Type *SrcTy, Type *DestTy, bool IsCall) {
    // Function bitcasts will often involve identical types at one or more
    // position. We can safely ignore those.
    if (SrcTy == DestTy)
      return;

    // We're only interested in conversions of structure types in our
    // CompatibleTypes container, so here we peel off any pointer, array,
    // vector types that may be wrapping the structure types.
    auto *SrcBaseTy = SrcTy;
    auto *DestBaseTy = DestTy;
    while ((SrcBaseTy->isPointerTy() && DestBaseTy->isPointerTy()) ||
           (SrcBaseTy->isArrayTy() && DestBaseTy->isArrayTy()) ||
           (SrcBaseTy->isVectorTy() && DestBaseTy->isVectorTy())) {
      if (SrcBaseTy->isPointerTy()) {
        SrcBaseTy = SrcBaseTy->getPointerElementType();
        DestBaseTy = DestBaseTy->getPointerElementType();
      } else {
        SrcBaseTy = SrcBaseTy->getSequentialElementType();
        DestBaseTy = DestBaseTy->getSequentialElementType();
      }
    }

    // If SrcTy != DestTy, how could the code above get us here?
    assert(SrcBaseTy != DestBaseTy);

    // A cast from a structure pointer to an i8* or vice versa isn't
    // interesting. We've already peeled the pointers, so here we just
    // look for i8.
    auto *Int8Ty = Type::getInt8Ty(SrcTy->getContext());
    if (SrcBaseTy == Int8Ty || DestBaseTy == Int8Ty)
      return;

    DEBUG_WITH_TYPE(DTRT_COMPAT_VERBOSE, {
      if (isTypeOfInterest(SrcBaseTy) || isTypeOfInterest(DestBaseTy))
        dbgs() << "\nDTRT-compt: recording type cast\n"
               << "    From: " << *SrcBaseTy << "\n"
               << "    To  : " << *DestBaseTy << "\n";
    });
    if (isTypeOfInterest(SrcBaseTy)) {
      if (IsCall) {
        TypeUseInfoMap[SrcBaseTy].FnBitcastToSet.insert(DestBaseTy);
      } else {
        TypeUseInfoMap[SrcBaseTy].BitcastToSet.insert(DestBaseTy);
      }
    }
    if (isTypeOfInterest(DestBaseTy)) {
      if (IsCall) {
        TypeUseInfoMap[DestBaseTy].FnBitcastFromSet.insert(SrcBaseTy);
      } else {
        TypeUseInfoMap[DestBaseTy].BitcastFromSet.insert(SrcBaseTy);
      }
    }
  }

  // This function looks at the SrcTy and DestTy to see if DestTy matches
  // the idiom for a VTable pointer.  VTable pointers have the form
  // i32 (<Ty>*)**.
  bool isVTableCast(Type *SrcTy, Type *DestTy) {
    auto *ST = dyn_cast<StructType>(SrcTy);
    if (!ST)
      return false;
    LLVMContext &Context = ST->getContext();
    auto *VTableTy = FunctionType::get(Type::getInt32Ty(Context),
                                       {ST->getPointerTo()}, false)
                         ->getPointerTo()
                         ->getPointerTo();
    return (DestTy == VTableTy);
  }

  // The caller of this function should peel off wrapping pointer, array,
  // and vector types.
  bool isTypeOfInterest(Type *Ty) {
    if (auto *ST = dyn_cast_or_null<StructType>(Ty))
      return CompatibleTypes.findValue(ST) != CompatibleTypes.end();
    return false;
  }
};

} // end anonymous namespace

char DTransResolveTypesWrapper::ID = 0;
INITIALIZE_PASS_BEGIN(DTransResolveTypesWrapper, "dtrans-resolvetypes",
                      "DTrans resolve types", false, false)
INITIALIZE_PASS_DEPENDENCY(DTransAnalysisWrapper)
INITIALIZE_PASS_DEPENDENCY(TargetLibraryInfoWrapperPass)
INITIALIZE_PASS_DEPENDENCY(WholeProgramWrapperPass)
INITIALIZE_PASS_END(DTransResolveTypesWrapper, "dtrans-resolvetypes",
                    "DTrans resolve types", false, false)

ModulePass *llvm::createDTransResolveTypesWrapperPass() {
  return new DTransResolveTypesWrapper();
}

// This function walks over the structure types in the specified module and
// identifies types which we would like to remap to one another. At this point
// we will only create opaque replacement types for the types to be remapped.
// The types will be populated when the base class calls populateTypes.
bool ResolveTypesImpl::prepareTypes(Module &M) {

  auto getContainedStructTy = [](Type *Ty) {
    auto *BaseTy = Ty;
    while (BaseTy->isPointerTy() || BaseTy->isArrayTy() ||
           BaseTy->isVectorTy()) {
      if (BaseTy->isPointerTy())
        BaseTy = BaseTy->getPointerElementType();
      else
        BaseTy = BaseTy->getSequentialElementType();
    }
    return dyn_cast<StructType>(BaseTy);
  };

  // This map records the result of whether or not a structure type has
  // external dependencies for any type we've checked. If a type is in the
  // map, we know whether or not it has a dependency on any type that is
  // referenced by an extern function. If a type is not in the map, we will
  // need to check and then add the result to the map.
  DenseMap<StructType *, bool> ExternDependencyCache;

  // Start by priming the map with the types that are directly used by
  // external functions.
  for (Function &F : M) {
    if (F.isDeclaration()) {
      auto *FnTy = F.getFunctionType();
      if (auto *RetST = getContainedStructTy(FnTy->getReturnType()))
        ExternDependencyCache[RetST] = true;
      for (auto *ParamTy : FnTy->params())
        if (auto *ParamST = getContainedStructTy(ParamTy))
          ExternDependencyCache[ParamST] = true;
    }
  }

  std::function<bool(StructType *, SmallPtrSetImpl<StructType *> &, bool &)>
      hasExternDependencyImpl = [&](StructType *Ty,
                                    SmallPtrSetImpl<StructType *> &VisitedTypes,
                                    bool &IncompleteResult) {
        // This function does a lot of recursing. Make sure we aren't
        // looking for a result we've already computed. VisitedTypes
        // keeps the recursion in check, but ExternDependencyCache
        // provides results we computed on previous calls.
        auto CachedResult = ExternDependencyCache.find(Ty);
        if (CachedResult != ExternDependencyCache.end())
          return CachedResult->second;
        // This happens when we're still in the process of figuring
        // out the result for the given type.
        if (!VisitedTypes.insert(Ty).second) {
          IncompleteResult = true;
          return false;
        }
        // Check the types which this type contains.
        for (auto *MemberType : Ty->elements()) {
          if (auto *ElemStTy = getContainedStructTy(MemberType)) {
            if (hasExternDependencyImpl(ElemStTy, VisitedTypes,
                                        IncompleteResult)) {
              ExternDependencyCache[Ty] = true;
              return true;
            }
          }
        }
        // Also, check the types which contain this type.
        for (auto *DepTy : TypeToDependentTypes[Ty]) {
          if (auto *ST = dyn_cast<StructType>(DepTy)) {
            if (hasExternDependencyImpl(ST, VisitedTypes, IncompleteResult)) {
              ExternDependencyCache[Ty] = true;
              return true;
            }
          }
        }
        // If we get here, the type has no extern dependency was found,
        // but that may be because some types were unresolved. The
        // result is not cached here in that case.
        if (!IncompleteResult)
          ExternDependencyCache[Ty] = false;
        return false;
      };

  std::function<bool(StructType *)> hasExternDependency = [&](StructType *Ty) {
    SmallPtrSet<StructType *, 16> VisitedTypes;
    bool IncompleteResult = false;
    bool Result = hasExternDependencyImpl(Ty, VisitedTypes, IncompleteResult);
    // By the time we get here, we will have visited all of the dependencies
    // and the IncompleteResult flag only indicates that intermediate
    // type dependencies were possibly unknown. The result at this point can
    // be cached.
    ExternDependencyCache[Ty] = Result;
    return Result;
  };

  // Sometimes previous optimizations will have left types that are not
  // used in a way that allows Module::getIndentifiedStructTypes() to find
  // them. This can confuse our mapping algorithm, so here we check for
  // missing types and add them to the set we're looking at.
  std::vector<StructType *> StTypes = M.getIdentifiedStructTypes();
  SmallPtrSet<StructType *, 32> SeenTypes;
  std::function<void(StructType *)> findMissedNestedTypes =
      [&](StructType *Ty) {
        for (Type *ElemTy : Ty->elements()) {
          // Look past pointer, array, and vector wrappers.
          // If the element is a structure, add it to the SeenTypes set.
          // If it wasn't already there, check for nested types.
          if (StructType *ElemStTy = getContainedStructTy(ElemTy))
            if (SeenTypes.insert(ElemStTy).second)
              findMissedNestedTypes(ElemStTy);
        }
        if (!Ty->hasName())
          return;
        StringRef TyName = Ty->getName();
        StringRef BaseName = getTypeBaseName(TyName);
        // If the type name didn't have a suffix, TyName and BaseName will be
        // the same. Checking the size is sufficient.
        if (TyName.size() == BaseName.size())
          return;
        // Add the base type now. This might be the only way it is found.
        StructType *BaseTy = M.getTypeByName(BaseName);
        if (!BaseTy || !SeenTypes.insert(BaseTy).second)
          return;
        findMissedNestedTypes(BaseTy);
      };
  for (StructType *Ty : M.getIdentifiedStructTypes()) {
    // If we've seen this type already, skip it.
    if (!SeenTypes.insert(Ty).second)
      continue;
    findMissedNestedTypes(Ty);
  }

  DenseMap<StructType *, SetVector<StructType *>> CandidateTypeSets;
  for (StructType *Ty : SeenTypes) {
    // Ignore unnamed types.
    if (!Ty->hasName())
      continue;

    StringRef TyName = Ty->getName();
    StringRef BaseName = getTypeBaseName(TyName);

    // If the type name didn't have a suffix, TyName and BaseName will be
    // the same. Checking the size is sufficient.
    if (TyName.size() == BaseName.size())
      continue;

    // If BaseName is a not a known type, claim the name for this type.
    // This will potentially allow us to remap other types with the same
    // base name to this type.
    StructType *BaseTy = M.getTypeByName(BaseName);
    if (!BaseTy) {
      LLVM_DEBUG(dbgs() << "resolve-types: Renaming " << TyName << " -> "
                        << BaseName << "\n");
      Ty->setName(BaseName);
      continue;
    }

    // Don't try to remap types with external uses.
    if (hasExternDependency(Ty))
      continue;

    DEBUG_WITH_TYPE(DTRT_VERBOSE,
                    dbgs() << "resolve-types: Found candidate type\n  "
                           << TyName << " -> " << BaseName << "\n");
    CandidateTypeSets[BaseTy].insert(Ty);
  }

  if (CandidateTypeSets.empty())
    return false;

  // For groups of functions with the same name, try to remap equivalent
  // types to one another. The tryToMapTypes function will perform the
  // remapping if possible. If the types are compatible but not equivalent
  // tryToMapTypes will add them to the CompatibleTypes data structure.
  //
  // EquivalenceClasses is an LLVM-defined data structure that is a hybrid of
  // a set and a linked list. Each element in the set is an ECValue object
  // that contains the actual data (StructType*, in our case) and pointers
  // to previous and next ECValues in the set that have been determined to
  // be "equivalent" by the terms of the EquivalenceClasses user (which in our
  // case actually means the types are compatible, not equivalent).
  //
  // The CompatibleTypes data structure thus provides an efficient way for us
  // to group together structure types that we will later compare against
  // one another to see if they should be remapped.
  //
  // CandidateTypeSets is a collection of structure types grouped into sets
  // where each member has the same base name as the other types in the set,
  // distinguished by a numeric suffix. CandidateTypeSets is implemented as
  // a map, where the key is the structure type with the base name and the
  // value is the set of types with similar names.
  //
  // For each type in a candidate set, if the type is equivalent to the base
  // type, we will remap to the base type. If not, we will compare it to
  // any other type from the original candidate set that also did not match
  // the base type or any other type from the set we have previously considered.
  // If it is not equivalent to any of these, it will be added to the set of
  // alternatives.
  bool TypesRemapped = false;
  EquivalenceClasses<StructType *> CompatibleTypes;
  for (auto Entry : CandidateTypeSets) {
    StructType *BaseTy = Entry.first;

    // Don't try to remap types with external uses. We didn't check the base
    // type above because we would have needed to check it for each possible
    // variant there whereas here we can check it just once.
    if (hasExternDependency(BaseTy))
      continue;

    const SetVector<StructType *> &CandTypes = Entry.second;

    SmallVector<StructType *, 4> Alternates;
    for (auto *CandTy : CandTypes) {
      if (tryToMapTypes(BaseTy, CandTy, CompatibleTypes)) {
        TypesRemapped = true;
        continue;
      }
      DEBUG_WITH_TYPE(DTRT_VERBOSE,
                      dbgs() << "resolve-types: Types are not equivalent.\n"
                             << "    Base: " << *BaseTy << "\n"
                             << "    Cand: " << *CandTy << "\n");
      // This type didn't match the base type, but it may match another
      // type with the same base name that also didn't match the base
      // type.
      bool AltMatchFound = false;
      for (auto *AltTy : Alternates) {
        if (tryToMapTypes(AltTy, CandTy, CompatibleTypes)) {
          DEBUG_WITH_TYPE(DTRT_VERBOSE,
                          dbgs() << "resolve-types: Found alt duplicate type.\n"
                                 << "    Base: " << *AltTy << "\n"
                                 << "    Cand: " << *CandTy << "\n");
          AltMatchFound = true;
          TypesRemapped = true;
          break;
        }
      }
      // If no match was found, add this to a list of alternate types
      // with this base name to be checked if other types don't match
      // the base type.
      if (!AltMatchFound)
        Alternates.push_back(CandTy);
    }
  }

  if (CompatibleTypes.getNumClasses()) {
    CompatibleTypeAnalyzer CTA(M, CompatibleTypes);
    CTA.run();

    for (EquivalenceClasses<StructType *>::iterator I = CompatibleTypes.begin(),
                                                    E = CompatibleTypes.end();
         I != E; ++I) {
      if (!I->isLeader())
        continue; // Skip over non-leader sets.
      for (auto MI = CompatibleTypes.member_begin(I),
                ME = CompatibleTypes.member_end();
           MI != ME; ++MI) {
        auto *Ty = *MI;

        // If we've already mapped this type, don't try to map it to something
        // different.
        if (OrigToNewTypeMapping.count(Ty)) {
          DEBUG_WITH_TYPE(DTRT_COMPAT, {
            auto *RemapTy = CTA.getRemapCandidate(Ty);
            if (Ty != RemapTy)
              dbgs() << "Not remapping " << Ty->getName() << " -> "
                     << RemapTy->getName()
                     << " because the source was previously remapped to:\n"
                     << "   " << OrigToNewTypeMapping[Ty]->getName() << "\n";
          });
          continue;
        }

        // Try a heuristic to see if we think this type should be remapped.
        auto *RemapTy = CTA.getRemapCandidate(Ty);
        if (Ty == RemapTy)
          continue;

        // In order to remap these types, we need to perform the same
        // direction remapping of all nested types. Make sure that's possible
        // before we start.

        // If the candidate type has been remapped, don't use it because the
        // heuristic only verifies this one type.
        if (hasBeenRemapped(RemapTy)) {
          DEBUG_WITH_TYPE(DTRT_COMPAT,
                          dbgs() << "Not remapping " << Ty->getName() << " -> "
                                 << RemapTy->getName() << " because the "
                                 << "destination was previously remapped.\n");
          continue;
        }

        // If the source type has been mapped, don't map it again.
        if (OrigToNewTypeMapping.count(Ty))
          continue;

        // Otherwise, try to apply the mapping.
        addTypeMapping(Ty, RemapTy);
        TypesRemapped = true;
      }
    }
    // TODO: Use the CompatibleTypeAnalyzer data to select types to map.
  }

  return TypesRemapped;
}

// This function walks the members of two types that we would like to remap
// and collects the other mappings that would need to occur to make this
// possible.
//
// TODO: Combine this with other functions that have very similar logic.
void ResolveTypesImpl::collectDependentTypeMappings(
    StructType *SrcTy, StructType *DestTy,
    DenseSet<std::pair<StructType *, StructType *>> &DependentMappings) {
  if (SrcTy == DestTy)
    return;

  assert(SrcTy && DestTy &&
         "collectDependentTypeMappings() called with nullptr!");

  // If we've seen this pair before, don't continue collecting.
  if (!DependentMappings.insert(std::make_pair(SrcTy, DestTy)).second)
    return;

  // llvm::Type provides a check for trivial equivalence. Test that.
  // If the layouts are identical, there will be nothing more to remap.
  if (SrcTy->isLayoutIdentical(DestTy))
    return;

  // If the source type has already been remapped, we can stop here.
  // If it was remapped to something other than DestTy, we'll detect that
  // before we try to remap and the remapping will be blocked.
  if (OrigToNewTypeMapping.count(SrcTy))
    return;

  // Otherwise, look at the members.
  unsigned NumElements = SrcTy->getNumElements();
  assert(DestTy->getNumElements() == NumElements &&
         "collectDependentTypeMappings() called with different size types!");
  for (unsigned i = 0; i < NumElements; ++i) {
    Type *SrcElemTy = SrcTy->getElementType(i);
    Type *DestElemTy = DestTy->getElementType(i);

    // If the elements are the same type, they won't be changed by remapping.
    if (SrcElemTy == DestElemTy)
      continue;

    // Unwrap pointer, vector and array types. We aren't using unwrapType()
    // here because we need to be sure the elements have the same level of
    // pointer/vector/array nesting.
    Type *SrcBaseTy = SrcElemTy;
    Type *DestBaseTy = DestElemTy;
    while ((SrcBaseTy->isPointerTy() && DestBaseTy->isPointerTy()) ||
           (SrcBaseTy->isArrayTy() && DestBaseTy->isArrayTy()) ||
           (SrcBaseTy->isVectorTy() && DestBaseTy->isVectorTy())) {
      if (SrcBaseTy->isPointerTy()) {
        SrcBaseTy = SrcBaseTy->getPointerElementType();
        DestBaseTy = DestBaseTy->getPointerElementType();
      } else {
        SrcBaseTy = SrcBaseTy->getSequentialElementType();
        DestBaseTy = DestBaseTy->getSequentialElementType();
      }
    }
    assert(SrcBaseTy != DestBaseTy &&
           "Distinct types unwrapped to the same base type??");

    // These must be structs or we wouldn't have attempted the mapping.
    auto *SrcElemST = cast<StructType>(SrcBaseTy);
    auto *DestElemST = cast<StructType>(DestBaseTy);

    // Otherwise, collect mappings from the elements of these types.
    return collectDependentTypeMappings(SrcElemST, DestElemST,
                                        DependentMappings);
  }
}

// This function fills in the body of types that we previously created for
// remapping. The base class populates any dependent types that were found.
void ResolveTypesImpl::populateTypes(Module &M) {
  // We will have multiple original types mapped to the same replacement
  // type, so we need to check as we process this map to see if the type
  // already has a body.
  for (auto &ONPair : IdentityDestMapping) {
    StructType *OrigTy = ONPair.first;
    StructType *ReplTy = ONPair.second;

    // If we've already populated the replacement type, go to the next entry.
    if (!ReplTy->isOpaque())
      continue;

    DEBUG_WITH_TYPE(DTRT_VERBOSE, dbgs() << "Populating type: " << *ReplTy
                                         << "\n  OrigTy: " << *OrigTy << "\n");

    SmallVector<Type *, 8> DataTypes;
    for (auto *MemberTy : OrigTy->elements()) {
      DataTypes.push_back(TypeRemapper->remapType(MemberTy));
      DEBUG_WITH_TYPE(DTRT_VERBOSE,
                      dbgs() << "MemberTy mapping: " << *MemberTy << " -> "
                             << *(TypeRemapper->remapType(MemberTy)) << "\n");
    }

    ReplTy->setBody(DataTypes, OrigTy->isPacked());
    LLVM_DEBUG(dbgs() << "resolve-types: New structure body: " << *ReplTy
                      << "\n");
  }
}

// This function determines whether or not a type has been remapped to
// a substantively different type. When a type is used as the destination of
// a mapping, we create a new opaque type to replace the original destination
// since it might require updates because of type dependencies. However, this
// function distinguishes between a type that has been remapped for that reason
// and a type that was actually remapped to a different type.
bool ResolveTypesImpl::hasBeenRemapped(StructType *Ty) {
  if (IdentityDestMapping.count(Ty))
    return (IdentityDestMapping[Ty] != OrigToNewTypeMapping[Ty]);
  return OrigToNewTypeMapping.count(Ty);
}

// This function wraps our TypeRemapper's addTypeMapping function so that we
// can provide a remapping of both SrcTy and DestTy.
void ResolveTypesImpl::addTypeMapping(StructType *SrcTy, StructType *DestTy) {
  LLVMContext &Context = SrcTy->getContext();

  // Our type resolution algorithm tries to match types with existing types,
  // but in order to handle the case where structure members are also remapped
  // we need to replace the base type as well as the duplicate types.
  // If the destination type has been remapped, use the type to which it is
  // mapped as the actual destination type. Otherwise, create a new type for
  // the destination type as well.
  StructType *ActualDestTy = OrigToNewTypeMapping[DestTy];
  if (!ActualDestTy) {
    ActualDestTy = StructType::create(
        Context, (Twine("__DTRT_" + DestTy->getName()).str()));
    TypeRemapper->addTypeMapping(DestTy, ActualDestTy);
    OrigToNewTypeMapping[DestTy] = ActualDestTy;
    IdentityDestMapping[DestTy] = ActualDestTy;
  }
  LLVM_DEBUG(dbgs() << "resolve-types: Mapping " << SrcTy->getName() << " -> "
                    << ActualDestTy->getName() << "\n    " << *SrcTy << "\n    "
                    << *ActualDestTy << "\n");
  TypeRemapper->addTypeMapping(SrcTy, ActualDestTy);
  assert(!OrigToNewTypeMapping.count(SrcTy) &&
         "Type mapping unexpectedly replaced!");
  OrigToNewTypeMapping[SrcTy] = ActualDestTy;
}

// Given two structure types that we think might be equivalent, this function
// compares the types to determine whether or not they are actually equivalent.
// If they are equivalent, we add a mapping from \p TyA to \p TyB (which will
// be resolved later by the DTransOptBase class). If the types are compatible
// but not equivalent, we add them to a union in the \p CompatibleTypes data
// structure, which will be used elsewhere to attempt further remapping.
// If the types are neither equivalent nor compatible, this function does
// nothing.
//
// The function returns true only if the types were successfully mapped. If
// the types are compatible or distinct, the function returns false.
bool ResolveTypesImpl::tryToMapTypes(
    StructType *TyA, StructType *TyB,
    EquivalenceClasses<StructType *> &CompatibleTypes) {
  CompareResult Res = compareTypes(TyA, TyB);
  switch (Res) {
  case CompareResult::Equivalent:
    addTypeMapping(TyB, TyA);
    return true;
  case CompareResult::Compatible:
    CompatibleTypes.unionSets(TyA, TyB);
    DEBUG_WITH_TYPE(DTRT_VERBOSE,
                    dbgs() << "resolve-types: Types are compatible.\n"
                           << "    Base: " << *TyA << "\n"
                           << "    Cand: " << *TyB << "\n");
    return false;
  case CompareResult::Distinct:
    return false;
  }
  llvm_unreachable("covered switch isn't covered?");
}

// Compare \p TyA and \p TyB to see if they are equivalent or compatible.
//
// The types are considered Equivalent if all member fields are the same
// at all levels of nesting except for the names of nested structure types
// and structure type names differ only by suffix.
//
// The types are considered Compatible if all member fields are the same
// at all levels of nesting except for a mismatch of pointer types. If a
// pair of structures being compared contain pointers to other types
// which are Compatible, then the containing types can be Compatible but
// they cannot be Equivalent.
//
// The types are considered Distinct if they are neither Equivalent nor
// Compatible. This happens when either: (a) the types have different
// numbers of members, (b) the types have non-pointer members of different
// types at some location, or (c) one type contains a pointer member where
// the other does not.
//
// Equivalent types can always be re-mapped to one another. Compatible types
// can only be remapped if the mismatched pointer is never accessed using
// the pointer type in question and pointers to the type are only ever bitcast
// as pointers to types in the same compatibility set.
//
// The existence of Equivalent types and Compatible types is an artifact of
// inexact matching by LLVM's IR Linker. They do not result in incorrect code
// but they can lead to overly conservative conclusions by the DTrans analysis.
ResolveTypesImpl::CompareResult
ResolveTypesImpl::compareTypes(StructType *TyA, StructType *TyB) {
  // llvm::Type provides a check for trivial equivalence. Try that first.
  if (TyA->isLayoutIdentical(TyB))
    return CompareResult::Equivalent;

  // Otherwise, do an element-by-element check for equivalent types.
  DenseSet<std::pair<Type *, Type *>> TypesSeen;
  return compareTypeMembers(TyA, TyB, TypesSeen);
}

// This is a helper function for compareTypes. This function does a member by
// member comparison of two structure types, calling itself recursively to
// compare nested structure types, whether they are pointers to the structures
// or instances. The result is as described in the compareTypes comment.
ResolveTypesImpl::CompareResult ResolveTypesImpl::compareTypeMembers(
    StructType *TyA, StructType *TyB,
    DenseSet<std::pair<Type *, Type *>> &TypesSeen) {

  DEBUG_WITH_TYPE(DTRT_VERBOSE, {
    if (TyA->hasName() && TyB->hasName())
      dbgs() << "compareTypeMembers(" << TyA->getName() << ", "
             << TyB->getName() << ")\n";
    else
      dbgs() << "compareTypeMembers(" << *TyA << ", " << *TyB << ")\n";
  });

  // Different packing rules out our mapping.
  if (TyA->isPacked() != TyB->isPacked()) {
    DEBUG_WITH_TYPE(DTRT_VERBOSE, dbgs() << "Pack mismatch\n");
    return CompareResult::Distinct;
  }

  // Different numbers of elements is another easy indicator.
  if (TyA->getNumElements() != TyB->getNumElements()) {
    DEBUG_WITH_TYPE(DTRT_VERBOSE, dbgs() << "Element count mismatch\n");
    return CompareResult::Distinct;
  }

  // If we've seen this pair before, don't check it again. The previous
  // check may still be in progress, but we can assume a match here and
  // if something reveals a mismatch in the original search the final
  // result will reflect that.
  if (!TypesSeen.insert(std::make_pair(TyA, TyB)).second) {
    DEBUG_WITH_TYPE(DTRT_VERBOSE, dbgs() << "Types seen, assume match\n");
    return CompareResult::Equivalent;
  }

  // If the two elements we are comparing are found to mismatch but they are
  // pointer types, the parent structures may still be compatible. When that
  // happens we need to record the fact that we have proven the parent types
  // are not equivalent, but continue checking for compatibility.
  bool ProvenNotEquivalent = false;

  unsigned NumElements = TyA->getNumElements();
  for (unsigned i = 0; i < NumElements; ++i) {
    Type *ElemATy = TyA->getElementType(i);
    Type *ElemBTy = TyB->getElementType(i);

    // We're looking for mismatches. If the types are the same, we can
    // skip the details below.
    if (ElemATy == ElemBTy)
      continue;

    // Look past pointer, array, or vector types to see their element types.
    // However, we need to track whether or not the base type we find is
    // wrapped by a pointer type so we can recognize compatibility if only
    // pointer types are different.
    bool ComparingPointerElements = false;
    while (ElemATy->isPointerTy() || ElemATy->isArrayTy() ||
           ElemATy->isVectorTy()) {
      if (ElemATy->isPointerTy()) {
        // If the two types have different levels of indirection, the structs
        // don't match.
        if (!ElemBTy->isPointerTy()) {
          DEBUG_WITH_TYPE(DTRT_VERBOSE, dbgs()
                                            << "Indirection level mismatch @ "
                                            << i << "\n");
          return CompareResult::Distinct;
        }
        // Otherwise, drill down on both pointers.
        ElemATy = ElemATy->getPointerElementType();
        ElemBTy = ElemBTy->getPointerElementType();
        ComparingPointerElements = true;
      } else {
        assert((ElemATy->isArrayTy() || ElemATy->isVectorTy()) &&
               "Unexpected sequential type!");
        // Both elements must be of the same sequential type.
        if ((ElemATy->isArrayTy() && !ElemBTy->isArrayTy()) ||
            (ElemATy->isVectorTy() && !ElemBTy->isVectorTy())) {
          DEBUG_WITH_TYPE(DTRT_VERBOSE,
                          dbgs() << "Array/vector mismatch @ " << i << "\n");
          return CompareResult::Distinct;
        }
        // Arrays and vectors are handled together this way.
        ElemATy = ElemATy->getSequentialElementType();
        ElemBTy = ElemBTy->getSequentialElementType();
        ComparingPointerElements = false;
      }
    }

    // There is no need to re-check for identical types of the pointer, array,
    // or vector element types because if those were identical the original
    // types would also have been identical.

    if (ElemATy->isStructTy() && ElemBTy->isStructTy()) {
      auto *StElemATy = cast<StructType>(ElemATy);
      auto *StElemBTy = cast<StructType>(ElemBTy);
      // If the types don't share a base name, we want to consider them
      // as being different.
      if (!typesHaveSameBaseName(StElemATy, StElemBTy)) {
        DEBUG_WITH_TYPE(DTRT_VERBOSE, {
          dbgs() << "Type name mismatch @ " << i << "\n";
          if (StElemATy->hasName())
            dbgs() << "StElemATy = " << StElemATy->getName() << "\n";
          else
            dbgs() << "StElemATy (unnamed) " << *StElemATy << "\n";
          if (StElemBTy->hasName())
            dbgs() << "StElemBTy = " << StElemBTy->getName() << "\n";
          else
            dbgs() << "StElemBTy (unnamed) " << *StElemBTy << "\n";
        });
        if (!ComparingPointerElements)
          return CompareResult::Distinct;
        ProvenNotEquivalent = true;
        continue;
      }
      // Try the simple equivalence check.
      if (StElemATy->isLayoutIdentical(StElemBTy))
        continue;
      // Otherwise, go to the next level of element-by-element checking.
      // If they don't match, we're finished.
      auto Result = compareTypeMembers(StElemATy, StElemBTy, TypesSeen);
      if (Result != CompareResult::Equivalent) {
        DEBUG_WITH_TYPE(DTRT_VERBOSE,
                        dbgs() << "Element member mismatch @ " << i << "\n");
        // If these nested structures are distinct but we found pointers
        // to the structures above, the parent structures may still be
        // compatible. If we did not find pointers to these structures
        // above, the parent structures are also distinct.
        if ((Result == CompareResult::Distinct) && !ComparingPointerElements)
          return CompareResult::Distinct;
        ProvenNotEquivalent = true;
        continue;
      }
      // If they did match, we need to continue scanning elements in the
      // current structure.
      DEBUG_WITH_TYPE(DTRT_VERBOSE,
                      dbgs() << "Element struct member match @ " << i << "\n");
    } else {
      // We've gotten past pointers, arrays, and structures.
      // If we get here it's either a scalar type or a function type.
      // We checked much earlier for the two types being identical, so
      // if we get here with scalar types, it's definitely a mismatch.
      // For now, let's skip the complexity of function type comparison
      // TODO: Support function types if needed.
      DEBUG_WITH_TYPE(DTRT_VERBOSE, dbgs()
                                        << "Element mismatch @ " << i << "\n");
      return CompareResult::Distinct;
    }
  }

  // If we get here, all of the members matched.
  DEBUG_WITH_TYPE(DTRT_VERBOSE,
                  dbgs() << "All members equivalent or compatible.\n");
  if (ProvenNotEquivalent)
    return CompareResult::Compatible;
  return CompareResult::Equivalent;
}

bool dtrans::ResolveTypesPass::runImpl(Module &M, DTransAnalysisInfo &DTInfo,
                                       const TargetLibraryInfo &TLI,
                                       WholeProgramInfo &WPInfo) {
  if (!WPInfo.isWholeProgramSafe())
    return false;

  if (!DTInfo.useDTransAnalysis())
    return false;

  DTransTypeRemapper TypeRemapper;
  ResolveTypesImpl Transformer(DTInfo, M.getContext(), M.getDataLayout(), TLI,
                               &TypeRemapper);
  return Transformer.run(M);
}

PreservedAnalyses dtrans::ResolveTypesPass::run(Module &M,
                                                ModuleAnalysisManager &AM) {
  auto &DTransInfo = AM.getResult<DTransAnalysis>(M);
  auto &TLI = AM.getResult<TargetLibraryAnalysis>(M);
  auto &WPInfo = AM.getResult<WholeProgramAnalysis>(M);

  if (!runImpl(M, DTransInfo, TLI, WPInfo))
    return PreservedAnalyses::all();

  // TODO: Mark the actual preserved analyses.
  PreservedAnalyses PA;
  PA.preserve<WholeProgramAnalysis>();
  return PA;
}
