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
