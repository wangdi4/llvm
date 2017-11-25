//===---------------- DTransOpt.cpp - DTransOpt example -*-----------------===//
//
// Copyright (C) 2017 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
// This file provides an example optimization pass that uses DTransAnalysis.
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/Intel_DTrans/DTransOpt.h"
#include "llvm/Analysis/Intel_DTrans/DTrans.h"
#include "llvm/Analysis/Intel_DTrans/DTransAnalysis.h"
#include "llvm/Analysis/Intel_WP.h"
#include "llvm/IR/Module.h"
#include "llvm/Pass.h"
#include "llvm/Transforms/IPO.h"
using namespace llvm;

#define DEBUG_TYPE "dtransopt"

namespace {

class DTransOptWrapper : public ModulePass {
private:
  DTransOptPass Impl;

public:
  static char ID;

  DTransOptWrapper() : ModulePass(ID) {
    initializeDTransOptWrapperPass(*PassRegistry::getPassRegistry());
  }

  bool runOnModule(Module &M) override {
    if (skipModule(M))
      return false;
    DTransAnalysisInfo &DTInfo =
        getAnalysis<DTransAnalysisWrapper>().getDTransInfo();
    auto PA = Impl.runImpl(M, DTInfo);
    return !PA.areAllPreserved();
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    // TODO: Mark the actual required and preserved analyses.
    AU.addRequired<DTransAnalysisWrapper>();
    AU.addPreserved<WholeProgramWrapperPass>();
  }
};

} // end anonymous namespace

char DTransOptWrapper::ID = 0;
INITIALIZE_PASS_BEGIN(DTransOptWrapper, "dtransopt",
                      "DTrans Optimization Example", false, false)
INITIALIZE_PASS_DEPENDENCY(DTransAnalysisWrapper)
INITIALIZE_PASS_END(DTransOptWrapper, "dtransopt",
                    "DTrans Optimization Example", false, false)

ModulePass *llvm::createDTransOptWrapperPass() {
  return new DTransOptWrapper();
}

PreservedAnalyses DTransOptPass::runImpl(Module &M,
                                         DTransAnalysisInfo &DTInfo) {
  bool Changed = false;

  // TODO: Implement the optimization here.

  if (!Changed)
    return PreservedAnalyses::all();

  // TODO: Mark the actual preserved analyses.
  PreservedAnalyses PA;
  PA.preserve<WholeProgramAnalysis>();
  return PA;
}

PreservedAnalyses DTransOptPass::run(Module &M, ModuleAnalysisManager &AM) {
  auto &DTransInfo = AM.getResult<DTransAnalysis>(M);

  return runImpl(M, DTransInfo);
}
