//===-- HIRGeneralUnroll.cpp - Implements GeneralUnroll class -------------===//
//
// Copyright (C) 2015-2019 Intel Corporation. All rights reserved.
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

#include "llvm/Transforms/Intel_LoopTransforms/HIRGeneralUnroll.h"

#include "llvm/ADT/SmallSet.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/ADT/Triple.h"

#include "llvm/IR/Function.h"
#include "llvm/IR/InstrTypes.h"

#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/MathExtras.h"
#include "llvm/Support/raw_ostream.h"

#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRDDAnalysis.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRLoopResource.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRLoopStatistics.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRSafeReductionAnalysis.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Framework/HIRFramework.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HIRInvalidationUtils.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HLNodeUtils.h"

#include "llvm/Transforms/Intel_LoopTransforms/HIRTransformPass.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/HIRTransformUtils.h"

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
    "hir-general-unroll-max-loop-cost", cl::init(40), cl::Hidden,
    cl::desc("Max allowed cost of the original loop which is to be unrolled"));

static cl::opt<bool> DisableSwitchGeneration(
    "hir-general-unroll-disable-switch-generation", cl::init(false), cl::Hidden,
    cl::desc("Disable switch generation in HIR General Unroll"));

static cl::opt<bool> DisableReplaceByFirstIteration(
    "hir-general-unroll-disable-replace-by-first-iteration", cl::init(false),
    cl::Hidden,
    cl::desc("Disable replace by first iteration in HIR General Unroll"));

namespace {

class HIRGeneralUnroll {
public:
  HIRGeneralUnroll(HIRFramework &HIRF, HIRLoopResource &HLR,
                   HIRDDAnalysis &HDDA, HIRSafeReductionAnalysis &HSRA,
                   HIRLoopStatistics &HLS, bool PragmaOnlyUnroll)
      : HIRF(HIRF), HLR(HLR), HDDA(HDDA), HSRA(HSRA), HLS(HLS),
        IsUnrollTriggered(false), Is32Bit(false),
        PragmaOnlyUnroll(PragmaOnlyUnroll) {}

  bool run();

private:
  HIRFramework &HIRF;
  HIRLoopResource &HLR;
  HIRDDAnalysis &HDDA;
  HIRSafeReductionAnalysis &HSRA;
  HIRLoopStatistics &HLS;

  bool IsUnrollTriggered;
  bool Is32Bit;
  bool PragmaOnlyUnroll;

  /// Processes and santitizes command line options.
  void sanitizeOptions();

  /// Main method to be invoked after all the innermost loops are gathered.
  void processGeneralUnroll(SmallVectorImpl<HLLoop *> &CandidateLoops);

  /// Computes and returns unroll factor for the loop using cost model. Returns
  /// 0 as an invalid unroll factor.
  unsigned computeUnrollFactor(const HLLoop *HLoop,
                               bool HasEnablingPragma) const;

  /// Returns true if we can attempt to unroll this loop.
  bool isApplicable(const HLLoop *Loop) const;

  /// Determines if Unrolling is profitable for the given Loop.
  bool isProfitable(const HLLoop *Loop, bool HasEnablingPragma,
                    unsigned *UnrollFactor) const;

  /// Returns a refined unroll factor for the loop based on reuse analysis.
  /// Returns 0 if unrolling is not profitable.
  unsigned refineUnrollFactorUsingReuseAnalysis(const HLLoop *Loop,
                                                unsigned CurUnrollFactor) const;

  bool isSwitchGenerationLegal(HLLoop *RemainderLoop);

  void replaceBySwitch(HLLoop *RemainderLoop, unsigned UnrollFactor);
};
} // namespace

// Collects loops in post (inner to outer) order.
struct PostLoopCollector final : public HLNodeVisitorBase {
  SmallVector<HLLoop *, 64> CandidateLoops;
  HLNode *SkipNode = nullptr;

  void visit(HLNode *Node) {}
  void postVisit(HLNode *Node) {}

  void postVisit(HLLoop *Loop) {
    if (Loop->isInnermost()) {
      CandidateLoops.push_back(Loop);
      SkipNode = Loop;
    } else if (Loop->hasGeneralUnrollEnablingPragma()) {
      CandidateLoops.push_back(Loop);
    }
  }

  bool skipRecursion(HLNode *Node) { return Node == SkipNode; }
};

struct IVUpdater final : public HLNodeVisitorBase {
  int UnrollNum;
  unsigned LoopLevel;

  IVUpdater(int UnrollNum, unsigned LoopLevel)
      : UnrollNum(UnrollNum), LoopLevel(LoopLevel) {}

  void visit(HLDDNode *Node);
  void visit(const HLNode *Node) {}
  void postVisit(const HLNode *Node) {}
};

void IVUpdater::visit(HLDDNode *Node) {
  for (auto It = Node->ddref_begin(), Et = Node->ddref_end(); It != Et; It++) {
    RegDDRef *OpRef = *It;

    OpRef->replaceIVByConstant(LoopLevel, UnrollNum);

    OpRef->makeConsistent({}, LoopLevel - 1);
  }
}

bool HIRGeneralUnroll::run() {
  // Skip if DisableHIRGeneralUnroll is enabled
  if (DisableHIRGeneralUnroll) {
    LLVM_DEBUG(dbgs() << "HIR LOOP General Unroll Transformation Disabled \n");
    return false;
  }

  LLVM_DEBUG(dbgs() << "General unrolling for Function : "
                    << HIRF.getFunction().getName() << "\n");

  IsUnrollTriggered = false;

  llvm::Triple TargetTriple(HIRF.getModule().getTargetTriple());
  Is32Bit = (TargetTriple.getArch() == llvm::Triple::x86);

  sanitizeOptions();

  // Gather the innermost loops as candidates.
  PostLoopCollector PLC;

  HIRF.getHLNodeUtils().visitAll(PLC);

  // Process General Unrolling
  processGeneralUnroll(PLC.CandidateLoops);

  return IsUnrollTriggered;
}

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

    bool HasEnablingPragma = Loop->hasGeneralUnrollEnablingPragma();

    // Disable all other unrolling if only pragma enabled unrolling is allowed.
    if (PragmaOnlyUnroll && !HasEnablingPragma) {
      continue;
    }

    // Perform a cost/profitability analysis on the loop
    // If all conditions are met, unroll it.
    if (isApplicable(Loop) &&
        isProfitable(Loop, HasEnablingPragma, &UnrollFactor)) {
      HLLoop *UnrolledLoop, *RemainderLoop;

      unrollLoop(Loop, UnrollFactor, &UnrolledLoop, &RemainderLoop);

      // Following logic will not be needed once we disable LLVM's loop unroll
      // pass after LoopOpt.
      // Add disabling pragma to unrolled loop.
      UnrolledLoop->markDoNotUnroll();

      IsUnrollTriggered = true;
      LoopsGenUnrolled++;

      if (!RemainderLoop) {
        continue;
      }

      if (RemainderLoop->isConstTripLoop()) {
        // TODO: Perform complete unroll
      } else if (UnrollFactor == 2) {
        // Replace the remainder loop with the first iteration when the unroll
        // factor is 2, because the remainder loop has at most one iteration
        if (DisableReplaceByFirstIteration) {
          continue;
        }

        RemainderLoop->replaceByFirstIteration();
      } else {
        replaceBySwitch(RemainderLoop, UnrollFactor);
      }
    }
  }
}

static bool isSwitchGenerationProfitable(HLLoop *RemainderLoop,
                                         unsigned UnrollFactor) {
  if (!RemainderLoop->isInnermost()) {
    return false;
  }

  if (UnrollFactor > 8) {
    return false;
  }

  return true;
}

bool HIRGeneralUnroll::isSwitchGenerationLegal(HLLoop *RemainderLoop) {
  bool IsLoopReversible = HIRTransformUtils::isLoopReversible(
      RemainderLoop, HDDA, HSRA, HLS, false, true);

  return IsLoopReversible;
}

void HIRGeneralUnroll::replaceBySwitch(HLLoop *RemainderLoop,
                                       unsigned UnrollFactor) {
  if (DisableSwitchGeneration) {
    return;
  }

  if (!isSwitchGenerationProfitable(RemainderLoop, UnrollFactor)) {
    return;
  }

  if (!isSwitchGenerationLegal(RemainderLoop)) {
    return;
  }

  if (!RemainderLoop->normalize()) {
    return;
  }

  HIRInvalidationUtils::invalidateBody(RemainderLoop);

  RegDDRef *ConditionRef = RemainderLoop->removeUpperDDRef();

  // We can skip ztt because if the trip count is zero, normalized upper bound
  // will be a big positive number and go through default switch case which does
  // nothing
  auto &HNU = RemainderLoop->getHLNodeUtils();
  auto &DDRU = HNU.getDDRefUtils();

  unsigned LoopLevel = RemainderLoop->getNestingLevel();
  ConditionRef->makeConsistent({}, LoopLevel - 1);

  auto Switch = HNU.createHLSwitch(ConditionRef);

  Type *IntType = ConditionRef->getDestType();
  unsigned CaseNum = 1;

  for (int I = UnrollFactor - 2; I >= 0; --I, ++CaseNum) {
    RegDDRef *CaseRef = DDRU.createConstDDRef(IntType, I);
    Switch->addCase(CaseRef);

    // Generate HLlabel at the beginning of each case
    HLLabel *Label = HNU.createHLLabel("L" + Twine(I));
    HLNodeUtils::insertAsFirstChild(Switch, Label, CaseNum);

    if (I != (int)(UnrollFactor - 2)) {
      // Generate HLGoto at the end of last case
      HLGoto *Goto = HNU.createHLGoto(Label);
      HLNodeUtils::insertAsLastChild(Switch, Goto, CaseNum - 1);
    }

    HLContainerTy LoopBody;

    if (I == 0) {
      HLNodeUtils::remove(&LoopBody, RemainderLoop->getFirstChild(),
                          RemainderLoop->getLastChild());
    } else {
      HLNodeUtils::cloneSequence(&LoopBody, RemainderLoop->getFirstChild(),
                                 RemainderLoop->getLastChild());
    }

    IVUpdater IVUD(I, LoopLevel);

    // Update the IVs in the LoopBody
    HLNodeUtils::visitRange(IVUD, &(LoopBody.front()), &(LoopBody.back()));

    HLNodeUtils::insertAfter(Label, &LoopBody);
  }

  HLNodeUtils::replace(RemainderLoop, Switch);
}

unsigned HIRGeneralUnroll::computeUnrollFactor(const HLLoop *HLoop,
                                               bool HasEnablingPragma) const {

  unsigned UnrollFactor;
  uint64_t TripCount;
  bool IsConstTripLoop = HLoop->isConstTripLoop(&TripCount);

  if (HasEnablingPragma && (UnrollFactor = HLoop->getUnrollPragmaCount())) {
    assert(UnrollFactor != 1 && "pragma unroll count of 1 not expected!");

    if (IsConstTripLoop && (TripCount < UnrollFactor)) {
      LLVM_DEBUG(dbgs() << "Skipping unroll of pragma enabled loop as trip "
                           "count is too small!\n");
      return 0;
    }

    return UnrollFactor;
  }

  if (IsConstTripLoop && (TripCount < 2)) {
    LLVM_DEBUG(
        dbgs() << "Skipping unroll of loop as trip count is too small!\n");
    return 0;
  }

  unsigned SelfCost = HLR.getSelfLoopResource(HLoop).getTotalCost();

  if (SelfCost > MaxLoopCost) {
    if (HasEnablingPragma) {
      return 2;
    } else {
      LLVM_DEBUG(
          dbgs()
          << "Skipping unroll of loop as loop body cost exceeds threshold!\n");
      return 0;
    }
  }

  // Exit if loop with minimum unroll factor of 2 exceeds threshold.
  if ((2 * SelfCost) > MaxUnrolledLoopCost) {
    if (HasEnablingPragma) {
      return 2;
    } else {
      LLVM_DEBUG(dbgs() << "Skipping unroll of loop as unrolled loop body cost "
                           "exceeds threshold!\n");
      return 0;
    }
  }

  if (HasEnablingPragma) {
    // Use factor of 2 for small trip count loops.
    if (IsConstTripLoop && (TripCount < MaxUnrollFactor)) {
      return 2;
    }
  } else if ((IsConstTripLoop ||
              (TripCount = HLoop->getMaxTripCountEstimate())) &&
             (TripCount < MinTripCountThreshold)) {
    LLVM_DEBUG(dbgs() << "Skipping unroll of small trip count loop!\n");
    return 0;
  }

  // Multi-exit loops have a higher chance of having a low trip count as they
  // can take early exits. We may cause degradations in those cases by
  // increasing code size. Unroll factor of 2 is the safest bet.
  UnrollFactor = (HLoop->getNumExits() > 1) ? 2 : MaxUnrollFactor;

  while ((UnrollFactor * SelfCost) > MaxUnrolledLoopCost) {
    UnrollFactor /= 2;
  }

  assert(UnrollFactor >= 2 && "Unexpected unroll factor!");

  return UnrollFactor;
}

bool HIRGeneralUnroll::isApplicable(const HLLoop *Loop) const {
  if (Loop->isVecLoop()) {
    LLVM_DEBUG(dbgs() << "Skipping unroll of vectorizable loop!\n");
    return false;
  }

  if (Loop->hasGeneralUnrollDisablingPragma()) {
    LLVM_DEBUG(dbgs() << "Skipping unroll of pragma disabled loop!\n");
    return false;
  }

  // Loop should be normalized before this pass.
  if (!Loop->isNormalized()) {
    LLVM_DEBUG(dbgs() << "Skipping unroll of non-normalized loop!\n");
    return false;
  }

  const LoopStatistics &LS = HLS.getSelfLoopStatistics(Loop);

  // Cannot unroll loop if it has calls with noduplicate attribute.
  if (LS.hasCallsWithNoDuplicate()) {
    LLVM_DEBUG(dbgs() << "Skipping unroll of loop containing call(s) with "
                         "NoDuplicate attribute!\n");
    return false;
  }

  return true;
}

bool HIRGeneralUnroll::isProfitable(const HLLoop *Loop, bool HasEnablingPragma,
                                    unsigned *UnrollFactor) const {

  if (!HasEnablingPragma) {
    bool IsMultiExit = (Loop->getNumExits() > 1);

    // 32bit platform seems to be more sensitive to register pressure/code size.
    // Unrolling too many loops leads to regression in the same benchmark which
    // is improved on 64-bit platform.
    if (Is32Bit && IsMultiExit) {
      LLVM_DEBUG(
          dbgs()
          << "Skipping unroll of multi-exit loops on 32 bit platform!\n");
      return false;
    }

    // Enable this when we find a convincing test case where unrolling helps.
    // All the current instances where it helps is when we have locality
    // between memrefs. These should be handled by scalar replacement (captured
    // in CMPLRS-41981). It is causing degradations in some benchmarks and the
    // reasons are not quite clear to me.
    if (Loop->isDoMultiExit()) {
      LLVM_DEBUG(dbgs() << "Skipping unroll of DO multi-exit loop!\n");
      return false;
    }

    const LoopStatistics &LS = HLS.getSelfLoopStatistics(Loop);

    // TODO: remove this condition?
    if (LS.hasSwitches()) {
      LLVM_DEBUG(
          dbgs() << "Skipping unroll of loop containing switch statement!\n");
      return false;
    }
  }

  // Determine unroll factor of the loop.
  if ((*UnrollFactor = computeUnrollFactor(Loop, HasEnablingPragma)) == 0) {
    return false;
  }

  if (!HasEnablingPragma &&
      ((*UnrollFactor =
            refineUnrollFactorUsingReuseAnalysis(Loop, *UnrollFactor)) == 0)) {
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
  const HLLoop *Loop;
  SmallSet<unsigned, 16> RvalTempBlobSymbases;
  int Reuse;
  bool CyclicalDefUse;

public:
  ReuseAnalyzer(const HLLoop *Loop)
      : Loop(Loop), Reuse(0), CyclicalDefUse(false) {}

  void analyze() {
    HLNodeUtils::visitRange(*this, Loop->child_begin(), Loop->child_end());
  }

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
      // Only consider reuse for copies which dominate the backedge path.
      if (RvalTempBlobSymbases.count(LvalSymbase) &&
          HLNodeUtils::dominates(Node, Loop->getLastChild())) {
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

  ReuseAnalyzer RA(Loop);

  RA.analyze();

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

PreservedAnalyses HIRGeneralUnrollPass::run(llvm::Function &F,
                                            llvm::FunctionAnalysisManager &AM) {
  HIRGeneralUnroll(AM.getResult<HIRFrameworkAnalysis>(F),
                   AM.getResult<HIRLoopResourceAnalysis>(F),
                   AM.getResult<HIRDDAnalysisPass>(F),
                   AM.getResult<HIRSafeReductionAnalysisPass>(F),
                   AM.getResult<HIRLoopStatisticsAnalysis>(F), false)
      .run();
  return PreservedAnalyses::all();
}

class HIRGeneralUnrollLegacyPass : public HIRTransformPass {
public:
  static char ID;
  bool PragmaOnlyUnroll;

  HIRGeneralUnrollLegacyPass(bool PragmaOnlyUnroll = false)
      : HIRTransformPass(ID), PragmaOnlyUnroll(PragmaOnlyUnroll) {
    initializeHIRGeneralUnrollLegacyPassPass(*PassRegistry::getPassRegistry());
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.setPreservesAll();
    AU.addRequiredTransitive<HIRFrameworkWrapperPass>();
    AU.addRequiredTransitive<HIRLoopResourceWrapperPass>();
    AU.addRequiredTransitive<HIRDDAnalysisWrapperPass>();
    AU.addRequiredTransitive<HIRSafeReductionAnalysisWrapperPass>();
    AU.addRequiredTransitive<HIRLoopStatisticsWrapperPass>();
  }

  bool runOnFunction(Function &F) override {
    if (skipFunction(F)) {
      return false;
    }

    return HIRGeneralUnroll(
               getAnalysis<HIRFrameworkWrapperPass>().getHIR(),
               getAnalysis<HIRLoopResourceWrapperPass>().getHLR(),
               getAnalysis<HIRDDAnalysisWrapperPass>().getDDA(),
               getAnalysis<HIRSafeReductionAnalysisWrapperPass>().getHSR(),
               getAnalysis<HIRLoopStatisticsWrapperPass>().getHLS(),
               PragmaOnlyUnroll)
        .run();
  }
};

char HIRGeneralUnrollLegacyPass::ID = 0;
INITIALIZE_PASS_BEGIN(HIRGeneralUnrollLegacyPass, "hir-general-unroll",
                      "HIR General Unroll", false, false)
INITIALIZE_PASS_DEPENDENCY(HIRFrameworkWrapperPass)
INITIALIZE_PASS_DEPENDENCY(HIRLoopResourceWrapperPass)
INITIALIZE_PASS_DEPENDENCY(HIRDDAnalysisWrapperPass)
INITIALIZE_PASS_DEPENDENCY(HIRSafeReductionAnalysisWrapperPass)
INITIALIZE_PASS_DEPENDENCY(HIRLoopStatisticsWrapperPass)
INITIALIZE_PASS_END(HIRGeneralUnrollLegacyPass, "hir-general-unroll",
                    "HIR General Unroll", false, false)

FunctionPass *llvm::createHIRGeneralUnrollPass(bool PragmaOnlyUnroll) {
  return new HIRGeneralUnrollLegacyPass(PragmaOnlyUnroll);
}
