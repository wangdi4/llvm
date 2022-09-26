//===--- HIRTransformUtils.cpp  -------------------------------------------===//
//
// Copyright (C) 2015-2021 Intel Corporation. All rights reserved.
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
#include "llvm/Transforms/Intel_LoopTransforms/Utils/HIRArrayContractionUtils.h"

#if INTEL_FEATURE_SW_DTRANS
#include "Intel_DTrans/Analysis/DTransImmutableAnalysis.h"
#endif // INTEL_FEATURE_SW_DTRANS
#include "llvm/Analysis/ConstantFolding.h"
#include "llvm/Analysis/Intel_LoopAnalysis/IR/HLNodeMapper.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/DDRefGatherer.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/DDRefGrouping.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/DDRefUtils.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/DDUtils.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/ForEach.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HIRInvalidationUtils.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HLNodeIterator.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HLNodeUtils.h"
#include "llvm/IR/Instructions.h"

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
using namespace llvm::loopopt::arraycontractionutils;

static cl::opt<bool> DisableConstantPropagation(
    "hir-transform-utils-disable-constprop", cl::init(false), cl::Hidden,
    cl::desc("Disable All Constant Propagation calls in LoopOpt"));

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
#if INTEL_FEATURE_SW_DTRANS
                                        FieldModRefResult *FieldModRef,
#endif // INTEL_FEATURE_SW_DTRANS
                                        bool IgnoreIVs) {
  assert(MemRef && "Memref is null!");
  assert(MemRef->isMemRef() && "Ref is not a memref!");
  assert(Loop && "Loop is null!");
  assert(HLNodeUtils::contains(Loop, MemRef->getHLDDNode()) &&
         "MemRef expected to be inside Loop!");

  HIRLMM LMMPass(Loop->getHLNodeUtils().getHIRFramework(), HDDA, HLS,
#if INTEL_FEATURE_SW_DTRANS
                 FieldModRef,
#endif // INTEL_FEATURE_SW_DTRANS
                 nullptr);
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

void HIRTransformUtils::adjustTCEstimatesForUnrollOrVecFactor(
    HLLoop *NewLoop, unsigned UnrollOrVecFactor) {
  NewLoop->setMaxTripCountEstimate(
      NewLoop->getMaxTripCountEstimate() / UnrollOrVecFactor,
      NewLoop->isMaxTripCountEstimateUsefulForDD());

  NewLoop->setLegalMaxTripCount(NewLoop->getLegalMaxTripCount() /
                                UnrollOrVecFactor);

  NewLoop->dividePragmaBasedTripCount(UnrollOrVecFactor);
}

HLLoop *HIRTransformUtils::createUnrollOrVecLoop(
    HLLoop *OrigLoop, unsigned UnrollOrVecFactor, uint64_t NewTripCount,
    const RegDDRef *NewTCRef, bool NeedRemainderLoop,
    OptReportBuilder &ORBuilder, OptimizationType OptTy, HLIf *RuntimeCheck,
    ProfInfo *Prof) {
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

    // Update unrolled/vectorized loop's max trip count info.
    adjustTCEstimatesForUnrollOrVecFactor(NewLoop, UnrollOrVecFactor);
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
  ORBuilder(*OrigLoop).moveOptReportTo(*NewLoop);
  if (OptTy == OptimizationType::Unroll) {

    if (NeedRemainderLoop) {
      // Loop unrolled with remainder by %d
      ORBuilder(*NewLoop).addRemark(OptReportVerbosity::Low, 25439u,
                                    UnrollOrVecFactor);
    } else {
      // Loop unrolled without remainder by %d
      ORBuilder(*NewLoop).addRemark(OptReportVerbosity::Low, 25438u,
                                    UnrollOrVecFactor);
    }

  } else if (OptTy == OptimizationType::UnrollAndJam) {

    // Loop has been unrolled and jammed by %d
    ORBuilder(*NewLoop).addRemark(OptReportVerbosity::Low, 25540u,
                                  UnrollOrVecFactor);
  } else {
    assert(OptTy == OptimizationType::Vectorizer &&
           "Invalid optimization type!");
    // Do nothing, Vectorizer will add remarks about vectorized loops
  }

  return NewLoop;
}

// Generates the following extra checks for the remainder loop to handle special
// case when trip count of the loop may be equal to range of IV type which is
// interpreted as zero-
//
// %bound.check = 8 * %tgu <u OrigTC; // Orig ZTT
// %zero.tc.check = OrigTC == 0;
// %combined.ztt = %bound.check  |  %zero.tc.check;
// if (%combined.ztt != 0)            // New ZTT
void generateZeroTripZtt(HLLoop *RemainderLoop, RegDDRef *OrigTCRef) {

  auto &HNU = RemainderLoop->getHLNodeUtils();
  auto *OrigZtt = RemainderLoop->extractZtt();

  auto PredIt = OrigZtt->pred_begin();
  auto *LHS = OrigZtt->removeLHSPredicateOperandDDRef(PredIt);
  auto *RHS = OrigZtt->removeRHSPredicateOperandDDRef(PredIt);

  auto *BoundCmpInst = HNU.createCmp(*PredIt, LHS, RHS, "bound.check");

  auto *ZeroRef =
      OrigZtt->getDDRefUtils().createNullDDRef(OrigTCRef->getDestType());
  auto *ZeroTripCmpInst =
      HNU.createCmp(CmpInst::ICMP_EQ, OrigTCRef, ZeroRef, "zero.tc.check");

  auto *OrLHS = BoundCmpInst->getLvalDDRef()->clone();
  auto *OrRHS = ZeroTripCmpInst->getLvalDDRef()->clone();

  auto *OrInst = HNU.createOr(OrLHS, OrRHS, "combined.ztt");

  HLNodeUtils::insertBefore(OrigZtt, BoundCmpInst);
  HLNodeUtils::insertBefore(OrigZtt, ZeroTripCmpInst);
  HLNodeUtils::insertBefore(OrigZtt, OrInst);

  auto *OrRef = OrInst->getLvalDDRef();

  auto *ZeroOrRef =
      OrRef->getDDRefUtils().createNullDDRef(OrRef->getDestType());

  OrigZtt->setLHSPredicateOperandDDRef(OrRef->clone(), PredIt);
  OrigZtt->setRHSPredicateOperandDDRef(ZeroOrRef, PredIt);
  OrigZtt->replacePredicate(PredIt, CmpInst::ICMP_NE);
}

void HIRTransformUtils::processRemainderLoop(
    HLLoop *OrigLoop, unsigned UnrollOrVecFactor, uint64_t NewTripCount,
    const RegDDRef *NewTCRef, const bool HasRuntimeCheck,
    bool NeedZeroTripCheck, const ProfInfo *Prof) {
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

    RegDDRef *OrigTCRef = OrigLoop->getTripCountDDRef();
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

    if (NeedZeroTripCheck) {
      generateZeroTripZtt(OrigLoop, OrigTCRef);
    }

    // Update remainder loop's trip count estimate.
    // TODO: can set useful for DD flag if loop is normalized.
    if (!HasRuntimeCheck && !NeedZeroTripCheck) {
      OrigLoop->setMaxTripCountEstimate(UnrollOrVecFactor - 1);
      OrigLoop->setLegalMaxTripCount(UnrollOrVecFactor - 1);
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

  // Original prefetching info does not apply to remainder loop.
  OrigLoop->clearPrefetchingPragmaInfo();

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
    OptReportBuilder &ORBuilder, OptimizationType OptTy, HLLoop **PeelLoop,
    const RegDDRef *PeelArrayRef,
    SmallVectorImpl<std::tuple<HLPredicate, RegDDRef *, RegDDRef *>>
        *RuntimeChecks) {

  uint64_t TrueVal = 0;
  uint64_t FalseVal = 0;
  bool ProfExists = OrigLoop->extractProfileData(TrueVal, FalseVal);
  bool NeedZeroTripCheck = false;

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
    if (PeelLoop) {
      *PeelLoop = PeelLp;
      assert((OptTy == OptimizationType::Vectorizer) &&
             "OptimizationType should be Vectorizer");
      // Remark: Peeled loop for vectorization
      ORBuilder(*PeelLp).addOrigin(25518u);
    }

    // After peeling a new ZTT was created for the main loop, extract it and add
    // outside the loop.
    OrigLoop->extractZtt();
    // TODO: Profiling info for peel loop?
    // CreateZttIf is not applicable here because generatePeelLoop aleady
    // called extractZtt..()
  } else {

    // Skip zero trip checks if runtime checks need to be emitted as the setup
    // is mutually exclusive.
    NeedZeroTripCheck = (!RuntimeChecks || RuntimeChecks->empty()) &&
                        OrigLoop->canTripCountEqualIVTypeRangeSize();

    // Extract Ztt, preheader, and postexit.
    // Notice that if OrigLoop hasZtt() was not true, no effect.
    OrigLoop->extractZttPreheaderAndPostexit();
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
      OrigLoop, UnrollOrVecFactor, NewTripCount, NewTCRef, NeedRemainderLoop,
      ORBuilder, OptTy, RuntimeCheck, ProfExists ? &Prof : nullptr);

  // Update the OrigLoop to remainder loop by setting bounds appropriately if
  // remainder loop is needed.
  if (NeedRemainderLoop) {
    processRemainderLoop(OrigLoop, UnrollOrVecFactor, NewTripCount, NewTCRef,
                         RuntimeCheck != nullptr, NeedZeroTripCheck,
                         ProfExists ? &Prof : nullptr);
    HLNodeUtils::addCloningInducedLiveouts(MainLoop, OrigLoop);

    // Since OrigLoop became a remainder and will be lexicographicaly
    // second to MainLoop, we move all the next siblings back there.
    ORBuilder(*MainLoop).moveSiblingsTo(*OrigLoop);
    if (OptTy == OptimizationType::Vectorizer) {
      // Remark: Remainder loop for vectorization
      ORBuilder(*OrigLoop).addOrigin(25519u);
    } else {
      assert(((OptTy == OptimizationType::Unroll) ||
              (OptTy == OptimizationType::UnrollAndJam)) &&
             "Invalid optimization type!");
      // Remark: Remainder loop
      ORBuilder(*OrigLoop).addOrigin(25491u);
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
                                  unsigned StripmineSize,
                                  bool AllowExplicitBoundInst) {
  uint64_t TripCount;
  bool IsConstTrip = FirstLoop->isConstTripLoop(&TripCount);

  // Caller should call canStripmine before
  assert(FirstLoop->canStripmine(StripmineSize, AllowExplicitBoundInst) &&
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
    unsigned DefAtLevel = UBRef->getDefinedAtLevel();
    UBRef->addBlobDDRef(UBRef->getSelfBlobIndex(), DefAtLevel);
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
      Lp->setLegalMaxTripCount(StripmineSize);
    }

    // Normalize
    bool Result = Lp->normalize(AllowExplicitBoundInst);
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

  } else if (UpperCE->convertToStandAloneBlobOrConstant()) {

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
    Lp->addLiveInTemp(NewUpperRef);

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
  Lp->setLegalMaxTripCount(Lp->getLegalMaxTripCount() * Multiplier);

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
      LHS = Loop->getLHSZttPredicateOperandDDRef(PredI)->clone();
      RHS = Loop->getRHSZttPredicateOperandDDRef(PredI)->clone();
    } else {
      LHS = Loop->removeLHSZttPredicateOperandDDRef(PredI);
      RHS = Loop->removeRHSZttPredicateOperandDDRef(PredI);
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

    replaceOperand(Ref, ConstantRef);
  }
  return true;
}

struct ConstantPropagater final : public HLNodeVisitorBase {
private:
  unsigned NumPropagated;
  unsigned NumFolded;
  unsigned NumConstGlobalLoads;
  unsigned NumInstsRemoved;
#if INTEL_FEATURE_SW_DTRANS
  DTransImmutableInfo *DTII;
#endif // INTEL_FEATURE_SW_DTRANS

  // Node passed in by caller
  const HLNode *OriginNode;

  // Current HLNode that is invalidated when changed
  HLNode *CurrLoopOrRegion;

  // Set of Nodes that have already been invalidated
  SmallPtrSet<HLNode *, 32> InvalidatedNodes;

  // Maps blobindex to RvalRef. The Ref is used to check domination
  // when we find a propagation candidate
  DenseMap<unsigned, RegDDRef *> IndexToRefMap;

  // Maps a loop to the set of seen symbases. Used to remove temp SBs
  // that may get folded away from constant propagation.
  DenseMap<HLLoop *, SmallSet<unsigned, 16>> LoopTempDefs;

  // Add a constant definition to IndexToRefMap
  void addConstPropDef(RegDDRef *LRef, RegDDRef *RRef);

  // Erase an index from the Map
  void removeConstPropIndex(unsigned Index, HLInst *Curr = nullptr);

  std::pair<bool, HLInst *> constantFold(HLInst *Inst);

  // Propagate constant values to any blobs in the Ref
  void propagateConstUse(RegDDRef *Ref);

  // Return true if CurrLoopOrRegion needs to be invalidated.
  bool checkInvalidated() { return InvalidatedNodes.count(CurrLoopOrRegion); }

  // If needed, invalidates CurrLoopOrRegion and marks it
  void doInvalidate() {
    if (InvalidatedNodes.count(CurrLoopOrRegion)) {
      return;
    }

    InvalidatedNodes.insert(CurrLoopOrRegion);

    if (auto *Loop = dyn_cast<HLLoop>(CurrLoopOrRegion)) {
      HIRInvalidationUtils::invalidateBody<HIRLoopStatistics>(Loop);
    } else if (auto *Region = dyn_cast<HLRegion>(CurrLoopOrRegion)) {
      HIRInvalidationUtils::invalidateNonLoopRegion<HIRLoopStatistics>(Region);
    } else {
      llvm_unreachable("Non Loop/Region encountered!");
    }
  }

  // Add SB to all loop parents set of live SBs
  void addTempDef(unsigned SB) {
    auto Loop = dyn_cast<HLLoop>(CurrLoopOrRegion);
    for (; Loop; Loop = Loop->getParentLoop()) {
      LoopTempDefs[Loop].insert(SB);
    }
  }

  // Try to delete constant definitions that have not been invalidated,
  // meaning they have all been legally propagated. Do this only for the
  // Node that was called by the visitor.
  void cleanupDefs(HLNode *Node) {
    if (Node != OriginNode) {
      return;
    }

    for (auto &Pair : IndexToRefMap) {
      auto DefRef = Pair.second;
      if (!DefRef) { // Invalidated when a use does not postdominate
        continue;
      }
      doInvalidate();
      NumInstsRemoved++;
      HLNodeUtils::remove(DefRef->getHLDDNode());
    }
  }

public:
#if INTEL_FEATURE_SW_DTRANS
  ConstantPropagater(DTransImmutableInfo *DTII, HLNode *Node)
#else  // INTEL_FEATURE_SW_DTRANS
  ConstantPropagater(HLNode *Node)
#endif // INTEL_FEATURE_SW_DTRANS
      : NumPropagated(0), NumFolded(0), NumConstGlobalLoads(0),
        NumInstsRemoved(0),
#if INTEL_FEATURE_SW_DTRANS
        DTII(DTII),
#endif // INTEL_FEATURE_SW_DTRANS
        OriginNode(Node) {
    if (isa<HLLoop>(Node) || isa<HLRegion>(Node)) {
      CurrLoopOrRegion = Node;
    } else if (HLLoop *ParentLoop = Node->getParentLoop()) {
      CurrLoopOrRegion = ParentLoop;
    } else if (HLRegion *ParentRegion = Node->getParentRegion()) {
      CurrLoopOrRegion = ParentRegion;
    } else {
      llvm_unreachable("Expect region or node at top level!");
    }
  }

  bool isChanged() const {
    return (NumPropagated > 0 || NumFolded > 0 || NumConstGlobalLoads > 0);
  }

  LLVM_DUMP_METHOD
  void dumpStatistics() const {
    dbgs() << "NumPropagated: " << NumPropagated << "\n";
    dbgs() << "NumFolded: " << NumFolded << "\n";
    dbgs() << "NumConstGlobalLoads: " << NumConstGlobalLoads << "\n";
    dbgs() << "NumInstsRemoved: " << NumInstsRemoved << "\n";
    dbgs() << "\n";
  }

  void visit(HLNode *Node) {}

  void postVisit(HLNode *Node) {}

  void visit(HLLoop *Loop) {
    CurrLoopOrRegion = Loop;

    if (IndexToRefMap.empty()) {
      return;
    }

    // Propagate refs to Loop Node
    visit(cast<HLDDNode>(Loop));

    // Do not propagate to livein SB.
    for (unsigned SB : make_range(Loop->live_in_begin(), Loop->live_in_end())) {
      unsigned Index = Loop->getBlobUtils().findTempBlobIndex(SB);
      if (Index != InvalidBlobIndex) {
        IndexToRefMap.erase(Index);
      }
    }
  }

  void postVisit(HLRegion *Region) {
    if (IndexToRefMap.empty()) {
      return;
    }

    // Remove liveout entries for this region from being cleaned up
    for (auto It = IndexToRefMap.begin(); It != IndexToRefMap.end(); It++) {
      unsigned TempIndex = It->first;
      unsigned SB = Region->getBlobUtils().getTempBlobSymbase(TempIndex);
      if (Region->isLiveOut(SB)) {
        IndexToRefMap.erase(It);
      }
    }

    cleanupDefs(Region);
  }

  void postVisit(HLLoop *Loop) {
    CurrLoopOrRegion = Loop->getParentLoop();
    if (!CurrLoopOrRegion) {
      CurrLoopOrRegion = Loop->getParentRegion();
    }

    // Remove liveout SBs that might have been folded away. Valid liveout
    // temps should have been marked during loop traversal
    SmallVector<unsigned, 4> DeadSBs;
    for (unsigned SB :
         make_range(Loop->live_out_begin(), Loop->live_out_end())) {
      if (!LoopTempDefs[Loop].count(SB)) {
        DeadSBs.push_back(SB);
      }
    }

    for (auto SB : DeadSBs) {
      Loop->removeLiveOutTemp(SB);
    }

    if (IndexToRefMap.empty()) {
      return;
    }

    // Remove liveout SB from propagation candidates
    for (unsigned SB :
         make_range(Loop->live_out_begin(), Loop->live_out_end())) {
      unsigned Index = Loop->getBlobUtils().findTempBlobIndex(SB);
      if (Index != InvalidBlobIndex) {
        IndexToRefMap.erase(Index);
      }
    }

    // Remove any entries Live in and are also defined in the same loop
    // Handle cases where there could be a backwards use of temp:
    // t = 0;
    // DO
    // ... = t
    // t = 1 <-- defined after use
    // ENDDO
    for (unsigned SB : make_range(Loop->live_in_begin(), Loop->live_in_end())) {
      unsigned Index = Loop->getBlobUtils().findTempBlobIndex(SB);
      if (Index != InvalidBlobIndex) {
        RegDDRef *Ref = IndexToRefMap[Index];
        if (Ref && (Ref->getLexicalParentLoop() == Loop)) {
          IndexToRefMap.erase(Index);
        }
      }
    }

    cleanupDefs(Loop);
  }

  HLDDNode *visit(HLDDNode *Node) {
    HLDDNode *ReplacedNode = nullptr;
    for (RegDDRef *Ref : make_range(Node->ddref_begin(), Node->ddref_end())) {
      propagateConstUse(Ref);

      // Try to replace constant array
#if INTEL_FEATURE_SW_DTRANS
      if (auto ConstantRef = DDRefUtils::simplifyConstArray(Ref, DTII)) {
#else  // INTEL_FEATURE_SW_DTRANS
      if (auto ConstantRef = DDRefUtils::simplifyConstArray(Ref)) {
#endif // INTEL_FEATURE_SW_DTRANS
        NumConstGlobalLoads++;
        LLVM_DEBUG(dbgs() << "Replaced const array load: "; Ref->dump();
                   dbgs() << "\n";);
        ReplacedNode = HIRTransformUtils::replaceOperand(Ref, ConstantRef);
      }
    }
    return ReplacedNode;
  }

  void visit(HLInst *Inst) {

    if (HLInst *CopyInst = cast_or_null<HLInst>(visit(cast<HLDDNode>(Inst)))) {
      Inst = CopyInst;
    }

    // Remove Lval as propagation candidate
    RegDDRef *LvalRef = Inst->getLvalDDRef();
    if (!LvalRef) {
      return;
    }

    unsigned LvalSB = LvalRef->getSymbase();
    bool IsLvalTerminalRef = LvalRef->isTerminalRef();

    if (IsLvalTerminalRef) {
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

    bool Folded;
    HLInst *FoldedInst;
    std::tie(Folded, FoldedInst) = constantFold(Inst);

    // Unless FoldedInst was removed after folding, track LvalSB if terminal
    if (Folded && !FoldedInst) {
      NumInstsRemoved++;
      return;
    }

    if (!IsLvalTerminalRef) {
      return;
    }

    assert(
        (!FoldedInst || FoldedInst->getLvalDDRef()->getSymbase() == LvalSB) &&
        "Symbase mismatch!");

    addTempDef(LvalSB);
  }
}; // end ConstantPropagater

void ConstantPropagater::removeConstPropIndex(unsigned Index, HLInst *Curr) {
  assert(Curr && "Constant Def Inst must be valid!\n");

  auto *Ref = IndexToRefMap[Index];
  if (!Ref) {
    return;
  }

  auto RefParentNode = Ref->getHLDDNode();
  if (HLNodeUtils::strictlyPostDominates(Curr, RefParentNode)) {
    doInvalidate();
    NumInstsRemoved++;
    HLNodeUtils::remove(RefParentNode);
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

// Attempts to fold the instruction, and checks candidate for future
// propagation.
std::pair<bool, HLInst *> ConstantPropagater::constantFold(HLInst *Inst) {
  auto isRefConst = [](RegDDRef *Ref) { return Ref->isFoldableConstant(); };
  bool HasConstTerm = std::any_of(Inst->rval_op_ddref_begin(),
                                  Inst->rval_op_ddref_end(), isRefConst);

  if (!HasConstTerm) {
    return std::make_pair(false, nullptr);
  }

  // Copy insts aren't folded but can be propagation candidates
  if (Inst->isCopyInst()) {
    RegDDRef *RvalRef = Inst->getRvalDDRef();
    if (RvalRef->isFoldableConstant()) {
      addConstPropDef(Inst->getLvalDDRef(), RvalRef);
    }
    return std::make_pair(false, nullptr);
  }

  bool NeedsInvalidation = !checkInvalidated();
  std::pair<bool, HLInst *> Result =
      HIRTransformUtils::constantFoldInst(Inst, NeedsInvalidation);
  bool Folded = Result.first;
  HLInst *FoldedInst = Result.second;

  // Returnval of <true, nullptr> means self-assignment was removed
  if (Folded) {
    NumFolded++;
  }

  // See if folded instruction is candidate for future propagation
  if (Folded && FoldedInst) {
    RegDDRef *LvalRef = FoldedInst->getLvalDDRef();
    RegDDRef *NewRvalRef = FoldedInst->getRvalDDRef();

    // Add candidate for future propagation
    if (LvalRef->isTerminalRef() && (NewRvalRef->isFoldableConstant())) {
      addConstPropDef(LvalRef, NewRvalRef);
    }
  }

  return Result;
}

void ConstantPropagater::propagateConstUse(RegDDRef *Ref) {
  bool IsLvalTerminalRef = false;

  // Bailout for Lval Selfblobs and skip any Lval TerminalRefs defined as
  // const
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
    if (!Pair.second || !Ref->usesTempBlob(Pair.first)) {
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

    int64_t IntVal;
    // We can just update the CE for int constant
    if (ConstRef->isIntConstant(&IntVal)) {
      Ref->replaceTempBlobByConstant(DefIndex, IntVal);
    } else if (Ref->isSelfBlob()) {
      Ref->replaceSelfBlobByConstBlob(
          ConstRef->getSingleCanonExpr()->getSingleBlobIndex());
    } else {
      llvm_unreachable("Could not replace ref!");
    }
    NumPropagated++;
  }
}

#if INTEL_FEATURE_SW_DTRANS
bool HIRTransformUtils::doConstantPropagation(HLNode *Node,
                                              DTransImmutableInfo *DTII) {
#else  // INTEL_FEATURE_SW_DTRANS
bool HIRTransformUtils::doConstantPropagation(HLNode *Node) {
#endif // INTEL_FEATURE_SW_DTRANS
  if (DisableConstantPropagation) {
    return false;
  }
#if INTEL_FEATURE_SW_DTRANS
  ConstantPropagater CP(DTII, Node);
#else  // INTEL_FEATURE_SW_DTRANS
  ConstantPropagater CP(Node);
#endif // INTEL_FEATURE_SW_DTRANS
  LLVM_DEBUG(dbgs() << "Before constprop\n"; Node->dump(););
  HLNodeUtils::visit(CP, Node);
  LLVM_DEBUG(dbgs() << "After constprop\n"; Node->dump(); CP.dumpStatistics(););
  return CP.isChanged();
}

std::pair<bool, HLInst *> HIRTransformUtils::constantFoldInst(HLInst *Inst,
                                                              bool Invalidate) {
  bool LLVMConstFolded = false;
  const Instruction *LLVMInst = Inst->getLLVMInstruction();

  if (isa<BinaryOperator>(LLVMInst)) {
    RegDDRef *RHS, *LHS, *Result;
    LHS = Inst->getOperandDDRef(1);
    RHS = Inst->getOperandDDRef(2);
    Result = nullptr;
    bool Negate = false;
    unsigned OpC = LLVMInst->getOpcode();

    ConstantFP *ConstR, *ConstL;
    Constant *ConstResult;
    // Use LLVM Framework to compute new equivalent constant ref
    if (RHS->isFPConstant(&ConstR) && LHS->isFPConstant(&ConstL)) {
      ConstResult = ConstantFoldBinaryOpOperands(
          OpC, ConstL, ConstR, Inst->getHLNodeUtils().getDataLayout());
      if (ConstResult) {
        Result = RHS->getDDRefUtils().createConstDDRef(ConstResult);
        LLVMConstFolded = true;
      }
    }

    if (!LLVMConstFolded) {
      // Fold without creating a new ref
      switch (OpC) {
      case Instruction::Add:
        if (RHS->isZero()) {
          Result = LHS;
        } else if (LHS->isZero()) {
          Result = RHS;
        }
        break;
      case Instruction::FAdd:
        if (RHS->isZero()) {
          Result = LHS;
        } else if (LHS->isZero()) {
          Result = RHS;
        }
        break;
      case Instruction::Sub:
        if (RHS->isZero()) {
          Result = LHS;
        }
        break;
      case Instruction::FSub:
        if (RHS->isZero()) {
          Result = LHS;
        }
        break;
      case Instruction::FMul: {

        // This case does the HIR equivalent of simplifyFMAFMul in
        // lib/Analysis/InstructionSimplify.cpp and also the "X * -1.0 --> -X"
        // peephole in InstCombinerImpl::visitFMul. Most of these operations
        // do not require any fast math flags, except that simplifying
        // multiply by zero to zero requires nnan (because 0.0*NaN=NaN) and
        // nsz (because the resulting zero sign would depend on the sign of
        // both operands).
        bool CanFoldZeroFMul =
            LLVMInst->hasNoNaNs() && LLVMInst->hasNoSignedZeros();

        if ((RHS->isZero() && CanFoldZeroFMul) || LHS->isOne()) {
          Result = RHS;
        } else if ((LHS->isZero() && CanFoldZeroFMul) || RHS->isOne()) {
          Result = LHS;
        } else {
          if (LHS->isMinusOne()) {
            Result = RHS;
          } else if (RHS->isMinusOne()) {
            Result = LHS;
          }
          Negate = true;
        }
      }
      break;

      case Instruction::Mul:
        if (RHS->isZero() || LHS->isOne()) {
          Result = RHS;
        } else if (LHS->isZero() || RHS->isOne()) {
          Result = LHS;
        }
        break;
      }
    }

    if (!Result) {
      return std::make_pair(false, nullptr);
    }

    // Check to see if invalidation required
    if (Invalidate) {
      HIRInvalidationUtils::invalidateParentLoopBodyOrRegion<HIRLoopStatistics>(
          Inst);
    }

    // Remove self assignments that may occur due to folding
    if (DDRefUtils::areEqual(Inst->getLvalDDRef(), Result)) {
      HLNodeUtils::remove(Inst);
      return std::make_pair(true, nullptr);
    }

    if (Negate) {
      HLInst *FNeg = Inst->getHLNodeUtils().createFNeg(
          Inst->removeOperandDDRef(Result), "", Inst->removeLvalDDRef(),
          nullptr, FastMathFlags::getFast());
      HLNodeUtils::replace(Inst, FNeg);
      return std::make_pair(true, FNeg);
    }

    RegDDRef *NewRval =
        (LLVMConstFolded) ? Result : Inst->removeOperandDDRef(Result);

    HLInst *NewInst = NewRval->isMemRef()
                          ? Inst->getHLNodeUtils().createLoad(
                                NewRval, "", Inst->removeLvalDDRef())
                          : Inst->getHLNodeUtils().createCopyInst(
                                NewRval, "", Inst->removeLvalDDRef());

    HLNodeUtils::replace(Inst, NewInst);
    return std::make_pair(true, NewInst);
  }

  return std::make_pair(false, nullptr);
}

bool HIRTransformUtils::doArrayScalarization(HLLoop *InnermostLp,
                                             SmallSet<unsigned, 8> &SBS) {
  return HIRArrayScalarization::doScalarization(InnermostLp, SBS);
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

HLDDNode *HIRTransformUtils::replaceOperand(RegDDRef *OldRef,
                                            RegDDRef *NewRef) {
  assert(OldRef && "OldRef is null!");
  assert(NewRef && "NewRef is null!");
  auto *Node = OldRef->getHLDDNode();
  auto *HInst = dyn_cast<HLInst>(Node);

  assert((!NewRef->isMemRef() || !isa<HLLoop>(Node)) &&
         "Memref is not a valid operand of loop!");

  if (!HInst) {
    Node->replaceOperandDDRef(OldRef, NewRef);
    return Node;
  }

  auto &HNU = Node->getHLNodeUtils();
  RegDDRef *OtherRef = nullptr;
  const Instruction *LLVMInst = HInst->getLLVMInstruction();

  // Replacing memref by non-memref.
  // We possibly need to replace load/store by copy.
  if (OldRef->isMemRef() && !NewRef->isMemRef()) {

    if (isa<StoreInst>(LLVMInst) && OldRef->isLval()) {
      OtherRef = Node->removeOperandDDRef(1);
      auto *NewNode = OtherRef->isMemRef()
                          ? HNU.createLoad(OtherRef, "ld", NewRef)
                          : HNU.createCopyInst(OtherRef, "cp", NewRef);
      HLNodeUtils::replace(Node, NewNode);
      return NewNode;
    }

    if (isa<LoadInst>(LLVMInst)) {
      OtherRef = Node->removeOperandDDRef(0u);
      auto *NewNode = HNU.createCopyInst(NewRef, "cp", OtherRef);
      HLNodeUtils::replace(Node, NewNode);
      return NewNode;
    }

  }
  // Replacing non-memref by memref in copy/gep/load inst. We need to replace
  // it by load/store.
  else if (!OldRef->isMemRef() && NewRef->isMemRef()) {

    if (HInst->isCopyInst() || isa<GetElementPtrInst>(LLVMInst) ||
        isa<LoadInst>(LLVMInst)) {
      bool ReplacingLval = OldRef->isLval();

      OtherRef = ReplacingLval ? Node->removeOperandDDRef(1)
                               : Node->removeOperandDDRef(0u);

      auto *NewNode = ReplacingLval ? HNU.createStore(OtherRef, "st", NewRef)
                                    : HNU.createLoad(NewRef, "ld", OtherRef);
      HLNodeUtils::replace(Node, NewNode);
      return NewNode;
    }
  }
  // Replacing AddressOf ref by terminal ref, change gep to copy.
  else if (isa<GetElementPtrInst>(LLVMInst) && OldRef->isAddressOf() &&
           NewRef->isTerminalRef()) {
    assert(NewRef->isNull() && "Unexpected pointer terminal ref!");

    OtherRef = Node->removeOperandDDRef(0u);
    auto *NewNode = HNU.createCopyInst(NewRef, "cp", OtherRef);
    HLNodeUtils::replace(Node, NewNode);
    return NewNode;
  }

  Node->replaceOperandDDRef(OldRef, NewRef);

  return Node;
}

bool HIRTransformUtils::contractMemRef(RegDDRef *ToContractRef,
                                       SmallSet<unsigned, 4> &PreservedDims,
                                       SmallSet<unsigned, 4> &ToContractDims,
                                       HLRegion &Reg,
                                       RegDDRef *&AfterContractRef) {

  return HIRArrayContractionUtil::contractMemRef(
      ToContractRef, PreservedDims, ToContractDims, Reg, AfterContractRef);
}

bool HIRTransformUtils::doSpecialSinkForPerfectLoopnest(HLLoop *OuterLp,
                                                        HLLoop *InnerLp,
                                                        HIRDDAnalysis &HDDA) {

  // Check the [OuterLp, InnerLp] formed loonest is near perfect,
  // except code in the preheader of the InnerLp.
  auto CheckLoopNestSanity = [](HLLoop *InnerLp) -> bool {
    // Check InnerLp:
    if (!InnerLp->hasPreheader() || InnerLp->hasPostexit()) {
      LLVM_DEBUG(dbgs() << "Expect InnerLp have only non-empty preheader\n";);
      return false;
    }

    // Only allow certain instruction types in InnerLp's preheader:
    for (auto I = InnerLp->pre_begin(), E = InnerLp->pre_end(); I != E; ++I) {
      HLInst *HInst = cast<HLInst>(I);
      auto *LLVMInst = HInst->getLLVMInstruction();

      // Only support the following instruction types:
      // - % (mod)
      // - select
      const unsigned Opcode = LLVMInst->getOpcode();
      if (Opcode == Instruction::SRem || Opcode == Instruction::URem ||
          Opcode == Instruction::Select) {
        continue;
      }

      LLVM_DEBUG(dbgs() << "Encounter unsupported instruction type\n";
                 HInst->dump(););
      return false;
    }

    return true;
  };

  // Do legal that HInst* (a mod operator) can sink into SinkLp
  //
  // It is legal to sink the HInst* into SinkLp if&f the HInst* remains
  // loop-inv after sinking.
  //
  // That is :
  // (1) HInst's Lval Ref is not redefined in SinkLp
  // and
  // (2) non of HInst's Rval Blob is redefined in SinkLp
  //
  auto DoLegalTestForSinking = [&](SmallVectorImpl<HLInst *> &PreLoopInsts,
                                   HLLoop *SinkLp, DDGraph &DDG) {
    SmallVector<DDRef *, 8> RefVec;

    for (auto *I : PreLoopInsts) {
      RefVec.push_back(I->getLvalDDRef());

      // Collect Rval blob:
      for (const RegDDRef *UseRef :
           make_range(I->rval_op_ddref_begin(), I->rval_op_ddref_end())) {

        if (UseRef->isSelfBlob()) {
          RefVec.push_back(const_cast<RegDDRef *>(UseRef));
        } else {
          for (auto *BRef :
               make_range(UseRef->blob_begin(), UseRef->blob_end())) {
            RefVec.push_back(const_cast<BlobDDRef *>(BRef));
          }
        }
      }
    }

    // *** Search any Ref's redefinition(s) in SinkLp:
    for (auto &Ref : RefVec) {
      unsigned LvalCount = 0, RvalCount = 0;
      if (DDUtils::countEdgeToLoop(DDG, Ref, SinkLp, LvalCount, RvalCount) &&
          LvalCount) {
        LLVM_DEBUG({
          dbgs() << "LvalCount: " << LvalCount << "\tRvalCount: " << RvalCount
                 << "\nFound Ref's redefinition in Lp. Offending Ref: ";
          Ref->dump(1);
          dbgs() << "\n";
        });
        return false;
      }
    }

    return true;
  };

  // ****  BEGIN FUNCTION: after the lambdas ****

  // Sanity check that the loopnest has the following properties:
  // - none-perfectness is due to only statements in InnerLp's preheader.
  if (!CheckLoopNestSanity(InnerLp)) {
    LLVM_DEBUG(dbgs() << "Fail in CheckLoopNestSanity(InnerLp)\n";);
    return false;
  }

  // Collect:
  SmallVector<HLInst *, 8> PreLoopInsts;
  for (auto I = InnerLp->pre_begin(), E = InnerLp->pre_end(); I != E; ++I) {
    PreLoopInsts.push_back(cast<HLInst>(I));
  }

  if (PreLoopInsts.empty()) {
    LLVM_DEBUG(dbgs() << "Nothing collected\n";);
    return false;
  }

  // Legal test:
  DDGraph DDG = HDDA.getGraph(InnerLp->getParentLoop());
  if (!DoLegalTestForSinking(PreLoopInsts, InnerLp, DDG)) {
    LLVM_DEBUG(dbgs() << "Fail DoLegalTestForSinking(.)\n";);
    return false;
  }

  // Do transformation: sink the statements from InnerLp's preheader into the
  // InnerLp.
  //
  // E.g.
  // [Before]
  //   ...
  // + DO i2 = 0, sext.i32.i64(%6) + -1, 1   <DO_LOOP>
  // |   + DO i3 = 0, sext.i32.i64(%3) + -1, 1   <DO_LOOP>
  // |   |      %1472 = i3 + 1  %  %3;
  // |   |      %1476 = i3 + %3 + -1  %  %3;
  // |   |   + DO i4 = 0, sext.i32.i64(%2) + -1, 1   <DO_LOOP>
  // |   |   |   ...
  // |   |   + END LOOP
  // |   + END LOOP
  // + END LOOP
  //
  // [After]
  //   ...
  // + DO i2 = 0, sext.i32.i64(%6) + -1, 1   <DO_LOOP>
  // |   + DO i3 = 0, sext.i32.i64(%3) + -1, 1   <DO_LOOP>
  // |   |   + DO i4 = 0, sext.i32.i64(%2) + -1, 1   <DO_LOOP>
  // |   |   |   %1472 = i3 + 1  %  %3;
  // |   |   |   %1476 = i3 + %3 + -1  %  %3;
  // |   |   |   ...
  // |   |   + END LOOP
  // |   + END LOOP
  // + END LOOP

  // - Update def@level for each use:
  unsigned const TargetLevel = InnerLp->getNestingLevel();
  for (auto *I : PreLoopInsts) {
    DDRef *DDRefSrc = I->getLvalDDRef();
    LLVM_DEBUG({
      dbgs() << "Src: ";
      I->dump();
      dbgs() << "DDRefSink(s): <" << DDG.getNumOutgoingEdges(DDRefSrc)
             << ">: \n";
    });
    for (auto II = DDG.outgoing_edges_begin(DDRefSrc),
              IE = DDG.outgoing_edges_end(DDRefSrc);
         II != IE; ++II) {
      DDEdge *Edge = (*II);
      if (RegDDRef *RRef = dyn_cast<RegDDRef>(Edge->getSink())) {
        RRef->updateDefLevel(TargetLevel);
      } else if (BlobDDRef *BRef = dyn_cast<BlobDDRef>(Edge->getSink())) {
        BRef->setDefinedAtLevel(TargetLevel);
      }
    }
  }

  // -Move each instruction from InnerLp's preheader into its body:
  for (auto I = PreLoopInsts.rbegin(), E = PreLoopInsts.rend(); I != E; ++I) {
    HLNodeUtils::moveAsFirstChild(InnerLp, *I);
    DDUtils::updateLiveinsLiveoutsForSinkedInst(InnerLp, *I, true);
  }

  // Update the temp DDRefs from linear-at-level to non-linear
  DDUtils::updateDDRefsLinearity(PreLoopInsts, DDG);
  HIRInvalidationUtils::invalidateBody(InnerLp);
  HIRInvalidationUtils::invalidateBody(InnerLp->getParentLoop());

  return true;
}
