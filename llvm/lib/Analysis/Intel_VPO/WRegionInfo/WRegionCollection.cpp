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

#include "llvm/Analysis/Intel_LoopAnalysis/Framework/HIRFramework.h"
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

/// \brief Inspect the BB to identify and create W-Regions
void WRegionCollection::getWRegionFromBB(BasicBlock *BB,
                                         WRStack<WRegionNode *> *S) {
  DEBUG(dbgs() << "\n=== getWRegionFromBB is processing this BB: " << *BB);
  WRegionNode *W;

  //
  // Iterate through all the intstructions in BB.
  //
  for (BasicBlock::iterator I = BB->begin(), E = BB->end(); I != E; ++I) {

    IntrinsicInst *Call = dyn_cast<IntrinsicInst>(&*I);

    if (Call) {
      Intrinsic::ID IntrinId = Call->getIntrinsicID();

      // Are we dealing with the directive.region.entry/exit representation?
      bool IsRegion = VPOAnalysisUtils::isRegionDirective(IntrinId);

      // Name of the directive or clause represented by this intrinsic
      StringRef DirOrClause = VPOAnalysisUtils::getDirOrClauseString(Call);

      DEBUG(dbgs() << "\n=== getWRegionFromBB found: " << DirOrClause << "\n");

      if (VPOAnalysisUtils::isOpenMPDirective(DirOrClause)) {

        int DirID = VPOAnalysisUtils::getDirectiveID(DirOrClause);

        // If the intrinsic represents an intel BEGIN directive, then
        // W is a pointer to an object for the corresponding WRN.
        // Otherwise, W is nullptr.
        W = WRegionUtils::createWRegion(DirID, BB, LI, S->size(), IsRegion);
        if (W) {
          // The intrinsic represents a BEGIN directive.
          // W points to the WRN created for it.

          assert((VPOAnalysisUtils::isBeginDirective(DirID) ||
                  VPOAnalysisUtils::isStandAloneBeginDirective(DirID)) &&
                 "An expected BEGIN directive is missing.");

          if (S->empty()) {
            // Top-level WRegionNode
            WRGraph->push_back(W);
          } else {
            WRegionNode *Parent = S->top();
            Parent->getChildren().push_back(W);
            W->setParent(Parent);
          }

          S->push(W);
          DEBUG(dbgs() << "\n  === New WRegion. ");
          DEBUG(dbgs() << "Stacksize after push = " << S->size() << "\n");
        } else if (VPOAnalysisUtils::isEndDirective(DirID) ||
                   VPOAnalysisUtils::isStandAloneEndDirective(DirID)) {
          // The intrinsic represents the END directive for the WRN that is
          // currently on S->top().
          // TODO: verify the END directive is the expected one

          assert(!(S->empty()) &&
                 "Unexpected empty WRN stack when seeing an END directive");

          W = S->top();
          W->finalize(BB); // set the ExitBB and wrap up the WRN

          S->pop();
          DEBUG(dbgs() << "\n  === Closed WRegion. ");
          DEBUG(dbgs() << "Stacksize after pop = " << S->size() << "\n");
        } else if (VPOAnalysisUtils::isListEndDirective(DirID) && 
                 !(S->empty())) {
          // We reach here only if using the intel_directive representation.
          // Under this representation, stand-alone directives don't have a
          // matchine end directive.
          W = S->top();
          if (VPOAnalysisUtils::isStandAloneBeginDirective(W->getDirID())) {
            // Current WRN is for a stand-alone directive, so
            // pop the stack as soon as DIR_QUAL_LIST_END is seen
            S->pop();
            DEBUG(dbgs() << "\n  === Closed WRegion (standalone dir). ");
            DEBUG(dbgs() << "Stacksize after pop = " << S->size() << "\n");
          }
        }
      } else if (VPOAnalysisUtils::isIntelClause(IntrinId)) { 
        // Process clauses from intel_directive_qual* intrinsics. We reach here
        // only if using the intel_directive_qual* representation.
        assert(!IsRegion && 
               "Unexpected directive.region.entry/exit representation");

        assert(!(S->empty()) &&
               "Unexpected empty WRN stack when seeing a clause");
        W = S->top();

        // Extract clause properties 
        ClauseSpecifier ClauseInfo(DirOrClause);

        // Parse the clause and update W
        W->parseClause(ClauseInfo, Call);
      }
    } // if (Call)
  } // for
  return;
}

/// \brief Invoking this routine with the entry BBs of a CFG puts all the BBs
/// of the CFG in the BBStack in a topologically sorted manner (ignoring
/// backedges). Popping BBStack gives BBs in sorted order. This ordering
/// allows getWRegionFromBB() to build nested OMP constructs correctly,
/// visiting all the inner constructs' directives before processing the END
/// directive of an enclosing outer construct.
void topSortBasicBlocks(
  BasicBlock *BB,
  WRStack<BasicBlock *> &BBStack,
  SmallPtrSetImpl<BasicBlock *> &Visited,
  bool DoVerifyBB
)
{
  // DEBUG(dbgs() << "\n=== topSortBasicBlocks visiting this BB: " << *BB);

  // Skip visited nodes.
  if (Visited.count(BB))
    return;

  // Verify that the directives in BB, if any, follow design rules
  if (DoVerifyBB) {
    bool PassedVerify = VPOAnalysisUtils::verifyBB(*BB, true);
    assert(PassedVerify && "Malformed directives in BBlock");
    (void) PassedVerify;
  }

  // Mark BB as "visited".
  Visited.insert(BB);

  // Visit all the successors first
  for (succ_iterator I = succ_begin(BB), E = succ_end(BB); I != E; ++I) {
    topSortBasicBlocks(*I, BBStack, Visited, DoVerifyBB);
  }

  // We are only interested in BBs that start with OMP directives. Paying the
  // cost now to look at BB's first instruction allows us to save memory by
  // only pushing BBs with such directives onto BBStack. It will also save 
  // compile time later in getWRegionFromBB, which no longer has to look at all
  // the BBs in the CFG. For typical OpenMP programs where the percentage of
  // BBs with OMP directives is small, this should result in net savings of
  // compile time.
  if (VPOAnalysisUtils::isIntelDirective(&(BB->front()))) {
    // DEBUG(dbgs() << "\n=== topSortBasicBlocks pushed this BB: " << *BB);
    BBStack.push(BB);
  }
}

void WRegionCollection::buildWRGraphFromLLVMIR(Function &F) {
  WRGraph = new (WRContainerTy);
  WRStack<WRegionNode *> S;

  // First, sort the CFG's BBs in topological order (ignoring backedges)
  // and put them in BBStack
  BasicBlock *RootBB = &F.getEntryBlock();
  WRStack<BasicBlock *> BBStack;
  SmallPtrSet<BasicBlock *, 32> Visited;

  // Having the last argument==true turns on the verifier by default.
  // TODO: guard it under a flag (or debug mode) when VPO is more stable.
  topSortBasicBlocks(RootBB, BBStack, Visited, true);

  // Then, visit the BBs in sorted order (by popping BBStack) to build WRNs
  while (!BBStack.empty()) {
    BasicBlock *BB = BBStack.top();
    getWRegionFromBB(BB, &S);
    BBStack.pop();
  }
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
