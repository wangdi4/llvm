#include "VPlanHCFGBuilder.h"
#include "../VPlanBuilder.h"
#include "Intel_LoopCFU.h"
#include "VPLoopInfo.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRDDAnalysis.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HLNodeUtils.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HLNodeVisitor.h"
#include "llvm/Analysis/LoopIterator.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/IR/IntrinsicInst.h"
#include <deque>

#define DEBUG_TYPE "VPlanHCFGBuilder"

using namespace llvm;
using namespace llvm::vpo;

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

static cl::opt<bool> DisableUniformRegions(
    "disable-uniform-regions", cl::init(false), cl::Hidden,
    cl::desc("Disable detection of uniform Regions in VPlan. All regions are "
             "set as divergent."));

// Split loops' preheader block that are not in canonical form
void VPlanHCFGBuilderBase::splitLoopsPreheader(VPLoop *VPL) {

  // TODO: So far, I haven't found a test case that hits one of these asserts.
  // The code commented out below should cover the second one.

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
  //    - there is no WRLp (auto-vectorization). We need an empty loop PH.
  //    - has multiple predecessors (it's a potential exit of another region).
  //    - is loop H of another loop.
  if (!WRLp || !PH->getSinglePredecessor() || VPLInfo->isLoopHeader(PH)) {
    PlanUtils.splitBlock(PH, VPLInfo, VPDomTree, VPPostDomTree);
  }

  // Apply simplification to subloops
  for (auto VPSL : VPL->getSubLoops()) {
    splitLoopsPreheader(VPSL);
  }
}

void VPlanHCFGBuilderBase::mergeLoopExits(VPLoop *VPL) {

  VPLoopInfo *VPLInfo = PlanUtils.getVPlan()->getVPLoopInfo();

  SmallVector<VPBlockBase *, 2> ExitBlocks;
  VPL->getUniqueExitBlocks(ExitBlocks);

  // Apply simplification to subloops
  for (auto VPSL : VPL->getSubLoops()) {
    mergeLoopExits(VPSL);
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
      PhiRecipe->addIncomingValue(VPConstantRecipe(ExitID),
                                  ExittingBlock);
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
    PlanUtils.connectBlocks(NewCascadedExit, CBR, ExitBlock,
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

  DEBUG(VPlan *Plan = PlanUtils.getVPlan();
        Plan->setName("LVP: Plain CFG for VF=4\n"); dbgs() << *Plan);
  FixDominance(CascadedExit);
}

// Split loops' exit block that are not in canonical form
void VPlanHCFGBuilderBase::splitLoopsExit(VPLoop *VPL) {

  VPLoopInfo *VPLInfo = PlanUtils.getVPlan()->getVPLoopInfo();

  VPBlockBase *Exit = VPL->getUniqueExitBlock();
  assert(Exit && "Only single-exit loops expected");

  // Split loop exit with multiple successors or that is preheader of another
  // loop
  VPBlockBase *PotentialH = Exit->getSingleSuccessor();
  if (!PotentialH ||
      (VPLInfo->isLoopHeader(PotentialH) &&
       VPLInfo->getLoopFor(PotentialH)->getLoopPreheader() == Exit)) {

    PlanUtils.splitBlock(Exit, VPLInfo, VPDomTree, VPPostDomTree);
  }

  // Apply simplification to subloops
  for (auto VPSL : VPL->getSubLoops()) {
    splitLoopsExit(VPSL);
  }
}

// Split basic blocks to increase the number of non-loop regions detected during
// the construction of the hierarchical CFG.
void VPlanHCFGBuilderBase::simplifyNonLoopRegions() {

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
        PlanUtils.splitBlock(CurrentBlock, Plan->getVPLoopInfo(), VPDomTree,
                             VPPostDomTree);
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
// construction.
void VPlanHCFGBuilderBase::simplifyPlainCFG() {

  IntelVPlan *Plan = PlanUtils.getVPlan();
  assert(isa<VPRegionBlock>(Plan->getEntry()) &&
         "VPlan entry is not a VPRegionBlock");
  VPRegionBlock *TopRegion = cast<VPRegionBlock>(Plan->getEntry());
  (void)TopRegion;
  VPLoopInfo *VPLInfo = Plan->getVPLoopInfo();

  assert((VPLInfo->size() == 1) && "Expected only 1 top-level loop");
  VPLoop *TopLoop = *VPLInfo->begin();

  splitLoopsPreheader(TopLoop);

  if (LoopMassagingEnabled) {
    // DEBUG(dbgs() << "Dominator Tree Before mergeLoopExits\n";
    // VPDomTree.print(dbgs()));
    mergeLoopExits(TopLoop);
    DEBUG(Verifier->verifyHierarchicalCFG(TopRegion));
    // DEBUG(dbgs() << "Dominator Tree After mergeLoopExits\n";
    // VPDomTree.print(dbgs()));
  }

  if (VPlanLoopCFU) {
    // TODO: Move VPLoopCFU to this file (like mergeLoopExits)?
    // TODO: SE and LI shouldn't be necessary at this point. We have to find a
    // way to implement it without LLVM-IR specific analyses. Temporarily
    // commenting this code to make progress.
    // VPLoopCFU LCFU(Plan, PlanUtils, SE, LI, VPLInfo, VPDomTree,
    // VPPostDomTree);
    // LCFU.makeInnerLoopControlFlowUniform();
  }

  splitLoopsExit(TopLoop);
  simplifyNonLoopRegions();
}

// Create new LoopRegion's using VPLoopInfo analysis and introduce them into the
// hierarchical CFG. This function doesn't traverse the whole CFG and region's
// size and block's parent are not properly updated. They are updated in
// buildNonLoopRegions.
void VPlanHCFGBuilderBase::buildLoopRegions() {

  VPLoopInfo *VPLInfo = PlanUtils.getVPlan()->getVPLoopInfo();

  // Auxiliary function that implements the main functionality of
  // buildLoopRegions
  std::function<void(VPLoop *)> buildLoopRegionsImpl = [&](VPLoop *VPL) {

    // Create new loop region
    VPLoopRegion *VPLR = createLoopRegion(VPL);

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

// Create new non-loop VPRegionBlock's and update the information of all the
// blocks in the hierarchical CFG. The hierarchical CFG is stable and contains
// consisten information after this step.
void VPlanHCFGBuilderBase::buildNonLoopRegions(VPRegionBlock *ParentRegion) {

  VPLoopInfo *VPLInfo = PlanUtils.getVPlan()->getVPLoopInfo();

  DEBUG(dbgs() << "Building Non-Loop Regions for " << ParentRegion->getName()
               << "\n"
               << "   Entry: " << ParentRegion->getEntry()->getName() << "\n"
               << "   Exit: " << ParentRegion->getExit()->getName() << "\n");

  SmallVector<VPBlockBase *, 16> WorkList;
  SmallPtrSet<VPBlockBase *, 16> Visited;
  WorkList.push_back(ParentRegion->getEntry());

  unsigned ParentSize = 0;
  bool ParentIsDivergent = false;

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
    ++ParentSize;

    // Pointer to a new subregion or existing VPLoopRegion subregion
    VPRegionBlock *SubRegion = dyn_cast<VPLoopRegion>(Current);
    VPBlockBase *RegionExit;

    // Non-loop VPRegion detection.
    if (!DisableNonLoopSubRegions && !SubRegion /* Skip VPLoopRegions */ &&
        isNonLoopRegion(Current, ParentRegion, RegionExit /*output*/)) {

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

    // New region was built or Current is a LoopRegion.
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
      buildNonLoopRegions(SubRegion);

    } else {
      // Current is a VPBasicBlock that didn't trigger the creation of a new
      // region.

      // Set Current's parent
      PlanUtils.setBlockParent(Current, ParentRegion);

      // Check if Current causes parent region to be divergent.
      ParentIsDivergent |= isDivergentBlock(Current);

      // No new region has been detected. Add Current's successors.
      for (auto Succ : Current->getSuccessors()) {
        DEBUG(dbgs() << "Adding " << Succ->getName() << " to WorkList"
                     << "\n");
        WorkList.push_back(Succ);
      }
    }
  }

  PlanUtils.setRegionSize(ParentRegion, ParentSize);
  PlanUtils.setRegionDivergent(ParentRegion, ParentIsDivergent);

  DEBUG(dbgs() << "End of HCFG build for " << ParentRegion->getName() << "\n");
}

void VPlanHCFGBuilderBase::buildHierarchicalCFG() {

  IntelVPlan *Plan = PlanUtils.getVPlan();

  // Build Top Region enclosing the plain CFG
  VPRegionBlock *TopRegion = buildPlainCFG();

  // Set Top Region as VPlan Entry
  Plan->setEntry(TopRegion);
  DEBUG(Plan->setName("HCFGBuilder: Plain CFG\n"); dbgs() << *Plan);

  DEBUG(Verifier->verifyHierarchicalCFG(TopRegion));

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
  // DEBUG(dbgs() << "Loop Info:\n"; LI->print(dbgs()));
  DEBUG(dbgs() << "VPLoop Info After buildPlainCFG:\n"; VPLInfo->print(dbgs()));

  // Compute postdom tree for the plain CFG.
  VPPostDomTree.recalculate(*TopRegion);
  DEBUG(dbgs() << "PostDominator Tree After buildPlainCFG:\n";
        VPPostDomTree.print(dbgs()));

  // Prepare/simplify CFG for hierarchical CFG construction
  simplifyPlainCFG();

  DEBUG(Plan->setName("HCFGBuilder: After simplifyPlainCFG\n");
        dbgs() << *Plan);
  DEBUG(dbgs() << "Dominator Tree After simplifyPlainCFG\n";
        VPDomTree.print(dbgs()));
  DEBUG(dbgs() << "PostDominator Tree After simplifyPlainCFG:\n";
        VPPostDomTree.print(dbgs()));
  DEBUG(dbgs() << "VPLoop Info After simplifyPlainCFG:\n";
        VPLInfo->print(dbgs()));

  DEBUG(Verifier->verifyHierarchicalCFG(TopRegion));

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
  buildLoopRegions();
  buildNonLoopRegions(TopRegion);

  DEBUG(Plan->setName("HCFGBuilder: After building HCFG\n"); dbgs() << *Plan;);

  DEBUG(Verifier->setVPLoopInfo(VPLInfo);
        Verifier->verifyHierarchicalCFG(TopRegion));
}

// Return true if a non-loop region can be formed from \p Entry. If so, \p Exit
// returns region's exit for the detected region.
bool VPlanHCFGBuilderBase::isNonLoopRegion(VPBlockBase *Entry,
                                           VPRegionBlock *ParentRegion,
                                           VPBlockBase *&Exit) {

  // Region's entry must have multiple successors and must be a VPBasicBlock at
  // this point. Also skip ParentRegion's Entry to prevent infinite recursion
  if (Entry == ParentRegion->getEntry() || Entry->getNumPredecessors() != 1 ||
      Entry->getNumSuccessors() < 2 || !isa<VPBasicBlock>(Entry))
    return false;

  VPBlockBase *PotentialExit =
      VPPostDomTree.getNode(Entry)->getIDom()->getBlock();
  // Region's exit must have a single successor
  if (PotentialExit->getNumSuccessors() != 1 ||
      !isa<VPBasicBlock>(PotentialExit) ||
      // TODO: Temporal check to skip regions that share exit node with parent
      // region.
      ParentRegion->getExit() == PotentialExit)
    return false;

  VPBlockBase *Dom = VPDomTree.getNode(PotentialExit)->getIDom()->getBlock();
  if (Dom != Entry ||
      !regionIsBackEdgeCompliant(Entry, PotentialExit, ParentRegion))
    return false;

  Exit = PotentialExit;
  return true;
}

// This is a temporal implementation to detect and discard non-loop regions
// whose entry and exit blocks are in different graph cycles. At this point, the
// only cycles we have to care about are those created by loop latches. This
// means that problematic potential non-loop regions will have entry and/or exit
// blocks immediately nested inside a VPLoopRegion (i.e., block's parent will be
// a VPLoopRegion). In order to detect such cases, we currently check whether
// the loop header is reachable starting from region's entry block up to
// region's exit block.
bool VPlanHCFGBuilderBase::regionIsBackEdgeCompliant(
    const VPBlockBase *Entry, const VPBlockBase *Exit,
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

// TODO
// Return true if \p Block is a VPBasicBlock that contains a successor selector
// (ConditionBitRecipe) that is not uniform. If Block is a VPRegionBlock, it
// returns false since a region can only have a single successor (by now).
bool VPlanHCFGBuilderBase::isDivergentBlock(VPBlockBase *Block) {
  if (DisableUniformRegions)
    return true;

  if (auto *VPBB = dyn_cast<VPBasicBlock>(Block)) {
    unsigned NumSuccs = Block->getNumSuccessors();
    if (NumSuccs < 2) {
      assert(!VPBB->getConditionBitRecipe() &&
             "Unexpected condition bit recipe");
      return false;
    } else {
      // Multiple successors. Checking uniformity of ConditionBitRecipe.
      VPConditionBitRecipeBase *CBR = VPBB->getConditionBitRecipe();
      assert(CBR && "Expected condition bit recipe.");

      if (auto *CBRWS = dyn_cast<VPConditionBitRecipeWithScalar>(CBR)) {
        // TODO: Temporal implementation for HIR
        if (Legal)
          return !Legal->isUniformForTheLoop(CBRWS->getScalarCondition());
        else
          return true;
      } else if (isa<VPBranchIfNotAllZeroRecipe>(CBR))
        // This recipe is uniform by definition.
        return false;
      else if (isa<VPNonUniformConditionBitRecipe>(CBR))
        // This recipe is divergent by definition.
        return true;
      else
        llvm_unreachable("Unsupported condition bit recipe.");
    }
  }

  // Regions doesn't change parent region divergence.
  assert(Block->getSinglePredecessor() && "Region with multiple successors");
  return false;
}

// Build plain CFG from incomming IR using only VPBasicBlock's that contain
// OneByOneRecipe's and ConditionBitRecipe's. Return VPRegionBlock that
// encloses all the VPBasicBlock's of the plain CFG.
class PlainCFGBuilder {
private:
  /// Outermost loop of the input loop nest.
  Loop *TheLoop;

  LoopInfo *LI;

  /// Output TopRegion.
  VPRegionBlock *TopRegion = nullptr;
  /// Number of VPBasicBlocks in TopRegion.
  unsigned TopRegionSize = 0;

  IntelVPlanUtils &PlanUtils;
  VPBuilderIR VPIRBuilder;

  DenseMap<Value *, VPConditionBitRecipeBase *> BranchCondMap;
  DenseMap<BasicBlock *, VPBasicBlock *> BB2VPBB;
  DenseMap<Value *, VPValue *> IRDef2VPValue;

  // Auxiliary functions
  bool isConditionForUniformBranch(Instruction *I);
  void setVPBBPredsFromBB(VPBasicBlock *VPBB, BasicBlock *BB);
  VPBasicBlock *createOrGetVPBB(BasicBlock *BB);
  void createRecipesForVPBB(VPBasicBlock *VPBB, BasicBlock *BB);
  bool isExternalDef(Instruction *Inst);
  VPValue *createOrGetVPOperand(Value *IROp);
  void createVPInstructionsForRange(BasicBlock::iterator I,
                                    BasicBlock::iterator J,
                                    VPBasicBlock *InsertionPoint);

public:
  PlainCFGBuilder(Loop *Lp, LoopInfo *LI, IntelVPlanUtils &Utils)
      : TheLoop(Lp), LI(LI), PlanUtils(Utils) {}

  VPRegionBlock *buildPlainCFG();
};

static bool isInstructionToIgnore(Instruction *I) {
  // DeadInstructions are not taken into account at this point. IV update and
  // loop latch condition need to be part of HCFG to constitute a
  // UniformConditionBitRecipe. If we treat them as dead instructions, we
  // would create a LiveInConditionBitRecipe for the loop latch condition,
  // which is not correct.
  return /*DeadInstructions.count(I) ||*/ isa<BranchInst>(I) ||
         isa<DbgInfoIntrinsic>(I);
}

bool PlainCFGBuilder::isConditionForUniformBranch(Instruction *I) {
  auto isBranchInst = [&](User *U) -> bool {
    return isa<BranchInst>(U) && TheLoop->contains(cast<Instruction>(U));
  };
  return TheLoop->contains(I) && /*Legal->isUniformForTheLoop(I)&&*/
         any_of(I->users(), isBranchInst);
}

// Set predecessors of \p VPBB in the same order as they are in LLVM \p BB.
void PlainCFGBuilder::setVPBBPredsFromBB(VPBasicBlock *VPBB, BasicBlock *BB) {

  for (BasicBlock *Pred : predecessors(BB)) {
    VPBasicBlock *PredVPBB = createOrGetVPBB(Pred);
    PlanUtils.appendBlockPredecessor(VPBB, PredVPBB);
  }
}

// Create a new empty VPBasicBlock for an incomming BasicBlock or retrieve an
// existing one if it was already created.
VPBasicBlock *PlainCFGBuilder::createOrGetVPBB(BasicBlock *BB) {

  VPBasicBlock *VPBB;
  auto BlockIt = BB2VPBB.find(BB);

  if (BlockIt == BB2VPBB.end()) {
    // New VPBB
    DEBUG(dbgs() << "Creating VPBasicBlock for " << BB->getName() << "\n");
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

// Return true if instruction is not defined either inside the loop nest or
// outermost loop PH or outermost loop exits. External definition applies only
// to Instruction since Constant and other Values will have a different
// representation.
bool PlainCFGBuilder::isExternalDef(Instruction *Inst) {

  BasicBlock *InstParent = Inst->getParent();
  assert(InstParent && "Expected instruction parent.");

  // - Check whether Instruction definition is in loop PH -
  BasicBlock *PH = TheLoop->getLoopPreheader();
  assert(PH && "Expected loop pre-header.");

  if (InstParent == PH) {
    // Instruction definition is in outermost loop PH.
    return false;
  }

  // - Check whether Instruction definition is in loop exits -
  SmallVector<BasicBlock *, 2> LoopExits;
  TheLoop->getUniqueExitBlocks(LoopExits);
  for (BasicBlock *Exit : LoopExits) {
    if (InstParent == Exit) {
      // Instruction definition is in outermost loop exit.
      return false;
    }
  }

  // - Check whether Instruction definition is in loop body -
  return TheLoop->isLoopInvariant(Inst);
}

// Helper function that is used by 'createVPInstructionsForRange' to create a
// new VPValue or retrieve an existing one for an Instruction's operand (IROp).
// This function must only be used to create/retrieve VPValues for
// *Instruction's operands* and not to create regular VPInstruction's. For the
// latter, you should look at 'createVPInstructionForRange'.
VPValue *PlainCFGBuilder::createOrGetVPOperand(Value *IROp) {
  // Constant operand
  if (Constant *IRConst = dyn_cast<Constant>(IROp))
    return PlanUtils.getVPlan()->getVPConstant(IRConst);

  auto VPValIt = IRDef2VPValue.find(IROp);
  // Operand has an associated VPInstruction or VPValue (for Values without
  // specific representation) that was previously created.
  if (VPValIt != IRDef2VPValue.end())
    return VPValIt->second;

  // Operand is not a Constant and doesn't have a previously created
  // VPInstruction/VPValue. This means that operand is:
  //   A) a definition external to VPlan,
  //   B) a use whose definition hasnt't been visited yet (phi's operands),
  //   C) any other Value without specific representation in VPlan.
  VPValue *NewVPVal;
  if (auto *InstOperand = dyn_cast<Instruction>(IROp)) {
    // A and B: Create instruction without operands and do not insert it in
    // the VPBasicBlock. For A, we insert it in the VPlan's pool of external
    // definitions. For B, it will be fixed and inserted when the definition
    // is processed.
    VPBuilder::InsertPointGuard Guard(VPIRBuilder);
    VPIRBuilder.clearInsertionPoint();
    NewVPVal = VPIRBuilder.createNaryOp(InstOperand->getOpcode(),
                                        {} /*No operands*/, InstOperand);
    if (isExternalDef(InstOperand))
      PlanUtils.getVPlan()->addExternalDef(cast<VPInstruction>(NewVPVal));
  } else // C
    // TODO: Add VPRaw? Specific representation for metadata?.
    // TODO: Memory deallocation of these VPValues.
    NewVPVal = new VPValue();

  IRDef2VPValue[IROp] = NewVPVal;
  return NewVPVal;
}

// Create VPInstruction's for a range of Instructions in a BasicBlock.
// VPInstruction's are created using a VPBuilder so new
// VPInstructionRangeRecipe's are created automatically when necessary.
void PlainCFGBuilder::createVPInstructionsForRange(
    BasicBlock::iterator I, BasicBlock::iterator J,
    VPBasicBlock *InsertionPoint) {

  VPIRBuilder.setInsertPoint(InsertionPoint);
  for (Instruction &InstRef : make_range(I, J)) {
    Instruction *Inst = &InstRef;
    auto VPValIt = IRDef2VPValue.find(Inst);

    VPInstruction *NewVPInst;
    if (VPValIt != IRDef2VPValue.end()) {
      // Inst is a definition with a user that has been previosly visited. We
      // have to set its operands properly and insert it into the
      // VPBasicBlock/Recipe.
      assert(isa<VPInstruction>(VPValIt->second) &&
             "Unexpected VPValue. Expected a VPInstruction");
      NewVPInst = cast<VPInstruction>(VPValIt->second);
      VPIRBuilder.insert(NewVPInst);
    } else {
      // Create new VPInstruction.
      // NOTE: We set operands later to factorize code in 'if' and 'else'
      // branches.
      NewVPInst = cast<VPInstruction>(VPIRBuilder.createNaryOp(
          Inst->getOpcode(), {} /*No operands*/, Inst));
      IRDef2VPValue[Inst] = NewVPInst;
    }

    // Translate LLVM-IR operands into VPValue operands and set them in the new
    // VPInstruction.
    for (Value *Op : Inst->operands())
      NewVPInst->addOperand(createOrGetVPOperand(Op));
  }
}

// Create new OnyByOneRecipes and ConditionBitRecipes in a VPBasicBlock, given
// its BasicBlock counterpart. This function must be invoked in RPO because
// creation of UniformConditionBitRecipe assumes that all predecessors have
// been visited.
void PlainCFGBuilder::createRecipesForVPBB(VPBasicBlock *VPBB, BasicBlock *BB) {

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
    if (isConditionForUniformBranch(&*I)) {
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

    // - Create new VPInstructions add add them to VPBB. -

    // Search for last live Instruction to close VPBB.
    BasicBlock::iterator J = I;
    for (++J; J != E; ++J) {
      Instruction *Instr = &*J;
      if (isInstructionToIgnore(Instr) || isConditionForUniformBranch(Instr))
        break; // Sequence of instructions not to ignore ended.
    }

    createVPInstructionsForRange(I, J, VPBB);

    I = J;
  }
}

VPRegionBlock *PlainCFGBuilder::buildPlainCFG() {

  // Create Top Region. It will be parent of all VPBBs
  TopRegion = PlanUtils.createRegion(false /*isReplicator*/);
  TopRegionSize = 0;

  // Scan the body of the loop in a topological order to visit each basic block
  // after having visited its predecessor basic blocks.
  LoopBlocksDFS DFS(TheLoop);
  DFS.perform(LI);

  for (BasicBlock *BB : make_range(DFS.beginRPO(), DFS.endRPO())) {

    // Create new VPBasicBlock and its recipes
    VPBasicBlock *VPBB = createOrGetVPBB(BB);
    createRecipesForVPBB(VPBB, BB);

    // Add successors and predecessors
    TerminatorInst *TI = BB->getTerminator();
    assert(TI && "Terminator expected");
    unsigned NumSuccs = TI->getNumSuccessors();

    // Note: we are not invoking createRecipesForVPBB for successor blocks at
    // this point because we would be breaking the RPO traversal
    if (NumSuccs == 1) {
      VPBasicBlock *SuccVPBB = createOrGetVPBB(TI->getSuccessor(0));
      assert(SuccVPBB && "VPBB Successor not found");

      PlanUtils.setBlockSuccessor(VPBB, SuccVPBB);
      VPBB->setCBlock(BB);
      VPBB->setTBlock(TI->getSuccessor(0));

    } else if (NumSuccs == 2) {
      VPBasicBlock *SuccVPBB0 = createOrGetVPBB(TI->getSuccessor(0));
      assert(SuccVPBB0 && "Successor 0 not found");
      VPBasicBlock *SuccVPBB1 = createOrGetVPBB(TI->getSuccessor(1));
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

      PlanUtils.setBlockTwoSuccessors(VPBB, CondBitR, SuccVPBB0, SuccVPBB1);
      VPBB->setCBlock(BB);
      VPBB->setTBlock(TI->getSuccessor(0));
      VPBB->setFBlock(TI->getSuccessor(1));

    } else {
      llvm_unreachable("Number of successors not supported");
    }

    // Set predecessors in the same order as they are in LLVM basic block.
    setVPBBPredsFromBB(VPBB, BB);
  }

  // Add outermost loop preheader to plain CFG. It needs explicit treatment
  // because it's not a successor of any block inside the loop.
  BasicBlock *PreheaderBB = TheLoop->getLoopPreheader();
  assert((PreheaderBB->getTerminator()->getNumSuccessors() == 1) &&
         "Unexpected loop preheader");

  VPBasicBlock *PreheaderVPBB = createOrGetVPBB(PreheaderBB);
  createRecipesForVPBB(PreheaderVPBB, PreheaderBB);
  VPBlockBase *HeaderVPBB = BB2VPBB[TheLoop->getHeader()];
  // Preheader's predecessors have already been set in RPO traversal.
  PlanUtils.setBlockSuccessor(PreheaderVPBB, HeaderVPBB);

  // Empty VPBasicBlock were created for loop exit BasicBlocks but they weren't
  // visited because they are not inside the loop. Create now recipes for them
  // and set its predecessors.
  SmallVector<BasicBlock *, 2> LoopExits;
  TheLoop->getUniqueExitBlocks(LoopExits);
  for (BasicBlock *BB : LoopExits) {
    VPBasicBlock *VPBB = BB2VPBB[BB];
    createRecipesForVPBB(VPBB, BB);
    setVPBBPredsFromBB(VPBB, BB);
  }

  // Top Region setup

  // Create a dummy block as Top Region's entry
  VPBlockBase *RegionEntry = PlanUtils.createBasicBlock();
  ++TopRegionSize;
  PlanUtils.setBlockParent(RegionEntry, TopRegion);
  PlanUtils.connectBlocks(RegionEntry, PreheaderVPBB);

  // Create a dummy block as Top Region's exit
  VPBlockBase *RegionExit = PlanUtils.createBasicBlock();
  ++TopRegionSize;
  PlanUtils.setBlockParent(RegionExit, TopRegion);

  // Connect dummy Top Region's exit.
  if (LoopExits.size() == 1) {
    VPBasicBlock *LoopExitVPBB = BB2VPBB[LoopExits.front()];
    PlanUtils.connectBlocks(LoopExitVPBB, RegionExit);
  } else {
    // If there are multiple exits in the outermost loop, we need another dummy
    // block as landing pad for all of them.
    assert(LoopExits.size() > 1 && "Wrong number of exit blocks");

    VPBlockBase *LandingPad = PlanUtils.createBasicBlock();
    ++TopRegionSize;
    PlanUtils.setBlockParent(LandingPad, TopRegion);

    // Connect multiple exits to landing pad
    for (auto ExitBB : make_range(LoopExits.begin(), LoopExits.end())) {
      VPBasicBlock *ExitVPBB = BB2VPBB[ExitBB];
      PlanUtils.connectBlocks(ExitVPBB, LandingPad);
    }

    // Connect landing pad to Top Region's exit
    PlanUtils.connectBlocks(LandingPad, RegionExit);
  }

  PlanUtils.setRegionEntry(TopRegion, RegionEntry);
  PlanUtils.setRegionExit(TopRegion, RegionExit);
  PlanUtils.setRegionSize(TopRegion, TopRegionSize);

  return TopRegion;
}

VPRegionBlock *VPlanHCFGBuilder::buildPlainCFG() {
  PlainCFGBuilder PCFGBuilder(TheLoop, LI, PlanUtils);
  VPRegionBlock *TopRegion = PCFGBuilder.buildPlainCFG();
  return TopRegion;
}

//===-------------===//
// HIR Specific Code //
//===-------------===//

/// \brief Visitor that traverses HLNode's (lexical links) in topological order
/// and build a plain CFG out of them. It returns a region (TopRegion)
/// containing the plain CFG.
///
/// It is inspired by AVR-based VPOCFG algorithm and uses a non-recursive
/// visitor to explicitly handle visits of "compound" HLNode's (HLIfs, HLLoop,
/// HLSwitch) and trigger the creation-closure of VPBasicBlocks.
///
/// Creation/closure of VPBasicBlock's is triggered by:
///   1) HLLoop Pre-header
///   *) HLoop Header
///   *) End of HLLoop body
///   *) HLoop Exit (Postexit)
///   *) If-then branch
///   *) If-else branch
///   *) End of HLIf
///   *) HLLabel
///   *) HLGoto
///
/// The algorithm keeps an active VPBasicBlock (ActiveVPBB) that is populated
/// with "instructions". When one of the previous conditions is met, a new
/// active VPBasicBlock is created and connected to its predecessors. A list of
/// VPBasicBlock (Predecessors) holds the predecessors to be connected to the
/// new active VPBasicBlock when it is created HLGoto needs special treatment
/// since its VPBasicBlock is not reachable from an HLLabel. For that reason, a
/// VPBasicBlock ending with an HLGoto is connected to its successor when HLGoto
/// is visited.
///
/// TODO's:
///   - Outer loops.
///   - Expose ZTT for inner loops.
///   - VPInstructions
///   - HLSwitch
///   - Loops with multiple exits.
///
class PlainCFGBuilderHIR : public HLNodeVisitorBase {
  friend HLNodeVisitor<PlainCFGBuilderHIR, false /*Recursive*/>;

private:
  /// Outermost loop of the input loop nest.
  HLLoop *TheLoop;

  /// HIR DDGraph that contains DD information for the incoming loop nest. It is
  /// used to navigate from sink blobs to their respective source blobs.
  const DDGraph &DDG;

  IntelVPlanUtils &PlanUtils;

  /// Map between loop header VPBasicBlock's and their respective HLLoop's. It
  /// is populated in this phase to keep the information necessary to create
  /// VPLoopRegionHIR's later in the H-CFG construction process.
  SmallDenseMap<VPBasicBlock *, HLLoop *> &Header2HLLoop;

  /// VPInstruction builder for HIR.
  VPBuilderHIR VPHIRBuilder;

  /// Output TopRegion.
  VPRegionBlock *TopRegion = nullptr;
  /// Number of VPBasicBlocks in TopRegion.
  unsigned TopRegionSize = 0;

  /// Hold the set of dangling predecessors to be connected to the next active
  /// VPBasicBlock.
  std::deque<VPBasicBlock *> Predecessors;

  /// Hold the VPBasicBlock that is being populated with instructions. Null
  /// value indicates that a new active VPBasicBlock has to be created.
  VPBasicBlock *ActiveVPBB = nullptr;

  /// Map between HLNode's that open a VPBasicBlock and such VPBasicBlock's.
  DenseMap<HLNode *, VPBasicBlock *> HLN2VPBB;

  /// Map between HLInst's and their respective VPValue's representing their definition.
  DenseMap<HLDDNode *, VPValue *> HLDef2VPValue;

  VPBasicBlock *createOrGetVPBB(HLNode *HNode = nullptr);
  void connectVPBBtoPreds(VPBasicBlock *VPBB);
  void updateActiveVPBB(HLNode *HNode = nullptr, bool IsPredecessor = true);

  // VPInstruction methods
  VPValue *createNoOperandVPInst(HLDDNode *DDNode = nullptr);
  VPValue *createOrGetVPDefFrom(const DDEdge *Edge);
  VPValue *createOrGetVPOperand(RegDDRef *HIROp);
  void buildVPOpsForDDNode(HLDDNode *HInst,
                           SmallVectorImpl<VPValue *> &VPValueOps);
  void createOrFixVPInstr(HLDDNode *DDNode);

  // Visitor methods
  void visit(const HLNode *Node) {}
  void postVisit(const HLNode *Node) {}

  void visit(HLLoop *HLp);
  void visit(HLIf *HIf);
  void visit(HLSwitch *HSw) {
    llvm_unreachable("Switches are not supported yet.");
  };
  void visit(HLInst *HInst);
  void visit(HLGoto *HGoto);
  void visit(HLLabel *HLabel);

public:
  PlainCFGBuilderHIR(HLLoop *Lp, const DDGraph &DDG, IntelVPlanUtils &Utils,
                     SmallDenseMap<VPBasicBlock *, HLLoop *> &H2HLLp)
      : TheLoop(Lp), DDG(DDG), PlanUtils(Utils), Header2HLLoop(H2HLLp) {}

  /// Build a plain CFG for an HLLoop loop nest. Return the TopRegion containing
  /// the plain CFG.
  VPRegionBlock *buildPlainCFG();
};

/// Retrieve an existing VPBasicBlock for \p HNode. It there is no existing
/// VPBasicBlock, a new VPBasicBlock is created and mapped to \p HNode. If \p
/// HNode is null, the new VPBasicBlock is not mapped to any HLNode.
VPBasicBlock *PlainCFGBuilderHIR::createOrGetVPBB(HLNode *HNode) {

  // Auxiliary function that creates an empty VPBasicBlock, set its parent to
  // TopRegion and increases TopRegion's size.
  auto createVPBB = [&]() -> VPBasicBlock * {
    VPBasicBlock *NewVPBB = PlanUtils.createBasicBlock();
    PlanUtils.setBlockParent(NewVPBB, TopRegion);
    ++TopRegionSize;

    return NewVPBB;
  };

  if (!HNode) {
    // No HLNode associated to this VPBB.
    return createVPBB();
  } else {
    // Try to retrieve existing VPBB for this HLNode. Otherwise, create a new
    // VPBB and add it to the map.
    auto BlockIt = HLN2VPBB.find(HNode);

    if (BlockIt == HLN2VPBB.end()) {
      // New VPBB
      // TODO: Print something more useful.
      DEBUG(dbgs() << "Creating VPBasicBlock for " << HNode->getHLNodeID()
                   << "\n");
      VPBasicBlock *VPBB = createVPBB();
      HLN2VPBB[HNode] = VPBB;
      // NewVPBB->setOriginalBB(BB);
      return VPBB;
    } else {
      // Retrieve existing VPBB
      return BlockIt->second;
    }
  }
}

/// Connect \p VPBB to all the predecessors in Predecessors and clear
/// Predecessors.
void PlainCFGBuilderHIR::connectVPBBtoPreds(VPBasicBlock *VPBB) {

  for (VPBasicBlock *Pred : Predecessors) {
    PlanUtils.appendBlockSuccessor(Pred, VPBB);
    PlanUtils.appendBlockPredecessor(VPBB, Pred);
  }

  Predecessors.clear();
}

// Update active VPBasicBlock only when this is null. It creates a new active
// VPBasicBlock, connect it to existing predecessors, set it as new insertion
// point in VPHIRBUilder and, if \p ISPredecessor is true, add it as predecessor
// of the (future) subsequent active VPBasicBlock's.
void PlainCFGBuilderHIR::updateActiveVPBB(HLNode *HNode, bool IsPredecessor) {
  if (!ActiveVPBB) {
    ActiveVPBB = createOrGetVPBB(HNode);
    connectVPBBtoPreds(ActiveVPBB);
    VPHIRBuilder.setInsertPoint(ActiveVPBB);

    if (IsPredecessor)
      Predecessors.push_back(ActiveVPBB);
  }
}

// Return the Constant representation of a constant RegDDRef.
static Constant *getConstantFromHIR(RegDDRef *RDDRef) {
  assert(RDDRef->getSingleCanonExpr() &&
         "Constant CanonExpr that is not Single CanonExpr?");
  CanonExpr *CExpr = RDDRef->getSingleCanonExpr();

  if (CExpr->isIntConstant()) {
    int64_t CECoeff = CExpr->getConstant();
    Type *CETy = CExpr->getDestType();

    // Null value for pointer types needs special treatment
    if (CECoeff == 0 && CETy->isPointerTy()) {
      return Constant::getNullValue(CETy);
    }
    return ConstantInt::getSigned(CETy, CECoeff);
  }

  ConstantFP *FPConst;
  if (CExpr->isFPConstant(&FPConst))
    return FPConst;

  if (CExpr->isNull())
    return ConstantPointerNull::get(cast<PointerType>(CExpr->getDestType()));

  llvm_unreachable("Unsupported HIR Constant.");
}

// Return the VPInstruction opcode for a given HLDDNode.
static unsigned getOpcodeFromHIR(HLDDNode *DDNode) {
  if (auto *HInst = dyn_cast<HLInst>(DDNode)) {
    assert(HInst->getLLVMInstruction() &&
           "Missing LLVM Instruction for HLInst.");
    return HInst->getLLVMInstruction()->getOpcode();
  }

  if (auto *HIf = dyn_cast<HLIf>(DDNode)) {
    assert(HIf->getNumPredicates() && "HLIf with no predicate?");
    Type *PredType = (*HIf->ddref_begin())->getDestType();

    if (PredType->isIntOrIntVectorTy()) {
      return Instruction::ICmp;
    }

    assert(PredType->isFPOrFPVectorTy() && "Expected a floating point type.");
    // HIR only generates multiple predicates for integers.
    assert(HIf->getNumPredicates() == 1 &&
           "Expected single predicate for FP type.");

    return Instruction::FCmp;
  }

  llvm_unreachable("Missing opcode for HLInst.");
}

// Create a VPInstruction with no operands and do not insert it in any
// VPBasicBlock.
VPValue *PlainCFGBuilderHIR::createNoOperandVPInst(HLDDNode *DDNode) {
  VPBuilder::InsertPointGuard Guard(VPHIRBuilder);
  VPHIRBuilder.clearInsertionPoint();
  unsigned Opcode = DDNode ? getOpcodeFromHIR(DDNode) : 0 /*No operand*/;
  VPValue *NewVPVal = VPHIRBuilder.createNaryOp(Opcode, {} /*No operands*/, DDNode);
  return NewVPVal;
}

// Returns the VPValue that defines Edge's sink.
VPValue *PlainCFGBuilderHIR::createOrGetVPDefFrom(const DDEdge *Edge) {
  // Get the HLDDNode causing the definition.
  HLDDNode *DefNode = Edge->getSrc()->getHLDDNode();
  auto VPValIt = HLDef2VPValue.find(DefNode);

  // Return the VPValue associated to the HLDDNode definition if it has been
  // visited previously.
  if (VPValIt != HLDef2VPValue.end())
    return VPValIt->second;

  // HLDDNode definition hasn't been visited yet. Create VPInstruction without
  // operands and do not insert it in the VPBasicBlock. This VPInstruciton will
  // be fixed and inserted when the HLDDNode definition is processed in
  // createVPInstructionsForRange.
  VPValue *NewVPInst = createNoOperandVPInst(DefNode);
  HLDef2VPValue[DefNode] = NewVPInst;
  return NewVPInst;
}

// Helper function that is used by 'buildVPOpsForDDNode' to create a new VPValue
// or retrieve an existing one for an HLDDNode's operand (HIROp). This function
// must only be used to create/retrieve VPValues for *HLDDNode's operands*
// and not to create regular VPInstruction's. For the latter, you should look at
// 'createOrFixVPInstr'.
VPValue *PlainCFGBuilderHIR::createOrGetVPOperand(RegDDRef *HIROp) {
  if (HIROp->isConstant()) {
    return PlanUtils.getVPlan()->getVPConstant(getConstantFromHIR(HIROp));
  }

  if (HIROp->isUnitaryBlob()) {
    // Operand represents a single temporal that doesn't need decomposition.
    // Conversions or single temporals with constant additive != 1 will not hit
    // here.
    auto OpInEdges = DDG.incoming(HIROp);

    // If operand has incoming DD edges, we need to retrieve (or create) the
    // VPValues associated to the DD sources (definition). If there are
    // multiple definitions, in addition, we introduce a semi-phi operation that
    // "blends" all the VPValue definitions.
    if (OpInEdges.begin() != OpInEdges.end()) {
      // Single definition.
      if (std::next(OpInEdges.begin()) == OpInEdges.end())
        return createOrGetVPDefFrom(*OpInEdges.begin());

      // Multiple definitions.
      SmallVector<VPValue *, 4> OpVPDefs;
      for (const DDEdge *Edge : OpInEdges) {
        OpVPDefs.push_back(createOrGetVPDefFrom(Edge));
      }

      return VPHIRBuilder.createSemiPhiOp(OpVPDefs);
    }

    // Operand has no incoming DD edges. This means that HIROp is a use whose
    // definition is outside VPlan.
    VPValue *NewVPVal = createNoOperandVPInst();
    PlanUtils.getVPlan()->addExternalDef(cast<VPInstruction>(NewVPVal));
    return NewVPVal;
  }

  // Operand is a complex RegDDRef that needs decomposition. As it may contain
  // different temps (uses), we cannot introduce them into the VPValue U-D
  // chain until decomposition happens. For that reason, we use a VPValue to
  // mark that operand needs to be fixed later.
  // FIXME: This memory is not being freed. It will be fixed when introducing
  // decomposition.
  return new VPValue();
}

// Return a sequence of VPValues (VPValueOps) that represents DDNode's operands
// in VPlan. In addition to the RegDDRef to VPValue translation, operands are
// sorted in the way VPlan expects them. Some operands, such as the LHS operand
// in some HIR instructions, are ignored because they are not explicitly
// represented as an operand in VPlan.
void PlainCFGBuilderHIR::buildVPOpsForDDNode(
    HLDDNode *DDNode, SmallVectorImpl<VPValue *> &VPValueOps) {

  auto *HInst = dyn_cast<HLInst>(DDNode);
  bool IsStore =
      HInst && HInst->getLLVMInstruction()->getOpcode() == Instruction::Store;

  // Collect operands necessary to build a VPInstruction out of an HLInst and
  // translate them into VPValue's. We skip LHS operands for most instructions.
  for (RegDDRef *HIROp :
       make_range(DDNode->op_ddref_begin(), DDNode->op_ddref_end())) {
    if (HIROp->isLval() && !IsStore)
      continue;

    VPValueOps.push_back(createOrGetVPOperand(HIROp));
  }

  // Fix discrepancies in the order of operands between HLInst and
  // VPInstruction:
  //     - Store: dest = store src -> store src dest
  if (IsStore)
    std::iter_swap(VPValueOps.begin(), std::next(VPValueOps.begin()));
}

// Main helper function that creates a VPInstruction for DDNode and inserts it
// in the active VPBasicBlock and the HLDef2VPValue map. If a VPInstruction was
// created before for this DDNode, the VPInstruction is retrieved, its operands
// are created and it's inserted in the active VPBasicBlock.
void PlainCFGBuilderHIR::createOrFixVPInstr(HLDDNode *DDNode) {

  DEBUG(dbgs() << "Creating or fixing:"; DDNode->dump(); dbgs() << "\n");

  // Translate HIR operands into VPValue operands. This needs to happen before
  // creating the VPInstruction because it may introduce new VPInstructions for
  // operands (e.g., semi-phis).
  SmallVector<VPValue *, 4> VPValueOps;
  buildVPOpsForDDNode(DDNode, VPValueOps);

  auto VPValIt = HLDef2VPValue.find(DDNode);
  VPInstruction *NewVPInst;

  if (VPValIt != HLDef2VPValue.end()) {
    // DDNode is a definition with a user that has been previously visited. We
    // have to set its operands properly and insert it into the
    // VPBasicBlock/Recipe.
    NewVPInst = cast<VPInstruction>(VPValIt->second);
    VPHIRBuilder.insert(NewVPInst);
  } else {
    // Create new VPInstruction.
    // NOTE: We set operands later to factorize code in 'if' and 'else'
    // branches.
    NewVPInst = cast<VPInstruction>(VPHIRBuilder.createNaryOp(
        getOpcodeFromHIR(DDNode), {} /*No operands*/, DDNode));

    HLDef2VPValue[DDNode] = NewVPInst;
  }

  // Set VPInstruction's operands.
  for (VPValue *Operand : VPValueOps) {
    NewVPInst->addOperand(Operand);
  }
}

void PlainCFGBuilderHIR::visit(HLLoop *HLp) {

  // TODO: Print something more useful.
  DEBUG(dbgs() << "Visiting HLLoop: " << HLp->getHLNodeID() << "\n");

  // - ZTT for inner loops -
  // TODO: isInnerMost(), ztt_pred_begin/end

  // - Loop PH -
  // Force creation of a new VPBB for PH.
  ActiveVPBB = nullptr;

  if (HLp->hasPreheader()) {
    HLNodeUtils::visitRange<false /*Recursive*/>(
        *this /*visitor*/, HLp->pre_begin(), HLp->pre_end());

    assert(ActiveVPBB == HLN2VPBB[&*HLp->pre_begin()] &&
           "Loop PH generates more than one VPBB?");
  } else {
    // There is no PH in HLLoop. Create dummy VPBB as PH. We could introduce
    // this dummy VPBB in simplifyPlainCFG, but according to the design for
    // LLVM-IR, we expect to have a loop with a PH as input. It's then better to
    // introduce the dummy PH here.
    updateActiveVPBB();
  }

  // - Loop H -
  // Force creation of a new VPBB for H.
  ActiveVPBB = nullptr;
  HLNodeUtils::visitRange<false /*Recursive*/>(
      *this /*visitor*/, HLp->child_begin(), HLp->child_end());

  // Map loop header VPBasicBlock with HLLoop for later loop region detection.
  VPBasicBlock *Header = HLN2VPBB[&*HLp->child_begin()];
  assert(Header && "Expected VPBasicBlock for loop header.");
  Header2HLLoop[Header] = HLp;

  // An HLoop will always have a single latch that will be also an exiting
  // block. Keep track of it. If there is no active VPBB, we have to create a
  // new one.
  // TODO: Materialize exit condition.
  updateActiveVPBB();
  // Connect Latch to Header and add ConditionBitRecipe.
  // TODO: Workaround. Setting a fake ConditionBitRecipe.
  PlanUtils.connectBlocks(ActiveVPBB /*Latch*/, Header);
  PlanUtils.setBlockConditionBitRecipe(
      ActiveVPBB /*Latch*/, PlanUtils.createUniformConditionBitRecipe(nullptr));

  // - Loop Exits -
  // Force creation of a new VPBB for Exit.
  ActiveVPBB = nullptr;

  if (HLp->hasPostexit()) {
    HLNodeUtils::visitRange<false /*Recursive*/>(
        *this /*visitor*/, HLp->post_begin(), HLp->post_end());

    assert(ActiveVPBB == HLN2VPBB[&*HLp->post_begin()] &&
           "Loop Exit generates more than one VPBB?");
  } else {
    // There is no Exit in HLLoop. Create dummy VPBB as Exit (see comment for
    // dummy PH).
    updateActiveVPBB();
  }
}

void PlainCFGBuilderHIR::visit(HLIf *HIf) {

  // - Condition -
  // We do not create a new active  VPBasicBlock for HLIf predicates
  // (condition). We reuse the previous one (if possible).
  // TODO: Predicates in HLIf are not HLInst's but CmpInst! We have to process
  // them separately and manually, creating VPInstructions for them and
  // combining them with AND operations.
  updateActiveVPBB(HIf);

  // Create (single, not decomposed) VPInstruction for HLIf's predicate.
  createOrFixVPInstr(HIf);

  VPBasicBlock *ConditionVPBB = ActiveVPBB;
  // assert("HLIf condition generates more than one VPBB?");
  // TODO: Workaround. Setting a fake ConditionBitRecipe.
  PlanUtils.setBlockConditionBitRecipe(
      ActiveVPBB, PlanUtils.createUniformConditionBitRecipe(nullptr));

  // - Then branch -
  // Force creation of a new VPBB for Then branch.
  ActiveVPBB = nullptr;
  HLNodeUtils::visitRange<false /*Recursive*/>(
      *this /*visitor*/, HIf->then_begin(), HIf->then_end());

  // - Else branch -
  if (HIf->hasElseChildren()) {
    // Hold predecessors from Then branch to be used after HLIf visit and before
    // visiting else branch.
    SmallVector<VPBasicBlock *, 2> ThenOutputPreds(Predecessors.begin(),
                                                   Predecessors.end());
    // Clear Predecessors before Else branch visit (we don't want to connect
    // Then branch VPBasicBlock's with Else branch VPBasicBlock's) and add HLIf
    // condition as new predecessor for Else branch.
    Predecessors.clear();
    Predecessors.push_back(ConditionVPBB);

    // Force creation of a new VPBB for Else branch.
    ActiveVPBB = nullptr;
    HLNodeUtils::visitRange<false /*Recursive*/>(
        *this /*visitor*/, HIf->else_begin(), HIf->else_end());

    // Prepend predecessors generated by Then branch to those in Predecessors
    // from Else branch.
    // to be used after HLIf visit.
    Predecessors.insert(Predecessors.begin(), ThenOutputPreds.begin(),
                        ThenOutputPreds.end());
  } else {
    // No Else branch

    // Add ConditionVPBB to Predecessors for HLIf successor. Predecessors
    // contains predecessors from Then branch.
    // TODO: In this order? back or front?
    Predecessors.push_back(ConditionVPBB);
  }

  // Force the creation of a new VPBB for the next HLNode.
  ActiveVPBB = nullptr;
}

void PlainCFGBuilderHIR::visit(HLInst *HInst) {
  // Create new VPBasicBlock if there isn't a reusable one.
  updateActiveVPBB(HInst);

  // Create VPInstruction for HInst
  createOrFixVPInstr(HInst);
}

void PlainCFGBuilderHIR::visit(HLGoto *HGoto) {

  // If there is an ActiveVPBB we have to remove it from Predecessors. HLGoto's
  // VPBB and HLLabel's VPBB are connected explicitly in this visit function
  // because they "break" the expected topological order traversal and,
  // therefore, need special treatment.
  if (ActiveVPBB) {
    // If this assert is raised, we would have to remove ActiveVPBB using
    // find/erase (more expensive).
    assert(Predecessors.back() == ActiveVPBB &&
           "Expected ActiveVPBB at the end of Predecessors.");
    Predecessors.pop_back();
  }

  // Create new VPBasicBlock if there isn't a reusable one. If a new ActiveVPBB
  // is created, do not add it to Predecessors (see previous comment).
  updateActiveVPBB(HGoto, false /*IsPredecessor*/);

  // Create (or get) a new VPBB for HLLabel and connect to HLGoto's VPBB.
  HLLabel *Label = HGoto->getTargetLabel();
  VPBasicBlock *LabelVPBB = createOrGetVPBB(Label);
  PlanUtils.connectBlocks(ActiveVPBB, LabelVPBB);

  // Force the creation of a new VPBasicBlock for the next HLNode.
  ActiveVPBB = nullptr;
}

void PlainCFGBuilderHIR::visit(HLLabel *HLabel) {
  // Force the creation of a new VPBasicBlock for an HLLabel.
  ActiveVPBB = nullptr;
  updateActiveVPBB(HLabel);
}

VPRegionBlock *PlainCFGBuilderHIR::buildPlainCFG() {
  // Create new TopRegion.
  TopRegion = PlanUtils.createRegion(false /*isReplicator*/);

  // Create a dummy VPBB as TopRegion's Entry.
  assert(!ActiveVPBB && "ActiveVPBB must be null.");
  updateActiveVPBB();
  PlanUtils.setRegionEntry(TopRegion, ActiveVPBB);

  // Trigger the visit of the loop nest.
  visit(TheLoop);

  // Create a dummy VPBB as TopRegion's Exit.
  ActiveVPBB = nullptr;
  updateActiveVPBB();
  PlanUtils.setRegionExit(TopRegion, ActiveVPBB);

  PlanUtils.setRegionSize(TopRegion, TopRegionSize);

  return TopRegion;
}

VPRegionBlock *VPlanHCFGBuilderHIR::buildPlainCFG() {
  PlainCFGBuilderHIR PCFGBuilder(TheLoop, DDG, PlanUtils, Header2HLLoop);
  VPRegionBlock *TopRegion = PCFGBuilder.buildPlainCFG();
  return TopRegion;
}

VPLoopRegion *VPlanHCFGBuilderHIR::createLoopRegion(VPLoop *VPLp) {
  assert(isa<VPBasicBlock>(VPLp->getHeader()) &&
         "Expected VPBasicBlock as Loop header.");
  HLLoop *HLLp = Header2HLLoop[cast<VPBasicBlock>(VPLp->getHeader())];
  assert(HLLp && "Expected HLLoop");
  return VPlanHCFGBuilderBase::PlanUtils.createLoopRegionHIR(VPLp, HLLp);
}

