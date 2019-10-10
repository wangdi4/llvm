//===----- HIRLoopDistributionGraph.h - Forms Distribution Graph  ---------===//
//
// Copyright (C) 2015-2019 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
// Provides a DAG of piblocks(PiGraph) which LoopDistribution can analyze for
// distribution points or reductions. A pi block is formally defined as one or
// more hlnodes in a loop which must remain in same loop. A group of hlnodes
// which form an SCC in DDgraph would be a piblock[cannot break cycle without
// violating dd constraints] as would an hlif and its children[cannot split
// it without violation of control flow constraints]. The edges, PiEdges,
// describe the DD constraints amongs pi blocks.
//
// PiGraph is formed from DistPPGraph, which is a condensed form of DD graph.
// See implementation for more details
//
//===----------------------------------------------------------------------===//

#ifndef INTEL_LOOPTRANSFORMS_HIR_LOOP_DIST_GRAPH
#define INTEL_LOOPTRANSFORMS_HIR_LOOP_DIST_GRAPH

#include "HIRLoopDistributionPreProcGraph.h"

namespace llvm {

namespace loopopt {
class PiGraph;
class HIRLoopStatistics;

// Similar to a dist node, this is in a indivisible bit of the program.
// However a piblock is an scc of dist nodes. The scc's should be emitted
// in lexically increasing order in order to preserve incoming program order.
class PiBlock {

public:
  enum class PiBlockType : unsigned char {
    SingleStmt = 0, // piblock is a single hlinst/hlif at top level,
                    // may or may not have self dependence
    MultipleStmt,   // piblock contains multiple stmts
    SingleLoop,     // single loop, may or may not have loop carried deps
    MultipleLoop,   // piblock contains multiple loops, but no stmts
    StmtAndLoop,    // piblock contains multiple stmts and loops
  };

  PiBlock(ArrayRef<DistPPNode *> SCCNodes, PiGraph *G)
      : Graph(G), DistPPNodes(SCCNodes.begin(), SCCNodes.end()) {
    setPiBlockType(SCCNodes);
    // DistNodes within an scc are not guaranteed to be emitted in any
    // particular order. Ensure they are ordered lexically
    std::sort(DistPPNodes.begin(), DistPPNodes.end(),
              [](DistPPNode *A, DistPPNode *B) -> bool {
                return A->getNode()->getTopSortNum() <
                       B->getNode()->getTopSortNum();
              });
  }

  PiGraph *getGraph() { return Graph; }

  SmallVectorImpl<DistPPNode *>::iterator dist_node_begin() {
    return DistPPNodes.begin();
  }

  SmallVectorImpl<DistPPNode *>::iterator dist_node_end() {
    return DistPPNodes.end();
  }

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  LLVM_DUMP_METHOD
  void dump() const;
#endif

  unsigned size() { return DistPPNodes.size(); }

  typedef std::pointer_to_unary_function<DistPPNode *, HLNode *> DistToHNodeFun;

  typedef mapped_iterator<SmallVectorImpl<DistPPNode *>::iterator,
                          DistToHNodeFun>
      nodes_iterator;

  // nodes_iterator/begin/end - Allow iteration over hl nodes in piblock in
  // lexical order
  nodes_iterator nodes_begin() {
    return map_iterator(DistPPNodes.begin(), DistToHNodeFun(DistToHNode));
  }
  nodes_iterator nodes_end() {
    return map_iterator(DistPPNodes.end(), DistToHNodeFun(DistToHNode));
  }

  static HLNode *DistToHNode(DistPPNode *DNode) { return DNode->getNode(); }
  PiBlockType getBlockType() { return BlockType; }

private:
  void setPiBlockType(ArrayRef<DistPPNode *> SCCNodes);
  PiGraph *Graph;
  SmallVector<DistPPNode *, 16> DistPPNodes;
  PiBlockType BlockType;
};

// A PiGraphEdge represents a list of dd edges between PiBlocks.
class PiGraphEdge {
private:
  PiBlock *Src;
  PiBlock *Sink;
  SmallVector<const DDEdge *, 16> DDEdges;

public:
  PiBlock *getSrc() { return Src; }
  PiBlock *getSink() { return Sink; }
  const PiBlock *getSrc() const { return Src; }
  const PiBlock *getSink() const { return Sink; }
  const SmallVector<const DDEdge *, 16> &getDDEdges() const { return DDEdges; }

  PiGraphEdge(PiBlock *Start, PiBlock *End,
              const SmallVectorImpl<const DDEdge *> &EdgeList)
      : Src(Start), Sink(End), DDEdges(EdgeList.begin(), EdgeList.end()) {
    assert(Start && End && "Null src/sink for pi edge");
  }

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  LLVM_DUMP_METHOD
  void dump() const;
#endif
};

// PiGraph is a DAG of piblocks. PiEdges represent a set of dd constraints
// between two piblocks. We often consider a topologically sorted version of
// this graph because that ordering of piblocks is valid if  each pi block is
// split into its own loop.
// Any dd constraint in the piedge must be = or < at loop level of this graph's
// loop.  If it was *, it would imply a cycle, meaning sink/src should be same
// piblock, an invalid graph.
// Because of this property, the pi blocks can be split into loops, and emitted
// as loops in the top. sort order as the dependence src is still executed
// before sink.
// Consider the following
//<28>         + DO i1 = 0, zext.i32.i64(%UB), 1   <DO_LOOP>
//<3>          |   %1 = (@B)[0][i1];
//<5>          |   %2 = (@C)[0][i1];
//<6>          |   %add = %1  +  %2;
//<9>          |   (@MC)[0][i1 + %blob1] = %add;
//<12>         |   %4 = (i32*)(@DC)[0][i1];
//<16>         |   (i32*)(@B)[0][i1 + 1] = %4;
//<17>         |   %6 = (@C)[0][i1];
//<20>         |   %7 = (@MC)[0][i1 + %blob2];
//<21>         |   %add15 = %6  +  %7;
//<22>         |   (@C)[0][i1] = %add15;
//<28>         + END LOOP
// The second SCC is 12,16 and the other is all other stmts. There is an edge
// from
// {12,16} to other scc due to B[][]  < dependence. Once we top sort this dag,
// we have the order {12,16}, {3,5...}
// This approach allows some needless stmt reordering. It may cause
// some reordering of disconnected nodes with no dependences
// do
//    B[i] = 0;
//    C[i] = 0;
//
// This approach may reorder first two stmts for no particular reason.
class PiGraph : public HIRGraph<PiBlock, PiGraphEdge> {
  // top. sorted list of piblocks
  SmallVector<PiBlock *, 64> PiBlocks;
  DenseMap<DistPPNode *, PiBlock *> DistPPNodeToPiBlock;
  DistPPGraph *PPGraph;

public:
  PiGraph(HLLoop *Loop, HIRDDAnalysis &DDA,
          HIRSparseArrayReductionAnalysis &SARA,
          bool ForceCycleForLoopIndepDep,
          bool CreateControlNodes) {

    PPGraph = new DistPPGraph(Loop, DDA, SARA, ForceCycleForLoopIndepDep,
                              CreateControlNodes);

    if (!isGraphValid()) {
      return;
    }

    // Simplify DistPPGraph into PiGraph
    createNodes();
    createEdges();
  }

  void createNodes();

  // Condenses all DistPPGraph edges into a single PiGraphEdge
  void createEdges();

  bool isGraphValid() { return PPGraph->isGraphValid(); }

  std::string getFailureReason() { return PPGraph->getFailureReason(); }

  SmallVectorImpl<PiBlock *>::iterator node_begin() { return PiBlocks.begin(); }
  SmallVectorImpl<PiBlock *>::iterator node_end() { return PiBlocks.end(); }
  unsigned size() { return PiBlocks.size(); }

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  LLVM_DUMP_METHOD void dump() const;
#endif

  virtual ~PiGraph() {
    for (PiBlock *PBlk : PiBlocks) {
      delete PBlk;
    }
    delete PPGraph;
  }

  Optional<std::pair<DistPPNode *, bool>>
  getControlDependence(DistPPNode *Dst) {
    assert(PPGraph && "PPGraph is not initialized");
    return PPGraph->getControlDependence(Dst);
  }

  bool hasControlDependences() const {
    assert(PPGraph && "PPGraph is not initialized");
    return PPGraph->hasControlDependences();
  }

private:
  void addPiBlock(ArrayRef<DistPPNode *> Nodes) {
    PiBlock *CurPiBlock = new PiBlock(Nodes, this);
    for (DistPPNode *DistNode : Nodes) {
      DistPPNodeToPiBlock[DistNode] = CurPiBlock;
    }
    PiBlocks.push_back(CurPiBlock);
  }
};

} // namespace loopopt
//===--------------------------------------------------------------------===//
// GraphTraits specializations for PiGraph. This will allow us to use
// Graph algorithm iterators such as SCCIterator. Must be in same namespace
// as GraphTraits
//===--------------------------------------------------------------------===//
//
template <> struct GraphTraits<loopopt::PiGraph *> {
  typedef loopopt::PiBlock NodeType;

  typedef loopopt::PiGraph::children_iterator ChildIteratorType;
  static NodeType *getEntryNode(loopopt::PiGraph *G) {
    return *(G->node_begin());
  }

  static inline ChildIteratorType child_begin(NodeType *N) {
    return N->getGraph()->children_begin(N);
  }
  static inline ChildIteratorType child_end(NodeType *N) {
    return N->getGraph()->children_end(N);
  }

  typedef std::pointer_to_unary_function<loopopt::PiBlock *, loopopt::PiBlock &>
      DerefFun;

  typedef mapped_iterator<SmallVectorImpl<loopopt::PiBlock *>::iterator,
                          DerefFun>
      nodes_iterator;

  static nodes_iterator nodes_begin(loopopt::PiGraph **G) {
    return map_iterator((*G)->node_begin(), DerefFun(NodePtrDeref));
  }
  static nodes_iterator nodes_end(loopopt::PiGraph **G) {
    return map_iterator((*G)->node_end(), DerefFun(NodePtrDeref));
  }

  static loopopt::PiBlock &NodePtrDeref(loopopt::PiBlock *PNode) {
    return *PNode;
  }

  static unsigned size(loopopt::PiGraph *G) { return G->size(); }
};
} // namespace llvm

#endif
