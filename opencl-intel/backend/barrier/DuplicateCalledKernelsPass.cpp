/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/
#include "DuplicateCalledKernelsPass.h"
#include "OCLPassSupport.h"
#include "MetaDataApi.h"

#include "llvm/IR/Instructions.h"
#include "llvm/Transforms/Utils/Cloning.h"

namespace intel {

  char DuplicateCalledKernels::ID = 0;

  OCL_INITIALIZE_PASS(DuplicateCalledKernels, "B-DuplicateCalledKernels", "Barrier Pass - Clone kernels called from other functions", false, true)

  DuplicateCalledKernels::DuplicateCalledKernels() : ModulePass(ID) {}

  bool DuplicateCalledKernels::runOnModule(Module &M) {

    Intel::MetaDataUtils mdUtils(&M);

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
      for ( Value::use_iterator ui = pFunc->use_begin(),
          ue = pFunc->use_end(); ui != ue; ++ui ) {
        if ( isa<CallInst>(*ui) ) {
          isCalledKernel = true;
          break;
        }
      }
      if ( !isCalledKernel ) continue;
      changed = true;

      ValueToValueMapTy vMap;
      Function* pNewFunc = CloneFunction(pFunc, vMap, false);
      pNewFunc->setName("__internal." + (*fi)->getName());
      M.getFunctionList().push_back(pNewFunc);

      //Run over old uses of pFuncToFix and replace with call to pNewFunc
      for ( Value::use_iterator ui = pFunc->use_begin(),
          ue = pFunc->use_end(); ui != ue; ++ui ) {
        CallInst *pCallInst = dyn_cast<CallInst>(*ui);
        if ( !pCallInst ) continue;
        //Replace call to kernel function with call to new function.
        pCallInst->replaceUsesOfWith(pFunc, pNewFunc);
      }

      // Since pFunc is being cloned, need to create suitable debug metadata.
      // TODO: this is a workaround to pass debugger test. Check how to handle it right.
      NamedMDNode *pllvmDebugCU = M.getNamedMetadata("llvm.dbg.cu");
      if (pllvmDebugCU) {
          for(int ui = 0, ue = pllvmDebugCU->getNumOperands(); ui < ue; ui++) {
            MDNode* pMetadata = pllvmDebugCU->getOperand(ui);
            replaceMDUsesOfFunc(pMetadata, pFunc, pNewFunc);
          }
      }
    }
    return changed;
  }

  void DuplicateCalledKernels::replaceMDUsesOfFunc(MDNode* pMetadata, Function* pFunc, Function* pNewFunc) {
    SmallVector<Value *, 16> values;
    for (int i = 0, e = pMetadata->getNumOperands(); i < e; ++i) {
      Value *elem = pMetadata->getOperand(i);
      if (elem) {
        if (MDNode *Node = dyn_cast<MDNode>(elem))
            replaceMDUsesOfFunc(Node, pFunc, pNewFunc);

        // Elem needs to be set again otherwise changes will be undone.
        elem = pMetadata->getOperand(i);
        if (pFunc == dyn_cast<Function>(elem))
          elem = pNewFunc;
      }
      values.push_back(elem);
    }
    MDNode* pNewMetadata = MDNode::get(pFunc->getContext(), ArrayRef<Value*>(values));
    // TODO: Why may pMetadata and pNewMetadata be the same value ?
    if (pMetadata != pNewMetadata)
      pMetadata->replaceAllUsesWith(pNewMetadata);
  }

} // namespace intel

/// Support for static linking of modules for Windows
/// This pass is called by a modified Opt.exe
extern "C" {
  llvm::ModulePass* createDuplicateCalledKernelsPass() {
    return new intel::DuplicateCalledKernels();
  }
}
