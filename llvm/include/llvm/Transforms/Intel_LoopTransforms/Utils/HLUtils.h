//===-------- HLUtils.h - Base class for utilities ----------*- C++ -*-===//
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
// This file defines the base class for utilites.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_UTILS_HLUTILS_H
#define LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_UTILS_HLUTILS_H

#include <assert.h>

#include "llvm/Support/Compiler.h"

namespace llvm {

namespace loopopt {

class HIRParser;

/// \brief Defines HLUtils base class for utilities
///
/// This class is mainly used to store static pointers
/// for the various analysis during HIR. These pointers
/// would be used internally by other utilities to avoid
/// passing them for each utility call.
///
class HLUtils {
private:
  /// \brief Make class uncopyable.
  void operator=(const HLUtils &) = delete;

  friend class HIRParser;

  static HIRParser *HIRParPtr;

  /// \brief Sets the HIRParser pointer
  static void setHIRParserPtr(HIRParser *HIRP) {
    assert(HIRP && " HIR Parser pointer is null");
    HIRParPtr = HIRP;
  }

protected:
  static HIRParser *getHIRParserPtr() { return HIRParPtr; }
};

} // End namespace loopopt

} // End namespace llvm

#endif
