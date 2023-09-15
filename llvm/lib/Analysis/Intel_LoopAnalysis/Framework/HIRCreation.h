//===----------- HIRCreation.h - Creates HIR nodes ------------*-- C++ --*-===//
//
// Copyright (C) 2015 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// This analysis is used to create HIR nodes out of identified HIR regions.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_ANALYSIS_INTEL_LOOPANALYSIS_HIRCREATION_H
#define LLVM_ANALYSIS_INTEL_LOOPANALYSIS_HIRCREATION_H

#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/ADT/SmallVector.h"

#include "llvm/IR/Dominators.h"
#include "llvm/Analysis/Intel_LoopAnalysis/IR/HLNode.h"

namespace llvm {

class Function;
class BasicBlock;
class SwitchInst;
class DominatorTree;
class PostDominatorTree;
class LoopInfo;
class Loop;
class BranchInst;

namespace loopopt {

class HLRegion;
class HLLabel;
class HLGoto;
class HLIf;
class HLSwitch;
class HIRRegionIdentification;
class HLNodeUtils;

/// This analysis creates and populates HIR regions with HLNodes using
/// the information provided by HIRRegionIdentification pass.
///
class HIRCreation {
  // HIRCleanup accesses Gotos and Labels populated by this pass.
  friend class HIRCleanup;

  // Analysis results
  DominatorTree &DT;
  PostDominatorTree &PDT;
  LoopInfo &LI;
  HIRRegionIdentification &RI;
  HLNodeUtils &HNU;

  /// Points to the region being processed.
  HLRegion *CurRegion;

  /// HLLabel map to be used by later passes.
  DenseMap<const BasicBlock *, HLLabel *> Labels;

  /// HLGotos vector to be used by later passes.
  SmallVector<HLGoto *, 64> Gotos;

  /// HLIfs map to be used by later passes.
  DenseMap<HLIf *, const BasicBlock *> Ifs;

  /// HLSwitches map to be used by later passes.
  SmallDenseMap<HLSwitch *, const BasicBlock *, 8> Switches;

  /// Maps loops to their early exits.
  SmallDenseMap<Loop *, SmallVector<BasicBlock *, 4>, 16> EarlyExits;

  /// Creates HLNodes corresponding to the terminator of the basic block.
  HLNode *populateTerminator(BasicBlock *BB, HLNode *InsertionPos);

  /// Creates HLNodes for the instructions in the basic block.
  HLNode *populateInstSequence(BasicBlock *BB, HLNode *InsertionPos);

  /// Helper to populate \p EndBBs before calling isReachableFrom().
  void populateEndBBs(const BasicBlock *BB,
                      SmallPtrSet<const BasicBlock *, 2> &EndBBs) const;

  /// Returns true if \p SuccessorBB is cross linked to (reachable from) the
  /// other successor of \p BI.
  bool isCrossLinked(const BranchInst *BI, const BasicBlock *SuccessorBB) const;

  /// Returns true if \p SuccessorBB is cross linked to (reachable from) any of
  /// the other successors of \p SI.
  bool isCrossLinked(const SwitchInst *SI, const BasicBlock *SuccessorBB) const;

  /// Sorts the dominator children of Node in reverse lexical order.
  /// Returns true/false based on whether any dominator children belong to
  /// 'CurRegion'.
  bool sortDomChildren(DomTreeNode *Node,
                       SmallVectorImpl<BasicBlock *> &SortedChildren) const;

  /// Performs lexical (preorder) walk of the dominator tree for the
  /// region.
  /// Returns the last HLNode for the current sub-tree.
  HLNode *doPreOrderRegionWalk(BasicBlock *BB, HLNode *InsertionPos);

public:
  HIRCreation(DominatorTree &DT, PostDominatorTree &PDT, LoopInfo &LI,
              HIRRegionIdentification &RI, HLNodeUtils &HNU)
      : DT(DT), PDT(PDT), LI(LI), RI(RI), HNU(HNU) {}

  /// Creates HLRegions out of IRRegions.
  void run(HLContainerTy &Regions);

  /// Returns the src bblock associated with this if. Asserts if it cannot be
  /// found.
  const BasicBlock *getSrcBBlock(HLIf *If) const;

  /// Sets the src bblock for \p If. This should only be done for new ifs
  /// created by the framework.
  void setSrcBBlock(HLIf *If, const BasicBlock *SrcBB);

  /// Returns the src bblock associated with this switch. Asserts if it cannot
  /// be found.
  const BasicBlock *getSrcBBlock(HLSwitch *Switch) const;

  /// Returns HIRRegionIdentification object.
  const HIRRegionIdentification &getRI() const { return RI; }
};

} // End namespace loopopt

} // End namespace llvm

#endif
