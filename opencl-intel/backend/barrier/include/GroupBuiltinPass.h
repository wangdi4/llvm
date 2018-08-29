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

