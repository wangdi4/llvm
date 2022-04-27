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

namespace loopopt {
class CanonExpr;
class HLLoop;
class RegDDRef;
}

namespace vpo {

// Substitute for SCEVAddRecExpr for HIR. The semantics of the expression is
// equivalent to {Base,+,Stride}<MainLoop>.
struct VPlanAddRecHIR {
  loopopt::CanonExpr *Base;
  // Original ref that the Base CanonExpr was part of. Used to make
  // ref created from Base consistent during vector code generation.
  // TODO - we need to see if we need to keep a vector of Refs for
  // tracking MinusExpr SCEVs. At this point, this does not look
  // necessary.
  const loopopt::RegDDRef *Ref;
  int64_t Stride;

  VPlanAddRecHIR(loopopt::CanonExpr *Base, int64_t Stride,
                 const loopopt::RegDDRef *Ref = nullptr)
      : Base(Base), Ref(Ref), Stride(Stride) {}
};

/// Implementation of VPlanScalarEvolution for HIR.
class VPlanScalarEvolutionHIR final : public VPlanScalarEvolution {
public:
  VPlanScalarEvolutionHIR(loopopt::HLLoop *MainLoop) : MainLoop(MainLoop) {}

  VPlanSCEV *computeAddressSCEV(const VPLoadStoreInst &LSI) override;

  VPlanSCEV *getMinusExpr(VPlanSCEV *LHS, VPlanSCEV *RHS) override;

  Optional<VPConstStepLinear> asConstStepLinear(VPlanSCEV *Expr) const override;

  Optional<VPConstStepInduction>
  asConstStepInduction(VPlanSCEV *Expr) const override;

public:
  static VPlanSCEV *toVPlanSCEV(VPlanAddRecHIR *Expr) {
    return reinterpret_cast<VPlanSCEV *>(Expr);
  }

  static VPlanAddRecHIR *toVPlanAddRecHIR(VPlanSCEV *Expr) {
    return reinterpret_cast<VPlanAddRecHIR *>(Expr);
  }

private:
  VPlanAddRecHIR *computeAddressSCEVImpl(const VPLoadStoreInst &LSI);

  VPlanAddRecHIR *getMinusExprImpl(VPlanAddRecHIR *LHS, VPlanAddRecHIR *RHS);

  VPlanAddRecHIR *
  makeVPlanAddRecHIR(loopopt::CanonExpr *Base, int64_t Stride,
                     const loopopt::RegDDRef *Ref = nullptr) const;

private:
  // Set of all VPlanAddRecHIR expressions created by the analysis. The analysis
  // owns the expressions. As soon as the analysis goes out of scope, all the
  // expressions are deallocated.
  // TODO: Replace SmallVector with FoldingSet.
  mutable SmallVector<std::unique_ptr<VPlanAddRecHIR>, 0> Storage;
  loopopt::HLLoop *MainLoop;
};

raw_ostream &operator<<(raw_ostream &OS, const VPlanAddRecHIR &E);

} // namespace vpo
} // namespace llvm

#endif // LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_VPLANHIR_INTEL_VPLAN_SCALAR_EVOLUTION_HIR_H

