//===--- HIRLMM.h -HIR Loop Memory Motion Pass ----------------*- C++ -*---===//
//
// Copyright (C) 2015 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//

#ifndef LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_LMM_H
#define LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_LMM_H

#include "llvm/Pass.h"
#include "llvm/Transforms/Intel_LoopTransforms/HIRTransformPass.h"

namespace llvm {
class Function;

namespace loopopt {

class HIRLMMPass : public HIRPassInfoMixin<HIRLMMPass> {
  bool LoopNestHoistingOnly;

public:
  HIRLMMPass(bool LoopNestHoistingOnly = false)
      : LoopNestHoistingOnly(LoopNestHoistingOnly) {}

  static constexpr auto PassName = "hir-lmm";
  PreservedAnalyses runImpl(Function &F, FunctionAnalysisManager &AM,
                            HIRFramework &HIRF);
};

} // namespace loopopt
} // namespace llvm

#endif
