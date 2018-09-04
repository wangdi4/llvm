//===- HIRCompleteUnroll.cpp - Implements CompleteUnroll class ------------===//
//
// Copyright (C) 2015-2018 Intel Corporation. All rights reserved.
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

#include "llvm/ADT/SmallSet.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"

#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRLoopStatistics.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRSafeReductionAnalysis.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Framework/HIRFramework.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/BlobUtils.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/CanonExprUtils.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/DDRefUtils.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HIRInvalidationUtils.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HLNodeUtils.h"
#include "llvm/Analysis/TargetTransformInfo.h"

#include "llvm/Transforms/Intel_LoopTransforms/Utils/HIRTransformUtils.h"

#include "HIRCompleteUnrollImpl.h"

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

// External interface
namespace llvm {
namespace loopopt {
namespace unroll {

void completeUnrollLoop(HLLoop *Loop) { HIRCompleteUnroll::doUnroll(Loop); }

} // namespace unroll
} // namespace loopopt
} // namespace llvm

HIRCompleteUnroll::HIRCompleteUnroll(HIRFramework &HIRF, DominatorTree &DT,
                                     const TargetTransformInfo &TTI,
                                     HIRLoopStatistics &HLS, HIRDDAnalysis &DDA,
                                     HIRSafeReductionAnalysis &HSRA,
                                     unsigned OptLevel, bool IsPreVec)
    : HIRF(HIRF), DT(DT), TTI(TTI), HLS(HLS), DDA(DDA), HSRA(HSRA),
      IsPreVec(IsPreVec) {

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

/// Visitor to update the CanonExpr.
struct HIRCompleteUnroll::CanonExprUpdater final : public HLNodeVisitorBase {
  const unsigned TopLoopLevel;
  // Contains values of IVs at each loop level for the current unrolled
  // iteration. Value of -1 represents a loop which isn't being unrolled.
  SmallVectorImpl<int64_t> &IVValues;
  const bool IsPragmaEnabledUnrolling;

  CanonExprUpdater(unsigned TopLoopLevel, SmallVectorImpl<int64_t> &IVValues,
                   bool IsPragma)
      : TopLoopLevel(TopLoopLevel), IVValues(IVValues),
        IsPragmaEnabledUnrolling(IsPragma) {}

  void processRegDDRef(RegDDRef *RegDD);
  void processCanonExpr(CanonExpr *CExpr);

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

// Structure to hold info of blobs simplified to constant by unrolling.
class SimplifiedTempBlob {
  unsigned Index;
  // Level of the simplified definition of the blob
  unsigned DefLevel;
  const HLInst *DefInst;
  // If blob definition was simplified using a rem (%) operation, we store the
  // constant factor here.
  unsigned RemFactor;

private:
  void initBlobFactor();

public:
  SimplifiedTempBlob(unsigned Index, unsigned DefLevel, const HLInst *DefInst)
      : Index(Index), DefLevel(DefLevel), DefInst(DefInst), RemFactor(0) {
    initBlobFactor();
  }

  // Resets info for existing blob.
  void reset(unsigned Level, const HLInst *HInst) {
    DefInst = HInst;
    DefLevel = Level;
    initBlobFactor();
  }

  unsigned getIndex() const { return Index; }
  unsigned getDefLevel() const { return DefLevel; }
  unsigned getRemFactor() const { return RemFactor; }

  const HLInst *getDefInst() const { return DefInst; }
};

void SimplifiedTempBlob::initBlobFactor() {
  auto Inst = DefInst->getLLVMInstruction();

  // Looking for something like this -
  // tmp = i1 % 4;

  auto OpCode = Inst->getOpcode();

  if ((OpCode != Instruction::URem) && (OpCode != Instruction::SRem)) {
    return;
  }

  auto RvalOp2 = DefInst->getOperandDDRef(2);

  int64_t Factor;

  if (!RvalOp2->isIntConstant(&Factor) || (Factor < 0) || (Factor > UINT_MAX)) {
    return;
  }

  RemFactor = Factor;
}

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

  // Structure to store CanonExpr related info.
  struct CanonExprInfo {
    unsigned NumSimplifiedTerms = 0;
    unsigned NumNonLinearTerms = 0;
    unsigned NumUnrollableIVBlobs = 0;
    bool HasUnrollableStandAloneIV = false;
  };

  class InvalidAllocaRefFinder;

  HIRCompleteUnroll &HCU;
  const HLLoop *CurLoop;
  const HLLoop *OuterLoop;

  const unsigned CurLevel;
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
  SmallVector<SimplifiedTempBlob, 8> &SimplifiedTempBlobs;

  // Keep track of invariant GEP refs that have been visited to avoid
  // duplicating savings.
  SmallVector<VisitedRefInfo, 16> VisitedGEPRefs;

  // Contains mem refs of parent of OuterLoop if it exists, else contains mem
  // refs of OuterLoop.
  HIRCompleteUnroll::MemRefGatherer::MapTy &OuterLoopMemRefMap;

  // Set of simplifiable alloca stores discovered in this loopnest.
  DenseMap<unsigned, const RegDDRef *> &AllocaStores;

  // Map of alloca stores and the profitability to propagate them.
  SmallDenseMap<unsigned, bool, 8> ProfitableAllocaStores;

  // Set of non-loop parent nodes which can be simplified by unrolling the
  // current loopnest.
  SmallPtrSet<const HLNode *, 8> &SimplifiedNonLoopParents;

  // Private constructor used for children loops.
  ProfitabilityAnalyzer(
      HIRCompleteUnroll &HCU, const HLLoop *CurLp, const HLLoop *OuterLp,
      unsigned ParentLoopNestTripCount,
      SmallVector<SimplifiedTempBlob, 8> &SimplifiedBlobs,
      HIRCompleteUnroll::MemRefGatherer::MapTy &MemRefMap,
      DenseMap<unsigned, const RegDDRef *> &AllocaStores,
      SmallPtrSet<const HLNode *, 8> &SimplifiedNonLoopParents)
      : HCU(HCU), CurLoop(CurLp), OuterLoop(OuterLp),
        CurLevel(CurLp->getNestingLevel()), Cost(0), ScaledCost(0), Savings(0),
        ScaledSavings(0), GEPCost(0), GEPSavings(0), NumMemRefs(0),
        NumDDRefs(0), SimplifiedTempBlobs(SimplifiedBlobs),
        OuterLoopMemRefMap(MemRefMap), AllocaStores(AllocaStores),
        SimplifiedNonLoopParents(SimplifiedNonLoopParents) {
    auto Iter = HCU.AvgTripCount.find(CurLp);
    assert((Iter != HCU.AvgTripCount.end()) && "Trip count of loop not found!");
    LoopNestTripCount = (ParentLoopNestTripCount * Iter->second);
  }

  /// Inserts a simplified temp blob with \p Index. It overwrite the previous
  /// entry for the same blob.
  void insertSimplifiedTempBlob(unsigned Index, const HLInst *DefInst);

  /// Removes the ennry for the blob with \p Index;
  void removeSimplifiedTempBlob(unsigned Index);

  /// Returns true if a simplified blob exists with \p Index. \p
  /// CurNodeBlobLevel indicates the blob level of the blob in \p CurNode.
  /// Populates factor of simplified blob in \p Factor.
  bool isSimplifiedTempBlob(unsigned Index, unsigned CurNodeBlobLevel,
                            const HLDDNode *CurNode,
                            unsigned *Factor = nullptr) const;

  /// level of any non-rem blob.
  unsigned populateRemBlobs(
      const RegDDRef *Ref,
      SmallVectorImpl<std::pair<unsigned, unsigned>> &RemBlobs) const;

  /// Returns max level of any non-simplified blob in Ref. Sets \p
  /// HasNonSimplifiedBlob if Ref contains a non-simplified blob (excluding base
  /// ptr). This flag is separate because the caller needs to know this even if
  /// the blob is defined at level 0.
  unsigned getMaxNonSimplifiedBlobLevel(const RegDDRef *Ref,
                                        bool &HasNonSimplifiedBlob) const;

  /// Returns true if unique occurences of \p Ref1 in \p Loop are refined based
  /// on locality analysis w.r.t \p Ref2. Refined occurences are set in \p
  /// RefinedRef1Occurences.
  bool refinedOccurencesUsingLocalityAnalysis(
      const RegDDRef *Ref1, const RegDDRef *Ref2, bool Ref1IsUnconditional,
      const HLLoop *Loop, unsigned &RefinedRef1Occurences) const;

  /// Returns true if Ref is in a sibling candidate loop of OuterLoop.
  bool isInSiblingCandidateLoop(const RegDDRef *Ref) const;

  /// Returns true if \p Ref has no data dependency in \p Loop. Returns refined
  /// occurences of Ref in \p RefinedOccurences.
  bool isDDIndependentInLoop(const RegDDRef *Ref, const HLLoop *Loop,
                             unsigned &RefinedOccurences) const;

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
  /// to true to indicate that \p Ref should be added to visited set. Sets \p
  /// SkipAddressSimplification to true to indicate that address simplification
  /// for the Ref should be ignored.
  bool visitedGEPRef(const RegDDRef *Ref, bool &AddToVisitedSet,
                     bool &SkipAddressSimplification);

  /// Processes a GEP DDRef for profitability. Returns true if Ref can be
  /// simplified to a constant.
  bool processGEPRef(const RegDDRef *Ref);

  /// Processes RegDDRef for profitability. Returns true if Ref can be
  /// simplified to a constant.
  bool processRef(const RegDDRef *Ref);

  /// Processes CanonExpr for profitability. Returns true if CE can be
  /// simplified to a constant.
  bool processCanonExpr(const CanonExpr *CE, const RegDDRef *ParentRef);

  /// Processes IVs in the CE. Returns true if they can be simplified to a
  /// constant.
  bool processIVs(const CanonExpr *CE, const RegDDRef *ParentRef,
                  CanonExprInfo &CEInfo);

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

  /// Updates all the visited blobs which contain the temp represented by self
  /// blob \p Ref. \p Simplified indicates whether the blob definition was
  /// simplified to a constant.
  void updateBlobs(const RegDDRef *Ref, bool Simplified);

  /// Returns percentage savings achieved by unrolling the loopnest.
  float getSavingsInPercentage() const;

  /// Returns true if this loop should be unrolled before vectorizer. This is a
  /// temporary workaround.
  bool isPreVectorProfitableLoop(const HLLoop *CurLoop) const;

  /// Returns true if we find a simplified alloca store with base ptr blob index
  /// \p BaseIndex which dominates \p AllocaLoadRef in a previous loopnest.
  bool foundSimplifiedDominatingStoreInPreviousLoopnest(
      const RegDDRef *AllocaLoadRef, unsigned BaseIndex);

  /// Returns true if we find a simplified alloca store with base ptr blob index
  /// \p BaseIndex which dominates \p AllocaLoadRef in current loopnest.
  bool foundSimplifiedDominatingStore(const RegDDRef *AllocaLoadRef,
                                      unsigned BaseIndex);

  /// Returns true if it should be considered profitable to propagate this
  /// alloca store.
  bool profitableToPropagateAllocaStore(const RegDDRef *AllocaStore,
                                        unsigned BaseIndex);

  /// Returns true if simplifiable \p MemRef can be optimized away. \p
  /// CanSimplifySubs indicates whether all its subscripts can be simplified to
  /// constant.
  bool canEliminate(const RegDDRef *MemRef, bool CanSimplifySubs);

  /// Scales the profitability by the given multiplier.
  void scale(unsigned Multiplier) {
    Cost *= Multiplier;
    Savings *= Multiplier;
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
  ProfitabilityAnalyzer(
      HIRCompleteUnroll &HCU, const HLLoop *CurLp,
      SmallVector<SimplifiedTempBlob, 8> &SimplifiedTempBlobs,
      HIRCompleteUnroll::MemRefGatherer::MapTy &MemRefMap,
      DenseMap<unsigned, const RegDDRef *> &AllocaStores,
      SmallPtrSet<const HLNode *, 8> &SimplifiedNonLoopParents)
      : ProfitabilityAnalyzer(HCU, CurLp, CurLp, 1, SimplifiedTempBlobs,
                              MemRefMap, AllocaStores,
                              SimplifiedNonLoopParents) {
    if (auto OuterLp = CurLp->getParentLoop()) {
      MemRefGatherer::gatherRange(OuterLp->child_begin(), OuterLp->child_end(),
                                  MemRefMap);
    } else {
      MemRefGatherer::gatherRange(CurLp->child_begin(), CurLp->child_end(),
                                  MemRefMap);
    }
  }

  /// Returns true if Ref is executed unconditionally in \p ParentLoop.
  bool isUnconditionallyExecuted(const RegDDRef *Ref,
                                 const HLNode *ParentLoop) const;

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

  // Process CanonExprs inside the RegDDRefs
  for (auto Iter = RegDD->canon_begin(), End = RegDD->canon_end(); Iter != End;
       ++Iter) {
    processCanonExpr(*Iter);
  }

  if (!IsPragmaEnabledUnrolling) {
    RegDD->makeConsistent(nullptr, TopLoopLevel - 1);
  } else {
    // Need to account for non-unrollable loops which can be encountered when
    // doing pragma based unrolling. For example-
    // #pragma unroll full
    // DO i1 = 0, 4
    //  DO i2 = 0, N
    //    // Final level of nodes here wil be 1.
    //  END DO
    // END DO
    //
    unsigned NonUnrollableLevels = 0;

    for (auto IVVal : IVValues) {
      if (IVVal == -1) {
        ++NonUnrollableLevels;
      }
    }

    RegDD->makeConsistent(nullptr, TopLoopLevel - 1 + NonUnrollableLevels);
  }
}

void HIRCompleteUnroll::CanonExprUpdater::processCanonExpr(CanonExpr *CExpr) {

  // Start replacing the IV's from TopLoopLevel to current loop level.
  auto LoopLevel = TopLoopLevel;
  unsigned UnrollableLevels = 0;

  for (auto IVVal : IVValues) {
    if (IVVal != -1) {
      ++UnrollableLevels;
      CExpr->replaceIVByConstant(LoopLevel, IVVal);
    } else {
      // Loop at this level is not being unrolled but its nesting level will be
      // reduced so we need to replace the corresponding IV with a lower leveled
      // IV. For example- #pragma unroll full DO i1 = 0, 4
      //   DO i2 = 0, N
      //     A[i2] = t;  << This will become A[i1] after unrolling.
      unsigned Index;
      int64_t Coeff;

      CExpr->getIVCoeff(LoopLevel, &Index, &Coeff);

      if (Coeff != 0) {
        CExpr->removeIV(LoopLevel);
        CExpr->setIVCoeff(LoopLevel - UnrollableLevels, Index, Coeff);
      }
    }
    ++LoopLevel;
  }

  CExpr->simplify(true);
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

  if (HCU.IsPreVec && CurLoop->isInnermost() && CurLoop->isDo()) {
    // compute safe reduction chain for innermost do loops if we are executing
    // before vectorizer. This is to add extra cost to the loops containing
    // reductions.
    HCU.HSRA.computeSafeReductionChains(CurLoop);
  }

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
         // NumMemRefs/NumDDRefs are computed in a scaled manner in
         // computeGEPInfo() so we need to divide it by the trip count to get
         // original count.
         ((NumMemRefs / TripCount) <= HCU.Limits.SmallLoopMemRefThreshold) &&
         ((NumDDRefs / TripCount) <= HCU.Limits.SmallLoopDDRefThreshold);
}

float HIRCompleteUnroll::ProfitabilityAnalyzer::getSavingsInPercentage() const {
  auto TotalCost = Cost + ScaledCost + GEPCost;

  float SafeCost = (TotalCost == 0) ? 1 : TotalCost;
  return ((Savings + ScaledSavings + GEPSavings) * 100) / SafeCost;
}

bool HIRCompleteUnroll::ProfitabilityAnalyzer::isProfitable() const {
  assert((CurLoop == OuterLoop) &&
         "isProfitable() should only be called for top level loop!");

  auto SavingsPercentage = getSavingsInPercentage();

  LLVM_DEBUG(dbgs() << "Cost: " << Cost << "\n");
  LLVM_DEBUG(dbgs() << "ScaledCost: " << ScaledCost << "\n");
  LLVM_DEBUG(dbgs() << "GEPCost: " << GEPCost << "\n");
  LLVM_DEBUG(dbgs() << "Savings: " << Savings << "\n");
  LLVM_DEBUG(dbgs() << "ScaledSavings: " << ScaledSavings << "\n");
  LLVM_DEBUG(dbgs() << "GEPSavings: " << GEPSavings << "\n");

  LLVM_DEBUG(dbgs() << "Savings in percentage: " << SavingsPercentage << "\n");

  LLVM_DEBUG(dbgs() << "Number of memrefs: " << NumMemRefs << "\n");
  LLVM_DEBUG(dbgs() << "Number of ddrefs: " << NumDDRefs << "\n");
  LLVM_DEBUG(dbgs() << "Loop: \n"; CurLoop->dump(); dbgs() << "\n");

  if (SavingsPercentage < HCU.Limits.SavingsThreshold) {
    return false;
  }

  // Use postvec(smaller) savings threshold to derive consistent scaling
  // factor for prevec and postvec passes.
  float ScalingFactor = (SavingsPercentage / PostVectorSavingsThreshold);
  ScalingFactor = std::min(ScalingFactor, HCU.Limits.MaxThresholdScalingFactor);

  auto Iter = HCU.TotalTripCount.find(CurLoop);
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
                           AllocaStores, SimplifiedNonLoopParents);
  PA.analyze();

  // Add the result of child loop profitability analysis.
  *this += PA;
}

void HIRCompleteUnroll::ProfitabilityAnalyzer::insertSimplifiedTempBlob(
    unsigned Index, const HLInst *DefInst) {

  for (auto &Blob : SimplifiedTempBlobs) {
    if (Blob.getIndex() == Index) {
      Blob.reset(CurLevel, DefInst);
      return;
    }
  }

  SimplifiedTempBlobs.emplace_back(Index, CurLevel, DefInst);
}

void HIRCompleteUnroll::ProfitabilityAnalyzer::removeSimplifiedTempBlob(
    unsigned Index) {
  for (auto It = SimplifiedTempBlobs.begin(), E = SimplifiedTempBlobs.end();
       It != E; ++It) {
    if (It->getIndex() == Index) {
      SimplifiedTempBlobs.erase(It);
      return;
    }
  }
}

bool HIRCompleteUnroll::ProfitabilityAnalyzer::isSimplifiedTempBlob(
    unsigned Index, unsigned CurNodeBlobLevel, const HLDDNode *CurNode,
    unsigned *Factor) const {
  // Blob is considered to be simplified if the following two conditions hold
  // true-
  //
  // 1) The simplified definition was found at current or deeper level than
  // specified in the current node.
  //
  // In the example below 't' should not be considered simplified at the inner
  // level as it is redefined.
  //
  // DO i1
  //   t = 0;
  //   DO i2
  //     t = t + A[i1]
  //   END DO
  // END DO
  //
  // 2) Simplified definition dominates current node.
  //
  // In the example below 't' should not be considered simplified as 't = 50'
  // doesn't dominate the use.
  //
  // DO i1
  //   t = 0;
  //   if () {
  //     ...
  //     t = 50;
  //   }
  //   t1 = t;
  // END DO
  for (auto &Blob : SimplifiedTempBlobs) {
    if ((Blob.getIndex() == Index) &&
        (Blob.getDefLevel() >= CurNodeBlobLevel) &&
        HLNodeUtils::dominates(Blob.getDefInst(), CurNode)) {
      if (Factor) {
        *Factor = Blob.getRemFactor();
      }
      return true;
    }
  }

  return false;
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
    insertSimplifiedTempBlob(TempIndex, cast<HLInst>(LvalRef->getHLDDNode()));
  } else {
    removeSimplifiedTempBlob(TempIndex);
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
  bool HasNonConstRval = false;

  for (; RefIt != End; ++RefIt, ++NumRvalOp) {
    RvalRef = *RefIt;
    // Only the first two rval operands of select are relevant for
    // simplification. t = (t1 < t2) ? t3 : t4
    bool IsSelectOperand = IsSelect && (NumRvalOp > 1);

    if (IsSelectOperand && (HInst->isAbs() || HInst->isMinOrMax())) {
      // Do not analyze last two operands of 'idiomatic' select.
      break;
    }

    bool SimplificationCandidate = !IsSelectOperand;

    if (SimplificationCandidate) {
      HasNonConstRval = HasNonConstRval || !RvalRef->isConstant();
    }

    if (!processRef(RvalRef)) {
      if (SimplificationCandidate) {
        CanSimplifyRvals = false;
      }
    }
  }

  if (CanSimplifyRvals) {
    // Only add savings if RHS was not already a constant.
    // RHS can turn constant after pre vec complete unroll. Since we do not
    // perform constant propagation/DCE after complete unroll we may mistakenly
    // identify it as savings in post vec complete unroll.
    if (HasNonConstRval) {
      // Add extra savings for ifs/switches.
      Savings += HInst ? 1 : 2;
    }

    if (!HInst) {
      SimplifiedNonLoopParents.insert(Node);
    }
  }

  if (HInst && (LvalRef = HInst->getLvalDDRef())) {
    bool AlreadySimplified = false;

    if (LvalRef->isTerminalRef()) {
      // If rvals can be simplified, consider terminal lval as also simplified.
      CanSimplifyLval = CanSimplifyRvals;
      AlreadySimplified = !HasNonConstRval;

      // Terminal lval refs are used to invalidate their encountered uses.
      updateBlobs(LvalRef, CanSimplifyRvals);

      if (!CanSimplifyLval) {
        NumDDRefs += LoopNestTripCount;
      }
    } else {
      CanSimplifyLval = processRef(LvalRef);
    }

    if (CanSimplifyLval && !AlreadySimplified) {
      ++Savings;
    }

    // Cost of load/store/gep/copy has already been accounted for in refs.
    if (isa<LoadInst>(Inst) || isa<StoreInst>(Inst) ||
        isa<GetElementPtrInst>(Inst) || HInst->isCopyInst()) {
      return;
    }
  }

  if (!CanSimplifyRvals || !CanSimplifyLval) {

    if (HInst) {
      // Add extra cost if instruction is a non-simplifiable reduction and we
      // are executing before vectorizer. We should prefer vectorizing
      // reductions rather than unrolling them.
      Cost +=
          (HCU.IsPreVec && LvalRef && HCU.HSRA.isSafeReduction(HInst)) ? 2 : 1;
    } else {
      // Add extra cost for ifs/switches.
      Cost += 2;
    }
  }
}

unsigned HIRCompleteUnroll::ProfitabilityAnalyzer::populateRemBlobs(
    const RegDDRef *Ref,
    SmallVectorImpl<std::pair<unsigned, unsigned>> &RemBlobs) const {
  assert(Ref->hasGEPInfo() && "GEP ref expected!");

  unsigned MaxNonRemBlobLevel = 0;
  auto CurNode = Ref->getHLDDNode();

  for (auto BIt = Ref->blob_cbegin(), End = Ref->blob_cend(); BIt != End;
       ++BIt) {
    auto Blob = *BIt;
    auto Index = Blob->getBlobIndex();
    unsigned BlobLevel =
        Blob->isNonLinear() ? CurLevel : Blob->getDefinedAtLevel();
    unsigned Factor;

    if (isSimplifiedTempBlob(Index, BlobLevel, CurNode, &Factor) && Factor) {
      RemBlobs.push_back(std::make_pair(BlobLevel, Factor));
    } else {
      MaxNonRemBlobLevel = std::max(MaxNonRemBlobLevel, BlobLevel);
    }
  }

  return MaxNonRemBlobLevel;
}

unsigned HIRCompleteUnroll::ProfitabilityAnalyzer::getMaxNonSimplifiedBlobLevel(
    const RegDDRef *Ref, bool &HasNonSimplifiedBlob) const {
  assert(Ref->hasGEPInfo() && "GEP ref expected!");

  unsigned MaxNonSimplifiedBlobLevel = 0;
  auto CurNode = Ref->getHLDDNode();

  unsigned BasePtrIndex = Ref->getBasePtrBlobIndex();

  for (auto BIt = Ref->blob_cbegin(), End = Ref->blob_cend(); BIt != End;
       ++BIt) {
    auto Blob = *BIt;
    auto Index = Blob->getBlobIndex();
    unsigned BlobLevel =
        Blob->isNonLinear() ? CurLevel : Blob->getDefinedAtLevel();

    if (!isSimplifiedTempBlob(Index, BlobLevel, CurNode)) {

      if (Index != BasePtrIndex) {
        HasNonSimplifiedBlob = true;
      }

      MaxNonSimplifiedBlobLevel =
          std::max(MaxNonSimplifiedBlobLevel, BlobLevel);
    }
  }

  return MaxNonSimplifiedBlobLevel;
}

bool HIRCompleteUnroll::ProfitabilityAnalyzer::isUnconditionallyExecuted(
    const RegDDRef *Ref, const HLNode *ParentLoop) const {
  auto *OuterParent = ParentLoop->getParent();
  auto *ParentNode = Ref->getHLDDNode()->getParent();

  while (ParentNode != OuterParent) {
    auto ParLoop = dyn_cast<HLLoop>(ParentNode);

    if (ParLoop) {
      // Ref is not unconditional in unrolled multi-exit loop.
      // TODO: This can be refined based on whether the early-exit jumps are
      // within ParentLoop (when ParLoop is a child loop of ParentLoop).
      if (ParLoop->getNumExits() > 1) {
        return false;
      }
    } else if (!SimplifiedNonLoopParents.count(ParentNode)) {
      // Marking refs unconditional based on simplified parents is
      // an optimistic assumption because simplified parent may mean that
      // parent's body (this ref) gets optimized away but it is hard to
      // check this accurately as we need redundant node logic.
      // Nevertheless, it seems better to make this assumption as we see
      // some performance regressions without it.
      return false;
    }

    ParentNode = ParentNode->getParent();
  }

  return true;
}

bool HIRCompleteUnroll::ProfitabilityAnalyzer::
    refinedOccurencesUsingLocalityAnalysis(
        const RegDDRef *Ref1, const RegDDRef *Ref2, bool Ref1IsUnconditional,
        const HLLoop *Loop, unsigned &RefinedRef1Occurences) const {
  // There is no reuse from Ref2 to Ref1 if Ref2 is conditional.
  if (!isUnconditionallyExecuted(Ref2, Loop)) {
    return false;
  }

  bool Ref1IsLval = Ref1->isLval();
  unsigned LoopLevel = Loop->getNestingLevel();

  if (Ref1IsLval) {
    // Redundant Ref1 store scenario:
    // A[i1-1] =  << Ref2
    // A[i1] =    << Ref1
    if (!Ref1IsUnconditional || !Ref2->isLval()) {
      return false;
    }
  } else {
    // Redundant Ref1 load scenario 1:
    //   = A[i1]    << Ref2
    // if ()
    //   = A[i1-1]  << Ref1
    //
    // Redundant Ref1 load scenario 2:
    // A[i1] =         << Ref2
    // if ()
    //       = A[i1-1] << Ref1
  }

  int64_t Dist;
  if (!DDRefUtils::getConstIterationDistance(Ref2, Ref1, LoopLevel, &Dist,
                                             true)) {
    return false;
  }

  assert(Dist != 0 && "Non-zero distance expected!");
  unsigned TripCount = HCU.AvgTripCount.find(Loop)->second;

  // Distance should be negative for (store -> store) case but it doesn't matter
  // as it will be accounted once per ref pair due to symmetry of checks.
  if ((Dist < 0) || (Dist >= TripCount)) {
    return true;
  }

  if (!RefinedRef1Occurences || (Dist < RefinedRef1Occurences)) {
    RefinedRef1Occurences = Dist;
  }

  return true;
}

bool HIRCompleteUnroll::ProfitabilityAnalyzer::isInSiblingCandidateLoop(
    const RegDDRef *Ref) const {

  auto *RefLoop = Ref->getParentLoop();
  auto *ParentLoop = OuterLoop->getParentLoop();

  // In PreVec pass give up if Ref is not directly contained in ParentLoop. This
  // is because the candidate loop check is an approximate check. The
  // profitability of sibling loops can be dependent on each other. Consider
  // this case-
  //
  // DO i1
  //   DO i2 = 0, 10
  //     A[i2] =
  //   END DO
  //
  //   DO i2 = 0, 10
  //     = A[i2]
  //   END DO
  // END DO
  //
  // Since both the i2 loops have constant trip count, both are added to
  // CandidateLoops after the trip count analysis phase. The profitability of
  // CandidateLoops is checked in lexical order. When we are evaluating the
  // profitability of first i2 loop, we do not know for sure whether the second
  // i2 loop will be unrolled. This function assumes that it will be unrolled
  // which is an optimistic assumption.
  // TODO: Investigate whether other optimistic assumptions should also be
  // guarded under IsPreVec check.
  if (HCU.IsPreVec && (RefLoop != ParentLoop)) {
    return false;
  }

  const HLLoop *OuterRefLoop = nullptr;
  while (RefLoop != ParentLoop) {
    OuterRefLoop = RefLoop;
    RefLoop = RefLoop->getParentLoop();
  }

  if (OuterRefLoop &&
      (std::find(HCU.CandidateLoops.begin(), HCU.CandidateLoops.end(),
                 OuterRefLoop) == HCU.CandidateLoops.end())) {
    return false;
  }

  return true;
}

bool HIRCompleteUnroll::ProfitabilityAnalyzer::isDDIndependentInLoop(
    const RegDDRef *Ref, const HLLoop *Loop,
    unsigned &RefinedOccurences) const {
  assert(Ref->isMemRef() && "Only mem ref is expected!");

  unsigned LoopLevel = Loop->getNestingLevel();
  bool IsOutermostLoop = (Loop == OuterLoop->getParentLoop());
  bool HasNonSimplifiedBlob = false;
  RefinedOccurences = 0;

  // OutermostLoop is the resulting loop after complete unroll so we also need
  // to check for structural invariance to conclude DD independence.
  if (IsOutermostLoop && (Ref->hasIV(LoopLevel) ||
                          (getMaxNonSimplifiedBlobLevel(
                               Ref, HasNonSimplifiedBlob) >= LoopLevel))) {
    return false;
  }

  if (AssumeDDIndependence) {
    return true;
  }

  bool IsRval = Ref->isRval();
  bool IsRefUnconditional = isUnconditionallyExecuted(Ref, Loop);
  bool IsUnconditional = IsRefUnconditional;

  // Check whether the ref can be hoisted outside the resulting loop after
  // unrolling.
  for (auto SymRef : OuterLoopMemRefMap[Ref->getSymbase()]) {
    if (SymRef == Ref) {
      continue;
    }

    bool AreEqual = DDRefUtils::areEqual(Ref, SymRef, true);
    bool SymRefIsRval = SymRef->isRval();
    bool AreRval = (IsRval && SymRefIsRval);

    // Guard doRefsAlias() under cheaper conditions where we know how to handle
    // refs. These conditions are used later in the loop.
    if (!AreEqual && !AreRval && !HCU.DDA.doRefsAlias(Ref, SymRef)) {
      continue;
    }

    if (IsOutermostLoop) {
      // Give up if SymRef is not contained in a candidate loop.
      if (!AreRval &&
          !HLNodeUtils::contains(OuterLoop, SymRef->getHLDDNode()) &&
          !isInSiblingCandidateLoop(SymRef)) {
        return false;
      }
    } else if (!HLNodeUtils::contains(Loop, SymRef->getHLDDNode())) {
      continue;
    }

    if (AreEqual) {
      // If there is an identical ref which is unconditional in loop, assume
      // this ref can be hoisted. This corresponds to loop memory motion's
      // profitability model.
      if (!IsUnconditional && isUnconditionallyExecuted(SymRef, Loop)) {
        IsUnconditional = true;
      }

      continue;
    }

    if (!IsOutermostLoop &&
        refinedOccurencesUsingLocalityAnalysis(Ref, SymRef, IsRefUnconditional,
                                               Loop, RefinedOccurences)) {
      // We refined Ref's occurrences using temporal locality with SymRef. No
      // further checks required.
      continue;
    }

    if (AreRval) {
      continue;
    }

    // We know that Ref and SymRef can alias.
    // Now we check whether SymRef can also be hoisted outside the loop.
    // This is to handle cases like this-
    // Ref: A[i2], SymRef: A[0]

    // If Ref has a symbolic and aliases with something, assume it is not
    // independent. For example-
    // Ref: A[i2+%t], SymRef A[0]
    if (HasNonSimplifiedBlob) {
      // Invalidate refined occurences in the presence of aliasing issues.
      RefinedOccurences = 0;
      return false;
    }

    // Give up if the refs do not look structurally similar: A[i1] and B[0].
    if (!DDRefUtils::haveEqualBaseAndShape(Ref, SymRef, false) ||
        !DDRefUtils::haveEqualOffsets(Ref, SymRef)) {
      RefinedOccurences = 0;
      return false;
    }

    bool SymRefHasBlob = false;

    // Check if SymRef is structurally invariant w.r.t loop or has symbolic.
    // Example where SymRef has symbolic-
    // Ref: A[i2], SymRef: A[%t]
    if ((getMaxNonSimplifiedBlobLevel(SymRef, SymRefHasBlob) >= LoopLevel) ||
        SymRefHasBlob) {
      RefinedOccurences = 0;
      return false;
    }

    if (IsOutermostLoop && SymRef->hasIV(LoopLevel)) {
      return false;
    }
  }

  return IsUnconditional;
}

HIRCompleteUnroll::ProfitabilityAnalyzer::GEPRefInfo
HIRCompleteUnroll::ProfitabilityAnalyzer::computeGEPInfo(const RegDDRef *Ref,
                                                         bool IsMemRef) const {

  SmallVector<std::pair<unsigned, unsigned>, 4> RemBlobs;

  unsigned MaxNonRemBlobLevel = populateRemBlobs(Ref, RemBlobs);

  if (MaxNonRemBlobLevel >= CurLevel) {
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

    unsigned TripCount = TCIt->second;
    unsigned Level = ParentLoop->getNestingLevel();
    unsigned RefinedOccurences = 0;

    // If ref contains IV of a loop or a blob defined at that level, all
    // references of the ref are considered unique w.r.t that level.
    // If ref is not DD independent in loop, there can be no savings
    // from unrolling.
    IsUnique = IsUnique || (MaxNonRemBlobLevel >= Level) ||
               (IsMemRef &&
                !isDDIndependentInLoop(Ref, ParentLoop, RefinedOccurences));

    if (IsUnique || Ref->hasIV(Level)) {

      TotalOccurences *= TripCount;

      if (!UniqueOccurences) {
        UniqueOccurences = RefinedOccurences ? RefinedOccurences : TripCount;
      } else {
        UniqueOccurences *= (RefinedOccurences ? RefinedOccurences : TripCount);
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
        TotalOccurences *= TripCount;
      }

      continue;

    } else {
      TotalOccurences *= TripCount;
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
  unsigned RefinedOccurences = 0;
  if (UniqueOccurences && IsMemRef && OutermostLoop &&
      isDDIndependentInLoop(Ref, OutermostLoop, RefinedOccurences)) {
    IsIndependent = true;
  }

  return GEPRefInfo(UniqueOccurences, TotalOccurences, IsIndependent);
}

class IntermediateAllocaStoreFinder final : public HLNodeVisitorBase {
  unsigned AllocaBaseIndex;
  const HLNode *EndNode;
  bool FoundStore;
  bool FoundEndNode;

public:
  IntermediateAllocaStoreFinder(unsigned AllocaBaseIndex, const HLNode *EndNode)
      : AllocaBaseIndex(AllocaBaseIndex), EndNode(EndNode), FoundStore(false),
        FoundEndNode(false) {}

  bool visit(const HLNode *Node) {
    FoundEndNode = (Node == EndNode);
    return FoundEndNode;
  }
  void visit(const HLInst *Inst);

  void postVisit(const HLNode *Node) {}

  bool isDone() const { return FoundStore || FoundEndNode; }

  bool foundIntermediateStore() const { return FoundStore; }
};

void IntermediateAllocaStoreFinder::visit(const HLInst *Inst) {

  if (visit(static_cast<const HLNode *>(Inst))) {
    return;
  }

  auto LvalRef = Inst->getLvalDDRef();

  if (!LvalRef || !LvalRef->isMemRef() || !LvalRef->accessesAlloca()) {
    return;
  }

  if (LvalRef->getBasePtrBlobIndex() == AllocaBaseIndex) {
    FoundStore = true;
  }
}

bool HIRCompleteUnroll::ProfitabilityAnalyzer::
    foundSimplifiedDominatingStoreInPreviousLoopnest(
        const RegDDRef *AllocaLoadRef, unsigned BaseIndex) {
  auto It = HCU.PrevLoopnestAllocaStores.find(BaseIndex);

  if (It != HCU.PrevLoopnestAllocaStores.end()) {
    auto PrevParentLoop = It->second;
    auto PrevRegion = PrevParentLoop->getParentRegion();
    auto CurRegion = OuterLoop->getParentRegion();

    auto LoadNode = AllocaLoadRef->getHLDDNode();

    const HLNode *FirstNode = nullptr;
    const HLNode *LastNode = nullptr;

    if (PrevRegion != CurRegion) {
      // Since we are dealing with different regions, we can only perform
      // approximate checks.

      if (!HCU.DT.dominates(PrevRegion->getExitBBlock(),
                            CurRegion->getEntryBBlock())) {
        // Previous region does not dominate current region so we remove store's
        // entry.
        HCU.PrevLoopnestAllocaStores.erase(It);
        return false;
      }

      const HLNode *LastRegionChild = PrevRegion->getLastChild();

      if ((PrevParentLoop->getParent() != PrevRegion) ||
          !HLNodeUtils::dominates(PrevParentLoop, LastRegionChild)) {
        // Store is not executed unconditionally in previous region so we remove
        // its entry.
        HCU.PrevLoopnestAllocaStores.erase(It);
        return false;
      }

      if (PrevParentLoop != LastRegionChild) {
        // Visit from PrevParentLoop's next node to end of region looking for
        // intermediate alloca stores.
        IntermediateAllocaStoreFinder IASF(BaseIndex, nullptr);
        HLNodeUtils::visitRange(IASF, PrevParentLoop->getNextNode(),
                                LastRegionChild);

        if (IASF.foundIntermediateStore()) {
          HCU.PrevLoopnestAllocaStores.erase(It);
          return false;
        }
      }

      FirstNode = &*(std::next(PrevRegion->getIterator()));
      LastNode = CurRegion;

    } else {
      if (!HLNodeUtils::dominates(PrevParentLoop, LoadNode)) {
        // Since the simplified store does not dominate this ref, we are most
        // likely out of its lexical scope. Hence, we remove its entry.
        HCU.PrevLoopnestAllocaStores.erase(It);
        return false;
      }

      const HLNode *OuterNode = OuterLoop;
      const HLNode *ParentNode = PrevParentLoop->getParent();
      const HLNode *TmpNode = nullptr;

      // Find the outer node of the load whose parent is the same as previous
      // loop's parent. This is to set the begin/end nodes for the visitor later
      // on.
      while ((TmpNode = OuterNode->getParent()) && (TmpNode != ParentNode)) {
        OuterNode = TmpNode;
      }

      // Could not reach previous loop's parent node. This is possible if
      // previous loop is inside a constant trip sibling loop of current loop.
      // Give up in those cases.
      if (!TmpNode) {
        HCU.PrevLoopnestAllocaStores.erase(It);
        return false;
      }

      FirstNode = PrevParentLoop->getNextNode();
      LastNode = OuterNode;
    }

    // Look for intermediate alloca stores which would invalidate the existing
    // entry.
    IntermediateAllocaStoreFinder IASF(BaseIndex, LoadNode);
    HLNodeUtils::visitRange(IASF, FirstNode, LastNode);

    if (IASF.foundIntermediateStore()) {
      HCU.PrevLoopnestAllocaStores.erase(It);
      return false;
    }

    return true;
  }

  return false;
}

bool HIRCompleteUnroll::ProfitabilityAnalyzer::foundSimplifiedDominatingStore(
    const RegDDRef *AllocaLoadRef, unsigned BaseIndex) {

  // First, look for simplified refs in current loopnest.
  auto It = AllocaStores.find(BaseIndex);

  if (It != AllocaStores.end()) {
    auto SimplifiedStore = It->second;

    int64_t Dist;
    // We found an alloca load/store combination where either-
    // 1) There is no constant distance (like A[i1], A[i2]) so we give up on
    // them. Or 2) There is no reuse between them. For example-
    //   A[i1] =
    //         = A[i1+1]
    if (!DDRefUtils::getConstIterationDistance(SimplifiedStore, AllocaLoadRef,
                                               AllocaLoadRef->getNodeLevel(),
                                               &Dist) ||
        (Dist < 0)) {
      return false;
    }

    if (!HLNodeUtils::dominates(SimplifiedStore->getHLDDNode(),
                                AllocaLoadRef->getHLDDNode())) {
      // Since the simplified store does not dominate this ref, we are most
      // likely out of its lexical scope. Hence, we remove its entry.
      AllocaStores.erase(It);
      return false;
    }

    return true;
  }

  // Now look for simplified refs in previous loopnests.
  return foundSimplifiedDominatingStoreInPreviousLoopnest(AllocaLoadRef,
                                                          BaseIndex);
}

class HIRCompleteUnroll::ProfitabilityAnalyzer::InvalidAllocaRefFinder final
    : public HLNodeVisitorBase {
  HIRCompleteUnroll::ProfitabilityAnalyzer &PA;
  unsigned AllocaBaseIndex;
  BasicBlock *StartBBlock;
  bool IsStartLoop;
  bool FoundInvalidRef;
  bool IsDone;

  bool foundInvalidRef() const { return FoundInvalidRef; }

  void reset(bool StartLoop) {
    FoundInvalidRef = false;
    IsDone = false;
    IsStartLoop = StartLoop;
  }

public:
  InvalidAllocaRefFinder(HIRCompleteUnroll::ProfitabilityAnalyzer &PA,
                         unsigned AllocaBaseIndex, BasicBlock *StartBBlock)
      : PA(PA), AllocaBaseIndex(AllocaBaseIndex), StartBBlock(StartBBlock),
        IsStartLoop(true), FoundInvalidRef(false), IsDone(false) {}

  void visit(const HLNode *Node) {}

  void visit(const HLRegion *Reg);
  void visit(const HLInst *Inst);

  void postVisit(const HLNode *Node) {}

  bool isDone() const { return FoundInvalidRef || IsDone; }

  bool foundInvalidUse(const HLNode *PrevNode, bool IsStartLoop);
};

void HIRCompleteUnroll::ProfitabilityAnalyzer::InvalidAllocaRefFinder::visit(
    const HLRegion *Reg) {

  if (!PA.HCU.DT.dominates(StartBBlock, Reg->getEntryBBlock())) {
    IsDone = true;
  }
}

void HIRCompleteUnroll::ProfitabilityAnalyzer::InvalidAllocaRefFinder::visit(
    const HLInst *Inst) {

  for (auto It = Inst->op_ddref_begin(), E = Inst->op_ddref_end(); It != E;
       ++It) {
    auto *Ref = *It;

    if (!Ref->isMemRef() || (Ref->getBasePtrBlobIndex() != AllocaBaseIndex)) {
      continue;
    }

    bool HasNonSimplifiedBlob = false;

    PA.getMaxNonSimplifiedBlobLevel(Ref, HasNonSimplifiedBlob);

    if (HasNonSimplifiedBlob) {
      FoundInvalidRef = true;
      break;
    }

    if (Ref->isLval()) {
      if (IsStartLoop) {
        // Keep looking for alloca refs if we found a simplifiable alloca store
        // in start loop.
        continue;
      } else {
        // Assume profitability if simplified alloca store is found.
        // TODO: This may need to be changed as it is optimisitc.
        IsDone = true;
        break;
      }
    }

    auto ParLoop = Inst->getParentLoop();
    bool IsCandidateLoop = false;

    // Check if any parent loop is unroll candidate.
    while (ParLoop) {
      if (PA.HCU.TopLevelCandidates.count(ParLoop)) {
        IsCandidateLoop = true;
        break;
      }

      ParLoop = ParLoop->getParentLoop();
    }

    FoundInvalidRef = !IsCandidateLoop;
    IsDone = true;
    break;
  }
}

bool HIRCompleteUnroll::ProfitabilityAnalyzer::InvalidAllocaRefFinder::
    foundInvalidUse(const HLNode *PrevNode, bool IsStartLoop) {

  reset(IsStartLoop);

  if (isa<HLRegion>(PrevNode)) {
    HLNodeUtils::visitRange(
        *this, std::next(PrevNode->getIterator()),
        HLContainerTy::const_iterator(
            PrevNode->getHLNodeUtils().getHIRFramework().hir_end()));

  } else {
    auto *StartNode = PrevNode->getNextNode();

    if (!StartNode) {
      return false;
    }

    auto *EndNode =
        HLNodeUtils::getLastLexicalChild(StartNode->getParent(), StartNode);
    HLNodeUtils::visitRange(*this, StartNode, EndNode);
  }

  return foundInvalidRef();
}

bool HIRCompleteUnroll::ProfitabilityAnalyzer::profitableToPropagateAllocaStore(
    const RegDDRef *AllocaStore, unsigned BaseIndex) {

  auto It = ProfitableAllocaStores.find(BaseIndex);

  if (It != ProfitableAllocaStores.end()) {
    return It->second;
  }

  auto *CurRegion = OuterLoop->getParentRegion();

  InvalidAllocaRefFinder IARF(*this, BaseIndex, CurRegion->getExitBBlock());

  // Look for invalid use in current loop.
  if (IARF.foundInvalidUse(AllocaStore->getHLDDNode(), true)) {
    ProfitableAllocaStores[BaseIndex] = false;
    return false;
  }

  // Look for invalid use in current region.
  if (IARF.foundInvalidUse(OuterLoop, false)) {
    ProfitableAllocaStores[BaseIndex] = false;
    return false;
  }

  // Give up on conditional loops.
  if (OuterLoop->getParent() != CurRegion) {
    ProfitableAllocaStores[BaseIndex] = false;
    return false;
  }

  // Look for invalid use in subsequent regions.
  if (IARF.foundInvalidUse(CurRegion, false)) {
    ProfitableAllocaStores[BaseIndex] = false;
    return false;
  }

  ProfitableAllocaStores[BaseIndex] = true;
  // We did not find any invalid alloca load/store in HIR regions. Assume
  // profitability.
  return true;
}

bool HIRCompleteUnroll::ProfitabilityAnalyzer::canEliminate(
    const RegDDRef *MemRef, bool CanSimplifySubs) {
  assert(MemRef->isMemRef() && "Memref expected!");

  if (CanSimplifySubs && MemRef->accessesConstantArray()) {
    return true;
  }

  if (!MemRef->accessesAlloca()) {
    return false;
  }

  unsigned BaseIndex = MemRef->getBasePtrBlobIndex();

  if (MemRef->isLval()) {
    if (CanSimplifySubs &&
        profitableToPropagateAllocaStore(MemRef, BaseIndex)) {
      // Alloca stores can be eliminated after unrolling by propagating the
      // assigned value directly into corresponding loads.
      AllocaStores[BaseIndex] = MemRef;

      // Restrict the optimistic assumption of considering alloca stores as
      // optimizable to post-vec complete unroll.
      return !HCU.IsPreVec;
    } else {
      // We encountered a non-simplifiable alloca store. Invalidate its entry
      // from the data structures.
      AllocaStores.erase(BaseIndex);
      HCU.PrevLoopnestAllocaStores.erase(BaseIndex);
      return false;
    }

  } else if (!CanSimplifySubs) {
    return false;
  }

  if (foundSimplifiedDominatingStore(MemRef, BaseIndex)) {
    return true;
  }

  // If all else fails, check whether we can unconditionally reach alloca's
  // parent bblock from the region predecessor bblock. If so, assume there are
  // dominating alloca stores.

  // First we check whether the current region has any alloca stores before
  // this node. If so, we have invalidating stores so we have to give up.
  //
  if (AllocaStores.count(BaseIndex)) {
    return false;
  }

  auto CurRegion = OuterLoop->getParentRegion();
  IntermediateAllocaStoreFinder IASF(BaseIndex, OuterLoop);

  HLNodeUtils::visitRange(IASF, CurRegion->child_begin(),
                          CurRegion->child_end());

  if (IASF.foundIntermediateStore()) {
    return false;
  }

  unsigned BaseSymbase = MemRef->getBasePtrSymbase();

  if (!CurRegion->isLiveIn(BaseSymbase)) {
    return false;
  }

  auto AllocaBB = cast<AllocaInst>(MemRef->getTempBaseValue())->getParent();
  auto PredBB = CurLoop->getParentRegion()->getPredBBlock();

  while (PredBB && (PredBB != AllocaBB)) {
    PredBB = PredBB->getSinglePredecessor();
  }

  return (PredBB == AllocaBB);
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

  bool CanSimplifyToConst = (IsMemRef && canEliminate(Ref, CanSimplifySubs));

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
      NumDDRefs += UniqueOccurences;
    }
  }

  if (AddToVisitedSet) {
    // Add linear refs to visited set to avoid duplicate processing.
    VisitedGEPRefs.emplace_back(Ref, SimplifiedToConstSavings,
                                UniqueOccurences);
  }

  return CanSimplifyToConst;
}

bool HIRCompleteUnroll::ProfitabilityAnalyzer::visitedGEPRef(
    const RegDDRef *Ref, bool &AddToVisitedSet,
    bool &SkipAddressSimplification) {
  assert(Ref->hasGEPInfo() && "GEP Ref expected!");

  unsigned DefLevel = Ref->getDefinedAtLevel();

  if (DefLevel == NonLinearLevel) {
    return false;
  }

  for (auto &RefInfo : VisitedGEPRefs) {
    if (DDRefUtils::areEqual(Ref, RefInfo.Ref)) {
      if (Ref->isRval() == RefInfo.Ref->isRval()) {

        if (RefInfo.SimplifiedToConstSavings != 0) {
          // Simplfied to const savings should be accounted for each occurence
          // of the ref.
          GEPSavings += RefInfo.SimplifiedToConstSavings;
        } else if (Ref->isMemRef()) {
          NumMemRefs += RefInfo.UniqueOccurences;
        }

        return true;
      }

      SkipAddressSimplification = true;
    }
  }

  AddToVisitedSet = true;
  return false;
}

bool HIRCompleteUnroll::ProfitabilityAnalyzer::processGEPRef(
    const RegDDRef *Ref) {
  bool AddToVisitedSet = false;
  bool SkipAddressSimplification = false;

  if (visitedGEPRef(Ref, AddToVisitedSet, SkipAddressSimplification)) {
    return false;
  }

  bool CanSimplify = true;
  unsigned NumAddressSimplifications = 0;
  bool AnyDimSimplified = false;
  bool HasNonZeroDimOrOffsets = false;

  // Processes embedded canon exprs and computes address simplification
  // opportunities. Address computation is associative. This means that
  // simplification of non-consecutive dimensions can be added. For example, we
  // will add up simplificaiton of first and third dimension for A[i1][%t][i2].
  for (unsigned I = 1, NumDims = Ref->getNumDimensions(); I <= NumDims; ++I) {
    auto CE = Ref->getDimensionIndex(I);

    HasNonZeroDimOrOffsets =
        HasNonZeroDimOrOffsets || Ref->hasNonZeroTrailingStructOffsets(I);

    if (!processCanonExpr(CE, Ref)) {
      CanSimplify = false;

    } else {
      int64_t Val;

      // Process based on whether the dimension was already a constant.
      if (CE->isIntConstant(&Val)) {
        HasNonZeroDimOrOffsets = HasNonZeroDimOrOffsets || (Val != 0);
      } else {

        if (I != 1) {
          // This applies to first dimension as well if stride is not 1 but
          // incorporating it leads to a skewed profitablity model causing perf
          // regressions.
          // TODO: fix the cost model.

          // Simplification of (index * stride).
          ++NumAddressSimplifications;

          if (AnyDimSimplified) {
            ++NumAddressSimplifications;
          }
        }

        AnyDimSimplified = true;
      }
    }
  }

  if (SkipAddressSimplification) {
    NumAddressSimplifications = 0;

  } else if (AnyDimSimplified) {

    // Add simplified dimensions and non-zero offsets.
    if (HasNonZeroDimOrOffsets) {
      ++NumAddressSimplifications;
    }

    if (Ref->accessesAlloca() || Ref->accessesInternalGlobalVar()) {
      // The base address is known at compile time for global vars and stack
      // frame offset can be simplified for allocas.
      ++NumAddressSimplifications;
    }
  }

  CanSimplify =
      addGEPCost(Ref, AddToVisitedSet, CanSimplify, NumAddressSimplifications);

  return CanSimplify;
}

bool HIRCompleteUnroll::ProfitabilityAnalyzer::processRef(const RegDDRef *Ref) {
  if (Ref->hasGEPInfo()) {
    return processGEPRef(Ref);
  }

  assert(Ref->isTerminalRef() && "Unexpected ref type!");

  bool Simplified = processCanonExpr(Ref->getSingleCanonExpr(), Ref);

  if (!Simplified) {
    NumDDRefs += LoopNestTripCount;
  }

  return Simplified;
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

  CanonExprInfo CEInfo;
  bool IsLinear = !CE->isNonLinear();

  if (CE->isConstantData()) {
    return true;
  }

  bool CanSimplifyIVs = processIVs(CE, ParentRef, CEInfo);

  bool CanSimplifyBlobs = processBlobs(CE, ParentRef, CEInfo.NumSimplifiedTerms,
                                       CEInfo.NumNonLinearTerms);

  bool NumeratorBecomesConstant = CanSimplifyIVs && CanSimplifyBlobs;

  // For each unrollable IV which has a blob, we can simplify the terms in the
  // first unrolled iteration when IV value is zero. For example, if i1 can be
  // unrolled and b2 can be simplified, CE: (b1*i1 + b2 + 2) has a total of 3
  // terms (2 additions) which can be folded when i1 is 0.
  if (CEInfo.NumUnrollableIVBlobs) {
    ScaledSavings += CEInfo.NumUnrollableIVBlobs + CEInfo.NumSimplifiedTerms +
                     (CE->getConstant() ? 1 : 0) - 1;
  }

  // Add 1 to savings each, for number of simplified IV/Blob additions.
  if (CEInfo.NumSimplifiedTerms) {
    Savings += (CEInfo.NumSimplifiedTerms - 1);
  }

  // Add 1 to cost each, for number of non-linear IV/Blob additions.
  if (CEInfo.NumNonLinearTerms) {
    Cost += (CEInfo.NumNonLinearTerms - 1);
  }

  // Add 1 to cost/savings for the constant based on linearity and IV
  // simplifications.
  if (CE->getConstant()) {
    if (CEInfo.NumSimplifiedTerms) {
      ++Savings;
    } else if (!IsLinear) {
      ++Cost;
    }
  } else if ((CEInfo.NumSimplifiedTerms == 1) &&
             CEInfo.HasUnrollableStandAloneIV) {
    // Make sure we add at least 1 to savings for turning any IV into a
    // constant. Otherwise converting simple expressions like A[i1] to A[0] will
    // not be considered savings.
    ++Savings;
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
    // TODO: ignore 'free' casts using TTI.
    if (NumeratorBecomesConstant) {
      ++Savings;
    } else if (!IsLinear) {
      ++Cost;
    }
  }

  return NumeratorBecomesConstant;
}

bool HIRCompleteUnroll::ProfitabilityAnalyzer::processIVs(
    const CanonExpr *CE, const RegDDRef *ParentRef, CanonExprInfo &CEInfo) {

  bool CanSimplifyIVs = true;
  unsigned OuterLevel = OuterLoop->getNestingLevel();
  bool IsLinear = !CE->isNonLinear();
  SmallSet<unsigned, 4> CurrentUnrollableIVBlobs;

  for (unsigned Level = 1; Level <= CurLevel; ++Level) {
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
          ++CEInfo.NumSimplifiedTerms;
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
                  CEInfo.NumNonLinearTerms, nullptr);

      if (IsUnrollableLoopLevel) {
        // Add to loop level blob set to avoid duplicate cost.
        VisitedUnrollableIVBlobs.insert(BlobIndex);
      }

    } else if (IsUnrollableLoopLevel) {

      // Add one for simplfication of multiplication with coefficient.
      if (Coeff != 1) {
        ++Savings;
      } else {
        CEInfo.HasUnrollableStandAloneIV = true;
      }

      ++CEInfo.NumSimplifiedTerms;

    } else {
      CanSimplifyIVs = false;
    }
  }

  CEInfo.NumUnrollableIVBlobs = CurrentUnrollableIVBlobs.size();

  return CanSimplifyIVs;
}

bool HIRCompleteUnroll::ProfitabilityAnalyzer::processBlobs(
    const CanonExpr *CE, const RegDDRef *ParentRef,
    unsigned &NumSimplifiedTerms, unsigned &NumNonLinearTerms) {
  bool CanSimplifyBlobs = true;
  bool IsLinear = !CE->isNonLinear();
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

  BInfo.VisitedAsUnrollableIVBlob = VisitedUnrollableIVBlobs.count(Index);

  unsigned DefLevel;
  auto CurNode = ParentRef->getHLDDNode();

  if (ParentRef->findTempBlobLevel(Index, &DefLevel)) {
    if (DefLevel == NonLinearLevel) {
      DefLevel = CurLevel;
    }

    BInfo.Simplified = isSimplifiedTempBlob(Index, DefLevel, CurNode);

  } else {
    BInfo.Simplified = false;
  }

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

    if (DefLevel == NonLinearLevel) {
      DefLevel = CurLevel;
    }

    if (isSimplifiedTempBlob(Idx, DefLevel, CurNode)) {
      ++NumSimplifiedTempBlobs;
    } else if (DefLevel == CurLevel) {
      Invariant = false;
      VisitedNonLinearBlobs.insert(
          std::make_pair(Idx, SmallVector<int64_t, 2>()));
    }
  }

  // All containing temp blobs can be simplified so we mark the blob as
  // simplified.
  if (NumSimplifiedTempBlobs == Indices.size()) {
    BInfo.Simplified = true;
    BInfo.NumOperations = BU.getNumOperations(Index, &HCU.TTI);

  } else if (!Invariant) {
    BInfo.Invariant = false;
    BInfo.NumOperations = BU.getNumOperations(Index, &HCU.TTI);

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

bool HIRCompleteUnroll::run() {
  // Skip if DisableHIRCompleteUnroll is enabled
  if (DisableHIRCompleteUnroll) {
    LLVM_DEBUG(dbgs() << "HIR LOOP Complete Unroll Transformation Disabled \n");
    return false;
  }

  LLVM_DEBUG(dbgs() << "Complete unrolling for Function : "
                    << HIRF.getFunction().getName() << "\n");

  // Storage for Outermost Loops
  SmallVector<HLLoop *, 64> OuterLoops;
  // Gather the outermost loops
  HIRF.getHLNodeUtils().gatherOutermostLoops(OuterLoops);

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

    SmallVector<HLLoop *, 8> ChildCandidateLoops;
    if (!OuterCandidateLoop->isInnermost()) {

      OuterCandidateLoop->getHLNodeUtils().gatherLoopsWithLevel(
          OuterCandidateLoop, ChildCandidateLoops,
          OuterCandidateLoop->getNestingLevel() + 1);
    }

    CandidateLoops.erase(CandidateLoops.begin() + Index);
    CandidateLoops.insert(CandidateLoops.begin() + Index,
                          ChildCandidateLoops.begin(),
                          ChildCandidateLoops.end());
  }
}

bool HIRCompleteUnroll::isApplicable(const HLLoop *Loop) const {

  // Throttle multi-exit/unknown loops.
  if (!Loop->isDo()) {
    LLVM_DEBUG(dbgs() << "Skipping complete unroll of non-DO loop!\n");
    return false;
  }

  // Ignore vectorizable loops
  if (Loop->isVecLoop()) {
    LLVM_DEBUG(dbgs() << "Skipping complete unroll of vectorizable loop!\n");
    return false;
  }

  // Handle normalized loops only.
  if (!Loop->isNormalized()) {
    LLVM_DEBUG(dbgs() << "Skipping complete unroll of non-normalized loop!\n");
    return false;
  }

  if (Loop->hasCompleteUnrollDisablingPragma()) {
    LLVM_DEBUG(dbgs() << "Skipping complete unroll due to presence of unroll "
                         "disabling pragma!\n");
    return false;
  }

  if (IsPreVec && Loop->hasVectorizeEnablingPragma()) {
    LLVM_DEBUG(
        dbgs()
        << "Skipping complete unroll due to presence of vector pragma!\n");
    return false;
  }

  auto &LS = HLS.getSelfLoopStatistics(Loop);

  // Cannot unroll loop if it has calls with noduplicate attribute.
  if (LS.hasCallsWithNoDuplicate()) {
    LLVM_DEBUG(
        dbgs() << "Skipping complete unroll of loop containing call(s) with "
                  "NoDuplicate attribute!\n");
    return false;
  }

  return true;
}

bool HIRCompleteUnroll::cannotHandleLiveouts(const HLLoop *Loop,
                                             int64_t MinUpper) const {
  // There are some corner cases where during unroll of triangular loops, the
  // inner needs to be removed and its liveout temp uses need to be elliminated
  // otherwise we would leave uninitialized temps in HIR. This is problematic
  // to handle during the transformation because it requires dead code
  // elimination functionality. It also requires correctly handling the cases
  // where we eliminate the loop's next node as HLNodeVisitor does not work in
  // those cases. Therefore, we detect this case in the analysis phase and
  // suppress the transformation.
  //
  // TODO: Remove this check when we have reasonably robust dead code
  // elimination in HIR.

  // If the loop has ztt, we simply eliminate the postexit during
  // transformation. Loop temps cannot dominate anything else outside the loop
  // so we cannot leave uninitialized temp after unrolling.
  if ((MinUpper >= 0) || Loop->hasZtt()) {
    return false;
  }

  // Check whether the loop has liveouts which are not livein to the loop.
  // This indicates a temp which dominates use(s) outside the loop.
  for (auto It = Loop->live_out_begin(), E = Loop->live_out_end(); It != E;
       ++It) {
    if (!Loop->isLiveIn(*It)) {
      return true;
    }
  }

  return false;
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

  if (cannotHandleLiveouts(Loop, MinUpper)) {
    return std::make_pair(-1, DepLevel);
  }

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

  // If user has specified unroll pragma, unroll the loop as long as it is legal
  // to do so.
  if (Loop->hasCompleteUnrollEnablingPragma()) {
    if (MinDepLevel == LoopLevel) {
      TopLevelCandidates.insert(Loop);
      CandidateLoops.push_back(Loop);
    }

    // We disable unroll of parent and children loops of a pragma enabled loop
    // unless they have a pragma as well. This is done to keep the
    // implementation (profitability and transformation) simple.
    return std::make_pair(-1, 0);
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
  SmallVector<SimplifiedTempBlob, 8> SimplifiedTempBlobs;
  MemRefGatherer::MapTy MemRefMap;
  DenseMap<unsigned, const RegDDRef *> AllocaStores;
  SmallPtrSet<const HLNode *, 8> SimplifiedNonLoopParents;

  // Skipping profitability check for pragma enabled loops can affect
  // profitability of sibling loops but this is probably okay as the user can
  // specify pragma on them as well.
  if (Loop->hasCompleteUnrollEnablingPragma()) {
    return true;
  }

  ProfitabilityAnalyzer PA(*this, Loop, SimplifiedTempBlobs, MemRefMap,
                           AllocaStores, SimplifiedNonLoopParents);

  PA.analyze();

  if (PA.isProfitable()) {
    // Store unconditional simplifiable alloca stores to be used in
    // profitability checks for subsequent loopnests.
    for (auto &Pair : AllocaStores) {
      if (PA.isUnconditionallyExecuted(Pair.second, Loop)) {
        PrevLoopnestAllocaStores[Pair.first] = Loop;
      }
    }

    return true;
  }

  return false;
}

void HIRCompleteUnroll::doUnroll(HLLoop *Loop) {

  LoopOptReportBuilder &LORBuilder =
      Loop->getHLNodeUtils().getHIRFramework().getLORBuilder();

  if (Loop->isInnermost()) {
    LORBuilder(*Loop).addRemark(OptReportVerbosity::Low,
                                "Loop completely unrolled");
  } else {
    LORBuilder(*Loop).addRemark(OptReportVerbosity::Low,
                                "Loopnest completely unrolled");
  }

  LORBuilder(*Loop).preserveLostLoopOptReport();

  HIRInvalidationUtils::invalidateParentLoopBodyOrRegion(Loop);
  // Also invalidate analyses for the loops being unrolled. Since we reuse the
  // instructions from the unrolled loops, not invalidating the analyses may
  // leave them in an inconsistent state. This is the case for safe reduction
  // analysis.
  HIRInvalidationUtils::invalidateLoopNestBody(Loop);

  Loop->getParentRegion()->setGenCode();

  SmallVector<int64_t, MaxLoopNestLevel> IVValues;
  CanonExprUpdater CEUpdater(Loop->getNestingLevel(), IVValues,
                             Loop->hasCompleteUnrollEnablingPragma());

  transformLoop(Loop, CEUpdater, true);

  assert(IVValues.empty() && "IV values were not cleaned up!");
}

// Transform (Complete Unroll) each loop inside the CandidateLoops vector
void HIRCompleteUnroll::transformLoops() {

  LoopnestsCompletelyUnrolled += CandidateLoops.size();

  // Transform the loop nest from outer to inner.
  for (auto &Loop : CandidateLoops) {
    HLNode *ParentNode = Loop->getParentLoop();
    if (!ParentNode) {
      ParentNode = Loop->getParentRegion();
    }

    doUnroll(Loop);

    HLNodeUtils::removeRedundantNodes(ParentNode);
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

  for (auto IVVal : IVValues) {
    UBVal += (IVVal * UBCE->getIVConstCoeff(LoopLevel));
    assert((IVVal != -1 || UBCE->getIVConstCoeff(LoopLevel) == 0) &&
           "Attempt to substitute IV of non-unrollable loop!");
    LoopLevel++;
  }

  return UBVal;
}

// Complete Unroll the given Loop, using provided LD as helper data
void HIRCompleteUnroll::transformLoop(HLLoop *Loop, CanonExprUpdater &CEUpdater,
                                      bool IsTopLevelLoop) {

  // Guard against the scanning phase setting it appropriately.
  assert(Loop && " Loop is null.");

  auto &IVValues = CEUpdater.IVValues;

  if (CEUpdater.IsPragmaEnabledUnrolling && !IsTopLevelLoop &&
      !Loop->hasCompleteUnrollEnablingPragma()) {
    // This is an inner loop of a pragma enabled outer loop.
    // We just need to update canon exprs.

    // Set an invalid IV value so the IV substitution can be skipped.
    IVValues.push_back(-1);

    // Update loop refs.
    for (auto RefIt = Loop->ddref_begin(), E = Loop->ddref_end(); RefIt != E;
         ++RefIt) {
      CEUpdater.processRegDDRef(*RefIt);
    }

    HLNodeUtils::visitRange<true, false>(CEUpdater, Loop->child_begin(),
                                         Loop->child_end());
    IVValues.pop_back();

    return;
  }

  int64_t LB = Loop->getLowerCanonExpr()->getConstant();
  int64_t UB = computeUB(Loop, CEUpdater.TopLoopLevel, IVValues);
  int64_t Step = Loop->getStrideCanonExpr()->getConstant();

  // This may be different than UB when Step is not 1.
  int64_t LastIVVal = LB + (((UB - LB) / Step) * Step);
  bool ZttHasBlob = false;

  // At this point loop preheader has been visited already but postexit is
  // not, so we need to handle postexit explicitly.

  if (UB < 0) {
    // We remove postexit so that visitor doesn't visit disconnected nodes.
    Loop->removePostexit();
    HLNodeUtils::remove(Loop);
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
    HLNodeUtils::visit<false>(CEUpdater,
                              Loop->extractZtt(CEUpdater.TopLoopLevel));
  } else {
    Loop->removeZtt();
  }

  HLNode *Marker = nullptr;

  if (!IsTopLevelLoop) {
    HLNodeUtils::visitRange(CEUpdater, Loop->post_begin(), Loop->post_end());
  }

  Loop->extractPreheaderAndPostexit();

  if (IsTopLevelLoop) {
    Marker = Loop->getHLNodeUtils().getOrCreateMarkerNode();
    // Replace top level loop of the unroll loopnest with marker node to avoid
    // top sort num recomputation.
    HLNodeUtils::replace(Loop, Marker);
  }

  auto OrigFirstChild = Loop->getFirstChild();
  auto OrigLastChild = Loop->getLastChild();

  IVValues.push_back(LB);

  // Container for cloning body.
  HLContainerTy LoopBody;

  // Iterate over Loop Child for unrolling with trip value incremented
  // each time. Thus, loop body will be expanded by no. of stmts x TripCount.
  for (int64_t IVVal = LB; IVVal < LastIVVal; IVVal += Step) {
    // Clone iteration
    HLNodeUtils::cloneSequence(&LoopBody, OrigFirstChild, OrigLastChild);

    // Store references as LoopBody will be empty after insertion.
    HLNode *CurFirstChild = &(LoopBody.front());
    HLNode *CurLastChild = &(LoopBody.back());

    HLNodeUtils::insertBefore(OrigFirstChild, &LoopBody);

    // Update IV value of loop for the current unrolled iteration for
    // substitution inside the canon expr.
    IVValues.back() = IVVal;

    // Update the CanonExpr
    HLNodeUtils::visitRange<true, false>(CEUpdater, CurFirstChild,
                                         CurLastChild);
  }

  // Reuse original children for last iteration.
  IVValues.back() = LastIVVal;
  HLNodeUtils::visitRange<true, false>(CEUpdater, OrigFirstChild,
                                       OrigLastChild);

  IVValues.pop_back();

  if (IsTopLevelLoop) {
    // Replace marker node with the unrolled loop children.
    HLNodeUtils::moveBefore(Marker, Loop->child_begin(), Loop->child_end());
    HLNodeUtils::remove(Marker);
  } else {
    HLNodeUtils::moveBefore(Loop, Loop->child_begin(), Loop->child_end());
    HLNodeUtils::remove(Loop);
  }
}

void HIRCompleteUnrollLegacyPass::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.setPreservesAll();
  AU.addRequiredTransitive<DominatorTreeWrapperPass>();
  AU.addRequiredTransitive<TargetTransformInfoWrapperPass>();
  AU.addRequiredTransitive<HIRFrameworkWrapperPass>();
  AU.addRequiredTransitive<HIRLoopStatisticsWrapperPass>();
  AU.addRequiredTransitive<HIRDDAnalysisWrapperPass>();
  AU.addRequiredTransitive<HIRSafeReductionAnalysisWrapperPass>();
}

bool HIRCompleteUnrollLegacyPass::runOnFunction(Function &F) {
  if (skipFunction(F)) {
    LLVM_DEBUG(dbgs() << "HIR LOOP Complete Unroll Transformation Disabled \n");
    return false;
  }

  return HIRCompleteUnroll(
             getAnalysis<HIRFrameworkWrapperPass>().getHIR(),
             getAnalysis<DominatorTreeWrapperPass>().getDomTree(),
             getAnalysis<TargetTransformInfoWrapperPass>().getTTI(F),
             getAnalysis<HIRLoopStatisticsWrapperPass>().getHLS(),
             getAnalysis<HIRDDAnalysisWrapperPass>().getDDA(),
             getAnalysis<HIRSafeReductionAnalysisWrapperPass>().getHSR(),
             OptLevel, IsPreVec)
      .run();
}
