//===----- HIRUnrollAndJam.cpp - Implements UnrollAndJam class ------------===//
//
// Copyright (C) 2015-2019 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
// Unroll & Jam unrolls the outer loop by some factor and then fuses (jams) the
// unrolled body. For example-
//
// Original loop-
// for(i=0; i<n; i++) {
//   for(j=0; j<m; j++) {
//     A[i] = A[i] + B[j];
//   }
// }
//
// Modified loop-
// t = n/2;
// for(i=0; i<t; i++) {
//   for(j=0; j<m; j++) {
//     A[2*i] = A[2*i] + B[j];
//     A[2*i+1] = A[2*i+1] + B[j];
//   }
// }
//
// for(i=2*t; i<n; i++) {
//   for(j=0; j<m; j++) {
//     A[i] = A[i] + B[j];
//   }
// }
//
// The algorithm is as follows-
//
// 1) Gather outermost loops and then traverse them one at a time. Store the
// loop hierarchy along with the unroll factors in a data structure.
//
// 2) Initialize unroll factors to the max unroll factor for each loop as we
// visit them.
//
// 3) Throttle loops by doing some quick legality checks. Throttling can happen
// recursively by following the parent loop chain.
//
// 4) During postVisit(), analyze the legality and profitability of loops which
// were not throttled in visit() and refine the unroll factor accordingly. The
// main analysis therefore happens in inner-to-outer order. More loops can be
// throttled in this stage.
//
// 5) Unroll non-throttled loops in outer-to-inner order.
//
// TODO: Add opt-report messages.
//===----------------------------------------------------------------------===//
#include "llvm/Transforms/Intel_LoopTransforms/HIRUnrollAndJam.h"
#include "HIRUnroll.h"

#include "llvm/ADT/Statistic.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/MathExtras.h"
#include "llvm/Support/raw_ostream.h"

#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/DDTests.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRDDAnalysis.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRLocalityAnalysis.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRLoopResource.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRLoopStatistics.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRSafeReductionAnalysis.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Framework/HIRFramework.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HIRInvalidationUtils.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HLNodeUtils.h"

#include "llvm/Transforms/Intel_LoopTransforms/HIRTransformPass.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/HIRTransformUtils.h"

#define DEBUG_TYPE "hir-unroll-and-jam"

using namespace llvm;
using namespace llvm::loopopt;

const unsigned DefaultMaxUnrollFactor = 8;
const unsigned AbsoluteMaxUnrollFactor = 8;

STATISTIC(LoopsUnrolledAndJammed, "Number of HIR loops unrolled and jammed");

static cl::opt<bool>
    DisableHIRUnrollAndJam("disable-hir-unroll-and-jam", cl::init(false),
                           cl::Hidden, cl::desc("Disable HIR Unroll And Jam"));

// This is the maximum unroll factor that we use for any loop.
static cl::opt<unsigned> MaxUnrollFactor(
    "hir-unroll-and-jam-max-factor", cl::init(DefaultMaxUnrollFactor),
    cl::Hidden, cl::desc("Max unroll factor for loops (should be power of 2)"));

// This is the minimum trip count threshold.
static cl::opt<unsigned> MinTripCountThreshold(
    "hir-unroll-and-jam-min-trip-count-threshold", cl::init(16), cl::Hidden,
    cl::desc("Min trip count of loops which can be unrolled (absolute minimum "
             "depends on max unroll factor)"));

// This determines the unroll factor of loops inside the loopnest.
static cl::opt<unsigned> MaxUnrolledLoopNestCost(
    "hir-unroll-and-jam-max-unrolled-loopnest-cost", cl::init(680), cl::Hidden,
    cl::desc(
        "Max allowed cost of the loopnest with the unroll factor factored in"));

// This ensures that most of the code is in the innermost loop.
static cl::opt<unsigned> MaxOuterLoopCost(
    "hir-unroll-and-jam-max-outer-loop-cost", cl::init(30), cl::Hidden,
    cl::desc("Max allowed cost of an outer loop in the loopnest"));

typedef SmallVector<std::pair<HLLoop *, HLLoop *>, 16> LoopMapTy;

// Implements unroll/unroll & jam for \p Loop.
void unrollLoopImpl(HLLoop *Loop, unsigned UnrollFactor, LoopMapTy *LoopMap,
                    HLLoop **UnrolledLoop = nullptr,
                    HLLoop **RemainderLoop = nullptr);

// External interface
namespace llvm {
namespace loopopt {
namespace unroll {
void unrollLoop(HLLoop *Loop, unsigned UnrollFactor, HLLoop **UnrolledLoop,
                HLLoop **RemainderLoop) {
  unrollLoopImpl(Loop, UnrollFactor, nullptr, UnrolledLoop, RemainderLoop);
}
} // namespace unroll
} // namespace loopopt
} // namespace llvm

namespace {

// Main unroll and jam class.
class HIRUnrollAndJam {
public:
  HIRUnrollAndJam(HIRFramework &HIRF, HIRLoopStatistics &HLS,
                  HIRLoopResource &HLR, HIRLoopLocality &HLA,
                  HIRDDAnalysis &DDA, HIRSafeReductionAnalysis &HSRA,
                  bool PragmaOnlyUnroll)
      : HIRF(HIRF), HLS(HLS), HLR(HLR), HLA(HLA), DDA(DDA), HSRA(HSRA),
        HaveUnrollCandidates(false), PragmaOnlyUnroll(PragmaOnlyUnroll) {}

  bool run();

private:
  struct LoopUnrollJamInfo {
    HLLoop *Lp;
    unsigned UnrollFactor;
    bool Analyzed;

    LoopUnrollJamInfo(HLLoop *Lp, unsigned UnrollFactor)
        : Lp(Lp), UnrollFactor(UnrollFactor), Analyzed(false) {}
  };

  typedef SmallVector<LoopUnrollJamInfo, 6> LoopUnrollJamInfoPerLevelTy;
  // Stores the info for each loop in the loopnest by loop level.
  typedef std::array<LoopUnrollJamInfoPerLevelTy, MaxLoopNestLevel>
      LoopNestUnrollJamInfoTy;

  HIRFramework &HIRF;
  HIRLoopStatistics &HLS;
  HIRLoopResource &HLR;
  HIRLoopLocality &HLA;
  HIRDDAnalysis &DDA;
  HIRSafeReductionAnalysis &HSRA;

  LoopNestUnrollJamInfoTy LoopNestUnrollJamInfo;
  bool HaveUnrollCandidates;
  bool PragmaOnlyUnroll;

  class Analyzer;

  /// Processes and santitizes command line options.
  void sanitizeOptions();

  /// Returns true if \p Lp's unroll factor is uninitialized.
  bool isUninitialized(HLLoop *Lp) const;

  /// Either retrieve or updates unroll factor of \p Lp according to \p Update
  /// argument.
  unsigned getOrUpdateUnrollFactor(HLLoop *Lp, unsigned UnrollFactor,
                                   bool Update);

  /// Replaces existing loops in LoopNestUnrollJamInfo with new loops based on
  /// \p LoopMap.
  void replaceLoops(LoopMapTy &LoopMap);

  /// Perform unroll & jam on all the loops with valid unroll factors in the
  /// loopnest represented by \p Lp.
  void unrollCandidates(HLLoop *Lp);

  /// Clears existing unroll candidates.
  void clearCandidates();

public:
  /// Initializes unroll factor for \p Lp.
  void initializeUnrollFactor(HLLoop *Lp);

  /// Returns unroll factor of \p Lp.
  unsigned getUnrollFactor(HLLoop *Lp);

  /// Updates unroll factor of \p Lp to \p UnrollFactor and returns the old
  /// unroll factor.
  unsigned updateUnrollFactor(HLLoop *Lp, unsigned UnrollFactor);

  /// Marks loop as unrollable.
  void throttle(HLLoop *Lp);

  /// Marks loop and all its parent loop as unrollable.
  void throttleRecursively(HLLoop *Lp, bool IsCostModelThrottling = false);

  /// Returns true if the loop is marked unrollable.
  bool isThrottled(HLLoop *Lp);

  /// Sets \p Lp as analyzed.
  void setAnalyzed(HLLoop *Lp);

  /// Returns true if \p Lp has already been analyzed.
  bool isAnalyzed(HLLoop *Lp) const;

  /// Computes the cost of the loopnest represented by \p Lp by taking into
  /// account unroll factors associated with
  unsigned computeLoopNestCost(HLLoop *Lp) const;

  /// Returns true if \p Lp has a non-innermost child loop.
  bool hasNonInnermostChildrenLoop(HLLoop *Lp) const;
};

// Assigns unroll factor to outer loops using legality and profitability
// analysis.
class HIRUnrollAndJam::Analyzer final : public HLNodeVisitorBase {
  HIRUnrollAndJam &HUAJ;

private:
  /// Computes and returns unroll factor for the loop using cost model. Returns
  /// 0 to indicate that unroll & jam should be throttled recursively and 1 to
  /// indicate throttling of \p HLoop only.
  unsigned computeUnrollFactorUsingCost(HLLoop *HLoop,
                                        bool HasEnablingPragma) const;

  /// Returns true if \p Lp can legally be unrolled & jammed.
  bool canLegallyUnrollAndJam(HLLoop *Lp) const;

  /// Refines unroll factor of \p Lp by analyzing the parent loop.
  void refineUnrollFactorUsingParentLoop(HLLoop *Lp, unsigned &UnrollFactor);

public:
  Analyzer(HIRUnrollAndJam &HUAJ) : HUAJ(HUAJ) {}

  /// Performs preliminary checks to throttle loops for unroll & jam.
  void visit(HLLoop *Lp);

  /// Peforms profitability and legality checks on outer loops.
  void postVisit(HLLoop *Lp);

  /// Do nothing for instructions.
  void visit(HLInst *Inst) {}

  /// Handle nodes other than HLLoop or HLInst.
  void visit(HLNode *Node);

  void postVisit(HLNode *) {}

  /// Driver function performing legality/profitability analysis on a loopnest
  /// represented by \p Lp.
  void analyze(HLLoop *Lp);
};

// Checks the legality of unroll & jam for a loop.
class LegalityChecker final : public HLNodeVisitorBase {
  DDGraph DDG;
  HIRSafeReductionAnalysis &HSRA;
  const HLLoop *CandidateLoop;
  unsigned LoopLevel;
  bool IsLegal;

  /// Returns true if it is legal to permute LoopLevel DV element with innermost
  /// level DV element. This is same as checking whether the two loops can be
  /// interchanged.
  bool isLegalToPermute(const DirectionVector &DV,
                        bool IsInnermostLoopDV) const;

  /// Returns true if Ref can be ignored for legality purposes.
  bool canIgnoreRef(const RegDDRef *Ref, const HLLoop *ParentLoop) const;

public:
  LegalityChecker(HIRDDAnalysis &DDA, HIRSafeReductionAnalysis &HSRA,
                  const HLLoop *Loop)
      : DDG(DDA.getGraph(Loop)), HSRA(HSRA), CandidateLoop(Loop),
        LoopLevel(Loop->getNestingLevel()), IsLegal(true) {}

  /// Iterates though DDRefs and checks legality of edge DVs.
  void visit(const HLDDNode *Node);

  void visit(const HLNode *Node) {}
  void postVisit(const HLNode *Node) {}

  bool isDone() const { return !IsLegal; }

  /// Driver function which checks legality of the loop.
  bool isLegal();
};
} // namespace

bool LegalityChecker::isLegal() {
  HLNodeUtils::visitRange(*this, CandidateLoop->child_begin(),
                          CandidateLoop->child_end());
  return IsLegal;
}

bool LegalityChecker::isLegalToPermute(const DirectionVector &DV,
                                       bool IsInnermostLoopDV) const {
  // Legality check is the same as interchanging CandidateLoop with the
  // innermost loop so we check whether swapping the corresponding DV elements
  // yields a legal DV.

  unsigned LastLevel = DV.getLastLevel();
  assert((LastLevel >= LoopLevel) && "DV has invalid last level!");

  DVKind LoopLevelDV = DV[LoopLevel - 1];
  DVKind InnermostDV = DV[LastLevel - 1];

  // Consider edges in outer loops as permuting LoopLevel with (*) after the
  // last level DV.
  if (!IsInnermostLoopDV) {
    LastLevel++;
    InnermostDV = DVKind::ALL;
  }

  // 1. We can always permute these combinations-
  // (<, <)
  // (=, =)
  // (>, >)
  if (LoopLevelDV == InnermostDV) {
    if ((LoopLevelDV == DVKind::LT) || (LoopLevelDV == DVKind::EQ) ||
        (LoopLevelDV == DVKind::GT)) {
      return true;
    }
  }

  // 2. Check if dependence is carried by an outer loop which makes interchange
  // legal.
  if (DV.isIndepFromLevel(LoopLevel)) {
    return true;
  }

  // 3. We cannot permute outer and inner DV elements if the direction is
  // reversed in any combination after the permutation. For example (*, <)
  // yields (<, <), (=, <) and (<, >) after decomposing. The direction of (<, >)
  // gets reversed after permutation.
  if (((LoopLevelDV & DVKind::LT) && (InnermostDV & DVKind::GT)) ||
      ((LoopLevelDV & DVKind::GT) && (InnermostDV & DVKind::LT))) {
    return false;
  }

  DVKind ValidDV, InvalidDV;

  // 4. Now we check if any of the DV elements between LoopLevel to innermost
  // level preserve the direction of the DV after permutation.
  if ((LoopLevelDV == DVKind::ALL) || (InnermostDV == DVKind::ALL)) {
    // (*, =) and (=, *) can only be permuted if all intervening levels are (=).
    assert(((LoopLevelDV == DVKind::ALL && InnermostDV == DVKind::EQ) ||
            (LoopLevelDV == DVKind::EQ && InnermostDV == DVKind::ALL)) &&
           "Unexpected Direction vector!");
    ValidDV = DVKind::NONE;
    InvalidDV = DVKind::NE;
  } else {
    // At this point either one of LoopLevelDV/InnermostDV is EQ (Ex - (<, =),
    // (=, >)) or it is a composite case which decays to the former case. For
    // example, (<, <=) decays to (<, =).
    // Direction would be preserved if we find an element with the same
    // direction as LoopLevelDV/InnermostDV before finding an element with the
    // reverse direction.
    if ((LoopLevelDV & DVKind::LT) || (InnermostDV & DVKind::LT)) {
      ValidDV = DVKind::LT;
      InvalidDV = DVKind::GT;
    } else {
      ValidDV = DVKind::GT;
      InvalidDV = DVKind::LT;
    }
  }

  for (unsigned I = LoopLevel + 1; I < LastLevel; ++I) {
    if (DV[I - 1] & InvalidDV) {
      return false;
    } else if (DV[I - 1] == ValidDV) {
      return true;
    }
  }

  // All intervening elements are (or decay to) EQ so it is ok to permute.
  return true;
}

bool LegalityChecker::canIgnoreRef(const RegDDRef *Ref,
                                   const HLLoop *ParentLoop) const {

  if (!Ref->isTerminalRef()) {
    return false;
  }

  if (!CandidateLoop->isLiveIn(Ref->getSymbase())) {
    return true;
  }

  // The only livein temp allowed is the loopnest reduction temp-
  // DO i1
  //   DO i2
  //     t1 = t1 + A[i2]
  //   END DO
  // END DO
  if (!ParentLoop->isInnermost()) {
    return false;
  }

  unsigned OpCode;
  if (!HSRA.isReductionRef(Ref, OpCode)) {
    return false;
  }

  // Checking edges of lval reduction temp is enough.
  if (Ref->isRval()) {
    return true;
  }

  // If there are any outgoing (flow/output) edges for the lval reduction temp
  // it means it isn't a loopnest level reduction.
  for (auto *Edge : DDG.outgoing(Ref)) {
    if (Edge->getSink()->getLexicalParentLoop() == ParentLoop) {
      continue;
    }

    return false;
  }

  return true;
}

void LegalityChecker::visit(const HLDDNode *Node) {

  auto *ParentLoop = Node->getLexicalParentLoop();
  bool IsInnermostLoop = ParentLoop->isInnermost();

  for (auto RefIt = Node->ddref_begin(), E = Node->ddref_end(); RefIt != E;
       ++RefIt) {

    auto *Ref = (*RefIt);

    if (canIgnoreRef(Ref, ParentLoop)) {
      continue;
    }

    for (auto *Edge : DDG.outgoing(Ref)) {

      if (!isLegalToPermute(
              Edge->getDV(),
              (IsInnermostLoop ||
               Edge->getSink()->getLexicalParentLoop()->isInnermost()))) {
        IsLegal = false;
        return;
      }
    }
  }
}

bool HIRUnrollAndJam::isUninitialized(HLLoop *Lp) const {
  for (auto &LoopInfo : LoopNestUnrollJamInfo[Lp->getNestingLevel() - 1]) {
    if (LoopInfo.Lp == Lp) {
      return false;
    }
  }

  return true;
}

void HIRUnrollAndJam::initializeUnrollFactor(HLLoop *Lp) {
  assert(isUninitialized(Lp) && "Attempt to reinitialize loop!");
  LoopNestUnrollJamInfo[Lp->getNestingLevel() - 1].emplace_back(
      Lp, Lp->isInnermost() ? 1 : MaxUnrollFactor);
}

unsigned HIRUnrollAndJam::getOrUpdateUnrollFactor(HLLoop *Lp,
                                                  unsigned UnrollFactor,
                                                  bool Update) {
  assert((!Update || (UnrollFactor <= MaxUnrollFactor) ||
          (UnrollFactor == Lp->getUnrollAndJamPragmaCount())) &&
         "Invalid unroll factor!");

  auto Level = Lp->getNestingLevel();

  for (auto &LoopInfo : LoopNestUnrollJamInfo[Level - 1]) {
    if (LoopInfo.Lp == Lp) {
      if (!Update) {
        return LoopInfo.UnrollFactor;

      } else {
        assert(((UnrollFactor < 2) || (UnrollFactor <= LoopInfo.UnrollFactor) ||
                (UnrollFactor == Lp->getUnrollAndJamPragmaCount())) &&
               "Unroll factor can only be refined downwards!");
        unsigned OldFactor = LoopInfo.UnrollFactor;

        if (OldFactor) {
          LoopInfo.UnrollFactor = UnrollFactor;
        }
        return OldFactor;
      }
    }
  }

  llvm_unreachable("Loop not found in loop tree!");
}

unsigned HIRUnrollAndJam::getUnrollFactor(HLLoop *Lp) {
  return getOrUpdateUnrollFactor(Lp, 0, false);
}

unsigned HIRUnrollAndJam::updateUnrollFactor(HLLoop *Lp,
                                             unsigned UnrollFactor) {
  if (UnrollFactor > 1) {
    HaveUnrollCandidates = true;
  }

  return getOrUpdateUnrollFactor(Lp, UnrollFactor, true);
}

bool HIRUnrollAndJam::isThrottled(HLLoop *Lp) {
  unsigned UF = getUnrollFactor(Lp);
  return (UF <= 1);
}

void HIRUnrollAndJam::setAnalyzed(HLLoop *Lp) {
  unsigned Level = Lp->getNestingLevel();

  for (auto &LoopInfo : LoopNestUnrollJamInfo[Level - 1]) {
    if (LoopInfo.Lp == Lp) {
      LoopInfo.Analyzed = true;
      return;
    }
  }

  llvm_unreachable("Loop not found in loop tree!");
}

bool HIRUnrollAndJam::isAnalyzed(HLLoop *Lp) const {
  unsigned Level = Lp->getNestingLevel();

  for (auto &LoopInfo : LoopNestUnrollJamInfo[Level - 1]) {
    if (LoopInfo.Lp == Lp) {
      return LoopInfo.Analyzed;
    }
  }

  llvm_unreachable("Loop not found in loop tree!");
}

void HIRUnrollAndJam::throttle(HLLoop *Lp) { updateUnrollFactor(Lp, 1); }

void HIRUnrollAndJam::throttleRecursively(HLLoop *Lp,
                                          bool IsCostModelThrottling) {
  assert(Lp && "Loop is null!");

  while (Lp && updateUnrollFactor(Lp, 0)) {
    // Do not throttle parent loops with enabling pragma based on cost model.
    if (IsCostModelThrottling) {
      while (Lp && Lp->hasUnrollAndJamEnablingPragma()) {
        Lp = Lp->getParentLoop();
      }
    } else {
      Lp = Lp->getParentLoop();
    }
  }
}

void HIRUnrollAndJam::Analyzer::visit(HLNode *Node) {
  assert((!isa<HLLoop>(Node) && !isa<HLInst>(Node)) &&
         "Loop or Inst not expected!");

  auto *ParentLoop = Node->getParentLoop();

  // Allow HLIfs in innermost loops. This is handled correctly by the
  // transformation although it may not be profitable in most cases.
  if (!isa<HLIf>(Node) || !ParentLoop->isInnermost()) {
    HUAJ.throttleRecursively(ParentLoop);
  }
}

void HIRUnrollAndJam::Analyzer::visit(HLLoop *Lp) {

  HUAJ.initializeUnrollFactor(Lp);

  if (!Lp->isDo()) {
    LLVM_DEBUG(dbgs() << "Skipping unroll & jam of non-DO loop!\n");
    HUAJ.throttleRecursively(Lp);
    return;
  }

  // TODO: What is the right behavior for vectorizable loops?
  if (Lp->isVecLoop()) {
    LLVM_DEBUG(dbgs() << "Skipping unroll & jam of vectorizable loop!\n");
    HUAJ.throttleRecursively(Lp);
    return;
  }

  auto &LS = HUAJ.HLS.getSelfLoopStatistics(Lp);

  // Cannot unroll loop if it has calls with noduplicate attribute.
  if (LS.hasCallsWithNoDuplicate()) {
    LLVM_DEBUG(
        dbgs() << "Skipping unroll & jam of loopnest containing call(s) with "
                  "NoDuplicate attribute !\n");
    HUAJ.throttleRecursively(Lp);
    return;
  }

  if (!Lp->isInnermost()) {
    if (!Lp->isNormalized()) {
      LLVM_DEBUG(dbgs() << "Skipping unroll & jam of non-normalized loop!\n");
      HUAJ.throttle(Lp);
      return;
    }

    if (Lp->hasUnrollAndJamDisablingPragma()) {
      LLVM_DEBUG(dbgs() << "Skipping unroll & jam of pragma disabled loop!\n");
      HUAJ.throttle(Lp);
      return;
    }

    if (Lp->hasVectorizeEnablingPragma()) {
      LLVM_DEBUG(dbgs() << "Skipping unroll & jam of vector pragma loop!\n");
      HUAJ.throttle(Lp);
      return;
    }

    if (Lp->hasUnrollEnablingPragma()) {
      LLVM_DEBUG(
          dbgs() << "Skipping unroll & jam as loop has unroll pragma!\n");
      HUAJ.throttle(Lp);
      return;
    }
  }

  // Throttle unroll of outer loop whose inner loop's bounds varies within the
  // outer loop, as they cannot be fused.
  if (Lp->getParentLoop()) {
    for (auto RefIt = Lp->ddref_begin(), E = Lp->ddref_end(); RefIt != E;
         ++RefIt) {

      auto CE = (*RefIt)->getSingleCanonExpr();

      if (unsigned DefLevel = CE->getDefinedAtLevel()) {
        LLVM_DEBUG(
            dbgs() << "Skipping unroll & jam for loopnest as it is illegal!\n");
        HUAJ.throttleRecursively(Lp->getParentLoopAtLevel(DefLevel));
      }

      for (auto IV = CE->iv_begin(), IVE = CE->iv_end(); IV != IVE; ++IV) {
        if (CE->getIVConstCoeff(IV) != 0) {
          LLVM_DEBUG(dbgs()
                     << "Skipping unroll & jam for loop as it is illegal!\n");
          HUAJ.throttle(Lp->getParentLoopAtLevel(CE->getLevel(IV)));
        }
      }
    }
  }
}

unsigned HIRUnrollAndJam::computeLoopNestCost(HLLoop *Lp) const {
  unsigned Cost = HLR.getSelfLoopResource(Lp).getTotalCost();

  if (Lp->isInnermost()) {
    return Cost;
  }

  bool ChildrenFound = false;

  // Immediate children appear in a contiguous chunk in the next level of
  // LoopNestUnrollJamInfo.
  for (auto &ChildLoopInfo : LoopNestUnrollJamInfo[Lp->getNestingLevel()]) {
    auto ChildLp = ChildLoopInfo.Lp;

    if (ChildLp->getParentLoop() != Lp) {
      if (!ChildrenFound) {
        // Haven't encountered any children yet, keep looking.
        continue;
      } else {
        break;
      }
    }
    ChildrenFound = true;

    unsigned UnrollFactor =
        ChildLoopInfo.UnrollFactor ? ChildLoopInfo.UnrollFactor : 1;

    Cost += (UnrollFactor * computeLoopNestCost(ChildLp));
  }

  assert(ChildrenFound && "No children found for non-innermost loop!");

  return Cost;
}

bool HIRUnrollAndJam::hasNonInnermostChildrenLoop(HLLoop *Lp) const {
  assert(!Lp->isInnermost() && "Innermost loop not expected!");

  bool ChildrenFound = false;

  // Immediate children appear in a contiguous chunk in the next level of
  // LoopNestUnrollJamInfo.
  for (auto &ChildLoopInfo : LoopNestUnrollJamInfo[Lp->getNestingLevel()]) {
    auto *ChildLp = ChildLoopInfo.Lp;

    if (ChildLp->getParentLoop() != Lp) {
      if (!ChildrenFound) {
        // Haven't encountered any children yet, keep looking.
        continue;
      } else {
        break;
      }
    }

    ChildrenFound = true;

    if (!ChildLp->isInnermost()) {
      return true;
    }
  }

  return false;
}

// Returns true if all loops in the loopnest have trip count less than equal
// to 16.
static bool isCompleteUnrollCandidate(HLLoop *OuterLp) {
  SmallVector<HLLoop *, 8> Loops;

  OuterLp->getHLNodeUtils().gatherAllLoops(OuterLp, Loops);

  uint64_t TC;
  for (auto Lp : Loops) {
    if (!Lp->isConstTripLoop(&TC) || (TC > 16)) {
      return false;
    }
  }

  return true;
}

unsigned HIRUnrollAndJam::Analyzer::computeUnrollFactorUsingCost(
    HLLoop *Lp, bool HasEnablingPragma) const {

  uint64_t TC;
  bool IsConstTC = Lp->isConstTripLoop(&TC);
  unsigned UnrollFactor;

  if ((UnrollFactor = Lp->getUnrollAndJamPragmaCount())) {
    assert(UnrollFactor != 1 && "pragma unroll count of 1 not expected!");

    if (IsConstTC && (TC < UnrollFactor)) {
      LLVM_DEBUG(
          dbgs() << "Skipping unroll & jam of pragma enabled loop as trip "
                    "count is too small!\n");
      return 1;
    }

    // Return pragma count as the unroll factor.
    return UnrollFactor;
  }

  if (IsConstTC && (TC < 2)) {
    LLVM_DEBUG(
        dbgs()
        << "Skipping unroll & jam of loop as trip count is too small!\n");
    return 1;
  }

  if ((IsConstTC || (TC = Lp->getMaxTripCountEstimate())) &&
      (TC < MinTripCountThreshold)) {
    if (HasEnablingPragma) {
      return 2;
    } else {
      LLVM_DEBUG(dbgs() << "Skipping unroll & jam of small trip count loop!\n");
      return 1;
    }
  }

  if (!HasEnablingPragma && IsConstTC && isCompleteUnrollCandidate(Lp)) {
    LLVM_DEBUG(
        dbgs()
        << "Skipping unroll & jam of possible complete unroll candidate!\n");
    return 1;
  }

  unsigned LoopCost = HUAJ.HLR.getSelfLoopResource(Lp).getTotalCost();

  if (LoopCost > MaxOuterLoopCost) {
    if (HasEnablingPragma) {
      return 2;
    } else {
      LLVM_DEBUG(
          dbgs() << "Skipping unroll & jam of loop as the loop body cost "
                    "exceeds threshold!\n");
      return 0;
    }
  }

  unsigned LoopNestCost = HUAJ.computeLoopNestCost(Lp);

  if ((2 * LoopNestCost) > MaxUnrolledLoopNestCost) {
    if (HasEnablingPragma) {
      return 2;
    } else {
      LLVM_DEBUG(
          dbgs() << "Skipping unroll & jam of loop as the unrolled loop body "
                    "cost exceeds threshold!\n");
      return 0;
    }
  }

  UnrollFactor = MaxUnrollFactor;

  while ((UnrollFactor * LoopNestCost) > MaxUnrolledLoopNestCost) {
    UnrollFactor /= 2;
  }

  assert(UnrollFactor >= 2 && "Unexpected unroll factor!");

  return UnrollFactor;
}

bool HIRUnrollAndJam::Analyzer::canLegallyUnrollAndJam(HLLoop *Lp) const {
  // TODO: use a smaller unroll factor if allowed by the distance vector.
  LegalityChecker LC(HUAJ.DDA, HUAJ.HSRA, Lp);

  return LC.isLegal();
}

void HIRUnrollAndJam::Analyzer::refineUnrollFactorUsingParentLoop(
    HLLoop *Lp, unsigned &UnrollFactor) {
  // Nothing to refine.
  if (UnrollFactor == 2) {
    return;
  }

  auto *ParLp = Lp->getParentLoop();

  if (!ParLp || HUAJ.isThrottled(ParLp) ||
      ParLp->hasUnrollAndJamEnablingPragma()) {
    return;
  }

  // For simplicity and accuracy of analysis, only consider perfect loopnest
  // case. We may not have iterated over other children of ParLoop yet as we are
  // in the postvisit() of Lp. If other children exist, they may affect
  // legality/cost model of ParLoop later.
  if (Lp != ParLp->getFirstChild() || Lp != ParLp->getLastChild()) {
    return;
  }

  // For simplicity, only refine factor for loops which contain only innermost
  // loops.
  if (HUAJ.hasNonInnermostChildrenLoop(Lp)) {
    return;
  }

  // Needed to compute unroll factor of ParLp using cost.
  HUAJ.updateUnrollFactor(Lp, UnrollFactor);

  unsigned ParUnrollFactor = computeUnrollFactorUsingCost(ParLp, false);

  // Factor of 1 indicates failed sanity (trip count) checks.
  if (ParUnrollFactor == 1) {
    HUAJ.throttle(ParLp);
    return;
  }

  // Factor of 0 indicates that cost threshold was exceeded. This is okay as we
  // can still unroll ParLp by decreasing unroll factor of Lp.
  if (ParUnrollFactor == 0) {
    ParUnrollFactor = 1;
  }

  if (!HUAJ.HLA.hasTemporalLocality(ParLp, 1)) {
    LLVM_DEBUG(dbgs() << "Skipping unroll & jam as loop does not have "
                         "temporal locality!\n");
    HUAJ.throttle(ParLp);
    return;
  }

  if (!canLegallyUnrollAndJam(ParLp)) {
    LLVM_DEBUG(dbgs() << "Skipping unroll & jam for loop as it is illegal!\n");
    HUAJ.throttle(ParLp);
    return;
  }

  HUAJ.setAnalyzed(ParLp);

  unsigned EqualizedUnrollFactor = UnrollFactor;
  unsigned EqualizedParUnrollFactor = ParUnrollFactor;
  unsigned TempUnrollFactor = UnrollFactor;

  // Try 'equalizing' factors by increasing parent loop factor and decreasing
  // current loop factor simultaneously. This logic assumes that factor
  // combination of (4, 4) is better than (2, 8). This was verified on a matmul
  // testcase on Xeon.
  while (ParUnrollFactor <= TempUnrollFactor) {
    EqualizedUnrollFactor = TempUnrollFactor;
    EqualizedParUnrollFactor = ParUnrollFactor;

    ParUnrollFactor *= 2;
    TempUnrollFactor /= 2;
  }

  HUAJ.updateUnrollFactor(ParLp, EqualizedParUnrollFactor);

  UnrollFactor = EqualizedUnrollFactor;
}

void HIRUnrollAndJam::Analyzer::postVisit(HLLoop *Lp) {

  if (Lp->isInnermost()) {
    // Need safe reduction info of innermost loops for handling loopnest
    // reductions. For example, we can unroll jam i1 loop even though t1 is
    // livein to the loop-
    // DO i1
    //   DO i2
    //     t1 = t1 + A[i1][i2];
    //   END DO
    // END DO
    HUAJ.HSRA.computeSafeReductionChains(Lp);
    return;
  }

  if (HUAJ.isThrottled(Lp) || HUAJ.isAnalyzed(Lp)) {
    return;
  }

  bool HasEnablingPragma = Lp->hasUnrollAndJamEnablingPragma();

  // Disable all other unrolling if only pragma enabled unrolling is allowed.
  if (HUAJ.PragmaOnlyUnroll && !HasEnablingPragma) {
    HUAJ.throttle(Lp);
    return;
  }

  unsigned UnrollFactor = computeUnrollFactorUsingCost(Lp, HasEnablingPragma);

  if (!UnrollFactor) {
    HUAJ.throttleRecursively(Lp, true);
    return;
  }

  if (UnrollFactor == 1) {
    HUAJ.throttle(Lp);
    return;
  }

  if (!HasEnablingPragma) {
    // TODO: refine unroll factor using extra cache lines accessed by
    // unrolling?
    if (!HUAJ.HLA.hasTemporalLocality(Lp, UnrollFactor - 1)) {
      LLVM_DEBUG(dbgs() << "Skipping unroll & jam as loop does not have "
                           "temporal locality!\n");
      HUAJ.throttle(Lp);
      return;
    }

    if (!canLegallyUnrollAndJam(Lp)) {
      LLVM_DEBUG(
          dbgs() << "Skipping unroll & jam for loop as it is illegal!\n");
      HUAJ.throttle(Lp);
      return;
    }

    refineUnrollFactorUsingParentLoop(Lp, UnrollFactor);
  }

  HUAJ.updateUnrollFactor(Lp, UnrollFactor);
}

void HIRUnrollAndJam::Analyzer::analyze(HLLoop *Lp) {
  HLNodeUtils::visit(*this, Lp);
}

void HIRUnrollAndJam::sanitizeOptions() {

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

void HIRUnrollAndJam::clearCandidates() {
  for (auto &UJInfo : LoopNestUnrollJamInfo) {
    UJInfo.clear();
  }

  HaveUnrollCandidates = false;
}

void HIRUnrollAndJam::replaceLoops(LoopMapTy &LoopMap) {

  for (auto &LoopPair : LoopMap) {
    unsigned LoopLevel = LoopPair.second->getNestingLevel();
    bool Found = false;

    for (auto &UJInfo : LoopNestUnrollJamInfo[LoopLevel - 1]) {
      if (UJInfo.Lp == LoopPair.first) {
        UJInfo.Lp = LoopPair.second;
        Found = true;
        break;
      }
    }
    (void)Found;
    assert(Found && "Inner loop not found!");
  }
}

void HIRUnrollAndJam::unrollCandidates(HLLoop *Lp) {
  if (!HaveUnrollCandidates) {
    return;
  }

  // Set gen code as we will be performing unroll & jam on at least one loop in
  // this loopnest.
  Lp->getParentRegion()->setGenCode();

  for (auto &LoopsPerLevel : LoopNestUnrollJamInfo) {
    for (auto &LoopUJInfo : LoopsPerLevel) {
      if (LoopUJInfo.UnrollFactor > 1) {
        LoopMapTy LoopMap;

        LoopUJInfo.Lp->markDoNotUnrollAndJam();

        unrollLoopImpl(LoopUJInfo.Lp, LoopUJInfo.UnrollFactor, &LoopMap);
        replaceLoops(LoopMap);
        LoopsUnrolledAndJammed++;
      }
    }
  }
}

bool HIRUnrollAndJam::run() {
  if (DisableHIRUnrollAndJam) {
    return false;
  }

  sanitizeOptions();

  SmallVector<HLLoop *, 16> OutermostLoops;

  HIRF.getHLNodeUtils().gatherOutermostLoops(OutermostLoops);

  Analyzer AY(*this);

  for (auto Loop : OutermostLoops) {
    AY.analyze(Loop);
    unrollCandidates(Loop);
    clearCandidates();
  }

  return false;
}

namespace {

typedef SmallVector<unsigned, 8> TempBlobIndexVecTy;
typedef std::pair<unsigned, TempBlobIndexVecTy> TempBlobIndexMap;

// Stores the mapping of temps in outer loops to renamed temps in each unrolled
// iteration.
typedef SmallVector<TempBlobIndexMap, 6> TempRenamingMapTy;

class UnrollHelper {
  class CanonExprUpdater;

  HLLoop *OrigTopLevelLoop;
  HLLoop *CurOrigLoop;
  LoopMapTy *LoopMap;
  HLLabel *UnknownLoopExitLabel;

  unsigned UnrollLevel;
  unsigned UnrollFactor;
  unsigned UnrollIteration;

  bool NeedRemainderLoop;

  TempRenamingMapTy TempRenamingMap;

  bool isLastUnrollIteration() const {
    return UnrollIteration == (UnrollFactor - 1);
  }

  bool shouldCreateNewLvalTemp(unsigned Symbase) const;

  void createLvalTempMapping(RegDDRef *Ref);

  void renameTemps(RegDDRef *Ref);

public:
  UnrollHelper(HLLoop *OrigTopLevelLoop, unsigned UnrollFactor,
               LoopMapTy *LoopMap, HLLabel *UnknownLoopExitLabel,
               bool NeedRemainderLoop)
      : OrigTopLevelLoop(OrigTopLevelLoop), CurOrigLoop(nullptr),
        LoopMap(LoopMap), UnknownLoopExitLabel(UnknownLoopExitLabel),
        UnrollLevel(OrigTopLevelLoop->getNestingLevel()),
        UnrollFactor(UnrollFactor), UnrollIteration(-1),
        NeedRemainderLoop(NeedRemainderLoop) {}

  void setUnrollIteration(unsigned Count) { UnrollIteration = Count; }
  unsigned getUnrollFactor() const { return UnrollFactor; }

  void updateLoopMap(HLLoop *OrigLoop, HLLoop *NewLoop) {
    assert(LoopMap && "Non-null loop map expected!");
    LoopMap->emplace_back(OrigLoop, NewLoop);
  }

  bool isUnrollJamMode() const { return LoopMap != nullptr; }
  bool isUnknownLoopUnroll() const { return UnknownLoopExitLabel != nullptr; }

  void patchIntermediateBottomTestForUnknownLoop(HLNode *BottomTest) const;

  void setCurOrigLoop(HLLoop *Loop) { CurOrigLoop = Loop; }

  bool needRemainderLoop() const { return NeedRemainderLoop; }

  void addRenamedTempsAsLiveinLiveout(HLLoop *NewLoop) const;

  static HLNode *getLastNodeInUnrollRange(HLNode *FirstNode);

  void updateNodeRange(HLNode *FirstNode, HLNode *LastNode);
};

// Updates CanonExprs for unroll/unroll & jam.
class UnrollHelper::CanonExprUpdater final : public HLNodeVisitorBase {
private:
  UnrollHelper &UHelper;

  void processRegDDRef(RegDDRef *RegDD);
  void processCanonExpr(CanonExpr *CExpr);

public:
  CanonExprUpdater(UnrollHelper &UHelper) : UHelper(UHelper) {}

  /// No processing needed for Goto
  void visit(HLGoto *Goto){};
  /// No processing needed for Label
  void visit(HLLabel *Label){};
  void visit(HLDDNode *Node);
  void visit(HLNode *Node) {
    llvm_unreachable(" Node not supported for unrolling.");
  };
  void postVisit(HLNode *Node) {}

  void createLvalTempMapping(RegDDRef *LvalRef);
};

} // namespace

void UnrollHelper::patchIntermediateBottomTestForUnknownLoop(
    HLNode *BottomTest) const {

  if (!UnknownLoopExitLabel) {
    return;
  }

  auto *BottomTestIf = cast<HLIf>(BottomTest);

  auto PredIter = BottomTestIf->pred_begin();
  auto *FirstChild = BottomTestIf->getFirstThenChild();

  auto Goto = cast<HLGoto>(FirstChild);

  // Invert predicate and make it jump to ExitLabel.
  BottomTestIf->invertPredicate(PredIter);
  Goto->setTargetLabel(UnknownLoopExitLabel);
}

bool UnrollHelper::shouldCreateNewLvalTemp(unsigned LvalSymbase) const {
  if (!isUnrollJamMode()) {
    return false;
  }

  if (isLastUnrollIteration()) {
    return false;
  }

  // Do not rename temps in loopnest reductions.
  // These are the only allowed livein temps for unroll & jam.
  if (OrigTopLevelLoop->isLiveIn(LvalSymbase)) {
    return false;
  }

  // Create new mapping for temps defined in outer loops.
  if (!CurOrigLoop->isInnermost()) {
    return true;
  }

  // If the temp is liveout of innermost loop we should create a mapping as
  // different values of the temp may be getting consumed in each outer loop
  // iteration. Usually, there will already be a mapping for it if we found its
  // definition in an outer loop but in some cases (when the innermost loop has
  // constant trip count) this may be the first encountered definition.
  //
  // Example-
  //
  // DO i1
  //   DO i2 = 0, 4
  //    t = A[i1 + i2]   << this should be renamed in innermost loop
  //   END DO
  //   B[i1] = t
  // END DO
  return CurOrigLoop->isLiveOut(LvalSymbase);
}

void UnrollHelper::addRenamedTempsAsLiveinLiveout(HLLoop *NewLoop) const {
  auto &BU = NewLoop->getBlobUtils();

  for (auto &TempEntry : TempRenamingMap) {
    unsigned OldSymbase = BU.getTempBlobSymbase(TempEntry.first);

    if (NewLoop->isLiveIn(OldSymbase)) {
      for (unsigned RenamedTempBlob : TempEntry.second) {
        NewLoop->addLiveInTemp(BU.getTempBlobSymbase(RenamedTempBlob));
      }
    }

    if (NewLoop->isLiveOut(OldSymbase)) {
      for (unsigned RenamedTempBlob : TempEntry.second) {
        NewLoop->addLiveOutTemp(BU.getTempBlobSymbase(RenamedTempBlob));
      }
    }
  }
}

void UnrollHelper::createLvalTempMapping(RegDDRef *Ref) {
  if (!Ref->isTerminalRef() || !Ref->isLval() || Ref->isFakeLval()) {
    return;
  }

  if (!shouldCreateNewLvalTemp(Ref->getSymbase())) {
    return;
  }

  unsigned OldTempIndex =
      Ref->isSelfBlob()
          ? Ref->getSelfBlobIndex()
          : Ref->getBlobUtils().findTempBlobIndex(Ref->getSymbase());

  // The temps is liveout of region with no uses inside it. It is fine to not
  // rename it from stability point of view. It might impact performance due to
  // unnecessary edges between multiple definitions. This can be resolved in
  // different ways including only relying on symbases instead of blob indices
  // to rename temps or not replicating such temps. Leaving that as a TODO until
  // we have a test case.
  if (OldTempIndex == InvalidBlobIndex) {
    return;
  }

  auto TempIt = TempRenamingMap.end();

  for (auto It = TempRenamingMap.begin(), E = TempRenamingMap.end(); It != E;
       ++It) {
    if (It->first == OldTempIndex) {
      if (It->second.size() > UnrollIteration) {
        // Temp has been renamed already for the current unrolled iteration. We
        // have found another temp definition. We should keep using the existing
        // mapping.
        return;
      }
      TempIt = It;
      break;
    }
  }

  unsigned NewTempIndex =
      Ref->getHLDDNode()->getHLNodeUtils().createAndReplaceTemp(Ref);

  if (TempIt != TempRenamingMap.end()) {
    TempIt->second.push_back(NewTempIndex);
  } else {
    TempRenamingMap.emplace_back(OldTempIndex,
                                 TempBlobIndexVecTy(1, NewTempIndex));
  }
}

void UnrollHelper::renameTemps(RegDDRef *Ref) {

  createLvalTempMapping(Ref);

  // No need to rename in the last unrolled iteration.
  // This preserves liveouts of the top level loop.
  if (!isLastUnrollIteration()) {

    for (auto &TempEntry : TempRenamingMap) {
      unsigned OldTempIndex = TempEntry.first;

      if (TempEntry.second.size() > UnrollIteration) {
        unsigned NewTempIndex = TempEntry.second[UnrollIteration];

        Ref->replaceTempBlob(OldTempIndex, NewTempIndex);
      }
    }
  }
}

HLNode *UnrollHelper::getLastNodeInUnrollRange(HLNode *FirstNode) {
  HLNode *LastNode = FirstNode;

  for (HLNode *NextNode = FirstNode; (NextNode && !isa<HLLoop>(NextNode));
       NextNode = NextNode->getNextNode()) {
    LastNode = NextNode;
  }

  return LastNode;
}

void UnrollHelper::updateNodeRange(HLNode *FirstNode, HLNode *LastNode) {
  CanonExprUpdater CEUpdater(*this);

  HLNodeUtils::visitRange(CEUpdater, FirstNode, LastNode);
}

void UnrollHelper::CanonExprUpdater::visit(HLDDNode *Node) {
  assert((UHelper.UnrollIteration < UHelper.getUnrollFactor()) &&
         "Invalid unroll count!");

  for (auto Iter = Node->ddref_begin(), End = Node->ddref_end(); Iter != End;
       ++Iter) {
    processRegDDRef(*Iter);
  }
}

void UnrollHelper::CanonExprUpdater::processRegDDRef(RegDDRef *Ref) {

  UHelper.renameTemps(Ref);

  for (auto Iter = Ref->canon_begin(), End = Ref->canon_end(); Iter != End;
       ++Iter) {
    processCanonExpr(*Iter);
  }
}

/// Processes CanonExpr to modify IV to:
/// IV*UF + (Original IVCoeff)*UnrollIteration.
void UnrollHelper::CanonExprUpdater::processCanonExpr(CanonExpr *CExpr) {
  CExpr->shift(UHelper.UnrollLevel, UHelper.UnrollIteration);

  CExpr->multiplyIVByConstant(UHelper.UnrollLevel, UHelper.UnrollFactor);
  // Cannot simplify unsigned division unless numerator is known to be
  // non-negative. Can use HLNodeUtils::isKnownNonNegative() if simplification
  // is required for performance.
  // CExpr->simplify(true);
}

static void createUnrolledNodeRange(HLNode *FirstNode, HLNode *LastNode,
                                    HLContainerTy &NodeRange,
                                    UnrollHelper &UHelper) {
  assert(NodeRange.empty() && "Empty node range expected!");

  HLNode *CurFirstChild = nullptr;
  HLNode *CurLastChild = nullptr;

  unsigned UnrollFactor = UHelper.getUnrollFactor();
  unsigned UnrollTrip =
      UHelper.needRemainderLoop() ? UnrollFactor : UnrollFactor - 1;

  // TODO: Consider not replicating node range which is invariant w.r.t loop
  // like t = 0.

  for (unsigned UnrollIter = 0; UnrollIter < UnrollTrip; ++UnrollIter) {
    HLNodeUtils::cloneSequence(&NodeRange, FirstNode, LastNode);

    CurFirstChild = (UnrollIter == 0)
                        ? &(NodeRange.front())
                        : &*(std::next(CurLastChild->getIterator()));
    CurLastChild = &(NodeRange.back());

    UHelper.setUnrollIteration(UnrollIter);
    UHelper.updateNodeRange(CurFirstChild, CurLastChild);

    UHelper.patchIntermediateBottomTestForUnknownLoop(CurLastChild);
  }

  // Reuse original nodes for the last unrolled iteration.
  if (!UHelper.needRemainderLoop()) {
    UHelper.setUnrollIteration(UnrollTrip);
    UHelper.updateNodeRange(FirstNode, LastNode);

    HLNodeUtils::remove(&NodeRange, FirstNode, LastNode);
  }
}

static void unrollLoopRecursive(HLLoop *OrigLoop, HLLoop *NewLoop,
                                UnrollHelper &UHelper, bool IsTopLoop) {
  HLContainerTy NodeRange;

  if (!IsTopLoop) {
    UHelper.setCurOrigLoop(OrigLoop->getParentLoop());

    // Unroll preheader/postexit for non top level loops.
    if (OrigLoop->hasPreheader()) {
      createUnrolledNodeRange(OrigLoop->getFirstPreheaderNode(),
                              OrigLoop->getLastPreheaderNode(), NodeRange,
                              UHelper);
      HLNodeUtils::insertAsFirstPreheaderNodes(NewLoop, &NodeRange);
    }

    if (OrigLoop->hasPostexit()) {
      createUnrolledNodeRange(OrigLoop->getFirstPostexitNode(),
                              OrigLoop->getLastPostexitNode(), NodeRange,
                              UHelper);
      HLNodeUtils::insertAsFirstPostexitNodes(NewLoop, &NodeRange);
    }

    // Opt reports for inner loops in unroll & jam mode are moved here.
    LoopOptReportBuilder &LORBuilder =
        OrigLoop->getHLNodeUtils().getHIRFramework().getLORBuilder();

    LORBuilder(*OrigLoop).moveOptReportTo(*NewLoop);
  }

  HLNode *CurFirstNode = OrigLoop->getFirstChild();

  if (UHelper.isUnknownLoopUnroll()) {
    // Skip loop label cloning for unknown loops.
    CurFirstNode = CurFirstNode->getNextNode();
  }

  bool IsUnrollJam = UHelper.isUnrollJamMode();

  // Avoid unnecessary node traversal for innermost loops and general unroll as
  // the body will be handled as a single node range.
  bool NeedSingleNodeRange = (!IsUnrollJam || OrigLoop->isInnermost());

  while (CurFirstNode) {
    HLNode *CurLastNode =
        (NeedSingleNodeRange)
            ? OrigLoop->getLastChild()
            : UnrollHelper::getLastNodeInUnrollRange(CurFirstNode);

    // Keep pointer to next node in case this one is moved (for last unrolled
    // iteration).
    HLNode *NextFirstNode = CurLastNode->getNextNode();

    HLLoop *ChildLoop = IsUnrollJam ? dyn_cast<HLLoop>(CurFirstNode) : nullptr;

    if (ChildLoop) {
      // Unroll & Jam mode
      assert((CurFirstNode == CurLastNode) &&
             "Single node range expected for loops!");

      HLLoop *NewInnerLoop = ChildLoop->cloneEmpty();
      UHelper.updateLoopMap(ChildLoop, NewInnerLoop);

      HLNodeUtils::insertAsLastChild(NewLoop, NewInnerLoop);
      unrollLoopRecursive(ChildLoop, NewInnerLoop, UHelper, false);

    } else {
      UHelper.setCurOrigLoop(OrigLoop);

      createUnrolledNodeRange(CurFirstNode, CurLastNode, NodeRange, UHelper);
      HLNodeUtils::insertAsLastChildren(NewLoop, &NodeRange);
    }

    CurFirstNode = NextFirstNode;
  }

  // Top level loop's liveins/liveouts do not change.
  if (!IsTopLoop) {
    UHelper.addRenamedTempsAsLiveinLiveout(NewLoop);
  }
}

static void unrollMainLoop(HLLoop *OrigLoop, HLLoop *MainLoop,
                           unsigned UnrollFactor, bool NeedRemainderLoop,
                           LoopMapTy *LoopMap) {

  auto &HNU = OrigLoop->getHLNodeUtils();
  HLLabel *UnknownLoopExitLabel = nullptr;

  // Unknown loop unrollng.
  if (OrigLoop == MainLoop) {
    assert(OrigLoop->isUnknown() && "Unknown loop expected!");
    assert(!LoopMap && "Cannot unroll & jam unknown loop!");

    // Extract postexit before adding an exit label.
    MainLoop->extractPostexit();

    // Insert exit label.
    UnknownLoopExitLabel = HNU.createHLLabel("loopexit");
    HLNodeUtils::insertAfter(MainLoop, UnknownLoopExitLabel);
  }

  UnrollHelper UHelper(OrigLoop, UnrollFactor, LoopMap, UnknownLoopExitLabel,
                       NeedRemainderLoop);

  HLNode *MarkerNode = HNU.getOrCreateMarkerNode();

  // Replace loop by marker node, until we are done populating it so we can
  // insert all the nodes in one go.
  // This saves multiple topsort num recalculations.
  HLNodeUtils::replace(MainLoop, MarkerNode);

  unrollLoopRecursive(OrigLoop, MainLoop, UHelper, true);

  // Insert loop back in HIR.
  HLNodeUtils::replace(MarkerNode, MainLoop);
}

// This function should be called only after
// ProfileData of Loops itself has updated through
// setupPeelMainAndRemainderLoops.
static void updateProfDataInLoopBody(HLLoop *UnrolledLoop,
                                     HLLoop *RemainderLoop,
                                     unsigned UnrollFactor,
                                     uint64_t LoopBackedgeWeightBeforeUnroll) {

  // Its children do not have ProfData either.
  if (!UnrolledLoop->getProfileData())
    return;

  HIRTransformUtils::divideProfileDataBy(
      UnrolledLoop->child_begin(), UnrolledLoop->child_end(), UnrollFactor);
  if (!RemainderLoop)
    return;

  // Now update remainder loop's body
  // We need to derived a denominator D, where
  // (T_new, F_new) of a HLNode in remainder loop body
  //    := (T_orig / D, F_orig / D).
  // Suppose L is the original loop, where UnrolledLoop and RemainderLoop are
  // derived.
  // We use following D for RemainderLoop:
  //  D := (T_orig of L) / { (T_orig of L) % (UnrollFacotr of L) }
  //
  // Rationale:
  // For the main unrolled loop,
  // (T_new, F_new) = (T_orig / U, F_orig / U)
  // where T/F_orig and U are branch_weights and Unroll factor of "L".
  //  Denominator for main loop = U = T_orig / T_new
  //
  // Similarly,
  //  Denominator for rem loop = T_orig /
  //    (new portion of branch_weights asssigned to rem loop).
  //  (new portion of branch_weights asssigned to rem loop) = T_orig % U
  //  Thus, Denominator for rem loop = T_orig / (T_orig % U).

  uint64_t RemainderLoopTrueWeight, FalseWeight;
  RemainderLoop->extractProfileData(RemainderLoopTrueWeight, FalseWeight);
  if (RemainderLoopTrueWeight == 0) {
    RemainderLoopTrueWeight = 1;
  }
  uint64_t Denominator =
      LoopBackedgeWeightBeforeUnroll / RemainderLoopTrueWeight;

  if (Denominator == 0)
    return;

  HIRTransformUtils::divideProfileDataBy(
      RemainderLoop->child_begin(), RemainderLoop->child_end(), Denominator);
}

void unrollLoopImpl(HLLoop *Loop, unsigned UnrollFactor, LoopMapTy *LoopMap,
                    HLLoop **UnrolledLoop, HLLoop **RemainderLoop) {
  assert(Loop && "Loop is null!");
  assert((UnrollFactor > 1) && "Invalid unroll factor!");

  bool NeedRemainderLoop = false;
  bool IsUnknownLoop = Loop->isUnknown();
  HLLoop *MainLoop = nullptr;

  LoopOptReportBuilder &LORBuilder =
      Loop->getHLNodeUtils().getHIRFramework().getLORBuilder();

  uint64_t LoopBackedgeWeightBeforeUnroll = 0;
  uint64_t FalseWeight = 0;
  Loop->extractProfileData(LoopBackedgeWeightBeforeUnroll, FalseWeight);
  if (IsUnknownLoop) {
    MainLoop = Loop;
    MainLoop->getParentRegion()->setGenCode();
    MainLoop->setNumExits(MainLoop->getNumExits() * UnrollFactor);
    MainLoop->dividePragmaBasedTripCount(UnrollFactor);

    LORBuilder(*MainLoop).addRemark(
        OptReportVerbosity::Low,
        "Unknown loop has been partially unrolled with %d factor",
        UnrollFactor);

  } else {
    // Create the unrolled main loop and setup remainder loop.
    MainLoop = HIRTransformUtils::setupPeelMainAndRemainderLoops(
        Loop, UnrollFactor, NeedRemainderLoop, LORBuilder,
        LoopMap ? OptimizationType::UnrollAndJam : OptimizationType::Unroll);
  }

  if (!NeedRemainderLoop && !IsUnknownLoop) {
    // Invalidate analysis for original loopnest if remainder loop is not needed
    // since we reuse the instructions inside them.
    HIRInvalidationUtils::invalidateLoopNestBody(Loop);
  }
  unrollMainLoop(Loop, MainLoop, UnrollFactor, NeedRemainderLoop, LoopMap);

  updateProfDataInLoopBody(MainLoop, NeedRemainderLoop ? Loop : nullptr,
                           UnrollFactor, LoopBackedgeWeightBeforeUnroll);

  // If a remainder loop is not needed get rid of the OrigLoop at this point.
  if (!NeedRemainderLoop && !IsUnknownLoop) {
    HLNodeUtils::remove(Loop);
  }

  if (UnrolledLoop) {
    *UnrolledLoop = MainLoop;
  }

  if (RemainderLoop) {
    *RemainderLoop = NeedRemainderLoop ? Loop : nullptr;
  }
}

PreservedAnalyses HIRUnrollAndJamPass::run(llvm::Function &F,
                                           llvm::FunctionAnalysisManager &AM) {
  HIRUnrollAndJam(AM.getResult<HIRFrameworkAnalysis>(F),
                  AM.getResult<HIRLoopStatisticsAnalysis>(F),
                  AM.getResult<HIRLoopResourceAnalysis>(F),
                  AM.getResult<HIRLoopLocalityAnalysis>(F),
                  AM.getResult<HIRDDAnalysisPass>(F),
                  AM.getResult<HIRSafeReductionAnalysisPass>(F), false)
      .run();

  return PreservedAnalyses::all();
}

class HIRUnrollAndJamLegacyPass : public HIRTransformPass {
public:
  static char ID;
  bool PragmaOnlyUnroll;

  HIRUnrollAndJamLegacyPass(bool PragmaOnlyUnroll = false)
      : HIRTransformPass(ID), PragmaOnlyUnroll(PragmaOnlyUnroll) {
    initializeHIRUnrollAndJamLegacyPassPass(*PassRegistry::getPassRegistry());
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.setPreservesAll();
    AU.addRequiredTransitive<HIRFrameworkWrapperPass>();
    AU.addRequiredTransitive<HIRLoopStatisticsWrapperPass>();
    AU.addRequiredTransitive<HIRLoopResourceWrapperPass>();
    AU.addRequiredTransitive<HIRLoopLocalityWrapperPass>();
    AU.addRequiredTransitive<HIRDDAnalysisWrapperPass>();
    AU.addRequiredTransitive<HIRSafeReductionAnalysisWrapperPass>();
  }

  bool runOnFunction(Function &F) override {
    if (skipFunction(F)) {
      return false;
    }

    return HIRUnrollAndJam(
               getAnalysis<HIRFrameworkWrapperPass>().getHIR(),
               getAnalysis<HIRLoopStatisticsWrapperPass>().getHLS(),
               getAnalysis<HIRLoopResourceWrapperPass>().getHLR(),
               getAnalysis<HIRLoopLocalityWrapperPass>().getHLL(),
               getAnalysis<HIRDDAnalysisWrapperPass>().getDDA(),
               getAnalysis<HIRSafeReductionAnalysisWrapperPass>().getHSR(),
               PragmaOnlyUnroll)
        .run();
  }
};

char HIRUnrollAndJamLegacyPass::ID = 0;
INITIALIZE_PASS_BEGIN(HIRUnrollAndJamLegacyPass, "hir-unroll-and-jam",
                      "HIR Unroll & Jam", false, false)
INITIALIZE_PASS_DEPENDENCY(HIRFrameworkWrapperPass)
INITIALIZE_PASS_DEPENDENCY(HIRLoopStatisticsWrapperPass)
INITIALIZE_PASS_DEPENDENCY(HIRLoopResourceWrapperPass)
INITIALIZE_PASS_DEPENDENCY(HIRLoopLocalityWrapperPass)
INITIALIZE_PASS_DEPENDENCY(HIRDDAnalysisWrapperPass)
INITIALIZE_PASS_DEPENDENCY(HIRSafeReductionAnalysisWrapperPass)
INITIALIZE_PASS_END(HIRUnrollAndJamLegacyPass, "hir-unroll-and-jam",
                    "HIR Unroll & Jam", false, false)

FunctionPass *llvm::createHIRUnrollAndJamPass(bool PragmaOnlyUnroll) {
  return new HIRUnrollAndJamLegacyPass(PragmaOnlyUnroll);
}
