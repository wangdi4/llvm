//===---- HIRSinkingForPerfectLoopnest.h -----------------------------===//
//
// Copyright (C) 2019 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_HIRSINKINGFORPERFECTLOOPNEST_H
#define LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_HIRSINKINGFORPERFECTLOOPNEST_H

#include "llvm/IR/PassManager.h"

namespace llvm {

namespace loopopt {

class HIRSinkingForPerfectLoopnestPass
    : public PassInfoMixin<HIRSinkingForPerfectLoopnestPass> {
public:
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &AM);
};

} // namespace loopopt

} // namespace llvm

#endif // LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_HIRSINKINGFORPERFECTLOOPNEST_H
