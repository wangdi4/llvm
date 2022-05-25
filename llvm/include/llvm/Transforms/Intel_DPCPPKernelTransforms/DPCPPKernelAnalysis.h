//==--- DPCPPKernelAnalysis.h - Analyze DPCPP kernel properties -- C++ -*---==//
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

#include "llvm/Analysis/CallGraph.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/DPCPPKernelCompilationUtils.h"

namespace llvm {
class LoopInfo;
class RuntimeService;

/// This pass analyze kernel properties and adds function attribute or metadata.
///   1. a metadata indicating whether a kernel will take WGLoopCreator
///      path or Barrier path.
///   2. an attribute and metadata whether function/kernel contains subgroup
///      builtin.
///   3. a metadata indicating there is global atomic in the kernel
///   4. a metadata indicating execution length, which is an estimated length of
///      instructions in the kernel.
class DPCPPKernelAnalysisPass : public PassInfoMixin<DPCPPKernelAnalysisPass> {
public:
  static StringRef name() { return "DPCPPKernelAnalysisPass"; }

  PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM);

  /// Glue for old PM.
  bool runImpl(Module &M, CallGraph &CG, const RuntimeService *RTS,
               function_ref<LoopInfo &(Function &)> GetLI);

  void print(raw_ostream &OS, const Module *M) const;

private:
  using FuncVec = SmallVector<Function *, 8>;
  using FuncSet = DPCPPKernelCompilationUtils::FuncSet;

  /// Fills the unsupported set with function that call (also indirectly)
  /// barrier (or implemented using barrier).
  void fillSyncUsersFuncs();

  /// Fills the unsupported set with function that have non constant
  /// dimension get***id calls, or indirect calls to get***id.
  void fillKernelCallers();

  /// Fills the subgroup-calling function set -- functions containing subroup
  /// builtins or subgroup barrier.
  void fillSubgroupCallingFuncs(CallGraph &CG);

  /// Current module.
  Module *M;

  /// Kernels.
  FuncSet Kernels;

  /// Set of unsupported funcs.
  FuncSet UnsupportedFuncs;

  /// Set of funcs containing subgroup builtins.
  FuncSet SubgroupCallingFuncs;
};

} // namespace llvm

#endif // LLVM_TRANSFORMS_INTEL_DPCPP_KERNEL_TRANSFORMS_KERNEL_ANALYSIS_H
