//===- IntelDebugRemoveXDeref.h -- Remove DW_OP_xderef Opcodes ------------===//
//
// Copyright (C) 2023 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// Removes no-op address space dereferences from debug location expressions.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_DEBUGINFO_INTELDEBUGREMOVEXDEREF_H
#define LLVM_DEBUGINFO_INTELDEBUGREMOVEXDEREF_H

namespace llvm {

class DbgVariableIntrinsic;
class Function;
class GlobalVariable;
class Module;
class TargetMachine;

class IntelDebugRemoveXDeref {
private:
  TargetMachine *TM;

public:
  IntelDebugRemoveXDeref() = delete;
  IntelDebugRemoveXDeref(TargetMachine *TM);
  bool run(Module &M);

protected:
  bool run(Function &F);
  bool run(DbgVariableIntrinsic &DVI);
  bool run(GlobalVariable &GV);
};

} // namespace llvm

#endif // LLVM_DEBUGINFO_INTELDEBUGREMOVEXDEREF_H
