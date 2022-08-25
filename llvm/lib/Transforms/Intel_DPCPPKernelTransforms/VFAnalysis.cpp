//==----------- VFAnalysis.h - Analyze VF related issues -------- C++ -*---==//
//
// Copyright (C) 2021-2022 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //

#include "llvm/Transforms/Intel_DPCPPKernelTransforms/VFAnalysis.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Analysis/CallGraph.h"
#include "llvm/IR/Attributes.h"
#include "llvm/IR/DiagnosticInfo.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Pass.h"
#include "llvm/Support/Debug.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/LegacyPasses.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/Utils/CompilationUtils.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/Utils/MetadataAPI.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/WeightedInstCount.h"
#include <unordered_set>

using namespace llvm;
using namespace CompilationUtils;

#define DEBUG_TYPE "dpcpp-kernel-vf-analysis"

static cl::opt<unsigned> DPCPPForceVF("dpcpp-force-vf", cl::init(0),
                                      cl::ReallyHidden);

bool DPCPPEnableSubGroupEmulation = true;
static cl::opt<bool, true>
    DPCPPEnableSubGroupEmulationOpt("dpcpp-enable-subgroup-emulation",
                                    cl::location(DPCPPEnableSubGroupEmulation),
                                    cl::Hidden,
                                    cl::desc("Enable sub-group emulation"));

bool DPCPPForceOptnone = false;
static cl::opt<bool, true> DPCPPForceOptnoneOpt(
    "dpcpp-force-optnone", cl::location(DPCPPForceOptnone), cl::Hidden,
    cl::desc("Force passes to process functions as if they have the optnone "
             "attribute"));

extern bool DPCPPEnableVectorizationOfByvalByrefFunctions;

DiagnosticKind VFAnalysisDiagInfo::Kind =
    static_cast<DiagnosticKind>(getNextAvailablePluginDiagnosticKind());

extern cl::opt<VFISAKind> IsaEncodingOverride;
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

void VFAnalysisInfo::deduceVF(Function *Kernel, unsigned HeuristicVF) {
  LLVM_DEBUG(dbgs() << "Deducing VF with given constraint:\n");
  DPCPPKernelMetadataAPI::KernelMetadataAPI KMD(Kernel);
  DPCPPKernelMetadataAPI::KernelInternalMetadataAPI KIMD(Kernel);
  CanFallBackToDefaultVF = false;

  // optnone --> disable vectorization
  if (Kernel->hasOptNone() || DPCPPForceOptnone) {
    KernelToVF[Kernel] = 1;
    LLVM_DEBUG(dbgs() << "Initial VF<optnone mode>: " << KernelToVF[Kernel]
                      << '\n');
    return;
  }

  if (KIMD.MaxWGDimensions.hasValue() && KIMD.MaxWGDimensions.get() == 0 &&
      KIMD.NoBarrierPath.get()) {
    // Workgroup loop won't be created, so there is no need to vectorize kernel.
    KernelToVF[Kernel] = 1;
    LLVM_DEBUG(dbgs() << "Initial VF<MaxWGDim is 0>: " << KernelToVF[Kernel]
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

  // Otherwise, get heuristic VF computed by WeightedInstCountAnalysis.
  KernelToVF[Kernel] = HeuristicVF;
  LLVM_DEBUG(dbgs() << "Initial VF<Heuristic>: " << KernelToVF[Kernel] << '\n');
  return;
}

bool VFAnalysisInfo::hasUnsupportedPatterns(Function *Kernel) {
  LLVM_DEBUG(dbgs() << "Checking unsupported patterns:\n");
  CallGraphNode *Node = (*CG)[Kernel];

  // Unsupported: Kernel calls a function with struct or complex return type
  if (hasFunctionCallInCGNodeIf(Node, [](const Function *CalledFunc) {
        return CalledFunc &&
               CalledFunc->hasFnAttribute(KernelAttribute::HasSubGroups) &&
               CalledFunc->getReturnType()->isStructTy();
      })) {
    LLVM_DEBUG(
        dbgs() << "Can't be vectorized - struct return type in callee\n");
    Kernel->getContext().diagnose(VFAnalysisDiagInfo(
        *Kernel,
        "Kernel can't be vectorized due to unsupported struct type return in "
        "callee",
        VFDK_Warn_UnsupportedVectorizationPattern, DS_Warning));
    return true;
  }

  // Unsupported: Kernel calls a function with byval/byref args and
  // - Either the called function contains subgroups.
  // - Or the called function is flaged as "kernel-call-once" (VPlan can't
  //   serialize in this situation).
  if (!DPCPPEnableVectorizationOfByvalByrefFunctions) {
    if (hasFunctionCallInCGNodeIf(Node, [](const Function *CalledFunc) {
          return hasByvalByrefArgs(CalledFunc) &&
                 (CalledFunc->hasFnAttribute(KernelAttribute::HasSubGroups) ||
                  CalledFunc->hasFnAttribute(KernelAttribute::CallOnce));
        })) {
      LLVM_DEBUG(dbgs() << "Can't be vectorized<byval/byref>\n");
      // Users have no control over byval/byref attributes, so we don't report
      // diangostic messages to users here.
      return true;
    }
  }

  // Unsupported: vec_type_hint on unsupported types.
  DPCPPKernelMetadataAPI::KernelMetadataAPI KMD(Kernel);
  if (!KMD.VecLenHint.hasValue() && KMD.VecTypeHint.hasValue()) {
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
      Kernel->getContext().diagnose(VFAnalysisDiagInfo(
          *Kernel,
          "Kernel can't be vectorized due to unsupported vec type hint",
          VFDK_Warn_UnsupportedVectorizationPattern, DS_Warning));
      return true;
    }
  }

  return false;
}

bool VFAnalysisInfo::tryFallbackUnimplementedBuiltins(Function *Kernel,
                                                      unsigned HeuristicVF) {
  LLVM_DEBUG(dbgs() << "Checking unimplemented builtins:\n");
  static std::unordered_set<unsigned> SupportedWorkGroupVFs{1,  4,  8,
                                                            16, 32, 64};
  static std::unordered_set<unsigned> SupportedSubGroupVFs{4, 8, 16, 32, 64};

  DPCPPKernelMetadataAPI::KernelInternalMetadataAPI KIMD(Kernel);
  StringRef FuncName;
  auto IsUnimplementedBuiltin = [&](const Function *CalledFunc) {
    // Builtin must be a declaration.
    unsigned VF = getVF(Kernel);
    if (!(CalledFunc && CalledFunc->isDeclaration()))
      return false;
    FuncName = CalledFunc->getName();
    bool CanEmulate = DPCPPEnableSubGroupEmulation && (VF == 1);
    if ((isSubGroupBuiltin(FuncName) && !SupportedSubGroupVFs.count(VF) &&
         !CanEmulate) ||
        (isWorkGroupBuiltin(FuncName) && !SupportedWorkGroupVFs.count(VF))) {
      if (!CanFallBackToDefaultVF)
        return true;
      // Can fallback to heuristic VF.
      KernelToVF[Kernel] = HeuristicVF;
      Kernel->getContext().diagnose(VFAnalysisDiagInfo(
          *Kernel,
          "Fall back vectorization width to " + Twine(getVF(Kernel)) +
              " due to unsupported vec_len_hint value for workgroup/subgroup "
              "builtins",
          VFDK_Warn_VecLenHintFalledBack, DS_Warning));
    }
    return false;
  };
  bool HasUnimplementedBuiltins = false;
  unsigned VF = getVF(Kernel);
  mapFunctionCallInCGNodeIf(
      (*CG)[Kernel], IsUnimplementedBuiltin, [&](const Function *CalledFunc) {
        UnimplementedBuiltinToVF[CalledFunc->getName()] = VF;
        HasUnimplementedBuiltins = true;
      });
  return HasUnimplementedBuiltins;
}

bool VFAnalysisInfo::isSubgroupBroken(Function *Kernel) {
  LLVM_DEBUG(dbgs() << "Checking subgroup semantics:\n");
  DPCPPKernelMetadataAPI::KernelMetadataAPI KMD(Kernel);
  DPCPPKernelMetadataAPI::KernelInternalMetadataAPI KIMD(Kernel);
  if (!KIMD.KernelHasSubgroups.hasValue() || !KIMD.KernelHasSubgroups.get())
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
          *Kernel, "Required subgroup size can't be 1 for subgroup calls",
          VFDK_Error_SubgroupBroken));
    }
  }

  if (Broken && !DPCPPEnableSubGroupEmulation) {
    LLVM_DEBUG(dbgs() << "Subgroup is broken and emulation is disabled!\n");
    Kernel->getContext().diagnose(VFAnalysisDiagInfo(
        *Kernel, "Subgroup calls in scalar function can't be resolved",
        VFDK_Error_SubgroupBroken));
  }

  return Broken;
}

/// Get default preferred VF according to ISA.
static unsigned getPreferredVectorizationWidth(VFISAKind ISA) {
  switch (ISA) {
  case VFISAKind::SSE:
  case VFISAKind::AVX:
    return 4;
  case VFISAKind::AVX2:
    return 8;
  case VFISAKind::AVX512:
    return 16;
  default:
    llvm_unreachable("unexpected ISA");
  }
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

void VFAnalysisInfo::analyzeModule(
    Module &M, function_ref<unsigned(Function &)> GetHeuristicVF) {
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
          "Only one of CL_CONFIG_CPU_VECTORIZER_MODE, intel_vec_len_hint "
          "and intel_reqd_sub_group_size can be specified",
          VFDK_Error_ConstraintConflict));
    }

    // Deduce VF from the given constraint.
    unsigned HeuristicVF = GetHeuristicVF(*Kernel);
    deduceVF(Kernel, HeuristicVF);

    // Detect VPLAN unsupported patterns.
    if (hasUnsupportedPatterns(Kernel))
      KernelToVF[Kernel] = 1;

    // Detect unimplemented WG/SG builtins.
    if (tryFallbackUnimplementedBuiltins(Kernel, HeuristicVF)) {
      // Map string map range to customized std::string range.
      auto R = map_range(
          UnimplementedBuiltinToVF, [&](const StringMapEntry<unsigned> &Entry) {
            return Entry.getKey().str() + " with vectorization width " +
                   std::to_string(Entry.getValue());
          });
      M.getContext().diagnose(VFAnalysisDiagInfo(
          *Kernel, "Unimplemented function(s): " + join(R, ", "),
          VFDK_Error_BuiltinNotImplemented));
    }

    // Deduce KernelToSGEmuSize.
    deduceSGEmulationSize(Kernel);

    // Confirm VF.
    unsigned VF = getVF(Kernel);
    if (!(VF && (VF & (VF - 1)) == 0)) {
      M.getContext().diagnose(
          VFAnalysisDiagInfo(*Kernel, "Vectorization width is not a power of 2",
                             VFDK_Error_VFInvalid));
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

INITIALIZE_PASS_BEGIN(VFAnalysisLegacy, DEBUG_TYPE, DEBUG_TYPE, /*cfg*/ false,
                      /*analysis*/ true)
INITIALIZE_PASS_DEPENDENCY(WeightedInstCountAnalysisLegacy)
INITIALIZE_PASS_END(VFAnalysisLegacy, DEBUG_TYPE, DEBUG_TYPE, /*cfg*/ false,
                    /*analysis*/ true)

ModulePass *llvm::createVFAnalysisLegacyPass() {
  return new VFAnalysisLegacy();
}

char VFAnalysisLegacy::ID = 0;

bool VFAnalysisLegacy::runOnModule(Module &M) {
  auto GetHeuristicVF = [&](Function &F) {
    return getAnalysis<WeightedInstCountAnalysisLegacy>(F)
        .getResult()
        .getDesiredVF();
  };
  Result.analyzeModule(M, GetHeuristicVF);
  return false;
}

void VFAnalysisLegacy::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.addRequired<WeightedInstCountAnalysisLegacy>();
  AU.setPreservesAll();
}

void VFAnalysisLegacy::print(raw_ostream &OS, const Module *M) const {
  getResult().print(OS);
}

// Provide a definition for the static class member used to identify passes.
AnalysisKey VFAnalysis::Key;

VFAnalysisInfo VFAnalysis::run(Module &M, ModuleAnalysisManager &AM) {
  FunctionAnalysisManager &FAM =
      AM.getResult<FunctionAnalysisManagerModuleProxy>(M).getManager();
  auto GetHeuristicVF = [&FAM](Function &F) {
    return FAM.getResult<WeightedInstCountAnalysis>(F).getDesiredVF();
  };
  VFAnalysisInfo Result;
  Result.analyzeModule(M, GetHeuristicVF);
  return Result;
}

PreservedAnalyses VFAnalysisPrinter::run(Module &M, ModuleAnalysisManager &AM) {
  AM.getResult<VFAnalysis>(M).print(OS);
  return PreservedAnalyses::all();
}
