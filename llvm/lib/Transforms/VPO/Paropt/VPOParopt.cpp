#if INTEL_COLLAB
//
// INTEL CONFIDENTIAL
//
// Modifications, Copyright (C) 2021 Intel Corporation
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you ("License"). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.
//
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
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/InitializePasses.h"

#include "llvm/Transforms/VPO/Paropt/VPOParopt.h"
#include "llvm/Transforms/VPO/Paropt/VPOParoptModuleTransform.h"
#include "llvm/Transforms/VPO/VPOPasses.h"
#include "llvm/Transforms/Utils.h"

#if INTEL_CUSTOMIZATION
#include "llvm/Analysis/BasicAliasAnalysis.h"
#include "llvm/Analysis/Intel_OptReport/OptReportBuilder.h"
#include "llvm/Analysis/Intel_OptReport/OptReportOptionsPass.h"
#include "llvm/Analysis/Intel_XmainOptLevelPass.h"
#include "llvm/Analysis/VPO/Intel_VPOParoptConfig.h"
#endif  // INTEL_CUSTOMIZATION

#define DEBUG_TYPE "VPOParopt"

using namespace llvm;
using namespace llvm::vpo;

static cl::opt<bool> DisableOffload(
  "vpo-paropt-disable-offload", cl::Hidden, cl::init(false),
  cl::desc("Ignore OpenMP TARGET construct in VPO Paropt."));

INITIALIZE_PASS_BEGIN(VPOParopt, "vpo-paropt", "VPO Paropt Module Pass", false,
                      false)
INITIALIZE_PASS_DEPENDENCY(LoopSimplify)
INITIALIZE_PASS_DEPENDENCY(WRegionInfoWrapperPass)
#if INTEL_CUSTOMIZATION
INITIALIZE_PASS_DEPENDENCY(OptReportOptionsPass)
INITIALIZE_PASS_DEPENDENCY(VPOParoptConfigWrapper)
INITIALIZE_PASS_DEPENDENCY(XmainOptLevelWrapperPass)
#endif  // INTEL_CUSTOMIZATION
INITIALIZE_PASS_DEPENDENCY(AssumptionCacheTracker)
INITIALIZE_PASS_DEPENDENCY(TargetTransformInfoWrapperPass)
INITIALIZE_PASS_DEPENDENCY(TargetLibraryInfoWrapperPass)
INITIALIZE_PASS_END(VPOParopt, "vpo-paropt", "VPO Paropt Module Pass", false,
                    false)

char VPOParopt::ID = 0;

ModulePass *llvm::createVPOParoptPass(unsigned Mode) {
  return new VPOParopt(
      (ParTrans | OmpPar | OmpVec | OmpTpv | OmpOffload | OmpTbb) & Mode);
}

VPOParopt::VPOParopt(unsigned MyMode) : ModulePass(ID), Impl(MyMode) {
  initializeVPOParoptPass(*PassRegistry::getPassRegistry());
}

VPOParoptPass::VPOParoptPass(unsigned MyMode) : Mode(MyMode) {
  LLVM_DEBUG(dbgs() << "\n\n====== Start VPO Paropt Pass ======\n\n");
}

void VPOParopt::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.addRequiredID(LoopSimplifyID);
  AU.addRequired<WRegionInfoWrapperPass>();
#if INTEL_CUSTOMIZATION
  AU.addRequired<OptReportOptionsPass>();
  AU.addRequired<VPOParoptConfigWrapper>();
  AU.addRequired<XmainOptLevelWrapperPass>();
#endif  // INTEL_CUSTOMIZATION
  AU.addRequired<AssumptionCacheTracker>();
  AU.addRequired<TargetTransformInfoWrapperPass>();
  AU.addRequired<TargetLibraryInfoWrapperPass>();
}

bool VPOParopt::runOnModule(Module &M) {
  auto &MTTI = getAnalysis<TargetTransformInfoWrapperPass>();
  auto &MAC = getAnalysis<AssumptionCacheTracker>();
  auto &MTLI = getAnalysis<TargetLibraryInfoWrapperPass>();
#if INTEL_CUSTOMIZATION
  auto OptLevel = getAnalysis<XmainOptLevelWrapperPass>().getOptLevel();
  LegacyAARGetter AARGetter(*this);
  auto &ParoptConfig = getAnalysis<VPOParoptConfigWrapper>().getResult();
#endif // INTEL_CUSTOMIZATION

  auto WRegionInfoGetter = [&](Function &F, bool *Changed) -> WRegionInfo & {
    assert(!F.isDeclaration() && "Cannot get analysis on declaration.");
    auto &WRI =
        getAnalysis<WRegionInfoWrapperPass>(F, Changed).getWRegionInfo();

    WRI.setTargetTransformInfo(&MTTI.getTTI(F));
    WRI.setAssumptionCache(&MAC.getAssumptionCache(F));
    WRI.setTargetLibraryInfo(&MTLI.getTLI(F));
#if INTEL_CUSTOMIZATION
    WRI.setAliasAnlaysis(&AARGetter(F));
    WRI.setupAAWithOptLevel(OptLevel);
    WRI.setVPOParoptConfig(&ParoptConfig);
#endif // INTEL_CUSTOMIZATION
    return WRI;
  };

#if INTEL_CUSTOMIZATION
  ORVerbosity = getAnalysis<OptReportOptionsPass>().getVerbosity();
  return Impl.runImpl(M, WRegionInfoGetter, OptLevel);
#else
  return Impl.runImpl(M, WRegionInfoGetter);
#endif  // INTEL_CUSTOMIZATION
}

PreservedAnalyses VPOParoptPass::run(Module &M, ModuleAnalysisManager &AM) {
#if INTEL_CUSTOMIZATION
  auto &ParoptConfig = AM.getResult<VPOParoptConfigAnalysis>(M);
#endif // INTEL_CUSTOMIZATION
  auto WRegionInfoGetter = [&](Function &F, bool *Changed) -> WRegionInfo & {
    auto &FAM =
        AM.getResult<FunctionAnalysisManagerModuleProxy>(M).getManager();
    auto &WRI = FAM.getResult<WRegionInfoAnalysis>(F);
    auto &TTI = FAM.getResult<TargetIRAnalysis>(F);
    auto &AC = FAM.getResult<AssumptionAnalysis>(F);
    auto &TLI = FAM.getResult<TargetLibraryAnalysis>(F);

    WRI.setTargetTransformInfo(&TTI);
    WRI.setAssumptionCache(&AC);
    WRI.setTargetLibraryInfo(&TLI);
#if INTEL_CUSTOMIZATION
    WRI.setVPOParoptConfig(&ParoptConfig);
#endif // INTEL_CUSTOMIZATION
    return WRI;
  };

#if INTEL_CUSTOMIZATION
  auto OptLevel = AM.getResult<XmainOptLevelAnalysis>(M).getOptLevel();
  ORVerbosity = AM.getResult<OptReportOptionsAnalysis>(M).getVerbosity();
  bool Changed = runImpl(M, WRegionInfoGetter, OptLevel);
#else
  bool Changed = runImpl(M, WRegionInfoGetter);
#endif  // INTEL_CUSTOMIZATION

  if (!Changed)
    return PreservedAnalyses::all();

  return PreservedAnalyses::none();
}

// Do paropt transformations on a given module. Current implementation creates
// VPOParoptModuleTransform instance for the module which implements all
// paropt's function and module level transformations.
bool VPOParoptPass::runImpl(
    Module &M,
    std::function<vpo::WRegionInfo &(Function &F, bool *Changed)>
#if INTEL_CUSTOMIZATION
        WRegionInfoGetter,
    unsigned OptLevel) {
#else
        WRegionInfoGetter) {
#endif // INTEL_CUSTOMIZATION

  LLVM_DEBUG(dbgs() << "\n====== VPO ParoptPass ======\n\n");

  // Disable offload constructs if -fopenmp-targets was not used on command line
  if (M.getTargetDevices().empty())
     DisableOffload = true;

  // AUTOPAR | OPENMP | SIMD | OFFLOAD
#if INTEL_CUSTOMIZATION
  VPOParoptModuleTransform VP(M, Mode, OptLevel, DisableOffload);
#else
  VPOParoptModuleTransform VP(M, Mode, /*OptLevel=*/2, DisableOffload);
#endif // INTEL_CUSTOMIZATION

  bool Changed = VP.doParoptTransforms(WRegionInfoGetter);

  LLVM_DEBUG(dbgs() << "\n====== End VPO ParoptPass ======\n\n");
  return Changed;
}
#endif // INTEL_COLLAB
