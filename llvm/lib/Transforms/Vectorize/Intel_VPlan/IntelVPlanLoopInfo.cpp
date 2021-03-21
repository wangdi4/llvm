//===-- IntelVPlanLoopInfo.cpp ------------------------------------*- C++ -*-===//
//
//   Copyright (C) 2015-2019 Intel Corporation. All rights reserved.
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
#include "IntelVPlan.h"
#include "IntelVPlanDominatorTree.h"
#include "IntelVPlanLoopIterator.h"
#include "IntelVPlanValue.h"
#include "llvm/Support/CommandLine.h"

using namespace llvm;
using namespace llvm::vpo;

#define DEBUG_TYPE "vplan-loop-info"

static cl::opt<unsigned>
    DefaultTripCount("vplan-default-trip-count", cl::init(300), cl::Hidden,
                     cl::desc("Default estimate for the loop trip count, if "
                              "can't be determined through other ways"));

void TripCountInfo::calculateEstimatedTripCount() {
  if (TripCount)
    return; // Already calculated.

  // No known trip count or known average tripcount. Try to estimate to the best
  // of our abilities.

  bool KnownMax = MaxTripCount != TripCountInfo::UnknownMaxTripCount;
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

// The following code snippet illustrates what is detected by the
// function.
// ...
// %ind.step = induction-init-step{add} i64 1
// ...
// %ind.phi = phi i64 [0, %preheader], [%add, %loop_latch]
// ...
// %add = add i64 %ind.phi, %ind.step
// %cmp = icmp sle i64 %add, %loop.invariant
//
bool VPLoop::hasNormalizedInduction() const {
  VPBasicBlock *Latch = getLoopLatch();
  if (!Latch)
    return false;
  VPBranchInst *Br = Latch->getTerminator();
  if (!Br || Br->getCondition() == nullptr)
    return false;
  VPCmpInst *Cond = dyn_cast<VPCmpInst>(Br->getCondition());
  if (!Cond)
    return false;
  if (Cond->getNumUsers() != 1)
    return false;
  VPInstruction *AddI = nullptr;
  auto getAddInstr = [&AddI, Cond, this](int NumOp) -> bool {
    // Check that the NumOp-th operand of Cond is an "add" instruction inside
    // the loop with one of operands equal to InductionInitStep(1) and another
    // operand of Cond is a loop invariant. The add instriuction is stored to
    // AddI for further checks.
    // In the example above it's the %add instruction.
    AddI = dyn_cast<VPInstruction>(Cond->getOperand(NumOp));
    if (AddI && AddI->getOpcode() == Instruction::Add && contains(AddI)) {
      VPValue *SecOp = Cond->getOperand(NumOp ^ 1);
      // Check upper bound.
      if (!isDefOutside(SecOp) && !isa<VPConstant>(SecOp))
        return false;
      // Check step.
      if (auto StepInit = dyn_cast<VPInductionInitStep>(AddI->getOperand(0)))
        SecOp = StepInit->getOperand(0);
      else {
        StepInit = cast<VPInductionInitStep>(AddI->getOperand(1));
        SecOp = StepInit->getOperand(0);
      }
      if (VPConstantInt *Step = dyn_cast<VPConstantInt>(SecOp))
        return Step->getValue() == 1;
    }
    return false;
  };
  if (getAddInstr(0) || getAddInstr(1)) {
    VPBasicBlock *Preheader = getLoopPreheader();
    VPBasicBlock *Header = getHeader();
    // Check that increment is used only in condition and in phi.
    for (auto *U : AddI->users()) {
      if (U == Cond)
        continue;
      VPPHINode *PN = dyn_cast<VPPHINode>(U);
      if (!PN)
        return false;
      if (PN->getParent() != Header)
        return false;
      // Header phi, check start value for 0.
      VPValue *Init = PN->getIncomingValue(Preheader);
      if (auto IndInit = dyn_cast<VPInductionInit>(Init)) {
        Init = IndInit->getStartValueOperand();
        if (isa<VPConstantInt>(Init) &&
            cast<VPConstantInt>(Init)->getValue() == 0)
          continue;
      }
      // The starting value is not induction-init(0).
      return false;
    }
    // All checks succeeded.
    return true;
  }
  return false;
}

std::pair<VPValue *, VPInstruction *> VPLoop::getLoopUpperBound() const {
  if (!hasNormalizedInduction())
    return std::make_pair<VPValue *, VPInstruction *>(nullptr, nullptr);
  VPCmpInst *Cond =
      cast<VPCmpInst>(getLoopLatch()->getTerminator()->getCondition());
  if (VPInstruction *Add = dyn_cast<VPInstruction>(Cond->getOperand(0)))
    if (Add->getOpcode() == Instruction::Add && contains(Add))
      return std::make_pair(Cond->getOperand(1), Cond);
  auto Add = dyn_cast<VPInstruction>(Cond->getOperand(1));
  assert((Add != nullptr && Add->getOpcode() == Instruction::Add &&
          contains(Add)) &&
         "Unexpected operand");
  (void)Add;
  return std::make_pair(Cond->getOperand(0), Cond);
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

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP) // INTEL
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
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP) // INTEL

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

void VPLoopInfo::analyze(const VPDominatorTree &DomTree) {
  assert(begin() == end() && "VPLoopInfo has already been run!");
  Base::analyze(DomTree);
}
