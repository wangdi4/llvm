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
  void addTypeMapping(StructType *SrcTy, StructType *DestTy);
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
StringRef ResolveTypesImpl::getTypeBaseName(StringRef TyName) {
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

  // Sometimes previous optimizations will have left types that are not
  // used in a way that allows Module::getIndentifiedStructTypes() to find
  // them. This can confuse our mapping algorithm, so here we check for
  // missing types and add them to the set we're looking at.
  std::vector<StructType*> StTypes = M.getIdentifiedStructTypes();
  SmallPtrSet<StructType*, 32> SeenTypes;
  std::function<void(StructType*)> findMissedNestedTypes = [&](StructType *Ty) {
    for (Type *ElemTy : Ty->elements()) {
      // Look past pointer, array, and vector wrappers.
      // If the element is a structure, add it to the SeenTypes set.
      // If it wasn't already there, check for nested types.
      if (StructType* ElemStTy = getContainedStructTy(ElemTy))
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
  bool DuplicateTypeFound = false;
  EquivalenceClasses<StructType *> CompatibleTypes;
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
      if (tryToMapTypes(BaseTy, CandTy, CompatibleTypes)) {
        DuplicateTypeFound = true;
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
          DuplicateTypeFound = true;
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

  // TODO: Use the CompatibleTypes data.

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

    DEBUG_WITH_TYPE(DTRT_VERBOSE,
                    dbgs() << "Populating type: " << *ReplTy << "\n  OrigTy: "
                            << *OrigTy << "\n");

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
                    << *ActualDestTy << "\n");
  TypeRemapper->addTypeMapping(SrcTy, ActualDestTy);
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
