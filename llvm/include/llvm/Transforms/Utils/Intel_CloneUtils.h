//===--------- Intel_CloneUtils.h - Utilites for Cloning ----------===//
//
// Copyright (C) 2019-2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //
///
/// \file
/// This file includes a set of utilities that are useful for cloning,
/// in particular, cloning heuristics.
///
// ===--------------------------------------------------------------------=== //

#ifndef LLVM_TRANSFORM_UTILS_INTEL_CLONEUTILS_H
#define LLVM_TRANSFORM_UTILS_INTEL_CLONEUTILS_H

#include "llvm/IR/Instructions.h"

namespace llvm {

class Function;

#if INTEL_FEATURE_SW_ADVANCED
//
// Return 'true' if 'F' is a recursive progression clone candidate. If
// 'TestCountForConstant' check that the period of the recursive progression
// is a reasonably small constant. If 'true' is returned, set '*ArgPos' to
// the position of the recursive progression argument, and set '*Start',
// '*Inc', and '*Count' to the initial value, increment, and number of terms
// in the recursive progression. Also set 'IsByRef' if the recursive
// progression argument is a by reference value, set 'ArgType' to the type
// of the recursive progression argument if the recursive progression
// argument is by value and to the pointer element type of the recursive
// progression argument if the recursive progression argument is by
// reference, and set 'IsCyclic' if the recursive progression is cyclic.
// If 'TestCountForConstant' is false, set '*Count' to 0. If any of 'ArgPos',
// 'Count', 'Start' 'Inc', 'IsByRef', 'ArgType', or 'IsCyclic' are nullptr,
// do not set the value on return.
//
// For an example of a recursive progression, see Intel_CloneUtils.cpp.
//
extern bool isRecProgressionCloneCandidate(
    Function &F, bool TestCountForConstant, unsigned *ArgPos = nullptr,
    unsigned *Count = nullptr, int *Start = nullptr, int *Inc = nullptr,
    bool *IsByRef = nullptr, Type **ArgType = nullptr,
    bool *IsCyclic = nullptr);

#endif // INTEL_FEATURE_SW_ADVANCED
//
// Return the unique callsite of 'F', if there is a unique callsite.
//
extern CallInst *uniqueCallSite(Function &F);
} // namespace llvm

#endif // LLVM_TRANSFORM_UTILS_INTEL_CLONEUTILS_H
