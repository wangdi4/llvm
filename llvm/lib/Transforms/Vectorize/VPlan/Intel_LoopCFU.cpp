//===-- Intel_LoopCFU.cpp -------------------------------------------------===//
//
//   Copyright (C) 2017 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//
//
// This file implements the algorithm to transform non-uniform loop control
// flow to uniform control flow based on VPlan data structures.
//
// Example 1:
//
// long A[100][100];
// void foo(long N, long *lb, long *ub) {
//   long i, j;
// #pragma omp simd
//   for(i=0;i<N;i++) {
//     for (j=lb[i]; j<ub[i]; j++) {  <--- non-uniform loop lower/upper bounds
//       A[j][i] = i+j;
//     }
//   }
// }
//
// Example 2:
//
// void foo(float **a, int m, int n, int k) {
//   int i, j;
//   for(i=0; i<m; i++) {
//     j = k;
//     while (j<=n && i%3 == 0) {  <--- uniform ub, but non-uniform i expr
//       if (a[i][j] == 0.0f)
//         a[i][j] = i + j;
//       j = j + 1;
//     }
//   }
// }
//
// In order to create uniform control flow, this algorithm splits the non-
// uniform loop body (assumed to be a single BasicBlock at the moment) into
// multiple VPBasicBlocks that will form a "triangle" based on when a particular
// mask bit for a lane is true or false. The first block is an entry that
// contains three recipes. The first recipe is a OneByOne that contains all phi
// instructions within the loop. This recipe is followed by an
// VPMaskGenerationRecipe, which is used as a placeholder for mask construction
// using a phi instruction. The last recipe in the entry block is the
// VPNonUniformConditionBitRecipe that serves as the successor selector of the
// true and false code paths corresponding to the mask. This recipe actually
// points to the VPMaskGenerationRecipe. The true path VPBasicBlock will contain
// a single OneByOneRecipe that will consist of the loop body instructions,
// minus the instructions that comprise the update to the loop index value and
// bottom test. Conversely, the false VPBasicBlock begins at the instruction
// that updates the loop index and includes the bottom test. However, this block
// is broken into two recipes. The first is a OneByOneRecipe that contains all
// aforementioned instructions with the exception of the branch. The second is a
// BranchIfNotAllZeroRecipe, which is used to test if all lanes are no longer
// active. Additionally, the branch instruction in the non-uniform loop ztt is
// also transformed into a BranchIfNotAllZeroRecipe.
//
//===----------------------------------------------------------------------===//

#include "Intel_LoopCFU.h"
#include "llvm/Support/Debug.h"

#define DEBUG_TYPE "LoopCFU"

using namespace llvm::vpo;

namespace llvm {

void VPLoopCFU::getLoopProperties(VPLoop *Lp, Value **LoopIdx,
                                  Value **LoopIdxInc, Value **LoopLB,
                                  Value **LoopUB, Value **BackedgeCond) {

  // Find the value within the loop exit condition that corresponds to the loop
  // lower/upper bounds, the loop index, and the increment of the loop index.
  // Consider the following LLVM:
  //
  // for.body4:                ; preds = %for.body4.preheader, %for.body4
  // (1) %j.020 = phi i64 [ %inc, %for.body4 ], [ %0, %for.body4.preheader ]
  // <loop body stmts>
  // (2) %inc = add nsw i64 %j.020, 1
  // (3) %2 = load i64, i64* %arrayidx2, align 8, !tbaa !1
  // (4) %cmp3 = icmp slt i64 %inc, %2
  // (5) br i1 %cmp3, label %for.body4, label %for.inc7.loopexit
  //
  // (1) - LoopIdx = %j.020, LoopLB = %0
  // (2) - LoopIdxInc
  // (3) - LoopUB
  //

  unsigned NumBackedges = 0;
  unsigned NumIncoming = 0;
  const VPBasicBlock *Backedge = nullptr;
  const VPBasicBlock *Incoming = nullptr;
  VPBasicBlock *Header = dyn_cast<VPBasicBlock>(Lp->getHeader());
  assert(Header && "Header should be a VPBasicBlock");
  for (const VPBlockBase *Pred : Header->getPredecessors()) {
    if (Lp->contains(Pred)) {
      // Note: Lp is expected to have a single backedge.
      Backedge = dyn_cast<VPBasicBlock>(Pred);
      assert(Backedge && "Backedge should be a VPBasicBlock");
      NumBackedges++;
    } else {
      // Note: Lp is expected to have a single incoming block.
      Incoming = dyn_cast<VPBasicBlock>(Pred);
      assert(Incoming && "Incoming should be a VPBasicBlock");
      NumIncoming++;
    }
  }
  assert(Backedge && "Could not find block containing loop backedge");
  assert(Incoming && "Could not find incoming block to loop");
  assert(NumBackedges == 1 && "Loop expected to have a single backedge");
  assert(NumIncoming == 1 && "Loop expected to have a single incoming block");

  const VPBasicBlock::RecipeListTy &BackedgeRecipeList = Backedge->getRecipes();
  const VPRecipeBase *BackedgeRecipe = &BackedgeRecipeList.back();
  const VPUniformConditionBitRecipe *UniformCBRecipe =
    dyn_cast<VPUniformConditionBitRecipe>(BackedgeRecipe);
  assert(UniformCBRecipe &&
         "Expected UniformConditionBitRecipe for loop backedge");

  *BackedgeCond = UniformCBRecipe->getScalarCondition();

  DEBUG(dbgs() << "Incoming Loop Block: " << Incoming->getName() << "\n");
  DEBUG(dbgs() << "Loop Header Block: " << Header->getName() << "\n");
  DEBUG(dbgs() << "Loop Backedge Block: " << Backedge->getName() << "\n");
  DEBUG(dbgs() << "Loop Backedge Condition: " << **BackedgeCond << "\n");

  // This piece of code finds the loop index and its update, which should be in
  // the form of an add instruction. This add instruction is used as the split
  // point for a new VPBasicBlock that will separate the instructions to be
  // executed under mask vs. the remainder of the instructions in the loop body
  // that need to be executed for all lanes. It is assumed that the update to
  // the loop index is the first instruction after the last instruction under
  // mask.
  //
  // Example:
  //
  // while.body:                            ; preds = %while.body.lr.ph, %if.end
  //   %indvars.iv =
  //       phi i64 [ %0, %while.body.lr.ph ], [ %indvars.iv.next, %if.end ]
  // 
  //   %j.034 = phi i32 [ %k, %while.body.lr.ph ], [ %add15, %if.end ]
  //   ...
  //   br i1 %cmp5, label %if.then, label %if.else
  //
  // if.then:                               ; preds = %while.body
  //   %5 = add nsw i64 %indvars.iv, %indvars.iv40
  //   %6 = trunc i64 %5 to i32
  //   br label %if.end
  //
  // if.else:                               ; preds = %while.body
  //   %mul = mul nsw i32 %j.034, %2
  //   %arrayidx12 = getelementptr inbounds float*, float** %a, i64 %indvars.iv
  //   %7 = load float*, float** %arrayidx12, align 8, !tbaa !1
  //   br label %if.end
  //
  // if.end:                                 ; preds = %if.else, %if.then
  //   %.sink16 = phi i64 [ %indvars.iv40, %if.else ], [ %indvars.iv, %if.then ]
  //   %.sink = phi float* [ %7, %if.else ], [ %3, %if.then ]
  //   %conv10.sink.in = phi i32 [ %mul, %if.else ], [ %6, %if.then ]
  //   %conv10.sink = sitofp i32 %conv10.sink.in to float
  //   %sext = shl i64 %.sink16, 32
  //   %idxprom13 = ashr exact i64 %sext, 32
  //   %arrayidx14 = getelementptr inbounds float, float* %.sink, i64 %idxprom13
  //   store float %conv10.sink, float* %arrayidx14, align 4, !tbaa !5
  // 
  //   ***Need to Split Here - store is the last instruction under mask
  //   "%indvars.iv.next = add ..." is the primary loop induction update
  //   For now, it is assumed these instructions are in this order.
  // 
  //   %indvars.iv.next = add nsw i64 %indvars.iv, 1
  //   %add15 = add nsw i32 %j.034, 1
  //   %cmp1 = icmp slt i64 %indvars.iv, %1
  //   br i1 %cmp1, label %while.body, label %for.inc.loopexit
  //
  // The logic here is that if either of the values %indvars.iv.next or
  // %indvars.iv that appear in the add instruction also appears in the
  // condition for the loop backedge (%cmp1), then this is the primary
  // induction update.

  SmallVector<const PHINode*, 2> PhiNodes;
  const VPBasicBlock::RecipeListTy &HeaderRecipeList = Header->getRecipes();
  const VPRecipeBase *HeaderRecipe = &HeaderRecipeList.front();
  const VPVectorizeOneByOneIRRecipe *OneByOneRecipe =
    dyn_cast<VPVectorizeOneByOneIRRecipe>(HeaderRecipe);
  assert(OneByOneRecipe &&
         "Expected VPVectorizeOneByOneIRRecipe as first recipe in loop header");

  for (auto It = OneByOneRecipe->begin(), End = OneByOneRecipe->end();
       It != End; ++It) {
    const VPInstructionIR *VPInst = cast<VPInstructionIR>(It);
    const Instruction *I = VPInst->getInstruction();
    if (const PHINode *Phi = dyn_cast<PHINode>(I)) {
      PhiNodes.push_back(Phi);
    }
  }

  Instruction *CondInst = dyn_cast<Instruction>(*BackedgeCond);
  assert(CondInst && "Expected loop backedge condition to be an instruction");
  BasicBlock *CondParent = CondInst->getParent();
  Instruction *Inc = nullptr;
  for (auto PN : PhiNodes) {
    Inc = dyn_cast<Instruction>(PN->getIncomingValueForBlock(CondParent));
    if (Inc) {
      if (Inc->getOpcode() == Instruction::Add && Inc->getOperand(0) == PN) {
        Instruction *IndVal =
          dyn_cast<Instruction>(CondInst->getOperand(0));
        assert(IndVal && "Expected induction value to be an instruction");
        Instruction *IncVal = dyn_cast<Instruction>(Inc->getOperand(0));
        assert(IncVal &&
               "Expected induction value incremented to be an instruction");
        if (Inc == IndVal || IncVal == IndVal) {
          PHINode *Phi = const_cast<PHINode*>(PN);
          *LoopIdx = Phi;
          *LoopIdxInc = Inc;
        }
      }
    }
  }

  // Find the loop lower bound. This is the incoming Value in the loop index Phi
  // that is incoming from the loop preheader.
  PHINode *LoopIdxPhi = cast<PHINode>(*LoopIdx);
  for (unsigned i = 0; i < LoopIdxPhi->getNumIncomingValues(); i++) {
    // It has already been asserted that there is a single incoming block and
    // single backedge to the loop. The lower bound will come from the single
    // incoming block. It has to be done this way because
    // getIncomingValueForBlock() cannot be called using VPBasicBlock.
    Value *Op = LoopIdxPhi->getIncomingValue(i);
    BasicBlock *Block = LoopIdxPhi->getIncomingBlock(i);
    if (Block != CondParent) {
      *LoopLB = Op;
    }
  }

  // Find the loop upper bound. Most likely operand 1 of the comparison in
  // the backedge, but check all operands just in case ordering differs.
  for (unsigned i = 0; i < CondInst->getNumOperands(); i++) {
    if (CondInst->getOperand(i) != Inc) {
      *LoopUB = CondInst->getOperand(i); 
    }
  }

  assert(*LoopIdx && "Could not find loop index");
  assert(*LoopIdxInc && "Could not find loop index update - not an add?");
  assert(*LoopLB && "Could not find loop lower bound");
  assert(*LoopUB && "Could not find loop upper bound");
    
  DEBUG(errs() << "Loop Index: " << **LoopIdx << "\n");
  DEBUG(errs() << "Loop Index Inc: " << **LoopIdxInc << "\n");
  DEBUG(errs() << "Loop Lower Bound: " << **LoopLB << "\n");
  DEBUG(errs() << "Loop Upper Bound: " << **LoopUB << "\n");
}

void VPLoopCFU::createBlockAndRecipeForTruePath(
  VPBasicBlock *EntryBlock, VPVectorizeOneByOneIRRecipe *OrigRecipe) {

  // Splits the existing loop entry block into a new entry that will contain all
  // phi instructions and a new block that will begin the mask true region.

  // Create a new VPBasicBlock and OneByOneRecipe for the instructions that will
  // be executed when the mask is true.
  VPBasicBlock *TrueBlock = PlanUtils.splitBlock(EntryBlock, VPLI, *DT, *PDT);

  // Find the last phi instruction in the original OneByOneRecipe for the loop
  // entry block. This is the point where the original recipe and the mask true
  // OneByOneRecipe will be split.
  VPInstructionContainerTy::iterator PHIEnd;
  for (auto It = OrigRecipe->begin(), End = OrigRecipe->end(); It != End;
       ++It) {
    VPInstructionIR *VPInst = cast<VPInstructionIR>(It);
    Instruction *I = VPInst->getInstruction();
    if (isa<PHINode>(I)) {
      PHIEnd = It;
    }
  }
  PHIEnd++; // Point to the next instruction after the last phi

  VPInstructionIR *BeginVPInst = cast<VPInstructionIR>(PHIEnd);
  Instruction *BeginInst = BeginVPInst->getInstruction();
  BasicBlock::iterator BeginInstIt = BeginInst->getIterator();

  // Get the last instruction in the original recipe and then get an iterator
  // to it. Bump the iterator by one because when the Inst2Recipe map and new
  // recipe is created, we must point to one instruction after the one to be
  // included.
  VPInstructionContainerTy::iterator End = OrigRecipe->end();
  End--;
  VPInstructionIR *EndVPInst = cast<VPInstructionIR>(End);
  Instruction *EndInst = EndVPInst->getInstruction();
  BasicBlock::iterator EndInstIt = EndInst->getIterator();
  EndInstIt++;

  // Update the Inst2Recipe map so that the original recipe no longer references
  // the instructions that were put into the new recipe.
  Plan->resetInst2RecipeRange(BeginInstIt, EndInstIt);

  VPOneByOneIRRecipeBase *TrueRecipe =
    PlanUtils.createOneByOneRecipe(BeginInstIt, EndInstIt, false);
  TrueBlock->addRecipe(TrueRecipe);

  // Remove the VPInstructions from the original recipe.
  OrigRecipe->removeInstructions(PHIEnd, OrigRecipe->end());

  VPBasicBlock::RecipeListTy &RecipeList = EntryBlock->getRecipes();
  VPRecipeBase *RecipeBase = &RecipeList.back();
  VPUniformConditionBitRecipe *OrigCBRecipe =
    dyn_cast<VPUniformConditionBitRecipe>(RecipeBase);
  assert(OrigCBRecipe &&
         "Expected UniformConditionBitRecipe in original loop entry");

  // Move the existing VPUniformConditionBitRecipe from the entry loop block to
  // the new block that begins the mask true code path. Actually, a new one is
  // created and added to the mask true block because calling removeRecipe() on
  // the existing one seems to invalidate the pointer.
  std::set<const VPBlockBase*> RecipeUsers =
    Plan->getRecipeUsers(OrigCBRecipe);
  Instruction *CondInst = cast<Instruction>(OrigCBRecipe->getScalarCondition());
  Plan->resetInst2Recipe(CondInst);
  VPUniformConditionBitRecipe *NewCBRecipe =
    PlanUtils.createUniformConditionBitRecipe(CondInst);
  Plan->setInst2Recipe(CondInst, NewCBRecipe);
  TrueBlock->addRecipe(NewCBRecipe);
  TrueBlock->setConditionBitRecipe(NewCBRecipe, Plan);

  // Update the VPBasicBlock users of the old VPUniformConditionBitRecipe to
  // point to the new one. Also remove the old recipe.
  for (auto It = RecipeUsers.begin(), End = RecipeUsers.end(); It != End;
       ++It) {
    VPBlockBase *Block = const_cast<VPBlockBase*>(*It);
    Block->setConditionBitRecipe(NewCBRecipe, Plan);
  }
  Plan->removeRecipeUsers(OrigCBRecipe);
  EntryBlock->removeRecipe(OrigCBRecipe);
  EntryBlock->setConditionBitRecipe(nullptr, Plan);
}

Instruction* VPLoopCFU::getLoopZtt(const VPLoop *Lp) {
  // PredBlock points to the basic block that has the ztt for the inner loop.
  // Here, Lp points to the outer loop.
  VPBasicBlock *LpPreHeader = dyn_cast<VPBasicBlock>(Lp->getLoopPreheader());
  assert(LpPreHeader && "Expected loop pre-header to be a VPBasicBlock");
  VPBasicBlock *PredBlock =
    dyn_cast<VPBasicBlock>(LpPreHeader->getSingleSuccessor());
  assert(PredBlock && "Could not find predicate basic block");
  VPBasicBlock::RecipeListTy &RecipeList = PredBlock->getRecipes();
  VPRecipeBase *RecipeBase = &RecipeList.back();
  VPUniformConditionBitRecipe *UniformCBR =
    dyn_cast<VPUniformConditionBitRecipe>(RecipeBase);
  assert(UniformCBR && "Expected UniformCBR for loop ztt");
  Value *Cond = UniformCBR->getScalarCondition();
  assert(isa<Instruction>(Cond) && "Condition should be an Instruction");
  return cast<Instruction>(Cond);
}

void VPLoopCFU::createRecipeForMask(Instruction *Pred,
                                    Value *BackedgeCond,
                                    VPBasicBlock *EntryBlock) {

  // The algorithm generates a VPBasicBlock "triangle" for mask true or false
  // code paths. The selector (VPConditionBitRecipe) here points to the recipe
  // that will generate the mask.

  Instruction *Cond = cast<Instruction>(BackedgeCond);

  VPMaskGenerationRecipe *MaskGenRecipe =
    PlanUtils.createMaskGenerationRecipe(Pred, Cond);
  EntryBlock->addRecipe(MaskGenRecipe);

  VPNonUniformConditionBitRecipe *NonUniformCBRecipe =
    PlanUtils.createNonUniformConditionBitRecipe(MaskGenRecipe);
  EntryBlock->addRecipe(NonUniformCBRecipe);
  EntryBlock->setConditionBitRecipe(NonUniformCBRecipe, Plan);
}

void VPLoopCFU::createAllZeroRecipeForLoopEntry(Instruction *Ztt) {
  // Insert an all zero check recipe for the inner loop ztt.
  VPRecipeBase *ParentRecipe = Plan->getRecipe(Ztt);
  VPUniformConditionBitRecipe *ParentCBRecipe =
    dyn_cast<VPUniformConditionBitRecipe>(ParentRecipe);
  assert(ParentCBRecipe &&
         "Expected VPUniformConditionBitRecipe for ztt block");
  VPBasicBlock *ParentBlock = Plan->getBasicBlock(Ztt);

  Plan->resetInst2Recipe(Ztt);
  ParentBlock->removeRecipe(ParentCBRecipe);
  VPBranchIfNotAllZeroRecipe *AllZeroRecipeZtt =
    PlanUtils.createBranchIfNotAllZeroRecipe(Ztt);
  ParentBlock->addRecipe(AllZeroRecipeZtt);

  std::set<const VPBlockBase*> RecipeUsers =
    Plan->getRecipeUsers(ParentCBRecipe);
  for (auto It = RecipeUsers.begin(), End = RecipeUsers.end(); It != End;
       ++It) {
    VPBlockBase *Block = const_cast<VPBlockBase*>(*It);
    Block->setConditionBitRecipe(AllZeroRecipeZtt, Plan);
  }
  Plan->removeRecipeUsers(ParentCBRecipe);
}

void VPLoopCFU::findSplitPtForTrueFalsePaths(
    Value *LoopIdxInc, VPVectorizeOneByOneIRRecipe *ParentRecipe,
    VPInstructionContainerTy::iterator& SplitPtIt) {

  // This code is here to find the VPInstruction iterator for the Loop index
  // increment since it is needed when creating a new recipe. There has to be
  // a better way to do this. Essentially, this code takes an LLVM Value, finds
  // it in the parent recipe, and returns the VPInstruction iterator to it from
  // the recipe.
  VPInstructionContainerTy::iterator ItEnd;
  for (SplitPtIt = ParentRecipe->begin(), ItEnd = ParentRecipe->end();
       SplitPtIt != ItEnd; ++SplitPtIt) {
    VPInstructionIR *VPInst = cast<VPInstructionIR>(SplitPtIt);
    Instruction *I = VPInst->getInstruction();
    if (I == LoopIdxInc) {
      return;
    }
  }

  llvm_unreachable("Can't find split point for true/false paths\n");
}

void VPLoopCFU::createBlockAndRecipesForFalsePath(
  VPInstructionContainerTy::iterator& SplitPtIt,
  VPInstructionContainerTy::iterator& End,
  VPBasicBlock *LastTrueBlock,
  VPBasicBlock *EntryBlock,
  Value *Cond) {

  // Create a VPBasicBlock and corresponding OneByOneRecipe for the mask false
  // path. 
  VPBasicBlock *FalseBlock =
    PlanUtils.splitBlock(LastTrueBlock, VPLI, *DT, *PDT);

  VPInstructionIR *BeginVPInst = cast<VPInstructionIR>(SplitPtIt);
  Instruction *BeginInst = BeginVPInst->getInstruction();
  BasicBlock::iterator BeginInstIt = BeginInst->getIterator();

  End--;
  VPInstructionIR *EndVPInst = cast<VPInstructionIR>(End);
  Instruction *EndInst = EndVPInst->getInstruction();
  BasicBlock::iterator EndInstIt = EndInst->getIterator();
  EndInstIt++;

  VPOneByOneIRRecipeBase *FalseRecipe =
    PlanUtils.createOneByOneRecipe(BeginInstIt, EndInstIt, false);
  FalseBlock->addRecipe(FalseRecipe);
  // TODO: Use connectBlocks instead? EntryBlock should have no successors.
  // Otherwise we would need a condition bit recipe.
  PlanUtils.appendBlockSuccessor(EntryBlock, FalseBlock);
  PlanUtils.appendBlockPredecessor(FalseBlock, EntryBlock);

  // Create a new Recipe that indicates an all zeros bypass at the end of the
  // loop. The new recipe will contain the original compare instruction from the
  // bottom test only. 
  Instruction *CondInst = cast<Instruction>(Cond);
  VPRecipeBase *CondRecipe = Plan->getRecipe(CondInst);
  VPUniformConditionBitRecipe *CondCBRecipe =
    dyn_cast<VPUniformConditionBitRecipe>(CondRecipe);
  assert(CondCBRecipe &&
         "Expected loop backedge to be inside VPUniformConditionBitRecipe");
  VPBasicBlock *ParentBlock = Plan->getBasicBlock(CondInst);

  Plan->resetInst2Recipe(CondInst);
  ParentBlock->removeRecipe(CondCBRecipe);

  VPBranchIfNotAllZeroRecipe *AllZeroRecipeExit =
    PlanUtils.createBranchIfNotAllZeroRecipe(CondInst);
  FalseBlock->addRecipe(AllZeroRecipeExit);
  FalseBlock->setConditionBitRecipe(AllZeroRecipeExit, Plan);

  std::set<const VPBlockBase*> RecipeUsers =
    Plan->getRecipeUsers(CondCBRecipe);

/*
  for (auto It = RecipeUsers.begin(), End = RecipeUsers.end(); It != End;
       ++It) {
    VPBlockBase *Block = const_cast<VPBlockBase*>(*It);
    // Don't set the EntryBlock VPConditionBitRecipe to the
    // VPBranchIfNotAllZeroRecipe at the loop exit since that block's
    // VPConditionBitRecipe is the VPNonUniformConditionBitRecipe that
    // consumes the VPMaskGenerationRecipe.
    if (Block != EntryBlock)
      Block->setConditionBitRecipe(AllZeroRecipeExit, Plan);
  }
*/
  Plan->removeRecipeUsers(CondCBRecipe);
}

void VPLoopCFU::makeInnerLoopControlFlowUniform() {
  VPLI = Plan->getVPLoopInfo();
  for (auto It = VPLI->begin(), End = VPLI->end(); It != End; ++It) {
    const VPLoop *Lp = *It;
    VecLoop = Lp;
    DEBUG(dbgs() << "Vectorizing at loop: " << *Lp << "\n");
    const std::vector<VPLoop*> SubLoops = Lp->getSubLoops();
    for (unsigned i = 0; i < SubLoops.size(); i++) {
      // Get the ZTT for the inner loop.
      Instruction *Ztt = getLoopZtt(Lp);
      DEBUG(errs() << "Loop ZTT: " << *Ztt << "\n");

      Value *LoopIdx = nullptr;
      Value *LoopIdxInc = nullptr;
      Value *LoopLB = nullptr;
      Value *LoopUB = nullptr;
      Value *BackedgeCond = nullptr;
      // Get the important properties of the inner loop.
      getLoopProperties(SubLoops[i], &LoopIdx, &LoopIdxInc, &LoopLB,
                        &LoopUB, &BackedgeCond);

      // Is the loop upper bound a constant? If not, assume non-uniform trip
      // count for now. Refine later with SLEV.
      Instruction *LoopUBInst = cast<Instruction>(LoopUB);
      Loop *LLVMLp = LI->getLoopFor(LoopUBInst->getParent());
      const SCEV *Scev = SE->getSCEVAtScope(LoopUB, LLVMLp);
      const SCEVConstant *ScevConst = dyn_cast<SCEVConstant>(Scev);
      if (!ScevConst) {
        createAllZeroRecipeForLoopEntry(Ztt);
        VPBasicBlock *EntryBlock =
          dyn_cast<VPBasicBlock>(SubLoops[i]->getHeader());
        assert(EntryBlock && "Expected EntryBlock to be a VPBasicBlock");
        VPBasicBlock::RecipeListTy &EntryRecipeList =
          EntryBlock->getRecipes();
        VPRecipeBase *EntryRecipe = &EntryRecipeList.front();
        VPVectorizeOneByOneIRRecipe *OneByOneRecipe =
          dyn_cast<VPVectorizeOneByOneIRRecipe>(EntryRecipe);
          assert(OneByOneRecipe &&
                 "Expected VPVectorizeOneByOneIRRecipe as first recipe in loop\
                  header");
        createBlockAndRecipeForTruePath(EntryBlock, OneByOneRecipe);
        createRecipeForMask(Ztt, BackedgeCond, EntryBlock); 
        VPInstructionContainerTy::iterator SplitPtIt;
        // LoopIdxInc is guaranteed to be an instruction here, so cast away.
        Instruction *IncInst = cast<Instruction>(LoopIdxInc);
        VPRecipeBase *ParentRecipe = Plan->getRecipe(IncInst);
        VPVectorizeOneByOneIRRecipe *ParentOBORecipe =
          dyn_cast<VPVectorizeOneByOneIRRecipe>(ParentRecipe);
        assert(ParentOBORecipe &&
               "Expected OneByOneRecipe recipe for loop index update");
        findSplitPtForTrueFalsePaths(LoopIdxInc, ParentOBORecipe, SplitPtIt);
        VPInstructionContainerTy::iterator End = ParentOBORecipe->end();
        createBlockAndRecipesForFalsePath(SplitPtIt, End,
                                          ParentOBORecipe->getParent(),
                                          EntryBlock, BackedgeCond);
        End++;
        ParentOBORecipe->removeInstructions(SplitPtIt, End);

        // TODO: Do we need to add a new recipe for updating the mask at the
        // bottom of inner loops that contain non-uniform conditions on the
        // loop backedge?

        DEBUG(errs() << "\nInst2Recipe Map:\n");
        DEBUG(Plan->printInst2Recipe());
      }
    }
  }
}

} // end llvm namespace
