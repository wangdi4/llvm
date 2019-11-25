// INTEL CONFIDENTIAL
//
// Copyright 2017-2019 Intel Corporation.
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you (License). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.

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
#include <llvm/InitializePasses.h>
#include <llvm/Transforms/Utils/UnrollLoop.h>

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
  OCL_INITIALIZE_PASS_BEGIN(PipeOrdering, "pipe-ordering",
               "Add barriers to loops with pipe builtin calls", false, false)
  OCL_INITIALIZE_PASS_DEPENDENCY(LoopInfoWrapperPass)
  OCL_INITIALIZE_PASS_END(PipeOrdering, "pipe-ordering",
               "Add barriers to loops with pipe builtin calls", false, false)

  PipeOrdering::PipeOrdering() : ModulePass(ID) {
      initializePipeOrderingPass(*PassRegistry::getPassRegistry());
  }

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

  static bool requiresBarrier(const Loop* L) {
    // When #pragma unroll is used on a loop, it states that the loop can be
    // (and should be) unrolled without affecting a correctness of
    // execution. Unrolled loop may not conform to the FPGA loop ordering
    // (because it may no longer be a loop), so for this case we don't need a
    // barrier.
    if (MDNode *LoopID = L->getLoopID()) {
      if (GetUnrollMetadata(LoopID, "llvm.loop.unroll.enable")) {
        return false;
      }
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
        if (requiresBarrier(Loop)) {
          auto BB = Loop->getHeader();
          BarrierRequired.insert(BB);
          continue;
        }
      }
      // If we have't found a loop which requires a barrier, let's go up on the
      // callstack
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
      // If there is no 'max_global_work_dim' attribute or it not equals to '0'
      // this may be an NDRange kernel
      auto KMd = KernelMetadataAPI(F);
      if (!KMd.MaxGlobalWorkDim.hasValue() || KMd.MaxGlobalWorkDim.get() != 0) {
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
