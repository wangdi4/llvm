/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/

#include "PhiCanon.h"
#include "Logger.h"
#include "OCLPassSupport.h"
#include "llvm/IR/Verifier.h"

namespace intel {

char intel::PhiCanon::ID = 0;

OCL_INITIALIZE_PASS_BEGIN(PhiCanon, "phicanon", "Phi Canonicalizer path (Two-based Phi)", false, false)
OCL_INITIALIZE_PASS_DEPENDENCY(DominatorTreeWrapperPass)
OCL_INITIALIZE_PASS_DEPENDENCY(PostDominatorTreeWrapperPass)
OCL_INITIALIZE_PASS_END(PhiCanon, "phicanon", "Phi Canonicalizer path (Two-based Phi)", false, false)

PhiCanon::PhiCanon() : FunctionPass(ID) {
    initializePhiCanonPass(*PassRegistry::getPassRegistry());
}

bool PhiCanon::runOnFunction(Function &F) {
  std::vector<BasicBlock*> bb_to_fix;
  bool changed = false;

  // find all multi entry blocks
  for (Function::iterator bbit = F.begin(); bbit != F.end(); ++bbit) {
    BasicBlock* bb = &*bbit;
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
  for (Function::iterator bbit = F.begin(); bbit != F.end(); ++bbit) {
    V_ASSERT(std::distance(pred_begin(&*bbit), pred_end(&*bbit)) < 3 &&
           "Phi canon failed");
  }

  return changed;
}

void PhiCanon::fixBlock(BasicBlock* toFix) {

  DominatorTreeWrapperPass & DTPass = getAnalysis<DominatorTreeWrapperPass>();
  DominatorTree *DT = &DTPass.getDomTree();
  PostDominatorTreeWrapperPass & PDTPass = getAnalysis<PostDominatorTreeWrapperPass>();
  PostDominatorTree *PDT = &PDTPass.getPostDomTree();

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
        DTPass.runOnFunction(*(new_bb->getParent()));
        DT = &DTPass.getDomTree();
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

  // Ok, that failed. Relax our criteria, and try to find pairs of BBs
  // that are either both dominated, or both not dominated by the block
  // to be fixed, merging them first.
  // The idea is that we don't want the fallback merge below to merge an edge going
  // entering a loop header from the outside with a backedge. Such merges can cause
  // irreducible control flow to be formed.
  // For example, consider a CFG with the basic blocks: A, B, C, D,
  // and the edges are: A => C, B => C, C => C, C => D
  // If the B => C edge is merge with the C => C self-loop, the result CFG
  // (after adding the new block N) is:
  // A => C, B => N, C => N, N => C, C => D
  // The loop now has blocks (C and N), and both of them have external incoming
  // edges (A => C, B => N) which is pretty awful. In particular, the control flow
  // was reducible in the original graph, but is irreducible now, which kills the
  // predicator.
  bb_it_current = pred_begin(toFix);
  while (bb_it_current != pred_end(toFix) &&
         std::distance(pred_begin(toFix), pred_end(toFix)) > 2) {
    BasicBlock *current_bb = *bb_it_current;

    bool pair_found = false;
    bool dominates = DT->dominates(toFix, current_bb);

    pred_iterator scan_bb_it = bb_it_current;
    for(scan_bb_it++; scan_bb_it != pred_end(toFix); scan_bb_it++) {
      BasicBlock *scan_bb = *scan_bb_it;

      if (dominates != DT->dominates(toFix, scan_bb)) continue;

      BasicBlock* new_bb = makeNewPhiBB(toFix, current_bb, scan_bb);
      // also - add new block to the dominator and postdominator trees
      DTPass.runOnFunction(*(new_bb->getParent()));
      DT = &DTPass.getDomTree();
      PDTPass.runOnFunction(*(new_bb->getParent()));
      PDT = &PDTPass.getPostDomTree();
      pair_found = true;
      break;
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
  // In theory, this should no longer ever be reached, since every time there are 3 edges,
  // at least one pair will either both dominate or both not-dominated the block to be
  // fixed. Still, leave this as a sanity check.
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

    if (v0 != v1) {
      // Move them to a new PHI in the new block
      PHINode* phi_new = PHINode::Create(
        phi->getType(), 2, "new_phi", &*new_bb->begin());
      phi_new->addIncoming(v0, prev0);
      phi_new->addIncoming(v1, prev1);

      // Replace old values with new PHI of new BB
      phi->removeIncomingValue(prev0);
      phi->removeIncomingValue(prev1);
      phi->addIncoming(phi_new, new_bb);
    }
    else {
      phi->removeIncomingValue(prev0);
      phi->removeIncomingValue(prev1);
      phi->addIncoming(v0, new_bb);
    }

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
        // Must stop after finding first match, as in switch there might be
        // more than one match that we want to keep for next phiCanon iter
        break;
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



