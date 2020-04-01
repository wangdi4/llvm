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

#include <llvm/ADT/Optional.h>

namespace llvm {
namespace vpo {

class VPValue;

/// Opaque SCEV-like expression.
class VPlanSCEV;

/// Linear variable represented as a uniform base and a constant step.
struct VPConstStepLinear {
  VPlanSCEV *UniformBase;
  int64_t Step;
};

/// Induction variable represented as an invariant base and a constant step.
struct VPConstStepInduction {
  VPlanSCEV *InvariantBase;
  int64_t Step;
};

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

  /// Return (LHS - RHS).
  virtual VPlanSCEV *getMinusExpr(VPlanSCEV *LHS, VPlanSCEV *RHS) = 0;

  /// Check if \p Expr is a linear value. If we can prove that it is, return its
  /// components in the corresponding data structure.
  virtual Optional<VPConstStepLinear>
  asConstStepLinear(VPlanSCEV *Expr) const = 0;

  /// Check if \p Expr is an induction variable. If we can prove that it is,
  /// return its components in the corresponding data structure.
  virtual Optional<VPConstStepInduction>
  asConstStepInduction(VPlanSCEV *Expr) const = 0;
};

/// Implementation of VPlanScalarEvolution for LLVM IR.
class VPlanScalarEvolutionLLVM : public VPlanScalarEvolution {
public:
  VPlanScalarEvolutionLLVM(ScalarEvolution &SE, const Loop *MainLoop)
      : SE(&SE), MainLoop(MainLoop) {}

  VPlanSCEV *getVPlanSCEV(const VPValue &V) override;

  VPlanSCEV *getMinusExpr(VPlanSCEV *LHS, VPlanSCEV *RHS) override;

  Optional<VPConstStepLinear> asConstStepLinear(VPlanSCEV *Expr) const override;

  Optional<VPConstStepInduction>
  asConstStepInduction(VPlanSCEV *Expr) const override;

  ScalarEvolution &getSE() { return *SE; }

  static const SCEV *toSCEV(VPlanSCEV *Expr) {
    return reinterpret_cast<SCEV *>(Expr);
  }

  static VPlanSCEV *toVPlanSCEV(const SCEV *Expr) {
    return reinterpret_cast<VPlanSCEV *>(const_cast<SCEV *>(Expr));
  }

private:
  ScalarEvolution *SE;
  const Loop *MainLoop;
};

} // namespace vpo
} // namespace llvm

#endif // LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTEL_VPLAN_SCALAR_EVOLUTION_H
