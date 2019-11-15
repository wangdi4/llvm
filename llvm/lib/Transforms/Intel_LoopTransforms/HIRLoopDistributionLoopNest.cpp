//===----- HIRLoopDistributionLoopNest.cpp - to enable perfect loops-------===//
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
#include "llvm/Transforms/Intel_LoopTransforms/HIRLoopDistributionForLoopNest.h"

using namespace llvm;
using namespace llvm::loopopt;
using namespace llvm::loopopt::distribute;

#define DEBUG_TYPE "hir-loop-distribute"

cl::opt<bool>
    DisableDistLoopNest("disable-hir-loop-distribute-loopnest",
                        cl::desc("Disable HIR Loop Distribution LoopNest"),
                        cl::Hidden, cl::init(false));

PreservedAnalyses
HIRLoopDistributionForLoopNestPass::run(llvm::Function &F,
                                        llvm::FunctionAnalysisManager &AM) {
  if (DisableDistLoopNest) {
    LLVM_DEBUG(
        dbgs() << "LOOP DISTRIBUTION enable perfect Loop Nest Disabled\n");
    return PreservedAnalyses::all();
  }

  HIRLoopDistribution(
      AM.getResult<HIRFrameworkAnalysis>(F), AM.getResult<HIRDDAnalysisPass>(F),
      AM.getResult<HIRSafeReductionAnalysisPass>(F),
      AM.getResult<HIRSparseArrayReductionAnalysisPass>(F),
      AM.getResult<HIRLoopResourceAnalysis>(F), DistHeuristics::NestFormation)
      .run();

  return PreservedAnalyses::all();
}

namespace {

class HIRLoopDistributionForLoopNestLegacyPass
    : public HIRLoopDistributionLegacyPass {
public:
  static char ID;

  HIRLoopDistributionForLoopNestLegacyPass()
      : HIRLoopDistributionLegacyPass(ID, DistHeuristics::NestFormation) {
    initializeHIRLoopDistributionForLoopNestLegacyPassPass(
        *PassRegistry::getPassRegistry());
  }

  bool runOnFunction(Function &F) override {
    if (DisableDistLoopNest) {
      LLVM_DEBUG(
          dbgs() << "LOOP DISTRIBUTION enable perfect Loop Nest Disabled\n");
      return false;
    }

    return HIRLoopDistributionLegacyPass::runOnFunction(F);
  }
};
} // namespace

char HIRLoopDistributionForLoopNestLegacyPass::ID = 0;
INITIALIZE_PASS_BEGIN(HIRLoopDistributionForLoopNestLegacyPass,
                      "hir-loop-distribute-loopnest",
                      "HIR Loop Distribution LoopNest", false, false)
INITIALIZE_PASS_DEPENDENCY(HIRFrameworkWrapperPass)
INITIALIZE_PASS_DEPENDENCY(HIRLoopStatisticsWrapperPass)
INITIALIZE_PASS_DEPENDENCY(HIRDDAnalysisWrapperPass)
INITIALIZE_PASS_DEPENDENCY(HIRLoopResourceWrapperPass)
INITIALIZE_PASS_DEPENDENCY(HIRSafeReductionAnalysisWrapperPass)
INITIALIZE_PASS_DEPENDENCY(HIRSparseArrayReductionAnalysisWrapperPass)
INITIALIZE_PASS_END(HIRLoopDistributionForLoopNestLegacyPass,
                    "hir-loop-distribute-loopnest",
                    "HIR Loop Distribution LoopNest", false, false)

FunctionPass *llvm::createHIRLoopDistributionForLoopNestPass() {
  return new HIRLoopDistributionForLoopNestLegacyPass();
}
