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

#include "llvm/Analysis/Intel_LoopAnalysis/HIRFramework.h"
#include "llvm/Analysis/Intel_VPO/WRegionInfo/WRegion.h"
#include "llvm/Analysis/Intel_VPO/WRegionInfo/WRegionUtils.h"
#include "llvm/Analysis/Intel_VPO/WRegionInfo/WRegionCollection.h"
#include "llvm/Analysis/Intel_VPO/WRegionInfo/WRegionPasses.h"
#include "llvm/Analysis/Intel_VPO/Utils/VPOAnalysisUtils.h"

using namespace llvm;
using namespace llvm::vpo;

#define DEBUG_TYPE "vpo-wrncollection"

INITIALIZE_PASS_BEGIN(WRegionCollection, "vpo-wrncollection",
                      "VPO Work-Region Collection", false, true)
INITIALIZE_PASS_DEPENDENCY(DominatorTreeWrapperPass)
INITIALIZE_PASS_DEPENDENCY(LoopInfoWrapperPass)
INITIALIZE_PASS_DEPENDENCY(ScalarEvolutionWrapperPass)
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
  AU.addRequired<LoopInfoWrapperPass>();
  AU.addRequired<ScalarEvolutionWrapperPass>();
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

      if (!VPOAnalysisUtils::isIntelDirectiveOrClause(IntrinId))
        // Intrin is not intel_directive or intel_directive_qual*
        continue;

      if (IntrinId == Intrinsic::intel_directive) {
        StringRef DirString = 
                            VPOAnalysisUtils::getDirectiveMetadataString(Call);
        int DirID = VPOAnalysisUtils::getDirectiveID(DirString);
        // If the intrinsic represents an intel BEGIN directive, then
        // W is a pointer to an object for the corresponding WRN.
        // Otherwise, W is nullptr.
        W = WRegionUtils::createWRegion(DirID, BB, LI, S->size());
        if (W) {
          // DEBUG(dbgs() << "\n Starting New WRegion{\n");

          // The intrinsic represents an intel BEGIN directive.
          // W is a pointer to an object for the corresponding WRN.

          if (S->empty())
            // Top-level WRegionNode
            WRGraph->push_back(W);
          else {
            WRegionNode *Parent = S->top();
            Parent->getChildren().push_back(W);
            W->setParent(Parent);
          }

          S->push(W);
          // DEBUG(dbgs() << "\nStacksize = " << S->size() << "\n");
        } else if (VPOAnalysisUtils::isEndDirective(DirID)) {
          // The intrinsic represents an intel END directive
          // TODO: verify the END directive is the expected one

          // DEBUG(dbgs() << "\n} Ending WRegion.\n");

          assert(!(S->empty()) &&
                 "Unexpected empty WRN stack when seeing an END directive");

          W = S->top();
          W->setExitBBlock(BB);

          // generate BB set;
          // TODO: Remove this call later; the client should do it on demand
          W->populateBBSet();

          S->pop();
          // DEBUG(dbgs() << "\nStacksize = " << S->size() << "\n");
        }
        else if (VPOAnalysisUtils::isListEndDirective(DirID) && 
                 !(S->empty())) {
          W = S->top();
          if (VPOAnalysisUtils::isStandAloneDirective(W->getDirID())) {
            // Current WRN is for a stand-alone directive, so
            // pop the stack as soon as DIR_QUAL_LIST_END is seen
            S->pop();
          }
        }
      } else { // Process clauses below
        assert(!(S->empty()) &&
               "Unexpected empty WRN stack when seeing a clause");
        W = S->top();
        StringRef ClauseString = 
                            VPOAnalysisUtils::getDirectiveMetadataString(Call);
        int ClauseID = VPOAnalysisUtils::getClauseID(ClauseString);
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
    } // if (Call)
  }   // for

  /// Walk over dominator children.
  for (auto D = Root->begin(), E = Root->end(); D != E; ++D) {
    auto DomChildBB = (*D)->getBlock();
    doPreOrderDomTreeVisit(DomChildBB, S);
  }

  return;
}

void WRegionCollection::buildWRGraphFromLLVMIR(Function &F) {
  WRGraph = new (WRContainerTy);
  WRStack<WRegionNode *> S;
  doPreOrderDomTreeVisit(&F.getEntryBlock(), &S);
  return;
}

bool WRegionCollection::runOnFunction(Function &F) {
  DEBUG(dbgs() << "\nENTER WRegionCollection::runOnFunction: "
               << F.getName() << "{\n");
  this->Func = &F;
  DT = &getAnalysis<DominatorTreeWrapperPass>().getDomTree();
  LI = &getAnalysis<LoopInfoWrapperPass>().getLoopInfo();
  SE = &getAnalysis<ScalarEvolutionWrapperPass>().getSE();

  DEBUG(dbgs() << "\n}EXIT WRegionCollection::runOnFunction: "
               << F.getName() << "\n");
  return false;
}

void WRegionCollection::buildWRGraph(InputIRKind IR) {
  DEBUG(dbgs() << "\nENTER WRegionCollection::buildWRGraph(InputIR=" 
               << IR <<"){\n");
  if (IR == HIR) {
    // TODO: move buildWRGraphFromHIR() from WRegionUtils to WRegionCollection
    //       after Vectorizer's HIR mode starts using this new interface
    auto HIRF = getAnalysisIfAvailable<loopopt::HIRFramework>();
    assert(HIRF && "HIR framework not available!");

    WRGraph = WRegionUtils::buildWRGraphFromHIR(*HIRF);
  } else if (IR == LLVMIR) {
    buildWRGraphFromLLVMIR(*Func);
  } else {
    llvm_unreachable("Unknown InputIRKind");
  }

  DEBUG(dbgs() << "\n} EXIT WRegionCollection::buildWRGraph\n");
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
#if !INTEL_PRODUCT_RELEASE
  /// TODO: implement later
  /// WR.print(OS);
#endif // !INTEL_PRODUCT_RELEASE
}

void WRegionCollection::verifyAnalysis() const {
  /// TODO: implement later
}
