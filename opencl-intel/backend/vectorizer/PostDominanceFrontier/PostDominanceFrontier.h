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
    calculate(DT, DT[this->Roots[0]]);
  }

  const DomSetType &calculate(const DomTreeT &DT, const DomTreeNodeT *Node);
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
