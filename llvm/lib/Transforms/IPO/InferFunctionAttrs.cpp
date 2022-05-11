//===- InferFunctionAttrs.cpp - Infer implicit function attributes --------===//
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

#include "llvm/Transforms/IPO/InferFunctionAttrs.h"
#include "llvm/Analysis/Intel_WP.h"                 // INTEL
#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"
#include "llvm/InitializePasses.h"
#include "llvm/Transforms/Utils/BuildLibCalls.h"
#include "llvm/Transforms/Utils/Local.h"
using namespace llvm;

#define DEBUG_TYPE "inferattrs"

#if INTEL_CUSTOMIZATION
// For experimenting with new keywords use -force-attribute option
const std::array<StringRef, 3> ErrorHandlingKeywords = {"croak", "warn", "signal"};

// We detect functions that contain special keywords in their names.
// If there is a match we assume they are error handling functions,
// and thus add 'cold' attribute to them. Later, inlining cost and
// branch probabilities will be adjusted based on this metadata.
static bool addColdAttrToErrHandleFunc(Function &F) {
  const StringRef FuncName = F.getName();
  bool FoundMatch =
      std::any_of(ErrorHandlingKeywords.begin(), ErrorHandlingKeywords.end(),
                  [&FuncName](const StringRef &ErrHndlKeyword) {
                    return FuncName.contains(ErrHndlKeyword);
                  });

  if (FoundMatch) {
    if (!F.hasFnAttribute(Attribute::Cold)) {
      F.addFnAttr(Attribute::Cold);
      return true;
    }
  }
  return false;
}
#endif // INTEL_CUSTOMIZATION

static bool inferAllPrototypeAttributes(
    Module &M, function_ref<TargetLibraryInfo &(Function &)> GetTLI) {
  bool Changed = false;

#if INTEL_CUSTOMIZATION
  for (Function &F : M.functions())
  {
    // We only infer things using the prototype and the name; we don't need
    // definitions.  This ensures libfuncs are annotated and also allows our
    // CGSCC inference to avoid needing to duplicate the inference from other
    // attribute logic on all calls to declarations (as declarations aren't
    // explicitly visited by CGSCC passes in the new pass manager.)
    if (F.isDeclaration() && !F.hasOptNone()) {
      if (!F.hasFnAttribute(Attribute::NoBuiltin))
        Changed |= inferNonMandatoryLibFuncAttrs(F, GetTLI(F));
      Changed |= inferAttributesFromOthers(F);
    }
    if (!F.hasFnAttribute(Attribute::OptimizeNone))
      Changed |= addColdAttrToErrHandleFunc(F);
  }
#endif // INTEL_CUSTOMIZATION

  return Changed;
}

PreservedAnalyses InferFunctionAttrsPass::run(Module &M,
                                              ModuleAnalysisManager &AM) {
  FunctionAnalysisManager &FAM =
      AM.getResult<FunctionAnalysisManagerModuleProxy>(M).getManager();
  auto GetTLI = [&FAM](Function &F) -> TargetLibraryInfo & {
    return FAM.getResult<TargetLibraryAnalysis>(F);
  };

  if (!inferAllPrototypeAttributes(M, GetTLI))
    // If we didn't infer anything, preserve all analyses.
    return PreservedAnalyses::all();

  // Otherwise, we may have changed fundamental function attributes, so clear
  // out all the passes.
  return PreservedAnalyses::none();
}

namespace {
struct InferFunctionAttrsLegacyPass : public ModulePass {
  static char ID; // Pass identification, replacement for typeid
  InferFunctionAttrsLegacyPass() : ModulePass(ID) {
    initializeInferFunctionAttrsLegacyPassPass(
        *PassRegistry::getPassRegistry());
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addPreserved<WholeProgramWrapperPass>();               // INTEL
    AU.addRequired<TargetLibraryInfoWrapperPass>();
  }

  bool runOnModule(Module &M) override {
    if (skipModule(M))
      return false;

    auto GetTLI = [this](Function &F) -> TargetLibraryInfo & {
      return this->getAnalysis<TargetLibraryInfoWrapperPass>().getTLI(F);
    };
    return inferAllPrototypeAttributes(M, GetTLI);
  }
};
}

char InferFunctionAttrsLegacyPass::ID = 0;
INITIALIZE_PASS_BEGIN(InferFunctionAttrsLegacyPass, "inferattrs",
                      "Infer set function attributes", false, false)
INITIALIZE_PASS_DEPENDENCY(TargetLibraryInfoWrapperPass)
INITIALIZE_PASS_END(InferFunctionAttrsLegacyPass, "inferattrs",
                    "Infer set function attributes", false, false)

Pass *llvm::createInferFunctionAttrsLegacyPass() {
  return new InferFunctionAttrsLegacyPass();
}
