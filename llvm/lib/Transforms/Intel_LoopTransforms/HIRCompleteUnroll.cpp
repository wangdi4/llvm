//===- HIRCompleteUnroll.cpp - Implements CompleteUnroll class ------------===//
//
// Copyright (C) 2015-2017 Intel Corporation. All rights reserved.
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

#include "HIRCompleteUnroll.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRLoopStatistics.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Framework/HIRFramework.h"

#include "llvm/ADT/SmallSet.h"
#include "llvm/ADT/Statistic.h"

#include "llvm/IR/Function.h"
#include "llvm/IR/LLVMContext.h"

#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"

#include "llvm/Analysis/Intel_LoopAnalysis/Utils/BlobUtils.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/CanonExprUtils.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/DDRefUtils.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HIRInvalidationUtils.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HLNodeUtils.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/HIRTransformUtils.h"

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

static cl::opt<unsigned> CommandLineOptLevel(
    "hir-complete-unroll-opt-level", cl::init(2), cl::Hidden,
    cl::desc(
        "Opt level for complete unroll (2 or 3). This affects unroll limits."));

const unsigned O2LoopTripThreshold = 64;
const unsigned O3LoopTripThreshold = 64;

// The trip count threshold is intentionally set to a high value as profitablity
// should be driven by the combination of trip count and loop resource.
static cl::opt<unsigned> LoopTripThreshold(
    "hir-complete-unroll-loop-trip-threshold", cl::init(0), cl::Hidden,
    cl::desc("Don't unroll if trip count of any loop is bigger than this "
             "threshold. 0 means default threshold."));

const unsigned O2LoopnestTripThreshold = 100;
const unsigned O3LoopnestTripThreshold = 100;

static cl::opt<unsigned> LoopnestTripThreshold(
    "hir-complete-unroll-loopnest-trip-threshold", cl::init(0), cl::Hidden,
    cl::desc("Don't unroll if total trip count of the loopnest is bigger than "
             "this threshold. 0 means default threshold."));

static cl::opt<unsigned> PreVectorSavingsThreshold(
    "hir-complete-unroll-pre-vector-savings-threshold", cl::init(80),
    cl::Hidden,
    cl::desc(
        "Least amount of savings (in percentage) for complete unrolling "
        "of a loopnest to be deemed profitable before vectorizer kicks in."));

static cl::opt<unsigned> PostVectorSavingsThreshold(
    "hir-complete-unroll-post-vector-savings-threshold", cl::init(40),
    cl::Hidden,
    cl::desc(
        "Least amount of savings (in percentage) for complete unrolling "
        "of a loopnest to be deemed profitable after vectorizer kicks in."));

const unsigned O2UnrolledLoopMemRefThreshold = 160;
const unsigned O3UnrolledLoopMemRefThreshold = 160;

static cl::opt<unsigned> UnrolledLoopMemRefThreshold(
    "hir-complete-unroll-memref-threshold", cl::init(0), cl::Hidden,
    cl::desc("Maximum number of memory refs allowed in completely unrolled "
             "loopnest. 0 means default threshold."));

const unsigned O2UnrolledLoopDDRefThreshold = 1000;
const unsigned O3UnrolledLoopDDRefThreshold = 1000;

static cl::opt<unsigned> UnrolledLoopDDRefThreshold(
    "hir-complete-unroll-ddref-threshold", cl::init(0), cl::Hidden,
    cl::desc("Maximum number of DDRefs allowed in "
             "completely unrolled loopnest. 0 means default threshold."));

static cl::opt<unsigned> SmallLoopMemRefThreshold(
    "hir-complete-unroll-small-memref-threshold", cl::init(16), cl::Hidden,
    cl::desc("Threshold for memory refs in small loops (higher probability of "
             "unrolling)"));

static cl::opt<unsigned>
    SmallLoopDDRefThreshold("hir-complete-unroll-small-ddref-threshold",
                            cl::init(32), cl::Hidden,
                            cl::desc("Threshold for DDRefs in small loops "
                                     "(higher probability of unrolling)"));

static cl::opt<unsigned> SmallLoopAdditionalSavingsThreshold(
    "hir-complete-unroll-extra-savings-threshold", cl::init(5), cl::Hidden,
    cl::desc("Threshold for extra savings added to small loops to give them "
             "higher probability of unrolling)"));

const float O2MaxThresholdScalingFactor = 8.0;
const float O3MaxThresholdScalingFactor = 8.0;

static cl::opt<float> MaxThresholdScalingFactor(
    "hir-complete-unroll-max-threshold-scaling-factor", cl::init(0.0),
    cl::Hidden,
    cl::desc("Used to scale the thresholds of the loop based on how profitable "
             "the loop is over the base savings threshold. 0 means default "
             "threshold."));

static cl::opt<bool>
    AssumeDDIndependence("hir-complete-unroll-assume-dd-independence",
                         cl::init(false), cl::Hidden,
                         cl::desc("Cost model will assume DD independence for "
                                  "all memrefs in the unroll loopnest"));

HIRCompleteUnroll::HIRCompleteUnroll(char &ID, unsigned OptLevel, bool IsPreVec)
    : HIRTransformPass(ID), IsPreVec(IsPreVec) {

  Limits.SavingsThreshold =
      IsPreVec ? PreVectorSavingsThreshold : PostVectorSavingsThreshold;
  Limits.SmallLoopMemRefThreshold = SmallLoopMemRefThreshold;
  Limits.SmallLoopDDRefThreshold = SmallLoopDDRefThreshold;
  Limits.SmallLoopAdditionalSavingsThreshold =
      SmallLoopAdditionalSavingsThreshold;

  if (OptLevel == 0) {
    OptLevel = CommandLineOptLevel;
  }

  if (OptLevel <= 2) {
    Limits.LoopTripThreshold =
        (LoopTripThreshold == 0) ? O2LoopTripThreshold : LoopTripThreshold;
    Limits.LoopnestTripThreshold = (LoopnestTripThreshold == 0)
                                       ? O2LoopnestTripThreshold
                                       : LoopnestTripThreshold;
    Limits.UnrolledLoopMemRefThreshold = (UnrolledLoopMemRefThreshold == 0)
                                             ? O2UnrolledLoopMemRefThreshold
                                             : UnrolledLoopMemRefThreshold;
    Limits.UnrolledLoopDDRefThreshold = (UnrolledLoopDDRefThreshold == 0)
                                            ? O2UnrolledLoopDDRefThreshold
                                            : UnrolledLoopDDRefThreshold;
    Limits.MaxThresholdScalingFactor = (MaxThresholdScalingFactor == 0.0)
                                           ? O2MaxThresholdScalingFactor
                                           : MaxThresholdScalingFactor;
  } else {
    Limits.LoopTripThreshold =
        (LoopTripThreshold == 0) ? O3LoopTripThreshold : LoopTripThreshold;
    Limits.LoopnestTripThreshold = (LoopnestTripThreshold == 0)
                                       ? O3LoopnestTripThreshold
                                       : LoopnestTripThreshold;
    Limits.UnrolledLoopMemRefThreshold = (UnrolledLoopMemRefThreshold == 0)
                                             ? O3UnrolledLoopMemRefThreshold
                                             : UnrolledLoopMemRefThreshold;
    Limits.UnrolledLoopDDRefThreshold = (UnrolledLoopDDRefThreshold == 0)
                                            ? O3UnrolledLoopDDRefThreshold
                                            : UnrolledLoopDDRefThreshold;
    Limits.MaxThresholdScalingFactor = (MaxThresholdScalingFactor == 0.0)
                                           ? O3MaxThresholdScalingFactor
                                           : MaxThresholdScalingFactor;
  }
}

void HIRCompleteUnroll::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.setPreservesAll();
  AU.addRequiredTransitive<HIRFramework>();
  AU.addRequiredTransitive<HIRLoopStatistics>();
}

/// Visitor to update the CanonExpr.
struct HIRCompleteUnroll::CanonExprUpdater final : public HLNodeVisitorBase {
  const unsigned TopLoopLevel;
  SmallVectorImpl<int64_t> &IVValues;

  CanonExprUpdater(unsigned TopLoopLevel, SmallVectorImpl<int64_t> &IVValues)
      : TopLoopLevel(TopLoopLevel), IVValues(IVValues) {}

  void processRegDDRef(RegDDRef *RegDD);
  void processCanonExpr(CanonExpr *CExpr, bool IsTerminal);

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

  // Structure to store GEP ref info.
  struct GEPRefInfo {
    unsigned UniqueOccurences;
    unsigned TotalOccurences;
    bool DDIndependentInUnrolledLoop;

    GEPRefInfo(unsigned UniqueOccurences, unsigned TotalOccurences,
               bool IsIndependent)
        : UniqueOccurences(UniqueOccurences), TotalOccurences(TotalOccurences),
          DDIndependentInUnrolledLoop(IsIndependent) {}
  };

  // Structure to store information about visited refs.
  struct VisitedRefInfo {
    const RegDDRef *Ref;
    unsigned SimplifiedToConstSavings;
    unsigned UniqueOccurences;

    VisitedRefInfo(const RegDDRef *Ref, unsigned SimplifiedToConstSavings,
                   unsigned UniqueOccurences)
        : Ref(Ref), SimplifiedToConstSavings(SimplifiedToConstSavings),
          UniqueOccurences(UniqueOccurences) {}
  };

  // Structure to store blob related info.
  struct BlobInfo {
    bool Invariant;
    // Indicates whether the non-linear blob has been encountered before.
    bool Visited;
    // Indicates that the blob has been visited as the coeff of an unrollable
    // IV.
    bool VisitedAsUnrollableIVBlob;
    // Indicates whether blob definition can be simplified to a constant.
    bool Simplified;
    // Number of operations in the non-linear/simplified blob.
    unsigned NumOperations;
    // Blob is multiplied with a new coeff.
    bool IsNewCoeff;

    BlobInfo()
        : Invariant(true), Visited(false), VisitedAsUnrollableIVBlob(false),
          Simplified(false), NumOperations(0), IsNewCoeff(0) {}
  };

  const HIRCompleteUnroll &HCU;
  const HLLoop *CurLoop;
  const HLLoop *OuterLoop;

  unsigned LoopNestTripCount;

  unsigned Cost;
  unsigned ScaledCost;
  unsigned Savings;
  unsigned ScaledSavings;
  // Cost/Savings of GEP refs.
  // This is kept separate because it is already scaled unlike other
  // Cost/Savings which is scaled after processing a loop.
  unsigned GEPCost;
  unsigned GEPSavings;

  unsigned NumMemRefs;
  unsigned NumDDRefs;

  // Keeps track of non-linear blobs that we encounter during our traversal so
  // they aren't penalized multiple times. Blobs are removed from the set when
  // we encounter a redefinition of a contained temp. The mapped value is the
  // set of coefficients that the blob was multiplied with.
  DenseMap<unsigned, SmallVector<int64_t, 2>> VisitedNonLinearBlobs;

  // Keeps track of IV blobs that have been encountered before to avoid
  // duplicate costs.
  SmallSet<unsigned, 8> VisitedUnrollableIVBlobs;

  // Keeps track of temp blob definitions which get simplified to a constant due
  // to unrolling. This can result in simplification of other instructions.
  // Blobs which are simplified using rem (%) operation have their factor stored
  // as the mapped value.
  DenseMap<unsigned, unsigned> &SimplifiedTempBlobs;

  // Keep track of invariant GEP refs that have been visited to avoid
  // duplicating savings.
  SmallVector<VisitedRefInfo, 16> VisitedGEPRefs;

  // Contains mem refs of parent of OuterLoop if it exists, else contains mem
  // refs of OuterLoop.
  HIRCompleteUnroll::MemRefGatherer::MapTy &OuterLoopMemRefMap;

  // Set of simplifiable alloca store base values discovered in this loopnest.
  SmallPtrSet<const Value *, 16> &AllocaStoreBases;

  // Private constructor used for children loops.
  ProfitabilityAnalyzer(const HIRCompleteUnroll &HCU, const HLLoop *CurLp,
                        const HLLoop *OuterLp, unsigned ParentLoopNestTripCount,
                        DenseMap<unsigned, unsigned> &SimplifiedBlobs,
                        HIRCompleteUnroll::MemRefGatherer::MapTy &MemRefMap,
                        SmallPtrSet<const Value *, 16> &AllocaStoreBases)
      : HCU(HCU), CurLoop(CurLp), OuterLoop(OuterLp), Cost(0), ScaledCost(0),
        Savings(0), ScaledSavings(0), GEPCost(0), GEPSavings(0), NumMemRefs(0),
        NumDDRefs(0), SimplifiedTempBlobs(SimplifiedBlobs),
        OuterLoopMemRefMap(MemRefMap), AllocaStoreBases(AllocaStoreBases) {
    auto Iter = HCU.AvgTripCount.find(CurLp);
    assert((Iter != HCU.AvgTripCount.end()) && "Trip count of loop not found!");
    LoopNestTripCount = (ParentLoopNestTripCount * Iter->second);
  }

  /// level of any non-rem blob.
  unsigned populateRemBlobs(
      const RegDDRef *Ref,
      SmallVectorImpl<std::pair<unsigned, unsigned>> &RemBlobs) const;

  /// Returns max level of any non-simplified blob in Ref.
  unsigned getMaxNonSimplifiedBlobLevel(const RegDDRef *Ref) const;

  /// Returns true if \p Ref has no data dependency in \p Loop.
  bool isDDIndependentInLoop(const RegDDRef *Ref, const HLLoop *Loop) const;

  /// Computes and returns info on \p Ref in the completely unrolled
  /// loopnest such as its unique occurences. Returns 0 UniqueOccurences for a
  /// ref which is DD independent without unrolling.
  GEPRefInfo computeGEPInfo(const RegDDRef *Ref, bool IsMemRef) const;

  /// Adds additional cost associated with a GEP ref. \p CanSimplifySubs
  /// indicates that all the subscripts can be simplified to constants.
  /// Returns true if \p Ref can be simplified to a constant.
  bool addGEPCost(const RegDDRef *Ref, bool AddToVisitedSet,
                  bool CanSimplifySubs, unsigned NumAddressSimplifications);

  /// Returns true if \p Ref has been visited already. Sets \p AddToVisitedSet
  /// to true to indicate that \p Ref should be added to visited set.
  bool visited(const RegDDRef *Ref, bool &AddToVisitedSet);

  /// Processes RegDDRef for profitability. Returns true if Ref can be
  /// simplified to a constant.
  bool processRef(const RegDDRef *Ref);

  /// Processes CanonExpr for profitability. Returns true if CE can be
  /// simplified to a constant.
  bool processCanonExpr(const CanonExpr *CE, const RegDDRef *ParentRef);

  /// Processes IVs in the CE. Returns true if they can be simplified to a
  /// constant.
  bool processIVs(const CanonExpr *CE, const RegDDRef *ParentRef,
                  unsigned &NumSimplifiedTerms, unsigned &NumNonLinearTerms,
                  unsigned &NumUnrollableIVBlobs);

  /// Processes blobs in the CE. Returns true if they can be simplified to a
  /// constant.
  bool processBlobs(const CanonExpr *CE, const RegDDRef *ParentRef,
                    unsigned &NumSimplifiedTerms, unsigned &NumNonLinearTerms);

  /// Adds the cost of the blob given its info and coefficient in the CE. \p
  /// UnrollableIVLevel stores the level of unrollable IV this blob is a
  /// coefficient of. Otherwise it is set to 0.
  void addBlobCost(const BlobInfo &BInfo, int64_t Coeff,
                   unsigned UnrollableIVLevel, unsigned &NumNonLinearTerms,
                   bool *IsVisitedNonLinearTerm);

  /// Returns all the info assodicated with the blob.
  BlobInfo getBlobInfo(unsigned Index, int64_t Coeff, const RegDDRef *ParentRef,
                       bool CEIsLinear);

  /// \p HInst represents a simplified blob. Returns the divisior if this is a
  /// rem operation.
  unsigned getBlobFactor(HLInst *HInst) const;

  /// Updates all the visited blobs which contain the temp represented by self
  /// blob \p Ref. \p Simplified indicates whether the blob definition was
  /// simplified to a constant.
  void updateBlobs(const RegDDRef *Ref, bool Simplified);

  /// Returns percentage savings achieved by unrolling the loopnest.
  float getSavingsInPercentage() const;

  /// Returns true if this loop should be unrolled before vectorizer. This is a
  /// temporary workaround.
  bool isPreVectorProfitableLoop(const HLLoop *CurLoop) const;

  /// Returns true if simplifiable \p MemRef can be optimized away.
  bool canEliminate(const RegDDRef *MemRef);

  /// Scales the profitability by the given multiplier.
  void scale(unsigned Multiplier) {
    Cost *= Multiplier;
    Savings *= Multiplier;
    NumDDRefs *= Multiplier;
  }

  // Adds profitability analysis results from PA to this.
  ProfitabilityAnalyzer &operator+=(const ProfitabilityAnalyzer &PA) {
    Cost += PA.Cost;
    ScaledCost += PA.ScaledCost;
    Savings += PA.Savings;
    ScaledSavings += PA.ScaledSavings;
    GEPCost += PA.GEPCost;
    GEPSavings += PA.GEPSavings;
    NumMemRefs += PA.NumMemRefs;
    NumDDRefs += PA.NumDDRefs;

    return *this;
  }

public:
  ProfitabilityAnalyzer(const HIRCompleteUnroll &HCU, const HLLoop *CurLp,
                        DenseMap<unsigned, unsigned> &SimplifiedTempBlobs,
                        HIRCompleteUnroll::MemRefGatherer::MapTy &MemRefMap,
                        SmallPtrSet<const Value *, 16> &AllocaStoreBases)
      : ProfitabilityAnalyzer(HCU, CurLp, CurLp, 1, SimplifiedTempBlobs,
                              MemRefMap, AllocaStoreBases) {
    if (auto OuterLp = CurLp->getParentLoop()) {
      MemRefGatherer::gatherRange(OuterLp->child_begin(), OuterLp->child_end(),
                                  MemRefMap);
    } else {
      MemRefGatherer::gatherRange(CurLp->child_begin(), CurLp->child_end(),
                                  MemRefMap);
    }
  }

  // Main interface of the analyzer.
  void analyze();

  // Returns true if loopnest is profitable.
  bool isProfitable() const;

  // Returns true if loop has a small body.
  bool isSmallLoop(unsigned TripCount) const;

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

////// CanonExpr Visitor Start

void HIRCompleteUnroll::CanonExprUpdater::visit(HLLoop *Loop) {
  transformLoop(Loop, *this, false);
}

void HIRCompleteUnroll::CanonExprUpdater::visit(HLDDNode *Node) {

  assert(!isa<HLLoop>(Node) && "Loop node not expected!");

  for (auto Iter = Node->ddref_begin(), End = Node->ddref_end(); Iter != End;
       ++Iter) {
    processRegDDRef(*Iter);
  }
}

void HIRCompleteUnroll::CanonExprUpdater::processRegDDRef(RegDDRef *RegDD) {
  bool IsTerminal = RegDD->isTerminalRef();

  // Process CanonExprs inside the RegDDRefs
  for (auto Iter = RegDD->canon_begin(), End = RegDD->canon_end(); Iter != End;
       ++Iter) {
    processCanonExpr(*Iter, IsTerminal);
  }

  RegDD->makeConsistent(nullptr, TopLoopLevel - 1);
}

void HIRCompleteUnroll::CanonExprUpdater::processCanonExpr(CanonExpr *CExpr,
                                                           bool IsTerminal) {

  // Start replacing the IV's from TopLoopLevel to current loop level.
  auto LoopLevel = TopLoopLevel;

  for (auto &Val : IVValues) {
    CExpr->replaceIVByConstant(LoopLevel, Val);
    LoopLevel++;
  }

  CExpr->simplify(IsTerminal);
}

///// CanonExpr Visitor End

///// ProfitabilityAnalyzer Visitor Start

bool HIRCompleteUnroll::ProfitabilityAnalyzer::isPreVectorProfitableLoop(
    const HLLoop *CurLoop) const {

  if (!HCU.IsPreVec || !CurLoop->isInnermost()) {
    return false;
  }

  auto Upper = CurLoop->getUpperCanonExpr();
  int64_t UpperVal;

  if (!Upper->isIntConstant(&UpperVal) || (UpperVal != 3)) {
    return false;
  }

  unsigned NumIfs = 0;
  unsigned NumSelects = 0;
  unsigned NumRems = 0;
  unsigned NumXORs = 0;

  for (auto NodeIt = CurLoop->child_begin(), E = CurLoop->child_end();
       NodeIt != E; ++NodeIt) {
    auto Node = &*NodeIt;

    if (isa<HLIf>(Node)) {
      ++NumIfs;

    } else if (auto HInst = dyn_cast<HLInst>(Node)) {
      unsigned OpCode = HInst->getLLVMInstruction()->getOpcode();

      if (OpCode == Instruction::URem || OpCode == Instruction::SRem) {
        ++NumRems;
      } else if (OpCode == Instruction::Select) {
        ++NumSelects;
      } else if (OpCode == Instruction::Xor) {
        ++NumXORs;
      }
    }
  }

  return (NumIfs == 4) && (NumRems == 2) && (NumSelects == 1) && (NumXORs == 3);
}

void HIRCompleteUnroll::ProfitabilityAnalyzer::analyze() {
  // TODO: Think about visiting the linear instructions at the end of the loop
  // body first so that they are treated as simplified. This happens when IV is
  // parsed as blob.
  CurLoop->getHLNodeUtils().visitRange<true, false>(
      *this, CurLoop->child_begin(), CurLoop->child_end());

  // Scale results by loop's average trip count.
  auto Iter = HCU.AvgTripCount.find(CurLoop);
  assert((Iter != HCU.AvgTripCount.end()) && "Trip count of loop not found!");

  bool IsPreVecProfitableLoop = isPreVectorProfitableLoop(CurLoop);

  // Check if the loop is small enough to assign some extra profitability to it
  // (for eliminating loop control) and give it higher chance of unrolling.
  if (isSmallLoop(Iter->second) || IsPreVecProfitableLoop) {
    Savings +=
        std::min(HCU.Limits.SmallLoopAdditionalSavingsThreshold, Iter->second);
  }

  // Workaround to make loop profitable till vectorizer fixes its cost model.
  if (IsPreVecProfitableLoop) {
    Savings *= 3;
  }

  scale(Iter->second);

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

bool HIRCompleteUnroll::ProfitabilityAnalyzer::isSmallLoop(
    unsigned TripCount) const {
  // Do not consider inner loops in a loopnest to be small loops. This skews the
  // profitability model.
  return !HCU.IsPreVec && (CurLoop == OuterLoop) &&
         // NumMemRefs are computed in a scaled manner in computeGEPInfo() so we
         // need to divide it by the trip count to get original count.
         ((NumMemRefs / TripCount) <= HCU.Limits.SmallLoopMemRefThreshold) &&
         (NumDDRefs <= HCU.Limits.SmallLoopDDRefThreshold);
}

float HIRCompleteUnroll::ProfitabilityAnalyzer::getSavingsInPercentage() const {
  auto TotalCost = Cost + ScaledCost + GEPCost;

  float SafeCost = (TotalCost == 0) ? 1 : TotalCost;
  return ((Savings + ScaledSavings + GEPSavings) * 100) / SafeCost;
}

bool HIRCompleteUnroll::ProfitabilityAnalyzer::isProfitable() const {

  auto SavingsPercentage = getSavingsInPercentage();

  DEBUG(dbgs() << "Cost: " << Cost << "\n");
  DEBUG(dbgs() << "ScaledCost: " << ScaledCost << "\n");
  DEBUG(dbgs() << "GEPCost: " << GEPCost << "\n");
  DEBUG(dbgs() << "Savings: " << Savings << "\n");
  DEBUG(dbgs() << "ScaledSavings: " << ScaledSavings << "\n");
  DEBUG(dbgs() << "GEPSavings: " << GEPSavings << "\n");

  DEBUG(dbgs() << "Savings in percentage: " << SavingsPercentage << "\n");

  DEBUG(dbgs() << "Number of memrefs: " << NumMemRefs << "\n");
  DEBUG(dbgs() << "Number of ddrefs: " << NumDDRefs << "\n");
  DEBUG(dbgs() << "Loop: \n"; CurLoop->dump(); dbgs() << "\n");

  if (SavingsPercentage < HCU.Limits.SavingsThreshold) {
    return false;
  }

  // Use postvec(smaller) savings threshold to derive consistent scaling factor
  // for prevec and postvec passes.
  float ScalingFactor = (SavingsPercentage / PostVectorSavingsThreshold);

  ScalingFactor = std::min(ScalingFactor, HCU.Limits.MaxThresholdScalingFactor);

  auto Iter = HCU.TotalTripCount.find(OuterLoop);
  assert((Iter != HCU.TotalTripCount.end()) && "Trip count of loop not found!");

  return (Iter->second <= (ScalingFactor * HCU.Limits.LoopnestTripThreshold)) &&
         (NumMemRefs <=
          (ScalingFactor * HCU.Limits.UnrolledLoopMemRefThreshold)) &&
         (NumDDRefs <= (ScalingFactor * HCU.Limits.UnrolledLoopDDRefThreshold));
}

void HIRCompleteUnroll::ProfitabilityAnalyzer::visit(const HLLoop *Lp) {
  // Analyze child loop.
  ProfitabilityAnalyzer PA(HCU, Lp, OuterLoop, LoopNestTripCount,
                           SimplifiedTempBlobs, OuterLoopMemRefMap,
                           AllocaStoreBases);
  PA.analyze();

  // Add the result of child loop profitability analysis.
  *this += PA;
}

unsigned
HIRCompleteUnroll::ProfitabilityAnalyzer::getBlobFactor(HLInst *HInst) const {
  auto Inst = HInst->getLLVMInstruction();

  // Looking for something like this -
  // tmp = i1 % 4;

  auto OpCode = Inst->getOpcode();

  if ((OpCode != Instruction::URem) && (OpCode != Instruction::SRem)) {
    return 0;
  }

  auto RvalOp2 = HInst->getOperandDDRef(2);

  int64_t Factor;

  if (!RvalOp2->isIntConstant(&Factor) || (Factor < 0) || (Factor > UINT_MAX)) {
    return 0;
  }

  return Factor;
}

void HIRCompleteUnroll::ProfitabilityAnalyzer::updateBlobs(
    const RegDDRef *LvalRef, bool Simplified) {

  auto &BU = LvalRef->getBlobUtils();
  auto TempIndex = LvalRef->isSelfBlob()
                       ? LvalRef->getSelfBlobIndex()
                       : BU.findTempBlobIndex(LvalRef->getSymbase());

  if (TempIndex == InvalidBlobIndex) {
    return;
  }

  auto TempBlob = BU.getBlob(TempIndex);

  for (auto &BlobCoeffsPair : VisitedNonLinearBlobs) {
    if (BU.contains(BU.getBlob(BlobCoeffsPair.first), TempBlob)) {
      VisitedNonLinearBlobs.erase(BlobCoeffsPair.first);
    }
  }

  if (Simplified) {
    unsigned Factor = getBlobFactor(cast<HLInst>(LvalRef->getHLDDNode()));
    SimplifiedTempBlobs.insert(std::make_pair(TempIndex, Factor));
  } else {
    SimplifiedTempBlobs.erase(TempIndex);
  }
}

void HIRCompleteUnroll::ProfitabilityAnalyzer::visit(const HLDDNode *Node) {
  auto HInst = dyn_cast<HLInst>(Node);
  auto Inst = HInst ? HInst->getLLVMInstruction() : nullptr;
  bool IsSelect = (Inst && isa<SelectInst>(Inst));

  auto RefIt = HInst ? HInst->rval_op_ddref_begin() : Node->op_ddref_begin();
  auto End = HInst ? HInst->rval_op_ddref_end() : Node->op_ddref_end();

  unsigned NumRvalOp = 0;
  const RegDDRef *LvalRef = nullptr;
  const RegDDRef *RvalRef = nullptr;

  // TODO: Add other types of non-simplifyable instructions.
  bool CanSimplifyRvals =
      (!Inst || (!isa<CallInst>(Inst) && !isa<ReturnInst>(Inst)));
  bool CanSimplifyLval = true;

  for (; RefIt != End; ++RefIt, ++NumRvalOp) {
    RvalRef = *RefIt;

    if (!processRef(RvalRef)) {
      // Only the first two operands of select are relavant for simplification.
      if (!IsSelect || (NumRvalOp < 2)) {
        CanSimplifyRvals = false;
      }
    }
  }

  if (HInst && (LvalRef = HInst->getLvalDDRef())) {
    // Terminal lval refs are only used to invalidate their encountered uses.
    if (LvalRef->isTerminalRef()) {
      // If rvals can be simplified, consider terminal lval as also simplified.
      CanSimplifyLval = CanSimplifyRvals;
      updateBlobs(LvalRef, CanSimplifyRvals);
    } else {
      CanSimplifyLval = processRef(LvalRef);
    }
  }

  // Ignore gep/copy instructions as all the cost has been accounted for in
  // refs.
  bool IsGEP = false;
  if (Inst && ((IsGEP = isa<GetElementPtrInst>(Inst)) || HInst->isCopyInst())) {
    if (IsGEP || !CanSimplifyRvals) {
      NumDDRefs += 2;
    }
    return;
  }

  // Load/Store instructions can be simplified/eliminated if they are constant
  // array/alloca accesses. We let them through to account for these savings.

  if (CanSimplifyRvals) {
    assert(RvalRef && "At least one rval ref is expected!");

    // Only add savings if rval is not already a constant.
    if ((NumRvalOp != 1) || !RvalRef->isConstant()) {
      ++Savings;
    }
  } else {
    // Account for ddrefs only if the node cannot be simplified.
    NumDDRefs += NumRvalOp;
  }

  if (LvalRef) {
    if (CanSimplifyLval) {
      ++Savings;
    } else {
      ++NumDDRefs;
    }
  }

  // Cost of load/store has already been accounted for in refs.
  if (Inst && (isa<LoadInst>(Inst) || isa<StoreInst>(Inst))) {
    return;
  }

  if (!CanSimplifyRvals || !CanSimplifyLval) {
    ++Cost;
  }
}

unsigned HIRCompleteUnroll::ProfitabilityAnalyzer::populateRemBlobs(
    const RegDDRef *Ref,
    SmallVectorImpl<std::pair<unsigned, unsigned>> &RemBlobs) const {
  assert(Ref->hasGEPInfo() && "GEP ref expected!");

  unsigned MaxNonRemBlobLevel = 0;
  unsigned CurLevel = CurLoop->getNestingLevel();

  for (auto BIt = Ref->blob_cbegin(), End = Ref->blob_cend(); BIt != End;
       ++BIt) {
    auto Blob = *BIt;
    auto Index = Blob->getBlobIndex();
    unsigned BlobLevel =
        Blob->isNonLinear() ? CurLevel : Blob->getDefinedAtLevel();

    auto Iter = SimplifiedTempBlobs.find(Index);

    if ((Iter != SimplifiedTempBlobs.end()) && Iter->second) {
      RemBlobs.push_back(std::make_pair(BlobLevel, Iter->second));
    } else {
      MaxNonRemBlobLevel = std::max(MaxNonRemBlobLevel, BlobLevel);
    }
  }

  return MaxNonRemBlobLevel;
}

unsigned HIRCompleteUnroll::ProfitabilityAnalyzer::getMaxNonSimplifiedBlobLevel(
    const RegDDRef *Ref) const {
  assert(Ref->hasGEPInfo() && "GEP ref expected!");

  unsigned MaxNonSimplifiedBlobLevel = 0;

  unsigned CurLevel = Ref->getParentLoop()->getNestingLevel();

  for (auto BIt = Ref->blob_cbegin(), End = Ref->blob_cend(); BIt != End;
       ++BIt) {
    auto Blob = *BIt;
    auto Index = Blob->getBlobIndex();
    unsigned BlobLevel =
        Blob->isNonLinear() ? CurLevel : Blob->getDefinedAtLevel();

    auto Iter = SimplifiedTempBlobs.find(Index);

    if (Iter == SimplifiedTempBlobs.end()) {
      MaxNonSimplifiedBlobLevel =
          std::max(MaxNonSimplifiedBlobLevel, BlobLevel);
    }
  }

  return MaxNonSimplifiedBlobLevel;
}

bool HIRCompleteUnroll::ProfitabilityAnalyzer::isDDIndependentInLoop(
    const RegDDRef *Ref, const HLLoop *Loop) const {
  assert(Ref->isMemRef() && "Only mem ref is expected!");

  unsigned LoopLevel = Loop->getNestingLevel();
  bool IsOutermostLoop = (Loop == OuterLoop->getParentLoop());

  // OutermostLoop is the resulting loop after complete unroll so we also need
  // to check for structural invariance to conclude DD independence.
  if (IsOutermostLoop && (Ref->hasIV(LoopLevel) ||
                          (getMaxNonSimplifiedBlobLevel(Ref) >= LoopLevel))) {
    return false;
  }

  if (AssumeDDIndependence) {
    return true;
  }

  // Assume refs with noalias metadata are invariant due to multiversioning.
  if (Loop->isInnermost() || HLNodeUtils::isPerfectLoopNest(Loop)) {
    AAMDNodes AANodes;
    Ref->getAAMetadata(AANodes);

    if (AANodes.NoAlias != nullptr) {
      return true;
    }
  }

  bool IsRval = Ref->isRval();
  auto BaseCE = Ref->getBaseCE();

  // Ref can be hoisted outside the loop if all other refs with the same base
  // are also structurally invariant.
  for (auto SymRef : OuterLoopMemRefMap[Ref->getSymbase()]) {
    if (SymRef == Ref) {
      continue;
    }

    if (IsRval && SymRef->isRval()) {
      continue;
    }

    if (!IsOutermostLoop &&
        !HLNodeUtils::contains(Loop, SymRef->getHLDDNode())) {
      continue;
    }

    if (DDRefUtils::areEqual(Ref, SymRef)) {
      continue;
    }

    // If bases do not match, Ref is not independent.
    if (!CanonExprUtils::areEqual(BaseCE, SymRef->getBaseCE())) {
      return false;
    }

    if (getMaxNonSimplifiedBlobLevel(SymRef) >= LoopLevel) {
      return false;
    }

    if (IsOutermostLoop && SymRef->hasIV(LoopLevel)) {
      return false;
    }
  }

  return true;
}

HIRCompleteUnroll::ProfitabilityAnalyzer::GEPRefInfo
HIRCompleteUnroll::ProfitabilityAnalyzer::computeGEPInfo(const RegDDRef *Ref,
                                                         bool IsMemRef) const {

  SmallVector<std::pair<unsigned, unsigned>, 4> RemBlobs;

  unsigned MaxNonRemBlobLevel = populateRemBlobs(Ref, RemBlobs);

  if (MaxNonRemBlobLevel >= CurLoop->getNestingLevel()) {
    return GEPRefInfo(LoopNestTripCount, LoopNestTripCount, false);
  }

  unsigned UniqueOccurences = 0;
  unsigned TotalOccurences = 1;
  const HLLoop *OutermostLoop = OuterLoop->getParentLoop();
  bool IsUnique = false;
  bool EncounteredRemBlob = false;

  // Compute unique occurences of ref based on its structure.
  for (const HLLoop *ParentLoop = CurLoop; ParentLoop != OutermostLoop;
       ParentLoop = ParentLoop->getParentLoop()) {

    auto TCIt = HCU.AvgTripCount.find(ParentLoop);
    assert((TCIt != HCU.AvgTripCount.end()) && "Trip count of loop not found!");

    unsigned Level = ParentLoop->getNestingLevel();

    // If ref contains IV of a loop or a blob defined at that level, all
    // references of the ref are considered unique w.r.t that level.
    // If ref is not DD independent in loop, there can be no savings
    // from unrolling.
    IsUnique = IsUnique || (MaxNonRemBlobLevel >= Level) ||
               (IsMemRef && !isDDIndependentInLoop(Ref, ParentLoop));

    if (IsUnique || Ref->hasIV(Level)) {

      TotalOccurences *= TCIt->second;

      if (!UniqueOccurences) {
        UniqueOccurences = TCIt->second;
      } else {
        UniqueOccurences *= TCIt->second;
      }
      continue;
    }

    // Multiply the factors of all rem blobs at this level to get the final
    // factor.
    unsigned Factor = 1;
    for (auto &Blob : RemBlobs) {
      if (Blob.first == Level) {
        assert(Blob.second && "Found rem blob with factor of 0!");
        Factor *= Blob.second;
      }
    }

    // Ref is invariant w.r.t this loop so it doesn't yield additional unique
    // occurences.
    if (Factor == 1) {
      // If rem blob was encountered in the inner loop, we still need to
      // update total occurences as the ref is not invariant without
      // unrolling.
      if (EncounteredRemBlob) {
        TotalOccurences *= TCIt->second;
      }

      continue;

    } else {
      TotalOccurences *= TCIt->second;
      EncounteredRemBlob = true;
    }

    // At least one rem blob is present at this level. The max number of
    // unique references depend on the factor of the rem operation.
    if (!UniqueOccurences) {
      UniqueOccurences = Factor;
    } else {
      UniqueOccurences *= Factor;
    }
  }

  // This can happen if rem factor is greater than trip count.
  if (UniqueOccurences > TotalOccurences) {
    UniqueOccurences = TotalOccurences;
  }

  bool IsIndependent = false;
  if (UniqueOccurences && IsMemRef && OutermostLoop &&
      isDDIndependentInLoop(Ref, OutermostLoop)) {
    IsIndependent = true;
  }

  return GEPRefInfo(UniqueOccurences, TotalOccurences, IsIndependent);
}

bool HIRCompleteUnroll::ProfitabilityAnalyzer::canEliminate(
    const RegDDRef *MemRef) {
  assert(MemRef->isMemRef() && "Memref expected!");

  if (MemRef->accessesConstantArray()) {
    return true;
  }

  if (!MemRef->accessesAlloca()) {
    return false;
  }

  auto BaseVal = MemRef->getTempBaseValue();

  if (MemRef->isLval()) {
    auto Node = MemRef->getHLDDNode();

    // Assume unconditional alloca stores can be eliminated after unrolling by
    // propagating the assigned value directly into corresponding loads.
    if (isa<HLLoop>(Node->getParent())) {
      AllocaStoreBases.insert(BaseVal);
      return true;
    }

    return false;
  }

  // Assume alloca load can be eliminated if we have encountered correspsonding
  // alloca store.
  return (HCU.UnrolledAllocaStoreBases.count(BaseVal) ||
          AllocaStoreBases.count(BaseVal));
}

bool HIRCompleteUnroll::ProfitabilityAnalyzer::addGEPCost(
    const RegDDRef *Ref, bool AddToVisitedSet, bool CanSimplifySubs,
    unsigned NumAddressSimplifications) {
  assert(Ref->hasGEPInfo() && "GEP ref expected!");

  bool IsMemRef = Ref->isMemRef();
  unsigned BaseCost = IsMemRef ? 2 : 1;

  // Here we account for savings from redundancies in GEP refs exposed due to
  // complete unroll.
  //
  // Consider this case-
  // DO i1 = 0, 10
  //   DO i2 = 0, 5
  //     A[i2] =
  //   END DO
  // END DO
  //
  // Unrolling of the i1 loopnest will yield redundant loads of A[i2] for each
  // i1 loop iteration.
  //
  // Another example with a rem blob-
  //
  // DO i1 = 0, 5
  //   %rem = i1 % 2;
  //   A[%rem] =
  // END DO
  //
  // A[%rem] can yield at most two different memory locations due to the rem
  // operation in a loop with a trip count of 6. So there are (6 - 2) = 4
  // redundant memory accesses.
  //
  // This is just an estimate as computing redundancies accurately is
  // mathematically complicated.
  // There are additional kinds of redundancies currently not taken into
  // account.
  // For example-
  // 1) Subscripts containing multiple IVs.
  // 2) Subscripts with a combination of IV and rem blobs.

  unsigned SimplifiedToConstSavings = 0;
  unsigned UniqueOccurences = 0;

  bool CanSimplifyToConst = (IsMemRef && CanSimplifySubs && canEliminate(Ref));

  if (CanSimplifyToConst) {
    // Everything goes to savings for refs which can be simplified to
    // constants.
    SimplifiedToConstSavings = (LoopNestTripCount * BaseCost);

    // Double savings for lval allocas assuming that unrolling the loop will
    // help eliminate at least 1 corresponding load alloca.
    if (Ref->isLval()) {
      SimplifiedToConstSavings *= 2;
    }

    // Account for address computations we saved.
    SimplifiedToConstSavings += (LoopNestTripCount * NumAddressSimplifications);

    GEPSavings += SimplifiedToConstSavings;

  } else {
    auto GEPInfo = computeGEPInfo(Ref, IsMemRef);
    UniqueOccurences = GEPInfo.UniqueOccurences;

    if (UniqueOccurences != 0) {
      GEPCost += (UniqueOccurences * BaseCost);
      GEPSavings += ((GEPInfo.TotalOccurences - UniqueOccurences) * BaseCost);

      // Account for address computations we saved.
      GEPSavings += (GEPInfo.TotalOccurences * NumAddressSimplifications);

      // This ref can be hoisted outside the unrolled loop so we add extra
      // savings.
      if (GEPInfo.DDIndependentInUnrolledLoop) {
        GEPSavings += (UniqueOccurences * BaseCost);
      }

      if (IsMemRef) {
        NumMemRefs += UniqueOccurences;
      }
    }
  }

  if (AddToVisitedSet) {
    // Add linear refs to visited set to avoid duplicate processing.
    VisitedGEPRefs.emplace_back(Ref, SimplifiedToConstSavings,
                                UniqueOccurences);
  }

  return CanSimplifyToConst;
}

bool HIRCompleteUnroll::ProfitabilityAnalyzer::visited(const RegDDRef *Ref,
                                                       bool &AddToVisitedSet) {
  if (!Ref->hasGEPInfo()) {
    return false;
  }

  unsigned DefLevel = Ref->getDefinedAtLevel();

  if (DefLevel == NonLinearLevel) {
    return false;
  }

  for (auto &RefInfo : VisitedGEPRefs) {
    if (DDRefUtils::areEqual(Ref, RefInfo.Ref) &&
        (Ref->isRval() == RefInfo.Ref->isRval())) {

      if (RefInfo.SimplifiedToConstSavings != 0) {
        // Simplfied to const savings should be accounted for each occurence
        // of the ref.
        GEPSavings += RefInfo.SimplifiedToConstSavings;
      } else if (Ref->isMemRef()) {
        NumMemRefs += RefInfo.UniqueOccurences;
      }

      return true;
    }
  }

  AddToVisitedSet = true;
  return false;
}

bool HIRCompleteUnroll::ProfitabilityAnalyzer::processRef(const RegDDRef *Ref) {

  bool AddToVisitedSet = false;

  if (visited(Ref, AddToVisitedSet)) {
    return false;
  }

  bool CanSimplify = true;
  unsigned NumAddressSimplifications = 0;
  unsigned NumAddressSimplificationTerms = 0;
  bool HasGEPInfo = Ref->hasGEPInfo();
  bool IsFirstCE = true;
  bool HasConstantTerm = false;

  for (auto CEIt = Ref->canon_begin(), E = Ref->canon_end(); CEIt != E;
       ++CEIt) {
    auto CE = *CEIt;

    if (!processCanonExpr(CE, Ref)) {
      CanSimplify = false;

    } else if (HasGEPInfo) {
      int64_t Val;
      bool IsConst = CE->isIntConstant(&Val);

      if (IsConst) {
        // If the CE is already constant, we haven't simplified anything but if
        // it is non-zero there is a possibility of this getting folded if we
        // simplify some other index.
        if (Val != 0) {
          HasConstantTerm = true;
        }
      } else {
        if (IsFirstCE) {
          ++NumAddressSimplificationTerms;
        } else {
          // Add one for simplification of index * stride.
          ++NumAddressSimplifications;
          ++NumAddressSimplificationTerms;
        }
      }
    }

    IsFirstCE = false;
  }

  if (HasGEPInfo) {
    if (Ref->accessesAlloca() || Ref->accessesInternalGlobalVar()) {
      // The base addess is known at compile time.
      ++NumAddressSimplificationTerms;
    }

    if (NumAddressSimplificationTerms) {
      NumAddressSimplifications += (NumAddressSimplificationTerms +
                                    static_cast<unsigned>(HasConstantTerm) - 1);
    }

    CanSimplify = addGEPCost(Ref, AddToVisitedSet, CanSimplify,
                             NumAddressSimplifications);
  }

  return CanSimplify;
}

/// Evaluates profitability of CanonExpr.
/// Example 1-
/// The profitability index of CE: (3 * i1 + 1) is 3. It is computed as
/// follows-
/// +1 for substitution of i2 by constant.
/// +1 for simplification of (3 * i1) to a constant.
/// +1 for simplification of (3 * i1 + 1) to a constant.
///
/// Example 2-
/// The profitability index of CE: (b1 * i1 + 1) where b1 is a linear temp is
/// 3. It is computed as follows-
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
/// being unrolled and b1 is a non-linear temp is 3. It is computed as
/// follows-
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
  unsigned NumUnrollableIVBlobs = 0;
  bool IsLinear = CE->isLinearAtLevel();

  if (CE->isConstantData()) {
    return true;
  }

  bool CanSimplifyIVs = processIVs(CE, ParentRef, NumSimplifiedTerms,
                                   NumNonLinearTerms, NumUnrollableIVBlobs);

  bool CanSimplifyBlobs =
      processBlobs(CE, ParentRef, NumSimplifiedTerms, NumNonLinearTerms);

  bool NumeratorBecomesConstant = CanSimplifyIVs && CanSimplifyBlobs;

  // For each unrollable IV which has a blob, we can simplify the terms in the
  // first unrolled iteration when IV value is zero. For example, if i1 can be
  // unrolled and b2 can be simplified, CE: (b1*i1 + b2 + 2) has a total of 3
  // terms (2 additions) which can be folded when i1 is 0.
  if (NumUnrollableIVBlobs) {
    ScaledSavings += NumUnrollableIVBlobs + NumSimplifiedTerms +
                     (CE->getConstant() ? 1 : 0) - 1;
  }

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
    unsigned &NumSimplifiedTerms, unsigned &NumNonLinearTerms,
    unsigned &NumUnrollableIVBlobs) {

  bool CanSimplifyIVs = true;
  unsigned NodeLevel = CurLoop->getNestingLevel();
  unsigned OuterLevel = OuterLoop->getNestingLevel();
  bool IsLinear = CE->isLinearAtLevel();
  SmallSet<unsigned, 4> CurrentUnrollableIVBlobs;

  for (unsigned Level = 1; Level <= NodeLevel; ++Level) {
    unsigned BlobIndex;
    int64_t Coeff;

    CE->getIVCoeff(Level, &BlobIndex, &Coeff);

    if (!Coeff) {
      continue;
    }

    bool IsUnrollableLoopLevel = (Level >= OuterLevel);

    if (BlobIndex != InvalidBlobIndex) {
      // For unroll loop levels, constant will be multiplied by simplified IV
      // so we conservatively pass the coeff as 1.
      auto BInfo = getBlobInfo(BlobIndex, IsUnrollableLoopLevel ? 1 : Coeff,
                               ParentRef, IsLinear);

      if (IsUnrollableLoopLevel) {
        if (BInfo.Simplified) {
          ++NumSimplifiedTerms;
        } else {
          CanSimplifyIVs = false;
        }

        // If same blob appears in multiple unrollable IVs or in IV and as a
        // stand-alone blob, it can be folded. For example, (b * i1 + b) can
        // be folded into (c * b) after IV is replaced by constant. We add 1 to
        // savings to account for the folding.
        if (CurrentUnrollableIVBlobs.count(BlobIndex) ||
            (CE->getBlobCoeff(BlobIndex) != 0)) {
          ++Savings;
        }

        CurrentUnrollableIVBlobs.insert(BlobIndex);

      } else {
        CanSimplifyIVs = false;
      }

      addBlobCost(BInfo, Coeff, IsUnrollableLoopLevel ? Level : 0,
                  NumNonLinearTerms, nullptr);

      if (IsUnrollableLoopLevel) {
        // Add to loop level blob set to avoid duplicate cost.
        VisitedUnrollableIVBlobs.insert(BlobIndex);
      }

    } else if (IsUnrollableLoopLevel) {

      // Add one for simplfication of multiplication with coefficient.
      if (Coeff != 1) {
        ++Savings;
      }

      ++NumSimplifiedTerms;

    } else {
      CanSimplifyIVs = false;
    }
  }

  // Make sure we add at least 1 to savings for turning any IV into a
  // constant. Otherwise converting simple expressions like A[i1] to A[0] will
  // not be considered savings.
  if (NumSimplifiedTerms != 0) {
    ++Savings;
  }

  NumUnrollableIVBlobs = CurrentUnrollableIVBlobs.size();

  return CanSimplifyIVs;
}

bool HIRCompleteUnroll::ProfitabilityAnalyzer::processBlobs(
    const CanonExpr *CE, const RegDDRef *ParentRef,
    unsigned &NumSimplifiedTerms, unsigned &NumNonLinearTerms) {
  bool CanSimplifyBlobs = true;
  bool IsLinear = CE->isLinearAtLevel();
  bool HasVisitedNonLinearTerms = false;

  for (auto Blob = CE->blob_begin(), E = CE->blob_end(); Blob != E; ++Blob) {

    auto BInfo = getBlobInfo(Blob->Index, Blob->Coeff, ParentRef, IsLinear);

    if (BInfo.Simplified) {
      ++NumSimplifiedTerms;
    } else {
      CanSimplifyBlobs = false;
    }

    // If this blob occured as IV blob coefficient, there are two cases-
    // 1) If the blob occured in the same CE, it was processed as part of the
    // folding logic in processIVs() OR 2) It needs to be ignored on the
    // assumption that that all (coeff * blob) combinations will be available
    // after unrolling the loop when the IV is subtituted with constants.
    if (VisitedUnrollableIVBlobs.count(Blob->Index)) {
      continue;
    }

    addBlobCost(BInfo, Blob->Coeff, 0, NumNonLinearTerms,
                &HasVisitedNonLinearTerms);
  }

  if (HasVisitedNonLinearTerms) {
    ++NumNonLinearTerms;
  }

  return CanSimplifyBlobs;
}

HIRCompleteUnroll::ProfitabilityAnalyzer::BlobInfo
HIRCompleteUnroll::ProfitabilityAnalyzer::getBlobInfo(unsigned Index,
                                                      int64_t Coeff,
                                                      const RegDDRef *ParentRef,
                                                      bool CEIsLinear) {
  BlobInfo BInfo;

  BInfo.Simplified = SimplifiedTempBlobs.count(Index);

  BInfo.VisitedAsUnrollableIVBlob = VisitedUnrollableIVBlobs.count(Index);

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

  unsigned NumSimplifiedTempBlobs = 0;

  for (auto Idx : Indices) {
    unsigned DefLevel;
    bool Found = ParentRef->findTempBlobLevel(Idx, &DefLevel);
    (void)Found;
    assert(Found && "Temp blob not found in Ref!");

    if (SimplifiedTempBlobs.count(Idx)) {
      ++NumSimplifiedTempBlobs;
    } else if (DefLevel == NonLinearLevel) {
      Invariant = false;
      VisitedNonLinearBlobs.insert(
          std::make_pair(Idx, SmallVector<int64_t, 2>()));
    }
  }

  // All containing temp blobs can be simplified so we mark the blob as
  // simplified.
  if (NumSimplifiedTempBlobs == Indices.size()) {
    BInfo.Simplified = true;
    BInfo.NumOperations = BU.getNumOperations(Index);

  } else if (!Invariant) {
    BInfo.Invariant = false;
    BInfo.NumOperations = BU.getNumOperations(Index);

    // Subtract operations based on contained simplified temps.
    if (NumSimplifiedTempBlobs) {
      BInfo.NumOperations -= (NumSimplifiedTempBlobs - 1);
    }

    auto Iter = VisitedNonLinearBlobs.find(Index);

    if (Iter != VisitedNonLinearBlobs.end()) {
      BInfo.Visited = true;

      // Check whether this is a new coeff for the blob and update the
      // coefficient set. Ignore unit coefficient.
      if (Coeff != 1) {
        bool FoundCoeff = false;
        for (auto PrevCoeff : Iter->second) {
          if (PrevCoeff == Coeff) {
            FoundCoeff = true;
            break;
          }
        }

        if (!FoundCoeff) {
          BInfo.IsNewCoeff = true;
          Iter->second.push_back(Coeff);
        }
      }
    } else {
      VisitedNonLinearBlobs.insert(
          std::make_pair(Index, SmallVector<int64_t, 2>{Coeff}));
    }
  }

  return BInfo;
}

void HIRCompleteUnroll::ProfitabilityAnalyzer::addBlobCost(
    const BlobInfo &BInfo, int64_t Coeff, unsigned UnrollableIVLevel,
    unsigned &NumNonLinearTerms, bool *IsVisitedNonLinearTerm) {

  unsigned OuterTripCnt = 0;

  if (UnrollableIVLevel != 0) {
    // If IV belongs to outer loop, cost incurred should be based on outer
    // loop trip count. For example, if the CanonExpr is (t * i1 + i2), number
    // of unique combinations of (t * i1) depend only on the outer loop trip
    // count.
    auto OuterLoop = CurLoop->getParentLoopAtLevel(UnrollableIVLevel);
    OuterTripCnt = HCU.AvgTripCount.find(OuterLoop)->second;
  }

  if (BInfo.Simplified) {
    Savings += (BInfo.NumOperations ? BInfo.NumOperations : 1);

    if (Coeff != 1) {
      ++Savings;
    }

  } else if (BInfo.VisitedAsUnrollableIVBlob) {
    // This blob has been visited as an IV blob before so we won't be adding any
    // additional cost. We do add additional savings if it has appeared as an IV
    // blob again this time.

    if (UnrollableIVLevel != 0) {
      // The first iteration when IV is zero can be simplified.
      ++ScaledSavings;

      // Multiplication of unrolled IV and constant coefficient can be folded.
      if (Coeff != 1) {
        ScaledSavings += OuterTripCnt;
      }
    }
  } else if (BInfo.Visited) {
    if (BInfo.IsNewCoeff) {
      ++Cost;
      ++NumNonLinearTerms;

    } else if (IsVisitedNonLinearTerm) {
      // We have seen this blob with the same coeff earlier. We collapse such
      // blobs into a single non-linear term assuming that such computation
      // happened in a previous CE in the loop. This is an optimistic assumption
      // but I think it has high chance of being true. For example- if we have
      // already seen (a + b + c) before with non-lnear blobs a, b and c and
      // then we encounter (a + b + d) we should not penalize addtion of (a + b)
      // again.
      *IsVisitedNonLinearTerm = true;

    } else {
      ++NumNonLinearTerms;
    }

  } else if (!BInfo.Invariant) {
    Cost += BInfo.NumOperations;

    if (Coeff != 1) {
      ++Cost;
    }
    ++NumNonLinearTerms;

  } else if (UnrollableIVLevel != 0) {
    // Multiplication of invariant blob with unrolled IV leads to increase in
    // size as the blob is multiplied by a different constant in each iteration
    // except first, when the IV is zero.

    ScaledCost += (OuterTripCnt - 1);

    // The first iteration when IV is zero can still be simplified.
    ++ScaledSavings;

    // Multiplication of unrolled IV and constant coefficient can be folded.
    if (Coeff != 1) {
      ScaledSavings += OuterTripCnt;
    }
  }
}

///// ProfitabilityAnalyzer Visitor End

bool HIRCompleteUnroll::runOnFunction(Function &F) {
  // Skip if DisableHIRCompleteUnroll is enabled
  if (DisableHIRCompleteUnroll || skipFunction(F)) {
    DEBUG(dbgs() << "HIR LOOP Complete Unroll Transformation Disabled \n");
    return false;
  }

  DEBUG(dbgs() << "Complete unrolling for Function : " << F.getName() << "\n");

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
    if (performTripCountAnalysis(Lp).first >= 0) {
      CandidateLoops.push_back(Lp);
    }
  }

  refineCandidates();

  transformLoops();
}

void HIRCompleteUnroll::refineCandidates() {

  for (unsigned Index = 0; Index != CandidateLoops.size();) {
    HLLoop *OuterCandidateLoop = CandidateLoops[Index];

    // If this loop is either not a top level candidate or is not profitable,
    // we remove it as a candidate and add its children as candidates instead.
    if (TopLevelCandidates.count(OuterCandidateLoop) &&
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

  auto &LS = HLS->getSelfLoopStatistics(Loop);

  // Cannot unroll loop if it has calls with noduplicate attribute.
  if (LS.hasCallsWithNoDuplicate()) {
    return false;
  }

  return true;
}

std::pair<int64_t, unsigned>
HIRCompleteUnroll::computeAvgTripCount(const HLLoop *Loop) {

  auto UpperCE = Loop->getUpperCanonExpr();
  unsigned LoopLevel = Loop->getNestingLevel();
  unsigned DepLevel = LoopLevel;

  if (UpperCE->hasBlob() || UpperCE->hasIVBlobCoeffs() ||
      (UpperCE->getDenominator() != 1)) {
    return std::make_pair(-1, DepLevel);
  }

  int64_t UpperVal = 0;

  if (UpperCE->isIntConstant(&UpperVal)) {
    int64_t TC = UpperVal + 1;
    if (TC >= Limits.LoopTripThreshold) {
      TC = -1;
    }

    return std::make_pair(TC, DepLevel);
  }

  // If triangular loop is disabled, we simply return high trip count,
  // to avoid unrolling triangular loops.
  if (DisableHIRTriCompleteUnroll) {
    return std::make_pair(-1, DepLevel);
  }

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

    if (UpperCE->getIVConstCoeff(Level)) {
      if (!CanUnrollParents) {
        return std::make_pair(-1, DepLevel);
      }

      // Set dependence level to the level of the outermost loop which has a
      // IV in UB.
      DepLevel = Level;
    }

    ParLoop = ParLoop->getParentLoop();
  }

  int64_t MinUpper = 0, MaxUpper = 0, AvgTripCnt = 0;

  // If we reached here, we should be able to compute the min/max trip count
  // of this loop.
  bool HasMin = HLNodeUtils::getExactMinValue(UpperCE, Loop, MinUpper);
  (void)HasMin;
  assert(HasMin && "Could not compute min value of upper!");

  // MinUpper can evaluate to a negative value. For purposes of calculating
  // average trip count for profitability analysis, we take the absolute
  // value.
  MinUpper = std::llabs(MinUpper);

  bool HasMax = HLNodeUtils::getExactMaxValue(UpperCE, Loop, MaxUpper);
  (void)HasMax;
  assert(HasMax && "Could not compute max value of upper!");

  // Loop never executes.
  if (MaxUpper < 0) {
    AvgTripCnt = 0;
  } else {
    AvgTripCnt = ((MinUpper + MaxUpper) / 2) + 1;
  }

  if (AvgTripCnt > Limits.LoopTripThreshold) {
    AvgTripCnt = -1;
  }

  return std::make_pair(AvgTripCnt, DepLevel);
}

std::pair<int64_t, unsigned>
HIRCompleteUnroll::performTripCountAnalysis(HLLoop *Loop) {
  SmallVector<HLLoop *, 8> CandidateChildLoops;

  int64_t AvgTripCnt = -1, TotalTripCnt = -1;
  int64_t MaxChildTripCnt = 1;
  unsigned LoopLevel = Loop->getNestingLevel();
  unsigned MinDepLevel = LoopLevel;

  bool IsLoopCandidate = isApplicable(Loop);

  if (IsLoopCandidate) {
    std::tie(AvgTripCnt, MinDepLevel) = computeAvgTripCount(Loop);

    if (AvgTripCnt >= 0) {
      AvgTripCount.insert(std::make_pair(Loop, AvgTripCnt));
    } else {
      IsLoopCandidate = false;
    }
  }

  if (!Loop->isInnermost()) {
    SmallVector<HLLoop *, 8> ChildLoops;
    Loop->getHLNodeUtils().gatherLoopsWithLevel(Loop, ChildLoops,
                                                LoopLevel + 1);

    for (auto &ChildLp : ChildLoops) {
      int64_t ChildTripCnt;
      unsigned ChildDepLevel;
      std::tie(ChildTripCnt, ChildDepLevel) = performTripCountAnalysis(ChildLp);

      if (ChildTripCnt >= 0) {
        CandidateChildLoops.push_back(ChildLp);

        MaxChildTripCnt = std::max(MaxChildTripCnt, ChildTripCnt);
        MinDepLevel = std::min(MinDepLevel, ChildDepLevel);

      } else {
        IsLoopCandidate = false;
      }
    }
  }

  if (IsLoopCandidate) {
    TotalTripCnt = AvgTripCnt * MaxChildTripCnt;
    IsLoopCandidate = (TotalTripCnt <= (Limits.LoopnestTripThreshold *
                                        Limits.MaxThresholdScalingFactor));
  }

  if (IsLoopCandidate) {
    TotalTripCount.insert(std::make_pair(Loop, TotalTripCnt));

    // Loop is not dependent on any outer loop so we add it as a top level
    // candidate.
    if (MinDepLevel == LoopLevel) {
      TopLevelCandidates.insert(Loop);
    }
  } else {
    TotalTripCnt = -1;
    // If current loop is not a candidate, store the children loops for
    // transformation.
    CandidateLoops.append(CandidateChildLoops.begin(),
                          CandidateChildLoops.end());
  }

  return std::make_pair(TotalTripCnt, MinDepLevel);
}

bool HIRCompleteUnroll::isProfitable(const HLLoop *Loop) {
  DenseMap<unsigned, unsigned> SimplifiedTempBlobs;
  MemRefGatherer::MapTy MemRefMap;
  SmallPtrSet<const Value *, 16> AllocaStoreBases;

  ProfitabilityAnalyzer PA(*this, Loop, SimplifiedTempBlobs, MemRefMap,
                           AllocaStoreBases);

  PA.analyze();

  if (PA.isProfitable()) {
    // Copy captured simplifiable alloca stores.
    for (auto BaseVal : AllocaStoreBases) {
      UnrolledAllocaStoreBases.insert(BaseVal);
    }

    return true;
  }

  return false;
}

// Transform (Complete Unroll) each loop inside the CandidateLoops vector
void HIRCompleteUnroll::transformLoops() {
  SmallVector<int64_t, MaxLoopNestLevel> IVValues;

  LoopnestsCompletelyUnrolled += CandidateLoops.size();

  // Transform the loop nest from outer to inner.
  for (auto &Loop : CandidateLoops) {
    auto &LS = HLS->getTotalLoopStatistics(Loop);
    bool HasIfsOrSwitches = LS.hasIfs() || LS.hasSwitches();

    auto Reg = Loop->getParentRegion();
    HLNode *ParentNode = Loop->getParentLoop();
    if (!ParentNode) {
      ParentNode = Reg;
    }

    // Generate code for the parent region and invalidate parent
    Reg->setGenCode();
    HIRInvalidationUtils::invalidateParentLoopBodyOrRegion(Loop);

    CanonExprUpdater CEUpdater(Loop->getNestingLevel(), IVValues);
    transformLoop(Loop, CEUpdater, true);

    assert(IVValues.empty() && "IV values were not cleaned up!");

    if (HasIfsOrSwitches) {
      HLNodeUtils::removeRedundantNodes(ParentNode);
    }
  }
}

int64_t HIRCompleteUnroll::computeUB(HLLoop *Loop, unsigned TopLoopLevel,
                                     SmallVectorImpl<int64_t> &IVValues) {
  int64_t UBVal = 0;

  const CanonExpr *UBCE = Loop->getUpperCanonExpr();
  if (UBCE->isIntConstant(&UBVal)) {
    return UBVal;
  }

  assert(!UBCE->hasBlob() && !UBCE->hasIVBlobCoeffs() &&
         (UBCE->getDenominator() == 1) &&
         "Blobs or non-unit denominator in loop upper not expected!");

  UBVal = UBCE->getConstant();

  auto LoopLevel = TopLoopLevel;

  for (auto Val : IVValues) {
    UBVal += (Val * UBCE->getIVConstCoeff(LoopLevel));
    LoopLevel++;
  }

  return UBVal;
}

// Complete Unroll the given Loop, using provided LD as helper data
void HIRCompleteUnroll::transformLoop(HLLoop *Loop, CanonExprUpdater &CEUpdater,
                                      bool IsTopLevelLoop) {

  // Guard against the scanning phase setting it appropriately.
  assert(Loop && " Loop is null.");

  // Container for cloning body.
  HLContainerTy LoopBody;
  HLNodeUtils &HNU = Loop->getHLNodeUtils();

  auto &IVValues = CEUpdater.IVValues;

  int64_t LB = Loop->getLowerCanonExpr()->getConstant();
  int64_t UB = computeUB(Loop, CEUpdater.TopLoopLevel, IVValues);
  int64_t Step = Loop->getStrideCanonExpr()->getConstant();
  bool ZttHasBlob = false;

  // At this point loop preheader has been visited already but postexit is
  // not, so we need to handle postexit explicitly.

  if (UB < 0) {
    // We remove postexit so that visitor doesn't visit disconnected nodes.
    Loop->removePostexit();
    HNU.remove(Loop);
    return;
  }

  // Check if ZTT has blob. If so, we have to hoist it.
  for (auto DDIt = Loop->ztt_ddref_begin(), E = Loop->ztt_ddref_end();
       DDIt != E; ++DDIt) {
    if ((*DDIt)->hasBlobDDRefs() || (*DDIt)->isSelfBlob()) {
      ZttHasBlob = true;
      break;
    }
  }

  if (ZttHasBlob) {
    HNU.visit<false>(CEUpdater, Loop->extractZtt(CEUpdater.TopLoopLevel + 1));
  } else {
    Loop->removeZtt();
  }

  HLNode *Marker = nullptr;

  if (!IsTopLevelLoop) {
    HNU.visitRange(CEUpdater, Loop->post_begin(), Loop->post_end());
  }

  Loop->extractPreheaderAndPostexit();

  if (IsTopLevelLoop) {
    Marker = HNU.getOrCreateMarkerNode();
    // Replace top level loop of the unroll loopnest with marker node to avoid
    // top sort num recomputation.
    HNU.replace(Loop, Marker);
  }

  auto OrigFirstChild = Loop->getFirstChild();
  auto OrigLastChild = Loop->getLastChild();

  IVValues.push_back(LB);

  // Iterate over Loop Child for unrolling with trip value incremented
  // each time. Thus, loop body will be expanded by no. of stmts x TripCount.
  for (int64_t IVVal = LB; IVVal < UB; IVVal += Step) {
    // Clone iteration
    HNU.cloneSequence(&LoopBody, OrigFirstChild, OrigLastChild);

    // Store references as LoopBody will be empty after insertion.
    HLNode *CurFirstChild = &(LoopBody.front());
    HLNode *CurLastChild = &(LoopBody.back());

    HNU.insertBefore(OrigFirstChild, &LoopBody);

    // Update IV value of loop for the current unrolled iteration for
    // substitution inside the canon expr.
    IVValues.back() = IVVal;

    // Update the CanonExpr
    HNU.visitRange<true, false>(CEUpdater, CurFirstChild, CurLastChild);
  }

  // Reuse original children for last iteration.
  IVValues.back() = UB;
  HNU.visitRange<true, false>(CEUpdater, OrigFirstChild, OrigLastChild);

  IVValues.pop_back();

  if (IsTopLevelLoop) {
    // Replace marker node with the unrolled loop children.
    HNU.moveBefore(Marker, Loop->child_begin(), Loop->child_end());
    HNU.remove(Marker);
  } else {
    HNU.moveBefore(Loop, Loop->child_begin(), Loop->child_end());
    HNU.remove(Loop);
  }
}

void HIRCompleteUnroll::releaseMemory() {
  CandidateLoops.clear();
  AvgTripCount.clear();
  TopLevelCandidates.clear();
  UnrolledAllocaStoreBases.clear();
}
