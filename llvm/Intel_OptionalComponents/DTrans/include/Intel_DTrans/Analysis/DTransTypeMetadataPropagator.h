//===----DTransTypeMetadataPropagator.h - DTrans metadata propagation------===//
//
// Copyright (C) 2022-2022 Intel Corporation. All rights reserved.
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

} // end namespace dtransOP
} // end namespace llvm

#endif // INTEL_DTRANS_ANALYSIS_DTRANSTYPEMETADATAPROPAGATOR_H
