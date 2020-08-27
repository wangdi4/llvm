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

#include "BuiltinLibInfo.h"

#include "llvm/IR/Function.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Pass.h"

namespace intel {

using namespace llvm;

class ResolveSubGroupWICall : public ModulePass {

public:
  static char ID;

  ResolveSubGroupWICall();

  llvm::StringRef getPassName() const override {
    return "ResolveSubGroupWICall";
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<BuiltinLibInfo>();
  }

  bool runOnModule(Module &M) override;

private:
  Value *replaceGetSubGroupSize(Instruction *insertBefore, Value *VF,
                                int32_t VD);
  Value *replaceGetMaxSubGroupSize(Instruction *insertBefore, Value *VF,
                                   int32_t VD);
  Value *replaceGetSubGroupLocalId(Instruction *insertBefore, Value *VF,
                                   int32_t VD);

  Value *replaceGetEnqueuedNumSubGroups(Instruction *insertBefore, Value *VF,
                                        int32_t VD);
  Value *replaceGetNumSubGroups(Instruction *insertBefore, Value *VF,
                                int32_t VD);
  Value *replaceSubGroupBarrier(Instruction *insertBefore, Value *VF,
                                int32_t VD);

  Value *replaceGetSubGroupId(Instruction *insertBefore, Value *VF, int32_t VD);

  // Helpers:
  CallInst *createWIFunctionCall(Module *M, char const *twine,
                                 std::string const &name,
                                 Instruction *insertBefore, Value *actPar);

  ConstantInt *createVFConstant(LLVMContext &, const DataLayout &, size_t VF);

private:
  // Constant values to be used by function calls
  Value *m_zero;
  Value *m_one;
  Value *m_two;

  // The return type for the work-item functions
  Type *m_ret;

  // Current Module.
  Module *m_pModule;

  // Pointer to runtime service object
  const RuntimeServices *m_rtServices;
};
} // namespace intel

#endif
