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

static cl::opt<bool> EnableLiveOutsRematerialization(
    "vplan-enable-liveout-remat", cl::init(true),
    cl::desc("Enable live-outs rematerialization in inner loops to avoid extra "
             "recurrence introduced by LoopCFU."));

namespace llvm {
namespace vpo {

void VPlanLoopCFU::run(VPLoop *VPL) {
  if (EnableLiveOutsRematerialization)
    rematerializeLiveOuts(VPL);

  VPlanDivergenceAnalysisBase *VPDA = Plan.getVPlanDA();
  VPLoopInfo *VPLI = Plan.getVPLoopInfo();

  VPBasicBlock *VPLPreHeader = VPL->getLoopPreheader();
  VPBasicBlock *VPLHeader = VPL->getHeader();
  VPBasicBlock *VPLLatch = VPL->getLoopLatch();
  VPBasicBlock *VPLExitBlock = VPL->getExitBlock();
  assert(VPLExitBlock && "Loop hasn't been canonicalized!");
  assert(VPLLatch == VPL->getExitingBlock() &&
         "Loop exits canonicalization hasn't been done!");

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

  // Note: this condition is always true for the outermost loop in loop
  // vectorization but that isn't guaranteed for function vectorization. Just
  // use the generic approach.
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
      if (VPLRegnPred->getSuccessor(1) == VPLPreHeader) {
        TopTest = VPBuilder()
                      .setInsertPoint(VPLRegnPred)
                      .createNot(TopTest, TopTest->getName() + ".not");
        VPDA->updateDivergence(*TopTest);
      }
      LLVM_DEBUG(dbgs() << "Top Test: "; TopTest->dump(); errs() << "\n");
    }
  }

  VPDominatorTree *DT = Plan.getDT();
  VPPostDominatorTree *PDT = Plan.getPDT();

  // Create a phi for the mask. We'll use it as a condtion to branch so creation
  // needs to happen before CFG manipulation.
  VPBuilder Builder;
  Builder.setInsertPointFirstNonPhi(VPLHeader);
  VPPHINode *LoopBodyMask =
      Builder.createPhiInstruction(BottomTest->getType(), "vp.loop.mask");
  VPDA->markDivergent(*LoopBodyMask);

  // Modify the CFG:
  //
  //  Header
  //(F)|  \ (T)
  //   |  MaskedRegionStart (previously header after all the phis)
  //   |   |
  //   |  MaskedRegionLast (previously latch), might be equal to Start
  //   |  /
  //   NewLoopLatch
  VPBasicBlock *MaskedRegionStart =
      VPBlockUtils::splitBlockBegin(VPLHeader, VPLI, DT, PDT);

  // Latch might have changed during splitting, ask VPL about new one.
  VPBasicBlock *MaskedRegionLast = VPL->getLoopLatch();
  VPBasicBlock *NewLoopLatch =
      VPBlockUtils::splitBlockEnd(MaskedRegionLast, VPLI, DT, PDT);

  // TODO: This is the only place that doesn't incrementally update DT/PDT.
  VPLHeader->setTerminator(MaskedRegionStart, NewLoopLatch, LoopBodyMask);

  if (TopTest)
    LoopBodyMask->addIncoming(TopTest, VPLPreHeader);
  else {
    // do/while loops won't have a top test so use 1 for incoming mask from
    // preheader.
    Constant *AllOnes = Constant::getAllOnesValue(BottomTest->getType());
    VPValue *One = Plan.getVPConstant(AllOnes);
    LoopBodyMask->addIncoming(One, VPLPreHeader);
  }

  Builder.setInsertPoint(NewLoopLatch);

  // If the back edge is the false successor of the loop latch, the bottom
  // test needs to be adjusted when masking the loop body by negating it.
  bool BackEdgeIsFalseSucc = NewLoopLatch->getSuccessor(1) == VPLHeader;

  if (BackEdgeIsFalseSucc) {
    assert(VPDA->isDivergent(*BottomTest));
    BottomTest = Builder.createNot(cast<VPInstruction>(BottomTest),
                                   BottomTest->getName() + ".not");
    VPDA->markDivergent(*BottomTest);
  }

  // Combine the bottom test with the current loop body mask - inactive
  // lanes need to to remain inactive.
  BottomTest = Builder.createAnd(BottomTest, LoopBodyMask,
                                 LoopBodyMask->getName() + ".next");
  VPDA->markDivergent(*BottomTest);

  // Compute and set the new condition bit in the loop latch. If all the
  // lanes are inactive, new condition bit will be true.
  auto *NewCondBit = Builder.createAllZeroCheck(BottomTest);
  VPDA->updateDivergence(*NewCondBit);
  LoopBodyMask->addIncoming(BottomTest, NewLoopLatch);

  NewLoopLatch->setTerminator(VPLExitBlock, VPLHeader, NewCondBit);
  Builder.setInsertPoint(NewLoopLatch);

  // Basic CFG and mask manipulations are finished. Now handle the live outs.
  //
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
  assert(VPL->isLCSSAForm() && "Loop isn't in LCSSA form!");
  assert(VPLExitBlock->getSinglePredecessor() &&
         "Loop not in canonical form!");
  VPBuilder BlendBuilder;
  BlendBuilder.setInsertPointFirstNonPhi(NewLoopLatch);
  for (VPPHINode &LCSSAPhi : VPLExitBlock->getVPPhis()) {
    auto *LiveOut = dyn_cast<VPInstruction>(LCSSAPhi.getIncomingValue(0u));
    if (!LiveOut || !VPL->contains(LiveOut))
      continue;

    auto *NewPhi =
        VPBuilder().setInsertPointFirstNonPhi(VPLHeader).createPhiInstruction(
            LiveOut->getType(), LiveOut->getName() + ".live.out.prev");
    VPDA->markDivergent(*NewPhi);
    VPValue *Blend = BlendBuilder.createSelect(
        LoopBodyMask, LiveOut, NewPhi, LiveOut->getName() + ".live.out.blend");
    VPDA->markDivergent(*Blend);

    // We need undef for all the predecessors except NewLoopLatch.
    auto *VPUndef = Plan.getVPConstant(UndefValue::get(LiveOut->getType()));
    for (VPBasicBlock *Pred : VPLHeader->getPredecessors())
      NewPhi->addIncoming(Pred == NewLoopLatch ? Blend : VPUndef, Pred);

    LCSSAPhi.setIncomingValue(0u, Blend);
    LLVM_DEBUG(dbgs() << "LoopCFU: Handled live-out: " << *LiveOut
                      << "Created blend: " << *Blend << "and phi: " << *NewPhi
                      << "\n";);
  }

  LLVM_DEBUG(
      dbgs() << "Subloop after inner loop control flow transformation\n");
  LLVM_DEBUG(VPL->printRPOT(dbgs(), Plan.getVPLoopInfo()));
}

static bool isRematerializable(const VPInstruction *Inst) {
  switch (Inst->getOpcode()) {
  default:
    return false;
  case Instruction::ICmp:
  case Instruction::Add:
  case Instruction::Sub:
  case Instruction::GetElementPtr:
  case Instruction::BitCast:
    return true;
  }
}

VPInstruction *VPlanLoopCFU::tryRematerializeLiveOut(VPLoop *VPL,
                                                     VPInstruction *LiveOut) {
  if (!isRematerializable(LiveOut))
    return nullptr;

  VPBasicBlock *Exit = VPL->getExitBlock();
  SmallVector<std::pair<VPValue * /* OrigOperand */,
                        VPValue * /* Rematerialization Op */>,
              4>
      LiveOutOps;

  for (VPValue *Op : LiveOut->operands()) {
    if (isa<VPConstant>(Op)) {
      // TODO: Extend to anything dominating the loop.
      LiveOutOps.emplace_back(Op, Op);
      continue;
    }
    auto LCSSAIt = llvm::find_if(Op->users(), [Exit](const VPUser *U) {
      auto *LCSSAPhi = dyn_cast<VPPHINode>(U);
      return LCSSAPhi && LCSSAPhi->getParent() == Exit;
    });
    if (LCSSAIt == Op->users().end())
      return nullptr;

    LiveOutOps.emplace_back(Op, *LCSSAIt);
  }

  auto *Rematerialized = LiveOut->clone();
  VPBuilder().setInsertPointFirstNonPhi(Exit).insert(Rematerialized);
  Plan.getVPlanDA()->markDivergent(*Rematerialized);

  for (const auto &It : LiveOutOps)
    Rematerialized->replaceUsesOfWith(It.first, It.second);

  LLVM_DEBUG(dbgs() << "Rematerialized " << *LiveOut << " in\n"
                    << *LiveOut->getParent() << "\ninto " << *Rematerialized
                    << "\n");
  return Rematerialized;
}

void VPlanLoopCFU::rematerializeLiveOuts(VPLoop *VPL) {
  if (!VPL->getParentLoop() && !VPL->isLCSSAForm())
    return;

  assert(VPL->isLCSSAForm() && "Loop isn't in LCSSA form!");
  VPBasicBlock *Exit = VPL->getExitBlock();
  assert(Exit && Exit->getSinglePredecessor() && "Loop not in canonical form!");
  SmallVector<VPPHINode *, 4> Phis;
  for (VPPHINode &LCSSAPhi : Exit->getVPPhis())
    Phis.push_back(&LCSSAPhi);

  for (VPPHINode *LCSSAPhi : Phis) {
    auto *LiveOut = dyn_cast<VPInstruction>(LCSSAPhi->getIncomingValue(0u));
    if (!LiveOut || !VPL->contains(LiveOut))
      continue;

    if (VPInstruction *Rematerialized = tryRematerializeLiveOut(VPL, LiveOut)) {
      LCSSAPhi->replaceAllUsesWith(Rematerialized);
      LCSSAPhi->getParent()->eraseInstruction(LCSSAPhi);
    }
  }
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
