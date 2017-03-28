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

#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/PostDominators.h"

#include "llvm/Analysis/Intel_LoopAnalysis/HIRCreation.h"
#include "llvm/Analysis/Intel_LoopAnalysis/HIRRegionIdentification.h"
#include "llvm/Analysis/Intel_LoopAnalysis/HIRSCCFormation.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Passes.h"

#include "llvm/Transforms/Intel_LoopTransforms/Utils/HLNodeUtils.h"

#include "llvm/IR/Dominators.h"
#include "llvm/IR/Instructions.h"

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

static cl::opt<bool> HIRPrinterDetails("hir-details",
                                       cl::desc("Show HIR with dd_ref details"),
                                       cl::init(false));

static cl::opt<bool>
    HIRFrameworkDetails("hir-framework-details",
                        cl::desc("Show framework detail in print"),
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
      Instruction *Cond = dyn_cast<Instruction>(BI->getCondition());
      DebugLoc CondDebugLoc = Cond ? Cond->getDebugLoc() : DebugLoc();

      // Create dummy if condition for now. Later on the compare instruction
      // operands will be substituted here and eliminated. If this is a bottom
      // test, it will be eliminated anyway.
      auto If = getHLNodeUtils().createHLIf(
          {CmpInst::Predicate::FCMP_TRUE, FastMathFlags(), CondDebugLoc},
          nullptr, nullptr);

      Ifs[If] = BB;

      // TODO: HLGoto targets should be assigned in a later pass.
      // TODO: Redundant gotos should be cleaned up during lexlink cleanup.
      HLGoto *ThenGoto = getHLNodeUtils().createHLGoto(BI->getSuccessor(0));
      getHLNodeUtils().insertAsFirstChild(If, ThenGoto, true);
      Gotos.push_back(ThenGoto);

      HLGoto *ElseGoto = getHLNodeUtils().createHLGoto(BI->getSuccessor(1));
      getHLNodeUtils().insertAsFirstChild(If, ElseGoto, false);
      Gotos.push_back(ElseGoto);

      getHLNodeUtils().insertAfter(InsertionPos, If);
      InsertionPos = If;

      If->setDebugLoc(BI->getDebugLoc());
    } else {
      auto Goto = getHLNodeUtils().createHLGoto(BI->getSuccessor(0));

      Gotos.push_back(Goto);

      getHLNodeUtils().insertAfter(InsertionPos, Goto);
      InsertionPos = Goto;

      Goto->setDebugLoc(BI->getDebugLoc());
    }
  } else if (SwitchInst *SI = dyn_cast<SwitchInst>(Terminator)) {
    auto Switch = getHLNodeUtils().createHLSwitch(nullptr);

    Switches[Switch] = BB;

    // Add dummy cases so they can be populated during the walk.
    for (unsigned I = 0, E = SI->getNumCases(); I < E; ++I) {
      Switch->addCase(nullptr);
    }

    // Add gotos to all the cases. They are added for convenience in forming
    // lexical links and will be eliminated later.
    auto DefaultGoto = getHLNodeUtils().createHLGoto(SI->getDefaultDest());
    getHLNodeUtils().insertAsFirstDefaultChild(Switch, DefaultGoto);
    Gotos.push_back(DefaultGoto);

    const DebugLoc &DbgLoc = SI->getDebugLoc();
    DefaultGoto->setDebugLoc(DbgLoc);

    unsigned Count = 1;

    for (auto I = SI->case_begin(), E = SI->case_end(); I != E; ++I, ++Count) {
      auto CaseGoto = getHLNodeUtils().createHLGoto(I.getCaseSuccessor());
      getHLNodeUtils().insertAsFirstChild(Switch, CaseGoto, Count);
      Gotos.push_back(CaseGoto);

      CaseGoto->setDebugLoc(DbgLoc);
    }

    getHLNodeUtils().insertAfter(InsertionPos, Switch);
    InsertionPos = Switch;

    Switch->setDebugLoc(SI->getDebugLoc());
  } else if (ReturnInst *RI = dyn_cast<ReturnInst>(Terminator)) {
    auto Inst = getHLNodeUtils().createHLInst(RI);
    getHLNodeUtils().insertAfter(InsertionPos, Inst);
    InsertionPos = Inst;
  } else {
    assert(0 && "Unhandled terminator type!");
  }

  return InsertionPos;
}

HLNode *HIRCreation::populateInstSequence(BasicBlock *BB,
                                          HLNode *InsertionPos) {
  auto Label = getHLNodeUtils().createHLLabel(BB);

  Labels[BB] = Label;

  if (auto Region = dyn_cast<HLRegion>(InsertionPos)) {
    getHLNodeUtils().insertAsFirstChild(Region, Label);
  } else {
    getHLNodeUtils().insertAfter(InsertionPos, Label);
  }

  InsertionPos = Label;

  for (auto I = BB->getFirstInsertionPt(), E = std::prev(BB->end()); I != E;
       ++I) {
    auto Inst = getHLNodeUtils().createHLInst(&*I);
    getHLNodeUtils().insertAfter(InsertionPos, Inst);
    InsertionPos = Inst;
  }

  InsertionPos = populateTerminator(BB, InsertionPos);

  return InsertionPos;
}

void HIRCreation::populateEndBBs(
    const BasicBlock *BB, SmallPtrSet<const BasicBlock *, 2> &EndBBs) const {
  EndBBs.insert(BB);

  auto Lp = LI->getLoopFor(BB);

  if (!Lp) {
    return;
  }

  EndBBs.insert(Lp->getHeader());
}

bool HIRCreation::isCrossLinked(const BranchInst *BI,
                                const BasicBlock *SuccessorBB) const {
  SmallPtrSet<const BasicBlock *, 1> FromBBs;
  SmallPtrSet<const BasicBlock *, 2> EndBBs;

  populateEndBBs(BI->getParent(), EndBBs);

  if (SuccessorBB == BI->getSuccessor(0)) {
    FromBBs.insert(BI->getSuccessor(1));
  } else {
    FromBBs.insert(BI->getSuccessor(0));
  }

  return RI->isReachableFrom(SuccessorBB, EndBBs, FromBBs);
}

bool HIRCreation::isCrossLinked(const SwitchInst *SI,
                                const BasicBlock *SuccessorBB) const {
  SmallPtrSet<const BasicBlock *, 8> FromBBs;
  SmallPtrSet<const BasicBlock *, 2> EndBBs;

  populateEndBBs(SI->getParent(), EndBBs);

  bool Skipped = false;

  if (SuccessorBB != SI->getDefaultDest()) {
    FromBBs.insert(SI->getDefaultDest());
  } else {
    Skipped = true;
  }

  for (auto I = SI->case_begin(), E = SI->case_end(); I != E; ++I) {
    if (SuccessorBB != I.getCaseSuccessor()) {
      FromBBs.insert(I.getCaseSuccessor());

    } else if (Skipped) {
      // Switch has common successor bblock for some of the cases which is
      // trivial case of cross linking.
      return true;
    } else {
      Skipped = true;
    }
  }

  return RI->isReachableFrom(SuccessorBB, EndBBs, FromBBs);
}

void HIRCreation::sortDomChildren(
    DomTreeNode *Node, SmallVectorImpl<BasicBlock *> &SortedChildren) const {

  // Sort the dom children of Node using post dominator relationship. If child1
  // post dominates child2, it should be visited after child2 otherwise forward
  // edges can turn into back edges.

  // TODO: look into dom child ordering for multi-exit loops.

  auto NodeBB = Node->getBlock();

  for (auto &I : (*Node)) {
    SortedChildren.push_back(I->getBlock());
  }

  SmallPtrSet<const BasicBlock *, 2> EndBBs;
  EndBBs.insert(NodeBB);

  // This check orders dom children that are reachable from other children,
  // before them.
  // This is because I couldn't think of an appropriate check for sorting in the
  // reverse order. So instead the children are visited in reverse order after
  // sorting.
  auto ReverseLexOrder = [this, EndBBs](BasicBlock *B1, BasicBlock *B2) {
    SmallPtrSet<const BasicBlock *, 8> FromBBs;
    FromBBs.insert(B2);

    // First check satisfies the strict weak ordering requirements of
    // comparator function.
    return ((B1 != B2) && RI->isReachableFrom(B1, EndBBs, FromBBs));
  };

  std::sort(SortedChildren.begin(), SortedChildren.end(), ReverseLexOrder);
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
          // Other successor is a backedge, link this one after the loop to be
          // able to get rid of the bottom test during loop formation
          !DT->dominates(BI->getSuccessor(1), BB) &&
          // This if successor is reachable from the other successor, link it
          // after the 'if' to prevent jumps between the then and else case.
          // There are couple of issues if we allow this jump-
          // 1) If it is from the else to then case it will look like a
          // backedge.
          // 2) It is harder for predicate related optimizations to deal with
          // such jumps.
          !isCrossLinked(BI, DomChildBB)) {
        doPreOrderRegionWalk(DomChildBB, IfTerm->getLastThenChild());
        continue;
      } else if ((DomChildBB == BI->getSuccessor(1)) &&
                 !DT->dominates(BI->getSuccessor(0), BB) &&
                 !isCrossLinked(BI, DomChildBB)) {
        doPreOrderRegionWalk(DomChildBB, IfTerm->getLastElseChild());
        continue;
      }

    } else if (auto SwitchTerm = dyn_cast<HLSwitch>(TermNode)) {
      // Link switch's case children.
      auto SI = cast<SwitchInst>(BB->getTerminator());

      // TODO: apply the same domination logic to switches as is applied to
      // 'ifs' to form loops with switch acting as the backedge.
      if ((DomChildBB == SI->getDefaultDest()) &&
          !isCrossLinked(SI, DomChildBB)) {
        doPreOrderRegionWalk(DomChildBB, SwitchTerm->getLastDefaultCaseChild());
        continue;
      }

      unsigned Count = 1;
      bool IsCaseChild = false;

      for (auto I = SI->case_begin(), E = SI->case_end(); I != E;
           ++I, ++Count) {
        if ((DomChildBB == I.getCaseSuccessor()) &&
            !isCrossLinked(SI, DomChildBB)) {
          doPreOrderRegionWalk(DomChildBB, SwitchTerm->getLastCaseChild(Count));
          IsCaseChild = true;
          break;
        }
      }

      if (IsCaseChild) {
        continue;
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
  if (auto LastIf = dyn_cast<HLIf>(LastChild)) {
    auto ExitRegionBB = getSrcBBlock(LastIf);
    assert(ExitRegionBB && "Could not find src bblock of if!");

    CurRegion->setExitBBlock(const_cast<BasicBlock *>(ExitRegionBB));

  } else {
    assert((CurRegion->exitsFunction() || CurRegion->getExitBBlock()) &&
           "Exit block of region not found!");
  }
}

void HIRCreation::create() {

  for (auto &I : *RI) {

    CurRegion = getHLNodeUtils().createHLRegion(I);

    HLNode *LastNode =
        doPreOrderRegionWalk(CurRegion->getEntryBBlock(), CurRegion);

    (void)LastNode;
    assert(isa<HLRegion>(LastNode->getParent()) && "Invalid last region node!");

    setExitBBlock();

    Regions.push_back(*CurRegion);
  }
}

bool HIRCreation::runOnFunction(Function &F) {
  DT = &getAnalysis<DominatorTreeWrapperPass>().getDomTree();
  PDT = &getAnalysis<PostDominatorTreeWrapperPass>().getPostDomTree();
  LI = &getAnalysis<LoopInfoWrapperPass>().getLoopInfo();
  RI = &getAnalysis<HIRRegionIdentification>();

  HNU.reset(F);

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
  getHLNodeUtils().destroyAll();
}

void HIRCreation::print(raw_ostream &OS, const Module *M) const {
  print(HIRPrinterDetails, OS, M);
}

void HIRCreation::print(bool FrameworkDetails, raw_ostream &OS,
                        const Module *M) const {
  formatted_raw_ostream FOS(OS);
  auto RegBegin = RI->begin();
  auto SCCF = &getAnalysis<HIRSCCFormation>();
  unsigned Offset = 0;
  bool PrintFrameworkDetails = HIRFrameworkDetails || FrameworkDetails;

  for (auto I = begin(), E = end(); I != E; ++I, ++Offset) {
    assert(isa<HLRegion>(I) && "Top level node is not a region!");
    const HLRegion *Region = cast<HLRegion>(I);
    if (!HIRPrintModified || Region->shouldGenCode()) {
      // Print SCCs in hir-parser output and in detailed mode.
      if (PrintFrameworkDetails) {
        SCCF->print(FOS, RegBegin + Offset);
      }

      FOS << "\n";
      Region->print(FOS, 0, PrintFrameworkDetails, HIRPrinterDetails);
    }
  }
  FOS << "\n";
}

void HIRCreation::verifyAnalysis() const {}
