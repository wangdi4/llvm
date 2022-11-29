//=------------------------ UpdateCallAttrs.cpp -*- C++ -*-------------------=//
//
// Copyright (C) 2020-2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //

#include "llvm/Transforms/Intel_DPCPPKernelTransforms/VectorVariant/UpdateCallAttrs.h"

#include "llvm/IR/Function.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/InitializePasses.h"

#define DEBUG_TYPE "dpcpp-kernel-update-call-attrs"

using namespace llvm;

extern bool DPCPPEnableVectorVariantPasses;

namespace llvm {

PreservedAnalyses UpdateCallAttrs::run(Module &M, ModuleAnalysisManager &MAM) {
  return runImpl(M) ? PreservedAnalyses::none() : PreservedAnalyses::all();
}

bool UpdateCallAttrs::runImpl(Module &M) {
  if (!DPCPPEnableVectorVariantPasses)
    return false;

  bool Modified = false;

  // Process all call instructions.
  for (auto &F : M)
    for (auto &Inst : instructions(F)) {
      if (Inst.getOpcode() != Instruction::Call)
        continue;

      CallInst &Call = cast<CallInst>(Inst);
      AttributeList Attrs = Call.getAttributes();
      if (Attrs.hasFnAttr("vector-variants"))
        continue;

      Function *Fn = Call.getCalledFunction();
      if (!Fn || !Fn->hasFnAttribute("vector-variants"))
        continue;

      // Update attributes.
      Attribute Attr = Fn->getFnAttribute("vector-variants");
      Attrs = Attrs.addFnAttribute(M.getContext(),
                                   "vector-variants", Attr.getValueAsString());
      Call.setAttributes(Attrs);

      Modified = true;
    }

  return Modified;
}
} // namespace llvm
