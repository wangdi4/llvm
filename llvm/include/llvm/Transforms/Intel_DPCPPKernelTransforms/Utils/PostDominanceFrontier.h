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

#ifndef LLVM_TRANSFORMS_INTEL_DPCPPKERNELTRANSFORMS_UTILS_POSTDOMINANCEFRONTIER_H
#define LLVM_TRANSFORMS_INTEL_DPCPPKERNELTRANSFORMS_UTILS_POSTDOMINANCEFRONTIER_H

#include "llvm/Analysis/DominanceFrontier.h"
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
    DomSetType &S = this->Frontiers[BB]; // The new set to fill in...
    if (DT.root_size() == 0)
      return S;

    if (BB) {
      for (auto *P : predecessors(BB)) {
        // Does Node immediately dominate this predecessor?
        DomTreeNode *SINode = DT[P];
        if (SINode && SINode->getIDom() != Node)
          S.insert(P);
      }
    }
    // At this point, S is DFlocal.  Now we union in DFup's of our children...
    // Loop through and visit the nodes that Node immediately dominates (Node's
    // children in the IDomTree)
    //
    for (DomTreeNode::const_iterator NI = Node->begin(), NE = Node->end();
         NI != NE; ++NI) {
      DomTreeNode *IDominee = *NI;
      const DomSetType &ChildDF = calculate(DT, IDominee);
      typename DomSetType::const_iterator CDFI = ChildDF.begin(),
                                          CDFE = ChildDF.end();
      for (; CDFI != CDFE; ++CDFI) {
        if (!DT.properlyDominates(Node, DT[*CDFI]))
          S.insert(*CDFI);
      }
    }
    return S;
  }
};

using PostDominanceFrontier = PostDominanceFrontierBase<BasicBlock>;

} // namespace llvm

#endif // LLVM_TRANSFORMS_INTEL_DPCPPKERNELTRANSFORMS_UTILS_POSTDOMINANCEFRONTIER_H
