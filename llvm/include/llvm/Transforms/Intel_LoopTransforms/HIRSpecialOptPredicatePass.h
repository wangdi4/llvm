//===----------- HIRSpecialOptPredicatePass.h ------------------*- C++-*---===//
//
// Copyright (C) 2023 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_HIRSPECIALOPTPREDICATE_H
#define LLVM_HIRSPECIALOPTPREDICATE_H

#include "llvm/Transforms/Intel_LoopTransforms/HIRTransformPass.h"

namespace llvm {

namespace loopopt {

class HIRSpecialOptPredicatePass
    : public HIRPassInfoMixin<HIRSpecialOptPredicatePass> {

public:
  static constexpr auto PassName = "hir-special-opt-predicate";
  PreservedAnalyses runImpl(Function &F, FunctionAnalysisManager &AM,
                            HIRFramework &HIRF);
};

} // namespace loopopt

} // namespace llvm

#endif // LLVM_HIRSPECIALOPTPREDICATE_H
