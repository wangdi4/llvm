#ifndef LLVM_TRANSFORMS_VECTORIZE_VPLAN_INTEL_LOOPCFU_H 
#define LLVM_TRANSFORMS_VECTORIZE_VPLAN_INTEL_LOOPCFU_H 

//#include "../VPlan.h"
#include "IntelVPlan.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"

using namespace llvm::vpo;

namespace llvm {

class VPLoopCFU { 
  
private: 
  IntelVPlan *Plan;
  IntelVPlanUtils PlanUtils;
  ScalarEvolution *SE;
  LoopInfo *LI;
  VPLoopInfo *VPLI;
  VPDominatorTree *DT;
  VPPostDominatorTree *PDT;
  const VPLoop *VecLoop;
  
public: 
  VPLoopCFU(IntelVPlan *Plan, IntelVPlanUtils &PlanUtils, ScalarEvolution *SE,
            LoopInfo *LI, VPLoopInfo *VPLInfo, VPDominatorTree *DomTree,
            VPPostDominatorTree *PostDomTree) :
            Plan(Plan), PlanUtils(PlanUtils), SE(SE), LI(LI), VPLI(VPLInfo),
            DT(DomTree), PDT(PostDomTree) {}

  void makeInnerLoopControlFlowUniform();

  void getLoopProperties(VPLoop *Lp, Value **LoopIdx, Value **LoopIdxInc,
                         Value **LoopLB, Value **LoopUB, Value **BackedgeCond);

  void createBlockAndRecipeForTruePath(
    VPBasicBlock *EntryBlock, VPVectorizeOneByOneIRRecipe *OrigRecipe);

  Instruction* getLoopZtt(const VPLoop *Lp);

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
