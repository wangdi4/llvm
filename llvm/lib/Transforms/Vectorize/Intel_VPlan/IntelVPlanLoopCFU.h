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

void VPlanPredicator::handleInnerLoopBackedges(VPLoopRegion *LoopRegion) {

#ifdef VPlanPredicator
  // Community version stores VPlan reference - to minimize changes get a
  // pointer to the VPlan here.
  auto *Plan = &(this->Plan);
#endif
  VPlanDivergenceAnalysis *VPDA = Plan->getVPlanDA();

  for (auto *SubLoop : LoopRegion->getVPLoop()->getSubLoops()) {
    auto *SubLoopRegion = cast<VPLoopRegion>(SubLoop->getHeader()->getParent());
    // First recurse into inner loop.
    handleInnerLoopBackedges(SubLoopRegion);

    LLVM_DEBUG(dbgs() << "Checking inner loop control flow uniformity for:\n");
    LLVM_DEBUG(dbgs() << "SubLoopRegion: " << SubLoopRegion->getName() << "\n");
    auto *SubLoopPreHeader = cast<VPBasicBlock>(SubLoop->getLoopPreheader());
    LLVM_DEBUG(dbgs() << "SubLoopPreHeader: " << SubLoopPreHeader->getName()
                      << "\n");
    auto *SubLoopHeader = cast<VPBasicBlock>(SubLoop->getHeader());
    LLVM_DEBUG(dbgs() << "SubLoopHeader: " << SubLoopHeader->getName() << "\n");
    auto *SubLoopLatch = cast<VPBasicBlock>(SubLoop->getLoopLatch());
    LLVM_DEBUG(dbgs() << "SubLoopLatch: " << SubLoopLatch->getName() << "\n");

    auto *SubLoopRegnPred = SubLoopRegion->getSinglePredecessor();
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
    LLVM_DEBUG(SubLoopRegion->dump());

    VPValue *TopTest = SubLoopRegnPred->getCondBit();
    if (TopTest) {
#ifdef VPlanPredicator
      auto *SubLoopRegnPredBlock = cast<VPBasicBlock>(SubLoopRegnPred);
      // If the subloop region is the false successor of the predecessor,
      // we need to negate the top test.
      if (SubLoopRegnPredBlock->getSuccessors()[1] == SubLoopRegion) {
        VPBuilder::InsertPointGuard Guard(Builder);
        Builder.setInsertPoint(SubLoopRegnPredBlock);
        TopTest = Builder.createNot(TopTest);
      }
#endif // VPlanPredicator
      LLVM_DEBUG(dbgs() << "Top Test: "; TopTest->dump(); errs() << "\n");
    }

    SubLoopRegion->computeDT();
    VPDominatorTree *DT = SubLoopRegion->getDT();
    SubLoopRegion->computePDT();
    VPPostDominatorTree *PDT = SubLoopRegion->getPDT();

    VPBasicBlock *RegionEntryBlock =
        VPBlockUtils::splitBlock(SubLoopHeader, VPLI, *DT, *PDT, Plan);

    VPBasicBlock *NewLoopHeader =
        VPBlockUtils::splitBlock(RegionEntryBlock, VPLI, *DT, *PDT, Plan);

    VPBasicBlock *NewLoopLatch =
        VPBlockUtils::splitBlock(SubLoopLatch, VPLI, *DT, *PDT, Plan);

    // Note: SubLoopLatch becomes the mask region exit
    VPBasicBlock *RegionExitBlock = SubLoopLatch;

    // Remove the loop backedge condition from its original parent and place
    // in the region exit. Not needed for correctness, but easier to see
    // when debugging because the successor of the region exit block is
    // the loop latch containing this condition as the condition bit.
    SubLoopLatch->removeRecipe(cast<VPInstruction>(BottomTest));
    RegionExitBlock->addRecipe(cast<VPInstruction>(BottomTest));

    // Move all instructions that are not phi nodes to the new loop body
    // header block. These were the instructions that were part of the
    // original loop header, but these instructions are now part of the loop
    // body that need to be under mask.
    SmallVector<VPRecipeBase *, 2> ToMove;
    SmallVector<VPRecipeBase *, 2> SubLoopHeaderPhis;
    for (auto &Inst : SubLoopHeader->getInstList()) {
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

      if (BackEdgeIsFalseSucc)
        BottomTest = Builder.createNot(cast<VPInstruction>(BottomTest));

      // Combine the bottom test with the current loop body mask - inactive
      // lanes need to to remain inactive.
      BottomTest =
          Builder.createAnd(BottomTest, cast<VPInstruction>(LoopBodyMask));

      // The subloop header phis are live out of the subloop. The incoming
      // value of such phis from the loop latch need to be blended in with
      // the phi value using bottom test as the mask.
      for (auto *Inst : SubLoopHeaderPhis) {
        auto *VPPhi = cast<VPPHINode>(Inst);
        auto *IncomingValForLatch = VPPhi->getIncomingValue(NewLoopLatch);
        auto *Blend =
            Builder.createSelect(BottomTest, IncomingValForLatch, VPPhi);
        VPPhi->setIncomingValue(VPPhi->getBlockIndex(NewLoopLatch), Blend);
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
      Plan->setCondBitUser(NewCondBit, NewLoopLatch);
    }
#endif // VPlanPredicator

    LoopBodyMask->addIncoming(BottomTest, NewLoopLatch);
    SubLoopHeader->addRecipe(LoopBodyMask);
    RegionEntryBlock->setCondBit(LoopBodyMask);
    Plan->setCondBitUser(LoopBodyMask, RegionEntryBlock);

    // Connect region entry/exit blocks so that predicate can be propagated
    // along mask=false path. i.e., this edge skips the loop body.
    RegionEntryBlock->appendSuccessor(RegionExitBlock);
    RegionExitBlock->appendPredecessor(RegionEntryBlock);

    VPRegionBlock *MaskRegion =
        new VPRegionBlock(VPBlockBase::VPRegionBlockSC,
                          VPlanUtils::createUniqueName("mask_region"));
    VPBlockUtils::insertRegion(MaskRegion, RegionEntryBlock, RegionExitBlock,
                               false);

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
    LLVM_DEBUG(
        dbgs() << "Subloop after inner loop control flow transformation\n");
    LLVM_DEBUG(SubLoopRegion->dump());
  }
}
#endif // INTEL_CUSTOMIZATION
#endif // LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_LOOPCFU_H
