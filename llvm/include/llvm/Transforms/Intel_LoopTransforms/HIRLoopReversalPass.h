//===--- HIRLoopReversal.h ------------------------------------*- C++ -*---===//
//
// Copyright (C) 2015-2020 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_HIRLOOPREVERSAL_H
#define LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_HIRLOOPREVERSAL_H

#include "llvm/IR/PassManager.h"
#include "llvm/Pass.h"

namespace llvm {
class Function;

namespace loopopt {

class HIRLoopReversalPass : public PassInfoMixin<HIRLoopReversalPass> {
public:
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &AM);
};

} // namespace loopopt
} // namespace llvm

#endif
