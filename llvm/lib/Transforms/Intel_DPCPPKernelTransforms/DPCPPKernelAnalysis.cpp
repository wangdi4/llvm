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
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/DPCPPKernelBarrierUtils.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/DPCPPKernelLoopUtils.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/Passes.h"

using namespace llvm;

namespace {

/// Legacy DPCPPKernel analysis pass.
class DPCPPKernelAnalysisLegacy : public ModulePass {
  DPCPPKernelAnalysisPass Impl;

public:
  /// Pass identifier.
  static char ID;

  DPCPPKernelAnalysisLegacy() : ModulePass(ID) {}

  StringRef getPassName() const override { return "DPCPPKernelAnalysisLegacy"; }

  bool runOnModule(Module &M) override { return Impl.runImpl(M); }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.setPreservesCFG();
  }

private:
  /// Print data collected by the pass on the given module.
  /// OS stream to print the info to.
  /// M pointer to the Module.
  void print(raw_ostream &OS, const Module *M = 0) const override;
};

} // namespace

char DPCPPKernelAnalysisLegacy::ID = 0;

INITIALIZE_PASS(DPCPPKernelAnalysisLegacy, "dpcpp-kernel-analysis",
                "Analyze which function go in barrier route", false, false)

void DPCPPKernelAnalysisLegacy::print(raw_ostream &OS, const Module *M) const {
  Impl.print(OS, M);
}

ModulePass *llvm::createDPCPPKernelAnalysisLegacyPass() {
  return new DPCPPKernelAnalysisLegacy();
}

void DPCPPKernelAnalysisPass::fillSyncUsersFuncs() {
  FuncSet BarrierRootSet;
  FuncSet SyncFunctions;

  // Get all synchronize built-ins declared in module
  DPCPPKernelCompilationUtils::getAllSyncBuiltinsDecls(SyncFunctions, M);

  BarrierRootSet.insert(SyncFunctions.begin(), SyncFunctions.end());

  DPCPPKernelLoopUtils::fillFuncUsersSet(BarrierRootSet, UnsupportedFuncs);
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

bool DPCPPKernelAnalysisPass::runImpl(Module &M) {
  this->M = &M;
  UnsupportedFuncs.clear();
  Kernels = DPCPPKernelCompilationUtils::getKernels(M);

  fillKernelCallers();
  fillSyncUsersFuncs();
  fillUnsupportedTIDFuncs();

  for (Function *Kernel : Kernels) {
    assert(Kernel && "nullptr is not expected in KernelList!");
    if (UnsupportedFuncs.count(Kernel)) {
      Kernel->addFnAttr(NO_BARRIER_PATH_ATTRNAME, "false");
    } else {
      Kernel->addFnAttr(NO_BARRIER_PATH_ATTRNAME, "true");
    }
  }

  return (Kernels.size() != 0);
}

void DPCPPKernelAnalysisPass::print(raw_ostream &OS, const Module *M) const {
  if (!M)
    return;

  OS << "\nDPCPPKernelAnalysisPass\n";

  for (Function *Kernel : Kernels) {
    assert(Kernel && "nullptr is not expected in KernelList!");

    std::string FuncName = Kernel->getName().str();

    assert(Kernel->hasFnAttribute(NO_BARRIER_PATH_ATTRNAME) &&
           "DPCPPKernelAnalysisPass: " NO_BARRIER_PATH_ATTRNAME
           " has to be set!");
    StringRef Value =
        Kernel->getFnAttribute(NO_BARRIER_PATH_ATTRNAME).getValueAsString();
    assert((Value == "true" || Value == "false") &&
           "DPCPPKernelAnalysisPass: unexpected " NO_BARRIER_PATH_ATTRNAME
           " value!");
    bool NoBarrierPath = Value == "true" ? true : false;

    if (NoBarrierPath) {
      OS << FuncName << " yes\n";
    } else {
      OS << FuncName << " no\n";
    }
  }
}

PreservedAnalyses DPCPPKernelAnalysisPass::run(Module &M,
                                               ModuleAnalysisManager &AM) {
  if (!runImpl(M))
    return PreservedAnalyses::all();
  return PreservedAnalyses::none();
}
