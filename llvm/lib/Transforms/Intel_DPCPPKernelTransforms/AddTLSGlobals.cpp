//===--- AddTLSGlobals.cpp - AddTLSGlobals pass - C++ -* ------------------===//
//
// Copyright (C) 2022 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/Intel_DPCPPKernelTransforms/AddTLSGlobals.h"
#include "llvm/InitializePasses.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/LegacyPasses.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/Utils/CompilationUtils.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/Utils/ImplicitArgsUtils.h"

using namespace llvm;

// Command line option for other passes to use TLS mode
bool EnableTLSGlobals;
static cl::opt<bool, true> OptEnableTLSGlobals(
    "dpcpp-kernel-enable-tls-globals", cl::desc("Enable TLS globals"),
    cl::location(EnableTLSGlobals), cl::init(false), cl::Hidden);

namespace {
/// @brief  AddTLSGlobals class adds TLS global variables to the module
class AddTLSGlobalsLegacy : public ModulePass {

public:
  static char ID;

  AddTLSGlobalsLegacy();

  StringRef getPassName() const override { return "AddTLSGlobalsLegacy"; }

  bool runOnModule(Module &M) override;

  virtual void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<ImplicitArgsAnalysisLegacy>();
    AU.addPreserved<ImplicitArgsAnalysisLegacy>();
  }

private:
  AddTLSGlobalsPass Impl;
};

} // namespace

char AddTLSGlobalsLegacy::ID = 0;

INITIALIZE_PASS_BEGIN(AddTLSGlobalsLegacy, "dpcpp-kernel-add-tls-globals",
                      "Adds TLS global variables to the module", false, false)
INITIALIZE_PASS_DEPENDENCY(ImplicitArgsAnalysisLegacy)
INITIALIZE_PASS_END(AddTLSGlobalsLegacy, "dpcpp-kernel-add-tls-globals",
                    "Adds TLS global variables to the module", false, false)

AddTLSGlobalsLegacy::AddTLSGlobalsLegacy() : ModulePass(ID) {
  initializeAddTLSGlobalsLegacyPass(*llvm::PassRegistry::getPassRegistry());
}

bool AddTLSGlobalsLegacy::runOnModule(Module &M) {
  ImplicitArgsAnalysisLegacy *IAA = &getAnalysis<ImplicitArgsAnalysisLegacy>();
  ImplicitArgsInfo *IAInfo = &(IAA->getResult());
  return Impl.runImpl(M, IAInfo);
}

PreservedAnalyses AddTLSGlobalsPass::run(Module &M, ModuleAnalysisManager &AM) {
  ImplicitArgsInfo *IAInfo = &AM.getResult<ImplicitArgsAnalysis>(M);
  if (!runImpl(M, IAInfo))
    return PreservedAnalyses::all();
  PreservedAnalyses PA;
  PA.preserve<ImplicitArgsAnalysis>();
  return PA;
}

bool AddTLSGlobalsPass::runImpl(Module &M, ImplicitArgsInfo *IAInfo) {
  this->M = &M;
  Ctx = &M.getContext();

  // Create TLS globals
  for (unsigned I = 0; I < ImplicitArgsUtils::NUM_IMPLICIT_ARGS; ++I) {
    // TODO handle name conflicts
    assert(!M.getGlobalVariable(ImplicitArgsUtils::getArgName(I)));
    Type *ArgType = IAInfo->getArgType(I);
    GlobalVariable *GV = new GlobalVariable(
        M, ArgType, false, GlobalValue::LinkOnceODRLinkage,
        UndefValue::get(ArgType), ImplicitArgsUtils::getArgName(I), nullptr,
        GlobalValue::GeneralDynamicTLSModel);
    GV->setAlignment(M.getDataLayout().getPreferredAlign(GV));
    if (I == ImplicitArgsUtils::IA_SLM_BUFFER)
      LocalMemBase = GV;
  }

  // Collect all module functions that are not declarations for handling
  SmallVector<Function *> FunctionsToHandle;
  for (Function &Func : M) {
    if (Func.isDeclaration()) {
      // Function is not defined inside module
      continue;
    }
    // No need to handle global ctors/dtors
    if (CompilationUtils::isGlobalCtorDtorOrCPPFunc(&Func))
      continue;

    FunctionsToHandle.push_back(&Func);
  }

  return true;
}

ModulePass *llvm::createAddTLSGlobalsLegacyPass() {
  return new AddTLSGlobalsLegacy();
}
