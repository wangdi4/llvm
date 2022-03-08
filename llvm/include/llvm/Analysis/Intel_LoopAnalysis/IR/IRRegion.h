//===--- IRRegion.h - Section of LLVM IR representing HLRegion --*- C++ -*-===//
//
// Copyright (C) 2015-2020 Intel Corporation. All rights reserved.
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
class Instruction;
class BasicBlock;
class Loop;
class raw_ostream;

namespace loopopt {

class HLRegion;

/// Section of LLVM IR which can be transformed into HLRegion.
///
/// IRRegion can be minimally defined as a pair of entry basic block and a set
/// of basic blocks (including the entry basic block).
class IRRegion {
public:
  typedef DenseSet<const BasicBlock *> RegionBBlocksSetTy;
  typedef SmallVector<const BasicBlock *, 32> RegionBBlocksTy;
  typedef SmallDenseMap<unsigned, const Value *, 16> LiveInMapTy;
  typedef SmallDenseMap<unsigned, SmallVector<const Instruction *, 2>, 16>
      LiveOutMapTy;
  typedef SmallDenseMap<const Instruction *, unsigned, 16> ReverseLiveOutMapTy;

  /// Iterators to iterate over bblocks and live-in/live-out sets.
  typedef RegionBBlocksTy::const_iterator const_bb_iterator;
  typedef LiveInMapTy::const_iterator const_live_in_iterator;
  typedef LiveOutMapTy::const_iterator const_live_out_iterator;

protected:
  /// Make class uncopyable.
  IRRegion(const IRRegion &) = delete;

  // Sets parent region.
  friend HLRegion;

private:
  BasicBlock *EntryBBlock;
  BasicBlock *ExitBBlock;
  // TODO: replace following two data structures by SetVector.
  // Contains all bblocks of region.
  // SmallVector of bblocks is used for deterministic iteration.
  RegionBBlocksTy BBlocks;
  // DenseSet of bblocks is used for faster query results to containsBBlock().
  RegionBBlocksSetTy BBlocksSet;

  // Contains bblocks which do not belong to any loops in the region. It is
  // empty for function level region.
  RegionBBlocksTy NonLoopBBlocks;

  // Vector or outermost loops in the region.
  SmallVector<const Loop *, 8> OutermostLps;

  // Map of symbase to their initial values which need to be materialized into
  // a store during HIRCG.
  LiveInMapTy LiveInMap;
  // Map of symbases to vector of instructions whose liveout uses need to be
  // materialized into loads during HIRCG. One symbase can be mapped to multiple
  // instructions in case of single operand phis where both the phi and its
  // operand are liveout of the region.
  LiveOutMapTy LiveOutMap;
  // Reverse set used by CG to handle liveouts.
  ReverseLiveOutMapTy ReverseLiveOutMap;
  HLRegion *ParentRegion;

  // Indicates that the region was formed for loop materialization.
  bool IsLoopMaterializationCandidate;
  // Indicates that the region is composed of all the function bblocks.
  bool IsFunctionLevel;

public:
  IRRegion(BasicBlock *Entry, BasicBlock *Exit, const RegionBBlocksTy &BBlocks,
           const RegionBBlocksTy &NonLoopBBlocks,
           ArrayRef<const Loop *> OutermostLoops,
           bool IsMaterializationCandidate = false,
           bool IsFunctionLevel = false);

  /// Move constructor. This is used by HIRRegionIdentification pass to
  /// push_back regions onto SmallVector.
  IRRegion(IRRegion &&);
  IRRegion &operator=(IRRegion &&);

  /// Dumps IRRegion.
  void dump() const;
  /// Prints IRRegion..
  void print(raw_ostream &OS, unsigned IndentWidth) const;

  /// Returns the entry(first) bblock of this region.
  BasicBlock *getEntryBBlock() const { return EntryBBlock; }

  /// Returns the exit(last) bblock of this region.
  BasicBlock *getExitBBlock() const { return ExitBBlock; }

  /// Replaces the existing entry bblock of the region by the new one.
  void replaceEntryBBlock(BasicBlock *NewEntryBB);

  /// Returns the predecessor bblock of this region.
  BasicBlock *getPredBBlock() const;
  /// Returns the successor bblock of this region.
  BasicBlock *getSuccBBlock() const;

  /// Returns true if this region contains BB.
  bool containsBBlock(const BasicBlock *BB) const {
    return BBlocksSet.count(BB);
  }

  void addBBlock(const BasicBlock *BB) {
    BBlocks.push_back(BB);
    BBlocksSet.insert(BB);
  }

  bool hasNonLoopBBlocks() const { return !NonLoopBBlocks.empty(); }

  /// Returns list of outermost loops of the region.
  ArrayRef<const Loop *> getOutermostLoops() const { return OutermostLps; }

  /// Returns true if region was formed for loop materialization.
  bool isLoopMaterializationCandidate() const {
    return IsLoopMaterializationCandidate;
  }

  /// Adds a live-in temp (represented using Symbase) with initial value
  /// InitVal to the region.
  void addLiveInTemp(unsigned Symbase, const Value *InitVal) {
    auto Ret = LiveInMap.insert(std::make_pair(Symbase, InitVal));
    (void)Ret;
    assert((Ret.second || (Ret.first->second == InitVal)) &&
           "Inconsistent livein value detected!");
  }

  /// Adds a live-out temp (represented using Symbase) to the region.
  void addLiveOutTemp(unsigned Symbase, const Instruction *Temp);

  /// Removes \p Symbase from the liveout temp set.
  void removeLiveOutTemp(unsigned Symbase);

  void replaceLiveOutTemp(unsigned OldSymbase, unsigned NewSymbase);

  /// Returns true if this symbase is live in to this region.
  bool isLiveIn(unsigned Symbase) const { return LiveInMap.count(Symbase); }

  /// Returns true if this symbase is live out of this region.
  bool isLiveOut(unsigned Symbase) const { return LiveOutMap.count(Symbase); }

  bool hasLiveOuts() const { return !LiveOutMap.empty(); }

  /// Returns true if the \p Symbase is livein and its stored value matches \p
  /// Val.
  bool isLiveInValue(unsigned Symbase, Value *Val) const {
    auto It = LiveInMap.find(Symbase);

    return (It != LiveInMap.end() && It->second == Val);
  }

  const_bb_iterator bb_begin() const { return BBlocks.begin(); }
  const_bb_iterator bb_end() const { return BBlocks.end(); }

  const_bb_iterator non_loop_bb_begin() { return NonLoopBBlocks.begin(); }
  const_bb_iterator non_loop_bb_end() { return NonLoopBBlocks.end(); }

  const_live_in_iterator live_in_begin() const { return LiveInMap.begin(); }
  const_live_in_iterator live_in_end() const { return LiveInMap.end(); }
  const_live_out_iterator live_out_begin() const { return LiveOutMap.begin(); }
  const_live_out_iterator live_out_end() const { return LiveOutMap.end(); }

  // Returns symbase of a liveout value. Asserts, if the value is not liveout.
  unsigned getLiveOutSymbase(const Instruction *Temp) const {
    auto It = ReverseLiveOutMap.find(Temp);
    assert(It != ReverseLiveOutMap.end() && "Temp is not liveout!");

    return It->second;
  }

  /// Returns true if this one region was created for the entire function.
  bool isFunctionLevel() const { return IsFunctionLevel; }
};

} // End namespace loopopt

} // End namespace llvm

#endif
