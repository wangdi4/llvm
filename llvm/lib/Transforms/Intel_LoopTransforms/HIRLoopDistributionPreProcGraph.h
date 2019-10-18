//===----- HIRLoopDistributionGraph.h - Forms Distribution Graph  --------===//
//
// Copyright (C) 2015-2019 Intel Corporation. All rights reserved.
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
//===---------------------------------------------------------------------===//

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

#ifndef INTEL_LOOPTRANSFORMS_HIR_LOOP_DIST_PREPROC_GRAPH
#define INTEL_LOOPTRANSFORMS_HIR_LOOP_DIST_PREPROC_GRAPH

#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/DDGraph.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRDDAnalysis.h"

#include "llvm/ADT/DenseMap.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/AllSCCIterator.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/CanonExprUtils.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/DDRefUtils.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HLNodeUtils.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HLNodeVisitor.h"
#include "llvm/Support/Debug.h"

#include <algorithm>

namespace llvm {

namespace loopopt {

class DistPPGraph;
class HIRSparseArrayReductionAnalysis;

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

class DistPPNode {
  // The HLNode for this dist node. All children hlnodes of this node
  // are represented by this dist node
  HLNode *HNode;
  DistPPGraph *Graph;

  // Indicates that PP node represents HLIf statement only, without its
  // children.
  bool IsControlNode;

public:
  DistPPNode(HLNode *N, DistPPGraph *G)
      : HNode(N), Graph(G), IsControlNode(false) {}

  DistPPGraph *getGraph() const { return Graph; }
  HLNode *getNode() const { return HNode; }

  void setControlNode() {
    assert(isa<HLIf>(getNode()));
    IsControlNode = true;
  }

  bool isControlNode() const { return IsControlNode; }

  bool hasMemRef() const;

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  LLVM_DUMP_METHOD void dump();
  unsigned getNum() const { return getNode()->getNumber(); }
#endif
};

// See specialization in ADT/DenseMapInfo.h for pointers.
template <typename PPNode> struct DenseDistPPNodeMapInfo {
  static_assert(
      std::is_same<DistPPNode, typename std::remove_cv<PPNode>::type>::value,
      "Node must be a CV-qualified DistPPNode");
  typedef PPNode T;
  static inline T *getEmptyKey() {
    uintptr_t Val = static_cast<uintptr_t>(-1);
    Val <<= PointerLikeTypeTraits<T *>::NumLowBitsAvailable;
    return reinterpret_cast<T *>(Val);
  }

  static inline T *getTombstoneKey() {
    uintptr_t Val = static_cast<uintptr_t>(-2);
    Val <<= PointerLikeTypeTraits<T *>::NumLowBitsAvailable;
    return reinterpret_cast<T *>(Val);
  }

  /// Specialized method.
  static unsigned getHashValue(const T *PtrVal) {
    return (unsigned(PtrVal->getNode()->getNumber() >> 0)) ^
           (unsigned(PtrVal->getNode()->getNumber() >> 5));
  }

  static bool isEqual(const T *LHS, const T *RHS) { return LHS == RHS; }
};
} // namespace loopopt

template <>
struct DenseMapInfo<loopopt::DistPPNode *>
    : public loopopt::DenseDistPPNodeMapInfo<loopopt::DistPPNode> {};

template <>
struct DenseMapInfo<const loopopt::DistPPNode *>
    : public loopopt::DenseDistPPNodeMapInfo<const loopopt::DistPPNode> {};

namespace loopopt {
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
             ArrayRef<const DDEdge *> EdgeList = {})
      : Src(DistSrc), Sink(DistSink),
        DDEdges(EdgeList.begin(), EdgeList.end()) {}

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  LLVM_DUMP_METHOD
  void dump() const;
#endif
};

class DistPPGraph : public HIRGraph<DistPPNode, DistPPEdge> {
  void addCycle(DistPPNode *NodeA, DistPPNode *NodeB);
  void constructUnknownSideEffectEdges(ArrayRef<DistPPNode *> UnsafeNodes);

public:
  // Marks graph as invalid for given reason
  // Possible failures could be too many nodes, edges etc
  void setInvalid(StringRef FailureReason) {
    GraphValidity = false;
    FailureString = FailureReason;
  }

  unsigned getNodeCount() { return DistPPNodeList.size(); }

  bool isGraphValid() { return GraphValidity; }
  std::string getFailureReason() { return FailureString; }

  SmallVectorImpl<DistPPNode *>::iterator node_begin() {
    return DistPPNodeList.begin();
  };

  SmallVectorImpl<DistPPNode *>::iterator node_end() {
    return DistPPNodeList.end();
  }

  DenseMap<HLNode *, DistPPNode *> &getNodeMap() { return HLToDistPPNodeMap; }

  DistPPGraph(HLLoop *Loop, HIRDDAnalysis &DDA,
              HIRSparseArrayReductionAnalysis &SARA,
              bool ForceCycleForLoopIndepDep, bool CreateControlNodes);

  // TODO destruction needs to be handled carefully if we want
  // to reuse graph from inner loop dist in outer loop distribution
  virtual ~DistPPGraph() {
    for (DistPPNode *Node : DistPPNodeList) {
      delete Node;
    }
  }

  void addNode(DistPPNode *NewNode) { DistPPNodeList.push_back(NewNode); }

  void addControlDependence(DistPPNode *Src, DistPPNode *Dst, bool Branch) {
    assert(ControlDeps.count(Dst) == 0 && "Duplicate control dependence found");
    ControlDeps[Dst] = std::make_pair(Src, Branch);
  }

  Optional<std::pair<DistPPNode *, bool>>
  getControlDependence(DistPPNode *Dst) {
    auto Iter = ControlDeps.find(Dst);
    if (Iter == ControlDeps.end()) {
      return {};
    }

    return Iter->second;
  }

  bool hasControlDependences() const {
    return !ControlDeps.empty();
  }

private:
  // unlike other hirgraphs, this one actually owns the memory for its nodes
  // Special note, the dist nodes(well more precisely DistPPNode->hnode) in this
  // list are in lexical order as dist nodes are created by a lexical walk
  // of hlnodes
  SmallVector<DistPPNode *, 36> DistPPNodeList;

  DenseMap<HLNode *, DistPPNode *> HLToDistPPNodeMap;

  // Control dependencies between: S1 -> (S2, True/False branch)
  // Means that S1 statement is control dependent on S2 statement's true or
  // false branch.
  DenseMap<DistPPNode *, std::pair<DistPPNode *, bool>> ControlDeps;

  std::string FailureString;
  bool GraphValidity = true;
};
} // namespace loopopt

//===--------------------------------------------------------------------===//
// GraphTraits specializations for DistPPGraph. This will allow us to use
// Graph algorithm iterators such as SCCIterator. Must be in same namespace
// as GraphTraits
//===--------------------------------------------------------------------===//
//
template <> struct GraphTraits<loopopt::DistPPGraph *> {
  typedef loopopt::DistPPNode *NodeRef;
  typedef loopopt::DistPPGraph::children_iterator ChildIteratorType;
  static NodeRef getEntryNode(loopopt::DistPPGraph *G) {
    return *(G->node_begin());
  }

  static inline ChildIteratorType child_begin(NodeRef N) {
    return N->getGraph()->children_begin(N);
  }
  static inline ChildIteratorType child_end(NodeRef N) {
    return N->getGraph()->children_end(N);
  }

  typedef std::pointer_to_unary_function<loopopt::DistPPNode *,
                                         loopopt::DistPPNode &>
      DerefFun;

  // nodes_iterator/begin/end - Allow iteration over all nodes(not node ptrs)
  // in the graph
  typedef mapped_iterator<SmallVectorImpl<loopopt::DistPPNode *>::iterator,
                          DerefFun>
      nodes_iterator;

  // GraphTraits requires argument to this be a pointer to template argument
  // type
  static nodes_iterator nodes_begin(loopopt::DistPPGraph **G) {
    return map_iterator((*G)->node_begin(), DerefFun(NodePtrDeref));
  }
  static nodes_iterator nodes_end(loopopt::DistPPGraph **G) {
    return map_iterator((*G)->node_end(), DerefFun(NodePtrDeref));
  }

  static loopopt::DistPPNode &NodePtrDeref(loopopt::DistPPNode *DNode) {
    return *DNode;
  }

  static unsigned size(loopopt::DistPPGraph *G) { return G->getNodeCount(); }
};
} // namespace llvm

#endif
