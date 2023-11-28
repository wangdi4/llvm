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
class Value;

namespace vpo {

class VPAssumptionCache;
class VPInstruction;
class VPlanSCEV;
class VPlanScalarEvolutionLLVM;
class VPDominatorTree;
class VPValue;

/// Analysis for computing KnownBits for a value or an expression.
class VPlanValueTracking {
public:
  /// Discriminant for LLVM-style RTTI.
  enum KindT { LLVM, HIR };
  VPlanValueTracking(KindT Kind) : Kind(Kind) {}

  virtual ~VPlanValueTracking() = default;

  /// Return which kind of value tracking this is.
  KindT getKind() const { return Kind; }

  /// Compute KnownBits for an arbitrary \p Expr at point \p CtxI.
  virtual KnownBits getKnownBits(VPlanSCEV *Expr,
                                 const VPInstruction *CtxI) = 0;

  /// Compute KnownBits for an arbitrary \p Expr, at the point \p CtxI, widened
  /// according to the given \p VF.
  KnownBits getKnownBits(const VPValue *Expr, const VPInstruction *CtxI,
                         unsigned VF = 1) const;

  /// Set whether underlying values are used when computing known bits.
  static void setUseUnderlyingValues(bool V) { UseUnderlyingValues = V; }

  /// Get whether underlying values are used when computing known bits.
  static bool getUseUnderlyingValues() { return UseUnderlyingValues; }

private:
  /// Controls whether underlying values are used when computing known bits.
  static inline bool UseUnderlyingValues = true;

  /// Discriminant for LLVM-style RTTI.
  KindT Kind;
};

/// Implementation of VPlanValueTracking for LLVM IR.
class VPlanValueTrackingLLVM final : public VPlanValueTracking {
  friend class VPlanValueTrackingImpl;

public:
  VPlanValueTrackingLLVM(VPlanScalarEvolutionLLVM &VPSE, const DataLayout &DL,
                         VPAssumptionCache *VPAC, const DominatorTree *DT,
                         const VPDominatorTree *VPDT)
      : VPlanValueTracking(VPlanValueTracking::LLVM), VPSE(&VPSE), DL(&DL),
        VPAC(VPAC), DT(DT), VPDT(VPDT) {}

  KnownBits getKnownBits(VPlanSCEV *Expr, const VPInstruction *CtxI) override;

  static bool classof(const VPlanValueTracking *VPVT) {
    return VPVT->getKind() == VPlanValueTracking::LLVM;
  }

private:
  KnownBits getKnownBitsImpl(const SCEV *Scev, const Instruction *CtxI);

private:
  VPlanScalarEvolutionLLVM *VPSE;
  const DataLayout *DL;
  VPAssumptionCache *VPAC;
  const DominatorTree *DT;
  const VPDominatorTree *VPDT;
};

} // namespace vpo
} // namespace llvm

#endif // LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTEL_VPLAN_VALUE_TRACKING_H
