/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/

#ifndef __BUILT_IN_FUNCTION_IMPORT_H__
#define __BUILT_IN_FUNCTION_IMPORT_H__

#include "BuiltinLibInfo.h"

#include <llvm/ADT/DenseSet.h>
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
    typedef std::vector<llvm::Function *>       FunctionsVec;

    // Type used to hold a set of Functions during traversal.
    typedef std::set<llvm::Function *>          FunctionsSet;

  public:
    // Pass identification, replacement for typeid.
    static char ID;

    /// @brief Constructor
    BIImport(const char *CPUPrefix = "");

    /// @brief Provides name of pass
    virtual llvm::StringRef getPassName() const {
      return "BIImport";
    }

    /// @brief Main entry point. Find all builtins to import, and import them
    ///        along with callees and globals.
    /// @param M The destination module.
    bool runOnModule(Module &M);

    virtual void getAnalysisUsage(AnalysisUsage &AU) const {
      AU.addRequired<BuiltinLibInfo>();
    }

  protected:
    /// @brief update svml function names from shared libraries to
    ///        reflect cpu prefix
    /// @param [IN] fn function to process
    /// @param [IN] pCPUPrefix prefix that will replace 'shared' substr
    void UpdateSvmlBuiltinName(Function *F, const char *CPUPrefix) const;

    /// @brief Get all the functions called by given function.
    /// @param [IN] pFunc The given function.
    /// @param [OUT] calledFuncs The list of all functions called by pFunc.
    void GetCalledFunctions(const Function *Func,
                            FunctionsVec &CalledFuncs) const;

    /// @brief Find all functions and global variables
    ///        from the \p Modules used by the function \p Root
    ///
    /// ExploreUses will recursively look in the functions called by
    /// the \p Root, gathering a complete list of \p Root dependencies.
    ///
    /// @param [IN] Root top level function for lookup
    /// @param [IN] Modules modules with definitions of \p Root dependencies
    /// @param [OUT] UsedFunctions functions used by the \p Root
    /// @param [OUT] UsedGlobals global variables used by the \p Root
    void ExploreUses(Function *Root,
                     SmallVectorImpl<Module *> &Modules,
                     SmallPtrSetImpl<GlobalValue *> &UsedFunctions,
                     SmallPtrSetImpl<GlobalVariable *> &UsedGlobals);
  protected:
    /// @brief holds cpu perfix that would replace 'shared' substr in svml funcs
    const std::string m_cpuPrefix;

    /// @brief Source module list - contains the source functions to import
    SmallVector<Module *, 2> m_runtimeModuleList;
  };

} //namespace Intel {

#endif // __BUILT_IN_FUNCTION_IMPORT_H__
