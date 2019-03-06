//===---------------- MemInitTrimDown.cpp - DTransMemInitTrimDownPass -----===//
//
// Copyright (C) 2019 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
//
// This file implements the DTrans Initial Memory Allocation Trim Down
// optimization pass.
//
//===----------------------------------------------------------------------===//

#include "Intel_DTrans/Transforms/MemInitTrimDown.h"
#include "Intel_DTrans/Analysis/DTrans.h"
#include "Intel_DTrans/Analysis/DTransAnalysis.h"
#include "Intel_DTrans/DTransCommon.h"

#include "llvm/Analysis/Intel_WP.h"
#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/IR/Module.h"
#include "llvm/Pass.h"


using namespace llvm;

#define DEBUG_TYPE "dtrans-meminittrimdown"

namespace {

class DTransMemInitTrimDownWrapper : public ModulePass {
private:
  dtrans::MemInitTrimDownPass Impl;

public:
  static char ID;

  DTransMemInitTrimDownWrapper() : ModulePass(ID) {
    initializeDTransMemInitTrimDownWrapperPass(
      *PassRegistry::getPassRegistry());
  }

  bool runOnModule(Module &M) override {
    if (skipModule(M))
      return false;
    DTransAnalysisWrapper &DTAnalysisWrapper =
        getAnalysis<DTransAnalysisWrapper>();
    DTransAnalysisInfo &DTInfo = DTAnalysisWrapper.getDTransInfo(M);

    bool Changed = Impl.runImpl(
        M, DTInfo, getAnalysis<TargetLibraryInfoWrapperPass>().getTLI(),
        getAnalysis<WholeProgramWrapperPass>().getResult());
    return Changed;
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<DTransAnalysisWrapper>();
    AU.addRequired<TargetLibraryInfoWrapperPass>();
    AU.addRequired<WholeProgramWrapperPass>();
    AU.addPreserved<DTransAnalysisWrapper>();
    AU.addPreserved<WholeProgramWrapperPass>();
  }
};

} // end anonymous namespace

char DTransMemInitTrimDownWrapper::ID = 0;
INITIALIZE_PASS_BEGIN(DTransMemInitTrimDownWrapper, "dtrans-meminittrimdown",
                      "DTrans Mem Init Trim Down", false, false)
INITIALIZE_PASS_DEPENDENCY(DTransAnalysisWrapper)
INITIALIZE_PASS_DEPENDENCY(TargetLibraryInfoWrapperPass)
INITIALIZE_PASS_DEPENDENCY(WholeProgramWrapperPass)
INITIALIZE_PASS_END(DTransMemInitTrimDownWrapper, "dtrans-meminittrimdown",
                    "DTrans Mem Init Trim Down", false, false)

ModulePass *llvm::createDTransMemInitTrimDownWrapperPass() {
  return new DTransMemInitTrimDownWrapper();
}

namespace llvm {

namespace dtrans {

class MemInitTrimDownImpl {

public:
  MemInitTrimDownImpl(Module &M, const DataLayout &DL,
               DTransAnalysisInfo &DTInfo, TargetLibraryInfo &TLI)
      : M(M), DL(DL), DTInfo(DTInfo), TLI(TLI) {};

  bool run(void);

private:
  Module &M;
  const DataLayout &DL;
  DTransAnalysisInfo &DTInfo;
  TargetLibraryInfo &TLI;
};


bool MemInitTrimDownImpl::run(void) {

  return false;
}

bool MemInitTrimDownPass::runImpl(Module &M, DTransAnalysisInfo &DTInfo,
                           TargetLibraryInfo &TLI, WholeProgramInfo &WPInfo) {

  auto TTIAVX2 = TargetTransformInfo::AdvancedOptLevel::AO_TargetHasAVX2;
  if (!WPInfo.isWholeProgramSafe() ||
      !WPInfo.isAdvancedOptEnabled(TTIAVX2))
    return false;

  if (!DTInfo.useDTransAnalysis())
    return false;

  auto &DL = M.getDataLayout();

  MemInitTrimDownImpl MemInitTrimDownI(M, DL, DTInfo, TLI);
  return MemInitTrimDownI.run();
}

PreservedAnalyses MemInitTrimDownPass::run(Module &M,
                                           ModuleAnalysisManager &AM) {
  auto &DTransInfo = AM.getResult<DTransAnalysis>(M);
  auto &WPInfo = AM.getResult<WholeProgramAnalysis>(M);

  if (!runImpl(M, DTransInfo, AM.getResult<TargetLibraryAnalysis>(M), WPInfo))
    return PreservedAnalyses::all();

  // TODO: Mark the actual preserved analyses.
  PreservedAnalyses PA;
  PA.preserve<WholeProgramAnalysis>();
  PA.preserve<DTransAnalysis>();
  return PA;
}

} // namespace dtrans

} // namespace llvm
