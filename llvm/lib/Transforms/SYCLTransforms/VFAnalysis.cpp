//==----------- VFAnalysis.h - Analyze VF related issues -------- C++ -*---==//
//
// Copyright (C) 2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //

#include "llvm/Transforms/SYCLTransforms/VFAnalysis.h"
#include "llvm/ADT/DepthFirstIterator.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Analysis/CallGraph.h"
#include "llvm/IR/Attributes.h"
#include "llvm/IR/DiagnosticInfo.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Transforms/SYCLTransforms/Utils/CompilationUtils.h"
#include "llvm/Transforms/SYCLTransforms/Utils/MetadataAPI.h"
#include "llvm/Transforms/SYCLTransforms/WeightedInstCount.h"
#include <unordered_set>

using namespace llvm;
using namespace CompilationUtils;
using namespace SYCLKernelMetadataAPI;

#define DEBUG_TYPE "sycl-kernel-vf-analysis"

static cl::opt<unsigned> SYCLForceVF("sycl-force-vf", cl::init(0),
                                      cl::ReallyHidden);

bool SYCLEnableSubGroupEmulation = true;
static cl::opt<bool, true>
    SYCLEnableSubGroupEmulationOpt("sycl-enable-subgroup-emulation",
                                    cl::location(SYCLEnableSubGroupEmulation),
                                    cl::Hidden,
                                    cl::desc("Enable sub-group emulation"));

#if INTEL_CUSTOMIZATION
// Enable vectorization at O0 optimization level.
cl::opt<bool> SYCLEnableO0Vectorization(
    "sycl-enable-o0-vectorization", cl::init(false), cl::Hidden,
    cl::desc("Enable vectorization at O0 optimization level"));

extern bool SYCLEnableVectorizationOfByvalByrefFunctions;
#endif // INTEL_CUSTOMIZATION

DiagnosticKind VFAnalysisDiagInfo::Kind =
    static_cast<DiagnosticKind>(getNextAvailablePluginDiagnosticKind());

#if !INTEL_CUSTOMIZATION
cl::opt<VFISAKind> IsaEncodingOverride(
    "sycl-vector-variant-isa-encoding-override", cl::init(VFISAKind::SSE),
    cl::Hidden,
    cl::desc("Override target CPU ISA encoding for Vector Variant passes."),
    cl::values(clEnumValN(VFISAKind::AVX512, "AVX512Core", "AVX512Core"),
               clEnumValN(VFISAKind::AVX2, "AVX2", "AVX2"),
               clEnumValN(VFISAKind::AVX, "AVX1", "AVX1"),
               clEnumValN(VFISAKind::SSE, "SSE42", "SSE42")));
#endif // end !INTEL_CUSTOMIZATION

extern cl::opt<VFISAKind> IsaEncodingOverride;
// Always get ISA, ForceVF from the global options.
// So that we could pass parameters to VFAnalysisInfo.
VFAnalysisInfo::VFAnalysisInfo()
    : ISA(IsaEncodingOverride.getValue()), ForceVF(SYCLForceVF.getValue()),
      CanFallBackToDefaultVF(false) {}

bool VFAnalysisInfo::hasConflictVFConstraints(Function *Kernel) {
  KernelMetadataAPI KMD(Kernel);
  bool MultiConflictConstraints = false;
  std::optional<unsigned> VecLen;

  if (KMD.VecLenHint.hasValue())
    VecLen = KMD.VecLenHint.get();

  if (KMD.ReqdIntelSGSize.hasValue()) {
    unsigned ReqdIntelSGSize = KMD.ReqdIntelSGSize.get();
    if (VecLen.has_value() && VecLen.value() != ReqdIntelSGSize)
      MultiConflictConstraints = true;
    VecLen = ReqdIntelSGSize;
  }

  if (isVFForced() && VecLen.has_value() && VecLen.value() != ForceVF)
    MultiConflictConstraints = true;

  LLVM_DEBUG(dbgs() << "Function <" << Kernel->getName()
                    << "> MultiConflictConstraints = "
                    << MultiConflictConstraints << '\n');

  return MultiConflictConstraints;
}

/// Find the minimum VecLength found in the node or its children.
static std::tuple<bool, bool, unsigned> getMinVecLength(CallGraphNode *Node) {
  unsigned MinVecLen = std::numeric_limits<unsigned>::max();
  bool FoundVecLen = false;
  bool HintOnly = true;

  for (auto *N : depth_first(Node)) {
    if (Function *F = N->getFunction(); F && !F->isDeclaration()) {
      KernelMetadataAPI KMD(F);
      if (KMD.hasVecLength()) {
        MinVecLen = std::min(MinVecLen, (unsigned)KMD.getVecLength());
        FoundVecLen = true;
      }
      HintOnly &= !KMD.ReqdIntelSGSize.hasValue();
    }
  }

  return {FoundVecLen, HintOnly, MinVecLen};
}

unsigned VFAnalysisInfo::deduceVF(Function *Kernel, unsigned HeuristicVF) {
  LLVM_DEBUG(dbgs() << "Deducing VF with given constraint:\n");
  KernelMetadataAPI KMD(Kernel);
  KernelInternalMetadataAPI KIMD(Kernel);
  CanFallBackToDefaultVF = false;

#if INTEL_CUSTOMIZATION
  // If O0 vectorization is NOT enabled, then we disable vectorization for
  // optnone kernels.
  if (!SYCLEnableO0Vectorization && Kernel->hasOptNone()) {
#endif // INTEL_CUSTOMIZATION
    LLVM_DEBUG(dbgs() << "Initial VF<optnone mode>: 1\n");
    return 1;
  }

  if (KIMD.MaxWGDimensions.hasValue() && KIMD.MaxWGDimensions.get() == 0 &&
      KIMD.NoBarrierPath.get()) {
    // Workgroup loop won't be created, so there is no need to vectorize kernel.
    LLVM_DEBUG(dbgs() << "Initial VF<MaxWGDim is 0>: 1\n");
    return 1;
  }

  // Allow intel_vec_len_hint and intel_reqd_sub_group_size overriding default
  // VF.
  if (auto [FoundVecLen, HintOnly, VecLen] = getMinVecLength((*CG)[Kernel]);
      FoundVecLen) {
    // For intel_vec_len_hint, allow falling back to default VF.
    if (HintOnly)
      CanFallBackToDefaultVF = true;
    LLVM_DEBUG(dbgs() << "Initial VF<From VecLength>: " << VecLen << '\n');
    return VecLen;
  }

  if (isVFForced()) {
    LLVM_DEBUG(dbgs() << "Initial VF<From ForceVF>: " << ForceVF << '\n');
    return ForceVF;
  }

  // Otherwise, get heuristic VF computed by WeightedInstCountAnalysis.
  LLVM_DEBUG(dbgs() << "Initial VF<Heuristic>: " << HeuristicVF << '\n');
  return HeuristicVF;
}

/// Detect VPLAN unsupported patterns.
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

#if INTEL_CUSTOMIZATION
  // Unsupported: Kernel calls a function with byval/byref args and
  // - Either the called function contains subgroups.
  // - Or the called function is flaged as "kernel-call-once" (VPlan can't
  //   serialize in this situation).
  // - Or the called function is an intel indirect call.
  if (!SYCLEnableVectorizationOfByvalByrefFunctions) {
    if (hasFunctionCallInCGNodeIf(Node, [](const Function *CalledFunc) {
          return hasByvalByrefArgs(CalledFunc) &&
                 (CalledFunc->hasFnAttribute(KernelAttribute::HasSubGroups) ||
                  CalledFunc->hasFnAttribute(KernelAttribute::CallOnce) ||
                  (CalledFunc &&
                   CalledFunc->getName().startswith("__intel_indirect_call")));
        })) {
      LLVM_DEBUG(dbgs() << "Can't be vectorized<byval/byref>\n");
      // Users have no control over byval/byref attributes, so we don't report
      // diangostic messages to users here.
      return true;
    }
  }
#endif // end INTEL_CUSTOMIZATION

  // Unsupported: vec_type_hint on unsupported types.
  KernelMetadataAPI KMD(Kernel);
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

  KernelInternalMetadataAPI KIMD(Kernel);
  StringRef FuncName;
  auto IsUnimplementedBuiltin = [&](const Function *CalledFunc) {
    // Builtin must be a declaration.
    unsigned VF = getVF(Kernel);
    if (!(CalledFunc && CalledFunc->isDeclaration()))
      return false;
    FuncName = CalledFunc->getName();
    bool CanEmulate = SYCLEnableSubGroupEmulation && (VF == 1);
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
  KernelMetadataAPI KMD(Kernel);
  KernelInternalMetadataAPI KIMD(Kernel);
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

  if (Broken && !SYCLEnableSubGroupEmulation) {
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
  KernelMetadataAPI KMD(Kernel);
  KernelInternalMetadataAPI KIMD(Kernel);
  auto MayNeedEmu = [&]() {
    return (KIMD.KernelHasSubgroups.hasValue() &&
            KIMD.KernelHasSubgroups.get()) ||
           (KMD.VecLenHint.hasValue() && KMD.VecLenHint.get() > 1) ||
           (KMD.ReqdIntelSGSize.hasValue() && KMD.ReqdIntelSGSize.get() > 1) ||
           (isVFForced() && ForceVF > 1);
  };
  unsigned SGEmuSize = MayNeedEmu() ? KernelToVF[Kernel] : 0;
  LLVM_DEBUG(dbgs() << "Initial SGEmuSize: " << SGEmuSize << '\n');

#if INTEL_CUSTOMIZATION
  // Explicitly disable vectorization by setting VF = 1.
  if (isSubgroupBroken(Kernel))
    KernelToVF[Kernel] = 1;
  else // No need to do emulation.
    SGEmuSize = 0;
#else  // INTEL_CUSTOMIZATION
  KernelToVF[Kernel] = 1;
#endif // INTEL_CUSTOMIZATION

  // Relax subgroup emulation size, since we don't have scalar implementation
  // for subgroup builtins.
  if (SGEmuSize == 1) {
    if (KMD.hasVecLength() && KMD.getVecLength() > 1)
      SGEmuSize = KMD.getVecLength();
    else
      SGEmuSize = getPreferredVectorizationWidth(ISA);
    LLVM_DEBUG(dbgs() << "Get SGEmuSize from default VF: " << SGEmuSize
                      << '\n');
  }

  if (!SYCLEnableSubGroupEmulation) {
    LLVM_DEBUG(dbgs() << "Subgroup emulation disabled.\n");
    SGEmuSize = 0;
  }

  assert(SGEmuSize != 1 && "Subgroup emulation size = 1 is not supported.");
  KernelToSGEmuSize[Kernel] = SGEmuSize;
}

void VFAnalysisInfo::analyzeModule(
    Module &M, function_ref<unsigned(Function &)> GetHeuristicVF) {
  CG = std::make_unique<CallGraph>(M);

  auto Kernels = KernelList(M).getList();
  for (auto *Kernel : Kernels) {
    KernelMetadataAPI KMD(Kernel);
    LLVM_DEBUG(dbgs() << "\nProcessing " << Kernel->getName() << '\n');

    // VF constraints should have the same value.
    // If not, emit a DiagInfo, which can be handled outside the optimizer by
    // a DiagnosticHandler.
    if (hasConflictVFConstraints(Kernel)) {
      M.getContext().diagnose(VFAnalysisDiagInfo(
          *Kernel,
          "Conflicting CL_CONFIG_CPU_VECTORIZER_MODE, intel_vec_len_hint "
          "and intel_reqd_sub_group_size are specified",
          VFDK_Error_ConstraintConflict));
    }

    // Deduce VF from the given constraint.
    unsigned HeuristicVF = GetHeuristicVF(*Kernel);
    KernelToVF[Kernel] =
        hasUnsupportedPatterns(Kernel) ? 1 : deduceVF(Kernel, HeuristicVF);

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

      M.getContext().diagnose(VFAnalysisDiagInfo(
          *Kernel, "Vectorization width " + Twine(VF) + " is not a power of 2",
          VFDK_Error_VFInvalid));
    }
  }
}

void VFAnalysisInfo::print(raw_ostream &OS) const {
  OS << "Kernel --> VF:\n";
  for (const auto &P : KernelToVF)
    OS << "  <" << P.getFirst()->getName() << "> : " << P.getSecond() << '\n';

  OS << "Kernel --> SGEmuSize:\n";
  for (const auto &P : KernelToSGEmuSize)
    OS << "  <" << P.getFirst()->getName() << "> : " << P.getSecond() << '\n';
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
