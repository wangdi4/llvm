/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/

#ifndef __UNDEF_EXTERNAL_FUNCS_H__
#define __UNDEF_EXTERNAL_FUNCS_H__

#include "BuiltinLibInfo.h"
#include "llvm/Pass.h"
#include "llvm/IR/Module.h"

using namespace llvm;

namespace intel {

  /// @brief UndefExternalFuncs analysis pass used to collect the names
  ///        undefined external functions called from the module if such exist
  class UndefExternalFuncs : public ModulePass {

  public:
    /// Pass identification, replacement for typeid
    static char ID;

    /// @brief Constructor
    /// @param undefinedExternalFunctions container to fill with undefined function names
    /// @param runtimeModules conatiner for all the runtime BI modules to check if the
    ///     function supplied by them or not.
    UndefExternalFuncs(std::vector<std::string> &undefinedExternalFunctions) :
        ModulePass(ID), m_pUndefinedExternalFunctions(&undefinedExternalFunctions) {}

    /// @brief Provides name of pass
    virtual llvm::StringRef getPassName() const {
      return "UndefExternalFuncs";
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
      AU.addRequired<intel::BuiltinLibInfo>();
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
  
} // namespace intel {

#endif //__UNDEFINED_EXTERNAL_FUNCTIONS_H__