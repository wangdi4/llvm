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

#include "llvm/ADT/SmallSet.h"
#include "llvm/ADT/Statistic.h"

#include "llvm/IR/Function.h"
#include "llvm/IR/InstrTypes.h"

#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/MathExtras.h"
#include "llvm/Support/raw_ostream.h"

#include "llvm/Analysis/Intel_LoopAnalysis/HIRFramework.h"
#include "llvm/Analysis/Intel_LoopAnalysis/HIRLoopResource.h"
#include "llvm/Analysis/Intel_LoopAnalysis/HIRLoopStatistics.h"

#include "llvm/Transforms/Intel_LoopTransforms/HIRTransformPass.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/HIRTransformUtils.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/HLNodeUtils.h"

#include "HIRUnroll.h"

#define DEBUG_TYPE "hir-general-unroll"

using namespace llvm;
using namespace llvm::loopopt;
using namespace llvm::loopopt::unroll;

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
    AU.addRequiredTransitive<HIRLoopStatistics>();
  }

private:
  HIRLoopResource *HLR;
  HIRLoopStatistics *HLS;

  bool IsUnrollTriggered;

  /// Processes and santitizes command line options.
  void sanitizeOptions();

  /// Main method to be invoked after all the innermost loops are gathered.
  void processGeneralUnroll(SmallVectorImpl<HLLoop *> &CandidateLoops);

  /// Computes and returns unroll factor for the loop using cost model. Returns
  /// 0 as an invalid unroll factor.
  unsigned computeUnrollFactor(const HLLoop *HLoop) const;

  /// Returns true if we can attempt to unroll this loop.
  bool isApplicable(const HLLoop *Loop) const;

  /// Determines if Unrolling is profitable for the given Loop.
  bool isProfitable(const HLLoop *Loop, unsigned *UnrollFactor) const;

  /// Returns a refined unroll factor for the loop based on reuse analysis.
  /// Returns 0 if unrolling is not profitable.
  unsigned refineUnrollFactorUsingReuseAnalysis(const HLLoop *Loop,
                                                unsigned CurUnrollFactor) const;
};
}

char HIRGeneralUnroll::ID = 0;
INITIALIZE_PASS_BEGIN(HIRGeneralUnroll, "hir-general-unroll",
                      "HIR General Unroll", false, false)
INITIALIZE_PASS_DEPENDENCY(HIRFramework)
INITIALIZE_PASS_DEPENDENCY(HIRLoopResource)
INITIALIZE_PASS_DEPENDENCY(HIRLoopStatistics)
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

  auto HIRF = &getAnalysis<HIRFramework>();
  HLR = &getAnalysis<HIRLoopResource>();
  HLS = &getAnalysis<HIRLoopStatistics>();

  IsUnrollTriggered = false;

  sanitizeOptions();

  // Gather the innermost loops as candidates.
  SmallVector<HLLoop *, 64> CandidateLoops;
  HIRF->getHLNodeUtils().gatherInnermostLoops(CandidateLoops);

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
      unrollLoop(Loop, UnrollFactor);
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
  // Ignore loops with SIMD directive.
  if (Loop->isSIMD()) {
    return false;
  }

  bool IsUnknown = Loop->isUnknown();

  // Loop should be normalized before this pass
  // TODO: Decide whether we can remove this, just to save compile time.
  if (!IsUnknown && !Loop->isNormalized()) {
    return false;
  }

  uint64_t TripCount;

  if (((!IsUnknown && Loop->isConstTripLoop(&TripCount)) ||
       (TripCount = Loop->getMaxTripCountEstimate())) &&
      (TripCount < MinTripCountThreshold)) {
    return false;
  }

  return true;
}

bool HIRGeneralUnroll::isProfitable(const HLLoop *Loop,
                                    unsigned *UnrollFactor) const {

  const LoopStatistics &LS = HLS->getSelfLoopStatistics(Loop);

  if (LS.hasSwitches() || LS.hasCalls()) {
    return false;
  }

  // Determine unroll factor of the loop.
  if ((*UnrollFactor = computeUnrollFactor(Loop)) == 0) {
    return false;
  }

  if ((*UnrollFactor =
           refineUnrollFactorUsingReuseAnalysis(Loop, *UnrollFactor)) == 0) {
    return false;
  }

  return true;
}

// Checks if we encounter backward uses of temp copies (register moves). If so,
// they can be eliminated by unrolling. For example, if we have the following
// loop body-
// t3 = t2 + A[i]
// t1 = B[i] + C[i]
// t2 = t1
//
// We can eliminate t2 = t1 by unrolling the loop and forward substituting t2 in
// first statement.
//
// We need to penalize backward uses if they are not coming from copy temps.
// This is because they may be part of a def-use chain. Unrolling def-use chains
// can add register pressure. For example-
// t1 = t2 + A[i]
// t2 = t1 + B[i]
//
// TODO: Add temporal locality analysis?
class ReuseAnalyzer final : public HLNodeVisitorBase {
private:
  SmallSet<unsigned, 16> RvalTempBlobSymbases;
  int Reuse;
  bool CyclicalDefUse;

public:
  ReuseAnalyzer() : Reuse(0), CyclicalDefUse(false) {}

  void visit(const HLDDNode *Node);

  void visit(const HLNode *Node) {}
  void postVisit(const HLNode *Node) {}

  bool hasReuse() const { return (Reuse > 0); }

  bool hasCyclicalDefUse() const { return CyclicalDefUse; }
};

void ReuseAnalyzer::visit(const HLDDNode *Node) {

  bool HasTerminalLval = false;
  auto LvalRef = Node->getLvalDDRef();
  unsigned LvalSymbase = InvalidSymbase;

  if (LvalRef && LvalRef->isTerminalRef()) {

    LvalSymbase = LvalRef->getSymbase();

    if (cast<HLInst>(Node)->isCopyInst()) {
      if (RvalTempBlobSymbases.count(LvalSymbase)) {
        ++Reuse;
      }
      // No more processing needed for copy instructions.
      return;
    }

    HasTerminalLval = true;
  }

  SmallVector<unsigned, 16> Symbases;

  auto RefIt =
      HasTerminalLval ? Node->rval_op_ddref_begin() : Node->op_ddref_begin();
  auto End = HasTerminalLval ? Node->rval_op_ddref_end() : Node->op_ddref_end();

  for (; RefIt != End; ++RefIt) {
    (*RefIt)->populateTempBlobSymbases(Symbases);
  }

  for (auto SB : Symbases) {
    RvalTempBlobSymbases.insert(SB);
  }

  // We are not really confirming a def-use cycle (in the interest of compile
  // time). This is conservative behavior so it can lead to missed unrolling
  // opportunities which is better than causing performance degradations.
  // TODO: refine the logic.
  if (HasTerminalLval && RvalTempBlobSymbases.count(LvalSymbase)) {
    CyclicalDefUse = true;
    --Reuse;
  }
}

unsigned HIRGeneralUnroll::refineUnrollFactorUsingReuseAnalysis(
    const HLLoop *Loop, unsigned CurUnrollFactor) const {
  // Profitability for unknown loops is tighter than do loops because unlike do
  // loops we cannot save bottom test computation for unknown loops.
  // TODO: add similar model for do loops?
  if (!Loop->isUnknown()) {
    return CurUnrollFactor;
  }

  ReuseAnalyzer RA;

  HLNodeUtils::visitRange(RA, Loop->child_begin(), Loop->child_end());

  if (!RA.hasReuse()) {
    // Loop is not profitable.
    return 0;

  } else if (RA.hasCyclicalDefUse()) {
    // Unrolling can still be profitable with cyclical def use if there is more
    // reuse. To minimize register pressure, we only unroll by 2.
    return 2;
  }

  return CurUnrollFactor;
}
