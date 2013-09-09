/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/

#include "KernelInfoPass.h"
#include "CompilationUtils.h"
#include "MetaDataApi.h"
#include "llvm/Support/InstIterator.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/PassManager.h"
#include <string>
#include <math.h>

namespace Intel { namespace OpenCL { namespace DeviceBackend {

  char KernelInfoPass::ID = 0;

  bool KernelInfoPass::runOnFunction(Function &Func) {
    m_mdUtils->getOrInsertKernelsInfoItem(&Func)->setKernelExecutionLength(getExecutionLength(&Func));
    m_mdUtils->getOrInsertKernelsInfoItem(&Func)->setKernelHasBarrier(containsBarrier(&Func));
    return false;
  }

  bool KernelInfoPass::containsBarrier(Function *pFunc) {
    for (inst_iterator ii = inst_begin(pFunc), e = inst_end(pFunc); ii != e; ++ii) {
      CallInst *pCall = dyn_cast<CallInst>(&*ii);
      if ( !pCall ) {
        continue;
      }
      std::string calledFuncName = pCall->getCalledFunction()->getName().str();
      if (calledFuncName.find("barrier") != std::string::npos) {
        return true;
      }
    }
    return false;
  }

  size_t KernelInfoPass::getExecutionEstimation(unsigned depth) {
    return (size_t)pow(10.0f, (int)depth);
  }

  size_t KernelInfoPass::getExecutionLength(Function *pFunc) {
    LoopInfo &LI = getAnalysis<LoopInfo>();

    size_t currLength = 0;
    for (Function::iterator i = pFunc->begin(), e = pFunc->end(); i != e; ++i) {
      currLength += i->size() * getExecutionEstimation(LI.getLoopDepth(i));
    }
    return currLength;
  }

  char KernelInfoWrapper::ID = 0;

  ModulePass* createKernelInfoWrapperPass() {
    return new KernelInfoWrapper();
  }
  
  bool KernelInfoWrapper::runOnModule(Module& M) {
    Intel::MetaDataUtils mdUtils(&M);
    KernelInfoPass* pKernelInfoPass = new KernelInfoPass(&mdUtils);

    llvm::FunctionPassManager FPM(&M);
    FPM.add(pKernelInfoPass);

    // Get all kernels
    CompilationUtils::FunctionSet kernelsFunctionSet;
    CompilationUtils::getAllKernels(kernelsFunctionSet, &M);

    // Run on all scalar functions for handling and handle them
    for ( CompilationUtils::FunctionSet::iterator fi = kernelsFunctionSet.begin(),
      fe = kernelsFunctionSet.end(); fi != fe; ++fi ) {
        Function *pFunc = dyn_cast<Function>(*fi);
        assert(pFunc && "got NULL kernel");
        FPM.run(*pFunc);
    }

    //Save Metadata to the module
    mdUtils.save(M.getContext());
    return false;
  }

}}}

