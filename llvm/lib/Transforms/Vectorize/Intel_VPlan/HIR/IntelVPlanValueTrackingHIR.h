//===- IntelVPlanValueTrackingHIR.h -----------------------------*- C++ -*-===//
//
//   Copyright (C) 2021 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_VPLANHIR_INTEL_VPLAN_VALUE_TRACKING_HIR_H
#define LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_VPLANHIR_INTEL_VPLAN_VALUE_TRACKING_HIR_H

#include "../IntelVPlanValueTracking.h"

namespace llvm {

namespace loopopt {
class HLLoop;
class BlobUtils;
} // namespace loopopt

namespace vpo {

struct VPlanAddRecHIR;

class VPlanValueTrackingHIR final : public VPlanValueTracking {
  friend class VPlanValueTrackingImpl;

public:
  VPlanValueTrackingHIR(loopopt::HLLoop *MainLoop, const DataLayout &DL,
                        VPAssumptionCache *VPAC, const DominatorTree *DT,
                        const VPDominatorTree *VPDT)
      : VPlanValueTracking(VPlanValueTracking::HIR), MainLoop(MainLoop),
        DL(&DL), VPAC(VPAC), DT(DT), VPDT(VPDT) {}

  KnownBits getKnownBits(VPlanSCEV *Expr, const VPInstruction *CtxI) override;

  static bool classof(const VPlanValueTracking *VPVT) {
    return VPVT->getKind() == VPlanValueTracking::HIR;
  }

private:
  KnownBits getKnownBitsImpl(VPlanAddRecHIR *Expr);

  KnownBits computeKnownBitsForScev(const SCEV *Expr, Instruction *CtxI) const;

private:
  loopopt::HLLoop *MainLoop = nullptr;
  const DataLayout *DL = nullptr;
  VPAssumptionCache *VPAC = nullptr;
  const DominatorTree *DT = nullptr;
  const VPDominatorTree *VPDT = nullptr;
};

} // namespace vpo
} // namespace llvm

#endif // LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_VPLANHIR_INTEL_VPLAN_VALUE_TRACKING_HIR_H
