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

  LLVM_DEBUG(dbgs() << "Before inner loop control flow transformation\n");
  LLVM_DEBUG(Plan->dump());

  VPlanDivergenceAnalysis *VPDA = Plan->getVPlanDA();

  for (auto *SubLoop : LoopRegion->getVPLoop()->getSubLoops()) {
    auto *SubLoopRegion = cast<VPLoopRegion>(SubLoop->getHeader()->getParent());
    LLVM_DEBUG(dbgs() << "Checking inner loop control flow uniformity for:\n");
    LLVM_DEBUG(dbgs() << "SubLoopRegion: " << SubLoopRegion->getName() << "\n");
    auto *SubLoopPreHeader = cast<VPBasicBlock>(SubLoop->getLoopPreheader());
    LLVM_DEBUG(dbgs() << "SubLoopPreHeader: " << SubLoopPreHeader->getName()
                      << "\n");
    auto *SubLoopHeader = cast<VPBasicBlock>(SubLoop->getHeader());
    LLVM_DEBUG(dbgs() << "SubLoopHeader: " << SubLoopHeader->getName() << "\n");
    auto *SubLoopLatch = cast<VPBasicBlock>(SubLoop->getLoopLatch());
    LLVM_DEBUG(dbgs() << "SubLoopLatch: " << SubLoopLatch->getName() << "\n");

    // Find inner loop top test
    assert(SubLoopRegion->getSinglePredecessor() &&
           "Assumed a single predecessor of subloop contains top test");
    VPValue *TopTest = SubLoopRegion->getSinglePredecessor()->getCondBit();
    if (TopTest)
      LLVM_DEBUG(dbgs() << "Top Test: "; TopTest->dump(); errs() << "\n");

    // Get inner loop bottom test
    VPValue *BottomTest = SubLoopLatch->getCondBit();
    assert(BottomTest && "Could not find loop exit condition\n");
    LLVM_DEBUG(dbgs() << "BottomTest: "; BottomTest->dump(); errs() << "\n");

    if (VPDA->isDivergent(*BottomTest)) {

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
      for (auto &Inst : SubLoopHeader->getInstList()) {
        if (!isa<VPPHINode>(Inst))
          ToMove.push_back(&Inst);
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
      LoopBodyMask->addIncoming(BottomTest, NewLoopLatch);
      SubLoopHeader->addRecipe(LoopBodyMask);
      RegionEntryBlock->setCondBit(LoopBodyMask, Plan);

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
        if (VPBasicBlock *RegionBasicBlock =
                dyn_cast<VPBasicBlock>(RegionBlock))
          RegionBasicBlock->setParent(MaskRegion);
      }
    }
  }

  LLVM_DEBUG(dbgs() << "After inner loop control flow transformation\n");
  LLVM_DEBUG(Plan->dump());
}
#endif // INTEL_CUSTOMIZATION
#endif // LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_LOOPCFU_H
