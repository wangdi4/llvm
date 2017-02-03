
#include "LoopVectorizationPlanner.h"
#include "llvm/Analysis/Intel_VPO/WRegionInfo/WRegionInfo.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/LoopIterator.h"

#define DEBUG_TYPE "vplan-loop-vec-planner" 

using namespace llvm;

unsigned LoopVectorizationPlannerBase::buildInitialVPlans(unsigned MinVF,
                                                          unsigned MaxVF) {
  // ILV->collectTriviallyDeadInstructions(TheLoop, Legal, DeadInstructions);

  unsigned StartRangeVF = MinVF;
  unsigned EndRangeVF = MaxVF + 1;

  unsigned i = 0;
  for (; StartRangeVF < EndRangeVF; ++i) {
    std::shared_ptr<VPlan> Plan = buildInitialVPlan(StartRangeVF, EndRangeVF);

    for (unsigned TmpVF = StartRangeVF; TmpVF < EndRangeVF; TmpVF *= 2)
      VPlans[TmpVF] = Plan;

    StartRangeVF = EndRangeVF;
    EndRangeVF = MaxVF + 1;
  }

  return i;
}

void LoopVectorizationPlannerBase::printCurrentPlans(const std::string &Title,
                                                     raw_ostream &O) {
  auto printPlan = [&](VPlan *Plan, const SmallVectorImpl<unsigned> &VFs,
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

  VPlan *Current = VPlans.begin()->second.get();

  SmallVector<unsigned, 4> VFs;
  for (auto &Entry : VPlans) {
    VPlan *Plan = Entry.second.get();
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

// Build top region + inner VPBBs with only one VectorOneByOneRecipe per VPBB
VPRegionBlock *LoopVectorizationPlanner::buildInitialCFG(
    IntelVPlanUtils &PlanUtils,
    DenseMap<BasicBlock *, VPBasicBlock *> &BB2VPBB,
    DenseMap<VPBasicBlock *, BasicBlock *> &VPBB2BB) {

  // Create top VPRegion. It will be parent of all VPBBs
  VPRegionBlock *TopRegion = PlanUtils.createRegion(false /*isReplicator*/);
  unsigned TopRegionSize = 0;

  auto createOrGetVPBB = [&](BasicBlock *BB) -> VPBasicBlock * {
    auto BlockIt = BB2VPBB.find(BB);

    VPBasicBlock *VPBB;
    if (BlockIt == BB2VPBB.end()) {
      VPBB = PlanUtils.createBasicBlock();
      BB2VPBB[BB] = VPBB;
      VPBB2BB[VPBB] = BB;
      PlanUtils.setBlockParent(VPBB, TopRegion);
      TopRegionSize++;
    } else {
      VPBB = BlockIt->second;
    }

    return VPBB;
  };

  // Scan the body of the loop in a topological order to visit each basic block
  // after having visited its predecessor basic blocks.
  LoopBlocksDFS DFS(TheLoop);
  DFS.perform(LI);

  for (BasicBlock *BB : make_range(DFS.beginRPO(), DFS.endRPO())) {
    BasicBlock::iterator I = BB->begin();
    BasicBlock::iterator E = BB->end();

    // New Recipe and VPBasicBlock
    bool Scalarized = false;
    VPRecipeBase *Recipe = PlanUtils.createOneByOneRecipe(I, E, Scalarized);
    VPBasicBlock *VPBB = createOrGetVPBB(BB);
    PlanUtils.appendRecipeToBasicBlock(Recipe, VPBB);

    // Add successors and predecessors
    // TODO: Old bug in VPOPredicator. Split setSuccessor into setSuccessor and
    // setPrecedessor. 
    TerminatorInst *TI = BB->getTerminator();
    assert(TI && "Terminator expected");
    unsigned NumSuccs = TI->getNumSuccessors();

    if (NumSuccs == 1) {
      VPBasicBlock *SuccVPBB = createOrGetVPBB(TI->getSuccessor(0));
      assert(SuccVPBB && "VPBB Successor not found");

      PlanUtils.setSuccessor(VPBB, SuccVPBB);
    } else if (NumSuccs == 2) {
      VPBasicBlock *SuccVPBB0 = createOrGetVPBB(TI->getSuccessor(0));
      assert(SuccVPBB0 && "Successor 0 not found");
      VPBasicBlock *SuccVPBB1 = createOrGetVPBB(TI->getSuccessor(1));
      assert(SuccVPBB1 && "Successor 1 not found");

      PlanUtils.setTwoSuccessors(VPBB, nullptr /*VPConditionBitRecipeBase*/,
                                 SuccVPBB0, SuccVPBB1);
    } else {
      llvm_unreachable("Number of successors not supported");
    }
  }

  VPBlockBase *VPLEntry = BB2VPBB[TheLoop->getHeader()];
  assert(TheLoop->getExitBlock() && "Multiple exits are not supported");
  VPBlockBase *VPLExit = BB2VPBB[TheLoop->getExitBlock()];

  // Top region setup

  // Create a dummy entry block for VPRegion as loop's header has predecessor
  // Use loop's exit as region's exit
  VPBlockBase *RegionEntry = PlanUtils.createBasicBlock();
  TopRegionSize++;
  PlanUtils.setBlockParent(RegionEntry, TopRegion);
  PlanUtils.setSuccessor(RegionEntry, VPLEntry);
  VPBlockBase *RegionExit = VPLExit;
  
  PlanUtils.setRegionEntry(TopRegion, RegionEntry);
  PlanUtils.setRegionExit(TopRegion, RegionExit);
  PlanUtils.setRegionSize(TopRegion, TopRegionSize);

  return TopRegion;
}

// TODO: Split into multiple functions
void LoopVectorizationPlanner::buildSubRegions(
    VPBasicBlock *Entry, VPRegionBlock *ParentRegion, VPDominatorTree &DomTree,
    VPDominatorTree &PostDomTree, IntelVPlanUtils &PlanUtils,
    DenseMap<BasicBlock *, VPBasicBlock *> &BB2VPBB,
    DenseMap<VPBasicBlock *, BasicBlock *> &VPBB2BB) {

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

    // If you hit this assert, the input CFG is very likely to be not compliant
    // either because it contains SEME loops or because loops don't have the
    // right form (bottom test).
    assert(isa<VPBasicBlock>(Current) && "Expected VPBasicBlock");

    // Only VPBasicBlock will go through this point as the initial CFG only
    // contains VPBasicBlocks
    VPBasicBlock *CurrentVPBB = cast<VPBasicBlock>(Current);

    // Increase ParentRegion's size
    ParentRegionSize++;

    // Set CurrentVPBB to visited
    Visited.insert(CurrentVPBB);

    // Get BB counterpart. If it doesn't exit, set it to nullptr
    BasicBlock *CurrentBB = nullptr;
    auto BBIt = VPBB2BB.find(CurrentVPBB);
    if (BBIt != VPBB2BB.end())
      CurrentBB = BBIt->second;

    // Pointer to future subregion, if created
    VPRegionBlock *NewRegion = nullptr;

    // Loop detection
    // Please, note that VPLoop's entry may be the entry block of a nested
    // non-loop region. We have to skip loop detection when visiting that entry
    // node the second time.
    // TODO: We may want to split loop's entry node or create a fake node.
    if (CurrentBB && LI->isLoopHeader(CurrentBB) &&
        CurrentVPBB != ParentRegion->getEntry()) {

      Loop *Lp = LI->getLoopFor(CurrentBB);
      // New VPLoop
      NewRegion = PlanUtils.createLoop();

      VPBasicBlock *RegionEntry = CurrentVPBB;
      // TODO: Provide new loop's latch/exit after massaging for SEME loops.
      // This information will be no longer valid after massaging.
      // TODO: So far, let's assume that loop's latch is the single VPBB exiting
      // the loop
      assert(Lp->getLoopLatch() && "Multiple latches are not supported");
      VPBasicBlock *RegionExit = BB2VPBB[Lp->getLoopLatch()];

      // Remove LoopLatch from RegionEntry's predecessors. RegionEntry cannot
      // have any predecessor. Link from/to Loop's preheader will be move to
      // VPLoop when region is connected to graph.
      PlanUtils.disconnectBlocks(RegionExit, RegionEntry);

      // Connect VPLoop to graph
      PlanUtils.insertRegion(NewRegion, RegionEntry, RegionExit);

      // Recursively build subregions inside VPLoop.
      // Please, note that VPLoop's entry may be the entry block of a nested
      // region, so we have to revisit it.
      buildSubRegions(RegionEntry, NewRegion, DomTree, PostDomTree, PlanUtils,
                      BB2VPBB, VPBB2BB);
    }
    // VPRegion detection. We only consider ParentRegion's entry when
    // ParentRegion is a VPLoop
    else if (CurrentVPBB->getNumSuccessors() > 1 &&
             (CurrentVPBB != ParentRegion->getEntry() ||
              isa<VPLoop>(ParentRegion))) {

      VPBlockBase *PostDom =
          PostDomTree.getNode(CurrentVPBB)->getIDom()->getBlock();
      VPBlockBase *Dom = DomTree.getNode(PostDom)->getIDom()->getBlock();
      assert(isa<VPBasicBlock>(PostDom) &&
             "Expected VPBasicBlock as post-dominator");
      assert(isa<VPBasicBlock>(Dom) && "Expected VPBasicBlock as dominator");

      // New VPRegion entry found.
      if (Dom == CurrentVPBB) {
        NewRegion = PlanUtils.createRegion(false /*isReplicator*/);

        // Create a fake exit VPBB to prevent that several regions share the
        // same exit VPBB
        // TODO: We may want to do this selectively.
        VPBasicBlock *FakeExit = PlanUtils.createBasicBlock();
        PlanUtils.insertBlockBefore(FakeExit, PostDom);

        // Connect new subregion to graph
        PlanUtils.insertRegion(NewRegion, Dom, FakeExit);

        // Recursively build subregions inside VPLoop.
        // Please, note that VPLoop's entry may be the entry block of a nested
        // region, so we have to revisit it.
        buildSubRegions(CurrentVPBB /*Dom*/, NewRegion, DomTree, PostDomTree,
                        PlanUtils, BB2VPBB, VPBB2BB);
      }

      // TODO: From VPOPredicator
      // For now, it is assumed we're dealing exclusively with innermost loop
      // vectorization. Mark the SESE region as divergent if the condition of
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

    if (NewRegion) {
      // Set NewRegion's parent
      PlanUtils.setBlockParent(NewRegion, ParentRegion);

      // Add NewRegion to visited, just in case it has multiple predecessors
      Visited.insert(NewRegion);

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
}

std::shared_ptr<VPlan>
LoopVectorizationPlanner::buildInitialVPlan(unsigned StartRangeVF,
                                            unsigned &EndRangeVF) {
  // TODO: StartRangeVF and EndRangeVF are not being used by now

  // VPlan and VPlanUtils for this VPlan
  std::shared_ptr<VPlan> SharedPlan = std::make_shared<VPlan>();
  VPlan *Plan = SharedPlan.get();
  IntelVPlanUtils PlanUtils(Plan);

  // BasicBlock to VPBasicBlock map for this VPlan
  DenseMap<BasicBlock *, VPBasicBlock *> BB2VPBB;
  DenseMap<VPBasicBlock *, BasicBlock *> VPBB2BB;

  // Build top VPRegion and initial VPBasicBlock-based CFG
  VPRegionBlock *TopRegion = buildInitialCFG(PlanUtils, BB2VPBB, VPBB2BB);

  // Set TopRegion as VPlan Entry
  Plan->setEntry(TopRegion);
  DEBUG(VPlanPrinter PlanPrinter(dbgs(), *Plan);
        PlanPrinter.dump("LVP: Initial CFG for VF=4"));

  // TODO: SEME-to-SESE loop massaging should happen here. TopRegion contains only
  // VPBasicBlocks (1:1 relation with original BBs). LoopInfo can be used by
  // means of BB2VPBB map. Loop massaging shouldn't modify loops' entries and
  // it should return the new loops' single exits.

  VPDominatorTree DomTree(false /* DominatorTree */);
  DomTree.recalculate(*TopRegion);
  DEBUG(dbgs() << "Dominator Tree:\n"; DomTree.print(dbgs()));
  VPDominatorTree PostDomTree(true /* Post-Dominator Tree */);
  PostDomTree.recalculate(*TopRegion);
  DEBUG(dbgs() << "PostDominator Tree:\n"; PostDomTree.print(dbgs()));

  assert(isa<VPBasicBlock>(TopRegion->getEntry()) &&
         "Expected VPBasicBlock as TopRegion's entry");

  // Build sub-regions, including VPLoops. At this point, only SESE regions are
  // expected. The algorithm will fail otherwise.
  buildSubRegions(cast<VPBasicBlock>(TopRegion->getEntry()), TopRegion, DomTree,
                  PostDomTree, PlanUtils, BB2VPBB, VPBB2BB);

  // TODO: CFG massaging for inner loops in outer loop vectorization scenarios
  // might happen here.

  // FOR STRESS TESTING, uncomment the following:
  // EndRangeVF = StartRangeVF * 2;

  return SharedPlan;
}
