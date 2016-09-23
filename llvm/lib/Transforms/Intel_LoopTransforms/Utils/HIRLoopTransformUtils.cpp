//===--- HIRLoopTransformUtils.cpp  ---------------------------*- C++ -*---===//
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
// This file implements HIRLoopTransformUtils class.
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/Intel_LoopTransforms/Utils/HIRLoopTransformUtils.h"
#include "llvm/Analysis/Intel_LoopAnalysis/HIRFramework.h"
#include "llvm/Support/Debug.h"
#include "llvm/Transforms/Intel_LoopTransforms/HIRLoopReversal.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/HIRInvalidationUtils.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/HLNodeUtils.h"

#define DEBUG_TYPE "hir-looptransform-utils"

using namespace llvm;
using namespace llvm::loopopt;
using namespace llvm::loopopt::reversal;

bool HIRLoopTransformUtils::isHIRLoopReversible(
    HLLoop *Lp,                     // INPUT + OUTPUT: a given loop
    HIRDDAnalysis &HDDA,            // INPUT: HIR DDAnalysis
    HIRSafeReductionAnalysis &HSRA, // INPUT: HIRSafeReductionAnalysis
    HIRLoopStatistics &HLS,         // INPUT: Existing HIRLoopStatitics
    bool DoProfitTest               // INPUT: Control Profit Tests
    ) {
  // 0.Sanity Checks
  assert(Lp && "HLLoop* can't be a nullptr\n");

  if (!Lp->isInnermost()) {
    DEBUG(
        dbgs() << "HIR LoopReversal can only work with an inner-most loop\n ";);
    return false;
  }

  // 1.Create an HIRLoopReversal object on stack
  HIRLoopReversal ReversalPass;

  // 2.Call HIRLoopReversal.isReversible(-)
  return ReversalPass.isReversible(
      Lp,           // HLLoop*
      HDDA,         // Existing DDAnalysis
      HSRA,         // Existing SafeReductionAnalysis
      HLS,          // Existing HIRLoopStatistics Analysis
      DoProfitTest, // Control Profit Test
      true,         // Always do Legal Tests
      false         // OFF Short-circuit action (doing analysis now)
      );
}

void HIRLoopTransformUtils::doHIRLoopReversal(
    HLLoop *Lp,                     // INPUT + OUTPUT: a given loop
    HIRDDAnalysis &HDDA,            // INPUT: HIR DDAnalysis
    HIRSafeReductionAnalysis &HSRA, // INPUT: HIRSafeReductionAnalysis
    HIRLoopStatistics &HLS          // INPUT: Existing HIRLoopStatitics
    ) {
  // 0.Sanity Checks
  assert(Lp && "HLLoop* can't be a nullptr\n");

  if (!Lp->isInnermost()) {
    DEBUG(
        dbgs() << "HIR LoopReversal can only work with an inner-most loop\n ";);
  }

  // 1.Create an HIRLoopReversal object on stack
  HIRLoopReversal ReversalPass;

  // 2.Call HIRLoopReversal.isReversible(-), expect the loop is reversible
  bool IsReversible = ReversalPass.isReversible(
      Lp,    // HLLoop*
      HDDA,  // Existing DDAnalysis
      HSRA,  // Existing SafeReductionAnalysis
      HLS,   // Existing HIRLoopStatistics Analysis
      false, // Ignore Profit Tests
      false, // Assert on legality
      true // ON Short-circuit action (expect the analysis has been done in the
           // previous API call)
      );

  // 3. Reverse the Loop
  assert(IsReversible && "Expect the loop is reversible\n");
  (void)IsReversible;

  ReversalPass.doHIRReversalTransform(Lp);
}

bool HIRLoopTransformUtils::isRemainderLoopNeeded(HLLoop *OrigLoop,
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
  if (TripCE->isSignedDiv() && (TripCE->getDenominator() != 1)) {
    // Create DDRef for Unroll Factor.
    RegDDRef *UFDD =
        DDRefUtils::createConstDDRef(Ref->getDestType(), UnrollOrVecFactor);
    TempInst = HLNodeUtils::createUDiv(Ref, UFDD, "tgu");
  } else {
    SmallVector<const RegDDRef *, 3> AuxRefs = {OrigLoop->getStrideDDRef(),
                                                OrigLoop->getLowerDDRef(),
                                                OrigLoop->getUpperDDRef()};

    // Use the same canon expr to generate the division.
    TripCE->divide(UnrollOrVecFactor, true);

    Ref->setSymbase(getHIRFramework()->getNewSymbase());

    Ref->makeConsistent(&AuxRefs, OrigLoop->getNestingLevel() - 1);

    TempInst = HLNodeUtils::createCopyInst(Ref, "tgu");
  }
  HLNodeUtils::insertBefore(const_cast<HLLoop *>(OrigLoop), TempInst);
  *NewTCRef = TempInst->getLvalDDRef();

  return true;
}

void HIRLoopTransformUtils::updateBoundDDRef(RegDDRef *BoundRef,
                                             unsigned BlobIndex,
                                             unsigned DefLevel) {
  // Overwrite symbase to a newly created one to avoid unnecessary DD edges.
  BoundRef->setSymbase(getHIRFramework()->getNewSymbase());

  // Add blob DDRef for the temp in UB.
  BoundRef->addBlobDDRef(BlobIndex, DefLevel);
  BoundRef->updateDefLevel();
}

HLLoop *HIRLoopTransformUtils::createUnrollOrVecLoop(HLLoop *OrigLoop,
                                                     unsigned UnrollOrVecFactor,
                                                     uint64_t NewTripCount,
                                                     const RegDDRef *NewTCRef,
                                                     bool VecMode) {
  HLLoop *NewLoop = OrigLoop->cloneEmptyLoop();

  // Number of exits do not change due to vectorization
  if (!VecMode) {
    NewLoop->setNumExits((OrigLoop->getNumExits() - 1) * UnrollOrVecFactor + 1);
  }

  HLNodeUtils::insertBefore(OrigLoop, NewLoop);

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
      NewUBRef->getSingleCanonExpr()->multiplyByConstant(UnrollOrVecFactor);
    }

    // Subtract 1.
    NewUBRef->getSingleCanonExpr()->addConstant(-1);

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

void HIRLoopTransformUtils::processRemainderLoop(HLLoop *OrigLoop,
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
    NewLBRef->getSingleCanonExpr()->multiplyByConstant(UnrollOrVecFactor);

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

HLLoop *HIRLoopTransformUtils::setupMainAndRemainderLoops(
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
