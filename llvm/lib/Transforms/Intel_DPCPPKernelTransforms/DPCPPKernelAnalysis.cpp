//==--- DPCPPKernelAnalysis.cpp - Detect barriers in DPCPP kernels- C++ -*--==//
//
// Copyright (C) 2020 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //

#include "llvm/Transforms/Intel_DPCPPKernelTransforms/DPCPPKernelAnalysis.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Instructions.h"
#include "llvm/InitializePasses.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/DPCPPKernelCompilationUtils.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/DPCPPKernelBarrierUtils.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/DPCPPKernelLoopUtils.h"

#include <string>

using namespace llvm;

INITIALIZE_PASS(DPCPPKernelAnalysis, "dpcpp-kernel-analysis",
                "analyzes which function go in barrier route", false, false)

namespace llvm {

char DPCPPKernelAnalysis::ID = 0;

DPCPPKernelAnalysis::DPCPPKernelAnalysis() : ModulePass(ID), M(nullptr) {}

DPCPPKernelAnalysis::~DPCPPKernelAnalysis() {}

void DPCPPKernelAnalysis::fillSyncUsersFuncs() {
  FuncSet BarrierRootSet;

  DPCPPKernelCompilationUtils::FuncSet SyncFunctions;

  // Get all synchronize built-ins declared in module
  DPCPPKernelCompilationUtils::getAllSyncBuiltinsDecls(SyncFunctions, M);

  BarrierRootSet.insert(SyncFunctions.begin(), SyncFunctions.end());

  DPCPPKernelLoopUtils::fillFuncUsersSet(BarrierRootSet, UnsupportedFuncs);
}

void DPCPPKernelAnalysis::fillUnsupportedTIDFuncs() {
  FuncSet DirectTIDUsers;
  std::string LID = "__builtin_get_local_id";
  fillUnsupportedTIDFuncs(LID.c_str(), DirectTIDUsers);
  DPCPPKernelLoopUtils::fillFuncUsersSet(DirectTIDUsers, UnsupportedFuncs);
}

bool DPCPPKernelAnalysis::isUnsupportedDim(Value *V) {
  ConstantInt *ConstDim = dyn_cast<ConstantInt>(V);
  // If arg is not a constant return true OR,
  // if it is illegal constant.
  if (!ConstDim || ConstDim->getValue().getZExtValue() > 2)
    return true;
  return false;
}

void DPCPPKernelAnalysis::fillUnsupportedTIDFuncs(const char *Name,
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

void DPCPPKernelAnalysis::fillKernelCallers() {
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
    if (KernelUsers.size()) {
      // TODO: Explore CallGraph fully to detect a case with
      // a kernel calling a functions calling a kernel.
      for (Function *F : KernelUsers)
        if (F->hasFnAttribute("sycl_kernel"))
          llvm_unreachable("Having a kernel calling a kernel is not supported!");
    }
  }

  // Also can not use explicit loops on kernel callers since the barrier
  // pass need to handle them in order to process the called kernels.
  FuncSet KernelSet(Kernels.begin(), Kernels.end());
  DPCPPKernelLoopUtils::fillFuncUsersSet(KernelSet, UnsupportedFuncs);
}

bool DPCPPKernelAnalysis::runOnModule(Module &M) {
  this->M = &M;
  UnsupportedFuncs.clear();
  Kernels.clear();

  for (auto &F : M) {
    if (F.hasFnAttribute("sycl_kernel"))
      Kernels.push_back(&F);
  }

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

void DPCPPKernelAnalysis::print(raw_ostream &OS, const Module *M) const {
  if (!M)
    return;

  OS << "\nDPCPPKernelAnalysis\n";

  for (Function *Kernel : Kernels) {
    assert(Kernel && "nullptr is not expected in KernelList!");

    std::string FuncName = Kernel->getName().str();

    assert(Kernel->hasFnAttribute(NO_BARRIER_PATH_ATTRNAME) &&
           "DPCPPKernelAnalysis: " NO_BARRIER_PATH_ATTRNAME " has to be set!");
    StringRef Value =
        Kernel->getFnAttribute(NO_BARRIER_PATH_ATTRNAME).getValueAsString();
    assert((Value == "true" || Value == "false") &&
           "DPCPPKernelAnalysis: unexpected " NO_BARRIER_PATH_ATTRNAME
           " value!");
    bool NoBarrierPath = Value == "true" ? true : false;

    if (NoBarrierPath) {
      OS << FuncName << " yes\n";
    } else {
      OS << FuncName << " no\n";
    }
  }
}

ModulePass *createDPCPPKernelAnalysisPass() {
  return new llvm::DPCPPKernelAnalysis();
}

} // namespace llvm
