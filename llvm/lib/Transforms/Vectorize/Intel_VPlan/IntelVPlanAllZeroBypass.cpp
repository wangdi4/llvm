//===-- VPlanAllZeroBypass.cpp ----------------------------------*- C++ -*-===//
// INTEL_CUSTOMIZATION
//
// INTEL CONFIDENTIAL
//
// Copyright (C) 2021 Intel Corporation
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you ("License"). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.
//
// end INTEL_CUSTOMIZATION
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
#include "IntelVPlanCostModel.h"
#include "IntelVPlanUtils.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"

#define DEBUG_TYPE "VPlanAllZeroBypass"

using namespace llvm;

// RegionThreshold is specified as the minimum cost of the region in order to
// insert the bypass.
static cl::opt<unsigned> RegionThreshold(
    "vplan-all-zero-bypass-region-threshold",
    cl::init(30), cl::Hidden,
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
    for (const auto &It : LiveOutMap) {
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
    SetVector<VPBasicBlock *> &RegionBlocks) {
  for (auto *Block : sese_depth_first(FirstBlockInBypassRegion,
                                      LastBlockInBypassRegion))
    RegionBlocks.insert(Block);
}

void VPlanAllZeroBypass::collectRegionLiveOuts(
    VPBasicBlock *FirstBlockInBypassRegion,
    VPBasicBlock *LastBlockInBypassRegion,
    LiveOutUsersMapTy &LiveOutMap) {

  SetVector<VPBasicBlock *> RegionBlocks;
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
  for (auto& It : LiveOutMap) {
    VPValue *LiveOut = It.first;
    LiveOutUsersTy &LiveOutUsers = It.second;
    VPPHINode *VPPhi = Builder.createPhiInstruction(LiveOut->getType());
    DA->updateVectorShape(VPPhi, DA->getVectorShape(*LiveOut));
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
    VPLoopInfo *VPLI,
    AllZeroBypassRegionsTy &AllZeroBypassRegionsFinal) {

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
  Plan.getVPlanDA()->markUniform(*AllZeroCheck);
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

  BypassPairTy BypassRegion = std::make_pair(BypassBegin, BypassEnd);
  AllZeroBypassRegionsFinal.push_back(BypassRegion);
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
  if (BaseCondInst && MaybePredInst &&
      MaybePredInst->getOpcode() == Instruction::PHI) {
    const VPBasicBlock *MaybePredParent = MaybePredInst->getParent();
    const VPBasicBlock *BaseCondParent = BaseCondInst->getParent();
    VPLoopInfo *VPLI = Plan.getVPLoopInfo();
    VPLoop *VPLp = VPLI->getLoopFor(MaybePredParent);
    assert(VPLp && "VPLoop object is expected to exist for the block");
    VPBasicBlock *Preheader = VPLp->getLoopPreheader();
    assert(Preheader && "Preheader is expected to exist for the loop");
    VPBasicBlock *Header = VPLp->getHeader();
    if (BaseCondParent == Preheader && MaybePredParent == Header) {
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
    SetVector<VPBasicBlock *> &RegionBlocks) {
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

bool VPlanAllZeroBypass::endRegionAtBlock(
    VPBasicBlock *Block,
    VPValue *CandidateBlockPred,
    SetVector<VPBasicBlock *> &RegionBlocks) {

  SmallVector<VPInstruction *, 4> UnpredicatedInsts;
  getUnpredicatedInstructions(Block, UnpredicatedInsts);
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
    return true;

  // All predecessors of the current block should already be included
  // in the region. Loop latches are pre-recorded above to allow this
  // check to be done here.
  if (any_of(Block->getPredecessors(),
             [&RegionBlocks] (VPBasicBlock *Pred) {
              return !RegionBlocks.count(Pred);
            }))
    return true;

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
  if (!Block->getPredicate() && Block->getNumSuccessors() == 2)
    return true;

  if (auto *BlockPred = Block->getPredicate())
    if (!isStricterOrEqualPred(BlockPred, CandidateBlockPred))
      return true;

  return false;
}

void VPlanAllZeroBypass::collectAllZeroBypassLoopRegions(
    AllZeroBypassRegionsTy &AllZeroBypassRegions,
    RegionsCollectedTy &RegionsCollected) {
  VPLoopInfo *VPLI = Plan.getVPLoopInfo();
  for (auto *VPLp : VPLI->getLoopsInPreorder()) {
    SetVector<VPBasicBlock *> RegionBlocks;
    VPBasicBlock *Preheader = VPLp->getLoopPreheader();
    VPValue *PreheaderPred = Preheader ? Preheader->getPredicate() : nullptr;
    // Loop is not predicated, so skip to the next one. E.g., outermost loop.
    if (!PreheaderPred)
      continue;
    // Loop was already included in another loop region under the same block-
    // predicate.
    if (regionFoundForBlock(Preheader, PreheaderPred, RegionsCollected))
      continue;
    // Record blocks in the region from the preheader to the exit block before
    // trying to extend the region.
    VPBasicBlock *Exit = VPLp->getExitBlock();
    assert(Plan.getDT()->dominates(VPLp->getHeader(), Exit) &&
           "Loop has multiple entries!");
    VPBasicBlock *RegionEnd = Exit;
    getRegionBlocks(Preheader, Exit, RegionBlocks);

    // Remain conservative about how loop regions are formed. Regions will start
    // at the preheader and extend via a single successor chain as long as the
    // block-predicate is the same for a block(s) or for another contiguous
    // loop. Extending regions using isStricterOrEqualPred in both the "upward"
    // and "downward" directions led to some stability issues because some
    // regions ended up overlapping others. More testing is needed to understand
    // how to reliably apply such region extension logic. For now, stability is
    // preferred over this optimization.

    // Try to extend loop region by going down single successor chain and
    // checking for predicate instructions that are the same as the preheader
    // block-predicate.
    VPBasicBlock *SingleSucc = Exit->getSingleSuccessor();
    while (SingleSucc) {
      // When extending the region, include single blocks with the same
      // block-predicate or loops that have isStricterOrEqualPred. Loop
      // regions are extended using isStricterOrEqualPred because it may
      // be possible to not have the same exact block-predicate, but a phi
      // with incoming predicate from the preheader.
      bool LoopHeader = VPLI->isLoopHeader(SingleSucc);
      VPValue *SingleSuccPred = SingleSucc->getPredicate();
      if (!LoopHeader && SingleSuccPred != PreheaderPred)
        break;
      if (LoopHeader && !isStricterOrEqualPred(SingleSuccPred, PreheaderPred))
        break;
      if (LoopHeader) {
        VPLoop *VPLp = VPLI->getLoopFor(SingleSucc);
        assert(VPLp && "VPLoop object is expected to exist for the block");
        VPBasicBlock *LoopExit = VPLp->getExitBlock();
        SetVector<VPBasicBlock *> LoopBlocks;
        getRegionBlocks(SingleSucc, LoopExit, LoopBlocks);
        RegionBlocks.insert(LoopBlocks.begin(), LoopBlocks.end());
        RegionEnd = LoopExit;
        SingleSucc = LoopExit->getSingleSuccessor();
      } else {
        RegionBlocks.insert(SingleSucc);
        RegionEnd = SingleSucc;
        SingleSucc = SingleSucc->getSingleSuccessor();
      }
    }

    BypassPairTy BypassRegion = std::make_pair(Preheader, RegionEnd);
    verifyBypassRegion(Preheader, RegionBlocks);
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

VPValue* VPlanAllZeroBypass::loopWasMadeUniform(VPLoop *VPLp) {
  if (VPLp) {
    auto *Latch = VPLp->getLoopLatch();
    auto *BranchInst = dyn_cast<VPBranchInst>(Latch->getTerminator());
    if (BranchInst && BranchInst->isConditional()) {
      auto *BranchCond = BranchInst->getCondition();
      auto *AllZeroCheck = dyn_cast<VPInstruction>(BranchCond);
      if (AllZeroCheck &&
          AllZeroCheck->getOpcode() == VPInstruction::AllZeroCheck)
        return AllZeroCheck->getOperand(0);
    }
  }
  return nullptr;
}

void VPlanAllZeroBypass::collectAllZeroBypassNonLoopRegions(
    AllZeroBypassRegionsTy &AllZeroBypassRegions,
    RegionsCollectedTy &RegionsCollected, VPlanCostModelInterface *CM,
    Optional<unsigned> VF) {

  // Probability is very low that for large VFs all lanes are 0.
  if (VF && *VF >= 32)
    return;

  VPLoopInfo *VPLI = Plan.getVPLoopInfo();

  // Detect blocks that are candidates to begin a bypass region. Candidate
  // blocks are those that contain block-predicates, have a single successor,
  // and have not already been included within a loop bypass region. Candidates
  // must have a single successor because the RPO iterator below assumes this
  // during region formation. We may later choose to relax this assumption if it
  // is possible to begin a region with a block containing multiple successors,
  // but this seems unlikely.
  SmallVector<VPBasicBlock *, 16> CandidateBlocks;
  ReversePostOrderTraversal<VPBasicBlock *> RPOT(&Plan.getEntryBlock());
  for (auto *Block : RPOT) {
    VPValue *BlockPred = Block->getPredicate();
    if (!BlockPred || !Block->getSingleSuccessor() ||
        regionFoundForBlock(Block, BlockPred, RegionsCollected))
      continue;
    VPLoop *VPLp = VPLI->getLoopFor(Block);
    if (auto *ExitCond = loopWasMadeUniform(VPLp)) {
      // Filter out any candidates where any blocks inside loops that have been
      // made uniform by loopcfu have a block-predicate that is the same as the
      // the condition of the all-zero check at the latch. There is no reason
      // to insert bypasses for these blocks because the loop body is already
      // controlled by the same condition. Since this function collects non-loop
      // regions only, this will not prevent a loop bypass from being inserted
      // where these massaged loops have a divergent top test.
      // BlockPred should be phi for loops made uniform, but this check
      // shouldn't harm anything.
      if (BlockPred == ExitCond)
        continue;

      auto *VPPhi = dyn_cast<VPPHINode>(BlockPred);
      if (VPPhi && any_of(VPPhi->incoming_values(),
                          [ExitCond] (const VPValue *IncomingVal) {
                              return IncomingVal == ExitCond;
                         }))
        continue;
    }
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
    SetVector<VPBasicBlock *> RegionBlocks;

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
        assert(VPLp && "VPLoop object is expected to exist for the block");
        VPBasicBlock *LoopExit = VPLp->getExitBlock();
        while (*BlockIt != LoopExit) {
          RegionBlocks.insert(*BlockIt);
          LastBB = *(++BlockIt);
        }
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
      if (endRegionAtBlock(*BlockIt, CandidateBlockPred, RegionBlocks))
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

    // Temporary workaround for CMPLRLLVM-30680:
    //
    // Probability of the mask being all-zero should take its part in the
    // decision to bypass. Ideally, we'd like to keep track of all ANDs/ORs and
    // known non-allzero masks in the expression tree (due to an earlier
    // bypass), but we're not there. For now, special case for a condition based
    // on a uniform value.
    unsigned EffectiveThreshold = RegionThreshold;
    auto *BlockPredI = dyn_cast<VPInstruction>(CandidateBlockPred);
    if (!BlockPredI ||
        (BlockPredI->getOpcode() == Instruction::And &&
         (Plan.getVPlanDA()->isUniform(*BlockPredI->getOperand(0)) ||
          Plan.getVPlanDA()->isUniform(*BlockPredI->getOperand(1)))))
      if (VF)
        EffectiveThreshold = EffectiveThreshold * 4 / *VF;

    // Cost model not yet available for function vectorization pipeline. It's
    // ok because there's really no reason for it there yet anyway since this
    // pipeline is only used for testing at the moment.
    VPInstructionCost RegionCost{EffectiveThreshold};
    if (CM)
      RegionCost = CM->getBlockRangeCost(CandidateBlock, LastBB);

    // If the region meets minimum cost requirements, record it for later
    // insertion.
    if (RegionCost.isValid() && RegionCost >= EffectiveThreshold) {
      AllZeroBypassRegionsTy::iterator InsertPt = AllZeroBypassRegions.end();
      for (AllZeroBypassRegionsTy::iterator It = AllZeroBypassRegions.begin();
           It != AllZeroBypassRegions.end(); ++It) {
        // Make sure regions are inserted in the correct order. This happens
        // when a loop region has already been inserted, but this current region
        // is actually an outer region around that one. This ensures we insert
        // regions from outermost to innermost. This is critical because we
        // first collect regions and then insert them. This avoids any CFG
        // modifications from affecting the current region being inserted.
        VPBasicBlock *RegionBegin = It->first;
        if (RegionBlocks.count(RegionBegin)) {
          InsertPt = It;
          break;
        }
      }
      BypassPairTy BypassRegion = std::make_pair(CandidateBlock, LastBB);
      verifyBypassRegion(CandidateBlock, RegionBlocks);
      AllZeroBypassRegions.insert(InsertPt, BypassRegion);
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
  // Final region entry/exit pairs so that we can check validity of region
  // formation.
  VPlanAllZeroBypass::AllZeroBypassRegionsTy AllZeroBypassRegionsFinal;
  for (auto BypassRegion : AllZeroBypassRegions)
    insertBypassForRegion(BypassRegion.first, BypassRegion.second, DT, PDT,
                          VPLI, AllZeroBypassRegionsFinal);
  Plan.computePDT();
  PDT = Plan.getPDT();
  for (const auto &BypassRegion : AllZeroBypassRegionsFinal) {
    (void)BypassRegion;
    assert(PDT->dominates(BypassRegion.second, BypassRegion.first) &&
           "Region exit does not post-dominate region entry");
  }

  // AZB modifies CFG if bypass regions list is non-empty.
  if (!AllZeroBypassRegions.empty()) {
    // AZB explicitly preserves DA, but not SVA.
    Plan.invalidateAnalyses({VPAnalysisID::SVA});
  }
}

} // end namespace vpo
} // end namespace llvm
