//===--------- AllSCCIterator.h - SCC Iterator over all components --------===//
//
// Copyright (C) 2016-2017 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// The all_scc_iterator is an extension of LLVM scc_iterator that enumerates
// all SCCs including components that are not reachable from the graph entry
// node.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_UTILS_ALLSCCITERATOR_H
#define LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_UTILS_ALLSCCITERATOR_H

#include "llvm/ADT/SCCIterator.h"

namespace llvm {

template <class GraphT, class GT = GraphTraits<GraphT>>
class all_scc_iterator : public scc_iterator<GraphT, GT> {
protected:
  using typename scc_iterator<GraphT, GT>::NodeRef;

  GraphT Graph;

  void GetNextSCCUnconnected() {
    if (this->CurrentSCC.empty() && this->VisitStack.empty()) {
      GraphT *GraphP = &const_cast<GraphT &>(Graph);
      for (auto CurNodeI = GT::nodes_begin(GraphP),
                LastNodeI = GT::nodes_end(GraphP);
           CurNodeI != LastNodeI; ++CurNodeI) {

        NodeRef CurNode = &*CurNodeI;
        typename DenseMap<NodeRef, unsigned>::iterator Visited =
            this->nodeVisitNumbers.find(CurNode);
        if (Visited == this->nodeVisitNumbers.end()) {
          // this node has never been seen, so find SCC based on it
          this->DFSVisitOne(CurNode);
          this->GetNextSCC();
          GetNextSCCUnconnected();
          return;
        }
      }
    }
  }

  all_scc_iterator(NodeRef entryN, const GraphT &G)
      : scc_iterator<GraphT, GT>(entryN), Graph(G) {
    GetNextSCCUnconnected();
  }

  all_scc_iterator(const GraphT &G) : scc_iterator<GraphT, GT>(), Graph(G) {}

public:
  static all_scc_iterator begin(const GraphT &G) {
    return all_scc_iterator(GT::getEntryNode(G), G);
  }

  static all_scc_iterator end(const GraphT &G) { return all_scc_iterator(G); }

  all_scc_iterator &operator++() {
    this->GetNextSCC();
    GetNextSCCUnconnected();
    return *this;
  }
};

/// Construct the begin iterator for a deduced graph type T.
template <class T> all_scc_iterator<T> all_scc_begin(const T &G) {
  return all_scc_iterator<T>::begin(G);
}

/// Construct the end iterator for a deduced graph type T.
template <class T> all_scc_iterator<T> all_scc_end(const T &G) {
  return all_scc_iterator<T>::end(G);
}

}

#endif
