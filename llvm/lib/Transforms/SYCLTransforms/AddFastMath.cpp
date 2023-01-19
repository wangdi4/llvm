//===--- AddFastMath.cpp - AddFastMath pass - C++ -* ---------------------===//
//
// Copyright (C) 2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
//
// This file contains implementation of AddFastMath Pass.
//
// When -cl-fast-relaxed-math is specified and the input is SPIR-V, 'fast' flag
// might be missing. The pass adds the flag to floating-point operations and
// call instructions returning floating-point values.
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/SYCLTransforms/AddFastMath.h"
#include "llvm/IR/InstIterator.h"

using namespace llvm;

bool AddFastMathPass::runImpl(Function &F) {
  bool Changed = false;
  // We do not set 'fast' for select and phi instructions to conform the
  // behavior of 'clang -cl-fast-relaxed-math', although these 2 instructions
  // support them.
  // See also: https://llvm.org/docs/LangRef.html#fast-math-flags
  for (Instruction &I : instructions(F)) {
    switch (I.getOpcode()) {
    case Instruction::Call: {
      const Type *Ty = I.getType();
      if (Ty->isArrayTy())
        Ty = Ty->getArrayElementType();
      Ty = Ty->getScalarType();
      if (!Ty->isFloatingPointTy())
        break;
    }
      LLVM_FALLTHROUGH;
    case Instruction::FAdd:
    case Instruction::FSub:
    case Instruction::FMul:
    case Instruction::FDiv:
    case Instruction::FRem:
    case Instruction::FCmp:
      I.setFast(true);
      Changed = true;
    }
  }
  return Changed;
}

PreservedAnalyses AddFastMathPass::run(Function &F,
                                       FunctionAnalysisManager &AM) {
  if (!runImpl(F))
    return PreservedAnalyses::all();
  return PreservedAnalyses::none();
}
