//==--- DPCPPKernelAnalysis.h - Detect barriers in DPCPP kernels- C++ -*---==//
//
// Copyright (C) 2020 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //

#ifndef LLVM_TRANSFORMS_INTEL_DPCPP_KERNEL_TRANSFORMS_KERNEL_ANALYSIS_H
#define LLVM_TRANSFORMS_INTEL_DPCPP_KERNEL_TRANSFORMS_KERNEL_ANALYSIS_H

#include "llvm/IR/PassManager.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/DPCPPKernelCompilationUtils.h"

namespace llvm {

// This class implements a pass that recieves a module and assigns each function
// an attribute indicating whether a kernel will take WGLoopCreator
// path or Barrier path.
// kernels can take WGLoopCreator if the following conditions
// are met:
// 1. does not have a barrier path.
// 2. does not contain a variable get***id call (although can be handled)
// 3. does not contain call to kernel or called by another kernel.
// 4. does not contain call to function that use get***id calls.
// both 3,4 should be eliminated by the inliner.
class DPCPPKernelAnalysisPass : PassInfoMixin<DPCPPKernelAnalysisPass> {
public:
  static StringRef name() { return "DPCPPKernelAnalysisPass"; }

  PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM);

  /// Glue for old PM.
  bool runImpl(Module &M);

  void print(raw_ostream &OS, const Module *M) const;

private:
  using FuncVec = SmallVector<Function *, 8>;
  using FuncSet = DPCPPKernelCompilationUtils::FuncSet;

  /// Returns true iff the Value is constant int < 3
  /// V - value to check.
  bool isUnsupportedDim(Value *V);

  /// Fills the unsupported set with function that call (also indirectly)
  /// barrier (or implemented using barrier).
  void fillSyncUsersFuncs();

  /// Fills the unsupported set with function that have non constant
  /// dimension get***id calls, or indirect calls to get***id.
  void fillUnsupportedTIDFuncs();

  /// Fills the unsupported set with function that have non constant
  /// dimension get***id calls, or indirect calls to get***id.
  void fillKernelCallers();

  /// Fills the unsupported set with function that have non constant
  /// dimension get***id calls, and gets the direct users of the call
  /// into TIDUsers.
  /// Name - name of get***id.
  /// DirectTIDUsers - set of direct get***id users.
  void fillUnsupportedTIDFuncs(StringRef Name, FuncSet &DirectTIDUsers);

  /// Current module.
  Module *M;

  /// Kernels.
  FuncSet Kernels;

  /// Set of unsupported funcs.
  FuncSet UnsupportedFuncs;
};

} // namespace llvm

#endif // LLVM_TRANSFORMS_INTEL_DPCPP_KERNEL_TRANSFORMS_KERNEL_ANALYSIS_H
