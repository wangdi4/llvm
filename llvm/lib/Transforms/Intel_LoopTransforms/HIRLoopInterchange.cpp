//===----- HIRLoopInterchange.cpp - Permutations of HIR loops -------------===//
//
// Copyright (C) 2015-2020 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// Performs loop interchange to achieve best legal permutations
// so the number of cache lines accessed is smallest.
// Input:  Temporal & spatial locality reuse per loop nest, DDG
//
// Steps:
// 1) Walk all loops, look for outer loops that is perfectly nested
// 2) Retrieve loop cost computed from Locality Analysis pass
// 3) If already in decreasing order (from outer to inner),  all done
// 4) Exclude loops that has pragma  for unroll or unroll & jam
//    Exclude triangular loop until later
// 5) Gather DV from DDG
//    instead of calling Demand driven, filter out DV that implies INDEP
//    e.g. dv (> = *)  and our loop interchange candidate is from level 2 to 3,
//    then this edge can be ignored.
//    drop other edges that will not prevent loop interchange: e.g.
//    safe reduction, loop indepedent dep (t1=;  = t1)
//    anti dep for temps (< *)
// 6) Sort loop nests based on cost and get permutation P1.
//    If it's all legal to interchange, proceed to Gencode
// 7) Construct next permutation P2 that's legal
//    based on the permutation P1, from outermost to innermost,
//    choose loop L into P2 if legal. Discard L from P1 and repeat to add
//    more on P2
// 8) Extract pre-hdr & post-exit of outermost loop
// 9) Gencode:  based on P1/P2,  update loop bounds/loop body
// 10) clear safe-reduction flag for cases like:
//       do i; do j; s = s + a(j) ->  do j; do i; s = s + a(j)
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/Intel_LoopTransforms/HIRLoopInterchangePass.h"

#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRDDAnalysis.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRLocalityAnalysis.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRLoopResource.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRSafeReductionAnalysis.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Framework/HIRFramework.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/DDUtils.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HIRInvalidationUtils.h"
#include "llvm/Transforms/Intel_LoopTransforms/HIRTransformPass.h"
#include "llvm/Transforms/Intel_LoopTransforms/Passes.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/HIRTransformUtils.h"

#include "HIRPrintDiag.h"

#include "llvm/ADT/SetOperations.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"

#include <sstream>

#define OPT_SWITCH "hir-loop-interchange"
#define OPT_DESC "HIR Loop Interchange"
#define DEBUG_TYPE OPT_SWITCH

using namespace llvm;
using namespace llvm::loopopt;

STATISTIC(LoopsInterchanged, "Number of HIR loops interchanged");

static cl::opt<bool> DisableHIRLoopInterchange("disable-hir-loop-interchange",
                                               cl::init(false), cl::Hidden,
                                               cl::desc("Disable " OPT_DESC));

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
static cl::opt<unsigned>
    PrintDiagLevel(OPT_SWITCH "-print-diag", cl::init(0), cl::ReallyHidden,
                   cl::desc("Print Diag why " OPT_DESC " did not happen."));

static cl::opt<std::string> PrintDiagFunc(
    OPT_SWITCH "-print-diag-func", cl::ReallyHidden,
    cl::desc("Print Diag why " OPT_DESC " did not happen for the function."));

static void printSmallSetInt(SmallSet<unsigned, 4> &IntSet, std::string Msg) {
  formatted_raw_ostream FOS(dbgs());
  if (Msg.size()) {
    FOS << Msg << ": " << IntSet.size() << "\t<";
  }

  for (unsigned V : IntSet) {
    FOS << V << ",";
  }
  FOS << ">\n";
}
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

static cl::opt<unsigned> SinkedPerfectLoopProfitablityTCThreshold(
    OPT_SWITCH "-sinked-perfect-profitability-tc-threshold", cl::init(16),
    cl::Hidden,
    cl::desc("TripCount threshold to enable " OPT_DESC
             " for sinked perfect loopnests"));

// Flag to enable offline preparation for special loop interchange.
static cl::opt<bool> PrepareSpecialInterchange(
    OPT_SWITCH "-prepare-special-interchange", cl::init(false), cl::Hidden,
    cl::desc(OPT_DESC "prepare for the special interchange"));

// Number of expected loopnest for the special interchange
static cl::opt<unsigned> ExpectNumSpecialInterchangeLoopnests(
    OPT_SWITCH "-expect-loopnests", cl::init(1), cl::Hidden,
    cl::desc(OPT_DESC "expect number of loopnests for special interchange"));

// Flag to enable offline preparation for running array-scalarization cleanup.
// [Note]
// - This is part of preparation for special loop interchange over a 3-level
//   loopnest.
//
// [Note]
// - Currently, only the relevant LIT tests will turn this on.
// - Driver will need to turn this flag on to trigger special loop interchange.
//
static cl::opt<bool> RunArrayScalarizationSymbases(
    "run-" OPT_SWITCH "-array-scalarization-symbases", cl::init(false),
    cl::Hidden, cl::desc("Run " OPT_DESC " Array Scalarization with Symbases"));

// Symbase values provided to help offline preparation for running
// array-scalarization cleanup.
//
// [Note]
// - This is part of preparation for special loop interchange.
//
static cl::list<unsigned> ArrayScalarizationSymbases(
    OPT_SWITCH "-array-scalarization-symbases", cl::CommaSeparated,
    cl::desc("Symbases for Array Scalarization Cleanup"));

// Threshold on arithmetic operations to help enable special loop interchange.
static cl::opt<unsigned> SpecialInterchangeArithOpNumThreadshold(
    OPT_SWITCH "-special-interchange-arith-op-num-threshold", cl::init(1400),
    cl::Hidden,
    cl::desc("Arith Operation Threshold to activate special interchange"));

// Threshold on memory operations to help enable special loop interchange.
static cl::opt<unsigned> SpecialInterchangeMemOpNumThreadshold(
    OPT_SWITCH "-special-interchange-mem-op-num-threshold", cl::init(200),
    cl::Hidden,
    cl::desc("Memory Operation Threshold to activate special interchange"));

// Threshold on arithmetic vs. memory operation ratio to help enable special
// loop interchange.
static cl::opt<unsigned> SpecialInterchangeArithToMemOpRatioThreadshold(
    OPT_SWITCH "-special-interchange-arith-2-mem-op-ratio-threshold",
    cl::init(6), cl::Hidden,
    cl::desc("Memory Operation Threshold to activate special interchange"));

static cl::opt<unsigned> SpecialInterchangeExpectedNestingDepth(
    OPT_SWITCH "-special-interchange-expected-nesting-depth", cl::init(2),
    cl::Hidden,
    cl::desc("Expected loopnest depth to activate special interchange"));

static cl::opt<unsigned> SpecialInterchangeExpectedModIndependentLoops(
    OPT_SWITCH "-special-interchange-expected-mod-independent-loops",
    cl::init(1), cl::Hidden,
    cl::desc("Expected loops that are independent of any mod instruction in "
             "special interchange"));

static cl::opt<unsigned> SpecialInterchangeLpSizeThresholdLB(
    OPT_SWITCH "-special-interchange-loop-size-threshold-lowerbound",
    cl::init(1600), cl::Hidden,
    cl::desc("Expected number of HLInsts (LowerBound) in the target "
             "innermost loop for special interchange"));

static cl::opt<unsigned> SpecialInterchangeLpSizeThresholdUB(
    OPT_SWITCH "-special-interchange-loop-size-threshold-upperbound",
    cl::init(2300), cl::Hidden,
    cl::desc("Expected number of HLInsts (UpperBound) in the target "
             "innermost loop for special interchange"));

// Check: is the given value V within the range of [LB, UB]?
template <typename T> static bool isInRange(T V, T LB, T UB) {
  assert((LB <= UB) && "Expect LB <= UB\n");
  return (V >= LB) && (V <= UB);
}

namespace {

enum DiagMsg {
  NON_LINEAR_DEF_OR_ALL_UNIT_STRIDES,
  ALREADY_IN_RIGHT_ORDER,
  BEST_LOCALITY_LOOP_CANNOT_BECOME_INNERMOST,
  NUM_DIAGS
};

// Initialization-list for array can be used, but
// explicit mapping between enum and msg were adopted for readablity.
inline std::array<std::string, NUM_DIAGS> createDiagMap() {
  std::array<std::string, NUM_DIAGS> Map;
  Map[NON_LINEAR_DEF_OR_ALL_UNIT_STRIDES] =
      "MemRefs are in unit stride or non-linear Defs.";
  Map[BEST_LOCALITY_LOOP_CANNOT_BECOME_INNERMOST] =
      "Cannot move best locality loop as innermost.";
  Map[ALREADY_IN_RIGHT_ORDER] =
      "Current Loop nest is already most favorable to locality.";

  return Map;
}

static std::array<std::string, NUM_DIAGS> DiagMap = createDiagMap();

void printDiag(DiagMsg Msg, StringRef FuncName = "",
               const HLLoop *Loop = nullptr,
               StringRef Header = "No Interchange: ", unsigned DiagLevel = 1) {
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  printDiag(PrintDiagFunc, PrintDiagLevel, DiagMap[Msg], FuncName, Loop, Header,
            DiagLevel);
#endif
}

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
static void printRefVector(SmallVectorImpl<const RegDDRef *> &RefVec,
                           std::string Msg = "", bool PrintPointer = false,
                           bool PrintDetail = false) {
  formatted_raw_ostream FOS(dbgs());
  if (Msg.size())
    FOS << Msg << ": " << RefVec.size() << "\n";
  unsigned Count = 0;
  for (const RegDDRef *Ref : RefVec) {
    FOS << Count++ << "\t";
    if (PrintPointer) {
      FOS << Ref << "\t";
    }
    Ref->print(FOS, PrintDetail);
    FOS << " : ";
    (Ref->isLval()) ? FOS << "W" : FOS << "R";
    FOS << "\n";
  }
}
#endif //! defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

typedef std::pair<HLLoop *, HLLoop *> CandidateLoopPair;
typedef DDRefGatherer<RegDDRef, MemRefs> MemRefGatherer;

class HIRLoopInterchange {
public:
  HIRLoopInterchange(HIRFramework &HIRF, HIRDDAnalysis &HDDA,
                     HIRLoopLocality &HLA, HIRSafeReductionAnalysis &HSRA,
                     HIRLoopStatistics &HLS, HIRLoopResource &HLR)
      : HIRF(HIRF), HDDA(HDDA), HLA(HLA), HSRA(HSRA), HLS(HLS), HLR(HLR),
        AnyLoopInterchanged(false), OutmostNestingLevel(-1),
        InnermostNestingLevel(-1), InnermostLoop(nullptr),
        OutermostLoop(nullptr) {}

  bool run(void);

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  void printDVs(void) {
    formatted_raw_ostream FOS(dbgs());
    FOS << "\nDVs: <" << DVs.size() << ">\n";
    for (unsigned I = 0, E = DVs.size(); I != E; ++I) {
      FOS << "DV[" << I << "]: ";
      DVs[I].print(FOS, true);
      FOS << "\n";
    }
  }
#endif

protected:
  HIRFramework &HIRF;
  HIRDDAnalysis &HDDA;
  HIRLoopLocality &HLA;
  HIRSafeReductionAnalysis &HSRA;
  HIRLoopStatistics &HLS;
  HIRLoopResource &HLR;

  bool AnyLoopInterchanged;
  unsigned OutmostNestingLevel;
  unsigned InnermostNestingLevel;
  HLLoop *InnermostLoop;
  HLLoop *OutermostLoop;
  struct CollectCandidateLoops;

  SmallVector<CandidateLoopPair, 12> CandidateLoops;
  SmallVector<const HLLoop *, MaxLoopNestLevel> SortedLoops;
  SmallVector<const HLLoop *, MaxLoopNestLevel> LoopPermutation;
  SmallVector<const HLLoop *, MaxLoopNestLevel> NearByPerm;
  SmallVector<const HLLoop *, 5> PerfectLoopsEnabled;
  SmallVector<DirectionVector, 16> DVs;
  std::map<const HLLoop *, InterchangeIgnorableSymbasesTy>
      CandLoopToIgnorableSymBases;

  bool shouldInterchange(const HLLoop *);
  bool getPermutation(const HLLoop *);

  // returns true means legal for any permutation
  bool isLegalForAnyPermutation(const HLLoop *Loop);
  //  SrcLevel and DstLevel start from 1

  bool isBestLocalityInInnermost(const HLLoop *Loop,
                                 const HLLoop *BestLocalityLoop);
  void getNearbyPermutation(const HLLoop *Loop);
  // SrcLevel and DstLevel start from 1
  void permuteNearBy(unsigned DstLevel, unsigned DstIndex, unsigned SrcLevel,
                     unsigned SrcIndex);
  bool transformLoop(HLLoop *Loop);

  void reportTransformation(LoopOptReportBuilder &LORBuilder);
  bool isInPresentOrder(SmallVectorImpl<const HLLoop *> &LoopNests) const;
};

// Returns true if a ref is in a shape of
//
// DO i1
//    DO i2
//         A[i2][i1]  // 1st dimension = outer dimension
//
// The loop may benefit from interchanging i1 and i2 loops.
// If this function returns true, we may try interchange.
//
// E.g.:
//  A[i2][i1]           returns true  // IVs increases at the outer dimension.
//  A[i3][i2][i1]       returns true
//  A[i2][i1][i3]       returns true
//  A[i3][i1][i2]       returns true
//  A[i1][i2][i3]       returns false
//  A[i2][i1 + 7]       returns true  // no special handling for const
//  A[5*i2][3*i1]       returns true  // no special handling for const coef
//  A[5*i2 + 2][3*i1]   returns true
//  A[i1][i1]           returns false // not increasing at outer dimensions
//  A[i1][i2]           returns false // not increasing at outer dimensions
//  A[b2*i2][b1*i1]     returns false // bails out with blob coeffs
//  A[i1 + i3][i2]      returns false // bails out with more than one IVs
//                                    // in one dimension
//  A[b*i2][i3][i1]     returns true  // due to "[i3][i1]"
//  A[i3][b*i1][i2]     returns true  // due to "[i3]..[i2]"
//
// TODO: Handling of more than one IVs in one dimension.
// E.g.
//   DO i1
//    DO i2
//       A[i1 + 2*i2]
//    END DO
//   END DO
// Interchanging i1 and i2 is useful.
//
bool areIVsIncreasingWithOuterDimensions(RegDDRef &Ref) {
  unsigned NodeLevel = Ref.getNodeLevel();
  unsigned MinLevelSoFar = MaxLoopNestLevel + 1;

  // Scan from the innermost dimension
  for (int I = 1, E = Ref.getNumDimensions(); I <= E; I++) {
    const CanonExpr *CE = Ref.getDimensionIndex(I);
    // Inspecting IVs in one dimension.

    // Upto one IVs with a constant coef.
    if (!CE->isLinearAtLevel(NodeLevel)) {
      continue;
    }
    // See if zero or 1(constant coeff only) IV's contained.
    // If only one IV with constant coeff exists, make sure
    // the level is larger than that of IV of previous dimension.
    // Currently, non-1 constant coeff of IV is not specially treated.
    unsigned LevelFound = 0;
    for (unsigned Level = 1; Level <= NodeLevel; Level++) {
      unsigned Index = 0;
      int64_t Coeff = 0;
      CE->getIVCoeff(Level, &Index, &Coeff);

      if (!Coeff) {
        continue;
      }

      if (Index != InvalidBlobIndex || LevelFound != 0) {
        LevelFound = 0;
        break;
      }

      LevelFound = Level;
    }

    // LevelFound = 0 if any of the following conditions is true.
    // 1. no IV is found in this dim.
    // 2. more than one IV is found.
    // 3. At least one IV has blob coeff.
    // In those cases, we never return true
    // at this moment but defer decision to next dimension.
    if (LevelFound > MinLevelSoFar) {
      // One IV with a const coef found
      return true; // may interchange
    } else if (LevelFound > 0) {
      // In case LevelFound == 0, we keep previously set MinLevelSoFar.
      MinLevelSoFar = LevelFound;
    }
  }

  return false; // do not try interchange
}

bool isSinkedPerfectLoopProfitableForInterchange(const HLLoop *OutermostLoop,
                                                 const HLLoop *InnermostLoop) {
  // Same as the existing logic.
  // TODO: Consider replacing the following logic with
  //       areIVsIncreasingWithOuterDimensions(RegDDRef &Ref)
  //       This replacement should be done with the study of the
  //       logic's interaction with locality util and/or its usage within
  //       HIRLoopInterchange. Also, consider working with fixing matmul6.ll.
  // E.G.
  //   DO i1
  //     DO i2
  //       DO i3
  //          a[i2][i1][i3]
  //  Intention is to interchange ( 1 2 3 ) --> ( 2 1 3 ).

  // If a constant TC is too small,
  // avoid aggressive interchange.
  // Scan TC from innermost's parent loop to outermost loop.
  const HLLoop *ParentLp = InnermostLoop->getParentLoop();
  const HLLoop *OutParentLp = OutermostLoop->getParentLoop();
  for (const HLLoop *Lp = ParentLp; Lp != OutParentLp;
       Lp = Lp->getParentLoop()) {
    uint64_t TripCount = -1;
    if (Lp->isConstTripLoop(&TripCount) &&
        TripCount < SinkedPerfectLoopProfitablityTCThreshold) {
      return false;
    }
  }

  // Examine MemRefs in Pre/postloop or prehead/postexit of the innermost.
  // Recursive = true (pre/postexit), RecursiveInsidedLoop = false (no child)
  MemRefGatherer::VectorTy Refs;
  MemRefGatherer::gatherRange<true, false>(ParentLp->child_begin(),
                                           ParentLp->child_end(), Refs);
  bool MayInterchange = false;
  for (RegDDRef *Ref : Refs) {
    LLVM_DEBUG(Ref->dump());
    MayInterchange = areIVsIncreasingWithOuterDimensions(*Ref);
    if (MayInterchange) {
      break;
    }
  }

  LLVM_DEBUG(dbgs() << "MayInterchange: " << MayInterchange << "\n";);

  return MayInterchange;
}

} // namespace

/// Gather all perfect Loop Nest and enable near perfect one if needed
struct HIRLoopInterchange::CollectCandidateLoops final
    : public HLNodeVisitorBase {

  HIRLoopInterchange &LIP;
  SmallVectorImpl<CandidateLoopPair> &CandidateLoops;
  HIRDDAnalysis &DDA;
  HLNode *SkipNode;
  StringRef FuncName;

  CollectCandidateLoops(HIRLoopInterchange &LoopIP,
                        SmallVectorImpl<CandidateLoopPair> &CandidateLoops,
                        HIRDDAnalysis &DDA, StringRef FuncName)
      : LIP(LoopIP), CandidateLoops(CandidateLoops), DDA(DDA),
        SkipNode(nullptr), FuncName(FuncName) {}

  void visit(HLLoop *Loop) {
    // Gather perfect loop nests
    // TODO: Skip Loop with hasUserCall when flag is sets
    const HLLoop *InnermostLoop = nullptr;

    if (Loop->isInnermost()) {
      SkipNode = Loop;
      return;
    }
    LLVM_DEBUG(dbgs() << "In collect Perfect loopnest\n");
    // Allow Triangular loop, allow Near Perfect loop (and return the result).
    bool IsPerfectNest =
        HLNodeUtils::isPerfectLoopNest(Loop, &InnermostLoop, false);

    if (!IsPerfectNest) {
      // Do not skip recursion.
      // We might find a perfect loop nest starting from an inner loop.
      return;
    }

    if (LIP.HLS.getSelfLoopStatistics(InnermostLoop)
            .hasCallsWithUnsafeSideEffects()) {
      LLVM_DEBUG(
          dbgs() << "\nSkipping loop with calls that have side effects\n");
      SkipNode = Loop;
      return;
    }

    for (const HLLoop *TmpLoop = InnermostLoop,
                      *EndLoop = Loop->getParentLoop();
         TmpLoop != EndLoop; TmpLoop = TmpLoop->getParentLoop()) {
      if (TmpLoop->hasUnrollEnablingPragma() ||
          TmpLoop->hasUnrollAndJamEnablingPragma() ||
          TmpLoop->hasVectorizeEnablingPragma()) {
        LLVM_DEBUG(dbgs() << "\nSkipping loop with unroll/vector pragma\n");
        SkipNode = Loop;
        return;
      }
    }

    if (IsPerfectNest) {
      // If the innermost loop is undosinking candidate, it was a near perfect
      // loop

      if (InnermostLoop->isUndoSinkingCandidate()) {
        if (isSinkedPerfectLoopProfitableForInterchange(Loop, InnermostLoop)) {
          CandidateLoops.push_back(
              std::make_pair(Loop, const_cast<HLLoop *>(InnermostLoop)));
          LIP.PerfectLoopsEnabled.push_back(InnermostLoop);
        }
      }

      LLVM_DEBUG(dbgs() << "\nIs Perfect Nest\n");

      if (!HLNodeUtils::hasNonUnitStrideRefs(InnermostLoop)) {
        printDiag(NON_LINEAR_DEF_OR_ALL_UNIT_STRIDES, FuncName, Loop);
      } else {
        LLVM_DEBUG(dbgs() << "\nHas non unit stride\n");
        CandidateLoopPair LoopPair =
            std::make_pair(Loop, const_cast<HLLoop *>(InnermostLoop));
        if (std::find(CandidateLoops.begin(), CandidateLoops.end(), LoopPair) ==
            CandidateLoops.end()) {
          CandidateLoops.push_back(
              std::make_pair(Loop, const_cast<HLLoop *>(InnermostLoop)));
        }
      }

      SkipNode = Loop;
      return;
    }
  }

  void visit(HLNode *Node) {}
  void postVisit(HLNode *Node) {}
  bool skipRecursion(const HLNode *Node) const { return Node == SkipNode; }
};

// HIR loop Special interchange is designed to
// 1. target perfect loop nests which can then enable tiling;
// 2. create more non-unit strides that differs from normal interchange where
//    profitability is driven by unit stride;
//    and
// 3. enable innermost-loop vectorization. The original innermost-loop refs have
//    mod function for the loop iv. Without HSI, this loop cannot be vectorized.
//
// In addition, the target loopnest has:
// - 3 levels of perfect loop nest,
//   and
// - a huge innermost-loop body, with certain number and ratio of memref op and
//   arith op.
//
class HIRSpecialLoopInterchange : public HIRLoopInterchange {
  HLNodeUtils &HNU;
  SmallSet<unsigned, 4> IndependentLoopLevelSet;
  HIRLoopLocality::RefGroupVecTy EqualityGroups;
  SmallSet<unsigned, 8> UniqueGroupSymbases;
  SmallSet<unsigned, 8> SBS;
  // Identify the candidate loopnest(s) that are suitable for special
  // interchange.
  SmallVector<CandidateLoopPair, 4> CandidateLoopPairs;

  // For each Loopnest in the region, collect each (OutermostLp, InnermostLp)
  // pair and identify the target loopnest for special interchange
  // transformation.
  bool collect();

  // Ensure a perfect loopnest exist (or make it a perfect loopnest if possible)
  bool makePerfectLoopnest(HLLoop *OuterLp, HLLoop *InnerLp);

  bool sinkForPerfectLoopnest(HLLoop *OuterLp, HLLoop *InnerLp);

  // Analyze the loopnest, save loopnest statistics + etc. info for later use in
  // profit analysis.
  bool analyzeLoop(HLLoop *InnerLp);

  // Perform array scalarization to cleanup certain memref(s) in the loopnest
  bool doCleanup(HLLoop *InnerLp);

  // Use a heuristic-based profit model targeting the special interchange
  bool isProfitable(HLLoop *InnerLp);

  // Identify target innermost level
  // - check all mod functions (%) and their relevance with loop level within
  //   the loop nest.
  // - the loop level that doesn't associate with any mod function will become
  //   the new innermost level.
  bool identifyTargetInnermostLevel(HLLoop *OuterLp, HLLoop *InnerLp);

  // Generate the target permutation for the special interchange.
  bool generatePermutation(HLLoop *OuterLp, HLLoop *InnerLp);

  // Peggy back to original Interchange pass's legal model
  bool isLegal(HLLoop *OuterLp);

  // Peggy back to original Interchange pass's transform function
  bool transform(HLLoop *OuterLp);

  void clearData(void) {
    SortedLoops.clear();
    IndependentLoopLevelSet.clear();
    EqualityGroups.clear();
    UniqueGroupSymbases.clear();
    SBS.clear();
    CandidateLoopPairs.clear();
  }

public:
  HIRSpecialLoopInterchange(HIRFramework &HIRF, HIRDDAnalysis &HDDA,
                            HIRLoopLocality &HLA,
                            HIRSafeReductionAnalysis &HSRA,
                            HIRLoopStatistics &HLS, HIRLoopResource &HLR)
      : HIRLoopInterchange(HIRF, HDDA, HLA, HSRA, HLS, HLR),
        HNU(HIRF.getHLNodeUtils()) {}

  bool run(void);
};

bool HIRSpecialLoopInterchange::collect(void) {
  SmallVector<HLLoop *, 64> InnermostLoops;
  HNU.gatherInnermostLoops(InnermostLoops);
  if (InnermostLoops.empty()) {
    LLVM_DEBUG(dbgs() << "No loop available\n";);
    return false;
  }

  for (HLLoop *TheInnermostLp : InnermostLoops) {
    unsigned Count = 0;
    HLLoop *OuterLp = TheInnermostLp;
    bool IsCandidate = true;
    while (Count < SpecialInterchangeExpectedNestingDepth) {
      OuterLp = OuterLp->getParentLoop();
      if (!OuterLp) {
        LLVM_DEBUG(dbgs() << "Expect a non-null OuterLp. This innermost lp is "
                             "not a candidate\n";);
        IsCandidate = false;
        break;
      }
      ++Count;
    }
    // Skip the current InnermostLp if IsCandidate is false
    if (!IsCandidate) {
      continue;
    }

    if (isInRange<unsigned>(TheInnermostLp->getNumChildren(),
                            SpecialInterchangeLpSizeThresholdLB,
                            SpecialInterchangeLpSizeThresholdUB)) {
      CandidateLoopPairs.push_back(std::make_pair(OuterLp, TheInnermostLp));
    }
  }

  // ExpectNumSpecialInterchangeLoopnests: default to 1
  if (CandidateLoopPairs.size() != ExpectNumSpecialInterchangeLoopnests) {
    LLVM_DEBUG(dbgs() << "Expect only 1 suitable loopnest\n";);
    return false;
  }

  LLVM_DEBUG({
    dbgs() << "Loopnest(s) collected: " << CandidateLoopPairs.size() << "\n";
    HLLoop *OuterLp = CandidateLoopPairs[0].first;
    OuterLp->dump();
  });

  return true;
}

// Handle cases similar to the loopnest below:
// .. ..
//   %1442 = %720  *  5.000000e-01; /* 5 prehdr instructions */
//   %1443 =  - %1442;
//   %1444 = %720  *  %7;
//   %1445 = %1444  *  2.000000e+00;*** ******** ***
//   %1446 = %1445  *  %501;
// + DO i2 = 0, sext.i32.i64(%6) + -1, 1   <DO_LOOP>
// |   + DO i3 = 0, sext.i32.i64(%3) + -1, 1   <DO_LOOP>
// |   |      %1472 = i3 + 1  %  %3;         *** to sink:    ***
// |   |      %1476 = i3 + %3 + -1  %  %3;   *** ops: mod(%) ***
// |   |   + DO i4 = 0, sext.i32.i64(%2) + -1, 1   <DO_LOOP>
// |   |   |   ...
// |   |   |   use(s) of %1472, %1476, %1442, etc.
// |   |   |   ...
// |   |   + END LOOP
// |   + END LOOP
// + END LOOP
//
// [Notes]
// - The loopnest is perfect except the preheader of the innermost loop.
// - The statements include mod(%) operator. It s not covered by copy, load, and
//   store, thus existing DDUtils::enablePerfectLoopNest(.) can't handle this.
// - Better to write code to do special sinking for the relevant case.
//
bool HIRSpecialLoopInterchange::sinkForPerfectLoopnest(HLLoop *OuterLp,
                                                       HLLoop *InnerLp) {

  // Check the [OuterLp, InnerLp] formed loonest is near perfect,
  // except code in preheader of InnerLp.
  auto CheckLoopNestSanity = [](HLLoop *InnerLp) -> bool {
    // Check InnerLp:
    if (!InnerLp->hasPreheader() || InnerLp->hasPostexit()) {
      LLVM_DEBUG(dbgs() << "Expect InnerLp have only non-empty preheader\n";);
      return false;
    }

    // Only allow certain instruction types in InnerLp's preheader:
    for (auto I = InnerLp->pre_begin(), E = InnerLp->pre_end(); I != E; ++I) {
      HLInst *HInst = cast<HLInst>(I);

      // Only support the following instruction type: %(mod)
      const unsigned Opcode = HInst->getLLVMInstruction()->getOpcode();
      if (!(Opcode == Instruction::SRem || Opcode == Instruction::URem)) {
        LLVM_DEBUG(dbgs() << "Encounter unsupported instruction type\n";
                   HInst->dump(););
        return false;
      }
    }

    return true;
  };

  // Do legal that HInst* (a mod operator) can sink into SinkLp
  //
  // It is legal to sink the HInst* into SinkLp if&f the HInst* remains loop-inv
  // after sinking.
  //
  // That is :
  // (1) HInst's Lval Ref is not redefined in SinkLp
  // and
  // (2) non of HInst's Rval Blob is redefined in SinkLp
  //
  auto DoLegalTestForSinking = [&](SmallVectorImpl<HLInst *> &PreLoopInsts,
                                   HLLoop *SinkLp, DDGraph &DDG) {
    SmallVector<DDRef *, 8> RefVec;

    for (auto *I : PreLoopInsts) {
      RefVec.push_back(I->getLvalDDRef());

      // Collect Rval blob:
      for (const RegDDRef *UseRef :
           make_range(I->rval_op_ddref_begin(), I->rval_op_ddref_end())) {

        if (UseRef->isSelfBlob()) {
          RefVec.push_back(const_cast<RegDDRef *>(UseRef));
        } else {
          for (auto *BRef :
               make_range(UseRef->blob_begin(), UseRef->blob_end())) {
            RefVec.push_back(const_cast<BlobDDRef *>(BRef));
          }
        }
      }
    }

    // *** Search any Ref's redefinition(s) in SinkLp:
    for (auto &Ref : RefVec) {
      unsigned LvalCount = 0, RvalCount = 0;
      if (DDUtils::countEdgeToLoop(DDG, Ref, SinkLp, LvalCount, RvalCount) &&
          LvalCount) {
        LLVM_DEBUG({
          dbgs() << "LvalCount: " << LvalCount << "\tRvalCount: " << RvalCount
                 << "\nFound Ref's redefinition in Lp. Offending Ref: ";
          Ref->dump(1);
          dbgs() << "\n";
        });
        return false;
      }
    }

    return true;
  };

  // ****  BEGIN FUNCTION: after the lambdas ****

  // Sanity check that the loopnest has the following properties:
  // - none-perfectness is due to only statements in InnerLp's preheader.
  if (!CheckLoopNestSanity(InnerLp)) {
    LLVM_DEBUG(dbgs() << "Fail in CheckLoopNestSanity(InnerLp)\n";);
    return false;
  }

  // Collect:
  SmallVector<HLInst *, 8> PreLoopInsts;
  for (auto I = InnerLp->pre_begin(), E = InnerLp->pre_end(); I != E; ++I) {
    PreLoopInsts.push_back(cast<HLInst>(I));
  }

  if (PreLoopInsts.empty()) {
    LLVM_DEBUG(dbgs() << "Nothing collected\n";);
    return false;
  }

  // Legal test:
  DDGraph DDG = HDDA.getGraph(InnerLp->getParentLoop());
  if (!DoLegalTestForSinking(PreLoopInsts, InnerLp, DDG)) {
    LLVM_DEBUG(dbgs() << "Fail DoLegalTestForSinking(.)\n";);
    return false;
  }

  // Do transformation: sink the statements from InnerLp's preheader into the
  // InnerLp.
  //
  // E.g.
  // [Before]
  //   ...
  // + DO i2 = 0, sext.i32.i64(%6) + -1, 1   <DO_LOOP>
  // |   + DO i3 = 0, sext.i32.i64(%3) + -1, 1   <DO_LOOP>
  // |   |      %1472 = i3 + 1  %  %3;
  // |   |      %1476 = i3 + %3 + -1  %  %3;
  // |   |   + DO i4 = 0, sext.i32.i64(%2) + -1, 1   <DO_LOOP>
  // |   |   |   ...
  // |   |   + END LOOP
  // |   + END LOOP
  // + END LOOP
  //
  // [After]
  //   ...
  // + DO i2 = 0, sext.i32.i64(%6) + -1, 1   <DO_LOOP>
  // |   + DO i3 = 0, sext.i32.i64(%3) + -1, 1   <DO_LOOP>
  // |   |   + DO i4 = 0, sext.i32.i64(%2) + -1, 1   <DO_LOOP>
  // |   |   |   %1472 = i3 + 1  %  %3;
  // |   |   |   %1476 = i3 + %3 + -1  %  %3;
  // |   |   |   ...
  // |   |   + END LOOP
  // |   + END LOOP
  // + END LOOP

  // - Update def@level for each use:
  unsigned const TargetLevel = InnerLp->getNestingLevel();
  for (auto *I : PreLoopInsts) {
    DDRef *DDRefSrc = I->getLvalDDRef();
    LLVM_DEBUG({
      dbgs() << "Src: ";
      I->dump();
      dbgs() << "DDRefSink(s): <" << DDG.getNumOutgoingEdges(DDRefSrc)
             << ">: \n";
    });
    for (auto II = DDG.outgoing_edges_begin(DDRefSrc),
              IE = DDG.outgoing_edges_end(DDRefSrc);
         II != IE; ++II) {
      DDEdge *Edge = (*II);
      if (RegDDRef *RRef = dyn_cast<RegDDRef>(Edge->getSink())) {
        RRef->updateDefLevel(TargetLevel);
      } else if (BlobDDRef *BRef = dyn_cast<BlobDDRef>(Edge->getSink())) {
        BRef->setDefinedAtLevel(TargetLevel);
      }
    }
  }

  // -Move each instruction from InnerLp's preheader into its body:
  for (auto I = PreLoopInsts.rbegin(), E = PreLoopInsts.rend(); I != E; ++I) {
    HLNodeUtils::moveAsFirstChild(InnerLp, *I);
    DDUtils::updateLiveinsLiveoutsForSinkedInst(InnerLp, *I, true);
  }

  // Update the temp DDRefs from linear-at-level to non-linear
  DDUtils::updateDDRefsLinearity(PreLoopInsts, DDG);
  HIRInvalidationUtils::invalidateBody(InnerLp);
  HIRInvalidationUtils::invalidateBody(InnerLp->getParentLoop());

  return true;
}

bool HIRSpecialLoopInterchange::makePerfectLoopnest(HLLoop *OuterLp,
                                                    HLLoop *InnerLp) {
  // Check if a perfect loopnest already exists
  bool IsNearPerfectLoopNest = false;
  if (HLNodeUtils::isPerfectLoopNest(OuterLp, nullptr, false,
                                     &IsNearPerfectLoopNest)) {
    return true;
  }
  if (!IsNearPerfectLoopNest) {
    return false;
  }

  // Do Special-interchange specific sinking:
  if (!sinkForPerfectLoopnest(OuterLp, InnerLp)) {
    LLVM_DEBUG(dbgs() << "Fail to create a perfect loopnest using "
                         "sinkForPerfectLoopnest(.) for (OuterLp,InnerLp)\n";);
    return false;
  }

  // Verify: a perfect loopnest is obtained through explicit sinking
  // This is need for interchange.
  if (HLNodeUtils::isPerfectLoopNest(OuterLp)) {
    LLVM_DEBUG(dbgs() << "Reached Perfect Loopnest\n";);
    return true;
  }

  return false;
}

bool HIRSpecialLoopInterchange::analyzeLoop(HLLoop *InnerLp) {
  // Collect equal MemRef group(s) from InnermostLp:
  HIRLoopLocality::populateEqualityGroups(InnerLp->child_begin(),
                                          InnerLp->child_end(), EqualityGroups,
                                          &UniqueGroupSymbases);
  if (EqualityGroups.empty()) {
    LLVM_DEBUG(dbgs() << "No MemRef(s) in Lp: " << InnerLp << "\n";);
    return false;
  }

  // Collect symbases
  unsigned ConstOnlySubsMemRefGroupCount = 0;
  for (auto &RefVec : EqualityGroups) {
    if (RefVec.empty() || !DDRefUtils::isMemRefAllDimsConstOnly(RefVec[0])) {
      continue;
    }
    const unsigned SB = RefVec[0]->getSymbase();
    SBS.insert(SB);

    LLVM_DEBUG({
      printRefVector(RefVec, "RefVec: ");
      dbgs() << "SB: " << SB << "\n";
    });

    ++ConstOnlySubsMemRefGroupCount;
  }

  if (SBS.empty() || (ConstOnlySubsMemRefGroupCount == 0)) {
    LLVM_DEBUG(dbgs() << "No suitable symbase(s) in loop: " << InnerLp
                      << "\n";);
    return false;
  }

  LLVM_DEBUG({
    dbgs() << "SBSet size: " << SBS.size() << "\n";
    std::for_each(SBS.begin(), SBS.end(),
                  [](unsigned SB) { dbgs() << SB << ","; });
    dbgs() << "\n"
           << "SuitableGroupCount: " << ConstOnlySubsMemRefGroupCount << "\n";
  });

  return true;
}

bool HIRSpecialLoopInterchange::doCleanup(HLLoop *InnerLp) {
  // Collection and analysis are conducted inside analyzeLoop()
  // - SBS is obtained from analyzeLoop()
  if (!HIRTransformUtils::doScalarization(HIRF, HDDA, InnerLp, SBS)) {
    LLVM_DEBUG(dbgs() << "Failure in innermostLp array scalarization\n";);
    return false;
  }

  HIRInvalidationUtils::invalidateBody<HIRSafeReductionAnalysis>(InnerLp);
  return true;
}

bool HIRSpecialLoopInterchange::isProfitable(HLLoop *InnerLp) {
  // Check:
  // - MemRef ops, arithmetic ops, and their relative ratios
  //
  // 1. # memrefs (load(s) and store(s) for both int and float types);
  // 2. # arith operations (+/-/*/.. etc.)
  // 3. ratio: memref ops vs. arith ops
  //
  // When all conditions meet, qualify this InnerLp as a profitable case for
  // special loop interchange.
  //
  auto &TLR = HLR.getTotalLoopResource(InnerLp);
  const unsigned NumOps = TLR.getNumIntAndFPOps();
  const unsigned NumMemOps = TLR.getNumMemOps();
  float ArithToMemRatio = 0.0000001f;
  if (NumMemOps) {
    ArithToMemRatio = (float)NumOps / (float)NumMemOps;
  }

  LLVM_DEBUG({
    dbgs() << "InnermostLp: \n";
    HLR.getTotalLoopResource(InnerLp).dump(InnerLp);
    dbgs() << "Total MemRefs: " << NumMemOps << "\n"
           << "Total Ops: " << NumOps << "\n"
           << "Arith-Ops / Mem-Ops Ratio: " << ArithToMemRatio << "\n";
  });

  return ((NumOps > SpecialInterchangeArithOpNumThreadshold) &&
          (NumMemOps > SpecialInterchangeMemOpNumThreadshold) &&
          (ArithToMemRatio > SpecialInterchangeArithToMemOpRatioThreadshold));
}

// Analyze each mod (%) instruction for its relevance to loop IV(s) and its
// use(s) in address computation for Refs.
//
// e.g. some mod instructions and their potential uses in memrefs:
// ...
// <867>  %mod55 = i2 + %"mat_times_vec_$NY_fetch36" + -1  %
//                 %"mat_times_vec_$NY_fetch36";
// ...
// <874>  %mod66 = i3 + %"mat_times_vec_$NX_fetch34" + -1  %
//                 %"mat_times_vec_$NX_fetch34";
// ...
// <1158> %mod62.i.i = %mod66 + 1  %  %"mat_times_vec_$NX_fetch34";
// ...
// <1165> %"jacobian_v_$Q[]144[][][]_fetch.i.i" =
//              (%"mat_times_vec_$Q")[i1 + 2][%mod.i.i22331][%mod62.i.i +
//              1][0];
// ...
// <2461> %"mat_times_vec_$X[]11327[][][]_fetch" =
//                  (%"mat_times_vec_$X")[%mod55 + 1][i3][0][i1 + 1];
//
// Check:
// 1. a mod instruction is used somewhere in at least 1 ref;
//    E.g. %mod55 is used in %"mat_times_vec_$X[]11327[][][]_fetch" =
//                            (%"mat_times_vec_$X")[%mod55 + 1][i3][0][i1 +
//                            1];
//
// 2. a mod instruction is associated with some loop-level iv, as:
// ...
// %mod66:         i3
// ...
// %mod62.i.i:     -> %mod66:    i3
//
// --------------------------------------------------------------------------
// This function will
// - collect all mod (%) instruction within the innermost level
// - for each mod instruction, check it is associated loop-level IV(s)
//   and
//   check the mod instruction's ultimate use(s) in memref(s)
// - select those remaining (independent) IV level(s) as the target of the
//   special loop interchange
//
bool HIRSpecialLoopInterchange::identifyTargetInnermostLevel(HLLoop *OuterLp,
                                                             HLLoop *InnerLp) {

  // Check if CE contains any blob whose Index is in KnownBlobMap
  auto CheckBlob = [&](const CanonExpr *CE,
                       DenseMap<unsigned, bool> &KnownBlobMap) -> bool {
    for (auto Blob : make_range(CE->blob_begin(), CE->blob_end())) {
      // Check Blob.Coeff and Blob.Index:
      // LLVM_DEBUG(dbgs() << "Blob.coeff: " << Blob.Coeff
      //                   << "\tBlob.Index: " << Blob.Index << "\n";);
      if (KnownBlobMap[Blob.Index]) {
        return true;
      }
    }
    return false;
  };

  // Check if the given mod(%) HLInst* is associated to any IV(s).
  // - Collect all relevant loop level(s) into the LevelSet
  //
  // [Note]
  // - the mod (%) HLInst* of interest are all in the form of CE0 = CE1 % CE2
  // where:
  //  CE0: blob only
  //  CE1: with iv, possibly blob and const
  //  CE2: blob only
  //
  auto CheckModInstAssociatedLoopLevel =
      [&](const HLInst *HInst,
          const SmallSet<unsigned, 4> &LoopLevels /* Input */,
          DenseMap<unsigned, bool> &KnownBlobMap /* Input+Output */,
          SmallSet<unsigned, 4> &LevelSet /* Output */) -> bool {
    bool Result = false;

    // Check: Lval is a self blob, save its blob index
    const RegDDRef *DestRef = HInst->getLvalDDRef();
    if (!DestRef->isSelfBlob()) {
      LLVM_DEBUG(dbgs() << "\nExpect Lval be a self blob\n";);
      return false;
    }
    unsigned DestBlobIndex =
        DestRef->getSingleCanonExpr()->getSingleBlobIndex();
    LLVM_DEBUG({
      dbgs() << "\nregular HInst dump -- (a mod (%) inst):\n";
      HInst->dump();
      dbgs() << "DestBlobIndex: " << DestBlobIndex << "\n";
    });

    // Scan the Rvals:
    for (auto *Ref :
         make_range(HInst->rval_op_ddref_begin(), HInst->rval_op_ddref_end())) {
      if (!Ref->isTerminalRef()) {
        continue;
      }

      // Ref is composed of a single CanonExpr:
      const CanonExpr *CE = Ref->getSingleCanonExpr();
      LLVM_DEBUG(dbgs() << "Rval CE: "; CE->dump(); dbgs() << "\n";);
      if (CheckBlob(CE, KnownBlobMap)) {
        Result = true;
        KnownBlobMap[DestBlobIndex] = true;
        continue;
      }

      if (!CE->hasIV()) {
        continue;
      }

      // Check any the associated loop level(s):
      LLVM_DEBUG(dbgs() << "has loop iv, CE: "; CE->dump(); dbgs() << "\n";);
      for (auto Level : LoopLevels) {
        if (CE->hasIV(Level)) {
          LevelSet.insert(Level);
          Result = true;
          KnownBlobMap[DestBlobIndex] = true;
        }
      }
    }

    return Result;
  };

  // Check if the other end of the edge is used in an address computation by
  // checking if the corresponding HLInst is a Load, Store, GEPT, or
  // Subscript.
  auto CheckEdgeInAddrComp = [&](const DDEdge *Edge,
                                 bool IsOutgoingEdge) -> bool {
    const HLInst *UseInst =
        IsOutgoingEdge ? dyn_cast<HLInst>(Edge->getSink()->getHLDDNode())
                       : dyn_cast<HLInst>(Edge->getSrc()->getHLDDNode());
    if (!UseInst) {
      return false;
    }

    // See the edge and related info:
    LLVM_DEBUG({
      dbgs() << "Edge: ";
      Edge->print(dbgs());
      dbgs() << "UseInst: ";
      UseInst->dump();
      const DDRef *UseRef = IsOutgoingEdge ? Edge->getSink() : Edge->getSrc();
      dbgs() << "UseRef: ";
      UseRef->dump();
      dbgs() << "\n--------------------------\n";
    });

    const Instruction *LLVMInst = UseInst->getLLVMInstruction();
    return (isa<LoadInst>(LLVMInst) || isa<StoreInst>(LLVMInst) ||
            isa<GetElementPtrInst>(LLVMInst) || isa<SubscriptInst>(LLVMInst));
  };

  // Check if the given HLInst* is used in any address computation.
  //
  // [Note]
  // - Can't use HLInst->getLLVMInstruction(), since the HLInst and its
  //   associated LLVM Instruction is decoupled.
  //   But, it is ok to check HLInst->getLLVMInstruction()->getOpCode().
  // - Use DDGraph, for each edge from the HLInst, analyze the edge's other
  // end.
  //
  auto CheckModInstUsedInRef = [&](const HLInst *HInst, DDGraph &DDG) -> bool {
    bool Result = true;
    const RegDDRef *LvalRef = HInst->getLvalDDRef();

    // Iterate over each outgoing edge:
    for (auto II = DDG.outgoing_edges_begin(LvalRef),
              EE = DDG.outgoing_edges_end(LvalRef);
         II != EE; ++II) {
      Result = CheckEdgeInAddrComp(*II, true) && Result;
    }

    // Iterate over each incoming edge:
    for (auto II = DDG.incoming_edges_begin(LvalRef),
              EE = DDG.incoming_edges_end(LvalRef);
         II != EE; ++II) {
      Result = CheckEdgeInAddrComp(*II, false) && Result;
    }

    return Result;
  };

  // ** BEGIN OF FUNCTION BODY : identifyTargetInnermostLevel() **
  SmallVector<const HLInst *, 8> ModInstVec;
  SmallSet<unsigned, 4> LoopLevels;
  DenseMap<unsigned, bool> KnownBlobMap;

  // Collect all loop levels from the loop pair:
  for (unsigned I = OuterLp->getNestingLevel(), E = InnerLp->getNestingLevel();
       I <= E; ++I) {
    LoopLevels.insert(I);
  }
  LLVM_DEBUG({ printSmallSetInt(LoopLevels, "LoopLevels"); });

  // Collect all rem (%) HLInst* from innermost lp:
  for (const HLNode &Node :
       make_range(InnerLp->child_begin(), InnerLp->child_end())) {
    const HLInst *HInst = cast<HLInst>(&Node);
    const unsigned Opcode = HInst->getLLVMInstruction()->getOpcode();
    if (Opcode == Instruction::SRem || Opcode == Instruction::URem) {
      ModInstVec.push_back(HInst);
    }
  }

  if (ModInstVec.empty()) {
    LLVM_DEBUG(dbgs() << "No rem (%) instruction in innermost lp\n";);
    return false;
  }

  // See all collected rem (%) HLIst(s):
  LLVM_DEBUG({
    dbgs() << "Mod HLInst(s): " << ModInstVec.size() << "\n";
    for (auto *I : ModInstVec) {
      I->dump();
    }
  });

  // For each % instruction, check:
  // - it is associated to some loop-level IV(s)
  // and
  // - it is used in address computation
  //
  // [Note]
  // - After hte loop, the final result is in LoopLevelsAssociated.
  //
  DDGraph DDG = HDDA.getGraph(InnerLp);
  SmallSet<unsigned, 4> LoopLevelsAssociated;
  for (auto *HModInst : ModInstVec) {
    // Is the ModInst is used in at least 1 address computation?
    if (!CheckModInstUsedInRef(HModInst, DDG)) {
      LLVM_DEBUG(dbgs() << "ModInst is NOT used in any address computation\n";);
      continue;
    }

    // Is the ModInst is associated to at least 1 IV?
    if (!CheckModInstAssociatedLoopLevel(HModInst, LoopLevels, /* Input */
                                         KnownBlobMap /* Input + Output */,
                                         LoopLevelsAssociated /* Output */)) {
      LLVM_DEBUG(dbgs() << "ModInst is NOT associated with any loop level\n";);
    }
  }

  LLVM_DEBUG({
    printSmallSetInt(LoopLevelsAssociated,
                     "Loop Levels associated with Mod (%) Insts");
  });

  // Obtain remaining (independent) IV level(s):
  // IndepLevels = AllLevels - AssociatedLevels
  IndependentLoopLevelSet =
      llvm::set_difference(LoopLevels, LoopLevelsAssociated);
  LLVM_DEBUG({
    printSmallSetInt(IndependentLoopLevelSet, "Independent Loop Level Set");
  });

  // Expect only 1 independent loop level:
  return (IndependentLoopLevelSet.size() ==
          SpecialInterchangeExpectedModIndependentLoops);
}

// [Actions]
// - construct the target permutation using the inner-most loop level obtained
//   from analysis.
//
// [note]
// - IndependentLoopLevelSet contains levels that can become the new
// inner-most
//   level.
//   E.g. IndependentLoopLevelSet = {1}, level 1 will become the
//   inner-most loop level in the target permutation.
//
bool HIRSpecialLoopInterchange::generatePermutation(HLLoop *OuterLp,
                                                    HLLoop *InnerLp) {
  // Obtain Original Permutation
  HLLoop *Lp = InnerLp;
  const signed OutermostLevel = OuterLp->getNestingLevel();
  signed CurLevel = Lp->getNestingLevel();
  while (CurLevel >= OutermostLevel) {
    SortedLoops.push_back(Lp);
    Lp = Lp->getParentLoop();
    --CurLevel;
  }
  std::reverse(SortedLoops.begin(), SortedLoops.end());

  // Print expected Original Permutation. E.g. (1 2 3)
  LLVM_DEBUG({
    dbgs() << "Original Permutation, Levels in loop nest:\n(";
    for (unsigned I = 0, E = SortedLoops.size(); I != E; ++I) {
      dbgs() << SortedLoops[I]->getNestingLevel() << " ";
    }
    dbgs() << ")\n";
  });

  // Construct the target permutation.
  //
  // For input permutation (1, 2, 3), expect target permutation: (2, 3, 1).
  // This is because IndependentLoopLevelSet = {1}, only the level 1 loop
  // can be moved to the innermost level, while all other levels are
  // maintained as is in their existing relative order. There are mod (%)
  // instructions associated to iv with level 2 and 3, but not on level 1.
  // Moving level 1 to innermost still has linear refs for the innermost loop,
  // which can further enable blocking and vectorization.
  //
  // Loopnest permutation order: (1 2 3) -> (2 3 1)
  //
  const unsigned TargetInnermostLevel = *IndependentLoopLevelSet.begin();
  LLVM_DEBUG(dbgs() << "TargetInnermostLevel: " << TargetInnermostLevel
                    << "\n";);
  auto It = std::find_if(SortedLoops.begin(), SortedLoops.end(),
                         [&](const HLLoop *Lp) {
                           return Lp->getNestingLevel() == TargetInnermostLevel;
                         });
  if (It == SortedLoops.end()) {
    LLVM_DEBUG(dbgs() << "TargetPermutationLevel not found in loop nest!";);
    return false;
  }

  const HLLoop *NewInnermostLp = *It;
  if (SortedLoops.erase(It) == SortedLoops.end()) {
    LLVM_DEBUG(dbgs() << "Failure to remove TargetPermutationLevel loop from "
                         "SortedLoops!";);
    return false;
  }
  SortedLoops.push_back(NewInnermostLp);

  LLVM_DEBUG({
    dbgs() << "Target Permutation, Levels in loop nest:\n(";
    for (unsigned I = 0, E = SortedLoops.size(); I != E; ++I) {
      dbgs() << SortedLoops[I]->getNestingLevel() << " ";
    }
    dbgs() << ")\n";
  });

  return true;
}

// [Actions]
// - Decide whether the desired permutation is legal for the special loop
//   interchange, by calling HIRLoopInterchange's existing legal model.
//
// [Note]
// - SortedLoops contains the desired permutation for the special interchange.
// - The loopneet needs to be cleaned up using
//   HIRTransformUtils::doArrayScalarization(.). Otherwise, the legal test
//   will fail.
//
bool HIRSpecialLoopInterchange::isLegal(HLLoop *OuterLp) {
  bool IsLegal = HIRLoopInterchange::getPermutation(OuterLp);
  LLVM_DEBUG({
    StringRef Msg = IsLegal ? "Legal" : "Illegal";
    dbgs() << Msg + " on outermost Lp: " << OuterLp << "\n";
  });

  return IsLegal;
}

bool HIRSpecialLoopInterchange::transform(HLLoop *OuterLp) {
  return HIRLoopInterchange::transformLoop(OuterLp);
}

bool HIRSpecialLoopInterchange::run() {
  if (!collect()) {
    return false;
  }

  bool Result = false;
  for (auto &P : CandidateLoopPairs) {
    clearData();
    HLLoop *OuterLp = P.first;
    HLLoop *InnerLp = P.second;
    OutermostLoop = OuterLp;
    InnermostLoop = InnerLp;
    OutmostNestingLevel = OutermostLoop->getNestingLevel();
    InnermostNestingLevel = InnermostLoop->getNestingLevel();

    if (!makePerfectLoopnest(OuterLp, InnerLp)) {
      LLVM_DEBUG(dbgs() << "Not a perfect loopnest\n";);
      continue;
    }

    if (!analyzeLoop(InnerLp)) {
      LLVM_DEBUG(dbgs() << "Failure in analyzeLoop()\n";);
      continue;
    }

    if (!doCleanup(InnerLp)) {
      LLVM_DEBUG(dbgs() << "Failure in "
                           "HIRSpecialLoopInterchange::doCleanup()\n";);
      continue;
    }

    if (!isProfitable(InnerLp)) {
      LLVM_DEBUG(
          dbgs() << "Failure in HIRSpecialLoopInterchange::isProfitable()\n";);
      continue;
    }

    if (!identifyTargetInnermostLevel(OuterLp, InnerLp)) {
      LLVM_DEBUG(dbgs() << "Failure in HIRSpecialLoopInterchange::"
                           "identifyTargetInnermostLevel()\n";);
      continue;
    }

    // Generate the target permutation for interchange.
    // E.g. (1, 2, 3) -> (2, 3, 1), where level 1 is the new innermost level
    if (!generatePermutation(OuterLp, InnerLp)) {
      LLVM_DEBUG(
          dbgs() << "Failure in "
                    "HIRSpecialLoopInterchange::generatePermutation()\n";);
      continue;
    }

    if (!isLegal(OuterLp)) {
      LLVM_DEBUG(
          dbgs() << "Failure in HIRSpecialLoopInterchange::isLegal()\n";);
      continue;
    }

    if (!transform(OuterLp)) {
      LLVM_DEBUG(
          dbgs() << "Failure in HIRSpecialLoopInterchange::transform()\n";);
      continue;
    }
    Result = true;
  }

  return Result;
}

bool HIRLoopInterchange::run(void) {
  if (DisableHIRLoopInterchange) {
    return false;
  }

  LLVM_DEBUG(dbgs() << "HIR Loop Interchange try for Function: "
                    << HIRF.getFunction().getName() << "()\n");

  // Special loop interchange:
  //
  // [Note]
  // This work will eventually be organized by the driver.
  // The code here models the driver's work and prepare the loopnest for
  // the special interchange.
  //
  // If the special loop interchange triggers, the flow will skip
  // the normal HIRLoopInterchange (HLI) pass and returns immediately.
  // Otherwise, the regular loop interchange pass will happen as normal.
  //
  if (PrepareSpecialInterchange) {
    HIRSpecialLoopInterchange HSLI(HIRF, HDDA, HLA, HSRA, HLS, HLR);
    if (HSLI.run()) {
      LLVM_DEBUG(dbgs() << "Triggered Special-loop interchange. Skip normal "
                           "interchange\n";);
      return true;
    }
  }

  AnyLoopInterchanged = false;

  // 1) Walk all loops, look for outer loops that are perfectly nested
  CollectCandidateLoops CCL(*this, CandidateLoops, HDDA,
                            HIRF.getFunction().getName());
  HIRF.getHLNodeUtils().visitAll(CCL);

  for (auto &Iter : CandidateLoops) {
    HLLoop *OutermostLp = Iter.first;
    InnermostLoop = Iter.second;
    InnermostNestingLevel = InnermostLoop->getNestingLevel();

    LLVM_DEBUG(dbgs() << "\nIn CandidateLoop:\n"; OutermostLp->dump());

    if (shouldInterchange(OutermostLp) && getPermutation(OutermostLp)) {
      transformLoop(OutermostLp);
    } else {
      if (std::find(PerfectLoopsEnabled.begin(), PerfectLoopsEnabled.end(),
                    OutermostLp) != PerfectLoopsEnabled.end()) {
        HIRInvalidationUtils::invalidateBody(OutermostLp);
      }
    }
  }

  CandidateLoops.clear();
  PerfectLoopsEnabled.clear();
  CandLoopToIgnorableSymBases.clear();

  return AnyLoopInterchanged;
}

bool HIRLoopInterchange::shouldInterchange(const HLLoop *Loop) {
  SortedLoops.clear();
  bool InterchangeNeeded = true;

  // Call Util in Locality Analysis to get Best Permutation
  HLA.sortedLocalityLoops(Loop, SortedLoops);

  if (isInPresentOrder(SortedLoops)) {
    printDiag(ALREADY_IN_RIGHT_ORDER, HIRF.getFunction().getName(), Loop);
    InterchangeNeeded = false;
  }

  LLVM_DEBUG(dbgs() << "\n\tBased on Locality Analysis:");
  LLVM_DEBUG(dbgs() << "\n\tInterchange Needed=" << InterchangeNeeded << "\n");
  return InterchangeNeeded;
}

/// Return true if loop can be interchanged with best permutation or
/// Nearby permutation
bool HIRLoopInterchange::getPermutation(const HLLoop *OutermostLp) {

  // 3) If already in decreasing order (from outer to inner) of loop cost,
  //   nothing needs to be done, Otherwise. Try to find a permutation that's
  //   legal.

  bool CanInterchange = false;
  LoopPermutation.clear();
  NearByPerm.clear();
  OutmostNestingLevel = OutermostLp->getNestingLevel();

  // Save it in local vector because it may change later
  for (auto &Lp : SortedLoops) {
    LoopPermutation.push_back(Lp);
  }

  // When returning legal == true, we can just interchange w/o
  // examining DV.
  if (isLegalForAnyPermutation(OutermostLp)) {
    LLVM_DEBUG(dbgs() << "\n\tBest permutation available\n");
    CanInterchange = true;
  } else {
    // Check if largest locality can be moved as innermost loop.
    // If no, Stop. Otherwise while loop in next function will loop forever.
    LLVM_DEBUG(printDVs(););

    const HLLoop *BestLocalityLoop = LoopPermutation.back();

    if (!isBestLocalityInInnermost(OutermostLp, BestLocalityLoop)) {
      CanInterchange = false;
    } else {
      // Find Nearby permutation
      getNearbyPermutation(OutermostLp);

      if (isInPresentOrder(NearByPerm)) {
        return false;
      }

      LoopPermutation = NearByPerm;

      LLVM_DEBUG({
        dbgs() << "\nNearby permutation obtained\n";
        for (auto &Lp : LoopPermutation) {
          Lp->dump();
        }
      });

      CanInterchange = true;
    }
  }

  if (!CanInterchange) {
    LLVM_DEBUG(dbgs() << "\nNo legal permutation available\n");
  }

  return CanInterchange;
}

///  Check if the best locality loop can stay or move as innermost
bool HIRLoopInterchange::isBestLocalityInInnermost(
    const HLLoop *Loop, const HLLoop *BestLocalityLoop) {
  unsigned SrcLevel = BestLocalityLoop->getNestingLevel();
  if (InnermostNestingLevel == BestLocalityLoop->getNestingLevel() ||
      DDUtils::isLegalForPermutation(InnermostNestingLevel, SrcLevel,
                                     OutmostNestingLevel, DVs)) {
    return true;
  }

  printDiag(BEST_LOCALITY_LOOP_CANNOT_BECOME_INNERMOST,
            HIRF.getFunction().getName(), Loop);
  return false;
}

///  Nearby Permutation:
///  "Optimizing for Parallelism and Data Locality, Kennedy & McKinley"
/// Input:
///  O = the original loop ordering, from 1 to n
///  DV =  set of dv for Loop
///  L = Permutation of O
/// Output:  P,   Nearby permutation
/// Steps:
///  P = null; k = 0; m = n
///  while (L != null)
///     for j=1,m
///       l =  lj in L
///       if all dv for {p1,...,pk,l} are legal {
///          P = {p1, .... pk, l}
///          L = L - {l};  k++;  m--;
///          break for
///       endif
///     endfor
///  endwhile
///
/// There is some issue in the paper.
/// The DVs in (p1,...pk,l) alone cannot determine the legality
/// The original DVs need to be permuted when L is changed and that should
/// used for legality check. In the example below, the first * can be moved
//  to the left but the second * cannot be moved before the first *.
/// Also, if l is already in the k-th position, select it w/o checking
/// legality because it is not changing in position.

/// Modified Algorithm:
///  P = {1 2 ... n}; k = 1; m = n
///  while (L != null)
///     for j=1,m
///       l =  lj in L
///       if ((l is already at k-th loop level  ||
///           (all Permuted dv are legal for l to be shifted as k-th loop)
///          P is updated  by putting l as the k-th loop
///          L = L - {l};  k++;  m--;
///          Permute dv accordingly if needed
///          break for
///       endif
///     endfor
///  endwhile
///
///  An example: assuming we have just 1 dv (* * = =)
///  Best permutation is L = (4 2 1 3)
///  P = (1 2 3 4)
///  (l3 is the one with best locality, should be in the innermost
//   loop when while loop terminates)
///
///  j=1,4 in  L
///    Is l4 at level 1 already? No
///    Is l4 legal as 1st level?  Yes
///    P = (4 1 2 3)  L = (2 1 3)
//     dv becomes (= * * =), corresponding  to (4 1 2 3)
///  j=1,3 in  L
///    Is l2 at level 2 already? No.
///    Is l2 legal as 2nd level? No. (* cannot shift pass *)
///    Is l1 at level 2 already? Yes.
//     No change in P or dv
///    P = (4 1 2 3) L =(2 3)
///  j=1,2
///    Is l2 at 3rd level? Yes.  No change in P or sv
///    P = (4 1 2 3) L =(3)
///  j=1,1
///    Is l3 at 4th level? Yes
///    P = (4 1 2 3) L =()  no change in dv

void HIRLoopInterchange::getNearbyPermutation(const HLLoop *Loop) {
  const HLNode *Node = Loop;

  //  Based on the paper above, the while loop will halt when
  //  the last loop in L can be moved legally as the innermost loop,
  //  which is verified before calling this function.
  while (Node) {
    const HLLoop *Lp = dyn_cast<HLLoop>(Node);
    if (!Lp) {
      break;
    }
    NearByPerm.push_back(const_cast<HLLoop *>(Lp));
    Node = Lp->getFirstChild();
  }

  unsigned DstIndex = 0;
  // NearByPerm contains [1,2,3...] in order
  // LoopPermutation contains optimal permutation, i.e. [4,2,1,3]
  while (!LoopPermutation.empty()) {
    DstIndex++;
    unsigned DstLevel = NearByPerm[DstIndex - 1]->getNestingLevel();

    bool Permuted = false;
    for (auto &I : LoopPermutation) {
      // Compute the SrcLevel and Index based on the NearbyPerm
      unsigned SrcLevel = I->getNestingLevel();
      auto Found_it = std::find_if(NearByPerm.begin(), NearByPerm.end(),
                                   [&SrcLevel](const auto &Lp) {
                                     return SrcLevel == Lp->getNestingLevel();
                                   });

      assert(Found_it != NearByPerm.end() && "Expected to find Loop!");
      unsigned SrcIndex = std::distance(NearByPerm.begin(), Found_it) + 1;

      if (!DDUtils::isLegalForPermutation(DstLevel, SrcLevel,
                                          OutmostNestingLevel, DVs)) {
        continue;
      }

      permuteNearBy(DstLevel, DstIndex, SrcLevel, SrcIndex);
      LoopPermutation.erase(&I);
      Permuted = true;
      break;
    }
    assert(Permuted && "Interchange expected to legally permute!");
    (void)Permuted;
  }
}

///  No need to interchange if suggested Permutation is same as present order
bool HIRLoopInterchange::isInPresentOrder(
    SmallVectorImpl<const HLLoop *> &LoopNest) const {
  unsigned PrevLevel = 1;

  for (auto &Loop : LoopNest) {
    unsigned Level = Loop->getNestingLevel();
    if (PrevLevel > Level) {
      return false;
    }
    PrevLevel = Level;
  }
  return true;
}

///  1. Move Loop at SrcLevel to DstLevel loop
///  2. Update all DV accordingly
///  Levels are relative to 1
void HIRLoopInterchange::permuteNearBy(unsigned DstLevel, unsigned DstIndex,
                                       unsigned SrcLevel, unsigned SrcIndex) {
  if (SrcIndex == DstIndex) {
    return;
  }
  assert((SrcIndex > DstIndex) && "Loops are shifting to the left");
  assert(SrcIndex <= NearByPerm.size() && "SrcIndex out of bounds!");
  const HLLoop *LoopSave = NearByPerm[SrcIndex - 1];
  NearByPerm.erase(NearByPerm.begin() + SrcIndex - 1);
  NearByPerm.insert(NearByPerm.begin() + DstIndex - 1, LoopSave);

  for (auto &WorkDV : DVs) {
    DVKind DVSrc = WorkDV[SrcLevel - 1];
    // Shift right by 1 for these [Dst : Src-1]
    for (unsigned JJ = SrcLevel; JJ > DstLevel; --JJ) {
      WorkDV[JJ - 1] = WorkDV[JJ - 2];
    }
    // Fill in Dst with Src
    WorkDV[DstLevel - 1] = DVSrc;
  }
}

/// Return true if legal for any permutations
bool HIRLoopInterchange::isLegalForAnyPermutation(const HLLoop *OutermostLoop) {

  //
  // 4) exclude loops that has pragma for unroll or unroll & jam.
  //    exclude triangular loop for now (TODO).

  // 5) Gather DV from DDG. Filter out loop indep dep for temps,
  // safe reduction.
  // We plan to avoid demand driven DD refining DV.

  HLLoop *Lp = const_cast<HLLoop *>(OutermostLoop);
  LLVM_DEBUG(dbgs() << "\n\tStart, End level\n"
                    << OutmostNestingLevel << " " << InnermostNestingLevel
                    << "\n");

  HSRA.computeSafeReductionChains(Lp);

  //  Set refineDV as false for now (last argument) until we see kernels
  //  that really need to refine DV.

  // The following visitor will gather DVs from DDG and push them into
  // HIRLoopInterchange::DVs;

  DDUtils::computeDVsForPermuteIgnoringSBs(DVs, Lp, InnermostNestingLevel, HDDA,
                                           HSRA, false,
                                           &CandLoopToIgnorableSymBases[Lp]);

  // If edges are selected,
  // there are dependencies to check out w.r.t to interchange order

  if (DVs.size() > 0) {
    return false;
  }
  LLVM_DEBUG(dbgs() << "\n\tDV array is empty\n");
  return true;
}

void HIRLoopInterchange::reportTransformation(
    LoopOptReportBuilder &LORBuilder) {
  // Do not do any string processing if OptReports are not needed.
  // "&& DebugFlag" should be deleted when lit-tests are rewritten to use opt
  // report info.
  if (!LORBuilder.getVerbosity() && !DebugFlag)
    return;

  HLLoop *OutermostLp = nullptr;

  std::ostringstream OS;
  OS << "Loopnest Interchanged: ( ";
  for (unsigned I = OutmostNestingLevel; I <= InnermostNestingLevel; ++I) {
    OS << I << " ";
  }
  OS << ") --> ( ";
  for (auto &I : LoopPermutation) {
    OS << I->getNestingLevel() << " ";
    if (I->getNestingLevel() == OutmostNestingLevel) {
      assert(!OutermostLp);
      OutermostLp = const_cast<HLLoop *>(I);
    }
  }
  OS << ")";
  LORBuilder(*OutermostLp).addRemark(OptReportVerbosity::Low, OS.str().c_str());

  // This is needed for lit-tests for now.
  LLVM_DEBUG(dbgs() << OS.str() << '\n');
}

bool HIRLoopInterchange::transformLoop(HLLoop *Loop) {
  // Invalidate all analysis for InnermostLoop:
  HIRInvalidationUtils::invalidateBounds(InnermostLoop);
  HIRInvalidationUtils::invalidateBody(InnermostLoop);
  LLVM_DEBUG(dbgs() << "\n\tBefore permute loopNest:"; Loop->dump());
  assert(OutmostNestingLevel == Loop->getNestingLevel());
  assert(InnermostNestingLevel == InnermostLoop->getNestingLevel());
  InnermostLoop->setIsUndoSinkingCandidate(false);

  HIRTransformUtils::permuteLoopNests(Loop, LoopPermutation,
                                      InnermostNestingLevel);

  LoopOptReportBuilder &LORBuilder =
      Loop->getHLNodeUtils().getHIRFramework().getLORBuilder();

  reportTransformation(LORBuilder);

  Loop->getParentRegion()->setGenCode();

  LoopsInterchanged++;
  AnyLoopInterchanged = true;

  return true;
}

PreservedAnalyses HIRLoopInterchangePass::runImpl(
    llvm::Function &F, llvm::FunctionAnalysisManager &AM, HIRFramework &HIRF) {
  HIRLoopInterchange(HIRF, AM.getResult<HIRDDAnalysisPass>(F),
                     AM.getResult<HIRLoopLocalityAnalysis>(F),
                     AM.getResult<HIRSafeReductionAnalysisPass>(F),
                     AM.getResult<HIRLoopStatisticsAnalysis>(F),
                     AM.getResult<HIRLoopResourceAnalysis>(F))
      .run();

  return PreservedAnalyses::all();
}

class HIRLoopInterchangeLegacyPass : public HIRTransformPass {
public:
  static char ID;

  HIRLoopInterchangeLegacyPass() : HIRTransformPass(ID) {
    initializeHIRLoopInterchangeLegacyPassPass(
        *PassRegistry::getPassRegistry());
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.setPreservesAll();
    AU.addRequiredTransitive<HIRFrameworkWrapperPass>();
    AU.addRequiredTransitive<HIRDDAnalysisWrapperPass>();
    AU.addRequiredTransitive<HIRSafeReductionAnalysisWrapperPass>();
    AU.addRequiredTransitive<HIRLoopLocalityWrapperPass>();
    AU.addRequiredTransitive<HIRLoopStatisticsWrapperPass>();
    AU.addRequiredTransitive<HIRLoopResourceWrapperPass>();
  }

  bool runOnFunction(Function &F) override {
    if (skipFunction(F)) {
      return false;
    }

    return HIRLoopInterchange(
               getAnalysis<HIRFrameworkWrapperPass>().getHIR(),
               getAnalysis<HIRDDAnalysisWrapperPass>().getDDA(),
               getAnalysis<HIRLoopLocalityWrapperPass>().getHLL(),
               getAnalysis<HIRSafeReductionAnalysisWrapperPass>().getHSR(),
               getAnalysis<HIRLoopStatisticsWrapperPass>().getHLS(),
               getAnalysis<HIRLoopResourceWrapperPass>().getHLR())
        .run();
  }
};

char HIRLoopInterchangeLegacyPass::ID = 0;
INITIALIZE_PASS_BEGIN(HIRLoopInterchangeLegacyPass, "hir-loop-interchange",
                      "HIR Loop Interchange", false, false)
INITIALIZE_PASS_DEPENDENCY(HIRFrameworkWrapperPass)
INITIALIZE_PASS_DEPENDENCY(HIRDDAnalysisWrapperPass)
INITIALIZE_PASS_DEPENDENCY(HIRLoopLocalityWrapperPass)
INITIALIZE_PASS_DEPENDENCY(HIRSafeReductionAnalysisWrapperPass)
INITIALIZE_PASS_DEPENDENCY(HIRLoopStatisticsWrapperPass)
INITIALIZE_PASS_DEPENDENCY(HIRLoopResourceWrapperPass)
INITIALIZE_PASS_END(HIRLoopInterchangeLegacyPass, "hir-loop-interchange",
                    "HIR Loop Interchange", false, false)

FunctionPass *llvm::createHIRLoopInterchangePass() {
  return new HIRLoopInterchangeLegacyPass();
}
