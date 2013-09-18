/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/

#ifndef __CLONE_BLOCK_INVOKE_FUNC_TO_KERNEL_H__
#define __CLONE_BLOCK_INVOKE_FUNC_TO_KERNEL_H__

#include "llvm/Pass.h"
#include "llvm/Module.h"

/*
    CloneBlockInvokeFuncToKernelPass pass finds in module
    all blockInvoke functions that may be enqueued as kernels
    -- for each found function F
      -- remove internal linkage type from blockInvoke. In order to have it in global values map
      -- creates kernel with name org_name+_kernel. Update opencl.kernels metadata
*/

namespace intel {
  using namespace llvm;
  struct CloneBlockInvokeFuncToKernel : public ModulePass {
    static char ID;
    /// ctor
    CloneBlockInvokeFuncToKernel()
      : ModulePass(ID) {}

    /// main function
    virtual bool runOnModule(Module &M);

  }; // struct CloneBlockInvokeFuncToKernel

  /// create pass
  Pass *createCloneBlockInvokeFuncToKernelPass();
} // namespace intel
#endif // __CLONE_BLOCK_INVOKE_FUNC_TO_KERNEL_H__
