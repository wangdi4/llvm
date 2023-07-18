//===- IntelVPlanTransformEarlyExitLoop.cpp ------------------------------===//
//
//   Copyright (C) 2023 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===---------------------------------------------------------------------===//
///
/// This file implements the VPTransformEarlyExitLoop class.
//===---------------------------------------------------------------------===//

#include "IntelVPlanTransformEarlyExitLoop.h"
#include "IntelVPlanBuilder.h"

using namespace llvm;
using namespace llvm::vpo;

#define DEBUG_TYPE "TransformEarlyExitLoop"

static LoopVPlanDumpControl
    TransformEarlyExitLoopDumpsControl("transformed-early-exit-loop",
                                       "transforming early-exit loop");

void VPTransformEarlyExitLoop::transform() {
  VPBuilder Builder;
  auto IsVPEECondPred = [](VPInstruction &I) {
    return isa<VPEarlyExitCond>(I);
  };

  // Capture early-exit condition that was generated for early-exit VPlan.
  auto VPEECondIt = find_if(vpinstructions(&Plan), IsVPEECondPred);

  // If we don't have one, it implies early-exit loop is not being explicitly
  // represented in VPlan IR. Nothing more to do in the transform.
  if (VPEECondIt == vpinst_end(&Plan))
    return;

  // Assert that we have only one VPEarlyExitCond.
  assert(count_if(vpinstructions(&Plan), IsVPEECondPred) == 1 &&
         "Currently only one early-exit condition is expected.");

  auto *VPEECond = cast<VPEarlyExitCond>(&*VPEECondIt);
  LLVM_DEBUG(dbgs() << "Captured early-exit condition: "; VPEECond->dump();
             dbgs() << "\n");
  Builder.setInsertPoint(VPEECond);
  Builder.setCurrentDebugLocation(VPEECond->getDebugLocation());

  // Step 1
  // ======
  //
  // Convert the VPEarlyExitCond into a VPEarlyExitExecMask to track the
  // lanes where loop body is executed. This is done by flipping false value to
  // true in early-exit condition until first true value is encountered. For
  // example, if the early-exit condition for 4 iterations looks like <0, 1, 0,
  // 1>, the execution mask for loop body should look like <1, 0, 0, 0>.
  // VPEarlyExitExecMask represents this idiomatic operation.

  auto *VPEEExecMask = Builder.create<VPEarlyExitExecMask>(
      "early.exit.exec.mask", VPEECond->getOperand(0));
  VPEECond->replaceAllUsesWith(VPEEExecMask);
  VPEECond->getParent()->eraseInstruction(VPEECond);

  // Early-exit condition is expected to have a single branch instruction user.
  assert(VPEEExecMask->getNumUsers() == 1 &&
         "Early-exit condition expected to have single unique user.");
  auto *EEBranch = cast<VPBranchInst>(*VPEEExecMask->user_begin());
  // Flip the edges of the branch since the CondBit now indicates mask that
  // executes loop body.
  EEBranch->swapSuccessors();

  // Step 2
  // ======
  //
  // Make the early-exit loop's backedge condition uniform. mergeLoopExits
  // massages the CFG to generate a divergent backedge condition emitted in the
  // new loop latch. We make it uniform by emitting an all-zero-check or
  // all-one-check. If the loop header is taken on true branch of backedge, then
  // we update the condition to be all-one-check (not + all-zero). Otherwise we
  // update the condition to be all-zero-check. TODO: Consider unifying the
  // implementation below with VPlanPredicator::fixupUniformInnerLoops and
  // similar code in LoopCFU in the future.
  auto *EEVPLoop = Plan.getVPLoopInfo()->getLoopFor(VPEEExecMask->getParent());
  VPBasicBlock *EELoopLatch = EEVPLoop->getLoopLatch();
  auto *EEBackedge = EELoopLatch->getTerminator();
  LLVM_DEBUG(dbgs() << "Captured early-exit backedge: "; EEBackedge->dump();
             dbgs() << "\n");

  VPBasicBlock *EEHeader = EEVPLoop->getHeader();
  assert((EEBackedge->getSuccessor(0) == EEHeader ||
          EEBackedge->getSuccessor(1) == EEHeader) &&
         "Early-exit loop's backedge does not lead to loop header.");
  bool IsHeaderTakenOnTrue =
      EEBackedge->getSuccessor(0 /*true succ*/) == EEHeader;

  // We insert new instructions before backedge to make it uniform.
  Builder.setInsertPoint(EEBackedge);
  auto *EELatchCond = EEBackedge->getCondition();
  // Canonicalization to determine if we need an all-one-check or all-zero-check
  // based on which branch the header is taken on.
  if (IsHeaderTakenOnTrue)
    EELatchCond = Builder.createNot(EELatchCond, "ee.latch.cond.canon");

  auto *EELatchAllZeroCheck =
      Builder.createAllZeroCheck(EELatchCond, "ee.mask.is.zero");
  // Update CondBit of backedge.
  EEBackedge->setCondition(EELatchAllZeroCheck);

  VPLAN_DUMP(TransformEarlyExitLoopDumpsControl, Plan);
}
