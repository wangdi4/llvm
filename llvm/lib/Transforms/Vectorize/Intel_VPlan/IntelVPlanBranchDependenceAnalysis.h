//===------------------------------------------------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file defines the VPlanBranchDependenceAnalysis class. Branch dependence
/// analysis is used by divergence analysis to identify divergent PHI nodes.
/// This is accomplished by determining if there are a minimum of two disjoint
/// control flow paths from a VPBasicBlock containing a VPCondBitVPVal to a
/// corresponding VPBasicBlock containing a PHI node, where both VPBasicBlocks
/// are within the same SESE region.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_VPLAN_BRANCH_DEPENDENCE_ANALYSIS_H
#define LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_VPLAN_BRANCH_DEPENDENCE_ANALYSIS_H

#include "IntelVPlan.h"
#include "IntelVPlanLoopInfo.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/SmallPtrSet.h"

namespace llvm {

namespace vpo {

class VPBasicBlock;
class VPRegionBlock;

using ConstBlockSet = SmallPtrSet<const VPBasicBlock *, 4>;

// The DivPasthDecide class implements queries for disjoint paths in the CFG.
// The BranchDependenceAnalysis relies on this information to infer
// control-induced divergence in phi nodes, including LCSSA phis.
class DivPathDecider {

public:
  DivPathDecider() = default;
  ~DivPathDecider() {}

  /// Determines if \p From, a block containing a conditional branch, causes
  /// divergent control flow through the \p LoopExit. i.e., there are multiple
  /// exit paths out of the loop.
  bool inducesDivergentExit(const VPBasicBlock &From,
                            const VPBasicBlock &LoopExit,
                            const VPLoop &TheLoop) const;

  /// Find N node-divergent paths from A to B, return true iff successful.
  bool findDisjointPaths(const VPBasicBlock &From, const VPBasicBlock &To,
                         unsigned N = 2U) const;

  /// A node within a graph representing the possible control flow paths that
  /// can be reached from a divergent branch. The type indicates the flow
  /// direction of the edge coming into the node.
  struct Node {
    enum SplitType { IN = 0, OUT = 1 } Type;
    const VPBlockBase &BB;
    Node(SplitType Type, const VPBlockBase &BB) : Type(Type), BB(BB) {}
  };

  using NodeList = SmallVector<const Node *, 8>;
  using Edge = std::pair<const Node *, const Node *>;
  using EdgeSet = DenseSet<Edge>;
  using PredecessorMap = DenseMap<const Node *, const Node *>;

private:
  mutable DenseMap<const VPBlockBase *, Node> InNodes, OutNodes;

  bool findDisjointPaths(const Node &Source, const NodeList &Sinks, unsigned N,
                         const VPLoop *TheLoop) const;

  /// Finds a path from the \p Source node to one of the \p Sink nodes, of
  /// which any edge has non-positive \p Flow. The \p Parent map stores the
  /// predecessor of each node. An optional loop \p TheLoop can be given to
  /// compute the possible flow through a loop exit.
  const Node *findPath(const Node &Source, const NodeList &Sinks,
                       const EdgeSet &Flow, PredecessorMap &Parent,
                       const VPLoop *TheLoop) const;

  /// Takes a path description and adjusts the \p Flow for every edge in it.
  /// The start of the path is \p Start and the end of the path is \p End.
  /// The map \p Parent stores the predecessor for each node N in the path.
  /// The network flow is \p Flow, where N != Start.
  void injectFlow(const Node &Start, const Node &End,
                  const PredecessorMap &Parent, EdgeSet &Flow) const;

  const Node *getInNode(const VPBlockBase &BB) const;
  const Node *getOutNode(const VPBlockBase &BB) const;
};

class VPlanBranchDependenceAnalysis {

private:
  mutable DivPathDecider DPD;

  VPBlockBase *RegionEntry;
  const VPDominatorTree *DomTree;
  const VPPostDominatorTree *PostDomTree;
  const VPLoopInfo *VPLI;

  mutable DenseMap<const VPBasicBlock *, ConstBlockSet> JoinBlocks;

public:
  VPlanBranchDependenceAnalysis(VPBlockBase *RegionEntry,
                                const VPDominatorTree *DT,
                                const VPPostDominatorTree *PDT,
                                const VPLoopInfo *VPLI);

  // Returns the set of blocks whose PHI nodes become divergent if the
  // condition of /p TermBlock results in a divergent branch.
  const ConstBlockSet &joinBlocks(VPBasicBlock *TermBlock) const;
};
} // namespace vpo
} // namespace llvm
#endif
