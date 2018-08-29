//===- IntelVPLoopAnalysisHIR.h -------------------------------------------===//
//
//   Copyright (C) 2018 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation. and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//
//
// \file
/// This file contains concrete realization of VPLoopAnalysis for HIR.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORM_VECTORIZE_INTEL_VPLAN_VPLAN_HIR_INTELVPLOOPANALYSISHIR_H
#define LLVM_TRANSFORM_VECTORIZE_INTEL_VPLAN_VPLAN_HIR_INTELVPLOOPANALYSISHIR_H

#include "../IntelVPlan.h"
#include "../IntelVPLoopAnalysis.h"
#include "llvm/Analysis/Intel_LoopAnalysis/IR/HLLoop.h"

namespace llvm {
namespace vpo {

class VPLoop;

class VPLoopAnalysisHIR : public VPLoopAnalysisBase {
private:
  void computeTripCountImpl(const VPLoopRegion *Lp) final;

public:
  explicit VPLoopAnalysisHIR(const uint64_t DefaultTripCount)
      : VPLoopAnalysisBase(DefaultTripCount) {}
};

} // namespace vpo
} // namespace llvm

#endif // LLVM_TRANSFORM_VECTORIZE_INTEL_VPLAN_VPLAN_HIR_INTELVPLOOPANALYSISHIR_H
