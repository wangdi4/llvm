//===-------- DDRefUtils.h - Utilities for DDRef class ----------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file defines the utilities for DDRef class.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_UTILS_DDREFUTILS_H
#define LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_UTILS_DDREFUTILS_H

#include "llvm/Support/Compiler.h"
#include "llvm/IR/Intel_LoopIR/ConstDDRef.h"
#include "llvm/IR/Intel_LoopIR/BlobDDRef.h"
#include "llvm/IR/Intel_LoopIR/RegDDRef.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/HLNodeUtils.h"

namespace llvm {

namespace loopopt {

/// \brief Defines utilities for DDRef class
///
/// It contains a bunch of static member functions which manipulate DDRefs.
/// It does not store any state.
///
class DDRefUtils {
private:
  /// \brief Do not allow instantiation.
  DDRefUtils() LLVM_DELETED_FUNCTION;

public:
  /// \brief Returns a new ConstDDRef.
  static ConstDDRef *createConstDDRef(CanonExpr *CE);

  /// \brief Returns a new RegDDRef.
  static RegDDRef *createRegDDRef(int SB);

  /// \brief Returns a new BlobDDRef.
  static BlobDDRef *createBlobDDRef(int SB, CanonExpr *CE,
                                    RegDDRef *Parent = nullptr);

  /// \brief Destroys the passed in DDRef.
  static void destroy(DDRef *Ref);
  /// \brief Destroys all DDRefs. Should only be called after code gen.
  static void destroyAll();
};

} // End namespace loopopt

} // End namespace llvm

#endif
