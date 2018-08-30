//===--- HIRTransformUtils.cpp  -------------------------------------------===//
//
// Copyright (C) 2015-2018 Intel Corporation. All rights reserved.
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

#include "llvm/Transforms/Intel_LoopTransforms/Utils/HIRTransformUtils.h"
#include "llvm/ADT/SmallSet.h"
#include "llvm/Support/Debug.h"

#include "HIRUnroll.h"
#include "llvm/Analysis/Intel_LoopAnalysis/IR/HLNodeMapper.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/ForEach.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HIRInvalidationUtils.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HLNodeUtils.h"

#include "HIRLMMImpl.h"
#include "HIRLoopReversalImpl.h"

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
  HIRLoopReversal ReversalPass(InnermostLp->getHLNodeUtils().getHIRFramework(),
                               HDDA, HLS, HSRA);

  // Call HIRLoopReversal.isReversible(-)
  return ReversalPass.isReversible(
      InnermostLp,  // HLLoop*
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
  HIRLoopReversal ReversalPass(InnermostLp->getHLNodeUtils().getHIRFramework(),
                               HDDA, HLS, HSRA);

  // Call HIRLoopReversal.isReversible(-), expect the loop is reversible
  bool IsReversible = ReversalPass.isReversible(
      InnermostLp, // HLLoop*
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

  HIRLMM LMMPass(InnermostLp->getHLNodeUtils().getHIRFramework(), HDDA, HLS);
  return LMMPass.doLoopMemoryMotion(InnermostLp);
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

    TempInst = OrigLoop->getHLNodeUtils().createCopyInst(Ref, "tgu");
  }
  HLNodeUtils::insertBefore(const_cast<HLLoop *>(OrigLoop), TempInst);
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

HLLoop *HIRTransformUtils::createUnrollOrVecLoop(
    HLLoop *OrigLoop, unsigned UnrollOrVecFactor, uint64_t NewTripCount,
    const RegDDRef *NewTCRef, LoopOptReportBuilder &LORBuilder,
    OptimizationType OptTy) {
  HLLoop *NewLoop = OrigLoop->cloneEmpty();

  // Number of exits do not change due to vectorization
  if (OptTy != OptimizationType::Vectorizer) {
    NewLoop->setNumExits((OrigLoop->getNumExits() - 1) * UnrollOrVecFactor + 1);
  }

  OrigLoop->getHLNodeUtils().insertBefore(OrigLoop, NewLoop);

  // Update the loop upper bound.
  if (NewTripCount != 0) {
    uint64_t NewBound;

    // For vectorizer mode, upper bound needs to be multiplied by
    // UnrollOrVecFactor since it is used as the stride
    NewBound = (OptTy == OptimizationType::Vectorizer)
                   ? (NewTripCount * UnrollOrVecFactor)
                   : NewTripCount;

    // Subtract 1.
    NewBound = NewBound - 1;

    NewLoop->getUpperCanonExpr()->setConstant(NewBound);
  } else {

    // Create 't-1' as new UB.
    assert(NewTCRef && " New Ref is null.");
    RegDDRef *NewUBRef = NewTCRef->clone();

    // For vectorizer mode, upper bound needs to be multiplied by
    // UnrollOrVecFactor since it is used as the stride
    if (OptTy == OptimizationType::Vectorizer) {
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

    NewLoop->addLiveInTemp(NewTCRef->getSymbase());

    // Generate the Ztt.
    NewLoop->createZtt(false);

    // Update unrolled/vectorized loop's trip count estimate.
    NewLoop->setMaxTripCountEstimate(
        NewLoop->getMaxTripCountEstimate() / UnrollOrVecFactor,
        NewLoop->isMaxTripCountEstimateUsefulForDD());
  }

  // Set the code gen for modified region
  assert(NewLoop->getParentRegion() && " Loop does not have a parent region.");
  NewLoop->getParentRegion()->setGenCode();

  // Vectorization uses UnrollOrVecFactor as stride
  if (OptTy == OptimizationType::Vectorizer) {
    NewLoop->getStrideDDRef()->getSingleCanonExpr()->setConstant(
        UnrollOrVecFactor);
  }

  // NewLoop is the main loop now and hence, we want to associate all the opt
  // report with it.
  LORBuilder(*OrigLoop).moveOptReportTo(*NewLoop);
  if (OptTy == OptimizationType::Unroll) {
    LORBuilder(*NewLoop).addRemark(OptReportVerbosity::Low,
                                   "Loop has been unrolled by %d factor",
                                   UnrollOrVecFactor);
  } else if (OptTy == OptimizationType::UnrollAndJam) {
    LORBuilder(*NewLoop).addRemark(OptReportVerbosity::Low,
                                   "Loop has been unrolled and jammed by %d",
                                   UnrollOrVecFactor);
  } else {
    assert(OptTy == OptimizationType::Vectorizer &&
           "Invalid optimization type!");
    // Do nothing, Vectorizer will add remarks about vectorized loops
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

    OrigLoop->addLiveInTemp(NewTCRef->getSymbase());

    OrigLoop->createZtt(false);

    // Update remainder loop's trip count estimate.
    // TODO: can set useful for DD flag if loop is normalized.
    OrigLoop->setMaxTripCountEstimate(UnrollOrVecFactor - 1);
  }

  LLVM_DEBUG(dbgs() << "\n Remainder Loop \n");
  LLVM_DEBUG(OrigLoop->dump());
}

namespace {

class TempDefFinder final : public HLNodeVisitorBase {
  SmallSet<unsigned, 4> &TempSymbases;
  SmallVector<unsigned, 4> FoundTempDefs;

public:
  TempDefFinder(SmallSet<unsigned, 4> &TempSymbases)
      : TempSymbases(TempSymbases) {}

  void visit(const HLNode *Node) {}
  void postVisit(const HLNode *Node) {}

  void visit(const HLInst *Inst);

  SmallVector<unsigned, 4> getFoundTempDefs() const { return FoundTempDefs; }
};

void TempDefFinder::visit(const HLInst *Inst) {
  auto LvalRef = Inst->getLvalDDRef();

  if (!LvalRef || !LvalRef->isTerminalRef()) {
    return;
  }

  unsigned TempDefSB = LvalRef->getSymbase();

  if (TempSymbases.count(TempDefSB)) {
    FoundTempDefs.push_back(TempDefSB);
  }
}

} // namespace

void HIRTransformUtils::addCloningInducedLiveouts(HLLoop *LiveoutLoop,
                                                  const HLLoop *OrigLoop) {
  // Creation of a new cloned loop (remainder loop, for example) can result in
  // additional liveouts from the lexically first loop. Consider this example
  // where t1 is livein but not liveout of the loop- DO i1
  //   t1 = t1 + ...
  // END DO
  //
  // After creating main and remainder loop, t1 becomes liveout of main loop.
  //
  // DO i1  << main loop
  //   t1 = t1 + ...
  // END DO
  //
  // DO i2  << remainder loop
  //   t1 = t1 + ...
  // END DO

  if (!OrigLoop) {
    OrigLoop = LiveoutLoop;
  }

  SmallSet<unsigned, 4> LiveoutCandidates;

  // Collect liveins which are not liveout of the loop.
  for (auto It = OrigLoop->live_in_begin(), E = OrigLoop->live_in_end();
       It != E; ++It) {
    unsigned Symbase = *It;

    if (!OrigLoop->isLiveOut(Symbase)) {
      LiveoutCandidates.insert(Symbase);
    }
  }

  if (LiveoutCandidates.empty()) {
    return;
  }

  TempDefFinder TDF(LiveoutCandidates);

  HLNodeUtils::visitRange(TDF, OrigLoop->child_begin(), OrigLoop->child_end());

  for (unsigned LiveoutSB : TDF.getFoundTempDefs()) {
    LiveoutLoop->addLiveOutTemp(LiveoutSB);
  }
}

HLLoop *HIRTransformUtils::setupMainAndRemainderLoops(
    HLLoop *OrigLoop, unsigned UnrollOrVecFactor, bool &NeedRemainderLoop,
    LoopOptReportBuilder &LORBuilder, OptimizationType OptTy) {
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
  HLLoop *MainLoop = createUnrollOrVecLoop(
      OrigLoop, UnrollOrVecFactor, NewTripCount, NewTCRef, LORBuilder, OptTy);

  // Update the OrigLoop to remainder loop by setting bounds appropriately if
  // remainder loop is needed.
  if (NeedRemainderLoop) {
    processRemainderLoop(OrigLoop, UnrollOrVecFactor, NewTripCount, NewTCRef);
    addCloningInducedLiveouts(MainLoop, OrigLoop);

    // Since OrigLoop became a remainder and will be lexicographicaly
    // second to MainLoop, we move all the next siblings back there.
    LORBuilder(*MainLoop).moveSiblingsTo(*OrigLoop);
    if (OptTy == OptimizationType::Vectorizer) {
      LORBuilder(*OrigLoop).addOrigin("Remainder loop for vectorization");
    } else if (OptTy == OptimizationType::Unroll) {
      LORBuilder(*OrigLoop).addOrigin("Remainder loop for partial unrolling");
    } else {
      assert(OptTy == OptimizationType::UnrollAndJam &&
             "Invalid optimization type!");
      LORBuilder(*OrigLoop).addOrigin("Remainder loop for unroll-and-jam");
    }
  }

  // Mark parent for invalidation
  HIRInvalidationUtils::invalidateParentLoopBodyOrRegion(OrigLoop);

  return MainLoop;
}

namespace {

class LabelRemapVisitor final : public HLNodeVisitorBase {
  const HLNodeMapper &Mapper;

public:
  LabelRemapVisitor(const HLNodeMapper &Mapper) : Mapper(Mapper) {}

  void visit(HLGoto *Goto) {
    if (Goto->isExternal()) {
      return;
    }

    HLLabel *TargetLabelClone = Mapper.getMapped(Goto->getTargetLabel());

    // If target label was cloned - remap goto to the cloned label.
    if (TargetLabelClone) {
      Goto->setTargetLabel(TargetLabelClone);
    }
  }

  void visit(HLNode *) {}
  void postVisit(HLNode *) {}
};
} // namespace

void HIRTransformUtils::remapLabelsRange(const HLNodeMapper &Mapper,
                                         HLNode *Begin, HLNode *End) {
  LabelRemapVisitor Visitor(Mapper);
  HLNodeUtils::visitRange(Visitor, Begin, End);
}

namespace {
struct UpdateDDRefForLoopPermutation final : public HLNodeVisitorBase {

  // Smallest value of OutmostNestingLevel & InnermostNestingLevel is 1.
  unsigned OutmostNestingLevel;
  unsigned InnermostNestingLevel;
  unsigned *NewLoopLevels;
  void updateDDRef(HLDDNode *Node, unsigned InnermostNestingLevel,
                   unsigned OutmostNestingLevel, unsigned *NewLoopLevels);
  void updateCE(CanonExpr *CE, unsigned InnermostNestingLevel,
                unsigned OutmostNestingLevel, unsigned *NewLoopLevels);

  UpdateDDRefForLoopPermutation(unsigned OutmostNestingLevel,
                                unsigned InnermostNestingLevel,
                                unsigned *NewLoopLevels)
      : OutmostNestingLevel(OutmostNestingLevel),
        InnermostNestingLevel(InnermostNestingLevel),
        NewLoopLevels(NewLoopLevels) {}

  void visit(const HLNode *Node) {}
  void visit(HLDDNode *Node) {
    updateDDRef(Node, InnermostNestingLevel, OutmostNestingLevel,
                NewLoopLevels);
  }
  void postVisit(HLNode *) {}
};
} // namespace

static void addLiveInToPermutedLoopnest(unsigned Symbase, HLLoop *InnermostLoop,
                                        HLLoop *OutermostLoop) {
  // Livein information of OutermostLoop remains unchanged.
  for (auto *Lp = InnermostLoop; Lp != OutermostLoop;
       Lp = Lp->getParentLoop()) {
    Lp->addLiveInTemp(Symbase);
  }
}

static void updatePermutedLoopnestLiveIns(HLLoop *InnermostLoop,
                                          HLLoop *OutermostLoop) {
  // Update livein information based on the new loop bounds after permutation.

  for (auto *Lp = InnermostLoop; Lp != OutermostLoop;
       Lp = Lp->getParentLoop()) {

    // Add each temp in bound ddrefs as livein to all the parent loops involved
    // in the permutation.
    for (auto RefIt = Lp->ddref_begin(), E = Lp->ddref_end(); RefIt != E;
         ++RefIt) {
      auto *Ref = *RefIt;

      if (Ref->isSelfBlob()) {
        addLiveInToPermutedLoopnest(Ref->getSymbase(), Lp, OutermostLoop);
      } else {
        for (auto BlobIt = Ref->blob_cbegin(), EB = Ref->blob_cend();
             BlobIt != EB; ++BlobIt) {
          addLiveInToPermutedLoopnest((*BlobIt)->getSymbase(), Lp,
                                      OutermostLoop);
        }
      }
    }
  }
}

void HIRTransformUtils::permuteLoopNests(
    HLLoop *OutermostLoop,
    const SmallVectorImpl<const HLLoop *> &LoopPermutation,
    unsigned InnermostNestingLevel) {

  SmallVector<HLLoop *, MaxLoopNestLevel> SavedLoops;

  // isPerfectLoopNest() allows Prehdr/PostExit
  // in outermost loop. If not extracted, it will lead to errors
  // in this case:
  // do i2=1,n   (before interchange)
  //    do i3 =1,6
  //    end
  // end
  //  a[i1] = 2 (PostExit)
  //
  if (OutermostLoop != LoopPermutation.front()) {
    OutermostLoop->extractPreheaderAndPostexit();
  }

  SmallVector<HLLoop *, MaxLoopNestLevel> OrigLoops;
  HLLoop *InnermostLoop = nullptr;

  for (auto &Lp : LoopPermutation) {
    HLLoop *LoopCopy = Lp->cloneEmpty();
    LoopCopy->setNestingLevel(Lp->getNestingLevel());
    SavedLoops.push_back(LoopCopy);

    // Preparation for sorting
    // TODO: remove 'const' from LoopPermutation.
    HLLoop *NonConstLp = const_cast<HLLoop *>(Lp);
    OrigLoops.push_back(NonConstLp);

    if (Lp->isInnermost()) {
      InnermostLoop = NonConstLp;
    }
  }

  // Sort by loop nesting level from the LoopPermutation
  // to get the current loopnest.
  // OrigLoopnests will be used to be the destination of the
  // loop permutation. This way non-perfect loopnest can be covered.
  std::sort(OrigLoops.begin(), OrigLoops.end(),
            [](HLLoop *A, HLLoop *B) -> bool {
              return A->getNestingLevel() < B->getNestingLevel();
            });

  // Range-based iteration is purposely avoided to visit
  // all LoopPermutation, SavedLoops and OrigLoops in sync.
  // OrigLoops are rewritten.
  for (int I = 0, Size = LoopPermutation.size(); I < Size; I++) {
    assert(OrigLoops[I] && "LoopPermutation logic is wrong");
    if (LoopPermutation[I] == OrigLoops[I]) {
      continue;
    }
    assert(OrigLoops[I] != SavedLoops[I] && "Dst, Src loop cannot be equal");
    *(OrigLoops[I]) = std::move(*(SavedLoops[I]));
  }

  // Update Loop Body
  unsigned NewLoopLevels[MaxLoopNestLevel];
  unsigned Idx = 0;

  for (auto &I : LoopPermutation) {
    NewLoopLevels[Idx++] = I->getNestingLevel();
  }

  unsigned OutmostNestingLevel = OutermostLoop->getNestingLevel();
  UpdateDDRefForLoopPermutation UpdateDDRef(
      OutmostNestingLevel, InnermostNestingLevel, &NewLoopLevels[0]);

  HLNodeUtils::visit(UpdateDDRef, OutermostLoop);

  updatePermutedLoopnestLiveIns(InnermostLoop, OutermostLoop);
}

void UpdateDDRefForLoopPermutation::updateDDRef(HLDDNode *Node,
                                                unsigned InnermostNestingLevel,
                                                unsigned OutmostNestingLevel,
                                                unsigned *NewLoopLevels) {

  for (auto I = Node->ddref_begin(), E = Node->ddref_end(); I != E; ++I) {
    RegDDRef *RegRef = *I;

    for (auto Iter = RegRef->canon_begin(), E2 = RegRef->canon_end();
         Iter != E2; ++Iter) {
      CanonExpr *CE = *Iter;
      updateCE(CE, InnermostNestingLevel, OutmostNestingLevel, NewLoopLevels);
    }
    if (RegRef->hasGEPInfo()) {
      updateCE(RegRef->getBaseCE(), InnermostNestingLevel, OutmostNestingLevel,
               NewLoopLevels);
    }
  }
}

void UpdateDDRefForLoopPermutation::updateCE(CanonExpr *CE,
                                             unsigned InnermostNestingLevel,
                                             unsigned OutmostNestingLevel,
                                             unsigned *NewLoopLevels) {

  if (!(CE->hasIV())) {
    return;
  }

  // Save Coffs
  int64_t ConstCoeff[MaxLoopNestLevel];
  unsigned BlobCoeff[MaxLoopNestLevel];

  unsigned II = 0;
  for (II = OutmostNestingLevel; II <= InnermostNestingLevel; ++II) {
    ConstCoeff[II - 1] = 0;
    BlobCoeff[II - 1] = 0;
  }
  for (auto CurIVPair = CE->iv_begin(), E2 = CE->iv_end(); CurIVPair != E2;
       ++CurIVPair) {
    II = CE->getLevel(CurIVPair);
    ConstCoeff[II - 1] = CE->getIVConstCoeff(CurIVPair);
    BlobCoeff[II - 1] = CE->getIVBlobCoeff(CurIVPair);
  }

  // For each level, replace coeffs with the new one
  // Indexes to local arrays here start with 0
  // Levels used start with at least 1
  for (unsigned OL = OutmostNestingLevel; OL <= InnermostNestingLevel; ++OL) {
    unsigned NL = NewLoopLevels[OL - OutmostNestingLevel];
    if (OL == NL || (ConstCoeff[OL - 1] == 0 && ConstCoeff[NL - 1] == 0)) {
      continue;
    }
    CE->removeIV(OL);
    CE->addIV(OL, BlobCoeff[NL - 1], ConstCoeff[NL - 1]);
  }
}

void HIRTransformUtils::stripmine(HLLoop *FirstLoop, HLLoop *LastLoop,
                                  unsigned StripmineSize) {
  uint64_t TripCount;
  bool IsConstTrip = FirstLoop->isConstTripLoop(&TripCount);

  // Caller should call canStripmine before
  assert(!(IsConstTrip && (TripCount <= StripmineSize)) &&
         "Caller should call canStripmine() first");

  HLNodeUtils *HNU = &(FirstLoop->getHLNodeUtils());
  unsigned Level = FirstLoop->getNestingLevel();

  HLLoop *NewLoop = FirstLoop->cloneEmpty();

  HLNodeUtils::insertBefore(FirstLoop, NewLoop);

  HLNodeUtils::moveAsLastChildren(NewLoop, FirstLoop->getIterator(),
                                  std::next(LastLoop->getIterator()));

  // Move Preheader of Firstloop to NewLoop
  // Move Postexit  of Lastloop to NewLoop
  // This is sufficent for current processing, subject to new requirements
  // when blocking is extended for multiple loop nests

  HNU->moveAsFirstPreheaderNodes(NewLoop, FirstLoop->pre_begin(),
                                 FirstLoop->pre_end());

  HNU->moveAsFirstPostexitNodes(NewLoop, LastLoop->post_begin(),
                                LastLoop->post_end());

  for (auto It = NewLoop->child_begin(), E = NewLoop->child_end(); It != E;
       ++It) {
    HLNode *NodeI = &(*It);
    HLLoop *Lp = dyn_cast<HLLoop>(NodeI);
    if (Lp) {
      HIRTransformUtils::updateStripminedLoopCE(Lp);
    }
  }

  RegDDRef *UBRef = NewLoop->getUpperDDRef();
  // Store original UB in MinOpRef1.
  RegDDRef *MinOpRef1 = UBRef->clone();

  CanonExpr *UBCE = UBRef->getSingleCanonExpr();

  //  UB / StripmineSize: (N-1) / 64
  UBCE->divide(StripmineSize);
  UBCE->simplify(true);

  RegDDRef *InnerLBRef =
      UBRef->getDDRefUtils().createRegDDRef(GenericRvalSymbase);

  CanonExpr *LBCE = NewLoop->getLowerCanonExpr();
  CanonExpr *InnerLoopLBCE = UBRef->getCanonExprUtils().createExtCanonExpr(
      LBCE->getSrcType(), LBCE->getDestType(), LBCE->isSExt());
  //  64*i1
  InnerLoopLBCE->setIVConstCoeff(Level, StripmineSize);
  InnerLBRef->setSingleCanonExpr(InnerLoopLBCE);

  auto *InnerUBRef = InnerLBRef->clone();

  // If trip count is evenly divisble by stripmine factor, the trip count of
  // stripmined loop will be the same as the factor.
  bool MinInstRequired = (!IsConstTrip || (TripCount % StripmineSize != 0));
  unsigned MinBlobSymbase = InvalidSymbase;

  if (MinInstRequired) {
    // -StripmineSize *i1 + original UB
    // Need to convert from unsign to sign first
    int64_t Coeff = StripmineSize;

    MinOpRef1->getSingleCanonExpr()->addIV(Level, InvalidBlobIndex, -Coeff,
                                           true);
    MinOpRef1->setSymbase(GenericRvalSymbase);

    // StripmineSize-1
    RegDDRef *MinOpRef2 = UBRef->getDDRefUtils().createConstDDRef(
        MinOpRef1->getDestType(), StripmineSize - 1);

    HLInst *MinInst = HNU->createMin(MinOpRef1, MinOpRef2);
    HLNodeUtils::insertAsFirstChild(NewLoop, MinInst);

    RegDDRef *BlobRef = MinInst->getLvalDDRef();
    unsigned MinBlobIndex = BlobRef->getSingleCanonExpr()->getSingleBlobIndex();
    MinBlobSymbase = BlobRef->getSymbase();

    // 64*i1 + %min
    CanonExpr *UBRefCE = InnerUBRef->getSingleCanonExpr();
    UBRefCE->setBlobCoeff(MinBlobIndex, 1);
    UBRefCE->setDefinedAtLevel(Level);

    InnerUBRef->addBlobDDRef(MinBlobIndex, Level);
  } else {
    InnerUBRef->getSingleCanonExpr()->setConstant(StripmineSize - 1);
  }

  // Normalize code will set linear at level
  for (auto It = NewLoop->child_begin(), E = NewLoop->child_end(); It != E;
       ++It) {

    HLLoop *Lp = dyn_cast<HLLoop>(&(*It));
    if (!Lp) {
      continue;
    }
    // Set Loop Bounds
    // Use original refs for the last loop.
    Lp->setLowerDDRef((Lp == LastLoop) ? InnerLBRef : InnerLBRef->clone());
    Lp->setUpperDDRef((Lp == LastLoop) ? InnerUBRef : InnerUBRef->clone());

    // Copy LiveInOut to enclosing loop
    for (auto LiveIn : make_range(Lp->live_in_begin(), Lp->live_in_end())) {
      NewLoop->addLiveInTemp(LiveIn);
    }

    for (auto LiveOut : make_range(Lp->live_out_begin(), Lp->live_out_end())) {
      NewLoop->addLiveOutTemp(LiveOut);
    }

    if (MinInstRequired) {
      Lp->addLiveInTemp(MinBlobSymbase);
      Lp->setMaxTripCountEstimate(StripmineSize, true);
    }

    // Normalize
    bool Result = Lp->normalize();
    assert(Result && "Not expecting cannot be normalized");
    (void)Result;
  }
}

void HIRTransformUtils::updateStripminedLoopCE(HLLoop *Loop) {

  // Update Inner loop CannonExpr  i1 -> i2
  unsigned ParentLevel = Loop->getNestingLevel() - 1;
  //   One more nest created. IV level needs to be increased
  //   e.g. from  3*i3+ 2*i2 to  3*i4+ 2*i3

  ForEach<HLDDNode>::visitRange(
      Loop->child_begin(), Loop->child_end(), [ParentLevel](HLDDNode *Node) {
        for (RegDDRef *Ref :
             llvm::make_range(Node->ddref_begin(), Node->ddref_end())) {
          for (CanonExpr *CE :
               llvm::make_range(Ref->canon_begin(), Ref->canon_end())) {
            unsigned BlobIndex;
            int64_t Coeff;
            for (unsigned Lvl = MaxLoopNestLevel - 1; Lvl >= ParentLevel;
                 --Lvl) {
              CE->getIVCoeff(Lvl, &BlobIndex, &Coeff);
              if (Coeff == 0) {
                continue;
              }
              CE->removeIV(Lvl);
              CE->setIVCoeff(Lvl + 1, BlobIndex, Coeff);
            }
          }
        }
      });
}

void HIRTransformUtils::completeUnroll(HLLoop *Loop) {
  assert(Loop->isConstTripLoop() &&
         "Cannot unroll non-constant trip count loop!");
  unroll::completeUnrollLoop(Loop);
}
