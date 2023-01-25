//===-- InfiniteLoopCreator.cpp -------------------------------------------===//
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

#include "llvm/Transforms/SYCLTransforms/InfiniteLoopCreator.h"
#include "llvm/Transforms/SYCLTransforms/Utils/CompilationUtils.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"

using namespace llvm;

#define DEBUG_TYPE "sycl-kernel-infinite-loop-creator"

PreservedAnalyses InfiniteLoopCreatorPass::run(Module &M,
                                               ModuleAnalysisManager &AM) {
  if (!runImpl(M))
    return PreservedAnalyses::all();
  return PreservedAnalyses::none();
}

static bool runOnFunction(Function *F) {
  auto It = std::find_if(F->begin(), F->end(), [](BasicBlock &BB) {
    return isa<ReturnInst>(BB.getTerminator());
  });
  if (It != F->end()) {
    BasicBlock *SingleRetBB = &(*It);
    BasicBlock *EntryBlock = &F->getEntryBlock();
    BasicBlock *InfiniteLoopEntry =
        EntryBlock->splitBasicBlock(EntryBlock->begin(), "infinite_loop_entry");
    if (SingleRetBB == EntryBlock) {
      // very simple function with one basic block, and now ret instuction is
      // moved into InfiniteLoopEntry basic block by splicBasicBlock
      SingleRetBB = InfiniteLoopEntry;
    }
    // we need to move all alloca instructions out of the infinite loop to avoid
    // overflowing of stack
    CompilationUtils::moveAlloca(InfiniteLoopEntry, EntryBlock);
    ReplaceInstWithInst(SingleRetBB->getTerminator(),
                        BranchInst::Create(InfiniteLoopEntry));
    return true;
  }
  return false;
}

bool InfiniteLoopCreatorPass::runImpl(Module &M) {
  bool HasChanges = false;
  for (auto *Kernel : SYCLKernelMetadataAPI::KernelList(M)) {
    auto Kmd = SYCLKernelMetadataAPI::KernelMetadataAPI(Kernel);
    // Since work-group autorun kernels must be launched with global_size =
    // (2^32, 2^32, 2^32), local_size = reqd_work_group_size and execution of
    // work-groups inside autorun kernels is serialized we don't need to wrap
    // kernel code with while true to allow all work-groups to be executed.
    if (Kmd.Autorun.hasValue() && Kmd.Autorun.get() &&
        Kmd.MaxGlobalWorkDim.hasValue() && 0 == Kmd.MaxGlobalWorkDim.get()) {
      HasChanges |= runOnFunction(Kernel);
    }
  }
  return HasChanges;
}
