#if INTEL_COLLAB
//===-------- VPOParopt.cpp - Paropt Pass for Auto-Par and OpenMP ---------===//
//
// Copyright (C) 2015-2018 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation. and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
// Authors:
// --------
// Xinmin Tian (xinmin.tian@intel.com)
//
// Major Revisions:
// ----------------
// Nov 2015: Initial Implementation of Paropt Pass (Xinmin Tian)
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file implements the ParOpt pass interface to perform transformation
/// for OpenMP and Auto-parallelization
///
//===----------------------------------------------------------------------===//

#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/InitializePasses.h"

#include "llvm/Transforms/Intel_VPO/Paropt/VPOParopt.h"
#include "llvm/Transforms/Intel_VPO/Paropt/VPOParoptModuleTransform.h"
#include "llvm/Transforms/Intel_VPO/VPOPasses.h"
#include "llvm/Transforms/Utils.h"

#define DEBUG_TYPE "VPOParopt"


using namespace llvm;
using namespace llvm::vpo;

static cl::opt<bool> SwitchToOffload(
    "switch-to-offload", cl::Hidden, cl::init(false),
    cl::desc("switch to offload mode (default = false)"));

INITIALIZE_PASS_BEGIN(VPOParopt, "vpo-paropt", "VPO Paropt Module Pass", false,
                      false)
INITIALIZE_PASS_DEPENDENCY(LoopSimplify)
INITIALIZE_PASS_DEPENDENCY(WRegionInfoWrapperPass)
INITIALIZE_PASS_END(VPOParopt, "vpo-paropt", "VPO Paropt Module Pass", false,
                    false)

char VPOParopt::ID = 0;

ModulePass *llvm::createVPOParoptPass(unsigned Mode, unsigned OptLevel) {
  return new VPOParopt(
      (ParTrans | OmpPar | OmpVec | OmpTpv | OmpOffload | OmpTbb) & Mode,
      OptLevel);
}

VPOParopt::VPOParopt(unsigned MyMode, unsigned OptLevel)
    : ModulePass(ID), Impl(MyMode, OptLevel) {
  initializeVPOParoptPass(*PassRegistry::getPassRegistry());
}

VPOParoptPass::VPOParoptPass(unsigned MyMode, unsigned OptLevel)
    : Mode(MyMode), OptLevel(OptLevel) {
  LLVM_DEBUG(dbgs() << "\n\n====== Start VPO Paropt Pass ======\n\n");
}

void VPOParopt::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.addRequiredID(LoopSimplifyID);
  AU.addRequired<WRegionInfoWrapperPass>();
}

bool VPOParopt::runOnModule(Module &M) {
  if (skipModule(M))
    return false;

  auto WRegionInfoGetter = [&](Function &F) -> WRegionInfo & {
    return getAnalysis<WRegionInfoWrapperPass>(F).getWRegionInfo();
  };

  return Impl.runImpl(M, WRegionInfoGetter);
}

PreservedAnalyses VPOParoptPass::run(Module &M, ModuleAnalysisManager &AM) {
  auto WRegionInfoGetter = [&](Function &F) -> WRegionInfo & {
    auto &FAM =
        AM.getResult<FunctionAnalysisManagerModuleProxy>(M).getManager();
    return FAM.getResult<WRegionInfoAnalysis>(F);
  };

  if (!runImpl(M, WRegionInfoGetter))
    return PreservedAnalyses::all();

  return PreservedAnalyses::none();
}

// Do paropt transformations on a given module. Current implementation creates
// VPOParoptModuleTransform instance for the module which implements all
// paropt's function and module level transformations.
bool VPOParoptPass::runImpl(
    Module &M,
    std::function<vpo::WRegionInfo &(Function &F)> WRegionInfoGetter) {

  LLVM_DEBUG(dbgs() << "\n====== VPO ParoptPass ======\n\n");

  // AUTOPAR | OPENMP | SIMD | OFFLOAD
  VPOParoptModuleTransform VP(M, Mode, OptLevel, SwitchToOffload);
  bool Changed = VP.doParoptTransforms(WRegionInfoGetter);

  LLVM_DEBUG(dbgs() << "\n====== End VPO ParoptPass ======\n\n");
  return Changed;
}
#endif // INTEL_COLLAB
