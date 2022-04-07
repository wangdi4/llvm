// Copyright 2012-2021 Intel Corporation.
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

#include "RemoveDuplicationBarrierPass.h"
#include "OCLPassSupport.h"

#include "llvm/IR/Instructions.h"
#include "llvm/Support/CommandLine.h"


namespace intel {

  char RemoveDuplicationBarrier::ID = 0;

  OCL_INITIALIZE_PASS(RemoveDuplicationBarrier, "B-RemoveDuplication",
                      "Barrier Pass - Remove duplication Barrier instructions",
                      false, false)
  // TODO: remove this option and use the same option in BarrierPass after the
  // pass is ported to DPCPPKernelTransforms.
  cl::opt<bool> OptIsNativeDebug("is-native-debug", cl::init(false), cl::Hidden,
                                 cl::desc("Use native debug"));

  RemoveDuplicationBarrier::RemoveDuplicationBarrier(bool IsNativeDebug)
      : ModulePass(ID), m_IsNativeDebug(IsNativeDebug || OptIsNativeDebug) {}

  // FIXME:This pass is not well constructed, we should reuse SGBarrierSimplify
  // to remove redundant barriers.
  bool RemoveDuplicationBarrier::runOnModule(Module &M) {
    //Initialize barrier utils class with current module
    m_util.init(&M);

    //Find all synchronize instructions
    InstVector syncInstructions = m_util.getAllSynchronizeInstructions();

    //This will hold all synchronyze instructions that will be removed
    InstVector toRemoveInstructions;

    for (InstVector::iterator ii = syncInstructions.begin(),
                              ie = syncInstructions.end();
         ii != ie; ++ii) {

      Instruction *pInst = dyn_cast<Instruction>(*ii);
      assert(pInst && "Sync list contains something other than instruction!");

      BasicBlock::iterator bb_prevIt(pInst);
      if (bb_prevIt != pInst->getParent()->begin()) {
        Instruction *pPrevInst = &*(--bb_prevIt);
        if (SyncType::None != m_util.getSyncType(pPrevInst)) {
          // This is not first synchronize instruction in the sequence
          continue;
        }
      }

      // At this point we are sure that pInst is the first instruction in
      // the sync instruction sequence.
      BasicBlock::iterator bb_it(pInst);
      BasicBlock *pBB = pInst->getParent();

      while (pBB->end() != (++bb_it)) {
        Instruction *pNextInst = dyn_cast<Instruction>(bb_it);
        assert(pNextInst &&
               "Basic Block contains something other than Instruction!");

        // Get sync type for both instructions
        SyncType typeInst = m_util.getSyncType(pInst);
        SyncType typeNextInst = m_util.getSyncType(pNextInst);

        if (SyncType::None == typeNextInst) {
          // End of sunc instruction sequence!
          break;
        }

        // Convert instruction to CallInst (for barrier to get its argument)
        CallInst *pNextCallInst = dyn_cast<CallInst>(bb_it);
        assert(pNextCallInst &&
               "sync list contains something other than CallInst!");

        if (SyncType::DummyBarrier == typeInst) {
          if (SyncType::Barrier == typeNextInst) {
            ConstantInt *pBarrierValue =
                dyn_cast<ConstantInt>(pNextCallInst->getArgOperand(0));
            assert(pBarrierValue && "dynamic barrier mem fence value!!");
            if ((CLK_GLOBAL_MEM_FENCE | CLK_CHANNEL_MEM_FENCE) &
                pBarrierValue->getZExtValue()) {
              // dummyBarrier-barrier(global) : don't remove instruction
              // Just update iterators
              pInst = pNextInst;
              continue;
            }
          }
          // Otherwise need to remove the next instruction
          // FIXME: Don't remove dummyBarrier-any, Barrier pass might fail
          // toRemoveInstructions.push_back(pNextInst);
          continue;
        }
        assert(SyncType::Barrier == typeInst &&
               SyncType::Barrier == typeNextInst &&
               "pInst and pNextInst must be barriers at this point!");
        ConstantInt *pBarrierValue =
            dyn_cast<ConstantInt>(pNextCallInst->getArgOperand(0));
        assert(pBarrierValue && "dynamic barrier mem fence value!!");
        // FIXME: we should also check memory scope.
        // if scope <= memory_scope_work_group, we can remove the barrier
        // anyway.
        if ((CLK_GLOBAL_MEM_FENCE | CLK_CHANNEL_MEM_FENCE) &
            pBarrierValue->getZExtValue()) {
          if (!(m_IsNativeDebug && pInst->getDebugLoc())) {
            // barrier()-barrier(global) : remove the first barrier
            // instruction
            toRemoveInstructions.push_back(pInst);
          }
          // Update iterator.
          pInst = pNextInst;
        } else {
          assert(
              (CLK_LOCAL_MEM_FENCE & pBarrierValue->getZExtValue()) &&
              "barrier mem fence argument is something else than local/global");
          if (!(m_IsNativeDebug && pNextInst->getDebugLoc())) {
            // barrier()-barrier(local) : remove the second barrier
            // instruction
            toRemoveInstructions.push_back(pNextInst);
          }
        }
      }
    }

    //Erase all collected sync instruction for removal
    for (InstVector::iterator ii = toRemoveInstructions.begin(),
                              ie = toRemoveInstructions.end();
         ii != ie; ++ii) {
      Instruction *pInst = dyn_cast<Instruction>(*ii);
      assert(pInst && "Sync list contains something other than instruction!");
      pInst->eraseFromParent();
    }
    return true;
  }

} // namespace intel

/// Support for static linking of modules for Windows
/// This pass is called by a modified Opt.exe
extern "C" {
void *createRemoveDuplicationBarrierPass(bool IsNativeDebug) {
  return new intel::RemoveDuplicationBarrier(IsNativeDebug);
}
}
