//===------ HIRUnroll.h - Interface for common unroll utilities *-- C++ --*---//
//
// Copyright (C) 2016-2018 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// This file lists shared unroll utilities.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_HIRUNROLL_H
#define LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_HIRUNROLL_H

namespace llvm {

class LoopOptReportBuilder;

namespace loopopt {

class HLLoop;

namespace unroll {
/// Performs general unrolling or unroll & jam on \p Loop based on whether it is
/// an innermost loop.
/// Returns the unrolled loop in \p UnrolledLoop and remainder loop in \p
/// RemainderLoop. Remainder loop can be null.
void unrollLoop(HLLoop *Loop, unsigned UnrollFactor, HLLoop **UnrolledLoop,
                HLLoop **RemainderLoop);

/// Performs complete unroll for \p Loop.
void completeUnrollLoop(HLLoop *Loop);

} // namespace unroll
} // namespace loopopt
} // namespace llvm

#endif /* LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_HIRUNROLL_H */
