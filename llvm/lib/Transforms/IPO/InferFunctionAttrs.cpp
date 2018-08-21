//===- InferFunctionAttrs.cpp - Infer implicit function attributes --------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/IPO/InferFunctionAttrs.h"
#include "llvm/Analysis/Intel_WP.h"                 // INTEL
#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Utils/BuildLibCalls.h"
using namespace llvm;

#define DEBUG_TYPE "inferattrs"

#if INTEL_CUSTOMIZATION
// For experimenting with new keywords use -force-attribute option
const std::array<StringRef, 2> ErrorHandlingKeywords = {"croak", "warn"};

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

static bool inferAllPrototypeAttributes(Module &M,
                                        const TargetLibraryInfo &TLI) {
  bool Changed = false;

#if INTEL_CUSTOMIZATION
  for (Function &F : M.functions())
  {
    // We only infer things using the prototype and the name; we don't need
    // definitions.
    if (F.isDeclaration() && !F.hasFnAttribute((Attribute::OptimizeNone)))
      Changed |= inferLibFuncAttributes(F, TLI);
    if (!F.hasFnAttribute(Attribute::OptimizeNone))
      Changed |= addColdAttrToErrHandleFunc(F);
  }
#endif // INTEL_CUSTOMIZATION

  return Changed;
}

PreservedAnalyses InferFunctionAttrsPass::run(Module &M,
                                              ModuleAnalysisManager &AM) {
  auto &TLI = AM.getResult<TargetLibraryAnalysis>(M);

  if (!inferAllPrototypeAttributes(M, TLI))
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

    auto &TLI = getAnalysis<TargetLibraryInfoWrapperPass>().getTLI();
    return inferAllPrototypeAttributes(M, TLI);
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
