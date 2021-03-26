//===- IntelVPlanScalarEvolutionHIR.cpp -------------------------*- C++ -*-===//
//
//   Copyright (C) 2021 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//

#include "IntelVPlanScalarEvolutionHIR.h"

#include "Intel_VPlan/IntelVPlan.h"
#include "Intel_VPlan/IntelVPlanValue.h"

#define DEBUG_TYPE "vplan-scalar-evolution"

using namespace llvm;
using namespace llvm::loopopt;
using namespace llvm::vpo;

VPlanSCEV *
VPlanScalarEvolutionHIR::computeAddressSCEV(const VPLoadStoreInst &LSI) {
  LLVM_DEBUG(dbgs() << "computeAddressSCEV(" << LSI << ")\n");
  LLVM_DEBUG(dbgs() << "  -> nil\n");
  return nullptr;
}

VPlanSCEV *VPlanScalarEvolutionHIR::getMinusExpr(VPlanSCEV *LHS,
                                                 VPlanSCEV *RHS) {
  return nullptr;
}

Optional<VPConstStepLinear>
VPlanScalarEvolutionHIR::asConstStepLinear(VPlanSCEV *Expr) const {
  return None;
}

Optional<VPConstStepInduction>
VPlanScalarEvolutionHIR::asConstStepInduction(VPlanSCEV *Expr) const {
  return None;
}
