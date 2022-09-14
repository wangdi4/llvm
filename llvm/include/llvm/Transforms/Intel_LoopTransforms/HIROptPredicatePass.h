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

#include "llvm/Transforms/Intel_LoopTransforms/HIRTransformPass.h"

namespace llvm {

namespace loopopt {

class HIROptPredicatePass : public HIRPassInfoMixin<HIROptPredicatePass> {
  bool EnablePartialUnswitch;
  bool EarlyPredicateOpt;

public:
  HIROptPredicatePass(bool EnablePartialUnswitch = true,
                      bool EarlyPredicateOpt = false)
      : EnablePartialUnswitch(EnablePartialUnswitch),
        EarlyPredicateOpt(EarlyPredicateOpt) {}
  static constexpr auto PassName = "hir-opt-predicate";
  PreservedAnalyses runImpl(Function &F, FunctionAnalysisManager &AM,
                            HIRFramework &HIRF);
};

} // namespace loopopt

} // namespace llvm

#endif // LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_HIROPTPREDICATE_H
