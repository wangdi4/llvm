//===------- DDTest.h - Provides Data Dependence Analysis -*-- C++ --*-----===//
//
// Copyright (C) 2015-2019 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//  This file contains common data structures for DD.
//
//  The other data structures are needed to implement dd tests, such as
//  DirectionVector. This is a common location to DDAnalysis and DDTest as both
//  need these defs, and we don't want those two including each other.
//
//===----------------------------------------------------------------------===//
#ifndef INTEL_LOOPANALYSIS_DDGRAPH
#define INTEL_LOOPANALYSIS_DDGRAPH

#include "llvm/Support/Debug.h"

#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/DDTests.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRGraph.h"

#include "llvm/Analysis/Intel_LoopAnalysis/IR/CanonExpr.h"
#include "llvm/Analysis/Intel_LoopAnalysis/IR/HLDDNode.h"
#include "llvm/Analysis/Intel_LoopAnalysis/IR/RegDDRef.h"

#include "llvm/Analysis/Intel_LoopAnalysis/Utils/DDRefGatherer.h"

namespace llvm {
namespace loopopt {
class HLNode;

typedef DDRefGatherer<DDRef, AllRefs ^ (ConstantRefs | GenericRValRefs |
                                        IsAddressOfRefs)>
    DDARefGatherer;

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

  DepType getEdgeType() const;

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
  bool isForwardDep() const;

  // Returns true if the edge prevents parallelization of Loop at Level
  // Note that this function only performs a quick check. It doesn't
  // perform the same level of analysis as ParVec analysis.
  bool preventsParallelization(unsigned Level) const {
    return !isInput() && hasCrossIterDepAtLevel(Level);
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
    // Cut down compile time for now by adding  more condition
    return DV.isRefinableAtLevel(Level);
  }

  bool isOutput() const { return getEdgeType() == DepType::OUTPUT; }
  bool isFlow() const { return getEdgeType() == DepType::FLOW; }
  bool isAnti() const { return getEdgeType() == DepType::ANTI; }
  bool isInput() const { return getEdgeType() == DepType::INPUT; }

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

  void print(raw_ostream &OS) const;

  void dump() const { print(dbgs()); }
};

typedef HIRGraph<DDRef, DDEdge> DDGraphTy;

// HIRDDAnalysis returns instances of this to ensure clients can access graph,
// but not modify it. Convenient places to reimplement iterators or filter
// graph as well
class DDGraph {
private:
  const HLNode *CurNode;
  DDGraphTy *G;

  template<bool IsIncoming>
  std::function<bool(const DDEdge *)> make_filter(const HLNode *Node) const {
    return DDGraphFilter<IsIncoming>(Node);
  }

  template<bool IsIncoming>
  class DDGraphFilter {
    unsigned FirstChildNum;
    unsigned LastChildNum;

    template <typename NodeTy> void init(const NodeTy *Node) {
      FirstChildNum = Node->getFirstChild()->getMinTopSortNum();
      LastChildNum = Node->getLastChild()->getMaxTopSortNum();
    }

  public:
    DDGraphFilter(const HLNode *Node) {
      if (!Node) {
        return;
      }

      if (const HLLoop *Loop = dyn_cast<HLLoop>(Node)) {
        init(Loop);
      } else {
        const HLRegion *Region = cast<HLRegion>(Node);
        init(Region);
      }
    }

    bool operator()(const DDEdge *Edge) {
      HLNode *ParentNode =
          (IsIncoming ? Edge->getSrc() : Edge->getSink())->getHLDDNode();

      if (!ParentNode) {
        return false;
      }

      unsigned Num = ParentNode->getTopSortNum();
      return (Num >= FirstChildNum && Num <= LastChildNum);
    }
  };

  const DDGraphTy *getGraphImpl() const {
    assert(G && "Trying to iterate over uninitialized graph!");
    return G;
  }

public:
  using FilterEdgeIterator =
      filter_iterator<DDGraphTy::EdgeIterator,
                      std::function<bool(const DDEdge *)>>;

  DDGraph() : CurNode(nullptr), G(nullptr) {}
  DDGraph(const HLNode *Node, DDGraphTy *Graph) : CurNode(Node), G(Graph) {}

  iterator_range<FilterEdgeIterator> incoming(const DDRef *Ref) const {
    return make_filter_range(
        llvm::make_range(getGraphImpl()->incoming_edges_begin(Ref),
                         getGraphImpl()->incoming_edges_end(Ref)),
        make_filter<true>(CurNode));
  }

  iterator_range<FilterEdgeIterator> outgoing(const DDRef *Ref) const {
    return make_filter_range(
        llvm::make_range(getGraphImpl()->outgoing_edges_begin(Ref),
                         getGraphImpl()->outgoing_edges_end(Ref)),
        make_filter<false>(CurNode));
  }

  FilterEdgeIterator incoming_edges_begin(const DDRef *Ref) const {
    return incoming(Ref).begin();
  }

  FilterEdgeIterator incoming_edges_end(const DDRef *Ref) const {
    return incoming(Ref).end();
  }

  FilterEdgeIterator outgoing_edges_begin(const DDRef *Ref) const {
    return outgoing(Ref).begin();
  }

  FilterEdgeIterator outgoing_edges_end(const DDRef *Ref) const {
    return outgoing(Ref).end();
  }

  // Returns the total number of incoming flow edges associated with this Ref
  // It includes the edges attached to blobs (if any) too
  unsigned getNumIncomingFlowEdges(const DDRef *Ref) const;

  // Returns the total number of outgoing edges associated with this Ref
  unsigned getNumOutgoingEdges(const DDRef *Ref) const {
    return std::distance(outgoing_edges_begin(Ref), outgoing_edges_end(Ref));
  };

  // Single edge going out of this DDRef.
  bool singleEdgeGoingOut(const DDRef *LRef);

  void print(raw_ostream &OS) const;

  void dump() const { print(dbgs()); }
};
} // namespace loopopt
} // namespace llvm

#endif
