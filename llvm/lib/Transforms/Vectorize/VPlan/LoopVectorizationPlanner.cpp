
#include "LoopVectorizationPlanner.h"
#include "llvm/Analysis/Intel_VPO/WRegionInfo/WRegionInfo.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/LoopIterator.h"

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

VPRegionBlock *LoopVectorizationPlanner::buildInitialCFG(
    IntelVPlanUtils &PlanUtils,
    DenseMap<BasicBlock *, VPBasicBlock *> &BB2VPBB) {

  auto createOrGetVPBB = [&](BasicBlock *BB) -> VPBasicBlock * {
    auto BlockIt = BB2VPBB.find(BB);

    VPBasicBlock *VPBB;
    if (BlockIt == BB2VPBB.end()) {
      VPBB = PlanUtils.createBasicBlock();
      BB2VPBB[BB] = VPBB;
      // Size++;
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
    // TODO: Bug in Predicator. Split setSuccessor and setPrecedessor. Not
    // sure if RPO may also be a problem.
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

  // Create top VPRegion

  VPRegionBlock *TopRegion = PlanUtils.createRegion(false /*isReplicator*/);
  // Create a dummy entry block for VPRegion as loop's header has predecessor
  // Use loop's exit as region's exit
  VPBlockBase *RegionEntry = PlanUtils.createBasicBlock();
  PlanUtils.setSuccessor(RegionEntry, VPLEntry);
  VPBlockBase *RegionExit = VPLExit;
  
  PlanUtils.setRegionEntry(TopRegion, RegionEntry);
  PlanUtils.setRegionExit(TopRegion, RegionExit);

  return TopRegion;
}

void LoopVectorizationPlanner::buildLoopRegions(
    IntelVPlanUtils &PlanUtils,
    DenseMap<BasicBlock *, VPBasicBlock *> &BB2VPBB) {

  auto createAndConnectVPLoop = [&](Loop *L) {
    VPLoop *VPL = PlanUtils.createLoop();

    VPBasicBlock *RegionEntry = BB2VPBB[L->getHeader()];
    assert(L->getLoopLatch() && "Multiple latches are not supported");
    VPBasicBlock *RegionExit = BB2VPBB[L->getLoopLatch()];

    // Remove LoopLatch from RegionEntry's predecessors. RegionEntry cannot have
    // any predecessor other than the VPLoop.
    PlanUtils.disconnectBlocks(RegionExit, RegionEntry);

    // Insert new VPLoop in VPlan
    PlanUtils.connectRegion(VPL, RegionEntry, RegionExit);

    return VPL;
  };

  // Create new VPLoop for main loop.
  createAndConnectVPLoop(TheLoop);

  // Iterate on sub-loops
  for (Loop *SubLoop : make_range(TheLoop->begin(), TheLoop->end())) {
    createAndConnectVPLoop(SubLoop);
  }
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

  // Build VPBasicBlock-based CFG and introduce VPLoops
  // The current algorithm works in two steps. A more complex (and maybe more
  // efficient) single step would have been possible but we still don't know how
  // SEME loops will fit here. If some massage is necessary before SESE VPLoops
  // construction for SEME loops, it will be easier to do it in the two step
  // approach.
  VPRegionBlock *TopRegion = buildInitialCFG(PlanUtils, BB2VPBB);
  buildLoopRegions(PlanUtils, BB2VPBB);
  //TODO: buildIfElseRegions

  // Set VPLoop as VPlan Entry
  Plan->setEntry(TopRegion);

  // FOR STRESS TESTING, uncomment the following:
  // EndRangeVF = StartRangeVF * 2;

  return SharedPlan;
}
