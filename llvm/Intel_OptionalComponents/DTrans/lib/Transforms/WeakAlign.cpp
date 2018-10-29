//===---------------- WeakAlign.cpp - DTransWeakAlignPass -------------------===//
//
// Copyright (C) 2018 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
//
// This file implements the DTrans Weak Align pass
//
//===----------------------------------------------------------------------===//

#include "Intel_DTrans/Transforms/WeakAlign.h"
#include "Intel_DTrans/DTransCommon.h"

using namespace llvm;

#define DEBUG_TYPE "dtrans-weakalign"

namespace {
  class DTransWeakAlignWrapper : public ModulePass {
  private:
    dtrans::WeakAlignPass Impl;

  public:
    static char ID;

    DTransWeakAlignWrapper() : ModulePass(ID) {
      initializeDTransWeakAlignWrapperPass(
        *PassRegistry::getPassRegistry());
    }

    bool runOnModule(Module &M) override {
      return Impl.runImpl(M);
    }

    void getAnalysisUsage(AnalysisUsage &AU) const override {
    }
  };
} //  end anonymous namespace

PreservedAnalyses dtrans::WeakAlignPass::run(Module &M,
  ModuleAnalysisManager &AM) {
  if (!runImpl(M))
    return PreservedAnalyses::all();

  PreservedAnalyses PA;
  return PA;
}

bool dtrans::WeakAlignPass::runImpl(Module &M) {
  bool Changed = false;
  return Changed;
}

char DTransWeakAlignWrapper::ID = 0;
INITIALIZE_PASS(DTransWeakAlignWrapper, "dtrans-weakalign",
  "DTrans weak align", false, false)

  ModulePass *llvm::createDTransWeakAlignWrapperPass() {
  return new DTransWeakAlignWrapper();
}


