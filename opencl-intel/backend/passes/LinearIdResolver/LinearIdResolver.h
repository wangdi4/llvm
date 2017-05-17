/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/

#ifndef __LINEAR_ID_RESOLVER_H__
#define __LINEAR_ID_RESOLVER_H__

#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/Instructions.h"

namespace intel {

  using namespace llvm;

  /// @brief This pass replaces opencl 2.0 work item functions get_{global,local}_linear_id
  /// with their explicit calculation using get_{global,local}_id, get_{global,local}_size,
  /// and get_global_offset
  class LinearIdResolver : public FunctionPass {

  public:

    static char ID;

    LinearIdResolver() : FunctionPass(ID) { }

    virtual llvm::StringRef getPassName() const {
      return "LinearIdResolver";
    }

    virtual bool runOnFunction(Function &F);

  private:

    /// @brief generate calculation sequence of get_local_linear_id
    /// @param M            - module
    /// @param insertBefore - insert before instruction
    Instruction * replaceGetLocalLinearId(Module * M, Instruction * insertBefore);

    /// @brief generate calculation sequence of get_global_linear_id
    /// @param M            - module
    /// @param insertBefore - insert before instruction
    Instruction * replaceGetGlobalLinearId(Module * M, Instruction * insertBefore);

    /// @brief create the actual call instruction to a given builtin function
    /// @param M            - module
    /// @param twine        - SSA register twine
    /// @param name         - name of the function to call
    /// @param insertBefore - insert before instruction
    /// @param actPar       - builtin parameter
    CallInst * createWIFunctionCall(Module * M, char const* twine, std::string const& name,
                                    Instruction * insertBefore, Value *actPar);

    // Constant values to be used by function calls
    Value * m_zero;
    Value * m_one;
    Value * m_two;

    // The return type for the work-item functions
    Type  * m_ret;

  };
}

#endif
