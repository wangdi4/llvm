// INTEL CONFIDENTIAL
//
// Copyright 2012-2019 Intel Corporation.
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

#include "KernelInfoPass.h"
#include "CompilationUtils.h"
#include "OCLAddressSpace.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/InitializePasses.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/Utils/MetadataAPI.h"
#include <math.h>
#include <string>

using namespace Intel::OpenCL::DeviceBackend;
using namespace DPCPPKernelMetadataAPI;

namespace intel {

  char KernelInfoPass::ID = 0;

  KernelInfoPass::KernelInfoPass() : FunctionPass(ID) {
    initializeLoopInfoWrapperPassPass(*PassRegistry::getPassRegistry());
  }

  bool KernelInfoPass::runOnFunction(Function &Func) {
    auto kernelInfo = KernelInternalMetadataAPI(&Func);
    kernelInfo.KernelExecutionLength.set(getExecutionLength(&Func));
    kernelInfo.KernelHasGlobalSync.set(containsGlobalSync(&Func));
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

          // handle constant int
          if(const llvm::ConstantInt* CI = dyn_cast<ConstantInt>(Arg0))
            // check in 0th argument CLK_GLOBAL_MEM_FENCE is set
            return CI->getZExtValue() & CLK_GLOBAL_MEM_FENCE;
          else
            // 0th argument is not constant
            // assuming the worst case - has CLK_GLOBAL_MEM_FENCE flag set
            return true;
        }
        PointerType* ptr = cast<PointerType>(Arg0->getType());

        // After switching GenericAddressStaticResolutionPass to
        // InferAddressSpacesPass, it's legal for a pointer to remain as
        // unresolved and live in generic address space until CodeGen. So if a
        // pointer is in generic address space, we just ignore it and assume
        // that it won't access global address space. This assumption won't
        // affect the correctness of the program, as the global synchronization
        // information is only used to calculate the workgroup size (see
        // Kernel.cpp and search 'HasGlobalSyncOperation' keyword) for
        // performance tunning.

        // [OpenCL 2.0] The following condition covers pipe built-ins as well
        // because the first arguments is a pipe which is a __global opaque pointer.
        if (IS_ADDR_SPACE_GLOBAL(ptr->getAddressSpace()))
          return true;
      }
    }
    return false;
  }

  size_t KernelInfoPass::getExecutionEstimation(unsigned depth) {
    return (size_t)pow(10.0f, (int)depth);
  }

  size_t KernelInfoPass::getExecutionLength(Function *pFunc) {
    LoopInfo &LI = getAnalysis<LoopInfoWrapperPass>().getLoopInfo();

    size_t currLength = 0;
    for (Function::iterator i = pFunc->begin(), e = pFunc->end(); i != e; ++i) {
      currLength += i->size() * getExecutionEstimation(LI.getLoopDepth(&*i));
    }
    return currLength;
  }

  char KernelInfoWrapper::ID = 0;

  bool KernelInfoWrapper::runOnModule(Module& M) {
    KernelInfoPass* pKernelInfoPass = new KernelInfoPass();

    llvm::legacy::FunctionPassManager FPM(&M);
    FPM.add(pKernelInfoPass);

    // Get all kernels
    CompilationUtils::FunctionSet kernelsFunctionSet;
    CompilationUtils::getAllKernels(kernelsFunctionSet, &M);

    // Run on all scalar functions for handling and handle them
    FPM.doInitialization();
    for ( CompilationUtils::FunctionSet::iterator fi = kernelsFunctionSet.begin(),
      fe = kernelsFunctionSet.end(); fi != fe; ++fi ) {
        Function *pFunc = dyn_cast<Function>(*fi);
        assert(pFunc && "got NULL kernel");
        FPM.run(*pFunc);
    }
    FPM.doFinalization();

    return false;
  }

} // namespace intel {

extern "C" {
  ModulePass* createKernelInfoWrapperPass() {
    return new intel::KernelInfoWrapper();
  }
}
