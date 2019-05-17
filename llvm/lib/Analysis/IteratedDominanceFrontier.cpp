//===- IteratedDominanceFrontier.cpp - Compute IDF ------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// Compute iterated dominance frontiers using a linear time algorithm.
//
//===----------------------------------------------------------------------===//

#include "llvm/Analysis/IteratedDominanceFrontier.h"
#include "llvm/IR/CFG.h"
#include "llvm/IR/Dominators.h"
#include <queue>

namespace llvm {

#if INTEL_CUSTOMIZATION
template <class NodeTy, bool IsPostDom, class BlockTy>
void IDFCalculator<NodeTy, IsPostDom, BlockTy>::calculate(
    SmallVectorImpl<BlockTy *> &PHIBlocks) {
#endif // INTEL_CUSTOMIZATION
  // Use a priority queue keyed on dominator tree level so that inserted nodes
  // are handled from the bottom of the dominator tree upwards. We also augment
  // the level with a DFS number to ensure that the blocks are ordered in a
  // deterministic way.
  typedef std::pair<DomTreeTy *, std::pair<unsigned, unsigned>> // INTEL
      DomTreeNodePair;
  typedef std::priority_queue<DomTreeNodePair, SmallVector<DomTreeNodePair, 32>,
                              less_second> IDFPriorityQueue;
  IDFPriorityQueue PQ;

  DT.updateDFSNumbers();

  for (BlockTy *BB : *DefBlocks) {        // INTEL
    if (DomTreeTy *Node = DT.getNode(BB)) // INTEL
      PQ.push({Node, std::make_pair(Node->getLevel(), Node->getDFSNumIn())});
  }

#if INTEL_CUSTOMIZATION
  SmallVector<DomTreeTy *, 32> Worklist;
  SmallPtrSet<DomTreeTy *, 32> VisitedPQ;
  SmallPtrSet<DomTreeTy *, 32> VisitedWorklist;
#endif // INTEL_CUSTOMIZATION

  while (!PQ.empty()) {
    DomTreeNodePair RootPair = PQ.top();
    PQ.pop();
    DomTreeTy *Root = RootPair.first; // INTEL
    unsigned RootLevel = RootPair.second.first;

    // Walk all dominator tree children of Root, inspecting their CFG edges with
    // targets elsewhere on the dominator tree. Only targets whose level is at
    // most Root's level are added to the iterated dominance frontier of the
    // definition set.

    Worklist.clear();
    Worklist.push_back(Root);
    VisitedWorklist.insert(Root);

    while (!Worklist.empty()) {
      DomTreeTy *Node = Worklist.pop_back_val(); // INTEL
      BlockTy *BB = Node->getBlock();            // INTEL
      // Succ is the successor in the direction we are calculating IDF, so it is
      // successor for IDF, and predecessor for Reverse IDF.
      auto DoWork = [&](BlockTy *Succ) {        // INTEL
        DomTreeTy *SuccNode = DT.getNode(Succ); // INTEL

        const unsigned SuccLevel = SuccNode->getLevel();
        if (SuccLevel > RootLevel)
          return;

        if (!VisitedPQ.insert(SuccNode).second)
          return;

        BlockTy *SuccBB = SuccNode->getBlock(); // INTEL
        if (useLiveIn && !LiveInBlocks->count(SuccBB))
          return;

        PHIBlocks.emplace_back(SuccBB);
        if (!DefBlocks->count(SuccBB))
          PQ.push(std::make_pair(
              SuccNode, std::make_pair(SuccLevel, SuccNode->getDFSNumIn())));
      };

      if (GD && !std::is_same<BlockTy, vpo::VPBlockBase>::value) { // INTEL
        for (auto Pair : children<
#if INTEL_CUSTOMIZATION
                 std::pair<const GraphDiff<BlockTy *, IsPostDom> *, NodeTy>>(
#endif // INTEL_CUSTOMIZATION
                 {GD, BB}))
          DoWork(Pair.second);
      } else {
        for (auto *Succ : children<NodeTy>(BB))
          DoWork(Succ);
      }

      for (auto DomChild : *Node) {
        if (VisitedWorklist.insert(DomChild).second)
          Worklist.push_back(DomChild);
      }
    }
  }
}

template class IDFCalculator<BasicBlock *, false>;
template class IDFCalculator<Inverse<BasicBlock *>, true>;
#if INTEL_CUSTOMIZATION
template class IDFCalculator<vpo::VPBlockBase *, false, vpo::VPBlockBase>;
#endif // INTEL_CUSTOMIZATION
}
