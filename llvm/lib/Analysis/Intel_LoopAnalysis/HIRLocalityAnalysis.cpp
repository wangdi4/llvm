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
// temporal reuse. Loop locality value, which is mainly the comparison between
// two loops is derived from spatial and temporal invariant. Temporal reuse will
// be used in the future for scalar replacement.
//
// Most of this work is based on “Optimizing for Parallelism and Data
// Locality”, Kennedy, Ken and McKinley, Kathryn S., ICS '92. However, there
// modification made such as we count cache hits instead of misses as it
// simplifies the analysis.
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
// 3. Add a testcase where DDRef occurs in UB/LB of Level-1 loop and check
//    results.
// 4. Add platform characteristics such as floats per cache line and also check
//    DDRef type size. Check if vectorization team or some other
//    way to get platform characteristics. Revisit BitWidth interface to compute
//    spatial trip.
// 5. Handle other cases, such as A[(i+3)/2], A[M[i]] and *(ptr+i+j)
// 6. Think about cases where int loads happen on float arrays. The analysis
//    should work fine, but add more test cases.

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
#include "llvm/Support/raw_ostream.h"

using namespace llvm;
using namespace llvm::loopopt;

#define DEBUG_TYPE "hir-locality-analysis"

static cl::opt<bool>
    SortedLocality("hir-sorted-locality", cl::init(false), cl::Hidden,
                   cl::desc("Computes sorted locality (for interchange) for "
                            "all loopnests inside function."));

static cl::opt<bool> TemporalLocality(
    "hir-temporal-locality", cl::init(false), cl::Hidden,
    cl::desc("Computes temporal (invariant + reuse) locality for all loops."));

// Symbolic constant to denote unknown 'N' trip count.
// TODO: Revisit this for scaling known loops.
const unsigned SymbolicConstTC = 20;

// Max distance between refs considered temporal reuse.
const unsigned DefaultReuseThreshold = 4;

// number of cache lines =
// WtFactor*((Total Cache Size)/(CacheLine size * Associativity))
const unsigned NumCacheLines = 16;

// Cache line size in bytes.
// TODO: Get data from Target Machine.
const unsigned CacheLineSize = 64;

// Floats per cache line.
// TODO: Assuming 4bytes of float. Change when sizeinfo is available.
// TODO: Similar for other data types. Revisit when bit width is available
// for the given data.
const unsigned FloatsPerCacheLine = CacheLineSize / 4;
const unsigned IntsPerCacheLine = CacheLineSize / 4;

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
      HLA.getTemporalLocality(Lp);
      const LocalityInfo &LI = LocalityByLevel[Lp->getNestingLevel() - 1];

      Lp->printHeader(FOS, 0, false);
      Lp->indent(FOS, 0);
      FOS << "TempInv: " << LI.TempInv << "\n";
      Lp->indent(FOS, 0);
      FOS << "TempReuse: " << LI.TempReuse << "\n";

      Lp->printFooter(FOS, 0);
    }
  }
}

void HIRLocalityAnalysis::computeTempInvLocality(const HLLoop *Loop,
                                                 RefGroupVecTy &RefGroups) {

  // We need to find invariant DDRefs and compute a temporal locality
  // based on the loop's trip count.
  // For eg. for(i=0; i<N; i++) { A[t] = 0; }. Here Array A is invariant
  // and we add 'N' to temporal locality.

  unsigned Level = Loop->getNestingLevel();

  LocalityInfo &LI = LocalityByLevel[Level - 1];
  assert((LI.TempInv == 0) && "Temporal invariant locality already populated!");

  // Compute Temp. Invariant Locality.
  for (auto &RefVec : RefGroups) {
    assert(!RefVec.empty() && " Ref Group is empty.");

    const RegDDRef *FirstRef = *(RefVec.begin());

    if (FirstRef->isStructurallyInvariantAtLevel(Level)) {
      // Invariant refs are grouped together so we add the locality for the
      // whole group.
      for (auto Ref : RefVec) {
        LI.TempInv += getTripCount(Loop) - 1;

        if (Ref->isLval()) {
          LI.TempInv += WriteWt;
        }
      }
    }
  }
}

bool HIRLocalityAnalysis::isMultipleIV(const RegDDRef *Ref, unsigned Level,
                                       unsigned *IVPos) {

  // TODO: Base IV is not handled.
  assert(!Ref->getBaseCE()->hasIV(Level) && " Base IV not handled.");

  int NumIV = 0;
  for (auto RefIter = Ref->canon_begin(), End = Ref->canon_end();
       RefIter != End; ++RefIter) {
    const CanonExpr *RefCE = *RefIter;
    if (RefCE->hasIV(Level)) {
      NumIV++;
      // Update the position
      if (IVPos) {
        *IVPos = std::distance(Ref->canon_begin(), RefIter) + 1;
      }
      // Multiple IV check.
      if (NumIV > 1)
        return true;
    }
  }

  return false;
}

void HIRLocalityAnalysis::computeTempReuseLocality(const HLLoop *Loop,
                                                   RefGroupVecTy &RefGroups,
                                                   unsigned ReuseThreshold) {

  unsigned Level = Loop->getNestingLevel();

  LocalityInfo &LI = LocalityByLevel[Level - 1];
  assert((LI.TempReuse == 0) && "Temporal reuse locality already populated!");

  for (auto &RefVec : RefGroups) {
    assert(!RefVec.empty() && " Ref Group is empty.");

    // The first reference is used for comparison.
    const RegDDRef *CompareRef = *(RefVec.begin());

    if (CompareRef->isStructurallyInvariantAtLevel(Level)) {
      continue;
    }

    for (auto RefIt = RefVec.begin() + 1, End = RefVec.end(); RefIt != End;
         ++RefIt) {
      int64_t Dist;

      bool Res = DDRefUtils::getConstIterationDistance(CompareRef, *RefIt,
                                                       Level, &Dist);
      (void)Res;
      assert(Res && "Invalid temporal locality group!");

      if (std::llabs(Dist) <= ReuseThreshold) {
        LI.TempReuse += 1;
      }
      // We should update the compare ref unconditionally. Consider this group-
      // A[i], A[i+2], A[i+4]
      // We should compute a reuse of 2 (for {A[i], A[i+2]} and {A[i+2],
      // A[i+4]}) with a threhsold of 2.
      CompareRef = *RefIt;
    }
  }
}

int64_t HIRLocalityAnalysis::getTripCount(const HLLoop *Loop) {
  return TripCountByLevel[Loop->getNestingLevel() - 1];
}

// This method computes the loop nest trip count for spatial locality.
// For example: A[i][j] would be calculated as (M - (M/4)) for j loop.
// for i loop spatial locality would be 0, assuming i & j have 'M' trip count.
// The diff happens in the spatial reuse computation.
uint64_t HIRLocalityAnalysis::computeSpatialTrip(const RegDDRef *Ref,
                                                 const HLLoop *Loop) {

  unsigned LoopLevel = Loop->getNestingLevel();
  uint64_t SpatialTrip = 0;
  // Get the lowest dimension to check level exist.
  // This will implicitly handle cases such as A[i+j].
  const CanonExpr *CE = Ref->getDimensionIndex(1);

  // Basic check since IV should exist.
  assert(CE->hasIV(LoopLevel) && " Canon Expr doesn't have IV.");

  // Check if IV Coeff is blob.
  // There will be no spatial reuse as blob dist can be large.
  if (CE->hasIVBlobCoeff(LoopLevel))
    return SpatialTrip;

  // For locality, -i or +i is same as reuse happens in both cases.
  uint64_t IVCoeff = std::llabs(CE->getIVConstCoeff(LoopLevel));

  // DEBUG(dbgs() << "\n BitSize: " <<
  //     (Ref->getBaseSrcType()->getPrimitiveSizeInBits()));

  uint64_t TripCnt = getTripCount(Loop);

  // TODO: Assuming currently only Int/Float type, extend later.
  // getPrimitiveSizeInBits returns 0 for pointer type. TODO: Need to
  // investigate target machine information, later.
  const unsigned ElemPerCacheLine =
      Ref->getDestType()->isIntegerTy() ? IntsPerCacheLine : FloatsPerCacheLine;

  // We need to guard with max as equation may produce negative value, depending
  // upon the parameters.
  SpatialTrip =
      std::max(TripCnt - IVCoeff * (TripCnt / ElemPerCacheLine), (uint64_t)0);
  return SpatialTrip;
}

void HIRLocalityAnalysis::computeSpatialLocality(const HLLoop *Loop,
                                                 RefGroupVecTy &RefGroups) {

  unsigned Level = Loop->getNestingLevel();

  LocalityInfo &LI = LocalityByLevel[Level - 1];
  assert((LI.Spatial == 0) && "Spatial locality already populated!");

  for (auto &RefVec : RefGroups) {
    assert(!RefVec.empty() && " Ref Group is empty.");

    // We need to compute spatial locality for only one from each RefGroup.
    const RegDDRef *Ref = *(RefVec.begin());

    // No Spatial Reuse for multiple IV subscripts e.g. A[i+1][i] and A[i][i].
    unsigned IVPos = 0;
    if (isMultipleIV(Ref, Level, &IVPos)) {
      continue;
    }

    // Spatial locality is computed only if IVPos is the lowest index.
    // i.e A[j][i] has spatial locality for i, but not for j.
    if (IVPos != 1) {
      continue;
    }

    // Perform trip count calculation for spatial computation.
    uint64_t SpatialTrip = computeSpatialTrip(Ref, Loop);
    // Spatial Trip can be zero if IV Coeff is large or coeff has blob.
    // We assume that blobs associated with IV (e.g. M*i) will have large
    // reuse distance, since it is difficult to predict the blob values at
    // compile time.
    if (!SpatialTrip)
      continue;

    // Compute the spatial locality. Higher is better.
    // Adds a constant offset if Ref is Write to help differentiate cases.
    LI.Spatial += SpatialTrip;
    if (Ref->isLval())
      LI.Spatial += WriteWt;
  }
}

void HIRLocalityAnalysis::printLocalityInfo(raw_ostream &OS,
                                            const HLLoop *Lp) const {

  unsigned Level = Lp->getNestingLevel();

  // lit tests are dependent on the printing information.
  const LocalityInfo &LI = LocalityByLevel[Level - 1];

  OS << "\n Locality Info for Loop level: " << Level;
  OS << "\t Locality Value: " << LI.getLocalityValue();
  OS << "\t Spatial: " << LI.Spatial;
  OS << "\t TempInv: " << LI.TempInv;
  OS << "\t TempReuse: " << LI.TempReuse << "\n";
}

void HIRLocalityAnalysis::initTripCountByLevel(
    const SmallVectorImpl<const HLLoop *> &Loops) {

  for (auto Loop : Loops) {
    uint64_t TripCnt = 0;
    bool ConstTripLoop = Loop->isConstTripLoop(&TripCnt);

    TripCountByLevel[Loop->getNestingLevel() - 1] =
        (ConstTripLoop && (TripCnt < SymbolicConstTC)) ? TripCnt
                                                       : SymbolicConstTC;
  }
}

bool HIRLocalityAnalysis::isSpatialMatch(const RegDDRef *Ref1,
                                         const RegDDRef *Ref2, unsigned Level,
                                         uint64_t MaxDiff) {

  // Put all invariant refs in one group.
  if (Ref1->isStructurallyInvariantAtLevel(Level) &&
      Ref2->isStructurallyInvariantAtLevel(Level)) {
    return true;
  }

  if (Ref1->getNumDimensions() != Ref2->getNumDimensions()) {
    return false;
  }

  unsigned NumConstDiff = 0;

  if (!CanonExprUtils::areEqual(Ref1->getBaseCE(), Ref2->getBaseCE(), true)) {
    return false;
  }

  for (unsigned I = Ref1->getNumDimensions(); I > 0; --I) {

    // TODO: Investigate whether refs with different offsets for the first
    // dimension can be placed in the same group?
    // For example- A[i].0 and A[i].1
    if (DDRefUtils::compareOffsets(Ref1, Ref2, I)) {
      return false;
    }

    // Check if both the CanonExprs have IV.
    const CanonExpr *Ref1CE = Ref1->getDimensionIndex(I);
    const CanonExpr *Ref2CE = Ref2->getDimensionIndex(I);

    // For cases such as A[i+1][j+1] and A[i][j], where j+1 and j will have
    // const diff, but need to be placed in different groups for i-loop
    // grouping.
    if (!Ref1CE->hasIV(Level)) {
      // Compare 'j' and 'j+1'
      if (!CanonExprUtils::areEqual(Ref1CE, Ref2CE, true)) {
        return false;
      } else {
        continue;
      }
    }

    // Difference between the two canon expr should be constant.
    int64_t Diff;

    if (!CanonExprUtils::getConstDistance(Ref1CE, Ref2CE, &Diff)) {
      return false;
    }

    // If Diff is greater than MaxDiff then place it in a
    // separate bucket.
    if (static_cast<uint64_t>(std::llabs(Diff)) > MaxDiff) {
      return false;
    }

    if (Diff != 0) {
      NumConstDiff++;
      // Multiple Const diff will be in separate groups.
      if (NumConstDiff > 1) {
        return false;
      }
    }
  }

  // Both RegDDRefs are same. This shouldn't exist as we have removed
  // duplicates.
  assert(NumConstDiff && " Duplicate DDRef found.");

  return true;
}

bool HIRLocalityAnalysis::isTemporalMatch(const RegDDRef *Ref1,
                                          const RegDDRef *Ref2, unsigned Level,
                                          uint64_t MaxDiff) {

  // Put all invariant refs in one group.
  if (Ref1->isStructurallyInvariantAtLevel(Level) &&
      Ref2->isStructurallyInvariantAtLevel(Level)) {
    return true;
  }

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
  LocalityRefGatherer::MapTy MemRefMap;
  LocalityRefGatherer::gatherRange(Loop->child_begin(), Loop->child_end(),
                                   MemRefMap);

  // Debugging
  DEBUG(LocalityRefGatherer::dump(MemRefMap));

  // Sort the Memory References.
  LocalityRefGatherer::sortAndUnique(MemRefMap, true);

  DEBUG(dbgs() << " After sorting and removing dups\n");
  DEBUG(LocalityRefGatherer::dump(MemRefMap));
  DEBUG(dbgs() << " End\n");

  initTripCountByLevel(LoopVec);

  // For each loop, we create a reference group based on the sorted Memory Ref
  // Mapping, then compute the temporal reuse and spatial locality. The
  // Reference groups are based on per loop basis.
  for (auto CurLoop : LoopVec) {

    // Copy the MemRefMap as it will be modified as the loop locality is
    // computed.
    LocalityRefGatherer::MapTy LoopMemRefMap = MemRefMap;

    // Create Groupings based on index.

    DDRefGrouping::groupMap(
        RefGroups, LoopMemRefMap,
        std::bind(isSpatialMatch, std::placeholders::_1, std::placeholders::_2,
                  CurLoop->getNestingLevel(), NumCacheLines));
    DEBUG(DDRefGrouping::dump(RefGroups));

    computeTempInvLocality(CurLoop, RefGroups);

    computeSpatialLocality(CurLoop, RefGroups);

    DEBUG(printLocalityInfo(dbgs(), CurLoop));

    // Clear the grouping after last use for current loop.
    RefGroups.clear();
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
    return LocalityByLevel[Lp1->getNestingLevel() - 1].getLocalityValue() <
           LocalityByLevel[Lp2->getNestingLevel() - 1].getLocalityValue();
  };

  std::sort(SortedLoops.begin(), SortedLoops.end(), Comp);
}

uint64_t HIRLocalityAnalysis::getTemporalLocality(const HLLoop *Lp,
                                                  unsigned ReuseThreshold) {
  assert(Lp && " Loop parameter is null!");

  unsigned Level = Lp->getNestingLevel();

  RefGroupVecTy RefGroups;
  LocalityRefGatherer::MapTy MemRefMap;

  // Clear existing locality and trip count info.
  LocalityByLevel[Level - 1].clear();
  // Don't need a multiplication factor of trip count so setting it to 2 because
  // temporal invariant locality uses multiplication factor of (TripCount - 1).
  TripCountByLevel[Level - 1] = 2;

  LocalityRefGatherer::gatherRange(Lp->child_begin(), Lp->child_end(),
                                   MemRefMap);
  LocalityRefGatherer::sortAndUnique(MemRefMap, true);

  // Create groups with max possible reuse distance.
  DDRefGrouping::groupMap(RefGroups, MemRefMap,
                          std::bind(isTemporalMatch, std::placeholders::_1,
                                    std::placeholders::_2, Level, ~0UL));

  computeTempInvLocality(Lp, RefGroups);

  computeTempReuseLocality(
      Lp, RefGroups, ReuseThreshold ? ReuseThreshold : DefaultReuseThreshold);

  return LocalityByLevel[Level - 1].getTemporalLocality();
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
