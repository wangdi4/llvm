//==--- BarrierInFunction.cpp - BarrierInFunction pass - C++ -*-------------==//
//
// Copyright (C) 2020 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //

#include "llvm/Transforms/Intel_DPCPPKernelTransforms/BarrierInFunctionPass.h"
#include "llvm/IR/CFG.h"
#include "llvm/IR/Instructions.h"
#include "llvm/InitializePasses.h"

using namespace llvm;

#define DEBUG_TYPE "dpcpp-kernel-barrier-in-function"

INITIALIZE_PASS(
    BarrierInFunction, DEBUG_TYPE,
    "Barrier Pass - Handle barrier instructions called from functions", false,
    true)

namespace llvm {

char BarrierInFunction::ID = 0;

BarrierInFunction::BarrierInFunction() : ModulePass(ID) {}

bool BarrierInFunction::runOnModule(Module &M) {
  // Initialize barrier utils class with current module.
  BarrierUtils.init(&M);

  // Find all the kernel functions.
  FuncVector &KernelFunctions = BarrierUtils.getAllKernelsWithBarrier();

  // Find all functions that call synchronize instructions.
  FuncSet &FunctionsWithSync =
      BarrierUtils.getAllFunctionsWithSynchronization();

  // Set of all functions that allready added to handle container.
  // Will be used to prevent handling functions more than once.
  FuncSet FunctionsAddedToHandle;
  // Add all kernel functions.
  FunctionsAddedToHandle.insert(KernelFunctions.begin(), KernelFunctions.end());
  // Add all functions with barriers (the set will assure no duplication).
  FunctionsAddedToHandle.insert(FunctionsWithSync.begin(),
                                FunctionsWithSync.end());

  // Vector of all functions to handle.
  FuncVector FunctionsToHandle;
  // It will be initialized with all above function we just added to the set.
  FunctionsToHandle.assign(FunctionsAddedToHandle.begin(),
                           FunctionsAddedToHandle.end());

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

      // TBD: This neeeds whole CFG exploration.
      // Skip handling of a kernel funciton unless it is a kernel.
      if (!(CI->getFunction()->hasFnAttribute("sycl_kernel")))
        continue;

      // Add Barrier before function call instruction.
      BarrierUtils.createBarrier(CI);

      // Add dummyBarrier after function call instruction.
      Instruction *DummyBarrierCall = BarrierUtils.createDummyBarrier();
      DummyBarrierCall->insertAfter(CI);

      Function *CallerFunc = CI->getParent()->getParent();

      // Add caller function to FunctionsToHandle and FunctionsAddedToHandle containers.
      bool Inserted = FunctionsAddedToHandle.insert(CallerFunc);
      if (Inserted)
        FunctionsToHandle.push_back(CallerFunc);
    }
  }

  // Remove all fiber instructions from non handled functions.
  removeFibersFromNonHandledFunctions(FunctionsAddedToHandle, M);

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
  BarrierUtils.createDummyBarrier(FirstInst);

  // Find all reachable return instructions in Func.
  InstVector RetInstructions;
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
    BarrierUtils.createBarrier(RetInst);
  }
}

void BarrierInFunction::removeFibersFromNonHandledFunctions(
    FuncSet &FunctionSet, Module &M) {
  // Don't need this for DPCPP.
}

ModulePass *createBarrierInFunctionPass() {
  return new llvm::BarrierInFunction();
}

} // namespace llvm
