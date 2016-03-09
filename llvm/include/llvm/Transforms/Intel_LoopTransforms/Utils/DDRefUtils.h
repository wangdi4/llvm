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

#include "llvm/Transforms/Intel_LoopTransforms/Utils/HIRUtils.h"

namespace llvm {

namespace loopopt {

/// \brief Defines utilities for DDRef class
///
/// It contains a bunch of static member functions which manipulate DDRefs.
/// It does not store any state.
class DDRefUtils : public HIRUtils {
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

  /// \brief Prints metadata nodes attached to RegDDRef.
  static void printMDNodes(formatted_raw_ostream &OS,
                           const RegDDRef::MDNodesTy &MDNodes);
};

} // End namespace loopopt

} // End namespace llvm

#endif
