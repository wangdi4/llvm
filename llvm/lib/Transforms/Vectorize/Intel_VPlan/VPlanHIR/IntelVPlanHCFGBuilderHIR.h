//===-- IntelVPlanHCFGBuilderHIR.h ------------------------------*- C++ -*-===//
//
//   Copyright (C) 2017-2019 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation. and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file defines VPlanHCFGBuilderHIR class which extends
/// VPlanHCFGBuilderBase with support to build a hierarchical CFG from HIR.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_VPLANHIR_INTELVPLANHCFGBUILDER_HIR_H
#define LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_VPLANHIR_INTELVPLANHCFGBUILDER_HIR_H

#include "../IntelVPlanHCFGBuilder.h"
#include "IntelVPlanVerifierHIR.h"

using namespace llvm::loopopt;

namespace llvm {

// Forward declarations
namespace loopopt {
class DDGraph;
class HLLoop;
class HIRSafeReductionAnalysis;
} // namespace loopopt

namespace vpo {

class VPlanHCFGBuilderHIR : public VPlanHCFGBuilder {

private:
  /// The outermost loop to be vectorized.
  HLLoop *TheLoop;

  /// HIR DDGraph that contains DD information for the incoming loop nest.
  const DDGraph &DDG;

  HIRSafeReductionAnalysis *SRA;

  /// Loop header VPBasicBlock to HLLoop map. To be used when building loop
  /// regions.
  SmallDenseMap<VPBasicBlock *, HLLoop *, 4> Header2HLLoop;

  VPRegionBlock *buildPlainCFG(VPLoopEntityConverterList &CvtVec) override;
  void passEntitiesToVPlan(VPLoopEntityConverterList &Cvts) override;

  void collectUniforms(VPRegionBlock *Region) override {
    // Do nothing for now
  }

public:
  VPlanHCFGBuilderHIR(const WRNVecLoopNode *WRL, HLLoop *Lp, VPlan *Plan,
                      HIRSafeReductionAnalysis *SRA, const DDGraph &DDG);

  VPLoopRegion *createLoopRegion(VPLoop *VPLp) override;
};

} // namespace vpo
} // namespace llvm

#endif // LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_VPLANHIR_INTELVPLANHCFGBUILDER_HIR_H
