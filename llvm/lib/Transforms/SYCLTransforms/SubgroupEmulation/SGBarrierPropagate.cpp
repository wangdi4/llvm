//== SGBarrierPropagate.cpp - Propagate subgroup barrier to all sync funcs ==//
//
// Copyright (C) 2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===---------------------------------------------------------------------===//

#include "llvm/Transforms/SYCLTransforms/SubgroupEmulation/SGBarrierPropagate.h"
#include "llvm/Transforms/SYCLTransforms/Utils/CompilationUtils.h"

using namespace llvm;
using namespace CompilationUtils;

#define DEBUG_TYPE "sycl-kernel-sg-emu-barrier-propagate"

bool SGBarrierPropagatePass::runImpl(Module &M, const SGSizeInfo *SSI) {
  Helper.initialize(M);

  FuncVec WorkList;
  FuncSet FunctionAdded;

  const FuncSet &SyncFunctions = Helper.getAllSyncFunctions();
  for (auto *F : SyncFunctions) {
    // Skip the functions called in vectorized kernel or not called by any
    // kernel.
    if (!SSI->hasEmuSize(F))
      continue;
    if (FunctionAdded.insert(F))
      WorkList.push_back(F);
  }

  bool Changed = !WorkList.empty();

  while (!WorkList.empty()) {
    auto *F = WorkList.pop_back_val();
    insertBarrierToFunction(*F);
    for (User *U : F->users()) {
      CallInst *CI = dyn_cast<CallInst>(U);
      if (!CI)
        continue;
      auto *PF = CI->getFunction();
      // Skip the functions called in vectorized kernel or not called by any
      // kernel.
      if (!SSI->hasEmuSize(PF))
        continue;
      Helper.insertBarrierBefore(CI);
      Helper.insertDummyBarrierAfter(CI);
      if (FunctionAdded.insert(PF))
        WorkList.push_back(PF);
    }
  }
  return Changed;
}

void SGBarrierPropagatePass::insertBarrierToFunction(Function &F) {
  auto *FirstInst = &*F.getEntryBlock().begin();
  assert(!isa<PHINode>(FirstInst) && "First instruction is phi");
  Helper.insertDummyBarrierBefore(FirstInst);
  // Since we have run UnifyFunctionExitNodes before, there should be only
  // one return here. But anyway, we could do this.
  for (BasicBlock &BB : F)
    if (auto *RI = dyn_cast<ReturnInst>(BB.getTerminator()))
      Helper.insertBarrierBefore(RI);
}

PreservedAnalyses SGBarrierPropagatePass::run(Module &M,
                                          ModuleAnalysisManager &AM) {
  if (!runImpl(M, &AM.getResult<SGSizeAnalysisPass>(M)))
    return PreservedAnalyses::all();

  PreservedAnalyses PA;
  PA.preserve<SGSizeAnalysisPass>();
  return PA;
}
