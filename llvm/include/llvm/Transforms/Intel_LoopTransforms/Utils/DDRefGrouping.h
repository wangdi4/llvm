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
  template <typename RefTy> using RefGroupTy = SmallVector<RefTy *, 8>;

  template <typename RefTy>
  using RefGroupVecTy = std::vector<RefGroupTy<RefTy>>;

private:
  DDRefGrouping() = delete;
  ~DDRefGrouping() = delete;
  DDRefGrouping(const DDRefGrouping &) = delete;
  DDRefGrouping &operator=(const DDRefGrouping &) = delete;

  template <typename GroupingPredicate, typename InVector, typename OutVec>
  static void groupImpl(OutVec &Groups, const InVector &MemRefVector,
                        GroupingPredicate Predicate,
                        unsigned &CurrentGroupIndex) {
    auto StartGroupIndex = CurrentGroupIndex;

    for (auto *Ref : MemRefVector) {
      bool MatchFound = false;

      // Check if DDRef matches any of the groups.
      for (unsigned GroupIndex = StartGroupIndex, MaxGroupIndex = Groups.size();
           GroupIndex < MaxGroupIndex; ++GroupIndex) {
        auto &GroupRefVec = Groups[GroupIndex];
        assert(!GroupRefVec.empty() && "Ref Group is empty.");
        if (Predicate(GroupRefVec[0], Ref)) {
          MatchFound = true;
          GroupRefVec.push_back(Ref);
          break;
        }
      }

      // Create a new group since no match was found.
      if (!MatchFound) {
        Groups.resize(Groups.size() + 1);
        Groups.back().emplace_back(Ref);
      }
    }

    // Update CurrentGroupIndex for next invocation of the function.
    CurrentGroupIndex = Groups.size();
  }

public:
  /// \brief Creates a reference group out of the Symbol to Mem Ref Table.
  /// GroupingPredicate is a callable bool(const RefTy *, const RefTy *)
  /// that gets two RegDDRefs and returns true if both belong to the same group.
  template <typename GroupingPredicate, typename InMap, typename OutVec>
  static void groupMap(OutVec &Groups, const InMap &MemRefMap,
                       GroupingPredicate Predicate) {
    // Incremented whenever a new group is created.
    unsigned MaxGroupNo = 0;
    for (auto SymVecPair : MemRefMap) {
      groupImpl(Groups, SymVecPair.second, Predicate, MaxGroupNo);
    }
  }

  /// \brief Creates a reference group out of the Mem Ref Vector.
  /// GroupingPredicate is a callable bool(const RefTy *, const RefTy *)
  /// that gets two RegDDRefs and returns true if both belong to the same group.
  template <typename GroupingPredicate, typename InVector, typename OutVec>
  static void groupVec(OutVec &Groups, const InVector &MemRefVector,
                    GroupingPredicate Predicate) {
    // Incremented whenever a new group is created.
    unsigned MaxGroupNo = 0;
    groupImpl(Groups, MemRefVector, Predicate, MaxGroupNo);
  }

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  /// \brief Prints out the array reference group mapping.
  template <typename RefTy>
  static void dump(const RefGroupVecTy<RefTy> &Groups) {
    for (auto It = Groups.begin(), E = Groups.end(); It != E; ++It) {
      auto &RefVec = *It;

      dbgs() << "Group " << It - Groups.begin() << " contains: \n";

      for (const DDRef *Ref : RefVec) {
        dbgs() << "\t";
        Ref->dump();

        const RegDDRef *RegRef = dyn_cast<const RegDDRef>(Ref);
        bool IsLval = RegRef ? RegRef->isLval() : false;

        dbgs() << " {sb:" << Ref->getSymbase() << "} -> isWrite:" << IsLval
               << "\n";
      }
    }
  }
#endif
};
}
}

#endif
