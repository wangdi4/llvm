//===-------- HLRegion.h - High level IR region node ------------*- C++ -*-===//
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
// This file defines the HLRegion node.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_IR_INTEL_LOOPIR_HLREGION_H
#define LLVM_IR_INTEL_LOOPIR_HLREGION_H

#include "llvm/IR/Intel_LoopIR/HLNode.h"
#include "llvm/IR/Intel_LoopIR/IRRegion.h"
#include "llvm/Analysis/Intel_LoopAnalysis/RegionIdentification.h"
#include <set>
#include <iterator>

namespace llvm {

class BasicBlock;

namespace loopopt {

/// \brief Top level node in High level IR
///
/// A High level region is a section of CFG which can be analyzed and
/// transformed independently of other sections of CFG. It typically consists
/// of a single loopnest.
class HLRegion : public HLNode {
public:
  /// List of children nodes inside region
  typedef HLContainerTy ChildNodeTy;

  /// Iterators to iterate over bblocks and live-in/live-out sets.
  typedef IRRegion::const_bb_iterator const_bb_iterator;
  typedef IRRegion::const_live_in_iterator const_live_in_iterator;
  typedef IRRegion::const_live_out_iterator const_live_out_iterator;

  /// Iterators to iterate over children nodes
  typedef ChildNodeTy::iterator child_iterator;
  typedef ChildNodeTy::const_iterator const_child_iterator;
  typedef ChildNodeTy::reverse_iterator reverse_child_iterator;
  typedef ChildNodeTy::const_reverse_iterator const_reverse_child_iterator;

protected:
  HLRegion(IRRegion *IReg);

  /// HLNodes are destroyed in bulk using HLNodeUtils::destroyAll(). iplist<>
  /// tries to
  /// access and destroy the nodes if we don't clear them out here.
  ~HLRegion() { Children.clearAndLeakNodesUnsafely(); }

  friend class HLNodeUtils;
  friend class HIRCreation;

  /// \brief Sets the entry(first) bblock of this region.
  void setEntryBBlock(BasicBlock *EntryBB) { IRReg->setEntryBBlock(EntryBB); }

  /// \brief Sets the exit(last) bblock of this region.
  void setExitBBlock(BasicBlock *ExitBB) { IRReg->setExitBBlock(ExitBB); }

  /// \brief Clone Implementation
  /// Do not support Region cloning.
  HLRegion *cloneImpl(GotoContainerTy *GotoList,
                      LabelMapTy *LabelMap) const override;

private:
  bool GenCode;
  IRRegion *IRReg;
  ChildNodeTy Children;

public:
  /// \brief Prints HLRegion.
  virtual void print(formatted_raw_ostream &OS, unsigned Depth) const override;

  /// \brief Returns the entry(first) bblock of this region.
  BasicBlock *getEntryBBlock() const { return IRReg->getEntryBBlock(); }
  /// \brief Returns the exit(last) bblock of this region.
  BasicBlock *getExitBBlock() const { return IRReg->getExitBBlock(); }

  /// \brief Returns the predecessor bblock of this region.
  BasicBlock *getPredBBlock() const { return IRReg->getPredBBlock(); }
  /// \brief Returns the successor bblock of this region.
  BasicBlock *getSuccBBlock() const { return IRReg->getSuccBBlock(); }

  /// \brief Returns true if this region contains BB.
  bool containsBBlock(const BasicBlock *BB) const {
    return IRReg->containsBBlock(BB);
  }

  /// \brief Adds a live-in temp to the region.
  void addLiveInTemp(unsigned Symbase, const Value *Temp) {
    IRReg->addLiveInTemp(Symbase, Temp);
  }

  /// \brief Adds a live-out temp to the region.
  void addLiveOutTemp(const Value *Temp) { IRReg->addLiveOutTemp(Temp); }

  /// \brief Returns true if this value is live out of this region.
  bool isLiveOut(const Value *Temp) const { return IRReg->isLiveOut(Temp); }

  /// BBlock iterator methods
  const_bb_iterator bb_begin() const { return IRReg->bb_begin(); }
  const_bb_iterator bb_end() const { return IRReg->bb_end(); }

  /// Live-in iterator methods
  const_live_in_iterator live_in_begin() const {
    return IRReg->live_in_begin();
  }
  const_live_in_iterator live_in_end() const { return IRReg->live_in_end(); }

  /// Live-out iterator methods
  const_live_out_iterator live_out_begin() const {
    return IRReg->live_out_begin();
  }
  const_live_out_iterator live_out_end() const { return IRReg->live_out_end(); }

  /// \brief Returns true if we need to generate code for this region.
  bool shouldGenCode() const { return GenCode; }
  void setGenCode(bool GC = true) { GenCode = GC; }

  /// Children iterator methods
  child_iterator child_begin() { return Children.begin(); }
  const_child_iterator child_begin() const { return Children.begin(); }
  child_iterator child_end() { return Children.end(); }
  const_child_iterator child_end() const { return Children.end(); }

  reverse_child_iterator child_rbegin() { return Children.rbegin(); }
  const_reverse_child_iterator child_rbegin() const {
    return Children.rbegin();
  }
  reverse_child_iterator child_rend() { return Children.rend(); }
  const_reverse_child_iterator child_rend() const { return Children.rend(); }

  /// Children acess methods

  /// \brief Returns the first child if it exists, otherwise returns null.
  HLNode *getFirstChild();
  const HLNode *getFirstChild() const {
    return const_cast<HLRegion *>(this)->getFirstChild();
  }
  /// \brief Returns the last child if it exists, otherwise returns null.
  HLNode *getLastChild();
  const HLNode *getLastChild() const {
    return const_cast<HLRegion *>(this)->getLastChild();
  }

  /// \brief Returns the number of children.
  unsigned getNumChildren() const { return Children.size(); }
  /// \brief Returns true if it has children.
  bool hasChildren() const { return !Children.empty(); }

  /// \brief Method for supporting type inquiry through isa, cast, and dyn_cast.
  static bool classof(const HLNode *Node) {
    return Node->getHLNodeID() == HLNode::HLRegionVal;
  }

  /// clone() - Do not support Cloning of Region.
  /// This is LLVM Unreachable code.
  HLRegion *clone() const override;
};

} // End namespace loopopt

} // End namespace llvm

#endif
