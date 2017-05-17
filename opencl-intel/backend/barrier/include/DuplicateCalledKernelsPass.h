/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/
#ifndef __DUPLICATE_CALLED_KERNELS_PASS_H__
#define __DUPLICATE_CALLED_KERNELS_PASS_H__

#include "BarrierUtils.h"

#include "llvm/Pass.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"

using namespace llvm;

namespace llvm {
  class DebugInfoFinder;
}

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

