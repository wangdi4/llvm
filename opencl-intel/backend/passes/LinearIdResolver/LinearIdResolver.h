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
  /// with their explicit calculation using get_{global,local}_id and get_{global,local}_size
  class LinearIdResolver : public FunctionPass {

  public:

    static char ID;

    LinearIdResolver() : FunctionPass(ID) { }

    virtual const char * getPassName() const {
      return "LinearIdResolver";
    }

    virtual bool runOnFunction(Function &F);

  private:

    /// @brief replaces get_{global,local}_linear_id with their calculation using
    /// get_{global,local}_id and get_{global,local}_size
    /// @param M        - module
    /// @param after    - insert the instruction after after
    /// @param idName   - id function to use (to distinguish between local and global)
    /// @param sizeName - size function to use (to distinguish between local and global)
    Instruction * replaceGetLinearId(Module * M, Instruction * after, std::string idName, std::string sizeName);

    /// @brief create the actual call instruction to a given builtin function
    /// @param M      - module
    /// @param name   - name of the function to call
    /// @param after  - insert the instruction after after
    /// @param actPar - builtin parameter
    CallInst * createWIFunctionCall(Module * M, std::string name, Instruction * after, Value *actPar);

    // Constant values to be used by function calls
    Value * m_zero;
    Value * m_one;
    Value * m_two;

    // The return type for the work-item functions
    Type  * m_ret;

  };
}

#endif
