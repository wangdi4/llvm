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
#include "llvm/Transforms/Intel_LoopTransforms/Utils/BlobUtils.h"
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
static cl::opt<unsigned> CompleteUnrollTripThreshold(
    "hir-complete-unroll-trip-threshold", cl::init(50), cl::Hidden,
    cl::desc("Don't unroll if total trip count is bigger than this "
             "threshold."));

static cl::opt<unsigned> UnrolledLoopMemRefThreshold(
    "hir-complete-unroll-memref-threshold", cl::init(120), cl::Hidden,
    cl::desc("Maximum number of memory refs allowed in completely unrolled "
             "loopnest"));

static cl::opt<unsigned>
    UnrolledLoopDDRefThreshold("hir-complete-unroll-ddref-threshold",
                               cl::init(350), cl::Hidden,
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

  /// \brief Computes and returns average trip count for profitability analysis.
  /// Returns -1 if it cannot be computed.
  int64_t computeAvgTripCount(const HLLoop *Loop);

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

  /// \brief Returns true if loop is profitable for complete unrolling.
  bool isProfitable(const HLLoop *Loop, int64_t TotalTripCnt) const;

  /// \brief Performs the complete unrolling transformation.
  static void transformLoop(HLLoop *Loop, HLLoop *OuterLoop,
                            SmallVectorImpl<int64_t> &TripValues);

  /// \brief Main routine to drive the complete unrolling transformation.
  void processCompleteUnroll(SmallVectorImpl<HLLoop *> &OuterLoops);

  /// \brief Processes a HLLoop to check if it is candidate for transformation.
  /// Returns the avg trip count of the loopnest. Non-negative value indicates
  /// that loopnest is a candidate.
  int64_t processLoop(HLLoop *Loop);

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

  int64_t ProfitabilityIndex;

  unsigned NumMemRefs;
  unsigned NumDDRefs;

  // Private constructor used for chilren loops.
  ProfitabilityAnalyzer(const HIRCompleteUnroll &HCU, const HLLoop *CurLp,
                        const HLLoop *OuterLp)
      : HCU(HCU), CurLoop(CurLp), OuterLoop(OuterLp), ProfitabilityIndex(0),
        NumMemRefs(0), NumDDRefs(0) {}

  /// Processes RegDDRef for profitability. Returns true if Ref can be
  /// simplified to a constant.
  bool processRef(const RegDDRef *Ref);

  /// Processes CanonExpr for profitability. Returns true if CE can be
  /// simplified to a constant.
  bool processCanonExpr(const CanonExpr *CE, const RegDDRef *ParentRef);

  /// Processes blob for profitability. Returns true if blob is profitable.
  bool processBlob(unsigned Index, const RegDDRef *ParentRef, bool CEIsLinear);

  /// Scales the profitability by the given multiplier.
  void scale(unsigned Multiplier) {
    ProfitabilityIndex *= Multiplier;
    NumMemRefs *= Multiplier;
    NumDDRefs *= Multiplier;
  }

  // Adds profitability analysis results from PA to this.
  ProfitabilityAnalyzer &operator+=(const ProfitabilityAnalyzer &PA) {
    ProfitabilityIndex += PA.ProfitabilityIndex;
    NumMemRefs += PA.NumMemRefs;
    NumDDRefs += PA.NumDDRefs;

    return *this;
  }

public:
  ProfitabilityAnalyzer(const HIRCompleteUnroll &HCU, const HLLoop *CurLp)
      : HCU(HCU), CurLoop(CurLp), OuterLoop(CurLp), ProfitabilityIndex(0),
        NumMemRefs(0), NumDDRefs(0) {}

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

///// ProfitabilityAnalyzer Visitor Start

void HIRCompleteUnroll::ProfitabilityAnalyzer::analyze() {

  HLNodeUtils::visitRange<true, false>(*this, CurLoop->child_begin(),
                                       CurLoop->child_end());

  // Scale results by loop's average trip count.
  auto It = HCU.AvgTripCount.find(CurLoop);
  assert((It != HCU.AvgTripCount.end()) && "Trip count of loop not found!");

  // Check if the loop is small enough to assign some extra profitability to it
  // (for eliminating loop control) and give it higher chance of unrolling.
  if (isSmallLoop()) {
    // Capping extra profitability at an arbitrary constant.
    ProfitabilityIndex += std::min(4u, It->second);
  }

  scale(It->second);

  // Add ztt's profitability. 
  if (CurLoop->hasZtt()) {
    for (auto RefIt = CurLoop->ztt_ddref_begin(), E = CurLoop->ztt_ddref_end();
         RefIt != E; ++RefIt) {
      processRef(*RefIt);
    }
    // Increment index by number of predicates eliminated.
    ProfitabilityIndex += CurLoop->getNumZttPredicates();
  }
}

bool HIRCompleteUnroll::ProfitabilityAnalyzer::isSmallLoop() const {
  return (NumMemRefs <= SmallLoopMemRefThreshold) &&
         (NumDDRefs <= SmallLoopDDRefThreshold);
}

bool HIRCompleteUnroll::ProfitabilityAnalyzer::isProfitable() const {
  return (ProfitabilityIndex >= 0) &&
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

void HIRCompleteUnroll::ProfitabilityAnalyzer::visit(const HLDDNode *Node) {

  auto HInst = dyn_cast<HLInst>(Node);
  bool IsBinaryOp = false;

  if (HInst) {
    IsBinaryOp = isa<BinaryOperator>(HInst->getLLVMInstruction());

    auto Ref = HInst->getLvalDDRef();

    if (Ref) {
      ++NumDDRefs;

      // Ignore terminal lval refs.
      if (!Ref->isTerminalRef()) {
        processRef(Ref);
      }
    }
  }

  bool CanSimplifyRvals = true;

  auto RefIt = HInst ? HInst->rval_op_ddref_begin() : Node->op_ddref_begin();
  auto End = HInst ? HInst->rval_op_ddref_end() : Node->op_ddref_end();

  for (; RefIt != End; ++RefIt) {
    auto Ref = *RefIt;
    ++NumDDRefs;

    if (!processRef(Ref)) {
      CanSimplifyRvals = false;
    }
  }

  // Add or subtract 1 according to whether binary operation can be simplified.
  if (IsBinaryOp) {
    CanSimplifyRvals ? ++ProfitabilityIndex : --ProfitabilityIndex;
  }
}

bool HIRCompleteUnroll::ProfitabilityAnalyzer::processRef(const RegDDRef *Ref) {

  bool CanSimplify = true;

  if (Ref->hasGEPInfo()) {
    CanSimplify = false;
    processCanonExpr(Ref->getBaseCE(), Ref);

    if (Ref->isMemRef()) {
      ++NumMemRefs;
    }
  }

  for (auto CEIt = Ref->canon_begin(), E = Ref->canon_end(); CEIt != E;
       ++CEIt) {
    if (!processCanonExpr(*CEIt, Ref)) {
      CanSimplify = false;
    }
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

  bool NumeratorBecomesConstant = true;
  unsigned NumSimplifiedIVs = 0;

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
      if (Coeff != 1) {
        // Add one more for simplfication of (Coeff * IV).
        ProfitabilityIndex += 2;
      } else {
        ++ProfitabilityIndex;
      }

      // Keep track of inductive terms simplified to constant.
      if (BlobIndex == InvalidBlobIndex) {
        ++NumSimplifiedIVs;
      }
    } else {
      NumeratorBecomesConstant = false;
      // IV multiplication gives us opportunity for CSE.
      if (Coeff != 1) {
        ++ProfitabilityIndex;
      }
    }

    if (BlobIndex != InvalidBlobIndex) {
      NumeratorBecomesConstant = false;
      // Add or subtract 1 for (Blob * IV) based on whether this blob is
      // profitable.
      processBlob(BlobIndex, ParentRef, IsLinear) ? ++ProfitabilityIndex
                                                  : --ProfitabilityIndex;
    }
  }

  // Add 1 each for number of simplified IV additions.
  if (NumSimplifiedIVs) {
    ProfitabilityIndex += (NumSimplifiedIVs - 1);
  }

  for (auto Blob = CE->blob_begin(), E = CE->blob_end(); Blob != E; ++Blob) {
    NumeratorBecomesConstant = false;

    bool IsProfitable = processBlob(Blob->Index, ParentRef, IsLinear);

    if (Blob->Coeff != 1) {
      // Add or subtract 1 for (Coeff * Blob) based on whether this blob is
      // profitable.
      IsProfitable ? ++ProfitabilityIndex : --ProfitabilityIndex;
    }
  }

  // Add or subtract 1 for the constant based on linearity and IV
  // simplifications.
  if (CE->getConstant()) {
    (IsLinear || NumSimplifiedIVs) ? ++ProfitabilityIndex
                                   : --ProfitabilityIndex;
  }

  // Add or subtract 1 for non-unit denominator based on linearity.
  if ((CE->getDenominator() != 1)) {
    IsLinear ? ++ProfitabilityIndex : --ProfitabilityIndex;
  }

  return NumeratorBecomesConstant;
}

bool HIRCompleteUnroll::ProfitabilityAnalyzer::processBlob(
    unsigned Index, const RegDDRef *ParentRef, bool CEIsLinear) {
  // Profitability of blob is evaluated using 'linearity index' defined as:
  // (number of contained linear temp blobs) - (number of contained non-linear
  // temp blobs).

  SmallVector<unsigned, 8> Indices;
  int LinearityIndex = 0;

  BlobUtils::collectTempBlobs(Index, Indices);

  if (CEIsLinear) {
    LinearityIndex = Indices.size();

  } else {

    for (auto Idx : Indices) {
      unsigned DefLevel;

      bool Found = ParentRef->findTempBlobLevel(Idx, &DefLevel);
      (void)Found;
      assert(Found && "Temp blob not found in Ref!");

      (DefLevel == NonLinearLevel) ? --LinearityIndex : ++LinearityIndex;
    }
  }

  ProfitabilityIndex += LinearityIndex;

  return (LinearityIndex > 0);
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

  HLS = &getAnalysis<HIRLoopStatistics>();

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
  for (auto &Lp : OuterLoops) {
    if (processLoop(Lp) >= 0) {
      CandidateLoops.push_back(Lp);
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

bool HIRCompleteUnroll::isApplicable(const HLLoop *Loop) const {

  // Throttle multi-exit/unknown loops.
  if (!Loop->isDo()) {
    return false;
  }

  // Ignore empty loops.
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
  bool HasMin = HLNodeUtils::getExactMinValue(UpperCE, Loop, MinUpper);
  (void)HasMin;
  assert(HasMin && "Could not compute min value of upper!");

  // MinUpper can evaluate to a negative value. For purposes of calculating
  // average trip count for profitability analysis, we take the absolute value.
  MinUpper = (MinUpper > 0) ? MinUpper : -MinUpper;

  bool HasMax = HLNodeUtils::getExactMaxValue(UpperCE, Loop, MaxUpper);
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

int64_t HIRCompleteUnroll::processLoop(HLLoop *Loop) {

  SmallVector<HLLoop *, 8> CandidateChildLoops;
  int64_t AvgTripCnt = -1;
  int64_t MaxChildTripCnt = 1;

  bool IsLoopCandidate = isApplicable(Loop);

  // Compute average trip count of current loop as it is used in profitability
  // analysis.
  if (IsLoopCandidate) {
    AvgTripCnt = computeAvgTripCount(Loop);
    IsLoopCandidate = (AvgTripCnt < 0) ? false : true;
  }

  // Visit children, only if it not the innermost, else
  // perform profitability analysis.
  if (!Loop->isInnermost()) {
    SmallVector<HLLoop *, 8> ChildLoops;
    // 1. Gather Loops starting from the outer-most level
    HLNodeUtils::gatherLoopsWithLevel(Loop, ChildLoops,
                                      Loop->getNestingLevel() + 1);

    // 2.Process each Loop for Complete Unrolling
    bool HasValidChildren = true;
    // Recurse through the children.
    for (auto &ChildLp : ChildLoops) {

      int64_t ChildTripCnt = processLoop(ChildLp);

      if (ChildTripCnt >= 0) {
        CandidateChildLoops.push_back(ChildLp);

        if (ChildTripCnt > MaxChildTripCnt) {
          MaxChildTripCnt = ChildTripCnt;
        }

      } else {
        HasValidChildren = false;
      }
    }

    IsLoopCandidate = IsLoopCandidate && HasValidChildren;
  }

  if (IsLoopCandidate) {
    // Compute trip count of loopnest.
    AvgTripCnt *= MaxChildTripCnt;
    IsLoopCandidate = isProfitable(Loop, AvgTripCnt);
  }

  // If current loop is not a candidate, store the children loops
  // for transformation.
  if (!IsLoopCandidate) {
    CandidateLoops.append(CandidateChildLoops.begin(),
                          CandidateChildLoops.end());
  }

  return IsLoopCandidate ? AvgTripCnt : -1;
}

bool HIRCompleteUnroll::isProfitable(const HLLoop *Loop,
                                     int64_t TotalTripCnt) const {

  if (TotalTripCnt > CompleteUnrollTripThreshold) {
    DEBUG(dbgs() << "TotalTripCnt:" << TotalTripCnt << "\n");
    return false;
  }

  const LoopStatistics &LS = HLS->getSelfLoopStatistics(Loop);

  if (LS.hasSwitches() || LS.hasCalls()) {
    return false;
  }

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
void HIRCompleteUnroll::transformLoop(HLLoop *Loop, HLLoop *OuterLoop,
                                      SmallVectorImpl<int64_t> &TripValues) {

  // Guard against the scanning phase setting it appropriately.
  assert(Loop && " Loop is null.");

  // Container for cloning body.
  HLContainerTy LoopBody;

  CanonExprVisitor CEVisit(OuterLoop, TripValues);

  int64_t LB = Loop->getLowerCanonExpr()->getConstant();
  int64_t UB = computeUB(Loop, OuterLoop, TripValues);
  int64_t Step = Loop->getStrideCanonExpr()->getConstant();

  // At this point loop preheader has been visited already but postexit is not,
  // so we need to handle postexit explicitly.
  if (UB < 0) {
    Loop->removePostexit();
    HLNodeUtils::remove(Loop);
    return;
  }

  if (Loop != OuterLoop) {
    HLNodeUtils::visitRange(CEVisit, Loop->post_begin(), Loop->post_end());
  }

  // Ztt is not needed since it has ateast one trip.
  Loop->removeZtt();
  Loop->extractPreheaderAndPostexit();

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

  HLNodeUtils::remove(Loop);
}

void HIRCompleteUnroll::releaseMemory() { CandidateLoops.clear(); }
