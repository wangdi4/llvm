//===--- HIRTransformUtils.cpp  -------------------------------------------===//
//
// Copyright (C) 2015-2016 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// This file implements HIRTransformUtils class.
//
//===----------------------------------------------------------------------===//

#include "llvm/Support/Debug.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/HIRTransformUtils.h"

#include "llvm/Transforms/Intel_LoopTransforms/HIRLoopReversal.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/HIRInvalidationUtils.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/HLNodeUtils.h"
#include "llvm/Transforms/Intel_LoopTransforms/HIRLMM.h"

#define DEBUG_TYPE "hir-transform-utils"

using namespace llvm;
using namespace llvm::loopopt;
using namespace llvm::loopopt::reversal;
using namespace llvm::loopopt::lmm;

bool HIRTransformUtils::isHIRLoopReversible(
    HLLoop *InnermostLp,            // INPUT + OUTPUT: a given loop
    HIRDDAnalysis &HDDA,            // INPUT: HIR DDAnalysis
    HIRSafeReductionAnalysis &HSRA, // INPUT: HIRSafeReductionAnalysis
    HIRLoopStatistics &HLS,         // INPUT: Existing HIRLoopStatitics
    bool DoProfitTest               // INPUT: Control Profit Tests
    ) {
  assert(InnermostLp && "HLLoop* can't be a nullptr\n");
  assert(InnermostLp->isInnermost() &&
         "HIR LoopReversal can only work with an inner-most loop\n");

  // Create an HIRLoopReversal object on stack
  HIRLoopReversal ReversalPass;

  // Call HIRLoopReversal.isReversible(-)
  return ReversalPass.isReversible(
      InnermostLp,  // HLLoop*
      HDDA,         // Existing DDAnalysis
      HSRA,         // Existing SafeReductionAnalysis
      HLS,          // Existing HIRLoopStatistics Analysis
      DoProfitTest, // Control Profit Test
      true,         // Always do Legal Tests
      false         // OFF Short-circuit action (doing analysis now)
      );
}

void HIRTransformUtils::doHIRLoopReversal(
    HLLoop *InnermostLp,            // INPUT + OUTPUT: an inner-most loop
    HIRDDAnalysis &HDDA,            // INPUT: HIR DDAnalysis
    HIRSafeReductionAnalysis &HSRA, // INPUT: HIRSafeReductionAnalysis
    HIRLoopStatistics &HLS          // INPUT: Existing HIRLoopStatitics
    ) {
  assert(InnermostLp && "HLLoop* can't be a nullptr\n");
  assert(InnermostLp->isInnermost() &&
         "HIR LoopReversal can only work with an inner-most loop\n");

  // Create an HIRLoopReversal object on stack
  HIRLoopReversal ReversalPass;

  // Call HIRLoopReversal.isReversible(-), expect the loop is reversible
  bool IsReversible = ReversalPass.isReversible(
      InnermostLp, // HLLoop*
      HDDA,        // Existing DDAnalysis
      HSRA,        // Existing SafeReductionAnalysis
      HLS,         // Existing HIRLoopStatistics Analysis
      false,       // Ignore Profit Tests
      false,       // Assert on legality
      true // ON Short-circuit action (expect the analysis has been done in the
           // previous API call)
      );

  // Reverse the Loop
  assert(IsReversible && "Expect the loop is reversible\n");
  (void)IsReversible;

  ReversalPass.doHIRReversalTransform(InnermostLp);
}

bool HIRTransformUtils::doHIRLoopRedundantMemoryMotion(
    HLLoop *InnermostLp,   // INPUT + OUTPUT: a given innermost loop
    HIRDDAnalysis &HDDA,   // INPUT: Existing HIR DDAnalysis
    HIRLoopStatistics &HLS // INPUT: Existing HIR LoopStatitics Analysis
    ) {
  assert(InnermostLp && "HLLoop* can't be null\n");
  assert(InnermostLp->isInnermost() && "HIR LRMM (Loop Redundant Memory "
                                       "Motion) can only work on an inner-most "
                                       "loop\n");

  // to implement!

  return false;
}

bool HIRTransformUtils::doHIRLoopInvariantMemoryMotion(
    HLLoop *InnermostLp,   // INPUT + OUTPUT: a given innermost loop
    HIRDDAnalysis &HDDA,   // INPUT: Existing HIR DDAnalysis
    HIRLoopStatistics &HLS // INPUT: Existing HIR LoopStatitics Analysis
    ) {
  assert(InnermostLp && "HLLoop* can't be null\n");
  assert(InnermostLp->isInnermost() && "HIR LIMM (Loop Invariant Memory "
                                       "Motion) can only work on an inner-most "
                                       "loop\n");

  HIRLMM LMMPass;
  return LMMPass.doLoopMemoryMotion(InnermostLp, HDDA, HLS);
}

bool HIRTransformUtils::doHIRLoopMemoryMotion(
    HLLoop *InnermostLp,   // INPUT + OUTPUT: a given innermost loop
    HIRDDAnalysis &HDDA,   // INPUT: HIR DDAnalysis
    HIRLoopStatistics &HLS // INPUT: Existing HIRLoopStatitics Analysis
    ) {
  assert(InnermostLp && "HLLoop* can't be null\n");
  assert(InnermostLp->isInnermost() &&
         "HIR LMM (Loop Memory Motion) can only work on an inner-most loop\n");

  // to implement!
  return false;
}

bool HIRTransformUtils::isRemainderLoopNeeded(HLLoop *OrigLoop,
                                              unsigned UnrollOrVecFactor,
                                              uint64_t *NewTripCountP,
                                              RegDDRef **NewTCRef) {

  uint64_t TripCount;

  if (OrigLoop->isConstTripLoop(&TripCount)) {

    uint64_t NewTripCount = TripCount / UnrollOrVecFactor;
    *NewTripCountP = NewTripCount;

    // Return true if UnrollOrVecFactor does not evenly divide TripCount.
    return ((NewTripCount * UnrollOrVecFactor) != TripCount);
  }

  // Process for non-const trip loop.
  RegDDRef *Ref = OrigLoop->getTripCountDDRef();
  // New instruction should only be created for non-constant trip loops.
  assert(!Ref->isIntConstant() && " Creating a new instruction for constant"
                                  "trip loops should not occur.");

  // This will create a new instruction for calculating ub of unrolled loop
  // and lb of remainder loop. The new instruction is t = (N/8) where 'N' is
  // the trip count of the original loop.
  HLInst *TempInst = nullptr;
  CanonExpr *TripCE = Ref->getSingleCanonExpr();
  auto &HNU = OrigLoop->getHLNodeUtils();

  if (TripCE->isSignedDiv() && (TripCE->getDenominator() != 1)) {
    // Create DDRef for Unroll Factor.
    RegDDRef *UFDD = Ref->getDDRefUtils().createConstDDRef(Ref->getDestType(),
                                                           UnrollOrVecFactor);
    TempInst = OrigLoop->getHLNodeUtils().createUDiv(Ref, UFDD, "tgu");
  } else {
    SmallVector<const RegDDRef *, 3> AuxRefs = {OrigLoop->getStrideDDRef(),
                                                OrigLoop->getLowerDDRef(),
                                                OrigLoop->getUpperDDRef()};

    // Use the same canon expr to generate the division.
    TripCE->divide(UnrollOrVecFactor);
    TripCE->simplify(true);

    Ref->setSymbase(Ref->getDDRefUtils().getNewSymbase());

    Ref->makeConsistent(&AuxRefs, OrigLoop->getNestingLevel() - 1);

    TempInst = HNU.createCopyInst(Ref, "tgu");
  }
  HNU.insertBefore(const_cast<HLLoop *>(OrigLoop), TempInst);
  *NewTCRef = TempInst->getLvalDDRef();

  return true;
}

void HIRTransformUtils::updateBoundDDRef(RegDDRef *BoundRef, unsigned BlobIndex,
                                         unsigned DefLevel) {
  // Overwrite symbase to a newly created one to avoid unnecessary DD edges.
  BoundRef->setSymbase(BoundRef->getDDRefUtils().getNewSymbase());

  // Add blob DDRef for the temp in UB.
  BoundRef->addBlobDDRef(BlobIndex, DefLevel);
  BoundRef->updateDefLevel();
}

HLLoop *HIRTransformUtils::createUnrollOrVecLoop(HLLoop *OrigLoop,
                                                 unsigned UnrollOrVecFactor,
                                                 uint64_t NewTripCount,
                                                 const RegDDRef *NewTCRef,
                                                 bool VecMode) {
  HLLoop *NewLoop = OrigLoop->cloneEmptyLoop();

  // Number of exits do not change due to vectorization
  if (!VecMode) {
    NewLoop->setNumExits((OrigLoop->getNumExits() - 1) * UnrollOrVecFactor + 1);
  }

  OrigLoop->getHLNodeUtils().insertBefore(OrigLoop, NewLoop);

  // Update the loop upper bound.
  if (NewTripCount != 0) {
    uint64_t NewBound;

    // For vectorizer mode, upper bound needs to be multiplied by
    // UnrollOrVecFactor since it is used as the stride
    NewBound = VecMode ? (NewTripCount * UnrollOrVecFactor) : NewTripCount;

    // Subtract 1.
    NewBound = NewBound - 1;

    NewLoop->getUpperCanonExpr()->setConstant(NewBound);
  } else {

    // Create 't-1' as new UB.
    assert(NewTCRef && " New Ref is null.");
    RegDDRef *NewUBRef = NewTCRef->clone();

    // For vectorizer mode, upper bound needs to be multiplied by
    // UnrollOrVecFactor since it is used as the stride
    if (VecMode) {
      auto Ret =
          NewUBRef->getSingleCanonExpr()->multiplyByConstant(UnrollOrVecFactor);
      assert(Ret && "multiplyByConstant() failed");
      (void)Ret;
    }

    // Subtract 1.
    NewUBRef->getSingleCanonExpr()->addConstant(-1, true);

    NewLoop->setUpperDDRef(NewUBRef);

    // Sets defined at level of bound ref to (nesting level - 1) as the UB temp
    // is defined just before the loop.
    updateBoundDDRef(NewUBRef, NewTCRef->getSelfBlobIndex(),
                     OrigLoop->getNestingLevel() - 1);

    // Generate the Ztt.
    NewLoop->createZtt(false);

    // Update unrolled/vectorized loop's trip count estimate.
    NewLoop->setMaxTripCountEstimate(NewLoop->getMaxTripCountEstimate() /
                                     UnrollOrVecFactor);
  }

  // Set the code gen for modified region
  assert(NewLoop->getParentRegion() && " Loop does not have a parent region.");
  NewLoop->getParentRegion()->setGenCode();

  // Vectorization uses UnrollOrVecFactor as stride
  if (VecMode) {
    NewLoop->getStrideDDRef()->getSingleCanonExpr()->setConstant(
        UnrollOrVecFactor);
  }

  return NewLoop;
}

void HIRTransformUtils::processRemainderLoop(HLLoop *OrigLoop,
                                             unsigned UnrollOrVecFactor,
                                             uint64_t NewTripCount,
                                             const RegDDRef *NewTCRef) {
  // Mark Loop bounds as modified.
  HIRInvalidationUtils::invalidateBounds(OrigLoop);

  // Modify the LB of original loop.
  if (NewTripCount) {
    // OrigLoop is a const-trip loop.
    RegDDRef *OrigLBRef = OrigLoop->getLowerDDRef();
    CanonExpr *LBCE = OrigLBRef->getSingleCanonExpr();
    LBCE->setConstant(NewTripCount * UnrollOrVecFactor);
  } else {

    // Non-constant trip loop, lb = (UnrollOrVecFactor)*t.
    RegDDRef *NewLBRef = NewTCRef->clone();
    auto Ret =
        NewLBRef->getSingleCanonExpr()->multiplyByConstant(UnrollOrVecFactor);
    assert(Ret && "multiplyByConstant() failed.");
    (void)Ret;

    OrigLoop->setLowerDDRef(NewLBRef);
    // Sets the defined at level of new LB to (nesting level - 1) as the LB temp
    // is defined just before the loop.
    updateBoundDDRef(NewLBRef, NewTCRef->getSelfBlobIndex(),
                     OrigLoop->getNestingLevel() - 1);

    OrigLoop->createZtt(false);

    // Update remainder loop's trip count estimate.
    OrigLoop->setMaxTripCountEstimate(UnrollOrVecFactor - 1);
  }

  DEBUG(dbgs() << "\n Remainder Loop \n");
  DEBUG(OrigLoop->dump());
}

HLLoop *HIRTransformUtils::setupMainAndRemainderLoops(
    HLLoop *OrigLoop, unsigned UnrollOrVecFactor, bool &NeedRemainderLoop,
    bool VecMode) {
  // Extract Ztt and add it outside the loop.
  OrigLoop->extractZtt();

  // Extract preheader and postexit
  OrigLoop->extractPreheaderAndPostexit();

  // Create UB instruction before the loop 't = (Orig UB)/(UnrollOrVecFactor)'
  // for non-constant trip loops. For const trip loops calculate the bound.
  RegDDRef *NewTCRef = nullptr;
  uint64_t NewTripCount = 0;
  NeedRemainderLoop = isRemainderLoopNeeded(OrigLoop, UnrollOrVecFactor,
                                            &NewTripCount, &NewTCRef);

  // Create the main loop.
  HLLoop *MainLoop = createUnrollOrVecLoop(OrigLoop, UnrollOrVecFactor,
                                           NewTripCount, NewTCRef, VecMode);

  // Update the OrigLoop to remainder loop by setting bounds appropriately if
  // remainder loop is needed.
  if (NeedRemainderLoop) {
    processRemainderLoop(OrigLoop, UnrollOrVecFactor, NewTripCount, NewTCRef);
  }

  // Mark parent for invalidation
  HIRInvalidationUtils::invalidateParentLoopBodyOrRegion(OrigLoop);

  return MainLoop;
}

/// Update Loop properties based on Input Permutations
/// Used by Loop Interchange now. Will be useful for loop blocking later
void HIRTransformUtils::permuteLoopNests(
    HLLoop *OutermostLoop, const SmallVectorImpl<const HLLoop *> &LoopPermutation) {

  SmallVector<HLLoop *, MaxLoopNestLevel> SavedLoops;
  HLLoop *DstLoop = OutermostLoop;

  // isPerfectLoopNest() allows Prehdr/PostExit
  // in outermost loop. If not extracted, it will lead to errors
  // in this case:
  // do i2=1,n   (before interchange)
  //    do i3 =1,6
  //    end
  // end
  //	 a[i1] = 2 (PostExit)
  //
  if (OutermostLoop != LoopPermutation.front()) {
    OutermostLoop->extractPreheaderAndPostexit();
  }

  for (auto &Lp : LoopPermutation) {
    HLLoop *LoopCopy = Lp->cloneEmptyLoop();
    LoopCopy->setNestingLevel(Lp->getNestingLevel());
    SavedLoops.push_back(LoopCopy);
  }

  for (auto &Lp : LoopPermutation) {
    assert(DstLoop && "Perfect loop nest expected");
    HLLoop *SrcLoop = nullptr;
    // Loop is already in desired position
    if (Lp == DstLoop) {
      DstLoop = dyn_cast<HLLoop>(DstLoop->getFirstChild());
      continue;
    }
    for (auto &Lp1 : SavedLoops) {
      // getNestingLevel() asserts for disconnected loops. It is set
      // explicitly
      // for saved loops in the previous loop so we access it directly.
      if (Lp->getNestingLevel() == Lp1->NestingLevel) {
        SrcLoop = Lp1;
        break;
      }
    }
    assert(SrcLoop && "Input Loop is null");
    assert(DstLoop != SrcLoop && "Dst, Src loop cannot be equal");
    // Move properties from SrcLoop to DstLoop.
    *DstLoop = std::move(*SrcLoop);

    DstLoop = dyn_cast<HLLoop>(DstLoop->getFirstChild());
  }
}

namespace {

class LabelRemapVisitor final : public HLNodeVisitorBase {
  const HLNodeMapper &Mapper;

public:
  LabelRemapVisitor(const HLNodeMapper &Mapper) : Mapper(Mapper) {}

  void visit(HLGoto *Goto) {
    if (!Goto->isExternal()) {
      Goto->setTargetLabel(Mapper.getMapped(Goto->getTargetLabel()));
    }
  }

  void visit(HLNode *) {}
  void postVisit(HLNode *) {}
};
}

void HIRTransformUtils::remapLabelsRange(const HLNodeMapper &Mapper,
                                         HLNode *Begin, HLNode *End) {
  LabelRemapVisitor Visitor(Mapper);
  HLNodeUtils::visitRange(Visitor, Begin, End);
}
