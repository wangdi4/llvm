#if INTEL_COLLAB
//===------- VPOParopt.cpp - Paropt Pass for OpenMP Transformations -------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
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

#include "llvm/Transforms/VPO/Paropt/VPOParopt.h"
#include "llvm/Transforms/VPO/Paropt/VPOParoptModuleTransform.h"
#include "llvm/Transforms/VPO/VPOPasses.h"
#include "llvm/Transforms/Utils.h"

#if INTEL_CUSTOMIZATION
#include "llvm/Analysis/Intel_OptReport/LoopOptReportBuilder.h"
#include "llvm/Analysis/Intel_OptReport/OptReportOptionsPass.h"
#endif  // INTEL_CUSTOMIZATION

#define DEBUG_TYPE "VPOParopt"

using namespace llvm;
using namespace llvm::vpo;

static cl::opt<bool> SwitchToOffload(
    "switch-to-offload", cl::Hidden, cl::init(false),
    cl::desc("switch to offload mode (default = false)"));

static cl::opt<bool> DisableOffload(
  "vpo-paropt-disable-offload", cl::Hidden, cl::init(false),
  cl::desc("Ignore OpenMP TARGET construct in VPO Paropt."));

INITIALIZE_PASS_BEGIN(VPOParopt, "vpo-paropt", "VPO Paropt Module Pass", false,
                      false)
INITIALIZE_PASS_DEPENDENCY(LoopSimplify)
INITIALIZE_PASS_DEPENDENCY(WRegionInfoWrapperPass)
#if INTEL_CUSTOMIZATION
INITIALIZE_PASS_DEPENDENCY(OptReportOptionsPass)
#endif  // INTEL_CUSTOMIZATION
INITIALIZE_PASS_DEPENDENCY(TargetLibraryInfoWrapperPass)
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
#if INTEL_CUSTOMIZATION
  AU.addRequired<OptReportOptionsPass>();
#endif  // INTEL_CUSTOMIZATION
  AU.addRequired<TargetLibraryInfoWrapperPass>();
}

bool VPOParopt::runOnModule(Module &M) {
  if (skipModule(M))
    return false;

  auto WRegionInfoGetter = [&](Function &F) -> WRegionInfo & {
    return getAnalysis<WRegionInfoWrapperPass>(F).getWRegionInfo();
  };

  auto TLIGetter = [&](Function &F) -> TargetLibraryInfo & {
    return getAnalysis<TargetLibraryInfoWrapperPass>().getTLI(F);
  };

#if INTEL_CUSTOMIZATION
  ORVerbosity = getAnalysis<OptReportOptionsPass>().getVerbosity();
#endif  // INTEL_CUSTOMIZATION

  return Impl.runImpl(M, WRegionInfoGetter, TLIGetter);
}

PreservedAnalyses VPOParoptPass::run(Module &M, ModuleAnalysisManager &AM) {
  auto WRegionInfoGetter = [&](Function &F) -> WRegionInfo & {
    auto &FAM =
        AM.getResult<FunctionAnalysisManagerModuleProxy>(M).getManager();
    return FAM.getResult<WRegionInfoAnalysis>(F);
  };

  auto TLIGetter = [&](Function &F) -> TargetLibraryInfo & {
    auto &FAM =
        AM.getResult<FunctionAnalysisManagerModuleProxy>(M).getManager();
    return FAM.getResult<TargetLibraryAnalysis>(F);
  };

#if INTEL_CUSTOMIZATION
  ORVerbosity = AM.getResult<OptReportOptionsAnalysis>(M).getVerbosity();
#endif  // INTEL_CUSTOMIZATION

  if (!runImpl(M, WRegionInfoGetter, TLIGetter))
    return PreservedAnalyses::all();

  return PreservedAnalyses::none();
}

// Do paropt transformations on a given module. Current implementation creates
// VPOParoptModuleTransform instance for the module which implements all
// paropt's function and module level transformations.
bool VPOParoptPass::runImpl(
    Module &M,
    std::function<vpo::WRegionInfo &(Function &F)> WRegionInfoGetter,
    std::function<TargetLibraryInfo &(Function &F)> TLIGetter) {

  LLVM_DEBUG(dbgs() << "\n====== VPO ParoptPass ======\n\n");

  // Disable offload constructs if -fopenmp-targets was not used on command line
  if (M.getTargetDevices().empty())
     DisableOffload = true;

  // AUTOPAR | OPENMP | SIMD | OFFLOAD
  VPOParoptModuleTransform VP(M, Mode, OptLevel, SwitchToOffload,
                              DisableOffload);
  bool Changed = VP.doParoptTransforms(WRegionInfoGetter, TLIGetter);

  LLVM_DEBUG(dbgs() << "\n====== End VPO ParoptPass ======\n\n");
  return Changed;
}
#endif // INTEL_COLLAB
