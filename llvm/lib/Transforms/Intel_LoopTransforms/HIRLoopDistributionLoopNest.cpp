//===----- HIRLoopDistributionLoopNest.cpp - to enable perfect loops-------===//
//
// Copyright (C) 2015-2018 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
// Served as a wrapper to call HIRLoopDistribution
//===----------------------------------------------------------------------===//
//

#include "HIRLoopDistribution.h"

using namespace llvm;
using namespace llvm::loopopt;
using namespace llvm::loopopt::distribute;

cl::opt<bool>
    DisableDistLoopNest("disable-hir-loop-distribute-loopnest",
                        cl::desc("Disable HIR Loop Distribution LoopNest"),
                        cl::Hidden, cl::init(false));

namespace {

class HIRLoopDistributionForLoopNest : public HIRLoopDistribution {

public:
  static char ID;

  HIRLoopDistributionForLoopNest()
      : HIRLoopDistribution(ID, DistHeuristics::NestFormation) {
    initializeHIRLoopDistributionForLoopNestPass(
        *PassRegistry::getPassRegistry());
  }

  bool runOnFunction(Function &F) override {

    if (DisableDistLoopNest || skipFunction(F)) {
      if (OptReportLevel >= 3) {
        dbgs() << "LOOP DISTRIBUTION enable perfect Loop Nest Diabled \n";
      }
      return false;
    }

    return HIRLoopDistribution::runOnFunction(F);
  }
};
} // namespace

char HIRLoopDistributionForLoopNest::ID = 0;
INITIALIZE_PASS_BEGIN(HIRLoopDistributionForLoopNest,
                      "hir-loop-distribute-loopnest",
                      "HIR Loop Distribution LoopNest", false, false)
INITIALIZE_PASS_DEPENDENCY(HIRFrameworkWrapperPass)
INITIALIZE_PASS_DEPENDENCY(HIRLoopStatisticsWrapperPass)
INITIALIZE_PASS_DEPENDENCY(HIRDDAnalysisWrapperPass)
INITIALIZE_PASS_DEPENDENCY(HIRLoopResourceWrapperPass)
INITIALIZE_PASS_DEPENDENCY(HIRSafeReductionAnalysisWrapperPass)
INITIALIZE_PASS_END(HIRLoopDistributionForLoopNest,
                    "hir-loop-distribute-loopnest",
                    "HIR Loop Distribution LoopNest", false, false)

FunctionPass *llvm::createHIRLoopDistributionForLoopNestPass() {
  return new HIRLoopDistributionForLoopNest();
}
