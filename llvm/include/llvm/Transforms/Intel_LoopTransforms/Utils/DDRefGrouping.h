//===-- DDRefGrouping.h - Implements DDRef Grouping utilities -*-- C++ --*-===//
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
// DDRefGrouping is an utility class to create DDRef groups
//
// A reference group is created at the specified level. All references
// in the group are differ by a constant.
//
// Simple Example:
// MemRefMap : B[J][I], B[J][I-1], B[J][I+1], B[J-1][I-1], B[J+1][I]
// RefGroup created for I-Level:
// Group 1 : B[J][I], B[J][I-1], B[J][I+1]
// Group 2 : B[J-1][I]
// Group 3 : B[J+1][I]
//
//===----------------------------------------------------------------------===//
#ifndef LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_UTILS_DDREFGROUPING_H
#define LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_UTILS_DDREFGROUPING_H

#include "llvm/Transforms/Intel_LoopTransforms/Utils/DDRefGatherer.h"

namespace llvm {

namespace loopopt {

class DDRefGrouping {
public:
  typedef MemRefGatherer::MapTy SymToMemRefTy;

  typedef SmallVector<RegDDRef *, 8> RefGroupTy;

  // RefGroupsTy data structure.
  // The first unsigned argument is the group number.
  typedef std::map<unsigned, RefGroupTy> RefGroupsTy;

private:
  DDRefGrouping() = delete;
  ~DDRefGrouping() = delete;
  DDRefGrouping(const DDRefGathererUtils &) = delete;
  DDRefGrouping &operator=(const DDRefGrouping &) = delete;

public:
  /// \brief Creates a reference group out of the Symbol to Mem Ref Table.
  /// GroupingPredicate is a callable bool(const RegDDRef *, const RegDDRef *)
  /// that gets two RegDDRefs and returns true if both belong to the same group.
  template <typename GroupingPredicate>
  static void createGroups(RefGroupsTy &Groups, const SymToMemRefTy &MemRefMap,
                           GroupingPredicate Predicate) {
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
          SmallVectorImpl<RegDDRef *> &GroupRefVec = Groups[GroupIndex];
          assert(!GroupRefVec.empty() && " Ref Group is empty.");
          if (Predicate(GroupRefVec[0], *VecIt)) {
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

/// \brief Prints out the array reference group mapping.
/// Primarily used for debugging.
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  static void dump(const RefGroupsTy &Groups);
#endif
};
}
}

#endif
