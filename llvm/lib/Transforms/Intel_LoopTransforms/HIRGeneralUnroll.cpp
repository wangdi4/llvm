//===-- HIRGeneralUnroll.cpp - Implements GeneralUnroll class -------------===//
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
// 2) Add a better heuristics for unrolling when platform characteristics are
//    supported.
// 3) Mark loops as modified for DD, which were transformed.
// 4) Update the reduction chain.
// 5) Add guard conditions for Preheader and Postexit. Refer older code.
//    e.g. if(t>0) then enter the unrolled loop.
// 6) Extend General Unrolling for cases where loop is not normalized.
// 7) Ztt support is added in unrolling. Add a working test case when utility
//    is added.
// 8) The Ztt of remainder loop can be avoided if we set t=(N-1)/8. Currently,
//    adding primary unrolled loop as focus. In this case, the remainder loop
//    is always executed. Investigate whether this version is better in
//    performance as compared to the existing one.

#include "llvm/ADT/Statistic.h"

#include "llvm/IR/Function.h"
#include "llvm/IR/InstrTypes.h"

#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/MathExtras.h"
#include "llvm/Support/raw_ostream.h"

#include "llvm/Analysis/Intel_LoopAnalysis/HIRFramework.h"
#include "llvm/Analysis/Intel_LoopAnalysis/HIRLoopResource.h"

#include "llvm/Transforms/Intel_LoopTransforms/HIRTransformPass.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/CanonExprUtils.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/DDRefUtils.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/HIRInvalidationUtils.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/HIRLoopTransformUtils.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/HLNodeUtils.h"

#define DEBUG_TYPE "hir-general-unroll"

using namespace llvm;
using namespace llvm::loopopt;

const unsigned DefaultMaxUnrollFactor = 8;
const unsigned AbsoluteMaxUnrollFactor = 16;

STATISTIC(LoopsGenUnrolled, "Number of HIR loops general unrolled");

static cl::opt<bool>
    DisableHIRGeneralUnroll("disable-hir-general-unroll", cl::init(false),
                            cl::Hidden,
                            cl::desc("Disable HIR Loop General Unrolling"));

// This is the maximum unroll factor that we use for any loop.
static cl::opt<unsigned> MaxUnrollFactor(
    "hir-general-unroll-max-factor", cl::init(DefaultMaxUnrollFactor),
    cl::Hidden, cl::desc("Max unroll factor for loops (should be power of 2)"));

// This is the minimum trip count threshold.
static cl::opt<unsigned> MinTripCountThreshold(
    "hir-general-unroll-min-trip-count-threshold", cl::init(32), cl::Hidden,
    cl::desc("Min trip count of loops which can be unrolled (absolute minimum "
             "depends on max unroll factor)"));

// This determines the unroll factor of loops inside the loopnest.
static cl::opt<unsigned>
    MaxUnrolledLoopCost("hir-general-unroll-max-unrolled-loop-cost",
                        cl::init(180), cl::Hidden,
                        cl::desc("Max allowed cost of the loop with the "
                                 "unroll factor factored in"));

static cl::opt<unsigned> MaxLoopCost(
    "hir-general-unroll-max-loop-cost", cl::init(50), cl::Hidden,
    cl::desc("Max allowed cost of the original loop which is to be unrolled"));

/// \brief Visitor to update the CanonExpr.
namespace {
class CanonExprVisitor final : public HLNodeVisitorBase {
private:
  unsigned Level;
  unsigned UnrollFactor;
  unsigned UnrollCnt;

  void processRegDDRef(RegDDRef *RegDD);
  void processCanonExpr(CanonExpr *CExpr);

public:
  CanonExprVisitor(unsigned L, unsigned UFactor, unsigned UCnt)
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

  HIRGeneralUnroll() : HIRTransformPass(ID) {
    initializeHIRGeneralUnrollPass(*PassRegistry::getPassRegistry());
  }

  bool runOnFunction(Function &F) override;
  void releaseMemory() override;

  void getAnalysisUsage(AnalysisUsage &AU) const {
    AU.setPreservesAll();
    AU.addRequiredTransitive<HIRFramework>();
    AU.addRequiredTransitive<HIRLoopResource>();
  }

private:
  HIRLoopResource *HLR;

  bool IsUnrollTriggered;

  /// Processes and santitizes command line options.
  void sanitizeOptions();

  /// \brief Main method to be invoked after all the innermost loops
  /// are gathered.
  void processGeneralUnroll(SmallVectorImpl<HLLoop *> &CandidateLoops);

  /// Computes and returns unroll factor for the loop using cost model. Returns
  /// 0 as an invalid unroll factor.
  unsigned computeUnrollFactor(const HLLoop *HLoop) const;

  /// Returns true if we can attempt to unroll this loop.
  bool isApplicable(const HLLoop *Loop) const;

  /// \brief Determines if Unrolling is profitable for the given Loop.
  bool isProfitable(const HLLoop *Loop, unsigned *UnrollFactor) const;

  /// \brief High level method which gives call to other sub-methods.
  void transformLoop(HLLoop *OrigLoop, unsigned UnrollFactor);

  /// \brief Performs the actual unrolling.
  void processUnrollLoop(HLLoop *OrigLoop, HLLoop *UnrollLoop,
                         unsigned UnrollFactor);
};
}

char HIRGeneralUnroll::ID = 0;
INITIALIZE_PASS_BEGIN(HIRGeneralUnroll, "hir-general-unroll",
                      "HIR General Unroll", false, false)
INITIALIZE_PASS_DEPENDENCY(HIRFramework)
INITIALIZE_PASS_DEPENDENCY(HIRLoopResource)
INITIALIZE_PASS_END(HIRGeneralUnroll, "hir-general-unroll",
                    "HIR General Unroll", false, false)

FunctionPass *llvm::createHIRGeneralUnrollPass() {
  return new HIRGeneralUnroll();
}

bool HIRGeneralUnroll::runOnFunction(Function &F) {
  // Skip if DisableHIRGeneralUnroll is enabled
  if (DisableHIRGeneralUnroll || skipFunction(F)) {
    DEBUG(dbgs() << "HIR LOOP General Unroll Transformation Disabled \n");
    return false;
  }

  DEBUG(dbgs() << "General unrolling for Function : " << F.getName() << "\n");

  HLR = &getAnalysis<HIRLoopResource>();

  IsUnrollTriggered = false;

  sanitizeOptions();

  // Gather the innermost loops as candidates.
  SmallVector<HLLoop *, 64> CandidateLoops;
  HLNodeUtils::gatherInnermostLoops(CandidateLoops);

  // Process General Unrolling
  processGeneralUnroll(CandidateLoops);

  return IsUnrollTriggered;
}

// Nothing to release?
void HIRGeneralUnroll::releaseMemory() {}

void HIRGeneralUnroll::sanitizeOptions() {

  // Set a sane unroll factor.
  if (MaxUnrollFactor < 2) {
    MaxUnrollFactor = 2;

  } else if (MaxUnrollFactor > AbsoluteMaxUnrollFactor) {
    MaxUnrollFactor = AbsoluteMaxUnrollFactor;

  } else if (!isPowerOf2_32(MaxUnrollFactor)) {
    MaxUnrollFactor = DefaultMaxUnrollFactor;
  }

  // Set a sane minimum trip threshold.
  unsigned MinExpectedThreshold = (2 * MaxUnrollFactor);

  if (MinTripCountThreshold < MinExpectedThreshold) {
    MinTripCountThreshold = MinExpectedThreshold;
  }
}

/// processGeneralUnroll - Main routine to perform unrolling.
/// First, performs cost analysis and then do the transformation.
void HIRGeneralUnroll::processGeneralUnroll(
    SmallVectorImpl<HLLoop *> &CandidateLoops) {

  // Visit each candidate loop to run cost analysis.
  for (auto &Loop : CandidateLoops) {
    unsigned UnrollFactor = 0;

    // Perform a cost/profitability analysis on the loop
    // If all conditions are met, unroll it.
    if (isApplicable(Loop) && isProfitable(Loop, &UnrollFactor)) {
      transformLoop(Loop, UnrollFactor);
      IsUnrollTriggered = true;
      LoopsGenUnrolled++;
    }
  }
}

unsigned HIRGeneralUnroll::computeUnrollFactor(const HLLoop *HLoop) const {
  auto SelfResource = HLR->getSelfLoopResource(HLoop);

  unsigned SelfCost = SelfResource.getTotalCost();

  // Exit if loop exceeds threshold.
  if (SelfCost > MaxLoopCost) {
    return 0;
  }

  // Exit if loop with minimum unroll factor of 2 exceeds threshold.
  if ((2 * SelfCost) > MaxUnrolledLoopCost) {
    return 0;
  }

  unsigned UnrollFactor = MaxUnrollFactor;

  while ((UnrollFactor * SelfCost) > MaxUnrolledLoopCost) {
    UnrollFactor /= 2;
  }

  assert(UnrollFactor >= 2 && "Unexpected unroll factor!");

  return UnrollFactor;
}

bool HIRGeneralUnroll::isApplicable(const HLLoop *Loop) const {

  // TODO: Preheader and PostExit not handled currently.
  if (Loop->hasPreheader() || Loop->hasPostexit()) {
    return false;
  }

  if (!Loop->hasChildren()) {
    return false;
  }

  // Ignore loops with SIMD directive.
  if (Loop->isSIMD()) {
    return false;
  }

  // Loop should be normalized before this pass
  // TODO: Decide whether we can remove this, just to save compile time.
  if (!Loop->isNormalized() || Loop->isUnknown()) {
    return false;
  }

  uint64_t TripCount;

  if ((Loop->isConstTripLoop(&TripCount) ||
       (TripCount = Loop->getMaxTripCountEstimate())) &&
      (TripCount < MinTripCountThreshold)) {
    return false;
  }

  return true;
}

bool HIRGeneralUnroll::isProfitable(const HLLoop *Loop,
                                    unsigned *UnrollFactor) const {

  // Ignore loops which have switch or function calls for unrolling.
  if (HLNodeUtils::hasSwitchOrCall(Loop->getFirstChild(),
                                   Loop->getLastChild())) {
    return false;
  }

  // Determine unroll factor of the loop.
  if ((*UnrollFactor = computeUnrollFactor(Loop)) == 0) {
    return false;
  }

  return true;
}

// transformLoop - Perform the unrolling transformation for
// the given loop.
void HIRGeneralUnroll::transformLoop(HLLoop *OrigLoop, unsigned UnrollFactor) {

  DEBUG(dbgs() << "\t GeneralUnroll Loop: ");
  DEBUG(OrigLoop->dump());

  bool NeedRemainderLoop = false;

  // Create the unrolled main loop and setup remainder loop.
  HLLoop *UnrollLoop = 
    HIRLoopTransformUtils::setupMainAndRemainderLoops(OrigLoop, UnrollFactor,
                                                      NeedRemainderLoop);

  processUnrollLoop(OrigLoop, UnrollLoop, UnrollFactor);

  // If a remainder loop is not needed get rid of the OrigLoop at this point.
  if (!NeedRemainderLoop) {
    HLNodeUtils::remove(OrigLoop);
  }

  DEBUG(dbgs() << "\n\t Transformed GeneralUnroll Loops No:"
               << LoopsGenUnrolled);
  DEBUG(UnrollLoop->dump());
}

void HIRGeneralUnroll::processUnrollLoop(HLLoop *OrigLoop, HLLoop *UnrollLoop,
                                         unsigned UnrollFactor) {

  // Container for cloning body.
  HLContainerTy LoopBody;

  // Loop through the 0th iteration unrolled loop children and create new
  // children
  // with updated References based on unroll factor.
  for (unsigned UnrollCnt = 0; UnrollCnt < UnrollFactor; ++UnrollCnt) {

    // Clone 0th iteration
    HLNodeUtils::cloneSequence(&LoopBody, OrigLoop->getFirstChild(),
                               OrigLoop->getLastChild());

    // Store references as LoopBody will be empty after insertion.
    HLNode *CurFirstChild = &(LoopBody.front());
    HLNode *CurLastChild = &(LoopBody.back());

    HLNodeUtils::insertAsLastChildren(UnrollLoop, &LoopBody);

    CanonExprVisitor CEVisit(UnrollLoop->getNestingLevel(), UnrollFactor,
                             UnrollCnt);
    HLNodeUtils::visitRange(CEVisit, CurFirstChild, CurLastChild);
  }
}
