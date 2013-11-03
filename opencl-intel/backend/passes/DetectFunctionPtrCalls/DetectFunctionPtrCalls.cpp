/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/

#define DEBUG_TYPE "detectfuncptrcalls"
#include "DetectFunctionPtrCalls.h"
#include "OCLPassSupport.h"
#include "MetaDataApi.h"
#include <llvm/IR/Instructions.h>
#include <llvm/Support/InstIterator.h>
#include <llvm/Support/Casting.h>
#include <llvm/Support/raw_ostream.h>

using namespace llvm;

extern "C" {
  Pass *createDetectFuncPtrCalls() { 
    return new intel::DetectFuncPtrCalls(); 
  }
}

namespace intel {
  
  char DetectFuncPtrCalls::ID = 0;
  OCL_INITIALIZE_PASS(DetectFuncPtrCalls, "detectfuncptrcall",
    "Detect Function Pointer Calls",
    false,
    false 
    )

  DetectFuncPtrCalls::DetectFuncPtrCalls() 
    : ModulePass(ID), m_DetectedFuncPtrCall(false) {
  }

  bool DetectFuncPtrCalls::runOnModule(Module &M) {
    m_DetectedFuncPtrCall = false;
    m_funcWithFuncPtrCall.clear();
    
    Intel::MetaDataUtils mdUtils(&M);

    // loop over functions in module
    for (Module::iterator mi = M.begin(), me = M.end(); mi != me; ++mi) {
      llvm::Function * pFunc = mi;
      if(DetectInFunction(*mi)){
        mdUtils.getOrInsertFunctionsInfoItem(pFunc)->setFuncPtrCall(true);
        m_DetectedFuncPtrCall = true;
      }
    }
    
    if(m_DetectedFuncPtrCall)
      mdUtils.save(M.getContext());
    
    return false;
  }

  // print out results
  void DetectFuncPtrCalls::print(raw_ostream &O, const Module *M) const{
    using namespace Intel; 

    if(m_DetectedFuncPtrCall){
      O << "DetectFuncPtrCalls: Found function pointer calls.\n";
      O << "DetectFuncPtrCalls: Functions with unresolved pointer calls:\n";

      MetaDataUtils mdUtils(const_cast<Module*>(M));
      MetaDataUtils::FunctionsInfoMap::iterator i = mdUtils.begin_FunctionsInfo();
      MetaDataUtils::FunctionsInfoMap::iterator e = mdUtils.end_FunctionsInfo();
      for(; i != e; ++i )
      {
        llvm::Function * pFunc = i->first;
        Intel::FunctionInfoMetaDataHandle kimd = i->second;
        if(kimd->isFuncPtrCallHasValue() && kimd->getFuncPtrCall()){
          O << pFunc->getName() << ".\n";
        }
      }
    }
    else 
      O << "DetectFuncPtrCalls: not found function pointer calls.\n";
  }

  bool DetectFuncPtrCalls::DetectInFunction(Function& F){
    // loop over instructions  
    for (inst_iterator I = inst_begin(F), E = inst_end(F); I != E; ++I){
      // detect call instruction  
      if (const CallInst* CI = dyn_cast<CallInst>(&*I)){
        // null if this is an indirect function invocation
        if(NULL == CI->getCalledFunction()){
          return true;
        }
      }
    }
    return false;
  }



} // namespace intel 



