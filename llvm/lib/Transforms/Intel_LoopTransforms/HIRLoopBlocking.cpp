//===--- HIRLoopBlocking.cpp - Implements Loop Blocking transformation ---===//
//
// Copyright (C) 2018 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// This file implements HIR Loop Blocking transformation, which blocks(tiles)
// complete loop nests.
//
// For example:
//
//   Original Loop
//   Do J = 1..N
//      Do K = 1..N
//         Do I = 1..N
//            C(I,J) = C(I,J) + A(I,K) * B(K,J)
//
//   Transform Loop after blocking J and K loops
//   DO J' = 1, N, S                      // by-strip loop
//     DO K' = 1, N, S                    // by-strip loop
//       DO JJ = J', min(J'+ S, N)        // stripmined loop
//         DO KK = K', min(K'+ S, N)      // stripmined loop
//           DO I = 1 .. N
//              C(I,JJ) = C(I,JJ) + A(I,KK) * B(KK,JJ)
//
//   Which loops are stripmined is determined by blocking algorithm.
//   1. Kennedy and Allen's algorithm in Optimizing Compilers for Modern
//   Architectures.
//   2. Outer two loops of typical matrix multiplication are blocked.
//   Actual transformation is done by two utils: stripmine and permute
//   (a.k.a stripmine and interchange)

#include "llvm/Transforms/Intel_LoopTransforms/HIRLoopBlocking.h"
#include "llvm/ADT/SmallSet.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/IR/Function.h"
#include "llvm/Pass.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"

#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRDDAnalysis.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRLocalityAnalysis.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRLoopStatistics.h"

#include "llvm/Analysis/Intel_LoopAnalysis/Framework/HIRFramework.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/DDUtils.h"

#include "llvm/Analysis/Intel_LoopAnalysis/Utils/DDRefGatherer.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/ForEach.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HIRInvalidationUtils.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HLNodeUtils.h"
#include "llvm/Analysis/Intel_OptReport/OptReportOptionsPass.h"
#include "llvm/Transforms/Intel_LoopTransforms/HIRTransformPass.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/HIRTransformUtils.h"

#define OPT_SWITCH "hir-loop-blocking"
#define OPT_DESC "HIR Loop Blocking"
#define DEBUG_TYPE OPT_SWITCH

using namespace llvm;
using namespace llvm::loopopt;

static cl::opt<bool> DisablePass("disable-" OPT_SWITCH, cl::init(false),
                                 cl::Hidden,
                                 cl::desc("Disable " OPT_DESC " pass"));

// If this check is disabled, blocking is applied to non-constant TC or to
// constant trip count not large enough above the threshold.
static cl::opt<bool>
    DisableTCCheck("disable-" OPT_SWITCH "-trip-count-check", cl::init(false),
                   cl::Hidden,
                   cl::desc("Disable trip count check in " OPT_DESC " pass"));

// Following check will be disabled.
//      Loop Depth > number of different IVs appearing
//      in one references of the innermost loop.
// For example, this check is true for matmul. Where loop depth is 3 and
// the number of different IVs appearing in one references is 2.
// c[i][j], a[i][k], b[k][j].
static cl::opt<bool> DisableLoopDepthCheck(
    "disable-" OPT_SWITCH "-loop-depth-check", cl::init(false), cl::Hidden,
    cl::desc("Disable loop depth check in " OPT_DESC " pass"));

// By default, HIR loop blocking works only for perfect loop nest.
// If this knob is turned on, this pass attempts to enable perfect loops
// nest out of near perfect loops by calling enablePerfectLoopNest.
static cl::opt<bool> LoopBlockingNearPerfectLoopNest(
    OPT_SWITCH "-near-perfect-loop-nest", cl::init(false), cl::Hidden,
    cl::desc(OPT_DESC " pass try to work on near perfect loop nests as well."));

// matmul: default algorithm, blocks outer two loops of daxpy form of matmul.
// kandr: Algorithm from the book by Ken Kennedy and Randy Allen,
//         blocks the inner two loops.
// All: block as many loops as possible regardless of profitability.
enum BlockingAlgo { MatMul, KAndR, All };

static cl::opt<BlockingAlgo> LoopBlockingAlgorithm(
    OPT_SWITCH "-algo", cl::Hidden, cl::desc("Choose blocking algorithm:"),
    cl::values(
        clEnumValN(MatMul, "matmul",
                   "default blocking algorithm (block outer two levels)"),
        clEnumValN(KAndR, "kandr", "algorithm from K&R book"),
        clEnumValN(All, "all", "block as many loops as possible")));

// Knobs for tuning parameters for blocking - block size
static cl::opt<int> LoopBlockingBlockSize(OPT_SWITCH "-blocksize",
                                          cl::init(128), // 4 was tested
                                          cl::Hidden,
                                          cl::desc("Size of Block in " OPT_DESC
                                                   " pass"));

// Trip count
static cl::opt<int> LoopBlockingTCThreshold(
    OPT_SWITCH "-tc-threshold",
    cl::init(1028), // 16 was tested
    cl::Hidden, cl::desc("Threshold of trip counts in " OPT_DESC " pass"));

// Upperbound for small stride in bytes.
static cl::opt<int> LoopBlockingStrideThreshold(OPT_SWITCH "-stride-threshold",
                                                cl::init(32), cl::Hidden,
                                                cl::desc(" " OPT_DESC " pass"));

namespace {

typedef std::pair<HLLoop *, HLLoop *> OutermostInnermostLoopPairTy;

typedef SmallPtrSet<HLLoop *, MaxLoopNestLevel> LoopSetTy;
// Outermost, Innermost, and Set of loops to stripmine
typedef std::tuple<HLLoop *, HLLoop *, LoopSetTy> LoopnestsAndToStripTy;

typedef SmallVector<LoopnestsAndToStripTy, 4> BlockingLoopNestInfoTy;

class HIRLoopBlockingLegacyPass : public HIRTransformPass {
public:
  static char ID;

  HIRLoopBlockingLegacyPass() : HIRTransformPass(ID) {
    initializeHIRLoopBlockingLegacyPassPass(*PassRegistry::getPassRegistry());
  }

  bool runOnFunction(Function &F) override;
  void releaseMemory() override{};

  void getAnalysisUsage(AnalysisUsage &AU) const {
    AU.addRequiredTransitive<HIRFrameworkWrapperPass>();
    AU.addRequiredTransitive<HIRDDAnalysisWrapperPass>();
    AU.addRequiredTransitive<HIRSafeReductionAnalysisWrapperPass>();
    AU.addRequiredTransitive<HIRLoopStatisticsWrapperPass>();

    AU.setPreservesAll();
  }
};

} // namespace

char HIRLoopBlockingLegacyPass::ID = 0;
INITIALIZE_PASS_BEGIN(HIRLoopBlockingLegacyPass, OPT_SWITCH, OPT_DESC, false,
                      false)
INITIALIZE_PASS_DEPENDENCY(HIRFrameworkWrapperPass)
INITIALIZE_PASS_DEPENDENCY(HIRDDAnalysisWrapperPass)
INITIALIZE_PASS_DEPENDENCY(HIRSafeReductionAnalysisWrapperPass)
INITIALIZE_PASS_DEPENDENCY(HIRLoopStatisticsWrapperPass)
INITIALIZE_PASS_END(HIRLoopBlockingLegacyPass, OPT_SWITCH, OPT_DESC, false,
                    false)

FunctionPass *llvm::createHIRLoopBlockingPass() {
  return new HIRLoopBlockingLegacyPass();
}

namespace {

// Stripmine the loops found in LoopsToStripmine,
// and record the resulting by-strip loops into ByStripLoops
// Returns the outermost loop. This can be different from the input
// OutermostLoop, because of stripmining.
HLLoop *stripmineSelectedLoops(HLLoop *InnermostLoop, HLLoop *OutermostLoop,
                               const LoopSetTy &LoopsToStripmine,
                               LoopSetTy &ByStripLoops) {
  HLLoop *NewOutermost = OutermostLoop;

  // For printing out debug or optreport info
  SmallVector<unsigned, 4> BlockedLevels;

  SmallVector<std::pair<HLLoop *, unsigned>, MaxLoopNestLevel> CurLoopNests;

  ForEach<HLLoop>::visit(NewOutermost, [&CurLoopNests](HLLoop *Lp) {
    CurLoopNests.push_back(std::make_pair(Lp, Lp->getNestingLevel()));
  });

  // Scan from outermost to innermost.
  // By stripmining outer loop before its inner loop,
  // def@level of min UB vars are correctly set.
  // Notice this is true before min's definition is not hoisted.
  // Once it is hoisted, def@level has to be updated.
  for (auto CurLoopInfo : CurLoopNests) {
    HLLoop *CurLoop = CurLoopInfo.first;
    if (!LoopsToStripmine.count(CurLoop)) {
      continue;
    }

    BlockedLevels.push_back(CurLoopInfo.second);

    // Stripmine as much as possible
    // Add some heuristic for stripmining
    HIRTransformUtils::stripmine(CurLoop, CurLoop, LoopBlockingBlockSize);

    HLLoop *ByStripLoop = CurLoop->getParentLoop();
    ByStripLoops.insert(ByStripLoop);
    if (CurLoop == OutermostLoop) {
      NewOutermost = ByStripLoop;
    }

    LLVM_DEBUG(dbgs() << "===: " << CurLoop->getNestingLevel() << "\n";);
    LLVM_DEBUG(dbgs() << " a New by-strip loop\n";);
    LLVM_DEBUG(NewOutermost->dump());
  }

  LLVM_DEBUG(std::for_each(
      BlockedLevels.begin(), BlockedLevels.end(),
      [](unsigned &Level) { dbgs() << "Blocked at Level " << Level << "\n"; }));

  LLVM_DEBUG(dbgs() << "A New Outermost loop\n";);
  LLVM_DEBUG(NewOutermost->dump());

  return NewOutermost;
}

// Populate permutataion after stripmining
// Follow the current outer to inner loop order except
// moving by-strip loops to outer level.
// e.g.
// Given loop nest
// i1
//   i2   -- by-strip loop
//     i3
//       i4 -- by-strip loop
//  Make it
//  i2  -- by-strip
//    i4 -- by-strip
//      i1
//        i3
//  Resulting LoopPermuation will be (i2, i4, i1, i3)
void populatePermutation(const HLLoop *Outermost, const HLLoop *Innermost,
                         const LoopSetTy &ByStripLoops,
                         SmallVectorImpl<const HLLoop *> &LoopPermutation) {

  unsigned Mid = ByStripLoops.size();
  unsigned Back =
      Innermost->getNestingLevel() - Outermost->getNestingLevel() + 1;
  for (const HLLoop *I = Innermost, *EndLoop = Outermost->getParentLoop();
       I != EndLoop; I = I->getParentLoop()) {
    LLVM_DEBUG(dbgs() << I->getNestingLevel() << " ";);

    if (ByStripLoops.count(I)) {
      LoopPermutation[--Mid] = I;
    } else {
      LoopPermutation[--Back] = I;
    }
  }

  assert(Mid == 0 && Back == ByStripLoops.size());

  LLVM_DEBUG(dbgs() << "\n";);
  LLVM_DEBUG(dbgs() << "LoopPermutation Res\n");
  for (auto &P : LoopPermutation) {
    LLVM_DEBUG(dbgs() << P->getNestingLevel() << " ";);
    (void)P;
  }
  LLVM_DEBUG(dbgs() << "\n";);
}

typedef DDRefGatherer<RegDDRef, MemRefs> MemRefGatherer;

// Returns the largest number of dimensions in all refs
// in the given Refs.
// SeenNonLinear is set true if any ref is non-linear.
// It does not count invariant subscript at OutermostLoopLevel.
// For e.g. with A[i1][i2], B[i2], and C[5][i2][i1],
// 2 will be returned.
unsigned calcMaxVariantDimension(MemRefGatherer::VectorTy &Refs,
                                 unsigned OutermostLoopLevel,
                                 bool &SeenNonLinear) {
  SeenNonLinear = false;
  unsigned Max = 0;
  for (RegDDRef *Ref : Refs) {
    if (Ref->isNonLinear()) {
      SeenNonLinear = true;
      return 0;
    }
    unsigned NumVariantDimensions = 0;
    for (int I = 1, E = Ref->getNumDimensions(); I <= E; I++) {
      const CanonExpr *CE = Ref->getDimensionIndex(I);
      if (CE->isInvariantAtLevel(OutermostLoopLevel, false)) {
        continue;
      }
      NumVariantDimensions++;
    }

    if (NumVariantDimensions > Max) {
      Max = NumVariantDimensions;
    }
  }
  return Max;
}

// From InnermostLoop to OutermostLoop, return the consecutive depth
// where TC is at least a certain threshold.
// NewOutermost will be updated if the given OutermostLoop's TC
// is smaller then the threshold.
unsigned calcConsecutiveDepthOverTCThreshold(const HLLoop *InnermostLoop,
                                             const HLLoop *OutermostLoop,
                                             HLLoop *&NewOutermost) {
  // Scan from Innermost outerward
  // See if TC is constant and over a certain threshold
  unsigned ConsecutiveDepth = 0;
  for (const HLLoop *Lp = InnermostLoop, *ELp = OutermostLoop->getParentLoop();
       Lp != ELp; Lp = Lp->getParentLoop()) {
    uint64_t ConstantTrip = 0;
    if (!Lp->isConstTripLoop(&ConstantTrip) ||
        ConstantTrip < (uint64_t)LoopBlockingTCThreshold) {
      break;
    }
    ConsecutiveDepth++;
    NewOutermost = const_cast<HLLoop *>(Lp);
  }

  return ConsecutiveDepth;
}

// Return true if at least one stripmining is needed
// This function does not check legality.
// Legality is checked later with another function.
// Just mark loops to stripmine and actual stripmine and interchange
// happen later if the resulting permutation is valid.
//
// The algorithm is based on Allen and Kennedy's simple blocking algorithm.
//   Do J = 1..N
//      Do K = 1..N
//         Do I = 1..N
//            C(I,J) = C(I,J) + A(I,K) * B(K,J)
// For example,
//    For K-loop there is reuse of C(I, J), which is manifested by
//    missing "K" in subscripts. In this case, K-loop's inner loop is
//    blocked.
// Direct application of the algorithm is done by
// BlockingAlgorithm = kandr
//
// In measurement, blocking K-loop, not its inner loop performs better.
// Default algorithm is by setting  BlockingAlgorithm = matmul.
bool determineProfitableStripmineLoop(
    const HLLoop *InnermostLoop, const HLLoop *OutermostLoop,
    const MemRefGatherer::VectorTy &Refs, LoopSetTy &LoopsToStripmine,
    SmallVectorImpl<unsigned> &ToStripLevels) {

  unsigned OuterLevel = OutermostLoop->getNestingLevel();
  unsigned InnerLevel = InnermostLoop->getNestingLevel();
  SmallVector<int, MaxLoopNestLevel + 1> NumRefsMissingAtLevel(
      MaxLoopNestLevel + 1, 0);
  SmallVector<int, MaxLoopNestLevel + 1> NumRefsWithSmallStrides(
      MaxLoopNestLevel + 1, 0);
  for (const RegDDRef *Ref : Refs) {

    for (unsigned Level = OuterLevel; Level <= InnerLevel; Level++) {
      int64_t Stride = 0;
      if (Ref->getConstStrideAtLevel(Level, &Stride) && Stride >= 1 &&
          // TODO: Adjust LoopBlockigStrideThreshold by
          //       the element type size if needed
          Stride <= LoopBlockingStrideThreshold) {
        // This logic is for implementing finding a loop level
        // whose subscript is found in contiguous dimension of a ref.
        // After all, this is a heuristic.

        NumRefsWithSmallStrides[Level]++;
      }

      // TODO: Consider using isInvariantAt..
      // Do we want to check Ref->isInvariantAtLevel() instead?
      // Even if it doesn't contains IVs, it may have blobs.
      // Might become more important for handling non-perfect loop nest
      bool LevelSeen = Ref->hasIV(Level);

      if (!LevelSeen) {
        NumRefsMissingAtLevel[Level]++;
      }
    } // all level
  }

  // Stripmining from the parent of the innermost
  // Scan from inner to outer, and choose loops to stripmine
  // based on NumRefsMissingAtLevel and NumRefsWithSmallStridesAtLevel
  unsigned Level = InnerLevel - 1;
  unsigned NumTotalLoops = InnerLevel;
  for (const HLLoop *Lp = InnermostLoop->getParentLoop(),
                    *ELp = OutermostLoop->getParentLoop(),
                    *InnerLp = InnermostLoop;
       Lp != ELp; InnerLp = Lp, Lp = Lp->getParentLoop(), Level = Level - 1) {
    // Reused carried across Level
    // OR
    // See if any refs deepest dimension has this Level IV with a small stride
    LLVM_DEBUG(dbgs() << "NumRefsMissingAtLevel " << Level << ": "
                      << NumRefsMissingAtLevel[Level] << "\n";);
    LLVM_DEBUG(dbgs() << "NumRefsWithSmallStrides" << Level << ": "
                      << NumRefsWithSmallStrides[Level] << "\n");
    bool NotRequired = false;
    if (NumTotalLoops >= MaxLoopNestLevel) {
      break;
    }
    if ((NumRefsMissingAtLevel[Level] > 0 ||
         NumRefsWithSmallStrides[Level] > 0) &&
        Lp->canStripmine(LoopBlockingBlockSize, NotRequired)) {

      NumTotalLoops++;

      if (LoopBlockingAlgorithm == KAndR) {
        // If so, block it innerloop
        LLVM_DEBUG(dbgs() << "Loop at Level " << InnerLp->getNestingLevel()
                          << " will be stripmined\n");
        LoopsToStripmine.insert(const_cast<HLLoop *>(InnerLp));
        ToStripLevels.push_back(InnerLp->getNestingLevel());
      } else if (LoopBlockingAlgorithm == MatMul) {
        // default behavior
        LLVM_DEBUG(dbgs() << "Loop at Level " << Lp->getNestingLevel()
                          << " will be stripmined\n");
        LoopsToStripmine.insert(const_cast<HLLoop *>(Lp));
        ToStripLevels.push_back(Lp->getNestingLevel());
      }
    }
  }

  LLVM_DEBUG(dbgs() << "determineProfitableStipmineLoop result: "
                    << !LoopsToStripmine.empty() << "\n");
  return !LoopsToStripmine.empty();
}

// Block as many loops as possible starting from the innermost
// Mainly for performance testing.
bool blockLoopsMaximally(const HLLoop *InnermostLoop,
                         const HLLoop *OutermostLoop,
                         LoopSetTy &LoopsToStripmine,
                         SmallVectorImpl<unsigned> &ToStripLevels) {

  unsigned NumTotalLoops = InnermostLoop->getNestingLevel();
  for (const HLLoop *Lp = InnermostLoop, *ELp = OutermostLoop->getParentLoop();
       Lp != ELp; Lp = Lp->getParentLoop()) {
    bool NotRequired = false;
    if (NumTotalLoops >= MaxLoopNestLevel) {
      break;
    }
    if (Lp->canStripmine(LoopBlockingBlockSize, NotRequired)) {

      NumTotalLoops++;

      LLVM_DEBUG(dbgs() << "Loop at Level " << Lp->getNestingLevel()
                        << " will be stripmined\n");
      LoopsToStripmine.insert(const_cast<HLLoop *>(Lp));
      ToStripLevels.push_back(Lp->getNestingLevel());
    }
  }

  LLVM_DEBUG(dbgs() << "determineProfitableStipmineLoop result: "
                    << !LoopsToStripmine.empty() << "\n");
  return !LoopsToStripmine.empty();
}

// Authored by Pankaj
bool isValidToBlock(DirectionVector &DV, unsigned OutermostLevel,
                    unsigned LevelToStrip) {
  if (DV[LevelToStrip - 1] == DVKind::EQ) {
    return true;
  }

  if (DV.isIndepFromLevel(OutermostLevel)) {
    return true;
  }

  bool HasLessThan = DV[LevelToStrip - 1] & DVKind::LT;
  bool HasGreaterThan = DV[LevelToStrip - 1] & DVKind::GT;

  DVKind ValidDV = DVKind::NONE;
  DVKind InvalidDV = DVKind::NONE;

  if (HasLessThan) {
    if (HasGreaterThan) {
      ValidDV = DVKind::NONE;
      InvalidDV = DVKind::ALL;
    } else {
      ValidDV = DVKind::LT;
      InvalidDV = DVKind::GT;
    }
  } else {
    ValidDV = DVKind::GT;
    InvalidDV = DVKind::LT;
  }

  for (unsigned I = OutermostLevel; I < LevelToStrip; I++) {
    if (DV[I - 1] & InvalidDV) {
      return false;
    }

    if (DV[I - 1] == ValidDV) {
      return true;
    }
  }

  return true;
}

bool isLegalToStripmineAndInterchange(
    const SmallVectorImpl<unsigned> &ToStripLevels, const HLLoop *OutermostLoop,
    const HLLoop *InnermostLoop, HIRDDAnalysis &DDA,
    HIRSafeReductionAnalysis &SRA, bool RefineDV) {

  // Collect DVs
  SmallVector<DirectionVector, 16> DVs;
  SRA.computeSafeReductionChains(OutermostLoop);

  // For temps, consider only temps that are live-in.
  // Other temps are OK to ignore for DV checks.
  SpecialSymbasesTy TempSBsToConsider;
  for (auto I : llvm::make_range(InnermostLoop->live_in_begin(),
                                 InnermostLoop->live_in_end())) {
    TempSBsToConsider.insert(I);
  }

  DDUtils::computeDVsForPermuteWithSBs(DVs, OutermostLoop,
                                       InnermostLoop->getNestingLevel(), DDA,
                                       SRA, RefineDV, &TempSBsToConsider);

  unsigned OutermostLevel = OutermostLoop->getNestingLevel();
  for (auto &DV : DVs) {
    for (unsigned I : ToStripLevels) {

      if (!isValidToBlock(DV, OutermostLevel, I)) {
        return false;
      }
    }
  }
  return true;
}

class CandidateCollector final : public HLNodeVisitorBase {
public:
  bool skipRecursion(const HLNode *Node) const { return Node == SkipNode; }

  void visit(HLLoop *Loop) {

    if (Loop->isInnermost()) {
      SkipNode = Loop;
      return;
    }

    const HLLoop *ConstInnermostLoop = nullptr;
    bool IsNearPerfect = false;
    bool IsPerfectNest = HLNodeUtils::isPerfectLoopNest(
        Loop, &ConstInnermostLoop, false, &IsNearPerfect);
    if (!IsPerfectNest && !IsNearPerfect) {
      return;
    }

    HLLoop *InnermostLoop = const_cast<HLLoop *>(ConstInnermostLoop);

    if (IsNearPerfect) {
      if (LoopBlockingNearPerfectLoopNest) {
        assert(!IsPerfectNest && "isPerfectLoopNest is malfunctioning");
        DDGraph DDG = DDA.getGraph(Loop);
        InterchangeIgnorableSymbasesTy IgnorableSymbases;
        if (!DDUtils::enablePerfectLoopNest(InnermostLoop, DDG,
                                            IgnorableSymbases)) {
          SkipNode = Loop;
          return;
        }
      } else {
        SkipNode = Loop;
        return;
      }
    }

    if (HLS.getTotalLoopStatistics(InnermostLoop)
            .hasCallsWithUnknownMemoryAccess()) {
      SkipNode = Loop;
      return;
    }

    // Gather DDRefs in the InnermostLoop
    MemRefGatherer::VectorTy Refs;
    MemRefGatherer::gatherRange(InnermostLoop->child_begin(),
                                InnermostLoop->child_end(), Refs);
    LLVM_DEBUG(dbgs() << "Innermost Loop references:\n");
    LLVM_DEBUG(MemRefGatherer::dump(Refs));

    // Now Loop is a perfect loop nest
    // Examine the innermost
    bool SeenNonLinear = false;
    unsigned MaxDimension =
        calcMaxVariantDimension(Refs, Loop->getNestingLevel(), SeenNonLinear);
    if (SeenNonLinear) {
      LLVM_DEBUG(dbgs() << "Failed: nonlinear ref in the innermost loop\n");
      // A heuristic choice: Choose not to block. Stop here.
      // This check is useful for blocking typical matrix multiplication.
      SkipNode = Loop;
      return;
    }
    unsigned LoopDepth =
        InnermostLoop->getNestingLevel() - Loop->getNestingLevel() + 1;
    if (!DisableLoopDepthCheck && LoopDepth <= MaxDimension) {
      LLVM_DEBUG(dbgs() << "Failed Maxdimension >= LoopDepth " << MaxDimension
                        << "," << LoopDepth << "\n");
      // A heuristic choice: Choose not to block. Stop here.
      // This check is useful for blocking typical matrix multiplication.
      SkipNode = Loop;
      return;
    }

    // Now scan through loop nest's TC
    HLLoop *NewOutermost = Loop;
    if (!DisableTCCheck) {
      // DisableTCCheck is false by default.
      // A heuristic choice potentially to be changed:
      // For example, in a typical matrix multiplication,
      // loop depth is 3 (i, j, k loops) and maximum number of dimensions
      // is 2 ([i][j], [i][k], [k][j]).
      // The loop depth is refined so that TC of a participating loop
      // is at least a TCThreshold.
      unsigned ConsecutiveDepth = calcConsecutiveDepthOverTCThreshold(
          InnermostLoop, Loop, NewOutermost);

      if (ConsecutiveDepth <= MaxDimension) {
        LLVM_DEBUG(dbgs() << "Failed MaxDimension >= ConsecutiveDepth "
                          << MaxDimension << "," << ConsecutiveDepth << "\n");
        // Choose not to block. Stop here.
        SkipNode = Loop;
        return;
      }
    }

    LoopSetTy ToStripmines;
    ToStripmines.clear();
    // SmallSet does not allow iteration, auxiliary data structure
    SmallVector<unsigned, MaxLoopNestLevel> ToStripLevels;

    bool IsToStripmine = false;
    if (LoopBlockingAlgorithm == All) {
      IsToStripmine = blockLoopsMaximally(InnermostLoop, NewOutermost,
                                          ToStripmines, ToStripLevels);
    } else {
      IsToStripmine = determineProfitableStripmineLoop(
          InnermostLoop, NewOutermost, Refs, ToStripmines, ToStripLevels);
      assert((IsToStripmine != (ToStripmines.empty())) && "Empty!!");
    }

    if (!IsToStripmine) {
      LLVM_DEBUG(dbgs() << "Failed determineProfitableStipmineLoop\n";);
    } else if (isLegalToStripmineAndInterchange(ToStripLevels, NewOutermost,
                                                InnermostLoop, DDA, SRA,
                                                false)) {
      OutermostToStrips.emplace_back(NewOutermost, InnermostLoop, ToStripmines);
    } else {
      LLVM_DEBUG(dbgs() << "Failed isLegalToStripmineAndInterchange\n";);
    }

    SkipNode = Loop;
    return;
  }

  void visit(const HLNode *) {}
  void postVisit(const HLNode *) {}

public:
  CandidateCollector(HIRDDAnalysis &DDAnalysis, HIRSafeReductionAnalysis &SRA,
                     HIRLoopStatistics &HLS,
                     BlockingLoopNestInfoTy &CandidateLoops)
      : SkipNode(nullptr), DDA(DDAnalysis), SRA(SRA), HLS(HLS),
        OutermostToStrips(CandidateLoops) {}

private:
  HLNode *SkipNode;
  HIRDDAnalysis &DDA;
  HIRSafeReductionAnalysis &SRA;
  HIRLoopStatistics &HLS;
  BlockingLoopNestInfoTy &OutermostToStrips;
};

// LoopPermutation: Info before permutation
// CurLoopNests : Info after permutation
void hoistMinDefs(const LoopSetTy &ByStripLoops,
                  const SmallVectorImpl<const HLLoop *> &LoopPermutation,
                  const SmallVectorImpl<HLLoop *> &CurLoopNests) {

  unsigned OutermostLevel = CurLoopNests.front()->getNestingLevel();
  unsigned InnermostLevel = CurLoopNests.back()->getNestingLevel();
  unsigned DestLevel = 0;
  for (auto Lp : LoopPermutation) {

    DestLevel++;

    if (!ByStripLoops.count(Lp)) {
      continue;
    }

    unsigned OrigLevel = Lp->getNestingLevel();
    HLLoop *LpWithDef = CurLoopNests[OrigLevel - 1];
    HLLoop *LpDest = CurLoopNests[DestLevel - 1];

    assert(LpWithDef && LpDest);
    assert(OrigLevel == LpWithDef->getNestingLevel() &&
           DestLevel == LpDest->getNestingLevel() && DestLevel <= OrigLevel);

    // Move the first child of LpWithDef to the first child of LpDest
    // First Child is min definition
    // Second Child is unit-stride loop with min as UB
    HLNode *FirstChild = (const_cast<HLLoop *>(LpWithDef))->getFirstChild();
    assert(FirstChild);
    if (!isa<HLInst>(FirstChild)) {
      // "min" may not be generated by stripmine
      // because the UB of the original loop was divisible by
      // Strip mine size
      continue;
    }

    const HLInst *FirstInst = cast<HLInst>(FirstChild);
    assert(isa<SelectInst>(FirstInst->getLLVMInstruction()));

    HLNodeUtils::moveAsFirstChild(LpDest, FirstChild);

    // Find the level where min blob are used as Loop UB
    auto FindUseMinLevel = [&LoopPermutation](unsigned OrigDefLevel) {
      unsigned ResLevel = 0;
      unsigned OrigUseLevel = OrigDefLevel + 1;
      unsigned DestLevel = 0;
      for (auto OrigLp : LoopPermutation) {
        DestLevel++;
        if (OrigLp->getNestingLevel() == OrigUseLevel) {
          ResLevel = DestLevel;
          break;
        }
      }
      return ResLevel;
    };

    unsigned DefMinLevel = DestLevel;
    unsigned UseMinLevel = FindUseMinLevel(OrigLevel);
    assert(UseMinLevel != 0);
    // Update Def@level of Loop UB with min blob
    CurLoopNests[UseMinLevel - 1]
        ->getUpperDDRef()
        ->getSingleCanonExpr()
        ->setDefinedAtLevel(DefMinLevel);

    // Update Live-in temp info.
    // From LpDest's child-loop to LpWithDef's child-loop,
    // "min" def is a live-in
    unsigned MinSB = FirstInst->getLvalDDRef()->getSymbase();
    LLVM_DEBUG(dbgs() << "MinSB: " << MinSB << " DefMinLevel: " << DefMinLevel
                      << " UseMinLevel: " << UseMinLevel << "\n");
    for (unsigned I = DefMinLevel + 1; I <= UseMinLevel; I++) {
      CurLoopNests[I - 1]->addLiveInTemp(MinSB);
    }
    for (unsigned I = OutermostLevel; I <= DefMinLevel; I++) {
      CurLoopNests[I - 1]->removeLiveInTemp(MinSB);
    }
    for (unsigned I = UseMinLevel + 1; I <= InnermostLevel; I++) {
      CurLoopNests[I - 1]->removeLiveInTemp(MinSB);
    }
  }
}

// Do stripmine & interchange
void doTransformation(BlockingLoopNestInfoTy &CandidateRangeToStrips) {

  for (auto &Triple : CandidateRangeToStrips) {

    // Stripmine
    LoopSetTy ByStripLoops;

    HLLoop *OutermostLoop;
    HLLoop *InnermostLoop;
    LoopSetTy ToStripmines;
    std::tie(OutermostLoop, InnermostLoop, ToStripmines) = Triple;
    HLLoop *NewOutermostLoop = stripmineSelectedLoops(
        InnermostLoop, OutermostLoop, ToStripmines, ByStripLoops);

    assert(!ByStripLoops.empty() && "Should be stripmined somewhere");

    // Populate Permutation
    int TotalDepth = InnermostLoop->getNestingLevel() -
                     NewOutermostLoop->getNestingLevel() + 1;
    SmallVector<const HLLoop *, MaxLoopNestLevel> LoopPermutation(TotalDepth,
                                                                  nullptr);
    populatePermutation(NewOutermostLoop, InnermostLoop, ByStripLoops,
                        LoopPermutation);

    // Interchange
    HIRTransformUtils::permuteLoopNests(NewOutermostLoop, LoopPermutation,
                                        InnermostLoop->getNestingLevel());

    SmallVector<HLLoop *, MaxLoopNestLevel> CurLoopNests;
    ForEach<HLLoop>::visit(NewOutermostLoop, [&CurLoopNests](HLLoop *Lp) {
      CurLoopNests.push_back(Lp);
    });

    // Hoist min var's definitions to Destination Levels
    LLVM_DEBUG(NewOutermostLoop->dump(1));
    hoistMinDefs(ByStripLoops, LoopPermutation, CurLoopNests);
    LLVM_DEBUG(dbgs() << "after hoist\n");
    LLVM_DEBUG(NewOutermostLoop->dump());

    // Invalidate
    NewOutermostLoop->getParentRegion()->setGenCode();
    HIRInvalidationUtils::invalidateLoopNestBody(NewOutermostLoop);
    HIRInvalidationUtils::invalidateParentLoopBodyOrRegion(NewOutermostLoop);
  }
}

bool doLoopBlocking(HIRFramework &HIRF, HIRDDAnalysis &DDA,
                    HIRSafeReductionAnalysis &SRA, HIRLoopStatistics &HLS) {

  BlockingLoopNestInfoTy CandidateRangeToStrips;
  CandidateCollector BlockingCandidateCollector(DDA, SRA, HLS,
                                                CandidateRangeToStrips);
  HIRF.getHLNodeUtils().visitAll(BlockingCandidateCollector);

  if (CandidateRangeToStrips.empty()) {
    return false;
  }

  for (auto &Triple : CandidateRangeToStrips) {
    assert(!(std::get<2>(Triple)).empty() && "An empty entry");
    (void)Triple;
  }

  // Do transformation
  doTransformation(CandidateRangeToStrips);

  return true;
}

} // namespace

bool HIRLoopBlockingLegacyPass::runOnFunction(Function &F) {
  if (DisablePass || skipFunction(F)) {
    return false;
  }

  LLVM_DEBUG(dbgs() << OPT_DESC " for Function : " << F.getName() << "\n");

  auto &DDA = getAnalysis<HIRDDAnalysisWrapperPass>().getDDA();
  auto &SRA = getAnalysis<HIRSafeReductionAnalysisWrapperPass>().getHSR();
  auto &HIRF = getAnalysis<HIRFrameworkWrapperPass>().getHIR();
  auto &HLS = getAnalysis<HIRLoopStatisticsWrapperPass>().getHLS();
  return doLoopBlocking(HIRF, DDA, SRA, HLS);
}

PreservedAnalyses HIRLoopBlockingPass::run(llvm::Function &F,
                                           llvm::FunctionAnalysisManager &AM) {
  // TODO: Is it a right way to skip function?
  if (DisablePass) {
    return PreservedAnalyses::all();
  }

  LLVM_DEBUG(dbgs() << OPT_DESC " for Function : " << F.getName() << "\n");

  // TODO: How to use the return boolean return value from doLoopBlocking?
  doLoopBlocking(AM.getResult<HIRFrameworkAnalysis>(F),
                 AM.getResult<HIRDDAnalysisPass>(F),
                 AM.getResult<HIRSafeReductionAnalysisPass>(F),
                 AM.getResult<HIRLoopStatisticsAnalysis>(F));

  return PreservedAnalyses::all();
}
