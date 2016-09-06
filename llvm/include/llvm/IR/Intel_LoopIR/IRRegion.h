//===--- IRRegion.h - Section of LLVM IR representing HLRegion --*- C++ -*-===//
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
// This file defines the IRRegion node.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_IR_INTEL_LOOPIR_IRREGION_H
#define LLVM_IR_INTEL_LOOPIR_IRREGION_H

#include <iterator>
#include <set>

#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/ADT/SmallVector.h"

namespace llvm {

class Value;
class BasicBlock;
class raw_ostream;

namespace loopopt {

/// \brief Section of LLVM IR which can be transformed into HLRegion.
///
/// IRRegion can be minimally defined as a pair of entry basic block and a set
/// of basic blocks (including the entry basic block).
class IRRegion {
public:
  typedef SmallPtrSet<const BasicBlock *, 32> RegionBBlocksTy;
  typedef SmallDenseMap<unsigned, const Value *, 16> LiveInSetTy;
  typedef LiveInSetTy LiveOutSetTy;

  /// Iterators to iterate over bblocks and live-in/live-out sets.
  typedef RegionBBlocksTy::const_iterator const_bb_iterator;
  typedef LiveInSetTy::const_iterator const_live_in_iterator;
  typedef LiveOutSetTy::const_iterator const_live_out_iterator;

protected:
  /// \brief Make class uncopyable.
  IRRegion(const IRRegion &) = delete;

  /// \brief Make class unassignable.
  void operator=(const IRRegion &) = delete;

private:
  BasicBlock *EntryBBlock;
  BasicBlock *ExitBBlock;
  RegionBBlocksTy BBlocks;
  // Set of (symbase - initial value) pairs which need to be materialized into
  // a store during HIRCG.
  LiveInSetTy LiveInSet;
  // Set of symbases/values whose live-out uses need to be materialized into a
  // load during HIRCG.
  LiveOutSetTy LiveOutSet;

public:
  IRRegion(BasicBlock *Entry, const RegionBBlocksTy &BBlocks);

  /// \brief Move constructor. This is used by HIRRegionIdentification pass to
  /// push_back regions onto SmallVector.
  IRRegion(IRRegion &&);

  /// \brief Dumps IRRegion.
  void dump() const;
  /// \brief Prints IRRegion..
  void print(raw_ostream &OS, unsigned IndentWidth) const;

  /// \brief Returns the entry(first) bblock of this region.
  BasicBlock *getEntryBBlock() const { return EntryBBlock; }

  /// \brief Returns the exit(last) bblock of this region.
  BasicBlock *getExitBBlock() const { return ExitBBlock; }

  /// \brief Sets the entry(first) bblock of this region.
  void setEntryBBlock(BasicBlock *EntryBB) { EntryBBlock = EntryBB; }

  /// \brief Sets the exit(last) bblock of this region.
  void setExitBBlock(BasicBlock *ExitBB) { ExitBBlock = ExitBB; }

  /// \brief Returns the predecessor bblock of this region.
  BasicBlock *getPredBBlock() const;
  /// \brief Returns the successor bblock of this region.
  BasicBlock *getSuccBBlock() const;

  /// \brief Returns true if this region contains BB.
  bool containsBBlock(const BasicBlock *BB) const { return BBlocks.count(BB); }

  void addBBlock(const BasicBlock *BB) { BBlocks.insert(BB); }

  /// \brief Adds a live-in temp (represented using Symbase) with initial value
  /// InitVal to the region.
  void addLiveInTemp(unsigned Symbase, const Value *InitVal) {
    auto Ret = LiveInSet.insert(std::make_pair(Symbase, InitVal));
    (void)Ret;
    assert((Ret.second || ((Ret.first->first == Symbase) &&
                           (Ret.first->second == InitVal))) &&
           "Inconsistent livein value detected!");
  }

  /// \brief Adds a live-out temp (represented using Symbase) to the region.
  void addLiveOutTemp(const Value *Temp, unsigned Symbase) {
    auto Ret = LiveOutSet.insert(std::make_pair(Symbase, Temp));
    (void)Ret;
    assert(Ret.second && "Liveout value already exists!");
  }

  /// \brief Returns true if this symbase is live in to this region.
  bool isLiveIn(unsigned Symbase) const { return LiveInSet.count(Symbase); }

  /// \brief Returns true if this symbase is live out of this region.
  bool isLiveOut(unsigned Symbase) const { return LiveOutSet.count(Symbase); }

  const_bb_iterator bb_begin() const { return BBlocks.begin(); }
  const_bb_iterator bb_end() const { return BBlocks.end(); }

  const_live_in_iterator live_in_begin() const { return LiveInSet.begin(); }
  const_live_in_iterator live_in_end() const { return LiveInSet.end(); }

  const_live_out_iterator live_out_begin() const { return LiveOutSet.begin(); }
  const_live_out_iterator live_out_end() const { return LiveOutSet.end(); }
};

} // End namespace loopopt

} // End namespace llvm

#endif
