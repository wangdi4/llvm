//===---- HIRPreVecCompleteUnroll.h ---------------------------------------===//
//
// Copyright (C) 2018-2020 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_HIRPREVECCOMPLETEUNROLL_H
#define LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_HIRPREVECCOMPLETEUNROLL_H

#include "llvm/IR/PassManager.h"

namespace llvm {

namespace loopopt {

class HIRPreVecCompleteUnrollPass
    : public PassInfoMixin<HIRPreVecCompleteUnrollPass> {
  unsigned OptLevel;
  bool PragmaOnlyUnroll;

public:
  HIRPreVecCompleteUnrollPass(unsigned OptLevel = 0,
                              bool PragmaOnlyUnroll = false)
      : OptLevel(OptLevel), PragmaOnlyUnroll(PragmaOnlyUnroll) {}
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &AM);
};

} // namespace loopopt

} // namespace llvm

#endif // LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_HIRPREVECCOMPLETEUNROLL_H
