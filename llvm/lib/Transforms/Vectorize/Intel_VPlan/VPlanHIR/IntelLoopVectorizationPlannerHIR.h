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
  /// True when operating in lightweight mode.
  bool LightWeightMode;

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

  void createLiveInOutLists(VPlanVector &Plan) override;

public:
  LoopVectorizationPlannerHIR(WRNVecLoopNode *WRL, HLLoop *Lp,
                              const TargetLibraryInfo *TLI,
                              const TargetTransformInfo *TTI,
                              const DataLayout *DL,
                              HIRVectorizationLegality *HIRLegal,
                              HIRDDAnalysis *DDA, VPlanVLSAnalysisHIR *VLSA,
                              bool LightWeightMode)
      : LoopVectorizationPlanner(WRL, /*Lp=*/nullptr, /*LI=*/nullptr, TLI, TTI,
                                 DL, nullptr, nullptr, VLSA),
        TheLoop(Lp), LightWeightMode(LightWeightMode), DDA(DDA),
        HIRLegality(HIRLegal) {}

  /// Generate the HIR code for the body of the vectorized loop according to the
  /// best selected VPlan. This function returns true if code generation was
  /// successful, false if there was any late bailout during CG.
  bool executeBestPlan(VPOCodeGenHIR *CG, unsigned UF);

  /// Returns true/false value if "llvm.loop.intel.vector.vecremainder"/
  /// "llvm.loop.intel.vector.novecremainder" metadata is specified. If there is
  ///  no such metadata, returns None.
  Optional<bool> readVecRemainderEnabledHIR() {
    if (TheLoop->getLoopStringMetadata("llvm.loop.intel.vector.vecremainder")) {
      DEBUG_WITH_TYPE("VPlan_pragma_metadata",
                       dbgs() << "Vector Remainder was set by the user's "
                                 "#pragma vecremainder\n");
      return true;
    }
    if (TheLoop->getLoopStringMetadata("llvm.loop.intel.vector.novecremainder")) {
      DEBUG_WITH_TYPE("VPlan_pragma_metadata",
                       dbgs() << "Scalar Remainder was set by the user's #pragma "
                                 "novecremainder\n");
      return false;
    }
    return None;
  }

  /// Returns true/false value if "llvm.loop.intel.vector.dynamic_align"/
  /// "llvm.loop.intel.vector.nodynamic_align" metadata is specified. If there
  /// is no such metadata, returns true.
  bool readDynAlignEnabledHIR() {
    if (TheLoop->getLoopStringMetadata("llvm.loop.intel.vector.dynamic_align")) {
      DEBUG_WITH_TYPE("VPlan_pragma_metadata",
                       dbgs() << "Dynamic Align was set by the user's "
                                 "#pragma vector dynamic_align\n");
      return true;
    }
    if (TheLoop->getLoopStringMetadata("llvm.loop.intel.vector.nodynamic_align")) {
      DEBUG_WITH_TYPE("VPlan_pragma_metadata",
                        dbgs() << "No dynamic Align was set by the user's "
                                  "#pragma vector nodynamic_align\n");
      return false;
    }
    return true;
  }

  /// Reads all metadata specified by pragmas
  void readLoopMetadata() {
    VectorlengthMD =
        TheLoop->getLoopStringMetadata("llvm.loop.intel.vector.vectorlength");
    IsVecRemainder = readVecRemainderEnabledHIR();
    IsDynAlign = readDynAlignEnabledHIR();
  }

  /// Return Loop Unroll Factor either forced by option or pragma
  /// or advised by optimizations.
  /// \p Forced indicates that Unroll Factor is forced.
  virtual unsigned getLoopUnrollFactor(bool *Forced = nullptr) override;

  /// Create and return Plan/VF specific CostModel object basic on global
  /// compilation settings such as presence of -x knob in command line.
  std::unique_ptr<VPlanCostModelInterface> createCostModel(
    const VPlanVector *Plan, unsigned VF) const final;

  bool unroll(VPlanVector &Plan) override;

  /// Return a pair of the <min, max> types' width used in the underlying loop.
  std::pair<unsigned, unsigned> getTypesWidthRangeInBits() const final {
    // FIXME: Implement this!
    return {8, 64};
  }
  virtual bool isNewCFGMergeEnabled() const override {
    return EnableNewCFGMergeHIR;
  }

  void createMergerVPlans(VPAnalysesFactoryBase &VPAF) override;

  void emitPeelRemainderVPLoops(unsigned VF, unsigned UF) override;
};

} // namespace vpo
} // namespace llvm

#endif // LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_VPLANHIR_INTELLOOPVECTORIZATIONPLANNER_HIR_H
