//===------------------- HIRMultiExitLoopReroll.h -------------------------===//
//
// Copyright (C) 2019 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_HIRMULTIEXITLOOPREROLL_H
#define LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_HIRMULTIEXITLOOPREROLL_H

#include "llvm/Transforms/Intel_LoopTransforms/HIRTransformPass.h"

namespace llvm {

namespace loopopt {

class HIRMultiExitLoopRerollPass
    : public HIRPassInfoMixin<HIRMultiExitLoopRerollPass> {
public:
  static constexpr auto PassName = "hir-multi-exit-loop-reroll";
  PreservedAnalyses runImpl(Function &F, FunctionAnalysisManager &AM,
                            HIRFramework &HIRF);
};

} // namespace loopopt

} // namespace llvm

#endif // LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_HIRMULTIEXITLOOPREROLL_H
