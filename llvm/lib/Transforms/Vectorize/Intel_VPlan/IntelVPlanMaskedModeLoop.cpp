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
        &RecurrentValsAndLiveOuts) {

  for (VPBasicBlock *VPBB : TopVPLoop->getBlocks()) {
    for (VPInstruction &VPInst : *VPBB) {
      VPUser::user_iterator VPPhiUserIt =
          getLoopHeaderVPPHIUser(VPInst.users(), TopVPLoop);
      if (VPPhiUserIt != VPInst.users().end())
        RecurrentValsAndLiveOuts.push_back(
            std::make_pair(cast<VPPHINode>(*VPPhiUserIt), &VPInst));
      else if (TopVPLoop->isLiveOut(&VPInst))
        RecurrentValsAndLiveOuts.push_back(std::make_pair(nullptr, &VPInst));
    }
  }
}

std::shared_ptr<VPlanMasked> MaskedModeLoopCreator::createMaskedModeLoop(void) {
  std::shared_ptr<VPlanMasked> MaskedVPlan =
      OrigVPlan->cloneMasked(VPAF, VPlanVector::UpdateDA::DoNotUpdateDA);

  // Collect information before applying masked mode transformation.
  VPLoop *TopVPLoop = *MaskedVPlan->getVPLoopInfo()->begin();
  assert(std::distance(MaskedVPlan->getVPLoopInfo()->begin(),
                       MaskedVPlan->getVPLoopInfo()->end()) == 1 &&
         "Expected single outermost loop!");
  VPInstruction *VPIndIncrement = getInductionVariable(TopVPLoop);
  VPBasicBlock *Header = TopVPLoop->getHeader();
  // Find the phi node of the header that its incoming value is the induction
  // variable.
  VPUser::user_iterator VPPhiUserIt =
      getLoopHeaderVPPHIUser(VPIndIncrement->users(), TopVPLoop);
  assert(VPPhiUserIt != VPIndIncrement->users().end() &&
         "A phi user is expected.");
  VPPHINode *VPIndInstVPPhi = cast<VPPHINode>(*VPPhiUserIt);

  // Find the upper bound of the current loop. For now, the masked loop will
  // have the same upper bound as the scalar loop.
  VPBasicBlock *Latch = TopVPLoop->getLoopLatch();
  assert(Latch && "Latch is expected to exist.");
  VPBranchInst *Term = Latch->getTerminator();
  VPInstruction *CondBit = cast<VPInstruction>(Latch->getCondBit());
  assert(*CondBit->users().begin() == Term && "CondBit has only one user.");
  assert(*VPIndIncrement->users().begin() == VPIndInstVPPhi &&
         *std::next(VPIndIncrement->users().begin()) == CondBit &&
         "VPIndIncrement has only two users: CondBit and VPIndInstVPPhi");

  // Get the values which are live-outs of the loop and live-outs through
  // backedge.
  SmallVector<std::pair<VPPHINode *, VPInstruction *>, 2>
      RecurrentValsAndLiveOuts;
  collectRecurrentValuesAndLiveOuts(TopVPLoop, RecurrentValsAndLiveOuts);

  // The induction increment and the exit comparison might be in a different
  // basic block than the latch. In this case, we have to move them to the
  // bottom of the latch.
  if (Term->getPrevNode() != CondBit)
    CondBit->moveBefore(Term);
  if (CondBit->getPrevNode() != VPIndIncrement)
    VPIndIncrement->moveBefore(CondBit);

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
  VPInstruction *NewHeaderCond = CondBit->clone();
  NewHeaderCond->replaceUsesOfWith(VPIndIncrement, VPIndInstVPPhi, true);
  Header->appendInstruction(NewHeaderCond);
  Header->setTerminator(HeaderSucc, NewLoopLatch, NewHeaderCond);

  // Create phi nodes in new latch for the values that are live-outs of the
  // loop and live-outs through backedge.
  VPBuilder VPBldr;
  VPBldr.setInsertPoint(NewLoopLatch, NewLoopLatch->begin());
  for (auto &Pair : RecurrentValsAndLiveOuts) {
    VPInstruction *LiveOutVal = Pair.second;
    VPPHINode *LiveOutVPPhi =
        VPBldr.createPhiInstruction(LiveOutVal->getType());
    auto InsnNotInLoop = [TopVPLoop](VPUser *U) {
      if (VPInstruction *I = dyn_cast<VPInstruction>(U))
        return !TopVPLoop->contains(I);
      if (VPPHINode *VPPHI = dyn_cast<VPPHINode>(U))
        return TopVPLoop->contains(VPPHI) &&
               VPPHI->getParent() == TopVPLoop->getHeader();
      return false;
    };
    LiveOutVal->replaceUsesWithIf(LiveOutVPPhi, InsnNotInLoop);
    LiveOutVPPhi->addIncoming(LiveOutVal, Latch);
    VPValue *IncomingValFromHeader =
        Pair.first ? cast<VPValue>(Pair.first)
                   : cast<VPValue>(MaskedVPlan->getVPConstant(
                         UndefValue::get(LiveOutVPPhi->getType())));
    LiveOutVPPhi->addIncoming(IncomingValFromHeader, Header);
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
