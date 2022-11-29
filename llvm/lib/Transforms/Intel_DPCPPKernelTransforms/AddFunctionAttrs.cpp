//===- AddFunctionAttrs.cpp - Add function attributes -----------*- C++ -*-===//
//
// Copyright (C) 2021-2022 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/Intel_DPCPPKernelTransforms/AddFunctionAttrs.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/InitializePasses.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/Utils/CompilationUtils.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/Utils/LoopUtils.h"

using namespace llvm;
using namespace CompilationUtils;

#define DEBUG_TYPE "dpcpp-kernel-add-function-attrs"

static bool isAMXMatrixIntrinsicFunction(Function &F) {
  switch (F.getIntrinsicID()) {
  case Intrinsic::experimental_matrix_load:
  case Intrinsic::experimental_matrix_store:
  case Intrinsic::experimental_matrix_mad:
    return true;
  default:
    return false;
  }
}

static bool addAMXMatrixIntrinsicAttributes(Module &M) {
  bool Changed = false;
  for (auto &F : M) {
    if (!isAMXMatrixIntrinsicFunction(F))
      continue;
    F.addFnAttr(Attribute::Convergent);
    F.addFnAttr(KernelAttribute::CallOnce);
    F.addFnAttr(KernelAttribute::UniformCall);
    F.addFnAttr(KernelAttribute::OCLVecUniformReturn);
    Changed = true;
  }
  return Changed;
}

static bool handlePrintfBuiltinAttributes(Module &M) {
  Function *F = M.getFunction(namePrintf());
  if (!F)
    return false;

  bool Changed = false;
  for (User *U : F->users()) {
    auto *CI = dyn_cast<CallInst>(U);
    if (!CI || CI->arg_size() < 2)
      continue;
    // Set NoBuiltin attribute to avoid replacements by 'puts'/'putc'.
    CI->addFnAttr(Attribute::NoBuiltin);
    Changed = true;
  }

  return Changed;
}

static bool handleSyncBuiltinAttributes(Module &M) {
  // Get all synchronize built-ins declared in module.
  FuncSet SyncBuiltins = getAllSyncBuiltinsDeclsForNoDuplicateRelax(M);
  if (SyncBuiltins.empty()) {
    // No synchronize functions to mark.
    return false;
  }

  // Get all function that calls synchronize built-ins in/direct.
  FuncSet SyncFunctions;
  LoopUtils::fillFuncUsersSet(SyncBuiltins, SyncFunctions);

  SyncFunctions.insert(SyncBuiltins.begin(), SyncBuiltins.end());

  for (Function *F : SyncFunctions) {
    // Process function (definitions and declaration attributes).
    F->setAttributes(
        F->getAttributes()
            .addFnAttribute(F->getContext(), Attribute::Convergent)
            .addFnAttribute(F->getContext(), KernelAttribute::ConvergentCall)
            .addFnAttribute(F->getContext(), KernelAttribute::CallOnce)
            .removeFnAttribute(F->getContext(),  Attribute::NoDuplicate));

    // Process call sites.
    for (User *U : F->users()) {
      if (auto *CI = dyn_cast<CallInst>(U)) {
        CI->setAttributes(
            CI->getAttributes()
                .addFnAttribute(CI->getContext(), Attribute::Convergent)
                .addFnAttribute(CI->getContext(), KernelAttribute::ConvergentCall)
                .addFnAttribute(CI->getContext(), KernelAttribute::CallOnce)
                .removeFnAttribute(CI->getContext(), Attribute::NoDuplicate));
      }
    }
  }
  return true;
}

bool AddFunctionAttrsPass::runImpl(Module &M) {
  bool Changed = false;

  // Add "convergent", "kernel-call-once", "kernel-uniform-call" and
  // "opencl-vec-uniform-return" to AMX matrix intrinsics.
  Changed |= addAMXMatrixIntrinsicAttributes(M);

  Changed |= handlePrintfBuiltinAttributes(M);

  Changed |= handleSyncBuiltinAttributes(M);

  return Changed;
}

PreservedAnalyses AddFunctionAttrsPass::run(Module &M,
                                            ModuleAnalysisManager &) {
  return runImpl(M) ? PreservedAnalyses::none() : PreservedAnalyses::all();
}
