//==-- DuplicateCalledKernels.cpp - DuplicateCalledKernels pass --*- C++ -*-==//
//
// Copyright (C) 2020 - 2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/Intel_DPCPPKernelTransforms/DuplicateCalledKernelsPass.h"
#include "llvm/IR/Instructions.h"
#include "llvm/InitializePasses.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/LegacyPasses.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/Utils/MetadataAPI.h"
#include "llvm/Transforms/Utils/Cloning.h"

using namespace llvm;

#define DEBUG_TYPE "dpcpp-kernel-duplicate-called-kernels"

char DuplicateCalledKernelsLegacy::ID = 0;

INITIALIZE_PASS(DuplicateCalledKernelsLegacy, DEBUG_TYPE,
                "DuplicateCalledKernelsLegacy Pass - Clone kernels called from "
                "other functions",
                false, false)

DuplicateCalledKernelsLegacy::DuplicateCalledKernelsLegacy() : ModulePass(ID) {
  initializeDuplicateCalledKernelsLegacyPass(*PassRegistry::getPassRegistry());
}

bool DuplicateCalledKernelsLegacy::runOnModule(Module &M) {
  return Impl.runImpl(M);
}

PreservedAnalyses DuplicateCalledKernelsPass::run(Module &M,
                                                  ModuleAnalysisManager &) {
  return runImpl(M) ? PreservedAnalyses::none() : PreservedAnalyses::all();
}

bool DuplicateCalledKernelsPass::runImpl(Module &M) {
  using namespace DPCPPKernelMetadataAPI;

  bool Changed = false;
  for (Function *Func : KernelList(&M)) {
    bool IsCalledKernel = false;
    for (User *U : Func->users()) {
      if (isa<CallInst>(U)) {
        IsCalledKernel = true;
        break;
      }
    }
    if (!IsCalledKernel)
      continue;
    Changed = true;

    ValueToValueMapTy VMap;
    Function *NewFunc = CloneFunction(Func, VMap, nullptr);
    NewFunc->setName("__internal." + Func->getName());

    // Run over old uses of pFuncToFix and replace with call to NewFunc
    for (User *U : Func->users()) {
      if (auto *CI = dyn_cast<CallInst>(U)) {
        // Replace call to kernel function with call to new function.
        CI->replaceUsesOfWith(Func, NewFunc);
      }
    }
  }
  return Changed;
}

ModulePass *llvm::createDuplicateCalledKernelsLegacyPass() {
  return new llvm::DuplicateCalledKernelsLegacy();
}
