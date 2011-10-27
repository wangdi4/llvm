/*********************************************************************************************
 * TODO: add Copyright © 2011, Intel Corporation
 *********************************************************************************************/

#include "RemoveDuplicationBarrierPass.h"

#include "llvm/Instructions.h"

namespace intel {

  char RemoveDuplicationBarrier::ID = 0;

  RemoveDuplicationBarrier::RemoveDuplicationBarrier() : ModulePass(ID) {}

  bool RemoveDuplicationBarrier::runOnModule(Module &M) {
    //Initialize barrier utils class with current module
    m_util.init(&M);

    //Find all synchronize instructions
    TInstructionVector& syncInstructions = m_util.getAllSynchronizeInstructuons();

    //This will hold all synchronyze instructions that will be removed
    TInstructionVector toRemoveInstructions;

    for (TInstructionVector::iterator ii = syncInstructions.begin(),
      ie = syncInstructions.end(); ii != ie; ++ii ) {

        Instruction *pInst = dyn_cast<Instruction>(*ii);
        assert( pInst && "Sync list contains something other than instruction!" );

        BasicBlock::iterator bb_prevIt(pInst);
        if ( bb_prevIt != pInst->getParent()->begin() ) {
          Instruction *pPrevInst = dyn_cast<Instruction>(--bb_prevIt);
          if( SYNC_TYPE_NONE != m_util.getSynchronizeType(pPrevInst) ) {
            //This is not first synchronize instruction in the sequence
            continue;
          }
        }

        //At this point we are sure that pInst is the first instruction in
        //the sync instruction sequence.
        BasicBlock::iterator bb_it(pInst);
        BasicBlock *pBB = pInst->getParent();

        while ( pBB->end() != (++bb_it) ) {
          Instruction *pNextInst = dyn_cast<Instruction>(bb_it);
          assert( pNextInst && "Basic Block contains something other than Instruction!" );

          //Get sync type for both instructions
          SYNC_TYPE typeInst = m_util.getSynchronizeType(pInst);
          SYNC_TYPE typeNextInst = m_util.getSynchronizeType(pNextInst);

          if ( SYNC_TYPE_NONE == typeNextInst ) {
            //End of sunc instruction sequence!
            break;
          }

          //Convert instruction to CallInst (for barrier to get its argument)
          CallInst *pNextCallInst = dyn_cast<CallInst>(bb_it);
          assert( pNextCallInst && "sync list contains something other than CallInst!" );

          if ( SYNC_TYPE_DUMMY_BARRIER == typeInst ) {
            if ( SYNC_TYPE_BARRIER == typeNextInst ) {
              ConstantInt *pBarrierValue = dyn_cast<ConstantInt>(pNextCallInst->getArgOperand(0));
              assert( pBarrierValue && "dynamic barrier mem fence value!!" );
              if ( CLK_GLOBAL_MEM_FENCE & pBarrierValue->getZExtValue() ) {
                //dummyBarrier-barrier(global) : don't remove instruction
                //Just update iterators
                pInst = pNextInst;
                continue;
              }
            }
            //Otherwise need to remove the next instruction
            toRemoveInstructions.push_back(pNextInst);
            continue;
          }
          if ( SYNC_TYPE_FIBER == typeNextInst ) {
            //any-fiber : remove the fiber instruction
            toRemoveInstructions.push_back(pNextInst);
            continue;
          }
          if ( SYNC_TYPE_FIBER == typeInst ) {
            //fiber-barrier : remove the fiber instruction
            assert( SYNC_TYPE_BARRIER == typeNextInst &&
              "pNextInst must be a barrier at this point!" );
            toRemoveInstructions.push_back(pInst);
            pInst = pNextInst;
            continue;
          }
          //barrier-barrier : remove the fiber instruction
          assert( SYNC_TYPE_BARRIER == typeInst && SYNC_TYPE_BARRIER == typeNextInst &&
            "pInst and pNextInst must be barriers at this point!" );
          ConstantInt *pBarrierValue = dyn_cast<ConstantInt>(pNextCallInst->getArgOperand(0));
          assert( pBarrierValue && "dynamic barrier mem fence value!!" );
          if ( CLK_GLOBAL_MEM_FENCE & pBarrierValue->getZExtValue() ) {
            //barrier()-barrier(global) : remove the first barrier instruction
            toRemoveInstructions.push_back(pInst);
            pInst = pNextInst;
          } else {
            assert( (CLK_LOCAL_MEM_FENCE & pBarrierValue->getZExtValue()) &&
              "barrier mem fence argument is something else than local/global" );
            //barrier()-barrier(local) : remove the second barrier instruction
            toRemoveInstructions.push_back(pNextInst);
          }
        }
    }

    //Erase all collected sync instruction for removal
    for (TInstructionVector::iterator ii = toRemoveInstructions.begin(),
      ie = toRemoveInstructions.end(); ii != ie; ++ii ) {
        Instruction *pInst = dyn_cast<Instruction>(*ii);
        assert( pInst && "Sync list contains something other than instruction!" );
        pInst->eraseFromParent();
    }
    return true;
  }

  //Register this pass...
  static RegisterPass<RemoveDuplicationBarrier> SBBB("r-d-b",
        "Remove duplication Barrier instructions", false, true);

} // namespace intel

/// Support for static linking of modules for Windows
/// This pass is called by a modified Opt.exe
extern "C" {
  void* createRemoveDuplicationBarrierPass() {
    return new intel::RemoveDuplicationBarrier();
  }
}