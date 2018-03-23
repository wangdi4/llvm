//===---------------- DeleteField.cpp - DTransDeleteFieldPass -------------===//
//
// Copyright (C) 2018 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
//
// This file implements the DTrans delete field optimization pass.
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/Intel_DTrans/DeleteField.h"
#include "llvm/Analysis/Intel_DTrans/DTrans.h"
#include "llvm/Analysis/Intel_DTrans/DTransAnalysis.h"
#include "llvm/Analysis/Intel_WP.h"
#include "llvm/IR/Module.h"
#include "llvm/Pass.h"
#include "llvm/Transforms/IPO.h"
using namespace llvm;

#define DEBUG_TYPE "dtrans-deletefield"

namespace {

class DTransDeleteFieldWrapper : public ModulePass {
private:
  dtrans::DeleteFieldPass Impl;

public:
  static char ID;

  DTransDeleteFieldWrapper() : ModulePass(ID) {
    initializeDTransDeleteFieldWrapperPass(*PassRegistry::getPassRegistry());
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

char DTransDeleteFieldWrapper::ID = 0;
INITIALIZE_PASS_BEGIN(DTransDeleteFieldWrapper, "dtrans-deletefield",
                      "DTrans delete field", false, false)
INITIALIZE_PASS_DEPENDENCY(DTransAnalysisWrapper)
INITIALIZE_PASS_END(DTransDeleteFieldWrapper, "dtrans-deletefield",
                    "DTrans delete field", false, false)

ModulePass *llvm::createDTransDeleteFieldWrapperPass() {
  return new DTransDeleteFieldWrapper();
}

PreservedAnalyses dtrans::DeleteFieldPass::runImpl(Module &M,
                                                   DTransAnalysisInfo &DTInfo) {
  bool Changed = false;

  // TODO: Implement the optimization here.

  if (!Changed)
    return PreservedAnalyses::all();

  // TODO: Mark the actual preserved analyses.
  PreservedAnalyses PA;
  PA.preserve<WholeProgramAnalysis>();
  PA.preserve<DTransAnalysis>();
  return PA;
}

PreservedAnalyses dtrans::DeleteFieldPass::run(Module &M,
                                               ModuleAnalysisManager &AM) {
  auto &DTransInfo = AM.getResult<DTransAnalysis>(M);

  return runImpl(M, DTransInfo);
}
