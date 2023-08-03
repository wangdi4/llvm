//===- DeduceMaxWGDim.cpp - Deduce max WG dimemsion executed ----*- C++ -*-===//
//
// Copyright (C) 2022 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/SYCLTransforms/DeduceMaxWGDim.h"
#include "llvm/ADT/DepthFirstIterator.h"
#include "llvm/Analysis/CallGraph.h"
#include "llvm/Transforms/SYCLTransforms/BuiltinLibInfoAnalysis.h"
#include "llvm/Transforms/SYCLTransforms/Utils/CompilationUtils.h"
#include "llvm/Transforms/SYCLTransforms/Utils/LoopUtils.h"
#include "llvm/Transforms/SYCLTransforms/Utils/MetadataAPI.h"
#include "llvm/Transforms/SYCLTransforms/Utils/RuntimeService.h"

using namespace llvm;
using namespace CompilationUtils;

#define DEBUG_TYPE "sycl-kernel-deduce-max-dim"

static bool runOnFunction(Function &F, CallGraph &CG) {
  // If we have subgroups, then at least one vector iteration is expected,
  // it can't be achieved without a loop.
  auto KIMD = SYCLKernelMetadataAPI::KernelInternalMetadataAPI(&F);
  if (KIMD.KernelHasSubgroups.hasValue() && KIMD.KernelHasSubgroups.get())
    return false;

  int MaxDim = -1;
  std::string GID = mangledGetGID();
  std::string LID = mangledGetLID();

  CallGraphNode *N = CG[&F];
  for (auto It = df_begin(N), E = df_end(N); It != E; ++It) {
    for (const auto &Pair : **It) {
      if (!Pair.first)
        continue;
      auto *Callee = Pair.second->getFunction();
      if (!Callee)
        continue;
      StringRef CalleeName = Callee->getName();
      // Work group builtins can't be achieved without work group loop.
      // If kernel has work group builtins, max WG dimension should not be
      // changed.
      if (isWorkGroupBuiltin(CalleeName))
        return false;

      if (CalleeName != GID && CalleeName != LID)
        continue;

      const auto *CI = cast<CallInst>(*Pair.first);
      auto *Dim = cast<ConstantInt>(CI->getOperand(0));
      MaxDim = std::max(MaxDim, int(Dim->getZExtValue()));
    }
  }

  // No point in saying that kernel needs 3D.
  if (MaxDim >= 2)
    return false;

  KIMD.MaxWGDimensions.set(MaxDim + 1);

  return true;
}

bool DeduceMaxWGDimPass::runImpl(Module &M, RuntimeService &RTS,
                                 CallGraph &CG) {
  FuncSet ForbiddenFuncUsers;
  LoopUtils::fillAtomicBuiltinUsers(M, RTS, ForbiddenFuncUsers);
  LoopUtils::fillWorkItemPipeBuiltinUsers(M, ForbiddenFuncUsers);
  LoopUtils::fillPrintfs(M, ForbiddenFuncUsers);

  // Run on all scalar kernels.
  bool Changed = false;
  FuncSet Kernels = getAllKernels(M);
  for (auto *F : Kernels)
    if (!F->hasOptNone() && !ForbiddenFuncUsers.contains(F))
      Changed |= runOnFunction(*F, CG);

  return Changed;
}

PreservedAnalyses DeduceMaxWGDimPass::run(Module &M,
                                          ModuleAnalysisManager &AM) {
  BuiltinLibInfo *BLI = &AM.getResult<BuiltinLibInfoAnalysis>(M);
  CallGraph &CG = AM.getResult<CallGraphAnalysis>(M);
  (void)runImpl(M, BLI->getRuntimeService(), CG);
  // Only modifies metadata.
  return PreservedAnalyses::all();
}
