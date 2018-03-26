//===-- LoopVectorizationPlannerHIR.cpp -----------------------------------===//
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

#include "LoopVectorizationPlannerHIR.h"
#include "VPOCodeGenHIR.h"

#define DEBUG_TYPE "LoopVectorizationPlannerHIR"

using namespace llvm;
using namespace llvm::vpo;

void LoopVectorizationPlannerHIR::executeBestPlan(VPOCodeGenHIR *CG) {
  assert(BestVF != 1 && "Non-vectorized loop should be handled elsewhere!");
  VPlan *Plan = getVPlanForVF(BestVF);

  CG->initializeVectorLoop(BestVF);
  Plan->executeHIR(CG);
  CG->finalizeVectorLoop();
}
