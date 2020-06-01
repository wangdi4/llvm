//===-- IntelVPlanLoopCFU.cpp ---------------------------------------------===//
//
//   Copyright (C) 2020 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//
//
// This file implements loop control flow uniformity transformation.
// Essentially, it replaces divergent conditions in backedes with VPAllZeroCheck
// instruction and creates the mask for the loop body.
//
//===----------------------------------------------------------------------===//

#include "IntelVPlanLoopCFU.h"
#include "IntelVPlanBuilder.h"

#define DEBUG_TYPE "vplan-loop-cfu"

namespace llvm {
namespace vpo {

void VPlanLoopCFU::run(VPLoop *VPL) {
  VPlanDivergenceAnalysis *VPDA = Plan.getVPlanDA();
  VPLoopInfo *VPLI = Plan.getVPLoopInfo();

  VPBasicBlock *VPLPreHeader = VPL->getLoopPreheader();
  VPBasicBlock *VPLHeader = VPL->getHeader();
  VPBasicBlock *VPLLatch = VPL->getLoopLatch();
  VPBasicBlock *VPLExitBlock = VPL->getExitBlock();
  assert(VPLExitBlock && "Loop hasn't been canonicalized!");

  // Now process VPL. For loop vectorization this is a no-op because the loop
  // is uniform. For function vectorization top-level loop isn't guaranteed to
  // be uniform and has to be canonicalized same other inner loops.

  LLVM_DEBUG(dbgs() << "Checking inner loop control flow uniformity for:\n");
  LLVM_DEBUG(dbgs() << "VPL at depth: " << VPL->getLoopDepth() << "\n");
  LLVM_DEBUG(dbgs() << "VPLPreHeader: " << VPLPreHeader->getName() << "\n");
  LLVM_DEBUG(dbgs() << "VPLHeader: " << VPLHeader->getName() << "\n");
  LLVM_DEBUG(dbgs() << "VPLLatch: " << VPLLatch->getName() << "\n");
  LLVM_DEBUG(dbgs() << "VPLExitBlock: " << VPLExitBlock->getName() << "\n");

  // Get inner loop bottom test.
  VPValue *BottomTest = VPLLatch->getCondBit();
  assert(BottomTest && "Could not find loop exit condition\n");
  LLVM_DEBUG(dbgs() << "BottomTest: "; BottomTest->dump(); errs() << "\n");

  if (!VPDA->isDivergent(*BottomTest)) {
    LLVM_DEBUG(dbgs() << "BottomTest is uniform\n");
    return;
  }

  LLVM_DEBUG(dbgs() << "BottomTest is divergent\n");

  LLVM_DEBUG(dbgs() << "VPL before inner loop control flow transformation\n");
  LLVM_DEBUG(VPL->printRPOT(dbgs(), Plan.getVPLoopInfo()));

  VPValue *TopTest = nullptr;
  if (VPBasicBlock *VPLRegnPred = VPLPreHeader->getSinglePredecessor()) {
    TopTest = VPLRegnPred->getCondBit();
    if (TopTest) {
      // If the subloop region is the false successor of the predecessor,
      // we need to negate the top test.
      if (VPLRegnPred->getSuccessors()[1] == VPLPreHeader) {
        VPBuilder Builder;
        Builder.setInsertPoint(VPLRegnPred);
        TopTest = Builder.createNot(TopTest, TopTest->getName() + ".not");
        VPDA->updateDivergence(*TopTest);
      }
      LLVM_DEBUG(dbgs() << "Top Test: "; TopTest->dump(); errs() << "\n");
    }
  }

  auto *Parent = VPLHeader->getParent();
  Parent->computeDT();
  VPDominatorTree *DT = Parent->getDT();
  Parent->computePDT();
  VPPostDominatorTree *PDT = Parent->getPDT();

  // Create a SESE region with a loop/bypass inside.
  VPBasicBlock *RegionEntryBlock =
      VPBlockUtils::splitBlockBegin(VPLHeader, VPLI, DT, PDT);
  VPBasicBlock *NewLoopHeader =
      VPBlockUtils::splitBlockBegin(RegionEntryBlock, VPLI, DT, PDT);
  (void)NewLoopHeader;

  // Latch might have changed during splitting.
  VPLLatch = VPL->getLoopLatch();
  VPBasicBlock *NewLoopLatch =
      VPBlockUtils::splitBlockEnd(VPLLatch, VPLI, DT, PDT);

  VPBasicBlock *RegionExitBlock;
  if (VPLLatch->empty())
    RegionExitBlock = VPLLatch;
  else
    RegionExitBlock = VPBlockUtils::splitBlockEnd(VPLLatch, VPLI, DT, PDT);

  // Remove the loop backedge condition from its original parent and place
  // in the region exit. Not needed for correctness, but easier to see
  // when debugging because the successor of the region exit block is
  // the loop latch containing this condition as the condition bit.This will
  // not work if the condition is a phi node. For this reason, only conditions
  // that are compare instructions are moved to the RegionExitBlock.
  if (dyn_cast<VPCmpInst>(BottomTest)) {
    VPBasicBlock *BottomTestBlock =
        cast<VPInstruction>(BottomTest)->getParent();
    BottomTestBlock->removeInstruction(cast<VPInstruction>(BottomTest));
    RegionExitBlock->addInstruction(cast<VPInstruction>(BottomTest));
  }

  // Construct loop body mask and insert into the loop header
  VPPHINode *LoopBodyMask = new VPPHINode(BottomTest->getType());
  LoopBodyMask->setName("vp.loop.mask");
  VPDA->markDivergent(*LoopBodyMask);

  if (TopTest)
    LoopBodyMask->addIncoming(TopTest, VPLPreHeader);
  else {
    // do/while loops won't have a top test so use 1 for incoming mask from
    // preheader.
    Constant *AllOnes = Constant::getAllOnesValue(BottomTest->getType());
    VPValue *One = Plan.getVPConstant(AllOnes);
    LoopBodyMask->addIncoming(One, VPLPreHeader);
  }

  VPBuilder Builder;
  Builder.setInsertPoint(RegionExitBlock);

  // If the back edge is the false successor of the loop latch, the bottom
  // test needs to be adjusted when masking the loop body by negating it.
  bool BackEdgeIsFalseSucc = NewLoopLatch->getSuccessors()[1] == VPLHeader;

  if (BackEdgeIsFalseSucc) {
    assert(VPDA->isDivergent(*BottomTest));
    BottomTest = Builder.createNot(cast<VPInstruction>(BottomTest),
                                   BottomTest->getName() + ".not");
    VPDA->markDivergent(*BottomTest);
  }

  // Combine the bottom test with the current loop body mask - inactive
  // lanes need to to remain inactive.
  BottomTest = Builder.createAnd(BottomTest, cast<VPInstruction>(LoopBodyMask),
                                 LoopBodyMask->getName() + ".next");
  VPDA->markDivergent(*BottomTest);

  // Update live-outs of the subloop. We should take the value which was
  // computed during the last not-masked-out iteration. For that, we need to
  // introduce a phi-node if one doesn't exist already. For example:
  //
  //    Header:
  //      %new_phi = phi [ undef, %PreHeader ], [ %LastComputed, %Latch ]
  //       ; ...
  //       br <...>
  //
  //    ...
  //
  //    SomeBB:
  //      %live_out.def =
  //      br <...>
  //
  //    ...
  //
  //    Latch: ; preds <...>
  //      ; Don't rewrite the live-out with the junk value of
  //      ; %live_out.def for masked-out iterations.
  //      %LastComputed = select %CurrentMask,  %live_out.def, %new_phi
  //      ; ...
  //      br i1 %exit_cond, label %ExitBB, label %Header
  //
  //    ExitBB
  //      %lcssa.live.out.phi = phi [ %LastComputed, %Latch ]
  //
  // We don't have control over BBs iteration order and at the point we
  // start processing Latch some blends might have been already created.
  // Don't try to process them as that's not needed.
  SmallSet<VPValue *, 8> CreatedBlends;
  for (auto *BB : VPL->blocks()) {
    // Skip BB if it corresponds to an inner loop - we have processed
    // instructions in it already and all live-outs are through LCSSA phi of
    // that inner loop right now. We will create one more if the result
    // lives out of the current loop too when processing that inner's loop
    // exit block.
    if (VPLI->getLoopFor(BB) != VPL)
      continue;

    // We will be adding instructions to the latch so do the copy to avoid
    // stale iterators.
    SmallVector<VPInstruction *, 16> Instructions(map_range(
        *BB,
        // Can't have a vector of references - take the address.
        [](VPInstruction &VPInst) -> VPInstruction * { return &VPInst; }));
    for (VPInstruction *Inst : Instructions) {
      if (CreatedBlends.count(Inst))
        // This instruction is a blend that we've just created, no
        // processing needed.
        continue;

      VPPHINode *LCSSAPhi = nullptr;
      VPValue *Blend = nullptr;

      SmallVector<VPUser *, 8> Users(Inst->user_begin(), Inst->user_end());
      for (VPUser *U : Users) {
        auto *UserInst = dyn_cast<VPInstruction>(U);
        if (!UserInst)
          continue;

        if (VPL->contains(UserInst))
          // Not live-out.
          continue;

        // Ok, Inst is live-out, need to create a proper blend for the
        // live-out value and update the use.
        if (!Blend) {
          // Create a new phi and use mask for the current iteration.
          auto *NewPhi = new VPPHINode(Inst->getType());
          NewPhi->setName(Inst->getName() + ".live.out.prev");
          VPDA->markDivergent(*NewPhi);

          // It can be either VPLHeader or NewLoopLatch - doesn't really
          // matter.
          assert(VPLHeader->getNumPredecessors() == 2 &&
                 "Expected exactly two predecessors for VPLHeader!");
          VPLHeader->addInstructionAfter(NewPhi, nullptr /* be the first */);

          // Create the blend before population NewPhi's incoming values
          // that blend will be one of them.
          Blend = Builder.createSelect(LoopBodyMask, Inst, NewPhi,
                                       Inst->getName() + ".live.out.blend");
          VPDA->markDivergent(*Blend);
          CreatedBlends.insert(Blend);

          // We need undef for all the predecessors except NewLoopLatch.
          auto *VPUndef = Plan.getVPConstant(UndefValue::get(Inst->getType()));
          for (VPBasicBlock *Pred : VPLHeader->getPredecessors())
            NewPhi->addIncoming(Pred == NewLoopLatch ? Blend : VPUndef, Pred);

          LLVM_DEBUG(dbgs() << "LoopCFU: Handled live-out: " << *Inst
                            << "Created blend: " << *Blend
                            << "and phi: " << *NewPhi << "\n";);
        }

        if (auto *Phi = dyn_cast<VPPHINode>(UserInst)) {
          if (Phi->getParent() == VPLExitBlock) {
            // LCSSA phi, just update incoming value.
            assert(Phi->getNumIncomingValues() == 1 &&
                   "Expected single incoming value for phi!");

            Phi->setIncomingValue((unsigned)0, Blend);
            // TODO: assert for being equivalent if already set. Also, the
            // previous value can be RAUW'ed with newly found one.
            LCSSAPhi = Phi;
            continue;
          }
        }

        // This UserInst isn't an LCSSA-phi. Change it to reference a proper
        // LCSSA-phi instead of the original non-blended live-out. This
        // isn't really required, but we will likely decide to preserve
        // LCSSA form through the whole VPlan pipeline, so make the IR
        // coming out of this transformation as much LCSSA-ish as possible.

        // Check if we have already found/created an LCSSA-like phi.
        if (!LCSSAPhi) {
          LCSSAPhi = new VPPHINode(Inst->getType());
          LCSSAPhi->setName(Inst->getName() + ".live.out.lcssa");
          VPDA->markDivergent(*LCSSAPhi);
          VPLExitBlock->addInstructionAfter(LCSSAPhi,
                                            nullptr /* be the first */);
          LCSSAPhi->addIncoming(Blend, NewLoopLatch);
        }

        U->replaceUsesOfWith(Inst, LCSSAPhi);
      }
    }
  }

  // Compute and set the new condition bit in the loop latch. If all the
  // lanes are inactive, new condition bit will be true.
  auto *NewCondBit = Builder.createAllZeroCheck(BottomTest);
  Plan.getVPlanDA()->updateDivergence(*NewCondBit);
  NewLoopLatch->clearSuccessors();
  NewLoopLatch->setTwoSuccessors(NewCondBit, VPLExitBlock, VPLHeader);

  LoopBodyMask->addIncoming(BottomTest, NewLoopLatch);
  VPLHeader->addInstruction(LoopBodyMask);
  RegionEntryBlock->setCondBit(LoopBodyMask);

  // Connect region entry/exit blocks so that predicate can be propagated
  // along mask=false path. i.e., this edge skips the loop body.
  RegionEntryBlock->appendSuccessor(RegionExitBlock);
  RegionExitBlock->appendPredecessor(RegionEntryBlock);

  LLVM_DEBUG(
      dbgs() << "Subloop after inner loop control flow transformation\n");
  LLVM_DEBUG(VPL->printRPOT(dbgs(), Plan.getVPLoopInfo()));
}

void VPlanLoopCFU::run() {
  auto *VPLI = Plan.getVPLoopInfo();

  for (auto *OuterVPLp : *VPLI)
    // Process from the innermost to the outermost.
    for (auto *VPLp : post_order(OuterVPLp))
      run(VPLp);

  // Recompute invalidated analyses.
  Plan.computeDT();
  Plan.computePDT();
}
} // namespace vpo
} // namespace llvm
