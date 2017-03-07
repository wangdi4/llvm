
#include "LoopVectorizationPlanner.h"
#include "llvm/Analysis/Intel_VPO/WRegionInfo/WRegionInfo.h"
#include "llvm/Analysis/LoopIterator.h"
#include "LoopVectorizationCodeGen.h"
#include "VPLoopInfo.h"

#define DEBUG_TYPE "LoopVectorizationPlanner"

  using namespace llvm;

  static cl::opt<bool> NonLoopSubRegionsEnabled(
      "vplan-enable-subregions",
      cl::init(false), // TODO: vplan-disable-subregions
      cl::desc("Enable construction of non-loop subregions in VPlan"));

  unsigned LoopVectorizationPlannerBase::buildInitialVPlans(unsigned MinVF,
                                                            unsigned MaxVF) {
    collectDeadInstructions();

    unsigned StartRangeVF = MinVF;
    unsigned EndRangeVF = MaxVF + 1;

    unsigned i = 0;
    for (; StartRangeVF < EndRangeVF; ++i) {
      std::shared_ptr<IntelVPlan> Plan =
          buildInitialVPlan(StartRangeVF, EndRangeVF);

      for (unsigned TmpVF = StartRangeVF; TmpVF < EndRangeVF; TmpVF *= 2)
        VPlans[TmpVF] = Plan;

      StartRangeVF = EndRangeVF;
      EndRangeVF = MaxVF + 1;
    }

    return i;
  }

  void LoopVectorizationPlannerBase::setBestPlan(unsigned VF, unsigned UF) {
    DEBUG(dbgs() << "Setting best plan to VF=" << VF << ", UF=" << UF << '\n');
    BestVF = VF;
    BestUF = UF;

    assert(VPlans.count(VF) && "Best VF does not have a VPlan.");
    // Delete all other VPlans.
    for (auto &Entry : VPlans) {
      if (Entry.first != VF)
        VPlans.erase(Entry.first);
    }
  }

  void LoopVectorizationPlannerBase::printCurrentPlans(const std::string &Title,
                                                       raw_ostream &O) {
    auto printPlan = [&](IntelVPlan *Plan, const SmallVectorImpl<unsigned> &VFs,
                         const std::string &Prefix) {
      std::string Title;
      raw_string_ostream RSO(Title);
      RSO << Prefix << " for VF=";
      if (VFs.size() == 1)
        RSO << VFs[0];
      else {
        RSO << "{";
        bool First = true;
        for (unsigned VF : VFs) {
          if (!First)
            RSO << ",";
          RSO << VF;
          First = false;
        }
        RSO << "}";
      }
      VPlanPrinter PlanPrinter(O, *Plan);
      PlanPrinter.dump(RSO.str());
    };

    if (VPlans.empty())
      return;

    IntelVPlan *Current = VPlans.begin()->second.get();

    SmallVector<unsigned, 4> VFs;
    for (auto &Entry : VPlans) {
      IntelVPlan *Plan = Entry.second.get();
      if (Plan != Current) {
        // Hit another VPlan. Print the current VPlan for the VFs it served thus
        // far and move on to the VPlan we just encountered.
        printPlan(Current, VFs, Title);
        Current = Plan;
        VFs.clear();
      }
      // Add VF to the list of VFs served by current VPlan.
      VFs.push_back(Entry.first);
    }
    // Print the current VPlan.
    printPlan(Current, VFs, Title);
  }

  // Build top region + inner VPBBs 
  VPRegionBlock *LoopVectorizationPlanner::buildPlainCFG(
      IntelVPlanUtils &PlanUtils, DenseMap<BasicBlock *, VPBasicBlock *> &BB2VPBB,
      DenseMap<VPBasicBlock *, BasicBlock *> &VPBB2BB) {

    DenseMap<Value *, VPConditionBitRecipeBase *> BranchCondMap;

    auto isInstructionToIgnore = [&](Instruction *I) -> bool {
      return DeadInstructions.count(I) || isa<BranchInst>(I) ||
             isa<DbgInfoIntrinsic>(I);
    };

    auto isConditionForUniformBranch = [&](Instruction *I) -> bool {
      auto isBranchInst = [&](User *U) -> bool {
        return isa<BranchInst>(U) && TheLoop->contains(cast<Instruction>(U));
      };
      return TheLoop->contains(I) && TheLoop->isLoopInvariant(I) &&
             any_of(I->users(), isBranchInst);
    };

    auto createRecipesForVPBB = [&](BasicBlock *BB, VPBasicBlock *VPBB) {
      BasicBlock::iterator I = BB->begin();
      BasicBlock::iterator E = BB->end();

      while (I != E) {
        for (; I != E && isInstructionToIgnore(&*I); ++I)
          ;
        if (I == E)
          break;

        BasicBlock::iterator J = I;
        for (++J; J != E; ++J) {
          Instruction *Instr = &*J;
          if (isInstructionToIgnore(Instr) || isConditionForUniformBranch(Instr))
            break; // Sequence of instructions not to ignore ended.
        }

        if (isConditionForUniformBranch(&*I)) {
          Instruction *Instr = &*I;
          VPUniformConditionBitRecipe *Recipe =
            PlanUtils.createUniformConditionBitRecipe(Instr);
          PlanUtils.appendRecipeToBasicBlock(Recipe, VPBB);
          for(User *U : Instr->users()) {
            if (isa<BranchInst>(U) && TheLoop->contains(cast<Instruction>(U)))
              BranchCondMap[cast<BranchInst>(U)->getCondition()] = Recipe;
          }
        }

        // Create new Recipe and add it to VPBB
        bool Scalarized = false;
        VPRecipeBase *Recipe = PlanUtils.createOneByOneRecipe(I, J, Scalarized);
        PlanUtils.appendRecipeToBasicBlock(Recipe, VPBB);

        I = J;
      }
    };

    // TODO: If we detect backedges, we should be able to use RPOT without
    // BB2VPBB/VPBB2BB maps
    auto createOrGetVPBB = [&](BasicBlock *BB, VPRegionBlock *TopRegion,
                               unsigned &TopRegionSize) -> VPBasicBlock * {
      auto BlockIt = BB2VPBB.find(BB);

      VPBasicBlock *VPBB;
      if (BlockIt == BB2VPBB.end()) {
        // New VPBB
        VPBB = PlanUtils.createBasicBlock();
        createRecipesForVPBB(BB, VPBB);
        BB2VPBB[BB] = VPBB;
        VPBB2BB[VPBB] = BB;
        PlanUtils.setBlockParent(VPBB, TopRegion);
        ++TopRegionSize;
      } else {
        VPBB = BlockIt->second;
      }

      return VPBB;
    };

    // Create top VPRegion. It will be parent of all VPBBs
    VPRegionBlock *TopRegion = PlanUtils.createRegion(false /*isReplicator*/);
    unsigned TopRegionSize = 0;

    // Scan the body of the loop in a topological order to visit each basic block
    // after having visited its predecessor basic blocks.
    // TODO: Remove LI
    LoopBlocksDFS DFS(TheLoop);
    DFS.perform(LI);
    // ReversePostOrderTraversal<BasicBlock *> RPOT(TheLoop->getLoopPreheader());


    for (BasicBlock *BB : make_range(DFS.beginRPO(), DFS.endRPO())) {
      // for (BasicBlock *BB : make_range(RPOT.begin(), RPOT.end())) {

      // Create new VPBasicBlock and its recipes
      VPBasicBlock *VPBB = createOrGetVPBB(BB, TopRegion, TopRegionSize);

      // Add successors and predecessors
      // TODO: Old bug in VPOPredicator. Split setSuccessor into setSuccessor and
      // setPrecedessor.
      TerminatorInst *TI = BB->getTerminator();
      assert(TI && "Terminator expected");
      unsigned NumSuccs = TI->getNumSuccessors();

      if (NumSuccs == 1) {
        VPBasicBlock *SuccVPBB =
            createOrGetVPBB(TI->getSuccessor(0), TopRegion, TopRegionSize);
        assert(SuccVPBB && "VPBB Successor not found");

        PlanUtils.setSuccessor(VPBB, SuccVPBB);
      } else if (NumSuccs == 2) {
        VPBasicBlock *SuccVPBB0 =
            createOrGetVPBB(TI->getSuccessor(0), TopRegion, TopRegionSize);
        assert(SuccVPBB0 && "Successor 0 not found");
        VPBasicBlock *SuccVPBB1 =
            createOrGetVPBB(TI->getSuccessor(1), TopRegion, TopRegionSize);
        assert(SuccVPBB1 && "Successor 1 not found");

        BranchInst *Br = cast<BranchInst>(TI);
        Value *Condition = Br->getCondition();
        VPConditionBitRecipeBase *CondBitR = nullptr;
        if (TheLoop->isLoopInvariant(Condition)) {
          if (BranchCondMap.count(Condition))
            CondBitR = BranchCondMap[Condition];
          else
            CondBitR = PlanUtils.createLiveInConditionBitRecipe(Condition);
        }
   
        PlanUtils.setTwoSuccessors(VPBB, CondBitR, SuccVPBB0, SuccVPBB1);
      } else {
        llvm_unreachable("Number of successors not supported");
      }
    }

    // Outermost loop preheader is not successor of any block inside the loop. It
    // needs explicit treatment. Add outermost loop preheader to CFG
    assert(
        (TheLoop->getLoopPreheader()->getTerminator()->getNumSuccessors() == 1) &&
        "Unexpected loop preheader");
    VPBlockBase *Preheader =
        createOrGetVPBB(TheLoop->getLoopPreheader(), TopRegion, TopRegionSize);
    VPBlockBase *Header = BB2VPBB[TheLoop->getHeader()];
    PlanUtils.setSuccessor(Preheader, Header);

    // Top region setup

    // Create a fake entry block as top region's entry
    VPBlockBase *RegionEntry = PlanUtils.createBasicBlock();
    ++TopRegionSize;
    PlanUtils.setBlockParent(RegionEntry, TopRegion);
    PlanUtils.setSuccessor(RegionEntry, Preheader);

    // Create a fake block as top region's exit
    VPBlockBase *RegionExit = PlanUtils.createBasicBlock();
    ++TopRegionSize;
    PlanUtils.setBlockParent(RegionExit, TopRegion);

    // Connect fake top region's exit.
    if (BasicBlock *SingleExitBlock = TheLoop->getUniqueExitBlock()) {
      PlanUtils.setSuccessor(BB2VPBB[SingleExitBlock], RegionExit);
    } else {
      // If there are multiple exits in the outermost loop, we need another fake
      // block as single exit (landing pad)
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

      // Connect landing pad block to top region's exit
      PlanUtils.setSuccessor(LandingPad, RegionExit);
    }

    PlanUtils.setRegionEntry(TopRegion, RegionEntry);
    PlanUtils.setRegionExit(TopRegion, RegionExit);
    PlanUtils.setRegionSize(TopRegion, TopRegionSize);

    return TopRegion;
  }

  // It turns A->B into A->NewSucc->B and updates VPLoopInfo, DomTree and
  // PostDomTree accordingly. 'A' must have a single successor. 'A' must be IDom
  // of 'B'. NewSucc will be added to A's loop.
  void LoopVectorizationPlanner::splitSingleSuccessorBlock(
      VPBlockBase *Block, VPLoopInfo *VPLInfo, VPDominatorTree &DomTree,
      VPDominatorTree &PostDomTree, IntelVPlanUtils &PlanUtils) {

    VPBlockBase *OldSucc = Block->getSingleSuccessor();
    assert(Block->getSingleSuccessor() && "Expected single successor");
    assert((DomTree.getNode(OldSucc)->getIDom()->getBlock() == Block) &&
           "Expected Block to be IDom of old successor");

    // Create new successor
    VPBasicBlock *NewSucc = PlanUtils.createBasicBlock();
    PlanUtils.insertBlockAfter(NewSucc, Block);

    // Add new successor to VPLoopInfo
    if (VPLoop *Loop = VPLInfo->getLoopFor(Block)) {
      Loop->addBasicBlockToLoop(NewSucc, *VPLInfo);
    }

    // Update dom/postdom information

    // Block is idom of new succ
    VPDomTreeNode *NewSuccDT = DomTree.addNewBlock(NewSucc, Block /*IDom*/);

    // New successor is idom of old succ
    VPDomTreeNode *OldSuccDT = DomTree.getNode(OldSucc);
    assert(OldSuccDT && "Expected DomTreeNode for old successor");
    DomTree.changeImmediateDominator(OldSuccDT, NewSuccDT);

    // Block's iposdom is new successor's ipostdom
    VPDomTreeNode *BlockPDT = PostDomTree.getNode(Block);
    assert(BlockPDT && "Expected DomTreeNode for Block");
    VPDomTreeNode *NewSuccPDT = PostDomTree.addNewBlock(
        NewSucc, BlockPDT->getIDom()->getBlock() /*IDom*/);

    // New successor is ipostdom of block
    PostDomTree.changeImmediateDominator(BlockPDT, NewSuccPDT);
  }

  // It turns A->B into A->NewSucc->B and updates VPLoopInfo, DomTree and
  // PostDomTree accordingly.
  VPBasicBlock *LoopVectorizationPlanner::splitBlock(VPBlockBase *Block,
                                                     VPLoopInfo *VPLInfo,
                                                     VPDominatorTree &DomTree,
                                                     VPDominatorTree &PostDomTree,
                                                     IntelVPlanUtils &PlanUtils) {
    // TODO: If Exit has multiple successor, move ConditionBitRecipe when it's
    // available

    VPBasicBlock *NewBlock = PlanUtils.createBasicBlock();
    PlanUtils.insertBlockAfter(NewBlock, Block);

    // Add NewBlock to VPLoopInfo
    if (VPLoop *Loop = VPLInfo->getLoopFor(Block)) {
      Loop->addBasicBlockToLoop(NewBlock, *VPLInfo);
    }

    // Update dom information
  NewBlock->setConditionBitRecipe(Block->getConditionBitRecipe());

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
      auto& Successors = NewBlock->getSuccessors();
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

  void LoopVectorizationPlanner::splitLoopsPreheader(VPLoop *VPL,
                                                     VPLoopInfo *VPLInfo,
                                                     VPDominatorTree &DomTree,
                                                     VPDominatorTree &PostDomTree,
                                                     IntelVPlanUtils &PlanUtils) {

    // TODO: So far, I haven't found a test case that hits one of these asserts.
    // The code commented out below should cover the second one.

    // TODO: Temporal assert to detect problematic cases
    unsigned NumExternalPreds = 0;
    for (const VPBlockBase *Pred : VPL->getHeader()->getPredecessors()) {
      if (!VPL->contains(Pred))
        ++NumExternalPreds;
    }
    assert((NumExternalPreds == 1) &&
           "Loop header's external predecessor is not 1");

    // TODO: Temporal assert to detect problematic cases
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
    //    TODO: too generic. Refine?
    //    - has multiple predecessors (it's a potential exit of another region)
    //    - is loop H of another loop
    if (!PH->getSinglePredecessor() || VPLInfo->isLoopHeader(PH)) {
      splitBlock(PH, VPLInfo, DomTree, PostDomTree, PlanUtils);
    }

    // Apply simplification to subloops
    for (auto VPSL : VPL->getSubLoops()) {
      splitLoopsPreheader(VPSL, VPLInfo, DomTree, PostDomTree, PlanUtils);
    }
  }

  void LoopVectorizationPlanner::splitLoopsExits(VPLoop *VPL, VPLoopInfo *VPLInfo,
                                                 VPDominatorTree &DomTree,
                                                 VPDominatorTree &PostDomTree,
                                                 IntelVPlanUtils &PlanUtils) {

    SmallVector<VPBlockBase *, 2> ExitBlocks;
    VPL->getUniqueExitBlocks(ExitBlocks);

    for (VPBlockBase *Exit : ExitBlocks) {

      // Split loop exit with multiple successors or that is preheader of another
      // loop
      VPBlockBase *PotentialH = Exit->getSingleSuccessor();
      if (!PotentialH ||
          (VPLInfo->isLoopHeader(PotentialH) &&
           VPLInfo->getLoopFor(PotentialH)->getLoopPreheader() == Exit)) {

        splitBlock(Exit, VPLInfo, DomTree, PostDomTree, PlanUtils);
      }
    }

    // Apply simplification to subloops
    for (auto VPSL : VPL->getSubLoops()) {
      splitLoopsExits(VPSL, VPLInfo, DomTree, PostDomTree, PlanUtils);
    }
  }

  void LoopVectorizationPlanner::simplifyNonLoopRegions(
      VPRegionBlock *TopRegion, VPLoopInfo *VPLInfo, VPDominatorTree &DomTree,
      VPDominatorTree &PostDomTree, IntelVPlanUtils &PlanUtils) {

    // TODO: WIP. It will enable the build of VPRegions that
    // currently are not built because they share entry/exit nodes
    // with other VPRegions.
    
    // TODO: revisit data structure for WorkList
    std::list<VPBlockBase *> WorkList;
    SmallPtrSet<VPBlockBase *, 2> Visited;

    WorkList.push_back(TopRegion->getEntry());

    while (!WorkList.empty()) {

      // Get CurrentVPBB and and skip it if visited.
      VPBlockBase *CurrentBlock = WorkList.back();
      WorkList.pop_back();
      if (Visited.count(CurrentBlock))
        continue;

      // Set CurrentVPBB to visited
      Visited.insert(CurrentBlock);

      // Potential VPRegion entry
      if (CurrentBlock->getNumSuccessors() > 1) {

        // Currently, this rule covers:
        //   - Loop H with multiple successors
        //   - Region exit that is another region entry
        //   - Loop latch+exiting block with multiple successors
        //
        // TODO: skip single basic block loops?
        if (CurrentBlock->getNumPredecessors() > 1) {
          splitBlock(CurrentBlock, VPLInfo, DomTree, PostDomTree, PlanUtils);
        }

        //VPBlockBase *PostDom =
        //    PostDomTree.getNode(CurrentBlock)->getIDom()->getBlock();
        //VPBlockBase *Dom = DomTree.getNode(PostDom)->getIDom()->getBlock();
        //assert(isa<VPBasicBlock>(PostDom) &&
        //       "Expected VPBasicBlock as post-dominator");
        //assert(isa<VPBasicBlock>(Dom) && "Expected VPBasicBlock as dominator");

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
        //if (Dom != CurrentBlock || PostDom->getNumSuccessors() > 1) {

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
        //    // All the predecessors will be in the same region. Move them all from
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

  void LoopVectorizationPlanner::simplifyPlainCFG(VPRegionBlock *TopRegion,
                                                  VPLoopInfo *VPLInfo,
                                                  VPDominatorTree &DomTree,
                                                  VPDominatorTree &PostDomTree,
                                                  IntelVPlanUtils &PlanUtils) {
    // VPLoop simplifications
    assert((VPLInfo->getNumTopLevelLoops() == 1) &&
           "Expected only 1 top-level loop");
    VPLoop *TopLoop = *VPLInfo->begin();

    splitLoopsPreheader(TopLoop, VPLInfo, DomTree, PostDomTree, PlanUtils);

    // TODO: SEME-to-SESE loop massaging should happen here.

    splitLoopsExits(TopLoop, VPLInfo, DomTree, PostDomTree, PlanUtils);

    // Non-loop region simplifications
    simplifyNonLoopRegions(TopRegion, VPLInfo, DomTree, PostDomTree, PlanUtils);
  }

  // TODO: Split into multiple functions
  // TODO: consts
  // TODO: Entry is not actually a region entry
  void LoopVectorizationPlanner::buildHierarchicalCFG(
      VPBasicBlock *Entry, VPRegionBlock *ParentRegion, VPLoopInfo *VPLInfo,
      VPDominatorTree &DomTree, VPDominatorTree &PostDomTree,
      IntelVPlanUtils &PlanUtils, DenseMap<BasicBlock *, VPBasicBlock *> &BB2VPBB,
      DenseMap<VPBasicBlock *, BasicBlock *> &VPBB2BB) {

    DEBUG(dbgs() << "Building HCFG for " << ParentRegion->getName() << "\n"
          << "   Entry: " << ParentRegion->getEntry()->getName() << "\n"
          << "   Exit: " << ParentRegion->getExit()->getName() << "\n");

    // TODO: revisit data structure for WorkList
    std::list<VPBlockBase *> WorkList;
    SmallPtrSet<VPBlockBase *, 2> Visited;

    WorkList.push_back(Entry);

    unsigned ParentRegionSize = 0;

    while (!WorkList.empty()) {

      // Get CurrentVPBB and and skip it if visited.
      VPBlockBase *Current = WorkList.back();
      WorkList.pop_back();
      if (Visited.count(Current))
        continue;

      DEBUG(dbgs() << "Visiting " << Current->getName()
                   << "(Entry: " << Current->getEntryBasicBlock() << ")"
                   << "\n";);

      // If you hit this assert, the input CFG is very likely to be not compliant
      // either because it contains SEME loops or because loops don't have the
      // right form (bottom test).
      assert(isa<VPBasicBlock>(Current) && "Expected VPBasicBlock");

      // Only VPBasicBlock will go through this point as the initial CFG only
      // contains VPBasicBlocks
      VPBasicBlock *CurrentVPBB = cast<VPBasicBlock>(Current);

      // Increase ParentRegion's size
      ++ParentRegionSize;

      // Set CurrentVPBB to visited
      Visited.insert(CurrentVPBB);

      // Pointer to future subregion, if created
      VPRegionBlock *NewRegion = nullptr;

      // Loop detection
      // TODO: Think about detecting loop from header and related issues
      // detecting non-loop regions.
      if (VPLoop *VPL = VPLInfo->getLoopFromPreHeader(CurrentVPBB)) {

        // Create new loop region
        VPLoopRegion *VPLR = PlanUtils.createLoop(VPL);
        NewRegion = VPLR;

        // TODO
        VPBasicBlock *VPLHeader = cast<VPBasicBlock>(VPL->getHeader());

        // At this point, VPLoop regions only contain information coming from
        // LLVM Loop and VPLoop massaging. Region's entry/exit/parent, etc.
        // haven't been set yet. We have to do so.

        // TODO: FIX SIZE (preheader)!

        // Set loop latch
        // TODO: Provide new loop's latch/exit after massaging for SEME loops.
        // This information will be no longer valid after massaging.
        // assert(Lp->getLoopLatch() && "Multiple latches are not supported yet");
        // PlanUtils.setLoopLatch(NewLoop, BB2VPBB[Lp->getLoopLatch()]);

        // Set VPLoop's entry and exit.
        // VPLoop's entry = loop preheader.
        VPBasicBlock *RegionEntry = CurrentVPBB;

        // If loop has single exit, region's exit is that exit block
        // TODO: cast
        VPBasicBlock *RegionExit = cast<VPBasicBlock>(VPL->getUniqueExitBlock());
        // If multiple exits, region's exit is IDOM(loop header)
        if (!RegionExit) {
          VPBlockBase *IDomBlock =
              PostDomTree.getNode(VPLHeader)->getIDom()->getBlock();
          assert(isa<VPBasicBlock>(IDomBlock) && "IDom must be a VPBasicBlock");

          RegionExit = cast<VPBasicBlock>(IDomBlock);
        }

        // Connect VPLoop to graph
        PlanUtils.insertRegion(VPLR, RegionEntry, RegionExit);
        DEBUG(dbgs() << "Creating VPLoopRegion " << VPLR->getName() << "\n"
                     << "   Entry: " << RegionEntry->getName() << "\n"
                     << "   Exit: " << RegionExit->getName() << "\n");

        // Recursively build subregions inside VPLoop from loop header
        buildHierarchicalCFG(VPLHeader, VPLR, VPLInfo, DomTree, PostDomTree,
                             PlanUtils, BB2VPBB, VPBB2BB);
      }
      // Non-loop VPRegion detection
      else if (NonLoopSubRegionsEnabled &&
               CurrentVPBB->getNumSuccessors() > 1 &&
               // Skip ParentRegion Entry to prevent infinite recursion
               CurrentVPBB != ParentRegion->getEntry()) {

        VPBlockBase *PostDom =
            PostDomTree.getNode(CurrentVPBB)->getIDom()->getBlock();

        // PostDom will be region's exit and it must have a single successor
        if (PostDom->getNumSuccessors() == 1) {

          VPBlockBase *Dom = DomTree.getNode(PostDom)->getIDom()->getBlock();
          assert(isa<VPBasicBlock>(PostDom) &&
                 "Expected VPBasicBlock as post-dominator");
          assert(isa<VPBasicBlock>(Dom) && "Expected VPBasicBlock as dominator");

          // New VPRegion entry found.
          if (Dom == CurrentVPBB &&
              // TODO: Temporal check to skip regions that share exit node
              // with parent region.
              PostDom->getParent()->getExit() != PostDom &&
              // Skip potential entry block if it has outgoing back-edges
              !PlanUtils.blockIsLoopLatch(Dom, VPLInfo)) {

            // Create new region
            NewRegion = PlanUtils.createRegion(false /*isReplicator*/);

            // Connect new subregion to graph
            PlanUtils.insertRegion(NewRegion, Dom /*Entry*/, PostDom /*Exit*/);
            DEBUG(dbgs() << "Creating VPRegion " << NewRegion->getName() << "\n"
                         << "   Entry: " << Dom->getName() << "\n"
                         << "   Exit: " << PostDom->getName() << "\n");

            // Recursively build subregions inside current region.
            assert(isa<VPBasicBlock>(Dom) &&
                   "Expected VPBasicBlock as successor");

            // TODO: FIX SIZE

            buildHierarchicalCFG(cast<VPBasicBlock>(Dom), NewRegion, VPLInfo,
                                 DomTree, PostDomTree, PlanUtils, BB2VPBB,
                                 VPBB2BB);
          }

          // TODO: From VPOPredicator
          // For now, it is assumed we're dealing exclusively with innermost
          // loop
          // vectorization. Mark the SESE region as divergent if the condition
          // of
          // the
          // branch is non-uniform with respect to this loop.
          // Value *Cmp = VBlock->getBranchCondition();
          // const SCEV *CmpSCEV = SE->getSCEV(Cmp);
          // if (!SE->isLoopInvariant(CmpSCEV, ALoop->getLoop())) {
          //  RegionStack.top()->setDivergent();
          //}
          // TODO: We make all regions divergent by now.
          // RegionStack.top()->setDivergent();
      }
    }

    if (NewRegion) {
      // Set NewRegion's parent
      PlanUtils.setBlockParent(NewRegion, ParentRegion);

      // Add NewRegion to visited, just in case it has multiple predecessors
      Visited.insert(NewRegion);

      // TODO
      // Update VPLoopInfo. Add new region to entry's loop
      // if (VPLoop *Loop = VPLInfo->getLoopFor(NewRegion->getEntry())) {
      //    Loop->addBasicBlockToLoop(NewRegion, *VPLInfo);
      //}

      // New region has been created. Add NewRegion's successors. NewRegion
      // subgraph has already been visited.
      for (auto Succ : NewRegion->getSuccessors()) {
        WorkList.push_back(Succ);
      }
    } else {
      // Set CurrentVPBB's parent
      PlanUtils.setBlockParent(CurrentVPBB, ParentRegion);

      // No new region has been created. Add CurrentVPBB's successors.
      for (auto Succ : CurrentVPBB->getSuccessors()) {
        WorkList.push_back(Succ);
      }
    }
  }

  PlanUtils.setRegionSize(ParentRegion, ParentRegionSize);

  DEBUG(dbgs() << "End of HCFG build for " << ParentRegion->getName() << "\n");
}

void LoopVectorizationPlanner::verifyHierarchicalCFG(
    const VPRegionBlock *TopRegion, const VPLoopInfo *VPLInfo) const {

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

  std::function<unsigned(const VPLoop *)> countLoopsInVPLoop =
      [&](const VPLoop *Loop) -> unsigned {

    const std::vector<VPLoop *> &SubLoops = Loop->getSubLoops();
    unsigned NumLoops = SubLoops.size();

    for (const VPLoop *VPSL : SubLoops)
      NumLoops += countLoopsInVPLoop(VPSL);

    return NumLoops;
  };

  // Compare number of loops in CFG with loops in VPLoopInfo
  unsigned NumLoopsInCFG = countLoopsInRegion(TopRegion);

  unsigned NumLoopsInLoopInfo = 0;
  for (VPLoop *TopLoop : make_range(VPLInfo->begin(), VPLInfo->end())) {
    NumLoopsInLoopInfo += 1 /*TopLoop*/ + countLoopsInVPLoop(TopLoop);
  }

  DEBUG(dbgs() << "Verify Hierarchical VPlan:\n";
        dbgs() << "  NumLoopsInCFG: " << NumLoopsInCFG << "\n";
        dbgs() << "  NumLoopsInLoopInfo: " << NumLoopsInLoopInfo << "\n";);

  assert((NumLoopsInCFG == NumLoopsInLoopInfo) &&
         "Number of loops in CFG doesn't match number of loops in VPLoopInfo");
}

// It builds a VPlan with the initial Hierarchical CFG (HCFG) from the input IR.
// The resulting HCFG won't have a one-to-one correspondence with the input CFG.
// The algorithm applies the following steps:
//
//// 1. buildPlainCFG: builds a plain CFG from the input IR. The plain CFG only
// contains VPBasicBlock's with VPOneByOneRecipe's (only
// VPVectorizeOneByOneRecipe's by now). A VPRegionBlock encloses all the
// VPBasicBlock's of the plain CFG (topmost VPRegionBlock). Two dummy
// VPBasicBlock's are used as topmost region's Entry
// and Exit.
//
// WIP/TODOs:
//     - Add VPScalarizeOneByOneRecipe's
//     - Add VPConditionalBitRecipe's (Matt)
//     - Temporal implementation: if incoming outermost loop has multiple exits,
//     a dummy VPBasicBlock is created as landing pad for all loop exits.
//
// 2. simplifyPlanCFG: applies transformations to plain CFG to make it suitable
// for construction of VPRegionBlock's in step 3. These transformations include:
//     - Loop preheader massaging
//     - Loop exits massaging
//     - SEME-to-SESE loop massaging (TODO, Satish)
//     - Non-loop region Entry and Exit massaging
//
// VPBlockBase-based DT/PDT trees and VPLoopInfo must be consistent after this
// step.
//
// WIP: Most of these massages are currently under development. This step is
// expected to change significantly.
//
// 3. buildHierarchicalCFG: builds VPLoop's and non-loop VPRegionBlock's using
// an outer-to-inner approach. The result of this step is a HCFG.
//
// WIP/TODOs:
//     - VPRegionBlock size is not computed appropriately.
//     - Add VPRegionBlock's to VPLoopInfo
//     - Revisit VPLoop detection
//     - VPBlockBase-based DT/PDT cannot be reused after this step. We need a
//       specific DT/PDT per regionregion of HCFG

std::shared_ptr<IntelVPlan>
LoopVectorizationPlanner::buildInitialVPlan(unsigned StartRangeVF,
                                            unsigned &EndRangeVF) {
  // TODO: StartRangeVF and EndRangeVF are not being used by now

  // VPlan and VPlanUtils for this VPlan
  std::shared_ptr<IntelVPlan> SharedPlan = std::make_shared<IntelVPlan>();
  IntelVPlan *Plan = SharedPlan.get();
  IntelVPlanUtils PlanUtils(Plan);

  // BasicBlock to VPBasicBlock map for this VPlan
  // TODO: So far, we are keeping these maps to be able to use LoopInfo after
  // buildPlainCFG. If we eventually copy and maintain the LoopInfo information
  // that we need in a separate data structure, we should be able to get rid of
  // these maps.
  DenseMap<BasicBlock *, VPBasicBlock *> BB2VPBB;
  DenseMap<VPBasicBlock *, BasicBlock *> VPBB2BB;

  // Build top VPRegion and initial VPBasicBlock-based CFG
  VPRegionBlock *TopRegion = buildPlainCFG(PlanUtils, BB2VPBB, VPBB2BB);

  // Set TopRegion as VPlan Entry
  Plan->setEntry(TopRegion);
  DEBUG(VPlanPrinter PlanPrinter(dbgs(), *Plan);
        PlanPrinter.dump("LVP: Plain CFG for VF=4"));

  // Compute dom tree for VPLInfo. We don't need post-dom tree.
  VPDominatorTree DomTree(false /* DominatorTree */);
  DomTree.recalculate(*TopRegion);
  DEBUG(dbgs() << "Dominator Tree After buildPlainCFG\n";
        DomTree.print(dbgs()));

  // TODO: If more efficient, we may want to "translate" LoopInfo to VPLoopInfo.
  // TODO: VPLInfo needs to be alive as long as VPlan. We could move theí 'new'
  // to VPlan constructor.
  VPLoopInfo *VPLInfo = new VPLoopInfo();
  Plan->setVPLoopInfo(VPLInfo);
  VPLInfo->analyze(DomTree);
  DEBUG(dbgs() << "VPLoop Info:\n"; VPLInfo->print(dbgs()));

  // Compute postdom tree.
  VPDominatorTree PostDomTree(true /* Post-Dominator Tree */);
  PostDomTree.recalculate(*TopRegion);
  DEBUG(dbgs() << "PostDominator Tree After buildPlainCFG:\n";
        PostDomTree.print(dbgs()));

  // TODO: simplifyPlainCFG is partially implemented. With the current
  // implementation we won't detect all non-loop SESE regions but this shouldn't
  // be critical. Algorithms should be able to work even if we only create loop
  // regions.
  simplifyPlainCFG(TopRegion, VPLInfo, DomTree, PostDomTree, PlanUtils);

  DEBUG(VPlanPrinter PlanPrinter(dbgs(), *Plan);
        PlanPrinter.dump("LVP: After simplifyPlainCFG for VF=4"));

  // Build sub-regions, including VPLoops. At this point, only SESE regions are
  // expected. The algorithm will fail otherwise
  buildHierarchicalCFG(cast<VPBasicBlock>(TopRegion->getEntry()), TopRegion,
                       VPLInfo, DomTree, PostDomTree, PlanUtils, BB2VPBB,
                       VPBB2BB);

  DEBUG(VPlanPrinter PlanPrinter(dbgs(), *Plan);
        PlanPrinter.dump("LVP: After buildHierarchicalCFG for VF=4"));

  verifyHierarchicalCFG(TopRegion, VPLInfo);

  // TODO: CFG massaging for inner loops in outer loop vectorization scenarios
  // might happen here.

  // FOR STRESS TESTING, uncomment the following:
  // EndRangeVF = StartRangeVF * 2;

  return SharedPlan;
}

void LoopVectorizationPlanner::executeBestPlan(VPOCodeGen &LB) {
  ILV = &LB;

  // Perform the actual loop widening (vectorization).
  // 1. Create a new empty loop. Unlink the old loop and connect the new one.
  ILV->createEmptyLoop();

  // 2. Widen each instruction in the old loop to a new one in the new loop.

  VPTransformState State{BestVF, BestUF, LI, DT, ILV->getBuilder(), ILV, Legal};
  State.CFG.PrevBB = ILV->getLoopVectorPH();

  VPlan *Plan = getVPlanForVF(BestVF);

  ILV->collectUniformsAndScalars(BestVF);

  Plan->vectorize(&State);

  // 3. Take care of phi's to fix: reduction, 1st-order-recurrence, loop-closed.
  ILV->finalizeLoop();
}

void LoopVectorizationPlanner::collectDeadInstructions() {
  VPOCodeGen::collectTriviallyDeadInstructions(TheLoop, Legal,
                                               DeadInstructions);
}
