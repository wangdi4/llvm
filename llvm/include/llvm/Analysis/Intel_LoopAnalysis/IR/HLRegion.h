//===-------- HLRegion.h - High level IR region node ------------*- C++ -*-===//
//
// Copyright (C) 2015-2018 Intel Corporation. All rights reserved.
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

#include "llvm/Analysis/Intel_LoopAnalysis/IR/HLNode.h"
#include "llvm/Analysis/Intel_LoopAnalysis/IR/IRRegion.h"
#include "llvm/IR/BasicBlock.h"
#include <iterator>
#include <set>

#include "llvm/Analysis/Intel_OptReport/LoopOptReport.h"
#include "llvm/Analysis/Intel_OptReport/LoopOptReportBuilder.h"
#include <functional>

namespace llvm {

class DbgInfoIntrinsic;

namespace loopopt {

/// \brief Top level node in High level IR
///
/// A High level region is a section of CFG which can be analyzed and
/// transformed independently of other sections of CFG. It typically consists
/// of a single loopnest.
class HLRegion final : public HLNode {
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

  typedef SmallDenseMap<unsigned, SmallVector<const DbgInfoIntrinsic *, 2>>
      DebugIntrinMap;

protected:
  HLRegion(HLNodeUtils &HNU, IRRegion &IReg);

  friend class HLNodeUtils;
  friend class HIRCreation;
  // Accesses getIRRegion().
  friend class HIRParser;

  /// \brief Sets the entry(first) bblock of this region.
  void setEntryBBlock(BasicBlock *EntryBB) { IRReg.setEntryBBlock(EntryBB); }

  /// \brief Sets the exit(last) bblock of this region.
  void setExitBBlock(BasicBlock *ExitBB) { IRReg.setExitBBlock(ExitBB); }

  /// \brief Clone Implementation
  /// Do not support Region cloning.
  HLRegion *cloneImpl(GotoContainerTy *GotoList, LabelMapTy *LabelMap,
                      HLNodeMapper *NodeMapper) const override;

  // Returns contained IRRegion.
  IRRegion &getIRRegion() { return IRReg; }

private:
  bool GenCode;
  IRRegion &IRReg;
  ChildNodeTy Children;

  // Symbase to llvm.dbg.* intrinsics.
  DebugIntrinMap DbgIntrinMap;

  // Optimization report for some nested but lost loops.
  LoopOptReport OptReport;

public:
  /// Returns the map between symbase and llvm.dbg.* intrinsics.
  const DebugIntrinMap &getDebugIntrinMap() const { return DbgIntrinMap; }

  /// Prints header for the region.
  void printHeader(formatted_raw_ostream &OS, unsigned Depth,
                   bool PrintIRRegion, bool Detailed) const;

  /// Prints body for the region.
  void printBody(formatted_raw_ostream &OS, unsigned Depth,
                 bool Detailed) const;

  /// Prints footer for the region.
  void printFooter(formatted_raw_ostream &OS, unsigned Depth) const;

  /// \brief Prints HLRegion.
  virtual void print(formatted_raw_ostream &OS, unsigned Depth,
                     bool Detailed = false) const override;
  /// \brief Prints HLRegion along with the contained IRRegion.
  void print(formatted_raw_ostream &OS, unsigned Depth, bool PrintIRRegion,
             bool Detailed) const;

  /// \brief Returns the entry(first) bblock of this region.
  BasicBlock *getEntryBBlock() const { return IRReg.getEntryBBlock(); }
  /// \brief Returns the exit(last) bblock of this region.
  BasicBlock *getExitBBlock() const { return IRReg.getExitBBlock(); }

  /// \brief Returns the predecessor bblock of this region.
  BasicBlock *getPredBBlock() const { return IRReg.getPredBBlock(); }
  /// \brief Returns the successor bblock of this region.
  BasicBlock *getSuccBBlock() const { return IRReg.getSuccBBlock(); }

  /// \brief Returns true if this region contains BB.
  bool containsBBlock(const BasicBlock *BB) const {
    return IRReg.containsBBlock(BB);
  }

  /// \brief Adds a live-in temp (represented using Symbase) with initial value
  /// InitVal to the region.
  void addLiveInTemp(unsigned Symbase, const Value *InitVal) {
    IRReg.addLiveInTemp(Symbase, InitVal);
  }

  /// \brief Adds a live-out temp (represented using Symbase) to the region.
  void addLiveOutTemp(unsigned Symbase, const Instruction *Temp) {
    IRReg.addLiveOutTemp(Symbase, Temp);
  }

  /// Replaces \p OldSymbase by \p NewSymbase as the new liveout temp.
  void replaceLiveOutTemp(unsigned OldSymbase, unsigned NewSymbase) {
    IRReg.replaceLiveOutTemp(OldSymbase, NewSymbase);
  }

  /// \brief Returns true if this symbase is live in to this region.
  bool isLiveIn(unsigned Symbase) const { return IRReg.isLiveIn(Symbase); }

  /// \brief Returns true if this symbase is live out of this region.
  bool isLiveOut(unsigned Symbase) const { return IRReg.isLiveOut(Symbase); }

  bool hasLiveOuts() const { return IRReg.hasLiveOuts(); }

  /// BBlock iterator methods
  const_bb_iterator bb_begin() const { return IRReg.bb_begin(); }
  const_bb_iterator bb_end() const { return IRReg.bb_end(); }

  /// Live-in iterator methods
  const_live_in_iterator live_in_begin() const { return IRReg.live_in_begin(); }
  const_live_in_iterator live_in_end() const { return IRReg.live_in_end(); }

  const_live_out_iterator live_out_begin() const {
    return IRReg.live_out_begin();
  }
  const_live_out_iterator live_out_end() const { return IRReg.live_out_end(); }
  // Returns symbase of a liveout value. Asserts, if the value is not liveout.
  unsigned getLiveOutSymbase(const Instruction *Temp) const {
    return IRReg.getLiveOutSymbase(Temp);
  }

  /// \brief Returns true if we need to generate code for this region.
  bool shouldGenCode() const { return GenCode; }
  void setGenCode(bool GC = true) { GenCode = GC; }

  /// Returns true if this one region was created for the entire function.
  bool isFunctionLevel() const { return IRReg.isFunctionLevel(); }

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
    return Node->getHLNodeClassID() == HLNode::HLRegionVal;
  }

  /// clone() - Do not support Cloning of Region.
  /// This is LLVM Unreachable code.
  HLRegion *clone(HLNodeMapper *NodeMapper = nullptr) const override;

  /// \brief Verifies HLRegion integrity.
  virtual void verify() const override;

  /// Returns true if the last child of the region is a return instruction.
  bool exitsFunction() const;

  LoopOptReport getOptReport() const { return OptReport; }
  void setOptReport(LoopOptReport R) { OptReport = R; }
  void eraseOptReport() { OptReport = nullptr; }
};

} // End namespace loopopt

// Traits of HLRegion for LoopOptReportBuilder.
template <> struct LoopOptReportTraits<loopopt::HLRegion> {
  using ObjectHandleTy = loopopt::HLRegion &;

  static LoopOptReport getOptReport(const loopopt::HLRegion &R) {
    return R.getOptReport();
  }

  static void setOptReport(loopopt::HLRegion &R, LoopOptReport OR) {
    R.setOptReport(OR);
  }

  static void eraseOptReport(loopopt::HLRegion &R) { R.eraseOptReport(); }

  static DebugLoc getDebugLoc(const loopopt::HLRegion &R) {
    return R.getDebugLoc();
  }
};

template <>
struct DenseMapInfo<loopopt::HLRegion *>
    : public loopopt::DenseHLNodeMapInfo<loopopt::HLRegion> {};

template <>
struct DenseMapInfo<const loopopt::HLRegion *>
    : public loopopt::DenseHLNodeMapInfo<const loopopt::HLRegion> {};

} // End namespace llvm

#endif
