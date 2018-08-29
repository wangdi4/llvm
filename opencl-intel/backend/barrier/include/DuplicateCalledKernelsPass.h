// INTEL CONFIDENTIAL
//
// Copyright 2012-2018 Intel Corporation.
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

#ifndef __DUPLICATE_CALLED_KERNELS_PASS_H__
#define __DUPLICATE_CALLED_KERNELS_PASS_H__

#include "BarrierUtils.h"

#include "llvm/Pass.h"
#include "llvm/IR/Module.h"

using namespace llvm;

namespace intel {


  /// @brief Duplicate Called Kernels pass, simply duplicate each kernel
  /// that is called from other kernel/function.
  /// When duplicating a kernel, this pass generate a new function
  /// that will be called instead of the original kernel.
  //  P.S. It assumes that CloneFunction handles llvm debug info right.
  class DuplicateCalledKernels : public ModulePass {

  public:
    static char ID;

    /// @brief C'tor
    DuplicateCalledKernels();

    /// @brief D'tor
    ~DuplicateCalledKernels() {}

    /// @brief Provides name of pass
    virtual llvm::StringRef getPassName() const {
      return "Intel OpenCL DuplicateCalledKernels";
    }

    /// @brief execute pass on given module
    /// @param M module to optimize
    /// @returns True if module was modified
    virtual bool runOnModule(Module &M);
  };

} // namespace intel

#endif // __DUPLICATE_CALLED_KERNELS_PASS_H__

