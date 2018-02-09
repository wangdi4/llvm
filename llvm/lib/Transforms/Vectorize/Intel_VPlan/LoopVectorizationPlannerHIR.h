//===-- LoopVectorizationPlannerHIR.h ---------------------------*- C++ -*-===//
//
//   Copyright (C) 2016-2017 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation. and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file defines LoopVectorizationPlannerHIR.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_LOOPVECTORIZATIONPLANNER_HIR_H
#define LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_LOOPVECTORIZATIONPLANNER_HIR_H

#include "LoopVectorizationPlanner.h"
#include "VPlanHCFGBuilderHIR.h"

namespace llvm {
namespace vpo {

class LoopVectorizationPlannerHIR : public LoopVectorizationPlannerBase {
private:
  /// The loop that we evaluate.
  HLLoop *TheLoop;

  /// HIR DDGraph that contains DD information for the incoming loop nest.
  const DDGraph &DDG;

  std::shared_ptr<IntelVPlan> buildInitialVPlan(unsigned StartRangeVF,
                                                unsigned &EndRangeVF) {
    // Create new empty VPlan
    std::shared_ptr<IntelVPlan> SharedPlan = std::make_shared<IntelVPlan>();
    IntelVPlan *Plan = SharedPlan.get();

    // Build hierarchical CFG
    VPlanHCFGBuilderHIR HCFGBuilder(WRLp, TheLoop, Plan, Legal, DDG);
    HCFGBuilder.buildHierarchicalCFG();

    return SharedPlan;
  }

public:
  LoopVectorizationPlannerHIR(WRNVecLoopNode *WRL, HLLoop *Lp,
                              const TargetLibraryInfo *TLI,
                              const TargetTransformInfo *TTI,
                              VPOVectorizationLegality *Legal,
                              const DDGraph &DDG)
      : LoopVectorizationPlannerBase(WRL, TLI, TTI, Legal), TheLoop(Lp),
        DDG(DDG) {}

  /// Generate the HIR code for the body of the vectorized loop according to the
  /// best selected VPlan.
  void executeBestPlan(VPOCodeGenHIR *CG);
};

} // namespace vpo
} // namespace llvm

#endif // LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_LOOPVECTORIZATIONPLANNER_HIR_H
