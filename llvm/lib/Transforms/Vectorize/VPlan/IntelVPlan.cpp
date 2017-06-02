
#include "IntelVPlan.h"
#include "LoopVectorizationCodeGen.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"

using namespace llvm;
using namespace llvm::vpo;

#define DEBUG_TYPE "intel-vplan"

static cl::opt<bool> DisableHCFGVerification(
    "vplan-disable-verification", cl::init(false),
    cl::desc("Disable VPlan HCFG verification"));

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
  VectorLatchBB = LastBB;

  // Do no try to update dominator tree as we may be generating vector loops
  // with inner loops. Right now we are not marking any analyses as
  // preserved - so this should be ok.
  // updateDominatorTree(State->DT, VectorPreHeaderBB, VectorLatchBB);
  State->Builder.restoreIP(CurrIP);
}

void VPBasicBlock::vectorize(VPTransformState *State) {

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

  // Reset MaskValue after vectorizing basic block recipes.
  State->ILV->setMaskValue(nullptr);

  DEBUG(dbgs() << "LV: filled BB:" << *NewBB);
}

// Verify that Top Region contains the same number of loops (VPLoopRegion) as
// VPLoopInfo and LoopInfo
static void verifyNumLoops(const VPRegionBlock *TopRegion, const Loop *TheLoop,
                           const VPLoopInfo *VPLInfo, const LoopInfo *LI) {

  std::function<unsigned(const VPRegionBlock *)> countLoopsInRegion =
      [&](const VPRegionBlock *Region) -> unsigned {

    unsigned NumLoops = 0;

    for (const VPBlockBase *VPB :
         make_range(df_iterator<const VPBlockBase *>::begin(Region->getEntry()),
                    df_iterator<const VPBlockBase *>::end(Region->getExit()))) {

      if (isa<VPLoopRegion>(VPB))
        ++NumLoops;

      // Count nested VPLoops
      if (const VPRegionBlock *VPR = dyn_cast<VPRegionBlock>(VPB))
        NumLoops += countLoopsInRegion(VPR);
    }

    return NumLoops;
  };

  // TODO: C++14 needed (generic lambdas). Replicating code by now
  std::function<unsigned(const VPLoop *)> countLoopsInVPLoop =
      [&](const VPLoop *Lp) -> unsigned {

    const std::vector<VPLoop *> &SubLoops = Lp->getSubLoops();
    unsigned NumLoops = SubLoops.size();

    for (const VPLoop *SL : SubLoops)
      NumLoops += countLoopsInVPLoop(SL);

    return NumLoops;
  };
  std::function<unsigned(const Loop *)> countLoopsInLoop =
      [&](const Loop *Lp) -> unsigned {

    const std::vector<Loop *> &SubLoops = Lp->getSubLoops();
    unsigned NumLoops = SubLoops.size();

    for (const Loop *SL : SubLoops)
      NumLoops += countLoopsInLoop(SL);

    return NumLoops;
  };

  // Compare number of loops in HCFG with loops in VPLoopInfo and LoopInfo

  unsigned NumLoopsInCFG = countLoopsInRegion(TopRegion);

  assert(VPLInfo->size() && "More than one top loop is not expected");
  unsigned NumLoopsInVPLoopInfo =
      1 /*TopLoop*/ + countLoopsInVPLoop(*VPLInfo->begin());
  unsigned NumLoopsInLoopInfo = 1 /*TopLoop*/ + countLoopsInLoop(TheLoop);

  //dbgs().indent(2) << "Verify Loops:\n";
  //dbgs().indent(4) << "NumLoopsInCFG: " << NumLoopsInCFG << "\n";
  //dbgs().indent(4) << "NumLoopsInVPLoopInfo: " << NumLoopsInVPLoopInfo << "\n";
  //dbgs().indent(4) << "NumLoopsInLoopInfo: " << NumLoopsInLoopInfo << "\n";

  assert(NumLoopsInCFG == NumLoopsInVPLoopInfo &&
         NumLoopsInVPLoopInfo == NumLoopsInLoopInfo &&
         "Number of loops in HCFG, VPLoopInfo and LoopInfo don't match");
}

// Verify loop information for blocks inside a loop region
static void verifyLoopRegions(const VPRegionBlock *TopRegion,
                              const VPLoopInfo *VPLInfo) {

  // Return true if \p Block is outside of the loop  SCC
  auto BlockIsOutsideOfLoopSCC = [](const VPBlockBase *Block,
                                    const VPLoopRegion *LoopR) {

    // Loop PH or Region Exit are outside
    if (LoopR->getEntry() == Block || LoopR->getExit() == Block)
      return true;

// In case we consider keeping more blocks after loop exit inside the loop
// region
#if 0
    SmallVector<VPBlockBase *, 2> LoopExits;
    LoopR->getVPLoop()->getUniqueExitBlocks(LoopExits);

    // TODO: Better approach?
    // Block is outside of loop SCC if is reachable from any loop exit
    for (VPBlockBase *LoopExit : LoopExits) {
      const auto &Begin = df_iterator<const VPBlockBase *>::begin(LoopExit);
      // Please note that this works only because region exit doesn't have
      // successors! df_iterator wouldn't stop in region exit if it had
      // successors.
      const auto &End = df_iterator<const VPBlockBase *>::end(LoopR->getExit());

      if (std::find(Begin, End, Block) != End)
        return true;
    }
#endif

    return false;
  };

  // Auxiliary recursive function that implements the main functionality of
  // verifyLoopRegions
  typedef std::function<void(const VPRegionBlock *, const VPLoopRegion *)> RT;
  RT verifyLoopRegionsImp = [&](const VPRegionBlock *Region,
                                const VPLoopRegion *ParentLoopR) {

    assert(Region && "Region cannot be null");

    // If Region is a VPLoopRegion, Region and VPLoopRegion must be equal
    assert((!isa<VPLoopRegion>(Region) || Region == ParentLoopR) &&
           "Wrong Region/ParentLoopR");

    const VPLoop *ParentLoop = nullptr;
    if (ParentLoopR) {
      ParentLoop = ParentLoopR->getVPLoop();
      assert(ParentLoopR && "Missing VPLoop for VPLoopRegion");
    }

    // VPLoopRegion/VPLoop specific checks if visiting a Region that is a
    // VPLoopRegion
    if (Region == ParentLoopR) {
      const VPBlockBase *Preheader = ParentLoopR->getEntry();
      assert(Preheader && Preheader == ParentLoop->getLoopPreheader() &&
             "Wrong loop preheader");
      const VPBlockBase *Header = Preheader->getSingleSuccessor();
      assert(Header && "Loop preheader must have a single successor");
      assert(Header == ParentLoop->getHeader() && "Wrong loop header");

      assert((VPLInfo->getLoopFor(Header) == ParentLoop) &&
             "Unexpected loop from loop header");
    }

    // Traverse blocks in Region
    SmallVector<const VPRegionBlock *, 4> SubRegions;
    for (const VPBlockBase *VPB :
         make_range(df_iterator<const VPBlockBase *>::begin(Region->getEntry()),
                    df_iterator<const VPBlockBase *>::end(Region->getExit()))) {

      const VPLoop *ContainerLoop;
      // Region is a VPLoopRegion. Blocks outside of the loop SCC shouldn't be
      // contained in this loop but in Region's parent loop (if any)
      if (Region == ParentLoopR && BlockIsOutsideOfLoopSCC(VPB, ParentLoopR)) {
        ContainerLoop = VPLInfo->getLoopFor(Region);
      } else {
        // Non-loop region, block should be contained in parent loop
        ContainerLoop = ParentLoop;
      }

      if (ContainerLoop)
        assert(ContainerLoop->contains(VPB) &&
               //TODO: Redundant?
               VPLInfo->getLoopFor(VPB) == ContainerLoop &&
               "Block is not contained in the right loop");
      else
        assert(!VPLInfo->getLoopFor(VPB) &&
               "Block should not be contained in any VPLoop");

      // Collect subregions for later visit
      if (const VPRegionBlock *SubRegion = dyn_cast<VPRegionBlock>(VPB))
        SubRegions.push_back(SubRegion);
    }

    // Visit subregions
    for (const VPRegionBlock *SR : SubRegions) {
      // If subregion is a loop region, use it as ParentLoop in the visit
      if (const VPLoopRegion *NewParentLoop = dyn_cast<VPLoopRegion>(SR)) {
        verifyLoopRegionsImp(SR, NewParentLoop);
      } else {
        verifyLoopRegionsImp(SR, ParentLoopR);
      }
    }
  };

  verifyLoopRegionsImp(TopRegion, nullptr /*ParentLoopRegion*/);
}

// Main function for VPRegionBlock verification
static void verifyRegions(const VPRegionBlock *Region) {

  const VPBlockBase *Entry = Region->getEntry();
  const VPBlockBase *Exit = Region->getExit();

  // At this point, we don't expect Entry or Exit to be another region
  assert(isa<VPBasicBlock>(Entry) && "Region entry is not a VPBasicBlock");
  assert(isa<VPBasicBlock>(Exit) && "Region exit is not a VPBasicBlock");

  // Entry and Exit shouldn't have any predecessor/successor, respectively
  assert(Entry->getNumPredecessors() == 0 && "Region entry has predecessors");
  assert(Exit->getNumSuccessors() == 0 && "Region exit has successors");

  // We are not creating all possible SESE regions. At this point, Entry must
  // have more than two successors and Exit more than two predecessors. This
  // doesn't apply to VPLoopRegion's or TopRegion.
  if (Region->getParent() != nullptr /*TopRegion*/ &&
      !isa<VPLoopRegion>(Region)) {
    assert(Entry->getNumSuccessors() > 1 &&
           "Region entry must have more than one successors");
    assert(Exit->getNumPredecessors() > 1 &&
           "Region exit must have more than one predecessors");
  }

  // Traverse Region's blocks
  SmallVector<const VPRegionBlock *, 4> SubRegions;
  unsigned NumBlocks = 0;
  for (const VPBlockBase *VPB :
       make_range(df_iterator<const VPBlockBase *>::begin(Region->getEntry()),
                  df_iterator<const VPBlockBase *>::end(Region->getExit()))) {
    // Compute Region's size
    ++NumBlocks;

    // Check block's parent
    assert(VPB->getParent() == Region && "VPBlockBase has wrong parent");

    // Check block's ConditionBitRecipe
    if (VPB->getNumSuccessors() > 1)
      assert(VPB->getConditionBitRecipe() && "Missing ConditionBitRecipe");
    else
      assert(!VPB->getConditionBitRecipe() && "Unexpected ConditionBitRecipe");

    // Check block's successors
    const auto &Successors = VPB->getSuccessors();
    for (const VPBlockBase *Succ : Successors) {
      // There must be only one instance of the successor in block's successor
      // list. TODO: This won't work for switch statements
      assert(std::count(Successors.begin(), Successors.end(), Succ) == 1 &&
             "Multiple instances of the same successor");

      // There must be a bidirectional link between block and successor
      const auto &SuccPreds = Succ->getPredecessors();
      assert(std::find(SuccPreds.begin(), SuccPreds.end(), VPB) !=
                 SuccPreds.end() &&
             "Missing predecessor link");
    }

    // Check block's predecessors
    const auto &Predecessors = VPB->getPredecessors();
    for (const VPBlockBase *Pred : Predecessors) {

      // Block and predecessor must be inside the same region
      assert(Pred->getParent() == VPB->getParent() &&
             "Predecessor is not in the same region");

      // There must be only one instance of the predecessor in block's
      // predecessor list. TODO: This won't work for switch statements
      assert(std::count(Predecessors.begin(), Predecessors.end(), Pred) == 1 &&
             "Multiple instances of the same predecessor");

      // There must be a bidirectional link between block and predecessor
      const auto &PredSuccs = Pred->getSuccessors();
      assert(std::find(PredSuccs.begin(), PredSuccs.end(), VPB) !=
                 PredSuccs.end() &&
             "Missing successor link");
    }

    // Collect subregions for later visit
    if (const VPRegionBlock *SubRegion = dyn_cast<VPRegionBlock>(VPB))
      SubRegions.push_back(SubRegion);
  }

  assert(NumBlocks == Region->getSize() && "Region has a wrong size");

  // Visit subregions
  for (const VPRegionBlock *SubRegion : SubRegions)
    verifyRegions(SubRegion);
}


// Main class for loop verification
static void verifyLoops(const VPRegionBlock *TopRegion, const Loop *TheLoop,
                        const VPLoopInfo *VPLInfo, const LoopInfo *LI) {

  verifyNumLoops(TopRegion, TheLoop, VPLInfo, LI);
  verifyLoopRegions(TopRegion, VPLInfo);
}

// Public interface to verify the hierarchical CFG
void IntelVPlanUtils::verifyHierarchicalCFG(const VPRegionBlock *TopRegion,
                                            const Loop *TheLoop,
                                            const VPLoopInfo *VPLInfo,
                                            const LoopInfo *LI) const {
  if (DisableHCFGVerification)
    return;

  assert(((!VPLInfo && !LI && !TheLoop) || (VPLInfo && LI && TheLoop)) &&
         "TheLoop, VPLInfo and LI must be all set (or not)");

  // dbgs() << "Verifying Hierarchical CFG:\n";

  if (VPLInfo && LI && TheLoop)
    verifyLoops(TopRegion, TheLoop, VPLInfo, LI);

  verifyRegions(TopRegion);
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
