//===-- IntelVPlanVerifierHIR.cpp -----------------------------------------===//
//
//   Copyright (C) 2017 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//
//
// This file defines VPlanVerifierHIR class which specializes VPlan
// verification algorithm with HIR-specific checks.
//
//===----------------------------------------------------------------------===//

#include "IntelVPlanVerifierHIR.h"
#include "IntelVPLoopRegionHIR.h"
#include "llvm/Analysis/Intel_LoopAnalysis/IR/HLLoop.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HLNodeUtils.h"

#define DEBUG_TYPE "vplan-verifier"

using namespace llvm;
using namespace vpo;
using namespace loopopt;

unsigned VPlanVerifierHIR::countLoopsInUnderlyingIR() const {
  assert(TheLoop && "TheLoop can't be null.");
  SmallVector<const HLLoop *, 8> Loops;
  TheLoop->getHLNodeUtils().gatherAllLoops(TheLoop, Loops);
  return Loops.size();
}

void VPlanVerifierHIR::verifyIRSpecificLoopRegion(
    const VPRegionBlock *Region) const {

  if (const auto *LoopRHIR = dyn_cast<VPLoopRegionHIR>(Region)) {
    assert(LoopRHIR->getHLLoop() &&
           "VPLoopRegionHIR must have a valid HLLoop.");
    (void)LoopRHIR;
  }
}

