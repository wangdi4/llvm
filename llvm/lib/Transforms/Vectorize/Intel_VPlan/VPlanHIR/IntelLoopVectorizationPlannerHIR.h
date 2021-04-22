//===-- IntelLoopVectorizationPlannerHIR.h ----------------------*- C++ -*-===//
//
//   Copyright (C) 2016-2019 Intel Corporation. All rights reserved.
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
#include "IntelVPlanHCFGBuilderHIR.h"
#include "IntelVPlanVLSAnalysisHIR.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRDDAnalysis.h"
#include "llvm/Support/CommandLine.h"

extern cl::opt<uint64_t> VPlanDefaultEstTripHIR;

namespace llvm {
namespace llopopt {
class HIRSafeReductionAnalysis;
}
namespace vpo {
using namespace loopopt;

class LoopVectorizationPlannerHIR : public LoopVectorizationPlanner {
private:
  /// The loop that we evaluate.
  HLLoop *TheLoop;

  /// HIR DDGraph that contains DD information for the incoming loop nest.
  HIRDDAnalysis *DDA;

  HIRVectorizationLegality *HIRLegality;

  std::shared_ptr<VPlanVector>
  buildInitialVPlan(VPExternalValues &Ext, VPUnlinkedInstructions &UVPI,
                    std::string VPlanName,
                    ScalarEvolution *SE = nullptr) override;

  /// Replace original upper bound of the loop with
  /// VPVectorTripCountCalculation.
  void emitVecSpecifics(VPlanVector *Plan) override;

protected:
  /// Check whether everything in the loop body is supported at the moment.
  /// We can have some unimplemented things and it's better to gracefully
  /// bailout in such cases than assert or generate incorrect code.
  bool canProcessLoopBody(const VPlanVector &Plan, const VPLoop &Loop) const override;

public:
  LoopVectorizationPlannerHIR(WRNVecLoopNode *WRL, HLLoop *Lp,
                              const TargetLibraryInfo *TLI,
                              const TargetTransformInfo *TTI,
                              const DataLayout *DL,
                              HIRVectorizationLegality *HIRLegal,
                              HIRDDAnalysis *DDA, VPlanVLSAnalysisHIR *VLSA)
      : LoopVectorizationPlanner(WRL, /*Lp=*/nullptr, /*LI=*/nullptr, TLI, TTI,
                                 DL, nullptr, nullptr, VLSA),
        TheLoop(Lp), DDA(DDA), HIRLegality(HIRLegal) {}

  /// Generate the HIR code for the body of the vectorized loop according to the
  /// best selected VPlan. This function returns true if code generation was
  /// successful, false if there was any late bailout during CG.
  bool executeBestPlan(VPOCodeGenHIR *CG, unsigned UF);

  /// Return Loop Unroll Factor either forced by option or pragma
  /// or advised by optimizations.
  /// \p Forced indicates that Unroll Factor is forced.
  virtual unsigned getLoopUnrollFactor(bool *Forced = nullptr) override;

  /// Return a pair of the <min, max> types' width used in the underlying loop.
  std::pair<unsigned, unsigned> getTypesWidthRangeInBits() const final {
    // FIXME: Implement this!
    return {8, 64};
  }
  virtual bool isNewCFGMergeEnabled() const override { return false;}
};

} // namespace vpo
} // namespace llvm

#endif // LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_VPLANHIR_INTELLOOPVECTORIZATIONPLANNER_HIR_H
