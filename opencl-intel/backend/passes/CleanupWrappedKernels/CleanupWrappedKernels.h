//==--- CleanupWrappedKernels.h - a removing wrapped kernels pass -*- C++ -*---==//
////
//// Copyright (C) 2017 Intel Corporation. All rights reserved.
////
//// The information and source code contained herein is the exclusive property
//// of Intel Corporation and may not be disclosed, examined or reproduced in
//// whole or in part without explicit written authorization from the company.
////
//// ===--------------------------------------------------------------------=== //

#ifndef __CLEANUP_WRAPPED_KERNELS_H__
#define __CLEANUP_WRAPPED_KERNELS_H__

#include <llvm/Pass.h>

namespace intel {
  // That pass deletes a body for wrapped kernels preserving Metadata.
  // TODO: remove kernel_wrapper metadata, instead of creating it we should
  // update the opencl.kernel metadata.
  class CleanupWrappedKernels : public llvm::ModulePass {
  public:
    CleanupWrappedKernels() : llvm::ModulePass(ID) {}

    // LLVM Module pass entry
    bool runOnModule(llvm::Module& M) override;
    // Pass identification, replacement for typeid
    static char ID;
  };
}

#endif // __CLEANUP_WRAPPED_KERNELS_H__
