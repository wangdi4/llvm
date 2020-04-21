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
#include "llvm/IR/Instructions.h"

namespace llvm {
namespace DPCPPKernelBarrierUtils {

void findAllUsesOfFunc(Module &M, const StringRef Name, InstSet &UsesSet) {
  UsesSet.clear();

  // Check if given function name is declared in the module.
  Function *F = M.getFunction(Name);
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

void getAllSyncInstructions(Module &M, InstVector &SyncInsts) {
  SyncInsts.clear();

  InstSet BarrierUses;
  findAllUsesOfFunc(M, StringRef(BarrierName), BarrierUses);

  InstSet DummyBarrierUses;
  findAllUsesOfFunc(M, StringRef(DummyBarrierName), DummyBarrierUses);

  for (Instruction *I : BarrierUses)
    SyncInsts.push_back(I);
  for (Instruction *I : DummyBarrierUses)
    SyncInsts.push_back(I);
}

/// Find all functions directly calling sync instructions.
void getAllFunctionsWithSynchronization(Module &M, FuncSet &SyncFuncs) {
  SyncFuncs.clear();

  InstVector SyncInsts;
  // Initialize m_syncInstructions
  getAllSyncInstructions(M, SyncInsts);

  for (auto *I : SyncInsts) {
    SyncFuncs.insert(I->getFunction());
  }
}

} // namespace DPCPPKernelBarrierUtils
} // namespace llvm
