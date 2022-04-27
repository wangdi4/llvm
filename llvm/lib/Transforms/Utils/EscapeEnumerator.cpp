//===- EscapeEnumerator.cpp -----------------------------------------------===//
// INTEL_CUSTOMIZATION
//
// INTEL CONFIDENTIAL
//
// Modifications, Copyright (C) 2021 Intel Corporation
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you ("License"). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.
//
// end INTEL_CUSTOMIZATION
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// Defines a helper class that enumerates all possible exits from a function,
// including exception handling.
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/Utils/EscapeEnumerator.h"
#include "llvm/ADT/Triple.h"
#include "llvm/Analysis/EHPersonalities.h"
#include "llvm/IR/Module.h"
#include "llvm/Transforms/Utils/Local.h"

using namespace llvm;

static FunctionCallee getDefaultPersonalityFn(Module *M) {
  LLVMContext &C = M->getContext();
  Triple T(M->getTargetTriple());
  EHPersonality Pers = getDefaultEHPersonality(T);
  return M->getOrInsertFunction(getEHPersonalityName(Pers),
                                FunctionType::get(Type::getInt32Ty(C), true));
}

IRBuilder<> *EscapeEnumerator::Next() {
  if (Done)
    return nullptr;

  // Find all 'return', 'resume', and 'unwind' instructions.
  while (StateBB != StateE) {
    BasicBlock *CurBB = &*StateBB++;

    // Branches and invokes do not escape, only unwind, resume, and return
    // do.
    Instruction *TI = CurBB->getTerminator();
    if (!isa<ReturnInst>(TI) && !isa<ResumeInst>(TI))
      continue;

    if (CallInst *CI = CurBB->getTerminatingMustTailCall())
      TI = CI;
    Builder.SetInsertPoint(TI);
    return &Builder;
  }

  Done = true;

  if (!HandleExceptions)
    return nullptr;

  if (F.doesNotThrow())
    return nullptr;

  // Find all 'call' instructions that may throw.
  // We cannot tranform calls with musttail tag.
  SmallVector<Instruction *, 16> Calls;
  for (BasicBlock &BB : F)
    for (Instruction &II : BB)
      if (CallInst *CI = dyn_cast<CallInst>(&II))
        if (!CI->doesNotThrow() && !CI->isMustTailCall())
          Calls.push_back(CI);

  if (Calls.empty())
    return nullptr;

  // Create a cleanup block.
  LLVMContext &C = F.getContext();
  BasicBlock *CleanupBB = BasicBlock::Create(C, CleanupBBName, &F);
  Type *ExnTy = StructType::get(Type::getInt8PtrTy(C), Type::getInt32Ty(C));
  if (!F.hasPersonalityFn()) {
    FunctionCallee PersFn = getDefaultPersonalityFn(F.getParent());
    F.setPersonalityFn(cast<Constant>(PersFn.getCallee()));
  }

  if (isScopedEHPersonality(classifyEHPersonality(F.getPersonalityFn()))) {
    report_fatal_error("Scoped EH not supported");
  }

  LandingPadInst *LPad =
      LandingPadInst::Create(ExnTy, 1, "cleanup.lpad", CleanupBB);
  LPad->setCleanup(true);
  ResumeInst *RI = ResumeInst::Create(LPad, CleanupBB);

  // Transform the 'call' instructions into 'invoke's branching to the
  // cleanup block. Go in reverse order to make prettier BB names.
  SmallVector<Value *, 16> Args;
  for (unsigned I = Calls.size(); I != 0;) {
    CallInst *CI = cast<CallInst>(Calls[--I]);
#if INTEL_CUSTOMIZATION
    changeToInvokeAndSplitBasicBlock(CI, CleanupBB, nullptr, nullptr, DTU);
#endif // INTEL_CUSTOMIZATION
  }

  Builder.SetInsertPoint(RI);
  return &Builder;
}
