//===--- HIRLoopDistributionGraph.cpp - Forms Distribution Graph  ---------===//
//
// Copyright (C) 2015-2019 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
// This file forms the Distribution Pre Proc Graph
//===----------------------------------------------------------------------===//

// In order to establish piblock graph, we start with
// the DD graph and reduce it to a smaller graph(DistPPGraph) as a
// preprocessing step. The nodes(DistPPNodes) in this graph are also
// indivisible groups of hlnodes, but this graph is permitted to have cycles.
// There is only a single DistPPEdge between nodes, but it can represent
// multiple DD edges. The number of edges/nodes in this graph is likely to
// be much more tractable than the DDGraph itself for scc detection/analysis.
// This graph is then analyzed for sccs, each of which forms a pi block.
//
// Consider the following loop nest
//          BEGIN REGION { }
//          <29>         + DO i1 = 0, 99998, 1   <DO_LOOP>
//          <30>         |   + DO i2 = 0, 99998, 1   <DO_LOOP>
//          <6>          |   |   %0 = (@B)[0][i1 + 1][i2 + 1];
//          <8>          |   |   %1 = (@C)[0][i1 + 1][i2 + 1];
//          <9>          |   |   %add = %0  +  %1;
//          <11>         |   |   (@A)[0][i1 + 1][i2 + 1] = %add;
//          <14>         |   |   %3 = (@A)[0][i1 + 1][i2];
//          <15>         |   |   %conv18 = %3  *  2.000000e+00;
//          <17>         |   |   (@D)[0][i1 + 1][i2 + 1] = %conv18;
//          <30>         |   + END LOOP
//          <29>         + END LOOP
//          END REGION
// If we are distributing the innermost loop, we have 7 stmts, each of which
// forms its own indivisible DistPPNode. DD edges are analyzed to form
// DistPPEdges. The resulting DistPPGraph is then analyzed
// for SCCs, each of which is a pi block. This results in a graph with two nodes
// PiBlock:
//<6>       %0 = (@B)[0][i1 + 1][i2 + 1];
//<8>       %1 = (@C)[0][i1 + 1][i2 + 1];
//<9>       %add = %0  +  %1;
//<11>      (@A)[0][i1 + 1][i2 + 1] = %add;
// PiBlock:
//<14>      %3 = (@A)[0][i1 + 1][i2];
//<15>      %conv18 = %3  *  2.000000e+00;
//<17>      (@D)[0][i1 + 1][i2 + 1] = %conv18;
// and a Pi Edge from first block to second block representing dd constraints(in
// this case only the flow < to/from A[][][])
//
// However consider the slightly modified version
//<38>         + DO i1 = 0, 99998, 1   <DO_LOOP>
//<39>         |   + DO i2 = 0, 99998, 1   <DO_LOOP>
//<6>          |   |   if (i2 + 1 < 27)
//<6>          |   |   {
//<11>         |   |      %0 = (@B)[0][i1 + 1][i2 + 1];
//<13>         |   |      %1 = (@C)[0][i1 + 1][i2 + 1];
//<14>         |   |      %add = %0  +  %1;
//<16>         |   |      (@A)[0][i1 + 1][i2 + 1] = %add;
//<6>          |   |   }
//<6>          |   |   else
//<6>          |   |   {
//<33>         |   |      %3 = (@A)[0][i1 + 1][i2];
//<34>         |   |      %conv19 = %3  *  2.000000e+00;
//<36>         |   |      (@D)[0][i1 + 1][i2 + 1] = %conv19;
//<6>          |   |   }
//<39>         |   + END LOOP
//<38>         + END LOOP
// The graph for the innermost loop has a single DistPPNode which contains the
// HLIf and indirectly its children.
//
// Graphs for the outermost loop also contain a single node for the inner
// HLLoop. It is assumed that if the innermost level loop was distributable,
// it would have already been done. This forces clients to analyze loops
// innermost to outermost if considering all distribution possibilities

#include "HIRLoopDistributionPreProcGraph.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRSparseArrayReductionAnalysis.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/ForEach.h"

using namespace llvm;
using namespace llvm::loopopt;

#define DEBUG_TYPE "hir-loop-distribution-pre-proc-graph"

// Walks all hlnodes and creates DistPPNodes in member DistPPGraph for them
struct DistributionNodeCreator final : public HLNodeVisitorBase {

  DistPPGraph *DGraph;
  DistPPNode *CurDistPPNode;
  SmallVector<DistPPNode *, 8> CurControlDepNodes;
  SmallVector<DistPPNode *, 8> UnsafeSideEffectNodes;
  bool CreateControlNodes;

  bool isDone() const { return !DGraph->isGraphValid(); }

  // establishes HLNode's corresponding DistPPNode
  void addToNodeMap(DistPPNode *DNode, HLNode *HNode) {
    assert(DNode && "Null Dist Node");
    DGraph->getNodeMap()[HNode] = DNode;
  }

  DistributionNodeCreator(DistPPGraph *G, bool CreateControlNodes)
      : DGraph(G), CurDistPPNode(nullptr),
        CreateControlNodes(CreateControlNodes) {}

  // Creates new PPNode if current node is not defined. Adds \p HNode to the
  // current PPNode.
  void startDistPPNode(HLNode *HNode, HLNode *ParentNode = nullptr) {

    // if CurDistPPNode is set it means we are visiting
    // children of an hlnode. Our distPPNode should be
    // our parent hlnode's distPPNode, which is CurDistPPNode
    if (!CurDistPPNode) {
      CurDistPPNode = new DistPPNode(ParentNode ? ParentNode : HNode, DGraph);

      DGraph->addNode(CurDistPPNode);

      if (!CurControlDepNodes.empty()) {
        auto &SrcDepNode = CurControlDepNodes.back();
        HLIf *If = cast<HLIf>(SrcDepNode->getNode());
        DGraph->addControlDependence(SrcDepNode, CurDistPPNode,
                                     If->isThenChild(HNode));
      }
    }

    addToNodeMap(CurDistPPNode, HNode);
  }

  // Stops current PPNode, subsequent calls to startDistPPNode() will create new
  // PPNode.
  void stopDistPPNode(HLNode *HNode) {
    // We are done visiting an hlnode's children
    // Clear CurDistPPNode so that we create new DistPPNodes
    if (CurDistPPNode->getNode() == HNode) {
      CurDistPPNode = nullptr;
    }
  }

  void visit(HLLoop *L) { startDistPPNode(L); }
  void postVisit(HLLoop *L) {
    if (!L->hasPostexit()) {
      stopDistPPNode(L);
    }
  }

  bool mayDistributeCondition(HLIf *If) {
    if (!CreateControlNodes) {
      return false;
    }

    // Allow distribution of top level HLIf only. This may be extended.
    if (!CurControlDepNodes.empty()) {
      return false;
    }

    for (auto &Ref : make_range(If->ddref_begin(), If->ddref_end())) {
      // Only distribute conditions with linear and privatizable (non-livein)
      // temps as they can be scalar-expanded.
      if (!Ref->isTerminalRef() ||
          (!Ref->isLinear() && Ref->isLiveIntoParentLoop())) {
        return false;
      }
    }

    return true;
  };

  void visit(HLIf *If) {
    // Check if CurDistPPNode is set, meaning we are processing children of a
    // single PPNode.
    bool MayDistributeParent = (CurDistPPNode == nullptr);
    startDistPPNode(If);

    if (MayDistributeParent && mayDistributeCondition(If)) {
      CurDistPPNode->setControlNode();
      CurControlDepNodes.push_back(CurDistPPNode);
      stopDistPPNode(If);
    }
  }

  void postVisit(HLIf *If) {
    if (!CurControlDepNodes.empty() &&
        CurControlDepNodes.back()->getNode() == If) {
      CurControlDepNodes.pop_back();
    } else {
      stopDistPPNode(If);
    }
  }

  void visit(HLSwitch *Switch) { startDistPPNode(Switch); }
  void postVisit(HLSwitch *Switch) { stopDistPPNode(Switch); }
  void visit(HLInst *I) {
    HLLoop *ParentLoop = I->getParentLoop();

    if (ParentLoop && ParentLoop->hasPreheader() &&
        (ParentLoop->getFirstPreheaderNode() == I)) {
      // Use loop for the DistPPNode starting from the first preheader node.
      startDistPPNode(I, ParentLoop);
    } else {
      startDistPPNode(I);
    }

    if (I->isUnsafeSideEffectsCallInst() || I->isUnknownAliasingCallInst()) {
      // Add unsafe side effect nodes without duplicates.
      if (UnsafeSideEffectNodes.empty() ||
          UnsafeSideEffectNodes.back() != CurDistPPNode) {
        UnsafeSideEffectNodes.push_back(CurDistPPNode);
      }
    }

    if (ParentLoop && ParentLoop->hasPostexit() &&
        (ParentLoop->getLastPostexitNode() == I)) {
      // Reset DistPPNode at the last postexit node.
      stopDistPPNode(ParentLoop);
    } else {
      stopDistPPNode(I);
    }
  }

  void visit(const HLLabel *L) {
    DGraph->setInvalid(
        "Cannot distribute graph with unstructured control flow");
  }

  void visit(const HLGoto *G) {
    DGraph->setInvalid(
        "Cannot distribute graph with unstructured control flow");
  }

  void visit(const HLNode *Node) {}
  void postVisit(const HLNode *Node) {}
};

// Creates DistPPEdges out of DDEdges and adds them to DistPPGraph
struct DistributionEdgeCreator final : public HLNodeVisitorBase {
  unsigned LoopLevel;
  DDGraph LoopDDGraph;
  HIRSparseArrayReductionAnalysis *SARA;
  DistPPGraph *DistG;
  bool ForceCycleForLoopIndepDep;
  unsigned EdgeCount = 0;
  typedef DenseMap<DistPPNode *, SmallVector<const DDEdge *, 16>> EdgeNodeMapTy;

  DistributionEdgeCreator(HIRSparseArrayReductionAnalysis *SARA,
                          DistPPGraph *DistPreProcGraph, DDGraph LoopDDGraph,
                          unsigned LoopLevel, bool ForceCycleForLoopIndepDep)
      : LoopLevel(LoopLevel), LoopDDGraph(LoopDDGraph), SARA(SARA),
        DistG(DistPreProcGraph),
        ForceCycleForLoopIndepDep(ForceCycleForLoopIndepDep) {}

  void processOutgoingEdges(const DDRef *Ref, EdgeNodeMapTy &EdgeMap) {
    DenseMap<HLNode *, DistPPNode *> &HLNodeToDistPPNode = DistG->getNodeMap();

    for (auto *Edge : LoopDDGraph.outgoing(Ref)) {
      HLDDNode *DstDDNode = Edge->getSink()->getHLDDNode();
      auto DstDistPPNodeI = HLNodeToDistPPNode.find(DstDDNode);

      assert(DstDistPPNodeI != HLNodeToDistPPNode.end() &&
             "Every hlnode in loop nest has a dist node, so this edge goes out "
             "of our loop nest. Don't need an edge in this case.");

      DistPPNode *DstDistNode = DstDistPPNodeI->second;

      // Add ddedge to list of edges for this sink DistPPNode
      EdgeMap[DstDistNode].push_back(Edge);
    }
  }

  bool needBackEdgeForIndepDep(const DDEdge *Edge) const {
    //  Relaxing the condition will cause temps splitted into
    //  different PiGroups resulting in scalar expansion

    // We want sparse array reduction chains to be distributed
    // So back edges to those instructions should not be constructed
    // Except edges in between sparse array reduction instructions
    const HLInst *SinkInst = dyn_cast<HLInst>(Edge->getSink()->getHLDDNode());
    const HLInst *SrcInst = dyn_cast<HLInst>(Edge->getSrc()->getHLDDNode());
    if (SinkInst && SrcInst && SARA->isSparseArrayReduction(SinkInst) &&
        !SARA->isSparseArrayReduction(SrcInst)) {

      // Do not create back edge for sparse array reduction terms (%add) but
      // create them for the index (%idx) 2->1, 3->1:
      //   <1> %idx =
      //   <2> %t = %p[%idx]
      //   <3> %p[%idx] = %t + %add
      auto *SinkBlobDDRef = dyn_cast<BlobDDRef>(Edge->getSink());
      if (!SinkBlobDDRef || SinkBlobDDRef->getParentDDRef()->isTerminalRef()) {
        return false;
      }
    }

    //  When max level is reached, cannot stripmine
    if (LoopLevel == MaxLoopNestLevel || ForceCycleForLoopIndepDep) {
      return true;
    }

    if (SrcInst && SrcInst->isInPreheaderOrPostexit()) {
      return true;
    }

    return false;
  }

  bool needBackwardEdge(const DDEdge *Edge) const {

    // Need to force a backward edge in the Dist Graph?
    // for t1 =
    //        = t1
    // DD only produce the flow (=) edge
    // Note: do not force cycle in Break Recurrence pass because Scalar
    // Expansion is performed

    if (Edge->isLoopIndependentDepTemp() && needBackEdgeForIndepDep(Edge)) {
      return true;
    }

    // Scalar temp Output Dep (*) has single edge

    DDRef *DDRefSrc = Edge->getSrc();
    HLNode *SrcHIR = DDRefSrc->getHLDDNode();
    RegDDRef *RegRef = dyn_cast<RegDDRef>(DDRefSrc);

    if (Edge->isOutput()) {
      assert(RegRef && "RegDDRef expected");
      if (RegRef->isTerminalRef() &&
          Edge->getDVAtLevel(LoopLevel) == DVKind::ALL) {
        return true;
      }
    }

    // For Memory refs with (<=), only have 1 DD Edge is formed which
    // should be sufficent for most transformations that have no reordering
    // within the same iteration, for the purpose of fast compile time.
    // For Dist, need to special case and add a backward edge if needed
    // This applies for all dep (F/A/O).
    // e.g.
    //     DO  i=1,50
    // s1:   A[100 -2 *i ] =
    // s2:   A[50 - i] =
    // We have   s2 : s1  output (<=)
    // Without forcing the backward edge,  Dist will end up with
    //  Loop1
    //    s2
    //  Loop2
    //    s1

    if (!RegRef) {
      return false;
    }

    DDRef *DDRefSink = Edge->getSink();
    if (Edge->getDVAtLevel(LoopLevel) == DVKind::LE) {
      HLNode *DstHIR = DDRefSink->getHLDDNode();
      if (!HLNodeUtils::dominates(SrcHIR, DstHIR)) {
        return true;
      }
    }

    return false;
  }

  void visit(HLDDNode *DDNode) {
    // src of edge is a node inside loop, which must have a dist node
    DenseMap<HLNode *, DistPPNode *> &HLNodeToDistPPNode = DistG->getNodeMap();
    DistPPNode *SrcDistPPNode = HLNodeToDistPPNode[DDNode];
    assert(SrcDistPPNode && "Missing dist node");

    EdgeNodeMapTy EdgeMap;

    for (auto SrcRef = DDNode->ddref_begin(), End = DDNode->ddref_end();
         SrcRef != End; ++SrcRef) {
      // Every outgoing edge is an incoming edge for a node in our loop nest
      // No need to iterate over both outgoing and incoming
      processOutgoingEdges(*SrcRef, EdgeMap);
      for (auto BSrcRef = (*SrcRef)->blob_begin(),
                BEnd = (*SrcRef)->blob_end();
           BSrcRef != BEnd; ++BSrcRef) {
        processOutgoingEdges(*BSrcRef, EdgeMap);
      }
    }

    // Create DistPPEdges, which cannot be modifed after addition to graph.
    for (auto PairI = EdgeMap.begin(), EndI = EdgeMap.end(); PairI != EndI;
         ++PairI) {

      DistG->addEdge(DistPPEdge(SrcDistPPNode, PairI->first, PairI->second));
      EdgeCount++;

      SmallVectorImpl<const DDEdge *> &EdgeList = PairI->second;

      for (auto *Edge : EdgeList) {

        if (needBackwardEdge(Edge)) {
          DistG->addEdge(
              DistPPEdge(PairI->first, SrcDistPPNode, PairI->second));
          EdgeCount++;
          break;
        }
      }
      // TODO early bailout should be here, even if reporting cant be done here
    }
  }

  void visit(const HLNode *Node) {}
  void postVisit(const HLNode *Node) {}
};

static bool ppSort(const DistPPNode *A, const DistPPNode *B) {
  return A->getNode()->getTopSortNum() < B->getNode()->getTopSortNum();
}

void DistPPGraph::addCycle(loopopt::DistPPNode *NodeA,
                           loopopt::DistPPNode *NodeB) {
  addEdge(DistPPEdge(NodeA, NodeB));
  addEdge(DistPPEdge(NodeB, NodeA));
}

void DistPPGraph::constructUnknownSideEffectEdges(
    ArrayRef<DistPPNode *> UnsafeNodes) {
  if (UnsafeNodes.empty()) {
    return;
  }

  auto UnsafeI = UnsafeNodes.begin();
  auto UnsafeE = UnsafeNodes.end();

  DistPPNode *UnsafeL = nullptr;
  DistPPNode *UnsafeR = *UnsafeI;

  // Create cycles for each pair of nodes {N, Ul}, {Ul, Ur}, {Ur, N}: where
  // N is a node with memory reference, and
  // Ul, Ur are left and right unsafe nodes.
  for (auto *Node : make_range(node_begin(), node_end())) {
    if (UnsafeI != UnsafeE && Node == *UnsafeI) {
      if (UnsafeL) {
        addCycle(UnsafeL, Node);
      }

      UnsafeL = UnsafeR;
      ++UnsafeI;

      if (UnsafeI == UnsafeE) {
        UnsafeR = nullptr;
      } else {
        UnsafeR = *UnsafeI;
      }

      continue;
    } else if (!Node->hasMemRef()) {
      // Skip nodes which does not access memory.
      continue;
    }

    if (UnsafeL) {
      addCycle(Node, UnsafeL);
    }

    if (UnsafeR) {
      addCycle(UnsafeR, Node);
    }
  }
}

DistPPGraph::DistPPGraph(HLLoop *Loop, HIRDDAnalysis &DDA,
                         HIRSparseArrayReductionAnalysis &SARA,
                         bool ForceCycleForLoopIndepDep,
                         bool CreateControlNodes) {
  const unsigned MaxDistPPSize = 128;
  const unsigned MaxDDEdges = 300;

  DistributionNodeCreator NodeCreator(this, CreateControlNodes);
  HLNodeUtils::visitRange(NodeCreator, Loop->getFirstChild(),
                          Loop->getLastChild());

  if (!isGraphValid()) {
    return;
  }

  auto NodeCount = getNodeCount();

  // Bail early before DD for invalid cases
  if (NodeCount > MaxDistPPSize) {
    setInvalid("Too many stmts to analyze");
    return;
  }

  if (NodeCount == 1) {
    setInvalid("Single Node Loop cannot be analyzed");
    return;
  }

  std::sort(node_begin(), node_end(), ppSort);

  constructUnknownSideEffectEdges(NodeCreator.UnsafeSideEffectNodes);

  unsigned Level = Loop->getNestingLevel();
  DDGraph DG = DDA.getGraph(Loop);

  DistributionEdgeCreator EdgeCreator(&SARA, this, DG, Level,
                                      ForceCycleForLoopIndepDep);
  HLNodeUtils::visitRange(EdgeCreator, Loop->getFirstChild(),
                          Loop->getLastChild());

  unsigned TotalEdges = EdgeCreator.EdgeCount;

  // Add Control Dependency edges
  for (auto &ControlDepPair : ControlDeps) {
    addEdge(DistPPEdge(ControlDepPair.second.first, ControlDepPair.first));
    ++TotalEdges;
  }

  if (TotalEdges > MaxDDEdges) {
    setInvalid("Too many DD edges for proper analysis");
    return;
  }

  std::for_each(node_begin(), node_end(), [this](const DistPPNode *N) {
    std::sort(this->mutable_incoming_edges_begin(N),
              this->mutable_incoming_edges_end(N),
              [](const DistPPEdge *a, const DistPPEdge *b) -> bool {
                return ppSort(a->Src, b->Src);
              });
    std::sort(this->mutable_outgoing_edges_begin(N),
              this->mutable_outgoing_edges_end(N),
              [](const DistPPEdge *a, const DistPPEdge *b) -> bool {
                return ppSort(a->Sink, b->Sink);
              });
  });
}

bool DistPPNode::hasMemRef() const {
  HLDDNode *DDNode = dyn_cast<HLDDNode>(getNode());
  if (!DDNode) {
    return false;
  }

  bool FoundMemRef = false;
  auto HasMemRef = [&](const RegDDRef *Ref) {
    FoundMemRef = FoundMemRef || Ref->isMemRef();
  };

  if (isControlNode()) {
    ForEach<const RegDDRef>::visit<false>(DDNode, HasMemRef);
  } else {
    ForEach<const RegDDRef>::visit(DDNode, HasMemRef);
  }

  return FoundMemRef;
}

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_METHOD
void DistPPNode::dump() {
  dbgs() << "\"" << getNum() << "\": ";

  auto ControlDep = Graph->getControlDependence(this);
  if (ControlDep) {
    dbgs() << "<dep " << ControlDep->first->getNode()->getNumber() << "> ";
  }

  if (isControlNode()) {
    cast<HLIf>(HNode)->dumpHeader();
    dbgs() << "\n";
    return;
  }

  HNode->dump();
}

LLVM_DUMP_METHOD
void DistPPEdge::dump() const {
  dbgs() << Src->getNum() << " -> " << Sink->getNum() << "\n";

  if (!DDEdges.empty()) {
    for (auto *DDEdge : DDEdges) {
      DDEdge->dump();
    }
  }
}
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
