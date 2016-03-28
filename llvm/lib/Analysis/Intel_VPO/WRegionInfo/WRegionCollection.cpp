//===------ WRegionCollection.cpp - Build WRN Graph -----*- C++ -*---------===//
//
//   Copyright (C) 2015-1016 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation. and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file implements the W-Region Collection pass.
///
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

#include "llvm/Analysis/Intel_VPO/WRegionInfo/WRegion.h"
#include "llvm/Analysis/Intel_VPO/WRegionInfo/WRegionUtils.h"
#include "llvm/Analysis/Intel_VPO/WRegionInfo/WRegionCollection.h"
#include "llvm/Analysis/Intel_VPO/WRegionInfo/WRegionPasses.h"
#include "llvm/Transforms/Intel_VPO/Utils/VPOUtils.h"

using namespace llvm;
using namespace llvm::vpo;

#define DEBUG_TYPE "vpo-wrncollection"

INITIALIZE_PASS_BEGIN(WRegionCollection, "vpo-wrncollection",
                      "VPO Work-Region Collection", false, true)
INITIALIZE_PASS_DEPENDENCY(DominatorTreeWrapperPass)
INITIALIZE_PASS_DEPENDENCY(LoopInfoWrapperPass)
INITIALIZE_PASS_DEPENDENCY(ScalarEvolutionWrapperPass)
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
  AU.addRequired<ScalarEvolutionWrapperPass>();
  AU.addRequired<LoopInfoWrapperPass>();
}

template <class T> void WRStack<T>::push(T X) {
  Stack_.push_back(X);
  return;
}

template <class T> void WRStack<T>::pop() {
  if (Stack_.size() > 0)
    Stack_.erase(Stack_.end() - 1);

  return;
}

template <class T> T WRStack<T>::top() {
  assert(Stack_.size() > 0);
  return Stack_.at(Stack_.size() - 1);
}

template <class T> size_t WRStack<T>::size() { return Stack_.size(); }

template <class T> bool WRStack<T>::empty() {
  return Stack_.size() == 0 ? true : false;
}

/// \brief TBD: get associated Loop Info for a given W-Region
bool WRegionCollection::isCandidateLoop(Loop &Lp) {

  /// Return false if we cannot handle this loop
  if (!Lp.isLoopSimplifyForm())
    return false;

  const SCEV *BETC = SE->getBackedgeTakenCount(&Lp);

  /// Only allow single BB loop for now.
  if (Lp.getBlocks().size() != 1)
    return false;

  /// Only allow constant trip count loop for now.
  if (!isa<SCEVConstant>(BETC))
    return false;

  return true;
}

/// \brief Visit the Dom Tree to identify all W-Regions
void WRegionCollection::doPreOrderDomTreeVisit(BasicBlock *BB,
                                               WRStack<WRegionNode *> *S) {
  // DEBUG(dbgs() << "\ndoPreOrderDomTreeVisit: processing BB:\n" << *BB);
  auto Root = DT->getNode(BB);
  WRegionNode *W;

  //
  // Iterate through all the intstructions in BB.
  //
  for (BasicBlock::iterator I = BB->begin(), E = BB->end(); I != E; ++I) {

    IntrinsicInst *Call = dyn_cast<IntrinsicInst>(&*I);

    if (Call) {
      Intrinsic::ID IntrinId = Call->getIntrinsicID();

      if (!VPOUtils::isIntelDirectiveOrClause(IntrinId))
        // Intrin is not intel_directive or intel_directive_qual*
        continue;

      StringRef DirOrClauseStr = VPOUtils::getDirectiveMetadataString(Call);
      if (IntrinId == Intrinsic::intel_directive) {

        // If the intrinsic represents an intel BEGIN directive, then
        // W is a pointer to an object for the corresponding WRN.
        // Otherwise, W is nullptr.
        W = WRegionUtils::createWRegion(DirOrClauseStr, BB, LI, S->size());
        if (W) {
          // DEBUG(dbgs() << "\n Starting New WRegion{\n");

          // The intrinsic represents an intel BEGIN directive.
          // W is a pointer to an object for the corresponding WRN.

          if (S->empty())
            // Top-level WRegionNode
            WRGraph->push_back(W);
          else {
            WRegionNode *Parent = S->top();
            if (!Parent->hasChildren()) {
              WRContainerTy::iterator WI(W);
              WRegionUtils::insertFirstChild(Parent, WI);
            } else {
              WRegionNode *C = Parent->getLastChild();
              WRContainerTy::iterator WI(C);
              WRegionUtils::insertAfter(WI, W);
            }
          }

          S->push(W);
          // DEBUG(dbgs() << "\nStacksize = " << S->size() << "\n");
        } else if (VPOUtils::isEndDirective(DirOrClauseStr)) {
          // The intrinsic represents an intel END directive
          // TODO: verify the END directive is the expected one

          // DEBUG(dbgs() << "\n} Ending WRegion.\n");
          W = S->top();
          W->setExitBBlock(BB);

          // generate BB set;
          // TODO: Remove this call later; the client will do it on demand
          W->populateBBSet();

          if (!S->empty())
            S->pop();
          // DEBUG(dbgs() << "\nStacksize = " << S->size() << "\n");
        }
      } else {
        // Process clauses below
        W = S->top();
        int ClauseID = VPOUtils::getClauseID(DirOrClauseStr);
        if (IntrinId == Intrinsic::intel_directive_qual) {
          // Handle clause with no arguments
          assert(Call->getNumArgOperands() == 1 &&
                 "Bad number of opnds for intel_directive_qual");
          W->handleQual(ClauseID);
        } else if (IntrinId == Intrinsic::intel_directive_qual_opnd) {
          // Handle clause with one argument
          assert(Call->getNumArgOperands() == 2 &&
                 "Bad number of opnds for intel_directive_qual_opnd");
          Value *V = Call->getArgOperand(1);
          W->handleQualOpnd(ClauseID, V);
        } else if (IntrinId == Intrinsic::intel_directive_qual_opndlist) {
          // Handle clause with argument list
          assert(Call->getNumArgOperands() >= 2 &&
                 "Bad number of opnds for intel_directive_qual_opndlist");
          W->handleQualOpndList(ClauseID, Call);
        }
      }

#if 0
      // TODO: implement WRNFlushNode and WRNCancelNode
      if (!S->empty() && I == E) {
        WRegionNode *A = S->top();
        if (WRegionNode* StandAloneWConstruct = dyn_cast<WRNFlushNode>(&*A) ||
            WRegionNode* StandAloneWConstruct = dyn_cast<WRNCancelNode>(&*A))
          S->pop();
      }
#endif

    } // if (Call)
  }   // for

  /// Walk over dominator children.
  for (auto D = Root->begin(), E = Root->end(); D != E; ++D) {
    auto DomChildBB = (*D)->getBlock();
    doPreOrderDomTreeVisit(DomChildBB, S);
  }

  return;
}

void WRegionCollection::doBuildWRegionGraph(Function &F) {

  DEBUG(dbgs() << "\nFunction = \n" << *this->Func);
  WRStack<WRegionNode *> S;

  doPreOrderDomTreeVisit(&F.getEntryBlock(), &S);

  return;
}

bool WRegionCollection::runOnFunction(Function &F) {
  this->Func = &F;

  DT = &getAnalysis<DominatorTreeWrapperPass>().getDomTree();
  SE = &getAnalysis<ScalarEvolutionWrapperPass>().getSE();
  LI = &getAnalysis<LoopInfoWrapperPass>().getLoopInfo();

  // Initialize maps from Directive/Clause strings to IDs
  // This has to be done before clients such as CFGRestructuring calls it
  VPOUtils::initDirectiveAndClauseStringMap();

#if 0
  // Run -vpo-cfg-restructuring transformation pass before this analysis.
  // Analysis passes can't modify LLVM IR.

  // CFG Restructuring, which puts directives into standalone basic blocks.
  // It maintains DominatorTree and LoopInfo.
  VPOUtils::CFGRestructuring(F, DT, LI);
#endif

  // TBD: This needs to be run for LLVM IR only path. For the HIR case,
  // standalone basic blocks created cause HIR region formation to not
  // include the SIMD directives which in turn causes WRegion formation
  // to fail. Commenting out the call for now.

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
