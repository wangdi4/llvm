/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/
#include "DuplicateCalledKernelsPass.h"
#include "OCLPassSupport.h"
#include "MetaDataApi.h"

#include "llvm/IR/DebugInfo.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Metadata.h"
#include "llvm/Transforms/Utils/Cloning.h"

#include <vector>

namespace intel {

  char DuplicateCalledKernels::ID = 0;

  OCL_INITIALIZE_PASS(DuplicateCalledKernels, "B-DuplicateCalledKernels", "Barrier Pass - Clone kernels called from other functions", false, true)

  DuplicateCalledKernels::DuplicateCalledKernels() : ModulePass(ID) {}

  bool DuplicateCalledKernels::runOnModule(Module &M) {

    Intel::MetaDataUtils mdUtils(&M);
    DebugInfoFinder finder;
    finder.processModule(M);

    TFunctionVector kernelFunctions;
    Intel::MetaDataUtils::KernelsList::const_iterator itr = mdUtils.begin_Kernels();
    Intel::MetaDataUtils::KernelsList::const_iterator end = mdUtils.end_Kernels();
    for (; itr != end; ++itr) {
      kernelFunctions.push_back((*itr)->getFunction());
    }
    bool changed = false;
    for(TFunctionVector::iterator fi = kernelFunctions.begin(),
        fe = kernelFunctions.end(); fi != fe; ++fi) {
      Function* pFunc = *fi;

      bool isCalledKernel = false;
      for ( Value::user_iterator ui = pFunc->user_begin(),
          ue = pFunc->user_end(); ui != ue; ++ui ) {
        if ( isa<CallInst>(*ui) ) {
          isCalledKernel = true;
          break;
        }
      }
      if ( !isCalledKernel ) continue;
      changed = true;

      ValueToValueMapTy vMap;
      Function* pNewFunc = CloneFunction(pFunc, vMap, nullptr);
      pNewFunc->setName("__internal." + (*fi)->getName());
      M.getFunctionList().push_back(pNewFunc);

      //Run over old uses of pFuncToFix and replace with call to pNewFunc
      for ( Value::user_iterator ui = pFunc->user_begin(),
          ue = pFunc->user_end(); ui != ue; ++ui ) {
        CallInst *pCallInst = dyn_cast<CallInst>(*ui);
        if ( !pCallInst ) continue;
        //Replace call to kernel function with call to new function.
        pCallInst->replaceUsesOfWith(pFunc, pNewFunc);
      }
    }
    return changed;
  }
} // namespace intel

/// Support for static linking of modules for Windows
/// This pass is called by a modified Opt.exe
extern "C" {
  llvm::ModulePass* createDuplicateCalledKernelsPass() {
    return new intel::DuplicateCalledKernels();
  }
}
