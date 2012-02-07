/*****************************************************************************\

Copyright (c) Intel Corporation (2010-2011).

    INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
    LICENSED ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,
    ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT
    PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY
    DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
    PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability,
    including liability for infringement of any proprietary rights, relating to
    use of the code. No license, express or implied, by estoppels or otherwise,
    to any intellectual property rights is granted herein.

File Name:  PrepareKernelArgs.h

\*****************************************************************************/

#ifndef __PREPARE_KERNEL_ARGS_H__
#define __PREPARE_KERNEL_ARGS_H__

#include "LocalBuffersAnalysis.h"
#include "cl_device_api.h"

#include "llvm/Pass.h"
#include "llvm/Module.h"
#include "llvm/Instructions.h"
#include "llvm/Support/IRBuilder.h"

#include <map>

namespace Intel { namespace OpenCL { namespace DeviceBackend {

  using namespace llvm;

  /// @brief  PrepareKernelArgs changes the way arguments are passed to kernels.
  ///         It changes the kernel to receive as arguments a single buffer
  ///         which contains the the kernel's original and implicit arguments.
  ///         loads the arguments and calls the originaol kernel.
  ///         The position of the arguments in the buffer is calcilated based on
  ///         the arguments' alignment, which in non LLVM dependant.
  /// @Author Marina Yatsina
  class PrepareKernelArgs : public ModulePass {

  public:
    /// Pass identification, replacement for typeid
    static char ID;

    /// @brief Constructor
    PrepareKernelArgs(SmallVectorImpl<Function*> &vectFunctions);

    /// @brief LLVM Module pass entry
    /// @param M Module to transform
    /// @returns true if changed
    bool runOnModule(Module &M);

  protected:
    /// @brief  Creates a wrapper function for the given function that receives
    ///         one buffer as argument, creates load instructions that load the
    ///         function arguments from the buffer, creates a call to the given 
    ///         funciton with the loaded arguments.
    /// @param  pFunc The kernel for which to create a wrapper
    /// @param  isVectorized True if this is a vectorized kernel, false otherwise
    /// @returns The new function wrapper function that calls the given function
    Function* runOnFunction(Function *pFunc, bool isVectorized);
    
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
    
    /// @brief  Creates the body of the wrapper function: 
    ///         creates load instructions that load the function arguments from the buffer, 
    ///         creates a call to the given pFunc with the loaded arguments.
    /// @param  builder An IR builder that allows to add instructions to the wrapper.
    /// @param  pFunc The kernel which is wrapped by the wrapper
    /// @param  pArgsBuffer The single buffer argument that is passed to the wrapper
    ///         the pFunc arguments neew to be loaded from this buffer
    /// @returns A parameters vector - the loaded values that need to be used when calling pFunc
    std::vector<Value*> createArgumentLoads(IRBuilder<>& builder, Function* pFunc, Argument *pArgsBuffer);
    

  private:
    /// @brief The llvm module this pass needs to update
    Module                     *m_pModule;

    /// @brief The llvm context
    LLVMContext                *m_pLLVMContext;
    
    /// @brief Maps each function and its metadata
    std::map<Function*, MDNode*> m_kernelsMetadata;

    /// @brief A pointer to the vectorized functions set gotten from the vectorizer pass
    SmallVectorImpl<Function*> *m_pVectFunctions;

    /// @brief Maps old function with its new cloned function
    ///        (that takes extra implicite arguments)
    std::map<Function*, Function*> m_oldToNewFunctionMap;

  };

}}} // namespace Intel { namespace OpenCL { namespace DeviceBackend {

#endif // __PREPARE_KERNEL_ARGS_H__
