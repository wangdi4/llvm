//===-- VPlanHCFGBuilder.h --------------------------------------*- C++ -*-===//
//
//   Copyright (C) 2015-2016 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation. and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file defines VPlanHCFGBuilder class that is used to build a
/// hierarchical CFG in VPlan. Further documentation can be found in document
/// 'VPlan Hierarchical CFG Builder'.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_VPLANHCFGBUILDER_H
#define LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_VPLANHCFGBUILDER_H

#include "VPlan.h"
#include "LoopVectorizationCodeGen.h" //Only for Legal.
#include "VPlanVerifier.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/Analysis/Intel_VPO/WRegionInfo/WRegionInfo.h"

namespace llvm {
class ScalarEvolution;
class Loop;

namespace vpo {

class VPlanHCFGBuilderBase {

protected:
  /// Hold WRegion information for TheLoop, if available.
  const WRNVecLoopNode *const WRLp;

  // Dominator/Post-Dominator analyses for VPlan Plan CFG to be used in the
  // construction of the H-CFG. These analyses are no longer valid once regions
  // are introduced.
  VPDominatorTree VPDomTree;
  VPPostDominatorTree VPPostDomTree;

  IntelVPlanUtils PlanUtils;

  /// VPlan verifier utility.
  VPlanVerifierBase *Verifier = nullptr;

  // TODO: Only used to determine if a condition is uniform. Decouple from
  // Legality.
  // TODO: This must be a reference. Using pointer to support temporal nullptr
  // from HIR.
  /// The legality analysis.
  VPOVectorizationLegality *Legal;

  // Holds instructions from the original loop that we predicated. Such
  // instructions reside in their own conditioned VPBasicBlock and represent
  // an optimization opportunity for sinking their scalarized operands thus
  // reducing their cost by the predicate's probability.
  // SmallPtrSet<Instruction *, 4> PredicatedInstructions;

  // Holds instructions from the original loop whose counterparts in the
  // vectorized loop would be trivially dead if generated. For example,
  // original induction update instructions can become dead because we
  // separately emit induction "steps" when generating code for the new loop.
  // Similarly, we create a new latch condition when setting up the structure
  // of the new loop, so the old one can become dead.
  // SmallPtrSet<Instruction *, 4> DeadInstructions;

  VPlanHCFGBuilderBase(const WRNVecLoopNode *WRL, IntelVPlan *Plan,
                       VPOVectorizationLegality *Legal)
      : WRLp(WRL), PlanUtils(Plan), Legal(Legal) {}

  virtual VPRegionBlock *buildPlainCFG() = 0;

  void simplifyPlainCFG();
  void splitLoopsPreheader(VPLoop *VPLp);
  void mergeLoopExits(VPLoop *VPLp);
  void splitLoopsExit(VPLoop *VPLp);
  void simplifyNonLoopRegions();

  void buildLoopRegions();
  void collectUniforms(VPRegionBlock *Region);
  void buildNonLoopRegions(VPRegionBlock *ParentRegion);

  // Utility functions.
  bool isNonLoopRegion(VPBlockBase *Entry, VPRegionBlock *ParentRegion,
                       VPBlockBase *&Exit);
  bool regionIsBackEdgeCompliant(const VPBlockBase *Entry,
                                 const VPBlockBase *Exit,
                                 VPRegionBlock *ParentRegion);
  bool isDivergentBlock(VPBlockBase *Block);

  virtual VPLoopRegion *createLoopRegion(VPLoop *VPLp) = 0;
public:
  /// Build hierarchical CFG for TheLoop. Update Plan with the resulting H-CFG.
  void buildHierarchicalCFG();
};

class VPlanHCFGBuilder : public VPlanHCFGBuilderBase {

private:
  /// The outermost loop to be vectorized.
  Loop *TheLoop;

  /// Loop Info analysis.
  LoopInfo *LI;

  /// Scalar Evolution analysis.
  ScalarEvolution *SE;

  VPRegionBlock *buildPlainCFG() override;

public:
  VPlanHCFGBuilder(const WRNVecLoopNode *WRL, Loop *Lp, IntelVPlan *Plan,
                   LoopInfo *LI, ScalarEvolution *SE,
                   VPOVectorizationLegality *Legal)
      : VPlanHCFGBuilderBase(WRL, Plan, Legal), TheLoop(Lp), LI(LI), SE(SE) {

    Verifier = new VPlanVerifier(Lp, LI);
    assert((!WRLp || WRLp->getTheLoop<Loop>() == TheLoop) &&
           "Inconsistent Loop information");
  }

  VPLoopRegion *createLoopRegion(VPLoop *VPLp) override {
    return PlanUtils.createLoopRegion(VPLp);
  }
};

} // namespace vpo
} // namespace llvm

#endif // LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_VPLANHCFGBUILDER_H
