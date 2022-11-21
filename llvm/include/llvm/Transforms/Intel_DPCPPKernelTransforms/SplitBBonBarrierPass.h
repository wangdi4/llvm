//==--- SplitBBonBarrierPass.h - Split BB along barrier call - C++ -*-------==//
//
// Copyright (C) 2020-2022 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //

#ifndef LLVM_TRANSFORMS_INTEL_DPCPP_KERNEL_TRANSFORMS_SPLIT_BB_ON_BARRIER
#define LLVM_TRANSFORMS_INTEL_DPCPP_KERNEL_TRANSFORMS_SPLIT_BB_ON_BARRIER

#include "llvm/IR/Module.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/Utils/BarrierUtils.h"

namespace llvm {

class BarrierUtils;

/// SplitBBonBarrier pass is a module pass used to assure
/// barrier/fiber instructions appears only at the begining of basic block
/// and not more than once in each basic block.
class SplitBBonBarrier : public PassInfoMixin<SplitBBonBarrier> {
public:
  PreservedAnalyses run(Module &M, ModuleAnalysisManager &MAM);

  bool runImpl(Module &M);

  static bool isRequired() { return true; }

private:
  /// This is barrier utility class
  BarrierUtils Utils;
};

/// SplitBBonBarrierLegacy pass for legacy pass manager.
class SplitBBonBarrierLegacy : public ModulePass {
  SplitBBonBarrier Impl;

public:
  static char ID;

  SplitBBonBarrierLegacy();

  StringRef getPassName() const override {
    return "Intel Kernel SplitBBonBarrier";
  }

  bool runOnModule(Module &M) override;
};

} // namespace llvm

#endif // LLVM_TRANSFORMS_INTEL_DPCPP_KERNEL_TRANSFORMS_SPLIT_BB_ON_BARRIER
