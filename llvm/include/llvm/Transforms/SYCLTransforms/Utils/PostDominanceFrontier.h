//===- PostDominanceFrontier.h - Backward dominator frontiers ---*- C++ -*-===//
//
// Copyright (C) 2022 Intel Corporation
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
//
// PostDominanceFrontierBase Class - Concrete subclass of DominanceFrontierBase
// that is used to compute a backward dominator frontiers.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_SYCLTRANSFORMS_UTILS_POSTDOMINANCEFRONTIER_H
#define LLVM_TRANSFORMS_SYCLTRANSFORMS_UTILS_POSTDOMINANCEFRONTIER_H

#include "llvm/Analysis/DominanceFrontier.h"
#include "llvm/Analysis/DominanceFrontierImpl.h"
#include "llvm/Analysis/PostDominators.h"

namespace llvm {

template <class BlockT>
class PostDominanceFrontierBase : public DominanceFrontierBase<BlockT, true> {
private:
  typedef GraphTraits<BlockT *> BlockTraits;

public:
  using DomTreeT = PostDomTreeBase<BlockT>;
  using DomTreeNodeT = DomTreeNodeBase<BlockT>;
  using DomSetType = typename DominanceFrontierBase<BlockT, true>::DomSetType;

  PostDominanceFrontierBase() : DominanceFrontierBase<BlockT, true>() {}

  void analyze(DomTreeT &DT) { calculate(DT, DT.getRootNode()); }

  const DominanceFrontier::DomSetType &calculate(const DomTreeT &DT,
                                                 const DomTreeNode *Node) {
    // Loop over CFG successors to calculate DFlocal[Node]
    BasicBlock *BB = Node->getBlock();
    if (DT.root_size() == 0)
      return this->Frontiers[BB];

    DomSetType *Result = nullptr;

    std::vector<DFCalculateWorkObject<BlockT>> WorkList;
    SmallPtrSet<BlockT *, 32> Visited;

    WorkList.emplace_back(BB, nullptr, Node, nullptr);
    do {
      DFCalculateWorkObject<BlockT> *CurrentW = &WorkList.back();
      assert(CurrentW && "missing work object.");

      BlockT *CurrentBB = CurrentW->currentBB;
      BlockT *ParentBB = CurrentW->parentBB;
      const DomTreeNode *CurrentNode = CurrentW->Node;
      const DomTreeNode *ParentNode = CurrentW->parentNode;
      assert(CurrentNode && "Invalid work object. Missing current Node");

      DomSetType &S = this->Frontiers[CurrentBB]; // The new set to fill in...
      if (CurrentBB) {
        // Visit each block only once.
        if (Visited.insert(CurrentBB).second) {
          // Loop over CFG predecessors to calculate DFLocal[CurrentNode]
          for (auto *P : predecessors(CurrentBB)) {
            // Does Node immediately dominate this predecessor?
            DomTreeNode *SINode = DT[P];
            if (SINode && SINode->getIDom() != CurrentNode)
              S.insert(P);
          }
        }
      }

      // At this point, S is DFlocal.  Now we union in DFup's of our children...
      // Loop through and visit the nodes that Node immediately dominates
      // (Node's children in the IDomTree)
      bool VisitChild = false;
      for (DomTreeNode::const_iterator NI = CurrentNode->begin(),
                                       NE = CurrentNode->end();
           NI != NE; ++NI) {
        DomTreeNode *IDominee = *NI;
        BasicBlock *ChildBB = IDominee->getBlock();
        if (Visited.count(ChildBB) == 0) {
          WorkList.emplace_back(ChildBB, CurrentBB, IDominee, CurrentNode);
          VisitChild = true;
        }
      }

      // If all children are visited or there isn't any child then pop this
      // block from the worklist.
      if (!VisitChild) {
        if (!ParentBB) {
          Result = &S;
          break;
        }

        typename DomSetType::const_iterator CDFI = S.begin(), CDFE = S.end();
        DomSetType &ParentSet = this->Frontiers[ParentBB];
        for (; CDFI != CDFE; ++CDFI) {
          if (!DT.properlyDominates(ParentNode, DT[*CDFI]))
            ParentSet.insert(*CDFI);
        }
        WorkList.pop_back();
      }

    } while (!WorkList.empty());

    return *Result;
  }
};

using PostDominanceFrontier = PostDominanceFrontierBase<BasicBlock>;

} // namespace llvm

#endif // LLVM_TRANSFORMS_SYCLTRANSFORMS_UTILS_POSTDOMINANCEFRONTIER_H
