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

#ifndef __PREPARE_KERNEL_ARGS_H__
#define __PREPARE_KERNEL_ARGS_H__

#include "LocalBuffAnalysis.h"

#include "llvm/Pass.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IRBuilder.h"
#include "ImplicitArgsAnalysis/ImplicitArgsAnalysis.h"

#include <map>

using namespace llvm;
using namespace intel;

namespace intel {
  /// @brief  PrepareKernelArgs changes the way arguments are passed to kernels.
  ///         It changes the kernel to receive as arguments a single buffer
  ///         which contains the the kernel's original and implicit arguments.
  ///         loads the arguments and calls the original kernel.
  ///         The position of the arguments in the buffer is calculated based on
  ///         the arguments' alignment, which in non LLVM dependant.
  class PrepareKernelArgs : public ModulePass {

  public:
    /// Pass identification, replacement for typeid
    static char ID;

    /// @brief Constructor
    PrepareKernelArgs();

    /// @brief Provides name of pass
    virtual llvm::StringRef getPassName() const {
      return "PrepareKernelArgs";
    }

    /// @brief LLVM Module pass entry
    /// @param M Module to transform
    /// @returns true if changed
    bool runOnModule(Module &M);

    virtual void getAnalysisUsage(AnalysisUsage &AU) const {
      AU.addRequired<ImplicitArgsAnalysis>();
    }
  protected:
    /// @brief  Creates a wrapper function for the given function that receives
    ///         one buffer as argument, creates load instructions that load the
    ///         function arguments from the buffer, creates a call to the given
    ///         funciton with the loaded arguments.
    /// @param  pKernel The kernel which is wrapped by the wrapper
    /// @returns true if changed
    bool runOnFunction(Function *pFunc);

    /// @brief  Creates a new function that receives as argument a single buffer
    ///         based on the given function's name, return type and calling convention.
    /// @param  pFunc The kernel for which to create a wrapper function
    /// @returns A new function
    Function* createWrapper(Function* pFunc);

    /// @brief  Creates the body of the wrapper function:
    ///         creates load instructions that load the function arguments from the buffer,
    ///         creates a call to the given pFunc with the loaded arguments.
    /// @param  pWrapper The kernel for which to create a wrapper function
    /// @param  pFunc The kernel which is wrapped by the wrapper
    void createWrapperBody(Function* pWrapper, Function* pFunc);

    /// @brief  Replaces function pointers to the original function by pointers
    ///         to wrapper one in device execution built-in calls.
    void replaceFunctionPointers(Function* pWrapper, Function* pFunc);

    /// @brief  Creates the body of the wrapper function:
    ///         creates load instructions that load the function arguments from the buffer,
    ///         creates a call to the given pFunc with the loaded arguments.
    /// @param  builder An IR builder that allows to add instructions to the wrapper.
    /// @param  pFunc The kernel which is wrapped by the wrapper
    /// @param  pArgsBuffer The single buffer argument that is passed to the wrapper
    ///         the pFunc arguments need to be loaded from this buffer
    /// @returns A parameters vector - the loaded values that need to be used when calling pFunc
    std::vector<Value *> createArgumentLoads(IRBuilder<> &builder,
                                             Function *pKernel,
                                             Argument *pArgsBuffer,
                                             Argument *pArgGID,
                                             Argument *RuntimeContext);

    Type* getGIDWrapperArgType() const;
    Type* getRuntimeContextWrapperArgType() const;
  private:
    /// @brief The llvm module this pass needs to update
    Module                *m_pModule;

    const DataLayout      *m_DL;

    /// @brief The llvm context
    LLVMContext           *m_pLLVMContext;

    intel::ImplicitArgsAnalysis *m_IAA;

    /// @brief Size of Moudle pointer in bits
    unsigned              m_PtrSizeInBytes;
    IntegerType*          m_SizetTy;
    IntegerType*          m_I8Ty;
    IntegerType*          m_I32Ty;
  };

} // namespace intel

#endif // __PREPARE_KERNEL_ARGS_H__
