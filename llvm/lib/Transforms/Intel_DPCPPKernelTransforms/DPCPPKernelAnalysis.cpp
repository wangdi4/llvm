//==--- DPCPPKernelAnalysis.cpp - Analyze DPCPP kernel properties - C++ -*--==//
//
// Copyright (C) 2020-2022 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //

#include "llvm/Transforms/Intel_DPCPPKernelTransforms/DPCPPKernelAnalysis.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/ADT/DepthFirstIterator.h"
#include "llvm/IR/Instructions.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/BuiltinLibInfoAnalysis.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/LegacyPasses.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/Utils/CompilationUtils.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/Utils/LoopUtils.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/Utils/MetadataAPI.h"
#include <cmath>

#define DEBUG_TYPE "dpcpp-kernel-analysis"

using namespace llvm;
using namespace CompilationUtils;

namespace {

/// Legacy DPCPPKernel analysis pass.
class DPCPPKernelAnalysisLegacy : public ModulePass {
  DPCPPKernelAnalysisPass Impl;

public:
  /// Pass identifier.
  static char ID;

  DPCPPKernelAnalysisLegacy() : ModulePass(ID) {
    initializeDPCPPKernelAnalysisLegacyPass(*PassRegistry::getPassRegistry());
  }

  StringRef getPassName() const override { return "DPCPPKernelAnalysisLegacy"; }

  bool runOnModule(Module &M) override {
    auto &RTS = getAnalysis<BuiltinLibInfoAnalysisLegacy>()
                    .getResult()
                    .getRuntimeService();
    auto GetLI = [&](Function &F) -> LoopInfo & {
      return getAnalysis<LoopInfoWrapperPass>(F).getLoopInfo();
    };
    return Impl.runImpl(M, getAnalysis<CallGraphWrapperPass>().getCallGraph(),
                        RTS, GetLI);
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<BuiltinLibInfoAnalysisLegacy>();
    AU.addRequired<CallGraphWrapperPass>();
    AU.addRequired<LoopInfoWrapperPass>();
    AU.setPreservesAll();
  }

private:
  /// Print data collected by the pass on the given module.
  /// OS stream to print the info to.
  /// M pointer to the Module.
  void print(raw_ostream &OS, const Module *M = 0) const override;
};

} // namespace

char DPCPPKernelAnalysisLegacy::ID = 0;

INITIALIZE_PASS_BEGIN(DPCPPKernelAnalysisLegacy, DEBUG_TYPE,
                      "Analyze kernel properties", false, false)
INITIALIZE_PASS_DEPENDENCY(BuiltinLibInfoAnalysisLegacy)
INITIALIZE_PASS_DEPENDENCY(CallGraphWrapperPass)
INITIALIZE_PASS_DEPENDENCY(LoopInfoWrapperPass)
INITIALIZE_PASS_END(DPCPPKernelAnalysisLegacy, DEBUG_TYPE,
                    "Analyze kernel properties", false, false)

void DPCPPKernelAnalysisLegacy::print(raw_ostream &OS, const Module *M) const {
  Impl.print(OS, M);
}

ModulePass *llvm::createDPCPPKernelAnalysisLegacyPass() {
  return new DPCPPKernelAnalysisLegacy();
}

void DPCPPKernelAnalysisPass::fillSyncUsersFuncs() {
  // Get all synchronize built-ins declared in module
  FuncSet SyncFunctions = getAllSyncBuiltinsDecls(*M);

  LoopUtils::fillFuncUsersSet(SyncFunctions, UnsupportedFuncs);
}

void DPCPPKernelAnalysisPass::fillMatrixCallFuncs() {
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
      MatrixIntrins.insert(&F);
      break;
    }
  }
  LoopUtils::fillFuncUsersSet(MatrixIntrins, MatrixCallFuncs);
}

void DPCPPKernelAnalysisPass::fillKernelCallers() {
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

void DPCPPKernelAnalysisPass::fillSubgroupCallingFuncs(CallGraph &CG) {
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

bool DPCPPKernelAnalysisPass::runImpl(
    Module &M, CallGraph &CG, const RuntimeService &RTS,
    function_ref<LoopInfo &(Function &)> GetLI) {
  this->M = &M;
  UnsupportedFuncs.clear();
  auto KernelList = CompilationUtils::getKernels(M);
  Kernels.insert(KernelList.begin(), KernelList.end());

  fillKernelCallers();
  fillSyncUsersFuncs();
  fillMatrixCallFuncs();
  fillSubgroupCallingFuncs(CG);

  for (Function *Kernel : Kernels) {
    assert(Kernel && "nullptr is not expected in KernelList!");
    DPCPPKernelMetadataAPI::KernelInternalMetadataAPI KIMD(Kernel);
    if (MatrixCallFuncs.count(Kernel))
      KIMD.HasMatrixCall.set(true);
    KIMD.NoBarrierPath.set(!UnsupportedFuncs.contains(Kernel));
    KIMD.KernelHasSubgroups.set(SubgroupCallingFuncs.contains(Kernel));
    KIMD.KernelHasGlobalSync.set(hasAtomicBuiltinCall(CG, RTS, Kernel));
    KIMD.KernelExecutionLength.set(getExecutionLength(Kernel, GetLI(*Kernel)));
  }

  LLVM_DEBUG(print(dbgs(), this->M));

  return (Kernels.size() != 0);
}

void DPCPPKernelAnalysisPass::print(raw_ostream &OS, const Module *M) const {
  if (!M)
    return;

  OS << "\nDPCPPKernelAnalysisPass\n";

  for (Function *Kernel : Kernels) {
    assert(Kernel && "nullptr is not expected in KernelList!");

    StringRef FuncName = Kernel->getName();

    DPCPPKernelMetadataAPI::KernelInternalMetadataAPI KIMD(Kernel);
    OS << "Kernel <" << FuncName << ">:\n";
    OS.indent(2) << "NoBarrierPath=" << KIMD.NoBarrierPath.get() << "\n";
    OS.indent(2) << "KernelHasMatrixCall=" << KIMD.HasMatrixCall.get() << "\n";
    OS.indent(2) << "KernelHasSubgroups=" << KIMD.KernelHasSubgroups.get()
                 << "\n";
    OS.indent(2) << "KernelHasGlobalSync=" << KIMD.KernelHasGlobalSync.get()
                 << "\n";
    OS.indent(2) << "KernelExecutionLength=" << KIMD.KernelExecutionLength.get()
                 << "\n";
  }

  OS << "\nFunctions that call subgroup builtins:\n";
  for (Function *F : SubgroupCallingFuncs)
    OS << "  " << F->getName() << '\n';
}

PreservedAnalyses DPCPPKernelAnalysisPass::run(Module &M,
                                               ModuleAnalysisManager &AM) {
  RuntimeService &RTS =
      AM.getResult<BuiltinLibInfoAnalysis>(M).getRuntimeService();
  auto &FAM = AM.getResult<FunctionAnalysisManagerModuleProxy>(M).getManager();
  auto GetLI = [&](Function &F) -> LoopInfo & {
    return FAM.getResult<LoopAnalysis>(F);
  };
  (void)runImpl(M, AM.getResult<CallGraphAnalysis>(M), RTS, GetLI);
  return PreservedAnalyses::all();
}
