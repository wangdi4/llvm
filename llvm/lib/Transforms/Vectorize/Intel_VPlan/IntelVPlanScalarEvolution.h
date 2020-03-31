//===- IntelVPlanScalarEvolution.h ------------------------------*- C++ -*-===//
//
//   Copyright (C) 2020 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTEL_VPLAN_SCALAR_EVOLUTION_H
#define LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTEL_VPLAN_SCALAR_EVOLUTION_H

#include <llvm/Analysis/ScalarEvolution.h>

namespace llvm {
namespace vpo {

class VPValue;

/// Opaque SCEV-like expression.
class VPlanSCEV;

/// SCEV-like analysis with a common interface for LLVM IR and HIR-based
/// VPlans.
class VPlanScalarEvolution {
public:
  VPlanScalarEvolution() = default;
  VPlanScalarEvolution(const VPlanScalarEvolution &) = delete;
  VPlanScalarEvolution &operator=(const VPlanScalarEvolution &) = delete;
  virtual ~VPlanScalarEvolution() {}

  /// Compute VPlanSCEV expression for value \p V.
  virtual VPlanSCEV *getVPlanSCEV(const VPValue &V) = 0;
};

/// Implementation of VPlanScalarEvolution for LLVM IR.
class VPlanScalarEvolutionLLVM : public VPlanScalarEvolution {
public:
  VPlanScalarEvolutionLLVM(ScalarEvolution &SE) : SE(&SE) {}

  VPlanSCEV *getVPlanSCEV(const VPValue &V) override;

  ScalarEvolution &getSE() { return *SE; }

  static const SCEV *toSCEV(VPlanSCEV *Expr) {
    return reinterpret_cast<SCEV *>(Expr);
  }

  static VPlanSCEV *toVPlanSCEV(const SCEV *Expr) {
    return reinterpret_cast<VPlanSCEV *>(const_cast<SCEV *>(Expr));
  }

private:
  ScalarEvolution *SE;
};

} // namespace vpo
} // namespace llvm

#endif // LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTEL_VPLAN_SCALAR_EVOLUTION_H
