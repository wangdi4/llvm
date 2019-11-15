//===----- HIRLoopDistributionMemRec.cpp - to break memory recurrence -----===//
//
// Copyright (C) 2015-2019 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
// Served as a wrapper to call HIRLoopDistribution
//===----------------------------------------------------------------------===//
//

#include "HIRLoopDistributionImpl.h"
#include "llvm/InitializePasses.h"
#include "llvm/Transforms/Intel_LoopTransforms/HIRLoopDistributionForMemRec.h"

using namespace llvm;
using namespace llvm::loopopt;
using namespace llvm::loopopt::distribute;

#define DEBUG_TYPE "hir-loop-distribute"

cl::opt<bool>
    DisableDistMemRec("disable-hir-loop-distribute-memrec",
                      cl::desc("Disable HIR Loop Distribution MemRec"),
                      cl::Hidden, cl::init(false));

PreservedAnalyses
HIRLoopDistributionForMemRecPass::run(llvm::Function &F,
                                      llvm::FunctionAnalysisManager &AM) {
  if (DisableDistMemRec) {
    LLVM_DEBUG(dbgs() << "LOOP DISTRIBUTION Break MemRec disabled\n");
    return PreservedAnalyses::all();
  }

  HIRLoopDistribution(
      AM.getResult<HIRFrameworkAnalysis>(F), AM.getResult<HIRDDAnalysisPass>(F),
      AM.getResult<HIRSafeReductionAnalysisPass>(F),
      AM.getResult<HIRSparseArrayReductionAnalysisPass>(F),
      AM.getResult<HIRLoopResourceAnalysis>(F), DistHeuristics::BreakMemRec)
      .run();

  return PreservedAnalyses::all();
}

namespace {

class HIRLoopDistributionForMemRecLegacyPass
    : public HIRLoopDistributionLegacyPass {

public:
  static char ID;

  HIRLoopDistributionForMemRecLegacyPass()
      : HIRLoopDistributionLegacyPass(ID, DistHeuristics::BreakMemRec) {
    initializeHIRLoopDistributionForMemRecLegacyPassPass(
        *PassRegistry::getPassRegistry());
  }

  bool runOnFunction(Function &F) override {
    if (DisableDistMemRec) {
      LLVM_DEBUG(dbgs() << "LOOP DISTRIBUTION Break MemRec disabled\n");
      return false;
    }

    return HIRLoopDistributionLegacyPass::runOnFunction(F);
  }
};
} // namespace

char HIRLoopDistributionForMemRecLegacyPass::ID = 0;
INITIALIZE_PASS_BEGIN(HIRLoopDistributionForMemRecLegacyPass,
                      "hir-loop-distribute-memrec",
                      "HIR Loop Distribution MemRec", false, false)
INITIALIZE_PASS_DEPENDENCY(HIRFrameworkWrapperPass)
INITIALIZE_PASS_DEPENDENCY(HIRLoopStatisticsWrapperPass)
INITIALIZE_PASS_DEPENDENCY(HIRDDAnalysisWrapperPass)
INITIALIZE_PASS_DEPENDENCY(HIRLoopResourceWrapperPass)
INITIALIZE_PASS_DEPENDENCY(HIRSafeReductionAnalysisWrapperPass)
INITIALIZE_PASS_DEPENDENCY(HIRSparseArrayReductionAnalysisWrapperPass)
INITIALIZE_PASS_END(HIRLoopDistributionForMemRecLegacyPass,
                    "hir-loop-distribute-memrec",
                    "HIR Loop Distribution MemRec", false, false)

FunctionPass *llvm::createHIRLoopDistributionForMemRecPass() {
  return new HIRLoopDistributionForMemRecLegacyPass();
}
