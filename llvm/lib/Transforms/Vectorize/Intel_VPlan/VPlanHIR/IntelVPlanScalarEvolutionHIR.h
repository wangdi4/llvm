//===- IntelVPlanScalarEvolutionHIR.h ---------------------------*- C++ -*-===//
//
//   Copyright (C) 2021 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_VPLANHIR_INTEL_VPLAN_SCALAR_EVOLUTION_HIR_H
#define LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_VPLANHIR_INTEL_VPLAN_SCALAR_EVOLUTION_HIR_H

#include "../IntelVPlanScalarEvolution.h"

namespace llvm {

namespace vpo {

/// Implementation of VPlanScalarEvolution for HIR.
class VPlanScalarEvolutionHIR final : public VPlanScalarEvolution {
public:
  VPlanScalarEvolutionHIR() {}

  VPlanSCEV *computeAddressSCEV(const VPLoadStoreInst &LSI) override;

  VPlanSCEV *getMinusExpr(VPlanSCEV *LHS, VPlanSCEV *RHS) override;

  Optional<VPConstStepLinear> asConstStepLinear(VPlanSCEV *Expr) const override;

  Optional<VPConstStepInduction>
  asConstStepInduction(VPlanSCEV *Expr) const override;
};

} // namespace vpo
} // namespace llvm

#endif // LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_VPLANHIR_INTEL_VPLAN_SCALAR_EVOLUTION_HIR_H

