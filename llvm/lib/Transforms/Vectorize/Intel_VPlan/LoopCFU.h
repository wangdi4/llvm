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
#ifndef LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_LOOPCFU_H 
#define LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_LOOPCFU_H 

#include "VPlan.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"

namespace llvm {
namespace vpo {

//TODO: Temporarily disabling all this code
#if 0
class VPLoopCFU { 
  
private: 
  IntelVPlan *Plan;
  IntelVPlanUtils PlanUtils;
  ScalarEvolution *SE;
  LoopInfo *LI;
  VPLoopInfo *VPLI;
  VPDominatorTree &DT;
  VPPostDominatorTree &PDT;
  const VPLoop *VecLoop;
  
public:
  VPLoopCFU(IntelVPlan *Plan, IntelVPlanUtils &PlanUtils, ScalarEvolution *SE,
            LoopInfo *LI, VPLoopInfo *VPLInfo, VPDominatorTree &DomTree,
            VPPostDominatorTree &PostDomTree) :
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
#endif

} // end vpo namespace
} // end llvm namespace
#endif // LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_LOOPCFU_H 
