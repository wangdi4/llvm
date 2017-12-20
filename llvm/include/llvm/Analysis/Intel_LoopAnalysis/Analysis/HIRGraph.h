//===------ HIRGraph.h - Base graph data structure --------*-- C++ --*-----===//
//
// Copyright (C) 2017 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_ANALYSIS_INTEL_LOOPANALYSIS_HIRGRAPH_H
#define LLVM_ANALYSIS_INTEL_LOOPANALYSIS_HIRGRAPH_H

#include "llvm/ADT/STLExtras.h"
#include <iterator>
#include <list>
#include <unordered_map>

namespace llvm {
namespace loopopt {

template <typename GraphNode, typename GraphEdge> class HIRGraph {
  typedef SmallVector<GraphEdge *, 4> GraphEdgeContainerTy;

  // It is assumed the common operation is to iterate over in/out edges
  // As such, we keep edge vectors for each node, with each edge stored
  // (as a struct vs ptr) twice; once in inEdges and once in outEdges
  mutable std::unordered_map<const GraphNode *, GraphEdgeContainerTy> InEdges;
  mutable std::unordered_map<const GraphNode *, GraphEdgeContainerTy> OutEdges;
  std::list<GraphEdge> EdgesList;

  void addEdgeImpl(GraphEdge &EdgePtr) {
    InEdges[EdgePtr.getSink()].push_back(&EdgePtr);
    OutEdges[EdgePtr.getSrc()].push_back(&EdgePtr);
  }

public:
  typedef typename GraphEdgeContainerTy::const_iterator EdgeIterator;

  typedef std::pointer_to_unary_function<GraphEdge *, GraphNode *>
      GraphNodeDerefFun;

  typedef mapped_iterator<EdgeIterator, GraphNodeDerefFun> children_iterator;

  static GraphNode *SinkFun(GraphEdge *E) { return E->getSink(); }

  children_iterator children_begin(GraphNode *Node) {
    return map_iterator(outgoing_edges_begin(Node), GraphNodeDerefFun(SinkFun));
  }

  children_iterator children_end(GraphNode *Node) {
    return map_iterator(outgoing_edges_end(Node), GraphNodeDerefFun(SinkFun));
  }

  iterator_range<EdgeIterator> incoming(const GraphNode *Node) const {
    return make_range(incoming_edges_begin(Node), incoming_edges_end(Node));
  }

  iterator_range<EdgeIterator> outgoing(const GraphNode *Node) const {
    return make_range(outgoing_edges_begin(Node), outgoing_edges_end(Node));
  }

  // Don't let others modify edges. We can only remove or add
  // edges
  EdgeIterator incoming_edges_begin(const GraphNode *Node) const {
    return InEdges[Node].begin();
  }

  EdgeIterator incoming_edges_end(const GraphNode *Node) const {
    return InEdges[Node].end();
  }

  EdgeIterator outgoing_edges_begin(const GraphNode *Node) const {
    return OutEdges[Node].begin();
  }

  EdgeIterator outgoing_edges_end(const GraphNode *Node) const {
    return OutEdges[Node].end();
  }

  GraphEdge &addEdge(const GraphEdge &E) {
    EdgesList.push_back(E);
    addEdgeImpl(EdgesList.back());
    return EdgesList.back();
  }

  GraphEdge &addEdge(GraphEdge &&E) {
    EdgesList.push_back(std::move(E));
    addEdgeImpl(EdgesList.back());
    return EdgesList.back();
  }

  void print(raw_ostream &OS) const {
    for (auto I = OutEdges.begin(), E = OutEdges.end(); I != E; ++I) {
      auto Edges = I->second;
      for (auto EIt = Edges.begin(), EdgesEnd = Edges.end(); EIt != EdgesEnd;
           ++EIt) {
        (*EIt)->print(OS);
      }
    }
  }

  void dump() const { print(dbgs()); }

  void clear() {
    InEdges.clear();
    OutEdges.clear();
    EdgesList.clear();
  }
};
}
}

#endif // LLVM_ANALYSIS_INTEL_LOOPANALYSIS_HIRGRAPH_H
