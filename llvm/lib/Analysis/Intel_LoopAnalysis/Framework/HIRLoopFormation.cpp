//===------- HIRLoopFormation.cpp - Creates HIR Loops ---------------------===//
//
// Copyright (C) 2015-2020 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// This file implements the HIRLoopFormation pass.
//
//===----------------------------------------------------------------------===//

#include "HIRLoopFormation.h"

#include "llvm/IR/Instructions.h"

#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"

#include "llvm/Analysis/Intel_LoopAnalysis/Framework/HIRRegionIdentification.h"

#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HLNodeUtils.h"

#include "HIRCleanup.h"
#include "HIRCreation.h"

using namespace llvm;
using namespace llvm::loopopt;

#define DEBUG_TYPE "hir-loop-formation"

namespace {
struct LoopCompareLess {
  bool operator()(const HIRLoopFormation::LoopPairTy &LP1,
                  const HIRLoopFormation::LoopPairTy &LP2) {
    return std::less<const Loop *>()(LP1.first, LP2.first);
  }
};

struct LoopCompareEqual {
  bool operator()(const HIRLoopFormation::LoopPairTy &LP1,
                  const HIRLoopFormation::LoopPairTy &LP2) {
    return LP1.first == LP2.first;
  }
};
} // namespace

HLLoop *HIRLoopFormation::findOrInsertHLLoopImpl(const Loop *Lp, HLLoop *HLoop,
                                                 bool Insert) {
  LoopPairTy LoopPair(Lp, HLoop);

  if (Loops.empty()) {
    if (Insert) {
      Loops.push_back(LoopPair);
    }
    return nullptr;
  }

  auto I =
      std::lower_bound(Loops.begin(), Loops.end(), LoopPair, LoopCompareLess());

  if (I != Loops.end() && LoopCompareEqual()(*I, LoopPair)) {
    if (Insert) {
      assert(false && "Multiple insertions not expected!");
    }
    return I->second;
  }

  if (Insert) {
    Loops.insert(I, LoopPair);
  }

  return nullptr;
}

void HIRLoopFormation::insertHLLoop(const Loop *Lp, HLLoop *HLoop) {
  findOrInsertHLLoopImpl(Lp, HLoop, true);
}

HLLoop *HIRLoopFormation::findHLLoop(const Loop *Lp) {
  return findOrInsertHLLoopImpl(Lp, nullptr, false);
}

APInt HIRLoopFormation::getAddRecRefinedSignedMax(
    const SCEVAddRecExpr *AddRec) const {
  // For a case like this-
  //
  // i = 0
  // DO
  //   ...
  //   i += 2 <NSW>
  // END DO
  //
  // ScalarEvolution can refine the max value of IV based on its initial value
  // of 0 and stride of 2 to (SignedMax - 1). If an IV with a positive stride
  // has NSW, its 'least' signed max value is (SignedMax - (Stride - 1)). This
  // function returns this value.
  APInt MaxVal =
      APInt::getSignedMaxValue(AddRec->getType()->getPrimitiveSizeInBits());

  if (!AddRec->isAffine()) {
    return MaxVal;
  }

  auto ConstStride = dyn_cast<SCEVConstant>(AddRec->getStepRecurrence(SE));

  if (!ConstStride) {
    return MaxVal;
  }

  auto &Stride = ConstStride->getAPInt();

  if (Stride.isNegative()) {
    return MaxVal;
  }

  MaxVal -= Stride;
  MaxVal += 1ULL;

  return MaxVal;
}

bool HIRLoopFormation::isNonNegativeNSWIV(const Loop *Lp,
                                          const PHINode *IVPhi) const {
  auto SC = SE.getSCEVForHIR(const_cast<PHINode *>(IVPhi),
                             getOutermostHIRParentLoop(Lp));

  if (!isa<SCEVAddRecExpr>(SC)) {
    return false;
  }

  auto Range = SE.getSignedRange(SC);

  if (!Range.getSignedMin().isNonNegative()) {
    return false;
  }

  // Checking minimum signed value of IV is enough to deduce NSW for single-exit
  // loops but we need more checks for multi-exit loops.
  if (Lp->getExitingBlock()) {
    return true;
  }

  // ScalarEvolution could have used an early exit to compute the range info on
  // the IV. For example, the loop below has a max trip count of 2 due to the
  // early exit so 'i' has a range of [5, 8). This doesn't mean we can use a
  // signed comparison to generate the bottom test as 'N' may be big positive
  // (negative signed) value.
  // for(i = 5; i < N; i++) {
  //   if (i == 7)
  //     goto exit;
  // }

  APInt Max = Range.getSignedMax();
  APInt RefinedMax = getAddRecRefinedSignedMax(cast<SCEVAddRecExpr>(SC));

  // If no range refinement was done for the max value of IV based on loop exit,
  // we can be sure it remains in non-negative range.
  return Max.sge(RefinedMax);
}

bool HIRLoopFormation::hasNSWSemantics(const Loop *Lp, Type *IVType,
                                       const SCEV *BECount) const {
  assert(IVType->isIntegerTy() && "Integer IV type expected!");

  // Loop has NSW if backedge taken count is in signed range.
  if (!isa<SCEVCouldNotCompute>(BECount) && SE.isKnownNonNegative(BECount)) {
    return true;
  }

  // Set NSW if there is a non-negative integer IV in the loop header (less
  // than or equal to the size of IVType).
  // For example-
  //
  // %p.addr.07 = phi i32* [ %incdec.ptr, %for.body ], [ %p, %entry ] <<
  // pointer IV
  // %indvars.iv = phi i64 [ %indvars.iv.next, %for.body ], [ 0, %entry ] <<
  // NSW IV of same size as pointer
  auto IVSize = IVType->getPrimitiveSizeInBits();
  auto HeaderBB = Lp->getHeader();

  for (auto &PhiInst : (*HeaderBB)) {
    if (!isa<PHINode>(PhiInst)) {
      break;
    }

    auto PhiTy = PhiInst.getType();

    if (!PhiTy->isIntegerTy() || (PhiTy->getPrimitiveSizeInBits() > IVSize)) {
      continue;
    }

    if (isNonNegativeNSWIV(Lp, &cast<PHINode>(PhiInst))) {
      return true;
    }
  }

  return false;
}

void HIRLoopFormation::setIVType(HLLoop *HLoop, const SCEV *BECount) const {
  Value *Cond;
  auto Lp = HLoop->getLLVMLoop();
  auto Latch = Lp->getLoopLatch();

  assert(Latch && "Loop doesn't have a latch!");

  auto Term = Latch->getTerminator();

  if (auto BottomTest = dyn_cast<BranchInst>(Term)) {
    assert(BottomTest->isConditional() &&
           "Loop bottom test is not a conditional branch!");
    Cond = BottomTest->getCondition();
  } else {
    llvm_unreachable("Cannot handle loop bottom test!");
  }

  assert(isa<Instruction>(Cond) &&
         "Loop exit condition is not an instruction!");

  auto *IVNode = RI.findIVDefInHeader(*Lp, cast<Instruction>(Cond));

  auto *IVType = IVNode ? IVNode->getType() : nullptr;

  // If the IVType is not an integer, assign it an integer type which is able to
  // represent the address space.
  if (!IVType || !IVType->isIntegerTy()) {
    IVType = Type::getIntNTy(
        Func->getContext(),
        Func->getParent()->getDataLayout().getPointerSizeInBits());
  }

  HLoop->setIVType(IVType);

  auto IsNSW = hasNSWSemantics(Lp, IVType, BECount);
  HLoop->setNSW(IsNSW);
}

bool HIRLoopFormation::populatedPreheaderPostexitNodes(
    HLLoop *HLoop, HLIf *IfParent, bool PredicateInversion) {

  auto PreBegIt =
      !PredicateInversion ? IfParent->then_begin() : IfParent->else_begin();
  auto PreEndIt = HLoop->getIterator();

  auto PostBegIt = std::next(HLoop->getIterator());
  auto PostEndIt =
      !PredicateInversion ? IfParent->then_end() : IfParent->else_end();

  bool HasPreheader = (PreBegIt != PreEndIt);
  bool HasPostexit = (PostBegIt != PostEndIt);

  if ((HasPreheader &&
       !HLNodeUtils::validPreheaderPostexitNodes(PreBegIt, PreEndIt)) ||
      (HasPostexit &&
       !HLNodeUtils::validPreheaderPostexitNodes(PostBegIt, PostEndIt))) {
    return false;
  }

  if (HasPreheader) {
    HLNodeUtils::moveAsFirstPreheaderNodes(HLoop, PreBegIt, PreEndIt);
  }

  if (HasPostexit) {
    HLNodeUtils::moveAsFirstPostexitNodes(HLoop, PostBegIt, PostEndIt);
  }

  return true;
}

void HIRLoopFormation::setZtt(HLLoop *HLoop) {

  auto Lp = HLoop->getLLVMLoop();
  bool PredicateInversion = false;

  // Check whether loop has an if parent.
  auto Parent = HLoop->getParent();

  if (!Parent) {
    return;
  }

  auto IfParent = dyn_cast<HLIf>(Parent);

  if (!IfParent) {
    return;
  }

  if (IfParent->hasElseChildren()) {
    if (IfParent->hasThenChildren()) {
      return;
    }
    PredicateInversion = true;
  }

  auto IfBB = HIRCr.getSrcBBlock(IfParent);
  auto IfBrInst = cast<BranchInst>(IfBB->getTerminator());

  if (!SE.isLoopZtt(Lp, getOutermostHIRParentLoop(Lp), IfBrInst,
                    PredicateInversion)) {
    return;
  }

  // This function returns false if the condition is acting like a ztt but
  // cannot be set as one due to presence of non-HLInst nodes so we need to
  // bail out.
  if (!populatedPreheaderPostexitNodes(HLoop, IfParent, PredicateInversion)) {
    return;
  }

  // IfParent should only contain the loop now.
  assert(((!PredicateInversion && (IfParent->getNumThenChildren() == 1)) ||
          (PredicateInversion && (IfParent->getNumElseChildren() == 1))) &&
         "Something went wrong during ztt recognition!");

  HLNodeUtils::moveBefore(IfParent, HLoop);
  HLNodeUtils::remove(IfParent);

  HLoop->setZtt(IfParent);

  if (PredicateInversion) {
    InvertedZttLoops.insert(HLoop);

    if (MDNode *OrigProfData = IfParent->getProfileData()) {
      // Get the profile data inverted for IfParent.
      MDNode *ProfData =
          HLNodeUtils::swapProfMetadata(HNU.getContext(), OrigProfData);
      IfParent->setProfileData(ProfData);
    }
  }
}

const Loop *HIRLoopFormation::getOutermostHIRParentLoop(const Loop *Lp) const {
  const Loop *ParLp, *TmpLp;

  ParLp = Lp;

  while ((TmpLp = ParLp->getParentLoop()) &&
         CurRegion->containsBBlock(TmpLp->getHeader())) {
    ParLp = TmpLp;
  }

  return ParLp;
}

static void setProfileData(HLIf *BottomTest, HLLabel *LoopLabel,
                           HLLoop *HLoop) {
  MDNode *ProfData = BottomTest->getProfileData();

  if (!ProfData) {
    return;
  }

  assert(BottomTest->hasThenChildren() || BottomTest->hasElseChildren());
  bool IsThenBackedge =
      BottomTest->getFirstThenChild() &&
      (cast<HLGoto>(BottomTest->getFirstThenChild())->getTargetLabel() ==
       LoopLabel);

  assert(
      IsThenBackedge ||
      (BottomTest->getFirstElseChild() &&
       (cast<HLGoto>(BottomTest->getFirstElseChild())->getTargetLabel() ==
        LoopLabel)));

  if (IsThenBackedge) {
    HLoop->setProfileData(BottomTest->getProfileData());
  } else {
    HLoop->setProfileData(HLNodeUtils::swapProfMetadata(
        (HLoop->getHLNodeUtils()).getContext(), BottomTest->getProfileData()));
  }
}

void HIRLoopFormation::processLoopExitGoto(HLIf *BottomTest, HLLabel *LoopLabel,
                                           HLLoop *HLoop) const {
  // If loop exit goto was not removed as redundant, it means that it is not the
  // lexical successor of the loop. In this case we need to preserve the goto by
  // moving it after the loop.
  //
  // Change from-
  //
  // + UNKNOWN LOOP i2
  // |  L:
  // |  ...
  // |  if () {
  // |    goto L;
  // |  else {
  // |    goto Exit;
  // |  }
  // + END LOOP
  //
  // OtherLabel:
  //
  // To-
  //
  // + UNKNOWN LOOP i2
  // |  L:
  // |  ...
  // |  if () {
  // |    goto L;
  // }  }
  // + END LOOP
  // goto Exit;
  //
  // OtherLabel:
  //
  if (!BottomTest->hasThenChildren()) {
    return;
  }

  assert(((BottomTest->getNumThenChildren() == 1) &&
          isa<HLGoto>(BottomTest->getFirstThenChild())) &&
         "Unexpected bottom test!");

  if (!BottomTest->hasElseChildren()) {
    return;
  }

  assert(((BottomTest->getNumElseChildren() == 1) &&
          isa<HLGoto>(BottomTest->getFirstElseChild())) &&
         "Unexpected bottom test!");

  auto ThenGoto = cast<HLGoto>(BottomTest->getFirstThenChild());

  // Check which goto represents backedge and move the other one after the loop.
  if (ThenGoto->getTargetLabel() == LoopLabel) {
    ThenGoto->getHLNodeUtils().moveAfter(HLoop,
                                         BottomTest->getFirstElseChild());
  } else {
    ThenGoto->getHLNodeUtils().moveAfter(HLoop, ThenGoto);
  }
}

void HIRLoopFormation::formLoops() {

  HLRegion *PrevRegion = nullptr;

  // Traverse RequiredLabels set computed by HIRCleanup phase to form loops.
  for (auto I = HIRC.getRequiredLabels().begin(),
            E = HIRC.getRequiredLabels().end();
       I != E; ++I) {
    auto Label = *I;
    BasicBlock *HeaderBB = Label->getSrcBBlock();

    if (!LI.isLoopHeader(HeaderBB)) {
      continue;
    }

    CurRegion = Label->getParentRegion();

    if (PrevRegion != CurRegion) {
      // Since RequiredLabels is sorted by node number, this should work like
      // lexical traversal of regions so we shouldn't be consuming extra compile
      // time by unnecessarily switching between regions.
      PrevRegion = CurRegion;
      SE.clearHIRCache();
    }

    // Found a loop
    Loop *Lp = LI.getLoopFor(HeaderBB);

    auto BECount =
        SE.getBackedgeTakenCountForHIR(Lp, getOutermostHIRParentLoop(Lp));

    bool IsUnknownLoop = isa<SCEVCouldNotCompute>(BECount);
    bool IsConstTripLoop = isa<SCEVConstant>(BECount);

    // Find HIR hook for the loop latch.
    auto LatchHook = HIRC.findHIRHook(Lp->getLoopLatch());

    assert((Label->getParent() == LatchHook->getParent()) &&
           "Wrong lexical links built!");

    HLContainerTy::iterator LabelIter(Label);
    HLContainerTy::iterator BottomTestIter(LatchHook);

    // Look for the bottom test.
    while (!isa<HLIf>(BottomTestIter)) {
      BottomTestIter = std::next(BottomTestIter);
    }

    HLIf *BottomTest = cast<HLIf>(&*BottomTestIter);

    // Create a new loop and move its children inside.
    HLLoop *HLoop = HNU.createHLLoop(Lp);
    HLNodeUtils::insertBefore(Label, HLoop);

    HLoop->setBranchDebugLoc(BottomTest->getDebugLoc());
    HLoop->setCmpTestDebugLoc(BottomTest->pred_begin()->DbgLoc);

    setProfileData(BottomTest, Label, HLoop);
    processLoopExitGoto(BottomTest, Label, HLoop);

    // Include Label and bottom test as explicit nodes inside the unknown loop.
    auto FirstChildIter = IsUnknownLoop ? LabelIter : std::next(LabelIter);
    auto EndIter = IsUnknownLoop ? std::next(BottomTestIter) : BottomTestIter;

    HLNodeUtils::moveAsFirstChildren(HLoop, FirstChildIter, EndIter);

    setIVType(HLoop, BECount);

    if (!IsUnknownLoop) {
      // Remove label and bottom test.
      HLNodeUtils::remove(Label);
      HLNodeUtils::remove(BottomTest);

      // Keep a reference to label and bottom test in case parsing needs it as a
      // backup option.
      LoopLabelAndBottomTestMap.insert(std::make_pair(
          HLoop, LoopLabelAndBottomTestPairTy(Label, BottomTest)));

      // TODO: Look into whether setting ztt is beneficial for unknown loops.
      if (!IsConstTripLoop) {
        setZtt(HLoop);
      }
    }

    // Add entry for (Lp -> HLLoop) mapping.
    insertHLLoop(Lp, HLoop);
  }
}

void HIRLoopFormation::run() {
  this->Func = &HNU.getFunction();
  formLoops();
}

void HIRLoopFormation::reattachLoopLabelAndBottomTest(HLLoop *Loop) {
  auto It = LoopLabelAndBottomTestMap.find(Loop);
  assert(It != LoopLabelAndBottomTestMap.end() && "Could not find loop label!");

  HLNodeUtils::insertAsFirstChild(Loop, It->second.first);
  HLNodeUtils::insertAsLastChild(Loop, It->second.second);

  LoopLabelAndBottomTestMap.erase(It);
}

void HIRLoopFormation::eraseStoredLoopLabelsAndBottomTests() {
  for (auto &LabelAndBottomTestPair : LoopLabelAndBottomTestMap) {
    HNU.destroy(LabelAndBottomTestPair.second.first);
    HNU.destroy(LabelAndBottomTestPair.second.second);
  }
}
