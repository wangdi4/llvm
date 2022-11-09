//===--- intelovls-test.h - -------------------------------------------*- C++
//-*-===//
//
// Copyright (C) 2016-2022 Intel Corporation. All rights reserved.
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

class ClientMemref : public OVLSMemref {
protected:
  friend class llvm::OVLSContext;
  ClientMemref(OVLSContext &Context, char MemrefId, int Dist, Type *ElemType,
               unsigned NumElements, OVLSAccessKind AKind, char IdxId,
               VectorType *IdxType, bool ConstVStride, char VsId, int VecStride)
      : OVLSMemref(Context, VLSK_ClientMemref,
                   OVLSType(ElemType->getPrimitiveSizeInBits(), NumElements),
                   AKind),
        MId(MemrefId), DataType(FixedVectorType::get(ElemType, NumElements)),
        Dist(Dist), IndexId(IdxId), IndexDist(), ConstVStride(ConstVStride),
        VecStride(VecStride), VsId(VsId), IndexType(IdxType) {}

public:
  // Constructor for Indexed Accesses.
  static ClientMemref *createIndexed(OVLSContext &Context, char MemrefId,
                                     int Distance, Type *ElemType,
                                     unsigned NumElements, // VectorType
                                     OVLSAccessKind AKind, char IdxId,
                                     VectorType *IdxType) {
    return Context.create<ClientMemref>(
        MemrefId, Distance, ElemType, NumElements, AKind, IdxId, IdxType,
        /*ConstVStride:*/ false, /*VsID:*/ 0, /*VecStride:*/ 0);
  }

  // Constructor for Strided Accesses with unknown strides.
  static ClientMemref *createUnknownStride(OVLSContext &Context, char MemrefId,
                                           int Distance, Type *ElemType,
                                           unsigned NumElements, // VectorType
                                           OVLSAccessKind AKind, bool CVStride,
                                           char VectorStrideId) {
    return Context.create<ClientMemref>(
        MemrefId, Distance, ElemType, NumElements, AKind, /*IdxId:*/ 0,
        /*IdxType:*/ nullptr, CVStride, /*VsID:*/ 0, /*VecStride:*/ 0);
  }

  // Constructor for Strided Accesses with constant strides.
  static ClientMemref *createConstStride(OVLSContext &Context, char MemrefId,
                                         int Distance, Type *ElemType,
                                         unsigned NumElements, // VectorType
                                         OVLSAccessKind AKind, bool CVStride,
                                         int VStride) {
    return Context.create<ClientMemref>(
        MemrefId, Distance, ElemType, NumElements, AKind,
        /*IdxId:*/ 0, /*IdxType:*/ nullptr, CVStride, /*VsId:*/ 0, VStride);
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
  bool haveSameIndexVector(const ClientMemref &Mrf) const;

  bool hasConstVStride() const { return ConstVStride; }
  bool hasVarVStride() const { return VsId != '\0'; }
  int getConstVectorStride() const { return VecStride; }
  char getVarVectorStride() const { return VsId; }

  bool haveSameVectorStride(const ClientMemref &Mrf) const;

  Optional<int64_t> getConstDistanceFrom(const OVLSMemref &Mrf) const override;

  bool canMoveTo(const OVLSMemref &MemRef) override { return true; }

  Optional<int64_t> getConstStride() const override {
    if (ConstVStride)
      return VecStride;
    return None;
  }

  bool dominates(const OVLSMemref &Mrf) const override { return true; }
  bool postDominates(const OVLSMemref &Mrf) const override { return true; }

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
