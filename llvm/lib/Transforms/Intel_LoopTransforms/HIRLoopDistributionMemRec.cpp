//===----- HIRLoopDistributionMemRec.cpp - to break memory recurrence -----===//
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

#include "HIRLoopDistributionImpl.h"

using namespace llvm;
using namespace llvm::loopopt;
using namespace llvm::loopopt::distribute;

cl::opt<bool>
    DisableDistMemRec("disable-hir-loop-distribute-memrec",
                      cl::desc("Disable HIR Loop Distribution MemRec"),
                      cl::Hidden, cl::init(false));

namespace {

class HIRLoopDistributionForMemRec : public HIRLoopDistribution {

public:
  static char ID;

  HIRLoopDistributionForMemRec()
      : HIRLoopDistribution(ID, DistHeuristics::BreakMemRec) {
    initializeHIRLoopDistributionForMemRecPass(
        *PassRegistry::getPassRegistry());
  }

  bool runOnFunction(Function &F) override {

    if (DisableDistMemRec || skipFunction(F)) {
      if (OptReportLevel >= 3) {
        dbgs() << "LOOP DISTRIBUTION Break MemRec disabled \n";
      }
      return false;
    }
    return HIRLoopDistribution::runOnFunction(F);
  }
};
} // namespace

char HIRLoopDistributionForMemRec::ID = 0;
INITIALIZE_PASS_BEGIN(HIRLoopDistributionForMemRec,
                      "hir-loop-distribute-memrec",
                      "HIR Loop Distribution MemRec", false, false)
INITIALIZE_PASS_DEPENDENCY(HIRFrameworkWrapperPass)
INITIALIZE_PASS_DEPENDENCY(HIRLoopStatisticsWrapperPass)
INITIALIZE_PASS_DEPENDENCY(HIRDDAnalysisWrapperPass)
INITIALIZE_PASS_DEPENDENCY(HIRLoopResourceWrapperPass)
INITIALIZE_PASS_DEPENDENCY(HIRSafeReductionAnalysisWrapperPass)
INITIALIZE_PASS_END(HIRLoopDistributionForMemRec, "hir-loop-distribute-memrec",
                    "HIR Loop Distribution MemRec", false, false)

FunctionPass *llvm::createHIRLoopDistributionForMemRecPass() {
  return new HIRLoopDistributionForMemRec();
}
