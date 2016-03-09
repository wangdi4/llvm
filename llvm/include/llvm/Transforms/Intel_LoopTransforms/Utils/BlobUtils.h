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

#include <stdint.h>
#include "llvm/Support/Compiler.h"

#include "llvm/Analysis/Intel_LoopAnalysis/HIRParser.h"

#include "llvm/Transforms/Intel_LoopTransforms/Utils/HIRUtils.h"

namespace llvm {

class Type;

namespace loopopt {

class HIRParser;

/// \brief Contains blob related utilities.
class BlobUtils : public HIRUtils {
private:
  /// \brief Do not allow instantiation.
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
                               unsigned *NewBlobIndex);

  /// \brief Returns the index of Blob in the blob table. Blob is first
  /// inserted, if it isn't already present in the blob table. Index range is
  /// [1, UINT_MAX]. There is a 1-1 mapping of temp blob index and symbase. This
  /// information is stored in the blob table. This interface is private
  /// because only the framework is allowed to create temp blobs for insertion
  /// in the blob table.
  static unsigned findOrInsertBlob(BlobTy Blob, unsigned Symbase);
 
public:
  /// \brief Returns the index of Blob in the blob table. Index range is [1,
  /// UINT_MAX]. Returns invalid value if the blob is not present in the table.
  static unsigned findBlob(BlobTy Blob);

  /// \brief Returns symbase corresponding to Blob. Returns invalid value for
  /// non-temp or non-present blobs.
  static unsigned findBlobSymbase(BlobTy Blob);

  /// \brief Returns the index of Blob in the blob table. Blob is first
  /// inserted, if it isn't already present in the blob table. Index range is
  /// [1, UINT_MAX].
  /// NOTE: New temp blobs can only be inserted by the framework.
  static unsigned findOrInsertBlob(BlobTy Blob);

  /// \brief Maps blobs in Blobs to their corresponding indices and inserts
  /// them in Indices.
  static void mapBlobsToIndices(const SmallVectorImpl<BlobTy> &Blobs,
                                SmallVectorImpl<unsigned> &Indices);

  /// \brief Returns blob corresponding to BlobIndex.
  static BlobTy getBlob(unsigned BlobIndex);

  /// \brief Returns symbase corresponding to BlobIndex. Returns invalid value
  /// for non-temp blobs.
  static unsigned getBlobSymbase(unsigned BlobIndex);

  /// \brief Returns true if this is a valid blob index.
  static bool isBlobIndexValid(unsigned BlobIndex);

  /// \brief Prints blob.
  static void printBlob(raw_ostream &OS, BlobTy Blob);

  /// \brief Prints scalar corresponding to symbase.
  static void printScalar(raw_ostream &OS, unsigned Symbase);

  /// \brief Checks if the blob is constant or not.
  /// If blob is constant, sets the return value in Val.
  static bool isConstantIntBlob(BlobTy Blob, int64_t *Val);

  /// \brief Returns true if Blob is a temp.
  static bool isTempBlob(BlobTy Blob);

  /// \brief Returns true if TempBlob always has a defined at level of zero.
  static bool isGuaranteedProperLinear(BlobTy TempBlob);

  /// \brief Returns true if Blob is a UndefValue.
  static bool isUndefBlob(BlobTy Blob);

  /// \brief Returns true if Blob represents a FP constant.
  static bool isConstantFPBlob(BlobTy Blob);

  /// \brief Returns a new blob created from passed in Val.
  static BlobTy createBlob(Value *Val, bool Insert = true,
                                      unsigned *NewBlobIndex = nullptr);

  /// \brief Returns a new blob created from a constant value.
  static BlobTy createBlob(int64_t Val, Type *Ty, bool Insert = true,
                                      unsigned *NewBlobIndex = nullptr);

  /// \brief Returns a blob which represents (LHS + RHS). If Insert is true its
  /// index is returned via NewBlobIndex argument.
  static BlobTy createAddBlob(BlobTy LHS,
                                         BlobTy RHS,
                                         bool Insert = true,
                                         unsigned *NewBlobIndex = nullptr);

  /// \brief Returns a blob which represents (LHS - RHS). If Insert is true its
  /// index is returned via NewBlobIndex argument.
  static BlobTy createMinusBlob(BlobTy LHS,
                                           BlobTy RHS,
                                           bool Insert = true,
                                           unsigned *NewBlobIndex = nullptr);
  /// \brief Returns a blob which represents (LHS * RHS). If Insert is true its
  /// index is returned via NewBlobIndex argument.
  static BlobTy createMulBlob(BlobTy LHS,
                                         BlobTy RHS,
                                         bool Insert = true,
                                         unsigned *NewBlobIndex = nullptr);
  /// \brief Returns a blob which represents (LHS / RHS). If Insert is true its
  /// index is returned via NewBlobIndex argument.
  static BlobTy createUDivBlob(BlobTy LHS,
                                          BlobTy RHS,
                                          bool Insert = true,
                                          unsigned *NewBlobIndex = nullptr);
  /// \brief Returns a blob which represents (trunc Blob to Ty). If Insert is
  /// true its index is returned via NewBlobIndex argument.
  static BlobTy createTruncateBlob(BlobTy Blob, Type *Ty,
                                              bool Insert = true,
                                              unsigned *NewBlobIndex = nullptr);
  /// \brief Returns a blob which represents (zext Blob to Ty). If Insert is
  /// true its index is returned via NewBlobIndex argument.
  static BlobTy
  createZeroExtendBlob(BlobTy Blob, Type *Ty, bool Insert = true,
                       unsigned *NewBlobIndex = nullptr);
  /// \brief Returns a blob which represents (sext Blob to Ty). If Insert is
  /// true its index is returned via NewBlobIndex argument.
  static BlobTy
  createSignExtendBlob(BlobTy Blob, Type *Ty, bool Insert = true,
                       unsigned *NewBlobIndex = nullptr);

  /// \brief Returns true if Blob contains SubBlob or if Blob == SubBlob.
  static bool contains(BlobTy Blob, BlobTy SubBlob);

  /// \brief Returns all the temp blobs present in Blob via TempBlobs vector.
  static void collectTempBlobs(BlobTy Blob,
                               SmallVectorImpl<BlobTy> &TempBlobs);
};

} // End namespace loopopt

} // End namespace llvm

#endif
