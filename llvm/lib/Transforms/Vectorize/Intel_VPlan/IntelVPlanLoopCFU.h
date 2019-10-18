//===-- IntelVPlanLoopCFU.h -------------------------------------*- C++ -*-===//
//
//   Copyright (C) 2019 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//
//
// This file implements inner loop control flow uniformity transformation.
//
//===----------------------------------------------------------------------===//
#ifndef LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_LOOPCFU_H
#define LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_LOOPCFU_H

#ifdef INTEL_CUSTOMIZATION

namespace llvm {
namespace vpo {
extern cl::opt<bool> DisableLCFUMaskRegion;
}
} // namespace llvm

void VPlanPredicator::handleInnerLoopBackedges(VPLoop *VPL) {

#ifdef VPlanPredicator
  // Community version stores VPlan reference - to minimize changes get a
  // pointer to the VPlan here.
  auto *Plan = &(this->Plan);
#endif
  VPlanDivergenceAnalysis *VPDA = Plan->getVPlanDA();

  for (auto *SubLoop : VPL->getSubLoops()) {
    // First recurse into inner loop.
    handleInnerLoopBackedges(SubLoop);

    LLVM_DEBUG(dbgs() << "Checking inner loop control flow uniformity for:\n");
    LLVM_DEBUG(dbgs() << "SubLoop at depth: " << SubLoop->getLoopDepth() << "\n");
    auto *SubLoopPreHeader = cast<VPBasicBlock>(SubLoop->getLoopPreheader());
    LLVM_DEBUG(dbgs() << "SubLoopPreHeader: " << SubLoopPreHeader->getName()
                      << "\n");
    auto *SubLoopHeader = cast<VPBasicBlock>(SubLoop->getHeader());
    LLVM_DEBUG(dbgs() << "SubLoopHeader: " << SubLoopHeader->getName() << "\n");
    auto *SubLoopLatch = cast<VPBasicBlock>(SubLoop->getLoopLatch());
    LLVM_DEBUG(dbgs() << "SubLoopLatch: " << SubLoopLatch->getName() << "\n");
    auto *SubLoopExitBlock = cast<VPBasicBlock>(SubLoop->getExitBlock());
    LLVM_DEBUG(dbgs() << "SubLoopExitBlock: " << SubLoopExitBlock->getName() << "\n");
    (void)SubLoopExitBlock; // Unused under old-predicator release build.

    auto *SubLoopRegnPred =
        SubLoopPreHeader->getSingleHierarchicalPredecessor();
    // Find inner loop top test
    assert(SubLoopRegnPred &&
           "Assumed a single predecessor of subloop contains top test");

    // Get inner loop bottom test
    VPValue *BottomTest = SubLoopLatch->getCondBit();
    assert(BottomTest && "Could not find loop exit condition\n");
    LLVM_DEBUG(dbgs() << "BottomTest: "; BottomTest->dump(); errs() << "\n");

    if (!VPDA->isDivergent(*BottomTest)) {
      LLVM_DEBUG(dbgs() << "BottomTest is uniform\n");
      continue;
    }

    LLVM_DEBUG(dbgs() << "BottomTest is divergent\n");

    LLVM_DEBUG(
        dbgs() << "SubLoop before inner loop control flow transformation\n");
    LLVM_DEBUG(SubLoop->printRPOT(dbgs(), Plan->getVPLoopInfo()));

    VPValue *TopTest = SubLoopRegnPred->getCondBit();
    if (TopTest) {
#ifdef VPlanPredicator
      auto *SubLoopRegnPredBlock = cast<VPBasicBlock>(SubLoopRegnPred);
      // If the subloop region is the false successor of the predecessor,
      // we need to negate the top test.
      if (SubLoopRegnPredBlock->getHierarchicalSuccessors()[1]
              ->getEntryBasicBlock() == SubLoopPreHeader) {
        VPBuilder::InsertPointGuard Guard(Builder);
        Builder.setInsertPoint(SubLoopRegnPredBlock);
        bool Divergent = VPDA->isDivergent(*TopTest);
        TopTest = Builder.createNot(TopTest, TopTest->getName() + ".not");
        if (Divergent)
          VPDA->markDivergent(*TopTest);
      }
#endif // VPlanPredicator
      LLVM_DEBUG(dbgs() << "Top Test: "; TopTest->dump(); errs() << "\n");
    }

    // TODO: We're going to remove hierarchical region that currently keeps the
    // DT/PDT. Once that's done, the owner would be either VPlan or HCFG,
    // probably. But there are other possibilities in the middle of the
    // transformation (like a single outermost region). Try to write this code
    // in a more generic way for now, and update once the final data structures
    // are settled.
    auto *Parent = SubLoopHeader->getParent();
    Parent->computeDT();
    VPDominatorTree *DT = Parent->getDT();
    Parent->computePDT();
    VPPostDominatorTree *PDT = Parent->getPDT();

    // Create a SESE region with a loop/bypass inside.

    VPBasicBlock *RegionEntryBlock =
        VPBlockUtils::splitBlock(SubLoopHeader, VPLI, *DT, *PDT);

    VPBasicBlock *NewLoopHeader =
        VPBlockUtils::splitBlock(RegionEntryBlock, VPLI, *DT, *PDT);

    VPBasicBlock *NewLoopLatch =
        VPBlockUtils::splitBlock(SubLoopLatch, VPLI, *DT, *PDT);

    // Note: SubLoopLatch becomes the mask region exit
    VPBasicBlock *RegionExitBlock = SubLoopLatch;

    // Remove the loop backedge condition from its original parent and place
    // in the region exit. Not needed for correctness, but easier to see
    // when debugging because the successor of the region exit block is
    // the loop latch containing this condition as the condition bit.This will
    // not work if the condition is a phi node. For this reason, only conditions
    // that are compare instructions are moved to the RegionExitBlock.
    if (dyn_cast<VPCmpInst>(BottomTest)) {
      VPBasicBlock *BottomTestBlock =
          cast<VPInstruction>(BottomTest)->getParent();
      BottomTestBlock->removeRecipe(cast<VPInstruction>(BottomTest));
      RegionExitBlock->addRecipe(cast<VPInstruction>(BottomTest));
    }

    // Move all instructions that are not phi nodes to the new loop body
    // header block. These were the instructions that were part of the
    // original loop header, but these instructions are now part of the loop
    // body that need to be under mask.
    SmallVector<VPRecipeBase *, 2> ToMove;
    SmallVector<VPRecipeBase *, 2> SubLoopHeaderPhis;
    for (auto &Inst : SubLoopHeader->vpinstructions()) {
      if (!isa<VPPHINode>(Inst))
        ToMove.push_back(&Inst);
      else
        SubLoopHeaderPhis.push_back(&Inst);
    }
    for (auto *Inst : ToMove) {
      SubLoopHeader->removeRecipe(Inst);
      NewLoopHeader->addRecipe(Inst);
    }

    // Construct loop body mask and insert into the loop header
    VPPHINode *LoopBodyMask = new VPPHINode(BottomTest->getType());
    LoopBodyMask->setName("vp.loop.mask");
    VPDA->markDivergent(*LoopBodyMask);

    if (TopTest)
      LoopBodyMask->addIncoming(TopTest, SubLoopPreHeader);
    else {
      // do/while loops won't have a top test so use 1 for incoming mask from
      // preheader.
      Constant *AllOnes = Constant::getAllOnesValue(BottomTest->getType());
      VPValue *One = Plan->getVPConstant(AllOnes);
      LoopBodyMask->addIncoming(One, SubLoopPreHeader);
    }

#ifdef VPlanPredicator
    // We are in the middle of transitioning to the new VPInstruction based
    // predicator implementation and only plan to handle inner loop flow
    // uniformity with the new predicator implementation. This code only
    // kicks in in the new predicator where we define VPlanPredicator as
    // NewVPlanPredicator.
    { // This scope is for the Guard (RAII)
      VPBuilder::InsertPointGuard Guard(Builder);
      Builder.setInsertPoint(RegionExitBlock);

      // If the back edge is the false successor of the loop latch, the bottom
      // test needs to be adjusted when masking the loop body by negating it.
      bool BackEdgeIsFalseSucc =
          NewLoopLatch->getSuccessors()[1]->getEntryBasicBlock() ==
          SubLoopHeader;

      if (BackEdgeIsFalseSucc) {
        assert(VPDA->isDivergent(*BottomTest));
        BottomTest = Builder.createNot(cast<VPInstruction>(BottomTest),
                                       BottomTest->getName() + ".not");
        VPDA->markDivergent(*BottomTest);
      }

      // Combine the bottom test with the current loop body mask - inactive
      // lanes need to to remain inactive.
      BottomTest =
          Builder.createAnd(BottomTest, cast<VPInstruction>(LoopBodyMask),
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
      for (auto *BlockBase : SubLoop->blocks()) {
        // Skip BB if it corresponds to an inner loop - we have processed
        // instructions in it already and all live-outs are through LCSSA phi of
        // that inner loop right now. We will create one more if the result
        // lives out of the current loop too when processing that inner's loop
        // exit block.
        if (VPLI->getLoopFor(BlockBase) != SubLoop)
          continue;

        auto *BB = dyn_cast<VPBasicBlock>(BlockBase);
        if (!BB)
          continue;

        // We will be adding instructions to the latch so do the copy to avoid
        // stale iterators.
        SmallVector<VPInstruction *, 16> Instructions(map_range(
            BB->vpinstructions(),
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

            if (SubLoop->contains(UserInst))
              // Not live-out.
              continue;

            // Ok, Inst is live-out, need to create a proper blend for the
            // live-out value and update the use.
            if (!Blend) {
              // Create a new phi and use mask for the current iteration.
              auto *NewPhi = new VPPHINode(Inst->getType());
              NewPhi->setName(Inst->getName() + ".live.out.prev");
              VPDA->markDivergent(*NewPhi);

              // It can be either SubLoopHeader or NewLoopLatch - doesn't really
              // matter.
              assert(SubLoopHeader->getNumPredecessors() == 2 &&
                     "Expected exactly two predecessors for SubLoopHeader!");
              SubLoopHeader->addRecipeAfter(NewPhi, nullptr /* be the first */);

              // Create the blend before population NewPhi's incoming values
              // that blend will be one of them.
              Blend = Builder.createSelect(LoopBodyMask, Inst, NewPhi,
                                           Inst->getName() + ".live.out.blend");
              VPDA->markDivergent(*Blend);
              CreatedBlends.insert(Blend);

              // We need undef for all the predecessors except NewLoopLatch.
              auto *VPUndef =
                  Plan->getVPConstant(UndefValue::get(Inst->getType()));
              for (VPBlockBase *Pred : SubLoopHeader->getPredecessors())
                NewPhi->addIncoming(Pred == NewLoopLatch ? Blend : VPUndef,
                                    cast<VPBasicBlock>(Pred));

              LLVM_DEBUG(dbgs() << "LoopCFU: Handled live-out: " << *Inst
                                << "Created blend: " << *Blend
                                << "and phi: " << *NewPhi << "\n";);
            }

            if (auto *Phi = dyn_cast<VPPHINode>(UserInst)) {
              if (Phi->getParent() == SubLoopExitBlock) {
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
              SubLoopExitBlock->addRecipeAfter(LCSSAPhi,
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

      // If back edge is the false successor, we can use new condition bit as
      // the loop latch condition. However, if back edge is the true
      // successor, we need to negate the new condition bit before using it as
      // the loop latch condition.
      if (!BackEdgeIsFalseSucc)
        NewCondBit = Builder.createNot(NewCondBit);
      NewLoopLatch->setCondBit(NewCondBit);
    }
#endif // VPlanPredicator

    LoopBodyMask->addIncoming(BottomTest, NewLoopLatch);
    SubLoopHeader->addRecipe(LoopBodyMask);
    RegionEntryBlock->setCondBit(LoopBodyMask);

    // Connect region entry/exit blocks so that predicate can be propagated
    // along mask=false path. i.e., this edge skips the loop body.
    RegionEntryBlock->appendSuccessor(RegionExitBlock);
    RegionExitBlock->appendPredecessor(RegionEntryBlock);

    if (!DisableLCFUMaskRegion) {
      VPRegionBlock *MaskRegion =
          new VPRegionBlock(VPBlockBase::VPRegionBlockSC,
                            VPlanUtils::createUniqueName("mask_region"));
      VPBlockUtils::insertRegion(MaskRegion, RegionEntryBlock, RegionExitBlock,
                                 false);

      auto *SubLoopRegion =
          cast_or_null<VPLoopRegion>(SubLoop->getHeader()->getParent());

      // The new region parent is the loop.
      MaskRegion->setParent(SubLoopRegion);
      SubLoop->addBasicBlockToLoop(MaskRegion, *VPLI);

      // All blocks in the new region must have the parent set to the new
      // region.
      for (VPBlockBase *RegionBlock :
           make_range(df_iterator<VPRegionBlock *>::begin(MaskRegion),
                      df_iterator<VPRegionBlock *>::end(MaskRegion))) {
        if (RegionBlock->getParent() == SubLoopRegion)
          RegionBlock->setParent(MaskRegion);
      }
    }

    LLVM_DEBUG(
        dbgs() << "Subloop after inner loop control flow transformation\n");
    LLVM_DEBUG(SubLoop->printRPOT(dbgs(), Plan->getVPLoopInfo()));
  }
}
#endif // INTEL_CUSTOMIZATION
#endif // LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_LOOPCFU_H
