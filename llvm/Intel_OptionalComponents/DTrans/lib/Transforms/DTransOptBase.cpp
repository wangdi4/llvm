//===-- DTransOptBase.cpp - Common base classes for DTrans Transforms---==//
//
// Copyright (C) 2018 Intel Corporation. All rights reserved.
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
// definition.
//
//===----------------------------------------------------------------------===//

#include "Intel_DTrans/Transforms/DTransOptBase.h"
#include "Intel_DTrans/Analysis/DTransAnalysis.h"
#include "Intel_DTrans/DTransCommon.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/Transforms/Utils/Cloning.h"

using namespace llvm;

#define DEBUG_TYPE "dtrans-optbase"

namespace {
//===----------------------------------------------------------------------===//
// Utility functions for llvm:Type objects
//===----------------------------------------------------------------------===//

// Helper method to dereference through all the pointer or array levels to get
// to a non-pointer/non-array type for the input type.
Type *unwrapType(Type *Ty) {
  Type *BaseTy = Ty;
  while (BaseTy->isPointerTy() || BaseTy->isArrayTy())
    if (BaseTy->isPointerTy())
      BaseTy = BaseTy->getPointerElementType();
    else
      BaseTy = BaseTy->getArrayElementType();

  return BaseTy;
}
} // end anonymous namespace

//===----------------------------------------------------------------------===//
// Implementation for the DTransTypeRemapper class methods
//===----------------------------------------------------------------------===//

// Return the new type for 'SrcTy'. If the type is not being changed 'SrcTy'
// will be returned.
llvm::Type *DTransTypeRemapper::remapType(llvm::Type *SrcTy) {
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

// Used by clients to set up the initial set of structure types that need
// be remapped.
// \param SrcTy The original structure type
// \param DestTy The new structure type to remap to
void DTransTypeRemapper::addTypeMapping(llvm::Type *SrcTy, llvm::Type *DestTy) {
  assert(!AllTypeMappingsAdded &&
         "Cannot add additional types after setting AllTypeMappingsAdded");
  assert(!hasRemappedType(SrcTy) &&
         "DTransTypeRemapper already contains mapping for type");

  SrcTypeToNewType[SrcTy] = DestTy;
}

// Return 'true' if \p SrcTy is contained in the map.
bool DTransTypeRemapper::hasRemappedType(llvm::Type *SrcTy) const {
  return SrcTypeToNewType.count(SrcTy);
}

// Return the type mapping for \p SrcTy, if there is one. If there is
// not one yet, return nullptr.
llvm::Type *DTransTypeRemapper::lookupTypeMapping(llvm::Type *SrcTy) const {
  auto It = SrcTypeToNewType.find(SrcTy);
  if (It != SrcTypeToNewType.end())
    return It->second;

  return nullptr;
}

// Return a previously computed type mapping for SrcTy, if it's been previously
// evaluated. Otherwise, nullptr.
llvm::Type *
DTransTypeRemapper::lookupCachedTypeMapping(llvm::Type *SrcTy) const {
  auto It = RemapSrcToDestTypeCache.find(SrcTy);
  if (It != RemapSrcToDestTypeCache.end())
    return It->second;

  return nullptr;
}

// If the \p SrcTy type needs to be replaced, return the replacement type.
// Otherwise, return nullptr.
//
// This routine walks pointers, function pointers, and arrays to compute
// replacement types.  It does not walk structure bodies, as those that need
// replacing should be populated in the type mapping table via addTypeMapping by
// the transformation pass. It also does not examine vector types, as those are
// composed of primitive types, which are not expected to need to be remapped.
llvm::Type *
DTransTypeRemapper::computeReplacementType(llvm::Type *SrcTy) const {
  // Check if the answer is already computed or if this is a basic structure
  // type that has been directly added to the type mapping table.
  llvm::Type *CurMapping = lookupTypeMapping(SrcTy);
  if (CurMapping)
    return CurMapping;

  if (SrcTy->isPointerTy()) {
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

    if (NeedsReplaced) {
      return FunctionType::get(ReplRetTy, makeArrayRef(DataTypes),
                               FunctionTy->isVarArg());
    }
  }

  // Note: We do not look for 'struct' types directly in the above loop
  // because those should have been directly added to the type mapping
  // via AddTypeMapping by the transformation.

  // Type does not need type replacement.
  return nullptr;
}

//===----------------------------------------------------------------------===//
// Implementation for the DTransOptBase class methods
//===----------------------------------------------------------------------===//

// This routine drives the transformation process.
// First, it prepares any new data types needed for the transformation,
// then invokes module/function transformation itself. See header file for
// complete description of actions.
bool DTransOptBase::run(Module &M) {
  if (!prepareTypesBaseImpl(M))
    return false;

  prepareModule(M);

  // Identify and clone any function prototypes for functions that will need
  // to be cloned.
  createCloneFunctionDeclarations(M);

  // Remap global variables that have type changes to their new types
  ValueMapper Mapper(VMap, RF_IgnoreMissingLocals, TypeRemapper, Materializer);
  convertGlobalVariables(M, Mapper);

  // Transform all the functions.
  transformIR(M, Mapper);

  removeDeadValues();

  return true;
}

// This method is responsible for creating all the new data types that are
// required by the transformation and any dependent types that need to be
// created as a result of creating those types. Returns 'true' if types are
// changed.
bool DTransOptBase::prepareTypesBaseImpl(Module &M) {
  // Invoke the derived class to populate the TypeRemapper with any types
  // the transformation is going to directly transform.
  bool Changed = prepareTypes(M);
  if (!Changed)
    return false;

  // Compute the set of types that each type is a dependee of. This will
  // enable the determination of other types that need to be rewritten when one
  // type changes.
  TypeDependencyMapping TypeToDependentTypes;
  buildTypeDependencyMapping(TypeToDependentTypes);

  // Identify and create new types for types dependent on changes made by the
  // derived class. This map holds the original type to the replacement type
  // that needs to be populated once all types are identified.
  TypeToTypeMap OrigToNewTypeReplacement;
  prepareDependentTypes(M, TypeToDependentTypes, OrigToNewTypeReplacement);

  // Inform the TypeRemapper that all types have been created, so that
  // populateDependentTypes can use the mapper to compute the types to use when
  // populating the structure bodies.
  TypeRemapper->setAllTypeMappingsAdded();
  populateDependentTypes(M, OrigToNewTypeReplacement);

  // Invoke the derived class to populate the bodies of types the transformation
  // is creating.
  populateTypes(M);
  return true;
}

// For each type, compute the set of structure and array types that either
// directly or indirectly uses the type. For example:
//   struct.test01 = { i32, i32 }
//   struct.test02 = { struct.test01, struct.test02* }
//   struct.test03 = { struct.test01*, struct.test02* }
//
// struct.test01 is used by struct.test02 and test03.
// struct.test02 is used by of struct.test03
// struct.test03 is not used by any types, and have an entry in the map.
//
// Note, we simplify the information down to the base structure type, so even
// though struct.test03 only contained a pointer reference to struct.test01, we
// still include struct.test03 in the set for struct.test01. This is because if
// struct.test01 gets replaced with a different type, then struct.test03 is
// going to need to be replaced as well. The map will also omit self-references.
//
// The map created will be used to populating a work list of types that will
// need to be processed when changing a specific type.
void DTransOptBase::buildTypeDependencyMapping(
    TypeDependencyMapping &TypeToDependentTypes) {

  for (auto *TI : DTInfo.type_info_entries()) {
    dtrans::TypeInfo::TypeInfoKind Kind = TI->getTypeInfoKind();
    if (Kind == dtrans::TypeInfo::StructInfo ||
        Kind == dtrans::TypeInfo::ArrayInfo)
      collectDependenciesForType(TI->getLLVMType(), TypeToDependentTypes);
  }

#if !defined(NDEBUG)
  DEBUG(dumpTypeDepenencyMapping(TypeToDependentTypes));
#endif // !defined(NDEBUG)
}

#if !defined(NDEBUG)
// Print the table of type dependencies in the following format:
//   Type: UsedBy
void DTransOptBase::dumpTypeDepenencyMapping(
    TypeDependencyMapping &TypeToDependentTypes) {
  auto PrintNameOrType = [](Type *Ty) {
    if (auto *StructTy = dyn_cast<StructType>(Ty))
      dbgs() << StructTy->getName();
    else
      dbgs() << *Ty;
  };

  dbgs() << "Type dependency mapping table:\n";
  for (auto &TySetTyPair : TypeToDependentTypes) {
    Type *Ty = TySetTyPair.first;
    PrintNameOrType(Ty);
    dbgs() << ": ";

    for (auto *UsedByType : TySetTyPair.second) {
      PrintNameOrType(UsedByType);
      dbgs() << ", ";
    }
    dbgs() << "\n";
  }
}
#endif // !defined(NDEBUG)

// Update the dependency mapping by analyzing the contents within 'Ty' to add
// 'Ty' to the depenency sets for each type contained within it. For example,
// processing the structure definition:
//   %struct.test08b = type { i32, %struct.test08c, void (%struct.test08a*)* }
//
// will result in the sets maintained for 'i32', 'struct.test08c' and
// 'struct.test08a' all containing 'struct.test08b' as a type is going to need
// to be replaced if any of those types are replaced.
void DTransOptBase::collectDependenciesForType(Type *Ty,
                                               TypeDependencyMapping &Map) {
  collectDependenciesForTypeRecurse(Ty, Ty, Map);
}

void DTransOptBase::collectDependenciesForTypeRecurse(
    Type *Dependee, Type *Ty, TypeDependencyMapping &Map) {
  auto UpdateTypeToDependentTypeMap = [&Map](Type *Depender, Type *Dependee) {
    if (Depender == Dependee)
      return;

    if (!Dependee->isAggregateType())
      return;

    DEBUG(dbgs() << "DTRANS-OPTBASE: Type dependency: Replacing " << *Depender
                 << " will require replacing  " << *Dependee << "\n");
    Map[Depender].insert(Dependee);
  };

  if (auto *StructTy = dyn_cast<StructType>(Ty)) {
    for (auto *MemberType : StructTy->elements()) {
      Type *BaseTy = unwrapType(MemberType);
      if (auto *FunctionTy = dyn_cast<FunctionType>(BaseTy))
        collectDependenciesForTypeRecurse(Dependee, FunctionTy, Map);
      else
        UpdateTypeToDependentTypeMap(BaseTy, Dependee);
    }
    return;
  }

  if (auto *ArrayTy = dyn_cast<ArrayType>(Ty)) {
    Type *BaseTy = unwrapType(ArrayTy->getElementType());
    if (auto *FunctionTy = dyn_cast<FunctionType>(BaseTy))
      collectDependenciesForTypeRecurse(Dependee, FunctionTy, Map);
    else
      UpdateTypeToDependentTypeMap(BaseTy, Dependee);

    return;
  }

  if (auto *FuncTy = dyn_cast<FunctionType>(Ty)) {
    Type *RetTy = FuncTy->getReturnType();
    Type *BaseTy = unwrapType(RetTy);
    UpdateTypeToDependentTypeMap(Dependee, BaseTy);

    unsigned Total = FuncTy->getNumParams();
    for (unsigned Idx = 0; Idx < Total; ++Idx) {
      Type *ParmTy = FuncTy->getParamType(Idx);
      Type *BaseTy = unwrapType(ParmTy);
      if (auto *BaseFuncTy = dyn_cast<FunctionType>(BaseTy))
        collectDependenciesForTypeRecurse(Dependee, BaseFuncTy, Map);
      else
        UpdateTypeToDependentTypeMap(BaseTy, Dependee);
    }
  }
}

// Identify and create opaque types.
void DTransOptBase::prepareDependentTypes(
    Module &M, TypeDependencyMapping &TypeToDependentTypes,
    TypeToTypeMap &OrigToNewTypeReplacement) {
  // Prepare a mapping from a Type to the set of all the types that depend upon
  // it.

  // Build a worklist of types that are going to need to be replaced based
  // on the types the transform is remapping. At this point the TypeRemapper
  // will only contain those types.
  SmallSet<Type *, 16> Processed;
  SmallSetVector<Type *, 16> Worklist;
  for (auto *TI : DTInfo.type_info_entries()) {
    Type *OrigTy = TI->getLLVMType();

    if (TypeRemapper->hasRemappedType(OrigTy)) {
      Worklist.insert(OrigTy);

      // Mark this type as processed because the child class will deal with
      // replacing it.
      Processed.insert(OrigTy);
    }
  }

  while (!Worklist.empty()) {
    Type *Ty = Worklist.pop_back_val();

    // Add all dependent types that have not been processed yet to the worklist
    if (TypeToDependentTypes.count(Ty)) {
      for (auto &Depends : TypeToDependentTypes[Ty])
        if (!Processed.count(Depends))
          Worklist.insert(Depends);
    }
    if (Processed.count(Ty))
      continue;

    Processed.insert(Ty);

    // We need to create an opaque structure type here for the replacement, and
    // add it to the TypeRemapper. This will enable the types for the new
    // structure bodies to be computed after all the structure replacements have
    // been determined. For other types, such as arrays, we don't need to do
    // anything here, the replacements for them will be computed on demand.
    if (auto *StructTy = dyn_cast<StructType>(Ty)) {
      Type *ReplacementTy = StructType::create(
          Context, Twine(DepTypePrefix + StructTy->getStructName()).str());
      TypeRemapper->addTypeMapping(Ty, ReplacementTy);
      OrigToNewTypeReplacement[Ty] = ReplacementTy;

      DEBUG(dbgs() << "DTRANS-OPTBASE: New type created: " << *ReplacementTy
                   << " as replacement for " << *Ty << "\n");
    }
  }
}

// This method sets the body of any dependent data types based on the remapped
// types.
void DTransOptBase::populateDependentTypes(
    Module &M, TypeToTypeMap &DependentTypeMapping) {

  for (auto &ONPair : DependentTypeMapping) {
    Type *OrigTy = ONPair.first;
    Type *ReplTy = ONPair.second;

    if (auto *StructTy = dyn_cast<StructType>(OrigTy)) {
      SmallVector<Type *, 8> DataTypes;
      for (auto *MemberTy : StructTy->elements())
        DataTypes.push_back(TypeRemapper->remapType(MemberTy));

      StructType *ReplStructTy = cast<StructType>(ReplTy);
      ReplStructTy->setBody(DataTypes, StructTy->isPacked());
      DEBUG(dbgs() << "DTRANS-OPTBASE: New structure body: " << *ReplStructTy
                   << "\n");
    }
  }
}

void DTransOptBase::transformIR(Module &M, ValueMapper &Mapper) {
  for (auto &F : M) {
    if (F.isDeclaration())
      continue;

    // The clone function body will be populated when processing the original
    // function. Skip over any functions that represent the clones.
    if (CloneFuncToOrigFuncMap.count(&F))
      continue;

    // Let the derived class perform any IR translation needed for the function
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

      CloneFunctionInto(CloneFunc, &F, VMap, true, Returns, "", &CodeInfo,
                        TypeRemapper, Materializer);

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
    } else {
      // Perform the type remapping for the function
      ValueMapper(VMap, RF_IgnoreMissingLocals, TypeRemapper, Materializer)
          .remapFunction(F);

      // Let the derived class perform any additional actions needed on the
      // remapped function.
      postprocessFunction(F, /*is_clone=*/false);
    }
  }
}

// Identify and create new function prototypes for dependent functions
void DTransOptBase::createCloneFunctionDeclarations(Module &M) {
  // Create a work list of all the function definitions that need to be
  // considered for cloning.
  std::vector<Function *> WL;
  for (auto &F : M) {
    if (!F.isDeclaration())
      WL.push_back(&F);
  }

  for (auto *F : WL) {
    // If the function signature changes as a result of the type remapping
    // then a clone will be necessary.
    Type *FuncTy = F->getType();
    Type *ReplTy = TypeRemapper->remapType(FuncTy);
    if (ReplTy != FuncTy) {
      Function *NewF =
          Function::Create(cast<FunctionType>(ReplTy->getPointerElementType()),
                           F->getLinkage(), F->getName(), &M);
      NewF->copyAttributesFrom(F);
      VMap[F] = NewF;

      // Save a forward and backward mapping between the original
      // function and the new function. (The forward mapping is also in the
      // VMap, but we maintain  this separately because that mapping will
      // be updated to also include entries for the cloned functions, which
      // complicates looking up whether a function is going to be cloned or
      // not)
      OrigFuncToCloneFuncMap[F] = NewF;
      CloneFuncToOrigFuncMap[NewF] = F;

      // Create VMap entries for the arguments that will be used during the
      // call to cloneFunctionInfo. This must be done to make the information
      // available for the later call to cloneFunctionInto.
      Function::arg_iterator DestI = NewF->arg_begin();
      for (Argument &I : F->args()) {
        DestI->setName(I.getName());
        VMap[&I] = &*DestI++;
      }

      DEBUG(dbgs() << "DTRANS-OPTBASE: Will clone: " << F->getName() << " "
                   << *F->getType() << " into: " << NewF->getName() << " "
                   << *NewF->getType() << "\n");
    }
  }
}

// Remap global variables for new types
void DTransOptBase::convertGlobalVariables(Module &M, ValueMapper &Mapper) {
  // TODO: Subsequent change will implement remapping for global vars.
}

// Update the module to remove objects that should no longer be referenced.
void DTransOptBase::removeDeadValues() {
  for (auto &OTCPair : OrigFuncToCloneFuncMap)
    OTCPair.first->eraseFromParent();

  OrigFuncToCloneFuncMap.clear();
  CloneFuncToOrigFuncMap.clear();

  // TODO: Subsequent change will implement removal for global variables that
  // have been converted to new types.
}
