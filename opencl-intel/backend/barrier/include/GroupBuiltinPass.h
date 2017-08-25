/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/
#ifndef __GROUP_BUILTIN_PASS_H__
#define __GROUP_BUILTIN_PASS_H__

#include "BarrierUtils.h"
#include "BuiltinLibInfo.h"

#include "llvm/Pass.h"
#include "llvm/IR/Module.h"

using namespace llvm;

namespace intel {

  /// @brief GroupBuiltinHandler pass is a module pass that handles calls to
  /// group built-ins instructions, e.g. async_copy, etc.
  /// It provides that their execution will be synchronized across all WIs
  class GroupBuiltin : public ModulePass {

  public:
    static char ID;

    /// @brief C'tor
    GroupBuiltin();

    /// @brief D'tor
    ~GroupBuiltin() {}

    /// @brief Provides name of pass
    virtual llvm::StringRef getPassName() const {
      return "Intel OpenCL GroupBuiltinPass";
    }

    /// @brief execute pass on given module
    /// @param M module to optimize
    /// @returns True if module was modified
    virtual bool runOnModule(Module &M);


    /// @brief Inform about usage/mofication/dependency of this pass
    virtual void getAnalysisUsage(AnalysisUsage &AU) const {
      AU.addRequired<BuiltinLibInfo>();
    }

  private:
    /// This module
    Module *m_pModule;

    /// This is a list of built-in modules
    SmallVector<Module*, 2> m_builtinModuleList;

    /// This context
    LLVMContext *m_pLLVMContext;

    /// size_t type
    IntegerType *m_pSizeT;

    /// This is barrier utility class
    BarrierUtils m_util;

    /// Generate initialization value for a WG function
    Constant *getInitializationValue(Function *pFunc);

    /// Implement call to get_local_linear_id(). 
    Instruction *getLinearID(CallInst *pWgCallInstr);

    /// Generate linear ID out of ID indices
    Value *calculateLinearID(CallInst *pWgCallInstr);

    /// Helper for WI function call generation.
    /// Generates a call to WI function upon its name and dimension index
    CallInst *getWICall(Instruction *pBefore, std::string funcName, unsigned dimIdx);

    /// Find a builtin function in builtin module list
    Function* FindFunctionInModule(const std::string& funcName);

  };

} // namespace intel

#endif // __GROUP_BUILTIN_PASS_H__

