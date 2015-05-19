/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/
#include "DuplicateCalledKernelsPass.h"
#include "OCLPassSupport.h"
#include "MetaDataApi.h"

#include "llvm/DebugInfo.h"
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
    NamedMDNode *pllvmDebugCU = M.getNamedMetadata("llvm.dbg.cu");

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
      Function* pNewFunc = CloneFunction(pFunc, vMap, false);
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

      // Since pFunc is being duplicated, need to also duplicate the relevant
      // debug metadata.
      if (pllvmDebugCU) {
        for (unsigned cuIter = 0; cuIter < pllvmDebugCU->getNumOperands(); cuIter++) {
          duplicateDebugMD(pllvmDebugCU->getOperand(cuIter), finder, pFunc, pNewFunc);
        }
      }
    }
    return changed;
  }

  const MDNode* DuplicateCalledKernels::findSubprogram(const DebugInfoFinder& finder,
      const Function* pFunc) const {
    for (DebugInfoFinder::iterator iter = finder.subprogram_begin(),
        end = finder.subprogram_end(); iter != end; iter++) {
      const MDNode* node = *iter;
      if (DISubprogram(node).describes(pFunc)) return node;
    }
    return NULL;
  }

  MDNode* DuplicateCalledKernels::duplicateMDnode(MDNode* node,
                          Value* toReplace,
                          Value* with) const {
    std::vector<Value*> operands;
    for (unsigned i = 0; i < node->getNumOperands(); i++) {
      Value* op = node->getOperand(i);
      operands.push_back(op != toReplace ? op : with);
    }
    if (toReplace == NULL) operands.push_back(with);

    return MDNode::get(node->getContext(), operands);
  }

  void DuplicateCalledKernels::duplicateDebugMD(MDNode* cuNode,
                                                const DebugInfoFinder& finder,
                                                Function* pFunc,
                                                Function* pNewFunc) const {
    // Duplicate the subprogram metadata. This works by changing the original
    // subprogram metadata to point to the new function, and creating a new
    // subprogram metadata for the original function. This way all lexical blocks
    // will point to the new function, which is required because we use those
    // for adding function-specific locals when debugging (in ImplicitGIDPass).
    DISubprogram funcSP(findSubprogram(finder, pFunc));
    funcSP.replaceFunction(pNewFunc);
    DISubprogram newFuncSP(duplicateMDnode(funcSP, pNewFunc, pFunc));

    // Duplicate the subprogram list and add the new subprogram metadata there
    DIArray spList = DICompileUnit(cuNode).getSubprograms();
    DIArray newSpList(duplicateMDnode(spList, NULL, newFuncSP));

    // Replace all uses of the old subprogram list with the new subprogram list
    spList->replaceAllUsesWith(newSpList);

    // Duplicate function-specific metadata, if any exists.
    Module& M = *(pFunc->getParent());
    NamedMDNode* fnSpecificMDNode = getFnSpecificMDNode(M, newFuncSP);
    if (fnSpecificMDNode) {
      NamedMDNode* newFnSpecificMDNode = getOrInsertFnSpecificMDNode(M, funcSP);
      for (unsigned i = 0; i < fnSpecificMDNode->getNumOperands(); i++) {
        newFnSpecificMDNode->addOperand(fnSpecificMDNode->getOperand(i));
      }
    }
  }

} // namespace intel

/// Support for static linking of modules for Windows
/// This pass is called by a modified Opt.exe
extern "C" {
  llvm::ModulePass* createDuplicateCalledKernelsPass() {
    return new intel::DuplicateCalledKernels();
  }
}
