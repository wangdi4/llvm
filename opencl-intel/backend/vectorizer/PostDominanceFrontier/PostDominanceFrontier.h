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

//===-------------------------------------
/// DominanceFrontier Class - Concrete subclass of DominanceFrontierBase that is
/// used to compute a backward dominator frontiers.
///
template <class BlockT>
class PostDominanceFrontierBase : public llvm::DominanceFrontierBase<BlockT> {
private:
  typedef llvm::GraphTraits<BlockT *> BlockTraits;

public:
  typedef llvm::DominatorTreeBase<BlockT> DomTreeT;
  typedef llvm::DomTreeNodeBase<BlockT> DomTreeNodeT;
  typedef typename llvm::DominanceFrontierBase<BlockT>::DomSetType DomSetType;

  PostDominanceFrontierBase() : llvm::DominanceFrontierBase<BlockT>(true) {}

  void analyze(DomTreeT &DT) {
    this->Roots = DT.getRoots();
    assert(this->Roots.size() == 1 &&
           "Only one entry block for post domfronts!");
    calculate(DT, DT.getRootNode());
  }

  const llvm::DominanceFrontier::DomSetType &
      calculate(const DomTreeT &DT, const llvm::DomTreeNode *Node) {
    // Loop over CFG successors to calculate DFlocal[Node]
    llvm::BasicBlock *BB = Node->getBlock();
    DomSetType &S = this->Frontiers[BB];       // The new set to fill in...
    if (this->getRoots().empty()) return S;

    if (BB)
      for (llvm::pred_iterator SI = pred_begin(BB), SE = pred_end(BB);
          SI != SE; ++SI) {
        llvm::BasicBlock *P = *SI;
        // Does Node immediately dominate this predecessor?
        llvm::DomTreeNode *SINode = DT[P];
        if (SINode && SINode->getIDom() != Node)
          S.insert(P);
      }
    // At this point, S is DFlocal.  Now we union in DFup's of our children...
    // Loop through and visit the nodes that Node immediately dominates (Node's
    // children in the IDomTree)
    //
    for (llvm::DomTreeNode::const_iterator
        NI = Node->begin(), NE = Node->end(); NI != NE; ++NI) {
      llvm::DomTreeNode *IDominee = *NI;
      const DomSetType &ChildDF = calculate(DT, IDominee);
      typename DomSetType::const_iterator CDFI = ChildDF.begin(), CDFE = ChildDF.end();
      for (; CDFI != CDFE; ++CDFI) {
        if (!DT.properlyDominates(Node, DT[*CDFI]))
          S.insert(*CDFI);
        }
      }
    return S;
  }
};

class PostDominanceFrontier : public llvm::FunctionPass {
  PostDominanceFrontierBase<llvm::BasicBlock> Base;

public:
  typedef llvm::DominatorTreeBase<llvm::BasicBlock> DomTreeT;
  typedef llvm::DomTreeNodeBase<llvm::BasicBlock> DomTreeNodeT;
  typedef llvm::DominanceFrontierBase<llvm::BasicBlock>::DomSetType DomSetType;
  typedef llvm::DominanceFrontierBase<llvm::BasicBlock>::iterator iterator;
  typedef llvm::DominanceFrontierBase<llvm::BasicBlock>::const_iterator const_iterator;

  static char ID; // Pass ID, replacement for typeid

  PostDominanceFrontier();

  PostDominanceFrontierBase<llvm::BasicBlock> &getBase() { return Base; }

  inline const std::vector<llvm::BasicBlock *> &getRoots() const {
    return Base.getRoots();
  }

  llvm::BasicBlock *getRoot() const { return Base.getRoot(); }

  bool isPostDominator() const { return Base.isPostDominator(); }

  iterator begin() { return Base.begin(); }

  const_iterator begin() const { return Base.begin(); }

  iterator end() { return Base.end(); }

  const_iterator end() const { return Base.end(); }

  iterator find(llvm::BasicBlock *B) { return Base.find(B); }

  const_iterator find(llvm::BasicBlock *B) const { return Base.find(B); }

  iterator addBasicBlock(llvm::BasicBlock *BB, const DomSetType &frontier) {
    return Base.addBasicBlock(BB, frontier);
  }

  void removeBlock(llvm::BasicBlock *BB) { return Base.removeBlock(BB); }

  void addToFrontier(iterator I, llvm::BasicBlock *Node) {
    return Base.addToFrontier(I, Node);
  }

  void removeFromFrontier(iterator I, llvm::BasicBlock *Node) {
    return Base.removeFromFrontier(I, Node);
  }

  bool compareDomSet(DomSetType &DS1, const DomSetType &DS2) const {
    return Base.compareDomSet(DS1, DS2);
  }

  bool compare(llvm::DominanceFrontierBase<llvm::BasicBlock> &Other) const {
    return Base.compare(Other);
  }

  void releaseMemory() override;

  bool runOnFunction(llvm::Function &) override;

  void getAnalysisUsage(llvm::AnalysisUsage &AU) const override;

  void print(llvm::raw_ostream &OS, const llvm::Module * = nullptr) const override;

  void dump() const;
};

llvm::FunctionPass* createPostDomFrontier();

} // end namespace intel

#endif
