//===-- IntelVPlanLoopUnroller.cpp ------------------------------*- C++ -*-===//
//
//   Copyright (C) 2020 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//

#include "IntelVPlanLoopUnroller.h"
#include "IntelVPlanClone.h"
#include "llvm/Analysis/Intel_LoopAnalysis/IR/HLInst.h"
#include "llvm/Support/CommandLine.h"

#define DEBUG_TYPE "VPlanLoopUnroller"

using namespace llvm::vpo;

static cl::opt<bool>
    VPlanPrintUnroll("vplan-print-after-unroll", cl::init(false),
                     cl::desc("Print plain dump after VPlan loop unrolling"));

void VPlanLoopUnroller::run(VPInstUnrollPartTy *VPInstUnrollPart) {
  assert(UF > 1 && "Can't unroll with unroll factor less than 2");

  VPLoopInfo *VPLI = Plan.getVPLoopInfo();
  assert(std::distance(VPLI->begin(), VPLI->end()) == 1 &&
         "Expected single outermost loop!");

  VPLoop *VPL = *VPLI->begin();
  assert(VPL->getSubLoops().empty() &&
         "Unrolling of loops with subloops is not supported");

  // Collect loop live-out users to process them after the unroll.
  DenseMap<VPUser *, VPInstruction *> LiveOutUsers;
  for (VPBasicBlock *Block : VPL->blocks())
    for (VPInstruction &Inst : *Block)
      for (VPUser *User : Inst.users())
        if (isa<VPExternalUse>(User)) {
          assert(LiveOutUsers.find(User) == LiveOutUsers.end() &&
                 "Only one instruction for a live-out user is supported");
          LiveOutUsers[User] = &Inst;
        } else if (auto UseInst = dyn_cast<VPInstruction>(User))
          if (!VPL->contains(UseInst)) {
            assert(LiveOutUsers.find(User) == LiveOutUsers.end() &&
                   "Only one instruction for a live-out user is supported");
            LiveOutUsers[User] = &Inst;
          }

  VPBasicBlock *Header = VPL->getHeader();
  assert(Header && "Expected single header block");
  VPBasicBlock *Latch = VPL->getLoopLatch();
  assert(Latch && "Expected single latch block");
  VPBasicBlock *CurrentLatch = Latch;

  // TODO: Support explicit uniform IV in the vectorized loop for HIR pipeline.
  if (auto *Cmp = dyn_cast<VPInstruction>(CurrentLatch->getCondBit()))
    if (auto *VTC = dyn_cast<VPVectorTripCountCalculation>(Cmp->getOperand(1)))
      VTC->setUF(VTC->getUF() * UF);

  SmallVector<VPCloneUtils::Value2ValueMapTy, 8> Clones(UF - 1);
  for (unsigned Part = 0; Part < UF - 1; Part++) {
    VPCloneUtils::cloneBlocksRange(Header, Latch, Clones[Part],
                                   Plan.getVPlanDA());
  }

  // Hold the current last update instruction for each header PHI node.
  DenseMap<VPInstruction *, VPValue *> PHILastUpdate;

  // Main loop. Repeats the loop N times (where N = UF - 1).
  //   For example:
  //     Header <---+
  //     [ phis ]   |
  //     |          |
  //     Body       |
  //     |          |
  //     Latch -----+
  //
  //   Will be expanded into (with UF = 3, N = 2):
  //     Header <----------+
  //     [ phis ]          |
  //     |                 |
  //     Body              |
  //     |                 |
  //     Latch             |
  //     |                 |
  //     Header Clone #1   |
  //     |                 |
  //     Body Clone #1     |
  //     |                 |
  //     Latch Clone #1    |
  //     |                 |
  //     Header Clone #2   |
  //     |                 |
  //     Body Clone #2     |
  //     |                 |
  //     Latch Clone #2 ---+
  for (unsigned Part = 0; Part < UF - 1; Part++) {
    VPCloneUtils::Value2ValueMapTy &ValueMap = Clones[Part];

    VPCloneUtils::Value2ValueMapTy ReverseMap;
    for (auto It : ValueMap)
      ReverseMap[It.second] = It.first;
    assert(ReverseMap.size() == ValueMap.size() &&
           "Expecting unique values only in ValueMap");

    auto *ClonedHeader = cast<VPBasicBlock>(ValueMap[Header]);
    auto *ClonedLatch = cast<VPBasicBlock>(ValueMap[Latch]);

    // Process induction/reduction PHI nodes.
    std::set<VPInstruction *> InstToRemove;
    for (VPPHINode &ClonedInst : ClonedHeader->getVPPhis()) {
      auto It = ReverseMap.find(&ClonedInst);
      assert(It != ReverseMap.end() &&
             "ReverseMap should contain all the cloned instructions");

      VPPHINode *OrigInst = dyn_cast<VPPHINode>(It->second);
      assert(OrigInst &&
             "The clone of VPPHINode expected to be a VPPHINode too");
      assert(OrigInst->getNumOperands() == 2 &&
             "All PHIs in loop header block should have only 2 value operands");

      // Try to deduce induction/reduction PHI node.
      VPInstruction *Op0 = dyn_cast<VPInstruction>(OrigInst->getOperand(0));
      VPInstruction *Op1 = dyn_cast<VPInstruction>(OrigInst->getOperand(1));

      VPInstruction *LastUpdate = nullptr;
      if (Op0 && VPL->contains(Op0) && (!Op1 || !VPL->contains(Op1)))
        LastUpdate = Op0;
      else if (Op1 && VPL->contains(Op1) && (!Op0 || !VPL->contains(Op0)))
        LastUpdate = Op1;
      assert(LastUpdate &&
             "Expecting to have only induction/reduction PHIs here");

      if (PHILastUpdate.find(OrigInst) == PHILastUpdate.end())
        PHILastUpdate[OrigInst] = LastUpdate;

      // Update the unrolled loop header PHI's incoming block
      // to cloned loop latch.
      OrigInst->setIncomingBlock(OrigInst->getBlockIndex(CurrentLatch),
                                 ClonedLatch);

      // For the current iteration replace a clone of the original PHI with
      // the current last update instruction.
      ValueMap[OrigInst] = PHILastUpdate[OrigInst];

      // Replace the last update instruction with the cloned one
      // related to the current iteration.
      PHILastUpdate[OrigInst] = ValueMap[LastUpdate];

      // Not actually a live-out, but this will help to replace phi's operand
      // to the final last update instruction
      LiveOutUsers[OrigInst] = LastUpdate;

      InstToRemove.insert(&ClonedInst);
    }

    for (auto It : InstToRemove)
      ClonedHeader->eraseInstruction(It);

    // Remap operands.
    VPValueMapper Mapper(ValueMap);
    auto UnrollerPart = [VPInstUnrollPart, Part](VPInstruction &Inst) {
      if (VPInstUnrollPart)
        VPInstUnrollPart->insert(std::make_pair(&Inst, Part + 1));
    };

    for (VPBasicBlock *Block : VPL->blocks())
      Mapper.remapOperands(Block, UnrollerPart);

    // Insert cloned blocks into the loop.
    ClonedLatch->clearSuccessors();

    for (auto Pred : ClonedHeader->getPredecessors())
      Pred->removeSuccessor(ClonedHeader);

    VPValue *CondBit = CurrentLatch->getCondBit();
    assert(CondBit && "The loop latch is expected to have CondBit");

    VPBlockUtils::moveSuccessors(CurrentLatch, ClonedLatch);
    CurrentLatch->appendSuccessor(ClonedHeader);

    // Move forward latch's condition.
    ClonedLatch->setCondBit(CondBit);
    CurrentLatch->setCondBit(nullptr);

    CurrentLatch = ClonedLatch;
  }

  // TODO: Implement as part of some earlier traversal to save compile time.
  // Add all cloned blocks into the loop.
  for (auto &ValueMap : Clones)
    for (auto Pair : ValueMap)
      if (isa<VPBasicBlock>(Pair.first))
        VPL->addBasicBlockToLoop(cast<VPBasicBlock>(Pair.second), *VPLI);

  // Replace uses of live-outs with the last unrolling part clone of them.
  VPCloneUtils::Value2ValueMapTy &ValueMap = Clones[UF - 2];
  for (auto It : LiveOutUsers)
    It.first->replaceUsesOfWith(It.second, ValueMap[It.second]);

  CurrentLatch->setCondBit(ValueMap[CurrentLatch->getCondBit()]);

  VPLAN_DUMP(VPlanPrintUnroll, "loop unrolling", Plan);
}
