#if INTEL_FEATURE_SW_ADVANCED
//===------------------- Intel_PartialInlineAnalysis.h ------------------===//
//===----------- Utilities for Intel Partial Inlining Analysis ----------===//
//
// Copyright (C) 2019 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //
///
/// \file
/// This file includes a set of analysis routines that are useful for partial
/// inlining, in particular, partial inlining heuristics.
///
// ===--------------------------------------------------------------------=== //

#ifndef LLVM_ANALYSIS_INTEL_PARTIALINLINEANALYSIS_H
#define LLVM_ANALYSIS_INTEL_PARTIALINLINEANALYSIS_H

#include <functional>

namespace llvm {

class Function;
class LoopInfo;
using LoopInfoFuncType = std::function<LoopInfo &(Function &)>;

// Return true if the function F is candidate for partial inline. This
// means that we can split the input function in three regions:
//  *Arguments
//  *Loop
//  *Exit
// Else return false.
bool isIntelPartialInlineCandidate(Function *F,
                                   LoopInfoFuncType &GetLoopInfo,
                                   bool PrepareForLTO);
} // namespace llvm

#endif // LLVM_ANALYSIS_INTEL_PARTIALINLINEANALYSIS_H
#endif // INTEL_FEATURE_SW_ADVANCED
