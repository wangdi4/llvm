//==----------- VFAnalysis.h - Analyze VF related issues -------- C++ -*---==//
//
// Copyright (C) 2021-2022 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //

#ifndef LLVM_TRANSFORMS_INTEL_DPCPP_KERNEL_TRANSFORMS_VF_ANALYSIS_H
#define LLVM_TRANSFORMS_INTEL_DPCPP_KERNEL_TRANSFORMS_VF_ANALYSIS_H

#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/StringMap.h"
#include "llvm/Analysis/CallGraph.h"
#include "llvm/Analysis/VectorUtils.h"
#include "llvm/IR/DiagnosticHandler.h"
#include "llvm/IR/DiagnosticInfo.h"
#include "llvm/IR/DiagnosticPrinter.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/PassManager.h"
#include "llvm/InitializePasses.h"
#include "llvm/Support/raw_ostream.h"

namespace llvm {

/// Checks all vectorization factor related issues for kernels.
/// The flow is:
/// 1. hasMultipleVFConstraints(): Check whether there is more than one VF
///    constraint. If more than one of ForceVF, intel_vec_len_hint
///    and intel_reqd_sub_group_size is defined, emit a VFAnalysisDiagInfo of
///    VFKD_ConstraintConflict kind, which should be handled outside the
///    optimizer.
/// 2. deduceVF(): Deduce initial VF according to given constraints. Initial
///    means this VF may be fallbacked. Currently just let intel_vec_len_hint
///    can be fallbacked.
/// 3. hasUnsupportedPatterns(): Check unsupported patterns in the kernel:
///    - Non-kernel function with byval/byref parameters and
///      "has-sub-groups"/"kernel-call-once" attribute.
///    - vec_type_hint on unsupported types.
///    Some other checks may also need to be added the list.
///    If there patterns are found, the kernel can't be vectorized and VF
///    can't be fallbacked.
/// 4. tryFallbackUnimplementedBuiltins(): Tries to fallback WG/SG builtins with
///    unimplemented VF to default VF. If failed, emit a VFAnalysisDiagInfo of
///    VFDK_BuiltinNotImplemented kind, which should be handled outside the
///    optimizer.
/// 5. deduceSGEmulationSize(): Deduce subgroup emulation size.
///    If kernel can't be vectorized, i.e subgroup semantics is broken:
///    - Either backup with subgroup emulation;
///    - Or emit a VFAnalaysisDiagInfo of VFDK_SubgroupBroken kind, which should
///      be handled outside the optimizer.
///    Otherwise, kernel can be vectorized. No need to do emulation.
class VFAnalysisInfo {
public:
  VFAnalysisInfo();

  void analyzeModule(Module &M,
                     function_ref<unsigned(Function &)> GetHeuristicVF);

  void print(raw_ostream &OS) const;

  unsigned getVF(Function *Kernel) const {
    assert(KernelToVF.count(Kernel));
    return KernelToVF.lookup(Kernel);
  }

  /// If the return value is 0, it means the kernel doesn't need to be emulated.
  unsigned getSubgroupEmulationSize(Function *Kernel) const {
    assert(KernelToSGEmuSize.count(Kernel));
    return KernelToSGEmuSize.lookup(Kernel);
  }

private:
  /// Returns whether `ForceVF` takes effect: when `ForceVF == 0`, it has no
  /// effect.
  bool isVFForced() { return ForceVF != 0; }

  /// Checks whether multiple of the following VF constraints are specified:
  /// - ForceVF
  /// - "intel_vec_len_hint" metadata
  /// - "intel_reqd_sub_group_size" metadata
  bool hasMultipleVFConstraints(Function *Kernel);

  /// Deduces VF according to the given constriant. Stores "Kernel" -->
  /// "VF" mapping into `KernelToVF`.
  /// - HeuristicVF Heuristic VF computed by WeightedInstCountAnalysis.
  void deduceVF(Function *Kernel, unsigned HeuristicVF);

  /// Checks whether the function has pattern that can't be vectorized:
  /// - Non-kernel function with byval/byref parameters and "has-sub-groups"
  ///   attribute.
  /// - Non-kernel function with byval/byref parameters and "kernel-call-once"
  ///   attribute.
  /// - vec_type_hint on unsupported types.
  bool hasUnsupportedPatterns(Function *Kernel);

  /// Tries to fallback WG/SG builtins with unimplemented VF to heuristic VF.
  /// Returns true if fallbacked successfully or there's no unimplemented
  /// builtins. Returns false on fallback failure, and records unimplemented
  /// builtins.
  bool tryFallbackUnimplementedBuiltins(Function *Kernel, unsigned HeuristicVF);

  /// Checks whether subgroup semantics is broken due to VF = 1.
  bool isSubgroupBroken(Function *Kernel);

  /// Deduces SGEmuSize according to VF and subgroup semantics.
  void deduceSGEmulationSize(Function *Kernel);

  /// ISA is used to deduce the default initial VF.
  VFISAKind ISA;

  /// Forced VF value. Has no effect when equals to zero.
  unsigned ForceVF;

  /// Indicates whether VF can be falled-back from "intel_vec_len_hint" to
  /// default VF.
  bool CanFallBackToDefaultVF;

  /// Unimplemented builtin name to VF mapping.
  StringMap<unsigned> UnimplementedBuiltinToVF;

  /// Kernel to VF mapping.
  SmallDenseMap<Function *, unsigned, 4> KernelToVF;

  /// Kernel to subgroup emulation size mapping.
  SmallDenseMap<Function *, unsigned, 4> KernelToSGEmuSize;

  /// CallGraph of the module.
  std::unique_ptr<CallGraph> CG;
};

enum VFAnalysisDiagKind {
  VFDK_Error_ConstraintConflict,
  VFDK_Error_BuiltinNotImplemented,
  VFDK_Error_SubgroupBroken,
  VFDK_Error_VFInvalid,
  VFDK_Warn_UnsupportedVectorizationPattern,
  VFDK_Warn_VecLenHintFalledBack
};

class VFAnalysisDiagInfo : public DiagnosticInfoWithLocationBase {
  const Twine &Msg;
  const VFAnalysisDiagKind VFDiagKind;

public:
  static DiagnosticKind Kind;

  VFAnalysisDiagInfo(const Function &F, const Twine &Msg,
                     VFAnalysisDiagKind VFDiagKind,
                     DiagnosticSeverity Severity = DS_Error)
      : DiagnosticInfoWithLocationBase(Kind, Severity, F, DiagnosticLocation()),
        Msg(Msg), VFDiagKind(VFDiagKind) {}

  static bool classof(const DiagnosticInfo *DI) {
    return DI->getKind() == Kind;
  }

  void print(DiagnosticPrinter &DP) const override {
    DP << "kernel \"" << getFunction().getName() << "\": " << Msg;
  }

  VFAnalysisDiagKind getVFDiagKind() const { return VFDiagKind; }
};

class VFAnalysis : public AnalysisInfoMixin<VFAnalysis> {
public:
  using Result = VFAnalysisInfo;

  VFAnalysisInfo run(Module &M, ModuleAnalysisManager &AM);

private:
  friend AnalysisInfoMixin<VFAnalysis>;

  static AnalysisKey Key;
};

/// Printer pass for VFAnalysis
class VFAnalysisPrinter : public PassInfoMixin<VFAnalysisPrinter> {
public:
  explicit VFAnalysisPrinter(raw_ostream &OS) : OS(OS) {}

  PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM);

private:
  raw_ostream &OS;
};

/// Legacy VFAnalysis module pass.
class VFAnalysisLegacy : public ModulePass {
private:
  VFAnalysisInfo Result;

public:
  static char ID;

  VFAnalysisLegacy() : ModulePass(ID) {
    initializeVFAnalysisLegacyPass(*PassRegistry::getPassRegistry());
  }

  StringRef getPassName() const override { return "VFAnalysisLegacy"; }

  bool runOnModule(Module &M) override;

  void getAnalysisUsage(AnalysisUsage &AU) const override;

  const VFAnalysisInfo &getResult() const { return Result; }

  void print(raw_ostream &OS, const Module *M) const override;
};
} // namespace llvm

#endif // LLVM_TRANSFORMS_INTEL_DPCPP_KERNEL_TRANSFORMS_VF_ANALYSIS_H
