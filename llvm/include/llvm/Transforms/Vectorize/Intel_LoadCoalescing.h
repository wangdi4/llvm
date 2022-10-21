//===- Intel_LoadCoalescing.h-Coalescing of Loads within a function--------===//
//
// Copyright (C) 2018-2019 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// This file declares classes for LoadCoalescing.
//
//===----------------------------------------------------------------------===//
#ifndef LLVM_TRANSFORMS_VECTORIZE_INTEL_LOADCOALESCING_H
#define LLVM_TRANSFORMS_VECTORIZE_INTEL_LOADCOALESCING_H

#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/SetVector.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/Analysis/Intel_MemRefAnalysis.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/PassManager.h"

#include <atomic>

namespace llvm {

class Value;
class BasicBlock;
class Function;
class DataLayout;
class Pass;
class TargetTransformInfo;
class AAResults;
class Instruction;

namespace vpmemrefanalysis {

/// The group represents a group of consecutive Loads.
class MemInstGroup {

  using InstrSetVec = llvm::SmallSetVector<llvm::Instruction *, 8>;

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  /// Unique ID for debugging.
  size_t Id;
#endif

  /// The vector of coalesced loads.
  InstrSetVec CoalescedLoads;

  /// The total size in bits of all the vectors in the group.
  size_t TotalVectorSize = 0;

  /// The size in bits of the scalar element.
  size_t ScalarSize = 0;

  const DataLayout &DL;

  size_t MaxVecRegSize;

  // Total number of scalar-elements in the group.
  size_t TotalScalarElements = 0;

public:
  MemInstGroup(const DataLayout &DL, size_t MaxVecRegSizeVal)
      : DL(DL), MaxVecRegSize(MaxVecRegSizeVal) {
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
    static std::atomic<size_t> StaticId(0);
    Id = StaticId++;
#endif
  }

  MemInstGroup(const MemInstGroup &) = delete;

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  /// Unique ID for debugging.
  size_t getId() const { return Id; }
#endif

  /// Helper for checking if G + RunnerSize > MaxVecRegSize
  bool willExceedSize(size_t RunnerSize, size_t MaxVecRegSize) const {
    return getTotalSize() + RunnerSize > MaxVecRegSize;
  };

  /// \Returns the number of loads to be coalesced.
  size_t size() const { return CoalescedLoads.size(); }

  /// \Returns the first load in the list of consecutive loads.
  Instruction *getHead() const { return CoalescedLoads.front(); }

  /// \Return true if a particular instruction can be inserted into the group.
  bool tryInsert(LoadInst *Inst);
  //
  /// \Returns the vector of loads.
  const InstrSetVec &getInstrs() const { return CoalescedLoads; }

  /// Append 'LI' of total byte size 'LISize' to the group.
  void append(Instruction *LI, size_t LISize);

  /// \Returns the number of scalar elements in the group.
  // TODO: Make this more robust for non-uniform sized elements.
  size_t getNumElements() const { return TotalVectorSize / ScalarSize; }

  /// \Return the scalar type of a single element.
  Type *getScalarType() const;

  /// \Returns the ith element of the group
  Instruction *getMember(size_t i) const {
    assert(i < size() && "Index exceeds Group bounds.");
    return CoalescedLoads[i];
  }

  /// \Return true if combining loads of this group would be profitable.
  bool isCoalescingLoadsProfitable(const TargetTransformInfo *TTI) const;

  /// Returns the wide vector type of the group after the loads get
  /// coalesced.
  VectorType *getWideType() const {
    return FixedVectorType::get(getScalarType(), getNumElements());
  }

  /// Return the total byte size of the group.
  size_t getTotalSize() const { return TotalVectorSize; };

  /// Return the total number of scalar elements in the group.
  size_t getTotalScalarElements() const { return TotalScalarElements; }

  /// Returns true if \p I is in the group.
  bool contains(Instruction *I) const { return CoalescedLoads.count(I); }
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  /// Debug print.
  void dump() const;
#endif
};

/// The Scheduler's DAG. This DAG is built for each Group and used
/// to schedule instructions in a Group.
class GroupDependenceGraph {
private:
  /// The top of the schedule.
  Instruction *TopI = nullptr;

  /// The bottom of the schedule.
  Instruction *BotI = nullptr;

protected:
  using PosMapTy = DenseMap<Instruction *, size_t>;

  /// Instruction to instruction order in the BB.
  PosMapTy PosMap;

  /// The nodes at the bottom of the DAG.
  SmallVector<Instruction *, 8> Roots;

  /// The BB being scheduled.
  BasicBlock *BB = nullptr;

  /// Alias Analysis.
  AAResults *AA = nullptr;

  /// This struct summarizes the data in all the outgoing edges of the DAG.
  struct NodeEntry {

    /// Points to all Sources Dependency Map.
    SmallVector<Instruction *, 2> Preds;

    /// The number of successors that have not been scheduled yet.
    unsigned UnscheduledSuccs = 0;
  };

  /// Maps each instruction to its outgoing edge data.
  SmallDenseMap<Instruction *, NodeEntry> NodeData;

  /// Returns the number of unscheduled successors of \p I.
  size_t getUnschedSuccsSafe(const Instruction *I) const;

  /// Collect the program ordering of instructions and populate PosMap. Returns
  /// true on success.
  bool collectPos(const MemInstGroup &G);

  /// \Returns the order of \p I in the program.
  size_t getPosition(Instruction *I) const { return PosMap.lookup(I); }

public:
  using DefsTy = SmallSetVector<Instruction *, 4>;

  GroupDependenceGraph() = delete;
  GroupDependenceGraph(const GroupDependenceGraph &) = delete;
  GroupDependenceGraph(BasicBlock *BB, AAResults *AA);

  /// Inserts into \p Defs all instructions that \p I depends on.
  void getDefs(Instruction *I, DefsTy &Defs);

  /// Decrement 'UnscheduledSuccs' and return true if \p Src has become ready.
  bool decrementUnscheduledSuccs(Instruction *Src);

  /// Returns true if \p I is within the TopI - BotI region.
  bool isInRegion(Instruction *I) const;

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  /// Debug print
  void dump() const;
#endif

  /// Builds the DAG for the region defined by \p G.
  bool buildDAG(const MemInstGroup &G);

  /// Returns the roots of the DAG.
  SmallVectorImpl<Instruction *> &getRoots() { return Roots; }

  /// Returns the vector of all the definitions of \p Dst.
  const SmallVectorImpl<Instruction *> &getDefs(Instruction *Dst) {
    return NodeData[Dst].Preds;
  }
};

/// The scheduler for legality check + code motion.
class Scheduler {

private:
  BasicBlock *BB = nullptr;
  std::unique_ptr<GroupDependenceGraph> DAG;

  SmallSetVector<Instruction *, 8> ReadyList;

  SmallSetVector<Instruction *, 16> BottomUpScheduledInstrs;

  /// Pop the first instruction in the ready list. We expect 'I' to be
  /// part  of the ReadyList.
  Instruction *popReady(Instruction *I);

  /// Pop the first instruction in the ready list that is not part of the
  /// Bundle. Returns nullptr on failure.
  Instruction *popNonBundleReady(const MemInstGroup &G);

  /// Schedule \p ReadyI and insert its immediate predecessors into the ready
  /// list.
  void scheduleReadyInstruction(Instruction *ReadyI);

  /// Returns true if all instructions in \p Bundle are in the ready list.
  bool isBundleInReadyList(const MemInstGroup &G) const;

public:
  Scheduler() = delete;
  Scheduler(const Scheduler &) = delete;
  Scheduler(BasicBlock *BB, AAResults *AA);

  /// Returns true if all instructions in \p Bundle can be scheduled together.
  bool trySchedule(const MemInstGroup &G);

  /// Apply the schedule that we have already saved in
  /// 'BottomUpScheduledInstrs'.
  void applySchedule();
};

/// The main class of the Load Coalescing transformation.
class LoadCoalescing {

  Function &F;
  const DataLayout &DL;
  ScalarEvolution &SE;
  TargetTransformInfo *TTI = nullptr;
  AAResults *AA = nullptr;
  IRBuilder<> Builder;
  BasicBlockMemRefAnalysis BBMemRefAnalysis;

  using BBLoadBucketsTy = BasicBlockMemRefAnalysis::BBLoadBucketsTy;
  using LoadBucketMembers = BasicBlockMemRefAnalysis::LoadBucketMembers;

  /// Handle for the scheduler.
  std::unique_ptr<Scheduler> SCH;

  /// The maximum vector register size (in bits) that the target allows.
  size_t MaxVecRegSize = 0;

  /// The minimum vector register size (in bits) that the target allows.
  size_t MinVecRegSize = 0;

  /// This member is used to collect all the load instructions that have been
  /// coalesced into a wider load and are no longer needed. We use this
  /// container to safely delete them from memory.
  SmallPtrSet<LoadInst *, 8> UnnecessaryInsts;

  /// This member function builds the largest possible group of loads
  /// starting at the seed and returns the next seed if we are able to
  /// successfully schedule the load instructions in the Group.
  bool buildMaximalGroup(const LoadBucketMembers &BucketMembers,
                         LoadBucketMembers::const_iterator &BucketMemIter,
                         MemInstGroup &G);
  /// Build groups of loads that can be coalesced and generate code for them.
  bool createGroupsAndGenerateCode(BasicBlock *CurrentBB);

  /// Run on a single BasicBlock.
  bool run(BasicBlock &Block);

  /// Analyzes the group, tries the schedule instructions in the group and
  /// returns the status if scheduling was successfule.
  bool scheduleGroup(const MemInstGroup &G);

  /// Generate code for the group.
  void codeGen(const MemInstGroup &G);

  /// \Return true if the two memory access are consecutive.
  bool areAdjacentMemoryAccesses(LoadInst *I1, LoadInst *I2);

public:
  LoadCoalescing() = delete;
  LoadCoalescing(const LoadCoalescing &) = delete;
  LoadCoalescing(Function &F, ScalarEvolution &SE, TargetTransformInfo *TTI,
                 AAResults *AA);

  /// Entry point.
  bool run();
};

} // end namespace vpmemrefanalysis

class LoadCoalescingPass : public PassInfoMixin<LoadCoalescingPass> {
  Function *F = nullptr;
  ScalarEvolution *SE = nullptr;
  TargetTransformInfo *TTI = nullptr;
  AAResults *AA = nullptr;

public:
  bool runImpl(Function *FVal, ScalarEvolution *SEVal,
               TargetTransformInfo *TTInfo, AAResults *AAInfo);
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &AM);
};

class LoadCoalescingLegacyPass : public FunctionPass {
  llvm::LoadCoalescingPass Impl;

public:
  static char ID; // Pass identification, replacement for typeid
  LoadCoalescingLegacyPass();
  bool runOnFunction(Function &F) override;
  void getAnalysisUsage(AnalysisUsage &AU) const override;
};

} // end namespace llvm

#endif // LLVM_TRANSFORMS_VECTORIZE_INTEL_LOADCOALESCING_H
