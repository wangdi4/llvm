//===-----------TypeMetadataReader.h - Decode metadata annotations---------===//
//
// Copyright (C) 2019-2020 Intel Corporation. All rights reserved.
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

#if !INTEL_INCLUDE_DTRANS
#error TypeMetadataReader.h include in an non-INTEL_INCLUDE_DTRANS build.
#endif

#ifndef INTEL_DTRANS_ANALYSIS_TYPEMETADATAREADER_H
#define INTEL_DTRANS_ANALYSIS_TYPEMETADATAREADER_H

#include "llvm/ADT/DenseMap.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/PassManager.h"


namespace llvm {
namespace dtrans {

class DTransType;
class DTransStructType;
class DTransTypeManager;

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
public:
  TypeMetadataReader(DTransTypeManager &TM) : TM(TM) {}

  // This method should be called first to walk the named metadata node,
  // "dtrans_types", to identify all the original field types of the structure
  // types. This will return 'true' if metadata was available, and all the
  // structure types were able to be resolved.
  bool initialize(Module &M);

  // This method returns a DTransType* by decoding the information in the
  // metadata node. Returns nullptr, if an error occurs during decoding.
  DTransType *decodeMDNode(MDNode *MD);

private:
  // Internal methods to help with decoding.
  DTransType *decodeMDFunctionNode(MDNode *MD);
  DTransType *decodeMDVoidNode(MDNode *MD);
  DTransType *decodeMDLiteralStructNode(MDNode *MD);
  DTransType *decodeMDStructRefNode(MDNode *MD);
  DTransType *decodeMDArrayNode(MDNode *MD);
  DTransType *decodeMDVectorNode(MDNode *MD);

  // Utility methods
  DTransType *createPointerToLevel(DTransType *DTTy, unsigned PtrLevel);
  void cacheMDDecoding(MDNode *MD, DTransType *DTTy);

  dtrans::DTransStructType *constructDTransStructType(MDNode *MD);
  llvm::StructType *populateDTransStructType(Module &M, MDNode *MD,
                                             dtrans::DTransStructType *DTStTy);

  DTransTypeManager &TM;

  // Cache of metadata nodes that have been resolved to types.
  // The DTransType* objects are owned by the DTransTypeManager,
  // and should not be destroyed by destruction of this class. This also
  // means that the lifetime of the DTransTypeManager must exceed the lifetime
  // of calls to this class.
  DenseMap<MDNode *, dtrans::DTransType *> MDToDTransTypeMap;
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
} // end namespace dtrans

#if !INTEL_PRODUCT_RELEASE
ModulePass *createDTransMetadataReaderTestWrapperPass();
#endif // !INTEL_PRODUCT_RELEASE

} // end namespace llvm

#endif // INTEL_DTRANS_ANALYSIS_TYPEMETADATAREADER_H
