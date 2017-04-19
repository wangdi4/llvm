//===--- intelovls-test.h - -------------------------------------------*- C++
//-*-===//
//
// Copyright (C) 2016 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
/// \file
/// \brief Common declarations for intelovls-test.h
//===----------------------------------------------------------------------===//
#ifndef LLVM_TOOLS_INTELOVLS_TEST_H
#define LLVM_TOOLS_INTELOVLS_TEST_H

#include "llvm/Analysis/Intel_OptVLS.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Type.h"
#include <iostream>

// This line is commented off to avoid building target information during
// default build. Without target information getCost() and getSequence() will
// not be called but getGroups() will. Uncomment the following line to enable
// cost analysis, so as to getSequence() and getGroups() to be called.
//#define OVLSTESTCLIENT

using namespace llvm;
using namespace std;

typedef class LLVMContext OVLSContext;

class ClientMemref : public OVLSMemref {
public:
  // Constructor for Indexed Accesses.
  ClientMemref(char MemrefId, int Distance, Type *ElemType,
               unsigned NumElements, // VectorType
               OVLSAccessType AType, char IdxId, VectorType *IdxType)
      : OVLSMemref(VLSK_ClientMemref,
                   OVLSType(ElemType->getPrimitiveSizeInBits(), NumElements),
                   AType) {
    MId = MemrefId;
    Dist = Distance;
    IndexId = IdxId;
    IndexType = IdxType;
    DataType = VectorType::get(ElemType, NumElements);
    ConstVStride = false;
    VecStride = 0;
  }
  // Constructor for Strided Accesses with unknown strides.
  ClientMemref(char MemrefId, int Distance, Type *ElemType,
               unsigned NumElements, // VectorType
               OVLSAccessType AType, bool CVStride, char VectorStrideId)
      : OVLSMemref(VLSK_ClientMemref,
                   OVLSType(ElemType->getPrimitiveSizeInBits(), NumElements),
                   AType) {
    MId = MemrefId;
    Dist = Distance;
    ConstVStride = CVStride;
    VecStride = 0;
    VsId = VectorStrideId;
    DataType = VectorType::get(ElemType, NumElements);
  }
  // Constructor for Strided Accesses with constant strides.
  ClientMemref(char MemrefId, int Distance, Type *ElemType,
               unsigned NumElements, // VectorType
               OVLSAccessType AType, bool CVStride, int VStride)
      : OVLSMemref(VLSK_ClientMemref,
                   OVLSType(ElemType->getPrimitiveSizeInBits(), NumElements),
                   AType) {
    MId = MemrefId;
    Dist = Distance;
    ConstVStride = CVStride;
    VecStride = VStride;
    VsId = '\0';
    DataType = VectorType::get(ElemType, NumElements);
  }

  static bool classof(const OVLSMemref *Mrf) {
    return Mrf->getKind() == VLSK_ClientMemref;
  }

  int getDistance() const { return Dist; }
  char getMemrefId() const { return MId; }
  int getIndexDistance() const { return IndexDist; }
  char getIndexId() const { return IndexId; }

  // Two memrefs are considered to have same index vectors if they
  // have the same index-id and same index-type.
  bool haveSameIndexVector(const ClientMemref &Mrf);

  bool hasConstVStride() const { return ConstVStride; }
  bool hasVarVStride() const { return VsId != '\0'; }
  int getConstVectorStride() const { return VecStride; }
  char getVarVectorStride() const { return VsId; }

  bool haveSameVectorStride(const ClientMemref &Mrf);

  bool isAConstDistanceFrom(const OVLSMemref &Mrf, int64_t *Distance);

  bool canMoveTo(const OVLSMemref &MemRef) { return true; }
  bool hasAConstStride(int64_t *Stride) const {
    if (ConstVStride) {
      *Stride = VecStride;
      return true;
    }
    return false;
  }

  // Checks whether Mrf and this have the same number of vector elements
  bool haveSameNumElements(const OVLSMemref &Mrf) {
    const ClientMemref *CLMrf = (const ClientMemref *)&Mrf;
    return (DataType->getNumElements() == (CLMrf->DataType)->getNumElements());
  }

  // TODO: Return the location of this in the code. Should reflect the relative
  // ordering between all Memrefs sent to the VLS engine by this client.
  // FIXME: For now just returning the MemrefID.
  unsigned getLocation() const {
    return getMemrefId(); // FIXME
  }

private:
  char MId;
  VectorType *DataType; // Data type for the memref
  int64_t Dist;         // Distance between two memrefs in bytes.
  char IndexId;
  int IndexDist;

  bool ConstVStride;
  int VecStride; // Represents an uniform distance(in bytes) between the
                 // elements of a strided access, can be scalar positive
                 // or negative integer. It will be ignored for indexed
                 // access, must be set to zero. For consecutive elements
                 // vector stride equals [+/-] the element_size in bytes.
                 // When VecStride equals elem_size it's a unit-strided
                 // access otherwise it's a non-unit-strided access.
  char VsId;     // Represents variable vector stride

  VectorType *IndexType; // There has to be a valid index type for an indexed
  // access. Number of elements in IndexType needs to match
  // the number of elements in VecType. For strided access
  // this will be ignored but must be set to <Undef x 0>.
};

#endif
