//===------------------------DTransDebug.h----------------------------===//
//
// Copyright (C) 2020 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===---------------------------------------------------------------------===//

//
// This file defines support routines for debug traces for DTrans classes
//

#if !INTEL_FEATURE_SW_DTRANS
#error DTransDebug.h include in an non-INTEL_FEATURE_SW_DTRANS build.
#endif

#ifndef INTEL_DTRANS_ANALYSIS_DTRANSDEBUG_H
#define INTEL_DTRANS_ANALYSIS_DTRANSDEBUG_H

namespace llvm {
namespace dtrans {

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

namespace debug {

// Predicate class for use with DEBUG_WITH_TYPE_P macro to control
// verbose message output.
class DebugFilter {
public:
  void reset() { setEnabled(true); }
  void setEnabled(bool Val) { Enabled = Val; }

  // predicate function for DEBUG_WITH_TYPE_P macro.
  bool operator()() { return Enabled; }

private:
  // Default to always emit. If message filtering is enabled,
  // calls to setEnabled will turn this on and off as needed.
  bool Enabled = true;
};

} // end namespace debug

#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

// A predicate test version of the DEBUG_WITH_TYPE macro, where the
// DEBUG_WITH_TYPE will only be executed when the predicate is 'true'. This is
// to enable verbose traces to be enabled/disabled based on the function being
// processed.
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
#define DEBUG_WITH_TYPE_P(PRED, TYPE, X)                                       \
  if (PRED()) {                                                                \
    DEBUG_WITH_TYPE(TYPE, X);                                                  \
  }
#else
#define DEBUG_WITH_TYPE_P(PRED, TYPE, X)                                       \
  do {                                                                         \
  } while (false)
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

} // end namespace dtrans
} // end namespace llvm

#endif // INTEL_DTRANS_ANALYSIS_DTRANSDEBUG_H
