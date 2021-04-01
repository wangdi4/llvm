//===--- DTransOPOptBase.cpp - Base class for DTrans Transforms -----==//
//
// Copyright (C) 2021-2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
//
// This file provides the base classes for DTrans Transformations that provide
// the common functionality needed for rewriting dependent data types and
// functions that change as the result of DTrans modifying a structure
// definition. This is to work with an opaque pointer representation.
//
//===----------------------------------------------------------------------===//

#include "Intel_DTrans/Transforms/DTransOPOptBase.h"

#include "Intel_DTrans/Analysis/DTransOPUtils.h"
#include "Intel_DTrans/Analysis/DTransTypes.h"
#include "Intel_DTrans/Analysis/DTransUtils.h"
#include "Intel_DTrans/Analysis/TypeMetadataReader.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/Debug.h"

#define DEBUG_TYPE "dtransop-optbase"

namespace llvm {
namespace dtransOP {

//===----------------------------------------------------------------------===//
// Implementation for the DTransOPTypeRemapper class methods
//===----------------------------------------------------------------------===//

// Return the new type for 'SrcTy'. If the type is not being changed, 'SrcTy'
// will be returned.
llvm::Type *DTransOPTypeRemapper::remapType(llvm::Type *SrcTy) {
  assert(AllTypeMappingsAdded && "remapType should not be used until all "
                                 "structure mappings have been added");

  llvm::Type *ReplTy = lookupCachedTypeMapping(SrcTy);
  if (!ReplTy) {
    ReplTy = computeReplacementType(SrcTy);

    // If there is no replacement type needed for the input type, we will cache
    // the input type as being the replacement type to avoid the need to
    // reanalyze the type in the future.
    if (!ReplTy)
      ReplTy = SrcTy;

    // Cache the result.
    RemapSrcToDestTypeCache[SrcTy] = ReplTy;
  }

  return ReplTy;
}

// DTransType version of remapType. This will not be called by the ValueMapper,
// but is helpful for the DTransOptBase class, and its derivations, for
// computing the effective type of a type that has dependencies on a type being
// replaced.
DTransType *DTransOPTypeRemapper::remapType(DTransType *SrcTy) {
  assert(AllTypeMappingsAdded && "remapType should not be used until all "
                                 "structure mappings have been added");

  DTransType *ReplTy = lookupCachedTypeMapping(SrcTy);
  if (!ReplTy) {
    ReplTy = computeReplacementType(SrcTy);

    // If there is no replacement type needed for the input type, we will cache
    // the input type as being the replacement type to avoid the need to
    // reanalyze the type in the future.
    if (!ReplTy)
      ReplTy = SrcTy;

    // Cache the result.
    DTransRemapSrcToDestTypeCache[SrcTy] = ReplTy;
  }

  return ReplTy;
}

// Used by clients to set up the initial set of structure types that need
// be remapped.
// \param SrcTy The original structure type
// \param DestTy The new structure type to remap to
// \param DTSrcTy The original structure type with the DTransType representation
// \param DTDestTy The new structure type with the DTransType representation
void DTransOPTypeRemapper::addTypeMapping(llvm::Type *SrcTy, llvm::Type *DestTy,
                                          DTransType *DTSrcTy,
                                          DTransType *DTDestTy) {
  assert(SrcTy && DestTy && DTSrcTy && DTDestTy &&
         "Null types are not permitted");
  assert(!AllTypeMappingsAdded &&
         "Cannot add additional types after setting AllTypeMappingsAdded");
  assert(!hasRemappedType(SrcTy) &&
         "DTransOPTypeRemapper already contains mapping for type");
  assert(!hasRemappedType(DTSrcTy) &&
         "DTransOPTypeRemapper already contains mapping for DTrans type");

  SrcTypeToNewType[SrcTy] = DestTy;
  DTransSrcTypeToNewType[DTSrcTy] = DTDestTy;
}

// Return 'true' if 'SrcTy' is contained in the map of types to be changed.
bool DTransOPTypeRemapper::hasRemappedType(llvm::Type *SrcTy) const {
  return SrcTypeToNewType.count(SrcTy);
}

// The 'DTransType' version of 'hasRemappedType()'.
bool DTransOPTypeRemapper::hasRemappedType(DTransType *SrcTy) const {
  return DTransSrcTypeToNewType.count(SrcTy);
}

// Return the type mapping for 'SrcTy', if there is one. If there is
// not one, return nullptr.
llvm::Type *DTransOPTypeRemapper::lookupTypeMapping(llvm::Type *SrcTy) const {
  auto It = SrcTypeToNewType.find(SrcTy);
  if (It != SrcTypeToNewType.end())
    return It->second;

  return nullptr;
}

// The 'DTransType' version of 'lookupTypeMapping()'.
DTransType *DTransOPTypeRemapper::lookupTypeMapping(DTransType *SrcTy) const {
  auto It = DTransSrcTypeToNewType.find(SrcTy);
  if (It != DTransSrcTypeToNewType.end())
    return It->second;

  return nullptr;
}

// Return a previously computed type mapping for 'SrcTy', if it's been
// previously evaluated. Otherwise, nullptr.
llvm::Type *
DTransOPTypeRemapper::lookupCachedTypeMapping(llvm::Type *SrcTy) const {
  auto It = RemapSrcToDestTypeCache.find(SrcTy);
  if (It != RemapSrcToDestTypeCache.end())
    return It->second;

  return nullptr;
}

// The 'DTransType' version of 'lookupCachedTypeMapping()'.
DTransType *
DTransOPTypeRemapper::lookupCachedTypeMapping(DTransType *SrcTy) const {
  auto It = DTransRemapSrcToDestTypeCache.find(SrcTy);
  if (It != DTransRemapSrcToDestTypeCache.end())
    return It->second;

  return nullptr;
}

// If the 'SrcTy' type needs to be replaced, return the replacement type.
// Otherwise, return nullptr.
//
// This routine walks pointer, functions, array and vector types to compute
// replacement types.  It does not walk structure bodies, as the ones that need
// replacing should be populated in the type mapping table via addTypeMapping()
// by the transformation pass before this function is used. PointerTypes will
// only be examined when this class is constructed with the 'UsingOpaquePtrs'
// flag configured to 'false'.
llvm::Type *
DTransOPTypeRemapper::computeReplacementType(llvm::Type *SrcTy) const {
  // Check if the answer is already computed or if this is a basic structure
  // type that has been directly added to the type mapping table.
  llvm::Type *CurMapping = lookupTypeMapping(SrcTy);
  if (CurMapping)
    return CurMapping;

  if (SrcTy->isPointerTy() && !UsingOpaquePtrs) {
    Type *ReplTy = computeReplacementType(SrcTy->getPointerElementType());
    if (!ReplTy)
      return nullptr;
    return ReplTy->getPointerTo();
  }

  if (SrcTy->isArrayTy()) {
    Type *ReplTy = computeReplacementType(SrcTy->getArrayElementType());
    if (!ReplTy)
      return nullptr;
    return ArrayType::get(ReplTy, SrcTy->getArrayNumElements());
  }

  if (SrcTy->isVectorTy()) {
    Type *ReplTy =
        computeReplacementType(cast<VectorType>(SrcTy)->getElementType());
    if (!ReplTy)
      return nullptr;
    return FixedVectorType::get(ReplTy,
                                cast<VectorType>(SrcTy)->getNumElements());
  }

  if (auto *FunctionTy = dyn_cast<FunctionType>(SrcTy)) {
    SmallVector<Type *, 8> DataTypes;

    bool NeedsReplaced = false;
    Type *RetTy = FunctionTy->getReturnType();
    Type *ReplRetTy = RetTy;
    Type *ReplTy = computeReplacementType(RetTy);
    if (ReplTy) {
      ReplRetTy = ReplTy;
      NeedsReplaced = true;
    }
    unsigned Total = FunctionTy->getNumParams();
    for (unsigned Idx = 0; Idx < Total; ++Idx) {
      Type *ParmTy = FunctionTy->getParamType(Idx);
      Type *ReplParmTy = ParmTy;
      Type *ReplTy = computeReplacementType(ParmTy);
      if (ReplTy) {
        ReplParmTy = ReplTy;
        NeedsReplaced = true;
      }

      DataTypes.push_back(ReplParmTy);
    }

    if (NeedsReplaced)
      return FunctionType::get(ReplRetTy, makeArrayRef(DataTypes),
                               FunctionTy->isVarArg());
  }

  if (auto *StructTy = dyn_cast<StructType>(SrcTy)) {
    if (StructTy->isLiteral()) {
      bool NeedsReplaced = false;

      SmallVector<Type *, 8> DataTypes;
      for (auto *MemberTy : StructTy->elements()) {
        Type *ReplMemTy = MemberTy;
        Type *ReplTy = computeReplacementType(MemberTy);
        if (ReplTy) {
          ReplMemTy = ReplTy;
          NeedsReplaced = true;
        }

        DataTypes.push_back(ReplMemTy);
      }

      if (NeedsReplaced)
        return StructType::get(StructTy->getContext(), DataTypes,
                               StructTy->isPacked());
    }
  }

  // Note: We do not look for named 'struct' types directly in the above tests
  // because those should have been directly added to the type mapping via
  // AddTypeMapping by the transformation.

  // Type does not need type replacement.
  return nullptr;
}

// The 'DTransType' version of 'computeReplacementType()'.
// Note, for DTransTypes, pointer types are always examined to compute the
// effective replacement type.
DTransType *
DTransOPTypeRemapper::computeReplacementType(DTransType *SrcTy) const {
  // Check if the answer is already computed or if this is a basic structure
  // type that has been directly added to the type mapping table.
  DTransType *CurMapping = lookupTypeMapping(SrcTy);
  if (CurMapping)
    return CurMapping;

  if (SrcTy->isPointerTy()) {
    DTransType *ReplTy = computeReplacementType(SrcTy->getPointerElementType());
    if (!ReplTy)
      return nullptr;
    return TM.getOrCreatePointerType(ReplTy);
  }

  if (auto *DTArType = dyn_cast<DTransArrayType>(SrcTy)) {
    DTransType *ReplTy =
        computeReplacementType(DTArType->getArrayElementType());
    if (!ReplTy)
      return nullptr;
    return DTransArrayType::get(TM, ReplTy, DTArType->getNumElements());
  }

  if (auto *DTVecType = dyn_cast<DTransVectorType>(SrcTy)) {
    DTransType *ReplTy = computeReplacementType(DTVecType->getElementType());
    if (!ReplTy)
      return nullptr;
    return DTransVectorType::get(TM, ReplTy, DTVecType->getNumElements());
  }

  if (auto *FunctionTy = dyn_cast<DTransFunctionType>(SrcTy)) {
    SmallVector<DTransType *, 8> DataTypes;

    bool NeedsReplaced = false;
    DTransType *RetTy = FunctionTy->getReturnType();
    DTransType *ReplRetTy = RetTy;
    DTransType *ReplTy = computeReplacementType(RetTy);
    if (ReplTy) {
      ReplRetTy = ReplTy;
      NeedsReplaced = true;
    }
    unsigned Total = FunctionTy->getNumArgs();
    for (unsigned Idx = 0; Idx < Total; ++Idx) {
      DTransType *ParmTy = FunctionTy->getArgType(Idx);
      DTransType *ReplParmTy = ParmTy;
      DTransType *ReplTy = computeReplacementType(ParmTy);
      if (ReplTy) {
        ReplParmTy = ReplTy;
        NeedsReplaced = true;
      }

      DataTypes.push_back(ReplParmTy);
    }

    if (NeedsReplaced)
      return DTransFunctionType::get(TM, ReplRetTy, DataTypes,
                                     FunctionTy->isVarArg());
  }

  if (auto *StructTy = dyn_cast<DTransStructType>(SrcTy)) {
    if (StructTy->isLiteralStruct()) {
      bool NeedsReplaced = false;

      SmallVector<DTransType *, 8> DataTypes;
      for (auto &FieldMember : StructTy->elements()) {
        DTransType *ReplMemTy = FieldMember.getType();
        assert(ReplMemTy && "Metadata reader had ambiguous types");
        DTransType *ReplTy = computeReplacementType(ReplMemTy);
        if (ReplTy) {
          ReplMemTy = ReplTy;
          NeedsReplaced = true;
        }

        DataTypes.push_back(ReplMemTy);
      }

      if (NeedsReplaced)
        return TM.getOrCreateLiteralStructType(StructTy->getContext(),
                                               DataTypes);
    }
  }

  // Note: We do not look for named 'struct' types directly in the above tests
  // because those should have been directly added to the type mapping via
  // AddTypeMapping by the transformation.

  // Type does not need type replacement.
  return nullptr;
}

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
void DTransOPTypeRemapper::dump() const {
  dbgs() << "DTransOPTypeRemapper LLVM Type Mappings:\n";
  for (auto &KV : SrcTypeToNewType)
    dbgs() << "  " << *(KV.first) << " -> " << *(KV.second) << "\n";
  dbgs() << "End of DTransOPTypeRemapper LLVM Type Mappings\n";

  dbgs() << "\nDTransOPTypeRemapper DTrans Type Mappings:\n";
  for (auto &KV : DTransSrcTypeToNewType) {
    dbgs() << "  ";
    KV.first->print(dbgs(), true);
    dbgs() << " -> ";
    KV.second->print(dbgs(), true);
    dbgs() << "\n";
  }
  dbgs() << "End DTransOPTypeRemapper DTrans Type Mappings\n";

  dbgs() << "\nDTransOPTypeRemapper LLVM Type Cached Types:\n";
  for (auto &KV : RemapSrcToDestTypeCache)
    dbgs() << "  " << *(KV.first) << " -> " << *(KV.second) << "\n";
  dbgs() << "End of DTransOPTypeRemapper LLVM Type Cached Types\n";

  dbgs() << "\nDTransOPTypeRemapper DTrans Type Cached Types:\n";
  for (auto &KV : DTransRemapSrcToDestTypeCache) {
    dbgs() << "  ";
    KV.first->print(dbgs(), true);
    dbgs() << " -> ";
    KV.second->print(dbgs(), true);
    dbgs() << "\n";
  }
  dbgs() << "End of DTransOPTypeRemapper DTrans Type Cached Types\n";
}
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

//===----------------------------------------------------------------------===//
// Implementation for the DTransOPOptBase class methods
//===----------------------------------------------------------------------===//

DTransOPOptBase::DTransOPOptBase(LLVMContext &Ctx, DTransTypeManager &TM,
                                 StringRef DepTypePrefix)
    : TM(TM), DepTypePrefix(DepTypePrefix),
      UsingOpaquePtrs(areOpaquePtrsEnabled(Ctx)),
      TypeRemapper(TM, UsingOpaquePtrs) {}

bool DTransOPOptBase::run(Module &M) {
  if (!prepareTypesBaseImpl(M))
    return false;

  LLVM_DEBUG({
    dbgs() << "\nTypeRemapper types after preparing types:\n";
    TypeRemapper.dump();
  });

  ValueMapper Mapper(VMap, RF_IgnoreMissingLocals, &TypeRemapper,
                     nullptr /*Materializer*/);

  updateDTransTypesMetadata(M, Mapper);

  // TODO: Implement the calls to perform the function transformation.
  return true;
}

// Identify and create new types for any types the child class is going
// to replace.
bool DTransOPOptBase::prepareTypesBaseImpl(Module &M) {
  KnownStructTypes = TM.getIdentifiedStructTypes();

  // Compute the set of types that each type is a dependee of. This will
  // enable the determination of other types that need to be rewritten when one
  // type changes.
  buildTypeDependencyMapping();

  // Invoke the derived class to populate the TypeRemapper with any types
  // the transformation is going to directly transform.
  bool Changed = prepareTypes(M);
  if (!Changed)
    return false;

  // Identify and create new types for types dependent on changes made by the
  // derived class. This map holds the original type to the replacement type
  // that needs to be populated once all types are identified.
  LLVMTypeToTypeMap OrigToNewTypeReplacement;
  prepareDependentTypes(M, OrigToNewTypeReplacement);

  // Inform the TypeRemapper that all types have been created, so that
  // populateDependentTypes can use the mapper to compute the types to use when
  // populating the structure bodies.
  TypeRemapper.setAllTypeMappingsAdded();

  populateDependentTypes(M, OrigToNewTypeReplacement);

  // Invoke the derived class to populate the bodies of types the transformation
  // is creating.
  populateTypes(M);

  return true;
}

// Identify and create types that need to be remapped because of an
// existing type that contains a reference to a type being changed by
// the transformation.
//
// For each type, compute the set of structure types that either directly or
// indirectly use the type. For example:
//   struct.test01 = { i32, i32 }
//   struct.test02 = { struct.test01, struct.test02* }
//   struct.test03 = { struct.test01*, struct.test02* }
//
// struct.test01 is directly used by struct.test02 and indirectly used by
// struct.test03.
// struct.test02 is indirectly used by struct.test03.
// struct.test03 is not used by other types.
//
// The map created will be used for populating the list of additional types that
// may need to be converted when changing a specific type.
void DTransOPOptBase::buildTypeDependencyMapping() {
  for (auto *StTy : KnownStructTypes)
    collectDependenciesForType(StTy);

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  LLVM_DEBUG({
    dumpTypeToTypeSetMapping("\nType dependency direct mapping table:",
                             TypeToDirectDependentTypes);
    dumpTypeToTypeSetMapping("\nType dependency pointer mapping table:",
                             TypeToPtrDependentTypes);
  });
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
}

// Collect the dependency mapping information by analyzing the body of
// 'Ty' to add 'Ty' to the dependency sets for each type contained within it.
// For example, processing the structure definition:
//   %struct.test08b = type { i32, %struct.test08c, void (%struct.test08a*)* }
// will result in the sets maintained for 'struct.test08c' and
// 'struct.test08a' containing 'struct.test08b' as a type that may need to be
// replaced if either of those types are replaced. (When opaque pointers are not
// used, changing either type will cause 'struct.test08b' to need to be changed.
// When opaque pointers are used, only changes to 'struct.test08c' would require
// 'struct.test08b' to be changed.
void DTransOPOptBase::collectDependenciesForType(DTransStructType *StructTy) {

  // Add a 'Dependee' to the set of types tracked for the 'Depender'.
  // 'IsPtrDep' controls whether it is a direct dependency (nested relationship)
  // or indirect dependency (pointer relationship).
  auto UpdateTypeToDependentTypeMap =
      [this](DTransType *Depender, DTransType *Dependee, bool IsPtrDep) {
        if (Depender == Dependee)
          return;

        if (!Dependee->isAggregateType() || !Depender->isAggregateType())
          return;

        LLVM_DEBUG({
          dbgs() << "DTRANS-OPTBASE: Type dependency: ";
          Depender->print(dbgs(), false);
          dbgs() << " used by ";
          Dependee->print(dbgs(), false);
          dbgs() << (IsPtrDep ? " [pointer]" : " [nesting]") << "\n";
        });

        // Pointer dependencies affect the description of the types in metadata.
        //    %struct.ptr_dep = type { p0 } ; Where p0 represents %struct.foo*
        // Direct dependencies affect both the type in IR and the description of
        // the type in metadata.
        //    %struct.outer = type { %struct.inner }
        if (IsPtrDep)
          TypeToPtrDependentTypes[Depender].insert(Dependee);
        else
          TypeToDirectDependentTypes[Depender].insert(Dependee);
      };

  // Update either the TypeToPtrDependentTypes or TypeToDirectDependentTypes set
  // to contain 'Dependee' as a member of the base type of 'Ty' based on the
  // relationship between the types. Examples:
  //  struct.B = type { struct.A }
  //    -> struct.B is a direct dependee of struct.A
  //  struct.B = type { [ 4 x struct.A* ] }
  //    -> struct.B is an indirect dependee of struct.A
  //  struct.B = type { struct A*(struct C*)* }
  //    -> struct.B is an indirect dependee of struct.A and struct.C
  //
  std::function<void(DTransType *, DTransType *)> AddDependentTypeEntry =
      [&UpdateTypeToDependentTypeMap,
       &AddDependentTypeEntry](DTransType *Ty, DTransType *Dependee) -> void {
    // If the dependent type is a literal struct, add the members of the
    // literal struct, rather than the literal struct itself.
    if (auto *StTy = dyn_cast<DTransStructType>(Ty)) {
      if (StTy->isLiteralStruct()) {
        for (auto &FieldMember : StTy->elements()) {
          DTransType *FieldTy = FieldMember.getType();
          AddDependentTypeEntry(FieldTy, Dependee);
        }
        return;
      }
    }

    if (auto *FuncTy = dyn_cast<DTransFunctionType>(Ty)) {
      DTransType *RetTy = FuncTy->getReturnType();
      assert(RetTy && "Incomplete function type");
      AddDependentTypeEntry(RetTy, Dependee);

      unsigned NumParams = FuncTy->getNumArgs();
      for (unsigned Idx = 0; Idx < NumParams; ++Idx) {
        DTransType *ArgTy = FuncTy->getArgType(Idx);
        assert(ArgTy && "Incomplete function type");
        AddDependentTypeEntry(ArgTy, Dependee);
      }

      return;
    }

    if (hasPointerType(Ty)) {
      DTransType *BaseTy = unwrapDTransType(Ty);
      if (auto *FunctionTy = dyn_cast<DTransFunctionType>(BaseTy))
        AddDependentTypeEntry(FunctionTy, Dependee);
      else
        UpdateTypeToDependentTypeMap(BaseTy, Dependee, /*IsPtrDep=*/true);
    } else {
      DTransType *BaseTy = unwrapDTransType(Ty);
      UpdateTypeToDependentTypeMap(BaseTy, Dependee, /*IsPtrDep=*/false);
    }
  };

  for (auto &FieldMember : StructTy->elements()) {
    DTransType *FieldTy = FieldMember.getType();
    assert(FieldTy && "Metadata for structure has ambiguous types");
    AddDependentTypeEntry(FieldTy, StructTy);
  }
}

// Identify and create opaque types.
void DTransOPOptBase::prepareDependentTypes(
    Module &M, LLVMTypeToTypeMap &OrigToNewTypeReplacement) {
  LLVM_DEBUG(dbgs() << "\nDTrans-OptBase: Preparing dependent types\n");

  // Build a worklist of types that are going to need to be replaced based
  // on the types the transform is remapping. At this point the TypeRemapper
  // will only contain those types.
  SmallSetVector<DTransType *, 16> Worklist;
  SmallPtrSet<DTransType *, 16> ChildTransforming;
  for (DTransType *OrigTy : KnownStructTypes) {
    if (TypeRemapper.hasRemappedType(OrigTy->getLLVMType())) {
      Worklist.insert(OrigTy);
      ChildTransforming.insert(OrigTy);
    }
  }

  // Find all the type dependencies that will need to be replaced as a result of
  // the types the child class is changing.
  SmallPtrSet<DTransType *, 16> Processed;
  while (!Worklist.empty()) {
    DTransType *Ty = Worklist.pop_back_val();
    if (Processed.count(Ty))
      continue;

    // Add all dependent types that have not been processed yet to the worklist
    if (TypeToDirectDependentTypes.count(Ty)) {
      for (auto &Depends : TypeToDirectDependentTypes[Ty])
        if (!Processed.count(Depends))
          Worklist.insert(Depends);
    }
    if (!UsingOpaquePtrs && TypeToPtrDependentTypes.count(Ty)) {
      for (auto &Depends : TypeToPtrDependentTypes[Ty])
        if (!Processed.count(Depends))
          Worklist.insert(Depends);
    }

    Processed.insert(Ty);
    if (ChildTransforming.count(Ty))
      continue;

    // We need to create an opaque structure type here for the replacement, and
    // add it to the TypeRemapper. This will enable the types for the new
    // structure bodies to be computed after all the structure replacements have
    // been determined. For other types, such as arrays, we don't need to do
    // anything here, the replacements for them will be computed on demand.
    llvm::Type *LLVMTy = Ty->getLLVMType();
    if (auto *StructTy = dyn_cast<llvm::StructType>(LLVMTy)) {
      // If StructTy is literal, defer creation until it is used.
      if (StructTy->isLiteral())
        continue;

      llvm::StructType *ReplacementTy = StructType::create(
          StructTy->getContext(),
          Twine(DepTypePrefix + StructTy->getStructName()).str());
      DTransType *DTransReplacementTy = TM.getOrCreateStructType(ReplacementTy);
      TypeRemapper.addTypeMapping(StructTy, ReplacementTy, Ty,
                                  DTransReplacementTy);
      OrigToNewTypeReplacement[StructTy] = ReplacementTy;

      LLVM_DEBUG(dbgs() << "DTrans-OptBase: New type created: "
                        << *ReplacementTy << " as replacement for " << *Ty
                        << "\n");
    }
  }
}

void DTransOPOptBase::populateDependentTypes(
    Module &M, const LLVMTypeToTypeMap &DependentTypeMapping) {
  LLVM_DEBUG(dbgs() << "\nDTrans-OptBase: Populating dependent types:\n");

  for (auto &ONPair : DependentTypeMapping) {
    Type *OrigTy = ONPair.first;
    Type *ReplTy = ONPair.second;
    if (auto *StructTy = dyn_cast<StructType>(OrigTy)) {
      if (StructTy->isOpaque())
        continue;

      SmallVector<Type *, 8> DataTypes;
      for (auto *MemberTy : StructTy->elements())
        DataTypes.push_back(TypeRemapper.remapType(MemberTy));

      StructType *ReplStructTy = cast<StructType>(ReplTy);
      ReplStructTy->setBody(DataTypes, StructTy->isPacked());

      LLVM_DEBUG(dbgs() << "DTRANS-OPTBASE: New LLVM structure body: "
                        << *ReplStructTy << "\n");

      // Set the body for the new DTransStructType that will be used to generate
      // the new metadata information.
      DTransStructType *DTOrigTy = TM.getStructType(StructTy->getName());
      assert(DTOrigTy && "Expected original DTrans type to have been created");
      DTransStructType *DTReplTy = TM.getStructType(ReplStructTy->getName());
      assert(DTReplTy &&
             "Expected replacement DTrans type to have been created");
      assert(DTReplTy->isOpaque() &&
             "Expected replacement to not have fields yet");

      SmallVector<DTransType *, 8> DTransDataTypes;
      for (auto &FieldMember : DTOrigTy->elements()) {
        DTransType *FieldTy = FieldMember.getType();
        assert(FieldTy && "Metadata reader had ambiguous types");
        DTransType *ReplFieldTy = TypeRemapper.remapType(FieldTy);
        assert(ReplFieldTy && "Failed to create field replacement type");
        DTransDataTypes.push_back(ReplFieldTy);
      }
      DTReplTy->setBody(DTransDataTypes);

      LLVM_DEBUG(dbgs() << "DTRANS-OPTBASE: New DTrans structure body: "
                        << *DTReplTy << "\n";);
    }
  }
}

void DTransOPOptBase::updateDTransTypesMetadata(Module &M,
                                                ValueMapper &Mapper) {
  NamedMDNode *DTMDTypes = TypeMetadataReader::getDTransTypesMetadata(M);
  if (!DTMDTypes)
    return;

  SmallVector<MDNode *, 32> Remaps;
  if (DTMDTypes) {
    for (MDNode *Op : DTMDTypes->operands()) {
      assert(Op->getNumOperands() >= 2 && "Invalid metadata operand");
      auto *TypeMD = dyn_cast<ConstantAsMetadata>(Op->getOperand(1));
      assert(TypeMD && isa<llvm::StructType>(TypeMD->getType()) &&
             "Expected struct type");
      llvm::StructType *StTy = cast<llvm::StructType>(TypeMD->getType());
      if (!TypeRemapper.hasRemappedType(StTy))
        Remaps.emplace_back(Mapper.mapMDNode(*Op));
      else {
        // Create metadata encoding.
        llvm::Type *ReplTy = TypeRemapper.remapType(StTy);
        if (!ReplTy->isStructTy())
          continue;

        StructType *ReplStructTy = cast<StructType>(ReplTy);
        DTransStructType *DTReplTy = TM.getStructType(ReplStructTy->getName());
        Remaps.emplace_back(DTReplTy->createMetadataStructureDescriptor());
      }
    }
    DTMDTypes->clearOperands();
    for (auto *M : Remaps)
      DTMDTypes->addOperand(M);
  }
}

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
// Print the table of type dependences
void DTransOPOptBase::dumpTypeToTypeSetMapping(
    StringRef Header, DTransTypeToTypeSetMap &TypeToDependentTypes) {

  auto DTransTypeToString = [](DTransType *Ty) {
    std::string OutputVal;
    raw_string_ostream OutputStream(OutputVal);
    Ty->print(OutputStream, false);
    OutputStream.flush();
    return OutputVal;
  };

  dbgs() << Header << "\n";
  for (auto &TySetTyPair : TypeToDependentTypes) {
    DTransType *Ty = TySetTyPair.first;
    Ty->print(dbgs(), false);
    dbgs() << ": ";

    dtrans::printCollectionSorted(dbgs(), TySetTyPair.second.begin(),
                                  TySetTyPair.second.end(), ", ",
                                  DTransTypeToString);
    dbgs() << "\n";
  }
}
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

} // namespace dtransOP
} // namespace llvm
