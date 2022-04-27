//===--- ExternalizeGlobalVariables.cpp -----------------------------------===//
//
// Copyright (C) 2022 Intel Corporation
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you ("License"). Unless the License provides otherwise, you may
// not use, modify, copy, publish, distribute, disclose or transmit this
// software or the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/Intel_DPCPPKernelTransforms/ExternalizeGlobalVariables.h"
#include "ImplicitArgsUtils.h"
#include "llvm/InitializePasses.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/DPCPPKernelCompilationUtils.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/LegacyPasses.h"

using namespace llvm;

#define DEBUG_TYPE "dpcpp-kernel-externalize-global-variables"
namespace {
class ExternalizeGlobalVariablesLegacy : public ModulePass {
public:
  static char ID;

  ExternalizeGlobalVariablesLegacy();

  bool runOnModule(Module &M) override;

  StringRef getPassName() const override {
    return "ExternalizeGlobalVariables";
  }

private:
  ExternalizeGlobalVariablesPass Impl;
};
} // namespace

ModulePass *llvm::createExternalizeGlobalVariablesLegacyPass() {
  return new ExternalizeGlobalVariablesLegacy();
}

char ExternalizeGlobalVariablesLegacy::ID = 0;

INITIALIZE_PASS(ExternalizeGlobalVariablesLegacy, DEBUG_TYPE,
                "ExternalizeGlobalVariables: change linkage type for global"
                " variables with private/internal linkage to external.",
                false, false)

bool ExternalizeGlobalVariablesLegacy::runOnModule(Module &M) {
  return Impl.runImpl(M);
}

ExternalizeGlobalVariablesLegacy::ExternalizeGlobalVariablesLegacy()
    : ModulePass(ID) {
  initializeExternalizeGlobalVariablesLegacyPass(
      *PassRegistry::getPassRegistry());
}

PreservedAnalyses
ExternalizeGlobalVariablesPass::run(Module &M, ModuleAnalysisManager &AM) {
  return runImpl(M) ? PreservedAnalyses::none() : PreservedAnalyses::all();
}

bool ExternalizeGlobalVariablesPass::runImpl(Module &M) {
  bool Changed = false;
  SmallSet<Value *, 8> TLSGlobals;
  for (unsigned I = 0; I < ImplicitArgsUtils::NUM_IMPLICIT_ARGS; ++I) {
    GlobalVariable *GV = DPCPPKernelCompilationUtils::getTLSGlobal(&M, I);
    TLSGlobals.insert(GV);
  }

  for (auto &GVar : M.globals()) {
    bool SkipConvertLinkage =
        TLSGlobals.count(&GVar) || !GVar.hasName() ||
        (GVar.hasName() && GVar.getName().startswith("llvm.")) ||
        (GVar.getLinkage() != GlobalValue::InternalLinkage &&
         GVar.getLinkage() != GlobalValue::PrivateLinkage);
    if (SkipConvertLinkage)
      continue;

    LLVM_DEBUG(dbgs() << "Converting " << GVar.getName()
                      << " to external linkage.\n");
    GVar.setLinkage(GlobalValue::ExternalLinkage);
    Changed = true;
  }

  return Changed;
}
