//==--- RemoveDuplicatedBarrier.cpp - RemoveDuplicatedBarrier pass - C++ -*-==//
//
// Copyright 2012 Intel Corporation.
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you (License). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implIEd warrantIEs, other than those that are expressly stated in the
// License.
//
// ===--------------------------------------------------------------------=== //

#include "llvm/Transforms/SYCLTransforms/RemoveDuplicatedBarrier.h"
#include "llvm/IR/Instructions.h"

#define DEBUG_TYPE "sycl-kernel-remove-duplicated-barrier"

using namespace llvm;

PreservedAnalyses RemoveDuplicatedBarrierPass::run(Module &M,
                                                   ModuleAnalysisManager &) {
  if (!runImpl(M))
    return PreservedAnalyses::all();
  return PreservedAnalyses::none();
}

// FIXME:This pass is not well constructed, we should reuse SGBarrierSimplify
// to remove redundant barriers.
bool RemoveDuplicatedBarrierPass::runImpl(Module &M) {
  // Initialize barrier utils class with current module
  BarrierUtil.init(&M);

  // Find all synchronize instructions
  CompilationUtils::InstVec syncInstructions =
      BarrierUtil.getAllSynchronizeInstructions();

  // This will hold all synchronize instructions that will be removed
  CompilationUtils::InstVec InstrsRemove;

  for (auto &Inst : syncInstructions) {
    BasicBlock::iterator BIPrev(Inst);
    if (BIPrev != Inst->getParent()->begin()) {
      Instruction *PrevInst = &*(--BIPrev);
      // This is not first synchronize instruction in the sequence
      if (SyncType::None != BarrierUtil.getSyncType(PrevInst))
        continue;
    }

    // At this point we are sure that Inst is the first instruction in
    // the sync instruction sequence.
    BasicBlock::iterator BI(Inst);
    BasicBlock *BB = Inst->getParent();

    while (BB->end() != (++BI)) {
      Instruction *NextInst = dyn_cast<Instruction>(BI);
      assert(NextInst &&
             "Basic Block contains something other than Instruction!");

      // Get sync type for both instructions
      SyncType TypeInst = BarrierUtil.getSyncType(Inst);
      SyncType NextTypeInst = BarrierUtil.getSyncType(NextInst);

      // End of sync instruction sequence!
      if (SyncType::None == NextTypeInst)
        break;

      // Convert instruction to CallInst (for barrier to get its argument)
      CallInst *NextCallInst = dyn_cast<CallInst>(BI);
      assert(NextCallInst &&
             "sync list contains something other than CallInst!");
      if (SyncType::DummyBarrier == TypeInst) {
        if (SyncType::Barrier == NextTypeInst) {
          ConstantInt *BarrierValue =
              dyn_cast<ConstantInt>(NextCallInst->getArgOperand(0));
          assert(BarrierValue && "dynamic barrier mem fence value!!");
          if ((CLK_GLOBAL_MEM_FENCE | CLK_CHANNEL_MEM_FENCE) &
              BarrierValue->getZExtValue()) {
            // dummyBarrier-barrier(global) : don't remove instruction
            // Just update iterators
            Inst = NextInst;
            continue;
          }
        }
        // Otherwise need to remove the next instruction
        // FIXME: Don't remove dummyBarrier-any, Barrier pass might fail
        // InstrsRemove.push_back(NextInst);
        continue;
      }

      assert(SyncType::Barrier == TypeInst &&
             SyncType::Barrier == NextTypeInst &&
             "Inst and NextInst must be barriers at this point!");
      ConstantInt *BarrierValue =
          dyn_cast<ConstantInt>(NextCallInst->getArgOperand(0));
      assert(BarrierValue && "dynamic barrier mem fence value!!");
      // FIXME: we should also check memory scope.
      // if scope <= memory_scope_work_group, we can remove the barrier
      // anyway.
      if ((CLK_GLOBAL_MEM_FENCE | CLK_CHANNEL_MEM_FENCE) &
          BarrierValue->getZExtValue()) {
        // barrier()-barrier(global) : remove the first barrier
        // instruction
        InstrsRemove.push_back(Inst);
        // Update iterator.
        Inst = NextInst;
      } else {
        assert(
            (CLK_LOCAL_MEM_FENCE & BarrierValue->getZExtValue()) &&
            "barrier mem fence argument is something else than local/global");
        // barrier()-barrier(local) : remove the second barrier
        // instruction
        InstrsRemove.push_back(NextInst);
      }
    }
  }

  // Erase all of collected sync instructions
  for (auto &Inst : InstrsRemove)
    Inst->eraseFromParent();

  return true;
}
