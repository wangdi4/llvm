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
//  1. For each loopnest, gather legal candidates (inner to outer) based on trip
//  count analysis.
//  2. Refine candidates (outer to inner) based on profitability and legality
//  (dependence on outer loops).
//  3. For each final candidate loop (outer to inner):
//       3.1 Clone LoopChild and insert it before the loop.
//       3.2 Update CanonExprs of LoopChild and recursively visit the inner
//           loops.
//       3.3 Delete Loop
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
//  (1) Extend it for non normalized loops.
//  (2) Add opt report.

#include "llvm/Analysis/Intel_LoopAnalysis/HIRFramework.h"

#include "llvm/ADT/Statistic.h"

#include "llvm/IR/Function.h"
#include "llvm/IR/LLVMContext.h"

#include "llvm/Pass.h"

#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"

#include "llvm/Transforms/Intel_LoopTransforms/HIRTransformPass.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/BlobUtils.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/CanonExprUtils.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/DDRefUtils.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/HIRInvalidationUtils.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/HIRTransformUtils.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/HLNodeUtils.h"

#define DEBUG_TYPE "hir-complete-unroll"

using namespace llvm;
using namespace llvm::loopopt;

// This stat maintains the number of hir loopnests completely unrolled.
STATISTIC(LoopnestsCompletelyUnrolled,
          "Number of HIR loopnests completely unrolled");

static cl::opt<bool>
    DisableHIRCompleteUnroll("disable-hir-complete-unroll", cl::init(false),
                             cl::Hidden,
                             cl::desc("Disable HIR Loop Complete Unrolling"));

static cl::opt<bool> DisableHIRTriCompleteUnroll(
    "disable-hir-tri-complete-unroll", cl::init(false), cl::Hidden,
    cl::desc("Disable HIR Triangular Complete Unrolling"));

// The trip count threshold is intentionally set to a high value as profitablity
// should be driven by the combination of trip count and loop resource.
// TODO: increase threshold after fixing compile time issues.
static cl::opt<unsigned> CompleteUnrollTripThreshold(
    "hir-complete-unroll-trip-threshold", cl::init(50), cl::Hidden,
    cl::desc("Don't unroll if total trip count is bigger than this "
             "threshold."));

// TODO: Create different thresholds for pre and post vector unrolling.
static cl::opt<unsigned> SavingsThreshold(
    "hir-complete-unroll-savings-threshold", cl::init(35), cl::Hidden,
    cl::desc("Least amount of savings (in percentage) for complete unrolling "
             "of a loopnest to be deemed profitable."));

// TODO: increase threshold after fixing compile time issues.
static cl::opt<unsigned> UnrolledLoopMemRefThreshold(
    "hir-complete-unroll-memref-threshold", cl::init(500), cl::Hidden,
    cl::desc("Maximum number of memory refs allowed in completely unrolled "
             "loopnest"));

// TODO: increase threshold after fixing compile time issues.
static cl::opt<unsigned>
    UnrolledLoopDDRefThreshold("hir-complete-unroll-ddref-threshold",
                               cl::init(1000), cl::Hidden,
                               cl::desc("Maximum number of DDRefs allowed in "
                                        "completely unrolled loopnest"));

static cl::opt<unsigned> SmallLoopMemRefThreshold(
    "hir-complete-small-memref-threshold", cl::init(16), cl::Hidden,
    cl::desc("Threshold for memory refs in small loops (higher probability of "
             "unrolling)"));

static cl::opt<unsigned>
    SmallLoopDDRefThreshold("hir-complete-small-ddref-threshold", cl::init(32),
                            cl::Hidden,
                            cl::desc("Threshold for DDRefs in small loops "
                                     "(higher probability of unrolling)"));

namespace {

class HIRCompleteUnroll : public HIRTransformPass {
public:
  static char ID;

  HIRCompleteUnroll() : HIRTransformPass(ID) {
    initializeHIRCompleteUnrollPass(*PassRegistry::getPassRegistry());
  }

  bool runOnFunction(Function &F) override;
  void releaseMemory() override;

  void getAnalysisUsage(AnalysisUsage &AU) const {
    AU.setPreservesAll();
    AU.addRequiredTransitive<HIRFramework>();
    AU.addRequiredTransitive<HIRLoopStatistics>();
  }

private:
  class CanonExprVisitor;
  class ProfitabilityAnalyzer;

  HIRLoopStatistics *HLS;

  /// Storage for loops which will be transformed.
  /// Only outermost loops to be transformed will be stored.
  SmallVector<HLLoop *, 32> CandidateLoops;

  // Caches average trip count of loops for profitability analysis.
  DenseMap<const HLLoop *, unsigned> AvgTripCount;

  // Returns true if loop is eligible for complete unrolling.
  bool isApplicable(const HLLoop *Loop) const;

  /// Computes and returns average trip count for profitability analysis.
  /// Returns -1 if it cannot be computed.
  int64_t computeAvgTripCount(const HLLoop *Loop);

  /// Refines the candidates based on profitability and dependence on outer
  /// loops.
  /// For example, we may have the following case:
  /// for(i=0;i<15; i++)
  ///   for(j=0; j<i; j++)
  /// Here, j loop is added as candidate, but because of profitability, we
  /// don't add 'i' loop, then we remove 'j' loop also.
  void refineCandidates();

  /// This function checks whether children loops contain any IV less than the
  /// OuterCandidateLevel.
  bool satisfiesDependency(unsigned OuterCandidateLevel,
                           SmallVectorImpl<HLLoop *> &ChildLoops) const;

  /// Returns true if loop is profitable for complete unrolling.
  bool isProfitable(const HLLoop *Loop) const;

  /// Performs the complete unrolling transformation.
  static void transformLoop(HLLoop *Loop, HLLoop *OuterLoop,
                            SmallVectorImpl<int64_t> &TripValues);

  /// Main routine to drive the complete unrolling transformation.
  void processCompleteUnroll(SmallVectorImpl<HLLoop *> &OuterLoops);

  /// Performs trip count analysis on the loopnest represented by \p Loop.
  /// Returns the avg trip count of the loopnest. Non-negative value indicates
  /// that loopnest is a candidate.
  int64_t performTripCountAnalysis(HLLoop *Loop);

  /// Routine to drive the transformation of candidate loops.
  void transformLoops();
};

/// Visitor to update the CanonExpr.
class HIRCompleteUnroll::CanonExprVisitor final : public HLNodeVisitorBase {
private:
  HLLoop *OuterLoop;
  SmallVectorImpl<int64_t> *TripValues;

  void processRegDDRef(RegDDRef *RegDD);
  void processCanonExpr(CanonExpr *CExpr, bool IsTerminal);

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

/// Determines if unrolling the loop nest would be profitable.
/// Profitability of the loopnest is determined by giving positive weight to
/// simplification opportunities and negative weight to chance of increase in
/// code size/register pressure. Loopnest is profitable if the accumulated
/// weight is positive.
///
/// Simplification opportunity includes the following cases-
/// 1) Substitution of IV by a constant.
/// 2) Presence of linear blobs (invariance can lead to CSE).
///
/// Code size increase includes the following cases-
/// 1) Presence of non-linear blobs.
class HIRCompleteUnroll::ProfitabilityAnalyzer final
    : public HLNodeVisitorBase {

  const HIRCompleteUnroll &HCU;
  const HLLoop *CurLoop;
  const HLLoop *OuterLoop;

  unsigned Cost;
  unsigned Savings;
  // Savings due to Refs being invariant w.r.t outer loops that are candidates
  // for unrolling. This is kept separate because it is already scaled unlike
  // Savings which is scaled at the end.
  unsigned OuterLoopSavings;

  unsigned NumMemRefs;
  unsigned NumDDRefs;
  bool AccessesStruct;

  // Keeps track of non-linear blobs that we encounter during our traversal so
  // they aren't penalized multiple times. Blobs are removed from the set when
  // we encounter a redefinition of a contained temp.
  DenseSet<unsigned> VisitedNonLinearBlobs;

  // Keeps track of temp blob definitions which get simplified to a constant due
  // to unrolling. This can result in simplification of other instructions.
  DenseSet<unsigned> SimplifiedTempBlobs;

  // Structure to store blob related info.
  struct BlobInfo {
    bool Invariant;
    // Indicates whether the non-linear blob has been encountered before.
    bool Visited;
    // Indicates whether blob definition can be simplified to a constant.
    bool Simplified;
    // Number of operations in the non-linear blob.
    unsigned NumOperations;

    BlobInfo()
        : Invariant(true), Visited(false), Simplified(false), NumOperations(0) {
    }
  };

  // Private constructor used for chilren loops.
  ProfitabilityAnalyzer(const HIRCompleteUnroll &HCU, const HLLoop *CurLp,
                        const HLLoop *OuterLp)
      : HCU(HCU), CurLoop(CurLp), OuterLoop(OuterLp), Cost(0), Savings(0),
        OuterLoopSavings(0), NumMemRefs(0), NumDDRefs(0),
        AccessesStruct(false) {}

  /// Processes RegDDRef for profitability. Returns true if Ref can be
  /// simplified to a constant.
  bool processRef(const RegDDRef *Ref);

  /// Processes CanonExpr for profitability. Returns true if CE can be
  /// simplified to a constant.
  bool processCanonExpr(const CanonExpr *CE, const RegDDRef *ParentRef);

  /// Processes IVs in the CE. Returns true if they can be simplified to a
  /// constant.
  bool processIVs(const CanonExpr *CE, const RegDDRef *ParentRef,
                  unsigned &NumSimplifiedTerms, unsigned &NumNonLinearTerms);

  /// Processes blobs in the CE. Returns true if they can be simplified to a
  /// constant.
  bool processBlobs(const CanonExpr *CE, const RegDDRef *ParentRef,
                    unsigned &NumSimplifiedTerms, unsigned &NumNonLinearTerms);

  /// Adds the cost of the blob given its info and coefficient in the CE.
  void addBlobCost(const BlobInfo &BInfo, int64_t Coeff,
                   unsigned &NumNonLinearTerms);

  /// Returns all the info assodicated with the blob.
  BlobInfo getBlobInfo(unsigned Index, const RegDDRef *ParentRef,
                       bool CEIsLinear);

  /// Updates all the visited blobs which contain the temp represented by self
  /// blob \p Ref. \p Simplified indicates whether the blob definition was
  /// simplified to a constant.
  void updateBlobs(const RegDDRef *Ref, bool Simplified);

  /// Returns percentage savings achieved by unrolling the loopnest.
  unsigned getSavingsInPercentage() const;

  /// Scales the profitability by the given multiplier.
  void scale(unsigned Multiplier) {
    Cost *= Multiplier;
    Savings *= Multiplier;
    NumMemRefs *= Multiplier;
    NumDDRefs *= Multiplier;
  }

  // Adds profitability analysis results from PA to this.
  ProfitabilityAnalyzer &operator+=(const ProfitabilityAnalyzer &PA) {
    Cost += PA.Cost;
    Savings += PA.Savings;
    OuterLoopSavings += PA.OuterLoopSavings;
    NumMemRefs += PA.NumMemRefs;
    NumDDRefs += PA.NumDDRefs;

    return *this;
  }

public:
  ProfitabilityAnalyzer(const HIRCompleteUnroll &HCU, const HLLoop *CurLp)
      : ProfitabilityAnalyzer(HCU, CurLp, CurLp) {}

  // Main interface of the analyzer.
  void analyze();

  // Returns true if loopnest is profitable.
  bool isProfitable() const;

  // Returns true if loop has a small body.
  bool isSmallLoop() const;

  void visit(const HLLoop *Lp);
  void visit(const HLDDNode *Node);

  // No processing needed for Gotos/Labels.
  void visit(const HLGoto *Goto) {}
  void visit(const HLLabel *Label) {}

  void visit(const HLNode *Node) {
    llvm_unreachable("Node not supported for Complete Unrolling.");
  }
  void postVisit(const HLNode *Node) {}

  bool isDone() const override { return AccessesStruct; }
};
}

////// CanonExpr Visitor Start

void HIRCompleteUnroll::CanonExprVisitor::visit(HLLoop *Loop) {
  transformLoop(Loop, OuterLoop, *TripValues);
}

void HIRCompleteUnroll::CanonExprVisitor::visit(HLDDNode *Node) {

  assert(!isa<HLLoop>(Node) && "Loop node not expected!");

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
  bool IsTerminal = RegDD->isTerminalRef();

  // Process CanonExprs inside the RegDDRefs
  for (auto Iter = RegDD->canon_begin(), End = RegDD->canon_end(); Iter != End;
       ++Iter) {
    processCanonExpr(*Iter, IsTerminal);
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
void HIRCompleteUnroll::CanonExprVisitor::processCanonExpr(CanonExpr *CExpr,
                                                           bool IsTerminal) {

  // Start replacing the IV's from OuterLoop level to current loop level.
  auto LoopLevel = OuterLoop->getNestingLevel();
  for (auto &TripV : *TripValues) {
    DEBUG(dbgs() << "Replacing CanonExpr IV by tripval :" << TripV << " \n");
    CExpr->replaceIVByConstant(LoopLevel, TripV);
    CExpr->simplify(IsTerminal);
    LoopLevel++;
  }
}

///// CanonExpr Visitor End

///// ProfitabilityAnalyzer Visitor Start

void HIRCompleteUnroll::ProfitabilityAnalyzer::analyze() {

  CurLoop->getHLNodeUtils().visitRange<true, false>(
      *this, CurLoop->child_begin(), CurLoop->child_end());

  // Scale results by loop's average trip count.
  auto It = HCU.AvgTripCount.find(CurLoop);
  assert((It != HCU.AvgTripCount.end()) && "Trip count of loop not found!");

  // Check if the loop is small enough to assign some extra profitability to it
  // (for eliminating loop control) and give it higher chance of unrolling.
  if (isSmallLoop()) {
    Savings += 4;
  }

  scale(It->second);

  // Add ztt's profitability.
  if (CurLoop->hasZtt()) {
    for (auto RefIt = CurLoop->ztt_ddref_begin(), E = CurLoop->ztt_ddref_end();
         RefIt != E; ++RefIt) {
      processRef(*RefIt);
    }
    // Increment index by number of predicates eliminated.
    Savings += CurLoop->getNumZttPredicates();
  }
}

bool HIRCompleteUnroll::ProfitabilityAnalyzer::isSmallLoop() const {
  return (NumMemRefs <= SmallLoopMemRefThreshold) &&
         (NumDDRefs <= SmallLoopDDRefThreshold);
}

unsigned
HIRCompleteUnroll::ProfitabilityAnalyzer::getSavingsInPercentage() const {
  unsigned SafeCost = (Cost == 0) ? 1 : Cost;
  return ((Savings + OuterLoopSavings) * 100) / SafeCost;
}

bool HIRCompleteUnroll::ProfitabilityAnalyzer::isProfitable() const {

  DEBUG(dbgs() << "Cost: " << Cost << "\n");
  DEBUG(dbgs() << "Savings: " << Savings << "\n");
  DEBUG(dbgs() << "OuterLoopSavings: " << OuterLoopSavings << "\n");

  DEBUG(dbgs() << "Savings in percentage: " << getSavingsInPercentage()
               << "\n");

  return (!AccessesStruct && getSavingsInPercentage() > SavingsThreshold) &&
         (NumMemRefs <= UnrolledLoopMemRefThreshold) &&
         (NumDDRefs <= UnrolledLoopDDRefThreshold);
}

void HIRCompleteUnroll::ProfitabilityAnalyzer::visit(const HLLoop *Lp) {
  // Analyze child loop.
  ProfitabilityAnalyzer PA(HCU, Lp, OuterLoop);
  PA.analyze();

  // Add the result of child loop profitability analysis.
  *this += PA;
}

void HIRCompleteUnroll::ProfitabilityAnalyzer::updateBlobs(const RegDDRef *Ref,
                                                           bool Simplified) {
  if (!Ref->isSelfBlob()) {
    return;
  }

  auto &BU = Ref->getBlobUtils();
  auto TempIndex = Ref->getSelfBlobIndex();
  auto TempBlob = BU.getBlob(TempIndex);

  for (auto Idx : VisitedNonLinearBlobs) {
    if (BU.contains(BU.getBlob(Idx), TempBlob)) {
      VisitedNonLinearBlobs.erase(Idx);
    }
  }

  if (Simplified) {
    SimplifiedTempBlobs.insert(TempIndex);
  } else {
    SimplifiedTempBlobs.erase(TempIndex);
  }
}

void HIRCompleteUnroll::ProfitabilityAnalyzer::visit(const HLDDNode *Node) {
  auto HInst = dyn_cast<HLInst>(Node);
  bool IsSelect = (HInst && isa<SelectInst>(HInst->getLLVMInstruction()));

  auto RefIt = HInst ? HInst->rval_op_ddref_begin() : Node->op_ddref_begin();
  auto End = HInst ? HInst->rval_op_ddref_end() : Node->op_ddref_end();

  unsigned NumRvalOp = 0;
  bool CanSimplifyRvals = true;
  const RegDDRef *LvalRef = nullptr;
  const RegDDRef *RvalRef = nullptr;

  for (; RefIt != End; ++RefIt, ++NumRvalOp) {
    RvalRef = *RefIt;
    ++NumDDRefs;

    if (!processRef(RvalRef)) {
      // Only the first two operands of select are relavant for simplification.
      if (!IsSelect || (NumRvalOp < 2)) {
        CanSimplifyRvals = false;
      }
    }
  }

  if (HInst && (LvalRef = HInst->getLvalDDRef())) {
    ++NumDDRefs;
    // Terminal lval refs are only used to invalidate their encountered uses.
    if (LvalRef->isTerminalRef()) {
      updateBlobs(LvalRef, CanSimplifyRvals);
    } else {
      processRef(LvalRef);
    }
  }

  // Add 1 to cost/savings based on whether candidate can be simplified.
  if (CanSimplifyRvals) {
    // Ignore instructions like t = 0.
    if (!HInst || !HInst->isCopyInst() || !LvalRef->isTerminalRef() ||
        !RvalRef->isConstant()) {
      ++Savings;
    }
  } else {
    ++Cost;
  }
}

bool HIRCompleteUnroll::ProfitabilityAnalyzer::processRef(const RegDDRef *Ref) {

  // Skip loops with struct references for now.
  // TODO: clean this up after creating pre and post vector passes.
  if (Ref->accessesStruct()) {
    AccessesStruct = true;
    return false;
  }

  bool CanSimplify = true;
  unsigned RefCost = 1;

  if (Ref->hasGEPInfo()) {

    if (Ref->isMemRef()) {
      ++NumMemRefs;
      ++RefCost;
    }

    processCanonExpr(Ref->getBaseCE(), Ref);

    // Only consider extra cost of GEP refs.
    if (Ref->isStructurallyInvariantAtLevel(Ref->getNodeLevel())) {
      Savings += RefCost;
    } else {
      CanSimplify = false;
      Cost += RefCost;
    }
  }

  for (auto CEIt = Ref->canon_begin(), E = Ref->canon_end(); CEIt != E;
       ++CEIt) {
    if (!processCanonExpr(*CEIt, Ref) && !Ref->hasGEPInfo()) {
      CanSimplify = false;
    }
  }

  // Accumulate savings w.r.t outer loops. Consider this case-
  // DO i1 = 0, 10
  //   DO i2 = 0, 5
  //     A[i2] =
  //   END DO
  // END DO
  //
  // Unrolling of the i1 loopnest will yield redundant loads of A[i2] for each
  // i1 loop iteration. Here we account for these savings.
  const HLLoop *OutermostLoop = OuterLoop->getParentLoop();
  bool InvariantInOuterLevel = false;

  for (const HLLoop *ParentLoop = CurLoop->getParentLoop();
       ParentLoop != OutermostLoop; ParentLoop = ParentLoop->getParentLoop()) {
    if (Ref->isStructurallyInvariantAtLevel(ParentLoop->getNestingLevel())) {
      auto It = HCU.AvgTripCount.find(ParentLoop);
      assert((It != HCU.AvgTripCount.end()) && "Trip count of loop not found!");
      RefCost *= It->second;
      InvariantInOuterLevel = true;
    }
  }

  if (InvariantInOuterLevel) {
    OuterLoopSavings += RefCost;
  }

  return CanSimplify;
}

/// Evaluates profitability of CanonExpr.
/// Example 1-
/// The profitability index of CE: (3 * i1 + 1) is 3. It is computed as
/// follows-
/// +1 for substitution of i2 by constant.
/// +1 for simplification of (3 * i2) to a constant.
/// +1 for simplification of (3 * i2 + 1) to a constant.
///
/// Example 2-
/// The profitability index of CE: (b1 * i1 + 1) where b1 is a linear temp is 3.
/// It is computed as follows-
/// +1 for substitution of i1 by constant.
/// +1 for linear blob b1.
/// +1 for b1 * i1 possibly resulting in opportunity for CSE.
///
/// Example 3-
/// The profitability index of CE: (b1 * i1 + 1) where b1 is a non-linear temp
/// is -1. It is computed as follows-
/// +1 for substitution of i1 by constant.
/// -1 for non-linear blob b1.
/// -1 for b1 * i1 resulting in code size increase because of non-linearity of
///    blob.
///
/// Example 4-
/// The profitability index of CE: (i1 + 2 * i2 + b1) where i1 loopnest is
/// being unrolled and b1 is a non-linear temp is 3. It is computed as follows-
/// +1 for substitution of i1 by constant.
/// +1 for substitution of i2 by constant.
/// +1 for simplification of (2 * i2) to a constant.
/// +1 for simplification of (i1 + 2 * i2) to a constant.
/// -1 for non-linear blob.
///
/// Example 5-
/// The profitability index of CE: (i1 + 2 * i2 + b1) where i2 loop is being
/// unrolled and b1 is a non-linear temp is 1. It is computed as follows-
/// +1 for substitution of i2 by constant.
/// +1 for simplification of (2 * i2) to a constant.
/// -1 for non-linear blob.
///
bool HIRCompleteUnroll::ProfitabilityAnalyzer::processCanonExpr(
    const CanonExpr *CE, const RegDDRef *ParentRef) {

  unsigned NumSimplifiedTerms = 0;
  unsigned NumNonLinearTerms = 0;
  bool IsLinear = CE->isLinearAtLevel();

  bool CanSimplifyIVs =
      processIVs(CE, ParentRef, NumSimplifiedTerms, NumNonLinearTerms);

  bool CanSimplifyBlobs =
      processBlobs(CE, ParentRef, NumSimplifiedTerms, NumNonLinearTerms);

  bool NumeratorBecomesConstant = CanSimplifyIVs && CanSimplifyBlobs;

  // Add 1 to savings each, for number of simplified IV/Blob additions.
  if (NumSimplifiedTerms) {
    Savings += (NumSimplifiedTerms - 1);
  }

  // Add 1 to cost each, for number of non-linear IV/Blob additions.
  if (NumNonLinearTerms) {
    Cost += (NumNonLinearTerms - 1);
  }

  // Add 1 to cost/savings for the constant based on linearity and IV
  // simplifications.
  if (CE->getConstant()) {
    if (NumSimplifiedTerms) {
      ++Savings;
    } else if (!IsLinear) {
      ++Cost;
    }
  }

  // Add 1 to cost/savings for non-unit denominator based on linearity.
  if ((CE->getDenominator() != 1)) {
    if (NumeratorBecomesConstant) {
      ++Savings;
    } else if (!IsLinear) {
      ++Cost;
    }
  }

  // Add 1 to cost/savings based on whether there is a hidden cast.
  if (CE->getSrcType() != CE->getDestType()) {
    if (NumeratorBecomesConstant) {
      ++Savings;
    } else if (!IsLinear) {
      ++Cost;
    }
  }

  return NumeratorBecomesConstant;
}

bool HIRCompleteUnroll::ProfitabilityAnalyzer::processIVs(
    const CanonExpr *CE, const RegDDRef *ParentRef,
    unsigned &NumSimplifiedTerms, unsigned &NumNonLinearTerms) {

  bool CanSimplifyIVs = true;
  unsigned NodeLevel = CurLoop->getNestingLevel();
  unsigned OuterLevel = OuterLoop->getNestingLevel();
  bool IsLinear = CE->isLinearAtLevel();

  for (unsigned Level = 1; Level <= NodeLevel; ++Level) {
    unsigned BlobIndex;
    int64_t Coeff;

    CE->getIVCoeff(Level, &BlobIndex, &Coeff);

    if (!Coeff) {
      continue;
    }

    if (Level >= OuterLevel) {
      // This IV belongs to one of the unroll candidates, add 1 for substitution
      // of IV by constant.
      ++Savings;

      if (Coeff != 1) {
        // Add one more for simplfication of (Coeff * IV).
        ++Savings;
      }

      // Keep track of inductive terms simplified to constant.
      if (BlobIndex == InvalidBlobIndex) {
        ++NumSimplifiedTerms;
      }
    } else {
      CanSimplifyIVs = false;
      // IV multiplication gives us opportunity for CSE.
      if (Coeff != 1) {
        ++Savings;
      }
    }

    if (BlobIndex != InvalidBlobIndex) {
      auto BInfo = getBlobInfo(BlobIndex, ParentRef, IsLinear);

      if (BInfo.Simplified && (Level >= OuterLevel)) {
        ++NumSimplifiedTerms;
      } else {
        CanSimplifyIVs = false;
      }

      // Coefficient of blob is passed as zero but any value other than 1 will
      // do. This is just to indicate whether we are multiplying the blob with
      // anything. In this case it is being multiplied by the IV.
      addBlobCost(BInfo, 0, NumNonLinearTerms);
    }
  }

  return CanSimplifyIVs;
}

bool HIRCompleteUnroll::ProfitabilityAnalyzer::processBlobs(
    const CanonExpr *CE, const RegDDRef *ParentRef,
    unsigned &NumSimplifiedTerms, unsigned &NumNonLinearTerms) {
  bool CanSimplifyBlobs = true;
  bool IsLinear = CE->isLinearAtLevel();

  for (auto Blob = CE->blob_begin(), E = CE->blob_end(); Blob != E; ++Blob) {
    auto BInfo = getBlobInfo(Blob->Index, ParentRef, IsLinear);

    if (BInfo.Simplified) {
      ++Savings;
      ++NumSimplifiedTerms;
    } else {
      CanSimplifyBlobs = false;
    }

    addBlobCost(BInfo, Blob->Coeff, NumNonLinearTerms);
  }

  return CanSimplifyBlobs;
}

HIRCompleteUnroll::ProfitabilityAnalyzer::BlobInfo
HIRCompleteUnroll::ProfitabilityAnalyzer::getBlobInfo(unsigned Index,
                                                      const RegDDRef *ParentRef,
                                                      bool CEIsLinear) {
  BlobInfo BInfo;

  BInfo.Simplified = SimplifiedTempBlobs.count(Index);

  if (CEIsLinear) {
    return BInfo;
  }

  SmallVector<unsigned, 8> Indices;

  auto &BU = ParentRef->getBlobUtils();

  BU.collectTempBlobs(Index, Indices);

  // Add non-linear blobs as visited so we only penalize them once. The blobs
  // are added at the top level and the leaf (temp) level. This is an
  // approximation to save compile time. To generate more accurate results we
  // would have to compare sub-expression trees which would be very expensive.
  bool Invariant = true;
  for (auto Idx : Indices) {
    unsigned DefLevel;
    bool Found = ParentRef->findTempBlobLevel(Idx, &DefLevel);
    (void)Found;
    assert(Found && "Temp blob not found in Ref!");

    if ((DefLevel == NonLinearLevel) && !SimplifiedTempBlobs.count(Idx)) {
      Invariant = false;
      VisitedNonLinearBlobs.insert(Idx);
    }
  }

  if (!Invariant) {
    BInfo.Invariant = false;
    BInfo.Visited = VisitedNonLinearBlobs.count(Index);
    VisitedNonLinearBlobs.insert(Index);
    BInfo.NumOperations = BU.getNumOperations(Index);
  }

  return BInfo;
}

void HIRCompleteUnroll::ProfitabilityAnalyzer::addBlobCost(
    const BlobInfo &BInfo, int64_t Coeff, unsigned &NumNonLinearTerms) {

  if (Coeff != 1) {
    if (BInfo.Simplified) {
      ++Savings;
    } else if (BInfo.Visited) {
      ++NumNonLinearTerms;
      ++Cost;
    } else if (!BInfo.Invariant) {
      ++NumNonLinearTerms;
      Cost += (BInfo.NumOperations + 1);
    }
  } else if (!BInfo.Invariant && !BInfo.Simplified && !BInfo.Visited) {
    ++NumNonLinearTerms;
    Cost += BInfo.NumOperations;
  }
}

///// ProfitabilityAnalyzer Visitor End

char HIRCompleteUnroll::ID = 0;
INITIALIZE_PASS_BEGIN(HIRCompleteUnroll, "hir-complete-unroll",
                      "HIR Complete Unroll", false, false)
INITIALIZE_PASS_DEPENDENCY(HIRFramework)
INITIALIZE_PASS_DEPENDENCY(HIRLoopStatistics)
INITIALIZE_PASS_END(HIRCompleteUnroll, "hir-complete-unroll",
                    "HIR Complete Unroll", false, false)

FunctionPass *llvm::createHIRCompleteUnrollPass() {
  return new HIRCompleteUnroll();
}

bool HIRCompleteUnroll::runOnFunction(Function &F) {
  // Skip if DisableHIRCompleteUnroll is enabled
  if (DisableHIRCompleteUnroll || skipFunction(F)) {
    DEBUG(dbgs() << "HIR LOOP Complete Unroll Transformation Disabled \n");
    return false;
  }

  DEBUG(dbgs() << "Complete unrolling for Function : " << F.getName() << "\n");
  DEBUG(dbgs() << "Trip Count Threshold : " << CompleteUnrollTripThreshold
               << "\n");

  // Do an early exit if Trip Threshold is less than 1
  // TODO: Check if we want give some feedback to user
  if (CompleteUnrollTripThreshold == 0) {
    return false;
  }

  auto HIRF = &getAnalysis<HIRFramework>();
  HLS = &getAnalysis<HIRLoopStatistics>();

  // Storage for Outermost Loops
  SmallVector<HLLoop *, 64> OuterLoops;
  // Gather the outermost loops
  HIRF->getHLNodeUtils().gatherOutermostLoops(OuterLoops);

  // Process Loop Complete Unrolling
  processCompleteUnroll(OuterLoops);

  return false;
}

/// processCompleteUnroll - Main routine to perform unrolling.
/// First, performs cost analysis and then do the transformation.
void HIRCompleteUnroll::processCompleteUnroll(
    SmallVectorImpl<HLLoop *> &OuterLoops) {

  // Walk over the outermost loops across the regions.
  for (auto &Lp : OuterLoops) {
    if (performTripCountAnalysis(Lp) >= 0) {
      CandidateLoops.push_back(Lp);
    }
  }

  refineCandidates();

  transformLoops();
}

void HIRCompleteUnroll::refineCandidates() {

  for (unsigned Index = 0; Index != CandidateLoops.size();) {
    HLLoop *OuterCandidateLoop = CandidateLoops[Index];

    SmallVector<HLLoop *, 8> ChildLoops;
    OuterCandidateLoop->getHLNodeUtils().gatherAllLoops(OuterCandidateLoop,
                                                        ChildLoops);
    // Check if the dependency of loops is satisfied. If not, then we add
    // the children loop as candidate and remove the current outermost loops.
    // TODO: store dependency level of loops during trip count analysis.
    if (satisfiesDependency(OuterCandidateLoop->getNestingLevel(),
                            ChildLoops) &&
        isProfitable(OuterCandidateLoop)) {
      Index++;
      continue;
    }

    if (!OuterCandidateLoop->isInnermost()) {
      OuterCandidateLoop->getHLNodeUtils().gatherLoopsWithLevel(
          OuterCandidateLoop, CandidateLoops,
          OuterCandidateLoop->getNestingLevel() + 1);
    }

    CandidateLoops.erase(CandidateLoops.begin() + Index);
  }
}

bool HIRCompleteUnroll::satisfiesDependency(
    unsigned OuterCandidateLevel, SmallVectorImpl<HLLoop *> &ChildLoops) const {

  for (auto &CLoop : ChildLoops) {
    CanonExpr *UBCE = CLoop->getUpperCanonExpr();
    for (unsigned Level = 1; Level < OuterCandidateLevel; ++Level) {
      if (UBCE->hasIV(Level)) {
        return false;
      }
    }
  }
  return true;
}

bool HIRCompleteUnroll::isApplicable(const HLLoop *Loop) const {

  // Throttle multi-exit/unknown loops.
  if (!Loop->isDo()) {
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

  // Temporarily moving the logic from isProfitable().
  // TODO: remove the checks later.
  const LoopStatistics &LS = HLS->getSelfLoopStatistics(Loop);

  if (LS.hasSwitches() || LS.hasCalls()) {
    return false;
  }

  return true;
}

int64_t HIRCompleteUnroll::computeAvgTripCount(const HLLoop *Loop) {

  auto UpperCE = Loop->getUpperCanonExpr();

  if (UpperCE->hasBlob() || UpperCE->hasBlobIVCoeffs() ||
      (UpperCE->getDenominator() != 1)) {
    return -1;
  }

  int64_t UpperVal = 0;

  if (UpperCE->isIntConstant(&UpperVal)) {
    int64_t TripCnt = UpperVal + 1;

    if ((TripCnt < 0) || (TripCnt > CompleteUnrollTripThreshold)) {
      return -1;
    }

    AvgTripCount.insert(std::make_pair(Loop, TripCnt));

    return TripCnt;
  }

  // If triangular loop is disabled, we simply return high trip count,
  // to avoid unrolling triangular loops.
  if (DisableHIRTriCompleteUnroll) {
    return -1;
  }

  unsigned LoopLevel = Loop->getNestingLevel();
  auto ParLoop = Loop->getParentLoop();
  bool CanUnrollParents = true;

  // This is a triangular loop unrolling candidate. Check whether all parent
  // loops on which this loop's upper canon is dependent can be unrolled as
  // well. CanUnrollParents is set to false by the first parent loop which
  // cannot be unrolled. Any occurence of parent loop IVs from then on makes
  // the loop ineligible for unrolling. Here's an example-
  //
  // DO i1 = 1, 5
  //   DO i2 = 1, %n
  //     DO i3 = 1, i1
  //
  // CanUnrollParents is set to false by i2 loop. Therefore, presence of i1 in
  // i3 loop's upper canon makes it ineligible for complete unrolling.
  for (unsigned Level = LoopLevel - 1; Level > 0; --Level) {

    if (!AvgTripCount.count(ParLoop)) {
      CanUnrollParents = false;
    }

    if (UpperCE->getIVConstCoeff(Level) && !CanUnrollParents) {
      return -1;
    }

    ParLoop = ParLoop->getParentLoop();
  }

  int64_t MinUpper = 0, MaxUpper = 0, AvgTripCnt = 0;

  // If we reached here, we should be able to compute the min/max trip count of
  // this loop.
  bool HasMin =
      Loop->getHLNodeUtils().getExactMinValue(UpperCE, Loop, MinUpper);
  (void)HasMin;
  assert(HasMin && "Could not compute min value of upper!");

  // MinUpper can evaluate to a negative value. For purposes of calculating
  // average trip count for profitability analysis, we take the absolute value.
  MinUpper = (MinUpper > 0) ? MinUpper : -MinUpper;

  bool HasMax =
      Loop->getHLNodeUtils().getExactMaxValue(UpperCE, Loop, MaxUpper);
  (void)HasMax;
  assert(HasMax && "Could not compute max value of upper!");

  // Loop never executes.
  if (MaxUpper < 0) {
    AvgTripCnt = 0;
  } else {
    AvgTripCnt = ((MinUpper + MaxUpper) / 2) + 1;
  }

  if (AvgTripCnt > CompleteUnrollTripThreshold) {
    return -1;
  }

  AvgTripCount.insert(std::make_pair(Loop, AvgTripCnt));

  return AvgTripCnt;
}

int64_t HIRCompleteUnroll::performTripCountAnalysis(HLLoop *Loop) {
  SmallVector<HLLoop *, 8> CandidateChildLoops;

  int64_t AvgTripCnt = -1;
  int64_t MaxChildTripCnt = 1;

  bool IsLoopCandidate = isApplicable(Loop);

  if (IsLoopCandidate) {
    AvgTripCnt = computeAvgTripCount(Loop);
    IsLoopCandidate = (AvgTripCnt < 0) ? false : true;
  }

  if (!Loop->isInnermost()) {
    SmallVector<HLLoop *, 8> ChildLoops;
    Loop->getHLNodeUtils().gatherLoopsWithLevel(Loop, ChildLoops,
                                                Loop->getNestingLevel() + 1);

    for (auto &ChildLp : ChildLoops) {
      int64_t ChildTripCnt = performTripCountAnalysis(ChildLp);

      if (ChildTripCnt >= 0) {
        CandidateChildLoops.push_back(ChildLp);

        if (ChildTripCnt > MaxChildTripCnt) {
          MaxChildTripCnt = ChildTripCnt;
        }

      } else {
        IsLoopCandidate = false;
      }
    }
  }

  if (IsLoopCandidate) {
    // Compute trip count of loopnest.
    AvgTripCnt *= MaxChildTripCnt;
    IsLoopCandidate = (AvgTripCnt <= CompleteUnrollTripThreshold);
  }

  // If current loop is not a candidate, store the children loops
  // for transformation.
  if (!IsLoopCandidate) {
    CandidateLoops.append(CandidateChildLoops.begin(),
                          CandidateChildLoops.end());
  }

  return IsLoopCandidate ? AvgTripCnt : -1;
}

bool HIRCompleteUnroll::isProfitable(const HLLoop *Loop) const {

  ProfitabilityAnalyzer PA(*this, Loop);

  PA.analyze();

  return PA.isProfitable();
}

// Transform (Complete Unroll) each loop inside the CandidateLoops vector
void HIRCompleteUnroll::transformLoops() {
  SmallVector<int64_t, MaxLoopNestLevel> TripValues;

  LoopnestsCompletelyUnrolled += CandidateLoops.size();

  // Transform the loop nest from outer to inner.
  for (auto &Loop : CandidateLoops) {
    // Generate code for the parent region and invalidate parent
    Loop->getParentRegion()->setGenCode();
    HIRInvalidationUtils::invalidateParentLoopBodyOrRegion(Loop);

    HLLoop *ParentLoop = Loop->getParentLoop();

    transformLoop(Loop, Loop, TripValues);

    if (ParentLoop) {
      HIRTransformUtils::eliminateRedundantPredicates(ParentLoop->child_begin(),
                                                      ParentLoop->child_end());
    }
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
    UBCE->simplify(true);
    LoopLevel++;
  }

  bool isIntConst = UBCE->isIntConstant(&UBVal);
  (void)isIntConst;
  assert(isIntConst && " Upper Bound is not a constant after IV substitution.");
  return UBVal;
}

// Complete Unroll the given Loop, using provided LD as helper data
void HIRCompleteUnroll::transformLoop(HLLoop *Loop, HLLoop *OuterLoop,
                                      SmallVectorImpl<int64_t> &TripValues) {

  // Guard against the scanning phase setting it appropriately.
  assert(Loop && " Loop is null.");

  // Container for cloning body.
  HLContainerTy LoopBody;
  HLNodeUtils &HNU = Loop->getHLNodeUtils();

  CanonExprVisitor CEVisit(OuterLoop, TripValues);

  int64_t LB = Loop->getLowerCanonExpr()->getConstant();
  int64_t UB = computeUB(Loop, OuterLoop, TripValues);
  int64_t Step = Loop->getStrideCanonExpr()->getConstant();

  // At this point loop preheader has been visited already but postexit is not,
  // so we need to handle postexit explicitly.
  if (UB < 0) {
    Loop->removePostexit();
    HNU.remove(Loop);
    return;
  }

  if (Loop != OuterLoop) {
    HNU.visitRange(CEVisit, Loop->post_begin(), Loop->post_end());
  }

  // Ztt is not needed since it has ateast one trip.
  Loop->removeZtt();
  Loop->extractPreheaderAndPostexit();

  // Iterate over Loop Child for unrolling with trip value incremented
  // each time. Thus, loop body will be expanded by no. of stmts x TripCount.
  for (int64_t TripVal = LB; TripVal <= UB; TripVal += Step) {
    // Clone iteration
    HNU.cloneSequence(&LoopBody, Loop->getFirstChild(), Loop->getLastChild());

    // Store references as LoopBody will be empty after insertion.
    HLNode *CurFirstChild = &(LoopBody.front());
    HLNode *CurLastChild = &(LoopBody.back());

    HNU.insertBefore(Loop, &LoopBody);

    // Trip Values vector is used to store the current IV
    // trip value for substitution inside the canon expr.
    TripValues.push_back(TripVal);

    // Update the CanonExpr
    CanonExprVisitor CEVisit(OuterLoop, TripValues);
    HNU.visitRange<true, false>(CEVisit, CurFirstChild, CurLastChild);

    TripValues.pop_back();
  }

  HNU.remove(Loop);
}

void HIRCompleteUnroll::releaseMemory() {
  CandidateLoops.clear();
  AvgTripCount.clear();
}
