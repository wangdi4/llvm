// INTEL CONFIDENTIAL
//
// Copyright 2020 Intel Corporation.
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
//
//
// This file contains implementation of AddFastMath Pass.
//
// When -cl-fast-relaxed-math is specified and the input is SPIR-V, 'fast' flag
// might be missing. The pass adds the flag to floating-point operations and
// call instructions returning floating-point values.
//

#include "AddFastMath.h"
#include "OCLPassSupport.h"
#include "llvm/IR/InstIterator.h"

using namespace llvm;

extern "C" {
FunctionPass *createAddFastMathPass() {
  return new intel::AddFastMath();
}
}

namespace intel {

char AddFastMath::ID = 0;

OCL_INITIALIZE_PASS(
    AddFastMath, "add-fast-math",
    "Add 'fast' flag for floating-point operations and call",
    false, false)

AddFastMath::AddFastMath() : FunctionPass(ID) {
  initializeAddFastMathPass(*llvm::PassRegistry::getPassRegistry());
}

bool AddFastMath::runOnFunction(Function &F) {
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

} // namespace intel
