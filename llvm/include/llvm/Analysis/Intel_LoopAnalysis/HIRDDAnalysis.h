//===---- HIRDDAnalysis.h - Provides Data Dependence Analysis --*-- C++--*-===//
//
// Copyright (C) 2015-2016 Intel Corporation. All rights reserved.
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

#ifndef INTEL_LOOPANALYSIS_HIR_DD_ANALYSIS
#define INTEL_LOOPANALYSIS_HIR_DD_ANALYSIS

#include "llvm/ADT/DenseMap.h"
#include "llvm/Pass.h"

#include "llvm/Analysis/AliasAnalysis.h"

#include "llvm/Analysis/Intel_LoopAnalysis/DDGraph.h"
#include "llvm/Analysis/Intel_LoopAnalysis/HIRAnalysisPass.h"

#include "llvm/Transforms/Intel_LoopTransforms/Utils/HLNodeVisitor.h"

#include <list>

namespace llvm {
class Function;
namespace loopopt {

class DDRef;
class HLNode;
class HLRegion;
class HLDDNode;
class HLLoop;
class HIRFramework;
class HIRLoopStatistics;
struct DirectionVector;

enum DDVerificationLevel {
  Region = 0,
  L1,
  L2,
  L3,
  L4,
  L5,
  L6,
  L7,
  L8,
  L9,
  Innermost
};

// HIRDDAnalysis returns instances of this to ensure clients can access graph,
// but not modify it. Convenient places to reimplement iterators or filter
// graph as well
class DDGraph {
private:
  const HLNode *CurNode;
  DDGraphTy *G;

public:
  class DDGraphFilter {
    unsigned FirstChildNum;
    unsigned LastChildNum;
    bool IsIncoming;

    template <typename NodeTy> void init(const NodeTy *Node) {
      FirstChildNum = Node->getFirstChild()->getMinTopSortNum();
      LastChildNum = Node->getLastChild()->getMaxTopSortNum();
    }

  public:
    DDGraphFilter(const HLNode *Node, bool IsIncoming)
        : IsIncoming(IsIncoming) {
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

private:
  DDGraphFilter IncomingFilter;
  DDGraphFilter OutgoingFilter;

  const DDGraphTy *getGraphImpl() const {
    assert(G && "Trying to iterate over uninitialized graph!");
    return G;
  }

public:
  using FilterEdgeIterator =
      filter_iterator<DDGraphTy::EdgeIterator, DDGraphFilter>;

  DDGraph()
      : CurNode(nullptr), G(nullptr), IncomingFilter(nullptr, true),
        OutgoingFilter(nullptr, false) {}

  DDGraph(const HLNode *Node, DDGraphTy *Graph)
      : CurNode(Node), G(Graph), IncomingFilter(Node, true),
        OutgoingFilter(Node, false) {}

  iterator_range<FilterEdgeIterator> incoming(const DDRef *Ref) const {
    return make_filter_range(
        llvm::make_range(getGraphImpl()->incoming_edges_begin(Ref),
                         getGraphImpl()->incoming_edges_end(Ref)),
        IncomingFilter);
  }

  iterator_range<FilterEdgeIterator> outgoing(const DDRef *Ref) const {
    return make_filter_range(
        llvm::make_range(getGraphImpl()->outgoing_edges_begin(Ref),
                         getGraphImpl()->outgoing_edges_end(Ref)),
        OutgoingFilter);
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
  /// Single edge going out of this DDRef.
  bool singleEdgeGoingOut(const DDRef *LRef);

  void print(raw_ostream &OS) const;

  void dump() const { print(dbgs()); }
};

class HIRDDAnalysis final : public HIRAnalysisPass {
public:
  HIRDDAnalysis() : HIRAnalysisPass(ID, HIRAnalysisPass::HIRDDAnalysisVal) {}
  static char ID;
  bool runOnFunction(Function &F) override;
  void print(raw_ostream &OS, const Module * = nullptr) const override;

  void getAnalysisUsage(AnalysisUsage &AU) const override;

  // \brief Marks a loop body as modified, causing DD to rebuild the graph
  // for this loop and its children. This should be done when modifying the
  // canon expr of a ddref in the loop, or adding/removing a ddref. This
  // invalidates the graph for this loop, and any children loops.
  // If modifying loop bounds, call markLoopBoundsModified instead.
  void markLoopBodyModified(const HLLoop *L) override;

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
  void markLoopBoundsModified(const HLLoop *L) override;

  // \brief Indicates to DDA that the refs at the topmost region level have
  // been modified. Ie a ref outside any loop nest has been modified.
  // Logically indicates that any graph for a loop nest is still ok,
  // but the region graph is now invalid
  // For example, PRE hoists out a ref from loop nest. That loop nest is
  // invalid and whole region graph must be rebuilt for out of loop edges.
  // However the other loop nest's graph is still valid
  // TODO better name
  void markNonLoopRegionModified(const HLRegion *R) override;

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
  DDGraph getGraph(const HLRegion *Region, bool InputEdgesReq = false) {
    return getGraphImpl(static_cast<const HLNode *>(Region), InputEdgesReq);
  }

  DDGraph getGraph(const HLLoop *Loop, bool InputEdgesReq = false) {
    return getGraphImpl(static_cast<const HLNode *>(Loop), InputEdgesReq);
  }

  /// \brief Refine DV by calling demand driven DD. Return true when RefineDV
  /// is set.
  bool refineDV(DDRef *SrcDDRef, DDRef *DstDDRef,
                unsigned InnermostNestingLevel, unsigned OutermostNestingLevel,
                DirectionVector &RefinedDV, DistanceVector &RefinedDistV,
                bool *IsIndependent);

  // TODO still needed? Call findDependences directly?
  // bool demandDrivenDD(DDRef* SrcRef, DDRef* SinkRef,
  //  DirectionVector* input_dv, DirectionVector* output_dv);

  // \brief Returns a new unused symbase ID.
  unsigned getNewSymbase();
  void releaseMemory() override;

  void verifyAnalysis() const override;

  /// \brief Method for supporting type inquiry through isa, cast, and dyn_cast.
  static bool classof(const HIRAnalysisPass *AP) {
    return AP->getHIRAnalysisID() == HIRAnalysisPass::HIRDDAnalysisVal;
  }

  // TODO
  // void print(raw_stream &OS, const Module* = nullptr) const override;
  //

  // init_incremental_rebuild(HLNode*)
private:
  Function *F;
  std::unique_ptr<AAResults> AAR;
  HIRFramework *HIRF;
  HIRLoopStatistics *HLS;

  // GraphState initializes to NoData by default.
  enum class GraphState : unsigned char {
    NoData,
    Invalid,
    Valid,
  };

  DenseMap<const HLNode *, GraphState> ValidationMap;

  /// The HLNode visitor that recursively marks HLNodes as invalid.
  class GraphStateUpdater;

  /// Returns tuple where the first value is a parent Loop or Region for \p Ref
  /// and the second is true or false whether the parent node is HLLoop.
  static std::tuple<const HLNode *, bool>
  getDDRefRegionLoopContainer(const DDRef *Ref);

  /// Returns true if the nodes between \p RefParent1 and \p RefParent2 are
  /// still valid and should not be constructed again.
  bool isEdgeValid(const DDRef *Ref1, const DDRef *Ref2);

  /// Marks every incoming or outgoing DD edge associated with the \p Loop
  /// as invalid.
  void invalidateGraph(const HLLoop *Loop, bool InvalidateInnerLoops);

  /// Returns true if the graph for the Node is already constructed and valid.
  bool graphForNodeValid(const HLNode *Node);

  DDGraph getGraphImpl(const HLNode *Node, bool InputEdgesReq = false);

  void buildGraph(const HLNode *Node, bool BuildInputEdges);

  // TODO: consider per-region graph instead of per-function graph.
  // full dd graph
  DDGraphTy FunctionDDGraph;

  bool edgeNeeded(DDRef *Ref1, DDRef *Ref2, bool InputEdgesReq);
  void setInputDV(DirectionVector &DV, HLNode *Node, DDRef *Ref1, DDRef *Ref2);

  // Used to rebuild graphs for node/regions based on cl options
  // in DDA's runonPass for verification purposes.
  class GraphVerifier final : public HLNodeVisitorBase {
  private:
    HIRDDAnalysis *CurDDA;
    DDVerificationLevel CurLevel;

  public:
    GraphVerifier(HIRDDAnalysis *DDA, DDVerificationLevel Level)
        : CurDDA(DDA), CurLevel(Level) {}

    void visit(HLRegion *Region);

    void visit(HLLoop *Loop);

    void visit(HLNode *Node) {}
    void postVisit(HLNode *Node) {}
  };
};
}
}

#endif
