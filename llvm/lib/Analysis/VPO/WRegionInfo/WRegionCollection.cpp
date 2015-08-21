//===------ WRegionCollection.cpp - Build WRN Graph -----*- C++ -*---------===//
//
//   Copyright (C) 2015 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation. and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//
//
//   This file implements the W-Region Collection pass.
//
//===----------------------------------------------------------------------===//

#include "llvm/Pass.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Dominators.h"
#include "llvm/ADT/DepthFirstIterator.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/LoopPass.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"

#include "llvm/Analysis/VPO/WRegionInfo/WRegion.h"
#include "llvm/Analysis/VPO/WRegionInfo/WRegionUtils.h"
#include "llvm/Analysis/VPO/WRegionInfo/WRegionCollection.h"
#include "llvm/Analysis/VPO/WRegionInfo/WRegionPasses.h"
#include "llvm/Transforms/VPO/Utils/VPOUtils.h"

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

/// \brief Visit the Dom Tree to identify all W-Regions 
void WRegionCollection::doPreOrderDomTreeVisit(
  BasicBlock *BB,
  WRStack<WRegionNode *> *S
)
{
  // DEBUG(dbgs() << "\ndoPreOrderDomTreeVisit: processing BB:\n" << *BB);
  auto Root = DT->getNode(BB);

  WRegionNode *W;
  
  //
  // Iterate through all the intstructions in BB.
  //
  for (BasicBlock::iterator I = BB->begin(), E = BB->end(); I != E; ++I) {

    IntrinsicInst* IntrinInst = dyn_cast<IntrinsicInst>(&*I);

    if (IntrinInst) { 
      if (IntrinInst->getIntrinsicID() == Intrinsic::intel_directive) {

        StringRef DirString = VPOUtils::getDirectiveMetadataString(IntrinInst);

        // If the intrinsic represents an intel BEGIN directive, then
        // W is a pointer to an object for the corresponding WRN.
        // Otherwise, W is nullptr.
        W = WRegionUtils::createWRegion(DirString, BB);
        if (W) {
          // DEBUG(dbgs() << "\n Starting New WRegion{\n");

          // The intrinsic represents an intel BEGIN directive.
          // W is a pointer to an object for the corresponding WRN.
          if (S->empty()) { 
            // Top-level WRegionNode
            WRGraph->push_back(W);
          }
          else {
            WRegionNode *Parent = S->top();
            if (!Parent->hasChildren()) {
              WRegionUtils::insertFirstChild(Parent, W);
            }
            else {
              WRegionNode *C = Parent->getLastChild();
              WRegionUtils::insertAfter(C, W);
            }
          }

          S->push(W);
          // DEBUG(dbgs() << "\nStacksize = " << S->size() << "\n");
        }
        else if (WRegionUtils::isEndDirective(DirString)) {
          // The intrinsic represents an intel END directive

          // DEBUG(dbgs() << "\n} Ending WRegion.\n");
          W = S->top(); 
          W->setExitBBlock(BB);

          // generate BB set; 
          // TODO: Remove this call later; the client will do it on demand
          W->populateBBlockSet();

          if (!S->empty()) S->pop();
          // DEBUG(dbgs() << "\nStacksize = " << S->size() << "\n");
        }
      }
      else if (IntrinInst->getIntrinsicID() == 
                              Intrinsic::intel_directive_qual) {
        WRegionUtils::handleDirQual(IntrinInst, S->top());
      }
      else if (IntrinInst->getIntrinsicID() == 
                              Intrinsic::intel_directive_qual_opnd) {
        WRegionUtils::handleDirQualOpnd(IntrinInst, S->top());
      }
      else if (IntrinInst->getIntrinsicID() == 
                              Intrinsic::intel_directive_qual_opndlist) {
        WRegionUtils::handleDirQualOpndList(IntrinInst, S->top());
      }

#if 0
      // TODO: implement WRNFlushNode and WRNCancelNode
      if (!S->empty() && I == E) {
        WRegionNode *A = S->top();
        if (WRegionNode* StandAloneWConstruct = dyn_cast<WRNFlushNode>(&*A) ||
            WRegionNode* StandAloneWConstruct = dyn_cast<WRNCancelNode>(&*A)) {
          S->pop();
        }
      }
#endif

    } // if (IntrinInst)
  } // for

  /// Walk over dominator children.
  for (auto D = Root->begin(), E = Root->end(); D != E; ++D) {
    auto DomChildBB = (*D)->getBlock();
    doPreOrderDomTreeVisit(DomChildBB, S);
  }

  return;
}


void WRegionCollection::doBuildWRegionGraph(Function &F) {

  DEBUG(dbgs() << "\nFunction = \n" << *this->Func );
  WRStack<WRegionNode *> S;

  doPreOrderDomTreeVisit(&F.getEntryBlock(), &S); 

  return;
}

bool WRegionCollection::runOnFunction(Function &F) {
  this->Func = &F;

  DT = &getAnalysis<DominatorTreeWrapperPass>().getDomTree();
  SE = &getAnalysis<ScalarEvolution>();
  LI = &getAnalysis<LoopInfoWrapperPass>().getLoopInfo();

  // CFG Restructuring, which puts directives into standalone basic blocks.
  // It maintains DominatorTree and LoopInfo.
  VPOUtils::CFGRestructuring(F, DT, LI);

  DEBUG(dbgs() << "W-Region Graph Construction Start {\n");
  WRGraph = new (WRContainerTy);
  doBuildWRegionGraph(F);
  DEBUG(dbgs() << "} W-Region Graph Construction End\n");

  // TODO: This return should return true if call to CFGRestruction()
  // has modifed the IR.
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
