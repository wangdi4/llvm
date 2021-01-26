//===---- HIRLocalityAnalysis.cpp - Computes Locality Analysis ------------===//
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
// This file implements the Locality Analysis pass.
// We classify locality into three categories: spatial, temporal invariant and
// temporal reuse. Loop locality is determined by analyzing the number of cache
// lines accessed by (spatial and temporal invariant) refs in the loop. Each
// unique cache line access inidicates a cache miss. This is used to order loops
// in sorted order for interchange.
//
// Most of this work is based on “Optimizing for Parallelism and Data
// Locality”, Kennedy, Ken and McKinley, Kathryn S., ICS '92.
//
// Note, loop locality value is calculated lazily/on-demand/whenever needed. We
// just store information if loop was modified or not. Thus, whenever
// transformations need this information, it calls the analysis routine. This
// helps in saving a lot of unnecessary computation.
//
//===----------------------------------------------------------------------===//

// TODO:
// 1. Check for corner conditions when locality overflows (uint64_t temp,
//    spatial) due to a lot of references.
// 2. Add support for HLIf's, switches and goto's.
// 3. Handle more complicated cases such as A[(i+3)/2].
// 4. Recognize and reduce double-counting of cache lines due to overlap between
// different ref categories. For exmaple, invariant ref A[0] overlaps with
// spatial refs A[i].
// 5. More accurately count cache lines accessed by refs with invariant lower
// dimensional subscripts like A[i][5] and A[i][t]. The current analysis assumes
// every element of the array is being accessed.

#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"

#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRLocalityAnalysis.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Framework/HIRFramework.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Passes.h"

#include "llvm/Analysis/Intel_LoopAnalysis/Utils/CanonExprUtils.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/DDRefGatherer.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/DDRefUtils.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HLNodeUtils.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HLNodeVisitor.h"

#include "llvm/IR/Type.h"

#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/FormattedStream.h"

using namespace llvm;
using namespace llvm::loopopt;

#define DEBUG_TYPE "hir-locality-analysis"

const unsigned DefaultReuseThreshold = 4;

static cl::opt<bool> SpatialLocality(
    "hir-spatial-locality", cl::init(false), cl::Hidden,
    cl::desc("Computes spatial locality for all innermost loops or perfect "
             "loopnests (in sorted order) inside function."));

static cl::opt<bool> TemporalLocality(
    "hir-temporal-locality", cl::init(false), cl::Hidden,
    cl::desc("Computes temporal (invariant + reuse) locality for all loops."));

static cl::opt<unsigned> TemporalReuseThreshold(
    "hir-temporal-reuse-threhsold", cl::init(DefaultReuseThreshold), cl::Hidden,
    cl::desc("Specifies reuse threhsold for temporal reuse."));

// Symbolic constant to denote unknown 'N' trip count.
// TODO: Revisit this for scaling known loops.
const uint64_t SymbolicConstTC = 100;

// Cache line size in bytes.
// TODO: Get data from Target Machine.
const unsigned CacheLineSize = 64;

FunctionPass *llvm::createHIRLocalityAnalysisPass() {
  return new HIRLoopLocalityWrapperPass();
}

AnalysisKey HIRLoopLocalityAnalysis::Key;
HIRLoopLocality HIRLoopLocalityAnalysis::run(Function &F,
                                             FunctionAnalysisManager &AM) {
  return HIRLoopLocality(AM.getResult<HIRFrameworkAnalysis>(F));
}

char HIRLoopLocalityWrapperPass::ID = 0;
INITIALIZE_PASS_BEGIN(HIRLoopLocalityWrapperPass, "hir-locality-analysis",
                      "HIR Locality Analysis", false, true)
INITIALIZE_PASS_DEPENDENCY(HIRFrameworkWrapperPass)
INITIALIZE_PASS_END(HIRLoopLocalityWrapperPass, "hir-locality-analysis",
                    "HIR Locality Analysis", false, true)

void HIRLoopLocalityWrapperPass::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.setPreservesAll();
  AU.addRequired<HIRFrameworkWrapperPass>();
}

// Performs a basic setup without actually running the locality
// analysis.
bool HIRLoopLocalityWrapperPass::runOnFunction(Function &F) {
  HLL.reset(
      new HIRLoopLocality(getAnalysis<HIRFrameworkWrapperPass>().getHIR()));
  return false;
}

void HIRLoopLocalityWrapperPass::releaseMemory() { HLL.reset(); }

// Collects (near)perfect loopnests and non-perfect innermost loops.
class PerfectLoopnestCollector final : public HLNodeVisitorBase {
  const HLNode *SkipNode;
  SmallVectorImpl<const HLLoop *> &Loops;

public:
  PerfectLoopnestCollector(SmallVectorImpl<const HLLoop *> &Loops)
      : SkipNode(nullptr), Loops(Loops) {}

  void visit(const HLNode *Node) {}
  void postVisit(const HLNode *Node) {}

  void visit(const HLLoop *Lp);

  bool skipRecursion(const HLNode *Node) const { return Node == SkipNode; }
};

void PerfectLoopnestCollector::visit(const HLLoop *Lp) {
  bool IsNearPerfect = false;

  // Skip recursing into innermost loops or perfect loopnests.
  if (Lp->isInnermost() ||
      HLNodeUtils::isPerfectLoopNest(Lp, nullptr, false, &IsNearPerfect) ||
      IsNearPerfect) {
    SkipNode = Lp;
  }

  Loops.push_back(Lp);
}

void HIRLoopLocality::printAnalysis(raw_ostream &OS) const {

  HIRLoopLocality &HLA = *const_cast<HIRLoopLocality *>(this);
  auto &HNU = HIRF.getHLNodeUtils();

  if (SpatialLocality) {
    OS << "Locality Information for all loops(sorted order):\n";
    SmallVector<const HLLoop *, 16> CandidateLoops;
    PerfectLoopnestCollector PLC(CandidateLoops);

    HNU.visitAll(PLC);

    for (auto Lp : CandidateLoops) {
      bool IsNearPerfect = false;

      if (!Lp->isInnermost() &&
          (HLNodeUtils::isPerfectLoopNest(Lp, nullptr, false, &IsNearPerfect) ||
           IsNearPerfect)) {
        SmallVector<const HLLoop *, MaxLoopNestLevel> SortedLoops;
        HLA.sortedLocalityLoops(Lp, SortedLoops);

        for (auto Lp : SortedLoops) {
          printLocalityInfo(OS, Lp);
        }
      } else {
        HLA.computeLoopLocality(Lp);
        printLocalityInfo(OS, Lp);
      }
    }
  } else if (TemporalLocality) {
    formatted_raw_ostream FOS(OS);

    FOS << "Temporal locality information for all loops:\n";
    SmallVector<const HLLoop *, 16> Loops;

    HNU.gatherAllLoops(Loops);

    for (auto Lp : Loops) {
      unsigned TempInv = HLA.getTemporalInvariantLocality(Lp, false);
      unsigned TempReuse =
          HLA.getTemporalReuseLocality(Lp, TemporalReuseThreshold, false);

      assert((HLA.getTemporalLocality(Lp, TemporalReuseThreshold, false) ==
              TempInv + TempReuse) &&
             "Mismatch between temporal locality implementations!");

      Lp->printHeader(FOS, 0, false);

      Lp->indent(FOS, 0);
      FOS << "TempInv: " << TempInv << "\n";
      Lp->indent(FOS, 0);
      FOS << "TempReuse: " << TempReuse << "\n";

      Lp->printFooter(FOS, 0);
    }
  }
}

uint64_t HIRLoopLocality::getTripCount(const HLLoop *Loop) {
  auto TripCnt = TripCountByLevel[Loop->getNestingLevel() - 1];
  assert(TripCnt && "Loop trip count is zero!");
  return TripCnt;
}

void HIRLoopLocality::updateTotalStrideAndRefs(LocalityInfo &LI,
                                               const RefGroupTy &RefGroup,
                                               uint64_t Stride) {
  auto Size = RefGroup.size();
  LI.TotalStride += Stride * Size;
  LI.TotalRefs += Size;

  for (auto Ref : RefGroup) {
    if (Ref->isLval()) {
      LI.TotalLvalStride += Stride;
      ++LI.TotalLvalRefs;
    }
  }
}

bool HIRLoopLocality::sharesLastCacheLine(uint64_t PrevTotalDist,
                                          uint64_t CurTotalDist,
                                          uint64_t NumRefBytesAccessed) {
  // This is where the PrevRef ended starting from the first ref of the group.
  auto PrevByteOffset = PrevTotalDist + NumRefBytesAccessed;

  // Bytes in the last cache line accessed by PrevRef.
  auto LastCacheLineOffset = PrevByteOffset % CacheLineSize;

  // PrevRef occupies the entire last cache line so there can be no sharing.
  if (LastCacheLineOffset == 0) {
    return false;
  }

  // Round up byte offset to find the starting of the next cache line.
  auto NextCacheLineByteOffset =
      PrevByteOffset - LastCacheLineOffset + CacheLineSize;

  // Return true/false based on whether the current ref starts from before or
  // after the next cache line.
  return CurTotalDist < NextCacheLineByteOffset;
}

uint64_t HIRLoopLocality::computeExtraCacheLines(LocalityInfo &LI,
                                                 const RefGroupTy &RefGroup,
                                                 unsigned Level,
                                                 uint64_t NumRefBytesAccessed,
                                                 uint64_t NumCacheLinesPerRef) {
  uint64_t ExtraCacheLines = 0;
  uint64_t PrevTotalDist = 0, TotalDist = 0;

  uint64_t CacheLineOffset = NumRefBytesAccessed % CacheLineSize;

  // References like A[i] and A[i + 10] may access overlapping cache lines
  // across loop iterations. The overlap depends on the distance. For example-
  // CacheLineSize = 64, TripCnt = 1
  // - If Dist = 40, refs access same cache lines.
  // - If Dist = 80, second ref accesses one extra cache line.
  // And so on...
  const RegDDRef *PrevRef = RefGroup.front();
  const RegDDRef *CurRef = nullptr;
  // Number of bytes accessed in last cache line by PrevRef.
  for (auto RefIt = RefGroup.begin() + 1, E = RefGroup.end(); RefIt != E;
       ++RefIt, PrevRef = CurRef) {
    CurRef = *RefIt;

    int64_t Dist;
    auto Res = DDRefUtils::getConstByteDistance(CurRef, PrevRef, &Dist);
    (void)Res;
    assert((Res && (Dist >= 0)) &&
           "Refs do not have constant non-negative distance!");

    if (Dist == 0) {
      continue;
    }

    PrevTotalDist = TotalDist;
    TotalDist += Dist;

    if ((Dist >= CacheLineSize) &&
        (static_cast<uint64_t>(Dist) >= NumRefBytesAccessed)) {
      // There is no overlap between PrevRef and CurRef and they start on
      // different cache lines.

      // (CacheLineSize - 1) is added to take the ceiling.
      ExtraCacheLines += ((TotalDist % CacheLineSize) + NumRefBytesAccessed +
                          CacheLineSize - 1) /
                         CacheLineSize;

      // Check whether the last cache line of PrevRef is the same as the first
      // cache line of CurRef. For example-
      // CacheLineSize = 64, TripCnt = 10, Stride = 8.
      // Group consists of A[i] and A[i+10].
      // A[i] accesses 2 cache lines and shares the 2nd cache line with A[i+10].
      if (sharesLastCacheLine(PrevTotalDist, TotalDist, NumRefBytesAccessed)) {
        --ExtraCacheLines;
      }

      // Reset CacheLineOffset based on CurRef.
      CacheLineOffset = (TotalDist + NumRefBytesAccessed) % CacheLineSize;

    } else {
      // PrevRef and CurRef overlap or lie on the same cache line.
      // Extra cache lines accessed is based how many cache lines does the
      // distance between them cover.

      // If offset is zero, we simulate starting a new cache line by setting
      // offset equal to cache line size.
      if (CacheLineOffset == 0) {
        CacheLineOffset = CacheLineSize;
      }

      // We subtract -1 so that an exact distance of cache line size doesn't
      // count as a new cache line access.
      ExtraCacheLines += (CacheLineOffset + Dist - 1) / CacheLineSize;
      CacheLineOffset = (CacheLineOffset + Dist) % CacheLineSize;
    }
  }

  return ExtraCacheLines;
}

void HIRLoopLocality::computeNumNoLocalityCacheLines(LocalityInfo &LI,
                                                     const RefGroupTy &RefGroup,
                                                     unsigned Level,
                                                     uint64_t TripCnt) {
  const RegDDRef *Ref = RefGroup.front();

  auto BaseCE = Ref->getBaseCE();
  auto NumDims = Ref->getNumDimensions();

  uint64_t NumCacheLines = 0;

  bool NonLinearBase = (BaseCE->getDefinedAtLevel() >= Level);

  // Get the highest varying dimension of Ref for the loop level. This will
  // determine the total bytes accessed by the Ref.
  // For example, consider A[0][%t] where %t is non-linear. The maximum number
  // of bytes that can be accessed is restricted by the dimension size so we use
  // this info.
  for (auto I = NumDims; I > 0; --I) {
    auto CE = Ref->getDimensionIndex(I);

    if (!NonLinearBase && CE->isInvariantAtLevel(Level)) {
      continue;
    }

    auto DimSize = Ref->getDimensionSize(I);
    // Dimension size is not available for pointer dimension
    // and variable size arrays
    if (DimSize == 0) {
      // Use info from the CE to construct a more accurate stride.
      int64_t Coeff;
      unsigned BlobIndex;

      CE->getIVCoeff(Level, &BlobIndex, &Coeff);
      auto IVCoeff = static_cast<uint64_t>(std::llabs(Coeff));

      if (!IVCoeff) {
        IVCoeff = 1;
      } else if (BlobIndex != InvalidBlobIndex) {
        // Replace blob by 4. Can we do better?
        IVCoeff *= 4;
      }

      // Dimension size is not available for the highest dimension so we assume
      // TripCnt number of elements.
      uint64_t NumElem = TripCnt;

      if (NonLinearBase || CE->getDefinedAtLevel() >= Level) {
        // Penalize non-linearity by adding more number of elements.
        // TODO: Is there a better way?
        NumElem += TripCnt / 2;
      }

      auto Denom = CE->getDenominator();

      // Calculate the number of actual accesses of Ref based on IV coefficient,
      // denominator and number of elements. The access is assumed to be
      // contiguous for simplicity. For example, for A[i/2] with loop trip count
      // of 5, the number of accesses are 3: A[0], A[1] and A[2].
      auto NumAccesses = ((IVCoeff * (NumElem - 1)) / Denom) + 1;
      auto DimStride = Ref->getDimensionConstStride(I);

      // Workaround for non-constant dimension stride in fortran.
      // TODO: refine later using lower dimension strides, if available.
      if (!DimStride) {
        DimStride = Ref->getDestTypeSizeInBytes();
      }

      DimSize = NumAccesses * DimStride;
    }

    // (CacheLineSize - 1) is added to take the ceiling.
    NumCacheLines = (DimSize + CacheLineSize - 1) / CacheLineSize;
    break;
  }
  assert(NumCacheLines && "NumCacheLines is zero!");

  // This is just an estimate.
  uint64_t NumRefBytesAccessed = (NumCacheLines * CacheLineSize);

  updateTotalStrideAndRefs(LI, RefGroup, NumRefBytesAccessed / TripCnt);

  uint64_t ExtraCacheLines = computeExtraCacheLines(
      LI, RefGroup, Level, NumRefBytesAccessed, NumCacheLines);

  LI.NumSpatialCacheLines += NumCacheLines + ExtraCacheLines;
}

void HIRLoopLocality::computeNumTempInvCacheLines(LocalityInfo &LI,
                                                  const RefGroupTy &RefGroup,
                                                  unsigned Level) {

  auto Ref = RefGroup.front();
  auto RefSize =
      Ref->getCanonExprUtils().getTypeSizeInBytes(Ref->getDestType());

  updateTotalStrideAndRefs(LI, RefGroup, 0);

  // (CacheLineSize - 1) is added to take the ceiling.
  auto NumCacheLines = (RefSize + CacheLineSize - 1) / CacheLineSize;

  uint64_t ExtraCacheLines =
      computeExtraCacheLines(LI, RefGroup, Level, RefSize, NumCacheLines);

  LI.NumTempInvCacheLines += NumCacheLines + ExtraCacheLines;
}

void HIRLoopLocality::computeNumSpatialCacheLines(LocalityInfo &LI,
                                                  const RefGroupTy &RefGroup,
                                                  unsigned Level,
                                                  uint64_t TripCnt,
                                                  uint64_t Stride) {
  auto NumRefBytesAccessed = (Stride * TripCnt);

  updateTotalStrideAndRefs(LI, RefGroup, Stride);

  // (CacheLineSize - 1) is added to take the ceiling.
  uint64_t NumCacheLines =
      (NumRefBytesAccessed + CacheLineSize - 1) / CacheLineSize;

  uint64_t ExtraCacheLines = computeExtraCacheLines(
      LI, RefGroup, Level, NumRefBytesAccessed, NumCacheLines);

  LI.NumSpatialCacheLines += NumCacheLines + ExtraCacheLines;
}

void HIRLoopLocality::computeNumCacheLines(const HLLoop *Loop,
                                           const RefGroupVecTy &RefGroups) {
  unsigned Level = Loop->getNestingLevel();
  uint64_t TripCnt = getTripCount(Loop);

  LocalityInfo &LI = LocalityByLevel[Level - 1];
  assert((LI.getNumCacheLines() == 0) && "Spatial locality already populated!");

  for (auto &RefVec : RefGroups) {
    assert(!RefVec.empty() && " Ref Group is empty.");

    // We need to compute spatial locality for only one from each RefGroup.
    const RegDDRef *Ref = RefVec.front();
    int64_t Stride;

    if (!Ref->getConstStrideAtLevel(Level, &Stride)) {
      computeNumNoLocalityCacheLines(LI, RefVec, Level, TripCnt);
    } else if (Stride == 0) {
      computeNumTempInvCacheLines(LI, RefVec, Level);
    } else {
      computeNumSpatialCacheLines(LI, RefVec, Level, TripCnt,
                                  std::llabs(Stride));
    }
  }
}

void HIRLoopLocality::printLocalityInfo(raw_ostream &OS,
                                        const HLLoop *Lp) const {

  unsigned Level = Lp->getNestingLevel();

  // lit tests are dependent on the printing information.
  const LocalityInfo &LI = LocalityByLevel[Level - 1];

  formatted_raw_ostream FOS(OS);
  unsigned ColNum = 35;

  FOS << "Locality Info for Loop level: " << Level;
  FOS.PadToColumn(ColNum);
  FOS << " NumCacheLines: " << LI.getNumCacheLines();
  ColNum += 25;
  FOS.PadToColumn(ColNum);
  FOS << "SpatialCacheLines: " << LI.NumSpatialCacheLines;
  ColNum += 25;
  FOS.PadToColumn(ColNum);
  FOS << "TempInvCacheLines: " << LI.NumTempInvCacheLines;
  ColNum += 25;
  FOS.PadToColumn(ColNum);
  FOS << "AvgLvalStride: " << LI.getAvgLvalStride();
  ColNum += 25;
  FOS.PadToColumn(ColNum);
  FOS << "AvgStride: " << LI.getAvgStride() << "\n";
}

void HIRLoopLocality::initTripCountByLevel(
    const SmallVectorImpl<const HLLoop *> &Loops) {

  for (auto Loop : Loops) {
    uint64_t TripCnt = 0;
    unsigned PragmaTripCnt;

    if (Loop->isConstTripLoop(&TripCnt)) {
      // In some cases, the loop reports an absurdly high trip count that causes
      // the math here to overflow. Clamp the trip count to a high value--it
      // should satisfy any threshold analysis anyways.
      TripCnt = std::min(TripCnt, (uint64_t)1 << 32);

      TripCountByLevel[Loop->getNestingLevel() - 1] = TripCnt;
    } else if (Loop->getPragmaBasedAverageTripCount(PragmaTripCnt) ||
               Loop->getPragmaBasedMaximumTripCount(PragmaTripCnt)) {
      // Prioritize a pragma-based average or max estimate, in that order. Note
      // that PragmaTripCnt is uint32_t, so we effectively have the same
      // headroom for avoiding overflow.
      TripCountByLevel[Loop->getNestingLevel() - 1] = PragmaTripCnt;
    } else if ((TripCnt = Loop->getMaxTripCountEstimate())) {
      // Clamp max trip count to SymbolicConstTC if based on estimate.
      TripCountByLevel[Loop->getNestingLevel() - 1] =
          std::min(TripCnt, SymbolicConstTC);
    } else {
      TripCountByLevel[Loop->getNestingLevel() - 1] = SymbolicConstTC;
    }
  }
}

/// Returns true if \p Ref1 and \p Ref2 belongs to the same array reference
/// group for the purposes of computing spatial locality for interchange.
///
/// We group together all the refs with constant distance.
///
/// For example, all the following refs will be in the same group:
///  A[i][j][k]
///  A[i][j+1][k]
///  A[i+1][j][k]
///  A[i+1][j][k-1]
///
/// They will be arranged in this order-
/// A[i][j][k], A[i][j+1][k], A[i+1][j][k-1], A[i+1][j][k]
///
/// After this grouping, we will compute the number of cache lines touched by
/// the group (equivalent to cache line misses) based on the stride at a
/// particular loop level.
/// The assmuption is that the first ref in the group lies at the beginning of
/// the cache line.
static bool isSpatialMatch(const RegDDRef *Ref1, const RegDDRef *Ref2) {
  return DDRefUtils::getConstByteDistance(Ref1, Ref2, nullptr);
}

/// Returns true if \p Ref1 and \p Ref2 belong to the same group for the
/// purposes of computing temporal locality.
/// If Ref1 and Ref2 have a constant iteration distance w.r.t IV at \p Level
/// not exceeding \p MaxDiff then they belong to the same group.
///
/// For example-
/// A[2*i] and A[2*i+2] == true
/// A[2*i] and A[2*i+1] == false
/// A[0] and A[0] == true
/// A[0] and A[1] == false
static bool isTemporalMatch(const RegDDRef *Ref1, const RegDDRef *Ref2,
                            unsigned Level, uint64_t MaxDiff) {
  int64_t Diff;

  if (!DDRefUtils::getConstIterationDistance(Ref1, Ref2, Level, &Diff)) {
    return false;
  }

  if (static_cast<uint64_t>(std::llabs(Diff)) > MaxDiff) {
    return false;
  }

  return true;
}

// This is a high level routine to compute different locality.
void HIRLoopLocality::computeLoopNestLocality(
    const HLLoop *Loop, const SmallVectorImpl<const HLLoop *> &LoopVec,
    RefGroupVecTy *SpatialGroups) {

  // Clear locality by level.
  for (auto &Loc : LocalityByLevel) {
    Loc.clear();
  }

  // Get the Symbase to Memory References.
  LocalityRefGatherer::VectorTy MemRefVec;
  LocalityRefGatherer::gatherRange(Loop->child_begin(), Loop->child_end(),
                                   MemRefVec);

  // Debugging
  LLVM_DEBUG(LocalityRefGatherer::dump(MemRefVec));

  // Sort the Memory References.
  LocalityRefGatherer::sortAndUnique(MemRefVec, true);

  LLVM_DEBUG(dbgs() << " After sorting and removing dups\n");
  LLVM_DEBUG(LocalityRefGatherer::dump(MemRefVec));
  LLVM_DEBUG(dbgs() << " End\n");

  initTripCountByLevel(LoopVec);

  // Use passed in ref groups, if provided.
  RefGroupVecTy TmpRefGroups;
  RefGroupVecTy &RefGroups = SpatialGroups ? *SpatialGroups : TmpRefGroups;

  DDRefGrouping::groupVec(
      RefGroups, MemRefVec,
      std::bind(isSpatialMatch, std::placeholders::_1, std::placeholders::_2));

  LLVM_DEBUG(DDRefGrouping::dump(RefGroups));

  for (auto CurLoop : LoopVec) {
    computeNumCacheLines(CurLoop, RefGroups);
    LLVM_DEBUG(printLocalityInfo(dbgs(), CurLoop));
  }
}

void HIRLoopLocality::sortedLocalityLoops(
    const HLLoop *OutermostLoop,
    SmallVector<const HLLoop *, MaxLoopNestLevel> &SortedLoops) {
  assert(OutermostLoop && " Loop parameter is null.");
  assert(SortedLoops.empty() && "SortedLoops vector is non-empty.");
  bool IsNearPerfect = false;
  assert((HLNodeUtils::isPerfectLoopNest(OutermostLoop, nullptr, false,
                                         &IsNearPerfect) ||
          IsNearPerfect) &&
         "Near perfect loopnest expected!");
  (void)IsNearPerfect;

  HIRF.getHLNodeUtils().gatherAllLoops(OutermostLoop, SortedLoops);
  computeLoopNestLocality(OutermostLoop, SortedLoops);

  auto Comp = [this](const HLLoop *Lp1, const HLLoop *Lp2) {
    auto Level1 = Lp1->getNestingLevel();
    auto Level2 = Lp2->getNestingLevel();

    auto &Loc1 = LocalityByLevel[Level1 - 1];
    auto &Loc2 = LocalityByLevel[Level2 - 1];

    // Loop which accesses higher number of cache lines has less locality.
    if (Loc1.getNumCacheLines() != Loc2.getNumCacheLines()) {
      return Loc1.getNumCacheLines() > Loc2.getNumCacheLines();
    }

    // Loop with higher average stride for refs has less locality.
    if (Loc1.getAvgStride() != Loc2.getAvgStride()) {
      return Loc1.getAvgStride() > Loc2.getAvgStride();
    }

    // Loop with higher average stride for lval refs has less locality.
    if (Loc1.getAvgLvalStride() != Loc2.getAvgLvalStride()) {
      return Loc1.getAvgLvalStride() > Loc2.getAvgLvalStride();
    }

    // Everything is equal, prefer to keep original loop order.
    return Level1 < Level2;
  };

  std::sort(SortedLoops.begin(), SortedLoops.end(), Comp);
}

unsigned HIRLoopLocality::getTemporalLocalityImpl(
    const HLLoop *Lp, unsigned ReuseThreshold,
    TemporalLocalityType LocalityType, bool IgnoreConditionalRefs,
    bool CheckPresence) {
  assert(Lp && " Loop parameter is null!");

  unsigned Level = Lp->getNestingLevel();

  RefGroupVecTy RefGroups;
  LocalityRefGatherer::MapTy MemRefMap;

  LocalityRefGatherer::gatherRange(Lp->child_begin(), Lp->child_end(),
                                   MemRefMap);
  LocalityRefGatherer::sort(MemRefMap);

  DDRefGrouping::groupMap(RefGroups, MemRefMap,
                          std::bind(isTemporalMatch, std::placeholders::_1,
                                    std::placeholders::_2, Level,
                                    CheckPresence ? ReuseThreshold : ~0U));

  unsigned NumTemporal = 0;
  bool NeedInvariant = (LocalityType & TemporalLocalityType::Invariant);
  bool NeedReuse = (LocalityType & TemporalLocalityType::Reuse);

  for (auto &RefVec : RefGroups) {
    if (CheckPresence && NumTemporal != 0) {
      // Only need true/false result.
      return 1;
    }

    auto *PrevRef = RefVec.front();

    bool IsInvariant = PrevRef->isStructurallyInvariantAtLevel(Level, true);
    if (IsInvariant && !NeedInvariant) {
      continue;
    }

    // Ideally, we should check that ref post-dominates the first child of
    // parent loop but the current check is cheap and works in most common case
    // (no gotos).
    bool IsPrevConditional =
        IgnoreConditionalRefs && !isa<HLLoop>(PrevRef->getParent());

    if (!IsPrevConditional && IsInvariant) {
      ++NumTemporal;
      continue;
    }

    for (auto RefIt = RefVec.begin() + 1, E = RefVec.end(); RefIt != E;
         ++RefIt) {
      auto *CurRef = *RefIt;

      bool IsConditional = !isa<HLLoop>(CurRef->getParent());

      // Consider reuse when only one of the refs is conditional-
      // DO
      //   = A[i+1]
      //   if ()
      //     A[i]
      // END DO
      //
      // TODO: Consider load/store and top sort number relationship.
      if (IgnoreConditionalRefs && IsPrevConditional && IsConditional) {
        continue;
      }

      if (IsInvariant) {
        ++NumTemporal;
        break;
      }

      // Since we are not uniquing refs, we can encounter multiple identical
      // refs.
      if (NeedReuse && !DDRefUtils::areEqual(PrevRef, CurRef) &&
          isTemporalMatch(PrevRef, CurRef, Level, ReuseThreshold)) {
        ++NumTemporal;
      }

      IsPrevConditional = IsConditional;
      PrevRef = CurRef;
    }
  }

  return NumTemporal;
}

void HIRLoopLocality::populateTemporalLocalityGroups(
    HLContainerTy::const_iterator Begin, HLContainerTy::const_iterator End,
    unsigned Level, unsigned ReuseThreshold, RefGroupVecTy &TemporalGroups,
    SmallSet<unsigned, 8> *UniqueGroupSymbases) {
  assert(((Level == 0 && ReuseThreshold == 0) ||
          CanonExpr::isValidLoopLevel(Level)) &&
         " Invalid combination of loop level and reuse threhold!");

  typedef DDRefGatherer<const RegDDRef, MemRefs | FakeRefs> MemRefGatherer;

  MemRefGatherer::MapTy MemRefMap;

  MemRefGatherer::gatherRange(Begin, End, MemRefMap);

  // No sorting needed for equality groups.
  if (ReuseThreshold != 0) {
    MemRefGatherer::sort(MemRefMap);
  }

  DDRefGrouping::groupMap(TemporalGroups, MemRefMap,
                          std::bind(isTemporalMatch, std::placeholders::_1,
                                    std::placeholders::_2, Level ? Level : 1,
                                    ReuseThreshold));

  if (UniqueGroupSymbases) {
    DenseMap<unsigned, unsigned> SymbaseCount;

    for (auto &RefVec : TemporalGroups) {
      auto Ref = *(RefVec.begin());
      SymbaseCount[Ref->getSymbase()]++;
    }

    for (auto &SymEntry : SymbaseCount) {
      if (SymEntry.second == 1) {
        UniqueGroupSymbases->insert(SymEntry.first);
      }
    }
  }
}

void HIRLoopLocality::populateSpatialLocalityGroups(
    const HLLoop *Lp, RefGroupVecTy &SpatialGroups) {
  assert(Lp && " Loop parameter is null!");
  computeLoopLocality(Lp, &SpatialGroups);
}

uint64_t HIRLoopLocality::getNumCacheLines(const HLLoop *Lp,
                                           RefGroupVecTy *SpatialGroups) {
  assert(Lp && " Loop parameter is null!");

  computeLoopLocality(Lp, SpatialGroups);

  return LocalityByLevel[Lp->getNestingLevel() - 1].getNumCacheLines();
}
