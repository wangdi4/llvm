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
    ~DuplicateCalledKernels() {};

    /// @brief Provides name of pass
    virtual const char *getPassName() const {
      return "Intel OpenCL DuplicateCalledKernels";
    }

    /// @brief execute pass on given module
    /// @param M module to optimize
    /// @returns True if module was modified
    virtual bool runOnModule(Module &M);

  private:
    /// @brief Duplicate a function's debug metadata.
    /// @param cuNode The MDNode of the CompilationUnit.
    /// @param finder A DebugInfoFinder instance that has already processed the
    ///               current module.
    /// @param pFunc pointer to the old function.
    /// @param pNewFunc pointer to the new function.
    void duplicateDebugMD(MDNode* cuNode, const DebugInfoFinder& finder,
                          Function* pFunc, Function* pNewFunc) const;

    /// @brief Duplicate an MDNode, replacing or adding a single operand.
    /// @param node The MDNode to duplicate.
    /// @param toReplace The value of the operand to replace. If NULL, the
    ///                  parameter 'with' will just be appended to the operands.
    /// @param with The value to use when replacing 'toReplace'.
    /// @return A pointer to the new MDNode. If 'toReplace' is not an existing
    ///         operand and is not NULL, or if 'toReplace' and 'with' are the
    ///         same and are not NULL, this will return the original node.
    MDNode* duplicateMDnode(MDNode* node, Value* toReplace, Value* with) const;

    /// @brief Find an MDNode corresponding to the subprogram debug metadata
    ///        for a given function.
    /// @param finder A DebugInfoFinder instance that has already processed the
    ///               current module.
    /// @param pFunc The function for which to find the subprogram metadata.
    const MDNode* findSubprogram(const DebugInfoFinder& finder, const Function* pFunc) const;
  };

} // namespace intel

#endif // __DUPLICATE_CALLED_KERNELS_PASS_H__

