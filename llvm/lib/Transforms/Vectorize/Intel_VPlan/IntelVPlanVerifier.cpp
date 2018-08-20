//===-- IntelVPlanVerifier.cpp --------------------------------------------===//
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
// aspects of a VPlan are correct.
//
//===----------------------------------------------------------------------===//

#include "IntelVPlanVerifier.h"

#define DEBUG_TYPE "vplan-verifier"

using namespace llvm;
using namespace llvm::vpo;

static cl::opt<bool>
    DisableHCFGVerification("vplan-disable-verification", cl::init(false),
                            cl::desc("Disable VPlan H-CFG verification"));

// Verify that Block is contained in the right VPLoop.
void VPlanVerifier::verifyContainerLoop(
    const VPBlockBase *Block, const VPLoopRegion *ParentLoopR,
    const SmallPtrSetImpl<const VPBlockBase *> &ExternalBlocks) const {
  const VPLoop *ContainerLoop = nullptr;

  if (ParentLoopR) {
    if (ExternalBlocks.count(Block))
      // If Block is outside of the loop cyle, Block shouldn't be contained in
      // this loop but in ParentRegion's parent loop (if any).
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

/// Collect VPBlockBases that are outside of the loop cycle of the loop in \p
/// ParentLoopR. Return \p ExternalBlock with the external blocks. If \p
/// ExternalBlock is not empty, it is cleared before collecting the new external
/// blocks.
static void CollectLoopExternalBlocks(
    const VPLoopRegion *ParentLoopR, const VPLoopInfo *VPLInfo,
    SmallPtrSetImpl<const VPBlockBase *> &ExternalBlocks) {
  ExternalBlocks.clear();

  // Add loop PH to ExternalBlocks.
  ExternalBlocks.insert(ParentLoopR->getEntry());

  // Add VPBlockBases after loop exitings to ExternalBlocks.
  SmallVector<VPBlockBase *, 4> LoopExits;
  ParentLoopR->getVPLoop()->getUniqueExitBlocks(LoopExits);
  for (const auto *LpExit : LoopExits)
    for (const auto *Block :
         make_range(df_iterator<const VPBlockBase *>::begin(LpExit),
                    df_iterator<const VPBlockBase *>::end(LpExit))) {
      // TODO: Regions outside the loop cycle are not expected. Implement them
      // when necessary.
      assert(!isa<VPRegionBlock>(Block) && "Regions not supported!");
      ExternalBlocks.insert(Block);
    }
}

// Verify information of LoopRegions nested in \p Region.
void VPlanVerifier::verifyLoopRegions(
    const VPRegionBlock *TopRegion) const {

  // Hold blocks outside of the loop cycle of ParentLoopR.
  SmallPtrSet<const VPBlockBase *, 8> ExternalBlocks;

  // VerifyLoopRegions implementation.
  typedef std::function<void(const VPRegionBlock *, const VPLoopRegion *)> RT;
  RT verifyLoopRegionsImp = [&](const VPRegionBlock *Region,
                                const VPLoopRegion *ParentLoopR) {

    assert(Region && "Region cannot be null");

    if (const auto *LoopR = dyn_cast<VPLoopRegion>(Region)) {
      // If Region is a LoopRegion, Region is the new ParentLoopR.
      ParentLoopR = LoopR;
      CollectLoopExternalBlocks(ParentLoopR, VPLInfo, ExternalBlocks);

      // Checks for underlying-IR-specific information.
      verifyIRSpecificLoopRegion(LoopR);
    }

    // Visit Region's CFG
    for (const VPBlockBase *VPB :
         make_range(df_iterator<const VPRegionBlock *>::begin(Region),
                    df_iterator<const VPRegionBlock *>::end(Region))) {
      // If subregion is a loop region, use it as ParentLoop in the visit
      if (const auto *LoopRegion = dyn_cast<VPLoopRegion>(VPB)) {
        verifyVPLoopInfo(LoopRegion);
      }

      verifyContainerLoop(VPB, ParentLoopR, ExternalBlocks);
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

// Count the number of VPLoop's in \p Lp, including itself.
template <class LoopT> static unsigned countLoopsInLoop(const LoopT *Lp) {

  const std::vector<LoopT *> &SubLoops = Lp->getSubLoops();
  unsigned NumLoops = 1;

  for (const LoopT *SL : SubLoops)
    NumLoops += countLoopsInLoop(SL);

  return NumLoops;
}

// Verify that TopRegion contains the same number of loops (VPLoopRegion) as
// VPLoopInfo and LoopInfo.
void VPlanVerifier::verifyNumLoops(const VPRegionBlock *TopRegion) const {

  // Compare number of loops in H-CFG with loops in VPLoopInfo and LoopInfo
  unsigned NumLoopsInCFG = countLoopRegionsInRegion(TopRegion);

  assert(VPLInfo->size() && "More than one top loop is not expected");
  unsigned NumLoopsInVPLoopInfo = countLoopsInLoop<VPLoop>(*VPLInfo->begin());
  unsigned NumLoopsInIR = countLoopsInUnderlyingIR();

  assert(NumLoopsInCFG == NumLoopsInVPLoopInfo &&
         NumLoopsInVPLoopInfo == NumLoopsInIR &&
         "Number of loops in H-CFG, VPLoopInfo and underlying IR don't match");

  (void)NumLoopsInCFG;
  (void)NumLoopsInVPLoopInfo;
  (void)NumLoopsInIR;
}

// Main class to verify loop information.
void VPlanVerifier::verifyLoops(const VPRegionBlock *TopRegion) const {
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

    // Check block's ConditionBit
    if (VPB->getNumSuccessors() > 1)
      assert(VPB->getCondBit() && "Missing condition bit!");
    else
      assert(!VPB->getCondBit() && "Unexpected condition bit!");

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

    if (const auto *VPBB = dyn_cast<VPBasicBlock>(VPB))
      verifyBBInstrs(VPBB);

    // Recurse here so that we won't need to do the traversal twice.
    if (const auto *SubRegion = dyn_cast<VPRegionBlock>(VPB))
      verifyRegions(SubRegion);
  }

  assert(NumBlocks == Region->getSize() && "Region has a wrong size");
}

// Public interface to verify the hierarchical CFG.
void VPlanVerifier::verifyHierarchicalCFG(
    const VPRegionBlock *TopRegion) const {

  if (DisableHCFGVerification)
    return;

  LLVM_DEBUG(dbgs() << "Verifying Hierarchical CFG.\n");

  if (VPLInfo)
    verifyLoops(TopRegion);

  verifyRegions(TopRegion);
}

unsigned VPlanVerifier::countLoopsInUnderlyingIR() const {
  assert(TheLoop && "TheLoop can't be null.");
  return countLoopsInLoop<Loop>(TheLoop);
}

void VPlanVerifier::verifyOperands(const VPUser *U) {
  for (const VPValue *Op : U->operands()) {
    (void)Op;
    assert(Op && "Null operand found!");
    // We expect that for each Op->U edge there is a matching U->Op edge.
    assert(Op->getNumUsersTo(U) == U->getNumOperandsFrom(Op) &&
           "Op->U and U->Op do not match!");
  }
}

void VPlanVerifier::verifyUsers(const VPValue *Def) {
  for (const VPUser *U : Def->users()) {
    (void)U;
    // We expect that for each Def->U edge there is a matching U->Def edge.
    assert(Def->getNumUsersTo(U) == U->getNumOperandsFrom(Def) &&
           "Def->U and U->Def do not match!");
  }
}

void VPlanVerifier::verifyInstr(const VPUser *U) {
  verifyOperands(U);
  verifyUsers(U);
}

void VPlanVerifier::verifyBBInstrs(const VPBasicBlock *VPBB) {
  for (const VPRecipeBase &R : *VPBB) {
    if (const auto *I = dyn_cast<VPInstruction>(&R))
      verifyInstr(I);
  }
}
