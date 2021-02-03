//=------------------ CreateSimdVariantPropagation.cpp -*- C++ -*------------=//
//
// Copyright (C) 2020 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //

#include "CreateSimdVariantPropagation.h"

#include "llvm/ADT/SmallSet.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"

#define DEBUG_TYPE "CreateSimdVariantPropagation"

using namespace llvm;

extern bool EnableVectorVariantPasses;

namespace intel {

char CreateSimdVariantPropagation::ID = 0;

OCL_INITIALIZE_PASS_BEGIN(
    CreateSimdVariantPropagation, "create-simd-variant-propagation",
    "Propagate __intel_create_simd_variant attributes to callees", false, false)
OCL_INITIALIZE_PASS_END(
    CreateSimdVariantPropagation, "create-simd-variant-propagation",
    "Propagate __intel_create_simd_variant attributes to callees", false, false)

CreateSimdVariantPropagation::CreateSimdVariantPropagation() : ModulePass(ID) {
  initializeCreateSimdVariantPropagationPass(*PassRegistry::getPassRegistry());
}

bool CreateSimdVariantPropagation::runOnModule(Module &M) {
  if (!EnableVectorVariantPasses)
    return false;

  bool Modified = false;

  DenseMap<Function *, SmallSet<std::string, 4>> VariantsToAdd;

  // Process all variant creation call instructions.
  for (auto &F : M) {

    if (!F.getName().startswith("__intel_create_simd_variant"))
      continue;

    for (User *U : F.users()) {

      auto *Call = dyn_cast<CallInst>(U);
      if (!Call || Call->getCalledFunction() != &F)
        continue;

      Value *Arg = Call->getArgOperand(0);
      Function *FPtr = cast<Function>(Arg);

      assert(Call->hasFnAttr("vector-variants"));
      Attribute Attr = Call->getFnAttr("vector-variants");
      StringRef VarsStr = Attr.getValueAsString();
      VariantsToAdd[FPtr].insert(VarsStr.str());
    }
  }

  for (auto It : VariantsToAdd) {
    Function *F = It.first;
    std::string Vars = join(It.second, ",");
    if (F->hasFnAttribute("vector-variants")) {
      Attribute Attr = F->getFnAttribute("vector-variants");
      Vars = (Attr.getValueAsString() + "," + Vars).str();
    }
    F->addFnAttr("vector-variants", Vars);
    Modified = true;
  }

  return Modified;
}

extern "C" {
ModulePass *createCreateSimdVariantPropagationPass() {
  return new intel::CreateSimdVariantPropagation();
}
}

} // namespace intel
