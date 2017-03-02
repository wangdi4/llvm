#ifndef VPLAN_PREDICATOR_H
#define VPLAN_PREDICATOR_H

#include "../VPlan.h"
#include "llvm/IR/Dominators.h"
#include "IntelVPlan.h"


namespace llvm {
	class VPlanPredicator {
	protected:
		IntelVPlan *Plan;
		VPLoopInfo *VPLI;
		void initializeGenPredicates(VPBasicBlock *VPBB);
		void getSuccessorsNoBE(VPBlockBase *PredBlock,
													 SmallVector<VPBlockBase *, 2> &Succs);
		VPPredicateRecipeBase *genOrUseIncomingPredicate(VPBlockBase *CurrBlock,
																										 VPBlockBase *PredBlock);
		void genAndAttachEmptyBlockPredicate(VPBlockBase *CurrBlock);
		bool isBackEdge(VPBlockBase *predBlock, VPBlockBase *CurrBlock);
		void propagateInputPredicates(VPBlockBase *CurrBlock,
																	VPRegionBlock *Region);
		void genLitReport(VPRegionBlock *Region);
		void predicateRegion(VPRegionBlock *Region);
	public:
		VPlanPredicator(IntelVPlan *plan)
			: Plan(plan), VPLI(plan->getVPLoopInfo()) { ; }
		// The driver function for the predicator
		void predicate(void);
	};
} // namespace llvm
#endif
