//===--- HIRLoopBlocking.cpp - Implements Loop Blocking transformation ---===//
//
// Copyright (C) 2018-2021 Intel Corporation. All rights reserved.
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
//
//   Loops can also be blocked via pragma directives. Pragmas info for loop
//   blocking contains information for level and factor. A level of 1
//   corresponds to the immediate loop after the directive, whereas a level
//   of 2 would refer to the 2nd loop from where the directive is declared.
//   The blocking factor is the blocksize or stripmine size for that loop.
//
//   Example 1:
//   #pragma block_loop factor (16) level (1)
//   Do J = 1..N
//      Do K = 1..N
//         Do I = 1..N
//
//   Example 1 would block the J loop and insert a by-strip loop as outermost
//
//
//   Example 2:
//   Do J = 1..N
//      #pragma block_loop factor (16) level (2)
//      Do K = 1..N
//         Do I = 1..N
//
//   Example 2 would block the I loop and insert its by-strip loop above the K
//   loop like so. The by strip loop is always placed where the pragma is
//   declared.
//
//   Do J = 1..N
//      Do II = 1..N  -- by-strip loop
//          Do K = 1..N
//             Do I = 1..16
//
//   A pragma without level is the same as blocking the entire loop nest with
//   whatever factor is declared.
//
//   #pragma block_loop factor (16)   <------equivalent as the line below
//   #pragma block_loop factor (16) level (1:3) <--- blocking everything
//   Do J = 1..N
//      Do K = 1..N
//         Do I = 1..N
//
//   Pragma TODO:
//   1) Current blocking depends on loopnests being perfect to help prove
//   interchange legality. Consider relaxing requirements for sinking and
//   possibly interchange logic to facilitate pragma blocking. User specified
//   pragmas should assume the memory refs do not alias similar to 'restrict'
//   keyword. It should be possible to handle non-perfect loopnests this way.

#include "llvm/ADT/SmallSet.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/ADT/Triple.h"
#include "llvm/IR/Function.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Intel_LoopTransforms/HIRLoopBlockingPass.h"

#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRDDAnalysis.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRLocalityAnalysis.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRLoopStatistics.h"

#include "llvm/Analysis/Intel_LoopAnalysis/Framework/HIRFramework.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/DDUtils.h"

#include "HIRPrintDiag.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/DDRefGatherer.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/DDRefGrouping.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/ForEach.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HIRInvalidationUtils.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HLNodeUtils.h"
#include "llvm/Analysis/Intel_OptReport/OptReportOptionsPass.h"
#include "llvm/Analysis/TargetTransformInfo.h"
#include "llvm/Transforms/Intel_LoopTransforms/HIRTransformPass.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/HIRTransformUtils.h"

#include "HIRLoopBlocking.h"
#include "HIRStencilPattern.h"

#define OPT_SWITCH "hir-loop-blocking"
#define OPT_DESC "HIR Loop Blocking"
#define OPT_SWITCH_PRAGMA "hir-pragma-loop-blocking"
#define OPT_DESC_PRAGMA "HIR Loop Blocking based on Pragma"
#define DEBUG_TYPE OPT_SWITCH

// Dump, delinearizaion and stencil-related messages.
#define LLVM_DEBUG_DELINEAR(X) DEBUG_WITH_TYPE(OPT_SWITCH "-delinear", X)
#define LLVM_DEBUG_DIAG_DETAIL(X) DEBUG_WITH_TYPE(OPT_SWITCH "-dump-detail", X)

using namespace llvm;
using namespace llvm::loopopt;
using namespace llvm::loopopt::blocking;

static cl::opt<bool> DisablePass("disable-" OPT_SWITCH, cl::init(false),
                                 cl::Hidden,
                                 cl::desc("Disable " OPT_DESC " pass"));

static cl::opt<bool>
    DisablePragmaPass("disable-" OPT_SWITCH_PRAGMA, cl::init(false), cl::Hidden,
                      cl::desc("Disable " OPT_DESC_PRAGMA " pass"));

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

// Never delinearize memrefs. Used for debugging.
static cl::opt<bool> ForceStopDelinearization(
    OPT_SWITCH "-no-delinear", cl::init(false), cl::Hidden,
    cl::desc("Unconditionally disable delinearization in " OPT_DESC " pass"));

// If this check is disabled, blocking is applied to non-constant TC or to
// constant trip count not large enough above the threshold.
static cl::opt<bool> EnableLoopBlockingNonConstTC(
    "enable-" OPT_SWITCH "-nonconst-trip-count", cl::init(true), cl::Hidden,
    cl::desc("Enable " OPT_DESC
             " pass even when some trip counts are non-const"));

// Flag to allow special sinking to occur for special loops
static cl::opt<bool> DisableSinkForMultiCopy(
    OPT_SWITCH "-disable-special-sink", cl::init(false), cl::Hidden,
    cl::desc(OPT_DESC "disable special sinking in HIR Loop Blocking"));
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
static cl::opt<uint64_t> LoopBlockingTCThreshold(
    OPT_SWITCH "-tc-threshold", cl::init(384), cl::Hidden,
    cl::desc("Threshold of trip counts in " OPT_DESC " pass"));

// Does not greatly affect global behavior of hir loop blocking.
static cl::opt<unsigned>
    MajorityStencilGroupThreshold(OPT_SWITCH "-stencil-group-threshold",
                                  cl::init(4), cl::Hidden,
                                  cl::desc(" " OPT_DESC " pass"));

// Upperbound for small stride in bytes.
// This knob is mainly for KAndR algorithm.
// Does not greatly affect global behavior of hir loop blocking.
static cl::opt<int> LoopBlockingStrideThreshold(OPT_SWITCH "-stride-threshold",
                                                cl::init(32), cl::Hidden,
                                                cl::desc(" " OPT_DESC " pass"));

static cl::opt<bool> OldVersion(OPT_SWITCH "-old-ver", cl::init(true),
                                cl::Hidden, cl::desc("Old " OPT_DESC " pass"));

static cl::opt<bool>
    SkipAntiPatternCheck(OPT_SWITCH "-skip-anti-pattern-check", cl::init(false),
                         cl::Hidden,
                         cl::desc("Skip loop blocking's anti pattern check"));

static std::array<std::string, NUM_DIAGS> DiagMap = createDiagMap();

void printDiag(DiagMsg Msg, StringRef FuncName, const HLLoop *Loop = nullptr,
               StringRef Header = "No Blocking: ", unsigned DiagLevel = 1) {

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  printDiag(PrintDiagFunc, PrintDiagLevel, DiagMap[Msg], FuncName, Loop, Header,
            DiagLevel);
#endif
}

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

// Utility that gets child loop for Loop Blocking LoopNests. Most are perfect
// loops, except By-strip loops might have loop boundary condition
const HLLoop *getChildLoop(const HLLoop *Loop, const LoopMapTy &LoopMap) {
  if (isNonByStripLoop(Loop, LoopMap)) {
    Loop = dyn_cast<HLLoop>(Loop->getFirstChild());
  } else {
    if (Loop->getNumChildren() == 1) {
      Loop = dyn_cast<HLLoop>(Loop->getFirstChild());
    } else if (Loop->getNumChildren() == 2) {
      Loop = dyn_cast<HLLoop>(Loop->getLastChild());
    } else {
      llvm_unreachable("Unexpected Loop with more than 2 children!");
    }
  }
  return Loop;
}

// Based on experiments.
// -O3 -xCORE-AVX512 -Ofast -mfpmath=sse -march=core-avx512
// In most cases, block size (i.e. stripmine size for stripmine() utility)
// 64 or 128 gave the best performance.
unsigned adjustBlockSize(uint64_t TC) {
  if (CommandLineBlockSize) {
    return CommandLineBlockSize;
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

// Return the by-strip loop for loopnest that has been stripmined using the
// relative offset specified by pragma. Relative offset ignores other bystrip
// loops. The found bystrip loop is placed where the pragma is declared.
const HLLoop *getByStripLoopatOffsetLevel(const HLLoop *Loop,
                                          const LoopMapTy &LoopMap,
                                          unsigned Level) {
  while (Level > 1) {
    Loop = getChildLoop(Loop, LoopMap);
    if (isNonByStripLoop(Loop, LoopMap)) {
      Level--;
    }
  }

  assert(isBlockedLoop(Loop, LoopMap) && "Expected loop to be blocked!\n");
  return Loop->getParentLoop();
}

// Check the entire loopnest starting from the innermost loop for pragmas
bool hasLoopBlockingPragma(HLLoop *InnermostLoop) {
  HLLoop *Loop = InnermostLoop;

  while (Loop) {
    auto PragmaInfo = Loop->getBlockingPragmaLevelAndFactors();
    if (!PragmaInfo.empty()) {
      return true;
    }
    Loop = Loop->getParentLoop();
  }
  return false;
}

// Scan entire loopnest for blocking pragmas and return the Outermost loop
// with such pragma. Save pragma levels and factors to LoopToPragma.
// Pragmas can be level specific, or apply to entire loopnest.
// Pragmas are processed in order. no_block pragma has highest priority, while
// valid pragmas honored on first come first serve basis.
// Final pragma setup is stored in \p LoopToPragma
HLLoop *getLoopBlockingPragma(HLLoop *InnermostLoop,
                              LoopPragmaMapTy &LoopToPragma) {
  HLLoop *OutermostPragmaLoop = nullptr;
  bool NoBlockPragma = false;

  // Find the OutermostPragmaLoop where pragma occurs
  for (HLLoop *Loop = InnermostLoop; Loop; Loop = Loop->getParentLoop()) {
    auto PragmaInfo = Loop->getBlockingPragmaLevelAndFactors();
    if (PragmaInfo.empty()) {
      continue;
    }
    OutermostPragmaLoop = Loop;
  }

  if (!OutermostPragmaLoop) {
    return nullptr;
  }

  // Pragmas are processed in lexical order
  for (HLLoop *Loop = OutermostPragmaLoop; Loop;
       Loop = dyn_cast<HLLoop>(Loop->getFirstChild())) {
    auto PragmaInfo = Loop->getBlockingPragmaLevelAndFactors();
    if (PragmaInfo.empty()) {
      continue;
    }
    RegDDRef *NoLevelFactor = nullptr;
    for (auto &Info : PragmaInfo) {
      if (Info.first != -1) {
        LoopToPragma[Loop].push_back(std::make_pair(Info.first, Info.second));
      } else {
        int64_t IntFactor;
        if (Info.second->isIntConstant(&IntFactor) && IntFactor == 0) {
          NoBlockPragma = true;
          break;
        }
        // Pragmalevel == -1 means no level was specified, and can only
        // occur once per Loop. Other pragmas will be ignored.
        assert(!NoLevelFactor &&
               "Multiple pragmas without levels not supported\n");
        NoLevelFactor = Info.second;
      }
    }
    // noblock pragma indicates no blocking for loopnest. Remove those, but
    // retain already processed pragma info for parent loops.
    // Consider the example:
    // #pragma block_loop factor(16)
    // for i = 0 ..
    //   ...
    //   #pragma noblock_loop
    //   for j = 0
    //     ...
    //
    // A blanket pragma would apply for entire loopnest but noblock pragma
    // for the inner loopnest would remove those pragmas already processed
    // for the outer loop.
    if (NoBlockPragma) {
      unsigned NoBlockLevel = Loop->getNestingLevel();
      for (auto It = LoopToPragma.begin(); It != LoopToPragma.end();) {
        unsigned PLoopLevel = It->first->getNestingLevel();
        for (auto VIt = It->second.begin(); VIt != It->second.end();) {
          unsigned LevelOffset = VIt->first;
          // For each loop with pragma info, check if looplevel + offset is
          // greater than the noblocklevel.
          // OffsetLevel is actually +1, so gt. is used
          if (PLoopLevel + LevelOffset > NoBlockLevel) {
            VIt = It->second.erase(VIt);
          } else {
            ++VIt;
          }
        }
        // Remove the LoopToPragma entry if it is empty after erasure
        if (It->second.empty()) {
          It = LoopToPragma.erase(It);
        } else {
          ++It;
        }
      }
      // No more pragmas are handled after no_block
      break;
    }

    if (!NoLevelFactor) {
      continue;
    }

    // Add Levels for -1 case
    int NumLevels = InnermostLoop->getNestingLevel() - Loop->getNestingLevel();
    for (int i = 0; i <= NumLevels; i++) {
      LoopToPragma[Loop].push_back(std::make_pair(i + 1, NoLevelFactor));
    }
  }

  // If no_block pragma results in no blocking, don't return outerloop
  if (LoopToPragma.empty()) {
    return nullptr;
  }

  LLVM_DEBUG(dbgs() << "Final LoopToPragma: \n"; for (auto &P
                                                      : LoopToPragma) {
    dbgs() << "LoopLevel: " << P.first->getNestingLevel() << "\n";
    for (auto &PP : P.second) {
      dbgs() << "Level: " << PP.first << ", ";
      PP.second->dump();
      dbgs() << "\n";
    }
  });

  return OutermostPragmaLoop;
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
    auto It = LoopMap.find(CurLoop);
    if (It == LoopMap.end() || It->second == 0) {
      continue;
    }

    BlockedLevels.push_back(CurLoopInfo.second);

    // By this time, we have already done normalization checks from analysis
    // However, special normalization may be required for stencil case, which
    // is called if regular normalization might fail.
    if (CurLoop->canStripmine(LoopMap[CurLoop])) {
      HIRTransformUtils::stripmine(CurLoop, CurLoop, LoopMap[CurLoop]);
    } else {
      HIRTransformUtils::stripmine(CurLoop, CurLoop, LoopMap[CurLoop], true);
    }

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

// Permutation for pragma follows the order in which the pragmas occur.
// By-strip loops are always moved to where the pragmas are declared.
// Take this 3 nested loop:
//
// for i1
//   for i2
//     for i3
//
//  If we block i1 and i3, then after stripmine, we have the following setup:
//
//  i1  -- by-strip loop
//    i2  -- blocked loop
//      i3
//        i4  -- by-strip loop
//          i5  -- blocked loop
//
// However, where the pragmas occur will change the permutation order.
// #pragma level (1)
// #pragma level (3)
// for i1
//   for i2
//     for i3
// will result in permutation of (1,4,2,3,5)
//
// #pragma level (1)
// for i1
//   #pragma level (2)
//   for i2
//     for i3
// will result in permutation of (1,2,4,3,5)
//
void populatePragmaPermutation(
    const HLLoop *Outermost, const HLLoop *Innermost, const LoopMapTy &LoopMap,
    const LoopPragmaMapTy &LoopToPragma,
    SmallVectorImpl<const HLLoop *> &LoopPermutation) {
  unsigned Front = 0;

  // Process Loop Pragmas from outermost to innermost. Pragmas are still
  // attached to blocked loops. When blocked loop is found, pull up all by-strip
  // loops from pragma, then set itself to get correct ordering
  for (const HLLoop *Lp = Outermost; Lp; Lp = getChildLoop(Lp, LoopMap)) {
    if (isNonByStripLoop(Lp, LoopMap)) {
      // Use pragma to assign permutations to all by-strip loops
      auto It = LoopToPragma.find(Lp);
      if (It != LoopToPragma.end()) {
        for (auto &Pair : It->second) {
          int Offset = Pair.first;
          const HLLoop *ByStripLoop =
              getByStripLoopatOffsetLevel(Lp, LoopMap, Offset);
          LoopPermutation[Front++] = ByStripLoop;
        }
      }
      LoopPermutation[Front++] = Lp;
    }
  }

  LLVM_DEBUG(dbgs() << "\n";);
  LLVM_DEBUG(dbgs() << "LoopPermutation Res\n");
  LLVM_DEBUG(
      for (auto &P
           : LoopPermutation) { dbgs() << P->getNestingLevel() << " "; });
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

HLLoop *getHighestAncestorWithTCThreshold(const LoopNestTCTy &LoopToTC,
                                          bool &AllConstTC) {
  HLLoop *NewOutermost = const_cast<HLLoop *>(LoopToTC.Innermost);
  unsigned Level = LoopToTC.Innermost->getNestingLevel();
  unsigned ConstTCCounts = 0;
  for (const HLLoop *Lp = LoopToTC.Innermost,
                    *ELp = LoopToTC.Outermost->getParentLoop();
       Lp != ELp; Lp = Lp->getParentLoop(), Level--) {
    uint64_t TCAtLevel = LoopToTC[Level];
    // non-const TC has TCAtLevel zero
    if (TCAtLevel < LoopBlockingTCThreshold) {
      if (!EnableLoopBlockingNonConstTC || LoopNestTCTy::isConstTC(TCAtLevel)) {
        break;
      }
    } else {
      ConstTCCounts++;
    }
    NewOutermost = const_cast<HLLoop *>(Lp);
  }

  unsigned OrigDepth = LoopToTC.Innermost->getNestingLevel() -
                       LoopToTC.Outermost->getNestingLevel() + 1;
  if (OrigDepth == ConstTCCounts)
    AllConstTC = true;

  return NewOutermost;
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

static RefAnalysisResult analyzeRefs(SmallVectorImpl<RegDDRef *> &Refs,
                                     HLLoop *Loop, bool ReplaceRefs = true) {
  bool HasNonLinear = false;
  for (const auto Ref : Refs) {
    unsigned SB = Ref->getSymbase();
    if (Ref->isNonLinear()) {
      HasNonLinear = true;
      // Allow blocking for nonlinear rvals
      // Non-linear refs are usually unprofitable. We can do profitability
      // checks later checked for IV computation (stencil B case)
      if (Loop->isLiveIn(SB) || Loop->isLiveOut(SB) || Ref->isLval()) {
        LLVM_DEBUG(dbgs() << "Found Nonlinear refs:\n";);
        return NON_LINEAR;
      }
    }
  }

  if (HasNonLinear) {
    return NON_LINEAR_READ_ONLY;
  }

  bool IsNonSIV = false;
  for (auto *Ref : Refs) {
    if (std::any_of(Ref->canon_begin(), Ref->canon_end(),
                    [](const CanonExpr *CE) {
                      return (CE->numIVs() > 1 || CE->numBlobs() > 0 ||
                              CE->hasIVBlobCoeffs());
                    })) {
      IsNonSIV = true;
      break;
    }
  }

  if (IsNonSIV) {
    // Try the second chance with delinearization.

    // Bail out if any ref has more than one dimension.
    // TODO: fold this logic into the util.
    if (std::any_of(Refs.begin(), Refs.end(), [](const RegDDRef *Ref) {
          return !Ref->isSingleDimension();
        })) {
      return NON_SIV;
    }

    SmallVector<RegDDRef *, 8> DelinearizedRefs;
    SmallVector<BlobTy, MaxLoopNestLevel> GroupSizes;
    if (!DDRefUtils::delinearizeRefs(Refs, DelinearizedRefs, &GroupSizes,
                                     true)) {
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
      std::copy(DelinearizedRefs.begin(), DelinearizedRefs.end(), Refs.begin());
    }
  }

  return SIV;
}

// The goal is to track all IVs that occur in memrefs for the loop. It
// is possible that memrefs have blobrefs instead of IVs like:
//
// %mat[i1][%mod6][i3 + 1]
//
// Trace through DDG to find the corresponding IV for tempblobs:
//
// %mod6 = i2 + 1  %  %"mat_times_vec_$N";
//
// We expect that the temp has an iv term like i2 here.
// If ModInst is a tempblob for Rem LHS term, we recurse again.
// IV level is aggregated into \p IVsAtLevel for later analysis.
static void getModBlobIVLevel(const BlobDDRef *BRef, DDGraph &DDG,
                              SmallVector<int, MaxLoopNestLevel> &IVsAtLevel,
                              DenseMap<const HLInst *, unsigned> &InstToIVmap) {
  for (const auto *Edge : DDG.incoming(BRef)) {
    const HLInst *ModInst = dyn_cast<HLInst>(Edge->getSrc()->getHLDDNode());
    if (!ModInst) {
      continue;
    }

    // Check if we've already found the IV for this inst
    auto It = InstToIVmap.find(ModInst);
    if (It != InstToIVmap.end()) {
      IVsAtLevel[It->second]++;
      continue;
    }

    const unsigned Opcode = ModInst->getLLVMInstruction()->getOpcode();
    if (Opcode != Instruction::SRem && Opcode != Instruction::URem) {
      continue;
    }

    // Check the LHS of Rem Instruction for IV term
    const auto *RemLHS = *ModInst->rval_op_ddref_begin();
    if (!RemLHS->isTerminalRef()) {
      continue;
    }

    const CanonExpr *CE = RemLHS->getSingleCanonExpr();
    if (CE->hasIV()) {
      unsigned IVLevel = CE->getFirstIVLevel();
      IVsAtLevel[IVLevel]++;
      InstToIVmap[ModInst] = IVLevel;
      LLVM_DEBUG(dbgs() << "IV found for Mod Inst: "; ModInst->dump(););
      continue;
    }

    // LLVM_DEBUG(dbgs() << "Mod Inst without IV found: "; ModInst->dump(););
    // In rare cases, we may see another blob instead of IV, like:
    // %mod62.i.i = %mod66 + 1  %  %"mat_times_vec_$NX_fetch34";
    // We can call function again with LHS BlobDDRef to trace with DDG
    if (RemLHS->numBlobDDRefs() == 1) {
      getModBlobIVLevel(*RemLHS->blob_begin(), DDG, IVsAtLevel, InstToIVmap);
    }
  }
}

// Checks if the innermost loop body has a certain stencil pattern.
// Checks
//  - kinds of binary operations.
//  - Per memref group (grouping is done by RefGrouper)
//    + Get the median by compareMemRefAddress.
//    + Check all other memrefs in the same group has
//      a constant distance.
//    + for a 3-D reference, upto 2 dimensions can have different
//      from those of median ref.
class StencilChecker {
  typedef DDRefGrouping::RefGroupVecTy<RegDDRef *> RefGroupVecTy;
  typedef DDRefGrouping::RefGroupTy<RegDDRef *> RefGroupTy;

public:
  StencilChecker(RefGroupVecTy &Groups, const HLLoop *Innermost, StringRef Func)
      : Groups(Groups), InnermostLoop(Innermost),
        MaxLevel(InnermostLoop->getNestingLevel()), MinLevel(MaxLevel + 1),
        Func(Func), Type(StencilType::NONE){};

  StencilType getStencilType() { return Type; }

  bool isProfitable(HIRDDAnalysis &DDA) {
    // Perfect stencils should always be profitable to block
    if (isStencilForm()) {
      printDiag(STENCIL_PROFIT, Func, InnermostLoop, "Stencil C");
      Type = StencilType::C;
      return true;
    }

    // Stencil is profitable when majority of stores have stencil pattern
    if (hasMajorityStencilRefs(DDA)) {
      printDiag(STENCIL_PROFIT, Func, InnermostLoop, "Stencil B");
      Type = StencilType::B;
      return true;
    }
    return false;
  }

  // Aggregates IVsAtLevel for this RefGroup. Take example ref:
  //
  // %A[i1][2][i3]
  //
  // Would accumulate the count for i1 and i3 in IVsAtLevel
  void aggregateAllRefIVs(const RefGroupTy &RefGroup,
                          SmallVector<int, MaxLoopNestLevel> &IVsAtLevel,
                          SmallVector<RegDDRef *, 16> &RefsWithModBlob) const {

    for (auto &Ref : RefGroup) {
      bool hasBlob = false;
      for (auto &CE : make_range(Ref->canon_begin(), Ref->canon_end())) {
        if (CE->numIVs() == 1) {
          IVsAtLevel[CE->getFirstIVLevel()]++;
        } else if ((CE->numBlobs() == 1) && (Ref->isNonLinear())) {
          // We only care about the %mod blob which we can trace inside the loop
          hasBlob = true;
        }
        // ignore constants or other CE types
      }

      if (hasBlob) {
        RefsWithModBlob.push_back(Ref);
      }
    }
  }

  // Look for multiple refgroups with majority stencil form.
  // Not all IVs must exactly be +/- in one dimension.
  // The mod blobs are pseudo stencil since they are IV +/- 1, but
  // we need to trace to the temp definition.
  bool hasMajorityStencilRefs(HIRDDAnalysis &DDA) {
    if (Groups.size() < MajorityStencilGroupThreshold) {
      LLVM_DEBUG(dbgs() << "Under Group Threshold Count: " << Groups.size()
                        << "\n";);
      return false;
    }

    unsigned StencilCount = 0;
    SmallVector<int, MaxLoopNestLevel> IVsAtLevel;
    IVsAtLevel.resize(MaxLoopNestLevel, 0);
    SmallVector<RegDDRef *, 16> RefsWithModBlob;

    for (auto &RefGroup : Groups) {
      if (RefGroup.size() < 2 ||
          DDRefUtils::isMemRefAllDimsConstOnly(RefGroup[0])) {
        continue;
      }

      // Check if refs exhibit stencil pattern with relaxed check.
      // Not all refs can be verified as stencil due to temp blobs.
      if (stencilpattern::areStructuallyStencilRefs(RefGroup, true)) {
        ++StencilCount;
      }

      // find all IV levels in memrefs
      aggregateAllRefIVs(RefGroup, IVsAtLevel, RefsWithModBlob);
    }

    // Magic number
    if (StencilCount < 4) {
      return false;
    }

    // Some refs may have IV based blob instead of IV, e.g:
    // (%X)[i3 + 2][i1 + 1][%mod + 1]
    // Use DDG to trace to the HLinst where %mod is defined
    // We want to verify that %mod is based on IV like so:
    // %mod = i2 + 1  %  %"$NY_fetch36" + 1;
    if (!RefsWithModBlob.empty()) {
      DDGraph DDG = DDA.getGraph(InnermostLoop);
      DenseMap<const HLInst *, unsigned> InstToIVmap;
      for (const auto Ref : RefsWithModBlob) {
        for (const auto &BRef :
             make_range(Ref->blob_begin(), Ref->blob_end())) {
          assert(BRef->getSingleCanonExpr()->numBlobs() == 1 &&
                 "CE expected to have single blob");
          getModBlobIVLevel(BRef, DDG, IVsAtLevel, InstToIVmap);
        }
      }
    }

    LLVM_DEBUG(int i = 0; for (auto IVs
                               : IVsAtLevel) {
      dbgs() << "Level - " << i++ << ", IVs - " << IVs << "\n";
    });

    // Target benchmark has 200+ IV counts for 3 levels
    unsigned Max = 0;
    unsigned ConsecutiveLevels = 0;
    for (unsigned IVLevel = 0; IVLevel < MaxLoopNestLevel; IVLevel++) {
      if (IVsAtLevel[IVLevel] > 100) {
        ConsecutiveLevels++;
      } else {
        if (ConsecutiveLevels) {
          Max = IVLevel - 1;
          break;
        }
      }
    }

    if (!ConsecutiveLevels) {
      return false;
    }

    // Set MinLevel here for later check - HasAllLevels
    MinLevel = Max - ConsecutiveLevels + 1;
    assert(Max == MaxLevel && "Stencil MaxLevel not innermost!");
    LLVM_DEBUG(dbgs() << "Stencil Count: " << StencilCount << "\n";);

    return true;
  }

  // One time check, only dependent on the innermost loopbody.
  // We only consider loop blocking of the perfect loopnest.
  bool isStencilForm() {
    if (!scanLoopBody(false)) {
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

    MinLevel = MinLevel <= PrevMinimumLevel ? MinLevel : PrevMinimumLevel;

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

  bool scanDiffsFromMedian(RefGroupTy &Group, unsigned &MinimumLevel) {

    const RegDDRef *Median = stencilpattern::getMedianRef(Group);
    LLVM_DEBUG_DIAG_DETAIL(dbgs() << "Median: "; Median->dump();
                           dbgs() << "\n";);

    if (!getMinLevel(Median, MinimumLevel)) {
      LLVM_DEBUG_DIAG_DETAIL(dbgs() << "Fail MinLevel\n");
      return false;
    }

    return stencilpattern::isSymetricCenteredAt(Median, Group);
  }

  // \p StrictlyStencil can be relaxed to allow non-stencil instructions
  // inside the loop as well.
  bool scanLoopBody(bool StrictlyStencil = true) const {
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
        continue;
      }

      if (isa<BinaryOperator>(Inst)) {
        if (!isStencilBinaryOperator(Inst->getOpcode()) && StrictlyStencil) {
          LLVM_DEBUG_DIAG_DETAIL(dbgs()
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
  RefGroupVecTy &Groups;
  const HLLoop *InnermostLoop;
  // All levels in [MinLevel, MaxLevel] appear (as IV) in every Group of
  // Groups.
  // Common maximum level for all groups
  unsigned MaxLevel;
  // Common minimum level for all groups
  unsigned MinLevel;

  // Used only for diagnosis or debug purposes.
  StringRef Func;
  StencilType Type;
};

// Arg is in-out.
// Returns true if the argument has changed.
// \p RelaxNormalization will allow normalization util to relax CE typecheck
bool updateLoopMapByStripmineApplicability(LoopMapTy &StripmineCandidateMap,
                                           bool RelaxNormalization = false) {
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
    } else if (!It->first->canStripmine(It->second, RelaxNormalization)) {
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

  void check(const HLLoop *InnermostLoop, const HLLoop *OutermostLoop,
             LoopMapTy &StripmineCandidateMap) {

    unsigned LoopNestDepth =
        InnermostLoop->getNestingLevel() - OutermostLoop->getNestingLevel() + 1;
    assert(StripmineCandidateMap.empty());

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
         Lp != ELp && NumTotalLoops < MaxLoopNestLevel;
         InnerLp = Lp, Lp = Lp->getParentLoop(), Level = Level - 1) {
      // Reused carried across Level
      // OR
      // See if any refs deepest dimension has this Level IV with a small stride
      LLVM_DEBUG(dbgs() << "NumRefsMissingAtLevel " << Level << ": "
                        << NumRefsMissingAtLevel[Level] << "\n";);
      LLVM_DEBUG(dbgs() << "NumRefsWithSmallStrides " << Level << ": "
                        << NumRefsWithSmallStrides[Level] << "\n");

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

    if (!IsCandFound)
      return false;

    // Look into the innermost loop again for blocking when algo is MatMul.
    // Experiments in skx showed blocking all three levels of matrix
    // multiplication gives best performance.
    if (NumTotalLoops < MaxLoopNestLevel && LoopBlockingAlgorithm == MatMul &&
        (std::any_of(std::next(NumRefsMissingAtLevel.begin(), OuterLevel),
                     std::next(NumRefsMissingAtLevel.begin(), InnerLevel + 1),
                     [](int Num) { return Num > 0; }))) {

      LLVM_DEBUG(dbgs() << "* Loop at Level "
                        << InnermostLoop->getNestingLevel()
                        << " will be stripmined\n");

      markAsToStripmine(InnermostLoop, StripmineCandidateMap);
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
                        KAndRChecker &KAndRProfitability,
                        T &StripmineSizeExplorer, HIRDDAnalysis &DDA,
                        HIRSafeReductionAnalysis &SRA, StringRef Func,
                        LoopMapTy &LoopMap) {

  for (HLLoop *Lp = OutermostLoop; Lp != InnermostLoop;
       Lp = cast<HLLoop>(Lp->getFirstChild())) {

    if (Lp->isMVFallBack()) {
      // We don't need to look inner loops because
      // all the innerloops are all multi-versioned.
      printDiag(MULTIVERSIONED_FALLBACK_LOOP, Func);
      break;
    }

    LoopMap.clear();
    KAndRProfitability.check(InnermostLoop, Lp, LoopMap);
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
                                      LoopMapTy &LoopMap, StencilType Type) {

  assert(std::distance(LoopNest.level_from_outer_begin(),
                       LoopNest.level_from_outer_end()) == 3 &&
         "Expected 3 Levels for StencilBlocking\n");
  unsigned InnerLevel = LoopNest.Innermost->getNestingLevel();
  unsigned OuterLevel = LoopNest.Outermost->getNestingLevel();
  unsigned Count = 0;
  for (unsigned Lvl = InnerLevel; Lvl >= OuterLevel; Lvl--, Count++) {
    // Only apply non-zero blocksizes
    if (StencilBlockingFactors[Type][Count]) {
      LoopMap[LoopNest.getLoop(Lvl)] = StencilBlockingFactors[Type][Count];
    }
  }
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

    pull3DStencilSmallStripmineSizes(LoopNest, LoopMap,
                                     StencilProfitability.getStencilType());
    // If we reach here, we want to always stripmine, so we add extra
    // normalization flag, see normalize()
    updateLoopMapByStripmineApplicability(LoopMap, true);

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
    HLLoop *InnermostLoop, HLLoop *OutermostLoop, HIRDDAnalysis &DDA,
    HIRSafeReductionAnalysis &SRA, StringRef Func, LoopMapTy &LoopMap) {

  // Try K&R + fixed stripmine sizes
  // Just use existing logic for now to avoid regression
  KAndRChecker KAndRProfitability(Refs, Func);
  LoopMapTy StripmineSizes;
  adjustBlockSize(LoopNestTC, StripmineSizes);

  StripmineSizeExplorerByDefault StripmineExplorer(StripmineSizes);
  HLLoop *ValidOutermost =
      exploreLoopNest(InnermostLoop, OutermostLoop, KAndRProfitability,
                      StripmineExplorer, DDA, SRA, Func, LoopMap);
  if (ValidOutermost) {
    return ValidOutermost;
  }

  return nullptr;
}

// LoopDepth are at most 2.
// Blocking inner loops doesn't help, because many mem refs are
// already unit-strided.
// Blocking outer loop only doesn't help, because it is
// mere strip-mining.
static bool isTrivialAntiPattern(const MemRefGatherer::VectorTy &Refs,
                                 unsigned InnermostLevel,
                                 unsigned OutermostLevel) {

  if (SkipAntiPatternCheck) {
    return false;
  }

  if (InnermostLevel - OutermostLevel > 1)
    return false;

  int Count = 0;
  for (auto Ref : Refs) {
    unsigned Index = InvalidBlobIndex;
    int64_t Coeff = 0;

    Ref->getDimensionIndex(1)->getIVCoeff(InnermostLevel, &Index, &Coeff);

    if (Index == InvalidBlobIndex && Coeff == 1) {
      Count++;
    }
  }

  return Count / ((float)Refs.size()) >= 0.4;
}

// Find marked loops that we want to perform sinking so blocking analysis
// will be triggered. These marked loops are likely profitable for
// blocking, but were not made perfect in prior passes. After sinking is
// done we return the outermost loop to be checked for blocking.
static const HLLoop *getOuterLoopAfterSpecialSinking(HLLoop *InnermostLp,
                                                     HIRDDAnalysis &DDA) {
  HLLoop *OuterCandidate = nullptr;

  // Check Loop for metadata
  for (HLLoop *Lp = InnermostLp; Lp; Lp = Lp->getParentLoop()) {
    if (Lp->getLoopStringMetadata(EnableSpecialLoopInterchangeMetaName)) {
      Lp->removeLoopMetadata(EnableSpecialLoopInterchangeMetaName);
      OuterCandidate = Lp;
      LLVM_DEBUG(dbgs() << "Found Candidate for Special Sink!\n";);
    }
  }

  if (!OuterCandidate) {
    return nullptr;
  }

  if (HLNodeUtils::isPerfectLoopNest(OuterCandidate)) {
    return OuterCandidate;
  }

  // Do Special-interchange specific sinking:
  if (!HIRTransformUtils::doSpecialSinkForPerfectLoopnest(OuterCandidate,
                                                          InnermostLp, DDA)) {
    LLVM_DEBUG(dbgs() << "Special Sink Failed!\n";);
    return nullptr;
  }

  return OuterCandidate;
}

// Returns the outermost loop where blocking will be applied
// in the range of [outermost, InnermostLoop]
HLLoop *findLoopNestToBlock(HIRFramework &HIRF, StringRef Func,
                            HIRDDAnalysis &DDA, HIRSafeReductionAnalysis &SRA,
                            HLLoop *InnermostLoop, bool Advanced,
                            bool SinkForMultiCopy, LoopMapTy &LoopMap) {

  const HLLoop *HighestAncestor = nullptr;
  // Do sinking for special loops we want to block when running multiple copies.
  // Return ancestor for these marked loopnest.
  if (SinkForMultiCopy) {
    LLVM_DEBUG(dbgs() << "Trying Special Sinking...\n";);
    HighestAncestor = getOuterLoopAfterSpecialSinking(InnermostLoop, DDA);
  }

  if (!HighestAncestor) {
    // Get the highest outermost ancestor of given InnermostLoop
    // that can make a perfect loop nest.
    HighestAncestor =
        HLNodeUtils::getHighestAncestorForPerfectLoopNest(InnermostLoop);
  }

  if (!HighestAncestor) {
    // Blocking is not applied to depth-1 loop nest.
    printDiag(INNERMOST_LOOP_NO_DO_LOOP, Func, InnermostLoop);

    return nullptr;
  }

  assert(HLNodeUtils::isPerfectLoopNest(HighestAncestor) &&
         "Expected Perfect LoopNest!");

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

  llvm::Triple TargetTriple(HIRF.getModule().getTargetTriple());
  bool Is32Bit = (TargetTriple.getArch() == llvm::Triple::x86);
  bool IsLikelySmall = false;
  if (!Refs.empty()) {
    IsLikelySmall = Refs.front()->getDestType()->isIntegerTy() &&
                    Refs.front()->getSrcType()->isIntegerTy();
  }

  LoopNestTCTy LoopNestTC(HighestAncestor, InnermostLoop);
  LoopNestTC.populateLoops();
  populateTCs(LoopNestTC);
  bool AllConstTC = false;
  HLLoop *AdjustedHighestAncestor =
      getHighestAncestorWithTCThreshold(LoopNestTC, AllConstTC);

  if (IsLikelySmall && !AllConstTC) {
    LLVM_DEBUG(dbgs() << "The input's TC is likely to be small\n");
    return nullptr;
  }

  if (isTrivialAntiPattern(Refs, InnermostLoop->getNestingLevel(),
                           AdjustedHighestAncestor->getNestingLevel())) {
    LLVM_DEBUG(dbgs() << "Trivial anti-pattern\n");
    return nullptr;
  }

  // Avoid delinearized results when 32-bit to avoid performance drop.
  // TODO: Extend blocking algorithm to work when num_dims >= loop_depth
  // We want to avoid blocking for integer input, thus enabling
  // delinearization. IsLikelySmall is for avoiding blocking in
  // coremark-pro/core. BitWidth was either 16 or 32.
  // Quite a special casing, but without exact information about
  // tripcount, we need a hack.

  bool DelinearizeRefs =
      !ForceStopDelinearization && (!Is32Bit || IsLikelySmall);

  RefAnalysisResult RefKind = analyzeRefs(Refs, InnermostLoop, DelinearizeRefs);

  // If any lval Ref is a non-linear, give up here.
  if (RefKind == RefAnalysisResult::NON_LINEAR) {
    printDiag(NON_LINEAR_REFS, Func);
    return nullptr;
  }

  // Default to use K&R + fixed stripmine sizes
  // All linear refs and LoopTC above minimum threshold for blocked loops.
  HLLoop *ValidOutermost = tryKAndRWithFixedStripmineSizes(
      Refs, LoopNestTC, InnermostLoop, AdjustedHighestAncestor, DDA, SRA, Func,
      LoopMap);

  if (ValidOutermost) {
    printDiag(SUCCESS_NON_SIV_OR_NON_ADVANCED, Func, AdjustedHighestAncestor,
              "SUCCESS");
    return ValidOutermost;
  } else {
    printDiag(NO_KANDR, Func, AdjustedHighestAncestor);
  }

  // Uniq refs for the purpose of memory footprint analysis.
  // For now, we won't do any memory footprint analysis.
  if (!CommandLineBlockSize &&
      (RefKind == RefAnalysisResult::SIV ||
       RefKind == RefAnalysisResult::NON_LINEAR_READ_ONLY) &&
      (!OldVersion || Advanced)) {

    // Grouping Refs for memory foot print and for stencil
    DDRefGrouping::RefGroupVecTy<RegDDRef *> Groups;
    DDRefIndexGrouping(Groups, Refs);

    // Try stencil pattern + fixed stripmine sizes.
    StencilChecker StencilProfitability(Groups, InnermostLoop, Func);
    if (!StencilProfitability.isProfitable(DDA)) {
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
  }

  printDiag(NO_PROFITABLE_BLOCKING_FOUND, Func, InnermostLoop,
            "Summary: No blocking from this innermost Loop");

  return nullptr;
}

// This is where pragma conflicts are resolved & legality checks are performed.
// Pragmas conflicts are prioritized in the order they appear in the program
// All LoopToPragma entries are assumed to have positive relative offsets.
// TODO: output warnings for pragma conflicts
HLLoop *setupPragmaBlocking(HIRDDAnalysis &DDA, HIRSafeReductionAnalysis &SRA,
                            HLLoop *InnermostLoop, HLLoop *OutermostPragmaLoop,
                            StringRef Func, LoopPragmaMapTy &LoopToPragma,
                            LoopMapTy &LoopMap) {

  bool IsNearPerfect = false;
  if (!OutermostPragmaLoop->isInnermost() &&
      !HLNodeUtils::isPerfectLoopNest(OutermostPragmaLoop, nullptr, false,
                                      &IsNearPerfect)) {
    LLVM_DEBUG(
        dbgs()
        << "Ignoring block_loop directive because loop is not perfect\n");
    return nullptr;
  }

  if (IsNearPerfect) {
    LLVM_DEBUG(
        dbgs()
        << "Ignoring block_loop directive because loop is near perfect\n");
    return nullptr;
  }

  // Setup loopmap here. Need to traverse the loop in order starting with
  // OutermostPragmaLoop
  for (HLLoop *Lp = OutermostPragmaLoop; Lp;
       Lp = dyn_cast<HLLoop>(Lp->getFirstChild())) {
    auto Query = LoopToPragma.find(Lp);
    if (Query == LoopToPragma.end()) {
      continue;
    }

    for (auto &LevelFactorPair : Query->second) {
      int PragmaLevel = LevelFactorPair.first;
      RegDDRef *Factor = LevelFactorPair.second;

      int64_t IntBlockSize;
      // A blocksize of 0 is invalid, while -1 means we use the default
      // blocksize.
      // TODO : Handle variable blocksizes
      if (!Factor->isIntConstant(&IntBlockSize) || IntBlockSize == 0) {
        LLVM_DEBUG(dbgs() << "Ignoring block_loop directive due to invalid "
                             "blocking factor "
                          << IntBlockSize << "\n");
        continue;
      }

      if (IntBlockSize == -1) {
        IntBlockSize = DefaultBlockSize;
      }

      // Get the loop that the pragma level is referring to
      const HLLoop *TargetLp = Lp;
      for (int i = 1; i < PragmaLevel; i++) {
        TargetLp = cast_or_null<HLLoop>(TargetLp->getFirstChild());
        if (!TargetLp) {
          LLVM_DEBUG(dbgs() << "Ignoring block_loop directive due to invalid "
                               "level\n");
          break;
        }
      }

      // Bailout due to invalid Pragma TargetLoop
      if (!TargetLp) {
        break;
      }

      if (LoopMap.find(TargetLp) == LoopMap.end()) {
        LoopMap[TargetLp] = (unsigned)IntBlockSize;
      } else {
        LLVM_DEBUG(dbgs() << "Ignoring block_loop directive for size "
                          << IntBlockSize << " due to conflict @ level : "
                          << TargetLp->getNestingLevel() << "\n");
      }
    }
  }

  // Check stripmine legality
  if (!LoopMap.empty()) {
    for (auto It = LoopMap.begin(); It != LoopMap.end();) {
      if (!It->first->canStripmine(It->second)) {
        LLVM_DEBUG(dbgs() << "Ignoring block_loop directive for size "
                          << It->second << " due to illegal blocksize\n");
        It = LoopMap.erase(It);
      } else {
        It++;
      }
    }
  }

  if (LoopMap.empty()) {
    printDiag(NO_STRIPMINE_APPL, Func);
    return nullptr;
  }

  // Bailout if blocking would result in more than max loop nest level
  if ((LoopMap.size() + InnermostLoop->getNestingLevel()) > MaxLoopNestLevel) {
    LLVM_DEBUG(dbgs() << "No Blocking! MaxLevels exceeded!\n");
    return nullptr;
  }

// From pragma specification:The loop-carried dependence is ignored
// during the processing of block_loop pragmas.
#if 0
  if (!isLegalToInterchange(LoopMap, OutermostPragmaLoop, InnermostLoop, DDA,
                            SRA, false)) {
    printDiag(NO_INTERCHANGE, Func);
    return nullptr;
  }
#endif

  OptReportBuilder &ORBuilder =
      InnermostLoop->getHLNodeUtils().getHIRFramework().getORBuilder();

  // Blocking using Pragma directives
  ORBuilder(*OutermostPragmaLoop).addRemark(OptReportVerbosity::Low, 25565u);

  LLVM_DEBUG(dbgs() << "Final LoopMap: \n"; for (auto &P
                                                 : LoopMap) {
    dbgs() << "LoopLevel: " << P.first->getNestingLevel() << "\n";
    dbgs() << "BlockFactor: " << P.second << "\n";
  });
  return OutermostPragmaLoop;
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

    HLLoop *LpDest =
        CurLoopNests[getIndexForLoopVectors(DestLevel, OutermostLevel)];
    unsigned OrigLevel = LpWithDef->getNestingLevel();
    // For pragma, by-strip loop does not always go to outermost
    assert(DestLevel <= OrigLevel);

    const HLInst *FirstInst = cast<HLInst>(FirstChild);
    assert(isa<SelectInst>(FirstInst->getLLVMInstruction()));

    HLNodeUtils::moveAsFirstChild(LpDest, FirstChild);

    unsigned DefMinLevel = DestLevel;
    unsigned UseMinLevel = FindUseMinLevel(OrigLevel);
    const RegDDRef *MinDef = FirstInst->getLvalDDRef();

    unsigned MinIndex = MinDef->getSelfBlobIndex();

    // Update Def@level of Loop UB w.r.t min blob and lb blob if we added inst
    // during normalization. The new UB needs to contain the correct level
    // based on both blobs, otherwise the CE will be inconsistent. Example:
    //   DO i2 = 0, 3, 1   <DO_LOOP>
    //   %min = (-14 * i2 + 49 <= 13) ? -14 * i2 + 49 : 13;
    //
    //     %lb = 14 * i2;
    //   DO i3 = 0, 14 * i2 + %min + -1 * %lb, 1   <DO_LOOP>  <MAX_TC_EST = 14>

    RegDDRef *UB =
        CurLoopNests[getIndexForLoopVectors(UseMinLevel, OutermostLevel)]
            ->getUpperDDRef();

    unsigned UBDefLevel = DefMinLevel;

    for (const auto &BRef : make_range(UB->blob_begin(), UB->blob_end())) {
      if (BRef->getBlobIndex() == MinIndex) {
        BRef->setDefinedAtLevel(DefMinLevel);
      }
      if (BRef->getDefinedAtLevel() > UBDefLevel) {
        UBDefLevel = BRef->getDefinedAtLevel();
      }
    }
    UB->getSingleCanonExpr()->setDefinedAtLevel(UBDefLevel);

    // Update Live-in temp info.
    // From LpDest's child-loop to LpWithDef's child-loop,
    // "min" def is a live-in
    unsigned MinSB = MinDef->getSymbase();
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

struct HIRLoopBlocking {

  HIRFramework &HIRF;
  HIRDDAnalysis &HDDA;
  HIRSafeReductionAnalysis &SRA;
  HIRLoopStatistics &HLS;
  TargetTransformInfo &TTI;

  StringRef FuncName;
  LoopPragmaMapTy LoopToPragma;
  bool HasPragma;
  bool SinkForMultiCopy;

  HIRLoopBlocking(HIRFramework &HIRF, HIRDDAnalysis &HDDA,
                  HIRSafeReductionAnalysis &SRA, HIRLoopStatistics &HLS,
                  TargetTransformInfo &TTI, bool SinkForMultiCopy)
      : HIRF(HIRF), HDDA(HDDA), SRA(SRA), HLS(HLS), TTI(TTI),
        SinkForMultiCopy(SinkForMultiCopy && !DisableSinkForMultiCopy) {}

  bool run(bool Pragma);

  void doTransformation(HLLoop *InnermostLoop, HLLoop *OutermostLoop,
                        LoopMapTy &LoopToBS);
};

// Do stripmine & interchange
void HIRLoopBlocking::doTransformation(HLLoop *InnermostLoop,
                                       HLLoop *OutermostLoop,
                                       LoopMapTy &LoopToBS) {

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  if (PrintBlockedLoops) {
    dbgs() << "== Before blocking in " << FuncName << " == \n";
    OutermostLoop->dump();
  }
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

  InnermostLoop->setIsUndoSinkingCandidate(false);

  // Remove preheader/postexit before creating outer by-strip loops
  OutermostLoop->extractPreheaderAndPostexit();

  // Stripmine
  HLLoop *NewOutermostLoop =
      stripmineSelectedLoops(InnermostLoop, OutermostLoop, LoopToBS);

  // Populate Permutation
  int TotalDepth = InnermostLoop->getNestingLevel() -
                   NewOutermostLoop->getNestingLevel() + 1;
  SmallVector<const HLLoop *, MaxLoopNestLevel> LoopPermutation(TotalDepth,
                                                                nullptr);

  if (HasPragma) {
    assert(!LoopToPragma.empty() && "Expect Pragma info to be present!\n");
    populatePragmaPermutation(NewOutermostLoop, InnermostLoop, LoopToBS,
                              LoopToPragma, LoopPermutation);
  } else {
    populatePermutation(NewOutermostLoop, InnermostLoop, LoopToBS,
                        LoopPermutation);
  }

  // Interchange
  HIRTransformUtils::permuteLoopNests(NewOutermostLoop, LoopPermutation,
                                      InnermostLoop->getNestingLevel());

  SmallVector<HLLoop *, MaxLoopNestLevel> CurLoopNests;
  ForEach<HLLoop>::visit(NewOutermostLoop, [&CurLoopNests](HLLoop *Lp) {
    CurLoopNests.push_back(Lp);
  });

  // Add OptReport after permutation to get the correct information.
  OptReportBuilder &ORBuilder =
      CurLoopNests.front()->getHLNodeUtils().getHIRFramework().getORBuilder();
  for (auto Lp : CurLoopNests) {
    const HLLoop *OrigLoop = getLoopForReferingInfoBeforePermutation(
        Lp, LoopPermutation, CurLoopNests.front()->getNestingLevel());
    if (isBlockedLoop(OrigLoop, LoopToBS)) {
      // blocked by %d
      ORBuilder(*Lp).addRemark(OptReportVerbosity::Low, 25566u,
                               LoopToBS[OrigLoop]);
    }
  }

  // Hoist min var's definitions to Destination Levels
  LLVM_DEBUG(NewOutermostLoop->dump(1));
  hoistMinDefs(LoopToBS, LoopPermutation, CurLoopNests);
  LLVM_DEBUG(dbgs() << "after hoist\n");
  LLVM_DEBUG(NewOutermostLoop->dump());

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  if (PrintBlockedLoops) {
    dbgs() << "== After blocking in " << FuncName << " == \n";
    NewOutermostLoop->dump();
  }
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

  // Mark as blocked and Invalidate
  InnermostLoop->setIsBlocked(true);
  NewOutermostLoop->getParentRegion()->setGenCode();
  HIRInvalidationUtils::invalidateLoopNestBody(NewOutermostLoop);
  HIRInvalidationUtils::invalidateParentLoopBodyOrRegion(NewOutermostLoop);
}

bool HIRLoopBlocking::run(bool ForPragma) {

  if (!TTI.isLibIRCAllowed()) {
    return false;
  }

  // Collect innermost loops first
  // Many of collections of data can be applied because we are
  // working on perfect or near-perfect loop
  // TODO: replace with loop-only iterator
  //       Keeping all innermost loops in all regions is not ideal.
  SmallVector<HLLoop *, 32> InnermostLoops;
  (HIRF.getHLNodeUtils()).gatherInnermostLoops(InnermostLoops);
  bool Advanced = TTI.isAdvancedOptEnabled(
      TargetTransformInfo::AdvancedOptLevel::AO_TargetHasIntelAVX2);

  LLVM_DEBUG(dbgs() << "Running Loop Blocking:\nMultiple Copies = "
                    << SinkForMultiCopy << "\nAdvanced = " << Advanced
                    << "\n";);

  bool Changed = false;
  for (auto *InnermostLoop : InnermostLoops) {

    if (InnermostLoop->isBlocked()) {
      LLVM_DEBUG(dbgs() << "Loop Already Blocked: \n");
      continue;
    }

    if (HLS.getTotalLoopStatistics(InnermostLoop)
            .hasCallsWithUnsafeSideEffects())
      continue;

    FuncName = HIRF.getFunction().getName();

    // Analysis phase of Blocking pass determines the OutermostLoop to block
    // and LoopMap which contains the blocksizes per loop which is passed to
    // the transformation step
    LoopMapTy LoopMap;
    HLLoop *OutermostLoop = nullptr;

    // Pragma constructs
    HasPragma = hasLoopBlockingPragma(InnermostLoop);
    LoopToPragma.clear();

    if (!ForPragma && !HasPragma) {
      OutermostLoop =
          findLoopNestToBlock(HIRF, FuncName, HDDA, SRA, InnermostLoop,
                              Advanced, SinkForMultiCopy, LoopMap);
    } else if (ForPragma && HasPragma) {
      HLLoop *OutermostPragmaLoop =
          getLoopBlockingPragma(InnermostLoop, LoopToPragma);
      if (!OutermostPragmaLoop) {
        continue;
      }
      OutermostLoop =
          setupPragmaBlocking(HDDA, SRA, InnermostLoop, OutermostPragmaLoop,
                              FuncName, LoopToPragma, LoopMap);
      assert(!LoopToPragma.empty() && "Expect Pragma info to be present!\n");
    } else {
      continue;
    }

    if (!OutermostLoop)
      continue;

    LLVM_DEBUG(dbgs() << "Loop to Block: \n");
    LLVM_DEBUG(OutermostLoop->dump());

    doTransformation(InnermostLoop, OutermostLoop, LoopMap);

    Changed = true;
  }
  return Changed;
}

class HIRLoopBlockingLegacyPass : public HIRTransformPass {
  bool SinkForMultiCopy;

public:
  static char ID;

  HIRLoopBlockingLegacyPass(bool SinkForMultiCopy = true)
      : HIRTransformPass(ID), SinkForMultiCopy(SinkForMultiCopy) {
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

class HIRPragmaLoopBlockingLegacyPass : public HIRTransformPass {
public:
  static char ID;

  HIRPragmaLoopBlockingLegacyPass() : HIRTransformPass(ID) {
    initializeHIRPragmaLoopBlockingLegacyPassPass(
        *PassRegistry::getPassRegistry());
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

FunctionPass *llvm::createHIRLoopBlockingPass(bool SinkForMultiCopy) {
  return new HIRLoopBlockingLegacyPass(SinkForMultiCopy);
}

char HIRPragmaLoopBlockingLegacyPass::ID = 0;
INITIALIZE_PASS_BEGIN(HIRPragmaLoopBlockingLegacyPass, OPT_SWITCH_PRAGMA,
                      OPT_DESC_PRAGMA, false, false)
INITIALIZE_PASS_DEPENDENCY(HIRFrameworkWrapperPass)
INITIALIZE_PASS_DEPENDENCY(HIRDDAnalysisWrapperPass)
INITIALIZE_PASS_DEPENDENCY(HIRSafeReductionAnalysisWrapperPass)
INITIALIZE_PASS_DEPENDENCY(HIRLoopStatisticsWrapperPass)
INITIALIZE_PASS_DEPENDENCY(TargetTransformInfoWrapperPass)
INITIALIZE_PASS_END(HIRPragmaLoopBlockingLegacyPass, OPT_SWITCH_PRAGMA,
                    OPT_DESC_PRAGMA, false, false)

FunctionPass *llvm::createHIRPragmaLoopBlockingPass() {
  return new HIRPragmaLoopBlockingLegacyPass();
}

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
  return HIRLoopBlocking(HIRF, DDA, SRA, HLS, TTI, SinkForMultiCopy).run(false);
}

PreservedAnalyses HIRLoopBlockingPass::runImpl(
    llvm::Function &F, llvm::FunctionAnalysisManager &AM, HIRFramework &HIRF) {
  if (DisablePass) {
    return PreservedAnalyses::all();
  }

  LLVM_DEBUG(dbgs() << OPT_DESC " for Function : " << F.getName() << "\n");

  HIRLoopBlocking(HIRF, AM.getResult<HIRDDAnalysisPass>(F),
                  AM.getResult<HIRSafeReductionAnalysisPass>(F),
                  AM.getResult<HIRLoopStatisticsAnalysis>(F),
                  AM.getResult<TargetIRAnalysis>(F), SinkForMultiCopy)
      .run(false);

  return PreservedAnalyses::all();
}

bool HIRPragmaLoopBlockingLegacyPass::runOnFunction(Function &F) {
  if (DisablePragmaPass || skipFunction(F)) {
    return false;
  }

  LLVM_DEBUG(dbgs() << OPT_DESC_PRAGMA " for Function : " << F.getName()
                    << "\n");

  auto &DDA = getAnalysis<HIRDDAnalysisWrapperPass>().getDDA();
  auto &SRA = getAnalysis<HIRSafeReductionAnalysisWrapperPass>().getHSR();
  auto &HIRF = getAnalysis<HIRFrameworkWrapperPass>().getHIR();
  auto &HLS = getAnalysis<HIRLoopStatisticsWrapperPass>().getHLS();
  auto &TTI = getAnalysis<TargetTransformInfoWrapperPass>().getTTI(F);
  return HIRLoopBlocking(HIRF, DDA, SRA, HLS, TTI, true).run(true);
}

PreservedAnalyses HIRPragmaLoopBlockingPass::runImpl(
    llvm::Function &F, llvm::FunctionAnalysisManager &AM, HIRFramework &HIRF) {
  if (DisablePass) {
    return PreservedAnalyses::all();
  }

  LLVM_DEBUG(dbgs() << OPT_DESC_PRAGMA " for Function : " << F.getName()
                    << "\n");

  HIRLoopBlocking(HIRF, AM.getResult<HIRDDAnalysisPass>(F),
                  AM.getResult<HIRSafeReductionAnalysisPass>(F),
                  AM.getResult<HIRLoopStatisticsAnalysis>(F),
                  AM.getResult<TargetIRAnalysis>(F), true)
      .run(true);

  return PreservedAnalyses::all();
}
