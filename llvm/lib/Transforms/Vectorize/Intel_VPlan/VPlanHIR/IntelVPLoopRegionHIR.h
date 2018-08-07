//===- IntelVPLoopRegionHIR.h ---------------------------------------------===//
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
/// This file contains declaration of VPLoopRegionHIR class.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORM_VECTORIZE_INTEL_VPLAN_VPLAN_HIR_INTELVPLOOPREGIONHIR_H
#define LLVM_TRANSFORM_VECTORIZE_INTEL_VPLAN_VPLAN_HIR_INTELVPLOOPREGIONHIR_H

#include "../IntelVPlan.h"
#include "IntelLoopVectorizationPlannerHIR.h"
#include "IntelVPlanDecomposerHIR.h"
#include "IntelVPlanVerifierHIR.h"

namespace llvm {
namespace vpo {

class VPLoopRegionHIR : public VPLoopRegion {
  friend class VPDecomposerHIR;
  friend class VPLoopAnalysisHIR;
  friend class VPlanVerifierHIR;
  friend class VPlanHCFGBuilderHIR;

private:
  // Pointer to the underlying HLLoop.
  loopopt::HLLoop *HLLp;

  const loopopt::HLLoop *getHLLoop() const { return HLLp; }
  loopopt::HLLoop *getHLLoop() { return HLLp; }

  VPLoopRegionHIR(const std::string &Name, VPLoop *VPLp, loopopt::HLLoop *HLLp)
      : VPLoopRegion(VPLoopRegionHIRSC, Name, VPLp), HLLp(HLLp) {}

public:
  /// Method to support type inquiry through isa, cast, and dyn_cast.
  static inline bool classof(const VPBlockBase *B) {
    return B->getVPBlockID() == VPBlockBase::VPLoopRegionHIRSC;
  }
};

} // namespace vpo
} // namespace llvm
#endif // LLVM_TRANSFORM_VECTORIZE_INTEL_VPLAN_VPLAN_HIR_INTELVPLOOPREGIONHIR_H
