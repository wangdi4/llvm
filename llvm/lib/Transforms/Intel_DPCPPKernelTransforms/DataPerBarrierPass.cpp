//==--- DataPerBarrierPass.h - Detect values dependent on TIDs - C++ -*-----==//
//
// Copyright (C) 2020 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //

#include "llvm/Transforms/Intel_DPCPPKernelTransforms/DataPerBarrierPass.h"
#include "llvm/IR/CFG.h"
#include "llvm/IR/Instructions.h"
#include "llvm/InitializePasses.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

INITIALIZE_PASS(DataPerBarrier, "dpcpp-kernel-data-per-barrier-analysis",
                "Intel DPCPP Barrier Pass - Collect Data per Barrier", false,
                true)

namespace llvm {
char DataPerBarrier::ID = 0;

DataPerBarrier::DataPerBarrier() : ModulePass(ID) {}

void DataPerBarrier::InitSynchronizeData() {
  // Internal Data used to calculate user Analysis Data.
  unsigned int CurrentAvailableID = 0;
  // Set hasFiber to false till we find a fiber.
  HasFiber = false;

  // Find all synchronize instructions.
  InstVector &SyncInstructions = BarrierUtils.getAllSyncInstructions();

  for (Instruction *I : SyncInstructions) {
    SyncType InstSyncType = BarrierUtils.getSynchronizeType(I);
    assert(SyncTypeNone != InstSyncType &&
           "Sync list contains non sync instruction!");

    if (SyncTypeFiber == InstSyncType) {
      // Module is using fiber instruction.
      // TODO: do we need this information per Function?
      HasFiber = true;
    }

    // Save id and type of current sync instruction.
    DataPerBarrierMap[I].ID = CurrentAvailableID++;
    DataPerBarrierMap[I].Type = InstSyncType;

    Function *F = I->getParent()->getParent();
    // Classify the sync instruction according to its parent function.
    SyncsPerFuncMap[F].insert(I);
  }
}

bool DataPerBarrier::runOnFunction(Function &F) {
  if (SyncsPerFuncMap.count(&F) == 0) {
    // Function has no barrir/fiber/dummy barrier instruction, simply return.
    return false;
  }

  // Calculate predecessors for all basic blocks.
  for (auto &BB : F) {
    FindPredecessors(&BB);
  }

  // Calculate successors and sync predecessors only for sync basic blocks.
  for (Instruction *SyncInst : SyncsPerFuncMap[&F]) {
    BasicBlock *BB = SyncInst->getParent();
    FindSuccessors(BB);
    FindBarrierPredecessors(SyncInst);
  }
  return false;
}

void DataPerBarrier::FindPredecessors(BasicBlock *BB) {
  BasicBlockSet &Predecessors = PredecessorMap[BB];
  std::vector<BasicBlock *> BasicBlocksToHandle;

  Predecessors.clear();
  BasicBlocksToHandle.push_back(BB);

  while (!BasicBlocksToHandle.empty()) {
    BasicBlock *BBToHandle = BasicBlocksToHandle.back();
    BasicBlocksToHandle.pop_back();
    for (auto *PredBB : predecessors(BBToHandle)) {
      if (Predecessors.count(PredBB)) {
        // PredBB was already added to Predecessors.
        continue;
      }
      // This is a new predecessor, add it to the Predecessors container.
      Predecessors.insert(PredBB);
      // Also add it to the BasicBlocksToHandle to calculate its predecessors.
      BasicBlocksToHandle.push_back(PredBB);
    }
  }
}

void DataPerBarrier::FindSuccessors(BasicBlock *BB) {
  BasicBlockSet &Successors = SuccessorMap[BB];
  std::vector<BasicBlock *> BasicBlocksToHandle;

  Successors.clear();
  BasicBlocksToHandle.push_back(BB);
  // Barrier basic block is always a successor basic block of itself!
  Successors.insert(BB);

  while (!BasicBlocksToHandle.empty()) {
    BasicBlock *BBToHandle = BasicBlocksToHandle.back();
    BasicBlocksToHandle.pop_back();
    for (auto *SuccBB : successors(BBToHandle)) {
      if (Successors.count(SuccBB)) {
        // SuccBB was already added to Successors.
        continue;
      }
      // This is a new successor, add it to the Successors container.
      Successors.insert(SuccBB);
      // Also add it to the BasicBlocksToHandle to calculate its successors.
      BasicBlocksToHandle.push_back(SuccBB);
    }
  }
}

void DataPerBarrier::FindBarrierPredecessors(Instruction *I) {
  BasicBlock *BB = I->getParent();
  Function *F = BB->getParent();
  InstSet &BarrierBBSet = SyncsPerFuncMap[F];
  BarrierRelated &InstBarrierRelated = BarrierPredecessorsMap[I];
  InstSet &BarrierPredecessors = InstBarrierRelated.RelatedBarriers;
  std::vector<BasicBlock *> BasicBlocksToHandle;
  BasicBlockSet BasicBlocksAddedForHandle;

  InstBarrierRelated.HasFiberRelated = false;
  BarrierPredecessors.clear();
  BasicBlocksToHandle.push_back(BB);

  while (!BasicBlocksToHandle.empty()) {
    BasicBlock *BBToHandle = BasicBlocksToHandle.back();
    BasicBlocksToHandle.pop_back();
    for (auto *PredBB : predecessors(BBToHandle)) {
      if (BasicBlocksAddedForHandle.count(PredBB)) {
        // PredBB was already handled.
        continue;
      }
      // This is a new predecessor.
      BasicBlocksAddedForHandle.insert(PredBB);
      Instruction *Inst = &*(PredBB->begin());
      if (BarrierBBSet.count(Inst)) {
        // This predecessor basic block conatins a barrier.
        BarrierPredecessors.insert(Inst);
        if (DataPerBarrierMap[Inst].Type == SyncTypeFiber) {
          // predecessor is a fiber instruction, update barrier related data.
          InstBarrierRelated.HasFiberRelated = true;
        }
      } else {
        // Add it to the BasicBlocksToHandle to calculate its predecessors.
        BasicBlocksToHandle.push_back(PredBB);
      }
    }
  }
}

void DataPerBarrier::print(raw_ostream &OS, const Module *M) const {
  if (!M) {
    OS << "No Module!\n";
    return;
  }
  // Print Module
  OS << *M;

  // Run on all barrier basic blocks
  OS << "\nsynchronize basic blocks\n";
  for (const auto &KV : SyncsPerFuncMap) {
    Function *F = KV.first;
    // Print function name
    OS << "+" << F->getName() << "\n";
    const InstSet &SyncInstSet = KV.second;
    for (const Instruction *SyncInst : SyncInstSet) {
      const BasicBlock *BB = SyncInst->getParent();
      // Print basic block name
      OS << "\t-" << BB->getName() << "\n";
    }
    OS << "*"
       << "\n";
  }

  // Run on all Predecessors
  OS << "\nbasic blocks predecessors\n";
  for (const auto &KV : PredecessorMap) {
    const BasicBlock *BBB = KV.first;
    // Print barrier basic block name
    OS << "+" << BBB->getName() << "\n";
    const BasicBlockSet &BBSet = KV.second;
    for (const BasicBlock *BB : BBSet) {
      // Print predecessor basic block name
      OS << "\t-" << BB->getName() << "\n";
    }
    OS << "*"
       << "\n";
  }

  // Run on all Successors
  OS << "\nsynchronize basic blocks successors\n";
  for (const auto &KV : SuccessorMap) {
    const BasicBlock *BBB = KV.first;
    // Print barrier basic block name
    OS << "+" << BBB->getName() << "\n";
    const BasicBlockSet &BBSet = KV.second;
    for (const BasicBlock *BB : BBSet) {
      // Print successor basic block name
      OS << "\t-" << BB->getName() << "\n";
    }
    OS << "*"
       << "\n";
  }

  // Run on all Barrier Predecessors
  OS << "\nsynchronize basic blocks barrier predecessors\n";
  for (const auto &KV : BarrierPredecessorsMap) {
    const Instruction *Inst = KV.first;
    const BasicBlock *BBB = Inst->getParent();
    // Print barrier basic block name
    OS << "+" << BBB->getName() << "\n";
    OS << "has fiber instruction as predecessors: " << KV.second.HasFiberRelated
       << "\n";
    const InstSet &IVec = KV.second.RelatedBarriers;
    for (const Instruction *InstPred : IVec) {
      const BasicBlock *BB = InstPred->getParent();
      // Print barrier predecessor basic block name
      OS << "\t-" << BB->getName() << "\n";
    }
    OS << "*"
       << "\n";
  }

  OS << "DONE";
}

ModulePass *createDataPerBarrierPass() { return new llvm::DataPerBarrier(); }

} // namespace llvm
