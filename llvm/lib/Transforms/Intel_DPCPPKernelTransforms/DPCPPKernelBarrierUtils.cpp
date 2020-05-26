//==--- DPCPPKernelBarrierUtils.cpp - Barrier helper functions - C++ -*-----==//
//
// Copyright (C) 2020 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //

#include "llvm/Transforms/Intel_DPCPPKernelTransforms/DPCPPKernelBarrierUtils.h"
#include "llvm/IR/CFG.h"
#include "llvm/IR/Instructions.h"

namespace llvm {

DPCPPKernelBarrierUtils::DPCPPKernelBarrierUtils()
    : M(nullptr), IsSyncDataInitialized(false) {
  clean();
}

void DPCPPKernelBarrierUtils::init(Module *M) {
  assert(M && "Trying to initialize BarrierUtils with NULL module");
  this->M = M;

  clean();
}

void DPCPPKernelBarrierUtils::clean() { IsSyncDataInitialized = false; }

void DPCPPKernelBarrierUtils::initializeSyncData() {
  if (IsSyncDataInitialized) {
    // Sync data already initialized.
    return;
  }

  // Clear old collected data!
  Barriers.clear();
  DummyBarriers.clear();

  // Find all calls to barrier().
  findAllUsesOfFunc(StringRef(BarrierName), Barriers);
  // Find all calls to dummyBarrier().
  findAllUsesOfFunc(StringRef(DummyBarrierName), DummyBarriers);

  IsSyncDataInitialized = true;
}

void DPCPPKernelBarrierUtils::findAllUsesOfFunc(StringRef Name,
                                                InstSet &UsesSet) {
  // Check if given function name is declared in the module.
  Function *F = M->getFunction(Name);
  if (!F) {
    // Function is not declared.
    return;
  }
  // Find all calls to given function name.
  for (auto *U : F->users()) {
    CallInst *Call = dyn_cast<CallInst>(U);
    assert(Call && "Something other than CallInst is using function!");
    // Add the call instruction into uses set.
    UsesSet.insert(Call);
  }
}

InstVector &DPCPPKernelBarrierUtils::getAllSyncInstructions() {
  // Initialize sync data if it is not done yet.
  initializeSyncData();

  // Clear old collected data!
  SyncInstructions.clear();

  SyncInstructions.append(Barriers.begin(), Barriers.end());
  SyncInstructions.append(DummyBarriers.begin(), DummyBarriers.end());

  return SyncInstructions;
}

/// Find all functions directly calling sync instructions.
FuncSet &DPCPPKernelBarrierUtils::getAllFunctionsWithSynchronization() {
  // Initialize SyncInstructions.
  getAllSyncInstructions();

  // Clear old collected data!
  SyncFunctions.clear();

  for (auto *I : SyncInstructions) {
    SyncFunctions.insert(I->getFunction());
  }

  return SyncFunctions;
}

SyncType DPCPPKernelBarrierUtils::getSynchronizeType(const Instruction *I) {
  // Initialize sync data if it is not done yet.
  initializeSyncData();

  if (!isa<CallInst>(I)) {
    // Not a call instruction, cannot be a synchronize instruction.
    return SyncTypeNone;
  }
  if (Barriers.count(const_cast<Instruction *>(I))) {
    // It is a barrier instruction.
    return SyncTypeBarrier;
  }
  if (DummyBarriers.count(const_cast<Instruction *>(I))) {
    // It is a dummyBarrier instruction.
    return SyncTypeDummyBarrier;
  }
  return SyncTypeNone;
}

bool DPCPPKernelBarrierUtils::isDummyBarrierCall(const CallInst *CI) {
  assert(CI && "Instruction should not be nullptr!");
  // Initialize sync data if it is not done yet.
  initializeSyncData();
  return DummyBarriers.count(const_cast<CallInst *>(CI));
}

bool DPCPPKernelBarrierUtils::isBarrierCall(const CallInst *CI) {
  assert(CI && "Instruction should not be nullptr!");
  // Initialize sync data if it is not done yet.
  initializeSyncData();
  return Barriers.count(const_cast<CallInst *>(CI));
}

BasicBlock *
DPCPPKernelBarrierUtils::findBasicBlockOfUsageInst(Value *V,
                                                   Instruction *UserInst) {
  if (!isa<PHINode>(UserInst)) {
    // Not PHINode, return usage instruction basic block.
    return UserInst->getParent();
  }
  // Usage is a PHINode, find previous basic block according to V.
  PHINode *PhiNode = cast<PHINode>(UserInst);
  BasicBlock *PrevBB = nullptr;
  for (auto *BB : predecessors(PhiNode->getParent())) {
    Value *PHINodeVal = PhiNode->getIncomingValueForBlock(BB);
    if (PHINodeVal == V) {
      // pBB is the previous basic block
      assert(!PrevBB && "PHINode is using V twice!");
      PrevBB = BB;
    }
  }
  assert(PrevBB && "Failed to find previous basic block!");
  return PrevBB;
}

} // namespace llvm
