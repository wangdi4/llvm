#ifndef VPLAN_PREDICATOR_H
#define VPLAN_PREDICATOR_H

#include "../VPlan.h"
#include "IntelVPlan.h"
#include "llvm/IR/Dominators.h"

namespace llvm {
namespace vpo {
class VPlanPredicator {
private:
  IntelVPlan *Plan;
  VPLoopInfo *VPLI;
  IntelVPlanUtils PlanUtils;
  enum EdgeType {
      EDGE_TYPE_UNINIT = 0,
      TRUE_EDGE,
      FALSE_EDGE,
      EDGE_TYPE_MAX,
  };
  EdgeType getEdgeTypeBetween(VPBlockBase *FromBlock, VPBlockBase *ToBlock);
  // Map to remember which BRs have already been generated
  // for corresponding CBRs.
  std::map<VPConditionBitRecipeWithScalar *, VPBooleanRecipe *> CBRtoBRMap;
  void initializeGenPredicates(VPBasicBlock *VPBB);
  int countSuccessorsNoBE(VPBlockBase *PredBlock, bool& HasBE);
  void getSuccessorsNoBE(VPBlockBase *PredBlock,
                         SmallVector<VPBlockBase *, 2> &Succs);
  VPBooleanRecipe *getConditionRecipe(VPConditionBitRecipeBase *CBR);
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
  // Linearize the CFG
  void linearize(VPBlockBase *);

public:
  VPlanPredicator(IntelVPlan *plan)
      : Plan(plan), VPLI(plan->getVPLoopInfo()), PlanUtils(plan) {}
  
  /// The driver function for the predicator
  void predicate(void);
};
} // namespace vpo
} // namespace llvm
#endif
