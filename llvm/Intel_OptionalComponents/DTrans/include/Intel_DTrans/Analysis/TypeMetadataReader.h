//===-----------TypeMetadataReader.h - Decode metadata annotations---------===//
//
// Copyright (C) 2019-2022 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//

// This file defines the class used to read metadata used for representing
// type information for DTrans. The metadata is to be produced by the
// compiler front-end to assist DTrans to recover pointer types when using
// opaque pointers.

#if !INTEL_FEATURE_SW_DTRANS
#error TypeMetadataReader.h include in an non-INTEL_FEATURE_SW_DTRANS build.
#endif

#ifndef INTEL_DTRANS_ANALYSIS_TYPEMETADATAREADER_H
#define INTEL_DTRANS_ANALYSIS_TYPEMETADATAREADER_H

#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/MapVector.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/PassManager.h"

namespace llvm {
class NamedMDNode;

namespace dtransOP {

class DTransType;
class DTransFunctionType;
class DTransStructType;
class DTransTypeManager;
class TypeMetadataTester;

// This class parses metadata descriptions of types for DTrans. Refer to the
// DTrans .rst documentation files for a description of the metadata format.
// There are two types of metadata that are handled:
//
// 1) Descriptions of the field members of structure types.
// 2) Annotations for certain instructions that help with DTrans type recovery.
// These annotations use different metadata tags, but share a common encoding.
//
// All the types created will be owned and managed by the DTransTypeManager
// object provided to the constructor of this class.
//
// Type 1 information is processed by the function initialize(). This
// method must be called when this class is instantiated to create all the
// structure types that will be needed, before calls to decodeMDNode. This is to
// simplify the case where decoding a structure requires another structure to be
// generated.
//
// Type 2 information can be decoded by calls to decodeMDNode.
class TypeMetadataReader {
  // Allow the TypeMetadataTester direct access to the decodeMDNode. All other
  // clients should not know about the specific metadata tags, and therefore
  // need to use the getDTransTypeFromMD interface.
  friend class TypeMetadataTester;

public:
  // Return 'true' if there is a NamedMDNode that contains the list of metadata
  // nodes used to describe all the structure types.
  static bool hasDTransTypesMetadata(Module &M);

  // Get the NamedMDNode for the module that contains the list of metadata nodes
  // used to describe all the structure types.
  static NamedMDNode* getDTransTypesMetadata(Module &M);

  // Walk the '!intel.dtrans.types', if it exists, populating 'Mapping' with the
  // StructType to MDNode that describes the structure. A MapVector is used to
  // help with testing to maintain an ordering of the items for checking the
  // output in a deterministic manner. However, the order may differ than the
  // original metadata node because opaque structure types will be placed into
  // the vector after all the non-opaque structures.
  //
  // When IncludeOpaque = false, opaque structure types will not be placed in
  // the map. Otherwise, opaque structure types will be placed in the map if
  // there is not a non-opaque structure available for it.
  //
  // Returns 'null' if '!intel.dtrans.types' does not exist, otherwise return
  // the metadata node.
  static NamedMDNode *
  mapStructsToMDNodes(Module &M, MapVector<StructType *, MDNode *> &Mapping,
                      bool IncludeOpaque);

  // Get the DTrans metadata node for a specific value. This can be used by
  // the transformations to be able to update the metadata when changing types.
  static MDNode *getDTransMDNode(const Value &V);

  TypeMetadataReader(DTransTypeManager &TM) : TM(TM) {}

  // This method should be called first to walk the named metadata node,
  // "dtrans_types", to identify all the original field types of the structure
  // types. This will return 'true' if metadata was available, and all the
  // structure types were able to be resolved.
  //
  // If StrictCheck is true, then the process for checking the metadata will
  // assert if any metadata is lost. Else, it will only assert for a structure
  // where has an opaque pointer type and the metadata is lost.
  bool initialize(Module &M, bool StrictCheck = true);

  // If the Value has DTrans type metadata that can be decoded, return the
  // DTransType, otherwise nullptr.
  DTransType *getDTransTypeFromMD(const Value *V);
private:
  // This method returns a DTransType* by decoding the information in the
  // metadata node. Returns nullptr, if an error occurs during decoding.
  DTransType *decodeMDNode(MDNode *MD);

  // Internal methods to help with decoding.
  DTransType *decodeMDFunctionNode(MDNode *MD);
  DTransType *decodeMDVoidNode(MDNode *MD);
  DTransType *decodeMDLiteralStructNode(MDNode *MD);
  DTransType *decodeMDStructRefNode(MDNode *MD);
  DTransType *decodeMDArrayNode(MDNode *MD);
  DTransType *decodeMDVectorNode(MDNode *MD);

  // Lookup a value from the FunctionToDTransTypeMap table.
  DTransFunctionType *getDTransType(const Function *F) const;

  // Build of cache of DTransFunctionType objects to map Functions to a
  // DTransFunctionType for each function that has DTrans metadata tags.
  void buildFunctionTypeTable(Module &M);

  // Decode the metadata 'MDTypeListNode' attached to Function 'F'.
  DTransFunctionType *decodeDTransFuncType(Function &F,
                                           const MDNode &MDTypeListNode);

  // Utility methods
  DTransType *createPointerToLevel(DTransType *DTTy, unsigned PtrLevel);
  void cacheMDDecoding(MDNode *MD, DTransType *DTTy);

  DTransStructType *constructDTransStructType(MDNode *MD);
  llvm::StructType *populateDTransStructType(Module &M, MDNode *MD,
                                             DTransStructType *DTStTy);
  void populateDTransStructTypeFromLLVMType(llvm::StructType *Sty,
                                            DTransStructType *DSTy);
  bool validateMDFieldType(DTransType *DTy, llvm::Type *LTy);

  DTransTypeManager &TM;

  // Cache of metadata nodes that have been resolved to types.
  // The DTransType* objects are owned by the DTransTypeManager,
  // and should not be destroyed by destruction of this class. This also
  // means that the lifetime of the DTransTypeManager must exceed the lifetime
  // of calls to this class.
  DenseMap<MDNode *, DTransType *> MDToDTransTypeMap;

  // Map of functions with DTrans metadata tags to a DTransFunctionType.
  DenseMap<Function *, DTransFunctionType *> FunctionToDTransTypeMap;
};

#if !INTEL_PRODUCT_RELEASE
// This pass is for testing the TypeMetadataReader class. This pass will not be
// part of the compiler pipeline.
class DTransTypeMetadataReaderTestPass
    : public PassInfoMixin<DTransTypeMetadataReaderTestPass> {

public:
  PreservedAnalyses run(Module &M, ModuleAnalysisManager &MAM);
};

#endif // !INTEL_PRODUCT_RELEASE
} // end namespace dtransOP

#if !INTEL_PRODUCT_RELEASE
ModulePass *createDTransMetadataReaderTestWrapperPass();
#endif // !INTEL_PRODUCT_RELEASE

} // end namespace llvm

#endif // INTEL_DTRANS_ANALYSIS_TYPEMETADATAREADER_H
