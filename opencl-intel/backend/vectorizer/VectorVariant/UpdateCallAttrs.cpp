//=------------------------ UpdateCallAttrs.cpp -*- C++ -*-------------------=//
//
// Copyright (C) 2020 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //

#include "UpdateCallAttrs.h"

#include "llvm/IR/Function.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"

#define DEBUG_TYPE "UpdateCallAttrs"

using namespace llvm;

extern bool EnableVectorVariantPasses;

namespace intel {

char UpdateCallAttrs::ID = 0;

OCL_INITIALIZE_PASS_BEGIN(UpdateCallAttrs, "update-call-attrs",
                          "Update vector attributes of call statements", false,
                          false)
OCL_INITIALIZE_PASS_END(UpdateCallAttrs, "update-call-attrs",
                        "Update vector attributes of call statements", false,
                        false)

UpdateCallAttrs::UpdateCallAttrs() : ModulePass(ID) {
  initializeUpdateCallAttrsPass(*PassRegistry::getPassRegistry());
}

bool UpdateCallAttrs::runOnModule(Module &M) {
  if (!EnableVectorVariantPasses)
    return false;

  bool Modified = false;

  // Process all call instructions.
  for (auto &F : M)
    for (auto &Inst : instructions(F)) {
      if (Inst.getOpcode() != Instruction::Call)
        continue;

      CallInst &Call = cast<CallInst>(Inst);
      AttributeList Attrs = Call.getAttributes();
      if (Attrs.hasAttribute(AttributeList::FunctionIndex, "vector-variants"))
        continue;

      Function *Fn = Call.getCalledFunction();
      if (!Fn || !Fn->hasFnAttribute("vector-variants"))
        continue;

      // Update attributes.
      Attribute Attr = Fn->getFnAttribute("vector-variants");
      Attrs = Attrs.addAttribute(M.getContext(), AttributeList::FunctionIndex,
                                 "vector-variants", Attr.getValueAsString());
      Call.setAttributes(Attrs);

      Modified = true;
    }

  return Modified;
}

extern "C" {
ModulePass *createUpdateCallAttrsPass() { return new intel::UpdateCallAttrs(); }
}

} // namespace intel
