//== SGBarrierPropagate.cpp - Propagate subgroup barrier to all sync funcs ==//
//
// Copyright (C) 2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===---------------------------------------------------------------------===//

#include "llvm/Transforms/Intel_DPCPPKernelTransforms/SubgroupEmulation/SGBarrierPropagate.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"
#include "llvm/PassRegistry.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/DPCPPKernelCompilationUtils.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/LegacyPasses.h"

using namespace llvm;
using namespace DPCPPKernelCompilationUtils;

extern bool DPCPPEnableSubGroupEmulation;

#define DEBUG_TYPE "dpcpp-kernel-sg-emu-barrier-propagate"

bool SGBarrierPropagatePass::runImpl(Module &M, const SGSizeInfo *SSI) {
  if (!DPCPPEnableSubGroupEmulation)
    return false;

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

namespace {

/// Legacy SGBarrierPropagate pass
class SGBarrierPropagateLegacy : public ModulePass {
  SGBarrierPropagatePass Impl;

public:
  static char ID;

  SGBarrierPropagateLegacy() : ModulePass(ID) {
    initializeSGBarrierPropagateLegacyPass(*PassRegistry::getPassRegistry());
  }

  StringRef getPassName() const override { return "SGBarrierPropagateLegacy"; }

  bool runOnModule(Module &M) override {
    return Impl.runImpl(M, &getAnalysis<SGSizeAnalysisLegacy>().getResult());
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<SGSizeAnalysisLegacy>();
    AU.addPreserved<SGSizeAnalysisLegacy>();
  }
};

} // namespace

char SGBarrierPropagateLegacy::ID = 0;

INITIALIZE_PASS_BEGIN(SGBarrierPropagateLegacy, DEBUG_TYPE,
                      "Propagate the sub_group_barrier", false, false)
INITIALIZE_PASS_DEPENDENCY(SGSizeAnalysisLegacy)
INITIALIZE_PASS_END(SGBarrierPropagateLegacy, DEBUG_TYPE,
                    "Propagate the sub_group_barrier", false, false)

PreservedAnalyses SGBarrierPropagatePass::run(Module &M,
                                          ModuleAnalysisManager &AM) {
  if (!runImpl(M, &AM.getResult<SGSizeAnalysisPass>(M)))
    return PreservedAnalyses::all();

  PreservedAnalyses PA;
  PA.preserve<SGSizeAnalysisPass>();
  return PA;
}

ModulePass *llvm::createSGBarrierPropagateLegacyPass() {
  return new SGBarrierPropagateLegacy();
}
