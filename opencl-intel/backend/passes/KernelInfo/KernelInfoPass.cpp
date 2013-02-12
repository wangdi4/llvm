/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/

#include "KernelInfoPass.h"
#include "llvm/Support/InstIterator.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/PassManager.h"
#include <string>
#include <math.h>

namespace Intel { namespace OpenCL { namespace DeviceBackend {

  char KernelInfo::ID = 0;

  bool KernelInfo::runOnFunction(Function &Func) {
    m_mapKernelInfo[Func.getName().str()].kernelExecutionLength = getExecutionLength(&Func);
    m_mapKernelInfo[Func.getName().str()].hasBarrier = conatinsBarrier(&Func);
    return false;
  }

  bool KernelInfo::conatinsBarrier(Function *pFunc) {
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

  size_t KernelInfo::getExecutionEstimation(unsigned depth) {
    return (size_t)pow(10.0f, (int)depth);
  }

  size_t KernelInfo::getExecutionLength(Function *pFunc) {
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
  
  bool KernelInfoWrapper::runOnModule(Module& Mod) {
    llvm::FunctionPassManager FPM(&Mod);
    KernelInfo* pKernelInfoPass = new KernelInfo();
    FPM.add(pKernelInfoPass);
    
    for (llvm::Module::iterator i = Mod.begin(), e = Mod.end(); i != e; ++i) {
        FPM.run(*i);
    }
    m_mapKernelInfo.clear();
    m_mapKernelInfo.insert(pKernelInfoPass->getKernelInfoMap().begin(), pKernelInfoPass->getKernelInfoMap().end());
    return false;
  }

  void getKernelInfoMap(ModulePass *pPass, std::map<std::string, TKernelInfo>& infoMap) {
    KernelInfoWrapper *pKU = (KernelInfoWrapper*)pPass;

    infoMap.clear();
    if ( NULL != pKU ) {
      infoMap.insert(pKU->getKernelInfoMap().begin(), pKU->getKernelInfoMap().end());
    }
  }


}}}

