//===-- IntelLoopVectorizationPlannerHIR.h ----------------------*- C++ -*-===//
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

#ifndef LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_VPLANHIR_INTELLOOPVECTORIZATIONPLANNER_HIR_H
#define LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_VPLANHIR_INTELLOOPVECTORIZATIONPLANNER_HIR_H

#include "../IntelLoopVectorizationPlanner.h"
#include "IntelVPLoopAnalysisHIR.h"
#include "IntelVPlanHCFGBuilderHIR.h"
#include "IntelVPlanVLSAnalysisHIR.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRDDAnalysis.h"
#include "llvm/Support/CommandLine.h"

extern cl::opt<uint64_t> VPlanDefaultEstTripHIR;

namespace llvm {
namespace vpo {

class LoopVectorizationPlannerHIR : public LoopVectorizationPlanner {
private:
  /// The loop that we evaluate.
  HLLoop *TheLoop;

  /// HIR DDGraph that contains DD information for the incoming loop nest.
  loopopt::HIRDDAnalysis *DDA;

  std::shared_ptr<VPLoopAnalysisBase> VPLA;

  std::shared_ptr<VPlan> buildInitialVPlan(unsigned StartRangeVF,
                                           unsigned &EndRangeVF) override {
    // Create new empty VPlan
    std::shared_ptr<VPlan> SharedPlan = std::make_shared<VPlan>(VPLA);
    VPlan *Plan = SharedPlan.get();

    // Build hierarchical CFG
    const DDGraph &DDG = DDA->getGraph(TheLoop);

    VPlanHCFGBuilderHIR HCFGBuilder(WRLp, TheLoop, Plan, Legal, DDG);
    HCFGBuilder.buildHierarchicalCFG();

    return SharedPlan;
  }

public:
  LoopVectorizationPlannerHIR(WRNVecLoopNode *WRL, HLLoop *Lp,
                              const TargetLibraryInfo *TLI,
                              const TargetTransformInfo *TTI,
                              const DataLayout *DL,
                              VPOVectorizationLegality *Legal,
                              HIRDDAnalysis *DDA, VPlanVLSAnalysisHIR *VLSA)
      : LoopVectorizationPlanner(WRL, nullptr, nullptr, nullptr, TLI, TTI, DL,
                                 nullptr, Legal, VLSA), 
        TheLoop(Lp), DDA(DDA) {
    VPLA = std::make_shared<VPLoopAnalysisHIR>(VPlanDefaultEstTripHIR);
  }

  /// Generate the HIR code for the body of the vectorized loop according to the
  /// best selected VPlan.
  void executeBestPlan(VPOCodeGenHIR *CG);
  void collectDeadInstructions() override {}
  /// Return a pair of the <min, max> types' width used in the underlying loop.
  std::pair<unsigned, unsigned> getTypesWidthRangeInBits() const final {
    // FIXME: Implement this!
    return {8, 64};
  }
};

} // namespace vpo
} // namespace llvm

#endif // LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_VPLANHIR_INTELLOOPVECTORIZATIONPLANNER_HIR_H
