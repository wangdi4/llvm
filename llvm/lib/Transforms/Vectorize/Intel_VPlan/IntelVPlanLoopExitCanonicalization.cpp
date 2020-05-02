//===-- IntelVPlanLoopExitCannonicalization.cpp ---------------------------===//
//
//   Copyright (C) 2020 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file implements utilities to perform loops' exits canonicalization that
/// is needed for a further loop control flow unification (LoopCFU)
/// transformation.
///
//===----------------------------------------------------------------------===//

#include "IntelVPlanLoopExitCanonicalization.h"
#include "IntelVPLoopAnalysis.h"
#include "IntelVPlanBuilder.h"
#include "IntelVPlanDominatorTree.h"
#include "IntelVPlanLoopInfo.h"

#define DEBUG_TYPE "vplan-loop-canonicalization"

using namespace llvm;
using namespace llvm::vpo;

// Replaces incoming block in all phi nodes of the PhiBlock.
static void updateBlocksPhiNode(VPBasicBlock *PhiBlock,
                                VPBasicBlock *PrevIncomingBlock,
                                VPBasicBlock *NewLIncomingBlock) {
  for (auto it = PhiBlock->begin(); it != PhiBlock->end(); ++it) {
    if (VPPHINode *VPPhi = dyn_cast<VPPHINode>(&*it)) {
      for (const auto *PBlk : VPPhi->blocks())
        if (PBlk == PrevIncomingBlock) {
          int Idx = VPPhi->getBlockIndex(PrevIncomingBlock);
          VPPhi->setIncomingBlock(Idx, NewLIncomingBlock);
        }
    } else
      break;
  }
}

// Move exit block's phi node to another block (new loop latch or intermediate
// block or cascaded if block). This is needed because the predecessors of the
// exit block change after disconnecting it from the exiting block. Thus, we
// might need to move the phi node of the exit block to the new loop latch or
// the intermediate block or the cascaded if block.
static void moveExitBlocksPhiNode(VPBasicBlock *ExitBlock,
                                  VPBasicBlock *NewBlock) {
  auto itNext = ExitBlock->begin();
  for (auto it = ExitBlock->begin(); it != ExitBlock->end(); it = itNext) {
    itNext = it;
    ++itNext;
    if (VPPHINode *ExitBlockVPPhi = dyn_cast<VPPHINode>(&*it)) {
      ExitBlock->removeInstruction(ExitBlockVPPhi);
      if (NewBlock->empty())
        NewBlock->addInstruction(ExitBlockVPPhi);
      else if (isa<VPPHINode>(&*NewBlock->begin()))
        NewBlock->addInstructionAfter(ExitBlockVPPhi, &*NewBlock->begin());
      else
        NewBlock->addInstruction(ExitBlockVPPhi, &*NewBlock->begin());
    } else
      break;
  }
}

// Removes a basic block from VPPHINode.
static void removeBlockFromVPPhiNode(VPBasicBlock *ExitingBlock,
                                     VPBasicBlock *ExitBlock) {
  auto itNext = ExitBlock->begin();
  for (auto it = ExitBlock->begin(); it != ExitBlock->end(); it = itNext) {
    itNext = it;
    ++itNext;
    if (VPPHINode *ExitBlockVPPhi = dyn_cast<VPPHINode>(&*it))
      ExitBlockVPPhi->removeIncomingValue(ExitingBlock);
    else
      break;
  }
}

// Checks if all the predecessors belong of an exit blocks are in the current
// loop.
static bool allPredsInLoop(VPBasicBlock *ExitBlock, VPLoop *VPL) {
  for (auto Pred : ExitBlock->getPredecessors())
    if (!(VPL->contains(Pred)))
      return false;
  return true;
}

// Checks if a basic block has a VPPHINode.
static bool hasVPPhiNode(VPBasicBlock *VPBB) {
  if ((!VPBB->empty()) && (isa<VPPHINode>(*VPBB->begin())))
    return true;
  return false;
}

// See below.
static void preserveSSAAfterLoopTransformations(VPLoop *VPL, VPlan *Plan,
                                                VPDominatorTree &VPDomTree);
namespace llvm {
namespace vpo {
// The merge loop exit transformation is applied to the inner loops of a loop
// nest. It consists of the mergeLoopExits function and six support functions
// (getBlocksExitBlock, updateBlocksPhiNode, moveExitBlocksPhiNode,
// removeBlockFromVPPhiNode, allPredsInLoop, hasVPPhiNode,
// getFirstNonPhiVPInst).
//
// The algorithm has three steps:
// 1. A new loop latch is created. All the side exits are redirected to the new
// loop latch. The new loop latch is followed by a number of cascaded ifs (if it
// is needed). The cascaded ifs redirect the control-flow to the right exit
// block.
// 2. For each exit block (other than the exit block of the loop latch), a new
// intermediate block (Intermediate_BB) is generated. The new blocks are
// connected with the corresponding exiting block(s) and the new loop latch. For
// each Intermediate_BB, a new incoming value (ExitID) is added in the phi node
// of the new loop latch.
// 3. The cascaded ifs are emitted (if it is needed).
//
// The following examples show how merge loop exits transformation works on
// different test cases:
//
// CASE 1: The inner loop has one side exit. Therefore, a Intermediate_BB is
// generated for the side exit that lands on BB4. The Intermediate_BB is
// connected with the new loop latch. Since there is only one side exit, one
// cascaded if block is emitted.
// -------------------------------------------
//  FROM                        TO
// -------------------------------------------
//
// -->BB1           ----------->BB1
// |   | \          |          /   \
// |   |  \         |         /     \
// ---BB2  BB4  =>  |       BB2     Intermediate_BB
//     |            |          \    /
//     |            |  ExitID=0 \  / ExitID=1
//    BB3           --------NEW_LOOP_LATCH
//                                |
//                                |
//                        CASCADED_IF_BLOCK
//                          (if ExitID==1)
//                             F / \ T
//                              /   \
//            (LatchExitBlock)BB3   BB4
//
// CASE 2: The inner loop has one side exit which lands on the same exit block
// as the loop latch. If the exiting block is the loop header, then we create an
// Intermediate_BB. This is needed for emitting correct masks. For any other
// basic block, we just redirect the exiting edge to the new loop latch. In this
// case, the cascaded if blocks are not needed.
// -------------------------------------------
//  FROM                        TO
// -------------------------------------------
//
// -->BB1               ------->BB1
// |   | \              |        | \
// |   |  \             |        |  \
// |  BB2  \            |       BB2  \
// |   | \  |           |        | \  \
// |   |  \ |    =>     |        |  \  Intermediate_BB
// ---BB3  ||           |       BB3  | /
//     |  //            |        |  / /
//     | //             |        | / /
//    BB4               ----NEW_LOOP_LATCH
//
// CASE 3: The inner loop has two side exits that land on the same exit block.
// Therefore, the control flow for BB1 and BB2 should be redirected on the same
// exit block. For this reason, one Intermediate_BB is emitted.
// -------------------------------------------
//  FROM                          TO
// -------------------------------------------
//
// -->BB1             ----------->BB1
// |   | \            |            | \
// |   |  \           |            |  \
// |  BB2  \      =>  |           BB2  \
// |   | \  \         |            | \  \
// |   |  BB4         |            |  Intermediate_BB
// ---BB3             |           BB3   /
//     |              |            |   /
//     |              |   ExitID=0 |  / ExitID=1
//    BB5             --------NEW_LOOP_LATCH
//                                |
//                                |
//                        CASCADED_IF_BLOCK
//                          (if ExitID==1)
//                             F / \ T
//                              /   \
//            (LatchExitBlock)BB5   BB4
//
// CASE 4: The inner loop has two side exits. Therefore, two Intermediate_BBs
// and two cascaded ifs are created.
// -------------------------------------------
//  FROM                          TO
// -------------------------------------------
//
// -->BB1             ----------->BB1--------------------------
// |   | \            |            |                          |
// |   |  \           |            |                          |
// |  BB2  BB4    =>  |           BB2                         |
// |   | \            |            | \                        |
// |   |  \           |            |  \                       V
// |   |   BB5        |            | Intermediate_BB2  Intermediate_BB1
// ---BB3             |           BB3 ExitID=1             ExitID=2
//     |              |            |   /                      |
//     |              |   ExitID=0 |  /                       |
//    BB6             --------NEW_LOOP_LATCH<------------------
//                                |
//                                |
//                        CASCADED_IF_BLOCK(HEAD)
//                          (if ExitID==2)
//                             F /    \ T
//                              /      \
//                          IFBLOCK    BB4
//                      (if ExitID==1)
//                         F /   \ T
//                          /     \
//        (LatchExitBlock)BB6     BB5
//
// CASE 5: Loop rotate does not always canonicalize the while loops. In this
// case, the original loop latch has an unconditional branch. The merge loop
// exits tranformation does the same transformation.
// -------------------------------------------
//  FROM                          TO
// -------------------------------------------
//
// -->BB1                ----------->BB1--------------------------
// |   | \               |            |                          |
// |   |  \              |            |                          |
// |  BB2  \       =>    |           BB2                         |
// |   | \  \            |            | \                        |
// |   |  \  \           |            |  \                       V
// |   |   \  \          |            | Intermediate_BB2  Intermediate_BB1
// ---BB3 BB4 BB5        |           BB3 ExitID=1             ExitID=2
//          \ /          |            |   /                      |
//          BB6          |   ExitID=0 |  /                       |
//                       --------NEW_LOOP_LATCH<------------------
//                                    |
//                                    |
//                            CASCADED_IF_BLOCK(HEAD)
//                              (if ExitID==2)
//                               F /    \ T
//                                /      \
//                              BB4      BB5
//                                \      /
//                                 \    /
//                                  BB6
//
void mergeLoopExits(VPLoop *VPL) {
  VPlan *Plan = VPL->getHeader()->getParent();
  VPLoopInfo *VPLInfo = Plan->getVPLoopInfo();

  // Check if the loop has multiple exits.
  SmallVector<VPLoop::Edge, 4> ExitEdges;
  VPL->getExitEdges(ExitEdges);

  // TODO: check uniformity of the loop preheader instead.
  if ((ExitEdges.size() < 2))
    return;

  // The merge loop exits transformation kicks-in.

  LLVM_DEBUG(dbgs() << "Before merge loop exits transformation.\n");
  LLVM_DEBUG(Plan->dump());

  SmallVector<VPBasicBlock *, 2> ExitBlocks;
  VPL->getUniqueExitBlocks(ExitBlocks);
  unsigned ExitID = 0;
  // ExitBlockIDPairs and ExitExitingBlocksMap are used for generating the
  // cascaded if blocks.
  SmallVector<std::pair<VPBasicBlock *, VPConstant *>, 2> ExitBlockIDPairs;
  SmallDenseMap<VPBasicBlock *, VPBasicBlock *> ExitExitingBlocksMap;
  VPBasicBlock *OrigLoopLatch = VPL->getLoopLatch();
  assert(OrigLoopLatch && "Transformation assumes single loop latch.");
  VPBasicBlock *LoopHeader = VPL->getHeader();
  // If the loop is a while loop (case 5), then it might not have a fall-through
  // edge. Therefore, in this case, the LatchExitBlock will be null.
  VPBasicBlock *LatchExitBlock = nullptr;
  // BackedgeCond checks whether the backedge is taken when the CondBit is
  // true or false.
  bool BackedgeCond = OrigLoopLatch->getSuccessor(0) == LoopHeader;
  if (OrigLoopLatch->getNumSuccessors() > 1) {
    // For for-loops, we get the exit block of the latch.
    assert(OrigLoopLatch->getNumSuccessors() == 2 &&
           "The loop latch should have two successors!");
    LatchExitBlock = BackedgeCond ? OrigLoopLatch->getSuccessor(1)
                                  : OrigLoopLatch->getSuccessor(0);
    ExitExitingBlocksMap[LatchExitBlock] = OrigLoopLatch;
  }

  // Step 1 : Creates a new loop latch and fills it with all the necessary
  // instructions.
  VPBasicBlock *NewLoopLatch =
      new VPBasicBlock(VPlanUtils::createUniqueName("new.loop.latch"), Plan);
  OrigLoopLatch->moveConditionalEOBTo(NewLoopLatch);
  NewLoopLatch->moveTripCountInfoFrom(OrigLoopLatch);
  VPBlockUtils::insertBlockAfter(NewLoopLatch, OrigLoopLatch);
  VPL->addBasicBlockToLoop(NewLoopLatch, *VPLInfo);
  // Remove the original loop latch from the blocks of the phi node of the loop
  // header.
  updateBlocksPhiNode(LoopHeader, OrigLoopLatch, NewLoopLatch);
  // Add a VPPHINode at the top of the new latch. This phi node shows whether
  // the control-flow reaches the new loop latch from the original loop latch or
  // one of the intermediate blocks or one of the exiting blocks.
  Type *Ty32 = Type::getInt32Ty(*Plan->getLLVMContext());
  Type *Ty1 = Type::getInt1Ty(*Plan->getLLVMContext());
  VPConstant *FalseConst = Plan->getVPConstant(ConstantInt::get(Ty1, 0));
  VPConstant *TrueConst = Plan->getVPConstant(ConstantInt::get(Ty1, 1));
  VPBuilder VPBldr;
  VPBldr.setInsertPoint(NewLoopLatch);
  VPPHINode *ExitIDVPPhi = VPBldr.createPhiInstruction(Ty32, "exit.id.phi");
  ExitIDVPPhi->addIncoming(Plan->getVPConstant(ConstantInt::get(Ty32, ExitID)),
                           OrigLoopLatch);

  // This phi node is a marker of the backedge. It shows if the backedge is
  // taken.
  VPPHINode *NewCondBit =
      VPBldr.createPhiInstruction(Ty1, "take.backedge.cond");
  if (LatchExitBlock) {
    VPInstruction *OldCondBit =
        dyn_cast<VPInstruction>(NewLoopLatch->getCondBit());
    NewCondBit->addIncoming(OldCondBit, OrigLoopLatch);
  } else {
    assert(BackedgeCond == true &&
           "In while loops, BackedgeCond should be true");
    NewCondBit->addIncoming(TrueConst, OrigLoopLatch);
  }
  // Update the condbit.
  NewLoopLatch->setCondBit(NewCondBit);

  // This is needed for the generation of cascaded if blocks.
  if (LatchExitBlock) {
    VPConstant *ExitIDConst =
        Plan->getVPConstant(ConstantInt::get(Ty32, ExitID));
    ExitBlockIDPairs.push_back(std::make_pair(LatchExitBlock, ExitIDConst));
  }

  // Step 2: Disconnects the exit blocks from the exiting blocks. If it is
  // needed, an intermediate basic block is created. Otherwise, the exiting
  // blocks are connected with the new loop latch.
  SmallPtrSet<VPBasicBlock *, 2> VisitedBlocks;
  SmallDenseMap<VPBasicBlock *, VPBasicBlock *> ExitBlockIntermediateBBMap;
  bool phiIsMovedToNewLoopLatch = false;
  // For each exit block (apart from the new loop latch), a new basic block is
  // created and the ExitID is emitted.
  for (auto &Edge : ExitEdges) {
    VPBasicBlock *ExitingBlock = Edge.first;
    VPBasicBlock *ExitBlock = Edge.second;

    if (ExitingBlock == OrigLoopLatch)
      continue;

    // This check is for case 2. In this case, we create a new intermediate
    // basic block only if the exiting block is the loop header. This is needed
    // for correct insertion of phis to make the loop exit condition live
    // through back edge. For any other basic block, we just have to redirect
    // the exiting block to the new loop latch.
    if (ExitBlock == LatchExitBlock) {
      VPBasicBlock *IntermediateBB = nullptr;
      if (ExitingBlock == LoopHeader) {
        IntermediateBB = new VPBasicBlock(
            VPlanUtils::createUniqueName("intermediate.bb"), Plan);
        VPL->addBasicBlockToLoop(IntermediateBB, *VPLInfo);
        IntermediateBB->insertBefore(ExitBlock);
        LoopHeader->replaceSuccessor(ExitBlock, IntermediateBB);
        IntermediateBB->appendSuccessor(NewLoopLatch);
        // Replace the exiting block with the new exiting block in the exit
        // block's phi node.
        if (hasVPPhiNode(ExitBlock) && allPredsInLoop(ExitBlock, VPL))
          updateBlocksPhiNode(ExitBlock, ExitingBlock, IntermediateBB);
      } else {
        ExitingBlock->replaceSuccessor(ExitBlock, NewLoopLatch);
        IntermediateBB = ExitingBlock;
      }
      ExitID++;
      VPConstant *ExitIDConst =
          Plan->getVPConstant(ConstantInt::get(Ty32, ExitID));
      ExitIDVPPhi->addIncoming(ExitIDConst, IntermediateBB);
      // If the backedge is taken under the true condition, then the edge from
      // the exiting block is taken under the false condition.
      NewCondBit->addIncoming(BackedgeCond ? FalseConst : TrueConst,
                              IntermediateBB);

      // The VPPHINode of the exit block should be moved in the new loop latch
      // if all the predecessors are in the loop. If not, then we remove the
      // exiting block from the basic blocks of the VPPHINode. If more than one
      // blocks of the loop land on the same exit block, then one of them is
      // replaced with new loop latch in the phi node's list and the others are
      // removed from the phi node's list.
      if (hasVPPhiNode(ExitBlock) && !phiIsMovedToNewLoopLatch) {
        if (allPredsInLoop(ExitBlock, VPL)) {
          if (VisitedBlocks.count(ExitBlock)) {
            // Remove the exiting block from the exit block's phi node.
            removeBlockFromVPPhiNode(ExitingBlock, ExitBlock);
          } else {
            // Move the phi node of the exit block to the new loop latch.
            moveExitBlocksPhiNode(ExitBlock, NewLoopLatch);
            phiIsMovedToNewLoopLatch = true;
          }
        } else
          updateBlocksPhiNode(ExitBlock, ExitingBlock, IntermediateBB);
      }
    } else {
      // This check ensures that only one Intermediate_BB is created (case 3) in
      // case more than one exiting blocks land on the same exit block.
      if (VisitedBlocks.count(ExitBlock)) {
        VPBasicBlock *ExistingIntermediateBB =
            ExitBlockIntermediateBBMap[ExitBlock];
        ExitingBlock->replaceSuccessor(ExitBlock, ExistingIntermediateBB);
        if (hasVPPhiNode(ExitBlock))
          removeBlockFromVPPhiNode(ExitingBlock, ExitBlock);
        continue;
      }

      VPBasicBlock *IntermediateBB = new VPBasicBlock(
          VPlanUtils::createUniqueName("intermediate.bb"), Plan);
      IntermediateBB->insertBefore(ExitBlock);
      VPL->addBasicBlockToLoop(IntermediateBB, *VPLInfo);
      // Remove ExitBlock from ExitingBlock's successors and add a new
      // intermediate block to its successors. ExitBlock's predecessor will be
      // updated after emitting cascaded if blocks.
      ExitingBlock->replaceSuccessor(ExitBlock, IntermediateBB);
      IntermediateBB->appendSuccessor(NewLoopLatch);

      if (hasVPPhiNode(ExitBlock))
        if (allPredsInLoop(ExitBlock, VPL))
          moveExitBlocksPhiNode(ExitBlock, IntermediateBB);

      // Add ExitID and update NewLoopLatch's phi node.
      ExitID++;
      VPConstant *ExitIDConst =
          Plan->getVPConstant(ConstantInt::get(Ty32, ExitID));
      ExitIDVPPhi->addIncoming(ExitIDConst, IntermediateBB);
      NewCondBit->addIncoming(BackedgeCond ? FalseConst : TrueConst,
                              IntermediateBB);
      ExitBlockIDPairs.push_back(std::make_pair(ExitBlock, ExitIDConst));
      ExitExitingBlocksMap[ExitBlock] = ExitingBlock;
      ExitBlockIntermediateBBMap[ExitBlock] = IntermediateBB;
    }
    VisitedBlocks.insert(ExitBlock);
  }

  // Step 3 : Creates cascaded if blocks. The head of the cascaded if blocks is
  // emitted after the new loop latch. The cascaded if blocks are not emitted
  // in case 2.

  // We need to collect all the cascaded if blocks in a small vector which will
  // be used in a separate step. Each cascaded if block should be assigned to
  // the correct loop of the loop nest. This does not happen along the
  // generation of the cascaded if blocks, but it happens in a separate step.
  // Because the choice of the correct loop depends on the loop that cascaded if
  // block's successors belong. Therefore, we first need to connect the cascaded
  // if blocks with their successors (other cascaded if blocks, exit blocks) and
  // next, we can decide which is the right loop for each cascaded if block.
  SmallVector<VPBasicBlock *, 2> CascadedIfBlocks;

  if (ExitBlocks.size() > 1) {
    VPBasicBlock *IfBlock = new VPBasicBlock(
        VPlanUtils::createUniqueName("cascaded.if.block"), Plan);
    CascadedIfBlocks.push_back(IfBlock);
    // Update the predecessors of the IfBlock.
    if (LatchExitBlock)
      NewLoopLatch->replaceSuccessor(LatchExitBlock, IfBlock);
    else
      NewLoopLatch->appendSuccessor(IfBlock);

    for (int i = 1, end = ExitBlockIDPairs.size(); i != end; ++i) {
      const auto &Pair = ExitBlockIDPairs[i];
      VPBasicBlock *ExitBlock = Pair.first;
      VPConstant *ExitID = Pair.second;
      auto *CondBr = new VPCmpInst(ExitIDVPPhi, ExitID, CmpInst::ICMP_EQ);
      IfBlock->appendInstruction(CondBr);
      VPBasicBlock *NextIfBlock = nullptr;
      // Emit cascaded if blocks.
      if (i != end - 1) {
        NextIfBlock = new VPBasicBlock(
            VPlanUtils::createUniqueName("cascaded.if.block"), Plan);
        CascadedIfBlocks.push_back(NextIfBlock);
      } else {
        // For for-loops, the NextIfBlock is the LatchExitBlock.
        NextIfBlock = ExitBlockIDPairs[0].first;
        VPBasicBlock *ExitingBlock = ExitExitingBlocksMap[NextIfBlock];
        updateBlocksPhiNode(NextIfBlock, ExitingBlock, IfBlock);
      }
      // Update the successors of the IfBlock.
      IfBlock->setTwoSuccessors(CondBr, ExitBlock, NextIfBlock);
      // If all the predecessors of the exit block are in the loop, then the phi
      // node is moved in the if block. If not, then we replace the ExitingBlock
      // with the IfBlock in exit block's phi node.
      if (hasVPPhiNode(ExitBlock)) {
        VPBasicBlock *ExitingBlock = ExitExitingBlocksMap[ExitBlock];
        updateBlocksPhiNode(ExitBlock, ExitingBlock, IfBlock);
      }
      IfBlock = NextIfBlock;
    }
  } else if (!LatchExitBlock && ExitBlocks.size() == 1) {
    VPBasicBlock *ExitBlock = ExitBlocks[0];
    NewLoopLatch->appendSuccessor(ExitBlock);
  }

  // Now, we should assign the cascaded if block to the right loop nesting
  // level. The cascaded if block should belong to the same loop nesting level
  // as its successors (exit blocks). Each exit block might belong to different
  // loop nesting level than the other exit block. In this case, the cascaded if
  // block will have the same loop nesting level as the exit block which is
  // deepest in the loop hierarchy.
  while (!CascadedIfBlocks.empty()) {
    // Start from the last cascaded if block because it has two exit blocks.
    VPBasicBlock *CurrentCascadedIfBlock = CascadedIfBlocks.pop_back_val();
    assert(CurrentCascadedIfBlock->getNumSuccessors() == 2 &&
           "Two successors are expected");
    VPLoop *Succ0Loop =
        VPLInfo->getLoopFor(CurrentCascadedIfBlock->getSuccessor(0));
    VPLoop *Succ1Loop =
        VPLInfo->getLoopFor(CurrentCascadedIfBlock->getSuccessor(1));
    // It's possible that successor blocks of CurrentCascadedIfBlock may fall
    // outside of any loopnest. If so, loop depth is set to 0.
    // The bigger the loop depth the deeper the loop is in the loop nest.
    unsigned LoopDepthOfSucc0 = Succ0Loop ? Succ0Loop->getLoopDepth() : 0;
    unsigned LoopDepthOfSucc1 = Succ1Loop ? Succ1Loop->getLoopDepth() : 0;
    VPLoop *CascadedIfBlockParentLoop =
        LoopDepthOfSucc0 >= LoopDepthOfSucc1 ? Succ0Loop : Succ1Loop;
    if (CascadedIfBlockParentLoop)
      CascadedIfBlockParentLoop->addBasicBlockToLoop(CurrentCascadedIfBlock,
                                                     *VPLInfo);
    CurrentCascadedIfBlock->insertAfter(NewLoopLatch);
  }

  Plan->computeDT();
  Plan->computePDT();

  // Emit phi nodes that will preserve SSA form if it is needed.
  preserveSSAAfterLoopTransformations(VPL, Plan, *Plan->getDT());

  assert(
      VPL->getExitingBlock() == NewLoopLatch &&
      "Only 1 exiting block is expeted after merge loop exits transformation");

  LLVM_DEBUG(dbgs() << "After merge loop exits transformation.\n");
  LLVM_DEBUG(Plan->dump());
}

// The single-exit while tranformation creates a new loop latch where the
// side-exit is redirected.
// -------------------------------------------
//  BEFORE                         AFTER
// -------------------------------------------
//
// -->BB1                ----------->BB1
// |   |\               |            |\
// |   | \              |            | \
// |  BB2 \       =>    |           BB2 \
// |   |   \            |            |   \
// |   |    \           |            |    \
// |   |    BB4         |            |     |
// ---BB3               |           BB3    |
//                      |            |     |
//                      |            |     |
//                       --------NEW_LOOP_LATCH
//                                   |
//                                   |
//                                  BB4
//
void singleExitWhileLoopCanonicalization(VPLoop *VPL) {
  VPlan *Plan = VPL->getHeader()->getParent();
  VPBasicBlock *OrigLoopLatch = VPL->getLoopLatch();
  assert(OrigLoopLatch && "Transformation assumes single loop latch.");
  if (OrigLoopLatch->getNumSuccessors() > 1)
    return;

  if (!VPL->getExitingBlock())
    return;

  LLVM_DEBUG(dbgs() << "Before single exit while loop transformation.\n");
  LLVM_DEBUG(Plan->dump());

  // Create new loop latch
  VPLoopInfo *VPLInfo = Plan->getVPLoopInfo();
  VPBasicBlock *NewLoopLatch = VPBlockUtils::splitBlockEnd(
      OrigLoopLatch, VPLInfo, Plan->getDT(), Plan->getPDT());
  NewLoopLatch->setName(VPlanUtils::createUniqueName("new.loop.latch"));
  // Update the control-flow for the ExitingBlock, the NewLoopLatch and the
  // ExitBlock.
  VPBasicBlock *ExitingBlock = VPL->getExitingBlock();
  VPBasicBlock *ExitBlock = VPL->getExitBlock();
  assert(ExitBlock && "Exiting block should have an exit block!");
  ExitingBlock->replaceSuccessor(ExitBlock, NewLoopLatch);
  NewLoopLatch->appendSuccessor(ExitBlock);
  // Update the blocks of the phi node (if it exists) in the exit block.
  updateBlocksPhiNode(ExitBlock, ExitingBlock, NewLoopLatch);

  // Fill-in NewLoopLatch with new instructions.
  // Emit the backedge condition.
  Type *Int1Ty = Type::getInt1Ty(*Plan->getLLVMContext());
  VPConstant *FalseConst = Plan->getVPConstant(ConstantInt::get(Int1Ty, 0));
  VPConstant *TrueConst = Plan->getVPConstant(ConstantInt::get(Int1Ty, 1));
  VPBuilder VPBldr;
  VPBldr.setInsertPoint(NewLoopLatch);
  VPPHINode *TakeBackedgeCond =
      VPBldr.createPhiInstruction(Int1Ty, "TakeBackedgeCond");
  TakeBackedgeCond->addIncoming(TrueConst, OrigLoopLatch);
  TakeBackedgeCond->addIncoming(FalseConst, ExitingBlock);
  NewLoopLatch->setCondBit(TakeBackedgeCond);

  // TODO: CMPLRLLVM-9535 Update VPDomTree and VPPostDomTree instead of
  // recalculating it.
  Plan->computeDT();
  Plan->computePDT();

  // Emit phi nodes that will preserve SSA form if it is needed.
  preserveSSAAfterLoopTransformations(VPL, Plan, *Plan->getDT());

  assert(VPL->getExitingBlock() == NewLoopLatch &&
         "Only 1 exiting block is expeted after single-exit while loop "
         "canonicalization");

  LLVM_DEBUG(dbgs() << "After single exit while loop transformation.\n");
  LLVM_DEBUG(Plan->dump());
}
} // namespace vpo
} // namespace llvm

// After merge loop exits transformation and while loop canonicalization, a new
// control-flow path is added between the loop header and the new loop latch.
//
// The following example shows why it is needed to preserve SSA. x is defined in
// BB2 and it is used in BB1. After single-exit while loop transformation, a new
// edge is added between BB1 and the NEW_LOOP_LATCH. This means that the value
// of x can end up to the NEW_LOOP_LATCH from multiple paths. For this reason, a
// new phi node is emitted in the NEW_LOOP_LATCH.
// -----------------------------------------------------------------------------
//  BEFORE                                          AFTER
// -----------------------------------------------------------------------------
//
// +->BB1                               +----------->BB1
// | x_phi1 = [x, BB3], [0, PreHeader]  | x_phi1 = [x_phi2, BB3], [0, PreHeader]
// | ...=x                              |           ...=x_phi1
// |   |\                               |             |\
// |   | \                              |             | \
// |  BB2 \       =>                    |            BB2 \
// | x=... \                            |           x=... \
// |   |    \                           |             |    \
// |   |     \                          |             |     \
// |   |     BB4                        |             |      |
// +--BB3                               |            BB3     |
//                                      |             |      |
//                                      |             |      |
//                                      +--------NEW_LOOP_LATCH
//                                        x_phi2 = [x, BB3] [undef, BB1]
//                                                    |
//                                                    |
//                                                   BB4
//
// Example 2: The following example shows what happens after merge loop exits
// transformation. The LoopHeader has two live-ins (values "x" and "y") whose
// definitions are in the loop body. Value "x" is defined in BB5. There is no
// definition of "x" in the other two paths that lead to NewLoopLatch
// (BB4-IntermediateBB2-NewLoopLatch and BB3-IntermediateBB1-NewLoopLatch). For
// this reason, x_phi2 is emitted in the NewLoopLatch. Value "y" is defined in
// BB2 which dominates NewLoopLatch. Hence, it is not needed to emit a phi node.
// We should preserve SSA not only for live-ins, but also for live-outs. Thus, a
// phi node (w_phi2) is emitted in NewLoopLatch.
//
// -----------------------------------------------------------------------------
//  BEFORE                                          AFTER
// -----------------------------------------------------------------------------
//
// +->BB1                           +--------------->BB1
// | x_phi1=[x,BB6],[0,PreHeader]   | x_phi1=[x_phi2,NewLoopLatch],[0,PreHeader]
// | y_phi=[y,BB6],[0,PreHeader]    |      y=[y,BB3],[0,PreHeader]
// | ...=x                          |             ...=x_phi1
// |   |                            |                 |
// |  BB2                           |                BB2
// |  y=...                         |               y=...
// |   |                            |                 |
// |  BB3                           |                BB3--------------------+
// |   |  \                         |                 |                     |
// |  BB4  EXIT_BB1                 |                BB4--------+   Intermediate
// | w=...                          |               w=...       |          BB1
// |   |  \                         |                 |         |           |
// |   |   \                        |                 |         |           |
// |  BB5   EXIT_BB2                |                BB5  Intermediate      |
// | x=...  w_phi1=[w, BB4]         |               x=...      BB2          |
// |   |    ...=w_phi1              |                 |   w_phi1 = [w, BB4] |
// |   |                            |                 |         |           |
// |   |                            |                 |         |           |
// +--BB6                           |                BB6        |           |
//     |                            |                 |         |           |
//     |                            |                 |         |           |
//  EXIT_BB3                        +-----------------NewLoopLatch<---------+
//                                  x_phi2=[x,BB6],[undef,IntermediateBB1],
//                                                 [undef,IntermediateBB2]
//                                  w_phi2=[undef,BB6],[undef,IntermediateBB1],
//                                                    [w_phi1,IntermediateBB2]
//                                                          |
//                                                          |
//                                                    CascadedIfBlock1
//                                                         /  \
//                                                        /    \
//                                          CascadedIfBlock2  EXIT_BB1
//                                                      /  \
//                                                     /    \
//                                                EXIT_BB3  EXIT_BB2
//                                                          ...=w_phi2
//
// Emit phi nodes that preserve SSA for the values that are defined in the loop
// and they are used in loop header or outside of the current loop.
static void preserveSSAAfterLoopTransformations(VPLoop *VPL, VPlan *Plan,
                                                VPDominatorTree &VPDomTree) {
  VPBasicBlock *NewLoopLatch = VPL->getLoopLatch();
  for (VPBasicBlock *DefBlock : VPL->getBlocks()) {
    if (VPDomTree.dominates(DefBlock, NewLoopLatch))
      continue;

    for (VPInstruction &Def : *DefBlock) {
      // Check which definitions have uses in the loop header or outside of the
      // loop and collect all the uses that need to be updated by the SSA phi
      // node.
      SmallVector<VPUser *, 2> UsesToUpdate;
      llvm::copy_if(
          Def.users(), std::back_inserter(UsesToUpdate), [VPL](auto &User) {
            return isa<VPExternalUse>(User) ||
                   !VPL->contains(cast<VPInstruction>(User)) ||
                   (isa<VPPHINode>(User) &&
                    cast<VPPHINode>(User)->getParent() == VPL->getHeader());
          });

      if (UsesToUpdate.empty())
        continue;

      // Create the SSA phi node.
      VPBuilder VPBldr;
      VPBldr.setInsertPoint(&*NewLoopLatch->begin());
      VPPHINode *PreserveSSAPhi = VPBldr.createPhiInstruction(
          Def.getType(), Def.getName() + ".ssa.phi");
      // Fill-in the phi node with its operands. If the DefBlock is not one of
      // the predecessors (BB2 in the example below), then we cannot use
      // DefBlock as the incoming block of the SSA phi (x_phi2). Instead, the
      // incoming block for this Def is any NewLoopLatch's predecessor that is
      // dominated by the DefBlock (BB3 in the following example).
      // ---------------------------------------------------------------------
      //  BEFORE                                          AFTER
      // ---------------------------------------------------------------------
      // +->BB1                         +----------------->BB1
      // | x_phi1=[x,BB6],              | x_phi1=[x_phi2,NewLoopLatch],
      // |   |    [0,PreHeader]         |        [0,PreHeader]      |
      // |   |     \                    |                   |       |
      // |  BB2   EXIT_BB1              |                  BB2     INTERME-
      // |  x=...                       |                  x=...   DIATEBB
      // |   |                          |                   |       |
      // |   |                          |                   |       |
      // +--BB3                         |                  BB3      |
      //     |                          |                   |       |
      //   EXIT_BB2                     +--------------NewLoopLatch-+
      //                                     x_phi2=[x, BB3],
      //                                            [undef,INTERMEDIATEBB]
      //                                                    |
      //                                             CascadedIfBlock
      //                                                  /    \
      //                                             EXITBB1  EXITBB2
      for (VPBasicBlock *Pred : NewLoopLatch->getPredecessors()) {
        VPValue *ValueToUse = VPDomTree.dominates(DefBlock, Pred)
                                  ? &Def
                                  : cast<VPValue>(Plan->getVPConstant(
                                        UndefValue::get(Def.getType())));
        PreserveSSAPhi->addIncoming(ValueToUse, Pred);
      }

      // Finally, update the uses of the definition with the SSA phi node.
      for (auto *Use : UsesToUpdate)
        Use->replaceUsesOfWith(&Def, PreserveSSAPhi);
    }
  }
}
