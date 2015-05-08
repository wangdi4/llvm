//===-------- HLUtils.h - Base class for utilities ----------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under TODO.
//
//===----------------------------------------------------------------------===//
//
// This file defines the base class for utilites.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_UTILS_HLUTILS_H
#define LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_UTILS_HLUTILS_H

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
