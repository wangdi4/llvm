//===----------- HIRCreation.h - Creates HIR nodes ------------*-- C++ --*-===//
//
// Copyright (C) 2015-2016 Intel Corporation. All rights reserved.
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

#include "llvm/Pass.h"

#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/SmallVector.h"

#include "llvm/IR/Dominators.h"

#include "llvm/IR/Intel_LoopIR/HLNode.h"

namespace llvm {

class Function;
class SwitchInst;
class DominatorTree;
struct PostDominatorTree;
class LoopInfo;

namespace loopopt {

class HLRegion;
class HLLabel;
class HLGoto;
class HLIf;
class HLSwitch;
class HIRRegionIdentification;

/// \brief This analysis creates and populates HIR regions with HLNodes using
/// the information provided by HIRRegionIdentification pass. 
///
class HIRCreation : public FunctionPass {
public:
  /// Iterators to iterate over regions
  typedef HLContainerTy::iterator iterator;
  typedef HLContainerTy::const_iterator const_iterator;
  typedef HLContainerTy::reverse_iterator reverse_iterator;
  typedef HLContainerTy::const_reverse_iterator const_reverse_iterator;

private:
  // HIRCleanup accesses Gotos and Labels populated by this pass.
  friend class HIRCleanup;

  /// Regions - HLRegions formed out of incoming LLVM IR.
  HLContainerTy Regions;

  /// DT - The dominator tree.
  DominatorTree *DT;

  /// PDT - The post-dominator tree.
  PostDominatorTree *PDT;

  /// LI - The loop information for the function being processed.
  LoopInfo *LI;

  /// RI - The region identification pass.
  HIRRegionIdentification *RI;

  /// CurRegion - Points to the region being processed.
  HLRegion *CurRegion;

  /// Labels - HLLabel map to be used by later passes.
  SmallDenseMap<const BasicBlock *, HLLabel *, 64> Labels;

  /// Gotos - HLGotos vector to be used by later passes.
  SmallVector<HLGoto *, 64> Gotos;

  /// Ifs - HLIfs map to be used by later passes.
  SmallDenseMap<HLIf *, const BasicBlock *, 32> Ifs;

  /// Switches - HLSwitches map to be used by later passes.
  SmallDenseMap<HLSwitch *, const BasicBlock *, 8> Switches;

  /// \brief Creates HLNodes corresponding to the terminator of the basic block.
  HLNode *populateTerminator(BasicBlock *BB, HLNode *InsertionPos);

  /// \brief Creates HLNodes for the instructions in the basic block.
  HLNode *populateInstSequence(BasicBlock *BB, HLNode *InsertionPos);

  /// \brief Returns true if the passed in BB post dominates all switch cases.
  bool postDominatesAllCases(SwitchInst *SI, BasicBlock *BB) const;

  /// \brief Sorts the dominator children of Node using post dominator
  /// relationship.
  void sortDomChildren(DomTreeNode *Node,
                       SmallVectorImpl<BasicBlock *> &SortedChildren) const;

  /// \brief Performs lexical (preorder) walk of the dominator tree for the
  /// region.
  /// Returns the last HLNode for the current sub-tree.
  HLNode *doPreOrderRegionWalk(BasicBlock *BB, HLNode *InsertionPos);

  /// \brief Sets the exit basic block of CurRegion using its last child.
  void setExitBBlock() const;

  /// \brief Creates HLRegions out of IRRegions.
  void create();

  /// \brief Contains implementation for print().
  void printImpl(raw_ostream &OS, bool FrameworkDetais) const;

public:
  static char ID; // Pass identification
  HIRCreation();

  bool runOnFunction(Function &F) override;
  void releaseMemory() override;
  void getAnalysisUsage(AnalysisUsage &AU) const override;
  void print(raw_ostream &OS, const Module * = nullptr) const override;
  /// \brief Prints framework details along with HLRegion like IRRegion and
  /// SCCs.
  void printWithFrameworkDetails(raw_ostream &OS) const;
  void verifyAnalysis() const override;

  /// Region iterator methods
  iterator begin() { return Regions.begin(); }
  const_iterator begin() const { return Regions.begin(); }
  iterator end() { return Regions.end(); }
  const_iterator end() const { return Regions.end(); }

  reverse_iterator rbegin() { return Regions.rbegin(); }
  const_reverse_iterator rbegin() const { return Regions.rbegin(); }
  reverse_iterator rend() { return Regions.rend(); }
  const_reverse_iterator rend() const { return Regions.rend(); }

  /// \brief Returns the src bblock associated with this if. Returns null if it
  /// fails to find one.
  const BasicBlock *getSrcBBlock(HLIf *If) const;

  /// \brief Returns the src bblock associated with this switch. Returns null if
  /// it fails to find one.
  const BasicBlock *getSrcBBlock(HLSwitch *Switch) const;
};

} // End namespace loopopt

} // End namespace llvm

#endif
