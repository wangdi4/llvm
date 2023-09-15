//==--- BarrierInFunction.cpp - BarrierInFunction pass - C++ -*-------------==//
//
// Copyright (C) 2020 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //

#include "llvm/Transforms/SYCLTransforms/BarrierInFunctionPass.h"
#include "llvm/IR/CFG.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Transforms/SYCLTransforms/Utils/CompilationUtils.h"
#include "llvm/Transforms/SYCLTransforms/Utils/MetadataAPI.h"

using namespace llvm;

#define DEBUG_TYPE "sycl-kernel-barrier-in-function"

PreservedAnalyses BarrierInFunction::run(Module &M, ModuleAnalysisManager &) {
  if (!runImpl(M))
    return PreservedAnalyses::all();
  return PreservedAnalyses::none();
}

bool BarrierInFunction::runImpl(Module &M) {
  // Initialize barrier utils class with current module.
  Utils.init(&M);

  // Find all the kernel functions.
  CompilationUtils::FuncVec KernelFunctions = Utils.getAllKernelsWithBarrier();

  // Find all functions that call synchronize instructions.
  CompilationUtils::FuncSet FunctionsWithSync =
      Utils.getAllFunctionsWithSynchronization();

  // Set of all functions that allready added to handle container.
  // Will be used to prevent handling functions more than once.
  CompilationUtils::FuncSet FunctionsAddedToHandle;
  // Add all kernel functions.
  FunctionsAddedToHandle.insert(KernelFunctions.begin(), KernelFunctions.end());
  // Add all functions with barriers (the set will assure no duplication).
  FunctionsAddedToHandle.insert(FunctionsWithSync.begin(),
                                FunctionsWithSync.end());

  // Vector of all functions to handle.
  CompilationUtils::FuncVec FunctionsToHandle;
  // It will be initialized with all above function we just added to the set.
  FunctionsToHandle.assign(FunctionsAddedToHandle.begin(),
                           FunctionsAddedToHandle.end());

  auto Kernels = CompilationUtils::getKernels(M);

  // As long as there are functions to handle...
  while (!FunctionsToHandle.empty()) {
    Function *FuncToHandle = FunctionsToHandle.pop_back_val();

    // Add dummyBarrier at function begin and barrier at function end.
    addBarrierCallsToFunctionBody(FuncToHandle);

    // Fix all calls to Func.
    for (auto *U : FuncToHandle->users()) {
      CallInst *CI = dyn_cast<CallInst>(U);
      // Usage of Func can be a global variable!
      if (!CI) {
        // Usage of Func is not a CallInst.
        continue;
      }

      // Add Barrier before function call instruction.
      Utils.createBarrier(CI);

      // Add dummyBarrier after function call instruction.
      Instruction *DummyBarrierCall = Utils.createDummyBarrier();
      DummyBarrierCall->insertAfter(CI);

      Function *CallerFunc = CI->getCaller();

      // Add caller function to FunctionsToHandle and FunctionsAddedToHandle containers.
      bool Inserted = FunctionsAddedToHandle.insert(CallerFunc);
      if (Inserted)
        FunctionsToHandle.push_back(CallerFunc);
    }
  }

  return true;
}

void BarrierInFunction::addBarrierCallsToFunctionBody(Function *Func) {
  BasicBlock *FirstBB = &*Func->begin();
  assert(FirstBB && "Function has no basic block!");
  assert(pred_begin(FirstBB) == pred_end(FirstBB) &&
         "Function first basic block has predecessor!");
  Instruction *FirstInst = &*FirstBB->begin();
  assert(!dyn_cast<PHINode>(FirstInst) && "First instruction is a PHINode");

  // Add dummyBarrier call before FirstInst.
  Utils.createDummyBarrier(FirstInst);

  // Find all reachable return instructions in Func.
  CompilationUtils::InstVec RetInstructions;
  for (auto &BB : *Func) {
    Instruction *Term = BB.getTerminator();
    if (isa<ReturnInst>(Term) &&
        (pred_begin(&BB) != pred_end(&BB) || &BB == FirstBB)) {
      // Found a reachable ret instruction, add to container.
      RetInstructions.push_back(Term);
    }
  }

  // Add barrier call before each ret instruction in Func.
  for (Instruction *RetInst: RetInstructions) {
    Utils.createBarrier(RetInst);
  }
}
