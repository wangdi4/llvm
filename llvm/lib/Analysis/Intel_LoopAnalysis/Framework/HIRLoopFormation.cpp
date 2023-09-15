//===------- HIRLoopFormation.cpp - Creates HIR Loops ---------------------===//
//
// Copyright (C) 2015 Intel Corporation. All rights reserved.
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

#if INTEL_FEATURE_SHARED_SW_ADVANCED
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

  auto ConstStride =
      dyn_cast<SCEVConstant>(AddRec->getStepRecurrence(ScopedSE));

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
  auto SC = ScopedSE.getSCEV(const_cast<PHINode *>(IVPhi));

  if (!isa<SCEVAddRecExpr>(SC)) {
    return false;
  }

  auto Range = ScopedSE.getSignedRange(SC);

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

  // Loop has signed IV if backedge taken count is in signed range.
  if (!isa<SCEVCouldNotCompute>(BECount) &&
      ScopedSE.isKnownNonNegative(BECount)) {
    return true;
  }

  // Single exit loop has signed IV if max backedge taken count is in signed
  // range.
  if (Lp->getExitingBlock()) {
    auto *MaxBECount = ScopedSE.getScopedConstantMaxBackedgeTakenCount(Lp);

    if (!isa<SCEVCouldNotCompute>(MaxBECount) &&
        ScopedSE.isKnownNonNegative(MaxBECount)) {
      return true;
    }
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

  SmallPtrSet<const Instruction *, 16> VisitedInsts;
  auto *IVNode = HIRCr.getRI().findIVDefInHeader(*Lp, cast<Instruction>(Cond),
                                                 VisitedInsts);

  auto *IVType = IVNode ? IVNode->getType() : nullptr;

  // If the IVType is not an integer, assign it an integer type which is able to
  // represent the address space.
  if (!IVType || !IVType->isIntegerTy() ||
      (!isa<SCEVCouldNotCompute>(BECount) &&
       !BECount->getType()->isIntegerTy()) ||
      // Due to a quirk of SSA, it is possible for the loop trip count to be
      // oustide the range of IV with i1 type so we use pointer sized IV
      // instead. The trip count of this loop is 2-
      //
      // for.i:
      //   %i.08.i = phi i1 [ true, %entry ], [ false, %for.i ]
      //   br i1 %i.08.i, label %for.i, label %exit
      (IVType->getPrimitiveSizeInBits() == 1)) {
    IVType = Type::getIntNTy(
        Func->getContext(),
        Func->getParent()->getDataLayout().getPointerSizeInBits());
  }

  HLoop->setIVType(IVType);

  bool IsSigned = hasNSWSemantics(Lp, IVType, BECount);
  HLoop->setHasSignedIV(IsSigned);
}

bool HIRLoopFormation::populatedPostexitNodes(HLLoop *HLoop, HLIf *ParentIf,
                                              bool PredicateInversion,
                                              bool &HasPostSiblingLoop) const {
  auto PostBegIt = std::next(HLoop->getIterator());
  auto PostEndIt =
      !PredicateInversion ? ParentIf->then_end() : ParentIf->else_end();

  HLInst *RegionExitInst = nullptr;
  bool HasPostexit = (PostBegIt != PostEndIt);

  for (auto It = PostBegIt; It != PostEndIt; ++It) {
    auto *Node = &*It;

    if (auto *Inst = dyn_cast<HLInst>(Node)) {

      // Keep track of region exit intrinsic to make sure it stays in the
      // postexit of this loop rather than moving it to the preheader of
      // post-sibling loop which would be incorrect.
      auto *Intrin = dyn_cast<IntrinsicInst>(Inst->getLLVMInstruction());
      if (Intrin &&
          Intrin->getIntrinsicID() == Intrinsic::directive_region_exit) {
        RegionExitInst = Inst;
      }

      continue;
    }

    // Handle the case of post-sibling loop.
    // If this utility is called in HIRLoopFormation phase, then the
    // post-sibling loop may not be formed so we look for loop header label. If
    // the utility was called from HIRFramework phase (for processing deferred
    // ZTT candidates), we will find a HLLoop.
    auto *Label = dyn_cast<HLLabel>(Node);

    if (isa<HLLoop>(Node) ||
        (Label && LI.isLoopHeader(Label->getSrcBBlock()))) {

      HasPostSiblingLoop = true;

      // Use RegionExitInst to mark the end of this loop's postexit.
      if (RegionExitInst) {
        PostEndIt = std::next(RegionExitInst->getIterator());

      } else {
        // Use the postexit nodes as next loop's preheader because instructions
        // are more likely to have been hoisted than sinked in the incoming IR.
        HasPostexit = false;
      }
      break;
    }

    return false;
  }

  if (HasPostexit) {
    HLNodeUtils::moveAsFirstPostexitNodes(HLoop, PostBegIt, PostEndIt);
  }

  return true;
}

bool HIRLoopFormation::populatedPreheaderPostexitNodes(
    HLLoop *HLoop, HLIf *IfParent, bool PredicateInversion,
    bool &HasPostSiblingLoop) {

  auto PreBegIt =
      !PredicateInversion ? IfParent->then_begin() : IfParent->else_begin();
  auto PreEndIt = HLoop->getIterator();

  bool HasPreheader = (PreBegIt != PreEndIt);

  if (HasPreheader &&
      !HLNodeUtils::validPreheaderPostexitNodes(PreBegIt, PreEndIt)) {
    return false;
  }

  if (!populatedPostexitNodes(HLoop, IfParent, PredicateInversion,
                           HasPostSiblingLoop)) {
    return false;
  }

  if (HasPreheader) {
    HLNodeUtils::moveAsFirstPreheaderNodes(HLoop, PreBegIt, PreEndIt);
  }

  return true;
}

bool HIRLoopFormation::setRecognizedZtt(HLLoop *HLoop, HLIf *IfParent,
                                        bool PredicateInversion) {
  bool HasPostSiblingLoop = false;

  // This function returns false if the condition is acting like a ztt but
  // cannot be set as one due to presence of non-HLInst nodes so we need to
  // bail out.
  if (!populatedPreheaderPostexitNodes(HLoop, IfParent, PredicateInversion,
                                       HasPostSiblingLoop)) {
    return false;
  }

  // IfParent should only contain the loop now.
  assert(HasPostSiblingLoop ||
         ((!PredicateInversion && (IfParent->getNumThenChildren() == 1)) ||
          (PredicateInversion && (IfParent->getNumElseChildren() == 1))) &&
             "Something went wrong during ztt recognition!");

  HLNodeUtils::moveBefore(IfParent, HLoop);

  if (HasPostSiblingLoop) {
    // If we have a sibling loop, use the cloned if as the Ztt.
    auto *IfParentClone = IfParent->cloneEmpty();
    // Set src block for the new if so parser can use it.
    HIRCr.setSrcBBlock(IfParentClone, HIRCr.getSrcBBlock(IfParent));
    IfParent = IfParentClone;

  } else {
    HLNodeUtils::remove(IfParent);
  }

  HLoop->setZtt(IfParent);

  if (PredicateInversion) {
    if (MDNode *OrigProfData = IfParent->getProfileData()) {
      // Get the profile data inverted for IfParent.
      MDNode *ProfData = HLNodeUtils::swapProfMetadata(
          HLoop->getHLNodeUtils().getContext(), OrigProfData);
      IfParent->setProfileData(ProfData);
    }
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

  bool IsDeferredZttCandidate = false;

  if (IfParent->hasElseChildren()) {
    if (IfParent->hasThenChildren()) {
      // Sometimes both if/else cases have children but one set of children are
      // optimized away after parsing so we can defer it to parser.
      IsDeferredZttCandidate = true;

      auto IsLoop = [=](HLNode &Node) { return &Node == HLoop; };

      PredicateInversion =
          std::any_of(IfParent->else_begin(), IfParent->else_end(), IsLoop);

    } else {
      PredicateInversion = true;
    }
  }

  auto IfBB = HIRCr.getSrcBBlock(IfParent);
  auto IfBrInst = cast<BranchInst>(IfBB->getTerminator());

  if (!ScopedSE.isLoopZtt(Lp, IfBrInst, PredicateInversion)) {
    return;
  }

  if (IsDeferredZttCandidate) {
    DeferredZtts.emplace_back(HLoop, IfParent);
    return;
  }

  if (!setRecognizedZtt(HLoop, IfParent, PredicateInversion)) {
    return;
  }

  if (PredicateInversion) {
    InvertedZttLoops.insert(HLoop);
  }
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

  auto *ThenGoto = cast<HLGoto>(BottomTest->getFirstThenChild());
  HLNode *MovedGoto = ThenGoto;

  // Check which goto represents backedge and move the other one after the loop.
  if (ThenGoto->getTargetLabel() == LoopLabel) {
    MovedGoto = BottomTest->getFirstElseChild();
    HLNodeUtils::moveAfter(HLoop, MovedGoto);

  } else {
    HLNodeUtils::moveAfter(HLoop, ThenGoto);
  }

  // If goto has a non-label successor, we create a dummy label so that the
  // verifier doesn't complain about dead nodes.
  if (!HLNodeUtils::isLexicalLastChildOfParent(MovedGoto)) {
    auto *LabelSucc = dyn_cast<HLLabel>(&*std::next(MovedGoto->getIterator()));

    if (!LabelSucc) {
      auto *Label = MovedGoto->getHLNodeUtils().createHLLabel("Unused");
      HLNodeUtils::insertAfter(MovedGoto, Label);
    }
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
      ScopedSE.setScope(CurRegion->getIRRegion().getOutermostLoops());
    }

    // Found a loop
    Loop *Lp = LI.getLoopFor(HeaderBB);

    auto BECount = ScopedSE.getScopedBackedgeTakenCount(Lp);

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

    // Include Label and bottom test as explicit nodes inside the unknown loop.
    auto FirstChildIter = IsUnknownLoop ? LabelIter : std::next(LabelIter);
    auto EndIter = IsUnknownLoop ? std::next(BottomTestIter) : BottomTestIter;

    HLNodeUtils::moveAsFirstChildren(HLoop, FirstChildIter, EndIter);
    processLoopExitGoto(BottomTest, Label, HLoop);

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

#endif // INTEL_FEATURE_SHARED_SW_ADVANCED

bool HIRLoopFormation::reattachLoopLabelAndBottomTest(HLLoop *Loop) {
  auto It = LoopLabelAndBottomTestMap.find(Loop);

  // Loop label and bottom test was never removed in the first place.
  if (It == LoopLabelAndBottomTestMap.end()) {
    return false;
  }

  HLNodeUtils::insertAsFirstChild(Loop, It->second.first);
  HLNodeUtils::insertAsLastChild(Loop, It->second.second);

  LoopLabelAndBottomTestMap.erase(It);

  return true;
}

void HIRLoopFormation::eraseStoredLoopLabelsAndBottomTests() {
  for (auto &LabelAndBottomTestPair : LoopLabelAndBottomTestMap) {
    HNU.destroy(LabelAndBottomTestPair.second.first);
    HNU.destroy(LabelAndBottomTestPair.second.second);
  }
}
