//===------ HIRUnroll.h - Interface for common unroll utilities *-- C++ --*---//
//
// Copyright (C) 2016-2017 Intel Corporation. All rights reserved.
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
namespace loopopt {

class HLLoop;

namespace unroll {
/// Performs general unrolling or unroll & jam on \p Loop based on whether it is an innermost loop.
/// NodeMapper is used by unroll & jam to update inner loop candidates when unrolling outer loops.
void unrollLoop(HLLoop *Loop, unsigned UnrollFactor);
}
}
}

#endif /* LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_HIRUNROLL_H */
