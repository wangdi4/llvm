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
class VPLoadStoreInst;

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

  /// Compute VPlanSCEV expression for the memory address accessed by \p LSI.
  /// NOTE: Implementations of this method rely on looking into underlying IR,
  ///       so this method cannot be used after underlying IR is modified. That
  ///       is, this method cannot be used in VPO CodeGen.
  /// NOTE: Clients should always prefer using VPLoadStoreInst::getAddressSCEV()
  ///       method. getAddressSCEV() is safe to use even in CodeGen, and it is
  ///       potentially more powerful (AddressSCEV can be set even for values
  ///       without underlying IR).
  virtual VPlanSCEV *computeAddressSCEV(const VPLoadStoreInst &LSI) = 0;

  /// Return (LHS - RHS).
  virtual VPlanSCEV *getMinusExpr(VPlanSCEV *LHS, VPlanSCEV *RHS) = 0;

  /// Check if \p Expr is a linear value. If we can prove that it is, return its
  /// components in the corresponding data structure.
  virtual std::optional<VPConstStepLinear>
  asConstStepLinear(VPlanSCEV *Expr) const = 0;

  /// Check if \p Expr is an induction variable. If we can prove that it is,
  /// return its components in the corresponding data structure.
  virtual std::optional<VPConstStepInduction>
  asConstStepInduction(VPlanSCEV *Expr) const = 0;

  // As of now, any access to private memory can be modified in-place without
  // invalidating the corresponding load/store instruction (e.g. AOS to SOA
  // transformation). If it is the case, it is incorrect to compute VPlanSCEV
  // based on underlying IR or HIR.
  static bool maybePointerToPrivateMemory(const VPValue &V);
};

/// Implementation of VPlanScalarEvolution for LLVM IR.
class VPlanScalarEvolutionLLVM : public VPlanScalarEvolution {
public:
  VPlanScalarEvolutionLLVM(ScalarEvolution &SE, const Loop *MainLoop,
                           LLVMContext &Context, const DataLayout *DL)
      : SE(&SE), MainLoop(MainLoop), Context(Context), DL(DL) {}

  VPlanSCEV *computeAddressSCEV(const VPLoadStoreInst &LSI) override;

  VPlanSCEV *getMinusExpr(VPlanSCEV *LHS, VPlanSCEV *RHS) override;

  std::optional<VPConstStepLinear>
  asConstStepLinear(VPlanSCEV *Expr) const override;

  std::optional<VPConstStepInduction>
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
  LLVMContext &Context;
  const DataLayout *DL;
};

} // namespace vpo
} // namespace llvm

#endif // LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTEL_VPLAN_SCALAR_EVOLUTION_H
