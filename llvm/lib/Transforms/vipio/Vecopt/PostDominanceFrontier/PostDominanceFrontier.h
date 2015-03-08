//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef POST_DOMINANANCE_FRONTIER_H
#define POST_DOMINANANCE_FRONTIER_H

#include "llvm/Analysis/PostDominators.h"
#include "llvm/Analysis/DominanceFrontier.h"

namespace intel {

/// PostDominanceFrontier Class - Concrete subclass of DominanceFrontier that is
/// used to compute the a post-dominance frontier.
///
struct PostDominanceFrontier : public llvm::FunctionPass,
                               public llvm::DominanceFrontierBase<llvm::BasicBlock> {
  static char ID;
  //  llvm::DominanceFrontierBase<llvm::BasicBlock> m_base;
  PostDominanceFrontier();

  virtual bool runOnFunction(llvm::Function &) {
    //    m_base.releaseMemory();
    Frontiers.clear();
    llvm::PostDominatorTree &DT = getAnalysis<llvm::PostDominatorTree>();
    Roots = DT.getRoots();
    if (const llvm::DomTreeNode *Root = DT.getRootNode())
      calculate(DT, Root);
    //TODO: Remove
    //print(errs());
    return false;
  }

  virtual void getAnalysisUsage(llvm::AnalysisUsage &AU) const {
    AU.setPreservesAll();
    AU.addRequired<llvm::PostDominatorTree>();
  }

private:
  const DomSetType& calculate(const llvm::PostDominatorTree &DT,
			      const llvm::DomTreeNode *Node);

public:
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  void dump() const;
#endif

};

llvm::FunctionPass* createPostDomFrontier();

} // End llvm namespace

#endif
