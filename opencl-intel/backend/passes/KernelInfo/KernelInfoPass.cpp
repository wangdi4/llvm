/*****************************************************************************\

Copyright (c) Intel Corporation (2010-2011).

    INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
    LICENSED ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,
    ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT
    PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY
    DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
    PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability,
    including liability for infringement of any proprietary rights, relating to
    use of the code. No license, express or implied, by estoppels or otherwise,
    to any intellectual property rights is granted herein.

File Name:  KernelInfoPass.cpp

\*****************************************************************************/

#include "KernelInfoPass.h"
#include "llvm/Support/InstIterator.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/PassManager.h"
#include <string>
#include <math.h>

namespace Intel { namespace OpenCL { namespace DeviceBackend {


  char KernelInfo::ID = 0;

  FunctionPass* createKernelInfoPass() {
    return new KernelInfo();
  }

  void getKernelInfoMap(FunctionPass *pPass, std::map<std::string, TKernelInfo>& infoMap) {
    KernelInfo *pKU = dynamic_cast<KernelInfo*>(pPass);

    infoMap.clear();
    if ( NULL != pKU ) {
      infoMap.insert(pKU->m_mapKernelInfo.begin(), pKU->m_mapKernelInfo.end());
    }
  }

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
    FunctionPass* pPass = createKernelInfoPass();
    FPM.add(pPass);
    
    for (llvm::Module::iterator i = Mod.begin(), e = Mod.end(); i != e; ++i) {
        FPM.run(*i);
    }
    getKernelInfoMap(pPass, m_mapKernelInfo);
    return false;
  }

  void getKernelInfoMap(ModulePass *pPass, std::map<std::string, TKernelInfo>& infoMap) {
    KernelInfoWrapper *pKU = dynamic_cast<KernelInfoWrapper*>(pPass);

    infoMap.clear();
    if ( NULL != pKU ) {
      infoMap.insert(pKU->m_mapKernelInfo.begin(), pKU->m_mapKernelInfo.end());
    }
  }


}}}

