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
#include "llvm/Analysis/Intel_WP.h"
#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Operator.h"
#include "llvm/Pass.h"
#include "llvm/Transforms/IPO.h"
using namespace llvm;

#define DEBUG_TYPE "dtrans-resolvetypes"

#define DTRT_VERBOSE "dtrans-resolvetypes-verbose"

namespace {

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

  void addTypeMapping(StructType *SrcTy, StructType *DestTy);
  bool areTypesEquivalent(StructType *TyA, StructType *TyB);
  bool areTypeMembersEquivalent(StructType *TyA, StructType *TyB,
                                DenseSet<std::pair<Type *, Type *>> &TypesSeen);
  bool typesHaveSameBaseName(StructType *StTyA, StructType *StTyB);
  StringRef getTypeBaseName(StringRef TyName);
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
//
// However, we want to avoid trimming too much in cases like this:
//
//   %struct.B.5.789 = type {...}
//
// And we don't want to attempt to match types like this:
//
//   %struct.CO = type {...}
//   %struct.CO2 = type {...}
//
// So we start by trimming trailing numbers, then look for and trim a
// single '.', and finally check to see if the base name constructed this
// way names a type. If it does, it's a candidate for our transformation.
StringRef ResolveTypesImpl::getTypeBaseName(StringRef TyName) {
  StringRef BaseName = TyName.rtrim("0123456789");
  // If no suffix was found, the size will be unchanged.
  if (TyName.size() == BaseName.size())
    return TyName;
  if (!BaseName.endswith("."))
    return TyName;
  return BaseName.drop_back(1);
}

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

  SmallPtrSet<StructType *, 32> ExternTypes;
  for (Function &F : M) {
    if (F.isDeclaration()) {
      auto *FnTy = F.getFunctionType();
      if (auto *RetST = getContainedStructTy(FnTy->getReturnType()))
        ExternTypes.insert(RetST);
      for (auto *ParamTy : FnTy->params())
        if (auto *ParamST = getContainedStructTy(ParamTy))
          ExternTypes.insert(ParamST);
    }
  }

  std::function<bool(StructType *, SmallPtrSetImpl<StructType *> &)>
      hasExternDependency =
          [&](StructType *Ty, SmallPtrSetImpl<StructType *> &VisitedTypes) {
            if (!VisitedTypes.insert(Ty).second)
              return false;
            if (ExternTypes.count(Ty))
              return true;
            for (auto *DepTy : TypeToDependentTypes[Ty]) {
              if (auto *ST = dyn_cast<StructType>(DepTy)) {
                if (ExternTypes.count(ST))
                  return true;
                if (hasExternDependency(ST, VisitedTypes))
                  return true;
              }
            }
            return false;
          };

  DenseMap<StructType *, SetVector<StructType *>> CandidateTypeSets;
  for (StructType *Ty : M.getIdentifiedStructTypes()) {
    // Ignore unnamed types.
    if (!Ty->hasName())
      continue;

    StringRef TyName = Ty->getName();
    StringRef BaseName = getTypeBaseName(TyName);

    // If the type name didn't have a suffix, TyName and BaseName will be
    // the same. Checking the size is sufficient.
    if (TyName.size() == BaseName.size())
      continue;

    // Check to see if BaseName is a known type.
    StructType *BaseTy = M.getTypeByName(BaseName);
    if (!BaseTy)
      continue;

    // Don't try to remap types with external uses.
    SmallPtrSet<StructType *, 4> VisitedTypes;
    if (hasExternDependency(Ty, VisitedTypes))
      continue;

    DEBUG_WITH_TYPE(DTRT_VERBOSE,
                    dbgs() << "resolve-types: Found candidate type\n  "
                           << TyName << " -> " << BaseName << "\n");
    CandidateTypeSets[BaseTy].insert(Ty);
  }

  if (CandidateTypeSets.empty())
    return false;

  bool DuplicateTypeFound = false;
  for (auto Entry : CandidateTypeSets) {
    StructType *BaseTy = Entry.first;

    // Don't try to remap types with external uses. We didn't check the base
    // type above because we would have needed to check it for each possible
    // variant there whereas here we can check it just once.
    SmallPtrSet<StructType *, 4> VisitedTypes;
    if (hasExternDependency(BaseTy, VisitedTypes))
      continue;

    const SetVector<StructType *> &CandTypes = Entry.second;

    SmallVector<StructType *, 4> Alternates;
    for (auto *CandTy : CandTypes) {
      if (!areTypesEquivalent(BaseTy, CandTy)) {
        DEBUG_WITH_TYPE(DTRT_VERBOSE,
                        dbgs() << "resolve-types: Rejected candidate type.\n"
                               << "    Base: " << *BaseTy << "\n"
                               << "    Cand: " << *CandTy << "\n");
        // This type didn't match the base type, but it may match another
        // type with the same base name that also didn't match the base
        // type.
        bool AltMatchFound = false;
        for (auto *AltTy : Alternates) {
          if (areTypesEquivalent(AltTy, CandTy)) {
            DEBUG_WITH_TYPE(DTRT_VERBOSE,
                            dbgs()
                                << "resolve-types: Found alt duplicate type.\n"
                                << "    Base: " << *AltTy << "\n"
                                << "    Cand: " << *CandTy << "\n");
            addTypeMapping(CandTy, AltTy);
            AltMatchFound = true;
            DuplicateTypeFound = true;
            break;
          }
        }
        // If no match was found, add this to a list of alternate types
        // with this base name to be checked if other types don't match
        // the base type.
        if (!AltMatchFound)
          Alternates.push_back(CandTy);
        continue;
      }
      addTypeMapping(CandTy, BaseTy);
      DuplicateTypeFound = true;
    }
  }
  return DuplicateTypeFound;
}

void ResolveTypesImpl::populateTypes(Module &M) {
  // We will have multiple original types mapped to the same replacement
  // type, so we need to check as we process this map to see if the type
  // already has a body.
  for (auto &ONPair : OrigToNewTypeMapping) {
    StructType *OrigTy = ONPair.first;
    StructType *ReplTy = ONPair.second;

    // If we've already populated the replacement type, go to the next entry.
    if (!ReplTy->isOpaque())
      continue;

    SmallVector<Type *, 8> DataTypes;
    for (auto *MemberTy : OrigTy->elements())
      DataTypes.push_back(TypeRemapper->remapType(MemberTy));

    ReplTy->setBody(DataTypes, OrigTy->isPacked());
    LLVM_DEBUG(dbgs() << "resolve-types: New structure body: " << *ReplTy
                      << "\n");
  }
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
  }
  LLVM_DEBUG(dbgs() << "resolve-types: Mapping " << SrcTy->getName() << " -> "
                    << ActualDestTy->getName() << "\n    " << *SrcTy << "\n    "
                    << *DestTy << "\n");
  TypeRemapper->addTypeMapping(SrcTy, ActualDestTy);
  OrigToNewTypeMapping[SrcTy] = ActualDestTy;
}

bool ResolveTypesImpl::areTypesEquivalent(StructType *TyA, StructType *TyB) {
  // llvm::Type provides a check for trivial equivalence. Try that first.
  if (TyA->isLayoutIdentical(TyB))
    return true;

  // If the layout is not identical, we want to check for layouts that would
  // be identical if we remapped equivalent types.

  // Otherwise, do an element-by-element check for equivalent types.
  DenseSet<std::pair<Type *, Type *>> TypesSeen;
  return areTypeMembersEquivalent(TyA, TyB, TypesSeen);
}

bool ResolveTypesImpl::areTypeMembersEquivalent(
    StructType *TyA, StructType *TyB,
    DenseSet<std::pair<Type *, Type *>> &TypesSeen) {

  DEBUG_WITH_TYPE(DTRT_VERBOSE, {
    if (TyA->hasName() && TyB->hasName())
      dbgs() << "areTypeMembersEquivalent(" << TyA->getName() << ", "
             << TyB->getName() << ")\n";
    else
      dbgs() << "areTypeMembersEquivalent(" << *TyA << ", " << *TyB << ")\n";
  });

  // Different packing rules out our mapping.
  if (TyA->isPacked() != TyB->isPacked()) {
    DEBUG_WITH_TYPE(DTRT_VERBOSE, dbgs() << "Pack mismatch\n");
    return false;
  }

  // Different numbers of elements is another easy indicator.
  if (TyA->getNumElements() != TyB->getNumElements()) {
    DEBUG_WITH_TYPE(DTRT_VERBOSE, dbgs() << "Element count mismatch\n");
    return false;
  }

  // If we've seen this pair before, don't check it again. The previous
  // check may still be in progress, but we can assume a match here and
  // if something reveals a mismatch in the original search the final
  // result will reflect that.
  if (!TypesSeen.insert(std::make_pair(TyA, TyB)).second) {
    DEBUG_WITH_TYPE(DTRT_VERBOSE, dbgs() << "Types seen, assume match\n");
    return true;
  }

  unsigned NumElements = TyA->getNumElements();
  for (unsigned i = 0; i < NumElements; ++i) {
    Type *ElemATy = TyA->getElementType(i);
    Type *ElemBTy = TyB->getElementType(i);

    // We're looking for mismatches. If the types are the same, we can
    // skip the details below.
    if (ElemATy == ElemBTy)
      continue;

    // Look past pointer, array, or vector types to see their element types.
    while (ElemATy->isPointerTy() || ElemATy->isArrayTy() ||
           ElemATy->isVectorTy()) {
      if (ElemATy->isPointerTy()) {
        // If the two types have different levels of indirection, the structs
        // don't match.
        if (!ElemBTy->isPointerTy()) {
          DEBUG_WITH_TYPE(DTRT_VERBOSE, dbgs()
                                            << "Indirection level mismatch @ "
                                            << i << "\n");
          return false;
        }
        // Otherwise, drill down on both pointers.
        ElemATy = ElemATy->getPointerElementType();
        ElemBTy = ElemBTy->getPointerElementType();
      } else {
        assert((ElemATy->isArrayTy() || ElemATy->isVectorTy()) &&
               "Unexpected sequential type!");
        // Both elements must be of the same sequential type.
        if ((ElemATy->isArrayTy() && !ElemBTy->isArrayTy()) ||
            (ElemATy->isVectorTy() && !ElemBTy->isVectorTy())) {
          DEBUG_WITH_TYPE(DTRT_VERBOSE,
                          dbgs() << "Array/vector mismatch @ " << i << "\n");
          return false;
        }
        // Arrays and vectors are handled together this way.
        ElemATy = ElemATy->getSequentialElementType();
        ElemBTy = ElemBTy->getSequentialElementType();
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
        return false;
      }
      // Try the simple equivalence check.
      if (StElemATy->isLayoutIdentical(StElemBTy))
        continue;
      // Otherwise, go to the next level of element-by-element checking.
      // If they don't match, we're finished.
      if (!areTypeMembersEquivalent(StElemATy, StElemBTy, TypesSeen)) {
        DEBUG_WITH_TYPE(DTRT_VERBOSE,
                        dbgs() << "Element member mismatch @ " << i << "\n");
        return false;
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

      return false;
    }
  }

  // If we get here, all of the members matched.
  DEBUG_WITH_TYPE(DTRT_VERBOSE, dbgs() << "All members matched.\n");
  return true;
}

bool ResolveTypesImpl::typesHaveSameBaseName(StructType *StTyA,
                                             StructType *StTyB) {
  if (!StTyA->hasName() && !StTyB->hasName())
    return true;
  if (!StTyA->hasName() || !StTyB->hasName())
    return false;
  return getTypeBaseName(StTyA->getName())
      .equals(getTypeBaseName(StTyB->getName()));
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
