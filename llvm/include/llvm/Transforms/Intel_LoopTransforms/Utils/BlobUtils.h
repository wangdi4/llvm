//===-------------- BlobUtils.h - Blob utilities --------------*- C++ -*---===//
//
// Copyright (C) 2015-2016 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// This file contains interface for blob utilities.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_UTILS_BLOBUTILS_H
#define LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_UTILS_BLOBUTILS_H

#include "llvm/Support/Compiler.h"
#include <stdint.h>

#include "llvm/Analysis/Intel_LoopAnalysis/HIRParser.h"

#include "llvm/Transforms/Intel_LoopTransforms/Utils/HIRUtils.h"

namespace llvm {

class Type;

namespace loopopt {

class HIRParser;

/// Contains blob related utilities.
class BlobUtils : public HIRUtils {
private:
  /// Do not allow instantiation.
  BlobUtils() = delete;

  friend class HIRParser;
  friend class CanonExprUtils;

  static HIRParser *HIRP;

  static void setHIRParser(HIRParser *HIRPar) {
    assert(HIRPar && " HIR Parser pointer is null!");
    HIRP = HIRPar;
  }

  static HIRParser *getHIRParser() { return HIRP; }

  // Only used by framework to create new temp blobs.
  static BlobTy createBlob(Value *TempVal, unsigned Symbase, bool Insert,
                           unsigned *NewBlobIndex) {
    return getHIRParser()->createBlob(TempVal, Symbase, Insert, NewBlobIndex);
  }

public:
  /// Returns the index of Blob in the blob table. Index range is [1, UINT_MAX].
  /// Returns invalid value if the blob is not present in the table.
  static unsigned findBlob(BlobTy Blob) {
    return getHIRParser()->findBlob(Blob);
  }

  /// Returns symbase corresponding to \p Blob. Asserts if a valid symbase is
  /// not found.
  static unsigned findTempBlobSymbase(BlobTy Blob) {
    return getHIRParser()->findTempBlobSymbase(Blob);
  }

  /// Returns temp blob index corresponding to symbase. Returns InvalidBlobIndex
  /// if blob cannot be found.
  static unsigned findTempBlobIndex(unsigned Symbase) {
    return getHIRParser()->findTempBlobIndex(Symbase);
  }

  /// Finds or inserts temp blob index corresponding to symbase and returns it.
  static unsigned findOrInsertTempBlobIndex(unsigned Symbase) {
    return getHIRParser()->findOrInsertTempBlobIndex(Symbase);
  }

  /// Returns the index of Blob in the blob table. Blob is first inserted, if it
  /// isn't already present in the blob table. Index range is [1, UINT_MAX].
  /// NOTE: New temp blobs can only be inserted by the framework.
  static unsigned findOrInsertBlob(BlobTy Blob) {
    return getHIRParser()->findOrInsertBlob(Blob, InvalidBlobIndex);
  }

  /// Maps blobs in Blobs to their corresponding indices and inserts them in
  /// Indices.
  static void mapBlobsToIndices(const SmallVectorImpl<BlobTy> &Blobs,
                                SmallVectorImpl<unsigned> &Indices) {
    getHIRParser()->mapBlobsToIndices(Blobs, Indices);
  }

  /// Returns blob corresponding to BlobIndex.
  static BlobTy getBlob(unsigned BlobIndex) {
    return getHIRParser()->getBlob(BlobIndex);
  }

  /// Returns symbase corresponding to BlobIndex. Asserts if a valid symbase is
  /// not found.
  static unsigned getTempBlobSymbase(unsigned BlobIndex) {
    return getHIRParser()->getTempBlobSymbase(BlobIndex);
  }

  /// Returns true if this is a valid blob index.
  static bool isBlobIndexValid(unsigned BlobIndex) {
    return getHIRParser()->isBlobIndexValid(BlobIndex);
  }

  /// Prints blob.
  static void printBlob(raw_ostream &OS, BlobTy Blob) {
    getHIRParser()->printBlob(OS, Blob);
  }

  /// Prints scalar corresponding to symbase.
  static void printScalar(raw_ostream &OS, unsigned Symbase) {
    getHIRParser()->printScalar(OS, Symbase);
  }

  /// Checks if the blob is constant or not.
  /// If blob is constant, sets the return value in Val.
  static bool isConstantIntBlob(BlobTy Blob, int64_t *Val) {
    return getHIRParser()->isConstantIntBlob(Blob, Val);
  }

  /// Returns true if Blob is a temp.
  static bool isTempBlob(BlobTy Blob) {
    return getHIRParser()->isTempBlob(Blob);
  }

  /// Returns true if this is a nested blob(SCEV tree with > 1 node).
  static bool isNestedBlob(BlobTy Blob) {
    return getHIRParser()->isNestedBlob(Blob);
  }

  /// Returns true if TempBlob always has a defined at level of zero.
  static bool isGuaranteedProperLinear(BlobTy TempBlob) {
    return getHIRParser()->isGuaranteedProperLinear(TempBlob);
  }

  /// Returns true if Blob is a UndefValue.
  static bool isUndefBlob(BlobTy Blob) {
    return getHIRParser()->isUndefBlob(Blob);
  }

  /// Returns true if Blob represents a FP constant.
  static bool isConstantFPBlob(BlobTy Blob, ConstantFP **Val = nullptr) {
    return getHIRParser()->isConstantFPBlob(Blob, Val);
  }

  /// Returns true if Blob represents a vector of constants.
  /// If yes, returns the underlying LLVM Value in Val
  static bool isConstantVectorBlob(BlobTy Blob, Constant **Val = nullptr) {
    return getHIRParser()->isConstantVectorBlob(Blob, Val);
  }

  /// Returns true if Blob represents a metadata.
  /// If blob is metadata, sets the return value in Val.
  static bool isMetadataBlob(BlobTy Blob, MetadataAsValue **Val = nullptr) {
    return getHIRParser()->isMetadataBlob(Blob, Val);
  }

  /// Returns true if \p Blob represents a signed extension value.
  /// If blob is sext, sets the return value in Val.
  static bool isSignExtendBlob(BlobTy Blob, BlobTy *Val = nullptr) {
    return getHIRParser()->isSignExtendBlob(Blob, Val);
  }

  /// Returns a new blob created from passed in Val.
  static BlobTy createBlob(Value *Val, bool Insert = true,
                           unsigned *NewBlobIndex = nullptr) {
    return getHIRParser()->createBlob(Val, InvalidSymbase, Insert,
                                      NewBlobIndex);
  }

  /// Returns a new blob created from a constant value.
  static BlobTy createBlob(int64_t Val, Type *Ty, bool Insert = true,
                           unsigned *NewBlobIndex = nullptr) {
    return getHIRParser()->createBlob(Val, Ty, Insert, NewBlobIndex);
  }

  /// Returns a blob which represents (LHS + RHS). If Insert is true its index
  /// is returned via NewBlobIndex argument.
  static BlobTy createAddBlob(BlobTy LHS, BlobTy RHS, bool Insert = true,
                              unsigned *NewBlobIndex = nullptr) {
    return getHIRParser()->createAddBlob(LHS, RHS, Insert, NewBlobIndex);
  }

  /// Returns a blob which represents (LHS - RHS). If Insert is true its index
  /// is returned via NewBlobIndex argument.
  static BlobTy createMinusBlob(BlobTy LHS, BlobTy RHS, bool Insert = true,
                                unsigned *NewBlobIndex = nullptr) {
    return getHIRParser()->createMinusBlob(LHS, RHS, Insert, NewBlobIndex);
  }

  /// Returns a blob which represents (LHS * RHS). If Insert is true its index
  /// is returned via NewBlobIndex argument.
  static BlobTy createMulBlob(BlobTy LHS, BlobTy RHS, bool Insert = true,
                              unsigned *NewBlobIndex = nullptr) {
    return getHIRParser()->createMulBlob(LHS, RHS, Insert, NewBlobIndex);
  }

  /// Returns a blob which represents (LHS / RHS). If Insert is true its index
  /// is returned via NewBlobIndex argument.
  static BlobTy createUDivBlob(BlobTy LHS, BlobTy RHS, bool Insert = true,
                               unsigned *NewBlobIndex = nullptr) {
    return getHIRParser()->createUDivBlob(LHS, RHS, Insert, NewBlobIndex);
  }

  /// Returns a blob which represents (trunc Blob to Ty). If Insert is true its
  /// index is returned via NewBlobIndex argument.
  static BlobTy createTruncateBlob(BlobTy Blob, Type *Ty, bool Insert = true,
                                   unsigned *NewBlobIndex = nullptr) {
    return getHIRParser()->createTruncateBlob(Blob, Ty, Insert, NewBlobIndex);
  }

  /// Returns a blob which represents (zext Blob to Ty). If Insert is true its
  /// index is returned via NewBlobIndex argument.
  static BlobTy createZeroExtendBlob(BlobTy Blob, Type *Ty, bool Insert = true,
                                     unsigned *NewBlobIndex = nullptr) {
    return getHIRParser()->createZeroExtendBlob(Blob, Ty, Insert, NewBlobIndex);
  }

  /// Returns a blob which represents (sext Blob to Ty). If Insert is true its
  /// index is returned via NewBlobIndex argument.
  static BlobTy createSignExtendBlob(BlobTy Blob, Type *Ty, bool Insert = true,
                                     unsigned *NewBlobIndex = nullptr) {
    return getHIRParser()->createSignExtendBlob(Blob, Ty, Insert, NewBlobIndex);
  }

  /// Returns a new blob with appropriate cast (SExt, ZExt, Trunc) applied on
  /// top of \p Blob. If Insert is true its index is returned via NewBlobIndex
  /// argument.
  static BlobTy createCastBlob(BlobTy Blob, bool IsSExt, Type *Ty,
                               bool Insert = true,
                               unsigned *NewBlobIndex = nullptr) {
    return getHIRParser()->createCastBlob(Blob, IsSExt, Ty, Insert,
                                          NewBlobIndex);
  }

  /// Returns true if Blob contains SubBlob or if Blob == SubBlob.
  static bool contains(BlobTy Blob, BlobTy SubBlob) {
    return getHIRParser()->contains(Blob, SubBlob);
  }

  /// Returns all the temp blobs present in Blob via TempBlobs vector.
  static void collectTempBlobs(BlobTy Blob,
                               SmallVectorImpl<BlobTy> &TempBlobs) {
    getHIRParser()->collectTempBlobs(Blob, TempBlobs);
  }

  /// Returns all the temp blobs present in blob with index \p BlobIndex via \p
  /// TempBlobIndices vector.
  static void collectTempBlobs(unsigned BlobIndex,
                               SmallVectorImpl<unsigned> &TempBlobIndices);

  /// Replaces \p OldTempIndex by \p NewTempIndex in \p BlobIndex and returns
  /// the new blob in \p NewBlobIndex. Returns true if substitution was
  /// performed.
  static bool replaceTempBlob(unsigned BlobIndex, unsigned OldTempIndex,
                              unsigned NewTempIndex,
                              unsigned &NewBlobIndex) {
    return getHIRParser()->replaceTempBlob(BlobIndex, OldTempIndex,
                                           NewTempIndex, NewBlobIndex);
  }
};

} // End namespace loopopt

} // End namespace llvm

#endif
