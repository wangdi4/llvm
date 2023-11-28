//===----DTransTypeMetadataPropagator.h - DTrans metadata propagation------===//
//
// Copyright (C) 2022 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
//
// This file defines the DTransTypeMetadataPropagator class, which is used to
// propagate DTrans type metadata.
//
//===----------------------------------------------------------------------===//

#if !INTEL_FEATURE_SW_DTRANS
#error DTransTypeMetadataPropagator.h include in an non-INTEL_FEATURE_SW_DTRANS build.
#endif

#ifndef INTEL_DTRANS_ANALYSIS_DTRANSTYPEMETADATAPROPAGATOR_H
#define INTEL_DTRANS_ANALYSIS_DTRANSTYPEMETADATAPROPAGATOR_H

#include "llvm/ADT/MapVector.h"

namespace llvm {
class AllocaInst;
class GlobalVariable;
class DataLayout;
class MDNode;
class Function;
class Module;
class StructType;
class Type;
class Value;

namespace dtransOP {

//
// This class provides light-weight support to propagate DTrans type metadata
// annotations to a new IR entity when the metadata needs to be modified for
// propagation.
// For example, given:
//   %struct.test = type { i32, i64*, %struct.foo*, i64 }
//   %a = alloca %struct.test
//
// A transformation that creates an allocation of a literal structure using a
// consecutive subset of fields, such as:
//   %a.sub = alloca { %struct.foo*, i64 }
//
// This class will create a new DTrans metadata type for the literal structure
// that describes the fields extracted to be placed on the alloca.
//
// This class will be extended for other cases where propagation is needed as
// necessary.
//
// Note: The intent is to avoid the need for other parts of the compiler
// to be familiar with the detailed implementation of the DTrans type metadata.
// By light-weight, this class differs from the TypeMetadataReader class in that
// it does NOT parse all the structure metadata to operate.
//
class DTransTypeMetadataPropagator {
public:
  // Try to set the DTrans metadata on 'NewAI', if one is needed, and the
  // type can be determined based on taking 'Size' bytes, beginning at the
  // specified 'Offset' of the type that was in 'OrigAI'.
  //
  // For example:
  //   NewAI : alloca { %struct.foo*, i64 }
  //   OrigAI: alloca %struct.test
  //   Offset: 16
  //   Size  : 16
  // would create Literal structure descriptor using the types that describe the
  // fields taken from %struct.test. In some cases, no metadata will be added
  // to 'NewAI' by this rouine.
  // - There may be no need for metadata, such as when the resulting alloca
  //   does not contain any pointer type references.
  // - It may not be possible to set a metadata type because the extraction size
  //   does not line up with the field boundaries.
  //
  void updateDTransMetadata(AllocaInst *NewAI, AllocaInst &OrigAI,
                            uint64_t Offset, uint64_t Size);

  // Helper to copy DTrans type metadata attachment from 'SrcValue' to
  // 'DestValue', if it exists.
  static void copyDTransMetadata(Value *DestValue, const Value *SrcValue);

  static void setDevirtVarDTransMetadata(GlobalVariable *OldGV,
                                         GlobalVariable *NewGV,
                                         uint64_t BeforeBytesSize,
                                         uint64_t AfterBytesSize);

  static void setGlobUsedVarDTransMetadata(GlobalVariable *OldGV,
                                           GlobalVariable *NewGV,
                                           uint64_t NewArrSize);

  static void setGlobAppendingVarDTransMetadata(const GlobalVariable *SrcGV,
                                                GlobalVariable *DstGV,
                                                GlobalVariable *NewGV,
                                                uint64_t NewArrSize);

  static void setNewGlobVarArrayDTransMetadata(GlobalVariable *GV);

private:
  void initialize(Module &M);

  MDNode *getDTransMDNode(llvm::StructType *Ty) const;

  bool identifyFieldRange(const DataLayout &DL, llvm::Type *Ty,
                          MDNode **StructMD, uint64_t Offset, uint64_t Size,
                          unsigned *ActualBeginFieldNum,
                          unsigned *ActualEndFieldNum, const AllocaInst &OrigAI,
                          const AllocaInst &NewAI);

  bool Initialized = false;
  bool DTransTypeMetadataAvailable = false;
  MapVector<llvm::StructType *, MDNode *> StructToMDDescriptor;
};

// Helper class that can retrieve the metadata node that describes a structure
// field type within the metadata node that describes a structure type. This is
// used during metadata propagation for argument promotion where a function is
// changed to take a field of a structure which is passed to a function instead
// of a pointer to the complete structure. This is meant to be used as a
// lightweight method of propagating the metadata without having to parse all
// the DTrans type metadata which requires having to build all the DTransTypes
// objects.
class DTransMDFieldNodeRetriever {
public:
  MDNode *GetNodeForField(Function *F, llvm::StructType *STy,
                          uint32_t FieldNum);

private:
  void ParseAllTypesTag(Module *M);

  // Module that this object has parse the named DTrans type metadata node for.
  Module *InitializedM = nullptr;
  MapVector<StructType *, MDNode *> TypeToMDDescriptor;
};

// Helper class used to set up the DTrans metadata and attributes on a function
// during the argument promotion transformation when that transformation changes
// a function parameter from being a pointer to a structure to being a pointer
// to one of fields within the structure instead.
class DTransTypeMDArgPromoPropagator {
public:
  // This class stores information about the current function being processed by
  // argument promotion, and must be initialized at the start of each new
  // function processed.
  void initialize(Function *F);

  // Checks whether the original argument was a pointer to a structure type,
  // based on the DTrans type metadata, and checks whether the new argument is a
  // field member of a structure that will need DTrans type metadata.
  void addArg(llvm::Type *NewArgTy, uint32_t OrigArgNo, uint32_t NewArgNo,
              uint32_t ByteOffset);

  // Updates the DTrans type metadata on the new function based on the
  // information collected by the calls to 'addArg'.
  void setMDAttributes(Function *NF);

private:
  DTransMDFieldNodeRetriever Retriever;

  // These members must be reinitialized each time a new IR function is
  // processed.
  struct PerFunction {
    Function *F;

    // The existing !intel.dtrans.func.type tag, if one exists.
    MDNode *MDTypeListNode;

    // List of parameter numbers and the MDNode* to add for DTrans type metadata
    // attributes once the new function signature is created.
    SmallVector<std::pair<uint32_t, MDNode *>, 8> DTransMDAttrs;
  } PerFunction;
};

} // end namespace dtransOP
} // end namespace llvm

#endif // INTEL_DTRANS_ANALYSIS_DTRANSTYPEMETADATAPROPAGATOR_H
