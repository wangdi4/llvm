//===- HIRLoopFusionGraph.cpp - Implements Loop Fusion Graph --------------===//
//
// Copyright (C) 2017-2019 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//

#include "HIRLoopFusionGraph.h"

#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRLoopStatistics.h"

#include "llvm/Analysis/Intel_LoopAnalysis/IR/CanonExpr.h"
#include "llvm/Analysis/Intel_LoopAnalysis/IR/HLLoop.h"
#include "llvm/Analysis/Intel_LoopAnalysis/IR/HLNode.h"

#include "llvm/Analysis/Intel_LoopAnalysis/Utils/CanonExprUtils.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/DDRefGatherer.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/DDRefGrouping.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HLNodeUtils.h"

#define DEBUG_TYPE "hir-loop-fusion"
#define DEBUG_FG(X) DEBUG_WITH_TYPE(DEBUG_TYPE "-fg", X)

static cl::opt<bool>
    ConstructNaiveEdges(DEBUG_TYPE "-naive-edges", cl::init(false), cl::Hidden,
                        cl::desc("Construct profitable edges just because "
                                 "loops are having common trip count"));

using namespace llvm;
using namespace llvm::loopopt;
using namespace llvm::loopopt::fusion;

typedef DDRefGatherer<DDRef, AllRefs ^ (ConstantRefs | GenericRValRefs |
                                        IsAddressOfRefs)>
    Gatherer;

bool fusion::isGoodLoop(const HLLoop *Loop) {
  return Loop->isDo() && Loop->isNormalized() &&
         !(Loop->isDistributedForMemRec() || Loop->hasUnrollEnablingPragma() ||
           Loop->hasVectorizeEnablingPragma() ||
           Loop->hasFusionDisablingPragma());
}

class fusion::FuseEdgeHeap {
public:
  struct FuseHeapEntity {
    unsigned Src;
    unsigned Dst;
    unsigned Weight;

    FuseHeapEntity(unsigned Src, unsigned Dst, unsigned Weight)
        : Src(Src), Dst(Dst), Weight(Weight) {}
  };

private:
  struct FuseHeapEntityImpl : FuseHeapEntity {
    bool IsRemoved;

    FuseHeapEntityImpl(unsigned Src, unsigned Dst, unsigned Weight)
        : FuseHeapEntity(Src, Dst, Weight), IsRemoved(false) {}
  };

  struct FuseHeapComparator {
    bool operator()(const FuseHeapEntityImpl *Entity1,
                    const FuseHeapEntityImpl *Entity2) {
      return Entity1->Weight < Entity2->Weight;
    }
  };

  std::list<FuseHeapEntityImpl> Container;

  SmallDenseMap<std::pair<unsigned, unsigned>, FuseHeapEntityImpl *> Map;
  mutable std::priority_queue<FuseHeapEntityImpl *,
                              SmallVector<FuseHeapEntityImpl *, 8>,
                              FuseHeapComparator>
      PQ;

  FuseHeapEntityImpl &getEntity(unsigned Src, unsigned Dst) {
    auto Iter = Map.find(std::make_pair(Src, Dst));
    assert(Iter != Map.end());
    return *Iter->second;
  }

  FuseHeapEntityImpl *tryGetEntity(unsigned Src, unsigned Dst) {
    auto Iter = Map.find(std::make_pair(Src, Dst));
    if (Iter == Map.end()) {
      return nullptr;
    }
    return Iter->second;
  }

public:
  bool empty() const {
    if (PQ.empty()) {
      return true;
    }

    auto Size = PQ.size();
    FuseHeapEntityImpl *Entity = PQ.top();

    while (Entity->IsRemoved) {
      // If node is removed;
      PQ.pop();

      Size--;
      if (Size == 0) {
        return true;
      }

      Entity = PQ.top();
    }

    return false;
  }

  // O(log(n))
  void push(unsigned Src, unsigned Dst, unsigned Weight) {
    if (tryGetEntity(Src, Dst) || tryGetEntity(Dst, Src)) {
      return;
    }

    Container.emplace_back(Src, Dst, Weight);

    auto *Entity = &Container.back();

    PQ.push(Entity);
    Map.try_emplace(std::make_pair(Src, Dst), Entity);
  }

  // Avg: O(log(n))
  // Worst: O(n*log(n))
  void pop() {
    // Skip IsRemoved elements.
    bool IsEmpty = empty();
    assert(!IsEmpty && "pop() on empty heap");
    (void)IsEmpty;

    FuseHeapEntityImpl *Entity = PQ.top();

    Map.erase(std::make_pair(Entity->Src, Entity->Dst));
    PQ.pop();
  }

  // O(1)
  FuseHeapEntity &top() const {
    bool IsEmpty = empty();
    assert(!IsEmpty && "pop() on empty heap");
    (void)IsEmpty;

    return *PQ.top();
  }

  // O(1).
  void remove(unsigned Src, unsigned Dst) {
    auto *Entry = tryGetEntity(Src, Dst);
    if (!Entry) {
      return;
    }

    Entry->IsRemoved = true;
    Map.erase(std::make_pair(Src, Dst));
  }

  // O(log(n))
  void reheapEdge(unsigned Src, unsigned Dst, unsigned NewWeight) {
    auto *Entity = tryGetEntity(Src, Dst);
    if (!Entity) {
      // Edge may be already handled and removed from the heap.
      return;
    }

    remove(Src, Dst);
    push(Src, Dst, NewWeight);
  }

  // O(1)
  void replaceEdge(unsigned Src, unsigned Dst, unsigned NewSrc,
                   unsigned NewDst) {
    auto *Entity = tryGetEntity(Src, Dst);
    if (!Entity) {
      // Edge may be already handled and removed from the heap.
      return;
    }

    Entity->Src = NewSrc;
    Entity->Dst = NewDst;

    Map.erase(std::make_pair(Src, Dst));
    Map.try_emplace(std::make_pair(NewSrc, NewDst), Entity);
  }
};

static unsigned areLoopsFusibleWithCommonTC(const HLLoop *Loop1,
                                            const HLLoop *Loop2) {
  const CanonExpr *UB1 = Loop1->getUpperCanonExpr();
  const CanonExpr *UB2 = Loop2->getUpperCanonExpr();

  // UBDist = L1_UB - L2_UB
  int64_t UBDist;
  if (!CanonExprUtils::getConstDistance(UB1, UB2, &UBDist)) {
    return 0;
  }

  if (std::abs(UBDist) > 3) {
    return 0;
  }

  if (!HLNodeUtils::dominates(Loop1, Loop2)) {
    return 0;
  }

  if (!HLNodeUtils::postDominates(Loop2, Loop1)) {
    return 0;
  }

  uint64_t CommonTC;
  if (!Loop1->isConstTripLoop(&CommonTC)) {
    // Consider max trip count estimation.
    CommonTC = 100;
  }

  // L1 |-------------------------|
  // L2 |----------------|
  //                     ^-UBDist-^
  //
  // Common trip count is (L1_TC - UBDist), if UBDist > 0
  //                       L1_TC, otherwise.

  if (UBDist > 0) {
    CommonTC -= UBDist;
  }

  return CommonTC;
}

static bool hasUnsafeSideEffects(HIRLoopStatistics &HLS, const HLLoop *Loop) {
  return HLS.getTotalLoopStatistics(Loop).hasCallsWithUnsafeSideEffects();
}

struct UnsafeSideEffectsDetector : HLNodeVisitorBase {
  HIRLoopStatistics &HLS;
  bool HasUnsafeSideEffects;
  const HLNode *SkipNode;

  UnsafeSideEffectsDetector(HIRLoopStatistics &HLS)
      : HLS(HLS), HasUnsafeSideEffects(false), SkipNode(nullptr) {}

  void visit(const HLNode *) {}
  void postVisit(const HLNode *) {}

  void visit(const HLInst *Inst) {
    assert(!HasUnsafeSideEffects && "Unsafe side effects already found");
    HasUnsafeSideEffects = Inst->isUnsafeSideEffectsCallInst();
  }

  void visit(const HLLoop *Loop) {
    assert(!HasUnsafeSideEffects && "Unsafe side effects already found");
    HasUnsafeSideEffects = hasUnsafeSideEffects(HLS, Loop);
    SkipNode = Loop;
  }

  bool skipRecursion(const HLNode *Node) const { return SkipNode == Node; }

  bool isDone() const { return HasUnsafeSideEffects; }
};

static bool hasUnsafeSideEffects(HIRLoopStatistics &HLS, HLNode *Node) {
  assert(!isa<HLLoop>(Node) && "Use call with an explicit HLLoop node type");

  UnsafeSideEffectsDetector V(HLS);
  HLNodeUtils::visit(V, Node);

  return V.HasUnsafeSideEffects;
}

FuseNode::FuseNode(HLLoop *Loop, bool HasUnsafeSideEffects)
    : HasUnsafeSideEffects(HasUnsafeSideEffects) {
  assert(isGoodLoop(Loop) && "Can not create a good node from a bad loop.");
  LoopsVector.push_back(Loop);
}

FuseNode::FuseNode(HLNode *BadNode, bool HasUnsafeSideEffects)
    : BadNode(BadNode), HasUnsafeSideEffects(HasUnsafeSideEffects) {}

void FuseNode::print(raw_ostream &OS) const {
  OS << "{ ";

  if (!isRemoved()) {
    for (HLLoop *Loop : LoopsVector) {
      OS << Loop->getNumber() << " ";
    }

    if (isBadNode()) {
      OS << BadNode->getNumber() << " ";
    }
  } else {
    OS << "R ";
  }

  OS << "}";

  if (isBadNode()) {
    OS << "B";
  }
}

unsigned FuseNode::getTopSortNumber() const {
  assert(!isRemoved() && "Node is removed");
  return getHLNode()->getTopSortNum();
}

void FuseNode::merge(const FuseNode &Node) {
  loops().append(Node.loops().begin(), Node.loops().end());
  HasUnsafeSideEffects = HasUnsafeSideEffects || Node.HasUnsafeSideEffects;
}

HLNode *FuseNode::getHLNode() const {
  assert(!isRemoved() && "Trying to get removed node");
  if (isBadNode()) {
    return BadNode;
  } else {
    return pilotLoop();
  }
}

LLVM_DUMP_METHOD
void FuseEdge::print(raw_ostream &OS) const {
  OS << " " << Weight;

  if (IsBadEdge) {
    OS << "B";
  }

  OS << "--> ";
}

void FuseEdge::merge(const FuseEdge &Edge) {
  Weight += Edge.Weight;
  IsBadEdge = IsBadEdge || Edge.IsBadEdge;
}

FuseEdge *FuseGraph::tryGetFuseEdge(unsigned Node1, unsigned Node2) {
  auto Iter = Edges.find(std::make_pair(Node1, Node2));

  if (Iter == Edges.end()) {
    Iter = Edges.find(std::make_pair(Node2, Node1));
    if (Iter == Edges.end()) {
      return nullptr;
    }
  }

  return &Iter->second;
}

unsigned FuseGraph::createFuseNode(GraphNodeMapTy &Map, HLNode *Node) {
  assert(Map.count(Node) == 0 && "Node already exist");
  unsigned &FuseNumber = Map[Node];

  HLLoop *Loop = dyn_cast<HLLoop>(Node);

  bool HasUnsafeSideEffects =
      Loop ? hasUnsafeSideEffects(HLS, Loop) : hasUnsafeSideEffects(HLS, Node);

  if (Loop && isGoodLoop(Loop)) {
    Vertex.emplace_back(Loop, HasUnsafeSideEffects);
  } else {
    Vertex.emplace_back(Node, HasUnsafeSideEffects);
  }

  FuseNumber = Vertex.size();

  return FuseNumber - 1;
}

unsigned FuseGraph::getFuseNode(GraphNodeMapTy &Map, HLNode *Node) {
  assert(Map.count(Node) != 0 && "Node doesn't exist");
  return Map[Node] - 1;
}

unsigned FuseGraph::areFusibleWithCommonTC(FusibleCacheTy &Cache,
                                           FuseNode &Node1, FuseNode &Node2) {
  auto Key = std::make_pair(&Node1, &Node2);
  auto Iter = Cache.find(Key);

  if (Iter == Cache.end()) {
    unsigned &CommonTC = Cache[Key]; // Default value is zero.

    bool MayBeFused =
        Node1.isGoodNode() && Node2.isGoodNode() &&
        (!Node1.hasUnsafeSideEffects() || !Node2.hasUnsafeSideEffects());

    if (MayBeFused) {
      CommonTC =
          areLoopsFusibleWithCommonTC(Node1.pilotLoop(), Node2.pilotLoop());
    }

    return CommonTC;
  }

  return Iter->second;
}

void FuseGraph::initGraphHelpers() {
  for (auto &Edge : Edges) {
    unsigned X = Edge.first.first;
    unsigned Y = Edge.first.second;

    if (Edge.second.IsUndirected) {
      Neighbors[X].insert(Y);
      Neighbors[Y].insert(X);
    } else {
      Successors[X].insert(Y);
      Predecessors[Y].insert(X);
    }
  }
}

void FuseGraph::initPathToInfo(NodeMapTy &LocalPathFrom,
                               NodeMapTy &LocalPathTo) {
  for (unsigned NodeV = 0, E = Vertex.size(); NodeV < E; ++NodeV) {
    for (unsigned NodeW : LocalPathFrom[NodeV]) {
      LocalPathTo[NodeW].insert(NodeV);
    }
  }
}

void FuseGraph::initPathInfo(FuseEdgeHeap &Heap) {
  // First initialize PathFrom structures. Also initialize Heap with edges.

  // Iterate in reverse topological order.
  for (unsigned I = 0, E = Vertex.size(); I < E; ++I) {
    unsigned NodeV = Vertex.size() - 1 - I;

    auto &PathFromV = PathFrom[NodeV];
    auto &BadPathFromV = BadPathFrom[NodeV];

    assert(PathFromV.empty());
    assert(BadPathFromV.empty());

    // Add identity path V -> V.
    PathFromV.insert(NodeV);

    if (Vertex[NodeV].isBadNode()) {
      // Add identity node to a bad path.
      BadPathFromV.insert(NodeV);
    }

    // Process every W successor of V. Append PathFrom[W] sets to PathFrom[V].
    for (unsigned NodeW : Successors[NodeV]) {
      auto &PathFromW = PathFrom[NodeW];
      auto &BadPathFromW = BadPathFrom[NodeW];

      PathFromV.insert(PathFromW.begin(), PathFromW.end());
      BadPathFromV.insert(BadPathFromW.begin(), BadPathFromW.end());

      FuseEdge &Edge = getFuseEdge(NodeV, NodeW);

      // If there is a bad node or edge consider the entire PathFrom[W] as bad.
      if (Edge.IsBadEdge || Vertex[NodeW].isBadNode()) {
        BadPathFromV.insert(PathFromW.begin(), PathFromW.end());
      }

      Heap.push(NodeV, NodeW, Edge.Weight);
    }

    // Process neighbors.
    for (unsigned NodeW : Neighbors[NodeV]) {
      if (PathFromV.count(NodeW)) {
        // Convert to a directed edge if there is a directed path.
        Neighbors[NodeV].erase(NodeW);
        Neighbors[NodeW].erase(NodeV);

        Successors[NodeV].insert(NodeW);
        Predecessors[NodeW].insert(NodeV);
      }

      Heap.push(NodeV, NodeW, getFuseEdge(NodeV, NodeW).Weight);
    }
  }

  // Invert PathFrom sets to produce the PathTo sets.
  initPathToInfo(PathFrom, PathTo);
  initPathToInfo(BadPathFrom, BadPathTo);
}

void FuseGraph::updateSlice(unsigned NodeX, NodeMapTy &LocalPathFrom,
                            NodeSetTy &NewPathSources, NodeSetTy &NewPathSinks,
                            NodeMapTy &Preds) {
  // - NodeX is the vertex being collapsed
  // - LocalPathFrom is the set that is being updated
  // - NewPathSources is the set of vertices that can reach NodeX
  //   but not the vertex it's being collapsed into
  // - NewPathSinks is the set of vertices reachable from the vertex being
  //   collapsed into but not from NodeX

  // Update LocalPathFrom sets backing up from NodeX in NewPathSources adding
  // vertices in NewPathSinks

  SmallSetVector<unsigned, 8> Worklist;
  SmallDenseSet<unsigned> WorklistEver;

  NodeMapTy NewPathFrom;
  NewPathFrom[NodeX] = NewPathSinks;

  Worklist.insert(NodeX);
  WorklistEver.insert(NodeX);

  // Traverse the subtree.
  while (!Worklist.empty()) {
    unsigned NodeW = Worklist.pop_back_val();

    // For each connected node.
    for (unsigned NodeY : Preds[NodeW]) {
      if (!NewPathSources.count(NodeY)) {
        continue;
      }

      if (!WorklistEver.count(NodeY)) {
        Worklist.insert(NodeY);
        WorklistEver.insert(NodeY);
      }

      auto &LocalPathFromNodeY = LocalPathFrom[NodeY];
      auto &NewPathFromNodeY = NewPathFrom[NodeY];

      for (unsigned NodeZ : NewPathFrom[NodeW]) {
        if (!LocalPathFromNodeY.count(NodeZ)) {
          LocalPathFromNodeY.insert(NodeZ);
          NewPathFromNodeY.insert(NodeZ);
        }
      }
    }
  }
}

void FuseGraph::updateBothWays(unsigned NodeV, unsigned NodeX,
                               NodeMapTy &LocalPathFrom,
                               NodeMapTy &LocalPathTo) {
  NodeSetTy NewPathSinks;
  NodeSetTy NewPathSources;

  // Collect nodes reachable from NodeV, but not from NodeX.
  NodeSetTy &LocalPathFromNodeX = LocalPathFrom[NodeX];
  for (unsigned Node : LocalPathFrom[NodeV]) {
    if (!LocalPathFromNodeX.count(Node)) {
      NewPathSinks.insert(Node);
    }
  }

  // Collect nodes that can reach NodeX, but cannot reach Node V.
  NodeSetTy &LocalPathToNodeV = LocalPathTo[NodeV];
  for (unsigned Node : LocalPathTo[NodeX]) {
    if (!LocalPathToNodeV.count(Node)) {
      NewPathSources.insert(Node);
    }
  }

  // Add V as a destination for each node in NewPathSources.
  LocalPathToNodeV.insert(NewPathSources.begin(), NewPathSources.end());
  for (unsigned Node : NewPathSources) {
    LocalPathFrom[Node].insert(NodeV);
  }

  // Add X as an origin for each node in NewPathSinks.
  LocalPathFromNodeX.insert(NewPathSinks.begin(), NewPathSinks.end());
  for (unsigned Node : NewPathSinks) {
    LocalPathTo[Node].insert(NodeX);
  }

  // Update PathFrom and PathTo sets.
  updateSlice(NodeX, LocalPathFrom, NewPathSources, NewPathSinks, Predecessors);
  updateSlice(NodeV, LocalPathTo, NewPathSinks, NewPathSources, Successors);
}

void FuseGraph::updatePathInfo(unsigned NodeV, unsigned NodeX) {
  if (!PathFrom[NodeV].count(NodeX)) {
    // Special case: undirected (V, X)
    updateBothWays(NodeX, NodeV, PathFrom, PathTo);
    updateBothWays(NodeX, NodeV, BadPathFrom, BadPathTo);
  }

  // Now update PathFrom for vertices that reach NodeX but not NodeV.
  updateBothWays(NodeV, NodeX, PathFrom, PathTo);
  updateBothWays(NodeV, NodeX, BadPathFrom, BadPathTo);
}

void FuseGraph::updateSuccessors(FuseEdgeHeap &Heap, unsigned NodeV,
                                 unsigned NodeX, const CollapseRangeTy &Range) {
  // V - the vertex into which merging is taking place.
  // X - the vertex currently being merged.

  // Update all successors of NodeX currently being merged.
  for (unsigned NodeY : Successors[NodeX]) {
    if (Range.count(NodeY)) {
      continue;
    }

    if (Successors[NodeV].count(NodeY)) {
      // Already a successor of NodeV, need to merge edges.
      auto &Edge = getFuseEdge(NodeV, NodeY);
      Edge.merge(getFuseEdge(NodeX, NodeY));

      Heap.reheapEdge(NodeV, NodeY, Edge.Weight);
      Heap.remove(NodeX, NodeY);
    } else if (Neighbors[NodeV].count(NodeY)) {
      // Already a neighbor of NodeV, need to replace by a directed edge.
      Successors[NodeV].insert(NodeY);
      Predecessors[NodeY].insert(NodeV);

      auto &Edge = getFuseEdge(NodeV, NodeY);
      Edge.merge(getFuseEdge(NodeX, NodeY));
      Edge.IsUndirected = false;

      Heap.reheapEdge(NodeV, NodeY, Edge.Weight);
      Heap.remove(NodeX, NodeY);

      Neighbors[NodeV].erase(NodeY);
      Neighbors[NodeY].erase(NodeV);
    } else {
      // No existing relationship, need to create a new edge.
      Successors[NodeV].insert(NodeY);
      Predecessors[NodeY].insert(NodeV);

      getOrCreateFuseEdge(NodeV, NodeY) = getFuseEdge(NodeX, NodeY);
      Heap.replaceEdge(NodeX, NodeY, NodeV, NodeY);
    }

    // Remove the node currently being merged from the predecessors of NodeY.
    Predecessors[NodeY].erase(NodeX);
  }
}

void FuseGraph::updatePredecessors(FuseEdgeHeap &Heap, unsigned NodeV,
                                   unsigned NodeX,
                                   const CollapseRangeTy &Range) {
  // V - the vertex into which merging is taking place.
  // X - the vertex currently being merged.

  // Update all predecessors of NodeX currently being merged.
  for (unsigned NodeY : Predecessors[NodeX]) {
    if (Range.count(NodeY)) {
      continue;
    }

    if (Predecessors[NodeV].count(NodeY)) {
      // Already a predecessor of NodeV, need to merge edges.
      auto &Edge = getFuseEdge(NodeY, NodeV);
      Edge.merge(getFuseEdge(NodeY, NodeX));

      Heap.reheapEdge(NodeY, NodeV, Edge.Weight);
      Heap.remove(NodeY, NodeX);
    } else if (Neighbors[NodeV].count(NodeY)) {
      // Already a neighbor of NodeV, need to replace by a directed edge.
      Predecessors[NodeV].insert(NodeY);
      Successors[NodeY].insert(NodeV);

      auto &Edge = getFuseEdge(NodeY, NodeV);
      Edge.merge(getFuseEdge(NodeY, NodeX));
      Edge.IsUndirected = false;

      Heap.reheapEdge(NodeY, NodeV, Edge.Weight);
      Heap.remove(NodeY, NodeX);

      Neighbors[NodeV].erase(NodeY);
      Neighbors[NodeY].erase(NodeV);
    } else {
      // No existing relationship, need to create a new edge.
      Predecessors[NodeV].insert(NodeY);
      Successors[NodeY].insert(NodeV);

      getOrCreateFuseEdge(NodeY, NodeV) = getFuseEdge(NodeY, NodeX);
      Heap.replaceEdge(NodeY, NodeX, NodeY, NodeV);
    }

    // Remove the node currently being merged from the successors of NodeY.
    Successors[NodeY].erase(NodeX);
  }
}

void FuseGraph::updateNeighbors(FuseEdgeHeap &Heap, unsigned NodeV,
                                unsigned NodeX, const CollapseRangeTy &Range) {
  // V - the vertex into which merging is taking place.
  // X - the vertex currently being merged.

  // Update all neighbors of NodeX currently being merged.
  for (unsigned NodeY : Neighbors[NodeX]) {
    if (Range.count(NodeY)) {
      continue;
    }

    if (PathFrom[NodeV].count(NodeY)) {
      // There is a directed path from V to Y. Make Y a successor of V.

      if (Successors[NodeV].count(NodeY)) {
        // Already a successor of NodeV, merge edges.
        auto &Edge = getFuseEdge(NodeV, NodeY);
        Edge.merge(getFuseEdge(NodeX, NodeY));

        Heap.reheapEdge(NodeV, NodeY, Edge.Weight);
        Heap.remove(NodeX, NodeY);
      } else {
        // No direct edge, create one.
        getOrCreateFuseEdge(NodeV, NodeY) = getFuseEdge(NodeX, NodeY);
        Heap.replaceEdge(NodeX, NodeY, NodeV, NodeY);

        Successors[NodeV].insert(NodeY);
      }
    } else if (PathFrom[NodeY].count(NodeV)) {
      // There is a directed path from Y to V. Make Y a predecessor of V.

      if (Predecessors[NodeV].count(NodeY)) {
        // Already a predecessor of NodeV, merge edges.
        auto &Edge = getFuseEdge(NodeY, NodeV);
        Edge.merge(getFuseEdge(NodeY, NodeX));

        Heap.reheapEdge(NodeY, NodeV, Edge.Weight);
        Heap.remove(NodeX, NodeY);
      } else {
        // No direct edge, create one.
        getOrCreateFuseEdge(NodeY, NodeV) = getFuseEdge(NodeY, NodeX);
        Heap.replaceEdge(NodeX, NodeY, NodeY, NodeV);

        Predecessors[NodeV].insert(NodeY);
      }
    } else if (Neighbors[NodeV].count(NodeY)) {
      // V and Y are already neighbors, merge edges.

      FuseEdge &Edge = getFuseEdge(NodeV, NodeY);
      Edge.merge(getFuseEdge(NodeX, NodeY));

      Heap.reheapEdge(NodeV, NodeY, Edge.Weight);
      Heap.remove(NodeX, NodeY);
    } else {
      // No existing relationship, make Y a neighbor of V.

      Neighbors[NodeV].insert(NodeY);
      Neighbors[NodeY].insert(NodeV);

      getOrCreateFuseEdge(NodeV, NodeY) = getFuseEdge(NodeX, NodeY);
      Heap.replaceEdge(NodeX, NodeY, NodeV, NodeY);
    }

    // Remove Y-X neighbor relationship.
    Neighbors[NodeY].erase(NodeX);
    Neighbors[NodeX].erase(NodeY);
  }
}

void FuseGraph::collapse(FuseEdgeHeap &Heap, unsigned NodeV,
                         const CollapseRangeTy &CollapseRange) {
  SmallVector<unsigned, 8> CollapseRangeVector(CollapseRange.begin(),
                                               CollapseRange.end());

  // Sort nodes from collapse range in the lexical order.
  std::sort(CollapseRangeVector.begin(), CollapseRangeVector.end());

  FuseNode &FNodeV = Vertex[NodeV];

#ifndef NDEBUG
  LLVM_DEBUG(dbgs() << "Collapsing: ");
  LLVM_DEBUG(dbgs() << NodeV << ":");
  LLVM_DEBUG(FNodeV.print(dbgs()));
  LLVM_DEBUG(dbgs() << " <-- ");
  for (unsigned NodeX : CollapseRangeVector) {
    if (NodeX == NodeV) {
      continue;
    }

    LLVM_DEBUG(dbgs() << NodeX << ":");
    LLVM_DEBUG(Vertex[NodeX].print(dbgs()));
    LLVM_DEBUG(dbgs() << " ");
  }
  LLVM_DEBUG(dbgs() << "\n");
#endif

  for (unsigned NodeX : CollapseRangeVector) {
    if (NodeX == NodeV) {
      continue;
    }

    FuseNode &NodeXRef = Vertex[NodeX];
    FNodeV.merge(NodeXRef);

    updatePathInfo(NodeV, NodeX);

    // O(E*V) each update
    updateSuccessors(Heap, NodeV, NodeX, CollapseRange);
    updatePredecessors(Heap, NodeV, NodeX, CollapseRange);
    updateNeighbors(Heap, NodeV, NodeX, CollapseRange);

    PathFrom.erase(NodeX);
    BadPathFrom.erase(NodeX);
    PathTo.erase(NodeX);
    BadPathTo.erase(NodeX);
    Predecessors.erase(NodeX);
    Successors.erase(NodeX);
    Neighbors.erase(NodeX);

    // PathFrom[NodeV].erase(NodeX);

    Successors[NodeV].erase(NodeX);
    Neighbors[NodeV].erase(NodeX);

    NodeXRef.remove();

    // DEBUG_FG(dbgs() << "!!!!DEBUG!!!!\n");
    // DEBUG_FG(dump());
  }
}

RefinedDependence FuseGraph::refineDependency(DDRef *Src, DDRef *Dst,
                                              unsigned CommonLevel,
                                              unsigned MaxLevel) const {
  auto RefinedDep = DDA.refineDV(Dst, Src, CommonLevel, MaxLevel, true);

  LLVM_DEBUG(dbgs() << "Forward dep: ");
  LLVM_DEBUG(DDA.refineDV(Src, Dst, CommonLevel, MaxLevel, true).dump());
  LLVM_DEBUG(dbgs() << "Backward dep: ");
  LLVM_DEBUG(RefinedDep.print(dbgs()));

  return RefinedDep;
}

bool FuseGraph::isLegalDependency(const DDEdge &Edge,
                                  unsigned CommonLevel) const {
  LLVM_DEBUG(Edge.dump());

  auto *SrcRef = Edge.getSrc();
  auto *DstRef = Edge.getSink();

  std::pair<unsigned, unsigned> MinMaxLevel =
      std::minmax(DstRef->getNodeLevel(), SrcRef->getNodeLevel());

  assert(CanonExprUtils::isValidLoopLevel(CommonLevel));

  // Special handle case when SrcRef or DstRef is in preheader or postexit.
  if (CommonLevel > MinMaxLevel.first) {
    // Condition means that at least one DDRef is outside of the loop.

    LLVM_DEBUG(dbgs() << "Preheader/postexit edge: ");

    // Possible edges before fusion are:
    //
    // 1: Preheader1
    // 2:  Loop1
    // 3: PostExit1
    //
    // 4: Preheader2
    // 5:  Loop2
    // 6: PostExit2
    //
    // 1 -> 4
    // 1 -> 5
    // 1 -> 6
    //
    // 2 -> 4 (!)
    // 2 -> 6
    //
    // 3 -> 4 (!)
    // 3 -> 5 (!)
    // 3 -> 6
    //
    // After the fusion:
    //
    // 1: Preheader1
    // 4: Preheader2
    // 2:  Loop1
    // 5:  Loop2
    // 3: PostExit1
    // 6: PostExit2
    //
    // Marked (!) edges are fusion preventive.
    //
    // Edges outgoing from preheader or incoming to postexit are all legal,
    // other edges are fusion preventive.

    HLInst *SrcInst = dyn_cast<HLInst>(SrcRef->getHLDDNode());
    HLInst *DstInst = dyn_cast<HLInst>(DstRef->getHLDDNode());

    return ((SrcInst && SrcInst->isInPreheader()) ||
            (DstInst && DstInst->isInPostexit()));
  }

  assert(CanonExprUtils::isValidLoopLevel(MinMaxLevel.second));

  auto RefinedDep =
      refineDependency(SrcRef, DstRef, CommonLevel, MinMaxLevel.second);

  if (RefinedDep.isIndependent()) {
    return true;
  }

  if (!RefinedDep.isRefined()) {
    return false;
  }

  for (unsigned Level = CommonLevel; Level <= MinMaxLevel.second; ++Level) {
    auto Dir = RefinedDep.getDV()[Level - 1];
    assert(Dir != DVKind::NONE);

    if ((Dir & DVKind::LT) != DVKind::NONE) {
      return false;
    }

    if ((Dir & DVKind::GT) != DVKind::NONE) {
      return true;
    }
  }

  return true;
}

void FuseGraph::constructFuseNodes(GraphNodeMapTy &GraphNodeMap,
                                   HLNodeRangeTy Children) {
  for (HLNode &Node : Children) {
    createFuseNode(GraphNodeMap, &Node);
  }
}

void FuseGraph::constructUnsafeSideEffectsChains() {
  constructUnsafeSideEffectsChainsOneWay(Vertex.begin(), Vertex.end());
  constructUnsafeSideEffectsChainsOneWay(Vertex.rbegin(), Vertex.rend());
}

template <typename Iter>
void FuseGraph::constructUnsafeSideEffectsChainsOneWay(Iter Begin, Iter End) {
  // Set to one past end element.
  auto FirstUMANode = std::find_if(Begin, End, [](const FuseNode &Node) {
    return Node.hasUnsafeSideEffects();
  });

  // Connect UMANode to every node after it but before next UMA node.
  for (auto NodeI = FirstUMANode; NodeI < End;) {
    auto NodeJ = std::next(NodeI);

    for (; NodeJ < End; ++NodeJ) {
      auto Src = std::distance(Vertex.begin(), &*NodeI);
      auto Dst = std::distance(Vertex.begin(), &*NodeJ);
      if (Src > Dst) {
        // Always create forward edges.
        std::swap(Src, Dst);
      }

      // This will create fake dependency to prevent reordering.
      getOrCreateFuseEdge(Src, Dst).IsBadEdge = true;

      if (NodeJ->hasUnsafeSideEffects()) {
        break;
      }
    }

    NodeI = NodeJ;
  }
}

void FuseGraph::constructDirectedEdges(
    DDGraph DDG, GraphNodeMapTy &GraphNodeMap, FusibleCacheTy &FusibleCache,
    HLNode *ParentNode, HLNodeRangeTy Children,
    SmallVectorImpl<RefNodePairTy> &RValNodePairs) {
  if (Children.begin() == Children.end()) {
    return;
  }

  unsigned MaxTopSort = std::prev(Children.end())->getMaxTopSortNum();

  HLLoop *ParentLoop;
  if (isa<HLRegion>(ParentNode)) {
    ParentLoop = nullptr;
  } else if (isa<HLLoop>(ParentNode)) {
    ParentLoop = cast<HLLoop>(ParentNode);
  } else {
    ParentLoop = ParentNode->getParentLoop();
  }

  // Fusion Level
  unsigned Level = ParentLoop ? ParentLoop->getNestingLevel() + 1 : 1;

  for (HLNode &Child : Children) {
    Gatherer::VectorTy Refs;
    Gatherer::gather(&Child, Refs);

    HLNode *SrcNode = &Child;
    unsigned SrcNumber = getFuseNode(GraphNodeMap, SrcNode);

    for (DDRef *Ref : Refs) {
      // Collect Directed Dependency Edges
      for (const DDEdge *DDEdge : DDG.outgoing(Ref)) {
        if (!DDEdge->isForwardDep()) {
          continue;
        }

        HLNode *DstNodeExact = DDEdge->getSink()->getHLDDNode();

        unsigned DstNodeExactTopsort = DstNodeExact->getTopSortNum();

        // Filter edges outside of Children range. SrcNode is from the range
        // already and we look at forward edges only.
        assert(DstNodeExactTopsort >= Children.begin()->getMinTopSortNum());
        if (DstNodeExactTopsort > MaxTopSort) {
          continue;
        }

        HLNode *DstNode = HLNodeUtils::getImmediateChildContainingNode(
            ParentNode, DstNodeExact);
        if (DstNode == SrcNode) {
          continue;
        }

        unsigned DstNumber = getFuseNode(GraphNodeMap, DstNode);

        FuseEdge &FEdge = getOrCreateFuseEdge(SrcNumber, DstNumber);
        if (!FEdge.IsBadEdge) {
          FuseNode &SrcFuseNode = Vertex[SrcNumber];
          FuseNode &DstFuseNode = Vertex[DstNumber];

          bool Legal = false;
          unsigned CommonTC =
              areFusibleWithCommonTC(FusibleCache, SrcFuseNode, DstFuseNode);

          LLVM_DEBUG(SrcFuseNode.print(dbgs()));
          LLVM_DEBUG(dbgs() << " --> ");
          LLVM_DEBUG(DstFuseNode.print(dbgs()));

          if (CommonTC) {
            LLVM_DEBUG(dbgs() << "\n");
            Legal = isLegalDependency(*DDEdge, Level);
            LLVM_DEBUG(dbgs() << " < " << (Legal ? "OK" : "illegal") << " >\n");
          } else {
            LLVM_DEBUG(dbgs() << " may not be fused.\n");
          }

          if (CommonTC && Legal) {
            FEdge.Weight += CommonTC;
          } else {
            FEdge.IsBadEdge = true;
          }
        }
      }

      // Collect RVals for undirected (input data dependency) edges
      RegDDRef *RDDRef = dyn_cast<RegDDRef>(Ref);
      if (RDDRef && RDDRef->isRval() && RDDRef->isMemRef()) {
        RValNodePairs.emplace_back(RDDRef, SrcNode);
      }
    }
  }
}

void FuseGraph::constructUndirectedEdges(
    GraphNodeMapTy &GraphNodeMap, FusibleCacheTy &FusibleCache,
    SmallVectorImpl<RefNodePairTy> &RValNodePairs) {
  typedef DDRefGrouping::RefGroupVecTy<RefNodePairTy> RValGroupTy;

  // Group RVal references by the equality
  RValGroupTy Groups;
  // TODO: use LocalityAnalysis
  DDRefGrouping::groupVec(
      Groups, RValNodePairs,
      [](const RefNodePairTy &Pair1, const RefNodePairTy &Pair2) {
        return DDRefUtils::areEqual(Pair1.first, Pair2.first);
      });

  // For each group of similar references
  for (auto &Group : Groups) {
    // Group them by the parent node
    RValGroupTy Groups2;
    DDRefGrouping::groupVec(
        Groups2, Group,
        [](const RefNodePairTy &Pair1, const RefNodePairTy &Pair2) {
          return Pair1.second == Pair2.second;
        });

    // Create edges between every pair of parent nodes
    for (int I = 0, E = Groups2.size(); I < E; ++I) {
      for (int J = I + 1; J < E; ++J) {
        unsigned FuseNumberA =
            getFuseNode(GraphNodeMap, Groups2[I].front().second);
        unsigned FuseNumberB =
            getFuseNode(GraphNodeMap, Groups2[J].front().second);

        unsigned CommonTC = areFusibleWithCommonTC(
            FusibleCache, Vertex[FuseNumberA], Vertex[FuseNumberB]);

        if (CommonTC) {
          FuseEdge *ExistingEdge = tryGetFuseEdge(FuseNumberA, FuseNumberB);

          if (!ExistingEdge) {
            // Create new undirected edge
            ExistingEdge = &getOrCreateFuseEdge(FuseNumberA, FuseNumberB);
            ExistingEdge->IsUndirected = true;
          }

          ExistingEdge->Weight += CommonTC;
        }
      }
    }
  }
}

void FuseGraph::constructNaiveEdges(GraphNodeMapTy &GraphNodeMap,
                                    FusibleCacheTy &FusibleCache) {
  for (int I = 0, E = Vertex.size(); I < E; ++I) {
    for (int J = I + 1; J < E; ++J) {
      unsigned CommonTC =
          areFusibleWithCommonTC(FusibleCache, Vertex[I], Vertex[J]);

      if (CommonTC) {
        FuseEdge *ExistingEdge = tryGetFuseEdge(I, J);

        if (!ExistingEdge) {
          // Create new undirected edge
          ExistingEdge = &getOrCreateFuseEdge(I, J);
          ExistingEdge->IsUndirected = true;
        }

        ExistingEdge->Weight += CommonTC;
      }
    }
  }
}

void FuseGraph::weightedFusion() {
  FuseEdgeHeap Heap;
  initPathInfo(Heap);

  LLVM_DEBUG(dbgs() << "\nFuse Graph before optimization\n");
  LLVM_DEBUG(dump());

  LLVM_DEBUG(dbgs() << "Processing edges from heap:\n");

  while (!Heap.empty()) {
    auto &MaxEntity = Heap.top();

    unsigned NodeV = MaxEntity.Src;
    unsigned NodeW = MaxEntity.Dst;

    Heap.pop();

    if (MaxEntity.Weight == 0) {
      // Skip fusion of non-beneficial edges.
      continue;
    }

    if (PathFrom[NodeW].count(NodeV)) {
      std::swap(NodeV, NodeW);
    }

    if (BadPathFrom[NodeV].count(NodeW)) {
      // Nodes are not connected anymore or there is a bad path between them.
      continue;
    }

    SmallSetVector<unsigned, 8> Worklist;
    SmallDenseSet<unsigned> WorklistEver;

    CollapseRangeTy CollapseRange;

    for (unsigned NodeX : Successors[NodeV]) {
      if (PathFrom[NodeX].count(NodeW)) {
        Worklist.insert(NodeX);
      }
    }

    if (Worklist.empty()) {
      if (!Neighbors[NodeV].count(NodeW)) {
        continue;
      }

      Worklist.insert(NodeW); // (NodeV, NodeW) undirected
    }

    WorklistEver.insert(Worklist.begin(), Worklist.end());

    while (!Worklist.empty()) {
      unsigned NodeX = Worklist.pop_back_val();

      CollapseRange.insert(NodeX);

      if (NodeX != NodeW) {
        for (unsigned NodeY : Successors[NodeX]) {
          if (PathFrom[NodeY].count(NodeW) && !WorklistEver.count(NodeY)) {
            Worklist.insert(NodeY);
            WorklistEver.insert(NodeY);
          }
        }
      }
    }

    CollapseRange.insert(NodeV);
    collapse(Heap, NodeV, CollapseRange);

    DEBUG_FG(dbgs() << "Fuse graph debug:\n");
    DEBUG_FG(dump());
  }
}

FuseGraph::FuseGraph(HIRDDAnalysis &DDA, HIRLoopStatistics &HLS, DDGraph DDG,
                     HLNode *ParentNode, HLNodeRangeTy Children)
    : DDA(DDA), HLS(HLS) {
  LLVM_DEBUG(dbgs() << "Fusion Graph initialization\n");
  LLVM_DEBUG(dbgs() << "DDG for the node <" << ParentNode->getNumber()
                    << ">:\n");
  LLVM_DEBUG(DDG.dump());
  LLVM_DEBUG(dbgs() << "\n");

  GraphNodeMapTy GraphNodeMap;
  FusibleCacheTy FusibleCache;
  SmallVector<RefNodePairTy, 8> RValNodePairs;

  constructFuseNodes(GraphNodeMap, Children);

  constructUnsafeSideEffectsChains();

  constructDirectedEdges(DDG, GraphNodeMap, FusibleCache, ParentNode, Children,
                         RValNodePairs);

  constructUndirectedEdges(GraphNodeMap, FusibleCache, RValNodePairs);

  if (ConstructNaiveEdges) {
    constructNaiveEdges(GraphNodeMap, FusibleCache);
  }

  initGraphHelpers();

  weightedFusion();
}

FuseGraph FuseGraph::create(HIRDDAnalysis &DDA, HIRLoopStatistics &HLS,
                            HLNode *ParentNode, HLNodeRangeTy Range) {
  assert(Range.begin()->getParent() == ParentNode &&
         "Nodes in Range should be children of ParentNode");

  DDGraph DDG;
  if (HLRegion *Region = dyn_cast<HLRegion>(ParentNode)) {
    DDG = DDA.getGraph(Region);
  } else if (HLLoop *Loop = dyn_cast<HLLoop>(ParentNode)) {
    DDG = DDA.getGraph(Loop);
  } else {
    HLLoop *ParentLoop = ParentNode->getParentLoop();
    if (ParentLoop) {
      DDG = DDA.getGraph(ParentLoop);
    } else {
      DDG = DDA.getGraph(ParentNode->getParentRegion());
    }
  }

  return FuseGraph(DDA, HLS, DDG, ParentNode, Range);
}

void FuseGraph::dumpNodeSet(const NodeMapTy &Container) const {
  for (unsigned NodeV = 0, E = Vertex.size(); NodeV < E; ++NodeV) {
    auto Iter = Container.find(NodeV);
    if (Iter == Container.end()) {
      continue;
    }

    for (unsigned NodeX : Iter->second) {
      dbgs() << NodeV << ":";
      Vertex[NodeV].print(dbgs());
      getFuseEdge(NodeV, NodeX).print(dbgs());
      dbgs() << NodeX << ":";
      Vertex[NodeX].print(dbgs());
      dbgs() << "\n";
    }
  }
}

void FuseGraph::dumpNodeRawMap(const NodeMapTy &Map) const {
  for (unsigned NodeV = 0, E = Vertex.size(); NodeV < E; ++NodeV) {
    auto Iter = Map.find(NodeV);
    if (Iter == Map.end()) {
      continue;
    }

    dbgs() << NodeV << ": ";

    for (unsigned NodeX : Iter->second) {
      dbgs() << NodeX << " ";
    }

    dbgs() << "\n";
  }
}

LLVM_DUMP_METHOD
void FuseGraph::dump() const {
  DEBUG_FG(dbgs() << "Nodes:\n");
  for (int I = 0, E = Vertex.size(); I < E; ++I) {
    DEBUG_FG(dbgs() << I << ":");
    DEBUG_FG(Vertex[I].print(dbgs()));
    DEBUG_FG(dbgs() << " ");
  }
  DEBUG_FG(dbgs() << "\n");

  dbgs() << "Undirected:\n";
  dumpNodeSet(Neighbors);

  DEBUG_FG(dbgs() << "Sucessors ");
  dbgs() << "Directed:\n";
  dumpNodeSet(Successors);

  DEBUG_FG(dbgs() << "Predecessors Directed:\n");
  DEBUG_FG(dumpNodeSet(Predecessors));

  DEBUG_FG(dbgs() << "PathTo:\n");
  DEBUG_FG(dumpNodeRawMap(PathTo));

  DEBUG_FG(dbgs() << "PathFrom:\n");
  DEBUG_FG(dumpNodeRawMap(PathFrom));

  DEBUG_FG(dbgs() << "BadPathTo:\n");
  DEBUG_FG(dumpNodeRawMap(BadPathTo));

  DEBUG_FG(dbgs() << "BadPathFrom:\n");
  DEBUG_FG(dumpNodeRawMap(BadPathFrom));

  dbgs() << "\n";
}

void FuseGraph::topologicalSort(
    SmallVectorImpl<const FuseNode *> &SortedFuseNodes) const {
  // Do the DFS of predecessors. No need to reverse SortedFuseNodes at the end.

  // Number of nodes in FuseGraph
  unsigned NumNodes = Vertex.size();

  SortedFuseNodes.reserve(NumNodes);
  BitVector Visited(NumNodes);

  SmallVector<unsigned, 8> WorkList;
  WorkList.reserve(NumNodes);

  for (unsigned NodeIdx = 0; NodeIdx < NumNodes; NodeIdx++) {
    if (Visited[NodeIdx] || Vertex[NodeIdx].isRemoved()) {
      continue;
    }

    WorkList.push_back(NodeIdx);

    while (!WorkList.empty()) {
      unsigned CurNodeIdx = WorkList.back();
      bool UnvisitedPredecessors = false;

      auto PredNodes = Predecessors.find(CurNodeIdx);
      if (PredNodes != Predecessors.end()) {
        // Sort predecessors to make the final sequence stable - original order
        // preserved if no reordering is required.
        SmallVector<unsigned, 8> SortedPredNodes(PredNodes->second.begin(),
                                                 PredNodes->second.end());

        // Use std::greater<unsigned>() because we push nodes to the back and
        // nodes from the back would be handled first.
        std::sort(SortedPredNodes.begin(), SortedPredNodes.end(),
                  std::greater<unsigned>());

        for (auto Pred : SortedPredNodes) {
          if (!Visited[Pred]) {
            WorkList.push_back(Pred);
            UnvisitedPredecessors = true;
          }
        }
      }

      if (!UnvisitedPredecessors) {
        WorkList.pop_back();

        assert(!Vertex[CurNodeIdx].isRemoved() && "Adding removed node");
        SortedFuseNodes.push_back(&Vertex[CurNodeIdx]);

        assert(!Visited[CurNodeIdx] && "Found already visited node");
        Visited[CurNodeIdx] = true;
      }
    }
  }
}
