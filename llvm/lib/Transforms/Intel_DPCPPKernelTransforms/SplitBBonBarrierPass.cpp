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
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/DPCPPKernelBarrierUtils.h"

using namespace llvm;

INITIALIZE_PASS(SplitBBonBarrier, "dpcpp-split-on-barrier",
                "DPCPP Barrier Pass - Split Basic Block on Barrier", false,
                true)

namespace llvm {

char SplitBBonBarrier::ID = 0;

SplitBBonBarrier::SplitBBonBarrier() : ModulePass(ID) {}

bool SplitBBonBarrier::runOnModule(Module &M) {
  // Initialize barrier utils class with current module.
  BarrierUtils.init(&M);

  // Find all synchronize instructions.
  InstVector& SyncInsts = BarrierUtils.getAllSyncInstructions();

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

ModulePass *createSplitBBonBarrierPass() {
  return new llvm::SplitBBonBarrier();
}

} // namespace llvm
