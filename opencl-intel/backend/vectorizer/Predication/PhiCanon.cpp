#include "llvm/Analysis/Verifier.h"
#include "PhiCanon.h"
#include "Logger.h"

namespace intel {

bool PhiCanon::runOnFunction(Function &F) {
  std::vector<BasicBlock*> bb_to_fix;
  bool changed = false;

  /// find all multi entry blocks
  for (Function::iterator bb = F.begin(); bb != F.end(); ++bb) {
    if (std::distance(pred_begin(bb), pred_end(bb)) > 2) {
      changed = true;
      bb_to_fix.push_back(bb);
    }
  }

  /// Fix all of the multi entry that we found
  for(std::vector<BasicBlock*>::iterator
      it = bb_to_fix.begin() ; it != bb_to_fix.end() ; ++it) {
    fixBlock(*it);
  }

  // verify this module
  V_ASSERT(!verifyFunction(F) && "I broke this module");

  // Check that we actually were able to remove all multi-entry phi
  for (Function::iterator bb = F.begin(); bb != F.end(); ++bb) {
    V_ASSERT(std::distance(pred_begin(bb), pred_end(bb)) < 3 &&
           "Phi canon failed");
  }

  return changed;
}

void PhiCanon::fixBlock(BasicBlock* toFix) {

  // While we are not done with this BB
  while (std::distance(pred_begin(toFix), pred_end(toFix)) > 2) {

    // Find the first two candidates
    BasicBlock* prev0 = *(pred_begin(toFix));
    BasicBlock* prev1 = *(++pred_begin(toFix));
    V_ASSERT(prev0 != prev1 && "Unable to get previous blocks ");

    // Create a new block
    BasicBlock* new_bb = BasicBlock::Create(
      toFix->getParent()->getContext(),
      "phi-split-bb" , toFix->getParent(), toFix);

    // Point new BB to old one
    BranchInst::Create(toFix, new_bb);
    V_ASSERT(std::find(pred_begin(toFix), pred_end(toFix), new_bb) !=
           pred_end(toFix));

    // Fix jump target of old predecessors to new BB
    fixBasicBlockSucessor(prev0, toFix, new_bb);
    fixBasicBlockSucessor(prev1, toFix, new_bb);
    V_ASSERT(std::find(pred_begin(new_bb), pred_end(new_bb), prev0) !=
           pred_end(new_bb));
    V_ASSERT(std::find(pred_begin(new_bb), pred_end(new_bb), prev1) !=
           pred_end(new_bb));

    // Fix all PHIs in old BB
    for (BasicBlock::iterator it = toFix->begin(),
         e = toFix->end(); it != e ; ++it) {

      // As long as we find PHIs
      PHINode* phi = dyn_cast<PHINode>(it);
      if (!phi) break;

      // Find values which we want to remove
      Value* v0 = phi->getIncomingValueForBlock(prev0);
      Value* v1 = phi->getIncomingValueForBlock(prev1);

      // Move them to a new PHI in the new block
      PHINode* phi_new = PHINode::Create(
        phi->getType(), "new_phi", new_bb->begin());
      phi_new->addIncoming(v0, prev0);
      phi_new->addIncoming(v1, prev1);

      // Replace old values with new PHI
      phi->removeIncomingValue(prev0);
      phi->removeIncomingValue(prev1);
      phi->addIncoming(phi_new, new_bb);
    }// for each PHI

  V_ASSERT(!verifyFunction(*toFix->getParent()) && "I broke this module");
  }// while more than two

}


void PhiCanon::fixBasicBlockSucessor(BasicBlock* to_fix,
                                     BasicBlock* old_target,
                                     BasicBlock* new_target) {

  TerminatorInst* term = to_fix->getTerminator();
  V_ASSERT(term && "Basic Block must have a terminator");

  /// Fix/replace branch inst
  if (BranchInst* br = dyn_cast<BranchInst>(term)) {
    for (unsigned int i=0; i<br->getNumSuccessors() ; ++i) {
      if (old_target == br->getSuccessor(i)) {
        br->setSuccessor(i, new_target);
      }
    }
  } /// Fix/replace switch inst
  else if (SwitchInst* sw = dyn_cast<SwitchInst>(term)) {
    for (unsigned int i=0; i<sw->getNumSuccessors() ; ++i) {
      if (old_target == sw->getSuccessor(i)) {
        sw->setSuccessor(i, new_target);
      }
    }
  } else if (IndirectBrInst* sw = dyn_cast<IndirectBrInst>(term)) {
    for (unsigned int i=0; i<sw->getNumSuccessors() ; ++i) {
      if (old_target == sw->getSuccessor(i)) {
        sw->setSuccessor(i, new_target);
      }
    }
  } else {
    V_ASSERT(false && "Unknown terminator type");
  }
}




} // namespace

/// Support for static linking of modules for Windows
/// This pass is called by a modified Opt.exe
extern "C" {
  FunctionPass* createPhiCanon() {
    return new intel::PhiCanon();
  }
}
char intel::PhiCanon::ID = 0;
static RegisterPass<intel::PhiCanon>
CLIPhiCannon("phicanon", "Phi Canonicalizer path (Two-based Phi)");

