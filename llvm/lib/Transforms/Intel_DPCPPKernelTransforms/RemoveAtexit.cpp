//==-- RemoveAtexit.cpp - Remove atexit call temporarily -------------------==//
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

#include "llvm/Transforms/Intel_DPCPPKernelTransforms/RemoveAtexit.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/InitializePasses.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/DPCPPKernelCompilationUtils.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/LegacyPasses.h"

using namespace llvm;

#define DEBUG_TYPE "dpcpp-kernel-remove-atexit"

namespace {
/// Legacy RemoveAtExitLegacy pass.
class RemoveAtExitLegacy : public ModulePass {
public:
  static char ID;

  RemoveAtExitLegacy() : ModulePass(ID) {
    initializeRemoveAtExitLegacyPass(*PassRegistry::getPassRegistry());
  }

  StringRef getPassName() const override { return "RemoveAtExitLegacy"; }

  bool runOnModule(Module &M) override;

private:
  RemoveAtExitPass Impl;
};

} // namespace

char RemoveAtExitLegacy::ID = 0;

INITIALIZE_PASS(RemoveAtExitLegacy, DEBUG_TYPE,
                "remove atexit call temporarily", false, false)

bool RemoveAtExitLegacy::runOnModule(Module &M) { return Impl.runImpl(M); }

ModulePass *llvm::createRemoveAtExitLegacyPass() {
  return new RemoveAtExitLegacy();
}

PreservedAnalyses RemoveAtExitPass::run(Module &M, ModuleAnalysisManager &AM) {
  return runImpl(M) ? PreservedAnalyses::none() : PreservedAnalyses::all();
}

bool RemoveAtExitPass::runImpl(Module &M) {
  auto *Func = M.getFunction("__cxa_atexit");
  if (!Func)
    return false;
  bool Changed = false;
  for (auto *U : make_early_inc_range(Func->users())) {
    if (!isa<CallInst>(U))
      continue;
    auto *CI = cast<CallInst>(U);
    if (!DPCPPKernelCompilationUtils::isGlobalCtorDtorOrCPPFunc(
            CI->getFunction()))
      continue;
    assert(CI->use_empty() && "It should have no users");
    CI->eraseFromParent();
    Changed = true;
  }
  return Changed;
}
