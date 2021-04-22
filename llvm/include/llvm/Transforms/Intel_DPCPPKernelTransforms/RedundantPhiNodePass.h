//==--- RedundantPhiNodePass.h - Remove redundant Phi nodes - C++ -*--------==//
//
// Copyright (C) 2020 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //

#ifndef LLVM_TRANSFORMS_INTEL_DPCPP_KERNEL_TRANSFORMS_REDUNDANT_PHIS_H
#define LLVM_TRANSFORMS_INTEL_DPCPP_KERNEL_TRANSFORMS_REDUNDANT_PHIS_H

#include "llvm/IR/Function.h"
#include "llvm/IR/PassManager.h"

namespace llvm {

/// RedundantPhiNode pass is a function pass that remove redundant PHINode
/// such that return same value for each entry block.
class RedundantPhiNode : public PassInfoMixin<RedundantPhiNode> {
public:
  static StringRef name() { return "Intel Kernel RedundantPhiNode"; }

  PreservedAnalyses run(Function &F, FunctionAnalysisManager &FAM);

  bool runImpl(Function &F);
};

/// RedundantPhiNodeLegacy pass for legacy pass manager.
class RedundantPhiNodeLegacy : public FunctionPass {
  RedundantPhiNode Impl;

public:
  static char ID;

  RedundantPhiNodeLegacy();

  StringRef getPassName() const override {
    return "Intel Kernel RedundantPhiNode";
  }

  bool runOnFunction(Function &F) override;
};

} // namespace llvm

#endif // LLVM_TRANSFORMS_INTEL_DPCPP_KERNEL_TRANSFORMS_REDUNDANT_PHIS_H
