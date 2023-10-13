//===----- HIRUnrollAndJam.cpp - Implements UnrollAndJam class ------------===//
//
// Copyright (C) 2015 Intel Corporation. All rights reserved.
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
#include "HIRUnroll.h"
#include "llvm/Transforms/Intel_LoopTransforms/HIRUnrollAndJamPass.h"

#include "llvm/ADT/Statistic.h"
#include "llvm/IR/Function.h"
#include "llvm/InitializePasses.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/MathExtras.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Utils/Cloning.h"

#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/DDTests.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRDDAnalysis.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRLocalityAnalysis.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRLoopResource.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRLoopStatistics.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRSafeReductionAnalysis.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Framework/HIRFramework.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HIRInvalidationUtils.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HLNodeUtils.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/SIMDIntrinsicChecker.h"

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
    "hir-unroll-and-jam-max-outer-loop-cost", cl::init(36), cl::Hidden,
    cl::desc("Max allowed cost of an outer loop in the loopnest"));

static cl::opt<unsigned> MaxLoopMemRefsThreshold(
    "hir-unroll-and-jam-max-unrolled-loop-memrefs", cl::init(26), cl::Hidden,
    cl::desc("Max allowed number of memrefs in the unrolled loopnest"));

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
                  HIRLoopResource &HLR, HIRDDAnalysis &DDA,
                  HIRSafeReductionAnalysis &HSRA, bool PragmaOnlyUnroll)
      : HIRF(HIRF), HLS(HLS), HLR(HLR), DDA(DDA), HSRA(HSRA),
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

  HLNode *SkipNode;
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
  Analyzer(HIRUnrollAndJam &HUAJ) : HUAJ(HUAJ), SkipNode(nullptr) {}

  /// Performs preliminary checks to throttle loops for unroll & jam.
  void visit(HLLoop *Lp);

  /// Peforms profitability and legality checks on outer loops.
  void postVisit(HLLoop *Lp);

  /// Do nothing for instructions.
  void visit(HLInst *Inst) {}

  /// Handle nodes other than HLLoop or HLInst.
  void visit(HLNode *Node);

  void postVisit(HLNode *) {}

  bool skipRecursion(const HLNode* Node) const { return Node == SkipNode; }

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
  bool isLegalToPermute(const DirectionVector &DV, const RegDDRef *SrcRef,
                        const HLLoop *SrcLoop) const;

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
                                       const RegDDRef *SrcRef,
                                       const HLLoop *SrcLoop) const {
  // Legality check is the same as interchanging CandidateLoop with the
  // innermost loop so we check whether swapping the corresponding DV elements
  // yields a legal DV.

  // 1. Check if dependence is carried by an outer loop which makes interchange
  // legal.
  if (DV.isIndepFromLevel(LoopLevel)) {
    return true;
  }

  unsigned LastLevel = DV.size();

  DVKind LoopLevelDV = DV[LoopLevel - 1];
  DVKind InnermostDV = DV[LastLevel - 1];

  bool IsInnermostLoopEdge =
      (SrcLoop->isInnermost() && (LastLevel == SrcLoop->getNestingLevel()));

  // 2. If the DV is independent at the candidate loop level (=), we can unroll
  // if-
  //
  // a) SrcRef is a temp ref which can be renamed, Or
  //
  // b) SrcRef is a memref which yields a distinct memory location in each
  // unrolled iteration. This is the same as checking whether the memref has
  // loop level IV. For example, for a candidate i1 loop, A[i1][i2] is a valid
  // ref but A[0][i2] is not, Or
  //
  // c) Edge is innermost loop level. What happens to innermost loop body is
  // more akin to unrolling rather than unroll & jam so it doesn't require a)
  // or b) above.
  if (LoopLevelDV == DVKind::EQ) {
    return (IsInnermostLoopEdge || SrcRef->isTerminalRef() ||
            (SrcRef->isMemRef() && SrcRef->hasIV(LoopLevel)));
  }

  // 3. Any dependency carried by the candidate loop in an outer loop should
  // prevent unroll & jam.
  if (!IsInnermostLoopEdge) {
    return false;
  }

  if (InnermostDV == DVKind::NONE) {
    // Ideally, we should treat NONE the same as EQ as either of them shouldn't
    // prevent unroll & jam but NONE seems to be incorrectly derived using
    // noalias metadata so we use conservative ALL DV.
    InnermostDV = DVKind::ALL;
  }

  // 4. We can always permute these combinations-
  // (<, <)
  // (=, =)
  // (>, >)
  if (LoopLevelDV == InnermostDV) {
    if ((LoopLevelDV == DVKind::LT) || (LoopLevelDV == DVKind::EQ) ||
        (LoopLevelDV == DVKind::GT)) {
      return true;
    }
  }

  // 5. We cannot permute outer and inner DV elements if the direction is
  // reversed in any combination after the permutation. For example (*, <)
  // yields (<, <), (=, <) and (<, >) after decomposing. The direction of (<, >)
  // gets reversed after permutation.
  if (((LoopLevelDV & DVKind::LT) && (InnermostDV & DVKind::GT)) ||
      ((LoopLevelDV & DVKind::GT) && (InnermostDV & DVKind::LT))) {
    return false;
  }

  DVKind ValidDV, InvalidDV;

  // 6. Now we check if any of the DV elements between LoopLevel to innermost
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

static bool isSIMDOrLifetimeIntrinsic(const HLNode *Node) {
  auto *HInst = dyn_cast<HLInst>(Node);

  if (!HInst) {
    return false;
  }

  if (HInst->isSIMDDirective()) {
    return true;
  }

  return HInst->isLifetimeIntrinsic();
}

static bool canIgnoreNode(const HLNode *Node) {
  // We can ignore edges from/to SIMD and liftime intrinsics as unroll & jam
  // will not violate their semantics.
  return isSIMDOrLifetimeIntrinsic(Node);
}

void LegalityChecker::visit(const HLDDNode *Node) {

  if (canIgnoreNode(Node)) {
    return;
  }

  auto *ParentLoop = Node->getLexicalParentLoop();

  for (auto RefIt = Node->ddref_begin(), E = Node->ddref_end(); RefIt != E;
       ++RefIt) {

    auto *Ref = (*RefIt);

    if (canIgnoreRef(Ref, ParentLoop)) {
      continue;
    }

    for (auto *Edge : DDG.outgoing(Ref)) {

      auto *SinkRef = Edge->getSink();

      if (canIgnoreNode(SinkRef->getHLDDNode())) {
        continue;
      }

      if (!isLegalToPermute(Edge->getDV(), Ref, ParentLoop)) {
        LLVM_DEBUG(dbgs() << "Illegal edge found: ");
        LLVM_DEBUG(Edge->dump());
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

  if (!Lp->isInnermost() &&
      HLNodeUtils::hasManyLifeTimeIntrinsics(Lp)) {
    LLVM_DEBUG(dbgs() << "Avoiding UaJ due to LifeTime\n");
    LLVM_DEBUG(Lp->dump());

    HUAJ.throttleRecursively(Lp);
    SkipNode = Lp;
    return;
  }

  if (!Lp->isDo()) {
    LLVM_DEBUG(dbgs() << "Skipping unroll & jam of non-DO loop!\n");
    HUAJ.throttleRecursively(Lp);
    return;
  }

  if (Lp->hasUnrollEnablingPragma()) {
    LLVM_DEBUG(dbgs() << "Skipping unroll & jam as loop has unroll pragma!\n");
    HUAJ.throttleRecursively(Lp);
    return;
  }

  const HLInst *SIMDEntryDir = Lp->getSIMDEntryIntrinsic();
  bool IsValidInnerLoop = !SIMDEntryDir;

  if (SIMDEntryDir) {
    SIMDIntrinsicChecker SIC(SIMDEntryDir, Lp);
    // We do not handle any clauses which have associated pre/post instructions
    // like reductions.
    IsValidInnerLoop = SIC.isHandleable() && !SIC.hasReductions();
  }

  if (!IsValidInnerLoop) {
    LLVM_DEBUG(dbgs() << "Skipping unroll & jam of non-handleable SIMD loop "
                         "and all its parent loops!\n");
    HUAJ.throttleRecursively(Lp);
    return;

  } else if (SIMDEntryDir) {
    LLVM_DEBUG(dbgs() << "Skipping unroll & jam of SIMD loop!\n");
    HUAJ.throttle(Lp);
    return;
  }

  auto &LS = HUAJ.HLS.getSelfStatistics(Lp);

  // Cannot unroll loop if it has calls with noduplicate attribute.
  if (LS.hasCallsWithNoDuplicate()) {
    LLVM_DEBUG(
        dbgs() << "Skipping unroll & jam of loopnest containing call(s) with "
                  "NoDuplicate attribute !\n");
    HUAJ.throttleRecursively(Lp);
    return;
  }

  if (LS.hasNonSIMDCallsWithUnsafeSideEffects()) {
    LLVM_DEBUG(
        dbgs() << "Skipping unroll & jam of loopnest containing call(s) with "
                  "unsafe side effects!\n");
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
  }

  // Throttle unroll of outer loop whose inner loop's bounds varies within the
  // outer loop, as they cannot be fused.
  if (Lp->getParentLoop()) {
    for (auto *Ref : make_range(Lp->ddref_begin(), Lp->ddref_end())) {

      if (unsigned DefLevel = Ref->getDefinedAtLevel()) {
        LLVM_DEBUG(
            dbgs() << "Skipping unroll & jam for loopnest as it is illegal!\n");
        HUAJ.throttleRecursively(Lp->getParentLoopAtLevel(DefLevel));
      }

      for (auto *CE : make_range(Ref->canon_begin(), Ref->canon_end())) {
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

  HLNodeUtils::gatherAllLoops(OuterLp, Loops);

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

  unsigned AvgTripCount = 0;
  if ((IsConstTC ||
       (Lp->getPragmaBasedAverageTripCount(AvgTripCount) &&
        (TC = AvgTripCount)) ||
       (TC = Lp->getMaxTripCountEstimate())) &&
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
          dbgs() << "Skipping unroll & jam of loop as the outer loop body cost "
                    "exceeds threshold, cost = "
                 << LoopCost << ", threshold = " << MaxOuterLoopCost << "\n");
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
                    "cost exceeds threshold, cost = "
                 << (2 * LoopNestCost)
                 << ", threshold = " << MaxUnrolledLoopNestCost << "\n");
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

  if (!HIRLoopLocality::hasTemporalLocality(ParLp, 1, true, true)) {
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

// Checks for specific pattern for \p LoadRef with \p LoadNum ranging from
// [1-4]. Refer to the caller below for the actual pattern.
static bool isMatchingLoad(const RegDDRef *LoadRef, unsigned LoadNum) {
  assert(LoadRef->isMemRef() && "Memref expected!");
  assert((LoadNum >= 1) && (LoadNum <= 4) && "Unexpected load number!");

  if (LoadRef->getNumDimensions() != 1) {
    return false;
  }

  auto *IndexCE = LoadRef->getDimensionIndex(1);

  if ((IndexCE->getDenominator() != 1) ||
      (IndexCE->getConstant() != (LoadNum - 1))) {
    return false;
  }

  int64_t Coeff;
  unsigned Index;

  IndexCE->getIVCoeff(3, &Index, &Coeff);

  if ((Index != InvalidBlobIndex) || (Coeff != 2)) {
    return false;
  }

  if (LoadNum == 1 || LoadNum == 2) {
    if ((IndexCE->numIVs() != 1) || (IndexCE->numBlobs() != 0)) {
      return false;
    }

    return true;
  }

  IndexCE->getIVCoeff(2, &Index, &Coeff);

  if ((Index == InvalidBlobIndex) || (IndexCE->numIVs() != 2) ||
      (IndexCE->numBlobs() != 1)) {
    return false;
  }

  return true;
}

// We want to disable unroll and jam for this pattern-
//
// + DO i2 = 0, ((-1 + sext.i32.i64(%11)) /u sext.i32.i64((2 * %86))), 1
// |   + DO i3 = 0, %86 + -2, 1   <DO_LOOP>
// |   |   %133 = (%84)[2 * i3];
// |   |   %134 = (%84)[2 * i3 + 1];
// |   |   %146 = (%7)[sext.i32.i64((4 * %86)) * i2 + 2 * i3 + 2 *
// zext.i32.i64(%86) + 2]; |   |   %150 = (%7)[sext.i32.i64((4 * %86)) * i2 + 2
// * i3 + 2 * zext.i32.i64(%86) + 3];
static bool isNonProfitablePattern(const HLLoop *Lp) {
  if (Lp->getNestingLevel() != 2) {
    return false;
  }

  const HLLoop *InnermostLp = nullptr;
  if (!HLNodeUtils::isPerfectLoopNest(Lp, &InnermostLp) ||
      (InnermostLp->getNestingLevel() != 3)) {
    return false;
  }

  auto *ChildNode = InnermostLp->getFirstChild();

  for (unsigned I = 1; I <= 4; ++I, ChildNode = ChildNode->getNextNode()) {
    auto *LInst = dyn_cast_or_null<HLInst>(ChildNode);

    if (!LInst) {
      return false;
    }

    if (!isa<LoadInst>(LInst->getLLVMInstruction())) {
      return false;
    }

    if (!isMatchingLoad(LInst->getRvalDDRef(), I)) {
      return false;
    }
  }

  return true;
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
    unsigned NumMemRefs = 0;

    HIRLoopLocality::LocalityRefGatherer::MapTy MemRefMap;

    HIRLoopLocality::LocalityRefGatherer::gatherRange(
        Lp->child_begin(), Lp->child_end(), MemRefMap);

    for (auto &Entry : MemRefMap) {
      NumMemRefs += Entry.second.size();
    }

    LLVM_DEBUG(dbgs() << "Number of memrefs in loop: " << NumMemRefs << "\n");

    // Unrolling loop with too many memrefs can result in memory stalls by
    // increasing presure on memory banks.
    if (NumMemRefs > MaxLoopMemRefsThreshold) {
      LLVM_DEBUG(dbgs() << "Skipping unroll & jam for loop and all its parents "
                           "as the number of memrefs exceeds threshold!\n");
      HUAJ.throttleRecursively(Lp);
      return;
    }

    if (!HIRLoopLocality::hasTemporalLocality(Lp, UnrollFactor - 1, MemRefMap,
                                              true, true)) {
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

    // Do another around of temporal locality check by taking aliasing into
    // account. We do this after the legality check as it requires DDGraph which
    // is built during legality check.
    // TODO: refine unroll factor during legality check using dependencies
    // carried by Lp and pass the refined factor here.
    if (!HIRLoopLocality::hasTemporalLocality(Lp, UnrollFactor - 1, true, true,
                                              &HUAJ.DDA)) {
      LLVM_DEBUG(dbgs() << "Skipping unroll & jam as aliasing prevents us from "
                           "exploiting temporal locality!\n");
      HUAJ.throttle(Lp);
      return;
    }

    if (isNonProfitablePattern(Lp)) {
      LLVM_DEBUG(
          dbgs() << "Skipping unroll & jam for non-profitable pattern!\n");
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

  bool Modified = false;

  for (auto Loop : OutermostLoops) {
    AY.analyze(Loop);
    unrollCandidates(Loop);

    Modified = Modified || HaveUnrollCandidates;

    clearCandidates();
  }

  return Modified;
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

  NoAliasScopeMapTy NoAliasScopeMap;

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

  HLLoop *getOrigTopLevelLoop() { return OrigTopLevelLoop; }

  void setUnrollIteration(unsigned Count) { UnrollIteration = Count; }
  unsigned getUnrollFactor() const { return UnrollFactor; }

  void updateLoopMap(HLLoop *OrigLoop, HLLoop *NewLoop) {
    assert(LoopMap && "Non-null loop map expected!");
    LoopMap->emplace_back(OrigLoop, NewLoop);
  }

  bool isUnrollJamMode() const { return LoopMap != nullptr; }
  bool isUnknownLoopUnroll() const { return UnknownLoopExitLabel != nullptr; }

  // Map of original noalias scopes to the corresponding cloned scopes for the
  // current unroll iteration at all loop levels involved in unroll/unroll &
  // jam.
  NoAliasScopeMapTy &getNoAliasScopeMap() { return NoAliasScopeMap; }

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
  unsigned CurLevel;

  void processRegDDRef(RegDDRef *RegDD);

  // Returns true if CExpr was modified.
  bool processCanonExpr(CanonExpr *CExpr);

public:
  CanonExprUpdater(UnrollHelper &UHelper)
      : UHelper(UHelper), CurLevel(UHelper.CurOrigLoop->getNestingLevel(
                              false /*AssertIfDetached*/)) {}

  /// No processing needed for Goto
  void visit(HLGoto *Goto){};
  /// No processing needed for Label
  void visit(HLLabel *Label){};

  void visit(HLLoop *Lp) {
    ++CurLevel;
    visit(cast<HLDDNode>(Lp));
  }

  void visit(HLDDNode *Node);
  void visit(HLNode *Node) {
    llvm_unreachable(" Node not supported for unrolling.");
  };

  void postVisit(HLLoop *Lp) { --CurLevel; }

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

  // Create new mapping for temps defined in outer loops.
  if (!CurOrigLoop->isInnermost()) {
    return true;
  }

  // Do not rename temps in loopnest reductions.
  // These are the only allowed livein temps for unroll & jam.
  if (OrigTopLevelLoop->isLiveIn(LvalSymbase)) {
    return false;
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

static bool isSIMDDirective(HLNode *Node) {

  auto *Inst = dyn_cast<HLInst>(Node);

  if (!Inst) {
    return false;
  }

  return Inst->isSIMDDirective() || Inst->isSIMDEndDirective();
}

HLNode *UnrollHelper::getLastNodeInUnrollRange(HLNode *FirstNode) {
  HLNode *LastNode = FirstNode;

  for (HLNode *NextNode = FirstNode;
       (NextNode && !isa<HLLoop>(NextNode) && !isSIMDDirective(NextNode));
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

  bool RemovedIV = false;
  for (auto *CE : Ref->canons()) {
    if (processCanonExpr(CE) && !CE->hasIV(UHelper.UnrollLevel)) {
      RemovedIV = true;
    }
  }

  if (RemovedIV) {
    Ref->makeConsistent({}, CurLevel);
  }

  Ref->replaceNoAliasScopeInfo(UHelper.getNoAliasScopeMap());
}

/// Processes CanonExpr to modify IV to:
/// IV*UF + (Original IVCoeff)*UnrollIteration.
bool UnrollHelper::CanonExprUpdater::processCanonExpr(CanonExpr *CExpr) {

  if (!CExpr->hasIV(UHelper.UnrollLevel))
    return false;

  CExpr->shift(UHelper.UnrollLevel, UHelper.UnrollIteration);

  CExpr->multiplyIVByConstant(UHelper.UnrollLevel, UHelper.UnrollFactor);
  // Cannot simplify unsigned division unless numerator is known to be
  // non-negative. Can use HLNodeUtils::isKnownNonNegative() if simplification
  // is required for performance.
  // CExpr->simplify(true);

  return true;
}

// Populates the scopes for all loop levels involved in unroll & jam. Since we
// are effectively unrolling the TopLevelLoop, the scopes at all inner levels
// have to be updated.
static void populateNoAliasScopeListsForLoopnest(
    HLLoop *OrigParentLoop, HLLoop *OrigTopLevelLoop,
    SmallVectorImpl<MDNode *> &NoAliasScopeLists) {

  for (auto Lp = OrigParentLoop, EndLp = OrigTopLevelLoop->getParentLoop();
       Lp != EndLp; Lp = Lp->getParentLoop()) {
    auto OrigNoAliasScopeLists = Lp->getNoAliasScopeLists();
    NoAliasScopeLists.append(OrigNoAliasScopeLists.begin(),
                             OrigNoAliasScopeLists.end());
  }
}

// Add cloned scopes for each orig parent loop to the corresponding new loop.
static void addClonedScopes(HLLoop *OrigParentLoop, HLLoop *NewParentLoop,
                            HLLoop *OrigTopLevelLoop,
                            NoAliasScopeMapTy &NoAliasScopeMap) {

  // Special casing for unknown loop unrolling. We need to copy the scope list
  // into a temporary vector to pass to addMappedNoAliasScopes() as both orig
  // and new loops are the same.
  if (OrigParentLoop == NewParentLoop) {
    auto OrigNoAliasScopeLists = OrigParentLoop->getNoAliasScopeLists();
    SmallVector<MDNode *, 4> OrigNoAliasScopeListsVec(
        OrigNoAliasScopeLists.begin(), OrigNoAliasScopeLists.end());

    NewParentLoop->addMappedNoAliasScopes(OrigNoAliasScopeListsVec,
                                          NoAliasScopeMap);
    return;
  }

  for (auto *EndLp = OrigTopLevelLoop->getParentLoop(); OrigParentLoop != EndLp;
       OrigParentLoop = OrigParentLoop->getParentLoop(),
            NewParentLoop = NewParentLoop->getParentLoop()) {
    NewParentLoop->addMappedNoAliasScopes(
        OrigParentLoop->getNoAliasScopeLists(), NoAliasScopeMap);
  }
}

// \p OrigParentLoop and \p NewParentLoop are the lexical parent loops of the
// node range used to update NoAlias scope info.
static void createUnrolledNodeRange(HLNode *FirstNode, HLNode *LastNode,
                                    HLContainerTy &NodeRange,
                                    UnrollHelper &UHelper,
                                    HLLoop *OrigParentLoop,
                                    HLLoop *NewParentLoop) {
  assert(NodeRange.empty() && "Empty node range expected!");

  HLNode *CurFirstChild = nullptr;
  HLNode *CurLastChild = nullptr;
  SmallVector<MDNode *, 8> NoAliasScopeLists;

  auto &Context = OrigParentLoop->getHLNodeUtils().getContext();
  auto &NoAliasScopeMap = UHelper.getNoAliasScopeMap();
  auto *OrigTopLevelLoop = UHelper.getOrigTopLevelLoop();

  populateNoAliasScopeListsForLoopnest(OrigParentLoop, OrigTopLevelLoop,
                                       NoAliasScopeLists);

  unsigned UnrollFactor = UHelper.getUnrollFactor();
  unsigned UnrollTrip =
      UHelper.needRemainderLoop() ? UnrollFactor : UnrollFactor - 1;

  // TODO: Consider not replicating node range which is invariant w.r.t loop
  // like t = 0.

  for (unsigned UnrollIter = 0; UnrollIter < UnrollTrip; ++UnrollIter) {
    cloneNoAliasScopes(NoAliasScopeLists, NoAliasScopeMap,
                       UHelper.isUnrollJamMode() ? "uj" : "gu", Context);

    addClonedScopes(OrigParentLoop, NewParentLoop, OrigTopLevelLoop,
                    NoAliasScopeMap);

    HLNodeUtils::cloneSequence(&NodeRange, FirstNode, LastNode);

    CurFirstChild = (UnrollIter == 0)
                        ? &(NodeRange.front())
                        : &*(std::next(CurLastChild->getIterator()));
    CurLastChild = &(NodeRange.back());

    UHelper.setUnrollIteration(UnrollIter);
    UHelper.updateNodeRange(CurFirstChild, CurLastChild);

    UHelper.patchIntermediateBottomTestForUnknownLoop(CurLastChild);

    // We clear scope mapping for the next iteration. This is currently a little
    // conservative as we do not preserve outer level scopes across inner
    // sibling loops. To update the information more precisely we will have to
    // store the mapping: {unroll iteration -> scopes} which is much more
    // complicated.
    NoAliasScopeMap.clear();
  }

  // Reuse original nodes for the last unrolled iteration.
  if (!UHelper.needRemainderLoop()) {
    cloneNoAliasScopes(NoAliasScopeLists, NoAliasScopeMap,
                       UHelper.isUnrollJamMode() ? "uj" : "gu", Context);

    addClonedScopes(OrigParentLoop, NewParentLoop, OrigTopLevelLoop,
                    NoAliasScopeMap);

    UHelper.setUnrollIteration(UnrollTrip);
    UHelper.updateNodeRange(FirstNode, LastNode);

    HLNodeUtils::remove(&NodeRange, FirstNode, LastNode);
  }
}

enum LoopChildrenType { Preheader, Postexit, Body };

static void unrollLoopRecursive(HLLoop *OrigLoop, HLLoop *NewLoop,
                                UnrollHelper &UHelper, bool IsTopLoop);

static void createAndInsertUnrolledLoopChildren(HLLoop *OrigLoop,
                                                HLLoop *NewLoop,
                                                UnrollHelper &UHelper,
                                                LoopChildrenType ChildrenTy) {

  HLNode *CurFirstNode = nullptr;

  switch (ChildrenTy) {
  case LoopChildrenType::Preheader:
    CurFirstNode = OrigLoop->getFirstPreheaderNode();
    break;

  case LoopChildrenType::Postexit:
    CurFirstNode = OrigLoop->getFirstPostexitNode();
    break;

  case LoopChildrenType::Body:
    CurFirstNode = OrigLoop->getFirstChild();
    break;
  }

  if (UHelper.isUnknownLoopUnroll()) {
    // Skip loop label cloning for unknown loops.
    CurFirstNode = CurFirstNode->getNextNode();
  }

  bool IsUnrollJam = UHelper.isUnrollJamMode();

  // Avoid unnecessary node traversal for innermost loops and general unroll as
  // the body will be handled as a single node range.
  bool NeedSingleNodeRange = (ChildrenTy == LoopChildrenType::Body) &&
                             (!IsUnrollJam || OrigLoop->isInnermost());

  HLContainerTy NodeRange;

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
      if (ChildrenTy == LoopChildrenType::Body) {
        UHelper.setCurOrigLoop(OrigLoop);
      }

      if (!NeedSingleNodeRange && isSIMDDirective(CurFirstNode)) {
        assert((CurFirstNode == CurLastNode) &&
               "Single node range expected for SIMD directive!");

        // We shouldn't replicate/unroll SIMD directive.
        NodeRange.push_front(*CurFirstNode->clone());

      } else {
        // Pass lexical orig and new parent loops to createUnrolledNodeRange()
        // so it can process NoAlias scopes.
        auto *OrigParentLoop = (ChildrenTy == LoopChildrenType::Body)
                                   ? OrigLoop
                                   : OrigLoop->getParentLoop();
        auto *NewParentLoop = (ChildrenTy == LoopChildrenType::Body)
                                  ? NewLoop
                                  : NewLoop->getParentLoop();

        createUnrolledNodeRange(CurFirstNode, CurLastNode, NodeRange, UHelper,
                                OrigParentLoop, NewParentLoop);
      }

      switch (ChildrenTy) {
      case LoopChildrenType::Preheader:
        HLNodeUtils::insertAsLastPreheaderNodes(NewLoop, &NodeRange);
        break;

      case LoopChildrenType::Postexit:
        HLNodeUtils::insertAsLastPostexitNodes(NewLoop, &NodeRange);
        break;

      case LoopChildrenType::Body:
        HLNodeUtils::insertAsLastChildren(NewLoop, &NodeRange);
        break;
      }
    }

    CurFirstNode = NextFirstNode;
  }
}

static void unrollLoopRecursive(HLLoop *OrigLoop, HLLoop *NewLoop,
                                UnrollHelper &UHelper, bool IsTopLoop) {
  // Clear NewLoop's NoAlias scope lists which were obtained from cloning
  // OrigLoop. New scope lists will be added when doing the actual unrolling
  // below. If we are unrolling unknown loop, both OrigLoop and NewLoop are the
  // same. We cannot erase the scope lists in this case as the original ones are
  // needs for cloning. It is okay to have extra scope lists in the NewLoop even
  // if they don't have uses.
  if (OrigLoop != NewLoop) {
    NewLoop->clearNoAliasScopeLists();
  }

  if (!IsTopLoop) {
    UHelper.setCurOrigLoop(OrigLoop->getParentLoop());

    // Unroll preheader for non top level loops.
    if (OrigLoop->hasPreheader()) {
      createAndInsertUnrolledLoopChildren(OrigLoop, NewLoop, UHelper,
                                          LoopChildrenType::Preheader);
    }

    // Opt reports for inner loops in unroll & jam mode are moved here.
    OptReportBuilder &ORBuilder =
        OrigLoop->getHLNodeUtils().getHIRFramework().getORBuilder();

    ORBuilder(*OrigLoop).moveOptReportTo(*NewLoop);
  }

  createAndInsertUnrolledLoopChildren(OrigLoop, NewLoop, UHelper,
                                      LoopChildrenType::Body);

  // Top level loop's liveins/liveouts do not change.
  if (!IsTopLoop) {
    UHelper.addRenamedTempsAsLiveinLiveout(NewLoop);

    // Unroll postexit for non top level loops.
    if (OrigLoop->hasPostexit()) {
      createAndInsertUnrolledLoopChildren(OrigLoop, NewLoop, UHelper,
                                          LoopChildrenType::Postexit);
    }
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

void unrollLoopImpl(HLLoop *Loop, unsigned UnrollFactor, LoopMapTy *LoopMap,
                    HLLoop **UnrolledLoop, HLLoop **RemainderLoop) {
  assert(Loop && "Loop is null!");
  assert((UnrollFactor > 1) && "Invalid unroll factor!");

  bool NeedRemainderLoop = false;
  bool IsUnknownLoop = Loop->isUnknown();
  HLLoop *MainLoop = nullptr;

  OptReportBuilder &ORBuilder =
      Loop->getHLNodeUtils().getHIRFramework().getORBuilder();

  if (IsUnknownLoop) {
    MainLoop = Loop;
    MainLoop->getParentRegion()->setGenCode();
    MainLoop->setNumExits(MainLoop->getNumExits() * UnrollFactor);
    MainLoop->dividePragmaBasedTripCount(UnrollFactor);
    MainLoop->getBottomTest()->divideProfileData(UnrollFactor);

    // While loop unrolled by %d
    ORBuilder(*MainLoop).addRemark(OptReportVerbosity::Low,
                                   OptRemarkID::WhileLoopUnrollFactor,
                                   UnrollFactor);

  } else {
    // Create the unrolled main loop and setup remainder loop.
    MainLoop = HIRTransformUtils::setupPeelMainAndRemainderLoops(
        Loop, UnrollFactor, NeedRemainderLoop, ORBuilder,
        LoopMap ? OptimizationType::UnrollAndJam : OptimizationType::Unroll);
  }

  if (!NeedRemainderLoop) {
    // Invalidate analysis for original loopnest if remainder loop is not needed
    // since we reuse the instructions inside them. For unknown loops we reuse
    // the entire loop, that's why we need to invalidate the loop body.
    HIRInvalidationUtils::invalidateLoopNestBody(Loop);
  }
  unrollMainLoop(Loop, MainLoop, UnrollFactor, NeedRemainderLoop, LoopMap);

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

  // If we are unrolling only and the loop is multi-exit, then we need to
  // check if the parent loop is multi-exit too. In that case, we need to
  // update the number of exits for all parent loops.
  if (!LoopMap && Loop->isMultiExit()) {
    auto *ParLoop = MainLoop->getParentLoop();
    if (ParLoop && ParLoop->isMultiExit())
      HLNodeUtils::updateNumLoopExits(MainLoop->getOutermostParentLoop());
  }
}

PreservedAnalyses HIRUnrollAndJamPass::runImpl(
    llvm::Function &F, llvm::FunctionAnalysisManager &AM, HIRFramework &HIRF) {
  ModifiedHIR =
      HIRUnrollAndJam(HIRF, AM.getResult<HIRLoopStatisticsAnalysis>(F),
                      AM.getResult<HIRLoopResourceAnalysis>(F),
                      AM.getResult<HIRDDAnalysisPass>(F),
                      AM.getResult<HIRSafeReductionAnalysisPass>(F),
                      PragmaOnlyUnroll)
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
INITIALIZE_PASS_DEPENDENCY(HIRDDAnalysisWrapperPass)
INITIALIZE_PASS_DEPENDENCY(HIRSafeReductionAnalysisWrapperPass)
INITIALIZE_PASS_END(HIRUnrollAndJamLegacyPass, "hir-unroll-and-jam",
                    "HIR Unroll & Jam", false, false)

FunctionPass *llvm::createHIRUnrollAndJamPass(bool PragmaOnlyUnroll) {
  return new HIRUnrollAndJamLegacyPass(PragmaOnlyUnroll);
}
