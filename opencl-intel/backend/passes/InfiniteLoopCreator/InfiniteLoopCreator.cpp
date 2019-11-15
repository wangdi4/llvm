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

#include "InfiniteLoopCreator.h"

#include "CompilationUtils.h"
#include "InitializePasses.h"
#include "MetadataAPI.h"
#include "OCLPassSupport.h"

#include "llvm/Analysis/LoopInfo.h"
#include "llvm/IR/Instructions.h"
#include "llvm/InitializePasses.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/Transforms/Utils/UnifyFunctionExitNodes.h"

using namespace llvm;
using namespace Intel::OpenCL::DeviceBackend;
using namespace Intel::MetadataAPI;

namespace intel {

char InfiniteLoopCreator::ID = 0;
OCL_INITIALIZE_PASS_BEGIN(
    InfiniteLoopCreator, "infinite-loop-creator",
    "Wrap body of autorun kernels by while (true) loop if necessary",
    /* Only looks at CFG */false, /* Analysis Pass */false)
OCL_INITIALIZE_PASS_DEPENDENCY(UnifyFunctionExitNodes)
OCL_INITIALIZE_PASS_END(
    InfiniteLoopCreator, "infinite-loop-creator",
    "Wrap body of autorun kernels by while (true) loop if necessary",
    /* Only looks at CFG */false, /* Analysis Pass */false)

InfiniteLoopCreator::InfiniteLoopCreator()
    : ModulePass(ID) {}

bool InfiniteLoopCreator::runOnModule(Module &M) {
  bool hasChanges = false;

  for (auto *Kernel: KernelList(M)) {
    auto kmd = KernelMetadataAPI(Kernel);
    // Since work-group autorun kernels must be launched with global_size =
    // (2^32, 2^32, 2^32), local_size = reqd_work_group_size and execution of
    // work-groups inside autorun kernels is serialized we don't need to wrap
    // kernel code with while true to allow all work-groups to be executed.
    if (kmd.Autorun.hasValue() && kmd.Autorun.get() &&
        kmd.MaxGlobalWorkDim.hasValue() && 0 == kmd.MaxGlobalWorkDim.get()) {
      hasChanges |= runOnFunction(Kernel);
    }
  }

  return hasChanges;
}

bool InfiniteLoopCreator::runOnFunction(Function *F) {
  if (BasicBlock *SingleRetBB =
          getAnalysis<UnifyFunctionExitNodes>(*F).getReturnBlock()) {
    BasicBlock *EntryBlock = &F->getEntryBlock();
    BasicBlock *InfiniteLoopEntry = EntryBlock->splitBasicBlock(
        EntryBlock->begin(), "infinite_loop_entry");
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
  }

  return true;
}

void InfiniteLoopCreator::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.addRequired<UnifyFunctionExitNodes>();
}

} // namespace intel

extern "C" {
  ModulePass *createInfiniteLoopCreatorPass() {
    return new intel::InfiniteLoopCreator();
  }
}
