/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/

#include "OCLBranchProbability.h"
#include "OCLPassSupport.h"
#include "InitializePasses.h"

extern "C" {
  /// @brief Creates new OCLBranchProbability function pass
  /// @returns new OCLBranchProbability function pass
  void* createOCLBranchProbabilityPass() {
    return new intel::OCLBranchProbability();
  }
}

namespace intel {

  char OCLBranchProbability::ID = 0;

  OCL_INITIALIZE_PASS_BEGIN(OCLBranchProbability, "ocl-branch-probability", "augment the general branch probability to be ocl directive aware", false, false)
  OCL_INITIALIZE_PASS_DEPENDENCY(WIAnalysis)
  OCL_INITIALIZE_PASS_DEPENDENCY(BranchProbabilityInfoWrapperPass)
  OCL_INITIALIZE_PASS_END(OCLBranchProbability, "ocl-branch-probability", "augment the general branch probability to be ocl directive aware", false, false)

  bool OCLBranchProbability::runOnFunction(Function &F) {
    m_BPI = &getAnalysis<BranchProbabilityInfoWrapperPass>().getBPI();
    assert (m_BPI && "Unable to get BranchProbabilityInfo");

    m_WIA = &getAnalysis<WIAnalysis>();
    assert (m_WIA && "Unable to get WIAnalysis");

    // We go over the function and look for branches that can be classified as high predicted in compile time
    // This function utilizes WIAnalysis and gives high priority for one of the successors in the following cases:
    // CONSECUTIVE == UNIFORM - the non-taken successor gets high priority
    // CONSECUTIVE != UNIFORM - the taken successor gets high priority
    // CONSECUTIVE ? UNIFORM and one of the successor's terminator is a return instruction
    //                        - the other successor gets high priority
    for (Function::iterator bb = F.begin(), bbe = F.end(); bb != bbe ; ++bb) {
      BasicBlock* I = &*bb;
      TerminatorInst* term = bb->getTerminator();
      BranchInst* br = dyn_cast<BranchInst>(term);

      if(!br || ! br->isConditional())
        continue;

      ICmpInst *cmp = dyn_cast<ICmpInst>(br->getCondition());
      assert (br->getNumSuccessors() == 2 && "A conditional branch should have two successors");

      // An array of weights for the outgoing edge of `br'
      // We assign an initial value for these outgoing edges and never assign this value to the
      // branch probability data structure. This is done in order to keep the original branch
      // probability pass values in case none of our heuristics decided to update the branch probability
      // of an edge. The probability for an outgoing edge `e' from a node `n' is calculated by
      // weight of (`e') / (sum of weights for all outgoing edges of `n')
      unsigned weights[2] = {NormalWeight, NormalWeight};
      if(cmp) {
        WIAnalysis::WIDependancy op0Dep = m_WIA->whichDepend(cmp->getOperand(0));
        WIAnalysis::WIDependancy op1Dep = m_WIA->whichDepend(cmp->getOperand(1));

        if ((op0Dep == WIAnalysis::CONSECUTIVE && op1Dep == WIAnalysis::UNIFORM) ||
            (op1Dep == WIAnalysis::CONSECUTIVE && op0Dep == WIAnalysis::UNIFORM)) {
          if (cmp->getPredicate() == CmpInst::ICMP_EQ) {
            weights[0] = NonTakenWeight;
            weights[1] = TakenWeight;
          }
          else if (cmp->getPredicate() == CmpInst::ICMP_NE) {
            weights[0] = TakenWeight;
            weights[1] = NonTakenWeight;
          }
          else if (isa<Constant>(cmp->getOperand(0)) || isa<Constant>(cmp->getOperand(1))) {
            if (isa<ReturnInst>(br->getSuccessor(0)->getTerminator())) {
              weights[0] = NonTakenWeight;
              weights[1] = TakenWeight;
            }
            if (isa<ReturnInst>(br->getSuccessor(1)->getTerminator())) {
              weights[0] = TakenWeight;
              weights[1] = NonTakenWeight;
            }
          }
        }
      }

      // Update the weights if needed
      for (unsigned i=0; i < br->getNumSuccessors(); ++i) {
        if (weights[i] != NormalWeight)
          m_BPI->setEdgeProbability(I, i,
            BranchProbability::getBranchProbability(weights[i], TakenWeight + NonTakenWeight));
      }
    }
    return false;
  }

  /// The following are wrapper functions for BranchProbabilityInfo

  BranchProbability OCLBranchProbability::getEdgeProbability(const BasicBlock *Src, unsigned IndexInSuccessors) const {
    return m_BPI->getEdgeProbability(Src, IndexInSuccessors);
  }

  BranchProbability OCLBranchProbability::getEdgeProbability(const BasicBlock *Src, const BasicBlock *Dst) const {
    return m_BPI->getEdgeProbability(Src, Dst);
  }

  bool OCLBranchProbability::isEdgeHot(const BasicBlock *Src, const BasicBlock *Dst) const {
    return m_BPI->isEdgeHot(Src, Dst);
  }

  const BasicBlock * OCLBranchProbability::getHotSucc(const BasicBlock *BB) const {
    return m_BPI->getHotSucc(BB);
  }

} // namespace intel
