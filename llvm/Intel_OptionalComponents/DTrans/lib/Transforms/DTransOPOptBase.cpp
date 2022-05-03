//===--- DTransOPOptBase.cpp - Base class for DTrans Transforms -----==//
//
// Copyright (C) 2021-2022 Intel Corporation. All rights reserved.
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

#include "Intel_DTrans/Analysis/DTrans.h"
#include "Intel_DTrans/Analysis/DTransOPUtils.h"
#include "Intel_DTrans/Analysis/DTransSafetyAnalyzer.h"
#include "Intel_DTrans/Analysis/DTransTypeMetadataBuilder.h"
#include "Intel_DTrans/Analysis/DTransTypes.h"
#include "Intel_DTrans/Analysis/DTransUtils.h"
#include "Intel_DTrans/Analysis/PtrTypeAnalyzer.h"
#include "Intel_DTrans/Analysis/TypeMetadataReader.h"
#include "llvm/IR/DebugInfo.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Transforms/Utils/Cloning.h"

#define DEBUG_TYPE "dtransop-optbase"

namespace llvm {

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
// This option is used during testing to allow changes of types for
// function declarations.
static cl::opt<bool> DTransOPOptBaseProcessFuncDecl(
    "dtransop-optbase-process-function-declaration", cl::init(false),
    cl::ReallyHidden);
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

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
    auto *SrcVecTy = cast<FixedVectorType>(SrcTy);
    Type *ReplTy = computeReplacementType(SrcVecTy->getElementType());
    if (!ReplTy)
      return nullptr;
    return FixedVectorType::get(ReplTy, SrcVecTy->getNumElements());
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

    for (Type *ParamTy : FunctionTy->params()) {
      Type *ReplParmTy = ParamTy;
      Type *ReplTy = computeReplacementType(ParamTy);
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

    for (auto *ArgTy : FunctionTy->args()) {
      assert(ArgTy && "Incomplete function type");
      DTransType *ReplArgTy = ArgTy;
      DTransType *ReplTy = computeReplacementType(ArgTy);
      if (ReplTy) {
        ReplArgTy = ReplTy;
        NeedsReplaced = true;
      }

      DataTypes.push_back(ReplArgTy);
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

DTransOPOptBase::DTransOPOptBase(LLVMContext &Ctx, DTransSafetyInfo *DTInfo,
                                 StringRef DepTypePrefix)
    : DTInfo(DTInfo), TM(DTInfo->getTypeManager()),
      DepTypePrefix(DepTypePrefix),
      UsingOpaquePtrs(!Ctx.supportsTypedPointers()),
      TypeRemapper(TM, UsingOpaquePtrs) {}

bool DTransOPOptBase::run(Module &M) {
  if (!prepareTypesBaseImpl(M))
    return false;

  LLVM_DEBUG({
    dbgs() << "\nTypeRemapper types after preparing types:\n";
    TypeRemapper.dump();
  });

  ValueMapper Mapper(VMap, RF_IgnoreMissingLocals, &TypeRemapper,
                     getMaterializer());

  updateDTransTypesMetadata(M, Mapper);

  // Identify and clone any function prototypes for functions that will need
  // to be cloned because of function signature changes due to type changing.
  // This needs to be done before converting global variables to handle any
  // initializers of the variables that refer to a function address.
  createCloneFunctionDeclarations(M);

  // Let the derived class do any work needed before variable and IR
  // transformations begin.
  prepareModule(M);

  // Remap global variables that have type changes to their new types.
  convertGlobalVariables(M, Mapper);

  // Transform all the functions.
  transformIR(M, Mapper);

  // Cleanup the IR for functions and variables that have been replaced.
  removeDeadValues();
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

      for (auto *ArgTy : FuncTy->args()) {
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

    if (TypeToPtrDependentTypes.count(Ty)) {
      // Normally, opaque pointer dependent types do not need to be remapped
      // because remapping an opaque pointer of type 'ptr' will still produce
      // the type 'ptr'. However, if a pointer type is being remapped to a
      // non-pointer type, then dependent types will need to be remapped.
      bool RewritePointerDependentTypes = !UsingOpaquePtrs;
      DTransType *PtrTy = TM.getOrCreatePointerType(Ty);
      DTransType *ReplTy = TypeRemapper.lookupTypeMapping(PtrTy);
      if (ReplTy && !ReplTy->isPointerTy())
        RewritePointerDependentTypes = true;

      if (RewritePointerDependentTypes)
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

      // Set the body for the new DTransStructType that will be used to generate
      // the new metadata information.
      StructType *ReplStructTy = cast<StructType>(ReplTy);
      DTransStructType *DTOrigTy = TM.getStructType(StructTy->getName());
      assert(DTOrigTy && "Expected original DTrans type to have been created");
      DTransStructType *DTReplTy = TM.getStructType(ReplStructTy->getName());
      assert(DTReplTy &&
             "Expected replacement DTrans type to have been created");
      assert(DTReplTy->isOpaque() &&
             "Expected replacement to not have fields yet");

      SmallVector<Type *, 8> DataTypes;
      SmallVector<DTransType *, 8> DTransDataTypes;
      for (auto &FieldMember : DTOrigTy->elements()) {
        DTransType *FieldTy = FieldMember.getType();
        assert(FieldTy && "Metadata reader had ambiguous types");
        DTransType *ReplFieldTy = TypeRemapper.remapType(FieldTy);
        assert(ReplFieldTy && "Failed to create field replacement type");
        DTransDataTypes.push_back(ReplFieldTy);
        DataTypes.push_back(ReplFieldTy->getLLVMType());
      }
      DTReplTy->setBody(DTransDataTypes);
      ReplStructTy->setBody(DataTypes, StructTy->isPacked());

      LLVM_DEBUG(dbgs() << "DTRANS-OPTBASE: New LLVM structure body: "
                        << *ReplStructTy << "\n");

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
      if (!TypeRemapper.hasRemappedType(StTy)) {
        Remaps.emplace_back(Mapper.mapMDNode(*Op));
      } else {
        // Create metadata encoding.
        llvm::Type *ReplTy = TypeRemapper.remapType(StTy);
        if (!ReplTy->isStructTy())
          continue;

        StructType *ReplStructTy = cast<StructType>(ReplTy);
        DTransStructType *DTReplTy = TM.getStructType(ReplStructTy->getName());
        assert(DTReplTy && "Expected base class or transformation to create "
                           "DTransStructType for remapped structure");
        Remaps.emplace_back(DTReplTy->createMetadataStructureDescriptor());
      }
    }
    DTMDTypes->clearOperands();
    for (auto *M : Remaps)
      DTMDTypes->addOperand(M);
  }
}

// Identify and create new function prototypes for dependent functions
void DTransOPOptBase::createCloneFunctionDeclarations(Module &M) {
  LLVM_DEBUG(dbgs() << "\nDTransOP-OptBase: Identifying functions to clone:\n");

  // Create a work list of all the function definitions that need to be
  // considered for cloning.
  std::vector<Function *> WL;
  for (auto &F : M)
    if (!F.isDeclaration())
      WL.push_back(&F);
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
    else if (DTransOPOptBaseProcessFuncDecl)
      WL.push_back(&F);
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

  for (auto *F : WL) {
    // If the function signature changes as a result of the type remapping,
    // then a clone will be necessary.
    Type *FuncValueTy = F->getValueType();
    Type *FuncReplValueTy = TypeRemapper.remapType(FuncValueTy);
    if (FuncReplValueTy == FuncValueTy) {
      // To support transformations that convert an opaque pointer type to an
      // integer, this needs to examine the function signature using the
      // DTransType representation to determine whether it takes or returns a
      // pointer that would be impacted.
      DTransType *DTy = DTInfo->getTypeMetadataReader().getDTransTypeFromMD(F);
      if (!DTy)
        continue;

      DTransType *DReplTy = TypeRemapper.remapType(DTy);
      if (DTy == DReplTy)
        continue;

      auto *DFnTy = cast<DTransFunctionType>(DTy);
      auto *DReplFnTy = cast<DTransFunctionType>(DReplTy);
      bool NeedsReplaced = false;
      DTransType *OrigRetTy = DFnTy->getReturnType();
      DTransType *ReplRetTy = DReplFnTy->getReturnType();
      assert(OrigRetTy && ReplRetTy && "Bad function sig");
      if (OrigRetTy->isPointerTy() != ReplRetTy->isPointerTy() ||
          (!OrigRetTy->isPointerTy() && OrigRetTy != ReplRetTy)) {
        NeedsReplaced = true;
      } else {
        unsigned NumArgs = F->arg_size();
        for (unsigned ArgIdx = 0; ArgIdx < NumArgs; ++ArgIdx) {
          DTransType *OrigArgTy = DFnTy->getArgType(ArgIdx);
          DTransType *ReplArgTy = DReplFnTy->getArgType(ArgIdx);
          assert(OrigArgTy && ReplArgTy && "Bad function sig");
          if (OrigArgTy->isPointerTy() != ReplArgTy->isPointerTy() ||
              (!OrigArgTy->isPointerTy() && OrigArgTy != ReplArgTy)) {
            NeedsReplaced = true;
            break;
          }
        }
      }
      if (NeedsReplaced)
        FuncReplValueTy = DReplTy->getLLVMType();
    }

    if (FuncReplValueTy != FuncValueTy) {
      Function *NewF = Function::Create(cast<FunctionType>(FuncReplValueTy),
                                        F->getLinkage(), F->getName(), &M);
      NewF->copyAttributesFrom(F);
      VMap[F] = NewF;
      OrigFuncToCloneFuncMap[F] = NewF;
      CloneFuncToOrigFuncMap[NewF] = F;

      // Create VMap entries for the arguments that will be used during the
      // call to cloneFunctionInfo. This must be done to make it available
      // in the call to cloneFunctionInto made by the transformIR function.
      Function::arg_iterator DestI = NewF->arg_begin();
      for (Argument &I : F->args()) {
        DestI->setName(I.getName());
        VMap[&I] = &*DestI++;
      }

      LLVM_DEBUG(dbgs() << "DTransOP-OptBase: Will clone: " << F->getName()
                        << " " << *F->getType() << " into: " << NewF->getName()
                        << " " << *NewF->getType() << "\n");
    }
  }
}

// Remap global variables to new types.
void DTransOPOptBase::convertGlobalVariables(Module &M, ValueMapper &Mapper) {
  LLVM_DEBUG(dbgs() << "\nIdentifying global variables to replace:\n");

  // Build a work list of global variables that are going to need to be
  // remapped due to their types getting changed. Store the existing
  // global variable and the value type of the replacement in the work list.
  SmallVector<std::pair<GlobalVariable *, Type *>, 8> GlobalToRemapValueTyWL;
  for (auto &GV : M.globals()) {
    // If the type changes as a result of the type remapping
    // then a clone of the variable will be necessary.
    Type *ValueTy = GV.getValueType();
    Type *ValueRemapTy = TypeRemapper.remapType(ValueTy);
    if (ValueRemapTy != ValueTy) {
      LLVM_DEBUG(dbgs() << "Need to replace global variable: " << GV << "\n");
      GlobalToRemapValueTyWL.push_back(std::make_pair(&GV, ValueRemapTy));
    } else {
      // The GlobalVariable type may not be changing, because the type is 'p0'.
      // However, the DTrans type metadata attached to the variable may need to
      // be updated in this case.
      remapDTransTypeMetadata(&GV, Mapper);
    }
  }

  // Create the new variables for the ones to be replaced. Some replacements
  // will be managed by the derived class, so a list of those is kept as well to
  // facilitate how the new variable will be initialized.
  DenseMap<GlobalVariable *, GlobalVariable *> LocalVMap;
  SmallPtrSet<GlobalVariable *, 4> SubclassHandledGVMap;
  for (auto &GVTypePair : GlobalToRemapValueTyWL) {
    GlobalVariable *GV = GVTypePair.first;

    // Give the derived class a chance to handle replacing the global variable.
    // This is necessary for cases where only the derived class will know how to
    // initialize the new variable, such as if fields are being deleted.
    GlobalVariable *NewGV = createGlobalVariableReplacement(GV, Mapper);
    if (NewGV) {
      SubclassHandledGVMap.insert(GV);
    } else {
      // Globals are always pointers, so the variable we want to create is
      // the element type of the pointer.
      Type *NewValueType = GVTypePair.second;

      // Create and set the properties of the variable. The initialization of
      // the variable will not occur until all variables have been created
      // because there may be references to other variables being replaced in
      // the initializer list which have not been processed yet.
      NewGV = new GlobalVariable(
          M, NewValueType, GV->isConstant(), GV->getLinkage(),
          /*init=*/nullptr, GV->getName(),
          /*insertbefore=*/nullptr, GV->getThreadLocalMode(),
          GV->getType()->getAddressSpace(), GV->isExternallyInitialized());
      NewGV->setAlignment(MaybeAlign(GV->getAlignment()));
      NewGV->copyAttributesFrom(GV);
      NewGV->copyMetadata(GV, /*Offset=*/0);
      remapDTransTypeMetadata(NewGV, Mapper);
    }

    // Save the mapping in our local list for use when filling in the
    // initializers.
    LocalVMap[GV] = NewGV;

    // Save the mapping in the remapping that will be used for modifying the IR
    // during cloning and function body remapping.
    VMap[GV] = NewGV;

    // The original global will no longer be referenced after all the IR is
    // remapped. Save a list of variables that need to be completely removed
    // after everything is processed.
    GlobalsForRemoval.push_back(GV);
  }

  // Create and initialize new aliases for all the aliases that have their type
  // changed. The original alias will be removed after all the functions
  // have been processed. The aliases need to be processed before the global
  // variable initializers are remapped in case a variable makes use of an
  // alias instead of the original variable or function.
  for (auto &Alias : M.getAliasList()) {
    Constant *Aliasee = Alias.getAliasee();
    // If the Aliasee is being mapped to something other than itself,
    // then this GlobalAlias needs to be updated.
    auto VMapIt = VMap.find(Aliasee);
    if (VMapIt != VMap.end() && VMapIt->second != Aliasee) {
      llvm::Type *ValTy = Alias.getValueType();
      llvm::Type *RemapValTy = TypeRemapper.remapType(ValTy);
      auto *NewAlias = GlobalAlias::create(
          RemapValTy, Alias.getType()->getAddressSpace(), Alias.getLinkage(),
          "", Mapper.mapConstant(*Aliasee), &M);
      NewAlias->takeName(&Alias);
      VMap[&Alias] = NewAlias;
      GlobalsForRemoval.push_back(&Alias);
      LLVM_DEBUG(dbgs() << "Global alias replacement:\n  Orig: " << Alias
                        << "  New : " << *NewAlias << "\n");
    }
  }

  // Create or update the initializers for all the global variables. This
  // handles newly created variables that had their types changed, and
  // existing variables that may have been initialized with the address of a
  // function or global that is being remapped.
  for (auto &GV : M.globals()) {
    GlobalVariable *OrigGV = &GV;
    GlobalVariable *VarToRemap = OrigGV;
    auto LocalVMapIt = LocalVMap.find(OrigGV);
    if (LocalVMapIt != LocalVMap.end())
      VarToRemap = LocalVMapIt->second;

    if (OrigGV->hasInitializer()) {
      // If the derived class handled the replacement variable creation, then
      // the derived class needs to handle the initialization.
      if (SubclassHandledGVMap.count(OrigGV))
        initializeGlobalVariableReplacement(OrigGV, VarToRemap, Mapper);
      else
        VarToRemap->setInitializer(
            Mapper.mapConstant(*OrigGV->getInitializer()));

      if (VarToRemap != OrigGV) {
        VarToRemap->takeName(OrigGV);
        postprocessGlobalVariable(OrigGV, VarToRemap);
        LLVM_DEBUG(dbgs() << "Global Var replacement:\n  Orig: " << *OrigGV
                          << "\n  New : " << *VarToRemap << "\n");
      }
    }
  }
}

// If there is DTrans type metadata attached to the Value, then update the
// metadata based on the type remapping taking place.
void DTransOPOptBase::remapDTransTypeMetadata(Value *V, ValueMapper &Mapper) {
  if (MDNode *MD = TypeMetadataReader::getDTransMDNode(*V))
    DTransTypeMetadataBuilder::addDTransMDNode(*V, Mapper.mapMDNode(*MD));
}

void DTransOPOptBase::initializeGlobalVariableReplacementBaseImpl(
    GlobalVariable *OrigGV, GlobalVariable *NewGV, ValueMapper &Mapper) {
  NewGV->setInitializer(Mapper.mapConstant(*OrigGV->getInitializer()));
}

void DTransOPOptBase::transformIR(Module &M, ValueMapper &Mapper) {
  // The CallInfo objects may need to have their types updated
  // following cloning/remapping of the function. This map is
  // used to find which CallInfo objects to update after
  // processing each function.
  DenseMap<Function *, SmallVector<dtrans::CallInfo *, 4>>
      FunctionToCallInfoVec;

  DTransSafetyInfo *TheDTInfo = DTInfo;
  auto InitializeFunctionCallInfoMapping = [TheDTInfo,
                                            &FunctionToCallInfoVec]() {
    for (auto *CInfo : TheDTInfo->call_info_entries()) {
      Function *F = CInfo->getInstruction()->getFunction();
      FunctionToCallInfoVec[F].push_back(CInfo);
    }
  };

  // Update the CallInfo objects for the function so that the types are the
  // remapped types. If the function was cloned, also update the Instruction
  // mapping to use the Instruction in the cloned Function, instead of the
  // original Function.
  DTransOPTypeRemapper &TheTypeRemapper = TypeRemapper;
  ValueToValueMapTy &TheVMap = VMap;
  auto UpdateCallInfoForFunction = [TheDTInfo, &TheTypeRemapper, &TheVMap,
                                    &FunctionToCallInfoVec](Function *F,
                                                            bool isCloned) {
    auto It = FunctionToCallInfoVec.find(F);
    if (It == FunctionToCallInfoVec.end())
      return;

    for (auto *CInfo : It->second) {
      if (isCloned)
        TheDTInfo->replaceCallInfoInstruction(
            CInfo, cast<Instruction>(TheVMap[CInfo->getInstruction()]));

      dtrans::CallInfoElementTypes &ElementTypes = CInfo->getElementTypesRef();
      for (auto &I : enumerate(ElementTypes))
        ElementTypes.setElemType(
            I.index(), TheTypeRemapper.remapType(I.value().getDTransType()));
    }
  };

  // Set up the mapping of Functions to CallInfo objects that need to
  // be processed as each function is transformed.
  InitializeFunctionCallInfoMapping();

  // Check for debug information that we do not want to be cloned when
  // cloning/remapping functions, and set those debug information objects to map
  // to themselves within the metadata portion of the Value-to-Value map.
  // Specifically, we need to do this for all DISubprogram metadata objects.
  // Otherwise the calls to CloneFunctionInto and remapFunction will create new
  // versions of these objects. Multiple llvm.debug.value intrinsic calls can
  // point to the same metadata object when the DILocalVariable comes from an
  // inlined routine. This leads to consistency problems because there would be
  // two DISubprogram scopes for the routine if we allowed the metadata to be
  // cloned (the variable would point to one scope, but the debug line location
  // would point to the other due to the way CloneFunctionInto operates.) We
  // delete the original function after a cloned function is created, so in the
  // end the DISubprogram object will be put on the clone.
  //
  // Note: we need to do this for all the DISubprograms that exist in the
  // metadata, not just the functions that exist in the module because some
  // inlined function bodies may have already been removed.
  DebugInfoFinder DIFinder;
  DIFinder.processModule(M);
  auto &MDVMap = VMap.MD();
  for (DISubprogram *SP : DIFinder.subprograms())
    MDVMap[SP].reset(SP);

  for (auto &F : M) {
    Function *ResultFunc = &F;
    if (F.isDeclaration())
      continue;

    // The clone function body will be populated when processing the original
    // function. Skip over any functions that represent the clones.
    if (CloneFuncToOrigFuncMap.count(&F))
      continue;

    // Let the derived class perform any IR translation needed for the function.
    processFunction(F);

    // Check whether the function should be cloned or remapped.
    if (OrigFuncToCloneFuncMap.count(&F)) {
      // The CloneFunctionInto function will populate a list of return
      // instructions and some information gathered during the cloning process
      // into these variables. We don't currently use that info.
      SmallVector<ReturnInst *, 8> Returns;
      ClonedCodeInfo CodeInfo;
      Function *CloneFunc = OrigFuncToCloneFuncMap[&F];
      assert(CloneFuncToOrigFuncMap[CloneFunc] == &F &&
             "CloneFuncToOrigFuncMap is invalid");

      CloneFunctionInto(CloneFunc, &F, VMap,
                        CloneFunctionChangeType::GlobalChanges, Returns, "",
                        &CodeInfo, &TypeRemapper, getMaterializer());
      UpdateCallInfoForFunction(&F, /* IsCloned=*/true);

      // CloneFunctionInto() copies all the parameter attributes of the original
      // function's arguments onto the arguments of the new function. For
      // attributes that reference data types, we need to update these for
      // the cloned function when changing the structure type. We do this after
      // the function body is cloned, because otherwise we would need to change
      // the attributes for the original function's arguments prior to the
      // cloning, but we do not want to modify the original function.
      updateAttributeTypes(CloneFunc);

      // Let the derived class perform any additional actions needed on the
      // cloned function. For example, if the transformation is changing
      // data types that will create incompatible parameter attributes, the
      // post-processing function should update them.
      //
      // Do the post processing before deleting the original function
      // because the original instructions may be needed to identify the cloned
      // instruction via the VMap table.
      postprocessFunction(F, /*is_clone=*/true);
      F.deleteBody();
      ResultFunc = CloneFunc;
    } else {
      ValueMapper(VMap, RF_IgnoreMissingLocals, &TypeRemapper,
                  getMaterializer())
          .remapFunction(F);
      UpdateCallInfoForFunction(&F, /* IsCloned=*/false);

      // Attributes may need to be updated on the functions that do not get
      // cloned when opaque pointers are enabled because the parameter type of
      // 'p0' will not change, but the type of object pointed-to may be changed
      // by DTrans.
      if (UsingOpaquePtrs)
        updateAttributeTypes(&F);

      // Let the derived class perform any additional actions needed on the
      // remapped function.
      postprocessFunction(F, /*is_clone=*/false);
    }

    // Update DTrans metadata attached to the function. This needs to be done
    // even if the function is not being cloned because a function using opaque
    // pointer types may not be cloned, but the type trcked for the pointer type
    // by DTrans may change.
    if (MDTuple *FuncMD = dyn_cast_or_null<MDTuple>(
            TypeMetadataReader::getDTransMDNode(*ResultFunc))) {
      SmallVector<Metadata *, 8> Remaps;
      for (const MDOperand &TypeNode : FuncMD->operands())
        Remaps.emplace_back(Mapper.mapMDNode(*cast<MDNode>(TypeNode.get())));

      auto *UpdatedMDTypes = MDTuple::getDistinct(F.getContext(), Remaps);
      DTransTypeMetadataBuilder::addDTransMDNode(*ResultFunc, UpdatedMDTypes);
    }
  }

  // After cloning or remapping functions, it may be necessary to update debug
  // information for the local variables being used by each function. The
  // metadata nodes for a DILocalVariable may have been cloned because the
  // DILocation or DICompositeType object references were cloned. The IR will
  // have been updated to reference the cloned instance within the
  // llvm.debug.declare intrinsic calls. However, the DISubprogram nodes are not
  // being cloned, causing the DISubprogram to still be referring to the
  // original DILocalVariable object. This will result in two variable instances
  // that have same name belonging to the same scope which will result in errors
  // when processing the debug information later. To resolve this, walk the
  // retained nodes element of the subprogram, and update the references for any
  // variable that has been remapped. This needs to be done for all subprograms
  // found by the DIFinder to handle the case of a function that has been
  // optimized away after being inlined.
  //
  // Alternatively, we could just prevent cloning of the DILocalVariables by
  // setting up the metadata remap table with self references to them, like
  // was done for the DISubprograms above. However, this would cause issues
  // with being able to update the structure type descriptions in the debug
  // information if we get to a point where we want the debug info to describe
  // the new types that DTrans is creating with DICompositeType entries.
  for (DISubprogram *SP : DIFinder.subprograms()) {
    if (auto *RawNode = SP->getRawRetainedNodes()) {
      auto *Node = cast<MDTuple>(RawNode);
      unsigned NumOperands = Node->getNumOperands();
      LLVM_DEBUG({
        if (NumOperands)
          dbgs() << "Updating local variable references for DISubprogram:\n"
                 << *SP << "\n";
      });

      for (unsigned i = 0; i < NumOperands; ++i) {
        auto &Op = Node->getOperand(i);
        Metadata *MDOp = *&Op;

        auto MappedTo = MDVMap.find(MDOp);
        if (MappedTo != MDVMap.end() && MDOp != MappedTo->second) {
          Node->replaceOperandWith(i, MappedTo->second);
          LLVM_DEBUG(dbgs() << "replacing retained node:\n"
                            << "  Was: " << *MDOp << "\n"
                            << "  Now: " << *MappedTo->second << "\n");
        }
      }
    }
  }

  LLVM_DEBUG({
    dbgs() << "Call info after transforming functions\n";
    DTInfo->printCallInfo();
  });
}

// Update the attributes which contain types which have been remapped to new
// types. For example:
//   define void @test01(%struct.type01b* byval(%struct.type01b) %in)
//
// Changing the parameter type will create a function of the form:
//  define void @test01.1(%_DT_struct.type01b* byval(%struct.type01b) %in)
//
// Update this to have the remapped type for the byval attribute.
//   define void @test01.1(%_DT_struct.type01b* byval(%_DT_struct.type01b) %in)
//
// With opaque pointers, the parameter type will not be directly changed, but
// the object type used in the attribute may still need to be updated.
// For example:
//   define void @test01(p0 byval(%struct.type01b) %in)
// will need to be updated to:
//   define void @test01(p0 byval(%_DT_struct.type01b) %in)
//
// Other attributes that contain types are: byref, sret, and preallocated.
void DTransOPOptBase::updateAttributeTypes(Function *F) {
  auto &TheRemapper = TypeRemapper;
  auto TypeChangeNeeded = [&TheRemapper](llvm::Type *Ty) -> llvm::Type * {
    llvm::Type *RemapTy = TheRemapper.remapType(Ty);
    if (Ty != RemapTy)
      return RemapTy;
    return nullptr;
  };

  LLVMContext &Context = F->getContext();
  for (auto &A : enumerate(F->args())) {
    // The attributes are mutually exclusive. Just find if any are present,
    // and update the type if needed.
    if (A.value().hasByValAttr()) {
      if (auto *RemapTy = TypeChangeNeeded(A.value().getParamByValType())) {
        F->removeParamAttr(A.index(), Attribute::ByVal);
        F->addParamAttr(A.index(),
                        Attribute::getWithByValType(Context, RemapTy));
      }
    } else if (A.value().hasByRefAttr()) {
      if (auto *RemapTy = TypeChangeNeeded(A.value().getParamByRefType())) {
        F->removeParamAttr(A.index(), Attribute::ByRef);
        F->addParamAttr(A.index(),
                        Attribute::getWithByRefType(Context, RemapTy));
      }
    } else if (A.value().hasStructRetAttr()) {
      if (auto *RemapTy = TypeChangeNeeded(A.value().getParamStructRetType())) {
        F->removeParamAttr(A.index(), Attribute::StructRet);
        F->addParamAttr(A.index(),
                        Attribute::getWithStructRetType(Context, RemapTy));
      }
    } else if (A.value().hasPreallocatedAttr()) {
      AttributeSet ParamAttrs = F->getAttributes().getParamAttrs(A.index());
      if (auto *RemapTy = TypeChangeNeeded(ParamAttrs.getPreallocatedType())) {
        F->removeParamAttr(A.index(), Attribute::Preallocated);
        F->addParamAttr(A.index(),
                        Attribute::getWithPreallocatedType(Context, RemapTy));
      }
    }
  }
}

// Remove functions and global variables that have been completely
// replaced due to the remapping.
void DTransOPOptBase::removeDeadValues() {
  // An initialized global may hold references to other globals or functions
  // being deleted. Let go of everything first so there are no dangling
  // references when deleting the objects.
  for (auto *GV : GlobalsForRemoval)
    GV->dropAllReferences();

  // Cleanup the functions that were cloned.
  for (auto &OTCPair : OrigFuncToCloneFuncMap)
    OTCPair.first->eraseFromParent();

  OrigFuncToCloneFuncMap.clear();
  CloneFuncToOrigFuncMap.clear();

  for (auto *GV : GlobalsForRemoval)
    GV->eraseFromParent();

  GlobalsForRemoval.clear();
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
