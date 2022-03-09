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
//===------- VPOParoptPrepare.cpp - Paropt Prepare Pass for OpenMP --------===//
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
// July 2016: Initial Implementation of Paropt Prepare Pass (Xinmin Tian)
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file implements the ParOpt prepare pass interface to perform Prepare
/// transformation for OpenMP parallelization, simd and Offloading
///
//===----------------------------------------------------------------------===//

#include "llvm/Pass.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/PassManager.h"
#include "llvm/IR/Verifier.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/InitializePasses.h"

#include "llvm/Analysis/VPO/WRegionInfo/WRegionInfo.h"

#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Utils.h"
#include "llvm/Transforms/VPO/VPOPasses.h"
#include "llvm/Transforms/VPO/Utils/VPOUtils.h"
#include "llvm/Transforms/VPO/Paropt/VPOParoptPrepare.h"
#include "llvm/Transforms/VPO/Paropt/VPOParoptTransform.h"

#if INTEL_CUSTOMIZATION
#include "llvm/Analysis/Intel_OptReport/OptReportBuilder.h"
#include "llvm/Analysis/Intel_OptReport/OptReportOptionsPass.h"
#include "llvm/Analysis/VPO/Intel_VPOParoptConfig.h"
#endif // INTEL_CUSTOMIZATION

using namespace llvm;
using namespace llvm::vpo;

#define DEBUG_TYPE "VPOParoptPrepare"

#ifndef NDEBUG
static cl::opt<bool> VerifyIRBeforeParopt(
    "vpo-paropt-verify-ir-before", cl::Hidden, cl::init(true),
    cl::desc("Enable IR verification before Paropt."));
#endif  // NDEBUG

static cl::opt<bool> PrepareDisableOffload(
  "vpo-paropt-prepare-disable-offload", cl::Hidden, cl::init(false),
  cl::desc("Ignore OpenMP TARGET construct in VPO Paropt Prepare."));

INITIALIZE_PASS_BEGIN(VPOParoptPrepare, "vpo-paropt-prepare",
                     "VPO Paropt Prepare Function Pass", false, false)
INITIALIZE_PASS_DEPENDENCY(LoopSimplify)
INITIALIZE_PASS_DEPENDENCY(AssumptionCacheTracker)
INITIALIZE_PASS_DEPENDENCY(TargetTransformInfoWrapperPass)
INITIALIZE_PASS_DEPENDENCY(TargetLibraryInfoWrapperPass)
INITIALIZE_PASS_DEPENDENCY(WRegionInfoWrapperPass)
#if INTEL_CUSTOMIZATION
INITIALIZE_PASS_DEPENDENCY(OptReportOptionsPass)
INITIALIZE_PASS_DEPENDENCY(VPOParoptConfigWrapper)
#endif // INTEL_CUSTOMIZATION
INITIALIZE_PASS_DEPENDENCY(OptimizationRemarkEmitterWrapperPass)
INITIALIZE_PASS_END(VPOParoptPrepare, "vpo-paropt-prepare",
                    "VPO Paropt Prepare Function Pass", false, false)

char VPOParoptPrepare::ID = 0;

FunctionPass *llvm::createVPOParoptPreparePass(unsigned Mode) {
  return new VPOParoptPrepare(Mode & (ParPrepare | OmpOffload));
}

VPOParoptPrepare::VPOParoptPrepare(unsigned MyMode)
    : FunctionPass(ID), Impl(MyMode) {
  LLVM_DEBUG(dbgs() << "\n\n====== Enter VPO Paropt Prepare ======\n\n");
  initializeVPOParoptPreparePass(*PassRegistry::getPassRegistry());
}

VPOParoptPreparePass::VPOParoptPreparePass(unsigned MyMode)
    : Mode(MyMode) {
  LLVM_DEBUG(dbgs() << "\n\n====== Enter VPO Paropt Prepare Pass ======\n\n");
}

void VPOParoptPrepare::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.addRequiredID(LoopSimplifyID);
  AU.addRequired<AssumptionCacheTracker>();
  AU.addRequired<TargetTransformInfoWrapperPass>();
  AU.addRequired<TargetLibraryInfoWrapperPass>();
  AU.addRequired<WRegionInfoWrapperPass>();
#if INTEL_CUSTOMIZATION
  AU.addRequired<OptReportOptionsPass>();
  AU.addRequired<VPOParoptConfigWrapper>();
#endif // INTEL_CUSTOMIZATION
  AU.addRequired<OptimizationRemarkEmitterWrapperPass>();
}

bool VPOParoptPrepare::runOnFunction(Function &F) {

  LLVM_DEBUG(dbgs() << "\n=== VPOParoptPrepare Start: " << F.getName()
                    << " {\n");

  // TODO: need Front-End to set F.hasOpenMPDirective()
  if (F.isDeclaration()) { // if(!F.hasOpenMPDirective()))
    LLVM_DEBUG(dbgs() << "\n}=== VPOParoptPrepare End (no change): "
                      << F.getName() << "\n");
    return false;
  }

  WRegionInfo &WI = getAnalysis<WRegionInfoWrapperPass>().getWRegionInfo();
  auto &TTI = getAnalysis<TargetTransformInfoWrapperPass>().getTTI(F);
  auto &AC = getAnalysis<AssumptionCacheTracker>().getAssumptionCache(F);
  auto &TLI = getAnalysis<TargetLibraryInfoWrapperPass>().getTLI(F);
  WI.setTargetTransformInfo(&TTI);
  WI.setAssumptionCache(&AC);
  WI.setTargetLibraryInfo(&TLI);

#if INTEL_CUSTOMIZATION
  ORVerbosity = getAnalysis<OptReportOptionsPass>().getVerbosity();
  auto &ParoptConfig = getAnalysis<VPOParoptConfigWrapper>().getResult();
  WI.setVPOParoptConfig(&ParoptConfig);
#endif // INTEL_CUSTOMIZATION
  auto &ORE = getAnalysis<OptimizationRemarkEmitterWrapperPass>().getORE();

  bool Changed = Impl.runImpl(F, WI, ORE);

  LLVM_DEBUG(dbgs() << "\n}=== VPOParoptPrepare End: " << F.getName() << "\n");
  LLVM_DEBUG(dbgs() << "\n====== Exit VPO Paropt Prepare ======\n\n");

  return Changed;
}

bool VPOParoptPreparePass::runImpl(Function &F, WRegionInfo &WI,
                                   OptimizationRemarkEmitter &ORE) {
  bool Changed = false;

  // Walk the W-Region Graph top-down, and create W-Region List
  WI.buildWRGraph();

  LLVM_DEBUG(dbgs() << "\n=== W-Region Graph Build Done: " << F.getName()
                    << "\n");

  if (WI.WRGraphIsEmpty()) {
    LLVM_DEBUG(dbgs() << "\nNo WRegion Candidates for Parallelization \n");
  }

  LLVM_DEBUG(WI.print(dbgs()));

  LLVM_DEBUG(errs() << "VPOParoptPreparePass: ");
  LLVM_DEBUG(errs().write_escaped(F.getName()) << '\n');

  LLVM_DEBUG(
      dbgs() << "\n === VPOParoptPreparePass before Transformation === \n");

#ifndef NDEBUG
  if (VerifyIRBeforeParopt) {
    // Run Paropt specific verification, first.
    VPOParoptUtils::verifyFunctionForParopt(
        F, VPOAnalysisUtils::isTargetSPIRV(F.getParent()));
    if (verifyFunction(F, &dbgs())) {
      LLVM_DEBUG(dbgs() << "ERROR: function verifier found errors "
                 "before VPOParoptPrepare:\n" << F << "\n");
      report_fatal_error("Function verifier found errors "
                         "before VPOParoptPrepare.  Use -mllvm "
                         "-debug-only=" DEBUG_TYPE " to get more information");
    }
  }
#endif  // NDEBUG

  // Disable offload constructs if -fopenmp-targets was not used on command line
  if (F.getParent()->getTargetDevices().empty())
    PrepareDisableOffload = true;

  // AUTOPAR | OPENMP | SIMD | OFFLOAD
  VPOParoptTransform VP(nullptr, &F, &WI, WI.getDomTree(), WI.getLoopInfo(),
                        WI.getSE(), WI.getTargetTransformInfo(),
                        WI.getAssumptionCache(), WI.getTargetLibraryInfo(),
                        WI.getAliasAnalysis(), Mode,
#if INTEL_CUSTOMIZATION
                        ORVerbosity,
#endif  // INTEL_CUSTOMIZATION
                        ORE, 2, PrepareDisableOffload);
  Changed = Changed | VP.paroptTransforms();

  LLVM_DEBUG(
      dbgs() << "\n === VPOParoptPreparePass after Transformation === \n");

  // Remove calls to directive intrinsics since the LLVM back end does not
  // know how to translate them.
  // VPOUtils::stripDirectives(F);

  return Changed;
}

PreservedAnalyses VPOParoptPreparePass::run(Function &F,
                                            FunctionAnalysisManager &AM) {

  LLVM_DEBUG(dbgs() << "\n=== VPOParoptPreparePass Start: " << F.getName()
                    << " {\n");

  // TODO: need Front-End to set F.hasOpenMPDirective()
  if (F.isDeclaration()) { // if(!F.hasOpenMPDirective()))
    LLVM_DEBUG(dbgs() << "\n}=== VPOParoptPreparePass End (no change): "
                      << F.getName() << "\n");
    return PreservedAnalyses::all();
  }

  auto &WI = AM.getResult<WRegionInfoAnalysis>(F);
  auto &TTI = AM.getResult<TargetIRAnalysis>(F);
  auto &AC = AM.getResult<AssumptionAnalysis>(F);
  auto &TLI = AM.getResult<TargetLibraryAnalysis>(F);
  WI.setTargetTransformInfo(&TTI);
  WI.setAssumptionCache(&AC);
  WI.setTargetLibraryInfo(&TLI);

#if INTEL_CUSTOMIZATION
  ORVerbosity = AM.getResult<OptReportOptionsAnalysis>(F).getVerbosity();
  auto &MAMProxy = AM.getResult<ModuleAnalysisManagerFunctionProxy>(F);
  auto *ParoptConfig =
      MAMProxy.getCachedResult<VPOParoptConfigAnalysis>(*F.getParent());
  WI.setVPOParoptConfig(ParoptConfig);
#endif // INTEL_CUSTOMIZATION
  auto &ORE = AM.getResult<OptimizationRemarkEmitterAnalysis>(F);

  bool Changed = runImpl(F, WI, ORE);

  LLVM_DEBUG(dbgs() << "\n}=== VPOParoptPreparePass End: " << F.getName()
                    << "\n");
  LLVM_DEBUG(dbgs() << "\n====== Exit VPO Paropt Prepare Pass======\n\n");

  if (!Changed)
    return PreservedAnalyses::all();

  return PreservedAnalyses::none();;
}
#endif // INTEL_COLLAB
