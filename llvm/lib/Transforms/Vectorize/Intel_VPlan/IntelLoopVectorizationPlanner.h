//===-- LoopVectorizationPlanner.h ------------------------------*- C++ -*-===//
//
//   Copyright (C) 2016-2020 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation. and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file defines LoopVectorizationPlanner.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTELLOOPVECTORIZATIONPLANNER_H
#define LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTELLOOPVECTORIZATIONPLANNER_H

#if INTEL_CUSTOMIZATION
#include "IntelVPlan.h"
#include "IntelVPlanLoopUnroller.h"
#else
#include "VPlan.h"
#endif
#include "llvm/ADT/DenseMap.h"

namespace llvm {
class Loop;
class ScalarEvolution;
class TargetLibraryInfo;
class TargetTransformInfo;

namespace loopopt {
class HLLoop;
class DDGraph;
}

using namespace llvm::loopopt;

namespace vpo {
#if INTEL_CUSTOMIZATION
class VPlanVLSAnalysis;
class WRNVecLoopNode;
#endif
class VPOCodeGen;
class VPOVectorizationLegality;
class WRNVecLoopNode;
class VPlanHCFGBuilder;
class VPlanCostModel;

extern bool PrintSVAResults;
extern bool PrintAfterCallVecDecisions;
extern bool LoopMassagingEnabled;
extern bool EnableSOAAnalysis;

/// LoopVectorizationPlanner - builds and optimizes the Vectorization Plans
/// which record the decisions how to vectorize the given loop.
/// In particular, represent the control-flow of the vectorized version,
/// the replication of instructions that are to be scalarized, and interleave
/// access groups.
class LoopVectorizationPlanner {
  friend class VPlanTestBase;

public:
#if INTEL_CUSTOMIZATION
  LoopVectorizationPlanner(WRNVecLoopNode *WRL, Loop *Lp, LoopInfo *LI,
                           const TargetLibraryInfo *TLI,
                           const TargetTransformInfo *TTI, const DataLayout *DL,
                           class DominatorTree *DT,
                           VPOVectorizationLegality *Legal,
                           VPlanVLSAnalysis *VLSA)
      : WRLp(WRL), TLI(TLI), TTI(TTI), DL(DL), Legal(Legal), TheLoop(Lp),
        LI(LI), DT(DT), VLSA(VLSA) {}
#endif // INTEL_CUSTOMIZATION

  virtual ~LoopVectorizationPlanner() {}
  /// Build initial VPlans according to the information gathered by Legal
  /// when it checked if it is legal to vectorize this loop.
  /// Returns the number of VPlans built, zero if failed.
  unsigned buildInitialVPlans(LLVMContext *Context, const DataLayout *DL,
                              std::string VPlanName,
                              ScalarEvolution *SE = nullptr);

  /// On VPlan construction, each instruction marked for predication by Legal
  /// gets its own basic block guarded by an if-then. This initial planning
  /// is legal, but is not optimal. This function attempts to leverage the
  /// necessary conditional execution of the predicated instruction in favor
  /// of other related instructions. The function applies these optimizations
  /// to all VPlans.
  // void optimizePredicatedInstructions();

  /// Select the best peeling variant for every VPlan.
  void selectBestPeelingVariants();

  /// Record CM's decision and dispose of all other VPlans.
  // void setBestPlan(unsigned VF, unsigned UF);

  // Preprocess best VPlan before CG, creating the needed auxiliary loops
  // (peel/remainder of different kinds) and merging them into flattened
  // cfg. \p UF and \p VF are selected unroll factor and vector factor,
  // correspondingly, for main VPlan.
  void emitPeelRemainderVPLoops(unsigned VF, unsigned UF);

  /// Generate the IR code for the body of the vectorized loop according to the
  /// best selected VPlan.
  void executeBestPlan(VPOCodeGen &LB);

  /// Feed information from explicit clauses to the loop Legality.
  /// This information is necessary for initial loop analysis in the CodeGen.
#if INTEL_CUSTOMIZATION
  template <class VPOVectorizationLegality>
#endif
  static void EnterExplicitData(WRNVecLoopNode *WRLp, VPOVectorizationLegality &Legal);

  /// Post VPlan FrontEnd pass to verify that we can process the VPlan that
  /// was constructed. There are some limitations in CG, CM, and other parts of
  /// VPlan vectorizer on which we better gracefully bail out than assert.
  bool canProcessVPlan(const VPlanVector &Plan);

  /// Select the best plan and dispose all other VPlans.
  /// \Returns the selected vectorization factor.
  template <typename CostModelTy = VPlanCostModel>
  unsigned selectBestPlan(void);

  /// Predicate all unique non-scalar VPlans
  void predicate(void);

  /// Insert all-zero bypasses for \p Plan.
  void insertAllZeroBypasses(VPlanVector *Plan, unsigned VF);

  /// Return Loop Unroll Factor either forced by option or pragma
  /// or advised by optimizations.
  /// \p Forced indicates that Unroll Factor is forced.
  virtual unsigned getLoopUnrollFactor(bool *Forced = nullptr);

  /// Perform VPlan loop unrolling if needed
  void
  unroll(VPlanVector &Plan,
         VPlanLoopUnroller::VPInstUnrollPartTy *VPInstUnrollPart = nullptr);

  template <typename CostModelTy = VPlanCostModel>
  void printCostModelAnalysisIfRequested(const std::string &Header);

  /// Generate the IR code for the body of the vectorized loop according to the
  /// best selected VPlan.
  // void executeBestPlan(InnerLoopVectorizer &LB);

  /// Return non-masked variant of VPlan for given VF.
  VPlanVector *getVPlanForVF(unsigned VF) const {
    auto It = VPlans.find(VF);
    return It != VPlans.end() ? It->second.MainPlan.get() : nullptr;
  }

  /// Return masked variant of VPlan for given VF.
  VPlanMasked *getMaskedVPlanForVF(unsigned VF) const {
    auto It = VPlans.find(VF);
    return It != VPlans.end() ? It->second.MaskedModeLoop.get() : nullptr;
  }

  bool hasVPlanForVF(const unsigned VF) const { return VPlans.count(VF) != 0; }

  auto getAllVPlans() const {
    return make_range(VPlans.begin(), VPlans.end());
  }

  struct VPlanPair {
    std::shared_ptr<VPlanVector> MainPlan;
    std::shared_ptr<VPlanMasked> MaskedModeLoop;
  };

  void appendVPlanPair(unsigned VF, const VPlanPair &PlanPair) {
    VPlans[VF] = PlanPair;
  }

protected:
  /// Build an initial VPlan according to the information gathered by Legal
  /// when it checked if it is legal to vectorize this loop. \return a VPlan
  /// that corresponds to vectorization factors starting from the given
  /// \p StartRangeVF and up to \p EndRangeVF, exclusive, possibly decreasing
  /// the given \p EndRangeVF.
  // TODO: If this function becomes more complicated, move common code to base
  // class.
  virtual std::shared_ptr<VPlanVector>
  buildInitialVPlan(unsigned StartRangeVF, unsigned &EndRangeVF,
                    VPExternalValues &Ext,
                    VPUnlinkedInstructions &UnlinkedVPInsts,
                    std::string VPlanName, ScalarEvolution *SE = nullptr);

  /// Transform to emit explict uniform Vector loop iv.
  virtual void emitVecSpecifics(VPlanVector *Plan);

  /// \Returns a pair of the <min, max> types' width used in the underlying loop.
  /// Doesn't take into account i1 type.
  virtual std::pair<unsigned, unsigned> getTypesWidthRangeInBits() const;

  /// Create VPLiveIn/VPLiveOut lists for VPEntities.
  virtual void createLiveInOutLists(VPlanVector &Plan);

  /// Check whether everything in the loop body is supported at the moment.
  /// We can have some unimplemented things and it's better to gracefully
  /// bailout in such cases than assert or generate incorrect code.
  virtual bool canProcessLoopBody(const VPlanVector &Plan,
                                  const VPLoop &Loop) const;

  /// If the \p Loop has a normalized IV then return upper bound of the loop and
  /// compare instruction where it's used. Otherwise return <nullptr, nullptr>.
  std::pair<VPValue *, VPInstruction *>
  getLoopUpperBound(const VPLoop *Loop) const;

  /// Returns true if the loop has normalized induction:
  /// - the main induction is integer
  /// - the induction is incremented with step 1
  /// - start value is 0
  /// - upper bound is invariant
  /// - the update instruction is used only in latch condition and
  ///   in the header phi
  /// - the latch condition is used only as back-edge condition.
  bool hasNormalizedInduction(const VPLoop *Loop) const;

  /// WRegion info of the loop we evaluate. It can be null.
  WRNVecLoopNode *WRLp;

  /// Target Library Info.
  const TargetLibraryInfo *TLI;

  /// Target Transform Info.
  const TargetTransformInfo *TTI;

  /// Data Layout
  const DataLayout *DL;

  /// The legality analysis.
  // TODO: Turn into a reference when supported for HIR.
  LoopVectorizationLegality *Legal;

  /// This class is copied from open-source LoopVectorize.cpp and it's supposed
  /// to be temporal. VPO doesn't need it but we have it to minimize divergency
  /// with TransformState.
  struct VPCallbackILV : public VPCallback {

    ~VPCallbackILV() override {}

    Value *getOrCreateVectorValues(Value *V, unsigned Part) override {
      llvm_unreachable("Not implemented");
      return nullptr;
    }
  };
  unsigned BestVF = 0;
  unsigned BestUF = 0;

  // Storage for common external data (VPExternalDefs, Uses, Consts etc).
  std::unique_ptr<VPExternalValues> Externals;

  // Storage for VPInstructions that have been removed from VPlan and unlinked.
  std::unique_ptr<VPUnlinkedInstructions> UnlinkedVPInsts;

private:
  /// Determine whether \p I will be scalarized in a given range of VFs.
  /// The returned value reflects the result for a prefix of the range, with \p
  /// EndRangeVF modified accordingly.
  // bool willBeScalarized(Instruction *I, unsigned StartRangeVF,
  //                      unsigned &EndRangeVF);

  /// Iteratively sink the scalarized operands of a predicated instruction into
  /// the block that was created for it.
  // void sinkScalarOperands(Instruction *PredInst, VPlan *Plan);

  void runInitialVecSpecificTransforms(VPlanVector *Plan);

  /// Main function that canonicalizes the CFG and applyies loop massaging
  /// transformations like mergeLoopExits transform.
  void doLoopMassaging(VPlanVector *Plan);

  /// Use results from VPEntity analysis to emit explicit VPInstruction-based
  /// representation. The analysis results can be invalidated/stale after this
  /// transform.
  void emitVPEntityInstrs(VPlanVector *Plan);

  /// Emit uniform IV for the vector loop and rewrite backedge condition to use
  /// it.
  //
  //   header:
  //     %iv = phi [ 0, preheader ], [ iv.next, latch ]
  //
  //   latch:
  //     %iv.next = %iv + VF
  //     %cond = %iv.next `icmp` TripCount
  //     br i1 %cond
  //
  // The order of latch's successors isn't changed which is ensured by selecting
  // proper icmp predicate (eq/ne). Original latch's CondBit is erased if there
  // are no remaining uses of it after the transformation above.
  void emitVectorLoopIV(VPlanVector *Plan, VPValue *TripCount, VPValue *VF);

  /// Utility to dump and verify VPlan details after initial set of transforms.
  void printAndVerifyAfterInitialTransforms(VPlan *Plan);

  /// The loop that we evaluate.
  Loop *TheLoop;

  /// Loop Info analysis.
  LoopInfo *LI;

  /// The dominators tree.
  class DominatorTree *DT;

#if INTEL_CUSTOMIZATION
  /// VPlan VLS Analysis.
  VPlanVLSAnalysis *VLSA;
#endif // INTEL_CUSTOMIZATION

  /// The profitablity analysis.
  // LoopVectorizationCostModel *CM;

  // TODO: Move to base class
  VPOCodeGen *ILV = nullptr;

  // InnerLoopVectorizer *ILV = nullptr;

  /// VPlans are shared between VFs, use smart pointers.
  DenseMap<unsigned, VPlanPair> VPlans;
};

} // namespace vpo
} // namespace llvm

#endif // LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTELLOOPVECTORIZATIONPLANNER_H
