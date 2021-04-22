//===-- IntelVPlanCostModelProprietary.cpp --------------------------------===//
//
//   Copyright (C) 2018-2019 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//
//
// This file implements VPlan cost modeling with Intel's IP.
//
//===----------------------------------------------------------------------===//

#include "IntelVPlanCostModelProprietary.h"
#include "IntelVPlan.h"
#include "IntelVPlanIdioms.h"
#include "IntelVPlanUtils.h"
#include "IntelVPlanVLSAnalysis.h"
#include "VPlanHIR/IntelVPlanVLSAnalysisHIR.h"
#include "VPlanHIR/IntelVPlanVLSClientHIR.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/DDRefUtils.h"
#include "llvm/Analysis/TargetTransformInfo.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Analysis/VectorUtils.h"
#include "IntelVPlanPatternMatch.h"

#define DEBUG_TYPE "vplan-cost-model-proprietary"

using namespace llvm::loopopt;

namespace llvm {

namespace vpo {

unsigned VPlanCostModelProprietary::getCost(const VPInstruction *VPInst) {
  unsigned TTICost = VPlanTTICostModel::getTTICost(VPInst);
  unsigned Cost = VPlanCostModel::getCost(VPInst);
  applyHeuristicsPipeline(TTICost, Cost, VPInst);
  return Cost;
}

} // namespace vpo

} // namespace llvm
