//===-------- DDRefUtils.h - Utilities for DDRef class ---*- C++ -*--------===//
//
// Copyright (C) 2015 Intel Corporation. All rights reserved.
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

#include "llvm/Support/Compiler.h"

#include "llvm/IR/Intel_LoopIR/BlobDDRef.h"
#include "llvm/IR/Intel_LoopIR/RegDDRef.h"

#include "llvm/Transforms/Intel_LoopTransforms/Utils/HLUtils.h"

namespace llvm {

namespace loopopt {

/// \brief Defines utilities for DDRef class
///
/// It contains a bunch of static member functions which manipulate DDRefs.
/// It does not store any state.
///
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

public:
  /// \brief Returns a new RegDDRef.
  static RegDDRef *createRegDDRef(int SB);

  /// \brief Returns a new BlobDDRef representing blob with Index. Level is the
  /// defined at level for the blob. Level of -1 means non-linear blob.
  static BlobDDRef *createBlobDDRef(unsigned Index, int Level = -1);

  /// \brief Returns a new RegDDRef representing blob with Index. Level is the
  /// defined at level for the blob. Level of -1 means non-linear blob.
  static RegDDRef *createSelfBlobRef(unsigned Index, int Level = -1);

  /// \brief Destroys the passed in DDRef.
  static void destroy(DDRef *Ref);

  /// \brief Returns a new symbase.
  static unsigned getNewSymBase();
};

} // End namespace loopopt

} // End namespace llvm

#endif
