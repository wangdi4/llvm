//==--- DataPerBarrierPass.h - Detect values dependent on TIDs - C++ -*-----==//
//
// Copyright (C) 2020 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //

#ifndef LLVM_TRANSFORMS_SYCLTRANSFORMS_DATA_PER_BARRIER_PASS_H
#define LLVM_TRANSFORMS_SYCLTRANSFORMS_DATA_PER_BARRIER_PASS_H

#include "llvm/ADT/MapVector.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Transforms/SYCLTransforms/Utils/BarrierUtils.h"

namespace llvm {

/// Collect data on barrier/fiber/dummy barrier instructions.
class DataPerBarrier {
public:
  using BBSet = CompilationUtils::BBSet;
  using InstSet = CompilationUtils::InstSet;

  typedef struct {
    InstSet RelatedBarriers;
  } BarrierRelated;
  typedef struct {
    unsigned int ID;
    SyncType Type;
  } BarrierData;

  using BasicBlock2BBSetMap = MapVector<BasicBlock *, BBSet>;
  using InstructionSetPerFunctionMap = MapVector<Function *, InstSet>;
  using Barrier2BarrierSetMap = MapVector<Instruction *, BarrierRelated>;
  using BarrierDataPerBarrierMap = MapVector<Instruction *, BarrierData>;

public:
  explicit DataPerBarrier(Module &M);

  /// Print data collected by the pass on the given module.
  /// OS stream to print the info regarding the module into.
  /// M pointer to the Module.
  void print(raw_ostream &OS, const Module *M) const;

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
    if (PredecessorMap.find(pBB) == PredecessorMap.end())
      FindPredecessors(pBB);
    return !PredecessorMap[pBB].empty();
  }

  /// Return predecessors of a given basic block.
  /// BB pointer to basic block.
  /// Returns basic blocks set of predecessors.
  BBSet &getPredecessors(BasicBlock *BB) {
    auto iter = PredecessorMap.find(BB);
    if (iter != PredecessorMap.end())
      return iter->second;

    return FindPredecessors(BB);
  }

  /// Return Successors of given basic block.
  /// BB pointer to basic block.
  /// Returns basic blocks set of Successors.
  BBSet &getSuccessors(BasicBlock *BB) {
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

  /// Return true if given function contains synchronize instruction.
  /// F pointer to Function.
  /// Returns true if and only if given function contains synchronize
  /// instruction.
  bool hasSyncInstruction(Function *F) {
    auto It = SyncsPerFuncMap.find(F);
    return It != SyncsPerFuncMap.end() && It->second.size() > 0;
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
  BBSet &FindPredecessors(BasicBlock *BB);

  /// Calculate Successors of given basic block.
  /// BB pointer to basic block.
  void FindSuccessors(BasicBlock *BB);

  /// Calculate Barrier Predecessors of given sync instruction.
  /// I pointer to sync instruction.
  void FindBarrierPredecessors(Instruction *I);

private:
  /// This is barrier utility class
  BarrierUtils Utils;

  // Analysis Data for pass user.
  InstructionSetPerFunctionMap SyncsPerFuncMap;
  BarrierDataPerBarrierMap DataPerBarrierMap;
  BasicBlock2BBSetMap PredecessorMap;
  BasicBlock2BBSetMap SuccessorMap;
  Barrier2BarrierSetMap BarrierPredecessorsMap;
};

/// DataPerBarrierAnalysis pass for new pass manager.
class DataPerBarrierAnalysis
    : public AnalysisInfoMixin<DataPerBarrierAnalysis> {
  friend AnalysisInfoMixin<DataPerBarrierAnalysis>;
  static AnalysisKey Key;

public:
  using Result = DataPerBarrier;
  Result run(Module &M, ModuleAnalysisManager &MAM);
};

/// Printer pass for DataPerBarrier.
class DataPerBarrierPrinter : public PassInfoMixin<DataPerBarrierPrinter> {
  raw_ostream &OS;

public:
  explicit DataPerBarrierPrinter(raw_ostream &OS) : OS(OS) {}
  PreservedAnalyses run(Module &M, ModuleAnalysisManager &MAM);
};

} // namespace llvm

#endif // LLVM_TRANSFORMS_SYCLTRANSFORMS_DATA_PER_BARRIER_PASS_H
