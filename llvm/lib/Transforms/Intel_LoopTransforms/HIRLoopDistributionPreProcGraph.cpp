//===--- HIRLoopDistributionGraph.cpp - Forms Distribution Graph  ---------===//
//
// Copyright (C) 2015-2018 Intel Corporation. All rights reserved.
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

using namespace llvm;
using namespace llvm::loopopt;

#define DEBUG_TYPE "hir-loop-distribution-pre-proc-graph"

// Walks all hlnodes and creates DistPPNodes in member DistPPGraph for them
struct DistributionNodeCreator final : public HLNodeVisitorBase {

  DistPPGraph *DGraph;
  DistPPNode *CurDistPPNode;

  bool isDone() const { return !DGraph->isGraphValid(); }

  // establishes HLNode's corresponding DistPPNode
  void addToNodeMap(DistPPNode *DNode, HLNode *HNode) {
    assert(DNode && "Null Dist Node");
    DGraph->getNodeMap()[HNode] = DNode;
  }

  DistributionNodeCreator(DistPPGraph *G) : DGraph(G), CurDistPPNode(nullptr) {}

  void visitDistPPNode(HLNode *HNode, HLNode *ParentNode = nullptr) {

    // if CurDistPPNode is set it means we are visiting
    // children of an hlnode. Our distPPNode should be
    // our parent hlnode's distPPNode, which is CurDistPPNode
    if (!CurDistPPNode) {
      CurDistPPNode = new DistPPNode(ParentNode ? ParentNode : HNode, DGraph);
      DGraph->addNode(CurDistPPNode);
    }

    addToNodeMap(CurDistPPNode, HNode);
  }

  void postVisitDistPPNode(HLNode *HNode) {
    // We are done visiting an hlnode's children
    // Clear CurDistPPNode so that we create new DistPPNodes
    if (CurDistPPNode->HNode == HNode) {
      CurDistPPNode = nullptr;
    }
  }

  void visit(HLLoop *L) { visitDistPPNode(L); }
  void postVisit(HLLoop *L) {
    if (!L->hasPostexit()) {
      postVisitDistPPNode(L);
    }
  }

  void visit(HLIf *If) { visitDistPPNode(If); }
  void postVisit(HLIf *If) { postVisitDistPPNode(If); }

  void visit(HLSwitch *Switch) { visitDistPPNode(Switch); }
  void postVisit(HLSwitch *Switch) { postVisitDistPPNode(Switch); }
  void visit(HLInst *I) {
    if (isa<CallInst>(I->getLLVMInstruction())) {
      DGraph->setInvalid("Cannot distribute loops with calls");
      return;
    }
    HLLoop *ParentLoop = I->getParentLoop();

    if (ParentLoop && ParentLoop->hasPreheader() &&
        (ParentLoop->getFirstPreheaderNode() == I)) {
      // Use loop for the DistPPNode starting from the first preheader node.
      visitDistPPNode(I, ParentLoop);
    } else {
      visitDistPPNode(I);
    }

    if (ParentLoop && ParentLoop->hasPostexit() &&
        (ParentLoop->getLastPostexitNode() == I)) {
      // Reset DistPPNode at the last postexit node.
      postVisitDistPPNode(ParentLoop);
    } else {
      postVisitDistPPNode(I);
    }
  }

  void visit(const HLLabel *L) {
    DGraph->setInvalid("Cannot distribute graph with control flow");
  }
  void visit(const HLGoto *G) {
    DGraph->setInvalid("Cannot distribute graph with control flow");
  }

  void visit(const HLNode *Node) {}
  void postVisit(const HLNode *Node) {}
};

// Creates DistPPEdges out of DDEdges and adds them to DistPPGraph
struct DistributionEdgeCreator final : public HLNodeVisitorBase {

  DDGraph *LoopDDGraph;
  HIRSparseArrayReductionAnalysis *SARA;
  DistPPGraph *DistG;
  HLLoop *Loop;
  bool ForceCycleForLoopIndepDep;
  unsigned EdgeCount = 0;
  typedef DenseMap<DistPPNode *, SmallVector<const DDEdge *, 16>> EdgeNodeMapTy;
  DistributionEdgeCreator(DDGraph *DDG, HIRSparseArrayReductionAnalysis *SARA,
                          DistPPGraph *DistPreProcGraph, HLLoop *Loop,
                          bool ForceCycleForLoopIndepDep)
      : LoopDDGraph(DDG), SARA(SARA), DistG(DistPreProcGraph), Loop(Loop),
        ForceCycleForLoopIndepDep(ForceCycleForLoopIndepDep) {}

  void processOutgoingEdges(const DDRef *Ref, EdgeNodeMapTy &EdgeMap) {
    DenseMap<HLNode *, DistPPNode *> &HLNodeToDistPPNode = DistG->getNodeMap();
    for (auto Edge = LoopDDGraph->outgoing_edges_begin(Ref),
              LastEdge = LoopDDGraph->outgoing_edges_end(Ref);
         Edge != LastEdge; ++Edge) {
      HLDDNode *DstDDNode = (*Edge)->getSink()->getHLDDNode();
      auto DstDistPPNodeI = HLNodeToDistPPNode.find(DstDDNode);
      if (DstDistPPNodeI == HLNodeToDistPPNode.end()) {
        // Every hlnode in loop nest has a dist node, so this edge goes out of
        // our loop nest. Don't need an edge in this case.
        continue;
      }
      // Add ddedge to list of edges for this sink DistPPNode
      DistPPNode *DstDistNode = DstDistPPNodeI->second;
      EdgeMap[DstDistNode].push_back(*Edge);
    }
  }

  bool needBackEdgeForIndepDep(const DDEdge *Edge, unsigned LoopLevel) const {
    //  Relaxing the condition will cause temps splitted into
    //  different PiGroups resulting in scalar expansion

    // We want sparse array reduction chains to be distributed
    // So back edges to those instructions should not be constructed
    // Except edges in between sparse array reduction instructions
    const HLInst *SinkInst = dyn_cast<HLInst>(Edge->getSink()->getHLDDNode());
    const HLInst *SrcInst = dyn_cast<HLInst>(Edge->getSrc()->getHLDDNode());
    if (SinkInst && SrcInst && SARA->isSparseArrayReduction(SinkInst) &&
        !SARA->isSparseArrayReduction(SrcInst)) {
      return false;
    }

    //  When max level is reached, cannot stripmine
    if (LoopLevel == MaxLoopNestLevel || ForceCycleForLoopIndepDep) {
      return true;
    }

    if (SrcInst && SrcInst->isInPreheaderOrPostexit()) {
      return true;
    }

    //  Except blobs in Sparse Array Reductions,
    //  We will not handle Blob DDREF for scalar expansion now
    //  because of direct replacement is done w/o creating an assignment.
    //  It is uncommon anyway.
    //  TODO: Check it helps performance
    DDRef *DDRefSink = Edge->getSink();
    if (isa<BlobDDRef>(DDRefSink)) {
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

    unsigned LoopLevel = Loop->getNestingLevel();

    if (Edge->isLoopIndependentDepTemp() &&
        needBackEdgeForIndepDep(Edge, LoopLevel)) {
      return true;
    }

    // Scalar temp Output Dep (*) has single edge

    DDRef *DDRefSrc = Edge->getSrc();
    HLNode *SrcHIR = DDRefSrc->getHLDDNode();
    RegDDRef *RegRef = dyn_cast<RegDDRef>(DDRefSrc);

    if (Edge->isOUTPUTdep()) {
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
      for (auto BSrcRef = (*SrcRef)->blob_cbegin(),
                BEnd = (*SrcRef)->blob_cend();
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

DistPPGraph::DistPPGraph(HLLoop *Loop, HIRDDAnalysis &DDA,
                         HIRSparseArrayReductionAnalysis &SARA,
                         bool ForceCycleForLoopIndepDep) {

  const unsigned MaxDDEdges = 300;

  createNodes(Loop);
  if (!isGraphValid()) {
    return;
  }

  DDGraph DDG = DDA.getGraph(Loop);
  DistributionEdgeCreator EdgeCreator(&DDG, &SARA, this, Loop,
                                      ForceCycleForLoopIndepDep);

  Loop->getHLNodeUtils().visitRange(EdgeCreator, Loop->getFirstChild(),
                                    Loop->getLastChild());

  if (EdgeCreator.EdgeCount > MaxDDEdges) {
    setInvalid("Too many DD edges for proper analysis");
  }

  auto PPSort = [](const DistPPNode *a, const DistPPNode *b) -> bool {
    return a->HNode->getTopSortNum() < b->HNode->getTopSortNum();
  };

  std::sort(node_begin(), node_end(), PPSort);

  std::for_each(node_begin(), node_end(), [this, PPSort](const DistPPNode *n) {
    std::sort(this->mutable_incoming_edges_begin(n),
              this->mutable_incoming_edges_end(n),
              [PPSort](const DistPPEdge *a, const DistPPEdge *b) -> bool {
                return PPSort(a->Src, b->Src);
              });
    std::sort(this->mutable_outgoing_edges_begin(n),
              this->mutable_outgoing_edges_end(n),
              [PPSort](const DistPPEdge *a, const DistPPEdge *b) -> bool {
                return PPSort(a->Sink, b->Sink);
              });
  });
}
void DistPPGraph::createNodes(HLLoop *Loop) {
  const unsigned MaxDistPPSize = 128;

  DistributionNodeCreator NodeCreator(this);
  Loop->getHLNodeUtils().visitRange(NodeCreator, Loop->getFirstChild(),
                                    Loop->getLastChild());
  // Bail early before DD for invalid cases
  if (getNodeCount() > MaxDistPPSize) {
    setInvalid("Too many stmts to analyze");
  }

  if (getNodeCount() == 1) {
    setInvalid("Single Node Loop cannot be analyzed");
  }
}
