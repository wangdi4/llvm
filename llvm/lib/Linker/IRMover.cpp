//===- lib/Linker/IRMover.cpp ---------------------------------------------===//
// INTEL_CUSTOMIZATION
//
// INTEL CONFIDENTIAL
//
// Modifications, Copyright (C) 2021-2022 Intel Corporation
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you ("License"). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.
//
// end INTEL_CUSTOMIZATION
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "llvm/Linker/IRMover.h"
#include "LinkDiagnosticInfo.h"
#include "llvm/ADT/SetVector.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/ADT/Triple.h"
#if INTEL_CUSTOMIZATION
#if INTEL_FEATURE_SW_DTRANS
#include "Intel_DTrans/Analysis/DTransTypeMetadataPropagator.h"
#include "Intel_DTrans/Analysis/DTransTypes.h"
#include "Intel_DTrans/Analysis/TypeMetadataReader.h"
#include "llvm/Demangle/Demangle.h"
#endif // INTEL_FEATURE_SW_DTRANS
#endif // INTEL_CUSTOMIZATION
#include "llvm/IR/AutoUpgrade.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DebugInfoMetadata.h"
#include "llvm/IR/DiagnosticPrinter.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/GVMaterializer.h"
#include "llvm/IR/GlobalValue.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/PseudoProbe.h"
#include "llvm/IR/TypeFinder.h"
#include "llvm/Object/ModuleSymbolTable.h"
#include "llvm/Support/CommandLine.h" // INTEL
#include "llvm/Support/Error.h"
#include "llvm/Support/Path.h"
#include "llvm/Transforms/Utils/ValueMapper.h"
#include <utility>
using namespace llvm;

#if INTEL_CUSTOMIZATION
#if INTEL_FEATURE_SW_DTRANS
using namespace dtransOP;

#define DEBUG_DTRANS_TYPES "irmover-dtrans-types"
#endif // INTEL_FEATURE_SW_DTRANS
#endif // INTEL_CUSTOMIZATION

//===----------------------------------------------------------------------===//
// TypeMap implementation.
//===----------------------------------------------------------------------===//

namespace {

#if INTEL_CUSTOMIZATION
static cl::opt<bool> TypeMerging(
    "irmover-type-merging", cl::Hidden, cl::init(true),
    cl::desc("enable type merge in irmover for smaller IR"));

#if INTEL_FEATURE_SW_DTRANS
// Enable mapping the types from the source module to the destination module
// using the DTrans information.
static cl::opt<bool> EnableMergeWithDTrans(
    "irmover-enable-merge-with-dtrans", cl::Hidden, cl::init(true),
    cl::desc("enabled types merging by using the DTrans information"));

// Enable mapping the types using the mangled names even if the DTrans metadata
// is incomplete. An incomplete metadata is when the DTrans metadata is not
// available for all structures in the source and destination modules. This
// should be used only for testing purposes since incomplete metadata can lead
// to incorrect type merging (e.g. mixing fields with opaque pointers, not
// repairing fields that should be function pointers, etc.).
//
// TODO: This option needs to be turned off once we move to opaque pointers
// and we expect to have complete DTrans medatata (e.g. passes generates new
// entries, exception handling is correct, etc.).
static cl::opt<bool> EnableIncompleteDTransMetadata(
    "irmover-enable-dtrans-incomplete-metadata", cl::Hidden, cl::init(true),
    cl::desc("enable to use DTrans metadata even if it was generated "
             "incomplete"));

// Enable to use the DTrans pointer types to check if two LLVM pointer types
// are isomorphic even they are NOT opaque pointers.
//
// TODO: We can turn this off once we move to opaque pointers, unless we want
// to use the DTrans metadata to do type mapping for the non-opaque pointers.
static cl::opt<bool> EnableFullDTransTypesCheck(
    "irmover-enable-full-dtrans-types-check", cl::Hidden, cl::init(true),
    cl::desc("use the dtrans type check even if the pointers aren't opaque"));

// Enable verifying that there is no repeated types or special empty structures
// in the destination module. An assertion will be called if one of the
// previous conditions happen. Also, check that the DTrans types in the
// destination's type manager match with the types in the DTrans metadata.
// This should be used for testing purposes.
static cl::opt<bool> EnableVerify(
    "irmover-enable-module-verify", cl::Hidden, cl::init(false),
    cl::desc("enable verifying destination module in irmover"));

// Enable a quick verification for no repeated struct types in the destination module.
// This verifier basically will check if two structures have the same base name
// then they must have different body. This should be used for testing and
// debugging purposes.
static cl::opt<bool> EnableQuickVerify(
    "irmover-enable-quick-module-verify", cl::Hidden, cl::init(false),
    cl::desc("enable a simple check in the destination module"));

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
static cl::opt<bool> TraceDTransMetadataLoss(
    "irmover-trace-dtrans-metadata-loss", cl::Hidden, cl::init(false),
    cl::desc("print a trace that shows which source module is inserting "
             "llvm::StructType without DTrans metadata"));
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
#endif // INTEL_FEATURE_SW_DTRANS

// Normalizes struct name for type merging.
static StringRef getStructName(const StructType *S);

#if INTEL_FEATURE_SW_DTRANS
// Collect the real structure name.
static StringRef getStructureNameClean(StructType *ST);

// Return true if the name doesn't contain extra numbering
static bool isStructureNameClean(StructType *ST);

// Return the mangled name for the input structure if it is available
static StringRef getMangledNameFromStructure(StructType *ST);

// Return true if the structure represents a base structure
static bool isBaseStructure(StructType *ST);

// Return true if the structure represents an anonymous structure
static bool isAnonStructure(StructType *ST);

/***************** Begin special functions ***************************/
// Return true if the input structure is a special empty structure
static  bool isSpecialEmptyStruct(StructType *ST);

// Return true if the input DTrans structure is a special empty structure
static bool isSpecialEmptyDTransStruct(DTransStructType *DTStruct);

// Return true if the input pointer type is a pointer to an empty
// structure, and the input DTransType represents a pointer to a function type.
static bool isSpecialEmptyStructToFuncMapping(PointerType *SrcPtr,
                                              DTransType *SrcField);
/***************** End special functions ****************************/

// Helper class to handle DTrans types and metadata
class DTransStructsMap {
public:
  DTransStructsMap(Module &M, bool AllowsIncompleteMD,
      std::vector<StructType *> &TypesInModule);
  DTransStructsMap(DTransTypeManager *TM,
      std::vector<StructType *> &TypesInModule) : TM(TM),
      MDReadCorrectly(true), isTMSetByConstruct(true) {
    populateDtransSTMap(TypesInModule);
  }
  ~DTransStructsMap() {
    DTransSTMap.clear();
    if (TM)
      if (isTMSetByConstruct)
        TM = nullptr;
      else
        delete TM;

    if (DtransTypeMDReader)
      delete DtransTypeMDReader;
  }

  // Return the DTransStructType mapped to the input StructType if it is
  // available, else return nullptr
  DTransStructType *getDTransStructure(StructType *ST) {
    if (!ST)
      return nullptr;

    return DTransSTMap[ST];
  }

  // Return the DTransStructType mapped to the input structure name if it
  // is available, else return nullptr
  DTransStructType *getDTransStructure(StringRef StructName) {
    if (!TM || StructName.empty())
      return nullptr;

    return TM->getStructType(StructName);
  }

  // True if the TypeMetadataReader was initialized correctly
  bool isMDReadCorrectly() { return MDReadCorrectly; }

  // Return the map used for mapping an LLVM structure type to a DTrans
  // structure type
  auto getDtransStructMap() const { return DTransSTMap; }
private:
  DTransTypeManager *TM = nullptr;
  TypeMetadataReader *DtransTypeMDReader = nullptr;
  DenseMap<StructType *, DTransStructType *> DTransSTMap;
  bool MDReadCorrectly = false;

  // True if the type manager was passed by the constructor
  bool isTMSetByConstruct = false;

  // Traverse through each input StructType, find the corresponding
  // DTransStruct type, and map them.
  void populateDtransSTMap(std::vector<StructType *> &TypesInModule);
};
#endif // INTEL_FEATURE_SW_DTRANS
#endif // INTEL_CUSTOMIZATION

class TypeMapTy : public ValueMapTypeRemapper {
  /// This is a mapping from a source type to a destination type to use.
  DenseMap<Type *, Type *> MappedTypes;

  /// When checking to see if two subgraphs are isomorphic, we speculatively
  /// add types to MappedTypes, but keep track of them here in case we need to
  /// roll back.
  SmallVector<Type *, 16> SpeculativeTypes;

  SmallVector<StructType *, 16> SpeculativeDstOpaqueTypes;

  /// This is a list of non-opaque structs in the source module that are mapped
  /// to an opaque struct in the destination module.
  SmallVector<StructType *, 16> SrcDefinitionsToResolve;

  /// This is the set of opaque types in the destination modules who are
  /// getting a body from the source module.
  SmallPtrSet<StructType *, 16> DstResolvedOpaqueTypes;

#if INTEL_CUSTOMIZATION
#if INTEL_FEATURE_SW_DTRANS
  DTransStructsMap *DTransSrcStructsMap = nullptr;
  DTransStructsMap *DTransDstStructsMap = nullptr;
  DTransTypeManager *DstTM = nullptr;

  // Store the source types that were visited but couldn't be mapped
  // to any type
  SetVector<Type *> VisitedTypes;

  // True if the conditions for using the DTrans type mapping are met,
  // else false.
  bool EnableDTransTypesMappingScheme = EnableMergeWithDTrans;

  // True if the conditions for using incomplete metadata are met, else false
  bool AllowsIncompleteMD = false;

  // If true then we are going to use the DTrans metadata to do type merging
  // even we have types pointers
  bool AllowsFullDTransTypeCheck = false;

/***************** Begin special functions ***************************/
  // Map the empty structure with the DTrans function type that will be used
  // for repair the structure.
  DenseMap<PointerType *, DTransPointerType *> TypesToRepair;
/***************** End special functions ***************************/
#endif // INTEL_FEATURE_SW_DTRANS
#endif // INTEL_CUSTOMIZATION

public:
#if INTEL_CUSTOMIZATION
#if INTEL_FEATURE_SW_DTRANS
  TypeMapTy(IRMover::IdentifiedStructTypeSet &DstStructTypesSet,
            DTransTypeManager *DstTM)
      : DstTM(DstTM), DstStructTypesSet(DstStructTypesSet) {}

  ~TypeMapTy() {
    if (DTransSrcStructsMap)
      delete DTransSrcStructsMap;

    if (DTransDstStructsMap)
      delete DTransDstStructsMap;
  }
#else // INTEL_FEATURE_SW_DTRANS
  TypeMapTy(IRMover::IdentifiedStructTypeSet &DstStructTypesSet)
      : DstStructTypesSet(DstStructTypesSet) {}
#endif // INTEL_FEATURE_SW_DTRANS
#endif // INTEL_CUSTOMIZATION

  IRMover::IdentifiedStructTypeSet &DstStructTypesSet;
  /// Indicate that the specified type in the destination module is conceptually
  /// equivalent to the specified type in the source module.
  void addTypeMapping(Type *DstTy, Type *SrcTy);

#if INTEL_CUSTOMIZATION
#if INTEL_FEATURE_SW_DTRANS
  /// Try mapping the structure types in the source module to the destination
  /// module using the DTrans information. Return true if the mapping was done,
  /// else return false.
  bool mapTypesToDTransData(Module &SrcM, Module &DstM);

  /// Update destination DTransTypesManager
  void updateDTransTypeManager();
#endif // INTEL_FEATURE_SW_DTRANS
#endif // INTEL_CUSTOMIZATION

  /// Produce a body for an opaque type in the dest module from a type
  /// definition in the source module.
  void linkDefinedTypeBodies();

  /// Return the mapped type to use for the specified input type from the
  /// source module.
  Type *get(Type *SrcTy);
  Type *get(Type *SrcTy, SmallPtrSet<StructType *, 8> &Visited);

  void finishType(StructType *DTy, StructType *STy, ArrayRef<Type *> ETypes);

  FunctionType *get(FunctionType *T) {
    return cast<FunctionType>(get((Type *)T));
  }

private:
  Type *remapType(Type *SrcTy) override { return get(SrcTy); }

  bool areTypesIsomorphic(Type *DstTy, Type *SrcTy);

#if INTEL_CUSTOMIZATION
#if INTEL_FEATURE_SW_DTRANS
  /// Return true if type is already mapped
  bool typeIsMapped(Type *Ty) { return MappedTypes[Ty] != nullptr; }

  /// Insert the input structure in the VisitedTypes list
  void insertVisitedType(StructType *ST);

  /// Copy the DTransType from source DTransTypeManager to
  /// destination DTransTypeManager
  DTransType *copyDTransType(Type *DstTy, Type *SrcTy,
      DTransType *DTSrcTy, SetVector<Type *> &VisitedTypes);

/***************** Begin special functions ***************************/
  /// Return true if the source type is an empty special structure
  /// that could be mapped to a function pointer, and update the map.
  bool handleEmptyStrSpecialCase(StructType *SrcStr, StructType *DstStr,
                                 unsigned FieldNum);

  /// Check if the input type needs to be repaired
  Type* tryToRepairType(Type *Ty, SmallPtrSet<StructType *, 8> &Visited);
/***************** End special functions ****************************/
#endif // INTEL_FEATURE_SW_DTRANS
#endif // INTEL_CUSTOMIZATION
};
}

#if INTEL_CUSTOMIZATION
#if INTEL_FEATURE_SW_DTRANS
// Initialize the DTrans types information using the input Module. If
// AllowsIncompleteMD and initializing the TypeMetadataReader are false
// then the map DTransSTMap won't be constructed.
DTransStructsMap::DTransStructsMap(Module &M, bool AllowsIncompleteMD,
    std::vector<StructType *> &TypesInModule) {
  TM = new DTransTypeManager(M.getContext());
  DtransTypeMDReader = new TypeMetadataReader(*TM);

  // We set the initializer false to prevent any assertion in the DTrans
  // metadata reader. If we didn't collect the metadata correctly and
  // incomplete metadata is not allowed then we won't generate the map.
  // The result of not generating the map is that the type mapping using
  // the DTrans metadata won't happen and we are going to use the
  // traditional type mapping. Also the type mapping verification can't
  // be used. Once we ensure that there is no metadata loss during the
  // compile step then we can replace the 'false' with '!AllowsIncompleteMD'.
  MDReadCorrectly = DtransTypeMDReader->initialize(M, false);

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  if (TraceDTransMetadataLoss) {
    // Always populate the DTrans map if we are debugging for missing metadata.
    // This will print which source module is adding a structure without
    // metadata.
    populateDtransSTMap(TypesInModule);
    return;
  }
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

  // If incomplete metadata is not allowed then there is no need to collect
  // the DTrans information
  if (!MDReadCorrectly && !AllowsIncompleteMD)
    return;

  populateDtransSTMap(TypesInModule);
}

// Traverse through each StructType in the input vector, find the corresponding
// DTransStruct type, and map them.
void DTransStructsMap::populateDtransSTMap(
    std::vector<StructType *> &TypesInModule) {

  // Traverse through the fields of the structure ST, and if a field
  // is a structure (nested structure), then add it into the map.
  // We need to do this because a structure can have nested structures
  // without name (e.g. literal structures), and we can miss the mapping.
  std::function<void(StructType *, DTransStructType *,
      DenseMap<StructType *, DTransStructType *> &,
      SetVector<StructType *> &)> MapNested =
      [&MapNested](StructType *ST, DTransStructType *DTStr,
      DenseMap<StructType *, DTransStructType *> &DTransSTMap,
      SetVector<StructType *> &Visited) -> void {
    if (!Visited.insert(ST))
      return;

    for (unsigned I = 0, E = ST->getNumElements(); I < E; I++) {
      auto *Field = dyn_cast<StructType>(ST->getElementType(I));
      if (!Field)
        continue;

      auto *DTField =
          dyn_cast_or_null<DTransStructType>(DTStr->getFieldType(I));

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
      if (TraceDTransMetadataLoss) {
        if (!DTField) {
          dbgs() << "    llvm::Type: " << *Field << "\n";
          dbgs() << "    DTransType: None\n\n";
        }
        else if (DTField->getReconstructError()) {
          dbgs() << "    llvm::Type: " << *Field << "\n";
          dbgs() << "    DTransType: ";
          DTField->dump();
          dbgs() << "\n\n";
        }
      }
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

      DTransSTMap.insert({Field, DTField});
      MapNested(Field, DTField, DTransSTMap, Visited);
    }
  };

  SetVector<StructType *> Visited;
  for (StructType *ST : TypesInModule) {
    if (Visited.contains(ST))
      continue;

    DTransStructType *DTStruct = TM->getStructType(ST->getName());
    DTransSTMap.insert({ST, DTStruct});

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
    if (TraceDTransMetadataLoss) {
      if (!DTStruct) {
        dbgs() << "    llvm::Type: " << *ST << "\n";
        dbgs() << "    DTransType: None\n\n";
      }
      else if (DTStruct->getReconstructError()) {
        dbgs() << "    llvm::Type: " << *ST << "\n";
        dbgs() << "    DTransType: ";
        DTStruct->dump();
        dbgs() << "\n\n";
      }
    }
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

    MapNested(ST, DTStruct, DTransSTMap, Visited);
  }
}

/*************************** Begin special functions *******************/
// These are special functions that allows to use the mangled name type
// mapping scheme when pointer types aren't opaque. They can to be removed
// once the CFE can generate opaque pointers. In order to use them we need
// from the front end to generate named empty structures and the DTrans
// type metadata (-Xclang -emit-dtrans-info). This is important because the
// CFE generates incomplete types. For example, assume that you have two
// modules and both includes the same type. The CFE can generate a module
// with the type incomplete in some cases:
//
//   Module A:
//     %struct.STRUCTNAME = type { i32(i32)*, i32 }
//
//   Module B:
//     %struct.STRUCTNAME = type { {}*, i32 }
//
// The structure %struct.STRUCTNAME in module B represents an incomplete type.
// If the IR mover is linking module A with module B then the IR mover will
// try to map the following:
//
//   %struct.STRUCTNAME.0 = type { {}*, i32 }
//   %struct.STRUCTNAME = type { i32(i32)*, i32 }
//
// The type mapping will fail because the type IDs aren't the same. If we try
// to force the mapping between the empty structure with the function pointer
// then we can potentially damage other types, for example:
//
//   %struct.NEWSTRUCT.0 = type { {}*, float }
//   %struct.NEWSTRUCT = type { float(float)*, float }
//
// Assume that we mapped the empty structure in %struct.STRUCTNAME.0 with the
// function type in %struct.STRUCTNAME, then the mapper will change all the
// empty structures and the IR will break:
//
//    %struct.NEWSTRUCT.0 = type { i32(i32)*, float }
//
// As a workaround, the CFE generates an unique name for the empty structures
// when the DTrans metadata is enabled and types are incomplete.
//
// The IR will look as follow:
//
//   %"__Intel$Empty$Struct" = {}
//   %"__Intel$Empty$Struct.0" = {}
//
//   %struct.STRUCTNAME.0 = type { __Intel$Empty$Struct*, i32 }
//   %struct.NEWSTRUCT.0 = type { __Intel$Empty$Struct.0*, float }
//
//   %struct.STRUCTNAME = type { i32(i32)*, i32 }
//   %struct.NEWSTRUCT = type { float(float)*, float }
//
// This sets an unique empty structure for each incomplete structure. It can
// be used to identify which function type will be used for each structure.
// Then we are going to handle the type merging depending on the linking order:
//
//   A) Merging an incomplete structure in source module with a complete
//      structure in the destination module
//
//   B) Adding a new incomplete structure from the source module to the
//      destination module
//
// Solving issue A:
// ----------------
//
// Assume that the IR mover is merging the following types:
//
//   Type in source module:
//       %struct.STRUCTNAME.0 = type { __Intel$Empty$Struct*, i32 }
//
//   Type in destination module:
//       %struct.STRUCTNAME = type { i32(i32)*, i32 }
//
// In this case we are going to map the unique structure with the function
// type, and the pointer to the empty structure with the pointer to the
// function type (function handleEmptyStrSpecialCase) when checking if
// two types are isomorphic. The type merging process will automatically
// merge the types and will update the instructions.
//
//
// Solving issue B:
// ----------------
//
// Assume that a new type is being added into the destination module (type
// wasn't mapped), and it is incomplete:
//
//   Type in source module:
//       %struct.STRUCTNAME = type { __Intel$Empty$Struct*, i32 }
//
// This case we are going to check if there is DTrans metadata for
// %struct.STRUCTNAME in the source module since it contains the actual
// information:
//
//   !5 = !{!"S", %struct.STRUCTNAME zeroinitializer, i32 2, !6, !8}
//   !6 = !{!7, i32 1}
//   !7 = !{!"F", i1 false, i32 1, !8, !8}
//   !8 = !{i32 0, i32 0}
//
// If the metadata exists, then we are going to construct the complete type
// in the destination module and map the incomplete type with the new
// type (repairTypes):
//
//   New type in destination module:
//       %struct.STRUCTNAME = type { i32(i32)*, i32 }
//
// The type mapper will automatically update the instructions with the new
// type.
//
// We don't handle the case when the IR mover tries to merge a complete type
// in the source module with an incomplete type in the destination module
// because it is expected to never happen, unless the DTrans metadata is
// incomplete. The destination module will always have the complete type since
// we are creating it or collecting it from a source module.
//
// Once the CFE can generate opaque pointers this won't be an issue because the
// types will be created as follow:
//
//   %struct.STRUCTNAME.0 = type { ptr, i32}
//   %struct.NEWSTRUCT.0 = type { ptr, float }
//
//   %struct.STRUCTNAME = type { ptr, i32 }
//   %struct.NEWSTRUCT = type { ptr, float }
//
// These types will be merged because they are just pointers.

// Return true if a special empty structure is being mapped to a function type
// and update the map.
// NOTE: It is assumed that SrcStr and DstStr are structures with the same
// properties (same number of elements, same mangled name, etc.).
bool TypeMapTy::handleEmptyStrSpecialCase(StructType *SrcStr,
                                          StructType *DstStr,
                                          unsigned FieldNum) {

  // Check if the source is a pointer to a special empty structure and
  // the destination is a pointer to a function type
  auto MapEmptyStrWithFunc = [this](PointerType *SrcPtr,
                                    PointerType *DstPtr,
                                    DTransType *SrcField) -> bool {

    // If the source is mapped then return the result
    if (MappedTypes[SrcPtr]) {
      if (MappedTypes[SrcPtr] == DstPtr)
        return true;
      return false;
    }

    StructType *SrcPtrTy = dyn_cast<StructType>(SrcPtr->getElementType());
    FunctionType *DstPtrTy = dyn_cast<FunctionType>(DstPtr->getElementType());

    // Source and destination aren't the form we want
    if (!SrcPtrTy || !DstPtrTy)
      return false;

    // Check if the source is an empty structure
    if (!isSpecialEmptyStruct(SrcPtrTy) &&
        !isSpecialEmptyStructToFuncMapping(SrcPtr, SrcField))
      return false;

    // If the source pointer is mapped then check the value
    if (MappedTypes[SrcPtrTy]) {
      if (MappedTypes[SrcPtrTy] == DstPtrTy)
        return true;
      return false;
    }

    // Map the empty structure with the function type
    MappedTypes[SrcPtrTy] = DstPtrTy;
    SpeculativeTypes.push_back(SrcPtrTy);

    // Map the pointer to the empty structure with the pointer to the function
    MappedTypes[SrcPtr] = DstPtr;
    SpeculativeTypes.push_back(SrcPtr);

    return true;
  };

  if (!EnableDTransTypesMappingScheme)
    return false;

  // Look for the following:
  //
  //   %"__Intel$Empty$Struct" = {}
  //   %"struct.MANGLEDNAME.STRUCT.0 = { "__Intel$Empty$Struct"* }"
  //   %"struct.MANGLEDNAME.STRUCT   = { i32(i32)* }"
  //
  //   SrcTy = %"struct.MANGLEDNAME.STRUCT.0
  //   DstTy = %"struct.MANGLEDNAME.STRUCT
  //
  // Source and destination must be structures with the same mangled name, same
  // properties (base, anonymous, etc.), and field I is a pointer to the
  // special empty structure in the source module and a pointer to a function
  // in the destination module.
  if (!SrcStr || !DstStr)
    return false;

  if (FieldNum > SrcStr->getNumElements())
    return false;

  PointerType *PtrSrc =
      dyn_cast<PointerType>(SrcStr->getElementType(FieldNum));
  PointerType *PtrDst =
      dyn_cast<PointerType>(DstStr->getElementType(FieldNum));

  auto DTStructSrc = DTransSrcStructsMap->getDTransStructure(SrcStr);
  if (!DTStructSrc)
    return false;

  DTransType *DTField = DTStructSrc->getFieldType(FieldNum);

  // This is only for non-opaque pointers. When the CFE generates opaque
  // pointers then we will never reach here since opaque pointers will match.
  if (!PtrSrc || !PtrDst || PtrSrc->isOpaque() || PtrDst->isOpaque() ||
      !DTField)
    return false;

  // Check if source and destination types match our pattern.
  if (MapEmptyStrWithFunc(PtrSrc, PtrDst, DTField))
    return true;

  return false;
}

// This function checks if the input type needs to be repaired and returns the
// new pointer type that was created. A type needs repair when it is a field
// of a structure that should be a function pointer but the CFE made it as a
// pointer to an empty structure. For example, assume that the CFE generated
// this structure:
//
//   %"__Intel$Empty$Struct" = type {}
//   %struct._ZTS11TestStructA.TestStructA =
//       type { %"__Intel$Empty$Struct"* }
//
// But the DTrans metadata says that the field is a function pointer:
//
//   !0 = !{!"S", %struct._ZTS11TestStructA.TestStructA zeroinitializer,
//          i32 1, !1}
//   !1 = !{!2, i32 1}
//   !2 = !{!"F", i1 false, i32 1, !3, !4}
//   !3 = !{i32 0, i32 0}
//   !4 = !{%struct._ZTS11TestStructA.TestStructA zeroinitializer, i32 1}
//
// Then it means that the field 0 in the structure
// %struct._ZTS11TestStructA.TestStructA.0 is incomplete and should be a
// function pointer rather than a pointer to an empty structure. To fix this
// we are going to create a function type representing the function, and a
// pointer type that points to the new function type.
//
//   i32(%struct._ZTS11TestStructA.TestStructA*)
//   i32(%struct._ZTS11TestStructA.TestStructA*)*
//
// Then we are going to map the types as follow
//
//   Pointer to empty structure with pointer to new function
//     (%"__Intel$Empty$Struct"* ->
//          i32(%struct._ZTS11TestStructA.TestStructA*)*)
//
//   Empty structure with new function
//     (%"__Intel$Empty$Struct" -> i32(%struct._ZTS11TestStructA.TestStructA*))
//
// This basically tells to the type mapper that we constructed a new type in
// the destination module for the incomplete type, therefore map with it.
//
// NOTE: The DTrans metadata should be available to use this. Once we move
// to opaque pointers we don't need to solve this issue. But it is useful to
// understand this problem.
Type* TypeMapTy::tryToRepairType(Type *Ty,
                                 SmallPtrSet<StructType *, 8> &Visited) {
  auto *PtrField = dyn_cast<PointerType>(Ty);
  if (!PtrField)
    return nullptr;

  if (TypesToRepair.count(PtrField) == 0)
    return nullptr;

  if (typeIsMapped(Ty))
    return MappedTypes[Ty];

  auto *DTFieldPtr = TypesToRepair[PtrField];
  StructType *PtrSrc = cast<StructType>(PtrField->getElementType());
  auto *DTFuncTy =
      cast<DTransFunctionType>(DTFieldPtr->getPointerElementType());

  // We need to generate the function type with the types in the
  // destination module.
  Type *FuncRetTy = get(DTFuncTy->getReturnType()->getLLVMType(), Visited);

  SmallVector<Type *, 8> Arguments;
  for (auto *Arg : DTFuncTy->args())
    Arguments.push_back(get(Arg->getLLVMType(), Visited));

  FunctionType *NewFunc =
      FunctionType::get(FuncRetTy, makeArrayRef(Arguments),
                        DTFuncTy->isVarArg());

  // Map the empty structure with the new function type
  MappedTypes[PtrSrc] = NewFunc;

  // Map the pointers
  PointerType *NewPtr =
      PointerType::get(NewFunc, PtrField->getAddressSpace());
  MappedTypes[PtrField] = NewPtr;

  return cast<Type>(NewPtr);
}
/******************* End special functions **********************************/

// Insert the input StructType, and its nested structures, into the
// VisitedTypes list.
void TypeMapTy::insertVisitedType(StructType *ST) {

  // Return the StructType from a DTransPointerType if it is available
  auto GetStructFromDTransPtr = [](DTransPointerType *DTPtr) -> StructType * {
    if (!DTPtr)
      return nullptr;

    DTransType *CurrPtr = cast<DTransType>(DTPtr);

    while (CurrPtr && isa<DTransPointerType>(CurrPtr)) {
      DTransPointerType *TempPtr = cast<DTransPointerType>(CurrPtr);
      CurrPtr = TempPtr->getPointerElementType();
    }

    assert(CurrPtr && "Null pointer element type from DTransPointerType");

    return dyn_cast<StructType>(CurrPtr->getLLVMType());
  };

  // Return the StructType from a PointerType if it is available
  auto GetStructFromPtr = [](PointerType *Ptr) -> StructType * {
    if (!Ptr || Ptr->isOpaque())
      return nullptr;

    Type *CurrPtr = cast<Type>(Ptr);

    while (CurrPtr && isa<PointerType>(CurrPtr)) {
      PointerType *TempPtr = cast<PointerType>(CurrPtr);
      CurrPtr = TempPtr->getElementType();
    }

    return dyn_cast<StructType>(CurrPtr);
  };

  if (!ST)
    return;

  // If the type is mapped then it can't be in the visited list
  if (typeIsMapped(ST))
    return;

  // Type was already inserted
  if (!VisitedTypes.insert(ST))
    return;

  // Traverse through the fields and insert those fields that are structures
  for (int I = 0, E = ST->getNumElements(); I < E; I++) {
    Type *Field = ST->getElementType(I);
    if (typeIsMapped(Field))
      continue;

    StructType *CurrStruct = nullptr;
    if (auto *Ptr = dyn_cast<PointerType>(Field)) {
      // If we have a pointer and it is opaque, or we are using DTrans metadata
      // for non-opaque pointers, then get element type of the pointer from
      // the DTrans metadata
      if (Ptr->isOpaque() || EnableFullDTransTypesCheck) {
        if (auto *DTStruct = DTransDstStructsMap->getDTransStructure(ST)) {
          auto *DTType = DTStruct->getFieldType(I);
          if (auto *DTFieldPtr = dyn_cast_or_null<DTransPointerType>(DTType))
            CurrStruct = GetStructFromDTransPtr(DTFieldPtr);
        }
      }

      // If we couldn't find the element type from the DTrans metadata then
      // try using llvm::PointerType only if Ptr is not opaque. We need
      // to check this in case the metadata is incomplete and non-opaque
      // pointers are enabled.
      if (!CurrStruct)
        CurrStruct = GetStructFromPtr(Ptr);
    } else if (auto *StrField = dyn_cast<StructType>(Field)) {
      CurrStruct = StrField;
    }

    // Inserted nested structures
    if (CurrStruct)
      insertVisitedType(CurrStruct);
  }
}

// Traverse through the types in the source module and see which types can be
// mapped to the destination module by matching the DTrans information.
bool TypeMapTy::mapTypesToDTransData(Module &SrcM, Module &DstM) {

  // Traverse through the types in the destination module and check which type
  // can be mapped with the input Structure.
  auto UpdateMangledTypeFromDst = [this](StructType *ST,
      SetVector<StructType *> &StructsWithDerivedNames) -> void {

    if (StructsWithDerivedNames.empty())
      return;

    StringRef SrcCompareName = getMangledNameFromStructure(ST);

    // Not all structures will have a mangled name. If a transformation in
    // the backend adds a new structure, then we need to try to catch it by
    // using the clean name (structure's name without extra numbering).
    bool UseMangledName = true;
    if (SrcCompareName.empty()) {
      UseMangledName = false;
      SrcCompareName = getStructureNameClean(ST);
    }

    if (SrcCompareName.empty())
      return;

    bool IsBase = isBaseStructure(ST);
    // Traverse only through the structures that contains derived names
    // (e.g. %struct.test.0)
    for (auto *DST : StructsWithDerivedNames) {
      // Base structures must match
      if (IsBase != isBaseStructure(DST))
        continue;

      StringRef DstCompareName;
      if (UseMangledName)
        DstCompareName = getMangledNameFromStructure(DST);
      else
        DstCompareName = getStructureNameClean(DST);

      if (DstCompareName.empty() || DstCompareName != SrcCompareName)
        continue;

      // Check if the input source type can be mapped with the current
      // destination
      addTypeMapping(DST, ST);
      if (MappedTypes[ST] == DST)
        break;
    }
  };

  // Return true if the DTrans metadata was collected and initialized
  // for the input module, else return false.
  auto BuildDTransStructures = [this](Module &M, DTransStructsMap **DTMap,
      std::vector<StructType *> &TypesInModule) -> bool {
    assert(!(*DTMap) && "DTransStructsMap already allocated");

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
    if (TraceDTransMetadataLoss) {
      dbgs() << "  Checking for metadata loss in "
             << "source module:\n";
    }
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

    DTransStructsMap *DTNewMap =
        new DTransStructsMap(M, AllowsIncompleteMD, TypesInModule);

    // The following checks are only for the source module since we are collecting
    // metadata from it.
    if (!(DTNewMap->isMDReadCorrectly())) {
      // If one of the following messages is printed then it means that there
      // is some metadata loss when the module was created. Try using the
      // following debug flag to identify which type is broken:
      // -mllvm -debug-only=irmover-dtrans-metadata-loss .

      // If incomplete metadata is not allowed then we aren't going to merge
      // the types using the mangled names.
      if (!AllowsIncompleteMD) {
        delete DTNewMap;
        DEBUG_WITH_TYPE(DEBUG_DTRANS_TYPES,
            dbgs() << "  ERROR: DTrans metadata collected incorrectly from "
                   << "source module, merging with mangled names "
                   << "disabled\n\n");
        return false;
      }

      DEBUG_WITH_TYPE(DEBUG_DTRANS_TYPES,
          dbgs() << "  WARNING: DTrans metadata collected incorrectly from "
                 << "source module\n\n");
    }
    *DTMap = DTNewMap;
    return true;
  };

/****************************** Begin special function ***********************/
  // Collect the fields that are pointers to empty structures but in the
  // DTrans metadata they are pointers to functions.
  auto CollectFieldsThatNeedRepair = [this](StructType *ST,
      DTransStructType *DTStruct) -> void {

    DEBUG_WITH_TYPE(DEBUG_DTRANS_TYPES,
        dbgs() << "    Fields that will be repaired: \n");
    for (unsigned I = 0, E = ST->getNumElements(); I < E; I++) {
      PointerType *PtrField = dyn_cast<PointerType>(ST->getElementType(I));
      if (!PtrField || PtrField->isOpaque())
        continue;

      StructType *PtrStrc = dyn_cast<StructType>(PtrField->getElementType());
      if (!PtrStrc)
        continue;

      if (TypesToRepair.count(PtrField) > 0)
        continue;

      auto *DTField = dyn_cast<DTransType>(DTStruct->getFieldType(I));
      // Maybe this should be an assert
      if (!DTField)
        continue;

      if (isSpecialEmptyStruct(PtrStrc) ||
          isSpecialEmptyStructToFuncMapping(PtrField, DTField)) {
        auto *DTFieldPtr =
            dyn_cast<DTransPointerType>(DTField);
        if (!DTFieldPtr)
          continue;

        auto *DTFuncTy =
            dyn_cast<DTransFunctionType>(DTFieldPtr->getPointerElementType());
        if (!DTFuncTy)
          continue;

        DEBUG_WITH_TYPE(DEBUG_DTRANS_TYPES, {
          dbgs() << "      Field [" << I << "]:\n";
          dbgs() << "        Original: " << *PtrField << "\n";
          dbgs() << "        New: ";
          DTFieldPtr->dump();
          dbgs() << "\n";
        });
        TypesToRepair.insert({PtrField, DTFieldPtr});
      }
    }
  };
/**************************** End special function ***************************/
  if (!EnableDTransTypesMappingScheme)
    return false;

  // If there is no DTrans metadata in the source module then we are going
  // to use the original types mapping scheme.
  NamedMDNode *DTransMDTypes = SrcM.getNamedMetadata("intel.dtrans.types");
  if (!DTransMDTypes || !DstTM) {
    EnableDTransTypesMappingScheme = false;
    return false;
  }

  assert((SrcM.getContext().supportsTypedPointers() ==
          DstM.getContext().supportsTypedPointers()) &&
          "Module mismatch for typed pointers support");

  // Incomplete metadata is only allowed when typed pointers are available
  // since we can reconstruct the types information in this case. If
  // opaque pointers are available then we can't restore the types
  // information.
  AllowsIncompleteMD = EnableIncompleteDTransMetadata &&
                       SrcM.getContext().supportsTypedPointers();

  // We are going to use the DTrans metadata for opaque pointers or typed
  // pointers if requested
  AllowsFullDTransTypeCheck = !SrcM.getContext().supportsTypedPointers() ||
                              EnableFullDTransTypesCheck;

  DEBUG_WITH_TYPE(DEBUG_DTRANS_TYPES,
      dbgs() << "Merging types from source module: "
             << SrcM.getName() << "\n\n");

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  if (TraceDTransMetadataLoss)
    dbgs() << "Merging types from source module: "
           << SrcM.getName() << "\n\n";
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

  std::vector<StructType *> SrcTypes = SrcM.getIdentifiedStructTypes();
  // NOTE: We collect the DTrans structures first because there is a chance
  // that the types name change while checking for isomorphism. If a types'
  // name changes then we can't collect the correct metadata from the types
  // manager.
  if (!BuildDTransStructures(SrcM, &DTransSrcStructsMap, SrcTypes)) {
    EnableDTransTypesMappingScheme = false;
    return false;
  }

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  if (TraceDTransMetadataLoss)
    dbgs() << "  Checking for metadata loss in "
           << "destination module:\n";
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

  // This SetVector will store the structures that have extra numbering at the
  // end of the name in the destination module. If we can't catch the type by
  // checking the clean name (e.g. %struct.test), then we need to check with
  // the list of derived names (e.g. %struct.test.0).
  SetVector<StructType *> StructsWithDerivedNames;

  // This std::vector will contain all the structures that have a name. It
  // will be used to create the DTransStructsMap for the destination module.
  std::vector<StructType *> DstTypes;
  for (auto *DTDstTy : DstTM->getIdentifiedStructTypes()) {
    auto *DST = cast<StructType>(DTDstTy->getLLVMType());

    // Perhaps this should be an assert, the destination module
    // should not have the special empty structures generated by the CFE.
    if (isSpecialEmptyStruct(DST))
      continue;

    if (!DST->hasName())
      continue;

    DstTypes.push_back(DST);

    if (isStructureNameClean(DST))
      continue;

    if (isAnonStructure(DST))
      continue;

    StructsWithDerivedNames.insert(DST);
  }

  // Destination DTrans structs map will always use the DstTM
  DTransDstStructsMap = new DTransStructsMap(DstTM, DstTypes);

  // Do the mapping
  for (StructType *ST : SrcTypes) {
    if (!ST->hasName())
      continue;

    // Ignore special empty strutures (this will go away once the CFE
    // generates OP)
    if (isSpecialEmptyStruct(ST))
      continue;

    // If the current source was mapped already, then continue.
    if (typeIsMapped(ST))
      continue;

    // Anonymous structures are a special case. They can be repeated in the
    // destination module. For example:
    //
    //   struct TestStruct {
    //
    //     struct {
    //       int i;
    //     };
    //
    //     struct {
    //       int j;
    //     };
    //   };
    //
    // There is a chance that the CFE produces the following IR from the
    // previous structure:
    //
    //   %struct.anon = type { i32 }
    //   %struct.anon.0 = type { i32 }
    //   %struct.TestStruct = type { %struct.anon, %struct.anon.0 }
    //
    // Notice that both anonymous structures will be added to the destination
    // module since they are technically two different structures in the source
    // module. If a new source module is being merged with the destination
    // module, then we need to make sure we don't mix the names (e.g., trying
    // to merge %struct.anon.0.1 with %struct.anon rather than %struct.anon.0).
    //
    // One thing we know is that anonymous structures are encapsulated
    // structures. This means that when the parent structures are being checked
    // for isomorphism, the anonymous structures will be verified too.
    // Therefore we can ignore them at this point.
    if (isAnonStructure(ST))
        continue;

    // We need to collect the DTrans structure now because there is a chance
    // that the structure name changes when it gets mapped.
    DTransStructType *DTStruct = DTransSrcStructsMap->getDTransStructure(ST);

    DEBUG_WITH_TYPE(DEBUG_DTRANS_TYPES,
        dbgs() << "  Source type: " << *ST << "\n");

    // Get the type name clean (without the extra numbering)
    StringRef SrcCompareName = getStructureNameClean(ST);

    // Check to see if the destination module has a struct with the clean name.
    // If so, then proceed to try doing the mapping.
    StructType *DST = StructType::getTypeByName(ST->getContext(), SrcCompareName);

    // Prioritize the check with the structure that contains the clean name
    if (DST && DstStructTypesSet.hasType(DST))
      addTypeMapping(DST, ST);

    // If the type wasn't mapped with the clean name structure, then we need
    // to check if it matches with one of the derived forms
    if (!typeIsMapped(ST))
      UpdateMangledTypeFromDst(ST, StructsWithDerivedNames);

/************************ Begin special functions ****************************/
    // Check if the type needs to be repaired. This can go away once we move
    // to opaque pointers since this check is for non-opaque pointers.
    bool TypeMayNeedRepair = false;
    if (typeIsMapped(ST)) {
      auto DstTy = cast<StructType>(MappedTypes[ST]);
      // If the destination type is opaque then we need to fix the types
      if (DstTy->isOpaque())
        TypeMayNeedRepair = true;
      DEBUG_WITH_TYPE(DEBUG_DTRANS_TYPES,
          dbgs() << "    Destination type: " << *DstTy << "\n");
    } else {
      TypeMayNeedRepair = true;
      DEBUG_WITH_TYPE(DEBUG_DTRANS_TYPES,
          dbgs() << "    Destination type: None\n");
    }

    if (TypeMayNeedRepair && DTStruct)
      CollectFieldsThatNeedRepair(ST, DTStruct);
/************************ End special functions ****************************/

    // If the type wasn't mapped then insert it in VisitedTypes. We don't
    // want to revisit or change these types (function TypeMapTy::get).
    if (!typeIsMapped(ST))
      insertVisitedType(ST);

    DEBUG_WITH_TYPE(DEBUG_DTRANS_TYPES, dbgs() << "\n");
    // TODO: Handle the case for dope vectors
  }

  return true;
}

#endif // INTEL_FEATURE_SW_DTRANS
#endif // INTEL_CUSTOMIZATION

void TypeMapTy::addTypeMapping(Type *DstTy, Type *SrcTy) {
  assert(SpeculativeTypes.empty());
  assert(SpeculativeDstOpaqueTypes.empty());

  // Check to see if these types are recursively isomorphic and establish a
  // mapping between them if so.
  if (!areTypesIsomorphic(DstTy, SrcTy)) {
    // Oops, they aren't isomorphic.  Just discard this request by rolling out
    // any speculative mappings we've established.
    for (Type *Ty : SpeculativeTypes)
      MappedTypes.erase(Ty);

    SrcDefinitionsToResolve.resize(SrcDefinitionsToResolve.size() -
                                   SpeculativeDstOpaqueTypes.size());
    for (StructType *Ty : SpeculativeDstOpaqueTypes)
      DstResolvedOpaqueTypes.erase(Ty);
  } else {
    // SrcTy and DstTy are recursively ismorphic. We clear names of SrcTy
    // and all its descendants to lower amount of renaming in LLVM context
    // Renaming occurs because we load all source modules to the same context
    // and declaration with existing name gets renamed (i.e Foo -> Foo.42).
    // As a result we may get several different types in the destination
    // module, which are in fact the same.
    for (Type *Ty : SpeculativeTypes)
      if (auto *STy = dyn_cast<StructType>(Ty))
        if (STy->hasName())
          STy->setName("");
  }
  SpeculativeTypes.clear();
  SpeculativeDstOpaqueTypes.clear();
}

/// Recursively walk this pair of types, returning true if they are isomorphic,
/// false if they are not.
bool TypeMapTy::areTypesIsomorphic(Type *DstTy, Type *SrcTy) {
#if INTEL_CUSTOMIZATION
#if INTEL_FEATURE_SW_DTRANS
  // Given a source structure, a destination structure and a field number,
  // collect the DTransPointer field from the structures DTrans information if
  // the fields are pointer type. If not, then check if the fields are array
  // or vector, and collect the elements type if they are a pointer types.
  // Return true if the information was collected, else return false.
  // NOTE: It is assumed that SrcStr and DstStr are structures with the same
  // properties (same number of elements, same mangled name, etc.)
  auto GetDTransPointerFields = [this](StructType *SrcST, StructType *DstST,
      unsigned FieldNum, DTransPointerType **SrcPtrField,
      DTransPointerType **DstPtrField) -> bool {

    *SrcPtrField = nullptr;
    *DstPtrField = nullptr;

    if (!EnableDTransTypesMappingScheme)
      return false;

    // If there is no opaque pointers or the use of DTrans metadata is not
    // allowed for typed pointers then we return.
    if (!AllowsFullDTransTypeCheck)
      return false;

    if (!SrcST || !DstST)
      return false;

    if (FieldNum > SrcST->getNumElements())
      return false;

    auto DTStructSrc = DTransSrcStructsMap->getDTransStructure(SrcST);
    auto DTStructDst = DTransDstStructsMap->getDTransStructure(DstST);
    if (!DTStructSrc || !DTStructDst)
      return false;

    // If the DTrans metadata was constructed incorrectly then we can't query
    // it.
    if (DTStructSrc->getReconstructError() ||
        DTStructDst->getReconstructError()) {
      if (AllowsIncompleteMD)
        return false;
      else
        llvm_unreachable("Collecting information from incomplete "
                         "DTrans type");
    }

    PointerType *PtrSrc =
        dyn_cast<PointerType>(SrcST->getElementType(FieldNum));
    PointerType *PtrDst =
        dyn_cast<PointerType>(DstST->getElementType(FieldNum));

    // Fields are pointer types, collect the DTrans type information
    if (PtrSrc && PtrDst) {
      assert((PtrSrc->isOpaque() == PtrDst->isOpaque()) &&
             "Mapping between opaque pointers from/to non-opaque pointers "
             "isn't allowed");

      auto *DTFieldPtrSrc =
          dyn_cast<DTransPointerType>(DTStructSrc->getFieldType(FieldNum));
      auto *DTFieldPtrDst =
          dyn_cast<DTransPointerType>(DTStructDst->getFieldType(FieldNum));

      if (!DTFieldPtrSrc || !DTFieldPtrDst)
        return false;

      *SrcPtrField = DTFieldPtrSrc;
      *DstPtrField = DTFieldPtrDst;

      return true;
    } else {

      // If the fields are array or vector types then we need to
      // collect the element type
      DTransType *DTSrcFieldTy = DTStructSrc->getFieldType(FieldNum);
      DTransType *DTDstFieldTy = DTStructDst->getFieldType(FieldNum);

      assert(DTSrcFieldTy && "Field comes from incomplete source structure");

      assert(DTDstFieldTy && "Field comes from incomplete "
                             " destination structure");

      // We need to make sure that both fields are array or vector,
      // but they can't be array and vector
      bool BothFieldsAreSeqTypes = (isa<DTransArrayType>(DTSrcFieldTy) &&
                                    isa<DTransArrayType>(DTDstFieldTy)) ||
                                   (isa<DTransVectorType>(DTSrcFieldTy) &&
                                    isa<DTransVectorType>(DTDstFieldTy));

      // Iterate in case we have nested arrays or vectors
      while (BothFieldsAreSeqTypes) {
        auto SrcArr = cast<DTransSequentialType>(DTSrcFieldTy);
        auto DstArr = cast<DTransSequentialType>(DTDstFieldTy);

        // Number of elements should be the same
        if (SrcArr->getNumElements() != DstArr->getNumElements())
          return false;

        DTSrcFieldTy = SrcArr->getElementType();
        DTDstFieldTy = DstArr->getElementType();

        // Check the element type in case we need to iterate
        BothFieldsAreSeqTypes = (isa<DTransArrayType>(DTSrcFieldTy) &&
                                 isa<DTransArrayType>(DTDstFieldTy)) ||
                                (isa<DTransVectorType>(DTSrcFieldTy) &&
                                 isa<DTransVectorType>(DTDstFieldTy));
      }

      auto *DTFieldPtrSrc = dyn_cast<DTransPointerType>(DTSrcFieldTy);
      auto *DTFieldPtrDst = dyn_cast<DTransPointerType>(DTDstFieldTy);

      // Both types must be pointer types, if not then:
      //   a. At least one array or vector is not a pointer (*int[] vs int[])
      //   b. Different dereference level (*int[][] vs *int[])
      if (!DTFieldPtrSrc || !DTFieldPtrDst)
        return false;

      auto *PtrSrc =
          dyn_cast_or_null<PointerType>(DTFieldPtrSrc->getLLVMType());
      auto *PtrDst =
          dyn_cast_or_null<PointerType>(DTFieldPtrDst->getLLVMType());

      // If the LLVM type is not a pointer type then it means that the DTrans
      // type is formed incorrectly. Perhaps we should assert here.
      if (!PtrSrc || !PtrDst)
        return false;

      assert((PtrSrc->isOpaque() == PtrDst->isOpaque()) &&
             "Mapping between opaque pointers from/to non-opaque pointers "
             "isn't allowed");

      // Found that the type of the array or vector is a pointer, return the
      // information.
      *SrcPtrField = DTFieldPtrSrc;
      *DstPtrField = DTFieldPtrDst;

      return true;
    }

    return false;
  };

  // Return true if the source pointer field matches the destination pointer
  // field, else return false.
  auto ArePointerFieldsIsomorphic = [this](DTransPointerType *SrcPtrField,
      DTransPointerType *DstPtrField) -> bool {
    Type *SrcElementTy = nullptr;
    Type *DstElementTy = nullptr;

    // Traverse through the dereference levels of SrcPtrTy and DstPtrTy to get
    // the LLVM pointer element type.
    DTransType *CurrSrc = SrcPtrField;
    DTransType *CurrDst = DstPtrField;

    while (isa<DTransPointerType>(CurrSrc) &&
           isa<DTransPointerType>(CurrDst)) {

      auto *SrcPtr = cast<DTransPointerType>(CurrSrc);
      auto *DstPtr = cast<DTransPointerType>(CurrDst);

      CurrSrc = SrcPtr->getPointerElementType();
      CurrDst = DstPtr->getPointerElementType();
    }

    SrcElementTy = CurrSrc->getLLVMType();
    DstElementTy = CurrDst->getLLVMType();

    // If there is no LLVM type then the DTrans type was created incorrectly
    assert(SrcElementTy &&
           "Failed to collect element type from source pointer");
    assert(DstElementTy &&
           "Failed to collect element type from destination pointer");

    // If two pointer types are the same then return
    if (SrcElementTy == DstElementTy)
      return true;

    // Now keep checking if the types are isomorphic
    return areTypesIsomorphic(DstElementTy, SrcElementTy);
  };

  // Return true if both input types are structures and have the same
  // properties, else return false
  auto StructsAreTheSame = [this](StructType *SrcStr,
                                  StructType *DstStr) -> bool {
    if (!EnableDTransTypesMappingScheme)
      return false;

    if (!SrcStr || !DstStr)
      return false;

    // Check for packed
    if (SrcStr->isPacked() != DstStr->isPacked())
      return false;

    // Number of elements must match
    if (SrcStr->getNumElements() != DstStr->getNumElements())
      return false;

    // Base or anonymous must match
    if (isBaseStructure(SrcStr) != isBaseStructure(DstStr))
      return false;

    if (isAnonStructure(SrcStr) != isAnonStructure(DstStr))
      return false;

    // Clean names must match
    StringRef SrcCleanName = getStructureNameClean(SrcStr);
    StringRef DstCleanName = getStructureNameClean(DstStr);

    return SrcCleanName == DstCleanName;
  };
#endif // INTEL_FEATURE_SW_DTRANS
#endif // INTEL_CUSTOMIZATION

  // Two types with differing kinds are clearly not isomorphic.
  if (DstTy->getTypeID() != SrcTy->getTypeID())
    return false;

  // If we have an entry in the MappedTypes table, then we have our answer.
  Type *&Entry = MappedTypes[SrcTy];
  if (Entry)
    return Entry == DstTy;

  // Two identical types are clearly isomorphic.  Remember this
  // non-speculatively.
  if (DstTy == SrcTy) {
    Entry = DstTy;
    return true;
  }

  // Okay, we have two types with identical kinds that we haven't seen before.

  // If this is an opaque struct type, special case it.
  if (StructType *SSTy = dyn_cast<StructType>(SrcTy)) {
    // Mapping an opaque type to any struct, just keep the dest struct.
    if (SSTy->isOpaque()) {
      Entry = DstTy;
      SpeculativeTypes.push_back(SrcTy);
      return true;
    }

    // Mapping a non-opaque source type to an opaque dest.  If this is the first
    // type that we're mapping onto this destination type then we succeed.  Keep
    // the dest, but fill it in later. If this is the second (different) type
    // that we're trying to map onto the same opaque type then we fail.
    if (cast<StructType>(DstTy)->isOpaque()) {
      // We can only map one source type onto the opaque destination type.
      if (!DstResolvedOpaqueTypes.insert(cast<StructType>(DstTy)).second)
        return false;
      SrcDefinitionsToResolve.push_back(SSTy);
      SpeculativeTypes.push_back(SrcTy);
      SpeculativeDstOpaqueTypes.push_back(cast<StructType>(DstTy));
      Entry = DstTy;
      return true;
    }
  }

  // If the number of subtypes disagree between the two types, then we fail.
  if (SrcTy->getNumContainedTypes() != DstTy->getNumContainedTypes())
    return false;

  // Fail if any of the extra properties (e.g. array size) of the type disagree.
  if (isa<IntegerType>(DstTy))
    return false; // bitwidth disagrees.
  if (PointerType *PT = dyn_cast<PointerType>(DstTy)) {
    if (PT->getAddressSpace() != cast<PointerType>(SrcTy)->getAddressSpace())
      return false;
  } else if (FunctionType *FT = dyn_cast<FunctionType>(DstTy)) {
    if (FT->isVarArg() != cast<FunctionType>(SrcTy)->isVarArg())
      return false;
  } else if (StructType *DSTy = dyn_cast<StructType>(DstTy)) {
    StructType *SSTy = cast<StructType>(SrcTy);
    if (DSTy->isLiteral() != SSTy->isLiteral() ||
        DSTy->isPacked() != SSTy->isPacked())
      return false;
  } else if (auto *DArrTy = dyn_cast<ArrayType>(DstTy)) {
    if (DArrTy->getNumElements() != cast<ArrayType>(SrcTy)->getNumElements())
      return false;
  } else if (auto *DVecTy = dyn_cast<VectorType>(DstTy)) {
    if (DVecTy->getElementCount() != cast<VectorType>(SrcTy)->getElementCount())
      return false;
  }

  // Otherwise, we speculate that these two types will line up and recursively
  // check the subelements.
  Entry = DstTy;
  SpeculativeTypes.push_back(SrcTy);

#if INTEL_CUSTOMIZATION
#if INTEL_FEATURE_SW_DTRANS
  // Precompute if the source and destination types are structures with same
  // properties.
  StructType *SrcStr = dyn_cast<StructType>(SrcTy);
  StructType *DstStr = dyn_cast<StructType>(DstTy);
  bool StructsMatches = StructsAreTheSame(SrcStr, DstStr);
#endif //INTEL_FEATURE_SW_DTRANS

  for (unsigned I = 0, E = SrcTy->getNumContainedTypes(); I != E; ++I) {
#if INTEL_FEATURE_SW_DTRANS
    if (StructsMatches) {
/************************* Begin special functions ***************************/
      // NOTE: Handle the special case when mapping a pointer to an empty
      // structure with a pointer to a function. This could be removed
      // once the CFE generates opaque pointers. Read the comments between
      // the tags "Begin special functions" and "End special functions" for
      // more details.
      if (handleEmptyStrSpecialCase(SrcStr, DstStr, I))
        continue;
/************************** End special functions ****************************/

      DTransPointerType *SrcPtrTy = nullptr;
      DTransPointerType *DstPtrTy = nullptr;
      // Use the DTrans information to identify if field I in SrcTy is a
      // pointer type and it matches with field I in DstTy.
      if (GetDTransPointerFields(SrcStr, DstStr, I, &SrcPtrTy, &DstPtrTy)) {
        assert(SrcPtrTy &&
            "Trying to check isomorphisim from a null source pointer");
        assert(DstPtrTy &&
            "Trying to check isomorphisim from a null destination pointer");

        // Even if ArePointerFieldsIsomorphic calls areTypesIsomosphic, we
        // can't continue the loop here if it returns true. The reason is
        // because ArePointerFieldsIsomorphic won't map pointers and the map
        // needs to be built even with pointer types. For example, assume
        // that we are mapping the following structures:
        //
        // %struct.outer_struct.0 = type { *%struct.inner_struct.0 }  ->
        //   %struct.outer_struct = type { *%struct.inner_struct }
        //
        // All the enclosed types in the structure need to be mapped too:
        //
        //   *%struct.inner_struct.0  -> *%struct.inner_struct
        //    %struct.inner_struct.0  ->  %struct.inner_struct
        //
        // This is the case of non-opaque pointers. In the case of opaque
        // pointers, the function ArePointerFieldsIsomorphic will map the
        // following types:
        //
        //   %struct.outer_struct.0 = type { ptr }  ->
        //     %struct.outer_struct = type { ptr }
        //
        //   %struct.inner_struct.0  ->  %struct.inner_struct
        //
        // Calling the recursion areTypesIsomorphic will map 'ptr' from
        // source with 'ptr' in destination and exit.
        //
        // Also, if ArePointerFieldsIsomorphic finds that two pointers aren't
        // isomorphic, then we can return since we know that both structures
        // aren't isomorphic.
        if (!ArePointerFieldsIsomorphic(SrcPtrTy, DstPtrTy))
          return false;
      }
    }
#endif // INTEL_FEATURE_SW_DTRANS
    if (!areTypesIsomorphic(DstTy->getContainedType(I),
                            SrcTy->getContainedType(I)))
      return false;
  }
#endif // INTEL_CUSTOMIZATION

  // If everything seems to have lined up, then everything is great.
  return true;
}

void TypeMapTy::linkDefinedTypeBodies() {
  SmallVector<Type *, 16> Elements;
  for (StructType *SrcSTy : SrcDefinitionsToResolve) {
    StructType *DstSTy = cast<StructType>(MappedTypes[SrcSTy]);
    assert(DstSTy->isOpaque());

    // Map the body of the source type over to a new body for the dest type.
    Elements.resize(SrcSTy->getNumElements());
    for (unsigned I = 0, E = Elements.size(); I != E; ++I)
      Elements[I] = get(SrcSTy->getElementType(I));

    DstSTy->setBody(Elements, SrcSTy->isPacked());
    DstStructTypesSet.switchToNonOpaque(DstSTy);
  }
  SrcDefinitionsToResolve.clear();
  DstResolvedOpaqueTypes.clear();
}

void TypeMapTy::finishType(StructType *DTy, StructType *STy,
                           ArrayRef<Type *> ETypes) {
  DTy->setBody(ETypes, STy->isPacked());

  // Steal STy's name.
  if (STy->hasName()) {
    SmallString<16> TmpName = STy->getName();
    STy->setName("");
    DTy->setName(TmpName);
  }

  DstStructTypesSet.addNonOpaque(DTy);
}

#if INTEL_CUSTOMIZATION
#if INTEL_FEATURE_SW_DTRANS

// Use the DTransType (DTSrcTy) and the LLVM type (SrcTy) from the source
// module and generate a DTransType with the information in the destination
// module. This function basically updates the DTrans type manager in the
// destination module (DstTM) with the new types added.
DTransType* TypeMapTy::copyDTransType(Type *DstTy, Type *SrcTy,
    DTransType *DTSrcTy, SetVector<Type *> &VisitedTypes) {

  if (!EnableDTransTypesMappingScheme)
    return nullptr;

  if (!SrcTy && !DstTy)
    return nullptr;

  // If the DTransType is missing then it means that we may have some metadata
  // loss incoming from the source module.
  if (!DTSrcTy)
    return nullptr;

  if (SrcTy && !DstTy)
    llvm_unreachable("Trying to copy a DTransType with null "
                     "destination structure");
  else if (!SrcTy && DstTy)
    llvm_unreachable("Trying to copy a DTransType with null "
                     "source structure");

  assert((DstTy == get(SrcTy)) && "Input destination type is "
      "different from the destination type in the table");

  DTransType *NewResultType = nullptr;

  // Handle Pointer type. The basic idea is that we are going to collect
  // the element type from the source DTransType, then find the corresponding
  // type in the destination module and create a pointer type.
  if (DTransPointerType *DTSrcPtr = dyn_cast<DTransPointerType>(DTSrcTy)) {
    DTransType *DTSrcPtrElem = DTSrcPtr->getPointerElementType();

    // Find the corresponding element type in the destination module. It
    // won't matter if the pointer is opaque or not, we have the dereference
    // levels in DTSrcPtrElem.
    Type *DstPrtElem = get(DTSrcPtrElem->getLLVMType());

    DTransType *DTDstPtrElem =
        copyDTransType(DstPrtElem, DTSrcPtrElem->getLLVMType(),
                       DTSrcPtrElem, VisitedTypes);

    NewResultType = DstTM->getOrCreatePointerType(DTDstPtrElem);
  }

  // Handle sequential (array and vector) types. In this case we are going to
  // collect the array's element type, find the corresponding type in the
  // destination module and create the new type.
  else if (auto *DTSrcSec = dyn_cast<DTransSequentialType>(DTSrcTy)) {
    auto *DTSrcElement = DTSrcSec->getElementType();
    Type *SrcElement = DTSrcElement->getLLVMType();
    Type *DstElement = get(SrcElement);
    unsigned NumElements = DTSrcSec->getNumElements();

    DTransType *DTElement =
        copyDTransType(DstElement, SrcElement, DTSrcElement, VisitedTypes);

    if (isa<DTransArrayType>(DTSrcSec))
      NewResultType =
          DstTM->getOrCreateArrayType(DTElement, NumElements);
    else if (isa<DTransVectorType>(DTSrcSec))
      NewResultType =
          DstTM->getOrCreateVectorType(DTElement, NumElements);
    else
      llvm_unreachable("Creating destination type from an unsupported Dtrans "
                       "sequential type");
  }

  // Handle structure types. The basic concept is to create the DTransTypes for
  // each field, and then create the DTransType for the structure.
  else if (DTransStructType *DTSrcST = dyn_cast<DTransStructType>(DTSrcTy)) {
    auto *DstST = cast<StructType>(DstTy);
    auto *SrcST = cast<StructType>(SrcTy);

    // If the structure is literal, then create the fields first and then
    // create the structure
    if (DstST->isLiteral()) {
      SmallVector<DTransType *, 4> FieldTypes;
      for (unsigned I = 0, E = SrcST->getNumElements(); I < E; I++) {
        Type *DstField = get(SrcST->getElementType(I));
        DTransType *DTSrcFieldTy = DTSrcST->getFieldType(I);
        DTransType *DTDstFieldTy =
            copyDTransType(DstField, SrcST->getElementType(I),
                           DTSrcFieldTy, VisitedTypes);
        FieldTypes.push_back(DTDstFieldTy);
      }
      DTransStructType *DTDstST =
          DstTM->getOrCreateLiteralStructType(DstTy->getContext(), FieldTypes);

      // If the source type has a reconstruction error, then we need to copy
      // that information.
      if (DTSrcST->getReconstructError())
        DTDstST->setReconstructError();

      NewResultType = DTDstST;
    }

    // If the structure exists, then check if the type from the source module
    // will define the body in the destination module
    else if (auto *DTDstSTTy = DstTM->getStructType(DstST->getName())) {
      if (DTDstSTTy->isOpaque() && !DTSrcST->isOpaque() &&
          VisitedTypes.insert(DstTy)) {
        std::vector<DTransType *> Fields;
        for (unsigned I = 0, E = SrcST->getNumElements(); I < E; I++) {
          Type *DstField = get(SrcST->getElementType(I));
          DTransType *DTSrcFieldTy = DTSrcST->getFieldType(I);
          DTransType *DTDstFieldTy =
              copyDTransType(DstField, SrcST->getElementType(I),
                             DTSrcFieldTy, VisitedTypes);
          Fields.push_back(DTDstFieldTy);
        }
        ArrayRef<DTransType *> NewRefFields(Fields);
        DTDstSTTy->setBody(NewRefFields);
      }
      NewResultType = DTDstSTTy;
    } else {
      // getOrCreateStructType generates a DTransStructType without the fields
      // set, just allocate the memory space.
      DTransStructType *DTDstST = DstTM->getOrCreateStructType(DstST);
      // If the source type has a reconstruction error, then we need to copy
      // that information.
      if (DTSrcST->getReconstructError())
        DTDstST->setReconstructError();

      for (unsigned I = 0, E = SrcST->getNumElements(); I < E; I++) {
        Type *DstField = get(SrcST->getElementType(I));
        DTransType *DTSrcFieldTy = DTSrcST->getFieldType(I);
        DTransType *DTDstFieldTy =
            copyDTransType(DstField, SrcST->getElementType(I), DTSrcFieldTy,
                           VisitedTypes);
        if (DTDstFieldTy)
          DTDstST->getField(I).addResolvedType(DTDstFieldTy);
      }

      NewResultType = DTDstST;
    }
  }

  // Handle function type. In this case we are going to generate the DTransType
  // for the return type and each parameter, and then we generate the
  // DTransType for the function. Also, we use the LLVM type information from
  // the source DTransFunctionType in order to create the destination type. The
  // reason is that the source LLVM type can be an empty structure and the
  // destination is a function type (CFE place holder).
  else if (auto *DTSrcFn = dyn_cast<DTransFunctionType>(DTSrcTy)) {
    // Create the DTransType for the return type
    FunctionType *DstFn = cast<FunctionType>(DstTy);
    DTransType *DTSrcRet = DTSrcFn->getReturnType();
    Type *SrcRetType = DTSrcRet->getLLVMType();
    Type *DstRetType = get(SrcRetType);
    DTransType *DTDstRet =
        copyDTransType(DstRetType, SrcRetType, DTSrcRet, VisitedTypes);

    // Handle each parameter
    SmallVector<DTransType *, 8> ParamTypes;
    for (auto *DTSrcParam : DTSrcFn->args()) {
      Type *SrcParam = DTSrcParam->getLLVMType();
      Type *DstParam = get(SrcParam);
      DTransType *DTDstParam = copyDTransType(DstParam, SrcParam,
                                              DTSrcParam, VisitedTypes);

      ParamTypes.push_back(DTDstParam);
    }

    // Create function type
    NewResultType = DstTM->getOrCreateFunctionType(DTDstRet, ParamTypes,
                                                   DstFn->isVarArg());
  }

  // TODO: DTransTypes doesn't support ScalableVectorType yet
  else if (isa<ScalableVectorType>(SrcTy)) {
    return nullptr;
  } else {
    // Else handle the first class types
    assert((SrcTy->isIntegerTy() || SrcTy->isFloatingPointTy() ||
            SrcTy->isVoidTy() || SrcTy->isMetadataTy() ||
            SrcTy->isTokenTy()) && "Source first class type must be based on "
            "scalar type");

    NewResultType = DstTM->getOrCreateAtomicType(DstTy);
  }

  assert(NewResultType && "New DTransType in destination module not created");
  return NewResultType;
}

// Traverse through each DTrans type created with the metadata from the source
// module and update the DTrans type manager of the destination module with
// the new LLVM types added.
void TypeMapTy::updateDTransTypeManager() {
  if (!EnableDTransTypesMappingScheme)
    return;

  SetVector<Type *> VisitedTypes;

  // The reason we use the map constructed for the source module is because
  // the structures' names can change when they are moved to the destination
  // module, and the DTransTypeMapper depends on the structure name to
  // find the corresponding DTransStructureType. The map constructed in
  // DTransSrcStructsMap uses the pointer to the structure type rather
  // than the structure name.
  for (auto StructPair : DTransSrcStructsMap->getDtransStructMap()) {
    if (!StructPair.second)
      continue;

/***************** Begin special functions ***************************/
    // We can safely skip the types used for mapping a pointer to an empty
    // structure with a pointer to a function type. One of the goals
    // of the IR mover is to fix this mapping and generate the correct
    // type. This should not be a problem with opaque pointers.
    if (isSpecialEmptyStruct(StructPair.first))
      continue;

    // There is a chance that IR mover can modify the name of the LLVM
    // structure type and we missed catching the special structure name.
    // Try checking the DTrans type.
    if (isSpecialEmptyDTransStruct(StructPair.second))
      continue;
/***************** End special functions ***************************/

    DTransType *DTSrcTy = cast<DTransType>(StructPair.second);
    Type *ST = StructPair.first;
    Type *DstTy = get(ST);

    // Create the DTrans structure type for the destination type
    copyDTransType(DstTy, ST, DTSrcTy, VisitedTypes);
  }
}
#endif // INTEL_FEATURE_SW_DTRANS
#endif //INTEL_CUSTOMIZATION

Type *TypeMapTy::get(Type *Ty) {
  SmallPtrSet<StructType *, 8> Visited;
  return get(Ty, Visited);
}

Type *TypeMapTy::get(Type *Ty, SmallPtrSet<StructType *, 8> &Visited) {
  // If we already have an entry for this type, return it.
  Type **Entry = &MappedTypes[Ty];
  if (*Entry)
    return *Entry;

#if INTEL_CUSTOMIZATION
#if INTEL_FEATURE_SW_DTRANS
  // If the type is a pointer to an empty structure, then we need to fix it
  // by replacing it with a pointer to a function. Read the comments between
  // the tags "Begin special functions" and "End special functions" for more
  // details.
  if (auto *RepairedType = tryToRepairType(Ty, Visited))
    return RepairedType;
#endif // INTEL_FEATURE_SW_DTRANS
#endif // INTEL_CUSTOMIZATION

  // These are types that LLVM itself will unique.
  bool IsUniqued = !isa<StructType>(Ty) || cast<StructType>(Ty)->isLiteral();

  if (!IsUniqued) {
#ifndef NDEBUG
    for (auto &Pair : MappedTypes) {
      assert(!(Pair.first != Ty && Pair.second == Ty) &&
             "mapping to a source type");
    }
#endif

    if (!Visited.insert(cast<StructType>(Ty)).second) {
      StructType *DTy = StructType::create(Ty->getContext());
      return *Entry = DTy;
    }
  }

  // If this is not a recursive type, then just map all of the elements and
  // then rebuild the type from inside out.
  SmallVector<Type *, 4> ElementTypes;

  // If there are no element types to map, then the type is itself.  This is
  // true for the anonymous {} struct, things like 'float', integers, etc.
  if (Ty->getNumContainedTypes() == 0 && IsUniqued)
    return *Entry = Ty;

  // Remap all of the elements, keeping track of whether any of them change.
  bool AnyChange = false;
  ElementTypes.resize(Ty->getNumContainedTypes());
  for (unsigned I = 0, E = Ty->getNumContainedTypes(); I != E; ++I) {
    ElementTypes[I] = get(Ty->getContainedType(I), Visited);
    AnyChange |= ElementTypes[I] != Ty->getContainedType(I);
  }

  // If we found our type while recursively processing stuff, just use it.
  Entry = &MappedTypes[Ty];
  if (*Entry) {
    if (auto *DTy = dyn_cast<StructType>(*Entry)) {
      if (DTy->isOpaque()) {
        auto *STy = cast<StructType>(Ty);
        finishType(DTy, STy, ElementTypes);
      }
    }
    return *Entry;
  }

  // If all of the element types mapped directly over and the type is not
  // a named struct, then the type is usable as-is.
  if (!AnyChange && IsUniqued)
    return *Entry = Ty;

  // Otherwise, rebuild a modified type.
  switch (Ty->getTypeID()) {
  default:
    llvm_unreachable("unknown derived type to remap");
  case Type::ArrayTyID:
    return *Entry = ArrayType::get(ElementTypes[0],
                                   cast<ArrayType>(Ty)->getNumElements());
  case Type::ScalableVectorTyID:
  case Type::FixedVectorTyID:
    return *Entry = VectorType::get(ElementTypes[0],
                                    cast<VectorType>(Ty)->getElementCount());
  case Type::PointerTyID:
    return *Entry = PointerType::get(ElementTypes[0],
                                     cast<PointerType>(Ty)->getAddressSpace());
  case Type::FunctionTyID:
    return *Entry = FunctionType::get(ElementTypes[0],
                                      makeArrayRef(ElementTypes).slice(1),
                                      cast<FunctionType>(Ty)->isVarArg());
  case Type::StructTyID: {
    auto *STy = cast<StructType>(Ty);
    bool IsPacked = STy->isPacked();
    if (IsUniqued)
      return *Entry = StructType::get(Ty->getContext(), ElementTypes, IsPacked);

    // If the type is opaque, we can just use it directly.
    if (STy->isOpaque()) {
      DstStructTypesSet.addOpaque(STy);
      return *Entry = Ty;
    }

#if INTEL_CUSTOMIZATION
#if INTEL_FEATURE_SW_DTRANS
    // If merging by mangled names is not enabled or the type is not
    // in the list of types that were verified then try checking
    // if it can collect the old type.
    if (!EnableDTransTypesMappingScheme || VisitedTypes.count(STy) == 0) {
#endif // INTEL_FEATURE_SW_DTRANS
    // Provide name of a struct.
    if (StructType *OldT = DstStructTypesSet.findNonOpaque(
            ElementTypes, IsPacked, getStructName(STy))) {
      STy->setName("");
      return *Entry = OldT;
    }
#if INTEL_FEATURE_SW_DTRANS
    }
#endif // INTEL_FEATURE_SW_DTRANS
#endif // INTEL_CUSTOMIZATION

    if (!AnyChange) {
      DstStructTypesSet.addNonOpaque(STy);
      return *Entry = Ty;
    }

    StructType *DTy = StructType::create(Ty->getContext());
    finishType(DTy, STy, ElementTypes);
    return *Entry = DTy;
  }
  }
}

LinkDiagnosticInfo::LinkDiagnosticInfo(DiagnosticSeverity Severity,
                                       const Twine &Msg)
    : DiagnosticInfo(DK_Linker, Severity), Msg(Msg) {}
void LinkDiagnosticInfo::print(DiagnosticPrinter &DP) const { DP << Msg; }

//===----------------------------------------------------------------------===//
// IRLinker implementation.
//===----------------------------------------------------------------------===//

namespace {
class IRLinker;

/// Creates prototypes for functions that are lazily linked on the fly. This
/// speeds up linking for modules with many/ lazily linked functions of which
/// few get used.
class GlobalValueMaterializer final : public ValueMaterializer {
  IRLinker &TheIRLinker;

public:
  GlobalValueMaterializer(IRLinker &TheIRLinker) : TheIRLinker(TheIRLinker) {}
  Value *materialize(Value *V) override;
};

class LocalValueMaterializer final : public ValueMaterializer {
  IRLinker &TheIRLinker;

public:
  LocalValueMaterializer(IRLinker &TheIRLinker) : TheIRLinker(TheIRLinker) {}
  Value *materialize(Value *V) override;
};

/// Type of the Metadata map in \a ValueToValueMapTy.
typedef DenseMap<const Metadata *, TrackingMDRef> MDMapT;

/// This is responsible for keeping track of the state used for moving data
/// from SrcM to DstM.
class IRLinker {
  Module &DstM;
  std::unique_ptr<Module> SrcM;

  /// See IRMover::move().
  IRMover::LazyCallback AddLazyFor;

  TypeMapTy TypeMap;
  GlobalValueMaterializer GValMaterializer;
  LocalValueMaterializer LValMaterializer;

  /// A metadata map that's shared between IRLinker instances.
  MDMapT &SharedMDs;

  /// Mapping of values from what they used to be in Src, to what they are now
  /// in DstM.  ValueToValueMapTy is a ValueMap, which involves some overhead
  /// due to the use of Value handles which the Linker doesn't actually need,
  /// but this allows us to reuse the ValueMapper code.
  ValueToValueMapTy ValueMap;
  ValueToValueMapTy IndirectSymbolValueMap;

  DenseSet<GlobalValue *> ValuesToLink;
  std::vector<GlobalValue *> Worklist;
  std::vector<std::pair<GlobalValue *, Value*>> RAUWWorklist;

#if INTEL_CUSTOMIZATION
#if INTEL_FEATURE_SW_DTRANS
  // When DTrans metadata tags are used to describe types of function
  // return values or parameters, we need to ensure that the metadata nodes
  // attached to declarations get remapped to the types contained within the
  // destination module. Otherwise, the metadata can be left pointing to the
  // memory object of the source module, resulting in a bad pointer reference
  // within the destination module. This worklist is for declarations that
  // are discovered while mapping the source module into the destination module
  // which will need to be processed.
  SmallPtrSet<GlobalObject *, 8> DTransMDRemapWorklist;

  DTransTypeManager *DstTM = nullptr;
#endif // INTEL_FEATURE_SW_DTRANS
#endif // INTEL_CUSTOMIZATION

  void maybeAdd(GlobalValue *GV) {
    if (ValuesToLink.insert(GV).second)
      Worklist.push_back(GV);
  }

  /// Whether we are importing globals for ThinLTO, as opposed to linking the
  /// source module. If this flag is set, it means that we can rely on some
  /// other object file to define any non-GlobalValue entities defined by the
  /// source module. This currently causes us to not link retained types in
  /// debug info metadata and module inline asm.
  bool IsPerformingImport;

  /// Set to true when all global value body linking is complete (including
  /// lazy linking). Used to prevent metadata linking from creating new
  /// references.
  bool DoneLinkingBodies = false;

  /// The Error encountered during materialization. We use an Optional here to
  /// avoid needing to manage an unconsumed success value.
  Optional<Error> FoundError;
  void setError(Error E) {
    if (E)
      FoundError = std::move(E);
  }

  /// Most of the errors produced by this module are inconvertible StringErrors.
  /// This convenience function lets us return one of those more easily.
  Error stringErr(const Twine &T) {
    return make_error<StringError>(T, inconvertibleErrorCode());
  }

  /// Entry point for mapping values and alternate context for mapping aliases.
  ValueMapper Mapper;
  unsigned IndirectSymbolMCID;

  /// Handles cloning of a global values from the source module into
  /// the destination module, including setting the attributes and visibility.
  GlobalValue *copyGlobalValueProto(const GlobalValue *SGV, bool ForDefinition);

  void emitWarning(const Twine &Message) {
    SrcM->getContext().diagnose(LinkDiagnosticInfo(DS_Warning, Message));
  }

  /// Given a global in the source module, return the global in the
  /// destination module that is being linked to, if any.
  GlobalValue *getLinkedToGlobal(const GlobalValue *SrcGV) {
    // If the source has no name it can't link.  If it has local linkage,
    // there is no name match-up going on.
    if (!SrcGV->hasName() || SrcGV->hasLocalLinkage())
      return nullptr;

    // Otherwise see if we have a match in the destination module's symtab.
    GlobalValue *DGV = DstM.getNamedValue(SrcGV->getName());
    if (!DGV)
      return nullptr;

    // If we found a global with the same name in the dest module, but it has
    // internal linkage, we are really not doing any linkage here.
    if (DGV->hasLocalLinkage())
      return nullptr;

    // If we found an intrinsic declaration with mismatching prototypes, we
    // probably had a nameclash. Don't use that version.
    if (auto *FDGV = dyn_cast<Function>(DGV))
      if (FDGV->isIntrinsic())
        if (const auto *FSrcGV = dyn_cast<Function>(SrcGV))
          if (FDGV->getFunctionType() != TypeMap.get(FSrcGV->getFunctionType()))
            return nullptr;

    // Otherwise, we do in fact link to the destination global.
    return DGV;
  }

  void computeTypeMapping();
#if INTEL_CUSTOMIZATION
#if INTEL_FEATURE_SW_DTRANS
  // Verify that the types in the destination module are unique
  void verifyDestinationModule();

  // Simple verifier that checks if all the types in the destination module
  // are unique
  void quickVerifyDestinationModule();
#endif // INTEL_FEATURE_SW_DTRANS
#endif // INTEL_CUSTOMIZATION

  Expected<Constant *> linkAppendingVarProto(GlobalVariable *DstGV,
                                             const GlobalVariable *SrcGV);

  /// Given the GlobaValue \p SGV in the source module, and the matching
  /// GlobalValue \p DGV (if any), return true if the linker will pull \p SGV
  /// into the destination module.
  ///
  /// Note this code may call the client-provided \p AddLazyFor.
  bool shouldLink(GlobalValue *DGV, GlobalValue &SGV);
  Expected<Constant *> linkGlobalValueProto(GlobalValue *GV,
                                            bool ForIndirectSymbol);

  Error linkModuleFlagsMetadata();

  void linkGlobalVariable(GlobalVariable &Dst, GlobalVariable &Src);
  Error linkFunctionBody(Function &Dst, Function &Src);
  void linkAliasAliasee(GlobalAlias &Dst, GlobalAlias &Src);
  void linkIFuncResolver(GlobalIFunc &Dst, GlobalIFunc &Src);
  Error linkGlobalValueBody(GlobalValue &Dst, GlobalValue &Src);

  /// Replace all types in the source AttributeList with the
  /// corresponding destination type.
  AttributeList mapAttributeTypes(LLVMContext &C, AttributeList Attrs);

  /// Functions that take care of cloning a specific global value type
  /// into the destination module.
  GlobalVariable *copyGlobalVariableProto(const GlobalVariable *SGVar);
  Function *copyFunctionProto(const Function *SF);
  GlobalValue *copyIndirectSymbolProto(const GlobalValue *SGV);

  /// Perform "replace all uses with" operations. These work items need to be
  /// performed as part of materialization, but we postpone them to happen after
  /// materialization is done. The materializer called by ValueMapper is not
  /// expected to delete constants, as ValueMapper is holding pointers to some
  /// of them, but constant destruction may be indirectly triggered by RAUW.
  /// Hence, the need to move this out of the materialization call chain.
  void flushRAUWWorklist();

  /// When importing for ThinLTO, prevent importing of types listed on
  /// the DICompileUnit that we don't need a copy of in the importing
  /// module.
  void prepareCompileUnitsForImport();
  void linkNamedMDNodes();

public:
#if INTEL_CUSTOMIZATION
#if INTEL_FEATURE_SW_DTRANS
  IRLinker(Module &DstM, MDMapT &SharedMDs,
           IRMover::IdentifiedStructTypeSet &Set, std::unique_ptr<Module> SrcM,
           ArrayRef<GlobalValue *> ValuesToLink,
           IRMover::LazyCallback AddLazyFor, bool IsPerformingImport,
           DTransTypeManager *DstTM)
      : DstM(DstM), SrcM(std::move(SrcM)), AddLazyFor(std::move(AddLazyFor)),
        TypeMap(Set, DstTM), GValMaterializer(*this), LValMaterializer(*this),
        SharedMDs(SharedMDs), DstTM(DstTM), IsPerformingImport(IsPerformingImport),
        Mapper(ValueMap, RF_ReuseAndMutateDistinctMDs | RF_IgnoreMissingLocals,
               &TypeMap, &GValMaterializer),
        IndirectSymbolMCID(Mapper.registerAlternateMappingContext(
            IndirectSymbolValueMap, &LValMaterializer)) {
    ValueMap.getMDMap() = std::move(SharedMDs);
    for (GlobalValue *GV : ValuesToLink)
      maybeAdd(GV);
    if (IsPerformingImport)
      prepareCompileUnitsForImport();
  }
#else // INTEL_FEATURE_SW_DTRANS
  IRLinker(Module &DstM, MDMapT &SharedMDs,
           IRMover::IdentifiedStructTypeSet &Set, std::unique_ptr<Module> SrcM,
           ArrayRef<GlobalValue *> ValuesToLink,
           IRMover::LazyCallback AddLazyFor, bool IsPerformingImport)
      : DstM(DstM), SrcM(std::move(SrcM)), AddLazyFor(std::move(AddLazyFor)),
        TypeMap(Set), GValMaterializer(*this), LValMaterializer(*this),
        SharedMDs(SharedMDs), IsPerformingImport(IsPerformingImport),
        Mapper(ValueMap, RF_ReuseAndMutateDistinctMDs | RF_IgnoreMissingLocals,
               &TypeMap, &GValMaterializer),
        IndirectSymbolMCID(Mapper.registerAlternateMappingContext(
            IndirectSymbolValueMap, &LValMaterializer)) {
    ValueMap.getMDMap() = std::move(SharedMDs);
    for (GlobalValue *GV : ValuesToLink)
      maybeAdd(GV);
    if (IsPerformingImport)
      prepareCompileUnitsForImport();
  }
#endif // INTEL_FEATURE_SW_DTRANS
#endif // INTEL_CUSTOMIZATION
  ~IRLinker() { SharedMDs = std::move(*ValueMap.getMDMap()); }

  Error run();
  Value *materialize(Value *V, bool ForIndirectSymbol);
};
}

/// The LLVM SymbolTable class autorenames globals that conflict in the symbol
/// table. This is good for all clients except for us. Go through the trouble
/// to force this back.
static void forceRenaming(GlobalValue *GV, StringRef Name) {
  // If the global doesn't force its name or if it already has the right name,
  // there is nothing for us to do.
  if (GV->hasLocalLinkage() || GV->getName() == Name)
    return;

  Module *M = GV->getParent();

  // If there is a conflict, rename the conflict.
  if (GlobalValue *ConflictGV = M->getNamedValue(Name)) {
    GV->takeName(ConflictGV);
    ConflictGV->setName(Name); // This will cause ConflictGV to get renamed
    assert(ConflictGV->getName() != Name && "forceRenaming didn't work");
  } else {
    GV->setName(Name); // Force the name back
  }
}

Value *GlobalValueMaterializer::materialize(Value *SGV) {
  return TheIRLinker.materialize(SGV, false);
}

Value *LocalValueMaterializer::materialize(Value *SGV) {
  return TheIRLinker.materialize(SGV, true);
}

Value *IRLinker::materialize(Value *V, bool ForIndirectSymbol) {
  auto *SGV = dyn_cast<GlobalValue>(V);
  if (!SGV)
    return nullptr;

  // When linking a global from other modules than source & dest, skip
  // materializing it because it would be mapped later when its containing
  // module is linked. Linking it now would potentially pull in many types that
  // may not be mapped properly.
  if (SGV->getParent() != &DstM && SGV->getParent() != SrcM.get())
    return nullptr;

  Expected<Constant *> NewProto = linkGlobalValueProto(SGV, ForIndirectSymbol);
  if (!NewProto) {
    setError(NewProto.takeError());
    return nullptr;
  }
  if (!*NewProto)
    return nullptr;

  GlobalValue *New = dyn_cast<GlobalValue>(*NewProto);
  if (!New)
    return *NewProto;

  // If we already created the body, just return.
  if (auto *F = dyn_cast<Function>(New)) {
    if (!F->isDeclaration())
      return New;
  } else if (auto *V = dyn_cast<GlobalVariable>(New)) {
    if (V->hasInitializer() || V->hasAppendingLinkage())
      return New;
  } else if (auto *GA = dyn_cast<GlobalAlias>(New)) {
    if (GA->getAliasee())
      return New;
  } else if (auto *GI = dyn_cast<GlobalIFunc>(New)) {
    if (GI->getResolver())
      return New;
  } else {
    llvm_unreachable("Invalid GlobalValue type");
  }

  // If the global is being linked for an indirect symbol, it may have already
  // been scheduled to satisfy a regular symbol. Similarly, a global being linked
  // for a regular symbol may have already been scheduled for an indirect
  // symbol. Check for these cases by looking in the other value map and
  // confirming the same value has been scheduled.  If there is an entry in the
  // ValueMap but the value is different, it means that the value already had a
  // definition in the destination module (linkonce for instance), but we need a
  // new definition for the indirect symbol ("New" will be different).
  if ((ForIndirectSymbol && ValueMap.lookup(SGV) == New) ||
      (!ForIndirectSymbol && IndirectSymbolValueMap.lookup(SGV) == New))
    return New;

  if (ForIndirectSymbol || shouldLink(New, *SGV))
    setError(linkGlobalValueBody(*New, *SGV));

  return New;
}

/// Loop through the global variables in the src module and merge them into the
/// dest module.
GlobalVariable *IRLinker::copyGlobalVariableProto(const GlobalVariable *SGVar) {
  // No linking to be performed or linking from the source: simply create an
  // identical version of the symbol over in the dest module... the
  // initializer will be filled in later by LinkGlobalInits.
  GlobalVariable *NewDGV =
      new GlobalVariable(DstM, TypeMap.get(SGVar->getValueType()),
                         SGVar->isConstant(), GlobalValue::ExternalLinkage,
                         /*init*/ nullptr, SGVar->getName(),
                         /*insertbefore*/ nullptr, SGVar->getThreadLocalMode(),
                         SGVar->getAddressSpace());
  NewDGV->setAlignment(SGVar->getAlign());
  NewDGV->copyAttributesFrom(SGVar);
  return NewDGV;
}

AttributeList IRLinker::mapAttributeTypes(LLVMContext &C, AttributeList Attrs) {
  for (unsigned i = 0; i < Attrs.getNumAttrSets(); ++i) {
    for (int AttrIdx = Attribute::FirstTypeAttr;
         AttrIdx <= Attribute::LastTypeAttr; AttrIdx++) {
      Attribute::AttrKind TypedAttr = (Attribute::AttrKind)AttrIdx;
      if (Attrs.hasAttributeAtIndex(i, TypedAttr)) {
        if (Type *Ty = Attrs.getAttributeAtIndex(i, TypedAttr).getValueAsType()) {
          Attrs = Attrs.replaceAttributeTypeAtIndex(C, i, TypedAttr, TypeMap.get(Ty));
          break;
        }
      }
    }
  }
  return Attrs;
}

/// Link the function in the source module into the destination module if
/// needed, setting up mapping information.
Function *IRLinker::copyFunctionProto(const Function *SF) {
  // If there is no linkage to be performed or we are linking from the source,
  // bring SF over.
  auto *F = Function::Create(TypeMap.get(SF->getFunctionType()),
                             GlobalValue::ExternalLinkage,
                             SF->getAddressSpace(), SF->getName(), &DstM);
  F->copyAttributesFrom(SF);
  F->setAttributes(mapAttributeTypes(F->getContext(), F->getAttributes()));
  return F;
}

/// Set up prototypes for any indirect symbols that come over from the source
/// module.
GlobalValue *IRLinker::copyIndirectSymbolProto(const GlobalValue *SGV) {
  // If there is no linkage to be performed or we're linking from the source,
  // bring over SGA.
  auto *Ty = TypeMap.get(SGV->getValueType());

  if (auto *GA = dyn_cast<GlobalAlias>(SGV)) {
    auto *DGA = GlobalAlias::create(Ty, SGV->getAddressSpace(),
                                    GlobalValue::ExternalLinkage,
                                    SGV->getName(), &DstM);
    DGA->copyAttributesFrom(GA);
    return DGA;
  }

  if (auto *GI = dyn_cast<GlobalIFunc>(SGV)) {
    auto *DGI = GlobalIFunc::create(Ty, SGV->getAddressSpace(),
                                    GlobalValue::ExternalLinkage,
                                    SGV->getName(), nullptr, &DstM);
    DGI->copyAttributesFrom(GI);
    return DGI;
  }

  llvm_unreachable("Invalid source global value type");
}

GlobalValue *IRLinker::copyGlobalValueProto(const GlobalValue *SGV,
                                            bool ForDefinition) {
  GlobalValue *NewGV;
  if (auto *SGVar = dyn_cast<GlobalVariable>(SGV)) {
    NewGV = copyGlobalVariableProto(SGVar);
  } else if (auto *SF = dyn_cast<Function>(SGV)) {
    NewGV = copyFunctionProto(SF);
  } else {
    if (ForDefinition)
      NewGV = copyIndirectSymbolProto(SGV);
    else if (SGV->getValueType()->isFunctionTy())
      NewGV =
          Function::Create(cast<FunctionType>(TypeMap.get(SGV->getValueType())),
                           GlobalValue::ExternalLinkage, SGV->getAddressSpace(),
                           SGV->getName(), &DstM);
    else
      NewGV =
          new GlobalVariable(DstM, TypeMap.get(SGV->getValueType()),
                             /*isConstant*/ false, GlobalValue::ExternalLinkage,
                             /*init*/ nullptr, SGV->getName(),
                             /*insertbefore*/ nullptr,
                             SGV->getThreadLocalMode(), SGV->getAddressSpace());
  }

  if (ForDefinition)
    NewGV->setLinkage(SGV->getLinkage());
  else if (SGV->hasExternalWeakLinkage())
    NewGV->setLinkage(GlobalValue::ExternalWeakLinkage);

  if (auto *NewGO = dyn_cast<GlobalObject>(NewGV)) {
    // Metadata for global variables and function declarations is copied eagerly.
    if (isa<GlobalVariable>(SGV) || SGV->isDeclaration())
      NewGO->copyMetadata(cast<GlobalObject>(SGV), 0);
  }

  // Remove these copied constants in case this stays a declaration, since
  // they point to the source module. If the def is linked the values will
  // be mapped in during linkFunctionBody.
  if (auto *NewF = dyn_cast<Function>(NewGV)) {
    NewF->setPersonalityFn(nullptr);
    NewF->setPrefixData(nullptr);
    NewF->setPrologueData(nullptr);
  }

  return NewGV;
}

static StringRef getTypeNamePrefix(StringRef Name) {
  size_t DotPos = Name.rfind('.');
  return (DotPos == 0 || DotPos == StringRef::npos || Name.back() == '.' ||
          !isdigit(static_cast<unsigned char>(Name[DotPos + 1])))
             ? Name
             : Name.substr(0, DotPos);
}


#if INTEL_CUSTOMIZATION
namespace {
// Normalizes struct name for type merging.
static StringRef getStructName(const StructType *S) {
  if (TypeMerging || !S->hasName())
    return "";

  return getTypeNamePrefix(S->getName());
}

#if INTEL_FEATURE_SW_DTRANS

// Collect the real structure name. For example, assume that the structure
// name is the following:
//
//   ST->getName() := "%class.TestClass.0.123"
//
// Then return the name without the extra numbering:
//
//   "%class.TestClass"
//
// If the type doesn't have a name then return an empty string.
static StringRef getStructureNameClean(StructType *ST) {
  if (!ST || !ST->hasName())
    return "";

  StringRef CurrName = ST->getName();

  // If the last character is not a number then don't spend time splitting
  // the string
  if (!isdigit(static_cast<unsigned char>(CurrName[CurrName.size() - 1])))
    return CurrName;

  bool KeepLooking = false;
  do {
    KeepLooking = false;

    // Split the string by the last dot
    // ["%class.TestClass.0", "123"]
    auto Split = CurrName.rsplit('.');
    if (Split.second.empty())
      break;

    // Check if the second string ("123") is an integer.
    // NOTE: The function getAsInteger returns true if the string is NOT an
    // integer. It represents that an error was found when collecting the
    // number.
    unsigned N;
    if (Split.second.getAsInteger(10, N))
      break;

    (void)N;
    CurrName = Split.first;
    KeepLooking = true;
  } while (KeepLooking);

  return CurrName;
}

// Return true if the name doesn't contain extra numbering at the end
// (e.g. %class.TestClass.0), else return false.
static bool isStructureNameClean(StructType *ST) {
  if (!ST)
    return false;

  if (!ST->hasName())
    return true;

  StringRef CurrName = ST->getName();

  // If the last character is not a number then don't spend time splitting
  // the string
  if (!isdigit(static_cast<unsigned char>(CurrName[CurrName.size() - 1])))
    return true;

  // Split the string at the last dot
  // ["%class.TestClass", "0"]
  auto Split = CurrName.rsplit('.');
  if (Split.second.empty())
    return true;

  // Check if the second string ("0") is an integer.
  // NOTE: The function getAsInteger returns true if the string is NOT an
  // integer. It indicates that an error was found when collecting the
  // number.
  unsigned N;
  if (Split.second.getAsInteger(10, N))
    return true;

  (void) N;
  return false;
}

// Return a StringRef with the mangled name of the structure's typeinfo
static StringRef getMangledNameFromStructure(StructType *ST) {
  StringRef None;
  if (!ST || !ST->hasName())
    return None;

  StringRef StructName = ST->getName();
  auto NameSplit = StructName.split('.');

  // Split "class._ZTSN6dealii5PointILi2EEE.dealii::Point" into
  // <"class", "_ZTSN6dealii5PointILi2EEE.dealii::Point">
  // and check that first is "struct", "class" or "union"
  if (NameSplit.first != "struct" &&
      NameSplit.first != "class" &&
      NameSplit.first != "union")
    return None;

  if (NameSplit.second.empty())
    return None;

  // Mangled names in MS start with dot ('.') while in Linux they start with
  // underscore ('_'). We need to be careful when spliting the names. The
  // best thing to do is to skip the first character, find the next dot and
  // collect the string.
  if (NameSplit.second[0] != '.' && NameSplit.second[0] != '_')
    return None;

  // Find the dot in "_ZTSN6dealii5PointILi2EEE.dealii::Point"
  size_t DotPos = NameSplit.second.find('.', 1);
  if (DotPos == StringRef::npos)
    return None;

  // Collect _ZTSN6dealii5PointILi2EEE
  auto MangledName = NameSplit.second.take_front(DotPos);

  // Demangle _ZTSN6dealii5PointILi2EEE
  std::string DemangledNameStr = llvm::demangle(MangledName.str());
  StringRef DemangledName(DemangledNameStr);

  if (DemangledName == MangledName)
    return None;

  // Demangled names in Linux start with "typeinfo name" while in Windows
  // they start with the data structure type:
  //   %"struct.XYZ" -> "struct ABC"
  //   %"class.XYZ"  -> "class ABC"
  //   %"union.XYZ"  -> "union ABC"
  if (DemangledName.startswith(StringRef("typeinfo name")) ||
      DemangledName.startswith(StringRef(NameSplit.first.str() + " ")))
	  return MangledName;

  return None;
}

// Return true if the input structure name is a "base" structure. These are
// structures used as base for other structures that needs ABI padding. For
// example:
//
//   %class.TestClass = type { int, [ 4 x i8 ] }
//   %class.TestClass.base = types { int }
//
// %class.TestClass is the regular class with ABI padding and
// %class.TestClass.base is the base class. Both class will have the same name
// mangling, therefore we need to make sure that we can differentiate them.
static bool isBaseStructure(StructType *ST) {
  if (!ST || !ST->hasName())
    return false;

  StringRef StructName = ST->getName();

  if (StructName.empty())
    return false;

  StringRef StrName = getStructureNameClean(ST);
  StringRef BaseTag = StrName.take_back(std::string(".base").size());
  return BaseTag == StringRef(".base");
}

// Return true if the input structure name is a anonymous structure. This is
// for catching this case:
//
// struct Test {
//   int I;
//   struct {
//     int J;
//   };
// };
//
// The CFE will create the following:
//
//   %struct.Test = type { i32, %struct.Test.anon }
//   %struct.Test.anon = type { i32 }
//
// We need to make sure that we don't map an anonymous structure with a
// non-anonymous one (or vice-versa).
static bool isAnonStructure(StructType *ST) {
  if (!ST || !ST->hasName())
    return false;

  StringRef StructName = ST->getName();

  if (StructName.empty())
    return false;

// NOTE: Anonymous structures use the parent's name. For example, assume the
// following template:
//
//   template <class T>
//   class TestClass {
//   public:
//     TestClass() { }
//     void setVal(T I) { val = I; }
//     T getVal() { return val; }
//
//   private:
//     T val;
//     union {
//       int valInt;
//       double valDouble;
//     };
//   };
//
// Then assume that there is an instantiation as follows:
//
//   TestClass<int> T;
//
// The CFE will try to determine which field of the union will be used in
// order to produce a structure with one element. Assuming that the CFE
// found that it will only be used the second field of the union, then
// the CFE will generate the following structures:
//
//   %class._ZTS9TestClassIiE.TestClass =
//       type { i32, %union._ZTSN9TestClassIiEUt_E.anon }
//   %union._ZTSN9TestClassIiEUt_E.anon = type { double }
//
// Although the field in the union is a double, the mangled name used is from
// the parent type, which is a template instantiated as an integer. Basically,
// the name of an anonymous structure can be split as follows:
//
//   %DATASTRUCTURE.PARENTNAME.anon.XYZ
//
//   * DATASTRUCTURE: class, structure, union
//   * PARENTNAME: parent's mangled name
//   * anon: tag mentioning that is an anonymous structure
//   * XYZ: extra numbering in case the name is repeated (multiple anonymous)

  StringRef StrName = getStructureNameClean(ST);
  StringRef AnonTag = StrName.take_back(std::string(".anon").size());
  return AnonTag == StringRef(".anon");
}


/***************** Begin special functions ***************************/

// Return true if the structure name starts with
// "__Intel$Empty$Struct", and if the structure is opaque or it is empty
// (no fields).
static bool isSpecialEmptyStruct(StructType *ST) {
  if (!ST || !ST->hasName())
    return false;

  if (!ST->isOpaque() && ST->getNumElements() != 0)
    return false;

  StringRef StructName = getStructName(ST);
  std::string PlaceHolderName = "__Intel$Empty$Struct";

  return StructName.startswith(StringRef(PlaceHolderName));
}

// Return true if the DTrans structure name starts with
// "__Intel$Empty$Struct", and if the structure is opaque or it is empty
// (no fields).
static bool isSpecialEmptyDTransStruct(DTransStructType *DTStruct) {
  if (!DTStruct || !DTStruct->hasName())
    return false;

  if (!DTStruct->isOpaque() && DTStruct->getNumFields() != 0)
    return false;

  StringRef StructName = DTStruct->getName();
  std::string PlaceHolderName = "__Intel$Empty$Struct";

  return StructName.startswith(StringRef(PlaceHolderName));
}

// Return true if the input source pointer is a pointer to an empty
// structure, and the DTransType of the source pointer is a function pointer.
//
// NOTE: This is a workaround to find if an empty structure is being
// mapped to a function pointer. The CFE is not always emitting the
// right naming. Also, this is only for typed pointers. The issue won't
// happen with opaque pointers and we can remove this check.
static bool isSpecialEmptyStructToFuncMapping(PointerType *SrcPtr,
                                       DTransType *SrcField) {

  if (!SrcPtr || !SrcField)
    return false;

  if (SrcPtr->isOpaque())
    return false;

  StructType *EmptyStr = dyn_cast<StructType>(SrcPtr->getElementType());
  if (!EmptyStr)
    return false;

  if (EmptyStr->hasName())
    return false;

  if (EmptyStr->getNumElements() != 0)
    return false;

  auto *PtrSrcField = dyn_cast<DTransPointerType>(SrcField);
  if (!PtrSrcField)
    return false;

  return isa<DTransFunctionType>(SrcField->getPointerElementType());
}
/***************** End special functions ***************************/
#endif // INTEL_FEATURE_SW_DTRANS
} // namespace

#if INTEL_FEATURE_SW_DTRANS
// Verify that there are not repeated types in the destination module. Also,
// this function checks that the DTrans types in the destination type manager
// match with the destination's DTrans metadata. This function is similar
// to quickVerifyDestinationModule, with the exception that it reads the
// DTrans metadata, traverses through the structures collected, and compares
// each entry with the same DTrans structure in the destination module. This
// process can be expensive and should only be used for debugging purposes.
void IRLinker::verifyDestinationModule() {

  DEBUG_WITH_TYPE(DEBUG_DTRANS_TYPES,
    dbgs() << "Running destination module verifier\n");

  NamedMDNode *DTransMDTypes = DstM.getNamedMetadata("intel.dtrans.types");
  if (!DTransMDTypes) {
    DEBUG_WITH_TYPE(DEBUG_DTRANS_TYPES,
        dbgs() << "Destination module doesn't have DTrans metadata to "
                  "verify\n");
    return;
  }

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  if (TraceDTransMetadataLoss)
    dbgs() << "  Checking for metadata loss in destination module "
           << "verification:\n";
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

  // We need to create a new DTrans map since the destination module now
  // has new types.
  bool AllowsIncompleteMD = DstM.getContext().supportsTypedPointers() &&
                            EnableIncompleteDTransMetadata;

  std::vector<StructType *> Types = DstM.getIdentifiedStructTypes();
  DTransStructsMap DTMap(DstM, AllowsIncompleteMD, Types);

  // If the incomplete metadata can't be used then we can't do the verification
  // in case of metadata loss.
  if (!DTMap.isMDReadCorrectly() && !AllowsIncompleteMD) {
    DEBUG_WITH_TYPE(DEBUG_DTRANS_TYPES,
      dbgs() << "Verification couldn't be completed due to metadata loss\n");
    return;
  }

  for (StructType *ST : Types) {
    if (!ST->hasName())
      continue;

    assert(!isSpecialEmptyStruct(ST)
           && "Found empty structure in the destination module");

    // We can safely skip anonymous structures since they are enclosed
    // structures. If an anonymous structure wasn't mapped correctly, then
    // the parent enclosing structure wasn't mapped and it will assert.
    if (isAnonStructure(ST))
      continue;

    auto *DTinMap = DTMap.getDTransStructure(ST);
    if (!DTinMap) {
      assert(AllowsIncompleteMD && "Missing DTransStructType in "
                                   "destination metadata");

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
      if (TraceDTransMetadataLoss)
        dbgs() << "Warning: Missing DTrans type in metadata for: " << *ST
               << "\n";
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

      continue;
    }

    // We only need to verify the structures that don't have clean names
    // since there is a chance that they are repeated structures. The reason
    // is that we can have two modules, with two structures that contain the
    // same mangled names but they are different. This happens usually in C:
    //
    //   file1.c:
    //
    //     struct Test {
    //       int i;
    //     }
    //
    //   file2.c:
    //
    //     struct Test {
    //       float j;
    //     }
    //
    // The modules generated at compile step will have the following structures:
    //
    //   file1.o:  %"struct.MANGLEDNAME.Test" = types { i32 }
    //   file2.o:  %"struct.MANGLEDNAME.Test" = types { float }
    //
    // During the merging process then we will have the following:
    //
    //   lto.o:
    //
    //     %"struct.MANGLEDNAME.Test" = types { i32 }
    //     %"struct.MANGLEDNAME.Test.0" = types { float }
    //
    // Although the mangled names matches, they are different structures.
    if (!isStructureNameClean(ST)) {
      // The structure names should be the same.
      StringRef CleanName = getStructureNameClean(ST);
      auto *DTCleanNameST = DTMap.getDTransStructure(CleanName);
      if (!DTCleanNameST) {
        assert(AllowsIncompleteMD && "Missing DTransStructType in "
                                     "type mapper");

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
       if (TraceDTransMetadataLoss)
          dbgs() << "Warning: Missing DTrans type in map for: " << *ST << "\n";
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

        continue;
      }

      DEBUG_WITH_TYPE(DEBUG_DTRANS_TYPES, {
        dbgs() << "Possible type mismatch between:\n";
        dbgs() << "  Structure name: " << ST->getName() << "\n";
        dbgs() << "  Clean name: " << CleanName << "\n";
      });

      assert(!(DTCleanNameST->compare(*DTinMap)) && "DTransType matches "
          "between two structures");

      DEBUG_WITH_TYPE(DEBUG_DTRANS_TYPES,
          dbgs() << "  Pass assertions, two different structures\n\n");
    }

    // Check that the DTrans type in the destination type manager matches
    // with the DTrans metadata collected.
    auto *DTinDstTM = DstTM->getStructType(ST->getName());
    if (!DTinDstTM) {
      assert(AllowsIncompleteMD && "Missing DTransStructType in destination "
          "type manager when it is available in DTrans metadata");

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
      if (TraceDTransMetadataLoss)
        dbgs() << "Warning: DTransStructType not generated in type "
               << "manager for: " << *ST << "\n";
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

      continue;
    }
    assert(DTinMap->compare(*DTinDstTM) && "Mismatch between DTrans type in "
        "destination type manager and DTrans destination metadata");

/***************** Begin special functions ***************************/
    // Make sure that the fields were repaired. This is only when we aren't
    // dealing with opaque pointers
    for (unsigned I = 0, E = ST->getNumElements(); I < E; I++) {

      auto *PtrField = dyn_cast<PointerType>(ST->getElementType(I));
      if (!PtrField)
        continue;

      assert(!isSpecialEmptyStructToFuncMapping(PtrField,
          DTinMap->getFieldType(I)) && "Found special empty field in "
          "a structure");
    }
/***************** End special functions ***************************/

  }

  DEBUG_WITH_TYPE(DEBUG_DTRANS_TYPES,
    dbgs() << "Destination module passed verification\n");
}

// Simple verifier that checks for repeated types in the destination module.
// It will traverse through each DTrans structure in the DTrans type mapper.
// If the name of the structure contains an extra numbering at the end
// (e.g. %struct.test.0), then it will collect the structure that has the clean
// name (%struct.test) and make sure that the bodies of both structures are
// different. Also, it checks the fields of each structure that it doesn't have
// pointers to empty structure when it should be pointers to function type.
void IRLinker::quickVerifyDestinationModule() {

  bool AllowsIncompleteMD = DstM.getContext().supportsTypedPointers() &&
                            EnableIncompleteDTransMetadata;
  (void) AllowsIncompleteMD;

  DEBUG_WITH_TYPE(DEBUG_DTRANS_TYPES,
    dbgs() << "Running destination module simple verifier\n");

  for (auto *DTinMap : DstTM->getIdentifiedStructTypes()) {
    auto *ST = cast<StructType>(DTinMap->getLLVMType());
    if (!ST->hasName())
      continue;

    assert(!isSpecialEmptyStruct(ST)
           && "Found empty structure in the destination module");

    // We can safely skip anonymous structures since they are enclosed
    // structures. If an anonymous structure wasn't mapped correctly, then
    // the parent enclosing structure wasn't mapped and it will assert.
    if (isAnonStructure(ST))
      continue;

    // We only need to verify the structures that don't have clean name
    // since there is a chance that they are repeated structures
    if (!isStructureNameClean(ST)) {
      // The structure names should be the same.
      StringRef CleanName = getStructureNameClean(ST);
      auto *DTCleanNameST = DstTM->getStructType(CleanName);
      if (!DTCleanNameST) {
        assert(AllowsIncompleteMD && "Missing DTransStructType in "
                                     "type mapper");

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
        if (TraceDTransMetadataLoss)
          dbgs() << "Warning: Missing DTrans type in map for: " << *ST
                 << "\n";
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

        continue;
      }

      DEBUG_WITH_TYPE(DEBUG_DTRANS_TYPES, {
        dbgs() << "Possible type mismatch between:\n";
        dbgs() << "  Structure name: " << ST->getName() << "\n";
        dbgs() << "  Clean name: " << CleanName << "\n";
      });

      assert(!(DTCleanNameST->compare(*DTinMap)) && "DTransType matches "
          "between two structures");

      DEBUG_WITH_TYPE(DEBUG_DTRANS_TYPES,
          dbgs() << "  Pass assertions, two different structures\n\n");
    }

/***************** Begin special functions ***************************/
    // Make sure that the fields were repaired. This is only when we aren't
    // dealing with opaque pointers
    for (unsigned I = 0, E = ST->getNumElements(); I < E; I++) {

      auto *PtrField = dyn_cast<PointerType>(ST->getElementType(I));
      if (!PtrField)
        continue;

      assert(!isSpecialEmptyStructToFuncMapping(PtrField,
          DTinMap->getFieldType(I)) && "Found special empty field in "
          "a structure");
    }
/***************** End special functions ***************************/
  }

  DEBUG_WITH_TYPE(DEBUG_DTRANS_TYPES,
    dbgs() << "Destination module passed simple verification\n");
}
#endif // INTEL_FEATURE_SW_DTRANS
#endif // INTEL_CUSTOMIZATION

/// Loop over all of the linked values to compute type mappings.  For example,
/// if we link "extern Foo *x" and "Foo *x = NULL", then we have two struct
/// types 'Foo' but one got renamed when the module was loaded into the same
/// LLVMContext.
void IRLinker::computeTypeMapping() {
#if INTEL_CUSTOMIZATION
#if INTEL_FEATURE_SW_DTRANS
  // Check if we can map the structures using the DTrans information
  bool IsMappingByDTransInfoEnabled =
      TypeMap.mapTypesToDTransData(*SrcM, DstM);
#endif // INTEL_FEATURE_SW_DTRANS
#endif // INTEL_CUSTOMIZATION

  for (GlobalValue &SGV : SrcM->globals()) {
    GlobalValue *DGV = getLinkedToGlobal(&SGV);
    if (!DGV)
      continue;

    if (!DGV->hasAppendingLinkage() || !SGV.hasAppendingLinkage()) {
      TypeMap.addTypeMapping(DGV->getType(), SGV.getType());
      continue;
    }

    // Unify the element type of appending arrays.
    ArrayType *DAT = cast<ArrayType>(DGV->getValueType());
    ArrayType *SAT = cast<ArrayType>(SGV.getValueType());
    TypeMap.addTypeMapping(DAT->getElementType(), SAT->getElementType());
  }

  for (GlobalValue &SGV : *SrcM)
    if (GlobalValue *DGV = getLinkedToGlobal(&SGV)) {
      if (DGV->getType() == SGV.getType()) {
        // If the types of DGV and SGV are the same, it means that DGV is from
        // the source module and got added to DstM from a shared metadata.  We
        // shouldn't map this type to itself in case the type's components get
        // remapped to a new type from DstM (for instance, during the loop over
        // SrcM->getIdentifiedStructTypes() below).
        continue;
      }

      TypeMap.addTypeMapping(DGV->getType(), SGV.getType());
    }

  for (GlobalValue &SGV : SrcM->aliases())
    if (GlobalValue *DGV = getLinkedToGlobal(&SGV))
      TypeMap.addTypeMapping(DGV->getType(), SGV.getType());

#if INTEL_CUSTOMIZATION
#if INTEL_FEATURE_SW_DTRANS
  // If mapping with DTrans information passed succesfully then we can skip
  // the next loop. We don't need to spend time traversing through the same
  // types again, plus calling getIdentifiedStructTypes is very expensive.
  if (!IsMappingByDTransInfoEnabled) {
#endif // INTEL_FEATURE_SW_DTRANS
#endif // INTEL_CUSTOMIZATION

  // Incorporate types by name, scanning all the types in the source module.
  // At this point, the destination module may have a type "%foo = { i32 }" for
  // example.  When the source module got loaded into the same LLVMContext, if
  // it had the same type, it would have been renamed to "%foo.42 = { i32 }".
  std::vector<StructType *> Types = SrcM->getIdentifiedStructTypes();
  for (StructType *ST : Types) {
    if (!ST->hasName())
      continue;

    if (TypeMap.DstStructTypesSet.hasType(ST)) {
      // This is actually a type from the destination module.
      // getIdentifiedStructTypes() can have found it by walking debug info
      // metadata nodes, some of which get linked by name when ODR Type Uniquing
      // is enabled on the Context, from the source to the destination module.
      continue;
    }

    auto STTypePrefix = getTypeNamePrefix(ST->getName());
    if (STTypePrefix.size() == ST->getName().size())
      continue;

    // Check to see if the destination module has a struct with the prefix name.
    StructType *DST = StructType::getTypeByName(ST->getContext(), STTypePrefix);
    if (!DST)
      continue;

    // Don't use it if this actually came from the source module. They're in
    // the same LLVMContext after all. Also don't use it unless the type is
    // actually used in the destination module. This can happen in situations
    // like this:
    //
    //      Module A                         Module B
    //      --------                         --------
    //   %Z = type { %A }                %B = type { %C.1 }
    //   %A = type { %B.1, [7 x i8] }    %C.1 = type { i8* }
    //   %B.1 = type { %C }              %A.2 = type { %B.3, [5 x i8] }
    //   %C = type { i8* }               %B.3 = type { %C.1 }
    //
    // When we link Module B with Module A, the '%B' in Module B is
    // used. However, that would then use '%C.1'. But when we process '%C.1',
    // we prefer to take the '%C' version. So we are then left with both
    // '%C.1' and '%C' being used for the same types. This leads to some
    // variables using one type and some using the other.
    if (TypeMap.DstStructTypesSet.hasType(DST))
      TypeMap.addTypeMapping(DST, ST);
  }

#if INTEL_CUSTOMIZATION
#if INTEL_FEATURE_SW_DTRANS
  }
#endif // INTEL_FEATURE_SW_DTRANS
#endif // INTEL_CUSTOMIZATION

  // Now that we have discovered all of the type equivalences, get a body for
  // any 'opaque' types in the dest module that are now resolved.
  TypeMap.linkDefinedTypeBodies();
}

static void getArrayElements(const Constant *C,
                             SmallVectorImpl<Constant *> &Dest) {
  unsigned NumElements = cast<ArrayType>(C->getType())->getNumElements();

  for (unsigned i = 0; i != NumElements; ++i)
    Dest.push_back(C->getAggregateElement(i));
}

/// If there were any appending global variables, link them together now.
Expected<Constant *>
IRLinker::linkAppendingVarProto(GlobalVariable *DstGV,
                                const GlobalVariable *SrcGV) {
  // Check that both variables have compatible properties.
  if (DstGV && !DstGV->isDeclaration() && !SrcGV->isDeclaration()) {
    if (!SrcGV->hasAppendingLinkage() || !DstGV->hasAppendingLinkage())
      return stringErr(
          "Linking globals named '" + SrcGV->getName() +
          "': can only link appending global with another appending "
          "global!");

    if (DstGV->isConstant() != SrcGV->isConstant())
      return stringErr("Appending variables linked with different const'ness!");

    if (DstGV->getAlign() != SrcGV->getAlign())
      return stringErr(
          "Appending variables with different alignment need to be linked!");

    if (DstGV->getVisibility() != SrcGV->getVisibility())
      return stringErr(
          "Appending variables with different visibility need to be linked!");

    if (DstGV->hasGlobalUnnamedAddr() != SrcGV->hasGlobalUnnamedAddr())
      return stringErr(
          "Appending variables with different unnamed_addr need to be linked!");

    if (DstGV->getSection() != SrcGV->getSection())
      return stringErr(
          "Appending variables with different section name need to be linked!");
  }

  // Do not need to do anything if source is a declaration.
  if (SrcGV->isDeclaration())
    return DstGV;

  Type *EltTy = cast<ArrayType>(TypeMap.get(SrcGV->getValueType()))
                    ->getElementType();

  // FIXME: This upgrade is done during linking to support the C API.  Once the
  // old form is deprecated, we should move this upgrade to
  // llvm::UpgradeGlobalVariable() and simplify the logic here and in
  // Mapper::mapAppendingVariable() in ValueMapper.cpp.
  StringRef Name = SrcGV->getName();
  bool IsNewStructor = false;
  bool IsOldStructor = false;
  if (Name == "llvm.global_ctors" || Name == "llvm.global_dtors") {
    if (cast<StructType>(EltTy)->getNumElements() == 3)
      IsNewStructor = true;
    else
      IsOldStructor = true;
  }

  PointerType *VoidPtrTy = Type::getInt8Ty(SrcGV->getContext())->getPointerTo();
  if (IsOldStructor) {
    auto &ST = *cast<StructType>(EltTy);
    Type *Tys[3] = {ST.getElementType(0), ST.getElementType(1), VoidPtrTy};
    EltTy = StructType::get(SrcGV->getContext(), Tys, false);
  }

  uint64_t DstNumElements = 0;
  if (DstGV && !DstGV->isDeclaration()) {
    ArrayType *DstTy = cast<ArrayType>(DstGV->getValueType());
    DstNumElements = DstTy->getNumElements();

    // Check to see that they two arrays agree on type.
    if (EltTy != DstTy->getElementType())
      return stringErr("Appending variables with different element types!");
  }

  SmallVector<Constant *, 16> SrcElements;
  getArrayElements(SrcGV->getInitializer(), SrcElements);

  if (IsNewStructor) {
    erase_if(SrcElements, [this](Constant *E) {
      auto *Key =
          dyn_cast<GlobalValue>(E->getAggregateElement(2)->stripPointerCasts());
      if (!Key)
        return false;
      GlobalValue *DGV = getLinkedToGlobal(Key);
      return !shouldLink(DGV, *Key);
    });
  }
  uint64_t NewSize = DstNumElements + SrcElements.size();
  ArrayType *NewType = ArrayType::get(EltTy, NewSize);

  // Create the new global variable.
  GlobalVariable *NG = new GlobalVariable(
      DstM, NewType, SrcGV->isConstant(), SrcGV->getLinkage(),
      /*init*/ nullptr, /*name*/ "", DstGV, SrcGV->getThreadLocalMode(),
      SrcGV->getAddressSpace());

  NG->copyAttributesFrom(SrcGV);
  forceRenaming(NG, SrcGV->getName());

  Constant *Ret = ConstantExpr::getBitCast(NG, TypeMap.get(SrcGV->getType()));

  Mapper.scheduleMapAppendingVariable(
      *NG,
      (DstGV && !DstGV->isDeclaration()) ? DstGV->getInitializer() : nullptr,
      IsOldStructor, SrcElements);

  // Replace any uses of the two global variables with uses of the new
  // global.
  if (DstGV) {
    RAUWWorklist.push_back(
        std::make_pair(DstGV, ConstantExpr::getBitCast(NG, DstGV->getType())));
  }

#if INTEL_CUSTOMIZATION
#if INTEL_FEATURE_SW_DTRANS
  // New EltTy is created (i.e not generated by CFE) when IsOldStructor is
  // true. Avoid attaching intel_dtrans_type metadata when CFE generated
  // type is not used for appending variables.
  if (!IsOldStructor)
    dtransOP::DTransTypeMetadataPropagator::setGlobAppendingVarDTransMetadata(
        SrcGV, DstGV, NG, NewSize);
#endif // INTEL_FEATURE_SW_DTRANS
#endif // INTEL_CUSTOMIZATION

  return Ret;
}

bool IRLinker::shouldLink(GlobalValue *DGV, GlobalValue &SGV) {
  if (ValuesToLink.count(&SGV) || SGV.hasLocalLinkage())
    return true;

  if (DGV && !DGV->isDeclarationForLinker())
    return false;

  if (SGV.isDeclaration() || DoneLinkingBodies)
    return false;

  // Callback to the client to give a chance to lazily add the Global to the
  // list of value to link.
  bool LazilyAdded = false;
  if (AddLazyFor)
    AddLazyFor(SGV, [this, &LazilyAdded](GlobalValue &GV) {
      maybeAdd(&GV);
      LazilyAdded = true;
    });
  return LazilyAdded;
}

Expected<Constant *> IRLinker::linkGlobalValueProto(GlobalValue *SGV,
                                                    bool ForIndirectSymbol) {
  GlobalValue *DGV = getLinkedToGlobal(SGV);

  bool ShouldLink = shouldLink(DGV, *SGV);

  // just missing from map
  if (ShouldLink) {
    auto I = ValueMap.find(SGV);
    if (I != ValueMap.end())
      return cast<Constant>(I->second);

    I = IndirectSymbolValueMap.find(SGV);
    if (I != IndirectSymbolValueMap.end())
      return cast<Constant>(I->second);
  }

  if (!ShouldLink && ForIndirectSymbol)
    DGV = nullptr;

  // Handle the ultra special appending linkage case first.
  if (SGV->hasAppendingLinkage() || (DGV && DGV->hasAppendingLinkage()))
    return linkAppendingVarProto(cast_or_null<GlobalVariable>(DGV),
                                 cast<GlobalVariable>(SGV));

  bool NeedsRenaming = false;
  GlobalValue *NewGV;
  if (DGV && !ShouldLink) {
    NewGV = DGV;
  } else {
    // If we are done linking global value bodies (i.e. we are performing
    // metadata linking), don't link in the global value due to this
    // reference, simply map it to null.
    if (DoneLinkingBodies)
      return nullptr;

    NewGV = copyGlobalValueProto(SGV, ShouldLink || ForIndirectSymbol);
    if (ShouldLink || !ForIndirectSymbol)
      NeedsRenaming = true;
#if INTEL_CUSTOMIZATION
#if INTEL_FEATURE_SW_DTRANS
    // A new GlobalValue has been introduced into the destination module that
    // may need to have DTrans metadata updated. We cannot remap the data
    // immediately because a FlushingRemapper is already in progress. Save
    // the Value object for updating the metadata attachment after all the IR
    // definitions have been mapped.
    if (NewGV->isDeclaration())
      if (auto *NewGO = dyn_cast<GlobalObject>(NewGV))
        if (NewGO->hasMetadata("intel.dtrans.func.type") ||
            NewGO->hasMetadata("intel_dtrans_type"))
          DTransMDRemapWorklist.insert(NewGO);
#endif // INTEL_FEATURE_SW_DTRANS
#endif // INTEL_CUSTOMIZATION
  }

  // Overloaded intrinsics have overloaded types names as part of their
  // names. If we renamed overloaded types we should rename the intrinsic
  // as well.
  if (Function *F = dyn_cast<Function>(NewGV))
    if (auto Remangled = Intrinsic::remangleIntrinsicFunction(F)) {
      NewGV->eraseFromParent();
      NewGV = *Remangled;
      NeedsRenaming = false;
    }

  if (NeedsRenaming)
    forceRenaming(NewGV, SGV->getName());

  if (ShouldLink || ForIndirectSymbol) {
    if (const Comdat *SC = SGV->getComdat()) {
      if (auto *GO = dyn_cast<GlobalObject>(NewGV)) {
        Comdat *DC = DstM.getOrInsertComdat(SC->getName());
        DC->setSelectionKind(SC->getSelectionKind());
        GO->setComdat(DC);
      }
    }
  }

  if (!ShouldLink && ForIndirectSymbol)
    NewGV->setLinkage(GlobalValue::InternalLinkage);

  Constant *C = NewGV;
  // Only create a bitcast if necessary. In particular, with
  // DebugTypeODRUniquing we may reach metadata in the destination module
  // containing a GV from the source module, in which case SGV will be
  // the same as DGV and NewGV, and TypeMap.get() will assert since it
  // assumes it is being invoked on a type in the source module.
  if (DGV && NewGV != SGV) {
    C = ConstantExpr::getPointerBitCastOrAddrSpaceCast(
      NewGV, TypeMap.get(SGV->getType()));
  }

  if (DGV && NewGV != DGV) {
    // Schedule "replace all uses with" to happen after materializing is
    // done. It is not safe to do it now, since ValueMapper may be holding
    // pointers to constants that will get deleted if RAUW runs.
    RAUWWorklist.push_back(std::make_pair(
        DGV,
        ConstantExpr::getPointerBitCastOrAddrSpaceCast(NewGV, DGV->getType())));
  }

  return C;
}

/// Update the initializers in the Dest module now that all globals that may be
/// referenced are in Dest.
void IRLinker::linkGlobalVariable(GlobalVariable &Dst, GlobalVariable &Src) {
  // Figure out what the initializer looks like in the dest module.
  Mapper.scheduleMapGlobalInitializer(Dst, *Src.getInitializer());
}

/// Copy the source function over into the dest function and fix up references
/// to values. At this point we know that Dest is an external function, and
/// that Src is not.
Error IRLinker::linkFunctionBody(Function &Dst, Function &Src) {
  assert(Dst.isDeclaration() && !Src.isDeclaration());

  // Materialize if needed.
  if (Error Err = Src.materialize())
    return Err;

  // Link in the operands without remapping.
  if (Src.hasPrefixData())
    Dst.setPrefixData(Src.getPrefixData());
  if (Src.hasPrologueData())
    Dst.setPrologueData(Src.getPrologueData());
  if (Src.hasPersonalityFn())
    Dst.setPersonalityFn(Src.getPersonalityFn());

  // Copy over the metadata attachments without remapping.
  Dst.copyMetadata(&Src, 0);

  // Steal arguments and splice the body of Src into Dst.
  Dst.stealArgumentListFrom(Src);
  Dst.getBasicBlockList().splice(Dst.end(), Src.getBasicBlockList());

  // Everything has been moved over.  Remap it.
  Mapper.scheduleRemapFunction(Dst);
  return Error::success();
}

void IRLinker::linkAliasAliasee(GlobalAlias &Dst, GlobalAlias &Src) {
  Mapper.scheduleMapGlobalAlias(Dst, *Src.getAliasee(), IndirectSymbolMCID);
}

void IRLinker::linkIFuncResolver(GlobalIFunc &Dst, GlobalIFunc &Src) {
  Mapper.scheduleMapGlobalIFunc(Dst, *Src.getResolver(), IndirectSymbolMCID);
}

Error IRLinker::linkGlobalValueBody(GlobalValue &Dst, GlobalValue &Src) {
  if (auto *F = dyn_cast<Function>(&Src))
    return linkFunctionBody(cast<Function>(Dst), *F);
  if (auto *GVar = dyn_cast<GlobalVariable>(&Src)) {
    linkGlobalVariable(cast<GlobalVariable>(Dst), *GVar);
    return Error::success();
  }
  if (auto *GA = dyn_cast<GlobalAlias>(&Src)) {
    linkAliasAliasee(cast<GlobalAlias>(Dst), *GA);
    return Error::success();
  }
  linkIFuncResolver(cast<GlobalIFunc>(Dst), cast<GlobalIFunc>(Src));
  return Error::success();
}

void IRLinker::flushRAUWWorklist() {
  for (const auto &Elem : RAUWWorklist) {
    GlobalValue *Old;
    Value *New;
    std::tie(Old, New) = Elem;

    Old->replaceAllUsesWith(New);
    Old->eraseFromParent();
  }
  RAUWWorklist.clear();
}

void IRLinker::prepareCompileUnitsForImport() {
  NamedMDNode *SrcCompileUnits = SrcM->getNamedMetadata("llvm.dbg.cu");
  if (!SrcCompileUnits)
    return;
  // When importing for ThinLTO, prevent importing of types listed on
  // the DICompileUnit that we don't need a copy of in the importing
  // module. They will be emitted by the originating module.
  for (unsigned I = 0, E = SrcCompileUnits->getNumOperands(); I != E; ++I) {
    auto *CU = cast<DICompileUnit>(SrcCompileUnits->getOperand(I));
    assert(CU && "Expected valid compile unit");
    // Enums, macros, and retained types don't need to be listed on the
    // imported DICompileUnit. This means they will only be imported
    // if reached from the mapped IR.
    CU->replaceEnumTypes(nullptr);
    CU->replaceMacros(nullptr);
    CU->replaceRetainedTypes(nullptr);

    // The original definition (or at least its debug info - if the variable is
    // internalized and optimized away) will remain in the source module, so
    // there's no need to import them.
    // If LLVM ever does more advanced optimizations on global variables
    // (removing/localizing write operations, for instance) that can track
    // through debug info, this decision may need to be revisited - but do so
    // with care when it comes to debug info size. Emitting small CUs containing
    // only a few imported entities into every destination module may be very
    // size inefficient.
    CU->replaceGlobalVariables(nullptr);

    // Imported entities only need to be mapped in if they have local
    // scope, as those might correspond to an imported entity inside a
    // function being imported (any locally scoped imported entities that
    // don't end up referenced by an imported function will not be emitted
    // into the object). Imported entities not in a local scope
    // (e.g. on the namespace) only need to be emitted by the originating
    // module. Create a list of the locally scoped imported entities, and
    // replace the source CUs imported entity list with the new list, so
    // only those are mapped in.
    // FIXME: Locally-scoped imported entities could be moved to the
    // functions they are local to instead of listing them on the CU, and
    // we would naturally only link in those needed by function importing.
    SmallVector<TrackingMDNodeRef, 4> AllImportedModules;
    bool ReplaceImportedEntities = false;
    for (auto *IE : CU->getImportedEntities()) {
      DIScope *Scope = IE->getScope();
      assert(Scope && "Invalid Scope encoding!");
      if (isa<DILocalScope>(Scope))
        AllImportedModules.emplace_back(IE);
      else
        ReplaceImportedEntities = true;
    }
    if (ReplaceImportedEntities) {
      if (!AllImportedModules.empty())
        CU->replaceImportedEntities(MDTuple::get(
            CU->getContext(),
            SmallVector<Metadata *, 16>(AllImportedModules.begin(),
                                        AllImportedModules.end())));
      else
        // If there were no local scope imported entities, we can map
        // the whole list to nullptr.
        CU->replaceImportedEntities(nullptr);
    }
  }
}

/// Insert all of the named MDNodes in Src into the Dest module.
void IRLinker::linkNamedMDNodes() {
  const NamedMDNode *SrcModFlags = SrcM->getModuleFlagsMetadata();
  for (const NamedMDNode &NMD : SrcM->named_metadata()) {
    // Don't link module flags here. Do them separately.
    if (&NMD == SrcModFlags)
      continue;
    // Don't import pseudo probe descriptors here for thinLTO. They will be
    // emitted by the originating module.
    if (IsPerformingImport && NMD.getName() == PseudoProbeDescMetadataName) {
      if (!DstM.getNamedMetadata(NMD.getName()))
        emitWarning("Pseudo-probe ignored: source module '" +
                    SrcM->getModuleIdentifier() +
                    "' is compiled with -fpseudo-probe-for-profiling while "
                    "destination module '" +
                    DstM.getModuleIdentifier() + "' is not\n");
      continue;
    }
    NamedMDNode *DestNMD = DstM.getOrInsertNamedMetadata(NMD.getName());
    // Add Src elements into Dest node.
    for (const MDNode *Op : NMD.operands())
      DestNMD->addOperand(Mapper.mapMDNode(*Op));
  }
}

/// Merge the linker flags in Src into the Dest module.
Error IRLinker::linkModuleFlagsMetadata() {
  // If the source module has no module flags, we are done.
  const NamedMDNode *SrcModFlags = SrcM->getModuleFlagsMetadata();
  if (!SrcModFlags)
    return Error::success();

  // Check for module flag for updates before do anything.
  UpgradeModuleFlags(*SrcM);

  // If the destination module doesn't have module flags yet, then just copy
  // over the source module's flags.
  NamedMDNode *DstModFlags = DstM.getOrInsertModuleFlagsMetadata();
  if (DstModFlags->getNumOperands() == 0) {
    for (unsigned I = 0, E = SrcModFlags->getNumOperands(); I != E; ++I)
      DstModFlags->addOperand(SrcModFlags->getOperand(I));

    return Error::success();
  }

  // First build a map of the existing module flags and requirements.
  DenseMap<MDString *, std::pair<MDNode *, unsigned>> Flags;
  SmallSetVector<MDNode *, 16> Requirements;
  SmallVector<unsigned, 0> Mins;
  DenseSet<MDString *> SeenMin;
  for (unsigned I = 0, E = DstModFlags->getNumOperands(); I != E; ++I) {
    MDNode *Op = DstModFlags->getOperand(I);
    uint64_t Behavior =
        mdconst::extract<ConstantInt>(Op->getOperand(0))->getZExtValue();
    MDString *ID = cast<MDString>(Op->getOperand(1));

    if (Behavior == Module::Require) {
      Requirements.insert(cast<MDNode>(Op->getOperand(2)));
    } else {
      if (Behavior == Module::Min)
        Mins.push_back(I);
      Flags[ID] = std::make_pair(Op, I);
    }
  }

  // Merge in the flags from the source module, and also collect its set of
  // requirements.
  for (unsigned I = 0, E = SrcModFlags->getNumOperands(); I != E; ++I) {
    MDNode *SrcOp = SrcModFlags->getOperand(I);
    ConstantInt *SrcBehavior =
        mdconst::extract<ConstantInt>(SrcOp->getOperand(0));
    MDString *ID = cast<MDString>(SrcOp->getOperand(1));
    MDNode *DstOp;
    unsigned DstIndex;
    std::tie(DstOp, DstIndex) = Flags.lookup(ID);
    unsigned SrcBehaviorValue = SrcBehavior->getZExtValue();
    SeenMin.insert(ID);

    // If this is a requirement, add it and continue.
    if (SrcBehaviorValue == Module::Require) {
      // If the destination module does not already have this requirement, add
      // it.
      if (Requirements.insert(cast<MDNode>(SrcOp->getOperand(2)))) {
        DstModFlags->addOperand(SrcOp);
      }
      continue;
    }

    // If there is no existing flag with this ID, just add it.
    if (!DstOp) {
      if (SrcBehaviorValue == Module::Min) {
        Mins.push_back(DstModFlags->getNumOperands());
        SeenMin.erase(ID);
      }
      Flags[ID] = std::make_pair(SrcOp, DstModFlags->getNumOperands());
      DstModFlags->addOperand(SrcOp);
      continue;
    }

    // Otherwise, perform a merge.
    ConstantInt *DstBehavior =
        mdconst::extract<ConstantInt>(DstOp->getOperand(0));
    unsigned DstBehaviorValue = DstBehavior->getZExtValue();

    auto overrideDstValue = [&]() {
      DstModFlags->setOperand(DstIndex, SrcOp);
      Flags[ID].first = SrcOp;
    };

    // If either flag has override behavior, handle it first.
    if (DstBehaviorValue == Module::Override) {
      // Diagnose inconsistent flags which both have override behavior.
      if (SrcBehaviorValue == Module::Override &&
          SrcOp->getOperand(2) != DstOp->getOperand(2))
        return stringErr("linking module flags '" + ID->getString() +
                         "': IDs have conflicting override values in '" +
                         SrcM->getModuleIdentifier() + "' and '" +
                         DstM.getModuleIdentifier() + "'");
      continue;
    } else if (SrcBehaviorValue == Module::Override) {
      // Update the destination flag to that of the source.
      overrideDstValue();
      continue;
    }

    // Diagnose inconsistent merge behavior types.
    if (SrcBehaviorValue != DstBehaviorValue) {
      bool MinAndWarn = (SrcBehaviorValue == Module::Min &&
                         DstBehaviorValue == Module::Warning) ||
                        (DstBehaviorValue == Module::Min &&
                         SrcBehaviorValue == Module::Warning);
      bool MaxAndWarn = (SrcBehaviorValue == Module::Max &&
                         DstBehaviorValue == Module::Warning) ||
                        (DstBehaviorValue == Module::Max &&
                         SrcBehaviorValue == Module::Warning);
      if (!(MaxAndWarn || MinAndWarn))
        return stringErr("linking module flags '" + ID->getString() +
                         "': IDs have conflicting behaviors in '" +
                         SrcM->getModuleIdentifier() + "' and '" +
                         DstM.getModuleIdentifier() + "'");
    }

    auto ensureDistinctOp = [&](MDNode *DstValue) {
      assert(isa<MDTuple>(DstValue) &&
             "Expected MDTuple when appending module flags");
      if (DstValue->isDistinct())
        return dyn_cast<MDTuple>(DstValue);
      ArrayRef<MDOperand> DstOperands = DstValue->operands();
      MDTuple *New = MDTuple::getDistinct(
          DstM.getContext(),
          SmallVector<Metadata *, 4>(DstOperands.begin(), DstOperands.end()));
      Metadata *FlagOps[] = {DstOp->getOperand(0), ID, New};
      MDNode *Flag = MDTuple::getDistinct(DstM.getContext(), FlagOps);
      DstModFlags->setOperand(DstIndex, Flag);
      Flags[ID].first = Flag;
      return New;
    };

    // Emit a warning if the values differ and either source or destination
    // request Warning behavior.
    if ((DstBehaviorValue == Module::Warning ||
         SrcBehaviorValue == Module::Warning) &&
        SrcOp->getOperand(2) != DstOp->getOperand(2)) {
      std::string Str;
      raw_string_ostream(Str)
          << "linking module flags '" << ID->getString()
          << "': IDs have conflicting values ('" << *SrcOp->getOperand(2)
          << "' from " << SrcM->getModuleIdentifier() << " with '"
          << *DstOp->getOperand(2) << "' from " << DstM.getModuleIdentifier()
          << ')';
      emitWarning(Str);
    }

    // Choose the minimum if either source or destination request Min behavior.
    if (DstBehaviorValue == Module::Min || SrcBehaviorValue == Module::Min) {
      ConstantInt *DstValue =
          mdconst::extract<ConstantInt>(DstOp->getOperand(2));
      ConstantInt *SrcValue =
          mdconst::extract<ConstantInt>(SrcOp->getOperand(2));

      // The resulting flag should have a Min behavior, and contain the minimum
      // value from between the source and destination values.
      Metadata *FlagOps[] = {
          (DstBehaviorValue != Module::Min ? SrcOp : DstOp)->getOperand(0), ID,
          (SrcValue->getZExtValue() < DstValue->getZExtValue() ? SrcOp : DstOp)
              ->getOperand(2)};
      MDNode *Flag = MDNode::get(DstM.getContext(), FlagOps);
      DstModFlags->setOperand(DstIndex, Flag);
      Flags[ID].first = Flag;
      continue;
    }

    // Choose the maximum if either source or destination request Max behavior.
    if (DstBehaviorValue == Module::Max || SrcBehaviorValue == Module::Max) {
      ConstantInt *DstValue =
          mdconst::extract<ConstantInt>(DstOp->getOperand(2));
      ConstantInt *SrcValue =
          mdconst::extract<ConstantInt>(SrcOp->getOperand(2));

      // The resulting flag should have a Max behavior, and contain the maximum
      // value from between the source and destination values.
      Metadata *FlagOps[] = {
          (DstBehaviorValue != Module::Max ? SrcOp : DstOp)->getOperand(0), ID,
          (SrcValue->getZExtValue() > DstValue->getZExtValue() ? SrcOp : DstOp)
              ->getOperand(2)};
      MDNode *Flag = MDNode::get(DstM.getContext(), FlagOps);
      DstModFlags->setOperand(DstIndex, Flag);
      Flags[ID].first = Flag;
      continue;
    }

    // Perform the merge for standard behavior types.
    switch (SrcBehaviorValue) {
    case Module::Require:
    case Module::Override:
      llvm_unreachable("not possible");
    case Module::Error: {
      // Emit an error if the values differ.
      if (SrcOp->getOperand(2) != DstOp->getOperand(2))
        return stringErr("linking module flags '" + ID->getString() +
                         "': IDs have conflicting values in '" +
                         SrcM->getModuleIdentifier() + "' and '" +
                         DstM.getModuleIdentifier() + "'");
      continue;
    }
    case Module::Warning: {
      break;
    }
    case Module::Max: {
      break;
    }
    case Module::Append: {
      MDTuple *DstValue = ensureDistinctOp(cast<MDNode>(DstOp->getOperand(2)));
      MDNode *SrcValue = cast<MDNode>(SrcOp->getOperand(2));
      for (const auto &O : SrcValue->operands())
        DstValue->push_back(O);
      break;
    }
    case Module::AppendUnique: {
      SmallSetVector<Metadata *, 16> Elts;
      MDTuple *DstValue = ensureDistinctOp(cast<MDNode>(DstOp->getOperand(2)));
      MDNode *SrcValue = cast<MDNode>(SrcOp->getOperand(2));
      Elts.insert(DstValue->op_begin(), DstValue->op_end());
      Elts.insert(SrcValue->op_begin(), SrcValue->op_end());
      for (auto I = DstValue->getNumOperands(); I < Elts.size(); I++)
        DstValue->push_back(Elts[I]);
      break;
    }
    }

  }

  // For the Min behavior, set the value to 0 if either module does not have the
  // flag.
  for (auto Idx : Mins) {
    MDNode *Op = DstModFlags->getOperand(Idx);
    MDString *ID = cast<MDString>(Op->getOperand(1));
    if (!SeenMin.count(ID)) {
      ConstantInt *V = mdconst::extract<ConstantInt>(Op->getOperand(2));
      Metadata *FlagOps[] = {
          Op->getOperand(0), ID,
          ConstantAsMetadata::get(ConstantInt::get(V->getType(), 0))};
      DstModFlags->setOperand(Idx, MDNode::get(DstM.getContext(), FlagOps));
    }
  }

  // Check all of the requirements.
  for (unsigned I = 0, E = Requirements.size(); I != E; ++I) {
    MDNode *Requirement = Requirements[I];
    MDString *Flag = cast<MDString>(Requirement->getOperand(0));
    Metadata *ReqValue = Requirement->getOperand(1);

    MDNode *Op = Flags[Flag].first;
    if (!Op || Op->getOperand(2) != ReqValue)
      return stringErr("linking module flags '" + Flag->getString() +
                       "': does not have the required value");
  }
  return Error::success();
}

/// Return InlineAsm adjusted with target-specific directives if required.
/// For ARM and Thumb, we have to add directives to select the appropriate ISA
/// to support mixing module-level inline assembly from ARM and Thumb modules.
static std::string adjustInlineAsm(const std::string &InlineAsm,
                                   const Triple &Triple) {
  if (Triple.getArch() == Triple::thumb || Triple.getArch() == Triple::thumbeb)
    return ".text\n.balign 2\n.thumb\n" + InlineAsm;
  if (Triple.getArch() == Triple::arm || Triple.getArch() == Triple::armeb)
    return ".text\n.balign 4\n.arm\n" + InlineAsm;
  return InlineAsm;
}

Error IRLinker::run() {
  // Ensure metadata materialized before value mapping.
  if (SrcM->getMaterializer())
    if (Error Err = SrcM->getMaterializer()->materializeMetadata())
      return Err;

  // Inherit the target data from the source module if the destination module
  // doesn't have one already.
  if (DstM.getDataLayout().isDefault())
    DstM.setDataLayout(SrcM->getDataLayout());

  // Copy the target triple from the source to dest if the dest's is empty.
  if (DstM.getTargetTriple().empty() && !SrcM->getTargetTriple().empty())
    DstM.setTargetTriple(SrcM->getTargetTriple());

  Triple SrcTriple(SrcM->getTargetTriple()), DstTriple(DstM.getTargetTriple());

  // During CUDA compilation we have to link with the bitcode supplied with
  // CUDA. libdevice bitcode either has no data layout set (pre-CUDA-11), or has
  // the layout that is different from the one used by LLVM/clang (it does not
  // include i128). Issuing a warning is not very helpful as there's not much
  // the user can do about it.
  bool EnableDLWarning = true;
  bool EnableTripleWarning = true;
  if (SrcTriple.isNVPTX() && DstTriple.isNVPTX()) {
    std::string ModuleId = SrcM->getModuleIdentifier();
    StringRef FileName = llvm::sys::path::filename(ModuleId);
    bool SrcIsLibDevice =
        FileName.startswith("libdevice") && FileName.endswith(".10.bc");
    bool SrcHasLibDeviceDL =
        (SrcM->getDataLayoutStr().empty() ||
         SrcM->getDataLayoutStr() == "e-i64:64-v16:16-v32:32-n16:32:64");
    // libdevice bitcode uses nvptx64-nvidia-gpulibs or just
    // 'nvptx-unknown-unknown' triple (before CUDA-10.x) and is compatible with
    // all NVPTX variants.
    bool SrcHasLibDeviceTriple = (SrcTriple.getVendor() == Triple::NVIDIA &&
                                  SrcTriple.getOSName() == "gpulibs") ||
                                 (SrcTriple.getVendorName() == "unknown" &&
                                  SrcTriple.getOSName() == "unknown");
    EnableTripleWarning = !(SrcIsLibDevice && SrcHasLibDeviceTriple);
    EnableDLWarning = !(SrcIsLibDevice && SrcHasLibDeviceDL);
  }

  if (EnableDLWarning && (SrcM->getDataLayout() != DstM.getDataLayout())) {
    emitWarning("Linking two modules of different data layouts: '" +
                SrcM->getModuleIdentifier() + "' is '" +
                SrcM->getDataLayoutStr() + "' whereas '" +
                DstM.getModuleIdentifier() + "' is '" +
                DstM.getDataLayoutStr() + "'\n");
  }

  if (EnableTripleWarning && !SrcM->getTargetTriple().empty() &&
      !SrcTriple.isCompatibleWith(DstTriple))
    emitWarning("Linking two modules of different target triples: '" +
                SrcM->getModuleIdentifier() + "' is '" +
                SrcM->getTargetTriple() + "' whereas '" +
                DstM.getModuleIdentifier() + "' is '" + DstM.getTargetTriple() +
                "'\n");

  DstM.setTargetTriple(SrcTriple.merge(DstTriple));

  // Loop over all of the linked values to compute type mappings.
  computeTypeMapping();

  std::reverse(Worklist.begin(), Worklist.end());
  while (!Worklist.empty()) {
    GlobalValue *GV = Worklist.back();
    Worklist.pop_back();

    // Already mapped.
    if (ValueMap.find(GV) != ValueMap.end() ||
        IndirectSymbolValueMap.find(GV) != IndirectSymbolValueMap.end())
      continue;

    assert(!GV->isDeclaration());
    Mapper.mapValue(*GV);
    if (FoundError)
      return std::move(*FoundError);
    flushRAUWWorklist();
  }

  // Note that we are done linking global value bodies. This prevents
  // metadata linking from creating new references.
  DoneLinkingBodies = true;
  Mapper.addFlags(RF_NullMapMissingGlobalValues);

#if INTEL_CUSTOMIZATION
#if INTEL_FEATURE_SW_DTRANS
  for (auto *GO : DTransMDRemapWorklist) {
    // Remap the metadata, if the GlobalObject is still a declaration. If it was
    // changed to a definition, then the remapping has already been done.
    if (GO->isDeclaration()) {
      // Function declarations use the tag "intel.dtrans.func.type"
      // Global variables use the tag "intel_dtrans_type"
      const char *MDName = "intel.dtrans.func.type";
      auto *MD = GO->getMetadata(MDName);
      if (!MD) {
        MDName = "intel_dtrans_type";
        MD = GO->getMetadata(MDName);
      }
      if (MD) {
        // Update the metadata tag. Remap it, if one has not been created yet.
        auto Mapping = ValueMap.getMappedMD(MD);
        if (Mapping)
          GO->setMetadata(MDName, cast<MDNode>(Mapping.getValue()));
        else
          GO->setMetadata(MDName, Mapper.mapMDNode(*MD));
      }
    }
  }
  DTransMDRemapWorklist.clear();
#endif // INTEL_FEATURE_SW_DTRANS
#endif // INTEL_CUSTOMIZATION

  // Remap all of the named MDNodes in Src into the DstM module. We do this
  // after linking GlobalValues so that MDNodes that reference GlobalValues
  // are properly remapped.
  linkNamedMDNodes();

  if (!IsPerformingImport && !SrcM->getModuleInlineAsm().empty()) {
    // Append the module inline asm string.
    DstM.appendModuleInlineAsm(adjustInlineAsm(SrcM->getModuleInlineAsm(),
                                               SrcTriple));
  } else if (IsPerformingImport) {
    // Import any symver directives for symbols in DstM.
    ModuleSymbolTable::CollectAsmSymvers(*SrcM,
                                         [&](StringRef Name, StringRef Alias) {
      if (DstM.getNamedValue(Name)) {
        SmallString<256> S(".symver ");
        S += Name;
        S += ", ";
        S += Alias;
        DstM.appendModuleInlineAsm(S);
      }
    });
  }

  // Reorder the globals just added to the destination module to match their
  // original order in the source module.
  Module::GlobalListType &Globals = DstM.getGlobalList();
  for (GlobalVariable &GV : SrcM->globals()) {
    if (GV.hasAppendingLinkage())
      continue;
    Value *NewValue = Mapper.mapValue(GV);
    if (NewValue) {
      auto *NewGV = dyn_cast<GlobalVariable>(NewValue->stripPointerCasts());
      if (NewGV)
        Globals.splice(Globals.end(), Globals, NewGV->getIterator());
    }
  }

#if INTEL_CUSTOMIZATION
#if INTEL_FEATURE_SW_DTRANS
  TypeMap.updateDTransTypeManager();

  // Call the full verification that checks for repeated types and the metadata
  if (EnableVerify)
    verifyDestinationModule();

  // Call the simple verification that checks the repeated types only
  if (EnableQuickVerify)
    quickVerifyDestinationModule();

  DEBUG_WITH_TYPE(DEBUG_DTRANS_TYPES,
      dbgs() << "\n-------------------------------------------------------\n");

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  if (TraceDTransMetadataLoss)
    dbgs() << "\n-------------------------------------------------------\n";
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

#endif // INTEL_FEATURE_SW_DTRANS
#endif // INTEL_CUSTOMIZATION

  // Merge the module flags into the DstM module.
  return linkModuleFlagsMetadata();
}

#if INTEL_CUSTOMIZATION
// Provide name of a struct.
IRMover::StructTypeKeyInfo::KeyTy::KeyTy(ArrayRef<Type *> E, bool P,
                                         StringRef Name)
    : ETypes(E), IsPacked(P), Name(Name) {}

// Provide name of a struct.
IRMover::StructTypeKeyInfo::KeyTy::KeyTy(const StructType *ST)
    : ETypes(ST->elements()), IsPacked(ST->isPacked()),
      Name(getStructName(ST)) {}

// Account for name of a struct.
bool IRMover::StructTypeKeyInfo::KeyTy::operator==(const KeyTy &That) const {
  return IsPacked == That.IsPacked && ETypes == That.ETypes &&
         Name == That.Name;
}
#endif // INTEL_CUSTOMIZATION

bool IRMover::StructTypeKeyInfo::KeyTy::operator!=(const KeyTy &That) const {
  return !this->operator==(That);
}

StructType *IRMover::StructTypeKeyInfo::getEmptyKey() {
  return DenseMapInfo<StructType *>::getEmptyKey();
}

StructType *IRMover::StructTypeKeyInfo::getTombstoneKey() {
  return DenseMapInfo<StructType *>::getTombstoneKey();
}

unsigned IRMover::StructTypeKeyInfo::getHashValue(const KeyTy &Key) {
  return hash_combine(hash_combine_range(Key.ETypes.begin(), Key.ETypes.end()),
                      Key.IsPacked, Key.Name); // INTEL
}

unsigned IRMover::StructTypeKeyInfo::getHashValue(const StructType *ST) {
  return getHashValue(KeyTy(ST));
}

bool IRMover::StructTypeKeyInfo::isEqual(const KeyTy &LHS,
                                         const StructType *RHS) {
  if (RHS == getEmptyKey() || RHS == getTombstoneKey())
    return false;
  return LHS == KeyTy(RHS);
}

bool IRMover::StructTypeKeyInfo::isEqual(const StructType *LHS,
                                         const StructType *RHS) {
  if (RHS == getEmptyKey() || RHS == getTombstoneKey())
    return LHS == RHS;
  return KeyTy(LHS) == KeyTy(RHS);
}

void IRMover::IdentifiedStructTypeSet::addNonOpaque(StructType *Ty) {
  assert(!Ty->isOpaque());
  NonOpaqueStructTypes.insert(Ty);
}

void IRMover::IdentifiedStructTypeSet::switchToNonOpaque(StructType *Ty) {
  assert(!Ty->isOpaque());
  NonOpaqueStructTypes.insert(Ty);
  bool Removed = OpaqueStructTypes.erase(Ty);
  (void)Removed;
  assert(Removed);
}

void IRMover::IdentifiedStructTypeSet::addOpaque(StructType *Ty) {
  assert(Ty->isOpaque());
  OpaqueStructTypes.insert(Ty);
}

StructType *IRMover::IdentifiedStructTypeSet::findNonOpaque(     // INTEL
    ArrayRef<Type *> ETypes, bool IsPacked, StringRef Name) {    // INTEL
  IRMover::StructTypeKeyInfo::KeyTy Key(ETypes, IsPacked, Name); // INTEL
  auto I = NonOpaqueStructTypes.find_as(Key);
  return I == NonOpaqueStructTypes.end() ? nullptr : *I;
}

bool IRMover::IdentifiedStructTypeSet::hasType(StructType *Ty) {
  if (Ty->isOpaque())
    return OpaqueStructTypes.count(Ty);
  auto I = NonOpaqueStructTypes.find(Ty);
  return I == NonOpaqueStructTypes.end() ? false : *I == Ty;
}

#if INTEL_CUSTOMIZATION
#if INTEL_FEATURE_SW_DTRANS
IRMover::IRMover(Module &M) : Composite(M), TM(M.getContext()) {
  TypeFinder StructTypes;
  StructTypes.run(M, /* OnlyNamed */ false);
  for (StructType *Ty : StructTypes) {
    if (Ty->isOpaque())
      IdentifiedStructTypes.addOpaque(Ty);
    else
      IdentifiedStructTypes.addNonOpaque(Ty);
  }
  // Self-map metadata in the destination module. This is needed when
  // DebugTypeODRUniquing is enabled on the LLVMContext, since metadata in the
  // destination module may be reached from the source module.
  for (auto *MD : StructTypes.getVisitedMetadata()) {
    SharedMDs[MD].reset(const_cast<MDNode *>(MD));
  }
}
#else // INTEL_FEATURE_SW_DTRANS
IRMover::IRMover(Module &M) : Composite(M) {
  TypeFinder StructTypes;
  StructTypes.run(M, /* OnlyNamed */ false);
  for (StructType *Ty : StructTypes) {
    if (Ty->isOpaque())
      IdentifiedStructTypes.addOpaque(Ty);
    else
      IdentifiedStructTypes.addNonOpaque(Ty);
  }
  // Self-map metadatas in the destination module. This is needed when
  // DebugTypeODRUniquing is enabled on the LLVMContext, since metadata in the
  // destination module may be reached from the source module.
  for (auto *MD : StructTypes.getVisitedMetadata()) {
    SharedMDs[MD].reset(const_cast<MDNode *>(MD));
  }
}
#endif // INTEL_FEATURE_SW_DTRANS
#endif //INTEL_CUSTOMIZATION

Error IRMover::move(std::unique_ptr<Module> Src,
                    ArrayRef<GlobalValue *> ValuesToLink,
                    LazyCallback AddLazyFor, bool IsPerformingImport) {
#if INTEL_CUSTOMIZATION
#if INTEL_FEATURE_SW_DTRANS
  IRLinker TheIRLinker(Composite, SharedMDs, IdentifiedStructTypes,
                       std::move(Src), ValuesToLink, std::move(AddLazyFor),
                       IsPerformingImport, &TM);
#else // INTEL_FEATURE_SW_DTRANS
  IRLinker TheIRLinker(Composite, SharedMDs, IdentifiedStructTypes,
                       std::move(Src), ValuesToLink, std::move(AddLazyFor),
                       IsPerformingImport);
#endif // INTEL_FEATURE_SW_DTRANS
#endif //INTEL_CUSTOMIZATION
  Error E = TheIRLinker.run();
  Composite.dropTriviallyDeadConstantArrays();
  return E;
}
