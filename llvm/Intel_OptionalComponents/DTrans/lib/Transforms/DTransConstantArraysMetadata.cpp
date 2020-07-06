//===--- DtransConstantArraysMetadata.cpp - Constant Arrays Metadata -*---===//
//
// Copyright (C) 2020 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===---------------------------------------------------------------------===//
// This pass will check if a field in an structure is an array with constant
// entries. If so, then collect the constant values and store them inside a
// metadata. This metadata can be used by the loop optimizer, constant
// propagation, or any other transformation that depends on constant values.
//
// This file implements Constant Arrays Metadata transformation.
//===---------------------------------------------------------------------===//

#include "Intel_DTrans/Transforms/DTransConstantArraysMetadata.h"
#include "Intel_DTrans/Analysis/DTrans.h"
#include "Intel_DTrans/Analysis/DTransAnalysis.h"
#include "Intel_DTrans/DTransCommon.h"
#include "llvm/Analysis/Intel_WP.h"
#include "llvm/Analysis/TargetTransformInfo.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"
#include "llvm/Transforms/IPO.h"

using namespace llvm;

#define DEBUG_TYPE "dtrans-constant-arrays-metadata"

// Actual implementation of constant arrays metadata
void dtrans::ConstantArraysMetadataPass::runImpl(Module &M,
                                                 DTransAnalysisInfo &DTInfo,
                                                 WholeProgramInfo &WPInfo) {

  if (!WPInfo.isWholeProgramSafe())
    return;

  if (!DTInfo.useDTransAnalysis())
    return;

}

namespace {

// Legacy pass manager
class DTransConstantArraysMetadataWrapper : public ModulePass {
private:
  dtrans::ConstantArraysMetadataPass Impl;

public:
  static char ID;

  DTransConstantArraysMetadataWrapper() : ModulePass(ID) {
    initializeDTransConstantArraysMetadataWrapperPass(
        *PassRegistry::getPassRegistry());
  }

  // Analyses needed
  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<DTransAnalysisWrapper>();
    AU.addRequired<WholeProgramWrapperPass>();
    AU.setPreservesAll();
  }

  // Run the implementation
  bool runOnModule(Module &M) override {

    if (skipModule(M))
      return false;

    DTransAnalysisInfo &DTInfo =
        getAnalysis<DTransAnalysisWrapper>().getDTransInfo(M);

    WholeProgramInfo &WPInfo =
        getAnalysis<WholeProgramWrapperPass>().getResult();

    Impl.runImpl(M, DTInfo, WPInfo);

    return false;
  }
};

} // end of anonymous namespace

char DTransConstantArraysMetadataWrapper::ID = 0;
INITIALIZE_PASS_BEGIN(DTransConstantArraysMetadataWrapper,
                      "dtrans-constant-arrays-metadata",
                      "DTrans constant arrays metadata", false, false)
INITIALIZE_PASS_DEPENDENCY(DTransAnalysisWrapper)
INITIALIZE_PASS_DEPENDENCY(WholeProgramWrapperPass)
INITIALIZE_PASS_END(DTransConstantArraysMetadataWrapper,
                    "dtrans-constant-arrays-metadata",
                    "DTrans constant arrays metadata", false, false)

ModulePass *llvm::createDTransConstantArraysMetadataWrapperPass() {
  return new DTransConstantArraysMetadataWrapper();
}

// New pass manager
PreservedAnalyses
dtrans::ConstantArraysMetadataPass::run(Module &M, ModuleAnalysisManager &AM) {

  auto &DTransInfo = AM.getResult<DTransAnalysis>(M);
  auto &WPInfo = AM.getResult<WholeProgramAnalysis>(M);

  runImpl(M, DTransInfo, WPInfo);

  return PreservedAnalyses::all();
}