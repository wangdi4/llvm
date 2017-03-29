//===------- DDTest.h - Provides Data Dependence Analysis -*-- C++ --*-----===//
//
// Copyright (C) 2015-2016 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//  This file contains common data structures for DD.
//  Somewhat misleadingly, it does not contain ddgraph itself, but rather its
//  parent class which contains the implementation of graph logic. This could
//  be ripped out to make a general graph in the future.
//
//  The other data structures are needed to implement dd tests, such as
//  DirectionVector. This is a common location to DDAnalysis and DDTest as both
//  need these defs, and we don't want those two including each other.
//
//  //TODO: in light of above comments, rename this?
//===----------------------------------------------------------------------===//
#ifndef INTEL_LOOPANALYSIS_DDGRAPH
#define INTEL_LOOPANALYSIS_DDGRAPH

#include "llvm/ADT/STLExtras.h"
#include "llvm/Analysis/Intel_LoopAnalysis/DDTests.h"
#include "llvm/IR/Intel_LoopIR/CanonExpr.h"
#include "llvm/IR/Intel_LoopIR/HLDDNode.h"
#include "llvm/IR/Intel_LoopIR/RegDDRef.h"
#include "llvm/Support/Debug.h"
#include <iterator>
#include <list>
#include <map>
#include <vector>

namespace llvm {
namespace loopopt {
class HLNode;

// TODO move to another location to make a generic graph?
// TODO doc req
// This was meant to be the basis of a general graph class
// focused on fast iteration at the cost of slow modification
// and extra memory
template <typename GraphNode, typename GraphEdge> class HIRGraph {
  typedef SmallVector<GraphEdge *, 4> GraphEdgeContainerTy;

  void addImpl(GraphEdge *EdgePtr) {
    InEdges[EdgePtr->getSink()].push_back(EdgePtr);
    OutEdges[EdgePtr->getSrc()].push_back(EdgePtr);
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

  void addEdge(const GraphEdge &E) {
    EdgesVector.push_back(E);
    addImpl(&EdgesVector.back());
  }

  void addEdge(GraphEdge &&E) {
    EdgesVector.push_back(std::move(E));
    addImpl(&EdgesVector.back());
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
    EdgesVector.clear();
  }

private:
  // It is assumed the common operation is to iterate over in/out edges
  // As such, we keep edge vectors for each node, with each edge stored
  // (as a struct vs ptr) twice; once in inEdges and once in outEdges
  mutable std::map<const GraphNode *, GraphEdgeContainerTy> InEdges;
  mutable std::map<const GraphNode *, GraphEdgeContainerTy> OutEdges;
  std::list<GraphEdge> EdgesVector;
};

class DDEdge {
private:
  enum class DepType : unsigned char {
    OUTPUT,
    INPUT,
    ANTI,
    FLOW,
  };

  DDRef *Src;
  DDRef *Sink;
  DirectionVector DV;
  DistanceVector DistVector;
  bool IsLoopIndepDepTemp;

public:
  DDEdge() {
    Src = Sink = nullptr;
    IsLoopIndepDepTemp = false;
  }
  DDEdge(DDRef *SrcRef, DDRef *SinkRef, const DirectionVector &DirV,
         const DistanceVector &DistV, bool IsLoopIndepDepTempIn = false)
      : Src(SrcRef), Sink(SinkRef), DV(DirV), DistVector(DistV) {
    IsLoopIndepDepTemp = IsLoopIndepDepTempIn;
  }

  DepType getEdgeType() const {
    RegDDRef *SrcRef = dyn_cast<RegDDRef>(Src);
    RegDDRef *SinkRef = dyn_cast<RegDDRef>(Sink);
    bool SrcIsLval = (SrcRef && SrcRef->isLval()) ? true : false;
    bool SinkIsLval = (SinkRef && SinkRef->isLval()) ? true : false;

    if (SrcIsLval) {
      if (SinkIsLval)
        return DepType::OUTPUT;
      return DepType::FLOW;
    } else {
      if (SinkIsLval)
        return DepType::ANTI;
      return DepType::INPUT;
    }
  }

  DDRef *getSrc() const { return Src; }
  DDRef *getSink() const { return Sink; }
  bool isLoopIndependentDepTemp() const { return IsLoopIndepDepTemp; }

  // Returns direction vector of the edge.
  const DirectionVector &getDV() const { return DV; }

  // Returns distance vector of the edge.
  const DistanceVector &getDistV() const { return DistVector; }

  // Returns DVKind element for a loop level.
  DVKind getDVAtLevel(unsigned Level) const { return DV[Level - 1]; }

  // Returns Distance for a loop level.
  DistTy getDistanceAtLevel(unsigned Level) const {
    return DistVector[Level - 1];
  }

  // Returns true if the edge is a Forward dependence
  bool isForwardDep() const {

    auto SrcTopSortNum = getSrc()->getHLDDNode()->getTopSortNum();
    auto SinkTopSortNum = getSink()->getHLDDNode()->getTopSortNum();

    // Handle the case A[I] = A[I] + B[I]
    // Case 1: The flow edge (from Lval to Rval) is backward.
    // Case 2: The anti edge (from Rval to Lval) is forward.
    if (SrcTopSortNum == SinkTopSortNum) {
      RegDDRef *SrcRef = dyn_cast<RegDDRef>(Src);
      bool SrcIsLval = (SrcRef && SrcRef->isLval());
      return !SrcIsLval;
    }
    return (SrcTopSortNum < SinkTopSortNum);
  }

  // Returns true if the edge prevents parallelization of Loop at Level
  // Note that this function only performs a quick check. It doesn't
  // perform the same level of analysis as ParVec analysis.
  bool preventsParallelization(unsigned Level) const {
    return !isINPUTdep() && hasCrossIterDepAtLevel(Level);
  }
  // Returns true if the edge prevents vectorization of Loop at Level
  // Note that this function only performs a quick check. It doesn't
  // perform the same level of analysis as ParVec analysis.
  bool preventsVectorization(unsigned Level) const {
    return preventsParallelization(Level) && !isForwardDep() &&
           (getSrc() != getSink());
  }
  // Proxy to isDVCrossIterDepAtLevel().
  bool hasCrossIterDepAtLevel(unsigned Level) const {
    return DV.isCrossIterDepAtLevel(Level);
  }
  // Proxy to isDVRefinableAtLevel().
  bool isRefinableDepAtLevel(unsigned Level) const {
    return DV.isRefinableAtLevel(Level);
  }

  bool isOUTPUTdep() const { return getEdgeType() == DepType::OUTPUT; }
  bool isFLOWdep() const { return getEdgeType() == DepType::FLOW; }
  bool isANTIdep() const { return getEdgeType() == DepType::ANTI; }
  bool isINPUTdep() const { return getEdgeType() == DepType::INPUT; }

  friend llvm::raw_ostream &operator<<(llvm::raw_ostream &out, DepType value) {
    const char *s = 0;
    switch (value) {
    case DepType::OUTPUT:
      s = "OUTPUT";
      break;
    case DepType::ANTI:
      s = "ANTI";
      break;
    case DepType::FLOW:
      s = "FLOW";
      break;
    case DepType::INPUT:
      s = "INPUT";
      break;
    }

    return out << s;
  }

  void print(raw_ostream &OS) const {
    formatted_raw_ostream FOS(OS);
    FOS << Src->getHLDDNode()->getNumber() << ":";
    FOS << Sink->getHLDDNode()->getNumber() << " ";
    Src->print(FOS);
    FOS << " --> ";
    Sink->print(FOS);
    FOS << " ";
    FOS << getEdgeType();
    FOS << " ";
    unsigned Level;
    for (Level = 0; Level < MaxLoopNestLevel; ++Level) {
      if (DV[Level] == DVKind::NONE) {
        break;
      }
    }
    DV.print(FOS, Level);
    DistVector.print(FOS, Level);
    FOS << " \n";
    // todo
  }

  void dump() const { print(dbgs()); }
};

// TODO better name?
// Specific graph type for dd is our "general" graph with
// nodes being ddrefs and edges being ddedge
typedef HIRGraph<DDRef, DDEdge> DDGraphTy;
}
}

#endif
