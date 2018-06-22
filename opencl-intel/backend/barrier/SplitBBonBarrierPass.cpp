// INTEL CONFIDENTIAL
//
// Copyright 2012-2018 Intel Corporation.
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

#include "SplitBBonBarrierPass.h"
#include "OCLPassSupport.h"

#include "llvm/IR/Instructions.h"

namespace intel {

  char SplitBBonBarrier::ID = 0;

  OCL_INITIALIZE_PASS(SplitBBonBarrier, "B-SplitOnBarrier", "Barrier Pass - Split Basic Block on Barrier", false, true)

  SplitBBonBarrier::SplitBBonBarrier() : ModulePass(ID) {}

  bool SplitBBonBarrier::runOnModule(Module &M) {
    //Initialize barrier utils class with current module
    m_util.init(&M);

    //Find all synchronize instructions
    TInstructionVector& syncInstructions = m_util.getAllSynchronizeInstructuons();

    for (TInstructionVector::iterator ii = syncInstructions.begin(),
      ie = syncInstructions.end(); ii != ie; ++ii ) {

        Instruction *pInst = dyn_cast<Instruction>(*ii);
        assert( pInst && "sync list contains something other than instruction!" );

        BasicBlock::iterator bb_it(pInst);
        BasicBlock *pOriginBB = pInst->getParent();
        if ( pOriginBB->begin() == bb_it ) {
          //Barrier instruction already at begin of basic block (do nothing)
          continue;
        }
        //Split basic block at barrier intsruction to make
        //it first instruction in the new basic block
        pOriginBB->splitBasicBlock(bb_it, "Barrier BB");
    }

    return true;
  }


} // namespace intel

/// Support for static linking of modules for Windows
/// This pass is called by a modified Opt.exe
extern "C" {
  void* createSplitBBonBarrierPass() {
    return new intel::SplitBBonBarrier();
  }
}
