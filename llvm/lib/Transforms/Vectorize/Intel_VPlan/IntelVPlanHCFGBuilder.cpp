//===-- IntelVPlanHCFGBuilder.cpp -----------------------------------------===//
//
//   Copyright (C) 2017 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file implements the algorithm that builds the hierarchical CFG in
/// VPlan. Further documentation can be found in document 'VPlan Hierarchical
/// CFG Builder'.
///
//===----------------------------------------------------------------------===//

#include "IntelVPlanHCFGBuilder.h"
#include "IntelLoopCFU.h"
#include "IntelVPlanBranchDependenceAnalysis.h"
#include "IntelVPlanBuilder.h"
#include "IntelVPlanDivergenceAnalysis.h"
#include "IntelVPlanLoopInfo.h"
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

#if INTEL_CUSTOMIZATION
static cl::opt<bool>
    DisableVPlanDA("disable-vplan-da", cl::init(true), cl::Hidden,
                   cl::desc("Disable VPlan divergence analysis"));

static cl::opt<bool>
    VPlanPrintSimplifyCFG("vplan-print-after-simplify-cfg", cl::init(false),
                          cl::desc("Print plain dump after VPlan simplify "
                                   "plain CFG"));
#endif

// Split loops' preheader block that are not in canonical form
void VPlanHCFGBuilder::splitLoopsPreheader(VPLoop *VPL) {

  // TODO: So far, I haven't found a test case that hits one of these asserts.
  // The code commented out below should cover the second one.

  VPLoopInfo *VPLInfo = Plan->getVPLoopInfo();

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
    VPBlockUtils::splitBlock(PH, VPLInfo, VPDomTree, VPPostDomTree, Plan);
  }

  // Apply simplification to subloops
  for (auto VPSL : VPL->getSubLoops()) {
    splitLoopsPreheader(VPSL);
  }
}

void VPlanHCFGBuilder::mergeLoopExits(VPLoop *VPL) {

  VPLoopInfo *VPLInfo = Plan->getVPLoopInfo();

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

    VPBasicBlock *NewCascadedExit =
        new VPBasicBlock(VPlanUtils::createUniqueName("BB"));
    llvm_unreachable("Fix CBR");
    VPInstruction *CBR =
        // new VPCmpInst(PhiRecipe, VPConstantRecipe(ExitID));
        nullptr; // FIXME: This should be fixed once recipes are replaced by
                 // VPInstructions
    VPRegionBlock *Parent = ExitBlock->getParent();
    NewCascadedExit->setParent(Parent);
    Parent->setSize(Parent->getSize() + 1);
    VPBlockUtils::connectBlocks(NewCascadedExit, CBR, ExitBlock,
                                LastCascadedExitBlock, Plan);
    // Add NewBlock to VPLoopInfo
    if (VPLoop *Loop = VPLInfo->getLoopFor(ExitBlock)) {
      Loop->addBasicBlockToLoop(NewCascadedExit, *VPLInfo);
    }
    if (ExitID == ExitBlocks.size())
      NewCascadedExit->appendRecipe(PhiRecipe);
    NewCascadedExit->appendRecipe(CBR);

    if (ExitID < ExitBlocks.size())
      return NewCascadedExit;

    for (auto ExittingBlock : ExittingBlocks) {
      VPBlockUtils::movePredecessor(
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

  LLVM_DEBUG(Plan->setName("LVP: Plain CFG for VF=4\n"); dbgs() << *Plan);
  FixDominance(CascadedExit);
}

// Split loops' exit block that are not in canonical form
void VPlanHCFGBuilder::splitLoopsExit(VPLoop *VPL) {

  VPLoopInfo *VPLInfo = Plan->getVPLoopInfo();

  VPBlockBase *Exit = VPL->getUniqueExitBlock();
  assert(Exit && "Only single-exit loops expected");

  // Split loop exit with multiple successors or that is preheader of another
  // loop
  VPBlockBase *PotentialH = Exit->getSingleSuccessor();
  if (!PotentialH ||
      (VPLInfo->isLoopHeader(PotentialH) &&
       VPLInfo->getLoopFor(PotentialH)->getLoopPreheader() == Exit)) {

    VPBlockUtils::splitBlock(Exit, VPLInfo, VPDomTree, VPPostDomTree, Plan);
  }

  // Apply simplification to subloops
  for (auto VPSL : VPL->getSubLoops()) {
    splitLoopsExit(VPSL);
  }
}

// Split basic blocks to increase the number of non-loop regions detected during
// the construction of the hierarchical CFG.
void VPlanHCFGBuilder::simplifyNonLoopRegions() {

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
        VPBlockUtils::splitBlock(CurrentBlock, Plan->getVPLoopInfo(), VPDomTree,
                                 VPPostDomTree, Plan);
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
void VPlanHCFGBuilder::simplifyPlainCFG() {

  assert(isa<VPRegionBlock>(Plan->getEntry()) &&
         "VPlan entry is not a VPRegionBlock");
  VPRegionBlock *TopRegion = cast<VPRegionBlock>(Plan->getEntry());
  (void)TopRegion;
  VPLoopInfo *VPLInfo = Plan->getVPLoopInfo();

  assert((VPLInfo->size() == 1) && "Expected only 1 top-level loop");
  VPLoop *TopLoop = *VPLInfo->begin();

  splitLoopsPreheader(TopLoop);

  if (LoopMassagingEnabled) {
    // LLVM_DEBUG(dbgs() << "Dominator Tree Before mergeLoopExits\n";
    // VPDomTree.print(dbgs()));
    mergeLoopExits(TopLoop);
    LLVM_DEBUG(Verifier->verifyHierarchicalCFG(TopRegion));
    // LLVM_DEBUG(dbgs() << "Dominator Tree After mergeLoopExits\n";
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
void VPlanHCFGBuilder::buildLoopRegions() {

  VPLoopInfo *VPLInfo = Plan->getVPLoopInfo();

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

    LLVM_DEBUG(dbgs() << "Creating new VPLoopRegion " << VPLR->getName() << "\n"
                      << "   Entry: " << RegionEntry->getName() << "\n"
                      << "   Exit: " << RegionExit->getName() << "\n");

    // Connect loop region to graph
    VPBlockUtils::insertRegion(VPLR, RegionEntry, RegionExit,
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

  LLVM_DEBUG(dbgs() << "Building LoopRegion's\n");

  for (VPLoop *VPL : make_range(VPLInfo->begin(), VPLInfo->end()))
    buildLoopRegionsImpl(VPL);
}

// Create new non-loop VPRegionBlock's and update the information of all the
// blocks in the hierarchical CFG. The hierarchical CFG is stable and contains
// consisten information after this step.
void VPlanHCFGBuilder::buildNonLoopRegions(VPRegionBlock *ParentRegion) {

  VPLoopInfo *VPLInfo = Plan->getVPLoopInfo();

  LLVM_DEBUG(
      dbgs() << "Building Non-Loop Regions for " << ParentRegion->getName()
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
    LLVM_DEBUG(dbgs() << "Visiting " << Current->getName()
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
      SubRegion = new VPRegionBlock(VPBlockBase::VPRegionBlockSC,
                                    VPlanUtils::createUniqueName("region"));

      LLVM_DEBUG(dbgs() << "Creating new VPRegion " << SubRegion->getName()
                        << "\n"
                        << "   Entry: " << Current->getName() << "\n"
                        << "   Exit: " << RegionExit->getName() << "\n");
      assert(RegionExit && "RegionExit cannot be null");

      VPBlockUtils::insertRegion(SubRegion, Current /*Entry*/, RegionExit,
                                 false /*recomputeSize*/);

      // Add new region to VPLoopInfo.
      if (VPLoop *Loop = VPLInfo->getLoopFor(SubRegion->getEntry())) {
        Loop->addBasicBlockToLoop(SubRegion, *VPLInfo);
      }
    }

    // New region was built or Current is a LoopRegion.
    if (SubRegion) {
      // Set SubRegion's parent
      SubRegion->setParent(ParentRegion);

      // Add SubRegion's successors to worklist.
      for (auto Succ : SubRegion->getSuccessors()) {
        LLVM_DEBUG(dbgs() << "Adding " << Succ->getName() << " to WorkList"
                          << "\n");
        WorkList.push_back(Succ);
      }

      // Recursively build non-regions inside subregion
      buildNonLoopRegions(SubRegion);

    } else {
      // Current is a VPBasicBlock that didn't trigger the creation of a new
      // region.

      // Set Current's parent
      Current->setParent(ParentRegion);

      // Check if Current causes parent region to be divergent.
      ParentIsDivergent |= isDivergentBlock(Current);

      // No new region has been detected. Add Current's successors.
      for (auto Succ : Current->getSuccessors()) {
        LLVM_DEBUG(dbgs() << "Adding " << Succ->getName() << " to WorkList"
                          << "\n");
        WorkList.push_back(Succ);
      }
    }
  }

  ParentRegion->setSize(ParentSize);
  ParentRegion->setDivergent(ParentIsDivergent);

  LLVM_DEBUG(dbgs() << "End of HCFG build for " << ParentRegion->getName()
                    << "\n");
}


// Go through the blocks in Region, collecting uniforms.
void VPlanHCFGBuilder::collectUniforms(VPRegionBlock *Region) {
  for (VPBlockBase *Block :
       make_range(df_iterator<VPRegionBlock *>::begin(Region),
                  df_iterator<VPRegionBlock *>::end(Region))) {
    if (auto *VPBB = dyn_cast<VPBasicBlock>(Block)) {
      if (Block->getNumSuccessors() >= 2) {
        // Multiple successors. Checking uniformity of Condition Bit
        // Instruction.
        VPValue *CBV = VPBB->getCondBit();
        assert(CBV && "Expected condition bit value.");

        bool isUniform = Legal->isUniformForTheLoop(CBV->getUnderlyingValue());
        if (isUniform)
          Plan->UniformCBVs.insert(CBV);
      }
    }
  }
}

void VPlanHCFGBuilder::buildHierarchicalCFG() {

  // Build Top Region enclosing the plain CFG
  VPRegionBlock *TopRegion = buildPlainCFG();

  // Collecte divergence information
  collectUniforms(TopRegion);

  // Set Top Region as VPlan Entry
  Plan->setEntry(TopRegion);
  LLVM_DEBUG(Plan->setName("HCFGBuilder: Plain CFG\n"); dbgs() << *Plan);

  LLVM_DEBUG(Verifier->verifyHierarchicalCFG(TopRegion));

  // Compute dom tree for the plain CFG for VPLInfo. We don't need post-dom tree
  // at this point.
  VPDomTree.recalculate(*TopRegion);
  LLVM_DEBUG(dbgs() << "Dominator Tree After buildPlainCFG\n";
             VPDomTree.print(dbgs()));

  // TODO: If more efficient, we may want to "translate" LoopInfo to VPLoopInfo.
  // Compute VPLInfo and keep it in VPlan
  VPLoopInfo *VPLInfo = new VPLoopInfo();
  VPLInfo->analyze(VPDomTree);
  Plan->setVPLoopInfo(VPLInfo);
  // LLVM_DEBUG(dbgs() << "Loop Info:\n"; LI->print(dbgs()));
  LLVM_DEBUG(dbgs() << "VPLoop Info After buildPlainCFG:\n";
             VPLInfo->print(dbgs()));

  // Compute postdom tree for the plain CFG.
  VPPostDomTree.recalculate(*TopRegion);
  LLVM_DEBUG(dbgs() << "PostDominator Tree After buildPlainCFG:\n";
             VPPostDomTree.print(dbgs()));

#if INTEL_CUSTOMIZATION
  // simplifyPlainCFG inserts empty blocks with CondBit recipes. This messes up
  // determining the influence region of a branch instruction. i.e., the
  // immediate post-dominator becomes this empty block instead of the actual
  // convergence point containing the phi. Running DA here allows reuse of the
  // current Dominator Trees and results in fewer modifications to the DA
  // algorithm since it was designed to run over a plain CFG. We should also
  // be able to leverage DA for use in the inner loop control flow uniformity
  // massaging for outer loop vectorization (done in simplifyPlainCFG). That
  // way, we only have to transform the CFG for inner loops known to be non-
  // uniform.
  // TODO: Right now DA is computed per VPlan for the outermost loop of the
  // VPlan region. We will need additional information provided to DA if we wish
  // to vectorize more than one loop, or vectorize a specific loop within the
  // VPlan that is not the outermost one.
  // TODO: Check to see how this ordering impacts loops with multiple exits in
  // mergeLoopExits(). It's possible that we may want to delay DA from running
  // until after loops with multiple exits are canonicalized to a single loop
  // exit. But, this means that the DA algorithm will have to be changed to have
  // to deal with empty loop pre-header blocks unless we can run mergeLoopExits
  // before the empty pre-header blocks are inserted.
  if (!DisableVPlanDA) {
    // TODO: Determine if we want to have a separate DA instance for each VF.
    // Currently, there is only one instance and no distinction between VFs.
    // i.e., values are either uniform or divergent for all VFs.
    auto *VPDA = new VPlanDivergenceAnalysis();
    VPDA->compute(*(VPLInfo->begin()), VPLInfo, &VPDomTree, &VPPostDomTree);
    Plan->setVPlanDA(VPDA);
  }
#endif /* INTEL_CUSTOMIZATION */

  // Prepare/simplify CFG for hierarchical CFG construction
  simplifyPlainCFG();

#if INTEL_CUSTOMIZATION
  if (VPlanPrintSimplifyCFG) {
    errs() << "Print after simplify plain CFG\n";
    Plan->dump(errs());
  }
#endif

  LLVM_DEBUG(Plan->setName("HCFGBuilder: After simplifyPlainCFG\n");
             dbgs() << *Plan);
  LLVM_DEBUG(dbgs() << "Dominator Tree After simplifyPlainCFG\n";
             VPDomTree.print(dbgs()));
  LLVM_DEBUG(dbgs() << "PostDominator Tree After simplifyPlainCFG:\n";
             VPPostDomTree.print(dbgs()));
  LLVM_DEBUG(dbgs() << "VPLoop Info After simplifyPlainCFG:\n";
             VPLInfo->print(dbgs()));

  LLVM_DEBUG(Verifier->verifyHierarchicalCFG(TopRegion));

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

  LLVM_DEBUG(Plan->setName("HCFGBuilder: After building HCFG\n");
             dbgs() << *Plan;);

  LLVM_DEBUG(Verifier->setVPLoopInfo(VPLInfo);
             Verifier->verifyHierarchicalCFG(TopRegion));
}

// Return true if a non-loop region can be formed from \p Entry. If so, \p Exit
// returns region's exit for the detected region.
bool VPlanHCFGBuilder::isNonLoopRegion(VPBlockBase *Entry,
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
bool VPlanHCFGBuilder::regionIsBackEdgeCompliant(const VPBlockBase *Entry,
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

// TODO
// Return true if \p Block is a VPBasicBlock that contains a successor selector
// (CondBit) that is not uniform. If Block is a VPRegionBlock,
// it returns false since a region can only have a single successor (by now).
bool VPlanHCFGBuilder::isDivergentBlock(VPBlockBase *Block) {
  if (DisableUniformRegions)
    return true;

  if (auto *VPBB = dyn_cast<VPBasicBlock>(Block)) {
    unsigned NumSuccs = Block->getNumSuccessors();
    if (NumSuccs < 2) {
      assert(!VPBB->getCondBit() && "Unexpected condition bit instruction");
      return false;
    } else {
      // Multiple successors. Checking uniformity of Condition Bit Instruction.
      VPValue *CBV = VPBB->getCondBit();
      assert(CBV && "Expected condition bit value.");

      // TODO: Temporal implementation for HIR
      return !Plan->UniformCBVs.count(CBV);
    }
  }

  // Regions doesn't change parent region divergence.
  assert(Block->getSinglePredecessor() && "Region with multiple successors");
  return false;
}

namespace {
// Build plain CFG from incomming IR using only VPBasicBlock's that contain
// VPInstructions. Return VPRegionBlock that encloses all the VPBasicBlock's
// of the plain CFG.
class PlainCFGBuilder {
private:
  /// Outermost loop of the input loop nest.
  Loop *TheLoop;

  LoopInfo *LI;
  // TODO: This should be removed together with the UniformCBVs set.
  LoopVectorizationLegality *Legal;

  // Output TopRegion.
  VPRegionBlock *TopRegion = nullptr;

  // Number of VPBasicBlocks in TopRegion.
  unsigned TopRegionSize = 0;

  VPlan *Plan;

  // Builder of the VPlan instruction-level representation.
  VPBuilder VPIRBuilder;

  // NOTE: The following maps are intentionally destroyed after the plain CFG
  // construction because subsequent VPlan-to-VPlan transformation may
  // invalidate them.
  // Map incoming BasicBlocks to their newly-created VPBasicBlocks.
  DenseMap<BasicBlock *, VPBasicBlock *> BB2VPBB;
  // Map incoming Value definitions to their newly-created VPValues.
  DenseMap<Value *, VPValue *> IRDef2VPValue;
  /// Map the branches to the condition VPInstruction they are controlled by
  /// (Possibly at a different VPBB).
  DenseMap<Value *, VPValue *> BranchCondMap;

  // Hold phi node's that need to be fixed once the plain CFG has been built.
  SmallVector<PHINode *, 8> PhisToFix;

  // Auxiliary functions
  void setVPBBPredsFromBB(VPBasicBlock *VPBB, BasicBlock *BB);
  void fixPhiNodes();
  VPBasicBlock *createOrGetVPBB(BasicBlock *BB);
  bool isExternalDef(Value *Val);
  VPValue *createOrGetVPOperand(Value *IROp);
  void createVPInstructionsForVPBB(VPBasicBlock *VPBB, BasicBlock *BB);

public:
  PlainCFGBuilder(Loop *Lp, LoopInfo *LI, LoopVectorizationLegality *Legal,
                  VPlan *Plan)
      : TheLoop(Lp), LI(LI), Legal(Legal), Plan(Plan) {}

  VPRegionBlock *buildPlainCFG();
};
} // anonymous namespace

// Set predecessors of \p VPBB in the same order as they are in LLVM \p BB.
void PlainCFGBuilder::setVPBBPredsFromBB(VPBasicBlock *VPBB, BasicBlock *BB) {
  SmallVector<VPBlockBase *, 8> VPBBPreds;
  // Collect VPBB predecessors.
  for (BasicBlock *Pred : predecessors(BB))
    VPBBPreds.push_back(createOrGetVPBB(Pred));

  VPBB->setPredecessors(VPBBPreds);
}

// Set operands to VPInstructions representing phi nodes from the input IR.
// VPlan Phi nodes were created without operands in a previous step of the H-CFG
// construction because those operands might not have been created in VPlan at
// that time despite the RPO traversal. This function expects all the
// instructions to have a representation in VPlan so operands of VPlan phis can
// be properly set.
void PlainCFGBuilder::fixPhiNodes() {
  for (auto *Phi : PhisToFix) {
    assert(IRDef2VPValue.count(Phi) && "Missing VPInstruction for PHINode.");
    VPValue *VPVal = IRDef2VPValue[Phi];
    assert(isa<VPInstruction>(VPVal) && "Expected VPInstruction for phi node.");
    auto *VPPhi = cast<VPInstruction>(VPVal);
    assert(VPPhi->getNumOperands() == 0 &&
           "Expected VPInstruction with no operands.");

    for (Value *Op : Phi->operands())
      VPPhi->addOperand(createOrGetVPOperand(Op));
  }
}

// Create a new empty VPBasicBlock for an incomming BasicBlock or retrieve an
// existing one if it was already created.
VPBasicBlock *PlainCFGBuilder::createOrGetVPBB(BasicBlock *BB) {

  VPBasicBlock *VPBB;
  auto BlockIt = BB2VPBB.find(BB);

  if (BlockIt == BB2VPBB.end()) {
    // New VPBB
    LLVM_DEBUG(dbgs() << "Creating VPBasicBlock for " << BB->getName() << "\n");
    VPBB = new VPBasicBlock(VPlanUtils::createUniqueName("BB"));
    BB2VPBB[BB] = VPBB;
    VPBB->setOriginalBB(BB);
    VPBB->setParent(TopRegion);
    ++TopRegionSize;
  } else {
    // Retrieve existing VPBB
    VPBB = BlockIt->second;
  }

  return VPBB;
}

// Return true if \p Val is considered an external definition in the context of
// the plain CFG construction.
//
// An external definition is either:
#if INTEL_CUSTOMIZATION
// 1. A Value that is neither a Constant nor an Instruction.
#else
// 1. A Value that is not an Instruction. This will be refined in the future.
#endif
// 2. An Instruction that is outside of the CFG snippet represented in VPlan.
// However, since we don't represent loop Instructions in loop PH/Exit as
// VPInstructions during plain CFG construction, those are also considered
// external definitions in this particular context.
bool PlainCFGBuilder::isExternalDef(Value *Val) {
#if INTEL_CUSTOMIZATION
  assert(!isa<Constant>(Val) &&
         "Constants should have been processed separately.");
#endif
  // All the Values that are not Instructions are considered external
  // definitions for now.
  Instruction *Inst = dyn_cast<Instruction>(Val);
  if (!Inst)
    return true;

  // Check whether Instruction definition is within the loop nest.
  return !TheLoop->contains(Inst);
}

// Create a new VPValue or retrieve an existing one for the Instruction's
// operand \p IROp. This function must only be used to create/retrieve VPValues
// for *Instruction's operands* and not to create regular VPInstruction's. For
// the latter, please, look at 'createVPInstructionsForVPBB'.
VPValue *PlainCFGBuilder::createOrGetVPOperand(Value *IROp) {
#if INTEL_CUSTOMIZATION
  // Constant operand
  if (Constant *IRConst = dyn_cast<Constant>(IROp))
    return Plan->getVPConstant(IRConst);
#endif

  auto VPValIt = IRDef2VPValue.find(IROp);
  if (VPValIt != IRDef2VPValue.end())
    // Operand has an associated VPInstruction or VPValue that was previously
    // created.
    return VPValIt->second;

#if INTEL_CUSTOMIZATION
  // Operand is not a Constant and doesn't have a previously created
  // VPInstruction/VPVailue. This means that operand is:
#else
  // Operand doesn't have a previously created VPInstruction/VPValue. This
  // means that operand is:
#endif
  //   A) a definition external to VPlan,
  //   B) any other Value without specific representation in VPlan.
  // For now, we use VPValue to represent A and B and classify both as external
  // definitions. We may introduce specific VPValue subclasses for them in the
  // future.
  assert(isExternalDef(IROp) && "Expected external definition as operand.");

  // A and B: Create VPValue and add it to the pool of external definitions and
  // to the Value->VPValue map.
  VPValue *NewVPVal = new VPValue(IROp);
  Plan->addExternalDef(NewVPVal);
  IRDef2VPValue[IROp] = NewVPVal;
  return NewVPVal;
}

// Create new VPInstructions in a VPBasicBlock, given its BasicBlock
// counterpart. This function must be invoked in RPO so that the operands of a
// VPInstruction in \p BB have been visited before. VPInstructions representing
// Phi nodes are created without operands to honor the RPO traversal. They will
// be fixed later by 'fixPhiNodes'.
void PlainCFGBuilder::createVPInstructionsForVPBB(VPBasicBlock *VPBB,
                                                  BasicBlock *BB) {
  VPIRBuilder.setInsertPoint(VPBB);
  for (Instruction &InstRef : *BB) {
    Instruction *Inst = &InstRef;
    // There shouldn't be any VPValue for Inst at this point. Otherwise, we
    // visited Inst when we shouldn't, breaking the RPO traversal order.
    assert(!IRDef2VPValue.count(Inst) &&
           "Instruction shouldn't have been visited.");

    if (auto *Br = dyn_cast<BranchInst>(Inst)) {
      // Branch instruction is not explicitly represented in VPlan but we need
      // to represent its condition bit when it's conditional.
      if (Br->isConditional())
        createOrGetVPOperand(Br->getCondition());

      // Skip the rest of the Instruction processing for Branch instructions.
      continue;
    }

    VPInstruction *NewVPInst;
    if (auto *Phi = dyn_cast<PHINode>(Inst)) {
      // Phi node's operands may have not been visited at this point. We create
      // an empty VPInstruction that we will fix once the whole plain CFG has
      // been built.
      NewVPInst = cast<VPInstruction>(VPIRBuilder.createNaryOp(
          Inst->getOpcode(), {} /*No operands*/, Inst));
      PhisToFix.push_back(Phi);
    } else {
      // Translate LLVM-IR operands into VPValue operands and set them in the
      // new VPInstruction.
      SmallVector<VPValue *, 4> VPOperands;
      for (Value *Op : Inst->operands())
        VPOperands.push_back(createOrGetVPOperand(Op));

#if INTEL_CUSTOMIZATION
      if (CmpInst *CI = dyn_cast<CmpInst>(Inst)) {
        assert(VPOperands.size() == 2 && "Expected 2 operands in CmpInst.");
        NewVPInst = VPIRBuilder.createCmpInst(VPOperands[0], VPOperands[1], CI);
      } else
#endif
      // Build VPInstruction for any arbitraty Instruction without specific
      // representation in VPlan.
      NewVPInst = cast<VPInstruction>(
          VPIRBuilder.createNaryOp(Inst->getOpcode(), VPOperands, Inst));
    }

    IRDef2VPValue[Inst] = NewVPInst;
  }
}

VPRegionBlock *PlainCFGBuilder::buildPlainCFG() {
  // 1. Create the Top Region. It will be the parent of all VPBBs.
  TopRegion = new VPRegionBlock(VPBlockBase::VPRegionBlockSC,
                                VPlanUtils::createUniqueName("region"));
  TopRegionSize = 0;

  // 2. Scan the body of the loop in a topological order to visit each basic
  // block after having visited its predecessor basic blocks.Create a VPBB for
  // each BB and link it to its successor and predecessor VPBBs. Note that
  // predecessors must be set in the same order as they are in the incomming IR.
  // Otherwise, there might be problems with existing phi nodes and algorithms
  // based on predecessors traversal.

  // Create loop PH. PH needs to be explicitly processed since it's not taken
  // into account by LoopBlocksDFS below. Since the loop PH may contain any
  // Instruction, related or not to the loop nest, we do not create
  // VPInstructions for them. Those Instructions used within the loop nest will
  // be modeled as external definitions.
  BasicBlock *PreheaderBB = TheLoop->getLoopPreheader();
  assert((PreheaderBB->getTerminator()->getNumSuccessors() == 1) &&
         "Unexpected loop preheader");
  VPBasicBlock *PreheaderVPBB = createOrGetVPBB(PreheaderBB);
  // Create empty VPBB for Loop H so that we can link PH->H. H's VPInstructions
  // will be created during RPO traversal.
  VPBlockBase *HeaderVPBB = createOrGetVPBB(TheLoop->getHeader());
  // Preheader's predecessors will be set during the loop RPO traversal below.
  PreheaderVPBB->setOneSuccessor(HeaderVPBB);

  LoopBlocksRPO RPO(TheLoop);
  RPO.perform(LI);

  for (BasicBlock *BB : RPO) {
    // Create or retrieve the VPBasicBlock for this BB and create its
    // VPInstructions.
    VPBasicBlock *VPBB = createOrGetVPBB(BB);
    createVPInstructionsForVPBB(VPBB, BB);

    // Set VPBB successors. We create empty VPBBs for successors if they don't
    // exist already. Recipes will be created when the successor is visited
    // during the RPO traversal.
    TerminatorInst *TI = BB->getTerminator();
    assert(TI && "Terminator expected");
    unsigned NumSuccs = TI->getNumSuccessors();

    if (NumSuccs == 1) {
      VPBasicBlock *SuccVPBB = createOrGetVPBB(TI->getSuccessor(0));
      assert(SuccVPBB && "VPBB Successor not found");
      VPBB->setOneSuccessor(SuccVPBB);
      VPBB->setCBlock(BB);
      VPBB->setTBlock(TI->getSuccessor(0));
    } else if (NumSuccs == 2) {
      VPBasicBlock *SuccVPBB0 = createOrGetVPBB(TI->getSuccessor(0));
      assert(SuccVPBB0 && "Successor 0 not found");
      VPBasicBlock *SuccVPBB1 = createOrGetVPBB(TI->getSuccessor(1));
      assert(SuccVPBB1 && "Successor 1 not found");

      // Set VPBB's condition bit.
      assert(isa<BranchInst>(TI) && "Unsupported terminator!");
      auto *Br = cast<BranchInst>(TI);
      Value *BrCond = Br->getCondition();
      // Look up the branch condition to get the corresponding VPValue
      // representing the condition bit in VPlan (which may be in another VPBB).
      assert(IRDef2VPValue.count(BrCond) &&
             "Missing condition bit in IRDef2VPValue!");
      VPValue *VPCondBit = IRDef2VPValue[BrCond];
      VPBB->setTwoSuccessors(VPCondBit, SuccVPBB0, SuccVPBB1, Plan);

      VPBB->setCBlock(BB);
      VPBB->setTBlock(TI->getSuccessor(0));
      VPBB->setFBlock(TI->getSuccessor(1));

    } else {
      llvm_unreachable("Number of successors not supported");
    }

    // Set VPBB predecessors in the same order as they are in the incoming BB.
    setVPBBPredsFromBB(VPBB, BB);
  }

  // 3. Process outermost loop exit. We created an empty VPBB for the loop
  // exit BBs during the RPO traversal of the loop nest but their predecessors
  // have to be properly set. Since a loop exit may contain any Instruction,
  // related or not to the loop nest, we do not create VPInstructions for them.
  SmallVector<BasicBlock *, 2> LoopExits;
  TheLoop->getUniqueExitBlocks(LoopExits);
  for (BasicBlock *BB : LoopExits) {
    VPBasicBlock *VPBB = BB2VPBB[BB];
    // Loop exit was already set as successor of the loop exiting BB.
    // We only set its predecessor VPBB now.
    setVPBBPredsFromBB(VPBB, BB);
  }

  // 4. The whole CFG has been built at this point so all the input Values must
  // have a VPlan couterpart. Fix VPlan phi nodes by adding their corresponding
  // VPlan operands.
  fixPhiNodes();

  // 5. Final Top Region setup.
  // Create a dummy block as Top Region's entry
  VPBlockBase *RegionEntry =
      new VPBasicBlock(VPlanUtils::createUniqueName("BB"));
  ++TopRegionSize;
  RegionEntry->setParent(TopRegion);
  VPBlockUtils::connectBlocks(RegionEntry, PreheaderVPBB);

  // Create a dummy block as Top Region's exit
  VPBlockBase *RegionExit =
      new VPBasicBlock(VPlanUtils::createUniqueName("BB"));
  ++TopRegionSize;
  RegionExit->setParent(TopRegion);

  // Connect dummy Top Region's exit.
  if (LoopExits.size() == 1) {
    VPBasicBlock *LoopExitVPBB = BB2VPBB[LoopExits.front()];
    VPBlockUtils::connectBlocks(LoopExitVPBB, RegionExit);
  } else {
    // If there are multiple exits in the outermost loop, we need another dummy
    // block as landing pad for all of them.
    assert(LoopExits.size() > 1 && "Wrong number of exit blocks");

    VPBlockBase *LandingPad =
        new VPBasicBlock(VPlanUtils::createUniqueName("BB"));
    ++TopRegionSize;
    LandingPad->setParent(TopRegion);

    // Connect multiple exits to landing pad
    for (auto ExitBB : make_range(LoopExits.begin(), LoopExits.end())) {
      VPBasicBlock *ExitVPBB = BB2VPBB[ExitBB];
      VPBlockUtils::connectBlocks(ExitVPBB, LandingPad);
    }

    // Connect landing pad to Top Region's exit
    VPBlockUtils::connectBlocks(LandingPad, RegionExit);
  }

  TopRegion->setEntry(RegionEntry);
  TopRegion->setExit(RegionExit);
  TopRegion->setSize(TopRegionSize);

  return TopRegion;
}

VPRegionBlock *VPlanHCFGBuilder::buildPlainCFG() {
  PlainCFGBuilder PCFGBuilder(TheLoop, LI, Legal, Plan);
  VPRegionBlock *TopRegion = PCFGBuilder.buildPlainCFG();
  return TopRegion;
}

