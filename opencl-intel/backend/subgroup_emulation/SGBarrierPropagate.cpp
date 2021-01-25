//=---------------------- SGBarrierPropagate.cpp -*- C++ -*------------------=//
//
// Copyright (C) 2020 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //

#include "SGBarrierPropagate.h"

#include "InitializePasses.h"
#include "OCLPassSupport.h"

#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Instructions.h"

#define DEBUG_TYPE "sg-barrier-propagate"

namespace intel {

OCL_INITIALIZE_PASS_BEGIN(SGBarrierPropagate, DEBUG_TYPE,
                          "Propagate the sub_group_barrier", false, false)
OCL_INITIALIZE_PASS_DEPENDENCY_INTEL(SGSizeAnalysis)
OCL_INITIALIZE_PASS_END(SGBarrierPropagate, DEBUG_TYPE,
                        "Propagate the sub_group_barrier", false, false)

char SGBarrierPropagate::ID = 0;

bool SGBarrierPropagate::runOnModule(Module &M) {
  Helper.initialize(M);
  SizeAnalysis = &getAnalysis<SGSizeAnalysis>();

  FuncVec WorkList;
  FuncSet FunctionAdded;

  const FuncSet &SyncFunctions = Helper.getAllSyncFunctions();
  for (auto *F : SyncFunctions) {
    // Skip the functions called in vectorized kernel or not called by any
    // kernel.
    if (!SizeAnalysis->hasEmuSize(F))
      continue;
    if (FunctionAdded.insert(F))
      WorkList.push_back(F);
  }

  bool Changed = !WorkList.empty();

  while (!WorkList.empty()) {
    auto *F = WorkList.pop_back_val();
    addBarrierToFunction(*F);
    for (User *U : F->users()) {
      CallInst *CI = dyn_cast<CallInst>(U);
      if (!CI)
        continue;
      auto *PF = CI->getFunction();
      // Skip the functions called in vectorized kernel or not called by any
      // kernel.
      if (!SizeAnalysis->hasEmuSize(PF))
        continue;
      Helper.insertBarrierBefore(CI);
      Helper.insertDummyBarrierAfter(CI);
      if (FunctionAdded.insert(PF))
        WorkList.push_back(PF);
    }
  }
  return Changed;
}

void SGBarrierPropagate::addBarrierToFunction(Function &F) {
  auto *FirstInst = &*F.getEntryBlock().begin();
  assert(!isa<PHINode>(FirstInst) && "First instruction is phi");
  Helper.insertDummyBarrierBefore(FirstInst);
  BBVec ReturningBlocks;
  // Since we have run UnifyFunctionExitNodes before, there should be only
  // one return here. But anyway, we could do this.
  for (BasicBlock &I : F)
    if (isa<ReturnInst>(I.getTerminator()))
      Helper.insertBarrierBefore(I.getTerminator());
}

} // namespace intel

extern "C" {
llvm::Pass *createSGBarrierPropagatePass() {
  return new intel::SGBarrierPropagate();
}
}
