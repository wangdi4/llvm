//==--- DPCPPKernelAnalysis.cpp - Detect barriers in DPCPP kernels- C++ -*--==//
//
// Copyright (C) 2020-2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //

#include "llvm/Transforms/Intel_DPCPPKernelTransforms/DPCPPKernelAnalysis.h"
#include "llvm/IR/Instructions.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/DPCPPKernelCompilationUtils.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/DPCPPKernelLoopUtils.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/LegacyPasses.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/Utils/MetadataAPI.h"

#define DEBUG_TYPE "dpcpp-kernel-analysis"

using namespace llvm;

static cl::opt<bool> DPCPPEnableNativeSubgroups(
    "dpcpp-enable-native-subgroups", cl::init(true), cl::Hidden,
    cl::desc("Enable native subgroup functionality"));

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
    return Impl.runImpl(M, getAnalysis<CallGraphWrapperPass>().getCallGraph());
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<CallGraphWrapperPass>();
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
                      "Analyze which function go in barrier route", false,
                      false)
INITIALIZE_PASS_DEPENDENCY(CallGraphWrapperPass)
INITIALIZE_PASS_END(DPCPPKernelAnalysisLegacy, DEBUG_TYPE,
                    "Analyze which function go in barrier route", false, false)

void DPCPPKernelAnalysisLegacy::print(raw_ostream &OS, const Module *M) const {
  Impl.print(OS, M);
}

ModulePass *llvm::createDPCPPKernelAnalysisLegacyPass() {
  return new DPCPPKernelAnalysisLegacy();
}

void DPCPPKernelAnalysisPass::fillSyncUsersFuncs() {
  // Get all synchronize built-ins declared in module
  FuncSet SyncFunctions =
      DPCPPKernelCompilationUtils::getAllSyncBuiltinsDecls(*M);

  DPCPPKernelLoopUtils::fillFuncUsersSet(SyncFunctions, UnsupportedFuncs);
}

void DPCPPKernelAnalysisPass::fillUnsupportedTIDFuncs() {
  FuncSet DirectTIDUsers;
  std::string LID = DPCPPKernelCompilationUtils::mangledGetLID();
  std::string GID = DPCPPKernelCompilationUtils::mangledGetGID();
  fillUnsupportedTIDFuncs(LID, DirectTIDUsers);
  fillUnsupportedTIDFuncs(GID, DirectTIDUsers);
  DPCPPKernelLoopUtils::fillFuncUsersSet(DirectTIDUsers, UnsupportedFuncs);
}

bool DPCPPKernelAnalysisPass::isUnsupportedDim(Value *V) {
  ConstantInt *ConstDim = dyn_cast<ConstantInt>(V);
  // If arg is not a constant return true OR,
  // if it is illegal constant.
  if (!ConstDim || ConstDim->getValue().getZExtValue() > 2)
    return true;
  return false;
}

void DPCPPKernelAnalysisPass::fillUnsupportedTIDFuncs(StringRef Name,
                                                      FuncSet &DirectTIDUsers) {
  Function *F = M->getFunction(Name);
  if (!F)
    return;
  for (auto *U : F->users()) {
    CallInst *CI = dyn_cast<CallInst>(U);
    if (!CI)
      continue;
    // if the tid is variable add it to the set
    Function *CallingFunc = CI->getParent()->getParent();
    DirectTIDUsers.insert(CallingFunc);
    if (isUnsupportedDim(CI->getArgOperand(0))) {
      UnsupportedFuncs.insert(CI->getParent()->getParent());
    }
  }
}

void DPCPPKernelAnalysisPass::fillKernelCallers() {
  for (Function *Kernel : Kernels) {
    if (!Kernel)
      continue;
    FuncSet KernelRootSet;
    FuncSet KernelUsers;
    KernelRootSet.insert(Kernel);
    DPCPPKernelLoopUtils::fillFuncUsersSet(KernelRootSet, KernelUsers);
    // The kernel has user functions meaning it is called by another kernel.
    // Since there is no barrier in it's start it will be executed
    // multiple time (because of the WG loop of the calling kernel).
    if (KernelUsers.size())
      UnsupportedFuncs.insert(Kernel);
  }

  // Also can not use explicit loops on kernel callers since the barrier
  // pass need to handle them in order to process the called kernels.
  FuncSet KernelSet(Kernels.begin(), Kernels.end());
  DPCPPKernelLoopUtils::fillFuncUsersSet(KernelSet, UnsupportedFuncs);
}

void DPCPPKernelAnalysisPass::fillSubgroupCallingFuncs(CallGraph &CG) {
  using namespace DPCPPKernelCompilationUtils;
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

bool DPCPPKernelAnalysisPass::runImpl(Module &M, CallGraph &CG) {
  this->M = &M;
  UnsupportedFuncs.clear();
  auto KernelList = DPCPPKernelCompilationUtils::getKernels(M);
  Kernels.insert(KernelList.begin(), KernelList.end());

  fillKernelCallers();
  fillSyncUsersFuncs();
  fillUnsupportedTIDFuncs();
  if (DPCPPEnableNativeSubgroups)
    fillSubgroupCallingFuncs(CG);

  for (Function *Kernel : Kernels) {
    assert(Kernel && "nullptr is not expected in KernelList!");
    DPCPPKernelMetadataAPI::KernelInternalMetadataAPI KIMD(Kernel);
    KIMD.NoBarrierPath.set(!UnsupportedFuncs.count(Kernel));
    if (DPCPPEnableNativeSubgroups)
      KIMD.KernelHasSubgroups.set(SubgroupCallingFuncs.count(Kernel));
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
    bool NoBarrierPath = KIMD.NoBarrierPath.get();
    bool KernelHasSubgroups = KIMD.KernelHasSubgroups.get();

    OS << "Kernel <" << FuncName << ">: NoBarrierPath=" << NoBarrierPath
       << " KernelHasSubgroups=" << KernelHasSubgroups << '\n';
  }

  OS << "\nFunctions that call subgroup builtins:\n";
  for (Function *F : SubgroupCallingFuncs)
    OS << "  " << F->getName() << '\n';
}

PreservedAnalyses DPCPPKernelAnalysisPass::run(Module &M,
                                               ModuleAnalysisManager &AM) {
  (void)runImpl(M, AM.getResult<CallGraphAnalysis>(M));
  return PreservedAnalyses::all();
}
