/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/

#ifndef __BLOCK_TO_FUNC_PTR_PASS_H__
#define __BLOCK_TO_FUNC_PTR_PASS_H__

#include "BuiltinLibInfo.h"
#include <llvm/Pass.h>
#include <llvm/IR/Module.h>

using namespace llvm;

// @name  BlockToFuncPtr
// @brief Replace pointers to %opencl.block opaque type  with function pointer casts.
// More concretely:
// This pass creates a cast for pointer to %opencl.block opaque type to
// function pointer. This transfomations is needed to link device execution
// built-ins without function type bitcast in BIImport.

namespace intel {

  class BlockToFuncPtr : public ModulePass {

  public:
    // Pass identification, replacement for typeid.
    static char ID;

    /// @brief Constructor
    BlockToFuncPtr() : ModulePass(ID) {}

    /// @brief Provides name of pass
    virtual StringRef getPassName() const {
      return "BlockToFuncPtr";
    }

    /// @brief Main entry point. Replace all block uses
    ///                          with function pointer casts.
    /// @param M The destination module.
    bool runOnModule(Module &M);

    virtual void getAnalysisUsage(AnalysisUsage &AU) const {
      AU.addRequired<BuiltinLibInfo>();
    }
  };
} // namespace intel
#endif // __BLOCK_TO_FUNC_PTR_PASS_H__

