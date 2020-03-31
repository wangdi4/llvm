//===- IntelVPlanScalarEvolution.cpp ----------------------------*- C++ -*-===//
//
//   Copyright (C) 2020 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//

#include "IntelVPlanScalarEvolution.h"

#include "IntelVPlan.h"

#define DEBUG_TYPE "vplan-scalar-evolution"

using namespace llvm;
using namespace llvm::vpo;

VPlanSCEV *VPlanScalarEvolutionLLVM::getVPlanSCEV(const VPValue &V) {
  const SCEV *Expr = V.isUnderlyingIRValid()
                         ? SE->getSCEV(V.getUnderlyingValue())
                         : SE->getCouldNotCompute();
  return toVPlanSCEV(Expr);
}
