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
