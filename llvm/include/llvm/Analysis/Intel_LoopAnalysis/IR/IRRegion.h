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

#include "llvm/ADT/DenseSet.h"
#include "llvm/ADT/SmallVector.h"

namespace llvm {

class Value;
class BasicBlock;
class raw_ostream;

namespace loopopt {

class HLRegion;

/// \brief Section of LLVM IR which can be transformed into HLRegion.
///
/// IRRegion can be minimally defined as a pair of entry basic block and a set
/// of basic blocks (including the entry basic block).
class IRRegion {
public:
  typedef DenseSet<const BasicBlock *> RegionBBlocksSetTy;
  typedef SmallVector<const BasicBlock *, 32> RegionBBlocksTy;
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

  // Sets parent region.
  friend HLRegion;

private:
  BasicBlock *EntryBBlock;
  BasicBlock *ExitBBlock;
  // SmallVector of bblocks is used for deterministic iteration.
  RegionBBlocksTy BBlocks;
  // DenseSet of bblocks is used for faster query results to containsBBlock().
  RegionBBlocksSetTy BBlocksSet;

  // Set of (symbase - initial value) pairs which need to be materialized into
  // a store during HIRCG.
  LiveInSetTy LiveInSet;
  // Set of symbases/values whose live-out uses need to be materialized into a
  // load during HIRCG.
  LiveOutSetTy LiveOutSet;
  HLRegion *ParentRegion;

  // Indicates that the region is composed of all the function bblocks.
  const bool IsFunctionLevel;

public:
  IRRegion(BasicBlock *Entry, const RegionBBlocksTy &BBlocks,
           bool IsFunctionLevel = false);

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
  bool containsBBlock(const BasicBlock *BB) const {
    return BBlocksSet.count(BB);
  }

  void addBBlock(const BasicBlock *BB) {
    BBlocks.push_back(BB);
    BBlocksSet.insert(BB);
  }

  /// \brief Adds a live-in temp (represented using Symbase) with initial value
  /// InitVal to the region.
  void addLiveInTemp(unsigned Symbase, const Value *InitVal) {
    auto Ret = LiveInSet.insert(std::make_pair(Symbase, InitVal));
    (void)Ret;
    assert((Ret.second || (Ret.first->second == InitVal)) &&
           "Inconsistent livein value detected!");
  }

  /// \brief Adds a live-out temp (represented using Symbase) to the region.
  void addLiveOutTemp(unsigned Symbase, const Value *Temp) {
    auto Ret = LiveOutSet.insert(std::make_pair(Symbase, Temp));
    (void)Ret;
    assert((Ret.second || (Ret.first->second == Temp)) &&
           "Inconsistent liveout value detected!");
  }

  void replaceLiveOutTemp(unsigned OldSymbase, unsigned NewSymbase) {
    auto It = LiveOutSet.find(OldSymbase);
    assert((It != LiveOutSet.end()) && "Old liveout temp not found!");

    auto Temp = It->second;
    LiveOutSet.erase(It);
    addLiveOutTemp(NewSymbase, Temp);
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

  /// Returns true if this one region was created for the entire function.
  bool isFunctionLevel() const { return IsFunctionLevel; }
};

} // End namespace loopopt

} // End namespace llvm

#endif
