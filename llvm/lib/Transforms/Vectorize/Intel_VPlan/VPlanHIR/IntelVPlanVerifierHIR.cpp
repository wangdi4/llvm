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
#include "llvm/Analysis/Intel_LoopAnalysis/IR/HLLoop.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HLNodeUtils.h"

#define DEBUG_TYPE "vplan-verifier"

using namespace llvm;
using namespace vpo;
using namespace loopopt;

VPlanVerifierHIR::VPlanVerifierHIR(const loopopt::HLLoop *HLLp)
    : VPlanVerifier(HLLp->getHLNodeUtils().getDataLayout()), TheLoop(HLLp) {}

unsigned VPlanVerifierHIR::countLoopsInUnderlyingIR() const {
  assert(TheLoop && "TheLoop can't be null.");
  SmallVector<const HLLoop *, 8> Loops;
  HLNodeUtils::gatherAllLoops(TheLoop, Loops);
  return Loops.size();
}
