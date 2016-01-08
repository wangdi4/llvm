//===-------- DDRefGrouping.cpp - Implements DDRef Grouping utilities -----===//
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
// This file implements DDRefGrouping class.
//
//===----------------------------------------------------------------------===//
#include "llvm/Support/Debug.h"

#include "llvm/Transforms/Intel_LoopTransforms/Utils/CanonExprUtils.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/DDRefGrouping.h"

using namespace llvm;
using namespace llvm::loopopt;

// This method checks if Ref2 matches Ref1 to be stored in the same array
// reference group.
// The check does a diff = Ref1-Ref2. If the diff is constant, and has one
// differing subscript then we found a match. For example: A[i][j] and
// A[i+1][j] belongs to same group, whereas A[i+1][j+1] and A[i][j] are in
// different groups.
bool DDRefGrouping::isGroupMemRefMatch(const RegDDRef *Ref1,
                                       const RegDDRef *Ref2,
                                       unsigned Level,
                                       uint64_t MaxDiff) {

  // TODO: Think about if we can delinearize the subscripts.
  if (Ref1->getNumDimensions() != Ref2->getNumDimensions())
    return false;

  unsigned NumConstDiff = 0;

  // Compare base CE.
  // TODO: Currently assuming it to be in different groups. Need to add
  // support for cases such as *(ptr+i) and *(ptr+i+1).
  if (!CanonExprUtils::areEqual(Ref1->getBaseCE(), Ref2->getBaseCE())) {
    //assert(false && " Handle Base CE for array groups.");
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
    uint64_t Diff = std::abs(Result->getConstant()) / Result->getDenominator();

    // If Diff is greater than MaxDiff then place it in a
    // separate bucket.
    if (MaxDiff != 0 && Diff > MaxDiff) {
      return false;
    }

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

void DDRefGrouping::createGroups(
    RefGroupsTy &Groups,
    const SymToMemRefTy &MemRefMap,
    unsigned Level,
    uint64_t MaxDiff) {

  // Incremented whenever a new group is created.
  unsigned MaxGroupNo = 0;

  for (auto SymVecPair = MemRefMap.begin(), Last = MemRefMap.end();
       SymVecPair != Last; ++SymVecPair) {

    // Keep track of the new groups to match existing DDRefs.
    unsigned StartGroupIndex = MaxGroupNo;

    auto &RefVec = SymVecPair->second;
    for (auto VecIt = RefVec.begin(), End = RefVec.end(); VecIt != End;
         ++VecIt) {

      bool MatchFound = false;

      // Check if DDRef matches any of the groups.
      for (unsigned GroupIndex = StartGroupIndex; GroupIndex < MaxGroupNo;
           ++GroupIndex) {
        SmallVectorImpl<const RegDDRef *> &GroupRefVec = Groups[GroupIndex];
        assert(!GroupRefVec.empty() && " Ref Group is empty.");
        if (isGroupMemRefMatch(GroupRefVec[0], *VecIt, Level, MaxDiff)) {
          MatchFound = true;
          GroupRefVec.push_back(*VecIt);
          break;
        }
      }

      // Create a new group since no match was found.
      if (!MatchFound) {
        Groups[MaxGroupNo++].push_back(*VecIt);
      }
    }
  }
}

// Used primarily for debugging.
void DDRefGrouping::dump(const RefGroupsTy &Groups) {
  dbgs() << "\n Reference Groups \n";
  for (auto SymVecPair = Groups.begin(), Last = Groups.end();
       SymVecPair != Last; ++SymVecPair) {
    auto &RefVec = SymVecPair->second;
    dbgs() << "Group " << SymVecPair->first
        << " {sb: " << RefVec.front()->getSymbase() << "} contains: \n";
    for (auto Ref = RefVec.begin(), E = RefVec.end(); Ref != E; ++Ref) {
      dbgs() << "\t";
      (*Ref)->dump();
      dbgs() << " -> isWrite:" << (*Ref)->isLval() << "\n";
    }
  }
}
