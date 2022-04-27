//===- HIRLoopFusionGraph.cpp - Implements Loop Fusion Graph --------------===//
//
// Copyright (C) 2017-2020 Intel Corporation. All rights reserved.
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

#include "llvm/Support/CommandLine.h"

#define DEBUG_TYPE "hir-loop-fusion"
#define DEBUG_FG(X) DEBUG_WITH_TYPE(DEBUG_TYPE "-fg", X)

static cl::opt<bool>
    ConstructNaiveEdges(DEBUG_TYPE "-naive-edges", cl::init(false), cl::Hidden,
                        cl::desc("Construct profitable edges just because "
                                 "loops are having common trip count"));

static cl::opt<int> NumCases(DEBUG_TYPE "-num-cases", cl::init(-1), cl::Hidden,
                             cl::desc("Fuse only first N number of edges."));

static cl::opt<bool>
    SkipVecProfitabilityCheck(DEBUG_TYPE "-skip-vec-prof-check",
                              cl::init(false), cl::Hidden,
                              cl::desc("Skip vectorization profitability check "
                                       "during fusion edges construction."));

using namespace llvm;
using namespace llvm::loopopt;
using namespace llvm::loopopt::fusion;

typedef DDRefGatherer<DDRef, AllRefs ^ (ConstantRefs | GenericRValRefs |
                                        IsAddressOfRefs)>
    Gatherer;

bool fusion::isGoodLoop(const HLLoop *Loop) {
  return !(Loop->isDistributedForMemRec() || Loop->hasUnrollEnablingPragma() ||
           Loop->hasVectorizeEnablingPragma() ||
           Loop->hasFusionDisablingPragma() || Loop->isSIMD());
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

  // Returns neighbor edge between NodeA and NodeB as it may be stored in the
  // heap.
  static std::pair<unsigned, unsigned> makeNeighborEdge(unsigned NodeA,
                                                        unsigned NodeB) {
    if (NodeB < NodeA) {
      return {NodeB, NodeA};
    }

    return {NodeA, NodeB};
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
    assert(!tryGetEntity(Src, Dst) && "Edge already exist");

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
  template <bool IsNeighbor> void remove(unsigned Src, unsigned Dst) {
    if (IsNeighbor) {
      std::tie(Src, Dst) = makeNeighborEdge(Src, Dst);
    }
    auto *Entry = tryGetEntity(Src, Dst);
    if (!Entry) {
      return;
    }

    Entry->IsRemoved = true;
    Map.erase(std::make_pair(Src, Dst));
  }

  // O(log(n))
  template <bool IsNeighbor, bool IsNewNeighbor>
  void reheapEdge(unsigned Src, unsigned Dst, int NewWeight) {
    unsigned NodeA = Src;
    unsigned NodeB = Dst;

    if (IsNeighbor) {
      std::tie(NodeA, NodeB) = makeNeighborEdge(Src, Dst);
    }

    if (!tryGetEntity(Src, Dst)) {
      // Edge may be already handled and removed from the heap.
      return;
    }

    remove<false>(NodeA, NodeB);

    if (IsNewNeighbor) {
      std::tie(Src, Dst) = makeNeighborEdge(Src, Dst);
    }

    push(Src, Dst, NewWeight);
  }

  // O(1)
  template <bool IsNeighbor, bool IsNewNeighbor>
  void replaceEdge(unsigned Src, unsigned Dst, unsigned NewSrc,
                   unsigned NewDst) {
    if (IsNeighbor) {
      std::tie(Src, Dst) = makeNeighborEdge(Src, Dst);
    }

    auto *Entity = tryGetEntity(Src, Dst);
    if (!Entity) {
      // Edge may be already handled and removed from the heap.
      return;
    }

    if (IsNewNeighbor) {
      std::tie(NewSrc, NewDst) = makeNeighborEdge(NewSrc, NewDst);
    }

    Entity->Src = NewSrc;
    Entity->Dst = NewDst;

    Map.erase(std::make_pair(Src, Dst));
    Map.try_emplace(std::make_pair(NewSrc, NewDst), Entity);
  }
};

static bool canHandleZtt(const HLLoop *Loop1, const HLLoop *Loop2,
                         int TCDifference) {
  // Return true right away if ZTTs are equal in both loops.
  if (HLNodeUtils::areEqualZttConditions(Loop1, Loop2)) {
    return true;
  }

  if (!Loop1->hasZtt() || !Loop2->hasZtt() ||
      Loop1->getNumZttPredicates() != 1 || Loop2->getNumZttPredicates() != 1) {
    return false;
  }

  // Reaching here means both loops have different Ztts, and since pre-
  // header/postexits are guarded by ztts, we conservatively bail out
  // if preheader/postexit will need to move after fusion. Fusion will
  // cause the fused loop to use the more restrictive Ztt, and the Peel
  // loop will use the less restrictive Ztt. Using TCDifference, we can
  // allow preheader for the Loop with smaller TC, and postexit for loop
  // with greater TC. Handing other cases would require detatching
  // pre/post insts to correctly retain ztt information.

  // Loop1 TC > Loop2 TC
  if (TCDifference > 0) {
    if (Loop1->hasPreheader() || Loop2->hasPostexit()) {
      return false;
    }
  } else if (TCDifference < 0) {
    if (Loop2->hasPreheader() || Loop1->hasPostexit()) {
      return false;
    }
  }

  auto PredI1 = Loop1->ztt_pred_begin();
  auto PredI2 = Loop2->ztt_pred_begin();

  if (*PredI1 != *PredI2) {
    return false;
  }

  auto GetSingleCEOrNull = [](const RegDDRef *Ref) -> const CanonExpr * {
    if (!Ref->isTerminalRef()) {
      return nullptr;
    }
    return Ref->getSingleCanonExpr();
  };

  auto *Loop1LHS =
      GetSingleCEOrNull(Loop1->getLHSZttPredicateOperandDDRef(PredI1));
  auto *Loop1RHS =
      GetSingleCEOrNull(Loop1->getRHSZttPredicateOperandDDRef(PredI1));
  auto *Loop2LHS =
      GetSingleCEOrNull(Loop2->getLHSZttPredicateOperandDDRef(PredI2));
  auto *Loop2RHS =
      GetSingleCEOrNull(Loop2->getRHSZttPredicateOperandDDRef(PredI2));

  if (!Loop1LHS || !Loop1RHS || !Loop2LHS || !Loop2RHS) {
    return false;
  }

  int64_t DistA, DistB;
  if (!CanonExprUtils::getConstDistance(Loop1LHS, Loop2LHS, &DistA) ||
      !CanonExprUtils::getConstDistance(Loop1RHS, Loop2RHS, &DistB)) {
    return false;
  }

  return std::abs(DistA) + std::abs(DistB) == std::abs(TCDifference);
}

static unsigned areLoopsFusibleWithCommonTC(const HLLoop *Loop1,
                                            const HLLoop *Loop2) {
  if (!Loop1->isDo() || !Loop1->isNormalized() || !Loop2->isDo() ||
      !Loop2->isNormalized()) {
    return 0;
  }

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

  // TODO: Allow only loops with special ZTTs for now. This needs to be improved.
  if (!canHandleZtt(Loop1, Loop2, UBDist)) {
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

  if (isVectorizable()) {
    OS << "V";
  }
}

unsigned FuseNode::getTopSortNumber() const {
  assert(!isRemoved() && "Node is removed");
  return getHLNode()->getTopSortNum();
}

void FuseNode::merge(const FuseNode &Node) {
  LoopsVector.append(Node.loops().begin(), Node.loops().end());
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

  if (Loop) {
    auto &FNode = Vertex.back();
    if (Loop->hasVectorizeEnablingPragma()) {
      FNode.setVectorizable(true);
    } else if (Loop->hasVectorizeDisablingPragma()) {
      FNode.setVectorizable(false);
    } else {
      FNode.setVectorizable(Loop->isInnermost());
    }
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
      addNeighborEdgeInternal(X, Y);
    } else {
      addDirectedEdgeInternal(X, Y);
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

void FuseGraph::excludePathPreventingVectorization(unsigned NodeV,
                                                    unsigned NodeW) {
  // Skip fusion if one loop is vectorizable and another is not.
  bool NodeVVectorizable = Vertex[NodeV].isVectorizable();
  bool NodeWVectorizable = Vertex[NodeW].isVectorizable();
  if (NodeVVectorizable != NodeWVectorizable) {
    auto &BadPathFromV = BadPathFrom[NodeV];
    auto &PathFromW = PathFrom[NodeW];
    BadPathFromV.insert(PathFromW.begin(), PathFromW.end());
  }
  return;
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

      // Skip fusion if one loop is vectorizable and another is not.
      excludePathPreventingVectorization(NodeV, NodeW);

      Heap.push(NodeV, NodeW, Edge.Weight);
    }

    // Process neighbors.
    for (unsigned NodeW : Neighbors[NodeV]) {
      unsigned Src = NodeV;
      unsigned Dst = NodeW;

      if (PathFromV.count(NodeW)) {
        // Convert to a directed edge if there is a directed path. Edge
        // already in a heap.
        eraseNeighborEdgeInternal(NodeV, NodeW);
        addDirectedEdgeInternal(NodeV, NodeW);
        continue;
      }

      if (Dst < Src) {
        // Skip reversed neighbor edges as their forward counterparts are
        // already added to a heap.
        continue;
      }

      // Skip fusion if one loop is vectorizable and another is not.
      excludePathPreventingVectorization(NodeV, NodeW);

      Heap.push(Src, Dst, getFuseEdge(Src, Dst).Weight);
    }
  }

  // Invert PathFrom sets to produce the PathTo sets.
  initPathToInfo(PathFrom, PathTo);
  initPathToInfo(BadPathFrom, BadPathTo);
}

void FuseGraph::updateSlice(unsigned NodeX, NodeMapTy &LocalPathFrom,
                            const NodeMapTy &LocalPathTo,
                            const NodeSetTy &NewPathSources,
                            const NodeSetTy &NewPathSinks) {
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
    auto PredWI = LocalPathTo.find(NodeW);
    if (PredWI == LocalPathTo.end()) {
      continue;
    }

    for (unsigned NodeY : PredWI->second) {
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
  // Note (*) mark in this function below.
  // updateSlice() is merely used to optimally update LocalPathFrom and
  // LocalPathTo sets, however the marked lines does the same but in O(V^2).
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
    // (*) LocalPathFrom[Node].insert(NewPathSinks.begin(), NewPathSinks.end());
  }

  // Add X as an origin for each node in NewPathSinks.
  LocalPathFromNodeX.insert(NewPathSinks.begin(), NewPathSinks.end());
  for (unsigned Node : NewPathSinks) {
    LocalPathTo[Node].insert(NodeX);
    // (*) LocalPathTo[Node].insert(NewPathSources.begin(),
    // NewPathSources.end());
  }

  // Update PathFrom and PathTo sets.
  updateSlice(NodeX, LocalPathFrom, LocalPathTo, NewPathSources, NewPathSinks);
  updateSlice(NodeV, LocalPathTo, LocalPathFrom, NewPathSinks, NewPathSources);
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

      Heap.reheapEdge<false, false>(NodeV, NodeY, Edge.Weight);
      Heap.remove<false>(NodeX, NodeY);
    } else if (Neighbors[NodeV].count(NodeY)) {
      // Already a neighbor of NodeV, need to replace by a directed edge.
      addDirectedEdgeInternal(NodeV, NodeY);

      auto &Edge = getFuseEdge(NodeV, NodeY);
      Edge.merge(getFuseEdge(NodeX, NodeY));
      Edge.IsUndirected = false;

      Heap.reheapEdge<true, false>(NodeV, NodeY, Edge.Weight);
      Heap.remove<false>(NodeX, NodeY);

      eraseNeighborEdgeInternal(NodeV, NodeY);
    } else {
      // No existing relationship, need to create a new edge.
      addDirectedEdgeInternal(NodeV, NodeY);

      // Note: must create new edge before getting the existing edge otherwise
      // we may invalidate existing edges.
      auto &FEdge = getOrCreateFuseEdge(NodeV, NodeY);
      FEdge = getFuseEdge(NodeX, NodeY);
      Heap.replaceEdge<false, false>(NodeX, NodeY, NodeV, NodeY);
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

      Heap.reheapEdge<false, false>(NodeY, NodeV, Edge.Weight);
      Heap.remove<false>(NodeY, NodeX);
    } else if (Neighbors[NodeV].count(NodeY)) {
      // Already a neighbor of NodeV, need to replace by a directed edge.
      addDirectedEdgeInternal(NodeY, NodeV);

      auto &Edge = getFuseEdge(NodeY, NodeV);
      Edge.merge(getFuseEdge(NodeY, NodeX));
      Edge.IsUndirected = false;

      Heap.reheapEdge<true, false>(NodeY, NodeV, Edge.Weight);
      Heap.remove<false>(NodeY, NodeX);

      eraseNeighborEdgeInternal(NodeV, NodeY);
    } else {
      // No existing relationship, need to create a new edge.
      addDirectedEdgeInternal(NodeY, NodeV);

      // Note on this call in updateSuccessors()
      auto &FEdge = getOrCreateFuseEdge(NodeY, NodeV);
      FEdge = getFuseEdge(NodeY, NodeX);
      Heap.replaceEdge<false, false>(NodeY, NodeX, NodeY, NodeV);
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

        Heap.reheapEdge<false, false>(NodeV, NodeY, Edge.Weight);
        Heap.remove<true>(NodeX, NodeY);
      } else {
        // No direct edge, create one.
        // Note on this call in updateSuccessors()
        auto &FEdge = getOrCreateFuseEdge(NodeV, NodeY);
        FEdge = getFuseEdge(NodeX, NodeY);
        Heap.replaceEdge<true, false>(NodeX, NodeY, NodeV, NodeY);

        addDirectedEdgeInternal(NodeV, NodeY);
      }
    } else if (PathFrom[NodeY].count(NodeV)) {
      // There is a directed path from Y to V. Make Y a predecessor of V.

      if (Predecessors[NodeV].count(NodeY)) {
        // Already a predecessor of NodeV, merge edges.
        auto &Edge = getFuseEdge(NodeY, NodeV);
        Edge.merge(getFuseEdge(NodeY, NodeX));

        Heap.reheapEdge<false, false>(NodeY, NodeV, Edge.Weight);
        Heap.remove<true>(NodeX, NodeY);
      } else {
        // No direct edge, create one.
        // Note on this call in updateSuccessors()
        auto &FEdge = getOrCreateFuseEdge(NodeY, NodeV);
        FEdge = getFuseEdge(NodeY, NodeX);
        Heap.replaceEdge<true, false>(NodeX, NodeY, NodeY, NodeV);

        addDirectedEdgeInternal(NodeY, NodeV);
      }
    } else if (Neighbors[NodeV].count(NodeY)) {
      // V and Y are already neighbors, merge edges.

      FuseEdge &Edge = getFuseEdge(NodeV, NodeY);
      Edge.merge(getFuseEdge(NodeX, NodeY));

      Heap.reheapEdge<true, true>(NodeV, NodeY, Edge.Weight);
      Heap.remove<true>(NodeX, NodeY);
    } else {
      // No existing relationship, make Y a neighbor of V.

      addNeighborEdgeInternal(NodeV, NodeY);

      // Note on this call in updateSuccessors()
      auto &FEdge = getOrCreateFuseEdge(NodeV, NodeY);
      FEdge = getFuseEdge(NodeX, NodeY);
      Heap.replaceEdge<true, true>(NodeX, NodeY, NodeV, NodeY);
    }

    // Remove Y-X neighbor relationship.
    eraseNeighborEdgeInternal(NodeY, NodeX);
  }
}

void FuseGraph::collapse(FuseEdgeHeap &Heap, unsigned NodeV,
                         const CollapseRangeTy &CollapseRange) {
  FuseNode &FNodeV = Vertex[NodeV];

#ifndef NDEBUG
  LLVM_DEBUG(dbgs() << "Collapsing: ");
  LLVM_DEBUG(dbgs() << NodeV << ":");
  LLVM_DEBUG(FNodeV.print(dbgs()));
  LLVM_DEBUG(dbgs() << " <-- ");
  for (unsigned NodeX : CollapseRange) {
    if (NodeX == NodeV) {
      continue;
    }

    LLVM_DEBUG(dbgs() << NodeX << ":");
    LLVM_DEBUG(Vertex[NodeX].print(dbgs()));
    LLVM_DEBUG(dbgs() << " ");
  }
  LLVM_DEBUG(dbgs() << "\n");
#endif

  for (unsigned NodeX : CollapseRange) {
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

// Current phase ordering is
//   1) Pre-vec complete unroll
//   2) Collapse
//   3) Fusion
// The i3 loop could be vectorized without fusion.
// In some cases, the performance is better if we unroll the i3 loop.
// We can have i2 vectorized or as a non-vector loop where loop carried scalar
// replacement helps.
//
// The fix here is to avoid fusion when the loop nests are different.
// i.e. Avoid fusing i1-i2 loop, otherwuse i2 ends up as non-innermost.
//
// zperf results indicate there is no need to check for large loop body.
// Current code checks for i3 loop with small constant trip count.
// TODO: Could be tuned to do more later.
bool hasDifferentLoopNests(DDRef *SrcRef, DDRef *DstRef) {
  auto SrcLp = SrcRef->getParentLoop();
  auto DstLp = DstRef->getParentLoop();

  // Case 1
  // do i1
  //   do i2
  //     (large loop body here)
  //     SrcRef
  //   enddo
  // enddo
  //
  // do i1
  //   do i2
  //     DstRef
  //     do i3 =1,8
  //     enddo
  //   enddo
  // enddo
  if (SrcLp->isInnermost() && !DstLp->isInnermost()) {
    for (auto NodeIt = DstLp->child_begin(), E = DstLp->child_end();
         NodeIt != E; ++NodeIt) {
      auto *Loop = dyn_cast<HLLoop>(NodeIt);
      if (Loop && Loop->isInnermost()) {
        uint64_t TC;
        if (Loop->isConstTripLoop(&TC) && TC <= 8) {
          LLVM_DEBUG(dbgs() << "\nBail out for different loop nests, case 1 \n");
          return true;
        }
      }
    }
  }

  // Case 2
  // do i1
  //   do i2
  //     (large Loop body here)
  //     SrcRef
  //   enddo
  // enddo
  //
  // do i1
  //   do i2
  //     do i3 =1,8
  //       DstRef
  //     enddo
  //   enddo
  // enddo
  if (SrcLp->isInnermost() && DstLp->isInnermost() &&
      SrcLp->getNestingLevel() < DstLp->getNestingLevel()) {
    uint64_t TC;
    if (DstLp->isConstTripLoop(&TC) && TC <= 8) {
      LLVM_DEBUG(dbgs() << "\nBail out for different loop nests, case 2 \n");
      return true;
    }
  }
  return false;
}

// Profitability check on the edge.
bool FuseGraph::isProfitableDependency(const DDEdge &Edge) const {
  auto *SrcRef = Edge.getSrc();
  auto *DstRef = Edge.getSink();

  if (hasDifferentLoopNests(SrcRef, DstRef)) {
    return false;
  }

  return true;
}

bool FuseGraph::isLegalDependency(const DDEdge &Edge,
                                  unsigned CommonLevel) const {
  LLVM_DEBUG(Edge.dump());

  auto *SrcRef = Edge.getSrc();
  auto *DstRef = Edge.getSink();

  std::pair<unsigned, unsigned> MinMaxLevel =
      std::minmax(DstRef->getNodeLevel(), SrcRef->getNodeLevel());

  assert(CanonExpr::isValidLoopLevel(CommonLevel));

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

  assert(CanonExpr::isValidLoopLevel(MinMaxLevel.second));

  auto RefinedDep =
      refineDependency(SrcRef, DstRef, CommonLevel, MinMaxLevel.second);

  if (RefinedDep.isIndependent()) {
    return true;
  }

  if (!RefinedDep.isRefined()) {
    return false;
  }

  auto Dir = RefinedDep.getDV()[CommonLevel - 1];
  assert(Dir != DVKind::NONE);

  if ((Dir & DVKind::LT) != DVKind::NONE) {
    return false;
  }

  if ((Dir & DVKind::GT) != DVKind::NONE) {
    return true;
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
    auto *SrcLoop = dyn_cast<HLLoop>(SrcNode);
    bool IsInnermost = SrcLoop && SrcLoop->isInnermost() &&
                       (SrcLoop->getNestingLevel() == Level);

    for (DDRef *Ref : Refs) {
      // Collect Directed Dependency Edges
      for (const DDEdge *DDEdge : DDG.outgoing(Ref)) {
        // Mark loop as non-vectorizable if the edge preventing vectorization is
        // found.
        if (!SkipVecProfitabilityCheck && IsInnermost &&
            DDEdge->preventsVectorization(Level)) {

          LLVM_DEBUG(dbgs() << "\nDDEdge preventing vectorization found: ");
          LLVM_DEBUG(DDEdge->print(dbgs()));

          HLNode *DstNodeExact = DDEdge->getSink()->getHLDDNode();
          HLNode *DstNode = HLNodeUtils::getImmediateChildContainingNode(
              ParentNode, DstNodeExact);
          if (SrcNode == DstNode) {
            FuseNode &SrcFuseNode = Vertex[SrcNumber];
            SrcFuseNode.setVectorizable(false);
          }
        }

        if (DDEdge->isBackwardDep()) {
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
          bool Profitable = true;
          unsigned CommonTC =
              areFusibleWithCommonTC(FusibleCache, SrcFuseNode, DstFuseNode);

          LLVM_DEBUG(SrcFuseNode.print(dbgs()));
          LLVM_DEBUG(dbgs() << " --> ");
          LLVM_DEBUG(DstFuseNode.print(dbgs()));

          if (CommonTC) {
            Legal = isLegalDependency(*DDEdge, Level);
            Profitable = Legal ? isProfitableDependency(*DDEdge) : false;

            LLVM_DEBUG(dbgs() << " < ");
            if (!Legal) {
              LLVM_DEBUG(dbgs() << "illegal");
            } else if (!Profitable) {
              LLVM_DEBUG(dbgs() << "unprofitable");
            } else {
              LLVM_DEBUG(dbgs() << "OK");
            }
            LLVM_DEBUG(dbgs() << " >\n");

          } else {
            LLVM_DEBUG(dbgs() << " may not be fused.\n");
          }

          if (CommonTC && Legal && Profitable) {
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

  verify(true);

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

    CollapseRangeTy CollapseRange;

    BitVector Visited(Vertex.size());

    auto &PathFromNodeV = PathFrom[NodeV];
    std::function<void(unsigned)> FindNodesFromVTo = [&](unsigned Node) {
      Visited.set(Node);
      // Note: predecessors may require lexical pre-sorting to make final
      // collapsed node stable w.r.t. order between internal nodes.
      for (unsigned NodeX : Predecessors[Node]) {
        if (PathFromNodeV.count(NodeX) && !Visited.test(NodeX)) {
          FindNodesFromVTo(NodeX);
        }
      }
      CollapseRange.insert(Node);
    };

    FindNodesFromVTo(NodeW);

    // Nothing to collapse. Heap edges between collapsed edges may be stale.
    if (CollapseRange.size() == 1) {
      // Or maybe it has one neighbor edge.
      if (!Neighbors[NodeV].count(NodeW)) {
        continue;
      }

      CollapseRange.clear();
      CollapseRange.insert(NodeV);
      CollapseRange.insert(NodeW); // (NodeV, NodeW) undirected
    }

    // Handle NumCases option.
    if (NumCases != -1) {
      if (NumCases == 0) {
        LLVM_DEBUG(dbgs() << "Fusion stopped because of the compiler option.");
        break;
      }

      NumCases = NumCases - 1;
    }

    collapse(Heap, NodeV, CollapseRange);

    DEBUG_FG(dbgs() << "Fuse graph debug:\n");
    DEBUG_FG(dump());

    verify(false);
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
      if (Visited[CurNodeIdx]) {
        WorkList.pop_back();
        continue;
      }

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

void FuseGraph::verifyDependentMaps(StringRef Title, const NodeMapTy &BaseMap,
                                    const NodeMapTy &CheckMap,
                                    bool ReverseEdges,
                                    bool IgnoreRemovedNodes) const {
  for (auto &Pair : BaseMap) {
    auto SrcNode = Pair.first;

    if (IgnoreRemovedNodes && Vertex[SrcNode].isRemoved()) {
      continue;
    }

    for (auto DstNode : Pair.second) {
      if (IgnoreRemovedNodes && Vertex[DstNode].isRemoved()) {
        continue;
      }

      auto CheckSrc = ReverseEdges ? DstNode : SrcNode;
      auto CheckDst = ReverseEdges ? SrcNode : DstNode;

      auto FoundI = CheckMap.find(CheckSrc);
      if (FoundI == CheckMap.end() || FoundI->second.count(CheckDst) == 0) {
        dbgs() << Title << " edge (" << CheckSrc << "->" << CheckDst
               << ") not found in the counterpart map\n";
        llvm_unreachable("Fusion graph internal inconsistency");
      }
    }
  }
}

void FuseGraph::verify(bool InitialState) const {
#ifndef NDEBUG
  verifyDependentMaps("Successor", Successors, Predecessors, true, false);
  verifyDependentMaps("Predecessor", Predecessors, Successors, true, false);
  verifyDependentMaps("Neighbor", Neighbors, Neighbors, true, false);
  verifyDependentMaps("PathFrom", PathFrom, PathTo, true, !InitialState);
  verifyDependentMaps("BadPathFrom", BadPathFrom, BadPathTo, true,
                      !InitialState);
  verifyDependentMaps("PathTo", PathTo, PathFrom, true, !InitialState);
  verifyDependentMaps("BadPathTo", BadPathTo, BadPathFrom, true, !InitialState);

  // Verify that Successors are in sync with PathFrom maps.
  verifyDependentMaps("Succ/PathFrom", Successors, PathFrom, false, false);
#endif // NDEBUG
}
