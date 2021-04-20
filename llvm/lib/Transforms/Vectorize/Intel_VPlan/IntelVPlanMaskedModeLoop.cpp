//===-- IntelVPlanMaskedModeLoop.cpp --------------------------------------===//
//
//   Copyright (C) 2020 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//

#include "IntelVPlanMaskedModeLoop.h"
#include "IntelVPSOAAnalysis.h"
#include "IntelVPlan.h"
#include "IntelVPlanBuilder.h"
#include "IntelVPlanDivergenceAnalysis.h"
#include "IntelVPlanDominatorTree.h"
#include "IntelVPlanLoopInfo.h"
#include "IntelVPlanValue.h"

#define DEBUG_TYPE "VPlanMaskedModeLoop"

using namespace llvm;
using namespace llvm::vpo;

cl::opt<bool> EnableMaskedVariant("vplan-enable-masked-variant",
                                  cl::init(false), cl::Hidden,
                                  cl::desc("Enable masked variant"));

static LoopVPlanDumpControl
    MaskedVariantDumpControl("create-masked-vplan", "emitting masked variant");

VPUser::user_iterator getLoopHeaderVPPHIUser(const VPUser::user_range &Users,
                                             VPLoop *TopVPLoop) {
  return llvm::find_if(Users, [TopVPLoop](auto &User) {
    return (isa<VPPHINode>(User) &&
            cast<VPPHINode>(User)->getParent() == TopVPLoop->getHeader());
  });
}

// Collect loop's recurrent values and live-outs. For each of these values, a
// phi node is emitted in the new loop latch (new_latch). For recurrent
// values, we also need to keep the phi node of the Header that has the
// recurrent value as an imcoming value.
static void collectRecurrentValuesAndLiveOuts(
    VPLoop *TopVPLoop,
    SmallVectorImpl<
        std::pair<VPPHINode * /*header's phi node*/,
                  VPInstruction * /* recurrent value in header's phi node*/>>
        &RecurrentValsAndLiveOuts,
    VPInstruction *MainInduction) {

  for (VPBasicBlock *VPBB : TopVPLoop->getBlocks()) {
    for (VPInstruction &VPInst : *VPBB) {
      VPUser::user_iterator VPPhiUserIt =
          getLoopHeaderVPPHIUser(VPInst.users(), TopVPLoop);
      if (&VPInst != MainInduction && VPPhiUserIt != VPInst.users().end())
        RecurrentValsAndLiveOuts.push_back(
            std::make_pair(cast<VPPHINode>(*VPPhiUserIt), &VPInst));
      else if (TopVPLoop->isLiveOut(&VPInst))
        RecurrentValsAndLiveOuts.push_back(std::make_pair(nullptr, &VPInst));
    }
  }
}

std::shared_ptr<VPlanMasked> MaskedModeLoopCreator::createMaskedModeLoop(void) {
  std::shared_ptr<VPlanMasked> MaskedVPlan(
      OrigVPlan->cloneMasked(VPAF, VPlanVector::UpdateDA::DoNotUpdateDA));

  // Collect information before applying masked mode transformation.
  VPLoop *TopVPLoop = MaskedVPlan->getMainLoop(true);
  VPInstruction *VPIndIncrement = getInductionVariable(TopVPLoop);
  VPBasicBlock *Header = TopVPLoop->getHeader();
  // Find the phi node of the header that its incoming value is the induction
  // variable.
  VPUser::user_iterator VPPhiUserIt =
      getLoopHeaderVPPHIUser(VPIndIncrement->users(), TopVPLoop);
  assert(VPPhiUserIt != VPIndIncrement->users().end() &&
         "A phi user is expected.");
  VPPHINode *VPIndInstVPPhi = cast<VPPHINode>(*VPPhiUserIt);
  // Get the values which are live-outs of the loop and live-outs through
  // backedge.
  SmallVector<std::pair<VPPHINode *, VPInstruction *>, 2>
      RecurrentValsAndLiveOuts;
  collectRecurrentValuesAndLiveOuts(TopVPLoop, RecurrentValsAndLiveOuts,
                                    VPIndIncrement);

  // Create a new condition bit in the latch.
  VPBasicBlock *Latch = TopVPLoop->getLoopLatch();
  assert(Latch && "Latch is expected to exist.");
  VPBranchInst *Term = Latch->getTerminator();
  VPInstruction *OrigCondBit = cast<VPInstruction>(Latch->getCondBit());
  assert(*OrigCondBit->users().begin() == Term &&
         OrigCondBit->getNumUsers() == 1 && "CondBit has only one user.");
  assert(llvm::all_of(VPIndIncrement->users(),
                      [VPIndInstVPPhi, OrigCondBit](auto U) {
                        return U == VPIndInstVPPhi || U == OrigCondBit;
                      }) &&
         "VPIndIncrement has only two users: CondBit and VPIndInstVPPhi");
  VPVectorTripCountCalculation *VectorTC;
  if (auto *VTC =
          dyn_cast<VPVectorTripCountCalculation>(OrigCondBit->getOperand(1)))
    VectorTC = VTC;
  else
    VectorTC = cast<VPVectorTripCountCalculation>(OrigCondBit->getOperand(0));
  auto *OrigTC = VectorTC->getOperand(0);
  VPCmpInst *NewBottomTest =
      new VPCmpInst(VPIndIncrement, OrigTC, CmpInst::ICMP_ULT);
  Latch->addInstruction(NewBottomTest, Term);
  // The new condition bit is divergent. However, in VPlan, it is required
  // that the bottom test is uniform. For this reason, we need to create a new
  // bottom test as it is shown below:
  // NewBottomTest = cmp IV, OrigTC
  // AllZeroChk = call allzero(NewBottomTest)
  // br AllZeroChk, LoopExitBlock, Header
  VPBuilder VPBldr;
  VPBldr.setInsertPoint(Term);
  auto *AllZeroChk = VPBldr.createAllZeroCheck(NewBottomTest);
  Latch->setTerminator(TopVPLoop->getExitBlock(), Header, AllZeroChk);
  // The induction increment might be in a different basic block than the latch.
  // In this case, we have to move the induction increment at the bottom of the
  // latch.
  if (OrigCondBit->getPrevNode() != VPIndIncrement ||
      VPIndIncrement->getParent() != Latch)
    VPIndIncrement->moveBefore(NewBottomTest);
  // Remove the original CondBit.
  VPBasicBlock *OrigCondBitBB = OrigCondBit->getParent();
  OrigCondBitBB->eraseInstruction(OrigCondBit);
  // Split latch before induction increment.
  VPBasicBlock *NewLoopLatch = VPBlockUtils::splitBlock(
      Latch, VPIndIncrement->getIterator(), MaskedVPlan->getVPLoopInfo());
  NewLoopLatch->setName("new_latch");

  // Find first non phi instruction in Header.
  auto NonVPPhiIt = find_if(
      *Header, [](const VPInstruction &Inst) { return !isa<VPPHINode>(Inst); });
  // Split header before first non phi instruction.
  assert(Header->getTerminator() && "BB should have a terminator");
  VPBasicBlock *HeaderSucc = VPBlockUtils::splitBlock(
      Header, NonVPPhiIt, MaskedVPlan->getVPLoopInfo());
  VPInstruction *NewHeaderCond = NewBottomTest->clone();
  NewHeaderCond->replaceUsesOfWith(VPIndIncrement, VPIndInstVPPhi, true);
  Header->appendInstruction(NewHeaderCond);
  Header->setTerminator(HeaderSucc, NewLoopLatch, NewHeaderCond);

  // Create phi nodes in new latch for the values that are live-outs of the
  // loop and live-outs through backedge.
  VPBldr.setInsertPoint(NewLoopLatch, NewLoopLatch->begin());
  auto InsnNotInLoop = [TopVPLoop](VPUser *U) {
    if (VPInstruction *I = dyn_cast<VPInstruction>(U))
      return !TopVPLoop->contains(I);
    return false;
  };
  for (auto &Pair : RecurrentValsAndLiveOuts) {
    VPInstruction *LiveOutVal = Pair.second;
    VPPHINode *LiveOutVPPhi =
        VPBldr.createPhiInstruction(LiveOutVal->getType());
    LiveOutVal->replaceUsesWithIf(LiveOutVPPhi, InsnNotInLoop);
    // New loop latch has two predecessors: loop header and the first part of
    // the old loop latch.
    auto NewLoopLatchPred =
        llvm::find_if(NewLoopLatch->getPredecessors(),
                      [Header](VPBasicBlock *Pred) { return Pred != Header; });
    assert(NewLoopLatchPred != NewLoopLatch->getPredecessors().end() &&
           "Basic block is not a predecessor of the new loop latch.");
    LiveOutVPPhi->addIncoming(LiveOutVal, *NewLoopLatchPred);
    VPValue *IncomingValFromHeader =
        Pair.first ? cast<VPValue>(Pair.first)
                   : cast<VPValue>(MaskedVPlan->getVPConstant(
                         UndefValue::get(LiveOutVPPhi->getType())));
    LiveOutVPPhi->addIncoming(IncomingValFromHeader, Header);
    if (Pair.first)
      Pair.first->setIncomingValue(Pair.first->getBlockIndex(NewLoopLatch),
                                   LiveOutVPPhi);
  }

  MaskedVPlan->getDT()->recalculate(*MaskedVPlan.get());
  MaskedVPlan->getPDT()->recalculate(*MaskedVPlan.get());
  auto MaskedVPlanDA = std::make_unique<VPlanDivergenceAnalysis>();
  MaskedVPlan->setVPlanDA(std::move(MaskedVPlanDA));
  MaskedVPlan->computeDA();

  VPLAN_DUMP(MaskedVariantDumpControl, MaskedVPlan.get());

  return MaskedVPlan;
}

// TODO: Update it with the heuristic that decides whether masked mode is
// needed.
bool MaskedModeLoopCreator::mayUseMaskedMode() { return EnableMaskedVariant; }

VPInstruction *MaskedModeLoopCreator::getInductionVariable(VPLoop *TopVPLoop) {
  VPInstruction *CondBit =
      dyn_cast<VPInstruction>(TopVPLoop->getLoopLatch()->getCondBit());
  for (unsigned Idx = 0; Idx < CondBit->getNumOperands(); Idx++) {
    VPInstruction *Op = dyn_cast<VPInstruction>(CondBit->getOperand(Idx));
    if (Op && Op->getOpcode() == Instruction::Add &&
        (isa<VPInductionInitStep>(Op->getOperand(1)) ||
         isa<VPInductionInitStep>(Op->getOperand(0))) &&
        llvm::find_if(Op->users(), [TopVPLoop](auto &User) {
          return (isa<VPPHINode>(User) &&
                  cast<VPPHINode>(User)->getParent() == TopVPLoop->getHeader());
        }) != Op->users().end())
      return Op;
  }
  return nullptr;
}
