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

File Name:  AddImplicitArgs.h

\*****************************************************************************/

#ifndef __ADD_IMPLICIT_ARGS_H__
#define __ADD_IMPLICIT_ARGS_H__

#include "LocalBuffersAnalysis.h"
#include "cl_device_api.h"

#include "llvm/Pass.h"
#include "llvm/Module.h"
#include "llvm/Instructions.h"

#include <map>

namespace Intel { namespace OpenCL { namespace DeviceBackend {

  using namespace llvm;

  /// @brief  AddImplicitArgs class adds the implicit arguments to signature 
  ///         of all function of the module (that are defined inside the module)
  /// @Author Marina Yatsina
  class AddImplicitArgs : public ModulePass {

  public:
    /// Pass identification, replacement for typeid
    static char ID;

    /// @brief Constructor
    AddImplicitArgs(Pass *pVectorizer, SmallVectorImpl<Function*> &vectFunctions);

    /// @brief LLVM Module pass entry
    /// @param M Module to transform
    /// @returns true if changed
    bool runOnModule(Module &M);

    /// @brief LLVM Interface
    /// @param AU Analysis
    virtual void getAnalysisUsage(AnalysisUsage &AU) const {
      // Depends on LocalBuffersAnalysis for finding all local buffers each function uses directly
      AU.addRequired<LocalBuffersAnalysis>();
    }

  protected:
    /// @brief Replaces the given function with a copy function that receives
    ///        the implicit arguemnts, maps call instructions that appear in
    ///        the given funciton and need to add implicit arguments to its
    ///        original arguments, i.e. calls to functions define in module.
    /// @param pFunc The function to create a copy of
    /// @param isAKernel true is the gevin function is a kernel
    /// @returns The new function that receives the implicit arguemnts
    Function* runOnFunction(Function *pFunc, bool isAKernel);

    /// @brief Adds implicit arguments structure declarations to the module
    void addWIInfoDeclarations();

  private:
    /// @brief The llvm module this pass needs to update
    Module                     *m_pModule;

    /// @brief The LocalBuffersAnalysis pass, on which the current pass depends
    LocalBuffersAnalysis       *m_localBuffersAnalysis;

    /// @brief The llvm context
    LLVMContext                *m_pLLVMContext;

    /// @brief Maps call instructions to the implicit arguments needed to patch up the call
    std::map<llvm::CallInst *, llvm::Value **> m_fixupCalls;

    /// @brief The Vectorizer pass needed to get the vectorized functions
    Pass                       *m_pVectorizer;

    /// @brief A pointer to the vectorized functions set gotten from the vectorizer pass
    SmallVectorImpl<Function*> *m_pVectFunctions;

    /// @brief Maps old function with its new cloned function
    ///        (that takes extra implicite arguments)
    std::map<Function*, Function*> m_oldToNewFunctionMap;

  };

}}} // namespace Intel { namespace OpenCL { namespace DeviceBackend {

#endif // __ADD_IMPLICIT_ARGS_H__
