//==--- SplitBBonBarrierPass.cpp - Split BB along barrier call - C++ -*-----==//
//
// Copyright (C) 2020 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //

#include "llvm/Transforms/Intel_DPCPPKernelTransforms/SplitBBonBarrierPass.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instructions.h"
#include "llvm/InitializePasses.h"

using namespace llvm;

PreservedAnalyses SplitBBonBarrier::run(Module &M, ModuleAnalysisManager &) {
  if (!runImpl(M))
    return PreservedAnalyses::all();
  return PreservedAnalyses::none();
}

bool SplitBBonBarrier::runImpl(Module &M) {
  // Initialize barrier utils class with current module.
  Utils.init(&M);

  // Find all synchronize instructions.
  CompilationUtils::InstVec SyncInsts = Utils.getAllSynchronizeInstructions();

  bool Changed = false;

  for (Instruction *I : SyncInsts) {
    BasicBlock::iterator BBIterator(I);
    BasicBlock *OriginBB = I->getParent();
    if (OriginBB->begin() == BBIterator) {
      // Barrier instruction already at begin of basic block (do nothing).
      continue;
    }
    // Split basic block at barrier intsruction to make
    // it first instruction in the new basic block.
    OriginBB->splitBasicBlock(BBIterator, "Split.Barrier.BB");
    Changed = true;
  }

  return Changed;
}
