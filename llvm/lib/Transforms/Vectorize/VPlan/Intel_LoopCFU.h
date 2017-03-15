#ifndef LLVM_TRANSFORMS_VECTORIZE_VPLAN_INTEL_LOOPCFU_H 
#define LLVM_TRANSFORMS_VECTORIZE_VPLAN_INTEL_LOOPCFU_H 

#include "../VPlan.h"
#include "IntelVPlan.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"

using namespace llvm::vpo;

namespace llvm {

class VPLoopCFU { 
  
private: 
  VPlan *Plan;
  IntelVPlanUtils PlanUtils;
  ScalarEvolution *SE;
  LoopInfo *LI;
  Loop *VecLoop;
  
public: 
  VPLoopCFU(VPlan *Plan, IntelVPlanUtils &PlanUtils, ScalarEvolution *SE,
            LoopInfo *LI) : Plan(Plan), PlanUtils(PlanUtils), SE(SE), LI(LI) {}

  void makeLoopControlFlowUniform();
  void visitBlock(VPBlockBase *Block);
  void visitRegion(VPRegionBlock *Region);
  void visitBasicBlock(VPBasicBlock *VPBB);

  void getLoopProperties(Loop *Lp, Value **LoopIdx, Value **LoopIdxInc,
                         Value **LoopLB, Value **LoopUB, Value **BackedgeCond);

  void createBlockAndRecipeForTruePath(
    VPBasicBlock *EntryBlock, VPVectorizeOneByOneIRRecipe *OrigRecipe);

  Instruction* getLoopPredicate(Loop *Lp);

  void createRecipeForMask(Instruction *Pred, Value *Backedge,
                           VPBasicBlock *EntryBlock);

  void createAllZeroRecipeForLoopEntry(Instruction *Ztt);

  void findSplitPtForTrueFalsePaths(
    Value *LoopIndex, VPVectorizeOneByOneIRRecipe *ParentOBORecipe,
    VPInstructionContainerTy::iterator& SplitPtIt);

  void createBlockAndRecipesForFalsePath(
    VPInstructionContainerTy::iterator& SplitPtIt,
    VPInstructionContainerTy::iterator& End,
    VPBasicBlock *LastTrueBlock,
    VPBasicBlock *EntryBlock,
    Value *Cond);
};

} // end llvm namespace

#endif // LLVM_TRANSFORMS_VECTORIZE_VPLAN_INTEL_LOOPCFU_H 
