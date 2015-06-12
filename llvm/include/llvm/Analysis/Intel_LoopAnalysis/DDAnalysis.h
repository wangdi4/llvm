//===------ DDAnalysis.h - Provides Data Dependence Analysis --*-- C++ --*-===//
//
// Copyright (C) 2015 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
// The primary purpose of this pass is to provide a lazily evaluated data
// dependence graph for HIR. Clients
// specify the the HLNode for which a DD graph is required.
// We try to avoid recomputation whenever possible, even if the HIR has been
// been modified. In order to do this, clients must specify how they modify HIR
// at the region/loop level. See the mark*Modified functions for more details
//
//===----------------------------------------------------------------------===//

#ifndef INTEL_LOOPANALYSIS_DDA
#define INTEL_LOOPANALYSIS_DDA

#include "llvm/Pass.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/Analysis/Intel_LoopAnalysis/DDGraph.h"
#include <list>

namespace llvm {
class Function;
namespace loopopt {

class DDRef;
class HLNode;
class HLRegion;
class HLLoop;
class HIRParser;
class SymbaseAssignment;
class DirectionVector;
class DDGraph;

typedef std::map<unsigned int, std::vector<llvm::loopopt::DDRef *>> SymToRefs;

/// \brief TODO
class DDAnalysis : public FunctionPass {
public:
  DDAnalysis() : FunctionPass(ID) {}
  static char ID;
  bool runOnFunction(Function &F) override;

  void getAnalysisUsage(AnalysisUsage &AU) const;

  // \brief Marks a loop body as modified, causing DD to rebuild the graph
  // for this loop and its children. This should be done when modifying the
  // canon expr of a ddref in the loop, or adding/removing a ddref. This
  // invalidates the graph for this loop, and any children loops.
  // If modifying loop bounds, call markLoopBoundsModified instead.
  void markLoopBodyModified(HLLoop *L);

  // \brief Indicates to DDA that the bounds for this loop have been modified.
  // Changing the loop bounds can have a more destructive effect on the ddgraph
  // as it may affect parent loops as well as children loops. Example:

  // do i 1, N
  //  do j 1, N
  //    a[i][j] = ...
  //    a[i+1][4] = ...
  // In this case, there is a dependence carried by i loop. However, changing
  // j bounds to 1 would make the references independent at all levels.
  // Thus, changing bounds invalidates graph for enclosing loop nest as well as
  // child loops
  void markLoopBoundsModified(HLLoop *L);

  // \brief Indicates to DDA that the refs at the topmost region level have
  // been modified. Ie a ref outside any loop nest has been modified.
  // Logically indicates that any graph for a loop nest is still ok,
  // but the region graph is now invalid
  // For example, PRE hoists out a ref from loop nest. That loop nest is
  // invalid and whole region graph must be rebuilt for out of loop edges.
  // However the other loop nest's graph is still valid
  // TODO better name
  void markTopLvlNonLoopNodeModified(HLRegion *R);

  // TODO needed for incremental rebuild if and when supported
  // markDDRefModified

  // \brief Returns the DD graph for the HLNode. For regions, this returns
  // the full dd graph. For loops, this returns the graph with dependencies
  // carried at levels >= than that of the loop nest. That is, we
  // assume IV of outer loops to be invariant in specified loop nest(To be
  // pedantic, DD testing assumes = for the outer loop levels )
  // do i
  //  do j
  //    a[i][j] = ...
  //    a[i+1][j] = ...
  // Thus, if we get graph for j loop, we will determine no dependence as
  // the first subscript is clearly distinct if we are in the same iteration
  // of i loop.
  //
  // This call may either recompute the graph if it is invalid, or simply return
  // it if still valid. Perform any legality checks possible before getting
  // graph to avoid expensive recomputation.
  // Note, atm the graph does not filter edges to ensure src/sink are in Node.
  // some edges may be pointing to a node that is not of interest
  DDGraph getGraph(HLNode *Node, bool InputEdgesReq = false);

  // TODO still needed? Call findDependences directly?
  // bool demandDrivenDD(DDRef* SrcRef, DDRef* SinkRef,
  //  DirectionVector* input_dv, DirectionVector* output_dv);

  // \brief Returns a new unused symbase ID.
  unsigned int getNewSymBase();
  void releaseMemory() override;
  // TODO
  // void print(raw_stream &OS, const Module* = nullptr) const override;
  // void verifyAnalysis() const override;
  //

  // init_incremental_rebuild(HLNode*)
private:
  Function *F;
  HIRParser *HIRP;
  SymbaseAssignment *SA;

  DenseMap<HLNode *, bool> GraphValidityMap;

  bool graphForNodeValid(HLNode *Node);

  void rebuildGraph(HLNode *Node, bool BuildInputEdges = false);

  // full dd graph
  DDGraphTy FunctionDDGraph;

  bool edgeNeeded(DDRef *Ref1, DDRef *Ref2, bool InputEdgesReq);
  DirectionVector getInputDV(HLNode *Node, DDRef *Ref1, DDRef *Ref2);
  void dumpSymBaseMap(SymToRefs &RefMap);
};

// DDAnalysis returns instances of this to ensure clients can access graph,
// but not modify it. Convenient places to reimplement iterators or filter
// graph as well
class DDGraph {
private:
  HLNode *CurNode;
  DDGraphTy *G;

public:
  // TODO friend to DDA?
  DDGraph(HLNode *Node, DDGraphTy *Graph) : CurNode(Node), G(Graph) {}

  std::vector<DDEdge>::const_iterator incoming_edges_begin(DDRef *Ref) {
    return G->incoming_edges_begin(Ref);
  }
  std::vector<DDEdge>::const_iterator incoming_edges_end(DDRef *Ref) {
    return G->incoming_edges_end(Ref);
  }
  std::vector<DDEdge>::const_iterator outgoing_edges_begin(DDRef *Ref) {
    return G->outgoing_edges_begin(Ref);
  }
  std::vector<DDEdge>::const_iterator outgoing_edges_end(DDRef *Ref) {
    return G->outgoing_edges_end(Ref);
  }

  // todo
  void dump() { G->dump(); }
};
}
}

#endif
