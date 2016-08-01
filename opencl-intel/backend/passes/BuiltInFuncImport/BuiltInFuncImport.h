/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/

#ifndef __BUILT_IN_FUNCTION_IMPORT_H__
#define __BUILT_IN_FUNCTION_IMPORT_H__

#include "BuiltinLibInfo.h"
#include <llvm/Pass.h>
#include <llvm/IR/Constants.h>
#include <llvm/Transforms/Utils/ValueMapper.h>

#include <vector>
#include <set>

using namespace llvm;

namespace intel {

  class BIImport : public ModulePass {
  protected:
    // Type used to hold a vector of Functions during traversal.
    typedef std::vector<llvm::Function*>       TFunctionsVec;

    // Type used to hold a set of Functions during traversal.
    typedef std::set<llvm::Function*>          TFunctionsSet;

  public:
    // Pass identification, replacement for typeid.
    static char ID;

    /// @brief Constructor
    BIImport(const char* CPUPrefix = "");

    /// @brief Provides name of pass
    virtual const char *getPassName() const {
      return "BIImport";
    }

    /// @brief Main entry point.
    ///        Find all builtins to import, and import them along with callees and globals.
    /// @param M The destination module.
    bool runOnModule(Module &M);

    virtual void getAnalysisUsage(AnalysisUsage &AU) const {
      AU.addRequired<BuiltinLibInfo>();
    }

  protected:
    /// @brief nuke the unused globals so we could materializeAll() quickly
    /// @param [IN] src_module RTL module to process
    void CleanUnusedGlobalsInitializers(Module *src_module) const;

    /// @brief nuke the unused functions bodies so we could materializeAll() quickly
    /// @param [IN] src_module RTL module to process
    void CleanUnusedFunctionsBodies(Module *src_module) const;

    /// @brief update svml function names from shared libraries to reflect cpu prefix
    /// @param [IN] fn function to process
    /// @param [IN] pCPUPrefix prefix that will replace 'shared' substr
    void UpdateSvmlBuiltinName(Function* fn, const char* pCPUPrefix) const;

    /// @brief Get all the functions called by given function.
    /// @param [IN] pFunc The given function.
    /// @param [OUT] calledFuncs The list of all functions called by pFunc.
    void GetCalledFunctions(const Function* pFunc, TFunctionsVec& calledFuncs) const;

    /// @brief Find functions in the list of RTL builtin modules
    /// @param [IN] funcName name of the function to find
    /// @return found function, it is either materialized or materialazible, or nullptr otherwise
    Function* FindFunctionBodyInModules(const std::string& funcName) const;

  protected:
    /// @brief holds cpu perfix that would replace 'shared' substr in svml funcs
    const std::string m_cpuPrefix;

    /// @brief Source module list - contains the source functions to import
    SmallVector<Module*, 2> m_runtimeModuleList;

    /// @brief holds original source module functions
    TFunctionsSet m_UserModuleFunctions;
  };

} //namespace Intel {

#endif // __BUILT_IN_FUNCTION_IMPORT_H__
