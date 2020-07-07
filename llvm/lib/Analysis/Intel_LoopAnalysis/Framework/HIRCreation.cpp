//===------- HIRCreation.cpp - Creates HIR Nodes --------------------------===//
//
// Copyright (C) 2015-2020 Intel Corporation. All rights reserved.
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

#include "HIRCreation.h"

#include "llvm/Support/CommandLine.h"

#include "llvm/Analysis/Intel_LoopAnalysis/Framework/HIRRegionIdentification.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Framework/HIRSCCFormation.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HLNodeUtils.h"

#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/PostDominators.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/Instructions.h"

using namespace llvm;
using namespace llvm::loopopt;

#define DEBUG_TYPE "hir-creation"

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

  HLNode *TermNode = nullptr;

  if (BranchInst *BI = dyn_cast<BranchInst>(Terminator)) {
    if (BI->isConditional()) {
      Instruction *Cond = dyn_cast<Instruction>(BI->getCondition());
      DebugLoc CondDebugLoc = Cond ? Cond->getDebugLoc() : DebugLoc();

      // Create dummy if condition for now. Later on the compare instruction
      // operands will be substituted here and eliminated. If this is a bottom
      // test, it will be eliminated anyway.
      auto If = HNU.createHLIf(
          {CmpInst::Predicate::FCMP_TRUE, FastMathFlags(), CondDebugLoc},
          nullptr, nullptr);

      Ifs[If] = BB;
      If->setDebugLoc(BI->getDebugLoc());
      If->setProfileData(BI->getMetadata(LLVMContext::MD_prof));

      HLGoto *ThenGoto = HNU.createHLGoto(BB, BI->getSuccessor(0));
      HLNodeUtils::insertAsFirstThenChild(If, ThenGoto);
      Gotos.push_back(ThenGoto);

      HLGoto *ElseGoto = HNU.createHLGoto(BB, BI->getSuccessor(1));
      HLNodeUtils::insertAsFirstElseChild(If, ElseGoto);
      Gotos.push_back(ElseGoto);

      TermNode = If;

    } else {
      auto Goto = HNU.createHLGoto(BB, BI->getSuccessor(0));

      Gotos.push_back(Goto);
      Goto->setDebugLoc(BI->getDebugLoc());

      TermNode = Goto;
    }
  } else if (SwitchInst *SI = dyn_cast<SwitchInst>(Terminator)) {
    auto Switch = HNU.createHLSwitch(nullptr);
    Switch->setProfileData(SI->getMetadata(LLVMContext::MD_prof));

    Switches[Switch] = BB;
    Switch->setDebugLoc(SI->getDebugLoc());

    // Add dummy cases so they can be populated during the walk.
    for (unsigned I = 0, E = SI->getNumCases(); I < E; ++I) {
      Switch->addCase(nullptr);
    }

    // Add gotos to all the cases. They are added for convenience in forming
    // lexical links and will be eliminated later.
    auto DefaultGoto = HNU.createHLGoto(BB, SI->getDefaultDest());
    HLNodeUtils::insertAsFirstDefaultChild(Switch, DefaultGoto);
    Gotos.push_back(DefaultGoto);

    const DebugLoc &DbgLoc = SI->getDebugLoc();
    DefaultGoto->setDebugLoc(DbgLoc);

    unsigned Count = 1;

    for (auto I = SI->case_begin(), E = SI->case_end(); I != E; ++I, ++Count) {
      auto CaseGoto = HNU.createHLGoto(BB, I->getCaseSuccessor());
      HLNodeUtils::insertAsFirstChild(Switch, CaseGoto, Count);
      Gotos.push_back(CaseGoto);

      CaseGoto->setDebugLoc(DbgLoc);
    }

    TermNode = Switch;

  } else if (isa<ReturnInst>(Terminator) || isa<UnreachableInst>(Terminator)) {
    TermNode = HNU.createHLInst(Terminator);

  } else {
    assert(0 && "Unhandled terminator type!");
  }

  // Insert new node into the region.
  if (auto Region = dyn_cast<HLRegion>(InsertionPos)) {
    HLNodeUtils::insertAsFirstChild(Region, TermNode);
  } else {
    HLNodeUtils::insertAfter(InsertionPos, TermNode);
  }

  return TermNode;
}

HLNode *HIRCreation::populateInstSequence(BasicBlock *BB,
                                          HLNode *InsertionPos) {
  auto Label = HNU.createHLLabel(BB);

  Labels[BB] = Label;

  if (auto Region = dyn_cast<HLRegion>(InsertionPos)) {
    HLNodeUtils::insertAsFirstChild(Region, Label);
  } else {
    HLNodeUtils::insertAfter(InsertionPos, Label);
  }

  InsertionPos = Label;

  for (auto I = BB->getFirstInsertionPt(), E = std::prev(BB->end()); I != E;
       ++I) {
    auto Inst = HNU.createHLInst(&*I);
    if (const SelectInst *SI = dyn_cast<SelectInst>(&*I)) {
      MDNode *Prof = SI->getMetadata(LLVMContext::MD_prof);
      Inst->setProfileData(Prof);
    }
    HLNodeUtils::insertAfter(InsertionPos, Inst);
    InsertionPos = Inst;
  }

  InsertionPos = populateTerminator(BB, InsertionPos);

  return InsertionPos;
}

void HIRCreation::populateEndBBs(
    const BasicBlock *BB, SmallPtrSet<const BasicBlock *, 2> &EndBBs) const {
  EndBBs.insert(BB);

  auto Lp = LI.getLoopFor(BB);

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

  return RI.isReachableFrom(SuccessorBB, EndBBs, FromBBs);
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
    if (SuccessorBB != I->getCaseSuccessor()) {
      FromBBs.insert(I->getCaseSuccessor());

    } else if (Skipped) {
      // Switch has common successor bblock for some of the cases which is
      // trivial case of cross linking.
      return true;
    } else {
      Skipped = true;
    }
  }

  return RI.isReachableFrom(SuccessorBB, EndBBs, FromBBs);
}

static void
sortUsingReachability(const HIRRegionIdentification &RI,
                      SmallVectorImpl<BasicBlock *> &BBlocks,
                      const SmallPtrSetImpl<const BasicBlock *> &EndBBs) {
  assert(!BBlocks.empty() && "Empty bblocks not expected!");

  unsigned I = BBlocks.size() - 1;
  unsigned LowestSwapIndex = 0;

  // We need to order bblocks such that if B1 reaches B2, we place B1 before B2.
  //
  // We will start from the highest index and swap it with lowest index bblock
  // which is reachable from it. We keep doing this until there is no swapping,
  // then move on to the next highest index.
  // TODO: Is there a more efficient algorithm to do the partial sort?
  while (I > 0) {
    SmallPtrSet<const BasicBlock *, 8> FromBBs;
    FromBBs.insert(BBlocks[I]);
    bool Swapped = false;

    // Find the lowest index J such that BBlocks[J] is reachable from
    // BBlocks[I], then swap and break.
    for (unsigned J = LowestSwapIndex; J < I; ++J) {
      if (RI.isReachableFrom(BBlocks[J], EndBBs, FromBBs)) {
        std::swap(BBlocks[I], BBlocks[J]);
        // We don't need to check for indices lower than this as long as we are
        // dealing with index I. This is because we know BBlocks[I] reaches
        // BBlocks[J], but doesn'treach any of the bblocks from [0, J-1]. Since
        // reachability is transitive, BBlocks[J] which will become the new
        // BBlocks[I] after swap cannot reach any of them either.
        LowestSwapIndex = J + 1;
        Swapped = true;
        break;
      }
    }

    // I is in the correct place if not swapped since none of the bblocks below
    // it are reachable from it.
    if (!Swapped) {
      LowestSwapIndex = 0;
      --I;
    }
  }
}

bool HIRCreation::sortDomChildren(
    DomTreeNode *Node, SmallVectorImpl<BasicBlock *> &SortedChildren) const {

  for (auto *I : (*Node)) {
    auto BB = I->getBlock();
    if (CurRegion->containsBBlock(BB)) {
      SortedChildren.push_back(BB);
    }
  }

  if (SortedChildren.empty()) {
    return false;
  }

  auto NodeBB = Node->getBlock();
  SmallPtrSet<const BasicBlock *, 1> EndBBs;
  EndBBs.insert(NodeBB);

  sortUsingReachability(RI, SortedChildren, EndBBs);

  return true;
}

HLNode *HIRCreation::doPreOrderRegionWalk(BasicBlock *BB,
                                          HLNode *InsertionPos) {

  auto Reg = dyn_cast<HLRegion>(InsertionPos);

  if (Reg && Reg->isFunctionLevel()) {
    // Populate just the terminator of the function entry bblock.
    InsertionPos = populateTerminator(BB, InsertionPos);
  } else {
    assert(CurRegion->containsBBlock(BB) && "Encountered non-region bblock!");
    // Visit(link) this bblock to HIR.
    InsertionPos = populateInstSequence(BB, InsertionPos);
  }

  SmallVector<BasicBlock *, 8> DomChildren;

  // Sort dominator children using reachability.`
  if (!sortDomChildren(DT.getNode(BB), DomChildren)) {
    // No children to process.
    return InsertionPos;
  }

  auto TermNode = InsertionPos;
  auto IfTerm = dyn_cast<HLIf>(TermNode);
  auto SwitchTerm = IfTerm ? nullptr : dyn_cast<HLSwitch>(TermNode);

  auto Lp = LI.getLoopFor(BB);
  bool IsMultiExitLoop = (Lp && !Lp->getExitingBlock());
  bool IsLoopLatch = (IfTerm && Lp && (Lp->getLoopLatch() == BB));

  // The only two cases where a dominator child can be a multi-exit loop latch
  // are:
  // 1) BB belongs to multi-exit loop, Or
  // 2) BB is a loop latch which dominates outer multi-exit loop's latch bblock.
  bool DomChildMayBeMultiExitLoopLatch = (IsMultiExitLoop || IsLoopLatch);

  // Walk over dominator children sorted using reachability.
  for (auto *DomChildBB : DomChildren) {
    Loop *DomChildLp = nullptr;

    // Loop latch should be processed at the same lexical level as the loop
    // header. This happens automatically for single-exit loops but for
    // multi-exit loops we delay it for when we encounter the corresponding loop
    // header up the call chain.
    if (DomChildMayBeMultiExitLoopLatch &&
        (DomChildLp = LI.getLoopFor(DomChildBB)) &&
        !DomChildLp->getExitingBlock() && DomChildLp->isLoopLatch(DomChildBB)) {
      continue;
    }

    // DomChildBB is an early exit. We should link it after the loop latch.
    if (IsMultiExitLoop && !IsLoopLatch && !Lp->contains(DomChildBB)) {
      // In the case of nested multi-exit loops, DomChildBB may belong to an
      // ancestor loop. The following code tries to get to the outermost
      // multi-exit parent loop of Lp which does not contain DomChildBB.
      auto *EarlyExitLp = Lp;
      auto *ParentLp = Lp->getParentLoop();
      while (ParentLp && !ParentLp->getExitingBlock() &&
             !ParentLp->contains(DomChildBB) &&
             CurRegion->containsBBlock(ParentLp->getHeader())) {
        EarlyExitLp = ParentLp;
        ParentLp = ParentLp->getParentLoop();
      }
      EarlyExits[EarlyExitLp].push_back(DomChildBB);
      continue;
    }

    // Link if's then/else children.
    if (IfTerm) {
      // We need to keep the bottom test empty so that loop formation can get
      // rid of it. Therefore, we link the successor after the if instead of
      // inside it.
      if (!IsLoopLatch) {
        auto BI = cast<BranchInst>(BB->getTerminator());

        if ((DomChildBB == BI->getSuccessor(0)) &&
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
                   !isCrossLinked(BI, DomChildBB)) {
          doPreOrderRegionWalk(DomChildBB, IfTerm->getLastElseChild());
          continue;
        }
      }

    } else if (SwitchTerm) {
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
        if ((DomChildBB == I->getCaseSuccessor()) &&
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

  // If BB is loop header of multi-exit loop, link its latch and early exits.
  if (IsMultiExitLoop && (Lp->getHeader() == BB)) {
    InsertionPos = doPreOrderRegionWalk(Lp->getLoopLatch(), InsertionPos);

    // Exit blocks also need to be sorted as we do for dom children to avoid
    // backward jumps.
    auto &ExitBlocks = EarlyExits[Lp];

    if (!ExitBlocks.empty()) {
      SmallPtrSet<const BasicBlock *, 1> EndBBs;
      EndBBs.insert(BB);

      sortUsingReachability(RI, ExitBlocks, EndBBs);

      for (auto *ExitBB : ExitBlocks) {
        InsertionPos = doPreOrderRegionWalk(ExitBB, InsertionPos);
      }
    }
  }

  return InsertionPos;
}

void HIRCreation::run(HLContainerTy &Regions) {

  for (auto &I : RI) {
    CurRegion = HNU.createHLRegion(I);

    EarlyExits.clear();

    HLNode *LastNode =
        doPreOrderRegionWalk(CurRegion->getEntryBBlock(), CurRegion);

    (void)LastNode;
    assert(isa<HLRegion>(LastNode->getParent()) && "Invalid last region node!");
    assert((CurRegion->getExitBBlock() || CurRegion->isFunctionLevel() ||
            CurRegion->exitsFunction()) &&
           "Region's exit block not found!");

    Regions.push_back(*CurRegion);
  }
}
