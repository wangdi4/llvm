//===---- HIRDDAnalysis.h - Provides Data Dependence Analysis --*-- C++--*-===//
//
// Copyright (C) 2015-2019 Intel Corporation. All rights reserved.
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
#include "llvm/IR/PassManager.h"
#include "llvm/Pass.h"

#include "llvm/Analysis/AliasAnalysis.h"

#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/DDGraph.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRAnalysisPass.h"

#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HLNodeVisitor.h"

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

class RefinedDependence {
  DirectionVector DV;
  DistanceVector DistV;
  bool Refined;
  bool Independent;

public:
  RefinedDependence() : Refined(false), Independent(false) {}

  DirectionVector &getDV() { return DV; }

  DistanceVector &getDist() { return DistV; }

  const DirectionVector &getDV() const { return DV; }

  const DistanceVector &getDist() const { return DistV; }

  bool isRefined() const { return Refined; }

  bool isIndependent() const { return Independent; }

  void setIndependent() { Independent = true; }

  void setRefined() { Refined = true; }

  LLVM_DUMP_METHOD
  void print(raw_ostream &OS) const {
    if (!isIndependent()) {
      DV.print(OS, false);
      OS << " ";
      DistV.print(OS, DV.getLastLevel());
    }

    OS << "< ";
    if (isRefined()) {
      OS << "refined ";
    }
    if (isIndependent()) {
      OS << "independent ";
    }
    OS << ">";
  }

  LLVM_DUMP_METHOD
  void dump() const {
    print(dbgs());
    dbgs() << "\n";
  }
};

class HIRDDAnalysis : public HIRAnalysis {
  // GraphState initializes to NoData by default.
  enum class GraphState : unsigned char {
    NoData,
    Invalid,
    Valid,
  };

  typedef DenseMap<const HLNode *, GraphState> ValidationMapTy;

  /// The HLNode visitor that recursively marks HLNodes as invalid.
  class GraphStateUpdater final : public HLNodeVisitorBase {
    ValidationMapTy &ValidityMapRef;
    GraphState State;
    const HLLoop *SkipLoop;

  public:
    GraphStateUpdater(HIRDDAnalysis::ValidationMapTy &ValidityMapRef,
                      HIRDDAnalysis::GraphState State)
        : ValidityMapRef(ValidityMapRef), State(State), SkipLoop(nullptr) {}

    void visit(const HLLoop *Loop) {
      ValidityMapRef[Loop] = State;
      if (Loop->isInnermost()) {
        SkipLoop = Loop;
      }
    }

    void visit(const HLRegion *Region) { ValidityMapRef[Region] = State; }

    bool skipRecursion(const HLNode *Node) const { return SkipLoop == Node; }

    void visit(const HLNode *Node) {}
    void postVisit(const HLNode *Node) {}
  };

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

public:
  HIRDDAnalysis(HIRFramework &HIRF, AAResults *AAR);
  HIRDDAnalysis(HIRDDAnalysis &&Arg)
      : HIRAnalysis(Arg.HIRF), AAR(std::move(Arg.AAR)),
        ValidationMap(std::move(Arg.ValidationMap)),
        RegionDDGraph(std::move(Arg.RegionDDGraph)) {}
  HIRDDAnalysis(const HIRDDAnalysis &) = delete;

  void printAnalysis(raw_ostream &OS) const override;

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
  DDGraph getGraph(const HLRegion *Region) {
    return getGraphImpl(Region, Region);
  }

  DDGraph getGraph(const HLLoop *Loop) {
    return getGraphImpl(Loop->getParentRegion(), Loop);
  }

  /// \brief Caller has DDG and the level it needs to refine. Should check
  /// isRefinable before calling refineDV
  bool isRefinableDepAtLevel(const DDEdge *Edge, unsigned Level) const;

  /// \brief Refine DV by calling demand driven DD.
  /// e.g. If we are testing for Vectorization for outer loop level 4
  ///  in a 5 level Loop,  Start nest = 4, Deepest nest = 5
  ///  The input DV will be set as  * from Start to Deepest
  ///  Input DV for DD in this case is (= = = * *).
  ///  When ForFusion is true, DD assumes both references are inside the
  ///  deepest nesting.

  RefinedDependence refineDV(const DDRef *SrcDDRef, const DDRef *DstDDRef,
                             unsigned StartNestingLevel,
                             unsigned DeepestNestingLevel,
                             bool ForFusion) const;

  /// Return true if \p SrcRef and \p DstRef alias based on the metadata/base
  /// info.
  /// Alias analyses are invoked on demand.
  /// Both the refs are supposed to be memrefs.
  bool doRefsAlias(const RegDDRef *SrcRef, const RegDDRef *DstRef) const;

  /// Forces DDG build for verification purposes.
  void forceBuild();

  // TODO still needed? Call findDependences directly?
  // bool demandDrivenDD(DDRef* SrcRef, DDRef* SinkRef,
  //  DirectionVector* input_dv, DirectionVector* output_dv);

  // init_incremental_rebuild(HLNode*)
private:
  std::unique_ptr<AAResults> AAR;

  ValidationMapTy ValidationMap;

  // Region Data Dependency Graph
  std::unordered_map<const HLRegion *, DDGraphTy> RegionDDGraph;

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

  DDGraph getGraphImpl(const HLRegion *Region, const HLNode *Node);

  void buildGraph(DDGraphTy &DDG, const HLNode *Node);

  void setInputDV(DirectionVector &DV, HLNode *Node, DDRef *Ref1, DDRef *Ref2);
};

class HIRDDAnalysisWrapperPass : public FunctionPass {
  std::unique_ptr<HIRDDAnalysis> DDA;

public:
  static char ID;
  HIRDDAnalysisWrapperPass() : FunctionPass(ID) {}

  bool runOnFunction(Function &F) override;
  void getAnalysisUsage(AnalysisUsage &AU) const override;
  void releaseMemory() override;

  void print(raw_ostream &OS, const Module * = nullptr) const override {
    getDDA().printAnalysis(OS);
  }

  HIRDDAnalysis &getDDA() { return *DDA; }
  const HIRDDAnalysis &getDDA() const { return *DDA; }
};

class HIRDDAnalysisPass : public AnalysisInfoMixin<HIRDDAnalysisPass> {
  friend struct AnalysisInfoMixin<HIRDDAnalysisPass>;

  static AnalysisKey Key;

public:
  using Result = HIRDDAnalysis;

  HIRDDAnalysis run(Function &F, FunctionAnalysisManager &AM);
};

class HIRDDAnalysisPrinterPass
    : public PassInfoMixin<HIRDDAnalysisPrinterPass> {
  raw_ostream &OS;

public:
  explicit HIRDDAnalysisPrinterPass(raw_ostream &OS) : OS(OS) {}

  PreservedAnalyses run(Function &F, FunctionAnalysisManager &AM) {
    AM.getResult<HIRDDAnalysisPass>(F).printAnalysis(OS);
    return PreservedAnalyses::all();
  }
};

} // namespace loopopt
} // namespace llvm

#endif
