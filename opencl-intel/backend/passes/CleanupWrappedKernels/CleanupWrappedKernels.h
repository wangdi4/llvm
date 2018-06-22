// INTEL CONFIDENTIAL
//
// Copyright 2017-2018 Intel Corporation.
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
