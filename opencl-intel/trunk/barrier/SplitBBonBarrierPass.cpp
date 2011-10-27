/*********************************************************************************************
 * TODO: add Copyright © 2011, Intel Corporation
 *********************************************************************************************/

#include "SplitBBonBarrierPass.h"

#include "llvm/Instructions.h"

namespace intel {

  char SplitBBonBarrier::ID = 0;

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

  //Register this pass...
  static RegisterPass<SplitBBonBarrier> SBBB("s-bb-b",
        "Split Basic Block on Barrier", false, true);

} // namespace intel

/// Support for static linking of modules for Windows
/// This pass is called by a modified Opt.exe
extern "C" {
  void* createSplitBBonBarrierPass() {
    return new intel::SplitBBonBarrier();
  }
}