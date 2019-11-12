//===--- HIRLoopBlocking.cpp - Implements Loop Blocking transformation ---===//
//
// Copyright (C) 2018-2019 Intel Corporation. All rights reserved.
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
//   Architectures. Inner two loops of typical matrix multiplications are
//   blocked.
//   2. Outer two loops of typical matrix multiplication are blocked.
//   3. All three loops of typical matrix multiplication are blocked. This
//   was backed by performance experiments in skylake machines.
//
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
static cl::opt<bool> EnableLoopBlockingNonConstTC(
    "enable-" OPT_SWITCH "-nonconst-trip-count", cl::init(true), cl::Hidden,
    cl::desc("Enable " OPT_DESC
             " pass even when some trip counts are non-const"));

// Following check will be disabled.
//      Loop Depth > number of different IVs appearing
//      in one references of the innermost loop.
// For example, this check is true for matmul. Where loop depth is 3 and
// the number of different IVs appearing in one references is 2.
// c[i][j], a[i][k], b[k][j].
static cl::opt<bool> DisableLoopDepthCheck(
    "disable-" OPT_SWITCH "-loop-depth-check", cl::init(false), cl::Hidden,
    cl::desc("Disable loop depth check in " OPT_DESC " pass"));

// matmul: block as many loops as possible. Will block all 3 levels
//         for typical matrix multiplications. Showed the best performance
//         for skx performance machines with following options:
//            -O3 -xCORE-AVX512 -Ofast -mfpmath=sse -march=core-avx512
// kandr: Algorithm from the book by Ken Kennedy and Randy Allen,
//         blocks the inner two loops.
// outer: Blocks outer two loops of daxpy form of matmul. Once showed
//        best performance with ICC on many machines.
enum BlockingAlgo { MatMul, KAndR, Outer };

static cl::opt<BlockingAlgo> LoopBlockingAlgorithm(
    OPT_SWITCH "-algo", cl::Hidden, cl::desc("Choose blocking algorithm:"),
    cl::values(
        clEnumValN(
            MatMul, "matmul",
            "default blocking algorithm (block as many loops as profitable)"),
        clEnumValN(KAndR, "kandr", "algorithm from K&R book"),
        clEnumValN(Outer, "outer", "block outer two levels")));

// Knobs for tuning parameters for blocking - block size
// If a value other than 0 is given, the value is used as the block size
// (i.e. stripmine size for stripmine() utility) for all loop levels
// blocked. Thus, logic for block sizes is ignored.
static cl::opt<int>
    CommandLineBlockSize(OPT_SWITCH "-blocksize", cl::init(0), cl::Hidden,
                         cl::desc("Size of Block in " OPT_DESC " pass"));

// Lower bound of trip counts where blocking is attempted.
// Default value is tuned by experiments on skylake machines:
// 512 is the first power of 2 TC value showed performance gain
// via loop blocking for two 512 by 512 square matrices multiplication.
// 384 = (256 + 512) / 2 gained some performance through loop blocking.
static cl::opt<int> LoopBlockingTCThreshold(
    OPT_SWITCH "-tc-threshold", cl::init(384), cl::Hidden,
    cl::desc("Threshold of trip counts in " OPT_DESC " pass"));

// Upperbound for small stride in bytes.
// This knob is mainly for KAndR algorithm.
// Does not greatly affect global behavior of hir loop blocking.
static cl::opt<int> LoopBlockingStrideThreshold(OPT_SWITCH "-stride-threshold",
                                                cl::init(32), cl::Hidden,
                                                cl::desc(" " OPT_DESC " pass"));
namespace {

// Following blocksize worked best in most of pow-2 and non-pow-2
// square matrix multiplications. Compile options used are
// marked in the beginning part of this file.
const unsigned DefaultBlockSize = 64;

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

  void getAnalysisUsage(AnalysisUsage &AU) const override {
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

// Based on experiments on skx performance machines with
// -O3 -xCORE-AVX512 -Ofast -mfpmath=sse -march=core-avx512
// In most cases, block size (i.e. stripmine size for stripmine() utility)
// 64 or 128 gave the best performance.
// When TC, also the width of square matrix, is large pow-2 numbers, e.g. 2048
// and 4096, block size(BS) 16 gave the best performance.
// This behavior grealtly relies on
// complete unroll of the innermost loop after vectorization.
// For that effect, it is assumed
// complete unroll before vectorization does not happen.
// Non pow-2 TC larger than 4096, still showed the best performance
// with BS = 64.
unsigned adjustBlockSize(const HLLoop *Lp, uint64_t TC) {
  if (CommandLineBlockSize) {
    return CommandLineBlockSize;
  }
  if (TC == 2048 || TC == 4096) {
    return 16;
  }
  return DefaultBlockSize;
}

unsigned adjustBlockSize(const HLLoop *Lp) {

  uint64_t ConstantTrip = 0;
  Lp->isConstTripLoop(&ConstantTrip);
  // If non-zero ConstantTrip found, pass that value, otherwise 0
  return adjustBlockSize(Lp, ConstantTrip);
}

void adjustBlockSize(const DenseMap<const HLLoop *, uint64_t> &LoopToTC,
                     DenseMap<const HLLoop *, unsigned> &LoopToBS) {
  for (auto I : LoopToTC) {
    LoopToBS[I.first] = adjustBlockSize(I.first, I.second);
  }
}

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
    unsigned BlockSize = adjustBlockSize(CurLoop);
    HIRTransformUtils::stripmine(CurLoop, CurLoop, BlockSize);

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
// It does not count invariant subscript at OutermostLoopLevel.
// For e.g. with {A[i1][i2], B[i2], C[5][i2][i1]},
// 2 will be returned because 2 is maximun of {2, 1, 2}.
unsigned calcMaxVariantDimension(const MemRefGatherer::VectorTy &Refs,
                                 unsigned OutermostLoopLevel) {
  unsigned Max = 0;
  for (RegDDRef *Ref : Refs) {
    unsigned NumVariantDimensions = 0;

    for (unsigned I :
         make_range(Ref->dim_index_begin(), Ref->dim_index_end())) {
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
} // namespace

void populateTCs(const HLLoop *InnermostLoop, const HLLoop *OutermostLoop,
                 DenseMap<const HLLoop *, uint64_t> &LoopToTC) {

  for (const HLLoop *Lp = InnermostLoop, *ELp = OutermostLoop->getParentLoop();
       Lp != ELp; Lp = Lp->getParentLoop()) {
    uint64_t ConstantTrip = 0;
    if (Lp->isConstTripLoop(&ConstantTrip)) {
      LoopToTC[Lp] = ConstantTrip;
    } else {
      LoopToTC[Lp] = 0;
    }
  }
}

// From InnermostLoop to OutermostLoop, return the consecutive depth
// where TC is at least a certain threshold.
// NewOutermost will be updated if the given OutermostLoop's TC
// is smaller then the threshold.
unsigned calcConsecutiveDepthOverTCThreshold(
    const HLLoop *InnermostLoop, const HLLoop *OutermostLoop,
    HLLoop *&NewOutermost, const DenseMap<const HLLoop *, uint64_t> &LoopToTC) {

  auto IsConstTC = [](uint64_t TC) { return TC > 0; };

  // Scan from Innermost outerward
  // See if TC is constant and over a certain threshold
  unsigned ConsecutiveDepth = 0;
  for (const HLLoop *Lp = InnermostLoop, *ELp = OutermostLoop->getParentLoop();
       Lp != ELp; Lp = Lp->getParentLoop()) {
    uint64_t TCAtLevel = LoopToTC.find(Lp)->second;
    // non-const TC has TCAtLevel zero
    if (TCAtLevel < (uint64_t)LoopBlockingTCThreshold) {
      if (!EnableLoopBlockingNonConstTC || IsConstTC(TCAtLevel)) {
        break;
      }
    }
    ConsecutiveDepth++;
    NewOutermost = const_cast<HLLoop *>(Lp);
  }

  return ConsecutiveDepth;
}

// Return true if at least one stripmining is needed.
// This function does not check legality.
// Legality is checked later with another function.
// Just mark loops to stripmine and actual stripmine and interchange
// happens later if the resulting permutation is valid.
//
// The algorithm is based on Allen and Kennedy's simple blocking algorithm.
//   Do J = 1..N
//      Do K = 1..N
//         Do I = 1..N
//            C(I,J) = C(I,J) + A(I,K) * B(K,J)
// For example,
//    For K-loop, there is reuse of C(I, J), which is manifested by
//    missing "K" in subscripts. In this case, K-loop's inner loop is
//    blocked.
// Direct application of the algorithm is done by
// BlockingAlgorithm = kandr
//
// Experiments showed blocking K-loop, not its inner loop
// performs better (outer).
// Furthermore, blocking all 3 loops for matrix multiplication performs
// best in skx performance machine (1 copy).
// Default algorithm is by setting  BlockingAlgorithm = matmul.
bool determineProfitableStripmineLoop(
    const HLLoop *InnermostLoop, const HLLoop *OutermostLoop,
    const MemRefGatherer::VectorTy &Refs, LoopSetTy &LoopsToStripmine,
    const DenseMap<const HLLoop *, unsigned> &LoopToBS,
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

      if (!LevelSeen &&
          (std::any_of(Ref->canon_begin(), Ref->canon_end(),
                       [](const CanonExpr *CE) { return CE->hasIV(); }))) {
        // Ref should have at least one IV.
        // If Ref has no IV (e.g. A[0]),
        // the ref should be ignored.

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
    LLVM_DEBUG(dbgs() << "NumRefsWithSmallStrides " << Level << ": "
                      << NumRefsWithSmallStrides[Level] << "\n");

    if (NumTotalLoops >= MaxLoopNestLevel) {
      break;
    }
    auto StripmineSize = LoopToBS.find(Lp)->second;
    if ((NumRefsMissingAtLevel[Level] > 0 ||
         NumRefsWithSmallStrides[Level] > 0) &&
        Lp->isStripmineRequired(StripmineSize) &&
        Lp->canStripmine(StripmineSize)) {

      NumTotalLoops++;

      // KAndR blocks its inner loop. Otherwise, the loop.
      HLLoop *ToStripmine = LoopBlockingAlgorithm == KAndR
                                ? const_cast<HLLoop *>(InnerLp)
                                : const_cast<HLLoop *>(Lp);
      LLVM_DEBUG(dbgs() << "Loop at Level " << ToStripmine->getNestingLevel()
                        << " will be stripmined\n");
      LoopsToStripmine.insert(ToStripmine);
      ToStripLevels.push_back(ToStripmine->getNestingLevel());
    }
  }

  // Look into the innermost loop again for blocking when algo is MatMul.
  // Experiments in skx showed blocking all three levels of matrix
  // multiplication gives best performance.
  if (LoopBlockingAlgorithm == MatMul &&
      (std::any_of(NumRefsMissingAtLevel.begin(), NumRefsMissingAtLevel.end(),
                   [](int Num) { return Num > 0; }))) {

    auto StripmineSize = LoopToBS.find(InnermostLoop)->second;
    if (InnermostLoop->isStripmineRequired(StripmineSize) &&
        InnermostLoop->canStripmine(StripmineSize)) {
      // calcConsecutiveDepthOverTCThreshold already checked TC
      // of the innermost Loop.
      LLVM_DEBUG(dbgs() << "* Loop at Level "
                        << InnermostLoop->getNestingLevel()
                        << " will be stripmined\n");
      LoopsToStripmine.insert(const_cast<HLLoop *>(InnermostLoop));
      ToStripLevels.push_back(InnermostLoop->getNestingLevel());
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
      LLVM_DEBUG(dbgs() << "Failed: Neither perfect nor near-perfect loop\n");
      return;
    }

    HLLoop *InnermostLoop = const_cast<HLLoop *>(ConstInnermostLoop);
    if (HLS.getTotalLoopStatistics(InnermostLoop)
            .hasCallsWithUnsafeSideEffects()) {
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
    if (std::any_of(Refs.begin(), Refs.end(),
                    [](const RegDDRef *Ref) { return Ref->isNonLinear(); })) {
      LLVM_DEBUG(dbgs() << "Failed: nonlinear ref in the innermost loop\n");
      // If any Ref is a non-linear, give up here.
      SkipNode = Loop;
      return;
    }

    unsigned MaxDimension =
        calcMaxVariantDimension(Refs, Loop->getNestingLevel());
    if (!DisableLoopDepthCheck) {
      // A heuristic choice: Choose not to block. Stop here.
      // This check is useful for blocking typical matrix multiplication.

      unsigned LoopDepth =
          InnermostLoop->getNestingLevel() - Loop->getNestingLevel() + 1;
      if (LoopDepth <= MaxDimension) {
        LLVM_DEBUG(dbgs() << "Failed: at MaxDimension < LoopDepth "
                          << MaxDimension << "," << LoopDepth << "\n");
        SkipNode = Loop;
        return;
      }
    }

    // Now scan through loop nest's TC
    HLLoop *NewOutermost = Loop;
    DenseMap<const HLLoop *, uint64_t> LoopToTC;
    populateTCs(InnermostLoop, Loop, LoopToTC);
    unsigned ConsecutiveDepth = calcConsecutiveDepthOverTCThreshold(
        InnermostLoop, Loop, NewOutermost, LoopToTC);

    // A heuristic choice potentially to be changed:
    // For example, in a typical matrix multiplication,
    // loop depth is 3 (i, j, k loops) and maximum number of dimensions
    // is 2 ([i][j], [i][k], [k][j]).
    // The loop depth is refined so that TC of a participating loop
    // is at least a TCThreshold.
    if (ConsecutiveDepth <= MaxDimension) {
      LLVM_DEBUG(dbgs() << "Failed: at MaxDimension < ConsecutiveDepth "
                        << MaxDimension << "," << ConsecutiveDepth << "\n");
      // Choose not to block. Stop here.
      SkipNode = Loop;
      return;
    }

    // Adjust BlockSize (a.k.a stripmine size) depending on TC
    DenseMap<const HLLoop *, unsigned> LoopToBS;
    adjustBlockSize(LoopToTC, LoopToBS);

    LoopSetTy ToStripmines;

    // SmallSet does not allow iteration, auxiliary data structure
    SmallVector<unsigned, MaxLoopNestLevel> ToStripLevels;
    bool IsToStripmine =
        determineProfitableStripmineLoop(InnermostLoop, NewOutermost, Refs,
                                         ToStripmines, LoopToBS, ToStripLevels);
    if (!IsToStripmine) {
      LLVM_DEBUG(dbgs() << "Failed: at determineProfitableStipmineLoop\n";);
      SkipNode = Loop;
      return;
    }

    if (IsNearPerfect) {
      // It is near-perfect and looks profitable
      DDGraph DDG = DDA.getGraph(Loop);
      InterchangeIgnorableSymbasesTy IgnorableSymbases;
      if (!DDUtils::enablePerfectLoopNest(InnermostLoop, DDG,
                                          IgnorableSymbases)) {
        LLVM_DEBUG(dbgs() << "Failed: at enabling a perfect loop nest\n";);
        SkipNode = Loop;
        return;
      }
    }

    if (isLegalToStripmineAndInterchange(ToStripLevels, NewOutermost,
                                         InnermostLoop, DDA, SRA, false)) {
      OutermostToStrips.emplace_back(NewOutermost, InnermostLoop, ToStripmines);
      // Done with this loopnest. We are going to work on this loopnest.
      SkipNode = Loop;
    } else {
      LLVM_DEBUG(dbgs() << "Failed: at isLegalToStripmineAndInterchange\n";);
      // Inner loop nests will have second chances
      // because it was invalid with respected to this outer loop.
    }

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

// LoopVectors contains loops at levels from [OutermostLoopLevel, ..]
// Whenever a loop's level is given, it should be adjusted by OutermostLoopLevel
// to index the correct entry of a container.
inline unsigned getIndexForLoopVectors(unsigned Level,
                                       unsigned OutermostLoopLevel) {
  return Level - OutermostLoopLevel;
}

// LoopPermutation: Info before permutation
// CurLoopNests : Info after permutation
void hoistMinDefs(const LoopSetTy &ByStripLoops,
                  const SmallVectorImpl<const HLLoop *> &LoopPermutation,
                  const SmallVectorImpl<HLLoop *> &CurLoopNests) {

  unsigned OutermostLevel = CurLoopNests.front()->getNestingLevel();

  // Find the level where min blob is used as Loop UB
  auto FindUseMinLevel = [&LoopPermutation,
                          OutermostLevel](unsigned OrigDefLevel) {
    unsigned OrigUseLevel = OrigDefLevel + 1;
    unsigned DestLevel = OutermostLevel - 1;
    for (auto OrigLp : LoopPermutation) {
      DestLevel++;
      if (OrigLp->getNestingLevel() == OrigUseLevel) {
        return DestLevel;
      }
    }
    llvm_unreachable("Stripmined loop is not found after permutation.");
  };

  unsigned InnermostLevel = CurLoopNests.back()->getNestingLevel();
  unsigned DestLevel = OutermostLevel - 1;

  for (auto LpWithDef : LoopPermutation) {

    DestLevel++;

    if (!ByStripLoops.count(LpWithDef)) {
      continue;
    }

    HLLoop *LpDest =
        CurLoopNests[getIndexForLoopVectors(DestLevel, OutermostLevel)];
    unsigned OrigLevel = LpWithDef->getNestingLevel();
    assert(DestLevel <= OrigLevel);

    // Move the first child of LpWithDef to the first child of LpDest
    // First Child is min definition
    // Second Child is unit-stride loop with min as UB
    HLNode *FirstChild = (const_cast<HLLoop *>(LpWithDef))->getFirstChild();
    if (!isa<HLInst>(FirstChild)) {
      // "min" may not be generated by stripmine
      // because the UB of the original loop was divisible by
      // Strip mine size
      continue;
    }

    const HLInst *FirstInst = cast<HLInst>(FirstChild);
    assert(isa<SelectInst>(FirstInst->getLLVMInstruction()));

    HLNodeUtils::moveAsFirstChild(LpDest, FirstChild);

    unsigned DefMinLevel = DestLevel;
    unsigned UseMinLevel = FindUseMinLevel(OrigLevel);
    // Update Def@level of Loop UB with min blob
    CurLoopNests[getIndexForLoopVectors(UseMinLevel, OutermostLevel)]
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
      CurLoopNests[getIndexForLoopVectors(I, OutermostLevel)]->addLiveInTemp(
          MinSB);
    }
    for (unsigned I = OutermostLevel; I <= DefMinLevel; I++) {
      CurLoopNests[getIndexForLoopVectors(I, OutermostLevel)]->removeLiveInTemp(
          MinSB);
    }
    for (unsigned I = UseMinLevel + 1; I <= InnermostLevel; I++) {
      CurLoopNests[getIndexForLoopVectors(I, OutermostLevel)]->removeLiveInTemp(
          MinSB);
    }
  }
}

// Used after loop permutation to reference data structure based on loops
// before permutation.
const HLLoop *getLoopForReferingInfoBeforePermutation(
    const HLLoop *LoopAfterPerm,
    const SmallVectorImpl<const HLLoop *> &LoopPermutation,
    const unsigned OutermostLoopLevel) {
  unsigned Level = LoopAfterPerm->getNestingLevel();
  return LoopPermutation[getIndexForLoopVectors(Level, OutermostLoopLevel)];
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

    InnermostLoop->setIsUndoSinkingCandidate(false);

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

    // Add OptReport after permutation.
    // ToStripmines knows which loops are blocked in terms of the level
    // before the permutation happens.
    LoopOptReportBuilder &LORBuilder = CurLoopNests.front()
                                           ->getHLNodeUtils()
                                           .getHIRFramework()
                                           .getLORBuilder();
    for (auto Lp : CurLoopNests) {
      const HLLoop *OrigLoop = getLoopForReferingInfoBeforePermutation(
          Lp, LoopPermutation, CurLoopNests.front()->getNestingLevel());
      if (ToStripmines.count(OrigLoop))
        LORBuilder(*Lp).addRemark(OptReportVerbosity::Low, "Loop is blocked");
    }

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
