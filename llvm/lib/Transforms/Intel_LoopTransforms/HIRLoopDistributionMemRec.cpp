//===----- HIRLoopDistributionMemRec.cpp - to break memory recurrence -----===//
//
// Copyright (C) 2015-2017 Intel Corporation. All rights reserved.
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
}

char HIRLoopDistributionForMemRec::ID = 0;
INITIALIZE_PASS_BEGIN(HIRLoopDistributionForMemRec,
                      "hir-loop-distribute-memrec",
                      "HIR Loop Distribution MemRec", false, false)
INITIALIZE_PASS_DEPENDENCY(HIRFramework)
INITIALIZE_PASS_DEPENDENCY(HIRLoopStatistics)
INITIALIZE_PASS_DEPENDENCY(HIRDDAnalysis)
INITIALIZE_PASS_END(HIRLoopDistributionForMemRec, "hir-loop-distribute-memrec",
                    "HIR Loop Distribution MemRec", false, false)

FunctionPass *llvm::createHIRLoopDistributionForMemRecPass() {
  return new HIRLoopDistributionForMemRec();
}
