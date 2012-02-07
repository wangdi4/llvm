/*********************************************************************************************
 * TODO: add Copyright © 2011, Intel Corporation
 *********************************************************************************************/
#include "RedundantPhiNodePass.h"
#include "BarrierUtils.h"

#include "llvm/Instructions.h"

namespace intel {

  char RedundantPhiNode::ID = 0;

  RedundantPhiNode::RedundantPhiNode() : FunctionPass(ID) {}

  bool RedundantPhiNode::runOnFunction(Function &F) {
    TInstructionVector toRemoveInstructions;
    for ( Function::iterator bi = F.begin(), be = F.end(); bi != be; ++bi ) {
      BasicBlock *pBB = dyn_cast<BasicBlock>(&*bi);
      for ( BasicBlock::iterator ii = pBB->begin(), ie = pBB->end(); ii != ie; ++ii ) {
        PHINode *pPhiNode = dyn_cast<PHINode>(&*ii);
        if ( !pPhiNode ) {
          //No more PhiNode in this BasicBlock
          break;
        }
        if ( pPhiNode->getNumIncomingValues() == 1 ) {
          pPhiNode->replaceAllUsesWith(pPhiNode->getIncomingValue(0));
          toRemoveInstructions.push_back(pPhiNode);
          continue;
        }
        //Found a PHINode
        assert( pPhiNode->getNumIncomingValues() == 2 &&
          "assume PhiCanon pass handled such PHINode!" );
        if ( pPhiNode->getIncomingValue(0) == pPhiNode->getIncomingValue(1) ) {
          //It is a Redundant PHINode add it to container for handling
          pPhiNode->replaceAllUsesWith(pPhiNode->getIncomingValue(0));
          toRemoveInstructions.push_back(pPhiNode);
        }
      }
    }
    //Remove all redundant Phi nodes
    for( TInstructionVector::iterator ii = toRemoveInstructions.begin(),
      ie = toRemoveInstructions.end(); ii != ie; ++ii ) {
        Instruction *pInst = dyn_cast<Instruction>(*ii);
        assert( pInst && "remove instruction container contains non instruction!" );
        pInst->eraseFromParent();
    }
    //The module was changed only if there were instructions to remove
    return !toRemoveInstructions.empty();
  }

  // Register this pass...
  static RegisterPass<RedundantPhiNode> DPB("B-RedundantPhiNode",
    "Barrier Pass - Handle barier instructions called from functions", false, true);

} // namespace intel

/// Support for static linking of modules for Windows
/// This pass is called by a modified Opt.exe
extern "C" {
  void* createRedundantPhiNodePass() {
    return new intel::RedundantPhiNode();
  }
}
