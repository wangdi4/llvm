//==--- PipeOrdering.cpp - Implementation of PipeOrdering pass -*- C++ -*---==//
//
// Copyright (C) 2017 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //

#include "PipeOrdering.h"

#include "BarrierUtils.h"
#include "CompilationUtils.h"
#include "InitializePasses.h"
#include "MetadataAPI.h"
#include "OCLPassSupport.h"
#include <llvm/ADT/DenseMap.h>
#include <llvm/ADT/SmallPtrSet.h>
#include <llvm/IR/InstIterator.h>
#include <llvm/IR/Instructions.h>

#include <algorithm>

using namespace llvm;
using namespace Intel::OpenCL::DeviceBackend;
using namespace Intel::MetadataAPI;

extern "C" {
  /// @brief Creates new PipeOrdering pass
  void *createPipeOrderingPass() { return new intel::PipeOrdering(); }
}
namespace intel {

  char PipeOrdering::ID = 0;

  /// Register pass to for opt
  OCL_INITIALIZE_PASS(PipeOrdering, "pipe-ordering",
               "Add barriers to loops with pipe builtin calls", false, false)

  bool PipeOrdering::runOnModule(Module &M) {
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
    m_kernels = KernelList(M).getList();

    for (auto &Func : M) {
      if (!Func.isDeclaration())
        continue;

      auto FuncName = Func.getName();
      auto PKind = CompilationUtils::getPipeKind(FuncName);
      if (PKind.Op == PipeKind::READWRITE)
        findCallersRequiringBarrier(&Func, ProcessedFuncs, BarrierRequired);
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

  void PipeOrdering::findCallersRequiringBarrier(
      Function *F, DenseMap<Function *, bool> &ProcessedFuncs,
      SmallPtrSetImpl<BasicBlock *> &BarrierRequired) {

    for (auto *User : F->users()) {
      auto *Call = dyn_cast<CallInst>(User);
      if (!Call)
        continue;
      auto *Caller = Call->getFunction();
      if (!isCalledFromNDRange(Caller, ProcessedFuncs))
        continue;

      LoopInfo &LI = getAnalysis<LoopInfoWrapperPass>(*Caller).getLoopInfo();
      if (auto Loop = LI.getLoopFor(Call->getParent())) {
        auto BB = Loop->getHeader();
        BarrierRequired.insert(BB);
        continue;
      }
      // If we have't found a loop, let's go up on the callstack
      findCallersRequiringBarrier(Caller, ProcessedFuncs, BarrierRequired);
    }
  }

  bool PipeOrdering::isCalledFromNDRange(
      Function *F, DenseMap<Function *, bool> &ProcessedFuncs) {

    // Check if we've already processed the function
    if (ProcessedFuncs.find(F) != ProcessedFuncs.end())
      return ProcessedFuncs[F];

    // Check if the function is a kernel
    if (std::find(m_kernels.begin(), m_kernels.end(), F) != m_kernels.end()) {
      // If there is no 'task' attribute or it set to 'false'
      // this may be an NDRange kernel
      auto KMd = KernelMetadataAPI(F);
      if (!KMd.Task.hasValue() || !KMd.Task.get()) {
        ProcessedFuncs.insert({F, true});
        return true;
      }
    }
    // If the function not a kernel or this is a SingleWI kernel, iterate
    // over all its calls to check if some of callers are NDRange kernels
    for (auto *User : F->users()) {
      auto *Call = dyn_cast<CallInst>(User);
      if (!Call)
        continue;
      auto *Caller = cast<CallInst>(User)->getFunction();
      if (isCalledFromNDRange(Caller, ProcessedFuncs)) {
        ProcessedFuncs.insert({F, true});
        return true;
      }
    }
    ProcessedFuncs.insert({F, false});
    return false;
  }
} // namespace intel
