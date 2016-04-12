//===------- HIRCreation.cpp - Creates HIR Nodes --------------------------===//
//
// Copyright (C) 2015-2016 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// This file implements the HIR creation pass.
//
//===----------------------------------------------------------------------===//

#include "llvm/Support/CommandLine.h"

#include "llvm/Analysis/PostDominators.h"
#include "llvm/Analysis/LoopInfo.h"

#include "llvm/Analysis/Intel_LoopAnalysis/HIRCreation.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Passes.h"
#include "llvm/Analysis/Intel_LoopAnalysis/HIRRegionIdentification.h"
#include "llvm/Analysis/Intel_LoopAnalysis/HIRSCCFormation.h"

#include "llvm/Transforms/Intel_LoopTransforms/Utils/HLNodeUtils.h"

#include "llvm/IR/Instructions.h"
#include "llvm/IR/Dominators.h"

using namespace llvm;
using namespace llvm::loopopt;

#define DEBUG_TYPE "hir-creation"

INITIALIZE_PASS_BEGIN(HIRCreation, "hir-creation", "HIR Creation", false, true)
INITIALIZE_PASS_DEPENDENCY(DominatorTreeWrapperPass)
INITIALIZE_PASS_DEPENDENCY(PostDominatorTreeWrapperPass)
INITIALIZE_PASS_DEPENDENCY(LoopInfoWrapperPass)
INITIALIZE_PASS_DEPENDENCY(HIRRegionIdentification)
INITIALIZE_PASS_DEPENDENCY(HIRSCCFormation)
INITIALIZE_PASS_END(HIRCreation, "hir-creation", "HIR Creation", false, true)

char HIRCreation::ID = 0;

static cl::opt<bool>
    HIRPrinterDetailed("hir-details", cl::desc("Show HIR with dd_ref details"),
                       cl::init(false));

static cl::opt<bool>
    HIRPrintModified("hir-print-modified",
                     cl::desc("Show modified HIR Regions only"),
                     cl::init(false));

FunctionPass *llvm::createHIRCreationPass() { return new HIRCreation(); }

HIRCreation::HIRCreation() : FunctionPass(ID) {
  initializeHIRCreationPass(*PassRegistry::getPassRegistry());
}

void HIRCreation::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.setPreservesAll();
  AU.addRequiredTransitive<DominatorTreeWrapperPass>();
  AU.addRequiredTransitive<PostDominatorTreeWrapperPass>();
  AU.addRequiredTransitive<LoopInfoWrapperPass>();
  AU.addRequiredTransitive<HIRRegionIdentification>();
  // Only used for printing.
  AU.addRequiredTransitive<HIRSCCFormation>();
}

const BasicBlock *HIRCreation::getSrcBBlock(HLIf *If) const {
  auto Iter = Ifs.find(If);

  if (Iter != Ifs.end()) {
    return Iter->second;
  }

  return nullptr;
}

const BasicBlock *HIRCreation::getSrcBBlock(HLSwitch *Switch) const {
  auto Iter = Switches.find(Switch);

  if (Iter != Switches.end()) {
    return Iter->second;
  }

  return nullptr;
}

HLNode *HIRCreation::populateTerminator(BasicBlock *BB, HLNode *InsertionPos) {
  auto Terminator = BB->getTerminator();

  if (BranchInst *BI = dyn_cast<BranchInst>(Terminator)) {
    if (BI->isConditional()) {
      // Create dummy if condition for now. Later on the compare instruction
      // operands will be substituted here and eliminated. If this is a bottom
      // test, it will be eliminated anyway.
      auto If = HLNodeUtils::createHLIf(CmpInst::Predicate::FCMP_TRUE, nullptr,
                                        nullptr);

      Ifs[If] = BB;

      // TODO: HLGoto targets should be assigned in a later pass.
      // TODO: Redundant gotos should be cleaned up during lexlink cleanup.
      HLGoto *ThenGoto = HLNodeUtils::createHLGoto(BI->getSuccessor(0));
      HLNodeUtils::insertAsFirstChild(If, ThenGoto, true);
      Gotos.push_back(ThenGoto);

      HLGoto *ElseGoto = HLNodeUtils::createHLGoto(BI->getSuccessor(1));
      HLNodeUtils::insertAsFirstChild(If, ElseGoto, false);
      Gotos.push_back(ElseGoto);

      HLNodeUtils::insertAfter(InsertionPos, If);
      InsertionPos = If;
    } else {
      auto Goto = HLNodeUtils::createHLGoto(BI->getSuccessor(0));

      Gotos.push_back(Goto);

      HLNodeUtils::insertAfter(InsertionPos, Goto);
      InsertionPos = Goto;
    }
  } else if (SwitchInst *SI = dyn_cast<SwitchInst>(Terminator)) {
    auto Switch = HLNodeUtils::createHLSwitch(nullptr);

    Switches[Switch] = BB;

    // Add dummy cases so they can be populated during the walk.
    for (unsigned I = 0, E = SI->getNumCases(); I < E; ++I) {
      Switch->addCase(nullptr);
    }

    // Add gotos to all the cases. They are added for convenience in forming
    // lexical links and will be eliminated later.
    auto DefaultGoto = HLNodeUtils::createHLGoto(SI->getDefaultDest());
    HLNodeUtils::insertAsFirstDefaultChild(Switch, DefaultGoto);
    Gotos.push_back(DefaultGoto);

    unsigned Count = 1;

    for (auto I = SI->case_begin(), E = SI->case_end(); I != E; ++I, ++Count) {
      auto CaseGoto = HLNodeUtils::createHLGoto(I.getCaseSuccessor());
      HLNodeUtils::insertAsFirstChild(Switch, CaseGoto, Count);
      Gotos.push_back(CaseGoto);
    }

    HLNodeUtils::insertAfter(InsertionPos, Switch);
    InsertionPos = Switch;
  } else if (ReturnInst *RI = dyn_cast<ReturnInst>(Terminator)) {
    auto Inst = HLNodeUtils::createHLInst(RI);
    HLNodeUtils::insertAfter(InsertionPos, Inst);
    InsertionPos = Inst;
  } else {
    assert(0 && "Unhandled terminator type!");
  }

  return InsertionPos;
}

HLNode *HIRCreation::populateInstSequence(BasicBlock *BB,
                                          HLNode *InsertionPos) {
  auto Label = HLNodeUtils::createHLLabel(BB);

  Labels[BB] = Label;

  if (auto Region = dyn_cast<HLRegion>(InsertionPos)) {
    HLNodeUtils::insertAsFirstChild(Region, Label);
  } else {
    HLNodeUtils::insertAfter(InsertionPos, Label);
  }

  InsertionPos = Label;

  for (auto I = BB->getFirstInsertionPt(), E = std::prev(BB->end()); I != E;
       ++I) {
    auto Inst = HLNodeUtils::createHLInst(&*I);
    HLNodeUtils::insertAfter(InsertionPos, Inst);
    InsertionPos = Inst;
  }

  InsertionPos = populateTerminator(BB, InsertionPos);

  return InsertionPos;
}

bool HIRCreation::postDominatesAllCases(SwitchInst *SI, BasicBlock *BB) const {

  if (!PDT->dominates(BB, SI->getDefaultDest())) {
    return false;
  }

  for (auto I = SI->case_begin(), E = SI->case_end(); I != E; ++I) {
    if (!PDT->dominates(BB, I.getCaseSuccessor())) {
      return false;
    }
  }

  return true;
}

void HIRCreation::sortDomChildren(
    DomTreeNode *Node, SmallVectorImpl<BasicBlock *> &SortedChildren) const {

  // Sort the dom children of Node using post dominator relationship. If child1
  // post dominates child2, it should be visited after child2 otherwise forward
  // edges can turn into back edges.

  // TODO: look into dom child ordering for multi-exit loops.

  for (auto &I : (*Node)) {
    SortedChildren.push_back(I->getBlock());
  }

  // This check orders children that post-dominate other children, before them.
  // This is because I couldn't think of an appropriate check for sorting in the
  // reverse order. So instead the children are visited in reverse order after
  // sorting.
  auto PostDomOrder = [this](BasicBlock *B1, BasicBlock *B2) {
    // First check satisfies the strict weak ordering requirements of
    // comparator function.
    return ((B1 != B2) && PDT->dominates(B1, B2));
  };

  std::sort(SortedChildren.begin(), SortedChildren.end(), PostDomOrder);
}

HLNode *HIRCreation::doPreOrderRegionWalk(BasicBlock *BB,
                                          HLNode *InsertionPos) {

  if (!CurRegion->containsBBlock(BB)) {
    return InsertionPos;
  }

  SmallVector<BasicBlock *, 8> DomChildren;
  auto Root = DT->getNode(BB);

  // Visit(link) this bblock to HIR.
  InsertionPos = populateInstSequence(BB, InsertionPos);
  auto TermNode = InsertionPos;

  // Sort dominator children.
  sortDomChildren(Root, DomChildren);

  // Walk over dominator children in reverse order since post-dominating
  // children preceed the children they dominate.
  for (auto RI = DomChildren.rbegin(), RE = DomChildren.rend(); RI != RE;
       ++RI) {

    auto DomChildBB = (*RI);

    // Link if's then/else children.
    if (auto IfTerm = dyn_cast<HLIf>(TermNode)) {
      auto BI = cast<BranchInst>(BB->getTerminator());

      if ((DomChildBB == BI->getSuccessor(0)) &&
          // If one of the 'if' successors post-dominates the other, it is
          // better to link it after the 'if' instead of linking it as a child.
          !PDT->dominates(DomChildBB, BI->getSuccessor(1))) {
        doPreOrderRegionWalk(DomChildBB, IfTerm->getLastThenChild());
        continue;
      } else if (DomChildBB == BI->getSuccessor(1) &&
                 !PDT->dominates(DomChildBB, BI->getSuccessor(0))) {
        doPreOrderRegionWalk(DomChildBB, IfTerm->getLastElseChild());
        continue;
      }

    } else if (auto SwitchTerm = dyn_cast<HLSwitch>(TermNode)) {
      // Link switch's case children.
      auto SI = cast<SwitchInst>(BB->getTerminator());

      if (!postDominatesAllCases(SI, DomChildBB)) {

        if (DomChildBB == SI->getDefaultDest()) {
          doPreOrderRegionWalk(DomChildBB,
                               SwitchTerm->getLastDefaultCaseChild());
          continue;
        }

        unsigned Count = 1;
        bool IsCaseChild = false;

        for (auto I = SI->case_begin(), E = SI->case_end(); I != E;
             ++I, ++Count) {
          if (DomChildBB == I.getCaseSuccessor()) {
            doPreOrderRegionWalk(DomChildBB,
                                 SwitchTerm->getLastCaseChild(Count));
            IsCaseChild = true;
            break;
          }
        }

        if (IsCaseChild) {
          continue;
        }
      }
    }

    // Keep linking dominator children.
    InsertionPos = doPreOrderRegionWalk(DomChildBB, InsertionPos);
  }

  return InsertionPos;
}

void HIRCreation::setExitBBlock() const {

  auto LastChild = CurRegion->getLastChild();
  assert(LastChild && "Last child of region is null!");
  // TODO: Handle other last child types later.
  assert(isa<HLIf>(LastChild) && "Unexpected last region child!");

  auto ExitRegionBB = getSrcBBlock(cast<HLIf>(LastChild));
  assert(ExitRegionBB && "Could not find src bblock of if!");

  CurRegion->setExitBBlock(const_cast<BasicBlock *>(ExitRegionBB));
}

void HIRCreation::create() {

  for (auto &I : *RI) {

    CurRegion = HLNodeUtils::createHLRegion(I);

    HLNode *LastNode =
        doPreOrderRegionWalk(CurRegion->getEntryBBlock(), CurRegion);

    (void)LastNode;
    assert(isa<HLRegion>(LastNode->getParent()) && "Invalid last region node!");

    setExitBBlock();

    Regions.push_back(CurRegion);
  }
}

bool HIRCreation::runOnFunction(Function &F) {
  DT = &getAnalysis<DominatorTreeWrapperPass>().getDomTree();
  PDT = &getAnalysis<PostDominatorTreeWrapperPass>().getPostDomTree();
  LI = &getAnalysis<LoopInfoWrapperPass>().getLoopInfo();
  RI = &getAnalysis<HIRRegionIdentification>();

  create();

  return false;
}

void HIRCreation::releaseMemory() {
  // Clear HIR regions
  Regions.clear();

  // Clear framework data structures.
  Labels.clear();
  Gotos.clear();
  Ifs.clear();
  Switches.clear();

  // Destroy all HLNodes.
  HLNodeUtils::destroyAll();
}

void HIRCreation::print(raw_ostream &OS, const Module *M) const {
  printImpl(OS, HIRPrinterDetailed);
}

void HIRCreation::printWithFrameworkDetails(raw_ostream &OS) const {
  printImpl(OS, true);
}

void HIRCreation::printImpl(raw_ostream &OS, bool FrameworkDetails) const {
  formatted_raw_ostream FOS(OS);
  auto RegBegin = RI->begin();
  auto SCCF = &getAnalysis<HIRSCCFormation>();
  unsigned Offset = 0;

  for (auto I = begin(), E = end(); I != E; ++I, ++Offset) {
    assert(isa<HLRegion>(I) && "Top level node is not a region!");
    const HLRegion *Region = cast<HLRegion>(I);
    if (!HIRPrintModified || Region->shouldGenCode()) {
      // Print SCCs in hir-parser output and in detailed mode.
      if (FrameworkDetails) {
        SCCF->print(FOS, RegBegin + Offset);
      }

      FOS << "\n";
      Region->print(FOS, 0, FrameworkDetails, HIRPrinterDetailed);
    }
  }
  FOS << "\n";
}

void HIRCreation::verifyAnalysis() const {}
