/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/

#include "KernelInfoPass.h"
#include "CompilationUtils.h"
#include "OCLAddressSpace.h"
#include "MetaDataApi.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/PassManager.h"
#include <string>
#include <math.h>

using namespace Intel::OpenCL::DeviceBackend;

namespace intel {

  char KernelInfoPass::ID = 0;

  bool KernelInfoPass::runOnFunction(Function &Func) {
    m_mdUtils->getOrInsertKernelsInfoItem(&Func)->setKernelExecutionLength(getExecutionLength(&Func));
    m_mdUtils->getOrInsertKernelsInfoItem(&Func)->setKernelHasBarrier(containsBarrier(&Func));
    m_mdUtils->getOrInsertKernelsInfoItem(&Func)->setKernelHasGlobalSync(containsGlobalSync(&Func));
    return false;
  }

  bool KernelInfoPass::containsGlobalSync(Function *pFunc) {
    for (inst_iterator ii = inst_begin(pFunc), e = inst_end(pFunc); ii != e; ++ii) {
      CallInst *pCall = dyn_cast<CallInst>(&*ii);
      if (!pCall) {
        continue;
      }
      Function *pFunc = pCall->getCalledFunction();
      if (!pFunc) {
        // we assume that indirect call can't introduce global synchronization.
        continue;
      }
      std::string calledFuncName = pFunc->getName().str();
      if (CompilationUtils::isAtomicBuiltin(calledFuncName)) {
        Value * Arg0 = pCall->getOperand(0);
        // handle atomic_work_item_fence(cl_mem_fence_flags flags, 
        // memory_order order, memory_scope scope) built-in
        if(CompilationUtils::isAtomicWorkItemFenceBuiltin(calledFuncName)){
          // !!! MUST be aligned with define in opencl_.h 
          // #define CLK_GLOBAL_MEM_FENCE   0x2
          static const uint64_t CLK_GLOBAL_MEM_FENCE = 2;

          // handle contant int
          if(const llvm::ConstantInt* CI = dyn_cast<ConstantInt>(Arg0))
            // check in 0th argument CLK_GLOBAL_MEM_FENCE is set
            return CI->getZExtValue() & CLK_GLOBAL_MEM_FENCE;
          else
            // 0th argument is not constant
            // assuming worst case - has CLK_GLOBAL_MEM_FENCE flag set
            return true;
        }
        PointerType* ptr = cast<PointerType>(Arg0->getType());
        assert(!IS_ADDR_SPACE_GENERIC(ptr->getAddressSpace()) &&
              "Generic address space must be resolved before KernelInfoPass.");
        // [OpenCL 2.0] The following condition covers pipe built-ins as well
        // because the first arguments is a pipe which is a __global opaque pointer.
        if (IS_ADDR_SPACE_GLOBAL(ptr->getAddressSpace()))
          return true;
      }
    }
    return false;
  }

  bool KernelInfoPass::containsBarrier(Function *pFunc) {
    for (inst_iterator ii = inst_begin(pFunc), e = inst_end(pFunc); ii != e; ++ii) {
      CallInst *pCall = dyn_cast<CallInst>(&*ii);
      if ( !pCall ) {
        continue;
      }
      Function *pFunc = pCall->getCalledFunction();
      if (!pFunc) {
        continue;
      }
      std::string calledFuncName = pFunc->getName().str();
      if (calledFuncName.find("barrier") != std::string::npos) {
        return true;
      }
      if (CompilationUtils::isWorkGroupBuiltin(calledFuncName) ||
          CompilationUtils::isWorkGroupAsyncOrPipeBuiltin(calledFuncName, pFunc->getParent())) {
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

  size_t KernelInfoWrapper::getProgramGlobalVariableTotalSize(const Module& M) {
    // if there is no global variables - return 0
    if (M.global_empty()) return 0;
    size_t totalSize = 0;
    const DataLayout &TD = *M.getDataLayout();
    for (Module::const_global_iterator it = M.global_begin(); it != M.global_end(); ++it) {
      PointerType* ptr = cast<PointerType>(it->getType());
      assert(ptr && "Global variable is always a pointer.");
      if (IS_ADDR_SPACE_GLOBAL(ptr->getAddressSpace())) {
        totalSize += TD.getTypeAllocSize(ptr->getContainedType(0));
      }
    }
    return totalSize;
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

    if (mdUtils.empty_ModuleInfoList()) {
      mdUtils.addModuleInfoListItem(Intel::ModuleInfoMetaDataHandle(Intel::ModuleInfoMetaData::get()));
    }
    Intel::ModuleInfoMetaDataHandle handle = mdUtils.getModuleInfoListItem(0);
    handle->setGlobalVariableTotalSize(getProgramGlobalVariableTotalSize(M));

    //Save Metadata to the module
    mdUtils.save(M.getContext());
    return false;
  }

} // namespace intel {

extern "C" {
  ModulePass* createKernelInfoWrapperPass() {
    return new intel::KernelInfoWrapper();
  }
}
