//===------------------------------------------------------------*- C++ -*-===//
//
//   Copyright (C) 2017 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_PREDICATOR_H
#define LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_PREDICATOR_H

#include "../Intel_VPlan.h"
#include "VPlan.h"
#include "llvm/IR/Dominators.h"

namespace llvm {
namespace vpo {
class VPlanPredicator {
private:
  enum EdgeType {
    EDGE_TYPE_UNINIT = 0,
    TRUE_EDGE,
    FALSE_EDGE,
    EDGE_TYPE_MAX,
  };

  IntelVPlan *Plan;
  VPLoopInfo *VPLI;
  IntelVPlanUtils PlanUtils;

  EdgeType getEdgeTypeBetween(VPBlockBase *FromBlock, VPBlockBase *ToBlock);
  int countSuccessorsNoBE(VPBlockBase *PredBlock, bool &HasBE);
  void getSuccessorsNoBE(VPBlockBase *PredBlock,
                         SmallVector<VPBlockBase *, 2> &Succs);

  VPPredicateRecipeBase *genEdgeRecipe(VPBasicBlock *PredBB, EdgeType ET);
  VPPredicateRecipeBase *genEdgeRecipe(VPBasicBlock *PredBB,
                                       VPPredicateRecipeBase *R,
                                       BasicBlock *From,
                                       BasicBlock *To);
  void propagatePredicatesAcrossBlocks(VPBlockBase *CurrBlock,
                                       VPRegionBlock *Region);
  void genLitReport(VPRegionBlock *Region);

  void predicateRegionRec(VPRegionBlock *Region);
  void optimizeRegionRec(VPRegionBlock *Region,
                         VPPredicateRecipeBase *IncomingAllOnesPred);
  // Linearize the CFG within Region.
  void linearizeRegionRec(VPRegionBlock *Region);

public:
  VPlanPredicator(IntelVPlan *Plan)
      : Plan(Plan), VPLI(Plan->getVPLoopInfo()), PlanUtils(Plan) {}
  
  /// The driver function for the predicator
  void predicate(void);
};
} // end namespace vpo
} // end namespace llvm
#endif // LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_PREDICATOR_H
