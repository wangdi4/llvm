//===-- DTransOptBase.cpp - Common base classes for DTrans Transforms---==//
//
// Copyright (C) 2018-2019 Intel Corporation. All rights reserved.
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
#include "Intel_DTrans/Transforms/DTransOptUtils.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/IR/DebugInfo.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Transforms/Utils/Cloning.h"

using namespace llvm;

#define DEBUG_TYPE "dtrans-optbase"
#define DEBUG_DTRANS_VERIFICATION "dtrans-verification"

namespace {
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
// This option is used during testing to allow changes of types for
// function declarations.
static cl::opt<bool>
    DTransOptBaseProcessFuncDecl("dtrans-optbase-process-function-declaration",
                                 cl::init(false), cl::ReallyHidden);
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
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

  if (SrcTy->isVectorTy()) {
    Type *ReplTy = computeReplacementType(SrcTy->getVectorElementType());
    if (!ReplTy)
      return nullptr;
    return VectorType::get(ReplTy, SrcTy->getVectorNumElements());
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

      if (NeedsReplaced) {
        return StructType::get(StructTy->getContext(), DataTypes,
                               StructTy->isPacked());
      }
    }
  }

  // Note: We do not look for named 'struct' types directly in the above tests
  // because those should have been directly added to the type mapping via
  // AddTypeMapping by the transformation.

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

#if !defined(NDEBUG)
  // Do a sanity check of the IR for the DTrans optimizations to catch any
  // problems introduced while the transformations are being developed. This
  // code may be removed later after DTrans is stable. verifyModule returns
  // 'true' if errors are found.
  if (verifyModule(M, &dbgs())) {
    DEBUG_WITH_TYPE(DEBUG_DTRANS_VERIFICATION, dbgs() << M);
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
  dtrans::collectAllStructTypes(M, KnownStructTypes);

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
  for (StructType *StTy : KnownStructTypes)
    collectDependenciesForType(StTy);

#if !defined(NDEBUG)
  LLVM_DEBUG(dumpTypeToTypeSetMapping("Type dependency mapping table:",
                                      TypeToDependentTypes));
#endif // !defined(NDEBUG)
}

void DTransOptBase::buildTypeEnclosingMapping() {
  for (StructType *StTy : KnownStructTypes)
    collectEnclosingForType(StTy);

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
      Type *BaseTy = dtrans::unwrapType(MemberType);
      if (auto *FunctionTy = dyn_cast<FunctionType>(BaseTy))
        collectDependenciesForTypeRecurse(Dependee, FunctionTy);
      else
        UpdateTypeToDependentTypeMap(BaseTy, Dependee);
    }
    return;
  }

  if (auto *ArrayTy = dyn_cast<ArrayType>(Ty)) {
    Type *BaseTy = dtrans::unwrapType(ArrayTy->getElementType());
    if (auto *FunctionTy = dyn_cast<FunctionType>(BaseTy))
      collectDependenciesForTypeRecurse(Dependee, FunctionTy);
    else
      UpdateTypeToDependentTypeMap(BaseTy, Dependee);

    return;
  }

  if (auto *FuncTy = dyn_cast<FunctionType>(Ty)) {
    Type *RetTy = FuncTy->getReturnType();
    Type *BaseTy = dtrans::unwrapType(RetTy);
    UpdateTypeToDependentTypeMap(BaseTy, Dependee);

    unsigned Total = FuncTy->getNumParams();
    for (unsigned Idx = 0; Idx < Total; ++Idx) {
      Type *ParmTy = FuncTy->getParamType(Idx);
      Type *BaseTy = dtrans::unwrapType(ParmTy);
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
  for (Type *OrigTy : KnownStructTypes) {
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
      // If StructTy is literal, defer creation until it is used.
      if (StructTy->isLiteral())
        continue;

      Type *ReplacementTy = StructType::create(
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

  // Check for debug information that we do not want to be cloned when
  // cloning/remapping functions, and set those objects to map to themselves
  // within the metadata portion of the Value-to-Value map. Specifically, we
  // need to do this for all DISubprogram metadata objects. Otherwise the calls
  // to CloneFucntionInto and remapFunction will create new versions of these
  // objects. Multiple llvm.debug.value intrinsic calls can point to the same
  // metadata object when the DILocalVariable comes from an inlined routine.
  // This leads to consistency problems because there would be two DISubprogram
  // scopes for the routine if we allowed the metadata to be cloned (the
  // variable would point to one scope, but the debug line location would point
  // to the other due to the way cloneFunctionInto operates.)
  // We delete the original function after a cloned function is created, so in
  // the end there will only be one DISubprogram object needed anyway.
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
    if (DTInfo) {
      dbgs() << "Call info after remapping\n";
      DTInfo->printCallInfo(dbgs());
    }
  });

  // The Function to CallInfo mapping is no longer needed, and can be released
  // now.
  resetFunctionCallInfoMapping();
}

// Set up the Function to CallInfo mapping that is needed for keeping
// the CallInfo objects up to date while the transformation is running.
//
// If the class is being used without the DTrans analysis, then call
// info tracking and updating will not be supported when transforming
// routines, and this function will not do anything.
void DTransOptBase::initializeFunctionCallInfoMapping() {
  if (!DTInfo)
    return;

  resetFunctionCallInfoMapping();

  for (auto *CInfo : DTInfo->call_info_entries()) {
    Function *F = CInfo->getInstruction()->getParent()->getParent();
    FunctionToCallInfoVec[F].push_back(CInfo);
  }
}

// This function is used to update the CallInfo objects associated
// with a specific function. The type list of each call info will be
// updated to reflect the remapped types. For cloned functions, the
// instruction pointer in the call info will be updated to point to the
// instruction in the cloned function.
//
// If the class is being used without the DTrans analysis, then call
// info tracking and updating will not be supported when transforming
// routines, and this function will not do anything.
void DTransOptBase::updateCallInfoForFunction(Function *F, bool isCloned) {
  if (!DTInfo)
    return;

  if (FunctionToCallInfoVec.count(F))
    for (auto *CInfo : FunctionToCallInfoVec[F]) {
      if (isCloned)
        DTInfo->replaceCallInfoInstruction(
            CInfo, cast<Instruction>(VMap[CInfo->getInstruction()]));

      dtrans::PointerTypeInfo &PTI = CInfo->getPointerTypeInfoRef();
      size_t Num = PTI.getNumTypes();
      for (size_t i = 0; i < Num; ++i)
        PTI.setType(i, TypeRemapper->remapType(PTI.getType(i)));
    }
}

// Update the attributes which contain types which have been remapped to new
// types, such as created when cloning the following function definition:
//   define void @test01(%struct.type01b* byval(%struct.type01b) %in)
//
// CloneFunctionInto() will have propagated the source attributes to produce:
//  define void @test01.1(%_DT_struct.type01b* byval(%struct.type01b) %in)
//
// Update this to have the remapped type for the byval attribute.
//   define void @test0.1(%_DT_struct.type01b* byval(%_DT_struct.type01b) %in)
//
void DTransOptBase::updateAttributeTypes(Function *CloneFunc) {
  unsigned ArgIdx = 0;
  LLVMContext &Context = CloneFunc->getContext();
  for (Argument &I : CloneFunc->args()) {
    if (I.hasByValAttr()) {
      llvm::Type *Ty = I.getParamByValType();
      llvm::Type *RemapTy = TypeRemapper->remapType(Ty);
      if (Ty != RemapTy) {
        CloneFunc->removeParamAttr(ArgIdx, Attribute::ByVal);
        CloneFunc->addParamAttr(ArgIdx,
                                Attribute::getWithByValType(Context, RemapTy));
      }
    }
    ++ArgIdx;
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
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
    else if (DTransOptBaseProcessFuncDecl)
      WL.push_back(&F);
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
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
      NewGV->setAlignment(MaybeAlign(GV->getAlignment()));
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
      Type *RemapTy = VMapIt->second->getType();
      auto *NewAlias = GlobalAlias::create(
          RemapTy->getPointerElementType(), Alias.getType()->getAddressSpace(),
          Alias.getLinkage(), "", Mapper.mapConstant(*Aliasee), &M);
      NewAlias->takeName(&Alias);
      VMap[&Alias] = NewAlias;
      GlobalsForRemoval.push_back(&Alias);

      LLVM_DEBUG(dbgs() << "DTRANS-OPTBASE: Global alias replacement:\n  Orig: "
                        << Alias << "  New : " << *NewAlias << "\n");
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
      }

      LLVM_DEBUG(dbgs() << "DTRANS-OPTBASE: Global Var replacement:\n  Orig: "
                        << *OrigGV << "\n  New : " << *VarToRemap << "\n");
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

void DTransOptBase::deleteCallInfo(dtrans::CallInfo *CInfo) {
  assert(DTInfo && "DTransOptBase::deleteCallInfo may only be used when base "
                   "class has DTrans analysis");

  Instruction *I = CInfo->getInstruction();
  Function *F = I->getParent()->getParent();
  auto &InfoVec = FunctionToCallInfoVec[F];

  auto It = std::find(InfoVec.begin(), InfoVec.end(), CInfo);
  assert(It != InfoVec.end() && "Unknown CallInfo for function");
  InfoVec.erase(It);
  DTInfo->deleteCallInfo(I);
}
