// INTEL CONFIDENTIAL
//
// Copyright 2019 Intel Corporation.
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

#ifndef __RESOLVE_SUBGROUP_WI_CALL__
#define __RESOLVE_SUBGROUP_WI_CALL__

#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/Instructions.h"

namespace intel {

  using namespace llvm;

  class ResolveSubGroupWICall : public FunctionPass {

  public:

    static char ID;

    ResolveSubGroupWICall();

    virtual llvm::StringRef getPassName() const override {
      return "ResolveSubGroupWICall";
    }

    virtual bool runOnFunction(Function &F) override;

  private:

    Value* replaceGetSubGroupSize(Module *M, Value *insertBefore, size_t VF);
    Value* replaceGetMaxSubGroupSize(Module *M, Value *insertBefore, size_t VF);
    Value* replaceGetSubGroupLocalId(Module *M, Value *insertBefore, size_t VF);

    Instruction* replaceGetEnqueuedNumSubGroups(
      Module *M, Instruction *insertBefore, size_t VF);
    Instruction* replaceGetNumSubGroups(
      Module *M, Instruction *insertBefore, size_t VF);

    Instruction* replaceGetSubGroupId(Module *M, Instruction *insertBefore, size_t VF);

    // Helpers:
    CallInst * createWIFunctionCall(
      Module *M, char const *twine, std::string const &name,
      Instruction *insertBefore, Value *actPar);

    ConstantInt* createVFConstant(LLVMContext&, const DataLayout&, size_t VF);

  private:

    // Constant values to be used by function calls
    Value *m_zero;
    Value *m_one;
    Value *m_two;

    // The return type for the work-item functions
    Type  *m_ret;

  };
}

#endif
