//==--- SYCLKernelAnalysis.cpp - Analyze SYCL kernel properties - C++ -*--==//
//
// Copyright (C) 2020 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //

#include "llvm/Transforms/SYCLTransforms/SYCLKernelAnalysis.h"
#include "llvm/ADT/DepthFirstIterator.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/SYCLTransforms/BuiltinLibInfoAnalysis.h"
#include "llvm/Transforms/SYCLTransforms/Utils/CompilationUtils.h"
#include "llvm/Transforms/SYCLTransforms/Utils/LoopUtils.h"
#include "llvm/Transforms/SYCLTransforms/Utils/MetadataAPI.h"
#include <cmath>

#define DEBUG_TYPE "sycl-kernel-analysis"

using namespace llvm;
using namespace CompilationUtils;

static cl::opt<bool> SYCLKernelAnalysisAssumeIsAMX(
    "sycl-kernel-analysis-assume-isamx", cl::init(false), cl::Hidden,
    cl::desc("make assumption for sycl kernel analysis's isamx"));

static cl::opt<bool> SYCLKernelAnalysisAssumeIsAMXFP16(
    "sycl-kernel-analysis-assume-isamxfp16", cl::init(false), cl::Hidden,
    cl::desc("make assumption for sycl kernel analysis's isamxfp16"));

DiagnosticKind SYCLKernelAnalysisDiagInfo::Kind =
    static_cast<DiagnosticKind>(getNextAvailablePluginDiagnosticKind());

void SYCLKernelAnalysisPass::fillSyncUsersFuncs() {
  // Get all synchronize built-ins declared in module
  FuncSet SyncFunctions = getAllSyncBuiltinsDecls(*M);

  LoopUtils::fillFuncUsersSet(SyncFunctions, UnsupportedFuncs);
}

#if INTEL_CUSTOMIZATION
void SYCLKernelAnalysisPass::fillMatrixCallFuncs() {
  FuncSet MatrixIntrins;
  for (auto &F : *M) {
    switch (F.getIntrinsicID()) {
    default:
      break;
    case Intrinsic::experimental_matrix_load:
    case Intrinsic::experimental_matrix_store:
    case Intrinsic::experimental_matrix_mad:
    case Intrinsic::experimental_matrix_sumad:
    case Intrinsic::experimental_matrix_usmad:
    case Intrinsic::experimental_matrix_uumad:
    case Intrinsic::experimental_matrix_extract_row_slice:
    case Intrinsic::experimental_matrix_insert_row_slice:
    case Intrinsic::experimental_matrix_fill:
      if (!IsAMX)
        M->getContext().diagnose(SYCLKernelAnalysisDiagInfo(
            F,
            "AMX matrix primitives are being used on an arch older than "
            "Sapphire Rapids! DPC++ joint matrix extension requires presence "
            "of AMX on Sapphire Rapids or later)",
            DKDK_Error_MatrixIntrinOnUnsupportedCPU));
      Type *MatrixElemType =
          F.getIntrinsicID() != Intrinsic::experimental_matrix_store
              ? cast<FixedVectorType>(F.getReturnType())->getElementType()
              : cast<FixedVectorType>(F.getFunctionType()->getParamType(0))
                    ->getElementType();
      if (!IsAMXFP16 && MatrixElemType->isHalfTy()) {
        M->getContext().diagnose(SYCLKernelAnalysisDiagInfo(
            F,
            "AMX-FP16 matrix primitives are being used on an arch older than "
            "Granite Rapids! DPC++ joint matrix extension requires presence "
            "of AMX on Sapphire Rapids or later)",
            DKDK_Error_MatrixIntrinOnUnsupportedCPU));
      }
      MatrixIntrins.insert(&F);
      break;
    }
  }
  LoopUtils::fillFuncUsersSet(MatrixIntrins, MatrixCallFuncs);
}
#endif // INTEL_CUSTOMIZATION

void SYCLKernelAnalysisPass::fillKernelCallers() {
  for (Function *Kernel : Kernels) {
    if (!Kernel)
      continue;
    FuncSet KernelRootSet;
    FuncSet KernelUsers;
    KernelRootSet.insert(Kernel);
    LoopUtils::fillFuncUsersSet(KernelRootSet, KernelUsers);
    // The kernel has user functions meaning it is called by another kernel.
    // Since there is no barrier in it's start it will be executed
    // multiple time (because of the WG loop of the calling kernel).
    if (KernelUsers.size())
      UnsupportedFuncs.insert(Kernel);
  }

  // Also can not use explicit loops on kernel callers since the barrier
  // pass need to handle them in order to process the called kernels.
  FuncSet KernelSet(Kernels.begin(), Kernels.end());
  LoopUtils::fillFuncUsersSet(KernelSet, UnsupportedFuncs);
}

void SYCLKernelAnalysisPass::fillSubgroupCallingFuncs(CallGraph &CG) {
  using namespace CompilationUtils;
  for (auto &F : *M) {
    if (F.isDeclaration())
      continue;
    if (hasFunctionCallInCGNodeIf(CG[&F], [&](const Function *CalledFunc) {
          return CalledFunc && CalledFunc->isDeclaration() &&
                 (isSubGroupBuiltin(CalledFunc->getName()) ||
                  isSubGroupBarrier(CalledFunc->getName()));
        })) {
      SubgroupCallingFuncs.insert(&F);
      F.addFnAttr(KernelAttribute::HasSubGroups);
    }
  }
}

static bool hasAddrSpaceQualifierFunc(CallGraph &CG, const RuntimeService &RTS,
                                      Function *F) {
  auto *Node = CG[F];
  for (auto It = df_begin(Node), E = df_end(Node); It != E; ++It) {
    for (const auto &Pair : **It) {
      if (!Pair.first)
        continue;
      Function *CalledFunc = Pair.second->getFunction();
      if (!CalledFunc)
        continue;
      if (CompilationUtils::isAddrspaceQualifierBuiltins(
              CalledFunc->getName())) {
        return true;
      }
    }
  }
  return false;
}

static bool hasAtomicBuiltinCall(CallGraph &CG, const RuntimeService &RTS,
                                 Function *F) {
  auto *Node = CG[F];
  for (auto It = df_begin(Node), E = df_end(Node); It != E; ++It) {
    for (const auto &Pair : **It) {
      if (!Pair.first)
        continue;
      CallInst *CI = cast<CallInst>(*Pair.first);
      Function *CalledFunc = Pair.second->getFunction();
      if (!CalledFunc || !RTS.isAtomicBuiltin(CalledFunc->getName()))
        continue;
      Value *Arg0 = CI->getOperand(0);

      // handle atomic_work_item_fence(cl_mem_fence_flags flags,
      // memory_order order, memory_scope scope) builtin.
      if (isAtomicWorkItemFenceBuiltin(CalledFunc->getName())) {
        // !!! MUST be aligned with define in clang/lib/Headers/opencl-c-base.h
        // #define CLK_GLOBAL_MEM_FENCE   0x2
        static const uint64_t CLK_GLOBAL_MEM_FENCE = 2;
        if (auto *C = dyn_cast<ConstantInt>(Arg0)) {
          return C->getZExtValue() & CLK_GLOBAL_MEM_FENCE;
        } else {
          // 0th argument is not constant.
          // Assume the worst case - has CLK_GLOBAL_MEM_FENCE flag set.
          return true;
        }
      }

      // After switching GenericAddressStaticResolutionPass to
      // InferAddressSpacesPass, it's legal for a pointer to remain as
      // unresolved and live in generic address space until CodeGen. So if a
      // pointer is in generic address space, we just ignore it and assume
      // that it won't access global address space. This assumption won't
      // affect the correctness of the program, as the global synchronization
      // information is only used to calculate the workgroup size (see
      // Kernel.cpp and search 'HasGlobalSyncOperation' keyword) for
      // performance tunning.

      // [OpenCL 2.0] The following condition covers pipe built-ins as well
      // because the first arguments is a pipe which is a __global opaque
      // pointer.
      if (cast<PointerType>(Arg0->getType())->getAddressSpace() ==
          ADDRESS_SPACE_GLOBAL)
        return true;
    }
  }
  return false;
}

static size_t getExecutionEstimation(unsigned Depth) {
  return (size_t)pow(10.f, (int)Depth);
}

/// Previously this is calculated before Barrier and PrepareKernelArgs Passes.
/// TODO This is a rough calculation. WeightedInstCount probably provides better
/// estimation.
static size_t getExecutionLength(Function *F, LoopInfo &LI) {
  size_t Length = 0;
  for (auto &BB : *F) {
    Length += BB.size() * getExecutionEstimation(LI.getLoopDepth(&BB));
  }
  return Length;
}

bool SYCLKernelAnalysisPass::runImpl(
    Module &M, CallGraph &CG, const RuntimeService &RTS,
    function_ref<LoopInfo &(Function &)> GetLI) {
  this->M = &M;
  UnsupportedFuncs.clear();
  auto KernelList = CompilationUtils::getKernels(M);
  Kernels.insert(KernelList.begin(), KernelList.end());

  fillKernelCallers();
  fillSyncUsersFuncs();
  fillMatrixCallFuncs(); // INTEL
  fillSubgroupCallingFuncs(CG);

  for (Function *Kernel : Kernels) {
    assert(Kernel && "nullptr is not expected in KernelList!");
    SYCLKernelMetadataAPI::KernelInternalMetadataAPI KIMD(Kernel);
#if INTEL_CUSTOMIZATION
    if (MatrixCallFuncs.count(Kernel))
      KIMD.HasMatrixCall.set(true);
#endif // INTEL_CUSTOMIZATION
    KIMD.NoBarrierPath.set(!UnsupportedFuncs.contains(Kernel));
    KIMD.KernelHasSubgroups.set(SubgroupCallingFuncs.contains(Kernel));
    KIMD.KernelHasGlobalSync.set(hasAtomicBuiltinCall(CG, RTS, Kernel));
    if (hasAddrSpaceQualifierFunc(CG, RTS, Kernel))
      KIMD.UseAddrSpaceQualifierFunc.set(true);
    KIMD.KernelExecutionLength.set(getExecutionLength(Kernel, GetLI(*Kernel)));
  }

  LLVM_DEBUG(print(dbgs(), this->M));

  return (Kernels.size() != 0);
}

void SYCLKernelAnalysisPass::print(raw_ostream &OS, const Module *M) const {
  if (!M)
    return;

  OS << "\nSYCLKernelAnalysisPass\n";

  for (Function *Kernel : Kernels) {
    assert(Kernel && "nullptr is not expected in KernelList!");

    StringRef FuncName = Kernel->getName();

    SYCLKernelMetadataAPI::KernelInternalMetadataAPI KIMD(Kernel);
    OS << "Kernel <" << FuncName << ">:\n";
    OS.indent(2) << "NoBarrierPath=" << KIMD.NoBarrierPath.get() << "\n";
    if (KIMD.HasMatrixCall.hasValue())
      OS.indent(2) << "KernelHasMatrixCall=" << KIMD.HasMatrixCall.get()
                   << "\n";
    OS.indent(2) << "KernelHasSubgroups=" << KIMD.KernelHasSubgroups.get()
                 << "\n";
    OS.indent(2) << "KernelHasGlobalSync=" << KIMD.KernelHasGlobalSync.get()
                 << "\n";
    OS.indent(2) << "KernelExecutionLength=" << KIMD.KernelExecutionLength.get()
                 << "\n";
    if (KIMD.UseAddrSpaceQualifierFunc.hasValue())
      OS.indent(2) << "UseAddrSpaceQualifierFunc="
                   << KIMD.UseAddrSpaceQualifierFunc.get() << "\n";
  }

  OS << "\nFunctions that call subgroup builtins:\n";
  for (Function *F : SubgroupCallingFuncs)
    OS << "  " << F->getName() << '\n';
}

PreservedAnalyses SYCLKernelAnalysisPass::run(Module &M,
                                              ModuleAnalysisManager &AM) {
  IsAMX = SYCLKernelAnalysisAssumeIsAMX || IsAMX;
  IsAMXFP16 = SYCLKernelAnalysisAssumeIsAMXFP16 || IsAMXFP16;
  RuntimeService &RTS =
      AM.getResult<BuiltinLibInfoAnalysis>(M).getRuntimeService();
  auto &FAM = AM.getResult<FunctionAnalysisManagerModuleProxy>(M).getManager();
  auto GetLI = [&](Function &F) -> LoopInfo & {
    return FAM.getResult<LoopAnalysis>(F);
  };
  (void)runImpl(M, AM.getResult<CallGraphAnalysis>(M), RTS, GetLI);
  return PreservedAnalyses::all();
}
