//==--- BarrierInFunction.h - BarrierInFunction - C++ -*--------------------==//
//
// Copyright (C) 2020 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //

#ifndef LLVM_TRANSFORMS_INTEL_DPCPP_KERNEL_BARRIER_IN_FUNCTION_PASS_H
#define LLVM_TRANSFORMS_INTEL_DPCPP_KERNEL_BARRIER_IN_FUNCTION_PASS_H

#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"
#include "llvm/Pass.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/DPCPPKernelBarrierUtils.h"

namespace llvm {

/// Barrier in function pass is a module pass that handles barrier/fiber
/// instructions called from non-kernel functions.
/// For each such function with a barrier add dummyBarrier call at its begin
/// and barrier call at its end, also add barrier call before each call to this
/// function and dummyBarrier call after each call to it (repeate this till
/// handling all functions). For each kernel add dummyBarrier call at its begin
/// and barrier call at its end.
/// Remove each fiber instruction that is called from function that has no
/// barrier.
class BarrierInFunction : public ModulePass {

public:
  BarrierInFunction();

  static char ID;

  llvm::StringRef getPassName() const override {
    return "Intel Kernel BarrierInFunction";
  }

  /// Execute pass on given module.
  /// M module to optimize,
  /// True if module was modified.
  bool runOnModule(Module &M) override;

private:
  /// Add dummyBarrier at function begin and barrier at function end.
  /// F function to modify.
  void addBarrierCallsToFunctionBody(Function *F);

  /// Remove all fiber instructions from non handled functions.
  /// FunctionsSet container with all handled functions,
  /// M module to optimize.
  void removeFibersFromNonHandledFunctions(FuncSet &FunctionsSet, Module &M);

private:
  /// This is barrier utility class.
  DPCPPKernelBarrierUtils BarrierUtils;
};

} // namespace llvm

#endif // LLVM_TRANSFORMS_INTEL_DPCPP_KERNEL_BARRIER_IN_FUNCTION_PASS_H
