//==--- DataPerBarrierPass.h - Detect values dependent on TIDs - C++ -*-----==//
//
// Copyright (C) 2020 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //

#ifndef LLVM_TRANSFORMS_INTEL_DPCPP_KERNEL_TRANSFORMS_DATA_PER_BARRIER_PASS_H
#define LLVM_TRANSFORMS_INTEL_DPCPP_KERNEL_TRANSFORMS_DATA_PER_BARRIER_PASS_H

#include "llvm/ADT/MapVector.h"
#include "llvm/IR/Module.h"
#include "llvm/Pass.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/DPCPPKernelBarrierUtils.h"

namespace llvm {

/// DataPerBarrier pass is an analysis module pass used to collect
/// data on barrier/fiber/dummy barrier instructions.
class DataPerBarrier : public ModulePass {
public:
  typedef struct {
    InstSet RelatedBarriers;
    bool HasFiberRelated;
  } BarrierRelated;
  typedef struct {
    unsigned int ID;
    SyncType Type;
  } BarrierData;

  using BasicBlock2BasicBlockSetMap = MapVector<BasicBlock *, BasicBlockSet>;
  using InstructionSetPerFunctionMap = MapVector<Function *, InstSet>;
  using Barrier2BarrierSetMap = MapVector<Instruction *, BarrierRelated>;
  using BarrierDataPerBarrierMap = MapVector<Instruction *, BarrierData>;

public:
  static char ID;

  DataPerBarrier();

  ~DataPerBarrier() {}

  llvm::StringRef getPassName() const override {
    return "Intel Kernel DataPerBarrier";
  }

  bool runOnModule(Module &M) override {
    BarrierUtils.init(&M);
    InitSynchronizeData();
    for (auto &F : M)
      runOnFunction(F);
    return false;
  }

  /// Inform about usage/mofication/dependency of this pass.
  void getAnalysisUsage(AnalysisUsage &AU) const override {
    // Analysis pass preserve all
    AU.setPreservesAll();
  }

  /// Print data collected by the pass on the given module.
  /// OS stream to print the info regarding the module into.
  /// M pointer to the Module.
  void print(raw_ostream &OS, const Module *M = 0) const override;

  /// Return sync instruction calls of given function.
  /// F pointer to Function.
  /// Returns container of all sync instructions call in pFunc.
  InstSet &getSyncInstructions(Function *F) {
    assert(SyncsPerFuncMap.count(F) && "Function has no sync data!");
    return SyncsPerFuncMap[F];
  }

  /// Return unique id for given sync instruction.
  /// I is a pointer to sync instruction.
  /// Returns unique id for given sync instruction.
  unsigned int getUniqueID(Instruction *I) {
    assert(DataPerBarrierMap.count(I) && "instruction has no sync data!");
    return DataPerBarrierMap[I].ID;
  }

  /// Return type of given sync instruction.
  /// I pointer to sync instruction.
  /// Returns unique id for given sync instruction.
  SyncType getSyncType(Instruction *I) {
    assert(DataPerBarrierMap.count(I) && "instruction has no sync data!");
    return DataPerBarrierMap[I].Type;
  }

  /// Check if predecessors of given basic block is available.
  /// Return true if available, false otherwise.
  bool hasPredecessors(BasicBlock *pBB) {
    return PredecessorMap.count(pBB);
  }

  /// Return predecessors of a given basic block.
  /// BB pointer to basic block.
  /// Returns basic blocks set of predecessors.
  BasicBlockSet &getPredecessors(BasicBlock *BB) {
    assert(PredecessorMap.count(BB) && "basic block has no predecessor data!");
    return PredecessorMap[BB];
  }

  /// Return Successors of given basic block.
  /// BB pointer to basic block.
  /// Returns basic blocks set of Successors.
  BasicBlockSet &getSuccessors(BasicBlock *BB) {
    assert(SuccessorMap.count(BB) && "basic block has no successor data!");
    return SuccessorMap[BB];
  }

  /// Return Barrier Predecessors of given sync instruction.
  /// I pointer to sync instruction.
  /// Returns basic blocks set of Barrier Predecessors.
  BarrierRelated &getBarrierPredecessors(Instruction *I) {
    assert(BarrierPredecessorsMap.count(I) &&
           "sync instruction has no barrier predecessor data!");
    return BarrierPredecessorsMap[I];
  }

  /// Return true if given function contains fiber instruction.
  /// F pointer to Function.
  /// Returns true if and only if given function contains fiber instruction.
  bool hasFiberInstruction(Function *F) {
    // TODO: currently this function returns false if module has no fiber
    // and true otherwise. If needed change it to answer per function!
    return HasFiber;
  }

  /// Return true if given function contains synchronize instruction.
  /// F pointer to Function.
  /// Returns true if and only if given function contains synchronize
  /// instruction.
  bool hasSyncInstruction(Function *F) {
    if (!SyncsPerFuncMap.count(F))
      return false;
    return (SyncsPerFuncMap[F].size() > 0);
  }

private:
  /// Execute pass on given function.
  /// F function to analyze.
  /// Returns True if function was modified.
  bool runOnFunction(Function &F);

  /// Initialize data of synchronize instruction in processed module.
  void InitSynchronizeData();

  /// Calculate Predecessors of given basic block.
  /// BB pointer to basic block.
  void FindPredecessors(BasicBlock *BB);

  /// Calculate Successors of given basic block.
  /// BB pointer to basic block.
  void FindSuccessors(BasicBlock *BB);

  /// Calculate Barrier Predecessors of given sync instruction.
  /// I pointer to sync instruction.
  void FindBarrierPredecessors(Instruction *I);

private:
  /// This is barrier utility class
  DPCPPKernelBarrierUtils BarrierUtils;

  // Analysis Data for pass user.
  InstructionSetPerFunctionMap SyncsPerFuncMap;
  BarrierDataPerBarrierMap DataPerBarrierMap;
  BasicBlock2BasicBlockSetMap PredecessorMap;
  BasicBlock2BasicBlockSetMap SuccessorMap;
  Barrier2BarrierSetMap BarrierPredecessorsMap;
  bool HasFiber;
};

} // namespace llvm

#endif // LLVM_TRANSFORMS_INTEL_DPCPP_KERNEL_TRANSFORMS_DATA_PER_BARRIER_PASS_H
