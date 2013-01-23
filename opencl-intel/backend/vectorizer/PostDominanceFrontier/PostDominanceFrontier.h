#ifndef POST_DOMINANANCE_FRONTIER_H
#define POST_DOMINANANCE_FRONTIER_H

#include "llvm/Analysis/PostDominators.h"
#include "llvm/Analysis/DominanceFrontier.h"
#include "llvm/Analysis/DominanceFrontier.h"

namespace intel {


/// PostDominanceFrontier Class - Concrete subclass of DominanceFrontier that is
/// used to compute the a post-dominance frontier.
///
struct PostDominanceFrontier : public llvm::DominanceFrontierBase {
  static char ID;
  PostDominanceFrontier();

  virtual bool runOnFunction(llvm::Function &) {
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
  const DomSetType &calculate(const llvm::PostDominatorTree &DT,
                              const llvm::DomTreeNode *Node);
};

llvm::FunctionPass* createPostDomFrontier();

} // End llvm namespace

#endif
