//===-- IntelVPlanFunctionVectorizer.cpp ----------------------------------===//
//
//   Copyright (C) 2020 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//
//
// Implements VPlanFunctionVectorizerPass/VPlanFunctionVectorizerLegacyPass.
//
// TODO: Assert for loop-simplified form (for inner loops).
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/Vectorize/IntelVPlanFunctionVectorizer.h"
#include "IntelVPlanCFGBuilder.h"
#include "llvm/Analysis/GlobalsModRef.h"
#include "llvm/Analysis/Intel_Andersens.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"
#include "llvm/Transforms/Utils.h"
#include "llvm/Transforms/Vectorize.h"

#define DEBUG_TYPE "vplan-function-vectorizer"

static cl::opt<bool> DumpAfterPlainCFG(
    "print-after-vplan-func-vec-cfg-import",
    cl::desc("Print after CFG import for VPlan Function vectorization "),
    cl::init(false), cl::Hidden);

using namespace llvm;
using namespace llvm::vpo;
namespace {
class VPlanFunctionVectorizer {
  Function &F;

public:
  VPlanFunctionVectorizer(Function &F) : F(F) {}

  bool run() {
    auto Plan = std::make_unique<VPlan>(&F.getContext(),
                                        &F.getParent()->getDataLayout());
    VPlanFunctionCFGBuilder Builder(Plan.get(), F);
    Builder.buildCFG();
    Plan->setName(F.getName());

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
    if (DumpAfterPlainCFG)
      Plan->dump(outs());
#endif // !NDEBUG || LLVM_ENABLE_DUMP

    return false;
  }
};
} // namespace
INITIALIZE_PASS_BEGIN(VPlanFunctionVectorizerLegacyPass, "vplan-func-vec",
                      "VPlan Function Vectorization Driver", false, false)
INITIALIZE_PASS_END(VPlanFunctionVectorizerLegacyPass, "vplan-func-vec",
                    "VPlan Function Vectorization Driver", false, false)

Pass *llvm::createVPlanFunctionVectorizerPass() {
  return new VPlanFunctionVectorizerLegacyPass();
}

char VPlanFunctionVectorizerLegacyPass::ID = 0;

VPlanFunctionVectorizerLegacyPass::VPlanFunctionVectorizerLegacyPass()
    : FunctionPass(ID) {
  initializeVPlanFunctionVectorizerLegacyPassPass(
      *PassRegistry::getPassRegistry());
}

void VPlanFunctionVectorizerLegacyPass::getAnalysisUsage(
    AnalysisUsage &AU) const {
  AU.addPreserved<AndersensAAWrapperPass>();
  AU.addPreserved<GlobalsAAWrapperPass>();
}

bool VPlanFunctionVectorizerLegacyPass::runOnFunction(Function &Fn) {
  if (skipFunction(Fn))
    return false;

  VPlanFunctionVectorizer FunctionVectorizer(Fn);
  return FunctionVectorizer.run();
}

PreservedAnalyses
VPlanFunctionVectorizerPass::run(Function &F, FunctionAnalysisManager &AM) {
  VPlanFunctionVectorizer FunctionVectorizer(F);
  if (!FunctionVectorizer.run())
    return PreservedAnalyses::all();

  auto PA = PreservedAnalyses::none();
  PA.preserve<AndersensAA>();
  PA.preserve<GlobalsAA>();
  return PA;
}
