//===----- HIRLoopDistributionGraph.h - Forms Distribution Graph  --------===//
//
// Copyright (C) 2015-2016 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
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

#include "llvm/Analysis/Intel_LoopAnalysis/DDAnalysis.h"

#include "llvm/Analysis/Intel_LoopAnalysis/DDGraph.h"

#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/SCCIterator.h"
#include "llvm/Support/Debug.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/CanonExprUtils.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/DDRefUtils.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/HLNodeUtils.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/HLNodeVisitor.h"

#include <algorithm>
namespace llvm {

namespace loopopt {

// TODO: Arbitrarily chosen
unsigned MaxDDEdges = 256;
unsigned MaxDistPPSize = 128;

class DistPPGraph;

// a distppnode[distribution preprocessing node] represents an indivisible(by
// loop dist anyway) chunk of the program. For example, an if block cannot be
// split by loop distribution, nor an inner loop when considering the outer
// loop. Stmts are perhaps a special case. Node splitting could theoretically
// do it. A distppnode is not a pi block, there could be cycles among nodes.
// A distppnode is not a HLDDNode either, a dist node can encompass a loop and
// all its children. All HLNodes contained by a loop share same dist node.
// This is a preprocessing step with then intent of making scc detection quicker
// by cutting down the number of nodes and edges. We expect far fewer
// DistPPNodes than HLDDNodes and fewer DistPPEdges than DDEdges

struct DistPPNode {
  // The HLNode for this dist node. All children hlnodes of this node
  // are represented by this dist node
  HLNode *HNode;
  DistPPGraph *Graph;
  DistPPNode(HLNode *N, DistPPGraph *G) : HNode(N), Graph(G) {}
  DistPPGraph *getGraph() { return Graph; }
  void dump() { HNode->dump(); }
};

// Edges in DistPPGraph. Represents a list of dd edges between two DistPPNodes
// Note that some of the DDEdges may not be part of the DDGraph. DD's sometimes
// skips creation of edges that are required for correct SCC formation(reverse
// edge for output * edge is one example)
struct DistPPEdge {
  DistPPNode *Src;
  DistPPNode *Sink;
  SmallVector<const DDEdge *, 16> DDEdges;
  DistPPNode *getSrc() const { return Src; }
  DistPPNode *getSink() const { return Sink; }
  DistPPEdge(DistPPNode *DistSrc, DistPPNode *DistSink,
             const SmallVectorImpl<const DDEdge *> &EdgeList)
      : Src(DistSrc), Sink(DistSink),
        DDEdges(EdgeList.begin(), EdgeList.end()) {}
  void print(raw_ostream &OS) const {
    // TODO
  }
};

class DistPPGraph : public HIRGraph<DistPPNode, DistPPEdge> {

public:
  void createNodes(HLLoop *Loop);
  unsigned getNodeCount() { return DistPPNodeList.size(); }

  // Marks graph as invalid for given reason
  // Possible failures could be too many nodes, edges etc
  void setInvalid(StringRef FailureReason) {
    GraphValidity = false;
    FailureString = FailureReason;
  }

  bool isGraphValid() { return GraphValidity; }
  std::string getFailureReason() { return FailureString; }

  SmallVectorImpl<DistPPNode *>::iterator node_begin() {
    return DistPPNodeList.begin();
  };

  SmallVectorImpl<DistPPNode *>::iterator node_end() {
    return DistPPNodeList.end();
  }

  DenseMap<HLNode *, DistPPNode *> &getNodeMap() { return HLToDistPPNodeMap; }

  DistPPGraph(HLLoop *Loop, DDAnalysis *DDA);

  // TODO destruction needs to be handled carefully if we want
  // to reuse graph from inner loop dist in outer loop distribution
  ~DistPPGraph() {
    for (DistPPNode *Node : DistPPNodeList) {
      delete Node;
    }
  }
  void addNode(DistPPNode *NewNode) { DistPPNodeList.push_back(NewNode); }

private:
  // unlike other hirgraphs, this one actually owns the memory for its nodes
  // Special note, the dist nodes(well more precisely DistPPNode->hnode) in this
  // list are in lexical order as dist nodes are created by a lexical walk
  // of hlnodes
  SmallVector<DistPPNode *, 36> DistPPNodeList;

  DenseMap<HLNode *, DistPPNode *> HLToDistPPNodeMap;

  std::string FailureString;
  bool GraphValidity = true;
};

// Walks all hlnodes and creates DistPPNodes in member DistPPGraph for them
struct DistributionNodeCreator final : public HLNodeVisitorBase {

  DistPPGraph *DGraph;
  DistPPNode *CurDistPPNode;

  bool isDone() const override { return !DGraph->isGraphValid(); }

  // establishes HLNode's corresponding DistPPNode
  void addToNodeMap(DistPPNode *DNode, HLNode *HNode) {
    assert(DNode && "Null Dist Node");
    DGraph->getNodeMap()[HNode] = DNode;
  }

  DistributionNodeCreator(DistPPGraph *G) : DGraph(G), CurDistPPNode(nullptr) {}

  void visitDistPPNode(HLNode *HNode) {
    DistPPNode *DNode = nullptr;

    // if CurDistPPNode is set it means we are visiting
    // children of an hlnode. Our distPPNode should be
    // our parent hlnode's distPPNode, which is CurDistPPNode
    if (CurDistPPNode) {
      DNode = CurDistPPNode;
    } else {
      DNode = new DistPPNode(HNode, DGraph);
      CurDistPPNode = DNode;
      DGraph->addNode(DNode);
    }

    addToNodeMap(DNode, HNode);
  }

  void postVisitDistPPNode(HLNode *HNode) {
    // We are done visiting an hlnode's children
    // Clear CurDistPPNode so that we create new DistPPNodes
    if (CurDistPPNode->HNode == HNode) {
      CurDistPPNode = nullptr;
    }
  }

  void visit(HLLoop *L) { visitDistPPNode(L); }
  void postVisit(HLLoop *L) { postVisitDistPPNode(L); }
  void visit(HLIf *If) { visitDistPPNode(If); }
  void postVisit(HLIf *If) { postVisitDistPPNode(If); }

  void visit(HLSwitch *Switch) { visitDistPPNode(Switch); }
  void postVisit(HLSwitch *Switch) { postVisitDistPPNode(Switch); }
  void visit(HLInst *I) {
    if (isa<CallInst>(I->getLLVMInstruction())) {
      DGraph->setInvalid("Cannot distribute loops with calls");
    }
    visitDistPPNode(I);
    postVisitDistPPNode(I);
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
  DistPPGraph *DistG;
  unsigned EdgeCount = 0;
  typedef DenseMap<DistPPNode *, SmallVector<const DDEdge *, 16>> EdgeNodeMapTy;
  DistributionEdgeCreator(DDGraph *DDG, DistPPGraph *DistPreProcGraph)
      : LoopDDGraph(DDG), DistG(DistPreProcGraph) {}

  void processOutgoingEdges(DDRef *Ref, EdgeNodeMapTy &EdgeMap) {
    DenseMap<HLNode *, DistPPNode *> &HLNodeToDistPPNode = DistG->getNodeMap();
    for (auto Edge = LoopDDGraph->outgoing_edges_begin(Ref),
              LastEdge = LoopDDGraph->outgoing_edges_end(Ref);
         Edge != LastEdge; ++Edge) {
      HLDDNode *DstDDNode = (Edge)->getSink()->getHLDDNode();
      auto DstDistPPNodeI = HLNodeToDistPPNode.find(DstDDNode);
      if (DstDistPPNodeI == HLNodeToDistPPNode.end()) {
        // Every hlnode in loop nest has a dist node, so this edge goes out of
        // our loop nest. Don't need an edge in this case.
        continue;
      }
      // Add ddedge to list of edges for this sink DistPPNode
      DistPPNode *DstDistNode = DstDistPPNodeI->second;
      EdgeMap[DstDistNode].push_back(&(*Edge));
    }
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
      // TODO early bailout should be here, even if reporting cant be done here
    }
  }

  void visit(const HLNode *Node) {}
  void postVisit(const HLNode *Node) {}
};

DistPPGraph::DistPPGraph(HLLoop *Loop, DDAnalysis *DDA) {
  createNodes(Loop);
  if (!isGraphValid()) {
    return;
  }

  DDGraph DDG = DDA->getGraph(Loop);
  DistributionEdgeCreator EdgeCreator(&DDG, this);
  HLNodeUtils::visitRange(EdgeCreator, Loop->getFirstChild(),
                          Loop->getLastChild());

  if (EdgeCreator.EdgeCount > MaxDDEdges) {
    setInvalid("Too many DD edges for proper analysis");
  }
}
void DistPPGraph::createNodes(HLLoop *Loop) {
  DistributionNodeCreator NodeCreator(this);
  HLNodeUtils::visitRange(NodeCreator, Loop->getFirstChild(),
                          Loop->getLastChild());
  // Bail early before DD for invalid cases
  if (getNodeCount() > MaxDistPPSize) {
    setInvalid("Too many stmts to analyze");
  }

  if (getNodeCount() == 1) {
    setInvalid("Single Node Loop cannot be analyzed");
  }
}

} // loopopt
//===--------------------------------------------------------------------===//
// GraphTraits specializations for DistPPGraph. This will allow us to use
// Graph algorithm iterators such as SCCIterator. Must be in same namespace
// as GraphTraits
//===--------------------------------------------------------------------===//
//
template <> struct GraphTraits<DistPPGraph *> {
  typedef DistPPNode NodeType;
  typedef DistPPGraph::children_iterator ChildIteratorType;
  static NodeType *getEntryNode(DistPPGraph *G) { return *(G->node_begin()); }

  static inline ChildIteratorType child_begin(NodeType *N) {
    return N->getGraph()->children_begin(N);
  }
  static inline ChildIteratorType child_end(NodeType *N) {
    return N->getGraph()->children_end(N);
  }

  typedef std::pointer_to_unary_function<DistPPNode *, DistPPNode &> DerefFun;

  // nodes_iterator/begin/end - Allow iteration over all nodes(not node ptrs)
  // in the graph
  typedef mapped_iterator<SmallVectorImpl<DistPPNode *>::iterator, DerefFun>
      nodes_iterator;

  // GraphTraits requires argument to this be a pointer to template argument
  // type
  static nodes_iterator nodes_begin(DistPPGraph **G) {
    return map_iterator((*G)->node_begin(), DerefFun(NodePtrDeref));
  }
  static nodes_iterator nodes_end(DistPPGraph **G) {
    return map_iterator((*G)->node_end(), DerefFun(NodePtrDeref));
  }

  static DistPPNode &NodePtrDeref(DistPPNode *DNode) { return *DNode; }

  static unsigned size(DistPPGraph *G) { return G->getNodeCount(); }
};
} // llvm
