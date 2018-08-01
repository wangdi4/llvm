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
#include "llvm/IR/Verifier.h"
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
  if (!DTInfo.useDTransAnalysis()) {
    // The DTransAnalysis type info lists are used when determining dependent
    // types to be handled by the base class. Without this the base class cannot
    // properly remap the types.
    LLVM_DEBUG(dbgs() << "DTRANS-OPTBASE: DTransAnalysis information is "
                         "required to be available in order to determine type "
                         "dependencies.\n");
    return false;
  }

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

#if !defined(NDEBUG)
  // Do a sanity check of the IR for the DTrans optimizations to catch any
  // problems introduced while the transformations are being developed. This
  // code may be removed later after DTrans is stable. verifyModule returns
  // 'true' if errors are found.
  if (verifyModule(M, &dbgs())) {
    LLVM_DEBUG(dbgs() << M);
    report_fatal_error(
        "Module verifier found errors following a DTrans optimization");
  }
#endif // !defined(NDEBUG)

  return true;
}

// This method is responsible for creating all the new data types that are
// required by the transformation and any dependent types that need to be
// created as a result of creating those types. Returns 'true' if types are
// changed.
bool DTransOptBase::prepareTypesBaseImpl(Module &M) {
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
void DTransOptBase::buildTypeDependencyMapping() {
  for (auto *TI : DTInfo.type_info_entries()) {
    dtrans::TypeInfo::TypeInfoKind Kind = TI->getTypeInfoKind();
    if (Kind == dtrans::TypeInfo::StructInfo ||
        Kind == dtrans::TypeInfo::ArrayInfo) {
      collectDependenciesForType(TI->getLLVMType());
    }
  }

#if !defined(NDEBUG)
  LLVM_DEBUG(dumpTypeToTypeSetMapping("Type dependency mapping table:",
                                      TypeToDependentTypes));
#endif // !defined(NDEBUG)
}

void DTransOptBase::buildTypeEnclosingMapping() {
  for (auto *TI : DTInfo.type_info_entries()) {
    dtrans::TypeInfo::TypeInfoKind Kind = TI->getTypeInfoKind();
    if (Kind == dtrans::TypeInfo::StructInfo ||
        Kind == dtrans::TypeInfo::ArrayInfo) {
      collectEnclosingForType(TI->getLLVMType());
    }
  }

#if !defined(NDEBUG)
  LLVM_DEBUG(dumpTypeToTypeSetMapping("Type enclosing mapping table:",
                                      TypeToEnclosingTypes));
#endif // !defined(NDEBUG)
}

#if !defined(NDEBUG)
// Print the table of type dependencies in the following format:
//   Type: UsedBy
void DTransOptBase::dumpTypeToTypeSetMapping(
    StringRef Header, TypeToTypeSetMap &TypeToDependentTypes) {
  auto PrintNameOrType = [](Type *Ty) {
    auto *StructTy = dyn_cast<StructType>(Ty);
    if (StructTy && StructTy->hasName())
      dbgs() << StructTy->getName();
    else
      dbgs() << *Ty;
  };

  dbgs() << Header << "\n";
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
void DTransOptBase::collectDependenciesForType(Type *Ty) {
  collectDependenciesForTypeRecurse(Ty, Ty);
}

void DTransOptBase::collectDependenciesForTypeRecurse(Type *Dependee,
                                                      Type *Ty) {
  auto UpdateTypeToDependentTypeMap = [this](Type *Depender, Type *Dependee) {
    if (Depender == Dependee)
      return;

    if (!Dependee->isAggregateType() || !Depender->isAggregateType())
      return;

    LLVM_DEBUG(dbgs() << "DTRANS-OPTBASE: Type dependency: Replacing "
                      << *Depender << " will require replacing  " << *Dependee
                      << "\n");

    TypeToDependentTypes[Depender].insert(Dependee);
  };

  if (auto *StructTy = dyn_cast<StructType>(Ty)) {
    for (auto *MemberType : StructTy->elements()) {
      Type *BaseTy = unwrapType(MemberType);
      if (auto *FunctionTy = dyn_cast<FunctionType>(BaseTy))
        collectDependenciesForTypeRecurse(Dependee, FunctionTy);
      else
        UpdateTypeToDependentTypeMap(BaseTy, Dependee);
    }
    return;
  }

  if (auto *ArrayTy = dyn_cast<ArrayType>(Ty)) {
    Type *BaseTy = unwrapType(ArrayTy->getElementType());
    if (auto *FunctionTy = dyn_cast<FunctionType>(BaseTy))
      collectDependenciesForTypeRecurse(Dependee, FunctionTy);
    else
      UpdateTypeToDependentTypeMap(BaseTy, Dependee);

    return;
  }

  if (auto *FuncTy = dyn_cast<FunctionType>(Ty)) {
    Type *RetTy = FuncTy->getReturnType();
    Type *BaseTy = unwrapType(RetTy);
    UpdateTypeToDependentTypeMap(BaseTy, Dependee);

    unsigned Total = FuncTy->getNumParams();
    for (unsigned Idx = 0; Idx < Total; ++Idx) {
      Type *ParmTy = FuncTy->getParamType(Idx);
      Type *BaseTy = unwrapType(ParmTy);
      if (auto *BaseFuncTy = dyn_cast<FunctionType>(BaseTy))
        collectDependenciesForTypeRecurse(Dependee, BaseFuncTy);
      else
        UpdateTypeToDependentTypeMap(BaseTy, Dependee);
    }
  }
}

void DTransOptBase::collectEnclosingForType(Type *Ty) {
  SmallVector<Type *, 8> TypeStack;
  collectEnclosingForTypeRecurse(TypeStack, Ty);
}

void DTransOptBase::collectEnclosingForTypeRecurse(
    SmallVectorImpl<Type *> &EnclosingTypes, Type *Ty) {
  // Do not collect non-aggregate types
  if (!Ty->isAggregateType())
    return;

  // Assign enclosing types for Ty.
  TypeToEnclosingTypes[Ty].insert(EnclosingTypes.begin(), EnclosingTypes.end());

  if (auto *StructTy = dyn_cast<StructType>(Ty)) {
    EnclosingTypes.push_back(Ty);

    for (auto *MemberType : StructTy->elements()) {
      collectEnclosingForTypeRecurse(EnclosingTypes, MemberType);
    }

    EnclosingTypes.pop_back();
    return;
  }

  if (auto *ArrayTy = dyn_cast<ArrayType>(Ty)) {
    EnclosingTypes.push_back(Ty);
    collectEnclosingForTypeRecurse(EnclosingTypes, ArrayTy->getElementType());
    EnclosingTypes.pop_back();
  }
}

const typename DTransOptBase::TypeToTypeSetMap::mapped_type &
DTransOptBase::getEnclosingTypes(Type *Ty) {
  assert(Ty->isAggregateType() &&
         "Only aggreagate types are corrently collected");

  if (TypeToEnclosingTypes.empty()) {
    buildTypeEnclosingMapping();
  }

  auto It = TypeToEnclosingTypes.find(Ty);
  if (It == TypeToEnclosingTypes.end()) {
    // Return empty set.
    return TypeToEnclosingTypes[nullptr];
  }

  return It->second;
}

// Identify and create opaque types.
void DTransOptBase::prepareDependentTypes(
    Module &M, TypeToTypeSetMap &TypeToDependentTypes,
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
      // If StructTy is literal, don't give the replacement a name.
      Type *ReplacementTy;
      if (StructTy->isLiteral())
        ReplacementTy = StructType::create(Context);
      else
        ReplacementTy = StructType::create(
          Context, Twine(DepTypePrefix + StructTy->getStructName()).str());
      TypeRemapper->addTypeMapping(Ty, ReplacementTy);
      OrigToNewTypeReplacement[Ty] = ReplacementTy;

      LLVM_DEBUG(dbgs() << "DTRANS-OPTBASE: New type created: "
                        << *ReplacementTy << " as replacement for " << *Ty
                        << "\n");
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
      LLVM_DEBUG(dbgs() << "DTRANS-OPTBASE: New structure body: "
                        << *ReplStructTy << "\n");
    }
  }
}

void DTransOptBase::transformIR(Module &M, ValueMapper &Mapper) {
  // Set up the mapping of Functions to CallInfo objects that need to
  // be processed as each function is transformed.
  initializeFunctionCallInfoMapping();

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
      updateCallInfoForFunction(&F, /* IsCloned=*/true);

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
      updateCallInfoForFunction(&F, /* IsCloned=*/false);

      // Let the derived class perform any additional actions needed on the
      // remapped function.
      postprocessFunction(F, /*is_clone=*/false);
    }
  }

  LLVM_DEBUG({
    dbgs() << "Call info after remapping\n";
    DTInfo.printCallInfo(dbgs());
  });

  // The Function to CallInfo mapping is no longer needed, and can be released
  // now.
  resetFunctionCallInfoMapping();
}

// Set up the Function to CallInfo mapping that is needed for keeping
// the CallInfo objects up to date while the transformation is running.
void DTransOptBase::initializeFunctionCallInfoMapping() {
  resetFunctionCallInfoMapping();

  for (auto *CInfo : DTInfo.call_info_entries()) {
    Function *F = CInfo->getInstruction()->getParent()->getParent();
    FunctionToCallInfoVec[F].push_back(CInfo);
  }
}

// This function is used to update the CallInfo objects associated
// with a specific function. The type list of each call info will be
// updated to reflect the remapped types. For cloned functions, the
// instruction pointer in the call info will be updated to point to the
// instruction in the cloned function.
void DTransOptBase::updateCallInfoForFunction(Function *F, bool isCloned) {
  if (FunctionToCallInfoVec.count(F))
    for (auto *CInfo : FunctionToCallInfoVec[F]) {
      if (isCloned)
        DTInfo.replaceCallInfoInstruction(
            CInfo, cast<Instruction>(VMap[CInfo->getInstruction()]));

      dtrans::PointerTypeInfo &PTI = CInfo->getPointerTypeInfoRef();
      size_t Num = PTI.getNumTypes();
      for (size_t i = 0; i < Num; ++i)
        PTI.setType(i, TypeRemapper->remapType(PTI.getType(i)));
    }
}

// Clear all the data in the Function to CallInfo mapping
void DTransOptBase::resetFunctionCallInfoMapping() {
  FunctionToCallInfoVec.clear();
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

      LLVM_DEBUG(dbgs() << "DTRANS-OPTBASE: Will clone: " << F->getName() << " "
                        << *F->getType() << " into: " << NewF->getName() << " "
                        << *NewF->getType() << "\n");
    }
  }
}

// Remap global variables that are of types being converted to new types
void DTransOptBase::convertGlobalVariables(Module &M, ValueMapper &Mapper) {
  // Build a work list of global variables that are going to need to be
  // remapped due to their types getting changed. Store the existing
  // global variable and the type the replacement should be in the work list.
  SmallVector<std::pair<GlobalVariable *, Type *>, 8> GlobalsWL;
  for (auto &GV : M.globals()) {
    // If the type changes as a result of the type remapping
    // then a clone of the variable will be necessary.
    Type *GVTy = GV.getType();
    Type *RemapTy = TypeRemapper->remapType(GVTy);
    if (RemapTy != GVTy) {
      LLVM_DEBUG(dbgs() << "DTRANS-OPTBASE: Need to replace global variable: "
                        << GV << "\n");
      GlobalsWL.push_back(std::make_pair(&GV, RemapTy));
    }
  }

  // Create the new variables for the ones to be replaced. Some replacements
  // will be managed by the derived class, so a list of those is kept as well to
  // facilitate how the new variable will be initialized.
  DenseMap<GlobalVariable *, GlobalVariable *> LocalVMap;
  SmallPtrSet<GlobalVariable *, 4> SubclassHandledGVMap;
  for (auto &GVTypePair : GlobalsWL) {
    GlobalVariable *GV = GVTypePair.first;

    // Give the derived class a chance to handle replacing the global variable.
    // This is necessary for cases where only the derived class will know how to
    // initialize the new variable, such as if fields are being deleted.
    GlobalVariable *NewGV = createGlobalVariableReplacement(GV);
    if (NewGV) {
      SubclassHandledGVMap.insert(GV);
    } else {
      // Globals are always pointers, so the variable we want to create is
      // the element type of the pointer.
      Type *RemapType = GVTypePair.second->getPointerElementType();

      // Create and set the properties of the variable. The initialization of
      // the variable will not occur until all variables have been created
      // because there may be references to other variables being replaced in
      // the initializer list which have not been processed yet.
      NewGV = new GlobalVariable(
          M, RemapType, GV->isConstant(), GV->getLinkage(),
          /*init=*/nullptr, GV->getName(),
          /*insertbefore=*/nullptr, GV->getThreadLocalMode(),
          GV->getType()->getAddressSpace(), GV->isExternallyInitialized());
      NewGV->setAlignment(GV->getAlignment());
      NewGV->copyAttributesFrom(GV);
      NewGV->copyMetadata(GV, /*Offset=*/0);
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
      }

      LLVM_DEBUG(dbgs() << "DTRANS-OPTBASE: Global Var replacement:\n  Orig: "
                        << *OrigGV << "\n  New : " << *VarToRemap << "\n");
    }
  }

  // Create and initialize new aliases for all the aliases that have their type
  // changed. The original alias will be removed after all the functions
  // have been processed.
  // TODO: Will the transformations need to post process the aliases, like it
  // does for global variables?
  for (auto &Alias : M.getAliasList()) {
    Constant *Aliasee = Alias.getAliasee();
    // If the Aliasee is being mapped to something other than itself,
    // then this GlobalAlias needs to be updated.
    auto VMapIt = VMap.find(Aliasee);
    if (VMapIt != VMap.end() && VMapIt->second != Aliasee) {
      Type *RemapTy = VMapIt->second->getType();
      auto *NewAlias = GlobalAlias::create(
          RemapTy->getPointerElementType(), Alias.getType()->getAddressSpace(),
          Alias.getLinkage(), "", Mapper.mapConstant(*Aliasee), &M);
      NewAlias->takeName(&Alias);
      VMap[&Alias] = NewAlias;
      GlobalsForRemoval.push_back(&Alias);

      LLVM_DEBUG(dbgs() << "DTRANS-OPTBASE: Global alias replacement:\n  Orig: "
                        << Alias << "\n  New : " << *NewAlias << "\n");
    }
  }
}

// Update the module to remove objects that should no longer be referenced.
void DTransOptBase::removeDeadValues() {

  // An initialized global may hold references to other globals or functions
  // being deleted. Let go of everything first so there are no dangling
  // references when deleting the objects.
  for (auto *GV : GlobalsForRemoval)
    GV->dropAllReferences();

  for (auto &OTCPair : OrigFuncToCloneFuncMap)
    OTCPair.first->eraseFromParent();

  OrigFuncToCloneFuncMap.clear();
  CloneFuncToOrigFuncMap.clear();

  for (auto *GV : GlobalsForRemoval)
    GV->eraseFromParent();

  GlobalsForRemoval.clear();
}

void DTransOptBase::updateCallSizeOperand(Instruction *I,
                                          dtrans::CallInfo *CInfo,
                                          llvm::Type *OrigTy,
                                          llvm::Type *ReplTy) {
  uint64_t OrigSize = DL.getTypeAllocSize(OrigTy);
  uint64_t ReplSize = DL.getTypeAllocSize(ReplTy);

  updateCallSizeOperand(I, CInfo, OrigSize, ReplSize);
}

// This function performs the actual replacement for the size parameter
// of a function call, by finding the original constant that is a
// multiple of \p OrigSize, and replacing that value with a multiple
// of \p ReplSize.
void DTransOptBase::updateCallSizeOperand(Instruction *I,
                                          dtrans::CallInfo *CInfo,
                                          uint64_t OrigSize,
                                          uint64_t ReplSize) {
  // Find the User value that has a constant integer multiple of the original
  // structure size as an operand.
  bool Found = false;
  SmallVector<std::pair<User *, unsigned>, 4> SizeUseStack;
  if (auto *AInfo = dyn_cast<dtrans::AllocCallInfo>(CInfo)) {
    dtrans::AllocKind AK = AInfo->getAllocKind();
    switch (AK) {
    case dtrans::AK_NotAlloc:
      llvm_unreachable("No AllocCallInfo for AK_NotAlloc!");
    case dtrans::AK_UserMalloc0:
      llvm_unreachable("AK_UserMalloc0 not yet supported!");
    case dtrans::AK_New:
    case dtrans::AK_Malloc:
    case dtrans::AK_Realloc:
    case dtrans::AK_Calloc:
    case dtrans::AK_UserMalloc: {
      unsigned SizeArgPos = 0;
      unsigned CountArgPos = 0;
      getAllocSizeArgs(AK, CallSite(I), SizeArgPos, CountArgPos, TLI);
      if (AK == dtrans::AK_Calloc) {
        Found =
            findValueMultipleOfSizeInst(I, CountArgPos, OrigSize, SizeUseStack);
        assert((Found || SizeUseStack.empty()) &&
               "SizeUseStack not empty after failed value search!");
        if (!Found)
          Found = findValueMultipleOfSizeInst(I, SizeArgPos, OrigSize,
                                              SizeUseStack);
      } else {
        Found =
            findValueMultipleOfSizeInst(I, SizeArgPos, OrigSize, SizeUseStack);
      }
      break;
    }
    }
  } else {
    // This asserts because we only expect alloc info or memfunc info.
    assert(isa<dtrans::MemfuncCallInfo>(CInfo) &&
           "Expected either alloc or memfunc!");
    // All memfunc calls have the size as operand 2.
    Found = findValueMultipleOfSizeInst(I, 2, OrigSize, SizeUseStack);
  }

  // The safety conditions should guarantee that we can find this constant.
  assert(Found && "Constant multiple of size not found!");

  replaceSizeValue(I, SizeUseStack, OrigSize, ReplSize);
}

void DTransOptBase::updatePtrSubDivUserSizeOperand(llvm::BinaryOperator *Sub,
                                                   llvm::Type *OrigTy,
                                                   llvm::Type *ReplTy) {
  uint64_t OrigSize = DL.getTypeAllocSize(OrigTy);
  uint64_t ReplSize = DL.getTypeAllocSize(ReplTy);

  updatePtrSubDivUserSizeOperand(Sub, OrigSize, ReplSize);
}

void DTransOptBase::updatePtrSubDivUserSizeOperand(llvm::BinaryOperator *Sub,
                                                   uint64_t OrigSize,
                                                   uint64_t ReplSize) {
  for (auto *U : Sub->users()) {
    auto *BinOp = cast<BinaryOperator>(U);
    assert((BinOp->getOpcode() == Instruction::SDiv ||
            BinOp->getOpcode() == Instruction::UDiv) &&
           "Unexpected user in updatePtrSubDivUserSizeOperand!");
    // The sub instruction must be operand zero.
    assert(BinOp->getOperand(0) == Sub &&
           "Unexpected operand use for ptr sub!");
    // Look for the size in operand 1.
    SmallVector<std::pair<User *, unsigned>, 4> SizeUseStack;
    bool Found = findValueMultipleOfSizeInst(U, 1, OrigSize, SizeUseStack);
    assert(Found && "Couldn't find size div for ptr sub!");
    if (Found)
      replaceSizeValue(BinOp, SizeUseStack, OrigSize, ReplSize);
  }
}

void DTransOptBase::replaceSizeValue(
    Instruction *BaseI,
    SmallVectorImpl<std::pair<User *, unsigned>> &SizeUseStack,
    uint64_t OrigSize, uint64_t ReplSize) {
  // If we need to replace a constant in some instruction other than our
  // call, we need to check all the values in our use stack before the call
  // to see if they have other users. If they do, we'll need to clone all
  // values before and including the first one with multiple uses.
  // Note that we are walking from the bottom of the stack here -- that is
  // from the call instruction back to the use where the constant was found.
  bool NeedToClone = false;
  std::pair<User *, unsigned> PrevPair;
  for (auto &UsePair : SizeUseStack) {
    // Skip over the base instruction, its number of users doesn't matter.
    if (UsePair.first == BaseI) {
      PrevPair = UsePair;
      continue;
    }

    // If we haven't seen a value with multiple uses yet, and this value
    // doesn't have multiple uses, don't clone it.
    if (!NeedToClone && (UsePair.first->getNumUses() == 1)) {
      PrevPair = UsePair;
      continue;
    }

    // Otherwise, we need to clone.
    NeedToClone = true;
    auto *OrigUse = cast<Instruction>(UsePair.first);
    auto *Clone = OrigUse->clone();
    if (OrigUse->hasName())
      Clone->setName(OrigUse->getName() + ".dt");
    Clone->insertBefore(OrigUse);
    UsePair.first = Clone;

    // Also replace the use of this value in the previous instruction
    // on the stack.
    User *PrevUser = PrevPair.first;
    unsigned PrevIdx = PrevPair.second;
    assert((PrevUser->getOperand(PrevIdx) == OrigUse) &&
           "Size use stack is broken!");
    PrevUser->setOperand(PrevIdx, Clone);

    // Get ready for the next iteration.
    PrevPair = UsePair;
  }

  // Figure out the multiplier, if any, needed for the size constant.
  std::pair<User *, unsigned> SizePair = SizeUseStack.back();
  User *SizeUser = SizePair.first;
  unsigned SizeOpIdx = SizePair.second;
  auto *ConstVal = cast<ConstantInt>(SizeUser->getOperand(SizeOpIdx));
  uint64_t ConstSize = ConstVal->getLimitedValue();
  assert((ConstSize % OrigSize) == 0 && "Size multiplier search is broken");
  uint64_t Multiplier = ConstSize / OrigSize;

  LLVM_DEBUG(dbgs() << "Delete field: Updating size operand (" << SizeOpIdx
                    << ") of " << *SizeUser << "\n");
  llvm::Type *SizeOpTy = SizeUser->getOperand(SizeOpIdx)->getType();
  SizeUser->setOperand(SizeOpIdx,
                       ConstantInt::get(SizeOpTy, ReplSize * Multiplier));
  LLVM_DEBUG(dbgs() << "  New value: " << *SizeUser << "\n");
}

// This helper function searches, starting with \p U operand \p Idx and
// following only multiply operations, for a User value with an operand that
// is a constant integer and is an exact multiple of the specified size. If a
// match is found, the \p UseStack vector will be populated with <User, Index>
// pairs of the use chain between \p U and the value where the constant was
// found.
//
// The return value indicates whether or not a match was found.
bool DTransOptBase::findValueMultipleOfSizeInst(
    User *U, unsigned Idx, uint64_t Size,
    SmallVectorImpl<std::pair<User *, unsigned>> &UseStack) {
  if (!U)
    return false;

  // Get the specified operand value.
  Value *Val = U->getOperand(Idx);

  // Is it a constant?
  if (auto *ConstVal = dyn_cast<ConstantInt>(Val)) {
    // If so, is it a multiple of the size?
    uint64_t ConstSize = ConstVal->getLimitedValue();
    if (ConstSize == ~0ULL || ((ConstSize % Size) != 0))
      return false;
    // If it is, this is what we were looking for.
    UseStack.push_back(std::make_pair(U, Idx));
    return true;
  }

  // Is it a binary operator?
  if (auto *BinOp = dyn_cast<BinaryOperator>(Val)) {
    // Not a mul? Then it's not what we're looking for.
    if (BinOp->getOpcode() != Instruction::Mul)
      return false;
    // If it is a mul, speculatively push the current value operand pair
    // on the use stack and then check both operands for a constant multiple.
    UseStack.push_back(std::make_pair(U, Idx));
    if (findValueMultipleOfSizeInst(BinOp, 0, Size, UseStack))
      return true;
    if (findValueMultipleOfSizeInst(BinOp, 1, Size, UseStack))
      return true;
    // If neither matched, get our pair off of the stack and return false.
    UseStack.pop_back();
    return false;
  }

  // Is it sext or zext?
  if (isa<SExtInst>(Val) || isa<ZExtInst>(Val)) {
    // If so, speculatively push the current value operand pair on the stack
    // and check the operand for a constant multiple.
    UseStack.push_back(std::make_pair(U, Idx));
    if (findValueMultipleOfSizeInst(cast<User>(Val), 0, Size, UseStack))
      return true;
    // If it didn't match, get our pair off of the stack and return false.
    UseStack.pop_back();
    return false;
  }

  // Otherwise, it's definitely not what we were looking for.
  return false;
}

void DTransOptBase::deleteCallInfo(dtrans::CallInfo *CInfo) {
  Instruction *I = CInfo->getInstruction();
  Function *F = I->getParent()->getParent();
  auto &InfoVec = FunctionToCallInfoVec[F];

  auto It = std::find(InfoVec.begin(), InfoVec.end(), CInfo);
  InfoVec.erase(It);
  DTInfo.deleteCallInfo(I);
}
