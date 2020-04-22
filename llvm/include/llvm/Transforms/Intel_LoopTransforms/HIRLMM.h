//===--- HIRLMM.h -HIR Loop Memory Motion Pass ----------------*- C++ -*---===//
//
// Copyright (C) 2015-2020 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//

#ifndef LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_LMM_H
#define LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_LMM_H

#include "llvm/IR/PassManager.h"
#include "llvm/Pass.h"

namespace llvm {
class Function;

namespace loopopt {

class HIRLMMPass : public PassInfoMixin<HIRLMMPass> {
  bool LoopNestHoistingOnly;

public:
  HIRLMMPass(bool LoopNestHoistingOnly = false)
      : LoopNestHoistingOnly(LoopNestHoistingOnly) {}
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &AM);
};

} // namespace loopopt
} // namespace llvm

#endif
