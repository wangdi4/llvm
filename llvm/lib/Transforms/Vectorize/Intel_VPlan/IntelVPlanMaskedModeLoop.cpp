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
#include "IntelVPlan.h"
#include "IntelVPlanBuilder.h"
#include "IntelVPlanDivergenceAnalysis.h"
#include "IntelVPlanDominatorTree.h"
#include "IntelVPlanExternals.h"
#include "IntelVPlanLoopInfo.h"
#include "IntelVPlanValue.h"

#define DEBUG_TYPE "VPlanMaskedModeLoop"

using namespace llvm;
using namespace llvm::vpo;

cl::opt<bool> EnableMaskedVariant("vplan-enable-masked-variant",
                                  cl::init(true), cl::Hidden,
                                  cl::desc("Enable masked variant"));

cl::opt<bool> EnableMaskedVariantHIR(
    "vplan-enable-masked-variant-hir", cl::init(true), cl::Hidden,
    cl::desc("Enable masked variant for HIR vectorizer"));

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
  assert(VPIndIncrement && "Expected a non-null induction-variable.");
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

  // Create new bottom test condition, which then will be cloned to use as loop
  // body guarding test. Due to that re-use, we can't rely on the original
  // condition and create a new one with strictly defined single order of
  // successors, predicate and order of operands of the compare (see
  // below). If the latch condition has an exact ub we use ULT predicate
  // otherwise we use ULE predicate to have ub+1 iterations
  VPCmpInst *NewBottomTest = new VPCmpInst(
      VPIndIncrement, OrigTC,
      TopVPLoop->exactUB() ? CmpInst::ICMP_ULT : CmpInst::ICMP_ULE);
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
  assert(VectorTC->getNumUsers() == 0 && "Expected no users of VectorTC");
  VectorTC->getParent()->eraseInstruction(VectorTC);
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
  // Helper lambda to search the recurrence/liveout tracking data structure and
  // return recurrent PHI for given VPValue, if found. Returns nullptr
  // otherwise.
  auto ValHasRecurrencePhi =
      [RecurrentValsAndLiveOuts](VPValue *V) -> VPPHINode * {
    for (auto &Pair : RecurrentValsAndLiveOuts) {
      if (Pair.second == V && Pair.first != nullptr)
        return Pair.first;
    }

    return nullptr;
  };
  // Helper lambda to check if given VPValue has a HIR-copy user which is
  // in-turn used in recurrence. The recurrent PHI is returned if found, nullptr
  // otherwise.
  auto ValHasCopyUsedInRecurrence =
      [&ValHasRecurrencePhi](VPValue *V) -> VPPHINode * {
    VPPHINode *RecurPhi = nullptr;
    for (auto *U : V->users()) {
      if (auto *CopyU = dyn_cast<VPHIRCopyInst>(U)) {
        if (auto *Phi = ValHasRecurrencePhi(CopyU)) {
          RecurPhi = Phi;
          break;
        }
      }
    }

    return RecurPhi;
  };

  // Map to keep live-in values for encountered VPPrivateFinal-s.
  // We fill it in the loop below and use during transforming
  // those VPPrivateFinal into VPPrivateFinalMasked.
  DenseMap<VPInstruction*, VPValue*> PrivFinalLiveInMap;

  for (auto &Pair : RecurrentValsAndLiveOuts) {
    VPInstruction *LiveOutVal = Pair.second;
    VPPHINode *LiveOutVPPhi =
        VPBldr.createPhiInstruction(LiveOutVal->getType());
    // New loop latch has two predecessors: loop header and the first part of
    // the old loop latch.
    auto NewLoopLatchPred =
        llvm::find_if(NewLoopLatch->getPredecessors(),
                      [Header](VPBasicBlock *Pred) { return Pred != Header; });
    assert(NewLoopLatchPred != NewLoopLatch->getPredecessors().end() &&
           "Basic block is not a predecessor of the new loop latch.");
    LiveOutVPPhi->addIncoming(LiveOutVal, *NewLoopLatchPred);
    VPValue *IncomingValFromHeader = Pair.first;
    if (!IncomingValFromHeader) {
      // Liveout w/o header phi. That can be either -
      // 1. a value which has a HIR-copy used in recurrence (or)
      // 2. an unconditional last private
      //
      // First take the out-of-the-loop use of that liveoyt.
      auto LOUserIt = llvm::find_if(LiveOutVal->users(), InsnNotInLoop);
      assert(LOUserIt != LiveOutVal->user_end() &&
             "Expected not-in-the-loop user");
      auto *UseInst = cast<VPInstruction>(*LOUserIt);

      if (UseInst->getOpcode() != VPInstruction::PrivateFinalUncond) {
        // Liveout value is expected to have a copy used in recurrence. We use
        // that recurrent PHI as incoming value from header.
        auto *RecurPhiFromCopy = ValHasCopyUsedInRecurrence(LiveOutVal);
        assert(RecurPhiFromCopy && "Liveout value expected to have a copy user "
                                   "involved in recurrence.");
        IncomingValFromHeader = RecurPhiFromCopy;
      } else {
        // In such cases we should take the loop incoming value as an
        // operand of the latch phi. Even the unconditional last private does
        // not have any incoming value, if this masked mode loop is a remainder
        // and there are no iterations executed in that remainder we need to
        // pass through the value that comes from the main loop.

        // We expect out-of-loop user of liveout to be PrivateFinal instruction
        // only.
        assert(UseInst->getOpcode() == VPInstruction::PrivateFinalUncond &&
               "Expected liveout private");

        // Then find the external use to get incoming value.
        auto Iter = llvm::find_if(UseInst->users(), [](VPUser *U) {
          return isa<VPExternalUse>(U) || isa<VPLiveOutValue>(U);
        });
        assert(Iter != UseInst->user_end() && "Expected non-null external use");

        int MergeId;
        if (auto *ExternalUse = dyn_cast<VPExternalUse>(*Iter))
          MergeId = ExternalUse->getMergeId();
        else
          MergeId = cast<VPLiveOutValue>(*Iter)->getMergeId();

        IncomingValFromHeader =
            const_cast<VPLiveInValue *>(MaskedVPlan->getLiveInValue(MergeId));
        // Save for future replacement
        LLVM_DEBUG(dbgs() << "IncomingVal for: ";
                   UseInst->printAsOperand(dbgs()); dbgs() << " is ";
                   IncomingValFromHeader->printAsOperand(dbgs()););
        PrivFinalLiveInMap[UseInst] = IncomingValFromHeader;
      }
    } else {
      // When the VPPHINode in the header exists.
      VPPHINode *HeaderPhi = Pair.first;
      HeaderPhi->setIncomingValue(HeaderPhi->getBlockIndex(NewLoopLatch),
                                  LiveOutVPPhi);
    }
    LiveOutVPPhi->addIncoming(IncomingValFromHeader, Header);
    LiveOutVal->replaceUsesWithIf(LiveOutVPPhi, InsnNotInLoop);
  }

  // Replace all POD unconditional last private final calculations with the
  // masked ones.
  auto *ExitBB = TopVPLoop->getExitBlock();
  assert(ExitBB && "Expected non-null exit block.");
  // Get instructions to process in advance as we will add/remove instructions.
  SmallVector<VPInstruction *, 4> ToProcess(map_range(
      make_filter_range(*ExitBB,
                        [](VPInstruction &I) {
                          return I.getOpcode() ==
                                     VPInstruction::PrivateFinalUncond ||
                                 I.getOpcode() ==
                                     VPInstruction::PrivateFinalUncondMem;
                        }),
      [](VPInstruction &I) { return &I; }));

  // Replace PrivateFinalUncond[Mem] with PrivateFinalMasked[Mem]
  for (VPInstruction *I : ToProcess) {
    VPBldr.setInsertPoint(&*I);
    VPInstruction *NewPriv;
    if (I->getOpcode() == VPInstruction::PrivateFinalUncond) {
      VPValue *Incoming = PrivFinalLiveInMap[I];
      assert(Incoming && "Expected non-null incoming value");
      NewPriv =
          VPBldr.createNaryOp(VPInstruction::PrivateFinalMasked, I->getType(),
                              {I->getOperand(0), NewHeaderCond, Incoming});
    } else {
      NewPriv =
          VPBldr.createNaryOp(VPInstruction::PrivateFinalMaskedMem,
                              I->getType(), {I->getOperand(0), NewHeaderCond});
    }
    NewPriv->setName(I->getName());
    NewPriv->setDebugLocation(I->getDebugLocation());

    I->replaceAllUsesWith(NewPriv);
    ExitBB->eraseInstruction(I);
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
