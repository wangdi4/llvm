//===-- VPlanAllZeroBypass.cpp ----------------------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file implements the VPlanAllZeroBypass class which contains the public
/// interfaces to insert all-zero bypasses around divergent regions. Regions
/// can be either loops or a set of blocks executed under the same mask.
///
//===----------------------------------------------------------------------===//

#include "IntelVPlanAllZeroBypass.h"
#include "IntelVPlanCostModelProprietary.h"
#include "IntelVPlanUtils.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"

#define DEBUG_TYPE "VPlanAllZeroBypass"

using namespace llvm;

// RegionThreshold is specified as the minimum cost of the region in order to
// insert the bypass.
static cl::opt<unsigned> RegionThreshold(
    "vplan-all-zero-bypass-region-threshold", cl::init(125), cl::Hidden,
    cl::desc("Tune bypass insertion based on cost of instructions in region"));

namespace llvm {
namespace vpo {

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
void VPlanAllZeroBypass::dumpBypassRegionLiveOuts(
    VPBasicBlock *FirstBlockInBypassRegion,
    VPBasicBlock *LastBlockInBypassRegion,
    LiveOutUsersMapTy &LiveOutMap) {
    dbgs() << "LiveOuts for region " << FirstBlockInBypassRegion->getName();
    dbgs() << " -> " << LastBlockInBypassRegion->getName() << "\n";
    for (auto It : LiveOutMap) {
      VPValue *LiveOutVal = It.first;
      LiveOutUsersTy LiveOutUsers = It.second;
      dbgs() << "LiveOutVal: " << *LiveOutVal;
      for (auto *User : LiveOutUsers)
        dbgs() << "LiveOutUser: " << *User;
    }
}
#endif

void VPlanAllZeroBypass::getRegionBlocks(
    VPBasicBlock *FirstBlockInBypassRegion,
    VPBasicBlock *LastBlockInBypassRegion,
    SmallPtrSetImpl<VPBasicBlock *> &RegionBlocks) {
  for (auto *Block : sese_depth_first(FirstBlockInBypassRegion,
                                      LastBlockInBypassRegion))
    RegionBlocks.insert(Block);
}

void VPlanAllZeroBypass::collectRegionLiveOuts(
    VPBasicBlock *FirstBlockInBypassRegion,
    VPBasicBlock *LastBlockInBypassRegion,
    LiveOutUsersMapTy &LiveOutMap) {

  SmallPtrSet<VPBasicBlock *, 4> RegionBlocks;
  getRegionBlocks(FirstBlockInBypassRegion, LastBlockInBypassRegion,
                  RegionBlocks);
  for (auto *RegionBlock : RegionBlocks) {
    for (auto &VPInst : *RegionBlock) {
      for (auto *User : VPInst.users()) {
        VPInstruction *UserInst = dyn_cast<VPInstruction>(User);
        if ((UserInst &&
             RegionBlocks.count(UserInst->getParent()) == 0) ||
             isa<VPExternalUse>(User))
          LiveOutMap[&VPInst].push_back(User);
      }
    }
  }

  LLVM_DEBUG(dumpBypassRegionLiveOuts(FirstBlockInBypassRegion,
                                      LastBlockInBypassRegion, LiveOutMap));
}

void VPlanAllZeroBypass::createLiveOutPhisAndReplaceUsers(
    VPBasicBlock *LastBlockInBypassRegion,
    VPBasicBlock *BypassBegin,
    VPBasicBlock *BypassEnd,
    LiveOutUsersMapTy &LiveOutMap) {

  auto *DA = Plan.getVPlanDA();
  Builder.setInsertPoint(BypassEnd, BypassEnd->begin());
  // Create a new phi at the end of the split for all values live-out of
  // the bypassed block and replace all users with this phi.
  for (auto It : LiveOutMap) {
    VPValue *LiveOut = It.first;
    LiveOutUsersTy &LiveOutUsers = It.second;
    VPPHINode *VPPhi = Builder.createPhiInstruction(LiveOut->getType());
    DA->updateVectorShape(VPPhi, DA->getVectorShape(LiveOut));
    VPPhi->addIncoming(LiveOut, LastBlockInBypassRegion);
    // Propagate 0/false along all-zero edge because live-outs could feed
    // into instructions used as block-predicates and these values cannot
    // be undef.
    VPValue *ValueOnAllZeroEdge =
        Plan.getVPConstant(Constant::getNullValue(LiveOut->getType()));
    assert(VPPhi->getParent()->getNumPredecessors() == 2 &&
           "Parent block of phi is expected to have 2 predecessors for bypass");
    VPPhi->addIncoming(ValueOnAllZeroEdge, BypassBegin);
    for (auto *User : LiveOutUsers)
      User->replaceUsesOfWith(LiveOut, VPPhi);
  }
}

void VPlanAllZeroBypass::insertBypassForRegion(
    VPBasicBlock *FirstBlockInBypassRegion,
    VPBasicBlock *LastBlockInBypassRegion,
    VPDominatorTree *DT,
    VPPostDominatorTree *PDT,
    VPLoopInfo *VPLI) {

  // Cannot be nullptr since the collection phase guarantees that a
  // block-predicate exists for FirstBlockInBypassRegion.
  VPValue *BlockPredicate = FirstBlockInBypassRegion->getPredicate();
  assert(BlockPredicate &&
         "Block predicate not found for first block in bypass region.");
  // VPBlockUtils::splitBlockAtPredicate, like other VPBlockUtils::splitBlock()
  // methods, inserts the new block as a successor of the one provided in the
  // call. Adjust the pointers/names of the blocks accordingly so that the CFG
  // is constructed correctly. Keeping original names of blocks helps with
  // debugging.
  std::string FirstBlockName = FirstBlockInBypassRegion->getName().str();
  bool SingleBlockRegion = FirstBlockInBypassRegion == LastBlockInBypassRegion;
  VPBasicBlock *BypassBegin =
      VPBlockUtils::splitBlockAtPredicate(FirstBlockInBypassRegion, VPLI,
                                          DT, PDT);
  std::swap(BypassBegin, FirstBlockInBypassRegion);
  BypassBegin->setName(VPlanUtils::createUniqueName("all.zero.bypass.begin"));
  // Keep original name of the first block in the region for debugging.
  FirstBlockInBypassRegion->setName(FirstBlockName);
  if (SingleBlockRegion)
    LastBlockInBypassRegion = FirstBlockInBypassRegion;

  // LastBlockInBypassRegion is located inside the bypass region. Split a new
  // block to serve as the bypass target.
  VPBasicBlock *BypassEnd =
      VPBlockUtils::splitBlockEnd(LastBlockInBypassRegion, VPLI, DT, PDT);
  BypassEnd->setName(VPlanUtils::createUniqueName("all.zero.bypass.end"));

  Builder.setInsertPoint(BypassBegin);
  VPValue *AllZeroCheck = Builder.createAllZeroCheck(BlockPredicate,
                                                     "all.zero.check");
  auto *DA = Plan.getVPlanDA();
  DA->markUniform(*AllZeroCheck);
  // No need to update dominator info because bypass regions are inserted from
  // the outermost region to innermost region. Any blocks inserted previously
  // will not affect the CFG of the current region.
  BypassBegin->setTerminator(BypassEnd, FirstBlockInBypassRegion, AllZeroCheck);

  // Map of VPValues that are live-out of the bypass region and the
  // corresponding users to be updated after bypass insertion.
  LiveOutUsersMapTy LiveOutMap;
  collectRegionLiveOuts(FirstBlockInBypassRegion,
                        LastBlockInBypassRegion, LiveOutMap);

  // Create new phis for live-out values and replace users.
  createLiveOutPhisAndReplaceUsers(LastBlockInBypassRegion, BypassBegin,
                                   BypassEnd, LiveOutMap);
}

// Returns true if predicate MaybePred is an anded part of BaseCond.
bool VPlanAllZeroBypass::isStricterOrEqualPred(const VPValue *MaybePred,
                                               const VPValue *BaseCond) {
  if (MaybePred == nullptr)
    return false;

  if (MaybePred == BaseCond)
    return true;

  const VPInstruction *MaybePredInst = dyn_cast<VPInstruction>(MaybePred);
  const VPInstruction *BaseCondInst = dyn_cast<VPInstruction>(BaseCond);
  // Handle potential case where the block-predicate of a loop header is a
  // phi and one of the incoming values to it is the block-predicate of the
  // preheader. For example:
  //
  // preheader:
  //  %pred = block-predicate %0
  //  br %header
  //
  // header:
  //  %iv = phi
  //  %mask = phi [ %0 ],[ %mask.next ]
  //  %pred.header = block-predicate %mask
  //
  if (BaseCondInst && MaybePredInst &&
      MaybePredInst->getOpcode() == Instruction::PHI) {
    const VPBasicBlock *MaybePredParent = MaybePredInst->getParent();
    const VPBasicBlock *BaseCondParent = BaseCondInst->getParent();
    VPLoopInfo *VPLI = Plan.getVPLoopInfo();
    VPLoop *VPLp = VPLI->getLoopFor(MaybePredParent);
    VPBasicBlock *Preheader = VPLp->getLoopPreheader();
    VPBasicBlock *Header = VPLp->getHeader();
    if (BaseCondParent == Preheader && MaybePredParent == Header) {
      const VPPHINode *HeaderPhi = cast<VPPHINode>(MaybePredInst);
      VPValue *PreheaderPred = Preheader->getBlockPredicate();
      if (PreheaderPred == BaseCond)
        return HeaderPhi->getIncomingValue(Preheader) == BaseCond;
    }
  }
  if (MaybePredInst && MaybePredInst->getOpcode() == Instruction::And) {
    return (isStricterOrEqualPred(MaybePredInst->getOperand(0), BaseCond) ||
            isStricterOrEqualPred(MaybePredInst->getOperand(1), BaseCond));
  }
  // If both parts of 'or' are anded with BaseCond, then we're safe to
  // extend the bypass region to include the block containing MaybePred.
  if (MaybePredInst && MaybePredInst->getOpcode() == Instruction::Or) {
    return (isStricterOrEqualPred(MaybePredInst->getOperand(0), BaseCond) &&
            isStricterOrEqualPred(MaybePredInst->getOperand(1), BaseCond));
  }

  return false;
}

static void getUnpredicatedInstructions(
    VPBasicBlock *Block,
    SmallVectorImpl<VPInstruction *> &UnpredicatedInsts) {
  for (auto &VPInst : *Block) {
    if (VPInst.getOpcode() == VPInstruction::Pred)
      break;
    UnpredicatedInsts.push_back(&VPInst);
  }
}

void VPlanAllZeroBypass::verifyBypassRegion(
    VPBasicBlock *FirstBlockInRegion,
    SmallPtrSetImpl<VPBasicBlock *> &RegionBlocks) {
  for (auto *RegionBlock : RegionBlocks) {
    // For any phis in the region we want to assert that any incoming blocks
    // to those phis are part of the region. This will catch if all-zero
    // bypass attempts to construct invalid regions. However, we don't want
    // to assert if the block containing the phi(s) is also the first block
    // in the region. In this case, the instructions appearing before
    // before the block predicate, including phis, will not be included in
    // the region because they will be hoisted above the all-zero check.
    if (RegionBlock == FirstBlockInRegion)
      continue;
    for (auto &VPInst : *RegionBlock) {
      if (auto *VPPhi = dyn_cast<VPPHINode>(&VPInst)) {
        for (unsigned I = 0; I < VPPhi->getNumIncomingValues(); ++I) {
          assert(RegionBlocks.count(VPPhi->getIncomingBlock(I)) &&
                 "Incoming block to phi is not in the same region.");
        }
      }
    }
  }
}

bool VPlanAllZeroBypass::regionFoundForBlock(
    VPBasicBlock *Block,
    VPValue *Pred,
    RegionsCollectedTy &RegionsCollected) {
  for (auto &Region :
       make_range(RegionsCollected.lower_bound(Pred),
                  RegionsCollected.upper_bound(Pred))) {
    auto &CollectedRegionBlocks = Region.second;
    if (CollectedRegionBlocks.count(Block))
      return true;
  }
  return false;
}

void VPlanAllZeroBypass::collectAllZeroBypassLoopRegions(
    AllZeroBypassRegionsTy &AllZeroBypassRegions,
    RegionsCollectedTy &RegionsCollected) {
  VPLoopInfo *VPLI = Plan.getVPLoopInfo();
  SmallPtrSet<VPBasicBlock *, 4> RegionBlocks;
  for (auto *VPLp : VPLI->getLoopsInPreorder()) {
    VPBasicBlock *Preheader = VPLp->getLoopPreheader();
    VPValue *PreheaderPred = Preheader ? Preheader->getPredicate() : nullptr;
    // Loop is not predicated, so skip to the next one. E.g., outermost loop.
    if (!PreheaderPred)
      continue;
    // Try to extend loop region by going up the single predecessor chain
    // and checking for predicate instructions that are anded with the
    // preheader block-predicate.
    VPBasicBlock *RegionBegin = Preheader;
    VPBasicBlock *SinglePred = Preheader->getSinglePredecessor();
    while (SinglePred) {
      VPValue *SinglePredPred = SinglePred->getPredicate();
      if (SinglePredPred &&
          !isStricterOrEqualPred(SinglePredPred, PreheaderPred))
        break;
      SmallVector<VPInstruction *, 4> UnpredicatedInsts;
      getUnpredicatedInstructions(SinglePred, UnpredicatedInsts);
      if (any_of(UnpredicatedInsts,
                 [PreheaderPred, this] (const VPInstruction *VPInst) {
             return (!isa<VPBranchInst>(VPInst) &&
                     !isStricterOrEqualPred(VPInst, PreheaderPred));
         }))
        break;
      RegionBegin = SinglePred;
      SinglePred = SinglePred->getSinglePredecessor();
    }

    // Going back up the single predecessor chain may have left us at a
    // block without a block-predicate. E.g., a block containing the def
    // of the preheader block-predicate as an 'and' instruction. In case
    // something like that happens, roll forward to the nearest successor
    // that has a block-predicate defined.
    while (!RegionBegin->getPredicate())
      RegionBegin = RegionBegin->getSingleSuccessor();

    // Try to extend loop region by going down single successor chain and
    // checking for predicate instructions that are anded with the preheader
    // block-predicate.
    VPBasicBlock *Exit = VPLp->getExitBlock();
    VPBasicBlock *RegionEnd = Exit;
    VPBasicBlock *SingleSucc = Exit->getSingleSuccessor();
    while (SingleSucc) {
      VPValue *SingleSuccPred = SingleSucc->getPredicate();
      if (SingleSuccPred &&
          !isStricterOrEqualPred(SingleSuccPred, PreheaderPred))
        break;
      SmallVector<VPInstruction *, 4> UnpredicatedInsts;
      getUnpredicatedInstructions(SingleSucc, UnpredicatedInsts);
      if (any_of(UnpredicatedInsts,
                 [PreheaderPred, this] (const VPInstruction *VPInst) {
             return (!isa<VPBranchInst>(VPInst) &&
                     !isStricterOrEqualPred(VPInst, PreheaderPred));
         }))
        break;
      RegionEnd = SingleSucc;
      SingleSucc = SingleSucc->getSingleSuccessor();
    }

    getRegionBlocks(RegionBegin, RegionEnd, RegionBlocks);
    BypassPairTy BypassRegion = std::make_pair(RegionBegin, RegionEnd);
    verifyBypassRegion(RegionBegin, RegionBlocks);
    AllZeroBypassRegions.push_back(BypassRegion);
    RegionsCollected.insert(std::make_pair(PreheaderPred, RegionBlocks));
  }
}

bool VPlanAllZeroBypass::blendTerminatesRegion(const VPBlendInst *Blend,
                                               VPValue *RegionPred) {
  for (unsigned I = 0, E = Blend->getNumIncomingValues(); I != E; ++I) {
    VPValue *IncomingPred = Blend->getIncomingPredicate(I);
    if (!isStricterOrEqualPred(IncomingPred, RegionPred))
      return true;
  }

  return false;
}

void VPlanAllZeroBypass::collectAllZeroBypassNonLoopRegions(
    AllZeroBypassRegionsTy &AllZeroBypassRegions,
    RegionsCollectedTy &RegionsCollected,
    VPlanCostModelProprietary *CM) {

  VPLoopInfo *VPLI = Plan.getVPLoopInfo();

  // Detect blocks that are candidates to begin a bypass region. Candidate
  // blocks are those that contain block-predicates, have a single successor,
  // and have not already been included within a loop bypass region. Candidates
  // must have a single successor because the RPO iterator below assumes this
  // during region formation. We may later choose to relax this assumption if it
  // is possible to begin a region with a block containing multiple successors,
  // but this seems unlikely.
  SmallVector<VPBasicBlock *, 16> CandidateBlocks;
  ReversePostOrderTraversal<VPBasicBlock *> RPOT(Plan.getEntryBlock());
  for (auto *Block : RPOT) {
    VPValue *BlockPred = Block->getPredicate();
    if (!BlockPred || !Block->getSingleSuccessor() ||
        regionFoundForBlock(Block, BlockPred, RegionsCollected))
      continue;
    CandidateBlocks.push_back(Block);
  }

  // Build non-loop bypass regions from outermost to innermost. This makes it
  // easier to prevent insertion of redundant regions.
  for (auto *CandidateBlock : CandidateBlocks) {
    assert(CandidateBlock->getSingleSuccessor() &&
           "Bypass region should start from linearized control flow");

    // CandidateBlockPred will never be null here due to previous nullptr
    // check for CandidateBlocks insertion above, but assert just in case.
    VPValue *CandidateBlockPred = CandidateBlock->getPredicate();
    assert(CandidateBlockPred &&
           "Candidate block for bypass should have a predicate");

    // Collect blocks for the region on-the-fly so we can check to make sure
    // incoming blocks for phis are in the region.
    SmallPtrSet<VPBasicBlock *, 4> RegionBlocks;

    // Candidate Block is included in the region as the first and last in the
    // region.
    VPBasicBlock *LastBB = CandidateBlock;
    RegionBlocks.insert(CandidateBlock);
    // Re-use existing RPOT beginning at VPlan entry. Advance RPOT iterator
    // to the single successor of CandidateBlock to begin candidate region
    // collection.
    auto BlockIt = RPOT.begin();
    while (*BlockIt != CandidateBlock->getSingleSuccessor())
      ++BlockIt;
    while (BlockIt != RPOT.end()) {
      // Process loops here. Record all loop blocks in RegionBlocks and advance
      // the RPO iterator to the next block after the exit and continue trying
      // to expand the region. Since this function collects non-loop regions,
      // the loop detection here is used to expand the region around the loop
      // only and is not necessarily used for stability. Although, if a larger
      // region is formed around this loop using the same block-predicate, then
      // inserting another region around the loop is redundant and can be
      // eliminated. We collect the set of LoopBlocks all at once, rather than
      // trying to follow block-predicates, for several reasons:
      // 1) Latch for divergent inner loops will not have a block-predicate
      //    because LoopCFU will make the loop exit condition uniform. However,
      //    there can still be a divergent top test entering such loops, so they
      //    can have a bypass.
      // 2) There's also really no need to traverse all blocks in the region
      //    and match the block-predicate when we can simply just add all blocks
      //    of a divergent loop at once.
      // 3) Loop bypasses are needed for stability and no cost modeling is done
      //    for them (see collectAllZeroBypassLoopRegions()).
      // 4) This allows the rest of the region collection algorithm to not have
      //    to deal with backedges. All predecessors of a block can easily be
      //    checked that they reside in the region.
      if (VPLI->isLoopHeader(*BlockIt) &&
          isStricterOrEqualPred((*BlockIt)->getPredicate(),
                                CandidateBlockPred)) {
        VPLoop *VPLp = VPLI->getLoopFor(*BlockIt);
        VPBasicBlock *LoopExit = VPLp->getExitBlock();
        SmallPtrSet<VPBasicBlock *, 4> LoopBlocks;
        RegionBlocks.insert(LoopBlocks.begin(), LoopBlocks.end());
        while (*BlockIt != LoopExit)
          LastBB = *(++BlockIt);
        ++BlockIt;
      }

      // Bypass region will include blocks that only compute predicates and
      // blocks where the block-predicate is anded with the one from the block
      // that will start the bypass (CandidateBlock). For example, after
      // predication, we can have the following VPlan snippet. Assuming the
      // region will include blocks beginning at BB20, the block-predicate
      // forming the all-zero bypass will be %vp7392. As long as each
      // subsequent block uses %vp7392 as a block-predicate or is anded with
      // it (or is an empty block), the region will include that block. So,
      // here a region can be formed around all blocks, BB20, BB6, and BB24.
      // Otherwise, the region ends at the block that is the predecessor of
      // where these conditions are not met.
      //
      // BB20:
      //  [DA: Div] i1 %vp7648 = block-predicate i1 %vp7392
      // SUCCESSORS(1):BB6
      // PREDECESSORS(1): BB23
      //
      // BB6:
      //  [DA: Div] i1 %vp41712 = block-predicate i1 %vp7392
      //  [DA: Div] i1 %vp18976 = icmp i32 %vp26608 i32 24
      //  [DA: Div] i1 %vp41904 = not i1 %vp18976
      // SUCCESSORS(1):BB24
      // PREDECESSORS(1): BB20
      //
      // BB24:
      //  [DA: Div] i1 %vp42384 = and i1 %vp7392 i1 %vp41904
      //  [DA: Div] i1 %vp42800 = and i1 %vp7392 i1 %vp18976
      // SUCCESSORS(1):BB9
      // PREDECESSORS(1): BB6
      //
      // In addition, there are currently no restrictions in VPlan to prevent
      // the following from happening where we have two blocks under the same
      // predicate, but there are other non-predicated instructions between
      // these blocks. Here, it would be incorrect to include both BB0 and BB1
      // within the same bypass region. In this case, two separate regions
      // should be formed individually around each block, preventing the store
      // from being bypassed if %pred is all-zero.
      //
      // BB0:
      //  block-predicate %pred
      //  br label %BB1
      //
      // BB1:
      //  store %x, %ptr ; This is *unmasked*, can't be skipped.
      //  block-predicate %pred
      //  br label %BB2
      //
      // BB2:
      //  ...
      SmallVector<VPInstruction *, 4> UnpredicatedInsts;
      getUnpredicatedInstructions(*BlockIt, UnpredicatedInsts);
      if (any_of(UnpredicatedInsts,
                 [CandidateBlockPred, this] (const VPInstruction *VPInst) {
                   if (const VPBlendInst *Blend = dyn_cast<VPBlendInst>(VPInst))
                     return blendTerminatesRegion(Blend, CandidateBlockPred);
                   // Skip phi nodes here because even though they are
                   // unpredicated we will use a separate check to ensure that
                   // all predecessors of the block containing the phi belong
                   // to the region.
                   return (!isa<VPPHINode>(VPInst) &&
                           !isa<VPBranchInst>(VPInst) &&
                           !isStricterOrEqualPred(VPInst, CandidateBlockPred));
               }))
        break;

      // All predecessors of the current block should already be included
      // in the region. Loop latches are pre-recorded above to allow this
      // check to be done here.
      if (any_of((*BlockIt)->getPredecessors(),
                 [&RegionBlocks] (VPBasicBlock *Pred) {
                  return !RegionBlocks.count(Pred);
                }))
        break;

      // Stop region collection for any blocks that don't have a block-
      // predicate and have reference to a CondBit. For now, it is
      // conservatively assumed that these blocks should not appear in
      // the region. TODO: check post-dominator of this block to see
      // if there is a block-predicate that is under the influence of
      // the block-predicate of the region. Reference CMPLRLLVM-20647.
      //
      // BB3:
      //  [DA: Div] i1 %vp27040 = block-predicate i1 %vp64112
      //  [DA: Div] i32 %vp25424 = add i32 %vp57152 i32 42
      // SUCCESSORS(1):BB4
      // PREDECESSORS(1): BB2
      //
      // BB4:
      //  <VPTerminator>
      //  Condition(BB1): [DA: Uni] i1 %vp57584 = icmp i32 %n i32 7
      // SUCCESSORS(2):BB5(i1 %vp57584), BB6(!i1 %vp57584)
      // PREDECESSORS(1): BB3
      if (!(*BlockIt)->getPredicate() && (*BlockIt)->getNumSuccessors() == 2)
        break;

      if (auto *BlockPred = (*BlockIt)->getPredicate())
        if (!isStricterOrEqualPred(BlockPred, CandidateBlockPred))
          break;

      LastBB = *BlockIt;
      RegionBlocks.insert(LastBB);
      ++BlockIt;
    }

    // Check to see if this region is inside another region that already
    // uses the same block-predicate to form the all-zero bypass. If so,
    // then the region is unnecessary.
    if (regionFoundForBlock(CandidateBlock, CandidateBlockPred,
                            RegionsCollected))
      continue;

    // Cost model not yet available for function vectorization pipeline. It's
    // ok because there's really no reason for it there yet anyway since this
    // pipeline is only used for testing at the moment.
    unsigned RegionCost = RegionThreshold;
    if (CM)
      RegionCost = CM->getBlockRangeCost(CandidateBlock, LastBB);

    // If the region meets minimum cost requirements, record it for later
    // insertion.
    if (RegionCost >= RegionThreshold) {
      BypassPairTy BypassRegion = std::make_pair(CandidateBlock, LastBB);
      verifyBypassRegion(CandidateBlock, RegionBlocks);
      AllZeroBypassRegions.push_back(BypassRegion);
      RegionsCollected.insert(std::make_pair(CandidateBlockPred, RegionBlocks));
    }
  }
}

void VPlanAllZeroBypass::insertAllZeroBypasses(
    AllZeroBypassRegionsTy &AllZeroBypassRegions) {
  //TODO: remove DT/PDT compute calls when predicator DT/PDT is updated
  // correctly for VPlan.
  Plan.computeDT();
  Plan.computePDT();
  VPDominatorTree *DT = Plan.getDT();
  VPPostDominatorTree *PDT = Plan.getPDT();
  VPLoopInfo *VPLI = Plan.getVPLoopInfo();
  for (auto BypassRegion : AllZeroBypassRegions)
    insertBypassForRegion(BypassRegion.first, BypassRegion.second, DT, PDT,
                          VPLI);
}

} // end namespace vpo
} // end namespace llvm
