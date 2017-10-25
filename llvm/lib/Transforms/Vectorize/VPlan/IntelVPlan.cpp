
#include "IntelVPlan.h"
#include "LoopVectorizationCodeGen.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"

using namespace llvm;
using namespace llvm::vpo;

#define DEBUG_TYPE "intel-vplan"

//Replicated from LoopVectorize.cpp

void VPVectorizeOneByOneIRRecipe::transformIRInstruction(
    Instruction *I, VPTransformState &State) {
  assert(I && "No instruction to vectorize.");
  State.ILV->vectorizeInstruction(I);
  // if (willAlsoPackOrUnpack(I)) { // Unpack instruction
  //  for (unsigned Part = 0; Part < State.UF; ++Part)
  //    for (unsigned Lane = 0; Lane < State.VF; ++Lane)
  //      State.ILV->getScalarValue(I, Part, Lane);
  // }
}

VPOneByOneIRRecipeBase *
IntelVPlanUtils::createOneByOneRecipe(const BasicBlock::iterator B,
                                      const BasicBlock::iterator E,
                                      // VPlan *Plan,
                                      bool isScalarizing) {
  // TODO
  // if (isScalarizing)
  //  return new VPScalarizeOneByOneRecipe(B, E, Plan);
  return new VPVectorizeOneByOneIRRecipe(B, E, Plan);
}

VPBranchIfNotAllZeroRecipe *
IntelVPlanUtils::createBranchIfNotAllZeroRecipe(Instruction *Cond) {
  return new VPBranchIfNotAllZeroRecipe(Cond, Plan);
}

VPMaskGenerationRecipe *
IntelVPlanUtils::createMaskGenerationRecipe(const Value *Pred,
                                            const Value *Backedge) {
  return new VPMaskGenerationRecipe(Pred, Backedge);
}

VPNonUniformConditionBitRecipe *
IntelVPlanUtils::createNonUniformConditionBitRecipe(
  const VPMaskGenerationRecipe *MaskRecipe) {
  return new VPNonUniformConditionBitRecipe(MaskRecipe);
}

// It turns A->B into A->NewSucc->B and updates VPLoopInfo, DomTree and
// PostDomTree accordingly.
VPBasicBlock *IntelVPlanUtils::splitBlock(VPBlockBase *Block,
                                          VPLoopInfo *VPLInfo,
                                          VPDominatorTree &DomTree,
                                          VPPostDominatorTree &PostDomTree) {
  VPBasicBlock *NewBlock = createBasicBlock();
  insertBlockAfter(NewBlock, Block);

  // Add NewBlock to VPLoopInfo
  if (VPLoop *Loop = VPLInfo->getLoopFor(Block)) {
    Loop->addBasicBlockToLoop(NewBlock, *VPLInfo);
  }

  // Update dom information

  VPDomTreeNode *BlockDT = DomTree.getNode(Block);
  SmallVector<VPDomTreeNode *, 2> BlockDTChildren(BlockDT->begin(),
                                                  BlockDT->end());
  // Block is NewBlock's idom.
  VPDomTreeNode *NewBlockDT = DomTree.addNewBlock(NewBlock, Block /*IDom*/);

  // NewBlock dominates all other nodes dominated by Block.
  for (VPDomTreeNode *Child : BlockDTChildren)
    DomTree.changeImmediateDominator(Child, NewBlockDT);

  // Update postdom information

  VPDomTreeNode *NewBlockPDT;
  if (VPBlockBase *NewBlockSucc = NewBlock->getSingleSuccessor()) {
    // NewBlock has a single successor. That successor is NewBlock's ipostdom.
    NewBlockPDT = PostDomTree.addNewBlock(NewBlock, NewBlockSucc /*IDom*/);
  } else {
    // NewBlock has multiple successors. NewBlock's ipostdom is the nearest
    // common post-dominator of both successors.

    // TODO: getSuccessor(0)
    auto &Successors = NewBlock->getSuccessors();
    VPBlockBase *Succ1 = *Successors.begin();
    VPBlockBase *Succ2 = *std::next(Successors.begin());

    NewBlockPDT = PostDomTree.addNewBlock(
        NewBlock, PostDomTree.findNearestCommonDominator(Succ1, Succ2));
  }

  VPDomTreeNode *BlockPDT = PostDomTree.getNode(Block);

  // TODO: remove getBlock?
  if (BlockPDT->getIDom()->getBlock() == NewBlockPDT->getIDom()->getBlock()) {
    // Block's old ipostdom is the same as NewBlock's ipostdom. Block's new
    // ipostdom is NewBlock
    PostDomTree.changeImmediateDominator(BlockPDT, NewBlockPDT);

  } else {
    // Otherwise, Block's new ipostdom is the nearest common post-dominator of
    // NewBlock and Block's old ipostdom
    PostDomTree.changeImmediateDominator(
        BlockPDT, PostDomTree.getNode(PostDomTree.findNearestCommonDominator(
                      NewBlock, BlockPDT->getIDom()->getBlock())));
  }

  return NewBlock;
}

/// Generate the code inside the body of the vectorized loop. Assumes a single
/// LoopVectorBody basic block was created for this; introduces additional
/// basic blocks as needed, and fills them all.
void IntelVPlan::vectorize(VPTransformState *State) {

  BasicBlock *VectorPreHeaderBB = State->CFG.PrevBB;
  BasicBlock *VectorHeaderBB = VectorPreHeaderBB->getSingleSuccessor();
  assert(VectorHeaderBB && "Loop preheader does not have a single successor.");
  BasicBlock *VectorLatchBB = VectorHeaderBB;
  auto CurrIP = State->Builder.saveIP();

  // 1. Make room to generate basic blocks inside loop body if needed.
  VectorLatchBB = VectorHeaderBB->splitBasicBlock(
      VectorHeaderBB->getFirstInsertionPt(), "vector.body.latch");
  Loop *L = State->LI->getLoopFor(VectorHeaderBB);
  L->addBasicBlockToLoop(VectorLatchBB, *State->LI);
  // Remove the edge between Header and Latch to allow other connections.
  // Temporarily terminate with unreachable until CFG is rewired.
  // Note: this asserts xform code's assumption that getFirstInsertionPt()
  // can be dereferenced into an Instruction.
  VectorHeaderBB->getTerminator()->eraseFromParent();
  State->Builder.SetInsertPoint(VectorHeaderBB);
  State->Builder.CreateUnreachable();
  // Set insertion point to vector loop PH
  State->Builder.SetInsertPoint(VectorPreHeaderBB->getTerminator());

  // 2. Generate code in loop body of vectorized version.
  State->CFG.PrevVPBB = nullptr;
  State->CFG.PrevBB = VectorPreHeaderBB;
  State->CFG.LastBB = VectorLatchBB;

  for (VPBlockBase *CurrentBlock = Entry; CurrentBlock != nullptr;
       CurrentBlock = CurrentBlock->getSingleSuccessor()) {
    assert(CurrentBlock->getSuccessors().size() <= 1 &&
           "Multiple successors at top level.");
    CurrentBlock->vectorize(State);
  }

  // 3. Fix the back edges
  for (auto Edge : State->CFG.EdgesToFix) {
    VPBasicBlock *FromVPBB = Edge.first;
    assert(State->CFG.VPBB2IRBB.count(FromVPBB) &&
           "The IR basic block should be ready at this moment");
    BasicBlock *FromBB = State->CFG.VPBB2IRBB[FromVPBB];
    BasicBlock *ToBB = Edge.second;

    // We should have conditional branch from FromBB to ToBB. Conditional
    // branch
    // is 2 edges - forward edge and backward edge.
    // The forward edge should be in-place, we are fixing the backward
    // edge only.
    assert(!isa<UnreachableInst>(FromBB->getTerminator()) &&
           "One edge should be in-place");

    BasicBlock *FirstSuccBB = FromBB->getSingleSuccessor();
    FromBB->getTerminator()->eraseFromParent();
    Value *Bit = FromVPBB->getConditionBitRecipe()->getConditionBit();
    assert(Bit && "Cannot create conditional branch with empty bit.");
    BranchInst::Create(FirstSuccBB, ToBB, Bit, FromBB);
  }

  // 4. Merge the temporary latch created with the last basic block filled.
  BasicBlock *LastBB = State->CFG.PrevBB;

  // Connect LastBB to VectorLatchBB to facilitate their merge.
  assert(isa<UnreachableInst>(LastBB->getTerminator()) &&
         "Expected VPlan CFG to terminate with unreachable");
  LastBB->getTerminator()->eraseFromParent();
  BranchInst::Create(VectorLatchBB, LastBB);

  // Merge LastBB with Latch.
  bool merged = MergeBlockIntoPredecessor(VectorLatchBB, nullptr, State->LI);
  assert(merged && "Could not merge last basic block with latch.");
  (void) merged;
  VectorLatchBB = LastBB;

  // Do no try to update dominator tree as we may be generating vector loops
  // with inner loops. Right now we are not marking any analyses as
  // preserved - so this should be ok.
  // updateDominatorTree(State->DT, VectorPreHeaderBB, VectorLatchBB);
  State->Builder.restoreIP(CurrIP);
}

void VPBasicBlock::vectorize(VPTransformState *State) {

  // Loop PH and Loop Exit VPBasicBlocks are part of VPLoopRegion but they are
  // actually ouside of the loop and they shouldn't be vectorized. We decided to
  // vectorize Loop PH, so this function is only returning false for Loop Exit.
  // TODO: Use a better name for this function.
  if (!isInsideLoop())
    return;

  VPIterationInstance *I = State->Instance;
  bool Replica = I && !(I->Part == 0 && I->Lane == 0);
  VPBasicBlock *PrevVPBB = State->CFG.PrevVPBB;
  VPBlockBase *SingleHPred = nullptr;
  BasicBlock *NewBB = State->CFG.PrevBB; // Reuse it if possible.
  VPLoopRegion *ParentLoop = nullptr;

  // 1. Create an IR basic block, or reuse one already available if possible.
  // The last IR basic block is reused in four cases:
  // A. the first VPBB reuses the pre-header BB - when PrevVPBB is null;
  // B. the second VPBB reuses the header BB;
  // C. when the current VPBB has a single (hierarchical) predecessor which
  //    is PrevVPBB and the latter has a single (hierarchical) successor; and
  // D. when the current VPBB is an entry of a region replica - where PrevVPBB
  //    is the exit of this region from a previous instance.
  // TODO: We currently cannot use VPLoopRegion class here
  if (PrevVPBB && (ParentLoop = dyn_cast<VPLoopRegion>(this->getParent())) &&
      ParentLoop->getEntry() == PrevVPBB /* B */ &&
      // TODO: We need to properly support outer-loop vectorization scenarios.
      // Latches for inner loops are not removed and we don't have VPlan,
      // VPLoopInfo, etc. accessible from State. Temporal fix: outermost loop's
      // parent is TopRegion. TopRegion's parent is null.
      !ParentLoop->getParent()->getParent()) {

    // Set NewBB to loop H basic block
    BasicBlock *LoopPH = State->CFG.PrevBB;
    NewBB = LoopPH->getSingleSuccessor();
    assert(NewBB && "Expected single successor from loop pre-header");
    State->Builder.SetInsertPoint(NewBB->getTerminator());
    State->CFG.PrevBB = NewBB;
  } else if (PrevVPBB /* A */ &&
             !((SingleHPred = getSingleHierarchicalPredecessor()) &&
               SingleHPred->getExitBasicBlock() == PrevVPBB &&
               PrevVPBB->getSingleHierarchicalSuccessor()) && /* C */
             !(Replica && getPredecessors().empty())) {       /* D */

    NewBB = createEmptyBasicBlock(State->CFG);
    State->Builder.SetInsertPoint(NewBB);
    // Temporarily terminate with unreachable until CFG is rewired.
    UnreachableInst *Terminator = State->Builder.CreateUnreachable();
    State->Builder.SetInsertPoint(Terminator);
    // Register NewBB in its loop. In innermost loops its the same for all
    // BB's.
    Loop *L = State->LI->getLoopFor(State->CFG.LastBB);
    L->addBasicBlockToLoop(NewBB, *State->LI);
    State->CFG.PrevBB = NewBB;
  }

  // 2. Fill the IR basic block with IR instructions.
  DEBUG(dbgs() << "LV: vectorizing VPBB:" << getName()
               << " in BB:" << NewBB->getName() << '\n');

  State->CFG.VPBB2IRBB[this] = NewBB;
  State->CFG.PrevVPBB = this;

  for (VPRecipeBase &Recipe : Recipes)
    Recipe.vectorize(*State);

  // ILV's MaskValue is set when we find a BlockPredicateRecipe in
  // VPBasicBlock's list of recipes. After generating code for all the
  // VPBasicBlock's instructions, we have to reset MaskValue in order not to
  // propagate its value to the next VPBasicBlock.
  State->ILV->setMaskValue(nullptr);

  DEBUG(dbgs() << "LV: filled BB:" << *NewBB);
}

using VPDomTree = DomTreeBase<VPBlockBase>;
template void llvm::DomTreeBuilder::Calculate<VPDomTree, VPRegionBlock>(
    VPDomTree &DT, VPRegionBlock &VPR);

using VPPostDomTree = PostDomTreeBase<VPBlockBase>;
template void llvm::DomTreeBuilder::Calculate<VPPostDomTree, VPRegionBlock>(
    VPPostDomTree &PDT, VPRegionBlock &VPR);

void VPUniformConditionBitRecipe::vectorize(VPTransformState &State) {
  if (isa<Instruction>(ScConditionBit)) {
    State.ILV->serializeInstruction(cast<Instruction>(ScConditionBit));
    ConditionBit = State.ILV->getScalarValue(ScConditionBit, 0);
  }
}
