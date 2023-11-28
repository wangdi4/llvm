//=------------------ CreateSimdVariantPropagation.cpp -*- C++ -*------------=//
//
// Copyright (C) 2020 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //

#include "llvm/Transforms/SYCLTransforms/Intel_VectorVariant/CreateSimdVariantPropagation.h"
#include "llvm/ADT/SmallSet.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/Analysis/CallGraph.h"
#include "llvm/Analysis/VectorUtils.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Instructions.h"

using namespace llvm;

extern bool SYCLEnableVectorVariantPasses;

PreservedAnalyses
CreateSimdVariantPropagation::run(Module &M, ModuleAnalysisManager &MAM) {
  if (!runImpl(M))
    return PreservedAnalyses::all();
  PreservedAnalyses PA;
  PA.preserve<CallGraphAnalysis>();
  PA.preserveSet<CFGAnalyses>();
  return PA;
}

bool CreateSimdVariantPropagation::runImpl(Module &M) {
  if (!SYCLEnableVectorVariantPasses)
    return false;

  bool Modified = false;

  DenseMap<Function *, SmallSet<std::string, 4>> VariantsToAdd;

  // Process all variant creation call instructions.
  for (auto &F : M) {

    if (F.hasOptNone())
      continue;

    if (!F.getName().startswith("__intel_create_simd_variant"))
      continue;

    for (User *U : F.users()) {

      auto *Call = dyn_cast<CallInst>(U);
      if (!Call || Call->getCalledFunction() != &F)
        continue;

      Value *Arg = Call->getArgOperand(0);
      Function *FPtr = cast<Function>(Arg);

      assert(Call->hasFnAttr(VectorUtils::VectorVariantsAttrName));
      Attribute Attr =
          Call->getCallSiteOrFuncAttr(VectorUtils::VectorVariantsAttrName);
      StringRef VarsStr = Attr.getValueAsString();
      VariantsToAdd[FPtr].insert(VarsStr.str());
    }
  }

  for (const auto &It : VariantsToAdd) {
    Function *F = It.first;
    std::string Vars = join(It.second, ",");
    if (F->hasFnAttribute(VectorUtils::VectorVariantsAttrName)) {
      Attribute Attr = F->getFnAttribute(VectorUtils::VectorVariantsAttrName);
      Vars = (Attr.getValueAsString() + "," + Vars).str();
    }
    F->addFnAttr(VectorUtils::VectorVariantsAttrName, Vars);
    Modified = true;
  }

  return Modified;
}
