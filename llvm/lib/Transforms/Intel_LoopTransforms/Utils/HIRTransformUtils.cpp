//===--- HIRTransformUtils.cpp  -------------------------------------------===//
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
// This file implements HIRTransformUtils class.
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/Intel_LoopTransforms/Utils/HIRTransformUtils.h"

#include "llvm/Analysis/Intel_LoopAnalysis/IR/HLNodeMapper.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/DDRefGatherer.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/ForEach.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HIRInvalidationUtils.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HLNodeIterator.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HLNodeUtils.h"

#include "HIRArrayScalarization.h"
#include "HIRDeadStoreElimination.h"
#include "HIRLMM.h"
#include "HIRLoopReversal.h"
#include "HIROptVarPredicate.h"
#include "HIRUnroll.h"

#include "llvm/ADT/SmallSet.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Support/Debug.h"

#define DEBUG_TYPE "hir-transform-utils"

using namespace llvm;
using namespace llvm::loopopt;
using namespace llvm::loopopt::reversal;
using namespace llvm::loopopt::lmm;
using namespace llvm::loopopt::dse;
using namespace llvm::loopopt::arrayscalarization;

namespace {

/// This function creates HLIf using predicates that are stored in RuntimeChecks
/// vector. This HLIf has following form:
///
/// if (pred1 && pred2 && ... && predN) {
/// } else {
///   tgu = 0;
/// }
/// // Remainder loop
HLIf *createRuntimeChecks(
    SmallVectorImpl<std::tuple<HLPredicate, RegDDRef *, RegDDRef *>>
        *RuntimeChecks,
    HLLoop *RemainderLoop, RegDDRef **NewTCRef,
    const HIRTransformUtils::ProfInfo *Prof) {
  assert(RuntimeChecks && !RuntimeChecks->empty() &&
         "At least one runtime check must be passed.");
  HLNodeUtils &HNU = RemainderLoop->getHLNodeUtils();

  HLIf *If = nullptr;
  for (auto &Check : *RuntimeChecks)
    if (!If)
      If = HNU.createHLIf(std::get<0>(Check), std::get<1>(Check),
                          std::get<2>(Check));
    else
      If->addPredicate(std::get<0>(Check), std::get<1>(Check),
                       std::get<2>(Check));
  if (Prof) {
    If->setProfileData(Prof->TrueWeight, Prof->FalseWeight);
  }

  // In case if RT-check failed, remainder loop has to start from the beginning.
  RegDDRef *Ref = RemainderLoop->getUpperDDRef();
  RegDDRef *LB = Ref->getDDRefUtils().createNullDDRef(Ref->getDestType());
  HLInst *TempInst = RemainderLoop->getHLNodeUtils().createCopyInst(LB, "tgu");
  HLNodeUtils::insertAsLastElseChild(If, TempInst);
  *NewTCRef = TempInst->getLvalDDRef();

  HLNodeUtils::insertBefore(RemainderLoop, If);

  return If;
}

} // end of anonymous namespace

bool HIRTransformUtils::isLoopReversible(
    HLLoop *InnermostLp, HIRDDAnalysis &HDDA, HIRSafeReductionAnalysis &HSRA,
    HIRLoopStatistics &HLS, bool DoProfitTest, bool SkipLoopBoundChecks) {
  assert(InnermostLp && "HLLoop* can't be a nullptr\n");
  assert(InnermostLp->isInnermost() &&
         "HIR LoopReversal can only work with an inner-most loop\n");

  // Create an HIRLoopReversal object on stack
  HIRLoopReversal ReversalPass(InnermostLp->getHLNodeUtils().getHIRFramework(),
                               HDDA, HLS, HSRA);

  // Call HIRLoopReversal.isReversible(-)
  return ReversalPass.isReversible(InnermostLp, DoProfitTest,
                                   true, // Always do Legal Tests
                                   SkipLoopBoundChecks);
}

void HIRTransformUtils::doLoopReversal(HLLoop *InnermostLp, HIRDDAnalysis &HDDA,
                                       HIRSafeReductionAnalysis &HSRA,
                                       HIRLoopStatistics &HLS) {
  assert(InnermostLp && "HLLoop* can't be a nullptr\n");
  assert(InnermostLp->isInnermost() &&
         "HIR LoopReversal can only work with an inner-most loop\n");

  // Create an HIRLoopReversal object on stack
  HIRLoopReversal ReversalPass(InnermostLp->getHLNodeUtils().getHIRFramework(),
                               HDDA, HLS, HSRA);

  // This check is required for the transformation because it collects all the
  // CEs which need to be transformed.
  // TODO: Fix this hidden state mutability.
  bool IsReversible = ReversalPass.isReversible(
      InnermostLp,
      false,  // Skip profit tests
      false,  // Assert that loop is legal
      false); // Don't care flag, loop bounds are always checked in legality
              // assertion mode.

  assert(IsReversible && "Expect the loop is reversible\n");
  (void)IsReversible;

  ReversalPass.doHIRReversalTransform(InnermostLp);
}

bool HIRTransformUtils::isLoopInvariant(const RegDDRef *MemRef,
                                        const HLLoop *Loop, HIRDDAnalysis &HDDA,
                                        HIRLoopStatistics &HLS,
                                        FieldModRefResult *FieldModRef,
                                        bool IgnoreIVs) {
  assert(MemRef && "Memref is null!");
  assert(MemRef->isMemRef() && "Ref is not a memref!");
  assert(Loop && "Loop is null!");
  assert(HLNodeUtils::contains(Loop, MemRef->getHLDDNode()) &&
         "MemRef expected to be inside Loop!");

  HIRLMM LMMPass(Loop->getHLNodeUtils().getHIRFramework(), HDDA, HLS, nullptr,
                 FieldModRef);
  return LMMPass.isLoopInvariant(MemRef, Loop, IgnoreIVs);
}

bool HIRTransformUtils::isRemainderLoopNeeded(HLLoop *OrigLoop,
                                              unsigned UnrollOrVecFactor,
                                              uint64_t *NewTripCountP,
                                              RegDDRef **NewTCRef,
                                              HLIf *RuntimeCheck) {

  uint64_t TripCount;

  if (OrigLoop->isConstTripLoop(&TripCount)) {

    uint64_t NewTripCount = TripCount / UnrollOrVecFactor;
    *NewTripCountP = NewTripCount;
    if (RuntimeCheck) {
      assert(*NewTCRef && "Trip count must be assigned in RT check.");
      RegDDRef *UpperRef = OrigLoop->getUpperDDRef();
      RegDDRef *LowerRef = UpperRef->getDDRefUtils().createConstDDRef(
          UpperRef->getDestType(), NewTripCount);
      HLInst *TempInst = OrigLoop->getHLNodeUtils().createCopyInst(
          LowerRef, "tgu", (*NewTCRef)->clone());
      HLNodeUtils::insertAsLastThenChild(RuntimeCheck, TempInst);
      return true;
    }

    // Return true if UnrollOrVecFactor does not evenly divide TripCount.
    return ((NewTripCount * UnrollOrVecFactor) != TripCount);
  }

  RegDDRef *Ref = OrigLoop->getTripCountDDRef();
  // Process for non-const trip loop.
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
    if (*NewTCRef)
      TempInst = OrigLoop->getHLNodeUtils().createUDiv(Ref, UFDD, "tgu",
                                                       (*NewTCRef)->clone());
    else
      TempInst = OrigLoop->getHLNodeUtils().createUDiv(Ref, UFDD, "tgu");
  } else {
    SmallVector<const RegDDRef *, 3> AuxRefs = {OrigLoop->getStrideDDRef(),
                                                OrigLoop->getLowerDDRef(),
                                                OrigLoop->getUpperDDRef()};

    // Use the same canon expr to generate the division.
    TripCE->divide(UnrollOrVecFactor);
    TripCE->simplify(true, true);

    Ref->setSymbase(Ref->getDDRefUtils().getNewSymbase());

    Ref->makeConsistent(AuxRefs, OrigLoop->getNestingLevel() - 1);

    if (*NewTCRef)
      TempInst = OrigLoop->getHLNodeUtils().createCopyInst(
          Ref, "tgu", (*NewTCRef)->clone());
    else
      TempInst = OrigLoop->getHLNodeUtils().createCopyInst(Ref, "tgu");
  }

  if (RuntimeCheck)
    HLNodeUtils::insertAsLastThenChild(RuntimeCheck, TempInst);
  else
    HLNodeUtils::insertBefore(const_cast<HLLoop *>(OrigLoop), TempInst);
  *NewTCRef = TempInst->getLvalDDRef();

  return true;
}

void HIRTransformUtils::updateBoundDDRef(RegDDRef *BoundRef, unsigned BlobIndex,
                                         unsigned DefLevel) {
  BoundRef->setSymbase(GenericRvalSymbase);

  // Add blob DDRef for the temp in UB.
  BoundRef->addBlobDDRef(BlobIndex, DefLevel);
  BoundRef->updateDefLevel();
}

// Divide TrueWeight with Denom and maintain Quotient and Rem
static void getFactoredWeights(HIRTransformUtils::ProfInfo *Prof,
                               uint64_t Denom) {
  if (!Prof)
    return;

  assert(Denom); // avoid divide by zero

  APInt Weight(64, Prof->TrueWeight);
  APInt AQuotient(64, 0);
  APInt::udivrem(Weight, Denom, AQuotient, Prof->Remainder);
  Prof->Quotient = Prof->TrueWeight == 0 ? 0
                                         : std::max(AQuotient.getLimitedValue(),
                                                    static_cast<uint64_t>(1));

  if (Prof->Remainder == 0 && Prof->TrueWeight > 2) {
    // Heuristic:
    // We adjust 0 remainder to 1 to avoid the situation where
    // remainder loop is completely missed because of modulo.
    // Just leave a small branch_weights.
    // If the original weight is already too small as 2 or less,
    // adjusting 0 to 1 for remainder loop might not help.
    Prof->Remainder = 1;
  }
}

HLLoop *HIRTransformUtils::createUnrollOrVecLoop(
    HLLoop *OrigLoop, unsigned UnrollOrVecFactor, uint64_t NewTripCount,
    const RegDDRef *NewTCRef, LoopOptReportBuilder &LORBuilder,
    OptimizationType OptTy, HLIf *RuntimeCheck, ProfInfo *Prof) {
  HLLoop *NewLoop = OrigLoop->cloneEmpty();

  // Number of exits do not change due to vectorization
  if (OptTy != OptimizationType::Vectorizer) {
    NewLoop->setNumExits((OrigLoop->getNumExits() - 1) * UnrollOrVecFactor + 1);
  }

  if (RuntimeCheck)
    // With runtime check assume that main loop is valid only when condition
    // of the rt check is true, thus insert it in the true branch.
    HLNodeUtils::insertAsLastThenChild(RuntimeCheck, NewLoop);
  else
    OrigLoop->getHLNodeUtils().insertBefore(OrigLoop, NewLoop);

  getFactoredWeights(Prof, UnrollOrVecFactor);

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
    if (Prof) {
      // Current heuristic is to assign
      // the same branch_weights to ztt with
      // that of branch_weights of backedge of new main loop's.
      // Can be changed as needed.
      // FalseWeight is not updated currently.
      NewLoop->getZtt()->setProfileData(Prof->Quotient, Prof->FalseWeight);
    }

    // Update unrolled/vectorized loop's trip count estimate.
    NewLoop->setMaxTripCountEstimate(
        NewLoop->getMaxTripCountEstimate() / UnrollOrVecFactor,
        NewLoop->isMaxTripCountEstimateUsefulForDD());

    NewLoop->dividePragmaBasedTripCount(UnrollOrVecFactor);
  }

  if (Prof) {
    // FalseWeight is not updated currently.
    NewLoop->setProfileData(Prof->Quotient, Prof->FalseWeight);
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
                                             const RegDDRef *NewTCRef,
                                             const bool HasRuntimeCheck,
                                             const ProfInfo *Prof) {
  // Mark Loop bounds as modified.
  HIRInvalidationUtils::invalidateBounds(OrigLoop);

  // Modify the LB of original loop.
  // For loops with runtime checks NewTCRef must be used as LB of the remainder
  // loop.
  if (!HasRuntimeCheck && NewTripCount) {
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
    if (Prof) {
      // Remainder loop's ztt (true branch)
      // gets the same weight as the remainder loop's trip count.
      // This is a heuristic.
      OrigLoop->getZtt()->setProfileData(Prof->Remainder, Prof->FalseWeight);
    }

    // Update remainder loop's trip count estimate.
    // TODO: can set useful for DD flag if loop is normalized.
    if (!HasRuntimeCheck) {
      OrigLoop->setMaxTripCountEstimate(UnrollOrVecFactor - 1);
      // New max trip count metadata can be applied.
      OrigLoop->setPragmaBasedMaximumTripCount(UnrollOrVecFactor - 1);
    }

    // Original min/avg trip count metadata does not apply to remainder loop.
    OrigLoop->removeLoopMetadata("llvm.loop.intel.loopcount_minimum");
    OrigLoop->removeLoopMetadata("llvm.loop.intel.loopcount_average");

    // Add unroll disabling pragma to non-const trip remainder loop so LLVM
    // unrolling pass doesn't unroll it.
    OrigLoop->markDoNotUnroll();
  }

  if (Prof) {
    // Set the remainder loop's profile data with Rem calcuated before.
    // Leaves FalseVal intact.
    // TODO: elaboration might be needed.
    OrigLoop->setProfileData(Prof->Remainder, Prof->FalseWeight);
  }

  LLVM_DEBUG(dbgs() << "\n Remainder Loop \n");
  LLVM_DEBUG(OrigLoop->dump());
}

HLLoop *HIRTransformUtils::setupPeelMainAndRemainderLoops(
    HLLoop *OrigLoop, unsigned UnrollOrVecFactor, bool &NeedRemainderLoop,
    LoopOptReportBuilder &LORBuilder, OptimizationType OptTy, HLLoop **PeelLoop,
    const RegDDRef *PeelArrayRef,
    SmallVectorImpl<std::tuple<HLPredicate, RegDDRef *, RegDDRef *>>
        *RuntimeChecks) {

  uint64_t TrueVal = 0;
  uint64_t FalseVal = 0;
  bool ProfExists = OrigLoop->extractProfileData(TrueVal, FalseVal);

  if (PeelArrayRef) {
    // Generic peeling of the loop to align acceses to the memref PeelArrayRef
    LLVM_DEBUG(dbgs() << "Generating peel loop!\n");
    HLLoop *PeelLp =
        OrigLoop->generatePeelLoop(PeelArrayRef, UnrollOrVecFactor);
    if (!PeelLp) {
      assert(false && "Could not generate peel loop.");
      // Initiate bail-out for prod build
      return nullptr;
    }
    if (PeelLoop)
      *PeelLoop = PeelLp;

    // After peeling a new ZTT was created for the main loop, extract it and add
    // outside the loop.
    OrigLoop->extractZtt();
    // TODO: Profiling info for peel loop?
  } else {
    // Extract Ztt and add it outside the loop.
    OrigLoop->extractZtt();

    // Extract preheader and postexit.
    OrigLoop->extractPreheaderAndPostexit();
  }

  HLIf *RuntimeCheck = nullptr;
  RegDDRef *NewTCRef = nullptr;
  // Branch weights are initialized here and
  // updated in createUnrollOrVecLoop() by UF/VF.
  ProfInfo Prof(TrueVal, FalseVal);
  if (RuntimeChecks && !RuntimeChecks->empty())
    RuntimeCheck = createRuntimeChecks(RuntimeChecks, OrigLoop, &NewTCRef,
                                       ProfExists ? &Prof : nullptr);

  // Create UB instruction before the loop 't = (Orig UB)/(UnrollOrVecFactor)'
  // for non-constant trip loops. For const trip loops calculate the bound.
  uint64_t NewTripCount = 0;
  NeedRemainderLoop = isRemainderLoopNeeded(
      OrigLoop, UnrollOrVecFactor, &NewTripCount, &NewTCRef, RuntimeCheck);

  // Initialize liveout temp before the main non-const trip count loop if there
  // is no peel loop.
  if (NewTripCount == 0 && !PeelArrayRef)
    OrigLoop->undefInitializeUnconditionalLiveoutTemps();

  // Create the main loop.
  // Profile data is calculated internally in createUnrollOrVecLoop
  HLLoop *MainLoop = createUnrollOrVecLoop(
      OrigLoop, UnrollOrVecFactor, NewTripCount, NewTCRef, LORBuilder, OptTy,
      RuntimeCheck, ProfExists ? &Prof : nullptr);

  // Update the OrigLoop to remainder loop by setting bounds appropriately if
  // remainder loop is needed.
  if (NeedRemainderLoop) {
    processRemainderLoop(OrigLoop, UnrollOrVecFactor, NewTripCount, NewTCRef,
                         RuntimeCheck != nullptr, ProfExists ? &Prof : nullptr);
    HLNodeUtils::addCloningInducedLiveouts(MainLoop, OrigLoop);

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
  } else
    assert((!RuntimeChecks || RuntimeChecks->empty()) &&
           "Remainder loop must exist with rt-checks.");

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
        for (auto BlobIt = Ref->blob_begin(), EB = Ref->blob_end();
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
  assert(FirstLoop->canStripmine(StripmineSize) &&
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

  if (UBRef->isSelfBlob()) {
    UBRef->addBlobDDRef(UBRef->getSelfBlobIndex(), Level - 1);
  }

  UBCE->divide(StripmineSize);
  UBCE->simplify(true, true);

  UBRef->makeConsistent({}, Level);

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

    MinOpRef1->makeConsistent(UBRef, Level);

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

static bool doesConstTCOverflowAfterMult(const HLLoop *Loop, unsigned IVBitSize,
                                         unsigned Multiplier) {
  uint64_t TripCnt;
  if (Loop->isConstTripLoop(&TripCnt)) {
    APInt APOrigTC(IVBitSize, TripCnt);
    APInt APMultiplier(IVBitSize, Multiplier);
    bool Overflow = false;
    (void)APOrigTC.umul_ov(APMultiplier, Overflow);
    return Overflow;
  }

  // For non-const TC, we do not know anything, just say don't overflow
  return false;
}

// Returns false if widenIV is not valid.
static bool widenIVIfNeeded(HLLoop *Lp, unsigned Multiplier) {

  if (doesConstTCOverflowAfterMult(Lp, 64, Multiplier)) {
    // Not valid if TC overflows u64
    return false;
  }

  unsigned IVSize = Lp->getIVType()->getPrimitiveSizeInBits();

  // This is the maximum size we support.
  if (IVSize == 64) {
    return true;
  }

  auto *UpperCE = Lp->getUpperCanonExpr();

  int64_t MaxVal;

  bool HasMax = (UpperCE->isIntConstant(&MaxVal) ||
                 HLNodeUtils::getMaxValue(UpperCE, Lp, MaxVal));

  bool HasSignedIV = Lp->hasSignedIV();

  if (HasMax) {
    int64_t MaxValForSize =
        HasSignedIV ? APInt::getSignedMaxValue(IVSize).getZExtValue()
                    : APInt::getMaxValue(IVSize).getZExtValue();

    if ((MaxVal * Multiplier) < MaxValForSize) {
      return true;
    }
  }

  auto &HNU = Lp->getHLNodeUtils();
  Type *NewIVType = IntegerType::get(HNU.getContext(), 64);

  Lp->setIVType(NewIVType);

  Lp->getLowerCanonExpr()->setSrcAndDestType(NewIVType);
  Lp->getStrideCanonExpr()->setSrcAndDestType(NewIVType);

  if (UpperCE->isIntConstant()) {
    UpperCE->setSrcAndDestType(NewIVType);

  } else if (UpperCE->convertToStandAloneBlob()) {

    unsigned Index = UpperCE->getSingleBlobIndex();
    UpperCE->getBlobUtils().createCastBlob(
        UpperCE->getBlobUtils().getBlob(Index), HasSignedIV, NewIVType, true,
        &Index);

    UpperCE->replaceSingleBlobIndex(Index);
    UpperCE->setSrcAndDestType(NewIVType);

  } else {
    auto *OldUpperRef = Lp->removeUpperDDRef();
    auto *UpperInst = HasSignedIV ? HNU.createSExt(NewIVType, OldUpperRef)
                                  : HNU.createZExt(NewIVType, OldUpperRef);

    HLNodeUtils::insertAsLastPreheaderNode(Lp, UpperInst);
    auto *NewUpperRef = UpperInst->getLvalDDRef()->clone();

    NewUpperRef->getSingleCanonExpr()->setDefinedAtLevel(Lp->getNestingLevel() -
                                                         1);

    Lp->setUpperDDRef(NewUpperRef);

    HIRInvalidationUtils::invalidateParentLoopBodyOrRegion(Lp);
  }

  return true;
}

static void updateTripCountPragma(HLLoop *Lp, unsigned Multiplier) {

  int64_t MaxPragmaTC = APInt::getMaxValue(32).getZExtValue();
  unsigned PragmaTripCount;

  // Removes trip count pragma if it overflows after rerolling.

  if (Lp->getPragmaBasedMinimumTripCount(PragmaTripCount)) {
    if ((static_cast<int64_t>(PragmaTripCount) * Multiplier) <= MaxPragmaTC) {
      Lp->setPragmaBasedMinimumTripCount(PragmaTripCount * Multiplier);
    } else {
      Lp->removePragmaBasedMinimumTripCount();
    }
  }

  if (Lp->getPragmaBasedMaximumTripCount(PragmaTripCount)) {
    if ((static_cast<int64_t>(PragmaTripCount) * Multiplier) <= MaxPragmaTC) {
      Lp->setPragmaBasedMaximumTripCount(PragmaTripCount * Multiplier);
    } else {
      Lp->removePragmaBasedMaximumTripCount();
    }
  }

  if (Lp->getPragmaBasedAverageTripCount(PragmaTripCount)) {
    if ((static_cast<int64_t>(PragmaTripCount) * Multiplier) <= MaxPragmaTC) {
      Lp->setPragmaBasedAverageTripCount(PragmaTripCount * Multiplier);
    } else {
      Lp->removePragmaBasedAverageTripCount();
    }
  }
}

bool HIRTransformUtils::multiplyTripCount(HLLoop *Lp, unsigned Multiplier) {
  assert(Lp->isNormalized() && "Normalized loop expected");

  auto *UpperRef = Lp->getUpperDDRef();
  bool UpperWasSelfBlob = UpperRef->isSelfBlob();
  auto *UpperCE = UpperRef->getSingleCanonExpr();
  unsigned OrigIndex =
      UpperWasSelfBlob ? UpperCE->getSingleBlobIndex() : InvalidBlobIndex;

  bool CanWidenIV = widenIVIfNeeded(Lp, Multiplier);
  if (!CanWidenIV) {
    return false;
  }

  UpperCE->addConstant(1, true);
  UpperCE->multiplyByConstant(Multiplier);
  UpperCE->addConstant(-1, true);

  if (UpperWasSelfBlob) {
    // Self-blob will turn into non-self blob so we need to add a blob ref.
    auto *BlobRef = UpperRef->getDDRefUtils().createBlobDDRef(
        OrigIndex, UpperCE->getDefinedAtLevel());
    UpperRef->addBlobDDRef(BlobRef);
    UpperRef->setSymbase(GenericRvalSymbase);
  }

  Lp->setMaxTripCountEstimate(Lp->getMaxTripCountEstimate() * Multiplier);

  updateTripCountPragma(Lp, Multiplier);
  return true;
}

void HIRTransformUtils::divideProfileDataBy(HLContainerTy::iterator Begin,
                                            HLContainerTy::iterator End,
                                            uint64_t Denominator) {
  std::for_each(
      HLRangeIterator(Begin), HLRangeIterator(End),
      [Denominator](HLNode *Node) { Node->divideProfileData(Denominator); });
}

void HIRTransformUtils::cloneOrRemoveZttPredicates(
    HLLoop *Loop, SmallVectorImpl<PredicateTuple> &ZTTs, bool Clone) {

  if (!Loop->hasZtt())
    return;

  for (auto PredI = Loop->ztt_pred_begin(), PredE = Loop->ztt_pred_end();
       PredI != PredE; ++PredI) {

    RegDDRef *LHS;
    RegDDRef *RHS;

    if (Clone) {
      LHS = Loop->getZttPredicateOperandDDRef(PredI, true)->clone();
      RHS = Loop->getZttPredicateOperandDDRef(PredI, false)->clone();
    } else {
      LHS = Loop->removeZttPredicateOperandDDRef(PredI, true);
      RHS = Loop->removeZttPredicateOperandDDRef(PredI, false);
    }

    ZTTs.emplace_back(LHS, *PredI, RHS);
  }
}

void HIRTransformUtils::mergeZtt(HLLoop *Loop,
                                 SmallVectorImpl<PredicateTuple> &ZTTs) {
  if (ZTTs.empty())
    return;

  RegDDRef *LHS;
  HLPredicate Pred;
  RegDDRef *RHS;

  auto ZttI = ZTTs.begin();

  if (!Loop->hasZtt()) {
    std::tie(LHS, Pred, RHS) = *ZttI++;
    Loop->createZtt(LHS, Pred, RHS);
  }

  for (auto E = ZTTs.end(); ZttI != E; ++ZttI) {
    std::tie(LHS, Pred, RHS) = *ZttI;

    Loop->addZttPredicate(Pred, LHS, RHS);
  }
}

bool HIRTransformUtils::doDeadStoreElimination(HLRegion &Region, HLLoop *Lp,
                                               HIRDDAnalysis &HDDA,
                                               HIRLoopStatistics &HLS) {
  assert(Lp && "Lp* can't be null\n");
  if (Lp->getParentRegion() != &Region) {
    LLVM_DEBUG(dbgs() << "Expect Loop be in the region provided\n";);
    return false;
  }

  HIRDeadStoreElimination DSE(Region.getHLNodeUtils().getHIRFramework(), HDDA,
                              HLS);
  return DSE.run(Region, Lp, /* IsRegion */ false);
}

bool HIRTransformUtils::doDeadStoreElimination(HLRegion &Region,
                                               HIRDDAnalysis &HDDA,
                                               HIRLoopStatistics &HLS) {
  HIRDeadStoreElimination DSE(Region.getHLNodeUtils().getHIRFramework(), HDDA,
                              HLS);
  return DSE.run(Region, nullptr, /* IsRegion */ true);
}

typedef DDRefGathererLambda<RegDDRef> MemRefGatherer;

bool HIRTransformUtils::doIdentityMatrixSubstitution(
    HLLoop *Loop, const RegDDRef *IdentityRef) {

  auto isTargetSymbase = [&](unsigned SB, const RegDDRef *Ref) {
    return (Ref->getSymbase() == SB);
  };

  LLVM_DEBUG(dbgs() << "Identity Ref:\n"; IdentityRef->dump(1); dbgs() << "\n");

  MemRefGatherer::VectorTy FoundRefs;
  MemRefGatherer::gatherRange(Loop->child_begin(), Loop->child_end(), FoundRefs,
                              std::bind(isTargetSymbase,
                                        IdentityRef->getSymbase(),
                                        std::placeholders::_1));

  if (FoundRefs.empty()) {
    return false;
  }

  if (std::any_of(FoundRefs.begin(), FoundRefs.end(),
                  [](const RegDDRef *Ref) { return Ref->isLval(); })) {
    return false;
  }

  for (auto &Ref : FoundRefs) {
    if (!DDRefUtils::haveEqualBaseAndShape(IdentityRef, Ref, false)) {
      continue;
    }

    // Dimension already checked from EqualBaseandShape
    CanonExpr *FirstCE = Ref->getDimensionIndex(1);
    CanonExpr *SecondCE = Ref->getDimensionIndex(2);

    int64_t Index1, Index2;
    if (!FirstCE->isIntConstant(&Index1) || !SecondCE->isIntConstant(&Index2)) {
      continue;
    }

    RegDDRef *ConstantRef;
    if (Index1 != Index2) {
      ConstantRef = Ref->getDDRefUtils().createNullDDRef(Ref->getDestType());
    } else {
      ConstantRef =
          Ref->getDDRefUtils().createConstOneDDRef(Ref->getDestType());
    }

    auto *Inst = cast<HLInst>(Ref->getHLDDNode());
    // Replace loadinst with copyinst
    if (isa<LoadInst>(Inst->getLLVMInstruction())) {
      auto *LvalRef = Inst->removeLvalDDRef();
      auto *CopyInst = Loop->getHLNodeUtils().createCopyInst(
          ConstantRef, "identity", LvalRef);
      HLNodeUtils::replace(Inst, CopyInst);
    } else {
      Ref->getHLDDNode()->replaceOperandDDRef(Ref, ConstantRef);
    }
  }
  return true;
}

struct ConstArraySubstituter final : public HLNodeVisitorBase {
  bool Changed;

  ConstArraySubstituter() : Changed(false) {}

  void postVisit(const HLNode *) {}

  bool isChanged() const { return Changed; }

  void visit(HLNode *Node) {}

  void visit(HLDDNode *Node) {
    bool LocalChange = false;
    for (RegDDRef *Ref :
         make_range(Node->rval_op_ddref_begin(), Node->rval_op_ddref_end())) {
      RegDDRef *ConstantRef = Ref->simplifyConstArray();
      if (!ConstantRef) {
        continue;
      }

      if (auto *Inst = dyn_cast<HLInst>(Ref->getHLDDNode())) {
        if (isa<LoadInst>(Inst->getLLVMInstruction())) {
          auto *LvalRef = Inst->removeLvalDDRef();
          auto *CopyInst = Node->getHLNodeUtils().createCopyInst(
              ConstantRef, "GlobConstRepl", LvalRef);
          HLNodeUtils::replace(Inst, CopyInst);
          LLVM_DEBUG(dbgs() << "Replaced const global array load\n";);
          LocalChange = true;
          continue;
        }
      }

      Ref->getHLDDNode()->replaceOperandDDRef(Ref, ConstantRef);
      LocalChange = true;
      LLVM_DEBUG(dbgs() << "Replaced const global array ref\n";);
    }

    if (LocalChange) {
      LLVM_DEBUG(Node->dump(););
      Changed = true;
    }
  }
};

struct ConstantPropagater final : public HLNodeVisitorBase {
  unsigned NumPropagated;
  unsigned NumFolded;
  const HLNode *OriginNode;

  // Map of blobindex to RvalRef. The Ref is used to check domination
  // when we find a propagation candidate
  DenseMap<unsigned, RegDDRef *> IndexToRefMap;

  ConstantPropagater(HLNode *Node)
      : NumPropagated(0), NumFolded(0), OriginNode(Node) {}

  bool isChanged() const { return (NumPropagated > 0 || NumFolded > 0); }

  // Add a constant definition to IndexToRefMap
  void addConstPropDef(RegDDRef *LRef, RegDDRef *RRef);

  // Erase an index from the Map
  void removeConstPropIndex(unsigned Index, HLInst *Curr = nullptr);

  void doConstFolding(HLInst *Inst);

  // Propagate constant values to any blobs in the Ref
  void propagateConstUse(RegDDRef *Ref);

  void visit(HLNode *Node) {}

  void postVisit(HLNode *Node) {}

  void visit(HLLoop *Loop) {

    if (IndexToRefMap.empty()) {
      return;
    }

    // Propagate refs to Loop Node
    visit(cast<HLDDNode>(Loop));

    // Do not propagate to livein SB. TODO: Only remove liveins defined in loop
    for (unsigned SB : make_range(Loop->live_in_begin(), Loop->live_in_end())) {
      unsigned Index = Loop->getBlobUtils().findTempBlobIndex(SB);
      if (Index != InvalidBlobIndex) {
        IndexToRefMap.erase(Index);
      }
    }
  }

  void postVisit(HLLoop *Loop) {
    // Remove liveout SB from propagation candidates
    for (unsigned SB :
         make_range(Loop->live_out_begin(), Loop->live_out_end())) {
      unsigned Index = Loop->getBlobUtils().findTempBlobIndex(SB);
      if (Index != InvalidBlobIndex) {
        IndexToRefMap.erase(Index);
      }
    }

    // Try to delete constant definitions that have not been invalidated,
    // meaning they have all been legally propagated. Do this only for the
    // Node that was called by the visitor.
    if (Loop == OriginNode) {
      for (auto &Pair : IndexToRefMap) {
        auto DefRef = Pair.second;
        if (!DefRef) { // Invalidated when a use does not postdominate
          continue;
        }
        HLNodeUtils::remove(DefRef->getHLDDNode());
      }
    }
  }

  void visit(HLDDNode *Node) {
    for (RegDDRef *Ref :
         make_range(Node->op_ddref_begin(), Node->op_ddref_end())) {
      propagateConstUse(Ref);
    }
  }

  void visit(HLInst *Inst) {
    // Propagate constants with call
    visit(cast<HLDDNode>(Inst));

    RegDDRef *LvalRef = Inst->getLvalDDRef();
    if (LvalRef && LvalRef->isTerminalRef()) {
      if (LvalRef->isSelfBlob()) {
        removeConstPropIndex(LvalRef->getSelfBlobIndex(), Inst);
      } else {
        unsigned Index =
            LvalRef->getBlobUtils().findTempBlobIndex(LvalRef->getSymbase());
        if (Index != InvalidBlobIndex) {
          removeConstPropIndex(Index, Inst);
        }
      }
    }
    doConstFolding(Inst);
  }
}; // end ConstantPropagater

void ConstantPropagater::removeConstPropIndex(unsigned Index, HLInst *Curr) {
  assert(Curr && "Contant Def Inst must be valid!\n");

  auto *Ref = IndexToRefMap[Index];
  if (Ref && HLNodeUtils::strictlyPostDominates(Curr, Ref->getHLDDNode())) {
    HLNodeUtils::remove(Ref->getHLDDNode());
  }

  IndexToRefMap.erase(Index);
}

void ConstantPropagater::addConstPropDef(RegDDRef *LRef, RegDDRef *RRef) {
  unsigned Index;
  if (LRef->isSelfBlob()) {
    Index = LRef->getSelfBlobIndex();
  } else {
    Index = LRef->getBlobUtils().findTempBlobIndex(LRef->getSymbase());
    if (Index == InvalidBlobIndex) {
      return;
    }
  }

  IndexToRefMap[Index] = RRef;
}

void ConstantPropagater::doConstFolding(HLInst *Inst) {
  auto isRefConst = [](RegDDRef *Ref) { return Ref->isFoldableConstant(); };
  bool HasConstTerm = std::any_of(Inst->rval_op_ddref_begin(),
                                  Inst->rval_op_ddref_end(), isRefConst);

  if (!HasConstTerm) {
    return;
  }

  RegDDRef *LvalRef = Inst->getLvalDDRef();

  // Copy insts aren't folded but can be propagation candidates
  if (Inst->isCopyInst()) {
    RegDDRef *RvalRef = Inst->getRvalDDRef();
    if (RvalRef->isFoldableConstant()) {
      addConstPropDef(LvalRef, RvalRef);
    }
    return;
  }

  // Try to fold instruction
  HLInst *FoldedInst = Inst->doConstantFolding();
  if (!FoldedInst) {
    return;
  }

  NumFolded++;
  RegDDRef *NewRvalRef = FoldedInst->getRvalDDRef();

  // Replacement of the instruction needs to happen first to ensure domination
  // checking is valid for constant definitions
  if (LvalRef->isTerminalRef() && (NewRvalRef->isFoldableConstant())) {
    addConstPropDef(LvalRef, NewRvalRef);
  }
}

void ConstantPropagater::propagateConstUse(RegDDRef *Ref) {
  bool IsLvalTerminalRef = false;
  // Bailout for Lval Selfblobs and skip any Lval TerminalRefs defined as const

  if (Ref->isLval()) {
    if (Ref->isSelfBlob()) {
      return;
    }

    if (Ref->isTerminalRef()) {
      IsLvalTerminalRef = true;
    }
  }

  unsigned SB = Ref->getSymbase();
  for (auto &Pair : IndexToRefMap) {
    if (!Pair.second || !Ref->usesTempBlob(Pair.first) ||
        (Ref->getSymbase() == ConstantSymbase)) {
      continue;
    }

    unsigned DefIndex = Pair.first;
    RegDDRef *ConstRef = Pair.second;

    // Skip constant blob definition
    if (IsLvalTerminalRef &&
        (Ref->getBlobUtils().getTempBlobSymbase(DefIndex) == SB)) {
      continue;
    }

    if (!HLNodeUtils::strictlyDominates(ConstRef->getHLDDNode(),
                                        Ref->getHLDDNode())) {
      // Invalidate entry so substitution cannot be performed and original
      // definition will not be eliminated
      Pair.second = nullptr;
      continue;
    }

    bool Replaced = false;
    int64_t IntVal;
    // We can just update the CE for int constant
    if (ConstRef->isIntConstant(&IntVal)) {
      Replaced = Ref->replaceTempBlobByConstant(DefIndex, IntVal);
    } else if (Ref->isSelfBlob()) {
      Ref->replaceSelfBlobByConstBlob(
          ConstRef->getSingleCanonExpr()->getSingleBlobIndex());
      Replaced = true;
    }

    if (!Replaced) { // if replacement didn't happen, invalidate entry
      Pair.second = nullptr;
    }
  }
}

bool HIRTransformUtils::doConstantPropagation(HLNode *Node) {
  ConstantPropagater CP(Node);
  LLVM_DEBUG(dbgs() << "Before constprop\n"; Node->dump(););
  HLNodeUtils::visit(CP, Node);
  LLVM_DEBUG(dbgs() << "After constprop\n"; Node->dump(););
  return CP.isChanged();
}

bool HIRTransformUtils::doConstantArraySubstitution(HLNode *Node) {
  ConstArraySubstituter CAS;
  HLNodeUtils::visit(CAS, Node);
  return CAS.isChanged();
}

bool HIRTransformUtils::doScalarization(HIRFramework &HIRF, HIRDDAnalysis &HDDA,
                                        HLLoop *InnermostLp,
                                        SmallSet<unsigned, 8> &SBS) {
  return HIRArrayScalarization(HIRF, HDDA).doScalarization(InnermostLp, SBS);
}

bool HIRTransformUtils::doOptVarPredicate(
    HLLoop *Loop, SmallVectorImpl<HLLoop *> &OutLoops,
    SmallPtrSetImpl<HLNode *> &NodesToInvalidate) {
  auto Pass = HIROptVarPredicateInterface::create(
      Loop->getHLNodeUtils().getHIRFramework());

  if (!Pass->processLoop(Loop, false, &OutLoops)) {
    return false;
  }

  auto &InvalidatedNodes = Pass->getNodesToInvalidate();
  NodesToInvalidate.insert(InvalidatedNodes.begin(), InvalidatedNodes.end());
  return true;
}

void HIRTransformUtils::setSelfBlobDDRef(RegDDRef *Ref, BlobTy Blob,
                                         unsigned BlobIndex) {

  int64_t Value;
  bool IsConstInt = Ref->getBlobUtils().isConstantIntBlob(Blob, &Value);

  assert(IsConstInt || Ref->getBlobUtils().getBlob(BlobIndex) == Blob);

  CanonExpr *CE = Ref->getSingleCanonExpr();
  CE->clear();

  if (IsConstInt) {
    CE->setConstant(Value);
    Ref->setSymbase(ConstantSymbase);
  } else {
    CE->setBlobCoeff(BlobIndex, 1);
    if (BlobUtils::isTempBlob(Blob)) {
      BlobUtils &BU = Ref->getBlobUtils();
      Ref->setSymbase(BU.findTempBlobSymbase(Blob));
    } else {
      Ref->setSymbase(GenericRvalSymbase);
    }
  }
}
