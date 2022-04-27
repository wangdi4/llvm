//===- DeduceMaxWGDim.cpp - Deduce max WG dimemsion executed ----*- C++ -*-===//
//
// Copyright (C) 2022 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/Intel_DPCPPKernelTransforms/DeduceMaxWGDim.h"
#include "llvm/Analysis/CallGraph.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/BuiltinLibInfoAnalysis.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/DPCPPKernelCompilationUtils.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/DPCPPKernelLoopUtils.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/LegacyPasses.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/Utils/MetadataAPI.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/Utils/RuntimeService.h"

using namespace llvm;
using namespace DPCPPKernelCompilationUtils;

#define DEBUG_TYPE "dpcpp-kernel-deduce-max-dim"

namespace {

/// Legacy DeduceMaxWGDim pass.
class DeduceMaxWGDimLegacy : public ModulePass {
  DeduceMaxWGDimPass Impl;

public:
  static char ID;

  DeduceMaxWGDimLegacy() : ModulePass(ID) {
    initializeDeduceMaxWGDimLegacyPass(*PassRegistry::getPassRegistry());
  }

  StringRef getPassName() const override { return "DeduceMaxWGDimLegacy"; }

  bool runOnModule(Module &M) override {
    BuiltinLibInfo *BLI =
        &getAnalysis<BuiltinLibInfoAnalysisLegacy>().getResult();
    return Impl.runImpl(M, BLI->getRuntimeService());
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<BuiltinLibInfoAnalysisLegacy>();
    // Only modifies metadata.
    AU.setPreservesAll();
  }
};

} // namespace

char DeduceMaxWGDimLegacy::ID = 0;

INITIALIZE_PASS_BEGIN(
    DeduceMaxWGDimLegacy, DEBUG_TYPE,
    "Deduce the maximum WG dimemsion that needs to be executed", false, false)
INITIALIZE_PASS_DEPENDENCY(BuiltinLibInfoAnalysisLegacy)
INITIALIZE_PASS_END(DeduceMaxWGDimLegacy, DEBUG_TYPE,
                    "Deduce the maximum WG dimemsion that needs to be executed",
                    false, false)

ModulePass *llvm::createDeduceMaxWGDimLegacyPass() {
  return new DeduceMaxWGDimLegacy();
}

bool DeduceMaxWGDimPass::runImpl(Module &M, RuntimeService *RTService) {
  DPCPPKernelLoopUtils::fillAtomicBuiltinUsers(M, RTService,
                                               ForbiddenFuncUsers);
  DPCPPKernelLoopUtils::fillInternalFuncUsers(M, ForbiddenFuncUsers);
  DPCPPKernelLoopUtils::fillWorkItemPipeBuiltinUsers(M, ForbiddenFuncUsers);
  DPCPPKernelLoopUtils::fillPrintfs(M, ForbiddenFuncUsers);

  // Run on all scalar kernels.
  bool Changed = false;
  FuncSet Kernels = getAllKernels(M);
  for (auto *F : Kernels)
    if (!F->hasOptNone())
      Changed |= runOnFunction(*F);

  return Changed;
}

bool DeduceMaxWGDimPass::runOnFunction(Function &F) {
  if (ForbiddenFuncUsers.contains(&F))
    return false;

  // If we have subgroups, then at least one vector iteration is expected,
  // it can't be achieved without a loop.
  auto KIMD = DPCPPKernelMetadataAPI::KernelInternalMetadataAPI(&F);
  if (KIMD.KernelHasSubgroups.hasValue() && KIMD.KernelHasSubgroups.get())
    return false;

  SmallVector<CallInst *, 8> TIDCalls;
  DPCPPKernelLoopUtils::getAllCallInFunc(mangledGetGID(), &F, TIDCalls);
  DPCPPKernelLoopUtils::getAllCallInFunc(mangledGetLID(), &F, TIDCalls);

  int MaxDim = -1;
  for (auto *I : TIDCalls) {
    auto *Dim = dyn_cast<ConstantInt>(I->getOperand(0));
    // If dimension argument is not a constant, fail.
    if (!Dim)
      return false;
    MaxDim = std::max(MaxDim, int(Dim->getZExtValue()));
  }

  // No point in saying that kernel needs 3D.
  if (MaxDim >= 2)
    return false;

  KIMD.MaxWGDimensions.set(MaxDim + 1);

  return true;
}

PreservedAnalyses DeduceMaxWGDimPass::run(Module &M,
                                          ModuleAnalysisManager &AM) {
  BuiltinLibInfo *BLI = &AM.getResult<BuiltinLibInfoAnalysis>(M);
  (void)runImpl(M, BLI->getRuntimeService());
  // Only modifies metadata.
  return PreservedAnalyses::all();
}
