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
  //
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
  //
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

  // Step 3
  // ======
  //
  // Identify the first lane in vector iteration that takes early-exit, if any.
  // We use the following properties/guarantees about the loop from
  // mergeLoopExits -
  //   - loop has a single merged exit block
  //   - merged exit block has a series of cascaded checks to divert
  //   control-flow to appropriate exit block
  //   - mergeLoopExits reserves the ExitID=0 to represent the main exit from
  //   loop and all side exits have ExitID > 0
  //
  // Based on these properties we compute mask to identify early-exit lane via
  // ExitID != 0. For example, if ExitID is <0, 0, 1, 2>, then mask will be <0,
  // 0, 1, 1> and early-exit lane will be 2.
  //
  VPBasicBlock *EEMergedExit = EEVPLoop->getExitBlock();
  assert(
      EEMergedExit &&
      "Explicit early-exit loop is expected to have a single merged exit BB.");

  // Get the merged exit blocks terminator. It should start the cascaded-if
  // series of operations.
  auto *ExitBranch = cast<VPBranchInst>(EEMergedExit->getTerminator());
  assert(ExitBranch->getNumSuccessors() == 2 &&
         "Expected 2 successors for merged exit of explicit early-exit loop.");
  auto *CascadedIfCond = cast<VPCmpInst>(ExitBranch->getCondition());

  // Obtain the exit ID PHI using operands of the cascaded-if CondBit. Pseudo
  // VPlan-IR for reference - merged.exit:
  //   %c = icmp eq %exit-id-phi, 0
  //   br %c, %main.exit, %side.exit
  unsigned ExitIDPhiOperandNum =
      isa<VPConstant>(CascadedIfCond->getOperand(0)) ? 1 : 0;
  auto *ExitIDPhi =
      cast<VPPHINode>(CascadedIfCond->getOperand(ExitIDPhiOperandNum));
  LLVM_DEBUG(dbgs() << "Captured exit ID phi: "; ExitIDPhi->dump();
             dbgs() << "\n");

  // Insert new instructions to compute the early-exit lane in merged exit,
  // before the cascaded-if condition.
  Builder.setInsertPoint(CascadedIfCond);
  // ExitID = 0 is reserved for main exit by mergeLoopExits. So use the login
  // ExitID != 0 to identify lanes that take side-exits.
  auto *EEIDMask = Builder.createCmpInst(
      CmpInst::ICMP_NE, ExitIDPhi,
      Plan.getVPConstant(ConstantInt::get(ExitIDPhi->getType(), 0)));
  auto *EELane = Builder.create<VPEarlyExitLane>("early.exit.lane", EEIDMask);

  // Emit custom select operations to choose between valid early-exit lane or
  // first/last lane -
  //   %cond = icmp ne %ee.lane, -1
  //   %ee.or.first.lane = select-val-or-lane %cond, %ee.lane, first
  //   %ee.or.last.lane = select-val-or-lane %cond, %ee.lane, last
  auto *ValidEELaneCond = Builder.createCmpInst(
      CmpInst::ICMP_NE, EELane,
      Plan.getVPConstant(ConstantInt::get(EELane->getType(), -1)));
  auto *EEOrFirstLane = Builder.create<VPSelectValOrLane>(
      "ee.or.first.lane", ValidEELaneCond, EELane, true /*UseFirstLane*/);
  auto *EEOrLastLane = Builder.create<VPSelectValOrLane>(
      "ee.or.last.lane", ValidEELaneCond, EELane, false /*UseFirstLane*/);

  // Step 4
  // ======
  //
  // Adjust entity finalization instructions. VPLoopEntity framework assumes
  // single exit loop and emits finalization in the main exit block. We need to
  // move it to the new merged exit block and adjust operands to account for
  // cases where early-exit is taken in loop.
  unsigned ExitIDConstOperandNum = 1 - ExitIDPhiOperandNum;
  unsigned ExitIDConst =
      cast<VPConstantInt>(CascadedIfCond->getOperand(ExitIDConstOperandNum))
          ->getZExtValue();
  // Identify the main exit block using the condition used in cascaded if block.
  // Note that ExitID = 0 is reserved for the main exit BB by mergeLoopExits.
  VPBasicBlock *MainExitBB = ExitIDConst != 0 ? ExitBranch->getSuccessor(1)
                                              : ExitBranch->getSuccessor(0);
  LLVM_DEBUG(dbgs() << "Main exit BB: " << MainExitBB->getName() << "\n\n");

  // List of instructions to move from main exit block to merged exit block.
  SmallVector<VPInstruction *, 4> InstsToMove;

  // Iterate over instructions in main exit block to process entity
  // finalization-related instructions.
  for (auto &I : *MainExitBB) {
    if (auto *IndFinal = dyn_cast<VPInductionFinal>(&I)) {
      // Induction finalization -
      // Convert induction-final to extract version using the value along
      // backedge and early-exit lane to specify the lane to extract from.
      VPPHINode *MainIVPhi = EEVPLoop->getInductionPHI();
      LLVM_DEBUG(dbgs() << "Main IV phi: "; MainIVPhi->dump(); dbgs() << "\n");
      VPValue *MainIVLatchVal = MainIVPhi->getIncomingValue(EELoopLatch);
      IndFinal->setExtractOperands(MainIVLatchVal, EEOrFirstLane);
      InstsToMove.push_back(IndFinal);
    } else if (I.getOpcode() == VPInstruction::PrivateFinalUncond) {
      // Unconditional lastprivate finalization -
      // Add early-exit lane as an extra operand to specify lane to extract
      // from.
      auto *PvtFinal = &I;
      PvtFinal->addOperand(EEOrLastLane);
      InstsToMove.push_back(PvtFinal);
    }
  }

  // Track the last inserted instruction in current BB.
  VPInstruction *LastInsertInst = EEOrLastLane;
  for (auto *I : InstsToMove) {
    MainExitBB->removeInstruction(I);
    EEMergedExit->addInstructionAfter(I, LastInsertInst);
    LastInsertInst = I;
  }

  // Step 5
  // ======
  //
  // Add instruction to compute the final value of exit ID at the merged exit
  // block. This extra wrapper is needed to account for cases where none of the
  // loop iterations take the early-exit.
  Builder.setInsertPoint(CascadedIfCond);
  auto *EEID =
      Builder.create<VPEarlyExitID>("early.exit.id", ExitIDPhi, EEOrLastLane);
  // Use the final value of exit ID to divert control flow in the cascaded-if.
  CascadedIfCond->replaceUsesOfWith(ExitIDPhi, EEID);

  VPLAN_DUMP(TransformEarlyExitLoopDumpsControl, Plan);
}
