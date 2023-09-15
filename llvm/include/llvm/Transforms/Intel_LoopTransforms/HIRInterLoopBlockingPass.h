#if INTEL_FEATURE_SW_ADVANCED
//===---- HIRInterLoopBlocking.h -----------------------------===//
//
// Copyright (C) 2020 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===-----------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_HIRINTERLOOPBLOCKING_H
#define LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_HIRINTERLOOPBLOCKING_H

#include "llvm/Transforms/Intel_LoopTransforms/HIRTransformPass.h"

namespace llvm {

namespace loopopt {

class HIRInterLoopBlockingPass
    : public HIRPassInfoMixin<HIRInterLoopBlockingPass> {
public:
  static constexpr auto PassName = "hir-inter-loop-blocking";
  PreservedAnalyses runImpl(Function &F, FunctionAnalysisManager &AM,
                            HIRFramework &HIRF);
};

} // namespace loopopt

} // namespace llvm

#endif // LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_HIRINTERLOOPBLOCKING_H
#endif // INTEL_FEATURE_SW_ADVANCED
