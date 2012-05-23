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

File Name:  UndefinedExternalFunctions.h

\*****************************************************************************/

#ifndef __UNDEFINED_EXTERNAL_FUNCTIONS_H__
#define __UNDEFINED_EXTERNAL_FUNCTIONS_H__

#include "llvm/Pass.h"
#include "llvm/Module.h"

using namespace llvm;

namespace Intel { namespace OpenCL { namespace DeviceBackend {

  /// @brief UndefExternalFunctions analysis pass used to collect the names
  ///        undefined external functions called from the module if such exist
  class UndefExternalFunctions : public ModulePass {

  public:
    /// Pass identification, replacement for typeid
    static char ID;

    /// @brief Constructor
    /// @param undefinedExternalFunctions container to fill with undefined function names
    /// @param runtimeModules conatiner for all the runtime BI modules to check if the
    ///     function supplied by them or not.
    UndefExternalFunctions(std::vector<std::string> &undefinedExternalFunctions, 
        const std::vector<llvm::Module*>& runtimeModules) :
        ModulePass(ID), m_pUndefinedExternalFunctions(&undefinedExternalFunctions), 
        m_RuntimeModules(runtimeModules) {}

    /// @brief Provides name of pass
    virtual const char *getPassName() const {
      return "UndefExternalFunctions";
    }

    /// @brief LLVM Module pass entry
    /// @param M Module to transform
    /// @returns true if changed
    bool runOnModule(Module &M);

    /// @brief LLVM Interface
    /// @param AU Analysis
    virtual void getAnalysisUsage(AnalysisUsage &AU) const {
      // Analysis pass preserve all
      AU.setPreservesAll();
    }

  protected:

    /// @brief Search the given function name in the runtime modules, return true
    ///        If found; false otherwise
    /// @param name function name
    bool SearchForFunction(const std::string& name);
    
   protected:
    /// Curently not used (it should be filled with names
    /// of non resolved external function called from this module.
    std::vector<std::string> *m_pUndefinedExternalFunctions;

    /// container of all the runtime modules which will be linked with the module
    /// JIT code, it supplies all the extenral functions implementations
    std::vector<llvm::Module*> m_RuntimeModules;

  };
  
}}} // namespace Intel { namespace OpenCL { namespace DeviceBackend {

#endif //__UNDEFINED_EXTERNAL_FUNCTIONS_H__