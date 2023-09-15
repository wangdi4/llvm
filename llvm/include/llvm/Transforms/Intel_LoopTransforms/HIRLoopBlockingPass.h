//===---- HIRLoopBlocking.h - Implements Loop Blocking Pass class ----===//
//
// Copyright (C) 2018 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_HIRLOOPBLOCKING_H
#define LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_HIRLOOPBLOCKING_H

#include "llvm/Transforms/Intel_LoopTransforms/HIRTransformPass.h"

namespace llvm {

namespace loopopt {

class HIRLoopBlockingPass : public HIRPassInfoMixin<HIRLoopBlockingPass> {
  bool SinkForMultiCopy;

public:
  HIRLoopBlockingPass(bool SinkForMultiCopy = true)
      : SinkForMultiCopy(SinkForMultiCopy) {}
  static constexpr auto PassName = "hir-loop-blocking";
  PreservedAnalyses runImpl(Function &F, FunctionAnalysisManager &AM,
                            HIRFramework &HIRF);
};

class HIRPragmaLoopBlockingPass
    : public HIRPassInfoMixin<HIRPragmaLoopBlockingPass> {
public:
  static constexpr auto PassName = "hir-pragma-loop-blocking";
  PreservedAnalyses runImpl(Function &F, FunctionAnalysisManager &AM,
                            HIRFramework &HIRF);
};

} // namespace loopopt

} // namespace llvm

#endif // LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_HIRLOOPBLOCKING_H
