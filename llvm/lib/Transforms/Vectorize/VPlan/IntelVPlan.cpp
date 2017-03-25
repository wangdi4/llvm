
#include "IntelVPlan.h"
#include "LoopVectorizationCodeGen.h"

using namespace llvm;
using namespace llvm::vpo;

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
                                          VPDominatorTree &PostDomTree) {

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

template void llvm::Calculate<VPRegionBlock, VPBlockBase *>(
    DominatorTreeBase<GraphTraits<VPBlockBase *>::NodeType> &DT,
    VPRegionBlock &VPR);
//template void llvm::Calculate<VPRegionBlock, Inverse<VPBlockBase *>>(
//    DominatorTreeBase<GraphTraits<Inverse<VPBlockBase *>>::NodeType> &DT,
//    VPRegionBlock &VPR);

void VPUniformConditionBitRecipe::vectorize(VPTransformState &State) {
  if (isa<Instruction>(ScConditionBit)) {
    State.ILV->serializeInstruction(cast<Instruction>(ScConditionBit));
    ConditionBit = State.ILV->getScalarValue(ScConditionBit, 0);
  }
}
