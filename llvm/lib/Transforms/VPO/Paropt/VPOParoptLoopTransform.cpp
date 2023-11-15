#if INTEL_COLLAB
//
// INTEL CONFIDENTIAL
//
// Modifications, Copyright (C) 2022 Intel Corporation
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you ("License"). Unless the License provides otherwise, you may
// not use, modify, copy, publish, distribute, disclose or transmit this
// software or the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.
//
//===-- VPOParoptLoopTransform.cpp - OpenMP Loop Transformation Pass --===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file implements the loop transform pass for OpenMP loops.
///
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/VPO/Paropt/VPOParoptLoopTransform.h"
#include "llvm/Analysis/OptimizationRemarkEmitter.h"
#include "llvm/Analysis/VPO/WRegionInfo/WRegionInfo.h"
#include "llvm/InitializePasses.h"
#include "llvm/Transforms/VPO/Paropt/VPOParoptTransform.h"
#include "llvm/Transforms/VPO/VPOPasses.h"

#if INTEL_CUSTOMIZATION
#include "llvm/Analysis/Intel_OptReport/OptReportBuilder.h"
#endif // INTEL_CUSTOMIZATION

using namespace llvm;
using namespace llvm::vpo;

#define OPT_SWITCH "vpo-paropt-loop-transform"
#define PASS_NAME "VPO Paropt Loop Transform Function Pass"
#define DEBUG_TYPE OPT_SWITCH

static cl::opt<bool>
    DisableVPOLoopTransform("disable-" OPT_SWITCH, cl::Hidden, cl::init(false),
                            cl::desc("Disable paropt loop transformation"));

namespace {
class VPOParoptLoopTransform : public FunctionPass {
public:
  static char ID;

  VPOParoptLoopTransform() : FunctionPass(ID) {
    initializeVPOParoptLoopTransformPass(*PassRegistry::getPassRegistry());
  }

  StringRef getPassName() const override { return PASS_NAME; }

  bool runOnFunction(Function &F) override;

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    // TODO: Depending on how much of analysis can be updated
    //       locally, call addPreserved()

    // LI, DT, SE are through WRegionInfoWrapperPass
    AU.addRequired<WRegionInfoWrapperPass>();
    AU.addRequired<OptimizationRemarkEmitterWrapperPass>();
  }
};

static bool transformLoops(Function &F, WRegionInfo &WI,
                           OptimizationRemarkEmitter &ORE) {
  bool Changed = false;

  // TODO: Is it a re-walk? If so, for innermost-loop, this
  //       re-walk may not be needed. WRGraph may not have been changed
  //       after the previous build. Also, isn't it a part of WRegionInfo?
  //       If WRegionInfo was set preserved by previous pass, the re-built
  //       shouldn't happen. Corect?
  // Walk the W-Region Graph top-down, and create W-Region List
  WI.buildWRGraph();

  if (WI.WRGraphIsEmpty()) {
    LLVM_DEBUG(dbgs() << "\nNo WRegion Candidates for Transformation \n");
    return Changed;
  }

  LLVM_DEBUG(WI.print(dbgs()));
  LLVM_DEBUG(dbgs() << PASS_NAME << " for Function: ");
  LLVM_DEBUG(dbgs().write_escaped(F.getName()) << '\n');

  VPOParoptTransform VP(nullptr, &F, &WI, WI.getDomTree(), WI.getLoopInfo(),
                        WI.getSE(), WI.getTargetTransformInfo(),
                        WI.getAssumptionCache(), WI.getTargetLibraryInfo(),
#if INTEL_CUSTOMIZATION
                        WI.getAliasAnalysis(),
                        /*MemorySSA=*/nullptr, LoopTransform,
                        OptReportVerbosity::None, ORE, 2, false);
#else
                        WI.getAliasAnalysis(), LoopTransform, ORE, 2, false);
#endif // INTEL_CUSTOMIZATION

  Changed |= VP.paroptTransforms();

  return Changed;
}
} // end anonymous namespace

char VPOParoptLoopTransform::ID = 0;
INITIALIZE_PASS_BEGIN(VPOParoptLoopTransform, DEBUG_TYPE, PASS_NAME, false,
                      false)
INITIALIZE_PASS_DEPENDENCY(WRegionInfoWrapperPass)
INITIALIZE_PASS_DEPENDENCY(OptimizationRemarkEmitterWrapperPass)

INITIALIZE_PASS_END(VPOParoptLoopTransform, DEBUG_TYPE, PASS_NAME, false, false)

FunctionPass *llvm::createVPOParoptLoopTransformPass() {
  return new VPOParoptLoopTransform();
}

bool VPOParoptLoopTransform::runOnFunction(Function &F) {
  if (DisableVPOLoopTransform)
    return false;

  WRegionInfo &WI = getAnalysis<WRegionInfoWrapperPass>().getWRegionInfo();
  auto &ORE = getAnalysis<OptimizationRemarkEmitterWrapperPass>().getORE();

  LLVM_DEBUG(dbgs() << "\n\n====== Enter " << PASS_NAME << " ======\n\n");

  bool Changed = transformLoops(F, WI, ORE);
  LLVM_DEBUG(dbgs() << "\n\n====== Exit  " << PASS_NAME << " ======\n\n");
  return Changed;
}

PreservedAnalyses VPOParoptLoopTransformPass::run(Function &F,
                                                  FunctionAnalysisManager &AM) {

  if (DisableVPOLoopTransform)
    return PreservedAnalyses::all();

  WRegionInfo &WI = AM.getResult<WRegionInfoAnalysis>(F);
  auto &ORE = AM.getResult<OptimizationRemarkEmitterAnalysis>(F);

  // These are need for rotating loops.
  // Currently, rotating is not needed, so skipped.
  // auto &TTI = AM.getResult<TargetIRAnalysis>(F);
  // auto &AC = AM.getResult<AssumptionAnalysis>(F);
  // auto &TLI = AM.getResult<TargetLibraryAnalysis>(F);
  // WI.setTargetTransformInfo(&TTI);
  // WI.setAssumptionCache(&AC);
  // WI.setTargetLibraryInfo(&TLI);

  PreservedAnalyses PA;

  LLVM_DEBUG(dbgs() << "\n\n====== Enter " << PASS_NAME << " ======\n\n");
  if (!transformLoops(F, WI, ORE))
    PA = PreservedAnalyses::all();
  else
    PA = PreservedAnalyses::none();
  LLVM_DEBUG(dbgs() << "\n\n====== Exit  " << PASS_NAME << " ======\n\n");

  return PA;
}
#endif // INTEL_COLLAB
