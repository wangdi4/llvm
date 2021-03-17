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
namespace vpo {

class VPlanValueTrackingHIR final : public VPlanValueTracking {
public:
  VPlanValueTrackingHIR(const DataLayout &DL) : DL(&DL) {}

  KnownBits getKnownBits(VPlanSCEV *Expr, const VPInstruction *CtxI) override;

private:
  const DataLayout *DL = nullptr;
};

} // namespace vpo
} // namespace llvm

#endif // LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_VPLANHIR_INTEL_VPLAN_VALUE_TRACKING_HIR_H
