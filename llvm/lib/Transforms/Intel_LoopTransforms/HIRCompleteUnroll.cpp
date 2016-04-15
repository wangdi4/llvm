//===- HIRCompleteUnroll.cpp - Implements CompleteUnroll class ------------===//
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
// This file implements HIRCompleteUnroll class which unrolls a HIR loop
// with small trip count.
//
// For example:
//
// Original Loop                     Transformed
// for(i=0; i<5; i++)                A[0] = B[0];
//    A[i] = B[i];                   A[1] = B[1];
//                                   ...
//                                   A[4] = B[4];
//
// The general algorithm is as follows:
//  1. Visit the Region
//  2. Recurse from outer to inner loops (Gathering phase for Transformation)
//       Perform cost analysis on the loop ( such as trip count of inner loops )
//       If all criteria meet, add loop to transformation list and return true,
//       Else return false indicating the parent loops should not be unrolled.
//  4. For each loop (outer to inner) of Transformed Loops
//       4.1 Clone LoopChild and insert it before the loop.
//       4.2 Update CanonExprs of LoopChild and recursively visit the inner
//           loops.
//       4.3 Delete Loop
//
// Unrolling would increase the register pressure based on the unroll factor.
// Current heuristic just uses trip count to determine if loop needs to be
// unrolled.
//
// Works by unrolling transformation from outermost to inner loops.
// It avoids outer loops if any of the inner loops are not completely unrolled.
// No candidate loops should have a switch or call statement.
//
//===----------------------------------------------------------------------===//

// TODO: Extensions to be added later.
//  (1) Safe reductions chains need to be updated or removed
//  (2) Using a simple heuristic (TripCount) for this implementation. We need to
//     extend it later to incorporate register pressure. Also, for multi-level
//     loops, we are currently summing the trip counts for the loop nest.
//  (3) Extend it for not normalized loop.
//  (4) Add opt report.

#include "llvm/Analysis/Intel_LoopAnalysis/HIRFramework.h"
#include "llvm/Transforms/Intel_LoopTransforms/HIRTransformPass.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/CanonExprUtils.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/DDRefUtils.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/HIRInvalidationUtils.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/HLNodeUtils.h"

#include "llvm/ADT/Statistic.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/Pass.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"

#define DEBUG_TYPE "hir-complete-unroll"

using namespace llvm;
using namespace llvm::loopopt;

// This stat maintains the number of hir loops completely unrolled.
STATISTIC(LoopsCompletelyUnrolled, "Number of HIR loops completely unrolled");
// This stat maintains count of all the candidates for complete unrolling,
// including those that were turned down due to cost model.
STATISTIC(LoopsAnalyzed, "Number of HIR loops analyzed for complete unrolling");

static cl::opt<unsigned> CompleteUnrollTripThreshold(
    "completeunroll-trip-threshold", cl::init(20), cl::Hidden,
    cl::desc("Don't unroll if total trip count is bigger than this,"
             "threshold."));

// Option to turn off the HIR Complete Unroll Pass
static cl::opt<bool>
    DisableHIRCompleteUnroll("disable-hir-complete-unroll", cl::init(false),
                             cl::Hidden,
                             cl::desc("Disable HIR Loop Complete Unrolling"));

static cl::opt<bool> DisableHIRTriCompleteUnroll(
    "disable-hir-tri-complete-unroll", cl::init(false), cl::Hidden,
    cl::desc("Disable HIR Triangular Complete Unrolling"));

namespace {

class HIRCompleteUnroll : public HIRTransformPass {
public:
  static char ID;

  HIRCompleteUnroll(int T = -1) : HIRTransformPass(ID) {
    initializeHIRCompleteUnrollPass(*PassRegistry::getPassRegistry());

    CurrentTripThreshold =
        (T == -1) ? CompleteUnrollTripThreshold : unsigned(T);
  }

  bool runOnFunction(Function &F) override;
  void releaseMemory() override;

  void getAnalysisUsage(AnalysisUsage &AU) const {
    AU.setPreservesAll();
    AU.addRequiredTransitive<HIRFramework>();
  }

private:
  unsigned CurrentTripThreshold;

  class CanonExprVisitor;

  /// Storage for loops which will be transformed.
  /// Only outermost loops to be transformed will be stored.
  SmallVector<HLLoop *, 32> CandidateLoops;

  // Level to TripCount mapping. This is just used during the analysis phase.
  unsigned TripCountCache[MaxLoopNestLevel + 1];

  /// \brief Computes the trip count necessary for complete unrolling.
  /// This routine handles cases such as triangular loops.
  void computeTripCount(const HLLoop *Loop);

  /// \brief This routine checks the dependency across the loop to check
  /// if all the loops are encapsulated before complete unrolling.
  /// For example, we may have the following case:
  /// for(i=0;i<15; i++)
  ///   for(j=0; j<i; j++)
  /// Here, j loop is added as candidate, but because of profitability, we
  /// don't add 'i' loop, then we remove 'j' loop also.
  void checkDependentLoops();

  /// \brief This routine is helper function to check if the child loops
  /// do not contain any IV less than the OuterCandidateLevel.
  bool checkDependency(unsigned OuterCandidateLevel,
                       SmallVectorImpl<HLLoop *> &ChildLoops) const;

  /// \brief Performs cost analysis to determine if a loop
  /// is eligible for complete unrolling. If loop meets all the criteria,
  /// it return true, else false. This routine updates the child trip count
  /// for use by parent loop.
  bool isProfitable(const HLLoop *Loop, int64_t *TripCnt) const;

  /// \brief Performs the complete unrolling transformation.
  static void transformLoop(HLLoop *&Loop, HLLoop *OuterLoop,
                            SmallVectorImpl<int64_t> &TripValues);

  /// \brief Main routine to drive the complete unrolling transformation.
  void processCompleteUnroll(SmallVectorImpl<HLLoop *> &OuterLoops);

  /// \brief Processes a HLLoop to check if it candidate for transformation.
  /// ChildTripCnt denotes the trip count of the children.
  bool processLoop(HLLoop *Loop, int64_t *TotalTripCnt);

  /// \brief Drives the profitability analysis when visiting a loop during
  /// transformation.
  bool processProfitablity(const HLLoop *Loop, int64_t *TripCnt) const;

  /// \brief Routine to drive the transformation of candidate loops.
  void transformLoops();
};

/// \brief Visitor to update the CanonExpr.
class HIRCompleteUnroll::CanonExprVisitor final : public HLNodeVisitorBase {
private:
  HLLoop *OuterLoop;
  SmallVectorImpl<int64_t> *TripValues;

  void processRegDDRef(RegDDRef *RegDD);
  void processCanonExpr(CanonExpr *CExpr);

public:
  CanonExprVisitor(HLLoop *OutLoop, SmallVectorImpl<int64_t> &TripValVec)
      : OuterLoop(OutLoop), TripValues(&TripValVec) {}

  void visit(HLDDNode *Node);
  void visit(HLLoop *Loop);
  // No processing needed for Goto and Label's
  void visit(HLGoto *Goto){};
  void visit(HLLabel *Label){};
  void visit(HLNode *Node) {
    llvm_unreachable(" Node not supported for Complete Unrolling.");
  }
  void postVisit(HLNode *Node) {}
};
}

////// CanonExpr Visitor Start

void HIRCompleteUnroll::CanonExprVisitor::visit(HLLoop *Loop) {
  transformLoop(Loop, OuterLoop, *TripValues);
}

void HIRCompleteUnroll::CanonExprVisitor::visit(HLDDNode *Node) {

  // Only expecting if and inst inside the loops.
  // Primarily to catch errors of other types.
  assert((isa<HLIf>(Node) || isa<HLInst>(Node)) && " Node not supported for "
                                                   "complete unrolling.");

  DEBUG(dbgs() << " CanonExprVisitor Node \n");
  DEBUG(Node->dump());

  for (auto Iter = Node->ddref_begin(), End = Node->ddref_end(); Iter != End;
       ++Iter) {
    processRegDDRef(*Iter);
  }
}

/// processRegDDRef - Processes RegDDRef to call the Canon Exprs
/// present inside it. This is an internal helper function.
void HIRCompleteUnroll::CanonExprVisitor::processRegDDRef(RegDDRef *RegDD) {
  // Process CanonExprs inside the RegDDRefs
  for (auto Iter = RegDD->canon_begin(), End = RegDD->canon_end(); Iter != End;
       ++Iter) {
    processCanonExpr(*Iter);
  }

  // Process GEP Base
  if (RegDD->hasGEPInfo()) {
    processCanonExpr(RegDD->getBaseCE());
  }

  RegDD->makeConsistent();

  // Example of an alternative way of updating DDRef which is useful when some
  // manual work is also involved-
  //
  // RegDD->updateBlobDDRefs(BlobDDRefs);
  // assert(BlobDDRefs.empty() && "New blobs found in DDRef after processing!");
  // RegDD->updateCELevel();
}

/// Processes CanonExpr to replace IV by TripVal.
/// This is an internal helper function.
void HIRCompleteUnroll::CanonExprVisitor::processCanonExpr(CanonExpr *CExpr) {

  // Start replacing the IV's from OuterLoop level to current loop level.
  int64_t LoopLevel = OuterLoop->getNestingLevel();
  for (auto &TripV : *TripValues) {
    DEBUG(dbgs() << "Replacing CanonExpr IV by tripval :" << TripV << " \n");
    CExpr->replaceIVByConstant(LoopLevel, TripV);
    LoopLevel++;
  }
}

///// CanonExpr Visitor End

char HIRCompleteUnroll::ID = 0;
INITIALIZE_PASS_BEGIN(HIRCompleteUnroll, "hir-complete-unroll",
                      "HIR Complete Unroll", false, false)
INITIALIZE_PASS_DEPENDENCY(HIRFramework)
INITIALIZE_PASS_END(HIRCompleteUnroll, "hir-complete-unroll",
                    "HIR Complete Unroll", false, false)

FunctionPass *llvm::createHIRCompleteUnrollPass(int Threshold) {
  return new HIRCompleteUnroll(Threshold);
}

bool HIRCompleteUnroll::runOnFunction(Function &F) {
  // Skip if DisableHIRCompleteUnroll is enabled
  if (DisableHIRCompleteUnroll) {
    DEBUG(dbgs() << "HIR LOOP Complete Unroll Transformation Disabled \n");
    return false;
  }

  DEBUG(dbgs() << "Complete unrolling for Function : " << F.getName() << "\n");
  DEBUG(dbgs() << "Trip Count Threshold : " << CurrentTripThreshold << "\n");

  // Do an early exit if Trip Threshold is less than 1
  // TODO: Check if we want give some feedback to user
  if (CurrentTripThreshold == 0) {
    return false;
  }

  // Storage for Outermost Loops
  SmallVector<HLLoop *, 64> OuterLoops;
  // Gather the outermost loops
  HLNodeUtils::gatherOutermostLoops(OuterLoops);

  // Process Loop Complete Unrolling
  processCompleteUnroll(OuterLoops);

  return false;
}

/// processCompleteUnroll - Main routine to perform unrolling.
/// First, performs cost analysis and then do the transformation.
void HIRCompleteUnroll::processCompleteUnroll(
    SmallVectorImpl<HLLoop *> &OuterLoops) {

  // Walk over the outermost loops across the regions.
  for (auto &Iter : OuterLoops) {
    int64_t TotalTripCnt = 0;
    if (processLoop(Iter, &TotalTripCnt)) {
      CandidateLoops.push_back(Iter);
    }
  }

  // Check the dependency across the loops as the parent
  // might not be a candidate for complete unrolling loop nest.
  checkDependentLoops();

  transformLoops();
}

// Here we remove loops from candidate loops where
// the parent loop is not a candidate but a dependent child loop
// is added to the candidate list.
void HIRCompleteUnroll::checkDependentLoops() {

  for (unsigned Index = 0; Index != CandidateLoops.size();) {

    HLLoop *OuterCandidateLoop = CandidateLoops[Index];
    SmallVector<HLLoop *, 8> ChildLoops;
    HLNodeUtils::gatherAllLoops(OuterCandidateLoop, ChildLoops);
    // Check if the dependency of loops is satisfied. If not, then we add
    // the children loop as candidate and remove the current outermost loops.
    if (checkDependency(OuterCandidateLoop->getNestingLevel(), ChildLoops)) {
      Index++;
      continue;
    }

    if (!OuterCandidateLoop->isInnermost()) {
      HLNodeUtils::gatherLoopsWithLevel(OuterCandidateLoop, CandidateLoops,
                                        OuterCandidateLoop->getNestingLevel() +
                                            1);
    }
    CandidateLoops.erase(CandidateLoops.begin() + Index);
  }
}

bool HIRCompleteUnroll::checkDependency(
    unsigned OuterCandidateLevel, SmallVectorImpl<HLLoop *> &ChildLoops) const {

  for (auto &CLoop : ChildLoops) {
    CanonExpr *UBCE = CLoop->getUpperCanonExpr();
    for (unsigned Level = 1; Level < OuterCandidateLevel; Level++) {
      if (UBCE->hasIV(Level)) {
        return false;
      }
    }
  }
  return true;
}

void HIRCompleteUnroll::computeTripCount(const HLLoop *Loop) {

  unsigned LoopLevel = Loop->getNestingLevel();
  CanonExpr *TripCE = Loop->getTripCountCanonExpr();
  // For not handled cases, substitute with larger trip count
  // to avoid unrolling.
  if (!TripCE || TripCE->hasBlob() || TripCE->hasBlobIVCoeffs() ||
      (TripCE->getDenominator() != 1) || TripCE->containsUndef()) {
    TripCountCache[LoopLevel] = CurrentTripThreshold + 1;
    return;
  }

  int64_t TripCount = 0;
  bool isConstTrip = TripCE->isIntConstant(&TripCount);
  if (isConstTrip) {
    assert(TripCount && " Zero Trip count loop found.");
    TripCountCache[LoopLevel] = TripCount;
    return;
  }

  // If triangular loop is disabled, we simply return high trip count,
  // to avoid unrolling triangular loops.
  if (DisableHIRTriCompleteUnroll) {
    TripCountCache[LoopLevel] = CurrentTripThreshold + 1;
    return;
  }

  // TripCE should only have IV and const.
  // Substitute the IV with their Trips-1. We are computing the max trip
  // here for the current level. If IVCoeff is negative, we substitute the
  // LB of parent i.e. 0 for normalized loops.
  for (unsigned Level = 1; Level < LoopLevel; ++Level) {
    if (TripCE->getIVConstCoeff(Level) > 0) {
      TripCE->replaceIVByConstant(Level, TripCountCache[Level] - 1);
    } else {
      TripCE->replaceIVByConstant(Level, 0);
    }
  }
  assert(TripCE->isIntConstant() && " Trip Count should be a constant.");
  // There can be negative trips for loops i.e. not computed at all.
  // In such cases, we simply return high trip count to avoid unrolling those
  // loops.
  if (TripCE->getConstant() <= 0) {
    TripCountCache[LoopLevel] = CurrentTripThreshold + 1;
  } else {
    TripCountCache[LoopLevel] = TripCE->getConstant() + 1;
  }
}

bool HIRCompleteUnroll::processLoop(HLLoop *Loop, int64_t *TotalTripCnt) {

  SmallVector<HLLoop *, 8> CandidateChildLoops;
  bool IsLoopCandidate = true;

  // Compute the trip count of current loop,
  // as it might be used by children loop e.g. triangular loops.
  computeTripCount(Loop);
  DEBUG(dbgs() << " Compute Trip Count for Level: " << Loop->getNestingLevel()
               << " " << TripCountCache[Loop->getNestingLevel()]);

  // Visit children, only if it not the innermost, else
  // perform profitability analysis.
  if (!Loop->isInnermost()) {
    SmallVector<HLLoop *, 8> ChildLoops;
    // 1. Gather Loops starting from the outer-most level
    HLNodeUtils::gatherLoopsWithLevel(Loop, ChildLoops,
                                      Loop->getNestingLevel() + 1);

    // 2.Process each Loop for Complete Unrolling
    bool ChildValid = true;
    // Recurse through the children.
    for (auto Iter = ChildLoops.begin(), E = ChildLoops.end(); Iter != E;
         ++Iter) {
      int64_t ChildTripCnt = 0;
      bool IsChildCandidate = processLoop(*Iter, &ChildTripCnt);
      if (IsChildCandidate) {
        CandidateChildLoops.push_back(*Iter);
      }
      ChildValid &= IsChildCandidate;
      (*TotalTripCnt) += ChildTripCnt;
    }

    IsLoopCandidate = ChildValid;
  }

  if (IsLoopCandidate) {
    IsLoopCandidate &= processProfitablity(Loop, TotalTripCnt);
  }

  // If current loop is not a candidate, store the children loops
  // for transformation.
  if (!IsLoopCandidate) {
    CandidateLoops.append(CandidateChildLoops.begin(),
                          CandidateChildLoops.end());
  }

  // Reset the trip count cache for current loop.
  TripCountCache[Loop->getNestingLevel()] = 0;

  return IsLoopCandidate;
}

bool HIRCompleteUnroll::processProfitablity(const HLLoop *Loop,
                                            int64_t *TripCnt) const {

  LoopsAnalyzed++;

  if (isProfitable(Loop, TripCnt)) {
    LoopsCompletelyUnrolled++;
    return true;
  }
  return false;
}

bool HIRCompleteUnroll::isProfitable(const HLLoop *Loop,
                                     int64_t *TripCnt) const {

  // Empty loop.
  if (!Loop->hasChildren()) {
    return false;
  }

  // Ignore loops with SIMD directive.
  if (Loop->isSIMD()) {
    return false;
  }

  // Handle normalized loops only.
  if (!Loop->isNormalized()) {
    return false;
  }

  int64_t ConstTripCount = TripCountCache[Loop->getNestingLevel()];
  assert(ConstTripCount && " Zero Trip loop found.");

  int64_t TotalTripCnt =
      Loop->isInnermost() ? ConstTripCount : ConstTripCount * (*TripCnt);

  DEBUG(dbgs() << " Const Trip Count: " << ConstTripCount << "\n");
  if (TotalTripCnt > CurrentTripThreshold) {
    DEBUG(dbgs() << "TotalTripCnt:" << TotalTripCnt << "\n");
    return false;
  }

  // Update the child trip count for outer loops.
  *TripCnt = TotalTripCnt;

  // Ignore loops which have switch or function calls for unrolling.
  if (HLNodeUtils::hasSwitchOrCall(Loop->getFirstChild(), Loop->getLastChild(),
                                   false)) {
    return false;
  }

  return true;
}

// Transform (Complete Unroll) each loop inside the CandidateLoops vector
void HIRCompleteUnroll::transformLoops() {

  SmallVector<int64_t, MaxLoopNestLevel> TripValues;

  // Transform the loop nest from outer to inner.
  for (auto &Loop : CandidateLoops) {
    transformLoop(Loop, Loop, TripValues);
  }
}

int64_t computeUB(HLLoop *Loop, HLLoop *OuterLoop,
                  SmallVectorImpl<int64_t> &TripValues) {
  int64_t UBVal = 0;

  CanonExpr *UBCE = Loop->getUpperCanonExpr();
  if (UBCE->isIntConstant(&UBVal)) {
    return UBVal;
  }

  UBCE = UBCE->clone();
  int64_t LoopLevel = OuterLoop->getNestingLevel();
  for (auto TripV : TripValues) {
    UBCE->replaceIVByConstant(LoopLevel, TripV);
    LoopLevel++;
  }

  bool isIntConst = UBCE->isIntConstant(&UBVal);
  (void)isIntConst;
  assert(isIntConst && " Upper Bound is not a constant after IV substitution.");
  return UBVal;
}

// Complete Unroll the given Loop, using provided LD as helper data
void HIRCompleteUnroll::transformLoop(HLLoop *&Loop, HLLoop *OuterLoop,
                                      SmallVectorImpl<int64_t> &TripValues) {

  // Guard against the scanning phase setting it appropriately.
  assert(Loop && " Loop is null.");

  // Container for cloning body.
  HLContainerTy LoopBody;

  CanonExprVisitor CEVisit(OuterLoop, TripValues);

  int64_t LB = Loop->getLowerCanonExpr()->getConstant();
  int64_t UB = computeUB(Loop, OuterLoop, TripValues);
  int64_t Step = Loop->getStrideCanonExpr()->getConstant();

  // Extract Preheader and postexit if there is atleast one trip.
  // Since, we work on normalized loops, checking UB+1 is sufficient.
  // TODO: Extend it for unnormalized loops.
  if ((UB + 1) > 0) {
    // Store the node pointers.
    HLNode *PreStart = Loop->getFirstPreheaderNode();
    HLNode *PreEnd = Loop->getLastPreheaderNode();
    HLNode *PostStart = Loop->getFirstPostexitNode();
    HLNode *PostEnd = Loop->getLastPostexitNode();

    // Ztt is not needed since it has ateast one trip.
    Loop->removeZtt();
    Loop->extractPreheaderAndPostexit();
    if (PreStart) {
      HLNodeUtils::visitRange<true, false>(CEVisit, PreStart, PreEnd);
    }
    if (PostStart) {
      HLNodeUtils::visitRange<true, false>(CEVisit, PostStart, PostEnd);
    }
  }

  // Iterate over Loop Child for unrolling with trip value incremented
  // each time. Thus, loop body will be expanded by no. of stmts x TripCount.
  for (int64_t TripVal = LB; TripVal <= UB; TripVal += Step) {
    // Clone iteration
    HLNodeUtils::cloneSequence(&LoopBody, Loop->getFirstChild(),
                               Loop->getLastChild());

    // Store references as LoopBody will be empty after insertion.
    HLNode *CurFirstChild = &(LoopBody.front());
    HLNode *CurLastChild = &(LoopBody.back());

    HLNodeUtils::insertBefore(Loop, &LoopBody);

    // Trip Values vector is used to store the current IV
    // trip value for substitution inside the canon expr.
    TripValues.push_back(TripVal);

    // Update the CanonExpr
    CanonExprVisitor CEVisit(OuterLoop, TripValues);
    HLNodeUtils::visitRange<true, false>(CEVisit, CurFirstChild, CurLastChild);

    TripValues.pop_back();
  }

  // DEBUG(dbgs() << " \n After transformation \n");
  // DEBUG(Loop->getParentRegion()->dump());

  assert(Loop->getParentRegion() && " Loop does not have a parent region.");
  Loop->getParentRegion()->setGenCode();

  // Invalidate all analysis for the loop.
  HIRInvalidationUtils::invalidateBody(Loop);

  // Delete the original loop.
  HLNodeUtils::erase(Loop);
  Loop = nullptr;
}

void HIRCompleteUnroll::releaseMemory() { CandidateLoops.clear(); }
