//===---- HIROptPredicate.h -------------------------------------------===//
//
// Copyright (C) 2018-2020 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_HIROPTPREDICATE_H
#define LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_HIROPTPREDICATE_H

#include "llvm/IR/PassManager.h"

namespace llvm {

namespace loopopt {

class HIROptPredicatePass : public PassInfoMixin<HIROptPredicatePass> {
  bool EnablePartialUnswitch;
  bool KeepLoopnestPerfect;

public:
  HIROptPredicatePass(bool EnablePartialUnswitch = true,
                      bool KeepLoopnestPerfect = false)
      : EnablePartialUnswitch(EnablePartialUnswitch),
        KeepLoopnestPerfect(KeepLoopnestPerfect) {}
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &AM);
};

} // namespace loopopt

} // namespace llvm

#endif // LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_HIROPTPREDICATE_H
