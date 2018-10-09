//===-- IntelLoopVectorizationPlannerHIR.cpp ------------------------------===//
//
//   Copyright (C) 2016-2017 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation. and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file implements LoopVectorizationPlannerHIR.
///
//===----------------------------------------------------------------------===//

#include "IntelLoopVectorizationPlannerHIR.h"
#include "IntelVPOCodeGenHIR.h"

#define DEBUG_TYPE "LoopVectorizationPlannerHIR"

using namespace llvm;
using namespace llvm::vpo;

cl::opt<uint64_t>
    VPlanDefaultEstTripHIR("vplan-default-est-trip-hir", cl::init(300),
                           cl::desc("Default estimated trip count"));

void LoopVectorizationPlannerHIR::executeBestPlan(VPOCodeGenHIR *CG) {
  assert(BestVF != 1 && "Non-vectorized loop should be handled elsewhere!");
  VPlan *Plan = getVPlanForVF(BestVF);

  // Collect OVLS memrefs and groups for the VF chosen by cost modeling.
  VPlanVLSAnalysis *VLSA = CG->getVLS();
  VLSA->getOVLSMemrefs(Plan, BestVF);

  CG->initializeVectorLoop(BestVF);
  Plan->executeHIR(CG);
  CG->finalizeVectorLoop();
}
