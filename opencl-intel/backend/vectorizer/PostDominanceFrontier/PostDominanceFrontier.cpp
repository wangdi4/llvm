//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include "llvm/Analysis/PostDominators.h"
#include "llvm/Instructions.h"
#include "llvm/Support/CFG.h"
#include "llvm/Support/Debug.h"
#include "llvm/ADT/DepthFirstIterator.h"
#include "llvm/ADT/SetOperations.h"
#include "llvm/Assembly/Writer.h"
#include "llvm/Analysis/DominatorInternals.h"
#include "PostDominanceFrontier.h"
#include "OCLPassSupport.h"
#include "InitializePasses.h"

/// Register pass to for opt
// static llvm::RegisterPass<intel::PostDominanceFrontier> PostDominanceFrontierPass("postdomfrontier", "Post-Dominance Frontier Construction", true, true);
using namespace llvm;


namespace intel {

char PostDominanceFrontier::ID = 0;

OCL_INITIALIZE_PASS_BEGIN(PostDominanceFrontier, "postdomfrontier",
                "Post-Dominance Frontier Construction", true, true)
OCL_INITIALIZE_PASS_DEPENDENCY(PostDominatorTree)
OCL_INITIALIZE_PASS_END(PostDominanceFrontier, "postdomfrontier",
                "Post-Dominance Frontier Construction", true, true)

//===----------------------------------------------------------------------===//
//  PostDominanceFrontier Implementation
//===----------------------------------------------------------------------===//
PostDominanceFrontier::PostDominanceFrontier()
    : llvm::DominanceFrontierBase(ID, true) {
        initializePostDominanceFrontierPass(*llvm::PassRegistry::getPassRegistry());
}

const llvm::DominanceFrontier::DomSetType &
PostDominanceFrontier::calculate(const llvm::PostDominatorTree &DT,
                                 const llvm::DomTreeNode *Node) {
  // Loop over CFG successors to calculate DFlocal[Node]
  llvm::BasicBlock *BB = Node->getBlock();
  DomSetType &S = Frontiers[BB];       // The new set to fill in...
  if (getRoots().empty()) return S;

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

    DomSetType::const_iterator CDFI = ChildDF.begin(), CDFE = ChildDF.end();
    for (; CDFI != CDFE; ++CDFI) {
      if (!DT.properlyDominates(Node, DT[*CDFI]))
        S.insert(*CDFI);
    }
  }

  return S;
}

llvm::FunctionPass* createPostDomFrontier() {
  return new PostDominanceFrontier();
}

}