//===- HIRLoopFusionGraph.h - Implements Loop Fusion Graph ----------------===//
//
// Copyright (C) 2017-2019 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_HIRLOOPFUSIONGRAPH_H
#define LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_HIRLOOPFUSIONGRAPH_H

#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/DenseSet.h"
#include "llvm/ADT/SetVector.h"
#include "llvm/ADT/SmallVector.h"

#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"

#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRDDAnalysis.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRLoopStatistics.h"

#include <list>
#include <queue>

namespace llvm {
namespace loopopt {

class HLNode;
class HLLoop;
class RegDDRef;

namespace fusion {

bool isGoodLoop(const HLLoop *Loop);

class FuseEdgeHeap;

// Represents a set of loops to fuse or a stand-alone statement. Fuse node is
// bad if it's removed or represents a stand-alone statement. Only good nodes
// may be fused.
class FuseNode {
  SmallVector<HLLoop *, 4> LoopsVector;
  HLNode *BadNode = nullptr;
  bool IsRemovedFlag = false;
  bool HasUnsafeSideEffects = false;

public:
  // Good node constructor
  FuseNode(HLLoop *Loop, bool HasUnsafeSideEffects);

  // Bad node constructor
  FuseNode(HLNode *BadNode, bool HasUnsafeSideEffects);

  bool isGoodNode() const { return BadNode == nullptr && !isRemoved(); }
  bool isBadNode() const { return !isGoodNode(); }

  LLVM_DUMP_METHOD
  void print(raw_ostream &OS) const;

  LLVM_DUMP_METHOD
  void dump() const {
    print(dbgs());
    dbgs() << "\n";
  }

  unsigned getTopSortNumber() const;

  void remove() { IsRemovedFlag = true; }

  bool isRemoved() const { return IsRemovedFlag; }

  HLLoop *pilotLoop() const { return loops().front(); }

  SmallVectorImpl<HLLoop *> &loops() {
    assert(isGoodNode() && "Getting loops from the bad node");
    return LoopsVector;
  }

  const SmallVectorImpl<HLLoop *> &loops() const {
    assert(isGoodNode() && "Getting loops from the bad node");
    return LoopsVector;
  }

  void merge(const FuseNode &Node);

  HLNode *getHLNode() const;

  bool hasUnsafeSideEffects() const { return HasUnsafeSideEffects; }
};

struct FuseEdge {
  unsigned Weight = 0;
  bool IsBadEdge = false;
  bool IsUndirected = false;

  LLVM_DUMP_METHOD
  void print(raw_ostream &OS) const;

  LLVM_DUMP_METHOD
  void dump() const {
    print(dbgs());
    dbgs() << "\n";
  }

  void merge(const FuseEdge &Edge);
};

class FuseGraph {
public:
  typedef SmallVector<FuseNode, 8> VertexContainerTy;

private:
  SmallDenseMap<std::pair<unsigned, unsigned>, FuseEdge> Edges;

  // Set of fuse node numbers.
  typedef SmallDenseSet<unsigned> NodeSetTy;

  // Map a fuse node number to a set of nodes.
  typedef std::unordered_map<unsigned, NodeSetTy> NodeMapTy;

  // Ordered set of fuse node numbers to be collapsed.
  typedef SmallSetVector<unsigned, 8> CollapseRangeTy;

  // Map a HLNode to a fuse node number.
  typedef SmallDenseMap<HLNode *, unsigned> GraphNodeMapTy;

  // Cache to store the result of areFusibleWithCommonTC() call.
  typedef SmallDenseMap<std::pair<FuseNode *, FuseNode *>, unsigned>
      FusibleCacheTy;

  typedef std::pair<RegDDRef *, HLNode *> RefNodePairTy;

  HIRDDAnalysis &DDA;
  HIRLoopStatistics &HLS;

  VertexContainerTy Vertex;

  // Note: NodeMapTy is a map of vertex V -> [Vi], where [Vi] is a set of
  //       vertices.

  // Maps V to a set of successors of V.
  NodeMapTy Successors;

  // Maps V to a set of predecessors of V.
  NodeMapTy Predecessors;

  // Maps V to a set of nodes that are connected with V with an
  // undirected edge.
  NodeMapTy Neighbors;

  // Maps V to a set of vertices reachable from V.
  NodeMapTy PathFrom;

  // Maps V to a set of vertices reachable from V where path contains a bad
  // node or edge.
  NodeMapTy BadPathFrom;

  // Maps V to a set of vertices that may reach V.
  NodeMapTy PathTo;

  // Maps V to a set of vertices that may reach V where path contains a bad
  // node or edge.
  NodeMapTy BadPathTo;

  // Note: The BadPath[To|From] sets are subsets of Path[To|From].

private:
  // Returns any edge, directed or undirected, that connects \p Node1 and \p
  // Node2. May return nullptr if nodes are not connected.
  FuseEdge *tryGetFuseEdge(unsigned Node1, unsigned Node2);

  // Returns any edge, directed or undirected, that connects \p Node1 and \p
  // Node2.
  FuseEdge &getFuseEdge(unsigned Node1, unsigned Node2) {
    auto *EdgePtr = tryGetFuseEdge(Node1, Node2);
    assert(EdgePtr != nullptr && "Edge not found");
    return *EdgePtr;
  }

  const FuseEdge &getFuseEdge(unsigned SrcNode, unsigned SinkNode) const {
    return const_cast<FuseGraph *>(this)->getFuseEdge(SrcNode, SinkNode);
  }

  // Returns the edge from \p SrcNode to \p SinkNode. Creates one if there is no
  // directed edge.
  FuseEdge &getOrCreateFuseEdge(unsigned SrcNode, unsigned SinkNode) {
    return Edges[std::make_pair(SrcNode, SinkNode)];
  }

  // Creates new fuse node for the \p Node.
  unsigned createFuseNode(GraphNodeMapTy &Map, HLNode *Node);

  // Returns fuse node that corresponds to the HLNode \p Node.
  unsigned getFuseNode(GraphNodeMapTy &Map, HLNode *Node);

  // Checks if two fuse nodes may be fused and returns common trip count.
  // Returns zero otherwise.
  unsigned areFusibleWithCommonTC(FusibleCacheTy &Cache, FuseNode &Node1,
                                  FuseNode &Node2);

  void initGraphHelpers();

  // Add directed edge to Successors and Predecessors maps.
  void addDirectedEdgeInternal(unsigned Src, unsigned Dst) {
    Successors[Src].insert(Dst);
    Predecessors[Dst].insert(Src);
  }

  // Add undirected edge to Neighbors map.
  void addNeighborEdgeInternal(unsigned NodeA, unsigned NodeB) {
    Neighbors[NodeA].insert(NodeB);
    Neighbors[NodeB].insert(NodeA);
  }

  void eraseNeighborEdgeInternal(unsigned NodeA, unsigned NodeB) {
    Neighbors[NodeA].erase(NodeB);
    Neighbors[NodeB].erase(NodeA);
  }

  // Init pathTo structures. O(V^2)
  void initPathToInfo(NodeMapTy &LocalPathFrom, NodeMapTy &LocalPathTo);

  // Init PathFrom, BadPathFrom, PathTo and BadPathTo structures.
  void initPathInfo(FuseEdgeHeap &Heap);

  // Update path info for a subtree of nodes.
  void updateSlice(unsigned NodeX, NodeMapTy &LocalPathFrom,
                   NodeSetTy &NewPathSources, NodeSetTy &NewPathSinks,
                   NodeMapTy &Preds);

  // Update both PathFrom and PathTo sets. Should be called twice to good and
  // bad path sets.
  void updateBothWays(unsigned NodeV, unsigned NodeX, NodeMapTy &LocalPathFrom,
                      NodeMapTy &LocalPathTo);

  // Assume collapse V <- X, update path info.
  void updatePathInfo(unsigned NodeV, unsigned NodeX);

  // Assume collapse V <- X, update successors.
  void updateSuccessors(FuseEdgeHeap &Heap, unsigned NodeV, unsigned NodeX,
                        const CollapseRangeTy &Range);

  // Assume collapse V <- X, update predecessors.
  void updatePredecessors(FuseEdgeHeap &Heap, unsigned NodeV, unsigned NodeX,
                          const CollapseRangeTy &Range);

  // Assume collapse V <- X, update neighbors.
  void updateNeighbors(FuseEdgeHeap &Heap, unsigned NodeV, unsigned NodeX,
                       const CollapseRangeTy &Range);

  // Collapse V <- [CollapseRange].
  void collapse(FuseEdgeHeap &Heap, unsigned NodeV,
                const CollapseRangeTy &CollapseRange);

  // Call demand driven DD to refine dependence vector between Src and Dst.
  RefinedDependence refineDependency(DDRef *Src, DDRef *Dst,
                                     unsigned CommonLevel,
                                     unsigned MaxLevel) const;

  // Returns true if \p Edge does not prevent fusion.
  bool isLegalDependency(const DDEdge &Edge, unsigned CommonLevel) const;

  void constructFuseNodes(GraphNodeMapTy &GraphNodeMap, HLNodeRangeTy Children);

  // It's illegal to fuse nodes with unsafe side effects and it's
  // also illegal to reorder them, so the fake dependency chain is created.
  void constructUnsafeSideEffectsChains();

  template <typename Iter>
  void constructUnsafeSideEffectsChainsOneWay(Iter Begin, Iter End);

  void constructDirectedEdges(DDGraph DDG, GraphNodeMapTy &GraphNodeMap,
                              FusibleCacheTy &FusibleCache, HLNode *ParentNode,
                              HLNodeRangeTy Children,
                              SmallVectorImpl<RefNodePairTy> &RValNodePairs);

  void constructUndirectedEdges(GraphNodeMapTy &GraphNodeMap,
                                FusibleCacheTy &FusibleCache,
                                SmallVectorImpl<RefNodePairTy> &RValNodePairs);

  void constructNaiveEdges(GraphNodeMapTy &GraphNodeMap,
                           FusibleCacheTy &FusibleCache);

  // Run weighted fusion algorithm.
  void weightedFusion();

  void dumpNodeSet(const NodeMapTy &Container) const;
  void dumpNodeRawMap(const NodeMapTy &Map) const;

  FuseGraph(HIRDDAnalysis &DDA, HIRLoopStatistics &HLS, DDGraph DDG,
            HLNode *ParentNode, HLNodeRangeTy Children);

  void verifyDependentMaps(StringRef Title, const NodeMapTy &BaseMap,
                           const NodeMapTy &CheckMap, bool ReverseEdges,
                           bool IgnoreRemovedNodes) const;

public:
  static FuseGraph create(HIRDDAnalysis &DDA, HIRLoopStatistics &HLS,
                          HLNode *ParentNode, HLNodeRangeTy Range);

  LLVM_DUMP_METHOD
  void dump() const;

  iterator_range<VertexContainerTy::iterator> getFuseNodes() {
    return make_range(Vertex.begin(), Vertex.end());
  }

  iterator_range<VertexContainerTy::const_iterator> getFuseNodes() const {
    return make_range(Vertex.begin(), Vertex.end());
  }

  // Populates the \p SortedFuseNodes with graph nodes in topological order.
  void
  topologicalSort(SmallVectorImpl<const FuseNode *> &SortedFuseNodes) const;

  void verify(bool InitialState) const;
};

} // namespace fusion
} // namespace loopopt
} // namespace llvm

#endif // LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_HIRLOOPFUSIONGRAPH_H
