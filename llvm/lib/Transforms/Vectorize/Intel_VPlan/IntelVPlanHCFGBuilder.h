//===-- IntelVPlanHCFGBuilder.h ---------------------------------*- C++ -*-===//
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

#ifndef LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTELVPLANHCFGBUILDER_H
#define LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTELVPLANHCFGBUILDER_H

#include "IntelLoopVectorizationCodeGen.h" //Only for Legal.
#include "IntelVPlan.h"
#include "IntelVPlanDominatorTree.h"
#include "IntelVPlanVerifier.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/Analysis/Intel_VPO/WRegionInfo/WRegionInfo.h"

namespace llvm {
class ScalarEvolution;
class Loop;

namespace vpo {

class VPlanHCFGBuilder {
private:
  /// The outermost loop to be vectorized.
  Loop *TheLoop;

  /// Loop Info analysis.
  LoopInfo *LI;

  /// Scalar Evolution analysis.
  ScalarEvolution *SE;

protected:
  /// Hold WRegion information for TheLoop, if available.
  const WRNVecLoopNode *const WRLp;

  // Dominator/Post-Dominator analyses for VPlan Plan CFG to be used in the
  // construction of the H-CFG. These analyses are no longer valid once regions
  // are introduced.
  VPDominatorTree VPDomTree;
  VPPostDominatorTree VPPostDomTree;

  VPlan *Plan;

  /// VPlan verifier utility.
  VPlanVerifier *Verifier = nullptr;

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

  virtual VPRegionBlock *buildPlainCFG();
  virtual VPLoopRegion *createLoopRegion(VPLoop *VPLp) {
    assert(VPLp && "Expected a valid VPLoop.");
    VPLoopRegion *Loop =
        new VPLoopRegion(VPlanUtils::createUniqueName("loop"), VPLp);
    return Loop;
  }

  void simplifyPlainCFG();
  void splitLoopsPreheader(VPLoop *VPLp);
  void mergeLoopExits(VPLoop *VPLp);
  void splitLoopsExit(VPLoop *VPLp);
  void simplifyNonLoopRegions();

  void buildLoopRegions();
  virtual void collectUniforms(VPRegionBlock *Region);
  void buildNonLoopRegions(VPRegionBlock *ParentRegion);

  // Utility functions.
  bool isNonLoopRegion(VPBlockBase *Entry, VPRegionBlock *ParentRegion,
                       VPBlockBase *&Exit);
  bool regionIsBackEdgeCompliant(const VPBlockBase *Entry,
                                 const VPBlockBase *Exit,
                                 VPRegionBlock *ParentRegion);
  bool isDivergentBlock(VPBlockBase *Block);

public:
  VPlanHCFGBuilder(Loop *Lp, LoopInfo *LI, ScalarEvolution *SE,
                   const WRNVecLoopNode *WRL, VPlan *Plan,
                   VPOVectorizationLegality *Legal)
      : TheLoop(Lp), LI(LI), SE(SE), WRLp(WRL), Plan(Plan), Legal(Legal) {
    // TODO: Turn Verifier pointer into an object when Patch #3 of Patch Series
    // #1 lands into VPO and VPlanHCFGBuilderBase is removed.
    Verifier = new VPlanVerifier(Lp, LI);
    assert((!WRLp || WRLp->getTheLoop<Loop>() == TheLoop) &&
           "Inconsistent Loop information");
  }

  /// Build hierarchical CFG for TheLoop. Update Plan with the resulting H-CFG.
  void buildHierarchicalCFG();
};

} // namespace vpo
} // namespace llvm

#endif // LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTELVPLANHCFGBUILDER_H
