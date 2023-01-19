//==--- RedundantPhiNodePass.h - Remove redundant Phi nodes - C++ -*--------==//
//
// Copyright (C) 2020 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //

#ifndef LLVM_TRANSFORMS_SYCLTRANSFORMS_REDUNDANT_PHIS_H
#define LLVM_TRANSFORMS_SYCLTRANSFORMS_REDUNDANT_PHIS_H

#include "llvm/IR/Function.h"
#include "llvm/IR/PassManager.h"

namespace llvm {

/// RedundantPhiNode pass is a function pass that remove redundant PHINode
/// such that return same value for each entry block.
class RedundantPhiNode : public PassInfoMixin<RedundantPhiNode> {
public:
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &FAM);

  bool runImpl(Function &F);
};

} // namespace llvm

#endif // LLVM_TRANSFORMS_SYCLTRANSFORMS_REDUNDANT_PHIS_H
