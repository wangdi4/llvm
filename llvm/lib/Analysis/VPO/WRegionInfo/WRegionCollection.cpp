//===- WRegionCollection.cpp - Identifies HIR Regions *- C++ -*---------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements the W-Region Collection pass.
//
//===----------------------------------------------------------------------===//

#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Dominators.h"
#include "llvm/ADT/DepthFirstIterator.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/LoopPass.h"
#include "llvm/Support/Debug.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"

#include "llvm/Analysis/VPO/WRegionInfo/WRegion.h"
#include "llvm/Analysis/VPO/WRegionInfo/WRegionUtils.h"
#include "llvm/Analysis/VPO/WRegionInfo/WRegionCollection.h"
#include "llvm/Analysis/VPO/WRegionInfo/WRegionPasses.h"

using namespace llvm;
using namespace llvm::vpo;

#define DEBUG_TYPE "vpo-wrncollection"

INITIALIZE_PASS_BEGIN(WRegionCollection, "vpo-wrncollection",
                                     "VPO Work-Region Collection", false, true)
INITIALIZE_PASS_DEPENDENCY(DominatorTreeWrapperPass)
INITIALIZE_PASS_DEPENDENCY(LoopInfoWrapperPass)
INITIALIZE_PASS_DEPENDENCY(ScalarEvolution)
INITIALIZE_PASS_DEPENDENCY(LCSSA)
INITIALIZE_PASS_END(WRegionCollection, "vpo-wrncollection",
                                     "VPO Work-Region Collection", false, true)

char WRegionCollection::ID = 0;

FunctionPass *llvm::createWRegionCollectionPass() {
  return new WRegionCollection();
}

WRegionCollection::WRegionCollection() : FunctionPass(ID) {
  initializeWRegionCollectionPass(*PassRegistry::getPassRegistry());
}

void WRegionCollection::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.setPreservesAll();
  AU.addRequired<DominatorTreeWrapperPass>();
  AU.addRequired<ScalarEvolution>();
  AU.addRequired<LoopInfoWrapperPass>();
}

template <class T>
void WRStack<T>::push(T x)
{
  Stack_.push_back(x);
  return;
}

template <class T>
void WRStack<T>::pop()
{
  if (Stack_.size() > 0) {
    Stack_.erase(Stack_.end() - 1);
  }
  return;
}

template <class T>
T WRStack<T>::top()
{
  assert(Stack_.size() > 0);
  return Stack_.at(Stack_.size()-1);
}

template <class T>
size_t WRStack<T>::size()
{
  return Stack_.size();
}

template <class T>
bool WRStack<T>::empty()
{
  return Stack_.size() == 0 ? true : false;
}

/// \brief TBD: get associated Loop Info for a given W-Region
bool WRegionCollection::isCandidateLoop(Loop &Lp) {

  /// Return false if we cannot handle this loop
  if (!Lp.isLoopSimplifyForm()) {
    return false;
  }

  const SCEV *BETC = SE->getBackedgeTakenCount(&Lp);

  /// Only allow single BB loop for now.
  if (Lp.getBlocks().size() != 1) {
    return false;
  }

  /// Only allow constant trip count loop for now.
  if (!isa<SCEVConstant>(BETC)) {
    return false;
  }
  return true;
}

/// \brief Visit the Sub CFG to collect all Basic Block in the Sub CFG 
void WRegionCollection::doPreOrderSubCFGVisit(
  BasicBlock   *BB,
  BasicBlock   *ExitBB,
  SmallPtrSetImpl<BasicBlock*> *preOrderTreeVisited
)
{ 
  if (!preOrderTreeVisited->count(BB)) {
    //DEBUG(dbgs() << *BB);
    preOrderTreeVisited->insert(BB);

    for (succ_iterator I = succ_begin(BB), E = succ_end(BB); I != E; ++I) {
      if (*I != ExitBB) {
        doPreOrderSubCFGVisit(*I, ExitBB, preOrderTreeVisited);
      }  
    }
  }
  return;
}

/// \brief Visit the Dom Tree to identify all W-Regions 
void WRegionCollection::doPreOrderDomTreeVisit(
  BasicBlock *BB,
  WRStack<WRegion *> *S
)
{
  DEBUG(dbgs() << *BB);
  auto Root = DT->getNode(BB);

  WRegion       *W;
  WRegionBSetTy BBSet;
  
  for (BasicBlock::iterator I = BB->begin(), E = BB->end(); I != E; ++I) { 

    /// We know we've encountered a call instruction, so we need to 
    /// determine if it's a call to the function pointed to by LLVM

    if (CallInst* callInst = dyn_cast<CallInst>(&*I)) {

      /// To determine if it's a call to the function pointed to by LLVM
      /// directive START instrinsic function
      //if (callInst->getCalledFunction() == DirStart) {
          //BBSet.insert(BB);

          W = new WRegion(BB, BB, BBSet);
          
          /// Top-level W-Region
          if (S->empty()) { 
             WRegions.push_back(W);
          }
          else {
             WRegion *Parent = S->top();
             if (!Parent->hasChildren()) {
               WRegionUtils::insertFirstChild(Parent, W);
             }
             else {
               WRegionNode *C = Parent->getLastChild();
               WRegionUtils::insertAfter(C, W);
             }
          }

          S->push(W);
          DEBUG(dbgs() << "\nStacksize = " << S->size() 
                       << " \n " << *W->getEntryBBlock() << "\n");
          break;
      //}

      /// To determine if it's a call to the function pointed to by LLVM
      /// directive END instrinsic function
      //if (callInst->getCalledFunction() == DirEnd) {
         
          SmallPtrSet<BasicBlock*, 16> preOrderTreeVisited;
          preOrderTreeVisited.clear();

          W = S->top(); 

          W->setExitBBlock(BB);

          /// generate BB set out of DFS Tree visting in the sub CFG
          doPreOrderSubCFGVisit(W->getEntryBBlock(), 
                                W->getExitBBlock(), &preOrderTreeVisited);

          BBSet = W->getBBlockSet(); 

          for (SmallPtrSetIterator<BasicBlock *>  
               I = preOrderTreeVisited.begin(), 
               E = preOrderTreeVisited.end(); I != E; ++I) {
            BB = *I;
            BBSet.insert(BB);
          }
    
 
          if (!S->empty()) S->pop();

          DEBUG(dbgs() << "\nStacksize = " << S->size() 
                       << " \n " << *W->getExitBBlock() << "\n");
          break;
      //}
    }
  }

  /// Walk over dominator children.
  for (auto D = Root->begin(), E = Root->end(); D != E; ++D) {
    auto DomChildBB = (*D)->getBlock();
    doPreOrderDomTreeVisit(DomChildBB, S);
  }

  return;
}

void WRegionCollection::doBuildWRegionGraph(Function &F) {

  DEBUG(dbgs() << "\nFunction = \n" << *this->Func );
  WRStack<WRegion *> S;

  doPreOrderDomTreeVisit(&F.getEntryBlock(), &S); 
  return;
}

bool WRegionCollection::runOnFunction(Function &F) {
  this->Func = &F;

  DT = &getAnalysis<DominatorTreeWrapperPass>().getDomTree();
  SE = &getAnalysis<ScalarEvolution>();
  LI = &getAnalysis<LoopInfoWrapperPass>().getLoopInfo();

  DEBUG(dbgs() << "W-Region Graph Construction Start\n");
  doBuildWRegionGraph(F);
  DEBUG(dbgs() << "W-Region Graph Construction End\n");

  return false;
}

void WRegionCollection::releaseMemory() {

#if 0
  for (auto &I : WRegions) {
    delete I;
  }
  WRegions.clear();
#endif

}

void WRegionCollection::print(raw_ostream &OS, const Module *M) const {
  /// TODO: implement later
  /// WR.print(OS);
}

void WRegionCollection::verifyAnalysis() const {
  /// TODO: implement later
}
