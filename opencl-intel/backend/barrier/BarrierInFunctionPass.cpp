// INTEL CONFIDENTIAL
//
// Copyright 2012-2018 Intel Corporation.
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you (License). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.

#include "BarrierInFunctionPass.h"
#include "OCLPassSupport.h"

#include "llvm/IR/Instructions.h"
#include "llvm/IR/CFG.h"

namespace intel {

  char BarrierInFunction::ID = 0;

  OCL_INITIALIZE_PASS(BarrierInFunction, "B-BarrierInFunction", "Barrier Pass - Handle barrier instructions called from functions", false, true)

  BarrierInFunction::BarrierInFunction() : ModulePass(ID) {}

  bool BarrierInFunction::runOnModule(Module &M) {
    //Initialize barrier utils class with current module
    m_util.init(&M);

    //Find all the kernel functions
    TFunctionVector& kernelFunctions = m_util.getAllKernelsWithBarrier();

    //Find all functions that call synchronize instructions
    TFunctionSet& functionsWithSync = m_util.getAllFunctionsWithSynchronization();

    //Set of all functions that allready added to handle container
    //Will be used to prevent handling functions more than once
    TFunctionSet functionsAddedToHandle;
    //Add all kernel functions
    functionsAddedToHandle.insert(kernelFunctions.begin(), kernelFunctions.end());
    //Add all functions with barriers (the set will assure no duplication)
    functionsAddedToHandle.insert(functionsWithSync.begin(), functionsWithSync.end());

    //Vector of all functions to handle
    TFunctionVector functionsToHandle;
    //It will be initialized with all above function we just added to the set
    functionsToHandle.assign(functionsAddedToHandle.begin(), functionsAddedToHandle.end());

    //As long as there are functions to handle...
    while ( !functionsToHandle.empty() ) {
      Function *pFuncToHandle = functionsToHandle.back();
      functionsToHandle.pop_back();

      // Add dummyBarrier at function begin and barrier at function end
      AddBarrierCallsToFunctionBody(pFuncToHandle);

      // Fix all calls to pFunc
      Function::user_iterator ui = pFuncToHandle->user_begin();
      Function::user_iterator ue = pFuncToHandle->user_end();
      for ( ; ui != ue; ui++ ) {
        CallInst *pCallInst = dyn_cast<CallInst>(*ui);
        // usage of pFunc can be a global variable!
        if( !pCallInst ) {
          // usage of pFunc is not a CallInst
          continue;
        }

        // Add Barrier before function call instruction
        m_util.createBarrier(pCallInst);

        // Add dummyBarrier after function call instruction
        Instruction *pDummyBarrierCall = m_util.createDummyBarrier();
        pDummyBarrierCall->insertAfter(pCallInst);

        Function *pCallerFunc = pCallInst->getParent()->getParent();
        if ( functionsAddedToHandle.count(pCallerFunc) ) {
          // Caller function already handled just continue
          continue;
        }

        // Add caller function to toHandle and AddedToHandle containers
        functionsToHandle.push_back(pCallerFunc);
        functionsAddedToHandle.insert(pCallerFunc);
      }
    }

    // Remove all fiber instructions from non handled functions
    RemoveFibersFromNonHandledFunctions(functionsAddedToHandle, M);

    return true;
  }

  void BarrierInFunction::AddBarrierCallsToFunctionBody(Function *pFunc) {
    BasicBlock *pFirstBB = &*pFunc->begin();
    assert( pFirstBB && "function has no basic block!" );
    assert( pred_begin(pFirstBB) == pred_end(pFirstBB) &&
      "function first basic block has predecessor!" );
    Instruction *pFirstInst = &*pFirstBB->begin();
    assert( !dyn_cast<PHINode>(pFirstInst) && "first instruction is a PHINode" );

    // Add dummyBarrier call before pFirstInst
    m_util.createDummyBarrier(pFirstInst);

    // Find all reachable return instructions in pFunc
    TInstructionVector retInstructions;
    for ( auto &BB : *pFunc ) {
      Instruction *term = BB.getTerminator();
      if ( isa<ReturnInst>(term) &&
           (pred_begin(&BB) != pred_end(&BB) || &BB == pFirstBB) ) {
        // Found a reachable ret instruction add to container
        retInstructions.push_back(term);
      }
    }

    // Add barrier call before each ret instruction in pFunc
    for ( TInstructionVector::iterator ii = retInstructions.begin(),
      ie = retInstructions.end(); ii != ie; ++ii ) {
        Instruction *pRetInst = *ii;
        m_util.createBarrier(pRetInst);
    }
  }

  void BarrierInFunction::RemoveFibersFromNonHandledFunctions(TFunctionSet& functionSet, Module &M) {
    TInstructionVector fibersToRemove;
    // Find all fiber call instructions that need to be removed
    Function *pFiber = M.getFunction(FIBER_FUNC_NAME);
    if( !pFiber ) {
      // Module contains no fiber instruction
      return;
    }
    Value::user_iterator ui = pFiber->user_begin();
    Value::user_iterator ue = pFiber->user_end();
    for ( ; ui != ue; ++ui ) {
      CallInst *pCall = dyn_cast<CallInst>(*ui);
      assert( pCall && "Something other than CallInst is using fiber function!" );
      Function *pFunc = pCall->getParent()->getParent();

      if( functionSet.count(pFunc) ) {
        // pFunc was handled keep fiber
        continue;
      }
      // pfunc was not handled remove fiber instruction
      fibersToRemove.push_back(pCall);
    }

    // Remove all fibers in fibersToRemove container from Module
    TInstructionVector::iterator ii = fibersToRemove.begin();
    TInstructionVector::iterator ie = fibersToRemove.end();
    for( ;ii != ie; ++ii ) {
      Instruction *pInstToRemove = *ii;
      pInstToRemove->eraseFromParent();
    }
  }


} // namespace intel

/// Support for static linking of modules for Windows
/// This pass is called by a modified Opt.exe
extern "C" {
  void* createBarrierInFunctionPass() {
    return new intel::BarrierInFunction();
  }
}
