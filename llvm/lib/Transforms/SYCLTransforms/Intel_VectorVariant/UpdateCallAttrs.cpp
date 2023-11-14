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
#include "llvm/Support/ModRef.h"
#include "llvm/Transforms/SYCLTransforms/Utils/CompilationUtils.h"

#define DEBUG_TYPE "sycl-kernel-update-call-attrs"

using namespace llvm;

extern bool SYCLEnableVectorVariantPasses;

namespace llvm {

PreservedAnalyses UpdateCallAttrs::run(Module &M, ModuleAnalysisManager &MAM) {
  return runImpl(M) ? PreservedAnalyses::none() : PreservedAnalyses::all();
}

// TID builtin does not access memory. The attribute may be proprogated to a
// user function. Call to the function is then uniform in vectorizer divergence
// analysis. However, the call should be divergent.
// So we change the function attribute to read/write memory.
static bool updateTIDBuiltinUserFuncAttrs(Module &M) {
  bool Changed = false;
  using namespace CompilationUtils;
  FuncSet Kernels = getAllKernels(M);
  std::string TIDNames[] = {mangledGetGID(), mangledGetLID(),
                            mangledGetSubGroupLocalId()};
  for (const auto &TIDName : TIDNames) {
    Function *TIDFunc = M.getFunction(TIDName);
    if (!TIDFunc)
      continue;
    SmallVector<Function *, 16> WorkList{TIDFunc};
    SmallPtrSet<Function *, 16> Visited;
    while (!WorkList.empty()) {
      Function *Curr = WorkList.pop_back_val();
      Visited.insert(Curr);
      for (User *U : Curr->users()) {
        auto *CI = dyn_cast<CallInst>(U);
        if (!CI)
          continue;
        auto *F = CI->getFunction();
        if (!F->hasFnAttribute(VectorUtils::VectorVariantsAttrName) ||
            Kernels.contains(F))
          continue;
        F->addFnAttr(Attribute::getWithMemoryEffects(M.getContext(),
                                                     MemoryEffects::unknown()));
        if (!Visited.contains(F))
          WorkList.push_back(F);
      }
    }
    Changed |= Visited.size() > 1;
  }

  return Changed;
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

  Modified |= updateTIDBuiltinUserFuncAttrs(M);

  return Modified;
}
} // namespace llvm
