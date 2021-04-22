//===- IntelVPlanValueTracking.h --------------------------------*- C++ -*-===//
//
//   Copyright (C) 2020 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTEL_VPLAN_VALUE_TRACKING_H
#define LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTEL_VPLAN_VALUE_TRACKING_H

#include <llvm/Support/KnownBits.h>

namespace llvm {

class AssumptionCache;
class DataLayout;
class DominatorTree;
class Instruction;
class SCEV;

namespace vpo {

class VPInstruction;
class VPlanSCEV;
class VPlanScalarEvolutionLLVM;
class VPValue;

/// Analysis for computing KnownBits for a value or an expression.
class VPlanValueTracking {
public:
  /// Compute KnownBits for an arbitrary \p Expr at point \p CtxI.
  virtual KnownBits getKnownBits(
    VPlanSCEV *Expr, const VPInstruction *CtxI) = 0;

  virtual ~VPlanValueTracking() {}
};

/// Implementation of VPlanValueTracking for LLVM IR.
class VPlanValueTrackingLLVM final : public VPlanValueTracking {
public:
  VPlanValueTrackingLLVM(VPlanScalarEvolutionLLVM &VPSE, const DataLayout &DL,
                         AssumptionCache *AC, const DominatorTree *DT)
      : VPSE(&VPSE), DL(&DL), AC(AC), DT(DT) {}

  KnownBits getKnownBits(VPlanSCEV *Expr, const VPInstruction *CtxI) override;

private:
  KnownBits getKnownBitsImpl(const SCEV *Scev, const Instruction *CtxI);

private:
  VPlanScalarEvolutionLLVM *VPSE;
  const DataLayout *DL;
  AssumptionCache *AC;
  const DominatorTree *DT;
};

} // namespace vpo
} // namespace llvm

#endif // LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTEL_VPLAN_VALUE_TRACKING_H
