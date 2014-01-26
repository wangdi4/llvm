/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/
#include "RedundantPhiNodePass.h"
#include "BarrierUtils.h"
#include "OCLPassSupport.h"

#include "llvm/IR/Instructions.h"

namespace intel {

  char RedundantPhiNode::ID = 0;

  OCL_INITIALIZE_PASS(RedundantPhiNode, "B-RedundantPhiNode", "Barrier Pass - Handle barier instructions called from functions", false, true)


  RedundantPhiNode::RedundantPhiNode() : FunctionPass(ID) {}

  bool RedundantPhiNode::runOnFunction(Function &F) {
    TInstructionVector toRemoveInstructions;
    for ( Function::iterator bi = F.begin(), be = F.end(); bi != be; ++bi ) {
      BasicBlock *pBB = &*bi;
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


} // namespace intel

/// Support for static linking of modules for Windows
/// This pass is called by a modified Opt.exe
extern "C" {
  void* createRedundantPhiNodePass() {
    return new intel::RedundantPhiNode();
  }
}
