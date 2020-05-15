//===--- HIRLoopBlocking.cpp - Implements Loop Blocking transformation ---===//
//
// Copyright (C) 2018-2020 Intel Corporation. All rights reserved.
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

#include "llvm/Transforms/Intel_LoopTransforms/HIRLoopBlockingPass.h"
#include "llvm/ADT/SmallSet.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/IR/Function.h"
#include "llvm/InitializePasses.h"
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
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/DDRefGrouping.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/ForEach.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HIRInvalidationUtils.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HLNodeUtils.h"
#include "llvm/Analysis/Intel_OptReport/OptReportOptionsPass.h"
#include "llvm/Analysis/TargetTransformInfo.h"
#include "llvm/Transforms/Intel_LoopTransforms/HIRTransformPass.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/HIRTransformUtils.h"

#define OPT_SWITCH "hir-loop-blocking"
#define OPT_DESC "HIR Loop Blocking"
#define DEBUG_TYPE OPT_SWITCH

// Dump, delinearizaion and stencil-related messages.
#define LLVM_DEBUG_DELINEAR(X) DEBUG_WITH_TYPE(OPT_SWITCH "-delinear", X)
#define LLVM_DEBUG_DIAG_DETAIL(X) DEBUG_WITH_TYPE(OPT_SWITCH "-dump-detail", X)

using namespace llvm;
using namespace llvm::loopopt;

static cl::opt<bool> DisablePass("disable-" OPT_SWITCH, cl::init(false),
                                 cl::Hidden,
                                 cl::desc("Disable " OPT_DESC " pass"));

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
// These options are for printing out information even with non-debug compiler.
static cl::opt<bool>
    PrintBlockedLoops(OPT_SWITCH "-print-affected-loops", cl::init(false),
                      cl::ReallyHidden,
                      cl::desc("Print loops affected by " OPT_DESC " pass"));

static cl::opt<unsigned>
    PrintDiagLevel(OPT_SWITCH "-diag-level", cl::init(0), cl::ReallyHidden,
                   cl::desc("Print Diag why " OPT_DESC " did not happen."));

static cl::opt<std::string> PrintDiagFunc(
    OPT_SWITCH "-print-diag-func", cl::ReallyHidden,
    cl::desc("Print Diag why " OPT_DESC " did not happen for the function."));
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

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
// TODO: to be removed. Pragma is coming.
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

static cl::opt<bool> OldVersion(OPT_SWITCH "-old-ver", cl::init(true),
                                cl::Hidden, cl::desc("Old " OPT_DESC " pass"));
namespace {

enum DiagMsg {
  NON_LINEAR_REFS,
  NO_MISSING_IVS_OR_SMALL_STRIDES,
  NO_PERFECTNEST,
  INNERMOST_LOOP_ONLY,
  INNERMOST_LOOP_NO_DO_LOOP,
  NO_PROFITABLE_BLOCKING_FOUND,
  MAX_DIMS_LESS_THAN_LOOP_NEST_DEPTH,
  MAX_DIMS_LESS_THAN_LOOP_NEST_DEPTH_2,
  NO_STRIPMINE_APPL,
  NO_INTERCHANGE,
  NO_KANDR,
  NO_KANDR_DEPTH_TEST_1,
  NO_KANDR_DEPTH_TEST_2,
  MULTIVERSIONED_FALLBACK_LOOP,
  NO_STENCIL_LOOP,
  NO_STENCIL_LOOP_BODY,
  NO_STENCIL_MEM_REFS,
  SUCCESS_NON_SIV,
  SUCCESS_NON_SIV_OR_NON_ADVANCED,
  SUCCESS_BASIC_SIV,
  SUCCESS_STENCIL,

  NUM_DIAGS,
};

inline std::array<std::string, NUM_DIAGS> createDiagMap() {
  std::array<std::string, NUM_DIAGS> Map;
  Map[NON_LINEAR_REFS] = "nonlinear memref in the innermost loop.";
  Map[NO_MISSING_IVS_OR_SMALL_STRIDES] =
      "no refs with missing ivs or with small strides.";
  Map[NO_PERFECTNEST] = "NearPerfect loop cannot become a perfect loop.";
  Map[INNERMOST_LOOP_ONLY] = "Innermost-loop-only in the loopnest.";
  Map[INNERMOST_LOOP_NO_DO_LOOP] = "Innermost is not Do-loop.";
  Map[NO_PROFITABLE_BLOCKING_FOUND] = "No profitable blocking found.";
  Map[MAX_DIMS_LESS_THAN_LOOP_NEST_DEPTH] = "max dims <= loop nest depth.";
  Map[MAX_DIMS_LESS_THAN_LOOP_NEST_DEPTH_2] =
      "max dims <= consecutive loop nest depth.";
  Map[NO_STRIPMINE_APPL] = "Stripmining is not possible.";
  Map[NO_INTERCHANGE] = "Interchange is not possible.";
  Map[NO_KANDR] = "KandR did not work.";
  Map[NO_KANDR_DEPTH_TEST_1] = "Failed KandR depth test 1.";
  Map[NO_KANDR_DEPTH_TEST_2] = "Failed KandR depth test 2.";
  Map[MULTIVERSIONED_FALLBACK_LOOP] = "The loop is mv fallback loop.";
  Map[NO_STENCIL_LOOP] = "The loop body is not a stencil function.";
  Map[NO_STENCIL_LOOP_BODY] = "Operations for stencil is missing.";
  Map[NO_STENCIL_MEM_REFS] = "Memrefs are not of stencil pattern.";
  Map[SUCCESS_NON_SIV] = "Non-Siv loop.";
  Map[SUCCESS_NON_SIV_OR_NON_ADVANCED] = "Non-Siv loop or Non-advanced.";
  Map[SUCCESS_BASIC_SIV] = "Basic algorithm.";
  Map[SUCCESS_STENCIL] = "Stencil pattern.";

  return Map;
}

static std::array<std::string, NUM_DIAGS> DiagMap = createDiagMap();

// TODO: Loop Interchange has the same function. Consolidate.
void printDiag(DiagMsg Msg, StringRef FuncName, const HLLoop *Loop = nullptr,
               StringRef Header = "No Blocking: ", unsigned Level = 1) {
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  if (Level > PrintDiagLevel)
    return;
  if (!PrintDiagFunc.empty() && !FuncName.equals(PrintDiagFunc)) {
    return;
  }
  dbgs() << "Func: " << FuncName << ", ";
  dbgs() << Header << " " << DiagMap[Msg] << "\n";
  if (Loop) {
    dbgs() << "Loop:" << Loop->getNumber() << "\n";
  }
#endif
}

// Following blocksize worked best in most of pow-2 and non-pow-2
// square matrix multiplications. Compile options used are
// marked in the beginning part of this file.
const unsigned DefaultBlockSize = 64;

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
    AU.addRequired<TargetTransformInfoWrapperPass>();

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
INITIALIZE_PASS_DEPENDENCY(TargetTransformInfoWrapperPass)
INITIALIZE_PASS_END(HIRLoopBlockingLegacyPass, OPT_SWITCH, OPT_DESC, false,
                    false)

FunctionPass *llvm::createHIRLoopBlockingPass() {
  return new HIRLoopBlockingLegacyPass();
}

namespace {

typedef DDRefGatherer<RegDDRef, MemRefs> MemRefGatherer;

/// Represent a Loop nest of [Outermost, Innermost]
/// Template argument T could be the type of a value, where
/// a value mapped to a loop. E.g. Stripmine size, T will be unsigned,
/// for a trip count, T will be uint64_t, and so on.
template <typename T> struct LoopNestValTy {
  const HLLoop *Outermost;
  const HLLoop *Innermost;

  LoopNestValTy(const HLLoop *Outermost, const HLLoop *Innermost)
      : Outermost(Outermost), Innermost(Innermost),
        InnermostLevel(Innermost->getNestingLevel()) {
    assert(InnermostLevel >= Outermost->getNestingLevel());
    LevelToVal.resize(InnermostLevel - Outermost->getNestingLevel() + 1, {});
  }

  T operator[](unsigned Level) const { return LevelToVal[getIndex(Level)]; }

  T &operator[](unsigned Level) { return LevelToVal[getIndex(Level)]; }

  T operator[](const HLLoop *Lp) const {
    return operator[](Lp->getNestingLevel());
  }

  T &operator[](const HLLoop *Lp) { return operator[](Lp->getNestingLevel()); }

  void setOutermostLoop(const HLLoop *NewOutermost) {
    Outermost = NewOutermost;
  }

  void populateLoops() {
    Loop.resize(InnermostLevel - Outermost->getNestingLevel() + 1, {});
    unsigned Level = InnermostLevel;
    for (const HLLoop *Lp = Innermost, *ELp = Outermost->getParentLoop();
         Lp != ELp; Lp = Lp->getParentLoop(), Level = Level - 1) {
      Loop[getIndex(Level)] = Lp;
    }
  }

  const HLLoop *getLoop(unsigned Level) const { return Loop[getIndex(Level)]; }

  IntegerRangeIterator level_from_outer_begin() const {
    return IntegerRangeIterator(Outermost->getNestingLevel());
  }
  IntegerRangeIterator level_from_outer_end() const {
    return IntegerRangeIterator(InnermostLevel + 1);
  }

private:
  unsigned InnermostLevel;
  // Contains TC by the increasing order of level (i.e. Outermost Level
  // to InnermostLevel)
  // For a non-const TC, "0" is populated.
  SmallVector<T, 8> LevelToVal;
  SmallVector<const HLLoop *, 8> Loop;

  // Vector contains Val from innermost to outermost.
  // This is for our applications, innermost does not change,
  // while outermost can change.
  unsigned getIndex(unsigned Level) const { return InnermostLevel - Level; }
};

class LoopNestTCTy : public LoopNestValTy<uint64_t> {
public:
  LoopNestTCTy(const HLLoop *Outermost, const HLLoop *Innermost)
      : LoopNestValTy(Outermost, Innermost) {}

  static bool isConstTC(uint64_t TC) { return TC > 0; };
};

// Mapping from a loop to its kind. Used for/during actual transformantion.
// For a loop to be strimined, (will become unit-strided), holds stripmine size.
// For a by-strip loop, holds BY_STRIP_LOOP_VAL, which is zero.
typedef std::map<const HLLoop *, unsigned> LoopMapTy;

enum LoopTypeVal { BY_STRIP_LOOP_VAL = 0, STRIPMINE_CAND_VAL = 0 };

inline void addByStripLoop(HLLoop *ByStripLoop, LoopMapTy &LoopMap) {
  LoopMap.emplace(std::make_pair(ByStripLoop, BY_STRIP_LOOP_VAL));
}

inline bool isNonByStripLoop(const HLLoop *Loop, const LoopMapTy &LoopMap) {
  // Return true for a non-byStrip loop.
  auto MapIt = LoopMap.find(Loop);
  return MapIt == LoopMap.end() || MapIt->second != BY_STRIP_LOOP_VAL;
}

inline bool isBlockedLoop(const HLLoop *Loop, const LoopMapTy &LoopMap) {
  // Return true for a non-byStrip loop.
  auto MapIt = LoopMap.find(Loop);
  return MapIt != LoopMap.end() && MapIt->second != BY_STRIP_LOOP_VAL;
}

inline void markAsToStripmine(const HLLoop *LoopToStripmine,
                              LoopMapTy &LoopMap) {
  LoopMap[LoopToStripmine] = STRIPMINE_CAND_VAL;
}

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
unsigned adjustBlockSize(uint64_t TC) {
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
  return adjustBlockSize(ConstantTrip);
}

void adjustBlockSize(const LoopNestTCTy &LoopNestTC, LoopMapTy &LoopToBS) {
  for (auto I : make_range(LoopNestTC.level_from_outer_begin(),
                           LoopNestTC.level_from_outer_end())) {
    LoopToBS[LoopNestTC.getLoop(I)] = adjustBlockSize(LoopNestTC[I]);
  }
}

// Stripmine the loops found in LoopMap,
// and record the resulting by-strip loops into LoopMap with BY_STRIP_LOOP_VAL.
// Returns the outermost loop. This can be different from the input
// OutermostLoop, because of stripmining.
// Arg \p LoopMap is in/out parameter.
HLLoop *stripmineSelectedLoops(HLLoop *InnermostLoop, HLLoop *OutermostLoop,
                               LoopMapTy &LoopMap) {
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
    if (!LoopMap.count(CurLoop)) {
      continue;
    }

    BlockedLevels.push_back(CurLoopInfo.second);

    HIRTransformUtils::stripmine(CurLoop, CurLoop, LoopMap[CurLoop]);

    HLLoop *ByStripLoop = CurLoop->getParentLoop();

    addByStripLoop(ByStripLoop, LoopMap);

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
                         const LoopMapTy &LoopMap,
                         SmallVectorImpl<const HLLoop *> &LoopPermutation) {

  unsigned ByStripSize = std::count_if(LoopMap.begin(), LoopMap.end(),
                                       [](LoopMapTy::const_reference S) {
                                         return S.second == BY_STRIP_LOOP_VAL;
                                       });
  unsigned Mid = ByStripSize;
  unsigned Back =
      Innermost->getNestingLevel() - Outermost->getNestingLevel() + 1;
  for (const HLLoop *I = Innermost, *EndLoop = Outermost->getParentLoop();
       I != EndLoop; I = I->getParentLoop()) {
    LLVM_DEBUG(dbgs() << I->getNestingLevel() << " ";);

    if (isNonByStripLoop(I, LoopMap)) {
      LoopPermutation[--Back] = I;
    } else {
      LoopPermutation[--Mid] = I;
    }
  }

  assert(Mid == 0 && Back == ByStripSize);

  LLVM_DEBUG(dbgs() << "\n";);
  LLVM_DEBUG(dbgs() << "LoopPermutation Res\n");
  for (auto &P : LoopPermutation) {
    LLVM_DEBUG(dbgs() << P->getNestingLevel() << " ";);
    (void)P;
  }
  LLVM_DEBUG(dbgs() << "\n";);
}

void populateTCs(LoopNestTCTy &LoopNestTC) {

  for (auto Level : make_range(LoopNestTC.level_from_outer_begin(),
                               LoopNestTC.level_from_outer_end())) {
    uint64_t ConstantTrip = 0;
    const HLLoop *Lp = LoopNestTC.getLoop(Level);
    assert(Lp);
    if (Lp->isConstTripLoop(&ConstantTrip)) {
      LoopNestTC[Level] = ConstantTrip;
    } else {
      LoopNestTC[Level] = 0;
    }
  }
}

// From InnermostLoop to OutermostLoop, return the consecutive depth
// where TC is at least a certain threshold.
// NewOutermost will be updated if the given OutermostLoop's TC
// is smaller then the threshold.
unsigned calcConsecutiveDepthOverTCThreshold(const LoopNestTCTy &LoopToTC,
                                             HLLoop *&NewOutermost) {

  // Scan from Innermost outerward
  // See if TC is constant and over a certain threshold
  NewOutermost = const_cast<HLLoop *>(LoopToTC.Innermost);
  unsigned ConsecutiveDepth = 0;
  unsigned Level = LoopToTC.Innermost->getNestingLevel();
  for (const HLLoop *Lp = LoopToTC.Innermost,
                    *ELp = LoopToTC.Outermost->getParentLoop();
       Lp != ELp; Lp = Lp->getParentLoop(), Level--) {
    uint64_t TCAtLevel = LoopToTC[Level];
    // non-const TC has TCAtLevel zero
    if (TCAtLevel < (uint64_t)LoopBlockingTCThreshold) {
      if (!EnableLoopBlockingNonConstTC || LoopNestTCTy::isConstTC(TCAtLevel)) {
        break;
      }
    }
    ConsecutiveDepth++;
    NewOutermost = const_cast<HLLoop *>(Lp);
  }

  return ConsecutiveDepth;
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

// See if the loops in StripmineCandidateMap can be interchanged
// outside OutermostLoop.
bool isLegalToInterchange(const LoopMapTy &StripmineCandidateMap,
                          const HLLoop *OutermostLoop,
                          const HLLoop *InnermostLoop, HIRDDAnalysis &DDA,
                          HIRSafeReductionAnalysis &SRA, bool RefineDV,
                          ArrayRef<unsigned> DelinearizedBlobIndices = {}) {

  SRA.computeSafeReductionChains(OutermostLoop);

  // For temps, consider only temps that are live-in.
  // Other temps are OK to ignore for DV checks.
  SpecialSymbasesTy TempSBsToConsider;
  for (auto I : llvm::make_range(InnermostLoop->live_in_begin(),
                                 InnermostLoop->live_in_end())) {
    TempSBsToConsider.insert(I);
  }

  // Collect DVs
  SmallVector<std::pair<DirectionVector, unsigned>, 16> DVs;
  DDUtils::computeDVsForPermuteWithSBs(DVs, OutermostLoop,
                                       InnermostLoop->getNestingLevel(), DDA,
                                       SRA, RefineDV, &TempSBsToConsider);

  unsigned OutermostLevel = OutermostLoop->getNestingLevel();
  for (auto &DV : DVs) {
    if (!DelinearizedBlobIndices.empty()) {
      // Search DV's blobIndex
      auto It = std::find(DelinearizedBlobIndices.begin(),
                          DelinearizedBlobIndices.end(), DV.second);
      if (It != DelinearizedBlobIndices.end())
        // This DV can be ignored, since can be interpreted as
        // all equal (=, =, ..., =)
        continue;
    }
    // TODO: It looks to me instead of for loop, we can only check
    //       maximum level (i.e. innermost level of ToStripLevels),
    //       after going over isValidToBlock's logic. But not completely sure
    //       for now.
    for (auto &Lp : StripmineCandidateMap) {
      if (!isValidToBlock(DV.first, OutermostLevel,
                          Lp.first->getNestingLevel())) {
        return false;
      }
    }
  }
  return true;
}

class RefAnalyzer {
public:
  enum RefAnalysisResult {
    RESULT_START = 0,
    OK, // de-linearized SIV form
    NON_LINEAR,
    NON_SIV,
  };

  static bool hasNonLinear(RefAnalysisResult Res) { return Res == NON_LINEAR; }
  static bool isSIV(RefAnalysisResult Res) { return Res == OK; }

  static RefAnalysisResult analyzeRefs(SmallVectorImpl<RegDDRef *> &Refs,
                                       bool ReplaceRefs = true) {
    if (std::any_of(Refs.begin(), Refs.end(),
                    [](const RegDDRef *Ref) { return Ref->isNonLinear(); })) {
      return NON_LINEAR;
    }

    bool IsNonSIV = false;
    for (auto *Ref : Refs) {
      for (auto *CE : make_range(Ref->canon_begin(), Ref->canon_end())) {
        if (CE->numIVs() > 1 || CE->numBlobs() > 0 || CE->hasIVBlobCoeffs()) {
          IsNonSIV = true;
          break;
        }
      }

      if (IsNonSIV) {
        break;
      }
    }

    if (IsNonSIV) {
      // Try the second chance with delinearization.

      // Bail out if any ref has more than one dimension.
      // TODO: fold this logic into the util.
      if (std::any_of(Refs.begin(), Refs.end(), [](const RegDDRef *Ref) {
            return !Ref->isSingleCanonExpr();
          })) {
        return NON_SIV;
      }

      SmallVector<RegDDRef *, 8> DelinearizedRefs;
      SmallVector<BlobTy, MaxLoopNestLevel> GroupSizes;
      if (!DDRefUtils::delinearizeRefs(Refs, DelinearizedRefs, &GroupSizes)) {
        return NON_SIV;
      }

      assert(DelinearizedRefs.size() == Refs.size());
      LLVM_DEBUG_DELINEAR(dbgs() << "Delinearized refs:\n";
                          for (auto Pair
                               : zip(Refs, DelinearizedRefs)) {
                            std::get<0>(Pair)->dump();
                            dbgs() << " -> ";
                            std::get<1>(Pair)->dump();
                            dbgs() << "\n";
                          });

      if (ReplaceRefs) {
        std::copy(DelinearizedRefs.begin(), DelinearizedRefs.end(),
                  Refs.begin());
      }
    }

    return OK;
  }
};

/// Checks if the innermost loop body has a certain stencil pattern.
/// Checks
///  - kinds of binary operations.
///  - Per memref group (grouping is done by RefGrouper)
///    + Get the median by compareMemRefAddress.
///    + Check all other memrefs in the same group has
///      a constant distance.
///    + for a 3-D reference, upto 2 dimensions can have different
///      from those of median ref.
class StencilChecker {
public:
  StencilChecker(RefGrouper::RefGroupVecTy &Groups, const HLLoop *Innermost, StringRef Func)
      : Groups(Groups), InnermostLoop(Innermost),
        MaxLevel(InnermostLoop->getNestingLevel()), MinLevel(MaxLevel + 1),
        Func(Func){};

  // One time check, only dependent on the innermost loopbody.
  // We only consider loop blocking of the perfect loopnest.
  bool isStencilForm() {
    if (!scanLoopBody()) {
      printDiag(NO_STENCIL_LOOP_BODY, Func, InnermostLoop);
      return false;
    }

    unsigned PrevMinimumLevel = MinLevel;
    for (unsigned I = 0, E = Groups.size(); I < E; I++) {
      unsigned Min;
      if (!scanDiffsFromMedian(Groups[I], Min)) {
        printDiag(NO_STENCIL_MEM_REFS, Func, InnermostLoop);
        return false;
      }

      if (I != 0 && PrevMinimumLevel != Min) {
        printDiag(NO_STENCIL_MEM_REFS, Func, InnermostLoop);
        return false;
      }

      PrevMinimumLevel = Min;
    }

    MinLevel = PrevMinimumLevel;

    return true;
  }

  /// Returns true if the Median Ref has all IVs in the range
  /// [OutermostLevel, InnermostLevel].
  /// To be called for each new loopnest.
  bool hasAllLevels(unsigned OutermostLevel, unsigned InnermostLevel) const {
    return InnermostLevel == MaxLevel && OutermostLevel >= MinLevel;
  }

private:
  bool getMinLevel(const RegDDRef *Median, unsigned &MinimumLevel) {
    // Set Max and MinLevel of Median
    // [MinLevel, MaxLevel] are consecutive levels where those IVs are
    // appearing in Median
    MinimumLevel = MaxLevel + 1;
    for (auto Level = InnermostLoop->getNestingLevel(); Level >= 1; Level--) {
      bool LevelFound = false;
      for (const CanonExpr *CE :
           make_range(Median->canon_begin(), Median->canon_end())) {
        if (CE->hasIV(Level)) {
          LevelFound = true;
          break;
        }
      }
      if (LevelFound)
        MinimumLevel = Level;
    }

    return MaxLevel >= MinimumLevel;
  }

  bool scanDiffsFromMedian(RefGrouper::RefGroupTy &Group, unsigned &MinimumLevel) {

    std::nth_element(Group.begin(), Group.begin() + Group.size() / 2,
                     Group.end(), DDRefUtils::compareMemRefAddress);
    const RegDDRef *Median = Group[Group.size() / 2];
    LLVM_DEBUG_DELINEAR(dbgs() << "Median :"; Median->dump(); dbgs() << " ";);
    if (!getMinLevel(Median, MinimumLevel)) {
      LLVM_DEBUG_DELINEAR(dbgs() << "Fail MinLevel\n");
      return false;
    }

    unsigned numCEs = Median->getNumDimensions();

    for (auto *Ref : Group) {
      if (Ref->getNumDimensions() != numCEs)
        return false;

      unsigned numNonZeroDist = 0;
      for (auto DimNum :
           make_range(Ref->dim_index_begin(), Ref->dim_index_end())) {
        int64_t Dist = 0;
        if (!CanonExprUtils::getConstDistance(Median->getDimensionIndex(DimNum),
                                              Ref->getDimensionIndex(DimNum),
                                              &Dist))
          return false;

        if (Dist != 0)
          numNonZeroDist++;
      }

      // At leas one dimension has const dist of zero.
      // TODO: This solves rhs_body, but might need more checks to be
      //       conservative enough as stencil function
      if (numNonZeroDist >= numCEs) {
        LLVM_DEBUG_DELINEAR(
            dbgs() << "Problem Ref:"; Ref->dump(); dbgs() << "\n";
            dbgs() << "F 4: group size: " << Group.size() << "\n";
            for (auto *Ref
                 : Group) {
              Ref->dump();
              dbgs() << "\n";
            });
        return false;
      }
    }

    return true;
  }

  bool scanLoopBody() const {
    bool LoadInstSeen = false;
    bool StoreInstSeen = false;
    bool StencilBinaryInstSeen = false;
    for (const HLNode &Node :
         make_range(InnermostLoop->child_begin(), InnermostLoop->child_end())) {
      const HLInst *HInst = dyn_cast<HLInst>(&Node);
      if (!HInst)
        continue;

      const Instruction *Inst = HInst->getLLVMInstruction();

      if (isa<LoadInst>(Inst)) {
        LoadInstSeen = true;
        continue;
      }

      if (isa<StoreInst>(Inst)) {
        StoreInstSeen = true;
        // TODO: for a Store, find the def of RVal temp,
        //       and the temp is defined by Add/FAdd.
        //       Currently, we don't have DD-edges to track
        //       for cactus.
        continue;
      }

      if (isa<BinaryOperator>(Inst)) {
        if (!isStencilBinaryOperator(Inst->getOpcode())) {
          LLVM_DEBUG_DELINEAR(dbgs()
                              << "Opcode: " << Inst->getOpcode() << "\n");
          return false;
        }
        StencilBinaryInstSeen = true;
      }
    }

    return LoadInstSeen && StoreInstSeen && StencilBinaryInstSeen;
  }

  bool isStencilBinaryOperator(unsigned Opcode) const {
    return Opcode == Instruction::Add || Opcode == Instruction::FAdd ||
           Opcode == Instruction::Sub || Opcode == Instruction::FSub ||
           Opcode == Instruction::Mul || Opcode == Instruction::FMul ||
           Opcode == Instruction::SDiv || Opcode == Instruction::UDiv ||
           Opcode == Instruction::FDiv;
  }

private:
  RefGrouper::RefGroupVecTy &Groups;
  const HLLoop *InnermostLoop;
  // All levels in [MinLevel, MaxLevel] appear (as IV) in every Group of
  // Groups.
  // Common maximum level for all groups
  unsigned MaxLevel;
  // Common minium level for all groups
  unsigned MinLevel;

  // Used only for diagnosis or debug purposes.
  StringRef Func;
};

// Arg is in-out.
// Returns true if the argument has changed.
bool updateLoopMapByStripmineApplicability(LoopMapTy &StripmineCandidateMap) {
  if (StripmineCandidateMap.empty()) {
    LLVM_DEBUG_DIAG_DETAIL(dbgs() << "LoopMap is empty.\n");

    return false;
  }

  bool IsUpdated = false;
  for (auto It = StripmineCandidateMap.begin(),
            EIt = StripmineCandidateMap.end();
       It != EIt;) {
    if (!It->first->isStripmineRequired(It->second)) {
      LLVM_DEBUG_DIAG_DETAIL(dbgs() << "Stripmine is not required\n");

      It = StripmineCandidateMap.erase(It);
      IsUpdated = true;
    } else if (!It->first->canStripmine(It->second)) {
      LLVM_DEBUG_DIAG_DETAIL(dbgs() << "Stripmine can not be done\n");

      It = StripmineCandidateMap.erase(It);
      IsUpdated = true;
    } else {
      ++It;
    }
  }

  return IsUpdated;
}

// Profitablity check based on KandR book algorithm.
// Checks whether memrefs with missing loop induction variables exist.
class KAndRChecker {
public:
  KAndRChecker(const MemRefGatherer::VectorTy &Refs, StringRef Func)
      : Refs(Refs), Func(Func) {
    NumRefsWithSmallStrides.resize(MaxLoopNestLevel + 1, 0);
    NumRefsMissingAtLevel.resize(MaxLoopNestLevel + 1, 0);
    countProBlockingRefs(Refs);
  }

  void check(unsigned LoopNestDepth, unsigned ConsecutiveDepth,
             const HLLoop *InnermostLoop, const HLLoop *OutermostLoop,
             LoopMapTy &StripmineCandidateMap) {

    assert(StripmineCandidateMap.empty());

    // Temporary comment-out for cactus
    // TODO: Find a way to avoid calling calcMaxVariantDimension inside loop
    // body.
    //       Maybe logic can be changed to
    //       guarantee that MaxDimension monotously decreased.
    unsigned MaxDimension =
        calcMaxVariantDimension(OutermostLoop->getNestingLevel());
    if (!DisableLoopDepthCheck) {
      // A heuristic choice: Choose not to block. Stop here.
      // This check is useful for blocking typical matrix multiplication.
      if (LoopNestDepth <= MaxDimension) {
        LLVM_DEBUG(dbgs() << "Failed: at MaxDimension < LoopNestDepth "
                          << MaxDimension << "," << LoopNestDepth << "\n");
        printDiag(NO_KANDR_DEPTH_TEST_1, Func, OutermostLoop,
                  "No Blocking: ", 2);
        // No more innerloop nest.
        return;
      }
    }

    if (ConsecutiveDepth <= MaxDimension) {
      LLVM_DEBUG(dbgs() << "Failed: at MaxDimension < ConsecutiveDepth "
                        << MaxDimension << "," << ConsecutiveDepth << "\n");
      printDiag(NO_KANDR_DEPTH_TEST_2, Func, OutermostLoop, "No Blocking: ", 2);
      // No more innerloop nest.
      return;
    }

    determineProfitableStripmineLoop(InnermostLoop, OutermostLoop,
                                     StripmineCandidateMap);
  }

private:
  void countProBlockingRefs(ArrayRef<RegDDRef *> Refs) {

    for (const RegDDRef *Ref : Refs) {
      for (auto Level :
           make_range(AllLoopLevelRange::begin(), AllLoopLevelRange::end())) {
#if 0
        // Commented out for now because of getConstStrideAtLevel
        // does not work on detached RegDDRefs.
        // We could have detached "de-linearized" DDRefs.
        // TODO: extend getConstStrideAtLevel() to work on
        //       detached RegDDRefs and enable this part again.
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
#endif

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
  }

  // Returns the largest number of dimensions in all refs
  // in the given Refs.
  // It does not count invariant subscript at OutermostLoopLevel.
  // For e.g. with {A[i1][i2], B[i2], C[5][i2][i1]},
  // 2 will be returned because 2 is maximun of {2, 1, 2}.
  unsigned calcMaxVariantDimension(unsigned OutermostLoopLevel) {
    unsigned Max = 0;
    for (RegDDRef *Ref : Refs) {
      unsigned NumVariantDimensions = 0;

      for (auto *CE : make_range(Ref->canon_begin(), Ref->canon_end())) {

        if (CE->isInvariantAtLevel(OutermostLoopLevel)) {
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
  bool determineProfitableStripmineLoop(const HLLoop *InnermostLoop,
                                        const HLLoop *OutermostLoop,
                                        LoopMapTy &StripmineCandidateMap) {

    assert(StripmineCandidateMap.empty());

    unsigned InnerLevel = InnermostLoop->getNestingLevel();
    unsigned OuterLevel = OutermostLoop->getNestingLevel();

    // Stripmining from the parent of the innermost
    // Scan from inner to outer, and choose loops to stripmine
    // based on NumRefsMissingAtLevel and NumRefsWithSmallStridesAtLevel
    unsigned Level = InnerLevel - 1;
    unsigned NumTotalLoops = InnerLevel;
    bool IsCandFound = false;
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
      if (NumRefsMissingAtLevel[Level] > 0 ||
          NumRefsWithSmallStrides[Level] > 0) {

        NumTotalLoops++;

        // KAndR blocks its inner loop. Otherwise, the loop.
        HLLoop *ToStripmine = LoopBlockingAlgorithm == KAndR
                                  ? const_cast<HLLoop *>(InnerLp)
                                  : const_cast<HLLoop *>(Lp);
        LLVM_DEBUG(dbgs() << "Loop at Level " << ToStripmine->getNestingLevel()
                          << " will be stripmined\n");

        markAsToStripmine(ToStripmine, StripmineCandidateMap);

        IsCandFound = true;
      }
    }

    // Look into the innermost loop again for blocking when algo is MatMul.
    // Experiments in skx showed blocking all three levels of matrix
    // multiplication gives best performance.
    if (LoopBlockingAlgorithm == MatMul &&
        (std::any_of(std::next(NumRefsMissingAtLevel.begin(), OuterLevel),
                     std::next(NumRefsMissingAtLevel.begin(), InnerLevel + 1),
                     [](int Num) { return Num > 0; }))) {

      LLVM_DEBUG(dbgs() << "* Loop at Level "
                        << InnermostLoop->getNestingLevel()
                        << " will be stripmined\n");

      markAsToStripmine(InnermostLoop, StripmineCandidateMap);

      IsCandFound = true;
    }

    LLVM_DEBUG(dbgs() << "determineProfitableStipmineLoop result: "
                      << IsCandFound << "\n");
    return IsCandFound;
  }

private:
  const MemRefGatherer::VectorTy &Refs;
  // Used only for diagnosis or debug purposes.
  StringRef Func;
  SmallVector<int, MaxLoopNestLevel + 1> NumRefsWithSmallStrides;
  SmallVector<int, MaxLoopNestLevel + 1> NumRefsMissingAtLevel;
};

class StripmineSizeExplorerByDefault {
public:
  StripmineSizeExplorerByDefault(const LoopMapTy &StripmineSizes)
      : StripmineSizes(StripmineSizes) {}

  void explore(LoopMapTy &LoopMap, HLLoop *OutermostLoop) {
    // TODO: OutermostLoop is not used.
    //       Dummy for conforming to a format
    pullFixedStripmineSize(StripmineSizes, LoopMap);
    updateLoopMapByStripmineApplicability(LoopMap);
  }

private:
  void pullFixedStripmineSize(const LoopMapTy &StripmineSizes,
                              LoopMapTy &LoopMap) {
    for (auto &Pair : LoopMap) {
      Pair.second = StripmineSizes.at(Pair.first);
    }
  }

private:
  const LoopMapTy &StripmineSizes;
};

// Different Outermost Loop is tried.
// Outermost loop's level increased(i.e. inner-loop) per try.
// When the first legal/profitable loopnest is found, tries stop.
// Used with KAndRChecker + fixed Stripmine size.
// To be used with memoryfootprint-based stripmine size explorer.
template <typename T>
HLLoop *exploreLoopNest(HLLoop *InnermostLoop, HLLoop *OutermostLoop,
                        unsigned ConsecutiveDepth,
                        KAndRChecker &KAndRProfitability,
                        T &StripmineSizeExplorer, HIRDDAnalysis &DDA,
                        HIRSafeReductionAnalysis &SRA, StringRef Func,
                        LoopMapTy &LoopMap) {

  unsigned LoopNestDepth =
      InnermostLoop->getNestingLevel() - OutermostLoop->getNestingLevel() + 1;

  for (HLLoop *Lp = OutermostLoop; Lp != InnermostLoop;
       Lp = cast<HLLoop>(Lp->getFirstChild())) {

    if (Lp->isMVFallBack()) {
      // We don't need to look inner loops because
      // all the innerloops are all multi-versioned.
      printDiag(MULTIVERSIONED_FALLBACK_LOOP, Func);
      break;
    }

    LoopMap.clear();
    KAndRProfitability.check(LoopNestDepth, ConsecutiveDepth, InnermostLoop, Lp,
                             LoopMap);
    LoopNestDepth--;
    ConsecutiveDepth--;

    // Try inner loopnest
    if (LoopMap.empty()) {
      printDiag(NO_MISSING_IVS_OR_SMALL_STRIDES, Func);
      continue;
    }

    StripmineSizeExplorer.explore(LoopMap, Lp);

    if (LoopMap.empty()) {
      // Even with inner loop nest, it will be unlikely to
      // see the difference.
      printDiag(NO_STRIPMINE_APPL, Func);
      break;
    }

    if (isLegalToInterchange(LoopMap, Lp, InnermostLoop, DDA, SRA, false)) {
      // Deepest candidate nest is found. At this point, we don't worry about
      // trying inner loop nests. Record the result and return with success;
      return Lp;
    } else {
      printDiag(NO_INTERCHANGE, Func);
    }
  } // end of loopnest exploration

  return nullptr;
}

void pull3DStencilSmallStripmineSizes(const LoopNestTCTy &LoopNest,
                                      LoopMapTy &LoopMap) {
  // Special casing with (none * 8 * none)
  // Block only the middle loop with 8
  int Count = 0;
  for (auto Level : make_range(LoopNest.level_from_outer_begin(),
                               LoopNest.level_from_outer_end())) {
    Count++;
    if (Count != 2)
      continue;

    LoopMap[LoopNest.getLoop(Level)] = 8;
  }

  assert(Count == 3);
}

// Used with 3d-stencil pattern check
HLLoop *exploreLoopNest(HLLoop *InnermostLoop, HLLoop *OutermostLoop,
                        StencilChecker &StencilProfitability,
                        HIRDDAnalysis &DDA, HIRSafeReductionAnalysis &SRA,
                        StringRef Func, LoopMapTy &LoopMap) {

  LoopNestTCTy LoopNest(OutermostLoop, InnermostLoop);
  auto &DelinearizableBlobIndices =
      OutermostLoop->getMVDelinearizableBlobIndices();

  for (HLLoop *Lp = OutermostLoop; Lp != InnermostLoop;
       Lp = cast<HLLoop>(Lp->getFirstChild())) {

    if (Lp->isMVFallBack()) {
      // We don't need to look inner loops because
      // all the innerloops are all multi-versioned.
      printDiag(MULTIVERSIONED_FALLBACK_LOOP, Func);
      break;
    }

    // Special casing for 3-D stencil
    unsigned OutermostLevel = Lp->getNestingLevel();
    unsigned InnermostLevel = InnermostLoop->getNestingLevel();
    unsigned Depth = InnermostLevel - OutermostLevel + 1;

    if (Depth > 3)
      continue;

    if (Depth < 3)
      break;

    if (!StencilProfitability.hasAllLevels(Lp->getNestingLevel(),
                                           InnermostLoop->getNestingLevel())) {
      continue;
    }

    // fill in LoopMap with [Outermost, Innermost]
    LoopNest.setOutermostLoop(Lp);
    LoopNest.populateLoops();
    pull3DStencilSmallStripmineSizes(LoopNest, LoopMap);
    updateLoopMapByStripmineApplicability(LoopMap);

    if (LoopMap.empty()) {
      // Even with inner loop nest, it will be unlikely to
      // see the difference.
      printDiag(NO_STRIPMINE_APPL, Func);
      break;
    }

    if (isLegalToInterchange(LoopMap, Lp, InnermostLoop, DDA, SRA, false,
                             DelinearizableBlobIndices)) {
      // Deepest candidate nest is found. At this point, we don't worry about
      // trying inner loop nests. Record the result and return with success;
      return Lp;
    } else {
      printDiag(NO_INTERCHANGE, Func);
    }
  }

  return nullptr;
}

HLLoop *tryKAndRWithFixedStripmineSizes(
    const MemRefGatherer::VectorTy &Refs, const LoopNestTCTy &LoopNestTC,
    HLLoop *InnermostLoop, HLLoop *OutermostLoop, unsigned ConsecutiveDepth,
    HIRDDAnalysis &DDA, HIRSafeReductionAnalysis &SRA, StringRef Func,
    LoopMapTy &LoopMap) {

  // Try K&R + fixed stripmine sizes
  // Just use existing logic for now to avoid regression
  KAndRChecker KAndRProfitability(Refs, Func);
  LoopMapTy StripmineSizes;
  adjustBlockSize(LoopNestTC, StripmineSizes);

  StripmineSizeExplorerByDefault StripmineExplorer(StripmineSizes);
  HLLoop *ValidOutermost = exploreLoopNest(
      InnermostLoop, OutermostLoop, ConsecutiveDepth, KAndRProfitability,
      StripmineExplorer, DDA, SRA, Func, LoopMap);
  if (ValidOutermost) {
    return ValidOutermost;
  }

  return nullptr;
}

// Returns the outermost loop where blocking will be applied
// in the range of [outermost, InnermostLoop]
HLLoop *findLoopNestToBlock(HIRDDAnalysis &DDA, HIRSafeReductionAnalysis &SRA,
                            HLLoop *InnermostLoop, bool Advanced,
                            StringRef Func, LoopMapTy &LoopMap) {

  // Get the highest outermost ancestor of given InnermostLoop
  // that can make a perfect loop nest.
  bool IsNearPerfect = false;
  const HLLoop *HighestAncestor =
      HLNodeUtils::getHighestAncestorForPerfectLoopNest(InnermostLoop,
                                                        IsNearPerfect);

  // Sink-pass is run before, so all near-perfect would have been perfect by
  // now.
  // If it is still IsNearPerfect, sinking pass was not able to
  // enable a perfect loop nest. Just bail out.
  if (IsNearPerfect) {
    printDiag(NO_PERFECTNEST, Func);
    return nullptr;
  }

  if (!HighestAncestor) {
    // Blocking is not applied to depth-1 loop nest.
    printDiag(INNERMOST_LOOP_NO_DO_LOOP, Func, InnermostLoop);

    return nullptr;
  }

  if (HighestAncestor == InnermostLoop) {
    // Blocking is not applied to depth-1 loop nest.
    printDiag(INNERMOST_LOOP_ONLY, Func);

    return nullptr;
  }

  if (HighestAncestor->isMVFallBack()) {
    printDiag(MULTIVERSIONED_FALLBACK_LOOP, Func, HighestAncestor);
    return nullptr;
  }

  // Check using MemRefs
  // TODO: see if pre-header and post-exit refs better be added.
  //       Currently, they are not added.
  MemRefGatherer::VectorTy Refs;
  MemRefGatherer::gatherRange(InnermostLoop->child_begin(),
                              InnermostLoop->child_end(), Refs);

  RefAnalyzer::RefAnalysisResult RefKind =
      RefAnalyzer::analyzeRefs(Refs, Advanced);

  // If any Ref is a non-linear, give up here.
  if (RefAnalyzer::hasNonLinear(RefKind)) {
    printDiag(NON_LINEAR_REFS, Func);
    return nullptr;
  }

  LoopNestTCTy LoopNestTC(HighestAncestor, InnermostLoop);
  LoopNestTC.populateLoops();
  populateTCs(LoopNestTC);
  HLLoop *AdjustedHighestAncestor = InnermostLoop;

  // TODO: This logic is not directly relavant to StencilChecker or
  //       MemFootPrint-based stripmine-size explorer.
  //       Move/adjust.
  //       As this logic is moved/modified LoopNestTC can be moved.
  unsigned ConsecutiveDepth =
      calcConsecutiveDepthOverTCThreshold(LoopNestTC, AdjustedHighestAncestor);
  LLVM_DEBUG(dbgs() << "ConsecutiveDepth: " << ConsecutiveDepth << "\n");

  // Uniq refs for the purpose of memory footprint analysis.
  // For now, we won't do any memory footprint analysis.
  if (!CommandLineBlockSize && RefAnalyzer::isSIV(RefKind) &&
      (!OldVersion || Advanced)) {

    // Try K&R as default.
    HLLoop *ValidOutermost = tryKAndRWithFixedStripmineSizes(
        Refs, LoopNestTC, InnermostLoop, AdjustedHighestAncestor,
        ConsecutiveDepth, DDA, SRA, Func, LoopMap);

    if (ValidOutermost) {
      printDiag(SUCCESS_BASIC_SIV, Func, AdjustedHighestAncestor, "SUCCESS");
      return ValidOutermost;
    } else {
      printDiag(NO_KANDR, Func, AdjustedHighestAncestor);
    }

    // Grouping Refs for memory foot print and for stencil
    RefGrouper::RefGroupVecTy Groups;
    RefGrouper Grouping(Refs, Groups);

    // Try stencil pattern + fixed stripmine sizes.
    StencilChecker StencilProfitability(Groups, InnermostLoop,
                                        Func);
    if (!StencilProfitability.isStencilForm()) {
      // No hope as a stencil with this innermost loop body.
      printDiag(NO_STENCIL_LOOP, Func, AdjustedHighestAncestor,
                "Summary: No Blocking in this loop nest 1");
      return nullptr;
    }

    // TODO: memoization of isLegalToInterchange could help compile-time.
    ValidOutermost =
        exploreLoopNest(InnermostLoop, AdjustedHighestAncestor,
                        StencilProfitability, DDA, SRA, Func, LoopMap);
    if (ValidOutermost) {
      printDiag(SUCCESS_STENCIL, Func, ValidOutermost, "SUCCESS");
      return ValidOutermost;
    }

    printDiag(NO_STENCIL_LOOP, Func, AdjustedHighestAncestor,
              "Summary: No Blocking in this loop nest 2");

  } else {
    // NON-SIV case - blocking with default size

    // Try K&R + fixed stripmine sizes
    // Just use existing logic for now to avoid regression
    HLLoop *ValidOutermost = tryKAndRWithFixedStripmineSizes(
        Refs, LoopNestTC, InnermostLoop, AdjustedHighestAncestor,
        ConsecutiveDepth, DDA, SRA, Func, LoopMap);

    if (ValidOutermost) {
      printDiag(SUCCESS_NON_SIV_OR_NON_ADVANCED, Func, AdjustedHighestAncestor,
                "SUCCESS");
      return ValidOutermost;
    } else {
      printDiag(NO_KANDR, Func, AdjustedHighestAncestor);
    }
  }

  printDiag(NO_PROFITABLE_BLOCKING_FOUND, Func, InnermostLoop,
            "Summary: No blocking from this innermost Loop");

  return nullptr;
}

// LoopVectors contains loops at levels from [OutermostLoopLevel, ..]
// Whenever a loop's level is given, it should be adjusted by OutermostLoopLevel
// to index the correct entry of a container.
inline unsigned getIndexForLoopVectors(unsigned Level,
                                       unsigned OutermostLoopLevel) {
  return Level - OutermostLoopLevel;
}

// LoopPermutation: Info before permutation
// CurLoopNests : Info after permutation
void hoistMinDefs(const LoopMapTy &LoopMap,
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

    if (isNonByStripLoop(LpWithDef, LoopMap)) {
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
void doTransformation(HLLoop *OutermostLoop, HLLoop *InnermostLoop,
                      LoopMapTy &LoopMap, StringRef FuncName) {

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  if (PrintBlockedLoops) {
    dbgs() << "== Before blocking in " << FuncName << " == \n";
    OutermostLoop->dump();
  }
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

  InnermostLoop->setIsUndoSinkingCandidate(false);

  // Stripmine
  HLLoop *NewOutermostLoop =
      stripmineSelectedLoops(InnermostLoop, OutermostLoop, LoopMap);

  // Populate Permutation
  int TotalDepth = InnermostLoop->getNestingLevel() -
                   NewOutermostLoop->getNestingLevel() + 1;
  SmallVector<const HLLoop *, MaxLoopNestLevel> LoopPermutation(TotalDepth,
                                                                nullptr);
  populatePermutation(NewOutermostLoop, InnermostLoop, LoopMap,
                      LoopPermutation);

  // Interchange
  HIRTransformUtils::permuteLoopNests(NewOutermostLoop, LoopPermutation,
                                      InnermostLoop->getNestingLevel());

  SmallVector<HLLoop *, MaxLoopNestLevel> CurLoopNests;
  ForEach<HLLoop>::visit(NewOutermostLoop, [&CurLoopNests](HLLoop *Lp) {
    CurLoopNests.push_back(Lp);
  });

  // Add OptReport after permutation to get the correct information.
  LoopOptReportBuilder &LORBuilder =
      CurLoopNests.front()->getHLNodeUtils().getHIRFramework().getLORBuilder();
  for (auto Lp : CurLoopNests) {
    const HLLoop *OrigLoop = getLoopForReferingInfoBeforePermutation(
        Lp, LoopPermutation, CurLoopNests.front()->getNestingLevel());
    if (isBlockedLoop(OrigLoop, LoopMap)) {
      LORBuilder(*Lp).addRemark(OptReportVerbosity::Low, "blocked by %d",
                                LoopMap[OrigLoop]);
    }
  }

  // Hoist min var's definitions to Destination Levels
  LLVM_DEBUG(NewOutermostLoop->dump(1));
  hoistMinDefs(LoopMap, LoopPermutation, CurLoopNests);
  LLVM_DEBUG(dbgs() << "after hoist\n");
  LLVM_DEBUG(NewOutermostLoop->dump());

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  if (PrintBlockedLoops) {
    dbgs() << "== After blocking in " << FuncName << " == \n";
    NewOutermostLoop->dump();
  }
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

  // Invalidate
  NewOutermostLoop->getParentRegion()->setGenCode();
  HIRInvalidationUtils::invalidateLoopNestBody(NewOutermostLoop);
  HIRInvalidationUtils::invalidateParentLoopBodyOrRegion(NewOutermostLoop);
} // namespace

bool doLoopBlocking(HIRFramework &HIRF, HIRDDAnalysis &DDA,
                    HIRSafeReductionAnalysis &SRA, HIRLoopStatistics &HLS,
                    TargetTransformInfo &TTI) {

  // Collect innermost loops first
  // Many of collections of data can be applied because we are
  // working on perfect or near-perfect loop
  // TODO: replace with loop-only iterator
  //       Keeping all innermost loops in all regions is not ideal.
  SmallVector<HLLoop *, 32> InnermostLoops;
  (HIRF.getHLNodeUtils()).gatherInnermostLoops(InnermostLoops);
  bool Advanced = TTI.isAdvancedOptEnabled(
      TargetTransformInfo::AdvancedOptLevel::AO_TargetHasAVX2);

  bool Changed = false;
  for (auto *InnermostLoop : InnermostLoops) {

    if (HLS.getTotalLoopStatistics(InnermostLoop)
            .hasCallsWithUnsafeSideEffects())
      continue;

    LoopMapTy LoopMap;
    HLLoop *OutermostLoop =
        findLoopNestToBlock(DDA, SRA, InnermostLoop, Advanced,
                            HIRF.getFunction().getName(), LoopMap);

    if (!OutermostLoop)
      continue;

    LLVM_DEBUG(dbgs() << "Loop to Block: \n");
    LLVM_DEBUG(OutermostLoop->dump());
    assert(OutermostLoop != InnermostLoop);
    assert(!LoopMap.empty());

    doTransformation(OutermostLoop, InnermostLoop, LoopMap,
                     HIRF.getFunction().getName());

    Changed = true;
  }
  return Changed;
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
  auto &TTI = getAnalysis<TargetTransformInfoWrapperPass>().getTTI(F);
  return doLoopBlocking(HIRF, DDA, SRA, HLS, TTI);
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
                 AM.getResult<HIRLoopStatisticsAnalysis>(F),
                 AM.getResult<TargetIRAnalysis>(F));

  return PreservedAnalyses::all();
}
