//===---- HIRLocalityAnalysis.cpp - Computes Locality Analysis ------------===//
//
// Copyright (C) 2015 Intel Corporation. All rights reserved.
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
#include "llvm/Analysis/Intel_LoopAnalysis/HIRLocalityAnalysis.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Passes.h"
#include "llvm/Analysis/Intel_LoopAnalysis/HIRParser.h"
#include "llvm/Analysis/Intel_LoopAnalysis/SymbaseAssignment.h"
#include "llvm/IR/Type.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/raw_ostream.h"

#include "llvm/Transforms/Intel_LoopTransforms/Utils/CanonExprUtils.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/DDRefUtils.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/HLNodeUtils.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/HLNodeVisitor.h"
#include "llvm/IR/Intel_LoopIR/DDRefGatherer.h"

using namespace llvm;
using namespace llvm::loopopt;

#define DEBUG_TYPE "hir-locality-analysis"

// Primarily used for debugging purpose.
// We do not print any information except DEBUG mode.
static cl::opt<bool> debugLocality(
    "hir-debug-locality", cl::init(false), cl::Hidden,
    cl::desc("Pre-computes locality for all loops inside function."));

FunctionPass *llvm::createHIRLocalityAnalysisPass() {
  return new HIRLocalityAnalysis();
}

char HIRLocalityAnalysis::ID = 0;
INITIALIZE_PASS_BEGIN(HIRLocalityAnalysis, "hir-locality-analysis",
                      "HIR Locality Analysis", false, true)
INITIALIZE_PASS_DEPENDENCY(SymbaseAssignment)
INITIALIZE_PASS_DEPENDENCY(HIRParser)
INITIALIZE_PASS_END(HIRLocalityAnalysis, "hir-locality-analysis",
                    "HIR Locality Analysis", false, true)

void HIRLocalityAnalysis::getAnalysisUsage(AnalysisUsage &AU) const {

  AU.setPreservesAll();
  AU.addRequired<HIRParser>();
  AU.addRequired<SymbaseAssignment>();
}

// Performs a basic setup without actually running the locality
// analysis.
bool HIRLocalityAnalysis::runOnFunction(Function &F) {

  // We use DEBUG throughout, so that nothing should be displayed
  // in production mode.
  if (debugLocality) {
    checkLocality();
  }

  return false;
}

// Only used when debugLocality is on.
void HIRLocalityAnalysis::checkLocality() {

  // For any changes, please run the llvm-lit tests
  // to check for any breakage.
  SmallVector<const HLLoop *, 16> Loops;
  HLNodeUtils::gatherOutermostLoops(Loops);
  for (auto &I : Loops) {
    SmallVector<LoopLocalityPair, 16> LoopLocality;
    sortedLocalityLoops(I, LoopLocality);
  }
}

void HIRLocalityAnalysis::releaseMemory() {

  // Delete all the Locality Info.
  for (auto &I : LocalityMap) {
    delete I.second;
  }
  LocalityMap.clear();
  // Below are redundant clears.
  RefGroups.clear();
  ConstTripCache.clear();
}

// Comparator for sorting Loops with Locality value.
struct compareLocalityValue {
  bool operator()(const LoopLocalityPair &left, const LoopLocalityPair &right) {
    return left.second < right.second;
  }
};

void HIRLocalityAnalysis::print(raw_ostream &OS, const Module *M) const {

  // TODO: Discuss about what information needs to be printed in production
  // mode.
  formatted_raw_ostream FOS(OS);
  FOS << "Locality Information for all loops(sorted order):\n";
  SmallVector<LoopLocalityPair, 16> LoopLocality;
  for (auto &I : LocalityMap) {
    LoopLocality.push_back(
        std::make_pair(I.first, (I.second)->getLocalityValue()));
  }
  std::sort(LoopLocality.begin(), LoopLocality.end(), compareLocalityValue());
  for (auto &I : LoopLocality) {
    const HLLoop *L = I.first;
    printLocalityInfo(OS, L);
  }
}

void HIRLocalityAnalysis::markLoopModified(const HLLoop *L) {

  assert(L && " Loop parameter is null.");

  // Mark loop as modified.
  LoopModificationMap[L] = true;

  // Mark all the parents as modified.
  const HLLoop *CurLoop = L->getParentLoop();
  while (CurLoop) {
    LoopModificationMap[CurLoop] = true;
    CurLoop = CurLoop->getParentLoop();
  }
}

// Compares the CanonExpr associated with a memory reference
// TODO: Think about moving this to CanonExprUtils and merging with areEqual.
bool compareMemRefCE(const CanonExpr *ACanon, const CanonExpr *BCanon) {

  // TODO: handle type comparison.
  // Not sure, if this is necessary at all.
  assert(CanonExprUtils::isTypeEqual(ACanon, BCanon) &&
         " Handle Canon Expr type comparison.");

  // Check the number of IV's.
  if (ACanon->numIVs() != BCanon->numIVs())
    return (ACanon->numIVs() < BCanon->numIVs());

  // Check the IV's.
  for (auto IVIt1 = ACanon->iv_begin(), IVIt2 = BCanon->iv_begin(),
            End = ACanon->iv_end();
       IVIt1 != End; ++IVIt1, ++IVIt2) {
    if (IVIt1->Coeff != IVIt2->Coeff)
      return (IVIt1->Coeff < IVIt2->Coeff);
    if (IVIt1->Index != IVIt2->Index)
      return (IVIt1->Index < IVIt2->Index);
  }

  // Check the number of blobs.
  if (ACanon->numBlobs() != BCanon->numBlobs())
    return (ACanon->numBlobs() < BCanon->numBlobs());

  // Check the Blob's.
  for (auto It1 = ACanon->blob_begin(), End = ACanon->blob_end(),
            It2 = BCanon->blob_begin();
       It1 != End; ++It1, ++It2) {

    if (It1->Coeff != It2->Coeff)
      return (It1->Coeff < It2->Coeff);
    if (It1->Index != It2->Index)
      return (It1->Index < It2->Index);
  }

  if (ACanon->getConstant() != BCanon->getConstant())
    return (ACanon->getConstant() < BCanon->getConstant());

  if (ACanon->getDenominator() != BCanon->getDenominator())
    return (ACanon->getDenominator() < BCanon->getDenominator());

  // Check division type for non-unit denominator.
  if ((ACanon->getDenominator() != 1) &&
      (ACanon->isSignedDiv() != BCanon->isSignedDiv())) {
    return ACanon->isSignedDiv();
  }

  // Assert, since the two canon expr should differ atleast one case,
  // as we have already checked for their equality.
  assert(false && " CanonExprs should be different.");

  return true;
}

// Sorting comparator operator for two DDRef.
// This sorting compares the two ddref and orders them based on
// the dimensions, IV's, blobs and then writes.
// For example: A[i+5][j], A[i][0] -> Read, A[i][j], A[i+k][0],
// A[i][0] -> Write will be sorted as
// A[i][0] -> Write, A[i][0] -> Read, A[i+k][0], A[i][j], A[i+5][j].
bool compareMemRef(const RegDDRef *A, const RegDDRef *B) {

  if (!CanonExprUtils::areEqual(A->getBaseCE(), B->getBaseCE()))
    return compareMemRefCE(A->getBaseCE(), B->getBaseCE());

  if (A->getNumDimensions() != B->getNumDimensions()) {
    return (A->getNumDimensions() < B->getNumDimensions());
  }

  // Check canon expr of the two ddrefs.
  for (auto AIter = A->canon_begin(), End = A->canon_end(),
            BIter = B->canon_begin();
       AIter != End; ++AIter, ++BIter) {

    const CanonExpr *ACanon = *AIter;
    const CanonExpr *BCanon = *BIter;

    if (!CanonExprUtils::areEqual(ACanon, BCanon)) {
      return compareMemRefCE(ACanon, BCanon);
    }
  }

  // Place writes first in case everything matches.
  if (B->isLval()) {
    return false;
  }

  return true;
}

void HIRLocalityAnalysis::sortMemRefs(SymToMemRefTy &MemRefMap) {

  // Sorts the memory reference based on the comparison provided
  // by compareMemRef.
  for (auto SymVecPair = MemRefMap.begin(), Last = MemRefMap.end();
       SymVecPair != Last; ++SymVecPair) {
    SmallVectorImpl<RegDDRef *> &RefVec = SymVecPair->second;
    std::sort(RefVec.begin(), RefVec.end(), compareMemRef);
  }
}

bool areEqualRef(RegDDRef *Reg1, RegDDRef *Reg2) {
  return DDRefUtils::areEqual(Reg1, Reg2);
}

void HIRLocalityAnalysis::removeDuplicates(SymToMemRefTy &MemRefMap) {

  // Removes the duplicates by comparing the Ref's in sorted order.
  for (auto SymVecPair = MemRefMap.begin(), Last = MemRefMap.end();
       SymVecPair != Last; ++SymVecPair) {
    SmallVectorImpl<RegDDRef *> &RefVec = SymVecPair->second;

    RefVec.erase(std::unique(RefVec.begin(), RefVec.end(), areEqualRef),
                 RefVec.end());
  }
}

void HIRLocalityAnalysis::computeTempInvLocality(const HLLoop *Loop,
                                                 SymToMemRefTy &MemRefMap) {

  // We need to find invariant DDRefs and compute a temporal locality
  // based on the loop's trip count.
  // For eg. for(i=0; i<N; i++) { A[t] = 0; }. Here Array A is invariant
  // and we add 'N' to temporal locality.

  LocalityInfo *LI = LocalityMap[Loop];

  // Compute Temp. Invariant Locality.
  for (auto SymVecPair = MemRefMap.begin(), Last = MemRefMap.end();
       SymVecPair != Last; ++SymVecPair) {

    SmallVectorImpl<RegDDRef *> &RefVec = SymVecPair->second;

    // Count Invariant References and remove them from the map.
    // Note, we need to check vector end, since elements are deleted.
    for (auto Iter = RefVec.begin(); Iter != RefVec.end();) {

      const RegDDRef *RegDD = *Iter;

      if (RegDD->isStructurallyInvariantAtLevel(Loop->getNestingLevel())) {
        Iter = RefVec.erase(Iter);
        // Adding 'N-1' to temporal locality.
        LI->TempInv += (getTripCount(Loop) - 1);
      } else {
        Iter++;
      }
    }
  }
}

// Removes the empty Symbases from the list.
// This is needed since the invariant DDRef will be removed from
// the list.
void HIRLocalityAnalysis::clearEmptySlots(SymToMemRefTy &MemRefMap) {

  // Note, we need to check map end since the elements
  // are deleted.
  for (auto SymVecPair = MemRefMap.begin(); SymVecPair != MemRefMap.end();) {

    SmallVectorImpl<RegDDRef *> &RefVec = SymVecPair->second;
    if (RefVec.empty()) {
      auto EraseIt = SymVecPair;
      SymVecPair++;
      MemRefMap.erase(EraseIt);
    } else {
      SymVecPair++;
    }
  }
}

// This method checks if Ref2 matches Ref1 to be stored in the same array
// reference group.
// The check does a diff = Ref1-Ref2. If the diff is constant, and has one
// differing subscript then we found a match. For example: A[i][j] and
// A[i+1][j] belongs to same group, whereas A[i+1][j+1] and A[i][j] are in
// different groups.
bool HIRLocalityAnalysis::isGroupMemRefMatch(const RegDDRef *Ref1,
                                             const RegDDRef *Ref2,
                                             unsigned Level) {

  // TODO: Think about if we can delinearize the subscripts.
  if (Ref1->getNumDimensions() != Ref2->getNumDimensions())
    return false;

  unsigned NumConstDiff = 0;

  // Compare base CE.
  // TODO: Currently assuming it to be in different groups. Need to add
  // support for cases such as *(ptr+i) and *(ptr+i+1).
  if (!CanonExprUtils::areEqual(Ref1->getBaseCE(), Ref2->getBaseCE())) {
    assert(false && " Handle Base CE for array groups.");
    return false;
  }

  for (auto Ref1Iter = Ref1->canon_begin(), End = Ref1->canon_end(),
            Ref2Iter = Ref2->canon_begin();
       Ref1Iter != End; ++Ref1Iter, ++Ref2Iter) {

    // Check if both the CanonExprs have IV.
    const CanonExpr *Ref1CE = *Ref1Iter;
    const CanonExpr *Ref2CE = *Ref2Iter;

    // For cases such as A[i+1][j+1] and A[i][j], where j+1 and j will have
    // const diff, but need to be placed in different groups for i-loop
    // grouping.
    if (!Ref1CE->hasIV(Level)) {
      // Compare 'j' and 'j+1'
      if (!CanonExprUtils::areEqual(Ref1CE, Ref2CE)) {
        return false;
      } else {
        continue;
      }
    }

    // TODO: Handle CanonExpr types inside MemRef's.
    // We can be very conservative and return false, but this might
    // lead to overestimated values. Currently, adding assert.
    assert(CanonExprUtils::isTypeEqual(Ref1CE, Ref2CE) &&
           " CanonExpr type mismatch.");

    // Diff the CanonExprs.
    const CanonExpr *Result = CanonExprUtils::cloneAndSubtract(Ref1CE, Ref2CE);

    // Result should not have any IV's or blobs.
    if (Result->hasBlob() || Result->hasIV())
      return false;

    // Difference between the two canon expr should be constant.
    int64_t Diff = std::abs(Result->getConstant()) / Result->getDenominator();

    // If Diff is greater than NumCacheLines then place it in a
    // separate bucket.
    if (Diff > NumCacheLines)
      return false;

    if (Diff != 0) {
      NumConstDiff++;
      // Multiple Const diff will be in separate groups.
      if (NumConstDiff > 1)
        return false;
    }
  }

  // Both RegDDRefs are same. This shouldn't exist as we have removed
  // duplicates.
  assert(NumConstDiff && " Duplicate DDRef found.");

  return true;
}

// Creates the RefGrouping.
// A reference group is created at the specified level.
// Only the IV at that level should change as specified in isGroupMemRefMatch.
// Simple Example:
// MemRefMap : B[J][I], B[J][I-1], B[J][I+1], B[J-1][I], B[J+1][I]
// RefGroup created for I-Level:
// Group 1 : B[J][I], B[J][I-1], B[J][I+1]
// Group 2 : B[J-1][I]
// Group 3 : B[J+1][I]
void HIRLocalityAnalysis::createRefGroups(SymToMemRefTy &MemRefMap,
                                          unsigned Level) {

  // Incremented whenever a new group is created.
  unsigned MaxGroupNo = 0;

  for (auto SymVecPair = MemRefMap.begin(), Last = MemRefMap.end();
       SymVecPair != Last; ++SymVecPair) {

    // Keep track of the new groups to match existing DDRefs.
    unsigned StartGroupIndex = MaxGroupNo;

    SmallVectorImpl<RegDDRef *> &RefVec = SymVecPair->second;
    for (auto VecIt = RefVec.begin(), End = RefVec.end(); VecIt != End;
         ++VecIt) {

      bool MatchFound = false;

      // Check if DDRef matches any of the groups.
      for (unsigned GroupIndex = StartGroupIndex; GroupIndex < MaxGroupNo;
           ++GroupIndex) {
        SmallVectorImpl<const RegDDRef *> &GroupRefVec = RefGroups[GroupIndex];
        assert(!GroupRefVec.empty() && " Ref Group is empty.");
        if (isGroupMemRefMatch(GroupRefVec[0], *VecIt, Level)) {
          MatchFound = true;
          GroupRefVec.push_back(*VecIt);
          break;
        }
      }

      // Create a new group since no match was found.
      if (!MatchFound) {
        RefGroups[MaxGroupNo++].push_back(*VecIt);
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

  assert(NumIV && " Atleast one IV position should exist.");
  return false;
}

// We need to find temporal reuse for cases such as A[i] and A[i+2],
// which can be allocated to a temp registers for future use.
bool HIRLocalityAnalysis::isTemporalReuse(const RegDDRef *Ref1,
                                          const RegDDRef *Ref2,
                                          unsigned IVPos) {

  // Compare the diff at subscript Pos.
  CanonExpr *Ref1CE = Ref1->getDimensionIndex(IVPos);
  CanonExpr *Ref2CE = Ref2->getDimensionIndex(IVPos);

  // Diff the CanonExprs.
  const CanonExpr *Result = CanonExprUtils::cloneAndSubtract(Ref1CE, Ref2CE);

  // TODO: Being conservative with Denom.
  if (Result->getDenominator() > 1)
    return false;

  assert(Result->isIntConstant() &&
         " Result is not constant for temporal reuse.");

  int64_t Diff = Result->getConstant();

  // If Diff is greater than Threshold, then temporal reuse exist.
  if (std::abs(Diff) < TempReuseThreshold)
    return true;

  return false;
}

void HIRLocalityAnalysis::computeTempReuseLocality(const HLLoop *Loop) {

  LocalityInfo *LI = LocalityMap[Loop];
  for (auto GroupVecPair = RefGroups.begin(), Last = RefGroups.end();
       GroupVecPair != Last; ++GroupVecPair) {

    SmallVectorImpl<const RegDDRef *> &RefVec = GroupVecPair->second;
    assert(!RefVec.empty() && " Ref Group is empty.");

    // The first reference is used for comparison.
    const RegDDRef *CompareRef = *(RefVec.begin());

    // No Temporal Reuse for multiple IV subscripts e.g. A[i+1][i] and A[i][i].
    unsigned IVPos = 0;
    if (isMultipleIV(CompareRef, Loop->getNestingLevel(), &IVPos)) {
      continue;
    }

    for (auto VecIt = RefVec.begin() + 1, End = RefVec.end(); VecIt != End;
         ++VecIt) {
      if (isTemporalReuse(CompareRef, *VecIt, IVPos)) {
        LI->TempReuse += 1;
      } else {
        // Update the compare ref.
        CompareRef = *VecIt;
      }
    }
  }
}

bool HIRLocalityAnalysis::isLoopModified(const HLLoop *Loop) {

  // Checks if status exist and returns the status.
  auto Iter = LoopModificationMap.find(Loop);
  if (Iter != LoopModificationMap.end())
    return Iter->second;

  // Default is to return true.
  return true;
}

int64_t HIRLocalityAnalysis::getTripCount(const HLLoop *Loop) {

  auto Iter = ConstTripCache.find(Loop);
  if (Iter != ConstTripCache.end()) {
    // Return Const Trip Count.
    return Iter->second;
  }

  // If Trip Cache doesn't have loop, it indicates 'N' trip count.
  return SymbolicConst;
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
  uint64_t IVCoeff = std::abs(CE->getIVConstCoeff(LoopLevel));

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

void HIRLocalityAnalysis::computeSpatialLocality(const HLLoop *Loop) {

  LocalityInfo *LI = LocalityMap[Loop];

  for (auto GroupVecPair = RefGroups.begin(), Last = RefGroups.end();
       GroupVecPair != Last; ++GroupVecPair) {

    SmallVectorImpl<const RegDDRef *> &RefVec = GroupVecPair->second;
    assert(!RefVec.empty() && " Ref Group is empty.");

    // We need to compute spatial locality for only one from each RefGroup.
    const RegDDRef *Ref = *(RefVec.begin());

    // No Spatial Reuse for multiple IV subscripts e.g. A[i+1][i] and A[i][i].
    unsigned IVPos = 0;
    if (isMultipleIV(Ref, Loop->getNestingLevel(), &IVPos)) {
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
    LI->Spatial += SpatialTrip;
    if (Ref->isLval())
      LI->Spatial += WriteWt;
  }
}

// Used primarily for debugging.
void HIRLocalityAnalysis::printRefGroups() {

  DEBUG(dbgs() << "\n Reference Groups \n");
  for (auto SymVecPair = RefGroups.begin(), Last = RefGroups.end();
       SymVecPair != Last; ++SymVecPair) {
    SmallVectorImpl<const RegDDRef *> &RefVec = SymVecPair->second;
    DEBUG(dbgs() << "Group " << SymVecPair->first << " contains: \n");
    for (auto Ref = RefVec.begin(), E = RefVec.end(); Ref != E; ++Ref) {
      DEBUG(dbgs() << "\t");
      DEBUG((*Ref)->dump());
      DEBUG(dbgs() << " -> isWrite:" << (*Ref)->isLval() << "\n");
    }
  }
}

void HIRLocalityAnalysis::printLocalityInfo(raw_ostream &OS,
                                            const HLLoop *L) const {

  // lit tests are dependent on the printing information.
  auto Iter = LocalityMap.find(L);
  assert((Iter != LocalityMap.end()) && " Locality info not found for loop.");
  const LocalityInfo *LI = Iter->second;
  OS << "\n Locality Info for Loop level: " << L->getNestingLevel();
  OS << "\t Locality Value: " << LI->getLocalityValue();
  OS << "\t Spatial: " << LI->Spatial;
  OS << "\t TempInv: " << LI->TempInv;
  OS << "\t TempReuse: " << LI->TempReuse << "\n";
}

void HIRLocalityAnalysis::initConstTripCache(
    SmallVectorImpl<const HLLoop *> *Loops) {

  for (auto Iter = Loops->begin(), End = Loops->end(); Iter != End; ++Iter) {
    const HLLoop *Loop = *Iter;

    int64_t TripCnt = 0;
    bool ConstTripLoop = Loop->isConstTripLoop(&TripCnt);

    // If Const Trip Loop and Trip is less than Symbolic 'N'
    // add it to trip cache. If loops are not present in trip cache,
    // they are assumed to be 'N'.
    if (ConstTripLoop && TripCnt < SymbolicConst)
      ConstTripCache[Loop] = TripCnt;
  }
}

// Clears out the locality map information if it exists, otherwise
// it creates a new one for first time visits to the loop.
void HIRLocalityAnalysis::resetLocalityMap(const HLLoop *L) {

  auto Iter = LocalityMap.find(L);
  if (Iter != LocalityMap.end()) {
    // Clear existing info.
    LocalityInfo *LI = Iter->second;
    LI->clear();
  } else {
    // Create a new info.
    LocalityMap[L] = new LocalityInfo();
  }
}

// This is a high level routine to compute different locality.
void HIRLocalityAnalysis::computeLocality(const HLLoop *Loop,
                                          bool EnableCache) {

  // Check if the Loop locality is valid from past computation.
  if (!isLoopModified(Loop) && EnableCache)
    return;

  // Get the Symbase to Memory References.
  SymToMemRefTy MemRefMap;
  MemRefGatherer::gather(Loop, MemRefMap);

  // Debugging
  DEBUG(dumpRefMap(MemRefMap));

  // Sort the Memory References.
  sortMemRefs(MemRefMap);
  DEBUG(dbgs() << " After sorting\n");
  DEBUG(dumpRefMap(MemRefMap));

  // Remove duplicate memory references.
  removeDuplicates(MemRefMap);

  DEBUG(dbgs() << " After sorting and removing dups\n");
  DEBUG(dumpRefMap(MemRefMap));
  DEBUG(dbgs() << " End\n");

  // Collect all the loops inside the loop nest.
  SmallVector<const HLLoop *, 16> LoopVec;
  HLNodeUtils::gatherAllLoops(Loop, LoopVec);

  initConstTripCache(&LoopVec);

  // For each loop, we create a reference group based on the sorted Memory Ref
  // Mapping, then compute the temporal reuse and spatial locality. The
  // Reference groups are based on per loop basis.
  for (auto Iter = LoopVec.begin(), End = LoopVec.end(); Iter != End; ++Iter) {

    const HLLoop *CurLoop = *Iter;

    // Copy the MemRefMap as it will be modified as the loop locality is
    // computed.
    SymToMemRefTy LoopMemRefMap = MemRefMap;

    // Clear the LocalityInfo if exists or create a new one.
    resetLocalityMap(CurLoop);

    computeTempInvLocality(CurLoop, LoopMemRefMap);

    // Remove the empty entries inside symbase since loop invariant entries
    // are erased by temp invariant locality pass.
    clearEmptySlots(LoopMemRefMap);

    DEBUG(dumpRefMap(LoopMemRefMap));

    // Create Groupings based on index.
    createRefGroups(LoopMemRefMap, CurLoop->getNestingLevel());
    DEBUG(printRefGroups());

    computeTempReuseLocality(CurLoop);

    computeSpatialLocality(CurLoop);

    DEBUG(printLocalityInfo(dbgs(), CurLoop));

    // Clear the grouping after last use for current loop.
    RefGroups.clear();
  }

  // Clears the trip count cache after use.
  ConstTripCache.clear();
}

uint64_t HIRLocalityAnalysis::getLocalityValue(const HLLoop *Loop) {
  assert(Loop && " Loop parameter is null.");

  // Compute the locality for the entire loop nest, if necessary.
  computeLocality(Loop);

  assert(LocalityMap.count(Loop) && " Loop locality information not found.");
  LocalityInfo *LI = LocalityMap[Loop];
  return LI->getLocalityValue();
}

void HIRLocalityAnalysis::verifyLocality(const HLLoop *Loop) {

  // If Loop is modified, then we cannot do a cache check (verify).
  if (isLoopModified(Loop)) {
    computeLocality(Loop, false);
  } else {
    // Compare the cached value with newly computed value.
    uint64_t CachedLoc = LocalityMap[Loop]->getLocalityValue();
    computeLocality(Loop, false);

    (void)CachedLoc;
    assert((CachedLoc != LocalityMap[Loop]->getLocalityValue()) &&
           " Cached locality information not consistent.");
  }
}

// The LoopLocality will contain Loops along with LocalityValue in sorted order.
// Note that the topmost Loop level must be passed to this method, similar
// to loop interchange use case.
void HIRLocalityAnalysis::sortedLocalityLoops(
    const HLLoop *OutermostLoop,
    SmallVectorImpl<LoopLocalityPair> &LoopLocality) {

  assert(OutermostLoop && " Loop parameter is null.");
  assert(LoopLocality.empty() && "LoopLocality vector is non-empty.");

// Compute the locality for the entire loop nest, if necessary.
// In debug mode, we test the cached loop information and recomputed
// value, whereas in non-debug mode, we use cached value. This is primarily
// done for testing.
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  // Debug mode.
  verifyLocality(OutermostLoop);
#else
  // Optimized mode.
  computeLocality(OutermostLoop, true);
#endif

  SmallVector<const HLLoop *, 16> Loops;
  HLNodeUtils::gatherAllLoops(OutermostLoop, Loops);

  // Store all the Loop Locality Information.
  for (auto Iter = Loops.begin(), End = Loops.end(); Iter != End; ++Iter) {
    const HLLoop *CurLoop = *Iter;
    assert(LocalityMap.count(CurLoop) &&
           " Locality information for loop not found.");
    LocalityInfo *LI = LocalityMap[CurLoop];
    LoopLocality.push_back(std::make_pair(CurLoop, LI->getLocalityValue()));
  }

  std::sort(LoopLocality.begin(), LoopLocality.end(), compareLocalityValue());
}
