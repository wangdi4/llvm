/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/

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