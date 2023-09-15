//===---- HIRDeadStoreElimination.h -----------------*- C++-*---===//
//
// Copyright (C) 2015 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// Implements HIR Dead Store Elimination Pass.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_HIRDEADSTOREELIMINATIONPASS_H
#define LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_HIRDEADSTOREELIMINATIONPASS_H

#include "llvm/Transforms/Intel_LoopTransforms/HIRTransformPass.h"

namespace llvm {

namespace loopopt {

class HIRDeadStoreEliminationPass
    : public HIRPassInfoMixin<HIRDeadStoreEliminationPass> {

public:
  static constexpr auto PassName = "hir-dead-store-elimination";
  PreservedAnalyses runImpl(Function &F, FunctionAnalysisManager &AM,
                            HIRFramework &HIRF);
};

} // namespace loopopt

} // namespace llvm

#endif // LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_DEADSTOREELIMINATIONPASS_H
