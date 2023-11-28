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
#include "llvm/Analysis/OptimizationRemarkEmitter.h"
#include "llvm/IR/DiagnosticInfo.h"
#include "llvm/Transforms/SYCLTransforms/BuiltinLibInfoAnalysis.h"
#include "llvm/Transforms/SYCLTransforms/Utils/CompilationUtils.h"
#include "llvm/Transforms/SYCLTransforms/Utils/LoopUtils.h"
#include "llvm/Transforms/SYCLTransforms/Utils/MetadataAPI.h"
#include "llvm/Transforms/SYCLTransforms/Utils/RuntimeService.h"

using namespace llvm;
using namespace CompilationUtils;

#define DEBUG_TYPE "sycl-kernel-deduce-max-dim"

static void runOnFunction(Function &F, CallGraph &CG,
                          OptimizationRemarkEmitter &ORE) {
  // If we have subgroups, then at least one vector iteration is expected,
  // it can't be achieved without a loop.
  auto KIMD = SYCLKernelMetadataAPI::KernelInternalMetadataAPI(&F);
  if (KIMD.KernelHasSubgroups.hasValue() && KIMD.KernelHasSubgroups.get())
    return;

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
        return;

      if (CalleeName != GID && CalleeName != LID)
        continue;

      const auto *CI = cast<CallInst>(*Pair.first);
      auto *Dim = cast<ConstantInt>(CI->getOperand(0));
      MaxDim = std::max(MaxDim, int(Dim->getZExtValue()));
    }
  }

  // No point in saying that kernel needs 3D.
  if (MaxDim >= 2)
    return;

  KIMD.MaxWGDimensions.set(MaxDim + 1);

  ORE.emit([&]() {
    return OptimizationRemark(DEBUG_TYPE, "MaxWGDimension", &F)
           << "max work-group dimension of kernel " << F.getName()
           << " is deduced to " << Twine(MaxDim + 1).str();
  });
}

PreservedAnalyses DeduceMaxWGDimPass::run(Module &M,
                                          ModuleAnalysisManager &AM) {
  BuiltinLibInfo *BLI = &AM.getResult<BuiltinLibInfoAnalysis>(M);
  CallGraph &CG = AM.getResult<CallGraphAnalysis>(M);
  auto &FAM = AM.getResult<FunctionAnalysisManagerModuleProxy>(M).getManager();

  FuncSet ForbiddenFuncUsers;
  LoopUtils::fillAtomicBuiltinUsers(M, BLI->getRuntimeService(),
                                    ForbiddenFuncUsers);
  LoopUtils::fillWorkItemPipeBuiltinUsers(M, ForbiddenFuncUsers);
  LoopUtils::fillPrintfs(M, ForbiddenFuncUsers);

  // Run on all scalar kernels.
  FuncSet Kernels = getAllKernels(M);
  for (auto *F : Kernels)
    if (!F->hasOptNone() && !ForbiddenFuncUsers.contains(F))
      runOnFunction(*F, CG,
                    FAM.getResult<OptimizationRemarkEmitterAnalysis>(*F));

  // Only modifies metadata.
  return PreservedAnalyses::all();
}
