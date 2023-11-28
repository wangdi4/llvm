//===-- IntelVPlanHCFGBuilderHIR.h ------------------------------*- C++ -*-===//
//
//   INTEL CONFIDENTIAL
//
//   Copyright (C) 2017 Intel Corporation
//
//   This software and the related documents are Intel copyrighted materials,
//   and your use of them is governed by the express license under which they
//   were provided to you ("License").  Unless the License provides otherwise,
//   you may not use, modify, copy, publish, distribute, disclose or treansmit
//   this software or the related documents without Intel's prior written
//   permission.
//
//   This software and the related documents are provided as is, with no
//   express or implied warranties, other than those that are expressly
//   stated in the License.
//
//===----------------------------------------------------------------------===//
///
/// \file IntelVPlanHCFGBuilderHIR.h
/// This file defines VPlanHCFGBuilderHIR class which extends
/// VPlanHCFGBuilderBase with support to build a hierarchical CFG from HIR.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_HIR_INTELVPLANHCFGBUILDER_HIR_H
#define LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_HIR_INTELVPLANHCFGBUILDER_HIR_H

#include "../IntelVPlanHCFGBuilder.h"
#include "IntelVPlanVerifierHIR.h"
#include "LegalityHIR.h"

using namespace llvm::loopopt;

namespace llvm {

// Forward declarations
namespace loopopt {
class DDGraph;
class HLLoop;
} // namespace loopopt

namespace vpo {

class VPlanHCFGBuilderHIR : public VPlanHCFGBuilder {

private:
  /// The outermost loop to be vectorized.
  HLLoop *TheLoop;

  /// HIR DDGraph that contains DD information for the incoming loop nest.
  const DDGraph &DDG;

  LegalityHIR *HIRLegality;

  /// Loop header VPBasicBlock to HLLoop map.
  SmallDenseMap<VPBasicBlock *, HLLoop *, 4> Header2HLLoop;

  bool buildPlainCFG(VPLoopEntityConverterList &CvtVec) override;

  void populateVPLoopMetadata(VPLoopInfo *VPLInfo) override;

  void passEntitiesToVPlan(VPLoopEntityConverterList &Cvts) override;

public:
  VPlanHCFGBuilderHIR(const WRNVecLoopNode *WRL, HLLoop *Lp, VPlanVector *Plan,
                      LegalityHIR *Legality, const DDGraph &DDG,
                      const DominatorTree &DT, AssumptionCache &AC);
};

} // namespace vpo
} // namespace llvm

#endif // LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_HIR_INTELVPLANHCFGBUILDER_HIR_H
