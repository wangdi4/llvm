//===-- HIRGeneralUnroll.cpp - Implements GeneralUnroll class -------------===//
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
// This file implements HIRGeneralUnroll class which unrolls a HIR loop
// with significantly larger trip count.
//
// For example:
//
// Original Loop                     Transformed ( UnrollFactor=8)
// for(i=0; i<N; i++)                t = (int)(N/8);
//    A[i] = B[i];                   for(iu=0; iu<=(t-1) ; iu++) {
//                                     A[iu*8] = B[iu*8];
//                                     ...
//                                     A[iu*8+7] = B[iu*8+7];
//
//                                   }
//                                   for(i=8*t; i<N; i++)
//                                     A[i] = B[i];
//
//                                    Note: 't' is avoided if N is constant
//
// The general algorithm is as follows:
//  1. Visit the Region
//  2. Extract the innermost loops
//  3. For each innermost loop
//    3.1 Get Trip Count and perform cost analysis. Ignore loops where not
//          profitable.
//    3.2 If Trip Count < Threshold, ignore this loop
//    3.3 Create a new Unrolled Loop
//    3.4 For UnrollCnt from [0 to UnrollFactor)
//          3.4.1 Append Cloned Original Loop Children into UnrolledLoop
//          3.4.2 Update Canon Exprs (IV*UnrollFactor + Coeff*UnrollCnt)
//                of UnrolledLoop Children.
//    3.5 Modify Original Loop to Remainder Loop with updated LowerBound
//        3.5.1 If Original Loop is Constant and TripCount%UnrollFactor = 0
//              Delete Original Loop as Remainder Loop is not needed.
//
// General Unrolling would increase the register pressure based on the unroll
// factor. Current heuristic just uses trip count to determine if loop needs
// to be unrolled.
//
//===----------------------------------------------------------------------===//

// TODO:
// 1) Optimize the remainder loop to produce switch statements. Think about
//    removing remainder loop if it is 1-trip for constant trip count loops.
// 2) Perform general unrolling for symbolic UpperBounds (e.g. 'N').
// 3) Add a better heuristics for unrolling when platform characteristics are
//    supported.
// 4) Mark loops as modified for DD, which were transformed.
// 5) Update the reduction chain.
// 6) Add guard conditions for Preheader and Postexit. Refer older code.
//    e.g. if(t>0) then enter the unrolled loop.
// 7) Extend General Unrolling for cases where loop is not normalized.

#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"

#include "llvm/Analysis/Intel_LoopAnalysis/HIRParser.h"
#include "llvm/Analysis/Intel_LoopAnalysis/DDAnalysis.h"

#include "llvm/Transforms/Intel_LoopTransforms/HIRTransformPass.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/CanonExprUtils.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/DDRefUtils.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/HLNodeUtils.h"

#define DEBUG_TYPE "hir-generalunroll"

using namespace llvm;
using namespace llvm::loopopt;

// TODO: This should be modified to a better heuristic.
static cl::opt<unsigned> GeneralUnrollTripThreshold(
    "genunroll-trip-threshold", cl::init(100), cl::Hidden,
    cl::desc("Don't unroll if innermost trip count is lesser than this,"
             "threshold."));

static cl::opt<unsigned>
    GeneralUnrollFactor("genunroll-factor", cl::init(8), cl::Hidden,
                        cl::desc("General Unrolling Factor for HIR Loop's."));

/// \brief Visitor to update the CanonExpr.
namespace {
class CanonExprVisitor {
private:
  unsigned Level;
  unsigned UnrollFactor;
  int64_t UnrollCnt;

  void processRegDDRef(RegDDRef *RegDD);
  void processCanonExpr(CanonExpr *CExpr);

public:
  CanonExprVisitor(unsigned L, unsigned UFactor, int64_t UCnt)
      : Level(L), UnrollFactor(UFactor), UnrollCnt(UCnt) {}

  /// \brief No processing needed for Goto
  void visit(HLGoto *Goto){};
  /// \brief No processing needed for Label
  void visit(HLLabel *Label){};
  void visit(HLDDNode *Node);
  void visit(HLNode *Node) {
    llvm_unreachable(" Node not supported for unrolling.");
  };
  void postVisit(HLNode *Node) {}
  bool isDone() { return false; }
  bool skipRecursion(HLNode *Node) { return false; }
};

} // namespace

void CanonExprVisitor::visit(HLDDNode *Node) {

  // Only expecting if and inst inside the innermost loops.
  // Primarily to catch errors of other types.
  assert((isa<HLIf>(Node) || isa<HLInst>(Node)) && " Node not supported for "
                                                   "unrolling.");

  for (auto Iter = Node->ddref_begin(), End = Node->ddref_end(); Iter != End;
       ++Iter) {
    processRegDDRef(*Iter);
  }
}

/// processRegDDRef - Processes RegDDRef to call the Canon Exprs
/// present inside it.
/// This is an internal helper function.
void CanonExprVisitor::processRegDDRef(RegDDRef *RegDD) {

  // Process CanonExprs inside the RegDDRefs
  for (auto Iter = RegDD->canon_begin(), End = RegDD->canon_end(); Iter != End;
       ++Iter) {
    processCanonExpr(*Iter);
  }

  // Process GEP Base
  if (RegDD->hasGEPInfo()) {
    processCanonExpr(RegDD->getBaseCE());
  }

  // Process GEP Strides
  for (auto Iter = RegDD->stride_begin(), End = RegDD->stride_end();
       Iter != End; ++Iter) {
    processCanonExpr(*Iter);
  }
}

/// processCanonExpr - Processes CanonExpr to modify IV to
/// IV*UF + (Original IVCoeff)*UnrollCnt.
/// This is an internal helper function.
void CanonExprVisitor::processCanonExpr(CanonExpr *CExpr) {

  // Shift the canon expr to create the offset.
  if (UnrollCnt)
    CExpr->shift(Level, UnrollCnt);

  // IV*UF .
  CExpr->multiplyIVByConstant(Level, UnrollFactor);
}

namespace {

class HIRGeneralUnroll : public HIRTransformPass {
public:
  static char ID;

  HIRGeneralUnroll(int T = -1, int UFactor = -1) : HIRTransformPass(ID) {
    initializeHIRGeneralUnrollPass(*PassRegistry::getPassRegistry());

    // TODO: Decide on whether we need to expose parameters to users.
    // Also, bounds for these parameters.
    CurrentTripThreshold = (T == -1) ? GeneralUnrollTripThreshold : unsigned(T);
    UnrollFactor = (UFactor == -1) ? GeneralUnrollFactor : unsigned(UFactor);
  }

  bool runOnFunction(Function &F) override;
  void releaseMemory() override;

  void getAnalysisUsage(AnalysisUsage &AU) const {
    AU.setPreservesAll();
    AU.addRequiredTransitive<HIRParser>();
    AU.addRequiredTransitive<DDAnalysis>();
  }

private:
  unsigned CurrentTripThreshold;
  unsigned UnrollFactor;

  SmallVector<const HLLoop *, 64> InnermostLoops;

  /// \brief Main method to be invoked after all the innermost loops
  /// are gathered.
  void processGeneralUnroll();
  /// \brief Determines if Unrolling is profitable for the given Loop.
  bool isProfitable(const HLLoop *Loop, bool *IsConstLoop, int64_t *TripCount);
  /// \brief High level method which gives call to other sub-methods.
  void transformLoop(HLLoop *OrigLoop, bool IsConstLoop, int64_t TripCount);
  /// \brief Performs the actual unrolling.
  void processUnrollLoop(HLLoop *OrigLoop, HLLoop *UnrollLoop);
  /// \brief Creates the unrolled loop.
  HLLoop *createUnrollLoop(HLLoop *OrigLoop, bool IsConstLoop,
                           int64_t TripCount, int64_t *NewUB);
  /// \brief Processes the remainder loop and determines if it necessary.
  void processRemainderLoop(HLLoop *OrigLoop, bool IsConstLoop,
                            int64_t TripCount, int64_t NewUB);
};
}

char HIRGeneralUnroll::ID = 0;
INITIALIZE_PASS_BEGIN(HIRGeneralUnroll, "HIRGeneralUnroll",
                      "HIR General Unroll", false, false)
INITIALIZE_PASS_DEPENDENCY(HIRParser)
INITIALIZE_PASS_END(HIRGeneralUnroll, "HIRGeneralUnroll", "HIR General Unroll",
                    false, false)

FunctionPass *llvm::createHIRGeneralUnrollPass(int Threshold, int UFactor) {
  return new HIRGeneralUnroll(Threshold, UFactor);
}

bool HIRGeneralUnroll::runOnFunction(Function &F) {
  DEBUG(dbgs() << "General unrolling for Function : " << F.getName() << "\n");
  DEBUG(dbgs() << "Trip Count Threshold : " << CurrentTripThreshold << "\n");
  DEBUG(dbgs() << "GeneralUnrollFactor : " << UnrollFactor << "\n");

  // Do an early exit if Trip Threshold is less than 1
  // TODO: Check if we want give some feedback to user
  if (CurrentTripThreshold == 0)
    return false;

  // Gather the innermost loops
  HLNodeUtils::gatherInnermostLoops(&InnermostLoops);

  processGeneralUnroll();

  return false;
}

void HIRGeneralUnroll::releaseMemory() { InnermostLoops.clear(); }

/// processGeneralUnroll - Main routine to perform unrolling.
/// First, performs cost analysis and then do the transformation.
void HIRGeneralUnroll::processGeneralUnroll() {

  int64_t TripCount = 0;
  bool isConstantLoop = false;
  // Visit each innermost loop to run cost analysis
  for (auto Iter = InnermostLoops.begin(), End = InnermostLoops.end();
       Iter != End; ++Iter, TripCount = 0, isConstantLoop = false) {

    const HLLoop *Loop = (*Iter);

    // Perform a cost/profitability analysis on the loop
    // If all conditions are met, unroll it.
    if (isProfitable(Loop, &isConstantLoop, &TripCount))
      transformLoop(const_cast<HLLoop *>(Loop), isConstantLoop, TripCount);
  }
}

/// isProfitable - Check if the loop trip count is less
/// than the trip count threshold. Return true, if this loop
/// is a candidate for general unrolling.
bool HIRGeneralUnroll::isProfitable(const HLLoop *Loop, bool *IsConstLoop,
                                    int64_t *TripCount) {

  // TODO: Preheader and PostExit not handled currently.
  if (Loop->hasPreheader() || Loop->hasPostexit())
    return false;

  // Loop should be normalized before this pass
  // TODO: Call isLoopNormalize() when available?

  assert((Loop->getNumChildren() > 0) && " Loop has no child.");

  const RegDDRef *UBRef = Loop->getUpperDDRef();
  if (!UBRef)
    return false;

  // Check if UB is Constant or not
  int64_t UBConst;
  if (!UBRef->isIntConstant(&UBConst)) {
    // TODO: Handle case where UB is a RegDDRef
    // We would need to create a temporary variable
    // for this purpose. Currently, returning false.
    *IsConstLoop = false;
    return false;
  }

  // TripCount is (Upper + 1) as loop is normalized.
  int64_t ConstTripCount = UBConst + 1;
  if (ConstTripCount < CurrentTripThreshold)
    return false;

  // Ignore loops which have switch or function calls for unrolling.
  if (HLNodeUtils::hasSwitchOrCall(Loop->getFirstChild(), Loop->getLastChild()))
    return false;

  // Set the trip count of this loop for later use in unrolling.
  *TripCount = ConstTripCount;
  *IsConstLoop = true;

  return true;
}

/// transformLoop - Perform the unrolling transformation for
/// the given loop.
void HIRGeneralUnroll::transformLoop(HLLoop *OrigLoop, bool IsConstLoop,
                                     int64_t TripCount) {

  // Create the unrolled main loop.
  int64_t NewUB = 0;
  HLLoop *UnrollLoop =
      createUnrollLoop(OrigLoop, IsConstLoop, TripCount, &NewUB);

  processUnrollLoop(OrigLoop, UnrollLoop);

  // Update the OrigLoop to remainder loop.
  processRemainderLoop(OrigLoop, IsConstLoop, TripCount, NewUB);

  // TODO: Mark loops as modified for DD
}

HLLoop *HIRGeneralUnroll::createUnrollLoop(HLLoop *OrigLoop, bool IsConstLoop,
                                           int64_t TripCount, int64_t *NewUB) {

  // TODO: Handle case when Loop UB is not constant.

  // TODO: Not sure if we need to add Ztt?
  /// Currently the clone utility handles it.
  HLLoop *NewLoop = OrigLoop->cloneEmptyLoop();
  NewLoop->setNumExits((OrigLoop->getNumExits() - 1) * UnrollFactor + 1);

  // Update the loop upper bound.
  *NewUB = (int64_t)(TripCount / UnrollFactor) - 1;
  NewLoop->getUpperCanonExpr()->setConstant(*NewUB);

  // TODO: Set the innermost flag
  HLNodeUtils::insertBefore(OrigLoop, NewLoop);

  // TODO: Handle innermost flag for multi-level loop nest
  // Set the code gen for modified region
  NewLoop->getParentRegion()->setGenCode();

  return NewLoop;
}

void HIRGeneralUnroll::processUnrollLoop(HLLoop *OrigLoop, HLLoop *UnrollLoop) {

  // Container for cloning body.
  HLContainerTy LoopBody;

  // Loop through the 0th iteration unrolled loop children and create new
  // children
  // with updated References based on unroll factor.
  for (int64_t UnrollCnt = 0; UnrollCnt < UnrollFactor; ++UnrollCnt) {

    // Clone 0th iteration
    HLNodeUtils::cloneSequence(&LoopBody, OrigLoop->getFirstChild(),
                               OrigLoop->getLastChild());

    // Store references as LoopBody will be empty after insertion.
    HLNode *CurFirstChild = &(LoopBody.front());
    HLNode *CurLastChild = &(LoopBody.back());

    HLNodeUtils::insertAsLastChildren(UnrollLoop, &LoopBody);

    CanonExprVisitor CEVisit(UnrollLoop->getNestingLevel(), UnrollFactor,
                             UnrollCnt);
    HLNodeUtils::visit<CanonExprVisitor>(CEVisit, CurFirstChild, CurLastChild);
  }
}

void HIRGeneralUnroll::processRemainderLoop(HLLoop *OrigLoop, bool IsConstLoop,
                                            int64_t TripCount, int64_t NewUB) {

  // Check if the Remainder Loop is necessary.
  // This condition occurs when the original constant Trip Count is divided by
  // UnrollFactor
  // without a remainder.
  if (IsConstLoop && (TripCount % UnrollFactor == 0)) {
    HLNodeUtils::erase(OrigLoop);
    return;
  }

  // Modify the LB of original loop
  // TODO: Handle case where original Loop UB is not constant.
  RegDDRef *OrigLBRef = OrigLoop->getLowerDDRef();
  CanonExpr *LBCE = OrigLBRef->getSingleCanonExpr();
  LBCE->setConstant((NewUB + 1) * UnrollFactor);
}
