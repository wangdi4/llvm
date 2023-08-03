//===- BarrierRegionInfo.h - Barrier region info ----------------*- C++ -*-===//
//
// Copyright (C) 2023 Intel Corporation
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you ("License"). Unless the License provides otherwise, you may
// not use, modify, copy, publish, distribute, disclose or transmit this
// software or the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_SYCLTRANSFORMS_UTILS_BARRIER_REGION_INFO_H
#define LLVM_TRANSFORMS_SYCLTRANSFORMS_UTILS_BARRIER_REGION_INFO_H

#include "llvm/ADT/DenseSet.h"
#include "llvm/ADT/MapVector.h"
#include "llvm/ADT/SetVector.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/Analysis/DominanceFrontier.h"
#include "llvm/Transforms/SYCLTransforms/Utils/BarrierUtils.h"

namespace llvm {

class Function;
class DominatorTree;
class BasicBlock;

/// Assumption
/// ==========
/// All barriers must be at the beginning of basic blocks. This is already done
/// by SplitBBonBarrier pass.
///
/// Terms
/// =====
/// - Sync basic block
///   A sync basic block (Sync BB) is a basic block who contains a barrier or a
///   dummy barrier. And the barrier is the first instruction of the basic
///   block according to the assumption above.
///
/// - Barrier region header
///   A basic block is a barrier region header iff it's
///     1) the entry block of a function, or
///     2) a sync BB, or
///     3) a dominance frontier of another region header.
///
/// - Barrier region
///   A barrier region starts with a region header, and contains a set of basic
///   blocks. A basic block (not a header) belongs to a barrier region iff its
///   idom
///     1) is the region header, or
///     2) belongs to the region.
///
/// Example
/// =======
/// Given a CFG as follows,
///
///       A
///   ,.-'|
///  |   *B
///  |   / \
///  |  C   D-.
///  |  |   |  | (backedge from F to D)
///  | *E  *F-'
///  |   \ /
///  |    G
///   `.  |
///     '-H
///       |
///      *I
///   (Blocks with * are sync BBs, i.e.,
///    block B, E, F and I start with barrier.)
///
/// barrier region headers are
///  - A (entry block)
///  - B, E, F, I (sync BB)
///  - D (dominance frontier of F)
///  - G (dominance frontier of A)
///  - H (dominance frontier of G)
///
/// so there are 8 barrier regions in total:
///  {A}, {B C}, {D}, {E}, {F}, {G}, {H} and {I}

class BarrierRegionInfo {
public:
  BarrierRegionInfo(Function *F, DominanceFrontier *DF, DominatorTree *DT) {
    analyze(F, DF, DT);
  }

  inline bool isRegionHeader(BasicBlock *BB) {
    return Regions.find(BB) != Regions.end();
  }

  inline BasicBlock *getRegionHeaderFor(BasicBlock *BB) {
    if (isRegionHeader(BB))
      return BB;
    assert(HeaderMap.find(BB) != HeaderMap.end() &&
           "BB doesn't belong to any region?");
    return HeaderMap[BB];
  }

  /// Returns true if the region header of \p LHS comes before the one of \p
  /// RHS in member variable \p Regions. See comments of \p Regions for
  /// details.
  bool compare(BasicBlock *LHS, BasicBlock *RHS) {
    auto LIt = Regions.find(getRegionHeaderFor(LHS)),
         RIt = Regions.find(getRegionHeaderFor(RHS));
    assert(LIt != Regions.end() && RIt != Regions.end() &&
           "Expected region headers");
    return LIt < RIt;
  }

private:
  void analyze(Function *F, DominanceFrontier *DF, DominatorTree *DT) {
    SetVector<BasicBlock *> Headers = collectRegionHeaders(F, DF, DT);
    constructRegions(F, Headers, DT);
  }

  void constructRegions(Function *F, SetVector<BasicBlock *> &Headers,
                        DominatorTree *DT) {
    // Create regions in order.
    for (auto *Header : Headers)
      Regions[Header];

    for (auto &BB : *F) {
      auto *HeaderNode = DT->getNode(&BB);
      assert(HeaderNode != nullptr && "Invalid header node!");
      BasicBlock *Header = HeaderNode->getBlock();
      while (!Headers.contains(Header)) {
        HeaderNode = HeaderNode->getIDom();
        Header = HeaderNode->getBlock();
      }

      if (Header != &BB) {
        auto &BlockSet = Regions[Header];
        BlockSet.insert(&BB);
        HeaderMap[&BB] = Header;
      }
    }
  }

  /// Returns all region headers in order. See comments of member variable
  /// \p Regions for details.
  static SetVector<BasicBlock *>
  collectRegionHeaders(Function *F, DominanceFrontier *DF, DominatorTree *DT) {
    std::list<BasicBlock *> Headers;
    Headers.push_back(&F->getEntryBlock());

    DenseSet<BasicBlock *> Visited;
    for (BasicBlock &BB : *F) {
      if (BarrierUtils::isBarrierOrDummyBarrierCall(&*BB.begin())) {
        Headers.push_back(&BB);
        Visited.insert(&BB);
      }
    }

    auto FrontierEnd = --Headers.end();
    SmallVector<BasicBlock *, 16> WorkList(Headers.begin(), Headers.end());
    do {
      auto *BB = WorkList.pop_back_val();
      if (!DT->isReachableFromEntry(BB))
        continue;
      for (BasicBlock *Frontier : DF->find(BB)->second) {
        if (Visited.insert(Frontier).second) {
          Headers.push_back(Frontier);
          WorkList.push_back(Frontier);
        }
      }
    } while (!WorkList.empty());

    // Remove duplicate dominance frontiers. We don't prevent inserting
    // duplicated elements above because we need to preserve the topological
    // order (see comments of the member Regions), so only the last occurrence
    // should be kept.
    Visited.clear();
    auto It = --Headers.end();
    while (It != FrontierEnd) {
      auto Next = --It;
      if (!Visited.insert(*It).second)
        Headers.erase(It);
      It = Next;
    }
    return {Headers.begin(), Headers.end()};
  }

private:
  /// A map between region headers and basic blocks belonging to them.
  /// Region headers are sorted in the following order:
  ///   Entry, Sync BBs ..., Dominance frontiers ...
  /// Dominance frontiers are also in topological order (ignoring loop
  /// backedges).
  MapVector<BasicBlock *, DenseSet<BasicBlock *>> Regions;

  /// A map between basic blocks and the region headers they belong to
  DenseMap<BasicBlock *, BasicBlock *> HeaderMap;
};

} // namespace llvm

#endif // LLVM_TRANSFORMS_SYCLTRANSFORMS_UTILS_BARRIER_REGION_INFO_H
