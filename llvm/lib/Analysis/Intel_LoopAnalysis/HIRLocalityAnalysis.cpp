//===---- HIRLocalityAnalysis.cpp - Computes Locality Analysis ------------===//
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

#include "llvm/Pass.h"

#include "llvm/Analysis/Intel_LoopAnalysis/HIRFramework.h"
#include "llvm/Analysis/Intel_LoopAnalysis/HIRLocalityAnalysis.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Passes.h"

#include "llvm/Transforms/Intel_LoopTransforms/Utils/CanonExprUtils.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/DDRefGatherer.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/DDRefUtils.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/HLNodeUtils.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/HLNodeVisitor.h"

#include "llvm/IR/Type.h"

#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/FormattedStream.h"

using namespace llvm;
using namespace llvm::loopopt;

#define DEBUG_TYPE "hir-locality-analysis"

const unsigned DefaultReuseThreshold = 4;

static cl::opt<bool>
    SortedLocality("hir-sorted-locality", cl::init(false), cl::Hidden,
                   cl::desc("Computes sorted locality (for interchange) for "
                            "all loopnests inside function."));

static cl::opt<bool> TemporalLocality(
    "hir-temporal-locality", cl::init(false), cl::Hidden,
    cl::desc("Computes temporal (invariant + reuse) locality for all loops."));

static cl::opt<unsigned> TemporalReuseThreshold(
    "hir-temporal-reuse-threhsold", cl::init(DefaultReuseThreshold), cl::Hidden,
    cl::desc("Specifies reuse threhsold for temporal reuse."));

// Symbolic constant to denote unknown 'N' trip count.
// TODO: Revisit this for scaling known loops.
const unsigned SymbolicConstTC = 100;

// Cache line size in bytes.
// TODO: Get data from Target Machine.
const unsigned CacheLineSize = 64;

// A small value to differentiate between Read vs Write.
const unsigned WriteWt = 4;

FunctionPass *llvm::createHIRLocalityAnalysisPass() {
  return new HIRLocalityAnalysis();
}

char HIRLocalityAnalysis::ID = 0;
INITIALIZE_PASS_BEGIN(HIRLocalityAnalysis, "hir-locality-analysis",
                      "HIR Locality Analysis", false, true)
INITIALIZE_PASS_DEPENDENCY(HIRFramework)
INITIALIZE_PASS_END(HIRLocalityAnalysis, "hir-locality-analysis",
                    "HIR Locality Analysis", false, true)

void HIRLocalityAnalysis::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.setPreservesAll();
  AU.addRequired<HIRFramework>();
}

// Performs a basic setup without actually running the locality
// analysis.
bool HIRLocalityAnalysis::runOnFunction(Function &F) {
  HIRF = &getAnalysis<HIRFramework>();
  return false;
}

void HIRLocalityAnalysis::print(raw_ostream &OS, const Module *M) const {

  HIRLocalityAnalysis &HLA = *const_cast<HIRLocalityAnalysis *>(this);
  auto &HNU = HIRF->getHLNodeUtils();

  if (SortedLocality) {
    OS << "Locality Information for all loops(sorted order):\n";
    SmallVector<const HLLoop *, 16> OutermostLoops;

    HNU.gatherOutermostLoops(OutermostLoops);

    for (auto Lp : OutermostLoops) {

      if (Lp->isInnermost() ||
          !HLNodeUtils::isPerfectLoopNest(Lp, nullptr, false, false, true,
                                          nullptr)) {
        continue;
      }

      SmallVector<const HLLoop *, MaxLoopNestLevel> SortedLoops;
      HLA.sortedLocalityLoops(Lp, SortedLoops);

      for (auto Lp : SortedLoops) {
        printLocalityInfo(OS, Lp);
      }
    }
  } else if (TemporalLocality) {
    formatted_raw_ostream FOS(OS);

    FOS << "Temporal locality information for all loops:\n";
    SmallVector<const HLLoop *, 16> Loops;

    HNU.gatherAllLoops(Loops);

    for (auto Lp : Loops) {
      unsigned TempInv = HLA.getTemporalInvariantLocality(Lp);
      unsigned TempReuse =
          HLA.getTemporalReuseLocality(Lp, TemporalReuseThreshold);

      assert((HLA.getTemporalLocality(Lp, TemporalReuseThreshold) ==
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

unsigned HIRLocalityAnalysis::getTripCount(const HLLoop *Loop) {
  return TripCountByLevel[Loop->getNestingLevel() - 1];
}

void HIRLocalityAnalysis::updateTotalStrideAndRefs(LocalityInfo &LI,
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

bool HIRLocalityAnalysis::sharesLastCacheLine(uint64_t PrevTotalDist,
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

unsigned HIRLocalityAnalysis::computeExtraCacheLines(
    LocalityInfo &LI, const RefGroupTy &RefGroup, unsigned Level,
    uint64_t NumRefBytesAccessed, unsigned NumCacheLinesPerRef) {
  unsigned ExtraCacheLines = 0;
  uint64_t PrevTotalDist = 0, TotalDist = 0;

  auto PrevRef = RefGroup.front();
  // Number of bytes accessed in last cache line by PrevRef.
  unsigned CacheLineOffset = NumRefBytesAccessed % CacheLineSize;

  // References like A[i] and A[i + 10] may access overlapping cache lines
  // across loop iterations. The overlap depends on the distance. For example-
  // CacheLineSize = 64, TripCnt = 1
  // - If Dist = 40, refs access same cache lines.
  // - If Dist = 80, second ref accesses one extra cache line.
  // And so on...
  for (auto RefIt = RefGroup.begin() + 1, E = RefGroup.end(); RefIt != E;
       ++RefIt) {
    auto CurRef = *RefIt;

    int64_t Dist;
    auto Res = DDRefUtils::getConstByteDistance(CurRef, PrevRef, &Dist);
    assert((Res && (Dist >= 0)) &&
           "Refs do not have constant non-negative distance!");

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
      // We subtract -1 so that an exact distance of cache line size doesn't
      // count as a new cache line access.
      ExtraCacheLines += (CacheLineOffset + Dist - 1) / CacheLineSize;
      CacheLineOffset = (CacheLineOffset + Dist) % CacheLineSize;
    }

    // Update PrevRef.
    PrevRef = CurRef;
  }

  return ExtraCacheLines;
}

void HIRLocalityAnalysis::computeNumNoLocalityCacheLines(
    LocalityInfo &LI, const RefGroupTy &RefGroup, unsigned Level,
    unsigned TripCnt) {
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
    // Dimension size is not available for pointer dimension.
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

      IVCoeff /= CE->getDenominator();

      // Dimension size is not available for the highest dimension so we assume
      // TripCnt number of elements.
      unsigned NumElem = TripCnt;

      if (NonLinearBase || CE->getDefinedAtLevel() >= Level) {
        // Penalize non-linearity by adding more number of elements.
        // TODO: Is there a better way?
        NumElem += TripCnt / 2;
      }

      // Total bytes accessed is determined by multiplying IV coefficient,
      // stride of the dimension and assumed number of elements in the
      // dimension.
      DimSize = IVCoeff * Ref->getDimensionStride(I) * NumElem;
    }

    // (CacheLineSize - 1) is added to take the ceiling.
    NumCacheLines = (DimSize + CacheLineSize - 1) / CacheLineSize;
    break;
  }
  assert(NumCacheLines && "NumCacheLines is zero!");

  // This is just an estimate.
  uint64_t NumRefBytesAccessed = (NumCacheLines * CacheLineSize);

  updateTotalStrideAndRefs(LI, RefGroup, NumRefBytesAccessed / TripCnt);

  unsigned ExtraCacheLines = computeExtraCacheLines(
      LI, RefGroup, Level, NumRefBytesAccessed, NumCacheLines);

  LI.NumSpatialCacheLines += NumCacheLines + ExtraCacheLines;
}

void HIRLocalityAnalysis::computeNumTempInvCacheLines(
    LocalityInfo &LI, const RefGroupTy &RefGroup, unsigned Level) {

  auto Ref = RefGroup.front();
  auto RefSize =
      Ref->getCanonExprUtils().getTypeSizeInBits(Ref->getDestType()) / 8;

  updateTotalStrideAndRefs(LI, RefGroup, 0);

  // (CacheLineSize - 1) is added to take the ceiling.
  auto NumCacheLines = (RefSize + CacheLineSize - 1) / CacheLineSize;

  unsigned ExtraCacheLines =
      computeExtraCacheLines(LI, RefGroup, Level, RefSize, NumCacheLines);

  LI.NumTempInvCacheLines += NumCacheLines + ExtraCacheLines;
}

void HIRLocalityAnalysis::computeNumSpatialCacheLines(
    LocalityInfo &LI, const RefGroupTy &RefGroup, unsigned Level,
    unsigned TripCnt, uint64_t Stride) {
  auto NumRefBytesAccessed = (Stride * TripCnt);

  updateTotalStrideAndRefs(LI, RefGroup, Stride);

  // (CacheLineSize - 1) is added to take the ceiling.
  uint64_t NumCacheLines =
      (NumRefBytesAccessed + CacheLineSize - 1) / CacheLineSize;

  unsigned ExtraCacheLines = computeExtraCacheLines(
      LI, RefGroup, Level, NumRefBytesAccessed, NumCacheLines);

  LI.NumSpatialCacheLines += NumCacheLines + ExtraCacheLines;
}

void HIRLocalityAnalysis::computeNumCacheLines(const HLLoop *Loop,
                                               const RefGroupVecTy &RefGroups) {
  unsigned Level = Loop->getNestingLevel();
  unsigned TripCnt = getTripCount(Loop);

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

void HIRLocalityAnalysis::printLocalityInfo(raw_ostream &OS,
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

void HIRLocalityAnalysis::initTripCountByLevel(
    const SmallVectorImpl<const HLLoop *> &Loops) {

  for (auto Loop : Loops) {
    uint64_t TripCnt = 0;
    bool ConstTripLoop = Loop->isConstTripLoop(&TripCnt);

    // Use max trip count estimate if available.
    if (!ConstTripLoop && (TripCnt = Loop->getMaxTripCountEstimate())) {
      ConstTripLoop = true;
    }

    TripCountByLevel[Loop->getNestingLevel() - 1] =
        (ConstTripLoop && (TripCnt < SymbolicConstTC)) ? TripCnt
                                                       : SymbolicConstTC;
  }
}

bool HIRLocalityAnalysis::isSpatialMatch(const RegDDRef *Ref1,
                                         const RegDDRef *Ref2) {
  return DDRefUtils::getConstByteDistance(Ref1, Ref2, nullptr);
}

bool HIRLocalityAnalysis::isTemporalMatch(const RegDDRef *Ref1,
                                          const RegDDRef *Ref2, unsigned Level,
                                          uint64_t MaxDiff) {
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
void HIRLocalityAnalysis::computeLoopNestLocality(
    const HLLoop *Loop, const SmallVectorImpl<const HLLoop *> &LoopVec) {

  RefGroupVecTy RefGroups;
  // Get the Symbase to Memory References.
  LocalityRefGatherer::VectorTy MemRefVec;
  LocalityRefGatherer::gatherRange(Loop->child_begin(), Loop->child_end(),
                                   MemRefVec);

  // Debugging
  DEBUG(LocalityRefGatherer::dump(MemRefVec));

  // Sort the Memory References.
  LocalityRefGatherer::sortAndUnique(MemRefVec, true);

  DEBUG(dbgs() << " After sorting and removing dups\n");
  DEBUG(LocalityRefGatherer::dump(MemRefVec));
  DEBUG(dbgs() << " End\n");

  initTripCountByLevel(LoopVec);

  DDRefGrouping::groupVec(
      RefGroups, MemRefVec,
      std::bind(isSpatialMatch, std::placeholders::_1, std::placeholders::_2));

  DEBUG(DDRefGrouping::dump(RefGroups));

  for (auto CurLoop : LoopVec) {
    computeNumCacheLines(CurLoop, RefGroups);
    DEBUG(printLocalityInfo(dbgs(), CurLoop));
  }
}

void HIRLocalityAnalysis::sortedLocalityLoops(
    const HLLoop *OutermostLoop,
    SmallVector<const HLLoop *, MaxLoopNestLevel> &SortedLoops) {
  assert(OutermostLoop && " Loop parameter is null.");
  assert(SortedLoops.empty() && "SortedLoops vector is non-empty.");
  assert(HLNodeUtils::isPerfectLoopNest(OutermostLoop, nullptr, false, false,
                                        true, nullptr) &&
         "Near perfect loopnest expected!");

  // Clear locality by level.
  for (auto &Loc : LocalityByLevel) {
    Loc.clear();
  }

  HIRF->getHLNodeUtils().gatherAllLoops(OutermostLoop, SortedLoops);
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

unsigned
HIRLocalityAnalysis::getTemporalInvariantLocalityImpl(const HLLoop *Lp,
                                                      bool CheckPresence) {
  assert(Lp && " Loop parameter is null!");

  unsigned Level = Lp->getNestingLevel();

  LocalityRefGatherer::MapTy MemRefMap;

  LocalityRefGatherer::gatherRange(Lp->child_begin(), Lp->child_end(),
                                   MemRefMap);
  LocalityRefGatherer::sortAndUnique(MemRefMap, true);

  unsigned NumInv = 0;

  for (auto &Refs : MemRefMap) {
    for (auto Ref : Refs.second) {
      if (Ref->isStructurallyInvariantAtLevel(Level)) {
        if (CheckPresence) {
          return 1;
        }
        ++NumInv;
      }
    }
  }

  return NumInv;
}

unsigned HIRLocalityAnalysis::getTemporalLocalityImpl(const HLLoop *Lp,
                                                      unsigned ReuseThreshold,
                                                      bool CheckPresence,
                                                      bool ReuseOnly) {
  assert(Lp && " Loop parameter is null!");

  unsigned Level = Lp->getNestingLevel();

  RefGroupVecTy RefGroups;
  LocalityRefGatherer::MapTy MemRefMap;

  LocalityRefGatherer::gatherRange(Lp->child_begin(), Lp->child_end(),
                                   MemRefMap);
  LocalityRefGatherer::sortAndUnique(MemRefMap, true);

  DDRefGrouping::groupMap(RefGroups, MemRefMap,
                          std::bind(isTemporalMatch, std::placeholders::_1,
                                    std::placeholders::_2, Level,
                                    CheckPresence ? ReuseThreshold : ~0U));

  unsigned NumTemporal = 0;

  for (auto &RefVec : RefGroups) {
    auto PrevRef = RefVec.front();

    bool IsInv = !ReuseOnly && PrevRef->isStructurallyInvariantAtLevel(Level);
    auto Size = RefVec.size();

    if (CheckPresence && (IsInv || (Size > 1))) {
      return 1;
    }

    if (IsInv) {
      assert((Size == 1) && "Invariant group should only contain one ref!");
      ++NumTemporal;
      continue;
    }

    for (auto RefIt = RefVec.begin() + 1, E = RefVec.end(); RefIt != E;
         ++RefIt) {
      auto CurRef = *RefIt;

      if (isTemporalMatch(PrevRef, CurRef, Level, ReuseThreshold)) {
        ++NumTemporal;
      }

      PrevRef = CurRef;
    }
  }

  return NumTemporal;
}

void HIRLocalityAnalysis::populateTemporalLocalityGroups(
    const HLLoop *Lp, unsigned ReuseThreshold, RefGroupVecTy &TemporalGroups) {
  assert(Lp && " Loop parameter is null!");

  LocalityRefGatherer::MapTy MemRefMap;

  LocalityRefGatherer::gatherRange(Lp->child_begin(), Lp->child_end(),
                                   MemRefMap);

  LocalityRefGatherer::sort(MemRefMap);

  DDRefGrouping::groupMap(TemporalGroups, MemRefMap,
                          std::bind(isTemporalMatch, std::placeholders::_1,
                                    std::placeholders::_2,
                                    Lp->getNestingLevel(), ReuseThreshold));
}
