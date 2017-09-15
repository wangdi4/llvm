//===-- VPlanVerifier.cpp --------------------------------------------------===//
//
//   Copyright (C) 2015-2017 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//
//
// This file defines VPlanVerifier class that is used to verify that several
// aspect of a VPlan are correct.
//
//===----------------------------------------------------------------------===//

#include "VPlanVerifier.h"

#define DEBUG_TYPE "vplan-verifier"

using namespace llvm;
using namespace vpo;

static cl::opt<bool> DisableHCFGVerification(
    "vplan-disable-verification", cl::init(false),
    cl::desc("Disable VPlan HCFG verification"));


// Verify that Block is contained in the right VPLoop.
void VPlanVerifier::verifyContainerLoop(const VPBlockBase *Block,
                                        const VPLoopRegion *ParentLoopR) const {
  const VPLoop *ContainerLoop = nullptr;

  if (ParentLoopR) {
    if (ParentLoopR->getEntry() == Block || ParentLoopR->getExit() == Block)
      // If Block is parent LoopRegion's Entry or Exit, Block shouldn't be
      // contained in this loop but in ParentRegion's parent loop (if any).
      ContainerLoop = VPLInfo->getLoopFor(Block->getParent());
    else
      // Block should be contained in parent loop.
      ContainerLoop = ParentLoopR->getVPLoop();
  }

  if (ContainerLoop)
    assert(ContainerLoop->contains(Block) &&
           "Block is not contained in the right loop");
  else
    // Check that the loop is not contained in any loop.
    assert(!VPLInfo->getLoopFor(Block) &&
           "Block should not be contained in any VPLoop");
}

// Verify VPLoop information in \p LoopRegion.
void VPlanVerifier::verifyVPLoopInfo(const VPLoopRegion *LoopRegion) const {

  const VPLoop *Loop = LoopRegion->getVPLoop();
  assert(LoopRegion && "Missing VPLoop for VPLoopRegion");

  const VPBlockBase *Preheader = LoopRegion->getEntry();
  assert(Preheader && Preheader == Loop->getLoopPreheader() &&
         "Wrong loop preheader");
  const VPBlockBase *Header = Preheader->getSingleSuccessor();
  assert(Header && "Loop preheader must have a single successor");
  assert(Header == Loop->getHeader() && "Wrong loop header");

  assert((VPLInfo->getLoopFor(Header) == Loop) &&
         "Unexpected loop from loop header");

  (void) Loop;
  (void) Header;
}

// Verify information of LoopRegions nested in \p Region.
void VPlanVerifier::verifyLoopRegions(const VPRegionBlock *TopRegion) const {

  // VerifyLoopRegions implementation.
  typedef std::function<void(const VPRegionBlock *, const VPLoopRegion *)> RT;
  RT verifyLoopRegionsImp = [&](const VPRegionBlock *Region,
                                const VPLoopRegion *ParentLoopR) {

    assert(Region && "Region cannot be null");

    // If Region is a LoopRegion, Region is the new ParentLoopR.

    if (const auto *LoopR = dyn_cast<VPLoopRegion>(Region))
      ParentLoopR = LoopR;

    // Visit Region's CFG
    for (const VPBlockBase *VPB :
         make_range(df_iterator<const VPRegionBlock *>::begin(Region),
                    df_iterator<const VPRegionBlock *>::end(Region))) {
      // If subregion is a loop region, use it as ParentLoop in the visit
      if (const auto *LoopRegion = dyn_cast<VPLoopRegion>(VPB)) {
        verifyVPLoopInfo(LoopRegion);
      }

      verifyContainerLoop(VPB, ParentLoopR);
    }

    // Visit SubRegion
    for (const VPBlockBase *VPB :
         make_range(df_iterator<const VPRegionBlock *>::begin(Region),
                    df_iterator<const VPRegionBlock *>::end(Region))) {

      if (const auto *SR = dyn_cast<VPRegionBlock>(VPB)) {
        verifyLoopRegionsImp(SR, ParentLoopR);
      }
    }
  };

  verifyLoopRegionsImp(TopRegion, nullptr /*ParentLoopRegion*/);
}

// Count the number of VPLoopRegion's nested in \p Region.
static unsigned countLoopRegionsInRegion(const VPRegionBlock *Region) {

  unsigned NumLoops = 0;

  for (const VPBlockBase *VPB :
       make_range(df_iterator<const VPBlockBase *>::begin(Region->getEntry()),
                  df_iterator<const VPBlockBase *>::end(Region->getExit()))) {

    if (isa<VPLoopRegion>(VPB))
      ++NumLoops;

    // Count nested VPLoops
    if (const VPRegionBlock *VPR = dyn_cast<VPRegionBlock>(VPB))
      NumLoops += countLoopRegionsInRegion(VPR);
  }

  return NumLoops;
}

// Count the number of VPLoop's nested in Loop \p Lp.
template <class LoopT> static unsigned countLoopsInLoop(const LoopT *Lp) {

  const std::vector<LoopT *> &SubLoops = Lp->getSubLoops();
  unsigned NumLoops = SubLoops.size();

  for (const LoopT *SL : SubLoops)
    NumLoops += countLoopsInLoop(SL);

  return NumLoops;
}

// Verify that TopRegion contains the same number of loops (VPLoopRegion) as
// VPLoopInfo and LoopInfo.
void VPlanVerifier::verifyNumLoops(const VPRegionBlock *TopRegion) const {

  // Compare number of loops in HCFG with loops in VPLoopInfo and LoopInfo
  unsigned NumLoopsInCFG = countLoopRegionsInRegion(TopRegion);

  assert(VPLInfo->size() && "More than one top loop is not expected");
  unsigned NumLoopsInVPLoopInfo =
      1 /*TopLoop*/ + countLoopsInLoop<VPLoop>(*VPLInfo->begin());
  unsigned NumLoopsInLoopInfo = 1 /*TopLoop*/ + countLoopsInLoop<Loop>(TheLoop);

  // dbgs().indent(2) << "Verify Loops:\n";
  // dbgs().indent(4) << "NumLoopsInCFG: " << NumLoopsInCFG << "\n";
  // dbgs().indent(4) << "NumLoopsInVPLoopInfo: " << NumLoopsInVPLoopInfo <<
  // "\n";
  // dbgs().indent(4) << "NumLoopsInLoopInfo: " << NumLoopsInLoopInfo << "\n";

  assert(NumLoopsInCFG == NumLoopsInVPLoopInfo &&
         NumLoopsInVPLoopInfo == NumLoopsInLoopInfo &&
         "Number of loops in HCFG, VPLoopInfo and LoopInfo don't match");

  (void) NumLoopsInCFG;
  (void) NumLoopsInVPLoopInfo;
  (void) NumLoopsInLoopInfo;
}

// Main class to verify loop information.
void VPlanVerifier::verifyLoops(const VPRegionBlock *TopRegion) const {
  assert(TheLoop && VPLInfo && LInfo && "Loop information is null.");

  verifyNumLoops(TopRegion);
  verifyLoopRegions(TopRegion);
}

// Main function for VPRegionBlock verification.
void VPlanVerifier::verifyRegions(const VPRegionBlock *Region) const {

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
  (void) Entry;
  (void) Exit;

  // Traverse Region's blocks
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
      (void) SuccPreds;
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
      (void)PredSuccs;
    }
  }

  assert(NumBlocks == Region->getSize() && "Region has a wrong size");

  // Visit subregions
  for (const VPBlockBase *VPB :
       make_range(df_iterator<const VPBlockBase *>::begin(Region->getEntry()),
                  df_iterator<const VPBlockBase *>::end(Region->getExit()))) {
    if (const auto *SubRegion = dyn_cast<VPRegionBlock>(VPB))
      verifyRegions(SubRegion);
  }
}

// Public interface to verify the hierarchical CFG.
void VPlanVerifier::verifyHierarchicalCFG(
    const VPRegionBlock *TopRegion) const {

  if (DisableHCFGVerification)
    return;

  assert(((!VPLInfo && !LInfo && !TheLoop) || (VPLInfo && LInfo && TheLoop)) &&
         "TheLoop, VPLInfo and LInfo must be all set (or not)");

  // dbgs() << "Verifying Hierarchical CFG:\n";

  if (VPLInfo && LInfo && TheLoop)
    verifyLoops(TopRegion);

  verifyRegions(TopRegion);
}

