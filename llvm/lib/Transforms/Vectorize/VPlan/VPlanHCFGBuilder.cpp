#include "VPlanHCFGBuilder.h"
#include "Intel_LoopCFU.h"
#include "VPLoopInfo.h"
#include "llvm/Analysis/Intel_VPO/WRegionInfo/WRegionInfo.h"
#include "llvm/Analysis/LoopIterator.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/IR/IntrinsicInst.h"

#define DEBUG_TYPE "VPlanHCFGBuilder"

using namespace llvm;
using namespace vpo;

static cl::opt<bool> DisableNonLoopSubRegions(
    "disable-vplan-subregions", cl::init(false), cl::Hidden,
    cl::desc("Disable construction of non-loop subregions in VPlan"));

static cl::opt<bool> LoopMassagingEnabled(
    "vplan-enable-loop-massaging",
    cl::init(false), // TODO: vplan-disable-loop-massaging
    cl::Hidden,
    cl::desc("Enable loop massaging in VPlan (Multiple to Singular Exit)"));

static cl::opt<bool> VPlanLoopCFU(
    "vplan-loop-cfu", cl::init(false), cl::Hidden,
    cl::desc("Perform inner loop control flow uniformity transformation"));

static bool isInstructionToIgnore(Instruction *I) {
  // DeadInstructions are not taken into account at this point. IV update and
  // loop latch condition need to be part of HCFG to constitute a
  // UniformConditionBitRecipe. If we treat them as dead instructions, we
  // would create a LiveInConditionBitRecipe for the loop latch condition,
  // which is not correct.
  return /*DeadInstructions.count(I) ||*/ isa<BranchInst>(I) ||
         isa<DbgInfoIntrinsic>(I);
}

static bool isConditionForUniformBranch(Instruction *I, const Loop *TheLoop) {
  auto isBranchInst = [&](User *U) -> bool {
    return isa<BranchInst>(U) && TheLoop->contains(cast<Instruction>(U));
  };
  return TheLoop->contains(I) && /*Legal->isUniformForTheLoop(I)&&*/
         any_of(I->users(), isBranchInst);
}

// Create new OnyByOneRecipes and ConditionBitRecipes in a VPBasicBlock, given
// its BasicBlock counterpart. This function must be invoked in RPO because
// creation of UniformConditionBitRecipe assumes that all predecessors have
// been visited.
static void createRecipesForVPBB(
    BasicBlock *BB, VPBasicBlock *VPBB, const Loop *TheLoop,
    IntelVPlanUtils &PlanUtils,
    DenseMap<Value *, VPConditionBitRecipeBase *> &BranchCondMap) {

  BasicBlock::iterator I = BB->begin();
  BasicBlock::iterator E = BB->end();

  while (I != E) {
    // Search for first live Instruction to open VPBB.
    while (I != E && isInstructionToIgnore(&*I))
      ++I;

    if (I == E)
      break;

    // If 'I' is a branch condition, create a UniformConditionBitRecipe.
    // Note that we don't have to add the recipe as successor selector at
    // this point. The branch using this conditiong might not be necessarily
    // in this VPBB.
    if (isConditionForUniformBranch(&*I, TheLoop)) {
      Instruction *Instr = &*I;
      VPUniformConditionBitRecipe *Recipe =
          PlanUtils.createUniformConditionBitRecipe(Instr);
      PlanUtils.appendRecipeToBasicBlock(Recipe, VPBB);
      VPlan *Plan = PlanUtils.getVPlan();
      Plan->setInst2Recipe(Instr, Recipe);

      for (User *U : Instr->users()) {
        if (isa<BranchInst>(U) && TheLoop->contains(cast<Instruction>(U)))
          BranchCondMap[cast<BranchInst>(U)->getCondition()] = Recipe;
      }

      // Move iterator forward to skip branch condition in next iteration
      ++I;
      continue;
    }

    // Create new OnebyOneRecipe and add it to VPBB

    // Search for last live Instruction to close VPBB.
    BasicBlock::iterator J = I;
    for (++J; J != E; ++J) {
      Instruction *Instr = &*J;
      if (isInstructionToIgnore(Instr) ||
          isConditionForUniformBranch(Instr, TheLoop))
        break; // Sequence of instructions not to ignore ended.
    }

    bool Scalarized = false;
    VPRecipeBase *Recipe = PlanUtils.createOneByOneRecipe(I, J, Scalarized);
    PlanUtils.appendRecipeToBasicBlock(Recipe, VPBB);

    I = J;
  }
}

// Create a new empty VPBasicBlock for an incomming BasicBlock or retrieve an
// existing one if it was already created.
static VPBasicBlock *
createOrGetVPBB(BasicBlock *BB, VPRegionBlock *TopRegion,
                DenseMap<BasicBlock *, VPBasicBlock *> &BB2VPBB,
                IntelVPlanUtils &PlanUtils, unsigned &TopRegionSize) {

  VPBasicBlock *VPBB;
  auto BlockIt = BB2VPBB.find(BB);

  if (BlockIt == BB2VPBB.end()) {
    // New VPBB
    VPBB = PlanUtils.createBasicBlock();
    BB2VPBB[BB] = VPBB;
    VPBB->setOriginalBB(BB);
    PlanUtils.setBlockParent(VPBB, TopRegion);
    ++TopRegionSize;
  } else {
    // Retrieve existing VPBB
    VPBB = BlockIt->second;
  }

  return VPBB;
}

// Build plain CFG from incomming IR using only VPBasicBlock's that contain
// OneByOneRecipe's and ConditionBitRecipe's. Return VPRegionBlock that
// encloses all the VPBasicBlock's of the plain CFG.
VPRegionBlock *VPlanHCFGBuilder::buildPlainCFG(HCFGState &State) {

  IntelVPlanUtils &PlanUtils = State.PlanUtils;
  Loop *TheLoop = State.TheLoop;

  // Temporal maps for this plan CFG
  DenseMap<BasicBlock *, VPBasicBlock *> BB2VPBB;
  DenseMap<Value *, VPConditionBitRecipeBase *> BranchCondMap;

  // Create Top Region. It will be parent of all VPBBs
  VPRegionBlock *TopRegion = PlanUtils.createRegion(false /*isReplicator*/);
  unsigned TopRegionSize = 0;

  // Scan the body of the loop in a topological order to visit each basic block
  // after having visited its predecessor basic blocks.
  LoopBlocksDFS DFS(TheLoop);
  DFS.perform(LI);

  for (BasicBlock *BB : make_range(DFS.beginRPO(), DFS.endRPO())) {

    DEBUG(dbgs() << "Building VPBasicBlock for " << BB->getName() << "\n");

    // Create new VPBasicBlock and its recipes
    VPBasicBlock *VPBB =
        createOrGetVPBB(BB, TopRegion, BB2VPBB, PlanUtils, TopRegionSize);
    createRecipesForVPBB(BB, VPBB, TheLoop, PlanUtils, BranchCondMap);

    // Add successors and predecessors
    TerminatorInst *TI = BB->getTerminator();
    assert(TI && "Terminator expected");
    unsigned NumSuccs = TI->getNumSuccessors();

    // Note: we are not invoking createRecipesForVPBB for successor blocks at
    // this point because we would be breaking the RPO traversal
    if (NumSuccs == 1) {
      VPBasicBlock *SuccVPBB = createOrGetVPBB(
          TI->getSuccessor(0), TopRegion, BB2VPBB, PlanUtils, TopRegionSize);
      assert(SuccVPBB && "VPBB Successor not found");

      PlanUtils.setSuccessor(VPBB, SuccVPBB);
      VPBB->setCBlock(BB);
      VPBB->setTBlock(TI->getSuccessor(0));

    } else if (NumSuccs == 2) {
      VPBasicBlock *SuccVPBB0 = createOrGetVPBB(
          TI->getSuccessor(0), TopRegion, BB2VPBB, PlanUtils, TopRegionSize);
      assert(SuccVPBB0 && "Successor 0 not found");
      VPBasicBlock *SuccVPBB1 = createOrGetVPBB(
          TI->getSuccessor(1), TopRegion, BB2VPBB, PlanUtils, TopRegionSize);
      assert(SuccVPBB1 && "Successor 1 not found");

      // Add ConditionBitRecipe to VPBB
      BranchInst *Br = cast<BranchInst>(TI);
      Value *Condition = Br->getCondition();
      VPConditionBitRecipeBase *CondBitR = nullptr;

      if (BranchCondMap.count(Condition)) {
        // ConditionBitRecipe is a UniformConditionBitRecipe
        CondBitR = BranchCondMap[Condition];
      } else {
        // Branch condition is a live-in value. Create a
        // LiveInConditionBitRecipe
        CondBitR = PlanUtils.createLiveInConditionBitRecipe(Condition);
      }

      PlanUtils.setTwoSuccessors(VPBB, CondBitR, SuccVPBB0, SuccVPBB1);
      VPBB->setCBlock(BB);
      VPBB->setTBlock(TI->getSuccessor(0));
      VPBB->setFBlock(TI->getSuccessor(1));

    } else {
      llvm_unreachable("Number of successors not supported");
    }
  }

  // Add outermost loop preheader to plain CFG. It needs explicit treatment
  // because it's not a successor of any block inside the loop.
  BasicBlock *PreheaderBB = TheLoop->getLoopPreheader();
  assert((PreheaderBB->getTerminator()->getNumSuccessors() == 1) &&
         "Unexpected loop preheader");

  VPBasicBlock *PreheaderVPBB = createOrGetVPBB(PreheaderBB, TopRegion, BB2VPBB,
                                                PlanUtils, TopRegionSize);
  createRecipesForVPBB(PreheaderBB, PreheaderVPBB, TheLoop, PlanUtils,
                       BranchCondMap);
  VPBlockBase *HeaderVPBB = BB2VPBB[TheLoop->getHeader()];
  PlanUtils.setSuccessor(PreheaderVPBB, HeaderVPBB);

  // Empty VPBasicBlock were created for loop exit BasicBlocks but they weren't
  // visited because they are not inside the loop. Create now recipes for them.
  SmallVector<BasicBlock *, 2> LoopExits;
  TheLoop->getUniqueExitBlocks(LoopExits);
  for (BasicBlock *BB : LoopExits)
    createRecipesForVPBB(BB, BB2VPBB[BB], TheLoop, PlanUtils, BranchCondMap);

  // Top Region setup

  // Create a dummy block as Top Region's entry
  VPBlockBase *RegionEntry = PlanUtils.createBasicBlock();
  ++TopRegionSize;
  PlanUtils.setBlockParent(RegionEntry, TopRegion);
  PlanUtils.setSuccessor(RegionEntry, PreheaderVPBB);

  // Create a dummy block as Top Region's exit
  VPBlockBase *RegionExit = PlanUtils.createBasicBlock();
  ++TopRegionSize;
  PlanUtils.setBlockParent(RegionExit, TopRegion);

  // Connect dummy Top Region's exit.
  if (LoopExits.size() == 1) {
    PlanUtils.setSuccessor(BB2VPBB[LoopExits.front()], RegionExit);
  } else {
    // If there are multiple exits in the outermost loop, we need another dummy
    // block as landing pad for all of them.
    SmallVector<BasicBlock *, 2> ExitBBs;
    TheLoop->getUniqueExitBlocks(ExitBBs);
    assert(ExitBBs.size() > 1 && "Wrong number of exit blocks");

    VPBlockBase *LandingPad = PlanUtils.createBasicBlock();
    ++TopRegionSize;
    PlanUtils.setBlockParent(LandingPad, TopRegion);

    // Connect multiple exits to landing pad
    for (auto ExitBB : make_range(ExitBBs.begin(), ExitBBs.end())) {
      VPBasicBlock *ExitVPBB = BB2VPBB[ExitBB];
      PlanUtils.setSuccessor(ExitVPBB, LandingPad);
    }

    // Connect landing pad to Top Region's exit
    PlanUtils.setSuccessor(LandingPad, RegionExit);
  }

  PlanUtils.setRegionEntry(TopRegion, RegionEntry);
  PlanUtils.setRegionExit(TopRegion, RegionExit);
  PlanUtils.setRegionSize(TopRegion, TopRegionSize);

  return TopRegion;
}

// Split loops' preheader block that are not in canonical form
void VPlanHCFGBuilder::splitLoopsPreheader(VPLoop *VPL, HCFGState &State) {

  // TODO: So far, I haven't found a test case that hits one of these asserts.
  // The code commented out below should cover the second one.

  IntelVPlanUtils &PlanUtils = State.PlanUtils;
  VPLoopInfo *VPLInfo = PlanUtils.getVPlan()->getVPLoopInfo();

  // Temporal assert to detect loop header with more than one loop external
  // predecessor
  unsigned NumExternalPreds = 0;
  for (const VPBlockBase *Pred : VPL->getHeader()->getPredecessors()) {
    if (!VPL->contains(Pred))
      ++NumExternalPreds;
  }
  assert((NumExternalPreds == 1) &&
         "Loop header's external predecessor is not 1");

  // Temporal assert to detect loop preheader with multiple successors
  assert((VPL->getLoopPreheader()->getNumSuccessors() == 1) &&
         "Loop preheader with multiple successors are not supported");

  // If PH has multiple successors, create new PH such that PH->NewPH->H
  // if (VPL->getLoopPreheader()->getNumSuccessors() > 1) {

  //  VPBlockBase *OldPreheader = VPL->getLoopPreheader();
  //  VPBlockBase *Header = VPL->getHeader();
  //  assert((DomTree.getNode(Header)->getIDom()->getBlock() == OldPreheader) &&
  //         "Header IDom is not Preheader");

  //  // Create new preheader
  //  VPBasicBlock *NewPreheader = PlanUtils.createBasicBlock();
  //  PlanUtils.insertBlockAfter(NewPreheader, OldPreheader);

  //  // Add new preheader to VPLoopInfo
  //  if (VPLoop *PHLoop = VPLInfo->getLoopFor(OldPreheader)) {
  //    PHLoop->addBasicBlockToLoop(NewPreheader, *VPLInfo);
  //  }

  //  // Update dom/postdom information

  //  // Old preheader is idom of new preheader
  //  VPDomTreeNode *NewPHDomNode =
  //      DomTree.addNewBlock(NewPreheader, OldPreheader /*IDom*/);

  //  // New preheader is idom of header
  //  VPDomTreeNode *DTHeader = DomTree.getNode(Header);
  //  assert(DTHeader && "Expected DomTreeNode for loop header");
  //  DomTree.changeImmediateDominator(DTHeader, NewPHDomNode);

  //  // Header is ipostdom of new preheader
  //  //VPDomTreeNode *NewPHPostDomNode =
  //  PostDomTree.addNewBlock(NewPreheader, Header /*IDom*/);

  //  // New preheader is not ipostdom of any block
  //
  //  // This is not true: New preheader is ipostdom of old preheader
  //  //VPDomTreeNode *PDTPreheader = PostDomTree.getNode(OldPreheader);
  //  //assert(PDTPreheader && "Expected DomTreeNode for loop preheader");
  //  //PostDomTree.changeImmediateDominator(PDTPreheader, NewPHPostDomNode);
  //}

  VPBlockBase *PH = VPL->getLoopPreheader();
  assert(PH && "Expected loop preheader");
  assert((PH->getNumSuccessors() == 1) &&
         "Expected preheader with single successor");

  // Split loop PH if:
  //    - there is no WRLoop (auto-vectorization). We need an empty loop PH.
  //    - has multiple predecessors (it's a potential exit of another region).
  //    - is loop H of another loop.
  if (!State.WRLoop || !PH->getSinglePredecessor() ||
      VPLInfo->isLoopHeader(PH)) {
    PlanUtils.splitBlock(PH, VPLInfo, State.VPDomTree, State.VPPostDomTree);
  }

  // Apply simplification to subloops
  for (auto VPSL : VPL->getSubLoops()) {
    splitLoopsPreheader(VPSL, State);
  }
}

void VPlanHCFGBuilder::mergeLoopExits(VPLoop *VPL, HCFGState &State) {

  VPDominatorTree &VPDomTree = State.VPDomTree;
  VPDominatorTree &VPPostDomTree = State.VPPostDomTree;
  IntelVPlanUtils &PlanUtils = State.PlanUtils;
  VPLoopInfo *VPLInfo = PlanUtils.getVPlan()->getVPLoopInfo();

  SmallVector<VPBlockBase *, 2> ExitBlocks;
  VPL->getUniqueExitBlocks(ExitBlocks);

  // Apply simplification to subloops
  for (auto VPSL : VPL->getSubLoops()) {
    mergeLoopExits(VPSL, State);
  }

  // If Exit-Blocks count is less than 2, then there is nothing to do.
  if (ExitBlocks.size() < 2) {
    return;
  }

  DenseMap<VPBlockBase *, VPBlockBase *> Exitting2ExitBlock;
  unsigned ExitCounter = 0;
  VPBlockBase *CascadedExit = nullptr;

  // This function generates the new merged multiple to single exit epilog.
  // This epilog is composed of cascading ifs directing the exit path for
  // each of the loop exiting options.
  // A new Phi recipe is added to the focal exit point indicating which
  // exiting path was taken, then compare statements for the cascading
  // if-blocks are generated to direct the flow to the respective exit path.
  // This function is invoked to build the cascading ifs iteratively from the
  // last cascading if to the first cascading if in the single exit focal point.
  //
  // The following tow variables are used to save data between consecutive calls
  // of the function. PhiRecipe - holds the phi recipe after being generated at
  // the first call. ExittingBlocks - holds the list of exitting-blocks.
  VPPhiValueRecipe *PhiRecipe = nullptr;
  SmallVector<VPBlockBase *, 4> ExittingBlocks;

  auto CreateCascadedExit =
      [&](VPBlockBase *LastCascadedExitBlock, VPBlockBase *ExittingBlock,
          VPBlockBase *ExitBlock, unsigned ExitID) -> VPBlockBase * {

    if (ExitID == 1) {
      PhiRecipe = new VPPhiValueRecipe();
      PhiRecipe->addIncomingValue(VPConstantRecipe(ExitID), ExittingBlock);
      ExittingBlocks.clear();
      ExittingBlocks.push_back(ExittingBlock);
      return ExitBlock;
    }

    ExittingBlocks.push_back(ExittingBlock);
    PhiRecipe->addIncomingValue(VPConstantRecipe(ExitID), ExittingBlock);

    VPBasicBlock *NewCascadedExit = PlanUtils.createBasicBlock();
    VPCmpBitRecipe *CBR =
        new VPCmpBitRecipe(PhiRecipe, VPConstantRecipe(ExitID));
    VPRegionBlock *Parent = ExitBlock->getParent();
    PlanUtils.setBlockParent(NewCascadedExit, Parent);
    PlanUtils.setRegionSize(Parent, Parent->getSize() + 1);
    PlanUtils.setTwoSuccessors(NewCascadedExit, CBR, ExitBlock,
                               LastCascadedExitBlock);
    // Add NewBlock to VPLoopInfo
    if (VPLoop *Loop = VPLInfo->getLoopFor(ExitBlock)) {
      Loop->addBasicBlockToLoop(NewCascadedExit, *VPLInfo);
    }
    if (ExitID == ExitBlocks.size())
      PlanUtils.appendRecipeToBasicBlock(PhiRecipe, NewCascadedExit);
    PlanUtils.appendRecipeToBasicBlock(CBR, NewCascadedExit);

    if (ExitID < ExitBlocks.size())
      return NewCascadedExit;

    for (auto ExittingBlock : ExittingBlocks) {
      PlanUtils.movePredecessor(
          ExittingBlock, Exitting2ExitBlock[ExittingBlock], NewCascadedExit);
    }

    return NewCascadedExit;
  };

  // This function handles the dominance and post-dominance required updates
  // after the above cascading-exits transformation.
  auto FixDominance = [&](VPBlockBase *LastCascadedExitBlock) -> void {

    VPBlockBase *NCD = nullptr;
    for (auto Pred : LastCascadedExitBlock->getPredecessors()) {
      if (!NCD) {
        NCD = Pred;
        continue;
      }
      NCD = VPDomTree.findNearestCommonDominator(NCD, Pred);
    }

    // Update dom information
    VPBlockBase *CascadedExit = nullptr;
    VPBlockBase *NextCascadedExit = LastCascadedExitBlock;
    for (unsigned i = 0; i < ExitBlocks.size() - 1; ++i) {
      CascadedExit = NextCascadedExit;
      NCD = VPDomTree.addNewBlock(CascadedExit, NCD /*IDom*/)->getBlock();
      VPDomTree.changeImmediateDominator(CascadedExit->getSuccessors()[0], NCD);
      NextCascadedExit = CascadedExit->getSuccessors()[1];
    }
    VPDomTree.changeImmediateDominator(NextCascadedExit, NCD);

    // Update post-dom information
    // CascadedExit contains the last cascaded if.
    for (unsigned i = 0; i < ExitBlocks.size() - 1; ++i) {
      VPBlockBase *NCPD = VPPostDomTree.findNearestCommonDominator(
          CascadedExit->getSuccessors()[0], CascadedExit->getSuccessors()[1]);
      VPPostDomTree.addNewBlock(CascadedExit, NCPD);
      CascadedExit = CascadedExit->getSinglePredecessor();
    }

    for (auto Pred : LastCascadedExitBlock->getPredecessors()) {
      VPPostDomTree.changeImmediateDominator(Pred, LastCascadedExitBlock);
    }

    VPPostDomTree.updateDFSNumbers();
    VPDomTree.updateDFSNumbers();
  };

  for (VPBlockBase *Exit : ExitBlocks) {
    ++ExitCounter;
    for (VPBlockBase *Pred : Exit->getPredecessors()) {
      // check if Pred is an exitting block. if not continue.
      if (!VPL->contains(Pred) || Exitting2ExitBlock[Pred])
        continue;
      Exitting2ExitBlock[Pred] = Exit;
      CascadedExit = CreateCascadedExit(CascadedExit, Pred, Exit, ExitCounter);
    }
  }
  // FixDominance(CascadedExit);

  DEBUG(VPlanPrinter PlanPrinter(dbgs(), *PlanUtils.getVPlan());
        PlanPrinter.dump("LVP: Plain CFG for VF=4"));
  FixDominance(CascadedExit);
  // exit(0);
}

// Split loops' exit block that are not in canonical form
void VPlanHCFGBuilder::splitLoopsExit(VPLoop *VPL, HCFGState &State) {

  IntelVPlanUtils &PlanUtils = State.PlanUtils;
  VPLoopInfo *VPLInfo = PlanUtils.getVPlan()->getVPLoopInfo();

  VPBlockBase *Exit = VPL->getUniqueExitBlock();
  assert(Exit && "Only single-exit loops expected");

  // Split loop exit with multiple successors or that is preheader of another
  // loop
  VPBlockBase *PotentialH = Exit->getSingleSuccessor();
  if (!PotentialH ||
      (VPLInfo->isLoopHeader(PotentialH) &&
       VPLInfo->getLoopFor(PotentialH)->getLoopPreheader() == Exit)) {

    PlanUtils.splitBlock(Exit, VPLInfo, State.VPDomTree, State.VPPostDomTree);
  }

  // Apply simplification to subloops
  for (auto VPSL : VPL->getSubLoops()) {
    splitLoopsExit(VPSL, State);
  }
}

// Split basic blocks to increase the number of non-loop regions detected during
// the construction of the hierarchical CFG.
void VPlanHCFGBuilder::simplifyNonLoopRegions(HCFGState &State) {

  IntelVPlanUtils &PlanUtils = State.PlanUtils;
  IntelVPlan *Plan = PlanUtils.getVPlan();
  assert(isa<VPRegionBlock>(Plan->getEntry()) &&
         "VPlan entry is not a VPRegionBlock");
  VPRegionBlock *TopRegion = cast<VPRegionBlock>(Plan->getEntry());

  SmallVector<VPBlockBase *, 32> WorkList;
  SmallPtrSet<VPBlockBase *, 32> Visited;

  WorkList.push_back(TopRegion->getEntry());

  while (!WorkList.empty()) {

    // Get Current and skip it if visited.
    VPBlockBase *CurrentBlock = WorkList.back();
    WorkList.pop_back();
    if (Visited.count(CurrentBlock))
      continue;

    // Set Current to visited
    Visited.insert(CurrentBlock);

    // Potential VPRegion entry
    if (CurrentBlock->getNumSuccessors() > 1) {

      // Currently, this rule covers:
      //   - Loop H with multiple successors
      //   - Region exit that is another region entry
      //   - Loop latch+exiting block with multiple predecessors
      //
      // TODO: skip single basic block loops?
      if (CurrentBlock->getNumPredecessors() > 1) {
        PlanUtils.splitBlock(CurrentBlock, Plan->getVPLoopInfo(),
                             State.VPDomTree, State.VPPostDomTree);
      }

      // TODO: WIP. The code below has to be revisited. It will enable the
      // construction of VPRegions that currently are not built because they
      // share entry/exit nodes with other VPRegions. This transformation would
      // require to introduce new recipes to split original phi instructions
      // that are in the problematic basic blocks.

      // VPBlockBase *PostDom =
      //    PostDomTree.getNode(CurrentBlock)->getIDom()->getBlock();
      // VPBlockBase *Dom = DomTree.getNode(PostDom)->getIDom()->getBlock();
      // assert(isa<VPBasicBlock>(PostDom) &&
      //       "Expected VPBasicBlock as post-dominator");
      // assert(isa<VPBasicBlock>(Dom) && "Expected VPBasicBlock as dominator");

      // TODO: This condition is currently too generic. It needs refinement.
      // However, if detecting more specific cases is expensive, we may want to
      // leave as it is.
      //
      // When we need to insert a fake exit block:
      //   - PostDom is exit of a region and entry of another region (PostDom
      //   numSucc > 1)
      //   - Dom != CurrentBlock:
      //       - Nested region shares exit with parent region. We need a fake
      //       exit for nested region to be created. With fake exit, Dom ==
      //       CurrentBlock
      //       - Dom != CurrentBlock even if we introduce the fake exit. We
      //       won't create region for these cases so we don't want to introduce
      //       fake exit. (TODO: We are currently introducing fake exit for this
      //       case).
      //       - Loops with multiple exiting blocks and region sharing exit
      //       (TODO)
      //       - Anything else?
      //
      // if (Dom != CurrentBlock || PostDom->getNumSuccessors() > 1) {

      //  // New fake exit
      //  VPBasicBlock *FakeExit = PlanUtils.createBasicBlock();
      //  PlanUtils.setBlockParent(FakeExit, TopRegion);

      //  // Set Predecessors
      //  if (Dom != CurrentBlock) {
      //    // Move only those predecessors from PostDom that are part of the
      //    // nested region (i.e. they are dominated by Dom)
      //    for (auto Pred : PostDom->getPredecessors()) {
      //      if (DomTree.dominates(Dom, Pred)) {
      //        PlanUtils.movePredecessor(Pred, PostDom /*From*/,
      //                                  FakeExit /*To*/);
      //      }
      //    }
      //  } else {
      //    // All the predecessors will be in the same region. Move them all
      //    from
      //    // PostDom to FakeExit
      //    PlanUtils.movePredecessors(PostDom, FakeExit);
      //  }

      //  // Add PostDom as single successor
      //  PlanUtils.setSuccessor(FakeExit, PostDom);

      //}
    }

    // Add successors to the worklist
    for (VPBlockBase *Succ : CurrentBlock->getSuccessors())
      WorkList.push_back(Succ);
  }
}

// Main function that canonicalizes the plain CFG and applyies transformations
// that enable the detection of more regions during the hierarchical CFG
// construction
void VPlanHCFGBuilder::simplifyPlainCFG(HCFGState &State) {

  IntelVPlanUtils &PlanUtils = State.PlanUtils;
  IntelVPlan *Plan = PlanUtils.getVPlan();
  assert(isa<VPRegionBlock>(Plan->getEntry()) &&
         "VPlan entry is not a VPRegionBlock");
  VPRegionBlock *TopRegion = cast<VPRegionBlock>(Plan->getEntry());
  VPLoopInfo *VPLInfo = Plan->getVPLoopInfo();

  assert((VPLInfo->getNumTopLevelLoops() == 1) &&
         "Expected only 1 top-level loop");
  VPLoop *TopLoop = *VPLInfo->begin();

  splitLoopsPreheader(TopLoop, State);

  if (LoopMassagingEnabled) {
    // DEBUG(dbgs() << "Dominator Tree Before mergeLoopExits\n";
    // State.VPDomTree.print(dbgs()));
    mergeLoopExits(TopLoop, State);
    PlanUtils.verifyHierarchicalCFG(TopRegion);
    // DEBUG(dbgs() << "Dominator Tree After mergeLoopExits\n";
    // State.VPDomTree.print(dbgs()));
  }

  if (VPlanLoopCFU) {
    // TODO: Move VPLoopCFU to this file (like mergeLoopExits) and use
    // HCFGState???
    VPLoopCFU LCFU(Plan, PlanUtils, SE, LI, VPLInfo, &State.VPDomTree,
                   &State.VPPostDomTree);
    LCFU.makeInnerLoopControlFlowUniform();
  }

  splitLoopsExit(TopLoop, State);
  simplifyNonLoopRegions(State);
}

// This is a temporal implementation to detect and discard non-loop regions
// whose entry and exit blocks are in different graph cycles. At this point, the
// only cycles we have to care about are those created by loop latches. This
// means that problematic potential non-loop regions will have entry and/or exit
// blocks immediately nested inside a VPLoopRegion (i.e., block's parent will be
// a VPLoopRegion). In order to detect such cases, we currently check whether
// the loop header is reachable starting from region's entry block up to
// region's exit block.
static bool regionIsBackEdgeCompliant(const VPBlockBase *Entry,
                                      const VPBlockBase *Exit,
                                      VPRegionBlock *ParentRegion) {

  // If the immediate parent region is not a loop region, current region won't
  // have any problem with loop cycles, so it's back edge compliant
  if (!isa<VPLoopRegion>(ParentRegion))
    return true;

  // Expensive check: check if loop header is inside the region
  VPLoop *ParentLoop = cast<VPLoopRegion>(ParentRegion)->getVPLoop();
  VPBlockBase *LoopHeader = ParentLoop->getHeader();
  assert(ParentLoop->getUniqueExitBlock() && "Only single-exit loops expected");
  assert(ParentLoop->contains(Entry) &&
         "Potential entry blocks should be inside the loop");
  assert(ParentLoop->contains(Exit) &&
         "Potential exit blocks should be inside the loop");

  SmallVector<const VPBlockBase *, 32> WorkList;
  SmallPtrSet<const VPBlockBase *, 32> Visited;
  WorkList.push_back(Entry);

  while (!WorkList.empty()) {
    const VPBlockBase *Current = WorkList.back();
    WorkList.pop_back();

    if (Visited.count(Current))
      continue;
    Visited.insert(Current);

    if (Current == LoopHeader)
      return false;

    // Add successors but skip Exit successors
    if (Current != Exit)
      for (auto Succ : Current->getSuccessors())
        WorkList.push_back(Succ);
  }

  return true;
}

// Create new LoopRegion's using VPLoopInfo analysis and introduce them into the
// hierarchical CFG. This function doesn't traverse the whole CFG and region's
// size and block's parent are not properly updated. They are updated in
// buildNonLoopRegions.
void VPlanHCFGBuilder::buildLoopRegions(HCFGState &State) {

  IntelVPlanUtils &PlanUtils = State.PlanUtils;
  VPLoopInfo *VPLInfo = PlanUtils.getVPlan()->getVPLoopInfo();

  // Auxiliary function that implements the main functionality of
  // buildLoopRegions
  std::function<void(VPLoop *)> buildLoopRegionsImpl = [&](VPLoop *VPL) {

    // Create new loop region
    VPLoopRegion *VPLR = PlanUtils.createLoop(VPL);

    // Set VPLoop's entry and exit.
    // Entry = loop preheader, Exit = loop single exit
    VPBlockBase *RegionEntry = VPL->getLoopPreheader();
    assert(RegionEntry && isa<VPBasicBlock>(RegionEntry) &&
           "Unexpected loop preheader");
    assert(VPL->getUniqueExitBlock() && "Only single-exit loops expected");
    VPBasicBlock *RegionExit = cast<VPBasicBlock>(VPL->getUniqueExitBlock());

    DEBUG(dbgs() << "Creating new VPLoopRegion " << VPLR->getName() << "\n"
                 << "   Entry: " << RegionEntry->getName() << "\n"
                 << "   Exit: " << RegionExit->getName() << "\n");

    // Connect loop region to graph
    PlanUtils.insertRegion(VPLR, RegionEntry, RegionExit,
                           false /*recomputeSize*/);

    // Update VPLoopInfo. Add new VPLoopRegion to region entry's loop (loop PH)
    // which, as expected, is not contained in this VPLoopRegion's VPLoop.
    if (VPLoop *Loop = VPLInfo->getLoopFor(RegionEntry)) {
      Loop->addBasicBlockToLoop(VPLR, *VPLInfo);
    }

    // Recursively build loop regions inside this loop
    for (VPLoop *SubVPL : VPL->getSubLoops())
      buildLoopRegionsImpl(SubVPL);
  };

  DEBUG(dbgs() << "Building LoopRegion's\n");

  for (VPLoop *VPL : make_range(VPLInfo->begin(), VPLInfo->end()))
    buildLoopRegionsImpl(VPL);
}

static bool isNonLoopRegion(VPBlockBase *Entry, VPRegionBlock *ParentRegion,
                            VPDominatorTree &VPDomTree,
                            VPDominatorTree &VPPostDomTree,
                            VPBlockBase *&OutputExit) {

  // Region's entry must have multiple successors and must be a VPBasicBlock at
  // this point. Also skip ParentRegion's Entry to prevent infinite recursion
  if (Entry == ParentRegion->getEntry() || Entry->getNumPredecessors() != 1 ||
      Entry->getNumSuccessors() < 2 || !isa<VPBasicBlock>(Entry))
    return false;

  VPBlockBase *Exit = VPPostDomTree.getNode(Entry)->getIDom()->getBlock();
  // Region's exit must have a single successor
  if (Exit->getNumSuccessors() != 1 || !isa<VPBasicBlock>(Exit) ||
      // TODO: Temporal check to skip regions that share exit node with parent
      // region.
      ParentRegion->getExit() == Exit)
    return false;

  VPBlockBase *Dom = VPDomTree.getNode(Exit)->getIDom()->getBlock();
  if (Dom != Entry || !regionIsBackEdgeCompliant(Entry, Exit, ParentRegion))
    return false;

  OutputExit = Exit;
  return true;
}

// Create new non-loop VPRegionBlock's and update the information of all the
// blocks in the hierarchical CFG. The hierarchical CFG is stable and contains
// consisten information after this step.
void VPlanHCFGBuilder::buildNonLoopRegions(VPRegionBlock *ParentRegion,
                                           HCFGState &State) {

  IntelVPlanUtils &PlanUtils = State.PlanUtils;
  VPLoopInfo *VPLInfo = PlanUtils.getVPlan()->getVPLoopInfo();

  DEBUG(dbgs() << "Building Non-Loop Regions for " << ParentRegion->getName()
               << "\n"
               << "   Entry: " << ParentRegion->getEntry()->getName() << "\n"
               << "   Exit: " << ParentRegion->getExit()->getName() << "\n");

  SmallVector<VPBlockBase *, 16> WorkList;
  SmallPtrSet<VPBlockBase *, 16> Visited;
  WorkList.push_back(ParentRegion->getEntry());

  unsigned ParentRegionSize = 0;

  while (!WorkList.empty()) {

    // Get Current and skip it if visited.
    VPBlockBase *Current = WorkList.back();
    WorkList.pop_back();
    if (Visited.count(Current))
      continue;

    Visited.insert(Current);
    DEBUG(dbgs() << "Visiting " << Current->getName()
                 << "(Entry: " << Current->getEntryBasicBlock() << ")"
                 << "\n";);

    // If you hit this assert, the input CFG is very likely to be not compliant
    // either because it contains a loop that is not supported or because loops
    // are not in canonical form.
    assert((isa<VPLoopRegion>(Current) || isa<VPBasicBlock>(Current)) &&
           "Expected VPBasicBlock or VPLoopRegion");

    // Increase ParentRegion's size
    ++ParentRegionSize;

    // Pointer to a new subregion or existing VPLoopRegion subregion
    VPRegionBlock *SubRegion = dyn_cast<VPLoopRegion>(Current);
    VPBlockBase *RegionExit;

    // Non-loop VPRegion detection.
    if (!DisableNonLoopSubRegions && !SubRegion /* Skip VPLoopRegions */ &&
        isNonLoopRegion(Current, ParentRegion, State.VPDomTree,
                        State.VPPostDomTree, RegionExit /*output*/)) {

      // Create new region and connect it to graph
      SubRegion = PlanUtils.createRegion(false /*isReplicator*/);

      DEBUG(dbgs() << "Creating new VPRegion " << SubRegion->getName() << "\n"
                   << "   Entry: " << Current->getName() << "\n"
                   << "   Exit: " << RegionExit->getName() << "\n");
      assert(RegionExit && "RegionExit cannot be null");

      PlanUtils.insertRegion(SubRegion, Current /*Entry*/, RegionExit,
                             false /*recomputeSize*/);

      // Add new region to VPLoopInfo.
      if (VPLoop *Loop = VPLInfo->getLoopFor(SubRegion->getEntry())) {
        Loop->addBasicBlockToLoop(SubRegion, *VPLInfo);
      }
    }

    if (SubRegion) {
      // Set SubRegion's parent
      PlanUtils.setBlockParent(SubRegion, ParentRegion);

      // Add SubRegion's successors to worklist.
      for (auto Succ : SubRegion->getSuccessors()) {
        DEBUG(dbgs() << "Adding " << Succ->getName() << " to WorkList"
                     << "\n");
        WorkList.push_back(Succ);
      }

      // Recursively build non-regions inside subregion
      buildNonLoopRegions(SubRegion, State);

    } else {
      // Set Current's parent
      PlanUtils.setBlockParent(Current, ParentRegion);

      // No new region has been detected. Add Current's successors.
      for (auto Succ : Current->getSuccessors()) {
        DEBUG(dbgs() << "Adding " << Succ->getName() << " to WorkList"
                     << "\n");
        WorkList.push_back(Succ);
      }
    }
  }

  PlanUtils.setRegionSize(ParentRegion, ParentRegionSize);

  DEBUG(dbgs() << "End of HCFG build for " << ParentRegion->getName() << "\n");
}

void VPlanHCFGBuilder::buildHierarchicalCFG(Loop *TheLoop,
                                            const WRNVecLoopNode *WRLoop,
                                            IntelVPlan *Plan) {
  assert((!WRLoop || WRLoop->getLoop() == TheLoop) &&
         "Inconsistent Loop information");

  // State used only to build this HCFG. It will be destroyed at the end of the
  // building process.
  HCFGState State(TheLoop, WRLoop, Plan);
  IntelVPlanUtils &PlanUtils = State.PlanUtils;
  VPDominatorTree &VPDomTree = State.VPDomTree;
  VPDominatorTree &VPPostDomTree = State.VPPostDomTree;

  // Build Top Region enclosing the plain CFG
  VPRegionBlock *TopRegion = buildPlainCFG(State);

  // Set Top Region as VPlan Entry
  Plan->setEntry(TopRegion);
  DEBUG(VPlanPrinter PlanPrinter(dbgs(), *Plan);
        PlanPrinter.dump("HCFGBuilder: Plain CFG"));

  PlanUtils.verifyHierarchicalCFG(TopRegion);

  // Compute dom tree for the plain CFG for VPLInfo. We don't need post-dom tree
  // at this point.
  VPDomTree.recalculate(*TopRegion);
  DEBUG(dbgs() << "Dominator Tree After buildPlainCFG\n";
        VPDomTree.print(dbgs()));

  // TODO: If more efficient, we may want to "translate" LoopInfo to VPLoopInfo.
  // Compute VPLInfo and keep it in VPlan
  VPLoopInfo *VPLInfo = new VPLoopInfo();
  VPLInfo->analyze(VPDomTree);
  Plan->setVPLoopInfo(VPLInfo);
  DEBUG(dbgs() << "Loop Info:\n"; LI->print(dbgs()));
  DEBUG(dbgs() << "VPLoop Info After buildPlainCFG:\n"; VPLInfo->print(dbgs()));

  // Compute postdom tree for the plain CFG.
  VPPostDomTree.recalculate(*TopRegion);
  DEBUG(dbgs() << "PostDominator Tree After buildPlainCFG:\n";
        VPPostDomTree.print(dbgs()));

  // Prepare/simplify CFG for hierarchical CFG construction
  simplifyPlainCFG(State);

  DEBUG(VPlanPrinter PlanPrinter(dbgs(), *Plan);
        PlanPrinter.dump("LVP: After simplifyPlainCFG"));
  DEBUG(dbgs() << "Dominator Tree After simplifyPlainCFG\n";
        VPDomTree.print(dbgs()));
  DEBUG(dbgs() << "PostDominator Tree After simplifyPlainCFG:\n";
        VPPostDomTree.print(dbgs()));
  DEBUG(dbgs() << "VPLoop Info After simplifyPlainCFG:\n";
        VPLInfo->print(dbgs()));

  PlanUtils.verifyHierarchicalCFG(TopRegion);

  // Build hierarchical CFG in two step: buildLoopRegions and
  // buildNonLoopRegions. There are two important things to notice:
  //    1. Regions' size and blocks' parent are not consistent after
  //       buildLoopRegions. buildLoopRegions doesn't require to traverse the
  //       CFG. It's more effient to recompute this information while traversing
  //       the CFG in buildNonLoopRegions.
  //    2. Dom/Postdom trees for the plain CFG are no longer valid after
  //       buildLoopRegions (there is no plain CFG anymore). However, we can
  //       still use them to build non-loop regions.
  //
  buildLoopRegions(State);
  buildNonLoopRegions(TopRegion, State);

  DEBUG(VPlanPrinter PlanPrinter(dbgs(), *Plan);
        PlanPrinter.dump("LVP: After building HCFG"));

  PlanUtils.verifyHierarchicalCFG(TopRegion, TheLoop, VPLInfo, LI);
}
