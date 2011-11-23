#include "llvm/Analysis/Verifier.h"
#include "PhiCanon.h"
#include "Logger.h"

namespace intel {

bool PhiCanon::runOnFunction(Function &F) {
  std::vector<BasicBlock*> bb_to_fix;
  bool changed = false;

  // find all multi entry blocks
  for (Function::iterator bb = F.begin(); bb != F.end(); ++bb) {
    if (std::distance(pred_begin(bb), pred_end(bb)) > 2) {
      changed = true;
      bb_to_fix.push_back(bb);
    }
  }

  // Fix all of the multi entry that we found
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

  DominatorTree     *DT  = &getAnalysis<DominatorTree>();
  PostDominatorTree *PDT = &getAnalysis<PostDominatorTree>();

  // Look for pair of BBs which comply with the following rules:
  // - none of them is dominated by the block-to-be-fixed
  // - there is a common dominator of these BBs, and it is postdominated by
  //   the block-to-be-fixed
  // - this common dominator doesn't dominate any other of BBs which precedes the
  //   block to be fixed (less blocks which are dominated by the block-to-be-fixed)
  //
  // If found, such a pair will be a candidate for single-entry to single-exit rule.
  //
  // In order to make that rule actual, we will create a new BB (with PHI) which will
  // be a common successor for both BBs of the pair, and thus will also become
  // a postdominator for the common dominator above

  pred_iterator bb_it_current = pred_begin(toFix);
  while (bb_it_current != pred_end(toFix) &&
         std::distance(pred_begin(toFix), pred_end(toFix)) > 2) {
    BasicBlock *current_bb = *bb_it_current;

    // Scan forward only
    bool pair_found = false;
    // filter-out BB dominated by toFix block (loop case)
    if (DT->dominates(toFix, current_bb)) {
      // continue iteration
      bb_it_current++;
      continue;
    }

    pred_iterator scan_bb_it = bb_it_current;
    for(scan_bb_it++; scan_bb_it != pred_end(toFix); scan_bb_it++) {
      BasicBlock *scan_bb = *scan_bb_it;

      // filter-out BB dominated by toFix block (loop case)
      if (DT->dominates(toFix, scan_bb)) continue;

      // is there a common dominator between current BB and being scanned one?
      BasicBlock *dom_bb = DT->findNearestCommonDominator(current_bb, scan_bb);
      if (!dom_bb) continue;

      // this common dominator must be post-dominated by the block to be fixed
      if (!PDT->dominates(toFix, dom_bb)) continue;

      // this common dominator doesn't dominate any other block which precedes the
      // block to be fixed
      pred_iterator bb_it = pred_begin(toFix);
      for (; bb_it != pred_end(toFix); bb_it++) {
        BasicBlock * a_bb = *bb_it;
        if (a_bb != current_bb && a_bb != scan_bb && DT->dominates(dom_bb, a_bb)) {
          // filter-out BB dominated by toFix block (loop case)
          if (!DT->dominates(toFix, a_bb)) break;
        }
      }
      if (bb_it == pred_end(toFix)) {
        // all conditions are met - create new PHI block
        BasicBlock* new_bb = makeNewPhiBB(toFix, current_bb, scan_bb);
        // also - add new block to the dominator and postdominator trees
        DT->runOnFunction(*(new_bb->getParent()));
        PDT->runOnFunction(*(new_bb->getParent()));
        pair_found = true;
        break;
      }
    }
    if (pair_found) {
      // BB tree was restructured - restart iteration
      bb_it_current = pred_begin(toFix);
    } else {
      // continue iteration
      bb_it_current++;
    }
  }

  // For the rest of BBs - make sure that 'toFix' BB will stay with 2 incoming edges
  while (std::distance(pred_begin(toFix), pred_end(toFix)) > 2) {
    // Find the first two candidates
    BasicBlock *current_bb = *(pred_begin(toFix));
    BasicBlock *scan_bb    = *(++pred_begin(toFix));
    V_ASSERT(current_bb != scan_bb && "Unable to get previous blocks ");
    // Create new PHI BB for the pair above
    // This BB will replace the pair as the toFix BB predecessor
    (void) makeNewPhiBB(toFix, current_bb, scan_bb);
  }

  V_ASSERT(!verifyFunction(*toFix->getParent()) && "I broke this module");

}

BasicBlock* PhiCanon::makeNewPhiBB(BasicBlock* toFix,
                                   BasicBlock* prev0, 
                                   BasicBlock* prev1) {
  // Below we create new BB with corresponding PHI instructions for
  // BBs of prev0 and prev1.

  // Create a new PHI block
  BasicBlock* new_bb = BasicBlock::Create(
    toFix->getParent()->getContext(),
    "phi-split-bb" , toFix->getParent(), toFix);

  // Point new PHI BB to old one
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

    // Replace old values with new PHI of new BB
    phi->removeIncomingValue(prev0);
    phi->removeIncomingValue(prev1);
    phi->addIncoming(phi_new, new_bb);

  }// for each PHI

  V_ASSERT(!verifyFunction(*toFix->getParent()) && "I broke this module");

  return new_bb;
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
