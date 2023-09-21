//==--- SYCLKernelAnalysis.h - Analyze SYCL kernel properties -- C++ -*---==//
//
// Copyright (C) 2020 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //

#ifndef LLVM_TRANSFORMS_SYCLTRANSFORMS_KERNEL_ANALYSIS_H
#define LLVM_TRANSFORMS_SYCLTRANSFORMS_KERNEL_ANALYSIS_H

#include "llvm/Analysis/CallGraph.h"
#include "llvm/IR/DiagnosticHandler.h"
#include "llvm/IR/DiagnosticInfo.h"
#include "llvm/IR/DiagnosticPrinter.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Transforms/SYCLTransforms/Utils/CompilationUtils.h"

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
class SYCLKernelAnalysisPass : public PassInfoMixin<SYCLKernelAnalysisPass> {
public:
  SYCLKernelAnalysisPass(bool IsAMX = false, bool IsAMXFP16 = false)
      : IsAMX(IsAMX), IsAMXFP16(IsAMXFP16) {}
  PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM);

  /// Glue for old PM.
  bool runImpl(Module &M, CallGraph &CG, const RuntimeService &RTS,
               function_ref<LoopInfo &(Function &)> GetLI);

  void print(raw_ostream &OS, const Module *M) const;

  static bool isRequired() { return true; }

private:
  using FuncVec = SmallVector<Function *, 8>;
  using FuncSet = CompilationUtils::FuncSet;

  /// Fills the unsupported set with function that call (also indirectly)
  /// barrier (or implemented using barrier).
  void fillSyncUsersFuncs();

  /// Fills the unsupported set with function that have non constant
  /// dimension get***id calls, or indirect calls to get***id.
  void fillKernelCallers();

#if INTEL_CUSTOMIZATION
  /// Fills the  set with function that uses matrix call directly or indirectly.
  void fillMatrixCallFuncs();
#endif // INTEL_CUSTOMIZATION

  /// Fills the subgroup-calling function set -- functions containing subroup
  /// builtins or subgroup barrier.
  void fillSubgroupCallingFuncs(CallGraph &CG);

  /// Current module.
  Module *M = nullptr;

  bool IsAMX;

  bool IsAMXFP16;

  /// Kernels.
  FuncSet Kernels;

  /// Set of unsupported funcs.
  FuncSet UnsupportedFuncs;

#if INTEL_CUSTOMIZATION
  /// Set of funcs use matrix call directly or indirectly.
  FuncSet MatrixCallFuncs;
#endif // INTEL_CUSTOMIZATION

  /// Set of funcs containing subgroup builtins.
  FuncSet SubgroupCallingFuncs;
};

enum SYCLKernelAnalysisDiagKind {
  DKDK_Error_MatrixIntrinOnUnsupportedCPU,
};

class SYCLKernelAnalysisDiagInfo : public DiagnosticInfoWithLocationBase {
  const Twine &Msg;
  const SYCLKernelAnalysisDiagKind DKDiagKind;

public:
  static DiagnosticKind Kind;

  SYCLKernelAnalysisDiagInfo(const Function &F, const Twine &Msg,
                             SYCLKernelAnalysisDiagKind DKDiagKind,
                             DiagnosticSeverity Severity = DS_Error)
      : DiagnosticInfoWithLocationBase(Kind, Severity, F, DiagnosticLocation()),
        Msg(Msg), DKDiagKind(DKDiagKind) {}

  static bool classof(const DiagnosticInfo *DI) {
    return DI->getKind() == Kind;
  }

  void print(DiagnosticPrinter &DP) const override {
    DP << getFunction().getName() << "\": " << Msg;
  }

  SYCLKernelAnalysisDiagKind getDKDiagKind() const { return DKDiagKind; }
};

} // namespace llvm

#endif // LLVM_TRANSFORMS_SYCLTRANSFORMS_KERNEL_ANALYSIS_H
