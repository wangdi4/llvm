//===- LoopStridedCodeMotion.h - Hoist strided value ------------*- C++ -*-===//
//
// Copyright (C) 2022 Intel Corporation
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you ("License"). Unless the License provides otherwise, you may
// not use, modify, copy, publish, distribute, disclose or transmit this
// software or the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.
//
//===----------------------------------------------------------------------===//
//
// This pass attempts to hoist strided instructions in a loop to preheader.
// It uses LoopWIAnalysis to find strided values.
// The pass also avoids hoisting an instruction if
//   * it is not profitable when its users can't be hoisted, which may lead to
//     create more phi nodes.
//   * it is FMul instruction without fast flag to avoid accuracy loss.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_SYCLTRANSFORMS_LOOPSTRIDEDCODEMOTION_H
#define LLVM_TRANSFORMS_SYCLTRANSFORMS_LOOPSTRIDEDCODEMOTION_H

#include "llvm/IR/PassManager.h"
#include "llvm/Transforms/Scalar/LoopPassManager.h"

namespace llvm {

class LoopStridedCodeMotionPass
    : public PassInfoMixin<LoopStridedCodeMotionPass> {
public:
  PreservedAnalyses run(Loop &L, LoopAnalysisManager &AM,
                        LoopStandardAnalysisResults &AR, LPMUpdater &U);
};

} // namespace llvm
#endif // LLVM_TRANSFORMS_SYCLTRANSFORMS_LOOPSTRIDEDCODEMOTION_H
