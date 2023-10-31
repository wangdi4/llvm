//=------------------------ UpdateCallAttrs.cpp -*- C++ -*-------------------=//
//
// Copyright (C) 2020 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //

#include "llvm/Transforms/SYCLTransforms/Intel_VectorVariant/UpdateCallAttrs.h"
#include "llvm/Analysis/VectorUtils.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"

#define DEBUG_TYPE "sycl-kernel-update-call-attrs"

using namespace llvm;

extern bool SYCLEnableVectorVariantPasses;

namespace llvm {

PreservedAnalyses UpdateCallAttrs::run(Module &M, ModuleAnalysisManager &MAM) {
  return runImpl(M) ? PreservedAnalyses::none() : PreservedAnalyses::all();
}

bool UpdateCallAttrs::runImpl(Module &M) {
  if (!SYCLEnableVectorVariantPasses)
    return false;

  bool Modified = false;

  // Process all call instructions.
  for (auto &F : M)
    for (auto &Inst : instructions(F)) {
      if (Inst.getOpcode() != Instruction::Call)
        continue;

      CallInst &Call = cast<CallInst>(Inst);
      AttributeList Attrs = Call.getAttributes();
      if (Attrs.hasFnAttr(VectorUtils::VectorVariantsAttrName))
        continue;

      Function *Fn = Call.getCalledFunction();
      if (!Fn || !Fn->hasFnAttribute(VectorUtils::VectorVariantsAttrName))
        continue;

      // Update attributes.
      Attribute Attr = Fn->getFnAttribute(VectorUtils::VectorVariantsAttrName);
      Attrs = Attrs.addFnAttribute(M.getContext(),
                                   VectorUtils::VectorVariantsAttrName,
                                   Attr.getValueAsString());
      Call.setAttributes(Attrs);

      Modified = true;
    }

  return Modified;
}
} // namespace llvm
