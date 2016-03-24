//===-------- DDRefUtils.h - Utilities for DDRef class ---*- C++ -*--------===//
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
// This file defines the utilities for DDRef class.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_UTILS_DDREFUTILS_H
#define LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_UTILS_DDREFUTILS_H

#include <map>

#include "llvm/Support/Compiler.h"

#include "llvm/IR/Intel_LoopIR/BlobDDRef.h"
#include "llvm/IR/Intel_LoopIR/RegDDRef.h"

#include "llvm/Transforms/Intel_LoopTransforms/Utils/HLUtils.h"

namespace llvm {

class MetadataAsValue;
class ConstantAggregateZero;
class ConstantDataVector;

namespace loopopt {

/// \brief Defines utilities for DDRef class
///
/// It contains a bunch of static member functions which manipulate DDRefs.
/// It does not store any state.
class DDRefUtils : public HLUtils {
private:
  /// \brief Do not allow instantiation.
  DDRefUtils() = delete;

  friend class HIRParser;
  friend class HLNodeUtils;

  /// \brief Destroys all DDRefs. Called during HIR cleanup.
  static void destroyAll();

  /// \brief Creates a non-linear self blob scalar RegDDRef from the passed in
  /// Value. Temp blobs from values are only created by framework.
  static RegDDRef *createSelfBlobRef(Value *Temp);

  /// \brief Return true if RegDDRef1 equals RegDDRef2.
  /// This routine compares the symbase, type and each of the canon exprs
  /// inside the references.
  static bool areEqualImpl(const RegDDRef *Ref1, const RegDDRef *Ref2,
                           bool IgnoreDestType);

  /// \brief Returns true if BlobDDRef1 equals BlobDDRef2.
  static bool areEqualImpl(const BlobDDRef *Ref1, const BlobDDRef *Ref2);

public:
  /// \brief Returns a new RegDDRef.
  static RegDDRef *createRegDDRef(unsigned SB);

  /// \brief Creates a new DDRef with single canon expr CE.
  static RegDDRef *createScalarRegDDRef(unsigned SB, CanonExpr *CE);

  /// \brief Returns a new constant RegDDRef from a int value.
  /// This routine will automatically create a single canon expr from the val
  /// and attach it to the new RegDDRef.
  static RegDDRef *createConstDDRef(Type *Ty, int64_t Val);

  /// \brief Returns a new constant RegDDRef from a metadata node.
  /// This routine will automatically create a single canon expr from metadata
  /// and attach it to the new RegDDRef.
  static RegDDRef *createMetadataDDRef(MetadataAsValue *Val);

  /// \brief Returns a new constant RegDDRef from a constant all-zero vector
  /// node. This routine will automatically create a single canon expr from
  /// ConstantAggregateZero and attach it to the new RegDDRef.
  static RegDDRef *createConstDDRef(ConstantAggregateZero *Val);

  /// \brief Returns a new constant RegDDRef from a constant data vector
  /// node. This routine will automatically create a single canon expr from
  /// ConstantDataVector and attach it to the new RegDDRef.
  static RegDDRef *createConstDDRef(ConstantDataVector *Val);

  /// \brief Returns a new RegDDRef with given type \p Ty and undefined
  /// value.
  static RegDDRef *createUndefDDRef(Type *Ty);

  /// \brief Returns a new BlobDDRef representing blob with Index. Level is the
  /// defined at level for the blob. Level of -1 means non-linear blob.
  static BlobDDRef *createBlobDDRef(unsigned Index, int Level = -1);

  /// \brief Returns a new RegDDRef representing blob with Index. Level is the
  /// defined at level for the blob. Level of -1 means non-linear blob.
  static RegDDRef *createSelfBlobRef(unsigned Index, int Level = -1);

  /// \brief Destroys the passed in DDRef.
  static void destroy(DDRef *Ref);

  /// \brief Returns a new symbase.
  static unsigned getNewSymbase();

  /// \brief Returns true if the two DDRefs, Ref1 and Ref2, are equal.
  /// IgnoreDestType parameter is only used for base destination type comparison
  /// of RegDDRef. This parameter is ignored in all other cases.
  static bool areEqual(const DDRef *Ref1, const DDRef *Ref2,
                       bool IgnoreDestType = false);

  /// \brief Returns true if it is able to compute a constant distance between
  /// \p Ref1 and \p Ref2.
  ///
  /// Context: This utility is called by optVLS, which tries to find neighboring
  /// vector loads/stores (the refs are not yet vectorized, but this is called
  /// at the point when we are considering to vectorize a certain loop).
  /// Normally it will be called for two memrefs that are strided and have the
  /// same stride (a[2*i], a[2*i+1]) or two memrefs that are indexed (indirect)
  /// and have the same index vector (a[b[i]], a[b[i]+1]). When each of these
  /// two refs is vectorized, we will need to generate a gather instruction for
  /// each. Instead, we want to examine whether we can load the neighboring
  /// elements of these two (vectorized) refs together with regular loads
  /// (followed by shuffles). The distance will tell us if we can fit two
  /// neighbors in the same vector register.
  ///
  /// \param [out] Distance holds the constant distance in bytes if obtained.
  /// The Distance can result from a difference in any of the subscripts --
  /// not only the innermost, and even in multiple subscripts. For example,
  /// the distance between a[2*i][j] and a[2*i+1][j+1] when 'a' is int a[8][8]
  /// is 36 bytes, which allows fitting both elements in one vector register.
  /// The caller will consider this and decide if it is more efficient to
  /// do that than to generate two separate gathers. A difference between
  /// struct accesses such as a[i].I and a[i].F where 'a' is an array of
  /// struct S {int I; float F;} will also be supported.
  static bool getConstDistance(const RegDDRef *Ref1, const RegDDRef *Ref2,
                               int64_t *Distance);
};

} // End namespace loopopt

} // End namespace llvm

#endif
