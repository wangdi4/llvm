//===-- IntelVPlanLoopInfo.cpp ------------------------------------*- C++ -*-===//
//
//   Copyright (C) 2015 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation. and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file defines VPLoopInfo analysis and VPLoop class. VPLoopInfo is a
/// specialization of LoopInfoBase for VPBasicBlock. VPLoops is a specialization
/// of LoopBase that is used to hold loop metadata from VPLoopInfo. Further
/// information can be found in VectorizationPlanner.rst.
///
//===----------------------------------------------------------------------===//

#include "IntelVPlanLoopInfo.h"
#include "IntelLoopVectorizationPlanner.h"
#include "IntelVPlan.h"
#include "IntelVPlanDominatorTree.h"
#include "IntelVPlanLoopIterator.h"
#include "IntelVPlanValue.h"
#include "llvm/IR/PatternMatch.h"
#include "llvm/Support/CommandLine.h"

using namespace llvm;
using namespace llvm::vpo;
using namespace llvm::PatternMatch;

#define DEBUG_TYPE "vplan-loop-info"

static cl::opt<unsigned, true>
    DefaultTripCountOpt("vplan-default-trip-count", cl::location(DefaultTripCount),
                        cl::Hidden,
                        cl::desc("Default estimate for the loop trip count, if "
                                 "can't be determined through other ways"));

namespace llvm {
namespace vpo {
unsigned DefaultTripCount = 303;
} // namespace vpo
} // namespace llvm

void TripCountInfo::calculateEstimatedTripCount() {
  if (TripCount)
    return; // Already calculated.

  // No known trip count or known average tripcount. Try to estimate to the best
  // of our abilities.

  bool KnownMax = MaxTripCount < DefaultTripCount;
  bool KnownMin = MinTripCount != 0;

  if (KnownMax && KnownMin)
    TripCount = (static_cast<VPLoop::TripCountTy>(MinTripCount) +
                 static_cast<VPLoop::TripCountTy>(MaxTripCount)) /
                2;
  else if (KnownMax)
    TripCount = MaxTripCount;
  else if (KnownMin)
    TripCount = MinTripCount;
  else
    TripCount = DefaultTripCount;
}

bool VPLoop::isDefOutside(const VPValue *VPVal) const {
  if (isa<VPExternalDef>(VPVal) || isa<VPLiveInValue>(VPVal))
    return true;
  if (auto *VPInst = dyn_cast<VPInstruction>(VPVal))
    return !contains(VPInst);
  return false;
}

bool VPLoop::isLiveOut(const VPInstruction* VPInst) const {
  if (!contains(VPInst))
    return false;
  for (const VPUser *U : VPInst->users()) {
    if (isa<VPExternalUse>(U) || isa<VPLiveOutValue>(U))
      return true;
    if (!contains(cast<VPInstruction>(U)))
      return true;
  }
  return false;
}

VPLoop::LatchCondDescr VPLoop::classifyLatchCond() const {
  VPCmpInst *Cond = getLatchComparison();
  assert(Cond && "expected comparison instruction");

  auto IsInstrWithOpcode = [](const VPValue *V, unsigned Opcode) -> bool {
    if (auto *I = dyn_cast<VPInstruction>(V))
      return I->getOpcode() == Opcode;
    return false;
  };

  auto IsInductionInc = [&IsInstrWithOpcode](const VPValue *V) -> bool {
    auto *I = dyn_cast<VPInstruction>(V);
    return I && I->getOpcode() == Instruction::Add &&
           (IsInstrWithOpcode(I->getOperand(0),
                              VPInstruction::InductionInitStep) ||
            IsInstrWithOpcode(I->getOperand(1),
                              VPInstruction::InductionInitStep));
  };

  if (llvm::find_if(Cond->users(), [&IsInstrWithOpcode](const VPUser *U) {
        return IsInstrWithOpcode(U, VPInstruction::AllZeroCheck);
      }) == Cond->user_end()) {
    // The first case: we have induction increment and compare with the loop
    // upper bound.
    VPInstruction *IAdd = nullptr;
    if (IsInductionInc(Cond->getOperand(0)))
      IAdd = cast<VPInstruction>(Cond->getOperand(0));
    else if (IsInductionInc(Cond->getOperand(1)))
      IAdd = cast<VPInstruction>(Cond->getOperand(1));
    if (IAdd)
      return LatchCondDescr{LckDoLoop, Cond, IAdd};
  } else {
    // Otherwise we can have the backedge variant with AllZero check.
    assert(Cond->getNumUsers() == 1 &&
           "Only one use of compare in AllZeroCheck is expected");
    // In this case the second operand of compare should be a Sub instructions
    // in preheader:
    //   Loop_preheader:
    //     %norm_ub = sub %ub %lower_bound
    //     ...
    //   Loop_latch:
    //     ...
    //     %ind_inc = add %ind_phi %ind_step
    //     %cmp = icmp ult %ind_inc %norm_ub
    //     %all_z = all-zero-check %cmp
    //
    if (IsInductionInc(Cond->getOperand(0)) &&
        (IsInstrWithOpcode(Cond->getOperand(1), Instruction::Sub))) {
      return LatchCondDescr{LckAllZero, Cond,
                            cast<VPInstruction>(Cond->getOperand(0))};
    }
  }
  return LatchCondDescr{LckUnknown, Cond, nullptr};
}

VPValue *VPLoop::getOrigLowerBound() const {
  LatchCondDescr CD = classifyLatchCond();
  switch (CD.Kind) {
  case LckDoLoop: {
    VPBasicBlock *Header = getHeader();
    auto Iter = llvm::find_if(CD.IndIncr->users(), [Header](auto &User) {
      return (isa<VPPHINode>(User) &&
              cast<VPPHINode>(User)->getParent() == Header);
    });
    if (Iter != CD.IndIncr->user_end()) {
      auto PhiNode = cast<VPPHINode>(*Iter);
      int IniNdx = PhiNode->getOperand(0) == CD.IndIncr ? 1 : 0;
      return cast<VPInductionInit>(PhiNode->getOperand(IniNdx))
          ->getStartValueOperand();
    }
  } break;
  case LckAllZero:
    return cast<VPInstruction>(CD.Cond->getOperand(1))->getOperand(1);
  case LckUnknown:
    // Not expected to be called for inner unknown loops.
    break;
  }
  llvm_unreachable("Unexpected loop latch");
  return nullptr;
}

std::pair<VPValue *, VPCmpInst *>
VPLoop::getLoopUpperBound(bool AssumeNormalizedIV, bool GetOrig) const {
  assert((AssumeNormalizedIV || hasNormalizedInduction()) &&
         "must have normilized unduction");

  LatchCondDescr CD = classifyLatchCond();
  switch (CD.Kind) {
  case LckDoLoop:
    if (CD.IndIncr == CD.Cond->getOperand(0))
      return std::make_pair(CD.Cond->getOperand(1), CD.Cond);
    else if (CD.IndIncr == CD.Cond->getOperand(1))
      return std::make_pair(CD.Cond->getOperand(0), CD.Cond);
    llvm_unreachable("Unexpected latch condition descriptor");
    break;
  case LckAllZero: {
    auto RetVal =
        GetOrig ? cast<VPInstruction>(CD.Cond->getOperand(1))->getOperand(0)
                : CD.Cond->getOperand(1);
    return std::make_pair(RetVal, CD.Cond);
  }
  case LckUnknown:
    // Not expected to be called for inner unknown loops.
    break;
  }
  llvm_unreachable("Unexpected loop latch");
  return std::make_pair<VPValue *, VPCmpInst *>(nullptr, nullptr);
}

// We look for the following pattern -
// %cond = phi/blend [ %cmp, BB1 ], [ false, BB2 ] (or)
// %cond = phi/blend [ false, BB2 ], [ %cmp, BB1 ]
//
// and return %cmp if matched. Return nullptr otherwise.
template <typename InstTy>
static VPCmpInst *getCompareInstFromPhiOrBlend(InstTy *I) {
  if (I->getNumIncomingValues() == 2) {
    auto *IV0 = I->getIncomingValue(0u);
    auto *IV1 = I->getIncomingValue(1u);
    if (isa<VPCmpInst>(IV0) && isa<VPConstant>(IV1))
      return cast<VPCmpInst>(IV0);
    else if (isa<VPConstant>(IV0) && isa<VPCmpInst>(IV1))
      return cast<VPCmpInst>(IV1);
  }

  return nullptr;
}

VPCmpInst *VPLoop::getLatchComparison() const {
  // The loop latch comparison is a VPCmpInst instruction which is normally
  // found as: latch block  -> terminator -> condition. Thus if we found it
  // there then we are done.
  // If we have masked mode loop vectorization, the original latch condition
  // may have been replaced with AllZeroCheck instruction. The instruction
  // checks for mask bits and is emitted in place of the original latch
  // condition. The original latch condition in such a case is its operand. In
  // case of early-exit loops latch condition is a phi/blend, so we recur into
  // the phi/blend operands to identify the compare instruction.
  VPValue *CondBit = getLoopLatch()->getTerminator()->getCondition();
  if (auto *CmpInst = dyn_cast<VPCmpInst>(CondBit))
    return CmpInst;
  auto *AllZeroCheck = dyn_cast<VPInstruction>(CondBit);
  if (AllZeroCheck &&
      AllZeroCheck->getOpcode() == VPInstruction::AllZeroCheck) {
    if (auto *CmpInst = dyn_cast<VPCmpInst>(AllZeroCheck->getOperand(0)))
      return CmpInst;
    else {
      auto *AZCheckOp = AllZeroCheck->getOperand(0);
      auto *Not = dyn_cast<VPInstruction>(AZCheckOp);
      if (Not && Not->getOpcode() == VPInstruction::Not)
        AZCheckOp = Not->getOperand(0);
      CondBit = AZCheckOp;
    }
  }
  auto *PhiOrBlend = dyn_cast<VPInstruction>(CondBit);
  // Check if condition is phi/blend. We recur on the incoming values in such
  if (PhiOrBlend && isa<VPPHINode>(PhiOrBlend)) {
    auto *Phi = cast<VPPHINode>(PhiOrBlend);
    if (auto *CmpI = getCompareInstFromPhiOrBlend<VPPHINode>(Phi))
      return CmpI;
  }
  if (PhiOrBlend && isa<VPBlendInst>(PhiOrBlend)) {
    auto *Blend = cast<VPBlendInst>(PhiOrBlend);
    if (auto *CmpI = getCompareInstFromPhiOrBlend<VPBlendInst>(Blend))
      return CmpI;
  }

  // All checks failed.
  return nullptr;
}

VPPHINode *VPLoop::getInductionPHI() const {
  LatchCondDescr CD = classifyLatchCond();
  assert(CD.Kind != LckUnknown && "Unexpected loop latch!");
  assert(CD.IndIncr &&
         "No induction increment for LckDoLoop or LckAllZero loop?");

  // Try to get the loop PHI by matching on the induction increment. In most
  // cases, this is a simple pattern match, but may be more complicated in the
  // presence of unrolling. If the loop is unrolled, we may need to walk back
  // from each cascaded increment until we reach the loop PHI.
  const auto Opcode = CD.IndIncr->getOpcode();
  VPInstruction *Increment = CD.IndIncr;
  VPPHINode *LoopPHI = nullptr;
  while (LoopPHI == nullptr) {
    VPInductionInitStep *Step = nullptr;
    // If the increment is of the form
    //    <increment> = <bin-op> <PHI> <step> or
    //    <increment> = <bin-op> <step> <PHI>
    if (match(Increment, m_c_BinOp(Opcode, m_Bind(LoopPHI), m_Bind(Step)))) {
      // Base case: we've found our loop PHI.
      break;
    }
    // If the increment is of the form
    //    <increment> = <bin-op> <prev-increment> <step> or
    //    <increment> = <bin-op> <step> <prev-increment>
    VPInstruction *PrevIncr = nullptr;
    if (match(Increment, m_c_BinOp(Opcode, m_Bind(PrevIncr), m_Bind(Step)))) {
      // Inductive case: update our current needle and continue searching.
      Increment = PrevIncr;
      continue;
    }
    // We must be operating on IR of unknown form. Bail out.
    llvm_unreachable(
        "latch increment has no PHI or cascaded increment operands?");
  }
  return LoopPHI;
}

VPInductionInit *VPLoop::getInductionInit() const {
  // Return the VPInductionInit which serves as the incoming value from the loop
  // preheader to the induction PHI used in the latch condition.
  return cast<VPInductionInit>(
      getInductionPHI()->getIncomingValue(getLoopPreheader()));
}

MDNode *VPLoop::getLoopID() const {
  // Get the latch block and check the terminator for the metadata.
  VPBranchInst *TI = getLoopLatch()->getTerminator();
  MDNode *LoopID = TI->getLoopIDMetadata();

  if (!LoopID || LoopID->getNumOperands() == 0 ||
      LoopID->getOperand(0) != LoopID)
    return nullptr;
  return LoopID;
}

// Check that 'BB' doesn't have any uses outside of the 'L'
//
// Unlike LLVM's version of this we don't allow unreachable blocks, so DT check
// isn't used here. Also, we have VPUsers that aren't VPInstruction, that
// required some modifications as well.
static bool isBlockInLCSSAForm(const VPLoop &L, const VPBasicBlock &BB) {
  for (const VPInstruction &I : BB) {
    // Tokens can't be used in PHI nodes and live-out tokens prevent loop
    // optimizations, so for the purposes of considered LCSSA form, we
    // can ignore them.
    if (I.getType()->isTokenTy())
      continue;

    for (const VPUser *U : I.users()) {
      const auto *UI = dyn_cast<VPInstruction>(U);
      if (!UI)
        return false;

      const VPBasicBlock *UserBB = UI->getParent();
      if (const auto *P = dyn_cast<VPPHINode>(UI))
        UserBB = P->getIncomingBlock(&I);

      // Check the current block, as a fast-path, before checking whether
      // the use is anywhere in the loop.  Most values are used in the same
      // block they are defined in.
      if (UserBB != &BB && !L.contains(UserBB))
        return false;
    }
  }
  return true;
}

bool VPLoop::isLCSSAForm() const {
  return all_of(this->blocks(), [this](const VPBasicBlock *BB) {
    return isBlockInLCSSAForm(*this, *BB);
  });
}

bool VPLoop::isRecursivelyLCSSAForm(const VPLoopInfo &LI) const {
  return all_of(this->blocks(), [&LI](const VPBasicBlock *BB) {
    return isBlockInLCSSAForm(*LI.getLoopFor(BB), *BB);
  });
}

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
void VPLoop::printRPOT(raw_ostream &OS, const VPLoopInfo *VPLI,
                       unsigned Indent) const {
  ReversePostOrderTraversal<
      const VPLoop *, VPLoopBodyTraits,
      std::set<std::pair<const VPLoop *, const VPBasicBlock *>>>
      RPOT(this);

  auto *Header = getHeader();

  for (std::pair<const VPLoop *, const VPBasicBlock *> Pair : RPOT) {
    const VPBasicBlock *BB = Pair.second;
    SmallString<32> NamePrefix;
    if (BB == Header)
      NamePrefix += "<header>";

    if (isLoopLatch(BB))
      NamePrefix += "<latch>";

    if (isLoopExiting(BB))
      NamePrefix += "<exiting>";

    unsigned BBIndent = Indent;
    if (VPLI)
      BBIndent +=
          (this->getLoopDepth() - VPLI->getLoopFor(BB)->getLoopDepth()) * 2;

    BB->print(OS, BBIndent, NamePrefix);
  }
  OS << "\n";
}
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

void VPLoop::setTripCountInfo(TripCountInfo TCInfo) {
  VPBasicBlock *Latch = getLoopLatch();
  Latch->setTripCountInfo(std::make_unique<TripCountInfo>(TCInfo));
}

TripCountInfo VPLoop::getTripCountInfo() const {
  VPBasicBlock *Latch = getLoopLatch();
  if (TripCountInfo *TCInfoPtr = Latch->getTripCountInfo())
    return *TCInfoPtr;

  LLVM_DEBUG(dbgs() << "No trip count information for VPLoop with header "
                    << getHeader()->getName()
                    << ", using default estimations.\n");
  TripCountInfo DefaultTC;
  DefaultTC.calculateEstimatedTripCount();
  return DefaultTC;
}

VPLoop *VPLoopInfo::AllocateLoop(VPLoop *SrcLoop) {
  VPLoop *TheLoop = Base::AllocateLoop();
  if (SrcLoop) {
    TheLoop->setUnderlyingLoop(SrcLoop->getUnderlyingLoop());
    TheLoop->setDebugLoc(SrcLoop->getDebugLoc());
  }
  return TheLoop;
}

void VPLoopInfo::analyze(const VPDominatorTree &DomTree) {
  assert(begin() == end() && "VPLoopInfo has already been run!");
  Base::analyze(DomTree);
}

void VPLoopInfo::invalidateUnderlyingLoops() {
  for (VPLoop *OuterLoop : *this)
    for (auto *VLP : post_order(OuterLoop))
      VLP->setUnderlyingLoop(nullptr);
}
