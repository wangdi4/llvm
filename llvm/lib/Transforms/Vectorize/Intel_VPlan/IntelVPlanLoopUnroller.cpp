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

void VPlanLoopUnroller::run() {
  assert(UF > 1 && "Can't unroll with unroll factor less than 2");

  VPLoopInfo *VPLI = Plan.getVPLoopInfo();
  assert(std::distance(VPLI->begin(), VPLI->end()) == 1 &&
         "Expected single outermost loop!");

  VPLoop *VPL = *VPLI->begin();
  assert(VPL->getSubLoops().empty() &&
         "Unrolling of loops with subloops is not supported");

  // Collect loop live-out users to process them after the unroll.
  DenseMap<VPUser *, VPInstruction *> LiveOutUsers;
  for (VPBlockBase *Block : VPL->blocks())
    if (VPBasicBlock *BasicBlock = dyn_cast<VPBasicBlock>(Block))
      for (VPInstruction &Inst : BasicBlock->getInstructions())
        for (VPUser *User : Inst.users())
          if (isa<VPExternalUse>(User)) {
            assert(LiveOutUsers.find(User) == LiveOutUsers.end() &&
                   "Only one instruction for a live-out user is supported");
            LiveOutUsers[User] = &Inst;
          }
          else if (auto UseInst = dyn_cast<VPInstruction>(User))
            if (!VPL->contains(UseInst))
            {
              assert(LiveOutUsers.find(User) == LiveOutUsers.end() &&
                     "Only one instruction for a live-out user is supported");
              LiveOutUsers[User] = &Inst;
            }

  VPBlockBase *Header = VPL->getHeader();
  assert(Header && "Expected single header block");
  VPBlockBase *Latch = VPL->getLoopLatch();
  assert(Latch && "Expected single latch block");
  VPBlockBase *CurrentLatch = Latch;

  struct CloneData {
    VPCloneUtils::Block2BlockMapTy BlockMap;
    VPCloneUtils::Value2ValueMapTy ValueMap;
  };

  SmallVector<CloneData, 8> Clones(UF - 1);
  for (unsigned Part = 0; Part < UF - 1; Part++) {
    CloneData &Data = Clones[Part];
    VPCloneUtils::cloneBlocksRange(Header, Latch, Data.BlockMap, Data.ValueMap,
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
    CloneData &Data = Clones[Part];
    VPCloneUtils::Block2BlockMapTy &BlockMap = Data.BlockMap;
    VPCloneUtils::Value2ValueMapTy &ValueMap = Data.ValueMap;

    VPCloneUtils::Value2ValueMapTy ReverseMap;
    for (auto It : ValueMap)
      ReverseMap[It.second] = It.first;
    assert(ReverseMap.size() == ValueMap.size() &&
           "Expecting unique values only in ValueMap");

    VPBasicBlock *ClonedHeader = dyn_cast<VPBasicBlock>(BlockMap[Header]);
    assert(ClonedHeader && "Header expected to be successfully cloned");

    VPBasicBlock *ClonedLatch = dyn_cast<VPBasicBlock>(BlockMap[Latch]);
    assert(ClonedLatch && "Latch expected to be successfully cloned");

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
      OrigInst->setIncomingBlock(
          OrigInst->getBlockIndex(dyn_cast<VPBasicBlock>(CurrentLatch)),
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
      ClonedHeader->getInstructions().remove(*It);

    // Remap operands.
    VPValueMapper Mapper(BlockMap, ValueMap);
    for (VPBlockBase *Block : VPL->blocks()) {
      auto ClonedBlock = BlockMap[Block];
      assert(ClonedBlock && "All blocks expected to be successfully cloned");

      if (VPBasicBlock *BasicBlock = dyn_cast<VPBasicBlock>(ClonedBlock))
        for (auto &Inst : BasicBlock->getInstructions())
          Mapper.remapInstruction(&Inst);

      // Fix cond bit.
      if (VPValue *CondBit = Block->getCondBit())
        ClonedBlock->setCondBit(ValueMap[CondBit]);

      // Fix cond predicate.
      if (VPValue *Predicate = Block->getPredicate())
        ClonedBlock->setPredicate(ValueMap[Predicate]);

      // Fix parent.
      ClonedBlock->setParent(Header->getParent());
    }

    // Insert cloned blocks into the loop.
    for (auto Succ : ClonedLatch->getSuccessors())
      VPBlockUtils::disconnectBlocks(ClonedLatch, Succ);

    for (auto Pred : ClonedHeader->getPredecessors())
      VPBlockUtils::disconnectBlocks(Pred, ClonedHeader);

    VPBlockUtils::moveSuccessors(CurrentLatch, ClonedLatch);
    VPBlockUtils::connectBlocks(CurrentLatch, ClonedHeader);

    // Move forward latch's condition.
    VPValue *CondBit = CurrentLatch->getCondBit();
    assert(CondBit && "The loop latch is expected to have CondBit");
    ClonedLatch->setCondBit(CondBit);
    CurrentLatch->setCondBit(nullptr);

    CurrentLatch = ClonedLatch;
  }

  // Add all cloned blocks into the loop.
  for (auto Data : Clones)
    for (auto Pair : Data.BlockMap)
      VPL->addBasicBlockToLoop(Pair.second, *VPLI);

  // Replace uses of live-outs with the last unrolling part clone of them.
  VPCloneUtils::Value2ValueMapTy &ValueMap = Clones[UF - 2].ValueMap;
  for (auto It : LiveOutUsers)
    It.first->replaceUsesOfWith(It.second, ValueMap[It.second]);

  CurrentLatch->setCondBit(ValueMap[CurrentLatch->getCondBit()]);

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  if (VPlanPrintUnroll) {
    outs() << "After loop unrolling\n";
    Plan.dump(outs(), true);
  }
#endif
}
