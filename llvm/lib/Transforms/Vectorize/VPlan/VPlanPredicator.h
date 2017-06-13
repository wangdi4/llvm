#ifndef VPLAN_PREDICATOR_H
#define VPLAN_PREDICATOR_H

#include "../VPlan.h"
#include "IntelVPlan.h"
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
  // Map to remember which BRs have already been generated
  // for corresponding CBRs.
  std::map<VPConditionBitRecipeWithScalar *, VPBooleanRecipe *> CBRtoBRMap;

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
  // Linearize the CFG within Region.
  void linearizeRegionRec(VPRegionBlock *Region);

public:
  VPlanPredicator(IntelVPlan *Plan)
      : Plan(Plan), VPLI(Plan->getVPLoopInfo()), PlanUtils(Plan) {}
  
  /// The driver function for the predicator
  void predicate(void);
};
} // namespace vpo
} // namespace llvm
#endif
