//==----------- VFAnalysis.h - Analyze VF related issues -------- C++ -*---==//
//
// Copyright (C) 2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //

#include "llvm/Transforms/Intel_DPCPPKernelTransforms/VFAnalysis.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Analysis/CallGraph.h"
#include "llvm/Analysis/Intel_VectorVariant.h"
#include "llvm/IR/Attributes.h"
#include "llvm/IR/DiagnosticInfo.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Pass.h"
#include "llvm/Support/Debug.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/DPCPPKernelCompilationUtils.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/LegacyPasses.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/Utils/MetadataAPI.h"
#include <unordered_set>

using namespace llvm;
using namespace DPCPPKernelCompilationUtils;

#define DEBUG_TYPE "dpcpp-kernel-vf-analysis"

static cl::opt<unsigned> DPCPPForceVF("dpcpp-force-vf", cl::init(0),
                                      cl::ReallyHidden);

// TODO: Enable subgroup emulation when related passes are ported.
bool DPCPPEnableSubGroupEmulation = false;
static cl::opt<bool, true>
    DPCPPEnableSubGroupEmulationOpt("dpcpp-enable-subgroup-emulation",
                                    cl::location(DPCPPEnableSubGroupEmulation),
                                    cl::Hidden,
                                    cl::desc("Enable sub-group emulation"));

extern bool DPCPPEnableVectorizationOfByvalByrefFunctions;

DiagnosticKind VFAnalysisDiagInfo::Kind =
    static_cast<DiagnosticKind>(getNextAvailablePluginDiagnosticKind());

extern cl::opt<VectorVariant::ISAClass> IsaEncodingOverride;
// Always get ISA, ForceVF from the global options.
// So that we could pass parameters to VFAnalysisInfo.
VFAnalysisInfo::VFAnalysisInfo()
    : ISA(IsaEncodingOverride.getValue()), ForceVF(DPCPPForceVF.getValue()),
      CanFallBackToDefaultVF(false) {}

bool VFAnalysisInfo::hasMultipleVFConstraints(Function *Kernel) {
  DPCPPKernelMetadataAPI::KernelMetadataAPI KMD(Kernel);
  bool MultiConstraint =
      (KMD.VecLenHint.hasValue() && KMD.ReqdIntelSGSize.hasValue()) ||
      (isVFForced() && KMD.hasVecLength());
  LLVM_DEBUG(dbgs() << "Function <" << Kernel->getName()
                    << "> MultiConstraint = " << MultiConstraint << '\n');
  return MultiConstraint;
}

/// Get default preferred VF according to ISA.
/// TODO: Future heuristics could be implemented here.
static unsigned getPreferredVectorizationWidth(VectorVariant::ISAClass ISA) {
  switch (ISA) {
  case VectorVariant::XMM:
  case VectorVariant::YMM1:
    return 4;
  case VectorVariant::YMM2:
    return 8;
  case VectorVariant::ZMM:
    return 16;
  default:
    llvm_unreachable("unexpected ISA");
  }
}

void VFAnalysisInfo::deduceVF(Function *Kernel) {
  LLVM_DEBUG(dbgs() << "Deducing VF with given constraint:\n");
  DPCPPKernelMetadataAPI::KernelMetadataAPI KMD(Kernel);
  DPCPPKernelMetadataAPI::KernelInternalMetadataAPI KIMD(Kernel);
  CanFallBackToDefaultVF = false;

  // optnone --> disable vectorization
  if (Kernel->hasOptNone()) {
    KernelToVF[Kernel] = 1;
    LLVM_DEBUG(dbgs() << "Initial VF<optnone mode>: " << KernelToVF[Kernel]
                      << '\n');
    return;
  }

  // Allow intel_vec_len_hint and intel_reqd_sub_group_size overriding default
  // VF.
  if (KMD.hasVecLength()) {
    KernelToVF[Kernel] = KMD.getVecLength();
    // For intel_vec_len_hint, allow falling back to default VF.
    if (KMD.VecLenHint.hasValue())
      CanFallBackToDefaultVF = true;

    LLVM_DEBUG(dbgs() << "Initial VF<From VecLength>: " << KernelToVF[Kernel]
                      << '\n');
    return;
  }

  if (isVFForced()) {
    KernelToVF[Kernel] = ForceVF;
    LLVM_DEBUG(dbgs() << "Initial VF<From ForceVF>: " << KernelToVF[Kernel]
                      << '\n');
    return;
  }

  // Otherwise, get default VF determined by ISA.
  KernelToVF[Kernel] = getPreferredVectorizationWidth(ISA);
  LLVM_DEBUG(dbgs() << "Initial VF<From ISA>: " << KernelToVF[Kernel] << '\n');
  return;
}

bool VFAnalysisInfo::hasUnsupportedPatterns(Function *Kernel) {
  LLVM_DEBUG(dbgs() << "Checking unsupported patterns:\n");
  CallGraphNode *Node = (*CG)[Kernel];

  // Unsupported: Kernel calls a function with byval/byref args and
  // - Either the called function contains subgroups.
  // - Or the called function is flaged as "kernel-call-once" (VPlan can't
  //   serialize in this situation).
  if (!DPCPPEnableVectorizationOfByvalByrefFunctions) {
    if (hasFunctionCallInCGNodeSatisfiedWith(Node, [](Function *CalledFunc) {
          return hasByvalByrefArgs(CalledFunc) &&
                 (CalledFunc->hasFnAttribute(KernelAttribute::HasSubGroups) ||
                  CalledFunc->hasFnAttribute(KernelAttribute::CallOnce));
        })) {
      LLVM_DEBUG(dbgs() << "Can't be vectorized<byval/byref>\n");
      return true;
    }
  }

  // Unsupported: vec_type_hint on unsupported types.
  DPCPPKernelMetadataAPI::KernelMetadataAPI KMD(Kernel);
  if (KMD.VecTypeHint.hasValue()) {
    bool Vectorizable = false;
    auto *HintType = KMD.VecTypeHint.getType();
    // Supported types:
    // - float, double
    // - i8, i16, i32, i64
    switch (HintType->getTypeID()) {
    default:
      break;
    case Type::FloatTyID:
    case Type::DoubleTyID:
      Vectorizable = true;
      break;
    case Type::IntegerTyID:
      switch (cast<IntegerType>(HintType)->getBitWidth()) {
      default:
        break;
      case 8:
      case 16:
      case 32:
      case 64:
        Vectorizable = true;
      };
    }
    if (!Vectorizable) {
      LLVM_DEBUG(dbgs() << "Can't be vectorized<VecTypeHint>\n");
      return true;
    }
  }

  return false;
}

bool VFAnalysisInfo::tryFallbackUnimplementedBuiltins(Function *Kernel) {
  LLVM_DEBUG(dbgs() << "Checking unimplemented builtins:\n");
  static std::unordered_set<unsigned> SupportedWorkGroupVFs{1,  4,  8,
                                                            16, 32, 64};
  static std::unordered_set<unsigned> SupportedSubGroupVFs{1, 4, 8, 16, 32, 64};
  if (!DPCPPEnableSubGroupEmulation)
    SupportedSubGroupVFs.erase(1);

  DPCPPKernelMetadataAPI::KernelInternalMetadataAPI KIMD(Kernel);
  unsigned VF = getVF(Kernel);
  StringRef FuncName;
  auto IsUnimplementedBuiltin = [&](Function *CalledFunc) {
    // Builtin must be a declaration.
    if (!(CalledFunc && CalledFunc->isDeclaration()))
      return false;
    FuncName = CalledFunc->getName();
    if ((isSubGroupBuiltin(FuncName) && SupportedSubGroupVFs.count(VF) == 0) ||
        (isWorkGroupBuiltin(FuncName) &&
         SupportedWorkGroupVFs.count(VF) == 0)) {
      if (!CanFallBackToDefaultVF)
        return true;
      // Can fallback to default VF.
      KernelToVF[Kernel] = getPreferredVectorizationWidth(ISA);
      LLVM_DEBUG(dbgs() << "Kernel <" << Kernel->getName()
                        << "> VF fall back to " << getVF(Kernel)
                        << " due to unsupported WG/SG width\n");
    }
    return false;
  };
  if (hasFunctionCallInCGNodeSatisfiedWith((*CG)[Kernel],
                                           IsUnimplementedBuiltin)) {
    UnimplementedBuiltinToVF[FuncName] = VF;
    LLVM_DEBUG(dbgs() << "VF = " << VF << " is unsupported for builtin <"
                      << FuncName << ">\n");
    return true;
  }
  return false;
}

bool VFAnalysisInfo::isSubgroupBroken(Function *Kernel) {
  LLVM_DEBUG(dbgs() << "Checking subgroup semantics:\n");
  DPCPPKernelMetadataAPI::KernelMetadataAPI KMD(Kernel);
  DPCPPKernelMetadataAPI::KernelInternalMetadataAPI KIMD(Kernel);
  if (!KIMD.KernelHasSubgroups.hasValue())
    return false;

  bool Broken = false;
  if (getVF(Kernel) == 1) {
    LLVM_DEBUG(dbgs() << "subgroup is broken<VF = 1>\n");
    Broken = true;
    // Even if sub-group emulation is enabled, intel_reqd_sub_group_size
    // can't be 1 because we only support sub-group size 4, 8, 16, 32, 64.
    // intel_reqd_sub_group_size is different from ForceVF and
    // intel_vec_len_hint, it must match the implementation of sub-group
    // built-ins while the latter two don't have this limitation.
    if (KMD.ReqdIntelSGSize.hasValue() && KMD.ReqdIntelSGSize.get() == 1) {
      Kernel->getContext().diagnose(VFAnalysisDiagInfo(
          *Kernel, "Subgroup is broken!", VFDK_SubgroupBroken));
    }
  }

  if (Broken && !DPCPPEnableSubGroupEmulation) {
    LLVM_DEBUG(dbgs() << "Subgroup is broken and emulation is disabled!\n");
    Kernel->getContext().diagnose(VFAnalysisDiagInfo(
        *Kernel, "Subgroup is broken!", VFDK_SubgroupBroken));
  }

  return Broken;
}

void VFAnalysisInfo::deduceSGEmulationSize(Function *Kernel) {
  KernelToSGEmuSize[Kernel] = KernelToVF[Kernel];
  LLVM_DEBUG(dbgs() << "Initial SGEmuSize: " << KernelToSGEmuSize[Kernel]
                    << '\n');

  // Explicitly disable vectorization by setting VF = 1.
  if (isSubgroupBroken(Kernel))
    KernelToVF[Kernel] = 1;
  else // No need to do emulation.
    KernelToSGEmuSize[Kernel] = 0;

  // Relax subgroup emulation size, since we don't have scalar implementation
  // for subgroup builtins.
  if (KernelToSGEmuSize[Kernel] == 1) {
    DPCPPKernelMetadataAPI::KernelMetadataAPI KMD(Kernel);
    DPCPPKernelMetadataAPI::KernelInternalMetadataAPI KIMD(Kernel);
    if (KMD.hasVecLength() && KMD.getVecLength() > 1)
      KernelToSGEmuSize[Kernel] = KMD.getVecLength();
    else
      KernelToSGEmuSize[Kernel] = getPreferredVectorizationWidth(ISA);
    LLVM_DEBUG(dbgs() << "Get SGEmuSize from default VF: "
                      << KernelToSGEmuSize[Kernel] << '\n');
  }

  if (!DPCPPEnableSubGroupEmulation) {
    LLVM_DEBUG(dbgs() << "Subgroup emulation disabled.\n");
    KernelToSGEmuSize[Kernel] = 0;
  }

  assert(KernelToSGEmuSize[Kernel] != 1 &&
         "Subgroup emulation size = 1 is not supported.");
}

void VFAnalysisInfo::analyzeModule(Module &M) {
  CG = std::make_unique<CallGraph>(M);

  auto Kernels = DPCPPKernelMetadataAPI::KernelList(M).getList();
  for (auto *Kernel : Kernels) {
    DPCPPKernelMetadataAPI::KernelMetadataAPI KMD(Kernel);
    LLVM_DEBUG(dbgs() << "\nProcessing " << Kernel->getName() << '\n');

    // Only one VF constraint can be specified.
    // If not, emit a DiagInfo, which can be handled outside the optimizer by
    // a DiagnosticHandler.
    if (hasMultipleVFConstraints(Kernel)) {
      M.getContext().diagnose(VFAnalysisDiagInfo(
          *Kernel,
          "Only allow specifying one of ForceVF, intel_vec_len_hint "
          "and intel_reqd_sub_group_size!",
          VFDK_ConstraintConflict));
    }

    // Deduce VF from the given constraint.
    deduceVF(Kernel);

    // Detect VPLAN unsupported patterns.
    if (hasUnsupportedPatterns(Kernel))
      KernelToVF[Kernel] = 1;

    // Detect unimplemented WG/SG builtins.
    if (tryFallbackUnimplementedBuiltins(Kernel)) {
      M.getContext().diagnose(VFAnalysisDiagInfo(
          *Kernel, "Unimplemented workgroup/subgroup builtin!",
          VFDK_BuiltinNotImplemented));
    }

    // Deduce KernelToSGEmuSize.
    deduceSGEmulationSize(Kernel);

    // Confirm VF.
    unsigned VF = getVF(Kernel);
    if (!(VF && (VF & (VF - 1)) == 0)) {
      M.getContext().diagnose(
          VFAnalysisDiagInfo(*Kernel, "VF is not power of 2", VFDK_VFInvalid));
    }
  }
}

void VFAnalysisInfo::print(raw_ostream &OS) const {
  OS << "Kernel --> VF:\n";
  for (auto P : KernelToVF)
    OS << "  <" << P.getFirst()->getName() << "> : " << P.getSecond() << '\n';

  OS << "Kernel --> SGEmuSize:\n";
  for (auto P : KernelToSGEmuSize)
    OS << "  <" << P.getFirst()->getName() << "> : " << P.getSecond() << '\n';
}

INITIALIZE_PASS(VFAnalysisLegacy, DEBUG_TYPE, DEBUG_TYPE, /*cfg*/ false,
                /*analysis*/ true)

ModulePass *llvm::createVFAnalysisLegacyPass() {
  return new VFAnalysisLegacy();
}

char VFAnalysisLegacy::ID = 0;

void VFAnalysisLegacy::print(raw_ostream &OS, const Module *M) const {
  getResult().print(OS);
}

// Provide a definition for the static class member used to identify passes.
AnalysisKey VFAnalysis::Key;

VFAnalysisInfo VFAnalysis::run(Module &M, ModuleAnalysisManager &) {
  VFAnalysisInfo Result;
  Result.analyzeModule(M);
  return Result;
}

PreservedAnalyses VFAnalysisPrinter::run(Module &M, ModuleAnalysisManager &AM) {
  AM.getResult<VFAnalysis>(M).print(OS);
  return PreservedAnalyses::all();
}
