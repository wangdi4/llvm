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
//===-- VPOParoptLoopCollapse.cpp - Paropt Loop Collapse Pass for OpenMP --===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
// Authors:
// Vyacheslav Zakharin (vyacheslav.p.zakharin@intel.com)
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file implements the loop nests collapsing pass for OpenMP loops.
///
//===----------------------------------------------------------------------===//

#include "llvm/Analysis/OptimizationRemarkEmitter.h"
#include "llvm/Analysis/VPO/WRegionInfo/WRegionInfo.h"
#include "llvm/InitializePasses.h"
#include "llvm/Transforms/VPO/Paropt/VPOParoptLoopCollapse.h"
#include "llvm/Transforms/VPO/Paropt/VPOParoptTransform.h"
#include "llvm/Transforms/VPO/VPOPasses.h"

#if INTEL_CUSTOMIZATION
#include "llvm/Analysis/Intel_OptReport/OptReportBuilder.h"
#endif  // INTEL_CUSTOMIZATION

using namespace llvm;
using namespace llvm::vpo;

#define DEBUG_TYPE "vpo-paropt-loop-collapse"
#define PASS_NAME "VPO Paropt Loop Collapse Function Pass"

namespace {
class VPOParoptLoopCollapse : public FunctionPass {
public:
  static char ID;

  VPOParoptLoopCollapse() : FunctionPass(ID) {
    initializeVPOParoptLoopCollapsePass(*PassRegistry::getPassRegistry());
  }

  StringRef getPassName() const override { return PASS_NAME; }

  bool runOnFunction(Function &F) override;

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<WRegionInfoWrapperPass>();
    AU.addRequired<OptimizationRemarkEmitterWrapperPass>();
  }
};

static bool collapseLoops(
    Function &F, WRegionInfo &WI, OptimizationRemarkEmitter &ORE) {
  bool Changed = false;

  LLVM_DEBUG(dbgs() << PASS_NAME << ": Before Par Sections Transformation");
  LLVM_DEBUG(dbgs() << F << " \n");

  Changed = VPOUtils::parSectTransformer(&F, WI.getDomTree(), WI.getLoopInfo());

  LLVM_DEBUG(dbgs() << PASS_NAME << ": After Par Sections Transformation");
  LLVM_DEBUG(dbgs() << F << " \n");

  // Walk the W-Region Graph top-down, and create W-Region List
  WI.buildWRGraph();

  if (WI.WRGraphIsEmpty()) {
    LLVM_DEBUG(dbgs() << "\nNo WRegion Candidates for Parallelization \n");
    return Changed;
  }

  LLVM_DEBUG(WI.print(dbgs()));
  LLVM_DEBUG(dbgs() << PASS_NAME << " for Function: ");
  LLVM_DEBUG(dbgs().write_escaped(F.getName()) << '\n');

  VPOParoptTransform VP(nullptr, &F, &WI, WI.getDomTree(), WI.getLoopInfo(),
                        WI.getSE(), WI.getTargetTransformInfo(),
                        WI.getAssumptionCache(), WI.getTargetLibraryInfo(),
                        WI.getAliasAnalysis(), OmpNoFECollapse,
#if INTEL_CUSTOMIZATION
                        OptReportVerbosity::None,
#endif  // INTEL_CUSTOMIZATION
                        ORE, 2, false);

  Changed |= VP.paroptTransforms();

  return Changed;
}
} // end anonymous namespace

char VPOParoptLoopCollapse::ID = 0;
INITIALIZE_PASS_BEGIN(VPOParoptLoopCollapse, DEBUG_TYPE, PASS_NAME,
                      false, false)
INITIALIZE_PASS_DEPENDENCY(WRegionInfoWrapperPass)
INITIALIZE_PASS_DEPENDENCY(OptimizationRemarkEmitterWrapperPass)
INITIALIZE_PASS_END(VPOParoptLoopCollapse, DEBUG_TYPE, PASS_NAME, false, false)

FunctionPass *llvm::createVPOParoptLoopCollapsePass() {
  return new VPOParoptLoopCollapse();
}

bool VPOParoptLoopCollapse::runOnFunction(Function &F) {
  WRegionInfo &WI = getAnalysis<WRegionInfoWrapperPass>().getWRegionInfo();
  auto &ORE = getAnalysis<OptimizationRemarkEmitterWrapperPass>().getORE();

  LLVM_DEBUG(dbgs() << "\n\n====== Enter " << PASS_NAME << " ======\n\n");
  bool Changed = collapseLoops(F, WI, ORE);
  LLVM_DEBUG(dbgs() << "\n\n====== Exit  " << PASS_NAME << " ======\n\n");
  return Changed;
}

PreservedAnalyses VPOParoptLoopCollapsePass::run(
    Function &F, FunctionAnalysisManager &AM) {
  WRegionInfo &WI = AM.getResult<WRegionInfoAnalysis>(F);
  auto &ORE = AM.getResult<OptimizationRemarkEmitterAnalysis>(F);

  PreservedAnalyses PA;

  LLVM_DEBUG(dbgs() << "\n\n====== Enter " << PASS_NAME << " ======\n\n");
  if (!collapseLoops(F, WI, ORE))
    PA = PreservedAnalyses::all();
  else
    PA = PreservedAnalyses::none();
  LLVM_DEBUG(dbgs() << "\n\n====== Exit  " << PASS_NAME << " ======\n\n");

  return PA;
}
#endif  // INTEL_COLLAB
