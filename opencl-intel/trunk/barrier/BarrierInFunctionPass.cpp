/*********************************************************************************************
 * TODO: add Copyright © 2011, Intel Corporation
 *********************************************************************************************/
#include "BarrierInFunctionPass.h"

#include "llvm/Instructions.h"
#include "llvm/Support/CFG.h"

namespace intel {

  char BarrierInFunction::ID = 0;

  BarrierInFunction::BarrierInFunction() : ModulePass(ID) {}

  bool BarrierInFunction::runOnModule(Module &M) {
    //Initialize barrier utils class with current module
    m_util.init(&M);

    //Find all the kernel functions
    TFunctionVector& kernelFunctions = m_util.getAllKernelFunctions();

    //Find all functions that call synchronize instructions
    TFunctionVector& functionsWithSync = m_util.getAllFunctionsWithSynchronization();

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
      Function::use_iterator ui = pFuncToHandle->use_begin();
      Function::use_iterator ue = pFuncToHandle->use_end();
      for ( ; ui != ue; ui++ ) {
        CallInst *pCallInst = dyn_cast<CallInst>(*ui);
        assert( pCallInst && "function use is not a call instruction!" );

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
    BasicBlock *pFirstBB = pFunc->begin();
    assert( pFirstBB && "function has no basic block!" );
    assert( pred_begin(pFirstBB) == pred_end(pFirstBB) &&
      "function first basic block has predecessor!" );
    Instruction *pFirstInst = pFirstBB->begin();
    assert( !dyn_cast<PHINode>(pFirstInst) && "first instruction is a PHINode" );

    // Add dummyBarrier call before pFirstInst
    m_util.createDummyBarrier(pFirstInst);

    // Find all ret instructions in pFunc
    // TODO: check if there is a better way!
    TInstructionVector retInstructions;
    for ( Function::iterator bi = pFunc->begin(), be = pFunc->end(); bi != be; ++bi ) {
      Instruction *term = bi->getTerminator();
      if ( isa<ReturnInst>(term) ) {
        // Found a ret instruction add to container
        retInstructions.push_back(term);
      }
    }

    // Add barrier call before each ret instruction in pFunc
    for ( TInstructionVector::iterator ii = retInstructions.begin(),
      ie = retInstructions.end(); ii != ie; ++ii ) {
        Instruction *pRetInst = dyn_cast<Instruction>(*ii);
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
    Value::use_iterator ui = pFiber->use_begin();
    Value::use_iterator ue = pFiber->use_end();
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
      Instruction *pInstToRemove = dyn_cast<Instruction>(*ii);
      pInstToRemove->eraseFromParent();
    }
  }

  // Register this pass...
  static RegisterPass<BarrierInFunction> DPB("b-i-f",
    "Handle barier instructions called from functions", false, true);

} // namespace intel

/// Support for static linking of modules for Windows
/// This pass is called by a modified Opt.exe
extern "C" {
  void* createBarrierInFunctionPass() {
    return new intel::BarrierInFunction();
  }
}