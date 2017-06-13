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
/// This file defines VPlanHCFGBuilder class that is used build a hierarchical
/// CFG in VPlan. Further documentation can be found in document 'VPlan
/// Hierarchical CFG Builder'.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_VECTORIZE_VPLAN_VPLANHCFGBUILDER_H
#define LLVM_TRANSFORMS_VECTORIZE_VPLAN_VPLANHCFGBUILDER_H

#include "IntelVPlan.h"
#include "llvm/ADT/DenseMap.h"
#include "LoopVectorizationCodeGen.h" //Only for Legal.
                                      //TODO: Move Legal to an independent file

namespace llvm {

class ScalarEvolution;
class Loop;

namespace vpo {

class WRNVecLoopNode;

class VPlanHCFGBuilder {

public:
  VPlanHCFGBuilder(LoopInfo *LI, ScalarEvolution *SE,
                   VPOVectorizationLegality &L)
      : LI(LI), SE(SE), Legal(L) {}

  /// Build hierarchical CFG for \p TheLoop using its WRegion analysis
  /// information (if available). \p Plan is updated with the resulting
  /// hierarhical CFG.
  void buildHierarchicalCFG(Loop *TheLoop, const WRNVecLoopNode *WRL,
                            IntelVPlan *Plan);

private:
  // It holds the state associated to the HCFG during its construction. This
  // state is alive only during the HCFG construction and must be destroy at the
  // end of the HCFG construction.
  struct HCFGState {
    Loop *const TheLoop;
    const WRNVecLoopNode *const WRLoop;
    VPDominatorTree VPDomTree;
    VPDominatorTree VPPostDomTree;
    IntelVPlanUtils PlanUtils;

    HCFGState(Loop *Lp, const WRNVecLoopNode *WRL, IntelVPlan *Plan)
        : TheLoop(Lp), WRLoop(WRL), VPDomTree(false /*PostDom*/),
          VPPostDomTree(true /*PostDom*/), PlanUtils(Plan) {}
  };

  /// Loop Info analysis.
  LoopInfo *LI;

  /// Scalar Evolution analysis.
  ScalarEvolution *SE;

  // VPO legality information. Only used to determine if a condition is uniform.
  // TODO: Temporal solution.
  VPOVectorizationLegality &Legal;

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
  //SmallPtrSet<Instruction *, 4> DeadInstructions;

  VPRegionBlock *buildPlainCFG(HCFGState &State);

  void simplifyPlainCFG(HCFGState &State);
  void splitLoopsPreheader(VPLoop *VPL, HCFGState &State);
  void mergeLoopExits(VPLoop *VPL, HCFGState &State);
  void splitLoopsExit(VPLoop *VPL, HCFGState &State);
  void simplifyNonLoopRegions(HCFGState &State);

  void buildLoopRegions(HCFGState &State);
  void buildNonLoopRegions(VPRegionBlock *ParentRegion, HCFGState &State);
};

} // End vpo namespace
} // End LLVM namespace

#endif //LLVM_TRANSFORMS_VECTORIZE_VPLAN_VPLANHCFGBUILDER_H
