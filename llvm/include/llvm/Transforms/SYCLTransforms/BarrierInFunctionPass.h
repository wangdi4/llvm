//==--- BarrierInFunction.h - BarrierInFunction - C++ -*--------------------==//
//
// Copyright (C) 2020 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //

#ifndef LLVM_TRANSFORMS_SYCLTRANSFORMS_BARRIER_IN_FUNCTION_PASS_H
#define LLVM_TRANSFORMS_SYCLTRANSFORMS_BARRIER_IN_FUNCTION_PASS_H

#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Transforms/SYCLTransforms/Utils/BarrierUtils.h"

namespace llvm {

class BarrierUtils;

/// Barrier in function pass is a module pass that handles barrier/fiber
/// instructions called from non-kernel functions.
/// For each such function with a barrier add dummyBarrier call at its begin
/// and barrier call at its end, also add barrier call before each call to this
/// function and dummyBarrier call after each call to it (repeate this till
/// handling all functions). For each kernel add dummyBarrier call at its begin
/// and barrier call at its end.
/// Remove each fiber instruction that is called from function that has no
/// barrier.
class BarrierInFunction : public PassInfoMixin<BarrierInFunction> {
public:
  bool runImpl(Module &M);

  PreservedAnalyses run(Module &M, ModuleAnalysisManager &MAM);

  static bool isRequired() { return true; }

private:
  /// Add dummyBarrier at function begin and barrier at function end.
  /// F function to modify.
  void addBarrierCallsToFunctionBody(Function *F);

private:
  /// This is barrier utility class.
  BarrierUtils Utils;
};

} // namespace llvm

#endif // LLVM_TRANSFORMS_SYCLTRANSFORMS_BARRIER_IN_FUNCTION_PASS_H
