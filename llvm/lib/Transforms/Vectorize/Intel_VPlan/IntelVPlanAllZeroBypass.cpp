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
#include "IntelVPlanUtils.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"

#define DEBUG_TYPE "VPlanAllZeroBypass"

using namespace llvm;

// A basic tuning knob to control the size of all-zero byass regions.
// RegionThreshold is specified in the minimum number of blocks that need to be
// in the region in order to insert the bypass.
static cl::opt<unsigned> RegionThreshold(
    "vplan-all-zero-bypass-region-threshold", cl::init(1), cl::Hidden,
    cl::desc("Tune the size of all-zero bypass regions in number of blocks"));

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
static bool isStricterOrEqualPred(const VPValue *MaybePred,
                                  const VPValue *BaseCond) {
  if (MaybePred == nullptr)
    return false;

  if (MaybePred == BaseCond)
    return true;

  const VPInstruction *MaybePredInst = dyn_cast<VPInstruction>(MaybePred);
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

void VPlanAllZeroBypass::verifyBypassRegion(BypassPairTy &Region) {
  SmallPtrSet<VPBasicBlock *, 4> RegionBlocks;
  getRegionBlocks(Region.first, Region.second, RegionBlocks);
  for (auto *RegionBlock : RegionBlocks) {
    // For any phis in the region we want to assert that any incoming blocks
    // to those phis are part of the region. This will catch if all-zero
    // bypass attempts to construct invalid regions. However, we don't want
    // to assert if the block containing the phi(s) is also the first block
    // in the region. In this case, the instructions appearing before
    // before the block predicate, including phis, will not be included in
    // the region because they will be hoisted above the all-zero check.
    if (RegionBlock == Region.first)
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

void VPlanAllZeroBypass::collectAllZeroBypassRegions(
    AllZeroBypassRegionsTy &AllZeroBypassRegions) {
  // Keep a map of the regions collected under a specific block-predicate
  // to avoid collecting/inserting unnecessary regions.
  RegionsInsertedTy RegionsCollected;

  // Detect blocks that are candidates to begin a bypass region. Candidate
  // blocks are simply those that contain block-predicates with single
  // successor since regions will be formed on blocks under linearized
  // control flow. This will skip divergent loop backedges appropriately,
  // and region formation for loops will still happen from the loop preheader
  // and include this block.
  SmallVector<VPBasicBlock *, 16> CandidateBlocks;
  ReversePostOrderTraversal<VPBasicBlock *> RPOT(Plan.getEntryBlock());
  for (auto *Block : RPOT) {
    VPValue *BlockPredicate = Block->getPredicate();
    if (BlockPredicate && Block->getSingleSuccessor())
      CandidateBlocks.push_back(Block);
  }

  // Build bypass regions from outermost to innermost. This makes it easier to
  // prevent insertion of redundant regions.
  for (auto *CandidateBlock : CandidateBlocks) {
    assert(CandidateBlock->getSingleSuccessor() &&
           "Bypass region should start from linearized control flow");
    // Candidate Block is included in the region as the first and last in the
    // region.
    VPBasicBlock *LastBB = CandidateBlock;
    // CandidateBlockPred will never be null here due to previous nullptr
    // check for CandidateBlocks insertion above, but assert just in case.
    VPValue *CandidateBlockPred = CandidateBlock->getPredicate();
    assert(CandidateBlockPred &&
           "Candidate block for bypass should have a predicate");
    unsigned NumBlocks = 1;
    // Re-use existing RPOT beginning at VPlan entry. Advance RPOT iterator
    // to the single successor of CandidateBlock to begin candidate region
    // collection.
    auto BlockIt = RPOT.begin();
    while (*BlockIt != CandidateBlock->getSingleSuccessor())
      ++BlockIt;
    while (BlockIt != RPOT.end()) {
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
                 [CandidateBlockPred](const VPInstruction *VPInst) {
                   // Skip phi nodes because even though they are considered
                   // unpredicated they will not necessarily be anded with
                   // CandidateBlockPred. If phis are not skipped, we will
                   // introduce an additional region for loop preheader blocks.
                   // Besides, we assert that any incoming blocks for phis are
                   // contained within the same region as the block containing
                   // the phi. The only requirement for blend is that all
                   // incoming values/predicates are defined before the use
                   // (i.e., regions are formed while maintaining proper SSA).
                   return (!isa<VPPHINode>(VPInst) &&
                           !isa<VPBlendInst>(VPInst) &&
                           !isa<VPBranchInst>(VPInst) &&
                           !isStricterOrEqualPred(VPInst, CandidateBlockPred));
                 }))
        break;

      if (auto *BlockPred = (*BlockIt)->getPredicate())
        if (!isStricterOrEqualPred(BlockPred, CandidateBlockPred))
          break;

      LastBB = *BlockIt;
      NumBlocks++;
      ++BlockIt;
    }

    // Check to see if this region is inside another region that already
    // uses the same block-predicate to form the all-zero bypass. If so,
    // then the region is unnecessary.
    bool InsertRegion = true;
    if (RegionsCollected.count(CandidateBlockPred)) {
      for (auto CollectedRegion : RegionsCollected[CandidateBlockPred]) {
        SmallPtrSet<VPBasicBlock *, 4> CollectedRegionBlocks;
        getRegionBlocks(CollectedRegion.first, CollectedRegion.second,
                        CollectedRegionBlocks);
        if (CollectedRegionBlocks.count(CandidateBlock)) {
          InsertRegion = false;
          break;
        }
      }
    }

    // Basic tuning for all-zero bypass candidate regions at the moment.
    // Later, we can switch to a cost-model based approach if necessary.
    if (InsertRegion && NumBlocks >= RegionThreshold) {
      BypassPairTy BypassRegion = std::make_pair(CandidateBlock, LastBB);
      verifyBypassRegion(BypassRegion);
      AllZeroBypassRegions.push_back(BypassRegion);
      RegionsCollected[CandidateBlockPred].push_back(BypassRegion);
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
