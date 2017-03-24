#ifndef VPLAN_PREDICATOR_H
#define VPLAN_PREDICATOR_H

#include "../VPlan.h"
#include "IntelVPlan.h"
#include "llvm/IR/Dominators.h"

namespace llvm {
namespace vpo {
class VPlanPredicator {
protected:
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
  // Map to remember which VBRs have already been generated
  // for corresponding CBRs.
  std::map<VPConditionBitRecipeWithScalar *, VPVectorizeBooleanRecipe *>
      CBRtoVBRMap;
  void initializeGenPredicates(VPBasicBlock *VPBB);
  int countSuccessorsNoBE(VPBlockBase *PredBlock);
  void getSuccessorsNoBE(VPBlockBase *PredBlock,
                         SmallVector<VPBlockBase *, 2> &Succs);
  VPVectorizeBooleanRecipe *getConditionRecipe(VPConditionBitRecipeBase *CBR);
  VPPredicateRecipeBase *genEdgeRecipe(VPBasicBlock *PredBB, EdgeType ET);
  void genAndAttachEmptyBlockPredicate(VPBlockBase *CurrBlock);
  void propagatePredicatesAcrossBlocks(VPBlockBase *CurrBlock,
                                       VPRegionBlock *Region);
  void genLitReport(VPRegionBlock *Region);
  void predicateRegion(VPRegionBlock *Region);

public:
  VPlanPredicator(IntelVPlan *plan)
      : Plan(plan), VPLI(plan->getVPLoopInfo()), PlanUtils(plan) {}
  // The driver function for the predicator
  void predicate(void);

  // Linearize the CFG
  void linearize(VPBlockBase *);
};
} // namespace vpo
} // namespace llvm
#endif
