//==-- PipeOrdering.cpp - Insert barriers to pipe-containing loops -- C++ -==//
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
// ===--------------------------------------------------------------------===//

#include "llvm/Transforms/SYCLTransforms/PipeOrdering.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Transforms/SYCLTransforms/Utils/BarrierUtils.h"
#include "llvm/Transforms/SYCLTransforms/Utils/CompilationUtils.h"
#include "llvm/Transforms/SYCLTransforms/Utils/MetadataAPI.h"
#include "llvm/Transforms/Utils/UnrollLoop.h"

#define DEBUG_TYPE "sycl-kernel-pipe-ordering"

using namespace llvm;
using namespace SYCLKernelMetadataAPI;

static bool isCalledFromNDRange(const SmallPtrSetImpl<Function *> &Kernels,
                                Function *F,
                                DenseMap<Function *, bool> &ProcessedFuncs) {
  // Check if we've already processed the function
  auto Iter = ProcessedFuncs.find(F);
  if (Iter != ProcessedFuncs.end())
    return Iter->getSecond();

  // Check if the function is a kernel
  if (Kernels.count(F)) {
    const Module *M = F->getParent();
    // For SYCL kernel, NDRange for all kernel execution.
    if (CompilationUtils::isGeneratedFromOCLCPP(*M)) {
      ProcessedFuncs.insert({F, true});
      return true;
    } else {
      // TODO: Even for OpenCL, it seems the following logic is not accurate. We
      // need to double check.
      // If there is no 'max_global_work_dim' attribute or it not equals to '0'
      // this may be an NDRange kernel
      auto KMD = KernelMetadataAPI(F);
      if (!KMD.MaxGlobalWorkDim.hasValue() || KMD.MaxGlobalWorkDim.get() != 0) {
        ProcessedFuncs.insert({F, true});
        return true;
      }
    }
  }
  // If the function not a kernel or this is a SingleWI kernel, iterate
  // over all its calls to check if some of callers are NDRange kernels
  for (auto *User : F->users()) {
    auto *CI = dyn_cast<CallInst>(User);
    if (!CI)
      continue;
    auto *Caller = CI->getFunction();
    assert(Caller && "CI must be contained in some function!");
    if (isCalledFromNDRange(Kernels, Caller, ProcessedFuncs)) {
      ProcessedFuncs.insert({F, true});
      return true;
    }
  }
  ProcessedFuncs.insert({F, false});
  return false;
}

static bool requiresBarrier(const Loop *L) {
  // When #pragma unroll is used on a loop, it states that the loop can be
  // (and should be) unrolled without affecting a correctness of
  // execution. Unrolled loop may not conform to the FPGA loop ordering
  // (because it may no longer be a loop), so for this case we don't need a
  // barrier.
  if (MDNode *LoopID = L->getLoopID())
    if (GetUnrollMetadata(LoopID, "llvm.loop.unroll.enable"))
      return false;
  return true;
}

static void
findCallersRequiringBarrier(const SmallPtrSetImpl<Function *> &Kernels,
                            Function &F,
                            DenseMap<Function *, bool> &ProcessedFuncs,
                            SmallPtrSetImpl<BasicBlock *> &BarrierRequired,
                            function_ref<LoopInfo &(Function &)> GetLI) {
  for (auto *User : F.users()) {
    auto *CI = dyn_cast<CallInst>(User);
    if (!CI)
      continue;
    auto *Caller = CI->getFunction();
    assert(Caller && "CI must be contained in some function!");
    if (!isCalledFromNDRange(Kernels, Caller, ProcessedFuncs))
      continue;

    LoopInfo &LI = GetLI(*Caller);
    if (auto *Loop = LI.getLoopFor(CI->getParent())) {
      if (requiresBarrier(Loop)) {
        auto *BB = Loop->getHeader();
        BarrierRequired.insert(BB);
        continue;
      }
    }
    // If we have't found a loop which requires a barrier, let's go up on the
    // callstack
    findCallersRequiringBarrier(Kernels, *Caller, ProcessedFuncs,
                                BarrierRequired, GetLI);
  }
}

bool PipeOrderingPass::runImpl(Module &M,
                               function_ref<LoopInfo &(Function &)> GetLI) {
  // We should add a barrier for all functions with loops and pipe BIs when
  // these are NDRange kernels or functions that have at least one NDRange
  // kernel as caller. Otherwise, barrier insertion is unnecessary.

  // Keep information about already processed functions in the module
  // 'true' means this is a NDRange kernel or a function that has at least one
  // NDRange kernel as caller
  // 'false' means this is a SingleWI kernel or a function that doesn't have
  // any NDRange kernels as callers
  DenseMap<Function *, bool> ProcessedFuncs;

  // Keep information about basic blocks which require a barrier
  SmallPtrSet<BasicBlock *, 16> BarrierRequired;

  // Get information about kernels in the module
  auto KL = KernelList(M);
  SmallPtrSet<Function *, 4> Kernels(KL.begin(), KL.end());

  for (auto &Func : M) {
    if (!Func.isDeclaration())
      continue;

    auto FuncName = Func.getName();
    auto PKind = CompilationUtils::getPipeKind(FuncName);
    if (PKind.Op == CompilationUtils::PipeKind::OpKind::ReadWrite)
      findCallersRequiringBarrier(Kernels, Func, ProcessedFuncs,
                                  BarrierRequired, GetLI);
  }
  if (BarrierRequired.empty())
    return false;

  BarrierUtils BU;
  BU.init(&M);
  for (auto *BB : BarrierRequired) {
    BU.createBarrier(&BB->back());
  }
  return true;
}

PreservedAnalyses PipeOrderingPass::run(Module &M, ModuleAnalysisManager &MAM) {
  auto &FAM = MAM.getResult<FunctionAnalysisManagerModuleProxy>(M).getManager();
  return runImpl(M,
                 [&FAM](Function &F) -> LoopInfo & {
                   return FAM.getResult<LoopAnalysis>(F);
                 })
             ? PreservedAnalyses::none()
             : PreservedAnalyses::all();
}
