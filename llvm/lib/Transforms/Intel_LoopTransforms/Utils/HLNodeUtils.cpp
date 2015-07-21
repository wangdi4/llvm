//===------- HLNodeUtils.cpp - Implements HLNodeUtils class ------*- C++==-===//
//
// Copyright (C) 2015 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// This file implements HLNodeUtils class.
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/Intel_LoopTransforms/Utils/HLNodeUtils.h"

using namespace llvm;
using namespace loopopt;

HLRegion *HLNodeUtils::createHLRegion(IRRegion *IRReg) {
  return new HLRegion(IRReg);
}

HLSwitch *HLNodeUtils::createHLSwitch(RegDDRef *ConditionRef) {
  return new HLSwitch(ConditionRef);
}

HLLabel *HLNodeUtils::createHLLabel(BasicBlock *SrcBB) {
  return new HLLabel(SrcBB);
}

HLGoto *HLNodeUtils::createHLGoto(BasicBlock *TargetBB, HLLabel *TargetL) {
  return new HLGoto(TargetBB, TargetL);
}

HLInst *HLNodeUtils::createHLInst(Instruction *In) { return new HLInst(In); }

HLIf *HLNodeUtils::createHLIf(CmpInst::Predicate FirstPred, RegDDRef *Ref1,
                              RegDDRef *Ref2) {
  return new HLIf(FirstPred, Ref1, Ref2);
}

HLLoop *HLNodeUtils::createHLLoop(const Loop *LLVMLoop, bool IsDoWh) {
  return new HLLoop(LLVMLoop, IsDoWh);
}

HLLoop *HLNodeUtils::createHLLoop(HLIf *ZttIf, RegDDRef *LowerDDRef,
                                  RegDDRef *UpperDDRef, RegDDRef *StrideDDRef,
                                  bool IsDoWh, unsigned NumEx) {
  return new HLLoop(ZttIf, LowerDDRef, UpperDDRef, StrideDDRef, IsDoWh, NumEx);
}

struct HLNodeUtils::CloneVisitor {

  HLContainerTy *CloneContainer;
  GotoContainerTy *GotoList;
  LabelMapTy *LabelMap;

  CloneVisitor(HLContainerTy *Container, GotoContainerTy *GList,
               LabelMapTy *LMap)
      : CloneContainer(Container), GotoList(GList), LabelMap(LMap) {}

  void visit(HLNode *Node) {
    CloneContainer->push_back(Node->cloneImpl(GotoList, LabelMap));
  }

  void postVisit(HLNode *Node) {}
  bool isDone() { return false; }

  void postVisitUpdate() { updateGotos(GotoList, LabelMap); }
};

void HLNodeUtils::updateGotos(GotoContainerTy *GotoList, LabelMapTy *LabelMap) {

  for (auto Iter = GotoList->begin(), End = GotoList->end(); Iter != End;
       ++Iter) {
    HLGoto *Goto = *Iter;
    auto LabelIt = LabelMap->find(Goto->getTargetLabel());
    if (LabelIt != LabelMap->end()) {
      // Update the Goto branch to new label
      Goto->setTargetLabel(LabelIt->second);
    }
  }
}

void HLNodeUtils::cloneSequenceImpl(HLContainerTy *CloneContainer,
                                    const HLNode *Node1, const HLNode *Node2) {

  GotoContainerTy GotoList;
  LabelMapTy LabelMap;

  // Check for Node2 as nullptr or a single node
  if (!Node2 || (Node1 == Node2)) {
    CloneContainer->push_back(Node1->cloneImpl(&GotoList, &LabelMap));
    updateGotos(&GotoList, &LabelMap);
    return;
  }

  HLContainerTy::iterator It1(const_cast<HLNode *>(Node1));
  HLContainerTy::iterator It2(const_cast<HLNode *>(Node2));

  HLNodeUtils::CloneVisitor CloneVisit(CloneContainer, &GotoList, &LabelMap);
  visit<HLNodeUtils::CloneVisitor>(&CloneVisit, It1, std::next(It2), false,
                                   true);
  CloneVisit.postVisitUpdate();
}

void HLNodeUtils::cloneSequence(HLContainerTy *CloneContainer,
                                const HLNode *Node1, const HLNode *Node2) {
  assert(Node1 && !isa<HLRegion>(Node1) &&
         " Node1 - Region Cloning is not allowed.");
  assert((!Node2 || !isa<HLRegion>(Node2)) &&
         " Node 2 - Region Cloning is not allowed.");
  assert(CloneContainer && " Clone Container is null.");
  cloneSequenceImpl(CloneContainer, Node1, Node2);
}

void HLNodeUtils::destroy(HLNode *Node) { Node->destroy(); }

void HLNodeUtils::destroyAll() { HLNode::destroyAll(); }

/// \brief Helper for updating loop info during insertion/removal.
///
/// It operates under two modes: finder and updater.
/// Under finder mode, it looks for a loop. This is used by the caller to set
/// the innermost flag during node removal.
/// Under updater mode, it updates nesting level and innermost flag of
/// involved loops. This mode is used during node insertion.
///
struct HLNodeUtils::LoopFinderUpdater {

  bool FinderMode;
  bool FoundLoop;

  LoopFinderUpdater(bool IsFinder) : FinderMode(IsFinder), FoundLoop(false) {}

  bool foundLoop() { return FoundLoop; }

  void visit(HLLoop *Loop) {
    if (FinderMode) {
      FoundLoop = true;
    } else {
      HLNodeUtils::updateLoopInfo(Loop);
    }
  }

  /// Catch-all visit functions
  void visit(HLNode *Node) {}
  void postVisit(HLNode *Node) {}

  bool isDone() {
    if (FinderMode && FoundLoop) {
      return true;
    }
    return false;
  }
};

void HLNodeUtils::updateLoopInfo(HLLoop *Loop) {
  HLLoop *ParentLoop = Loop->getParentLoop();

  if (ParentLoop) {
    Loop->setNestingLevel(ParentLoop->getNestingLevel() + 1);
    ParentLoop->setInnermost(false);
  } else {
    Loop->setNestingLevel(1);
  }
}

void HLNodeUtils::updateLoopInfoRecursively(HLContainerTy::iterator First,
                                            HLContainerTy::iterator Last) {

  HLNodeUtils::LoopFinderUpdater LoopUpdater(false);
  visit<HLNodeUtils::LoopFinderUpdater>(&LoopUpdater, First, Last);
}

void HLNodeUtils::insertInternal(HLContainerTy &InsertContainer,
                                 HLContainerTy::iterator Pos,
                                 HLContainerTy *RemoveContainer,
                                 HLContainerTy::iterator First,
                                 HLContainerTy::iterator Last) {

  if (!RemoveContainer) {
    InsertContainer.insert(Pos, First);
  } else {
    InsertContainer.splice(Pos, *RemoveContainer, First, Last);
  }
}

void HLNodeUtils::insertImpl(HLNode *Parent, HLContainerTy::iterator Pos,
                             HLContainerTy *RemoveContainer,
                             HLContainerTy::iterator First,
                             HLContainerTy::iterator Last, bool UpdateSeparator,
                             bool PostExitSeparator, int CaseNum) {

  assert(!isa<HLRegion>(First) && "Transformations should not add/reorder "
                                  "regions!");

  assert(Parent && "Parent is missing!");
  unsigned I = 0, Distance = 1;

  if (RemoveContainer) {
    Distance = std::distance(First, Last);
  }
#ifndef NDEBUG
  else {
    assert(!First->getParent() &&
           "Node is already linked, please remove it first!!");
  }
#endif

  if (auto Reg = dyn_cast<HLRegion>(Parent)) {
    insertInternal(Reg->Children, Pos, RemoveContainer, First, Last);
  } else if (auto Loop = dyn_cast<HLLoop>(Parent)) {
    insertInternal(Loop->Children, Pos, RemoveContainer, First, Last);

    if (UpdateSeparator) {
      if (Pos == Loop->ChildBegin) {
        Loop->ChildBegin = std::prev(Pos, Distance);
      }
      if (PostExitSeparator && Pos == Loop->PostexitBegin) {
        Loop->PostexitBegin = std::prev(Pos, Distance);
      }
    }

  } else if (auto If = dyn_cast<HLIf>(Parent)) {
    insertInternal(If->Children, Pos, RemoveContainer, First, Last);

    if (UpdateSeparator && (Pos == If->ElseBegin)) {
      If->ElseBegin = std::prev(Pos, Distance);
    }

  } else if (auto Switch = dyn_cast<HLSwitch>(Parent)) {
    insertInternal(Switch->Children, Pos, RemoveContainer, First, Last);

    /// Update all the empty cases which are lexically before this one.
    if (UpdateSeparator) {
      unsigned E = (CaseNum == -1) ? Switch->getNumCases() : CaseNum;
      for (unsigned I = 0; I < E; ++I) {
        if (Pos == Switch->CaseBegin[I]) {
          Switch->CaseBegin[I] = std::prev(Pos, Distance);
        }
      }
    }

  } else {
    llvm_unreachable("Unknown parent type!");
  }

  for (auto It = Pos; I < Distance; ++I, It--) {
    std::prev(It)->setParent(Parent);
  }

  updateLoopInfoRecursively(std::prev(Pos, Distance), Pos);
}

void HLNodeUtils::insertBefore(HLNode *Pos, HLNode *Node) {
  assert(Pos && "Pos is null!");
  assert(Node && "Node is null!");
  insertImpl(Pos->getParent(), Pos, nullptr, Node, Node, true, true);
}

void HLNodeUtils::insertBefore(HLNode *Pos, HLContainerTy *NodeContainer) {
  assert(Pos && "Pos is null!");
  assert(NodeContainer && "NodeContainer is null!");

  insertImpl(Pos->getParent(), Pos, NodeContainer, NodeContainer->begin(),
             NodeContainer->end(), true, true);
}

/// This function doesn't require updating separators as they point to the
/// beginning of a lexical scope and hence will remain unaffected by this
/// operation.
void HLNodeUtils::insertAfter(HLNode *Pos, HLNode *Node) {
  assert(Pos && "Pos is null!");
  assert(Node && "Node is null!");
  HLContainerTy::iterator It(Pos);

  insertImpl(Pos->getParent(), std::next(It), nullptr, Node, Node, false);
}

void HLNodeUtils::insertAfter(HLNode *Pos, HLContainerTy *NodeContainer) {
  assert(Pos && "Pos is null!");
  assert(NodeContainer && "NodeContainer is null!");
  HLContainerTy::iterator It(Pos);

  insertImpl(Pos->getParent(), std::next(It), NodeContainer,
             NodeContainer->begin(), NodeContainer->end(), false);
}

void HLNodeUtils::insertAsChildImpl(HLNode *Parent,
                                    HLContainerTy *RemoveContainer,
                                    HLContainerTy::iterator First,
                                    HLContainerTy::iterator Last,
                                    bool IsFirstChild) {

  if (auto Reg = dyn_cast<HLRegion>(Parent)) {
    insertImpl(Reg, IsFirstChild ? Reg->child_begin() : Reg->child_end(),
               RemoveContainer, First, Last, false);
  } else if (auto Loop = dyn_cast<HLLoop>(Parent)) {
    insertImpl(Loop, IsFirstChild ? Loop->child_begin() : Loop->child_end(),
               RemoveContainer, First, Last, true, false);
  } else {
    llvm_unreachable("Parent can only be region or loop!");
  }
}

void HLNodeUtils::insertAsFirstChild(HLRegion *Reg, HLNode *Node) {
  assert(Node && "Node is null!");
  insertAsChildImpl(Reg, nullptr, Node, Node, true);
}

void HLNodeUtils::insertAsLastChild(HLRegion *Reg, HLNode *Node) {
  assert(Node && "Node is null!");
  insertAsChildImpl(Reg, nullptr, Node, Node, false);
}

void HLNodeUtils::insertAsFirstChild(HLLoop *Loop, HLNode *Node) {
  assert(Node && "Node is null!");
  insertAsChildImpl(Loop, nullptr, Node, Node, true);
}

void HLNodeUtils::insertAsLastChild(HLLoop *Loop, HLNode *Node) {
  assert(Node && "Node is null!");
  insertAsChildImpl(Loop, nullptr, Node, Node, false);
}

void HLNodeUtils::insertAsFirstChild(HLIf *If, HLNode *Node, bool IsThenChild) {
  assert(If && "If is null!");
  assert(Node && "Node is null!");

  insertImpl(If, IsThenChild ? If->then_begin() : If->else_begin(), nullptr,
             Node, Node, !IsThenChild);
}

void HLNodeUtils::insertAsLastChild(HLIf *If, HLNode *Node, bool IsThenChild) {
  assert(If && "If is null!");
  assert(Node && "Node is null!");

  insertImpl(If, IsThenChild ? If->then_end() : If->else_end(), nullptr, Node,
             Node, !IsThenChild);
}

void HLNodeUtils::insertAsChildImpl(HLSwitch *Switch,
                                    HLContainerTy *RemoveContainer,
                                    HLContainerTy::iterator First,
                                    HLContainerTy::iterator Last,
                                    unsigned CaseNum, bool isFirstChild) {
  insertImpl(Switch, isFirstChild ? Switch->case_child_begin_internal(CaseNum)
                                  : Switch->case_child_end_internal(CaseNum),
             RemoveContainer, First, Last, (CaseNum != 0), false, CaseNum);
}

void HLNodeUtils::insertAsFirstDefaultChild(HLSwitch *Switch, HLNode *Node) {
  assert(Node && "Node is null!");
  insertAsChildImpl(Switch, nullptr, Node, Node, 0, true);
}

void HLNodeUtils::insertAsLastDefaultChild(HLSwitch *Switch, HLNode *Node) {
  assert(Node && "Node is null!");
  insertAsChildImpl(Switch, nullptr, Node, Node, 0, false);
}

void HLNodeUtils::insertAsFirstChild(HLSwitch *Switch, HLNode *Node,
                                     unsigned CaseNum) {
  assert(Node && "Node is null!");
  assert((CaseNum > 0) && (CaseNum <= Switch->getNumCases()) &&
         "CaseNum is out of range!");
  insertAsChildImpl(Switch, nullptr, Node, Node, CaseNum, true);
}

void HLNodeUtils::insertAsLastChild(HLSwitch *Switch, HLNode *Node,
                                    unsigned CaseNum) {
  assert(Node && "Node is null!");
  assert((CaseNum > 0) && (CaseNum <= Switch->getNumCases()) &&
         "CaseNum is out of range!");
  insertAsChildImpl(Switch, nullptr, Node, Node, CaseNum, false);
}

void HLNodeUtils::insertAsPreheaderPostexitImpl(
    HLLoop *Loop, HLContainerTy *RemoveContainer, HLContainerTy::iterator First,
    HLContainerTy::iterator Last, bool IsPreheader, bool IsFirstChild) {

  HLContainerTy::iterator Pos;

  Pos = IsPreheader ? (IsFirstChild ? Loop->pre_begin() : Loop->pre_end())
                    : (IsFirstChild ? Loop->post_begin() : Loop->post_end());

  insertImpl(Loop, Pos, RemoveContainer, First, Last, !IsPreheader,
             !IsPreheader);
}

void HLNodeUtils::insertAsFirstPreheaderNode(HLLoop *Loop, HLNode *Node) {
  assert(Node && "Node is null!");
  insertAsPreheaderPostexitImpl(Loop, nullptr, Node, Node, true, true);
}

void HLNodeUtils::insertAsLastPreheaderNode(HLLoop *Loop, HLNode *Node) {
  assert(Node && "Node is null!");
  insertAsPreheaderPostexitImpl(Loop, nullptr, Node, Node, true, false);
}

void HLNodeUtils::insertAsFirstPostexitNode(HLLoop *Loop, HLNode *Node) {
  assert(Node && "Node is null!");
  insertAsPreheaderPostexitImpl(Loop, nullptr, Node, Node, false, true);
}

void HLNodeUtils::insertAsLastPostexitNode(HLLoop *Loop, HLNode *Node) {
  assert(Node && "Node is null!");
  insertAsPreheaderPostexitImpl(Loop, nullptr, Node, Node, false, false);
}

bool HLNodeUtils::foundLoopInRange(HLContainerTy::iterator First,
                                   HLContainerTy::iterator Last) {
  HLNodeUtils::LoopFinderUpdater LoopFinder(true);

  visit<HLNodeUtils::LoopFinderUpdater>(&LoopFinder, First, Last);

  return LoopFinder.foundLoop();
}

void HLNodeUtils::removeInternal(HLContainerTy &Container,
                                 HLContainerTy::iterator First,
                                 HLContainerTy::iterator Last, bool Erase) {
  HLNode *Node;

  for (auto I = First, Next = I, E = Last; I != E; I = Next) {

    Next++;
    Node = Container.remove(I);

    if (Erase) {
      destroy(Node);
    }
#ifndef NDEBUG
    else {
      /// Used to catch errors where user tries to insert an already linked
      /// node.
      Node->setParent(nullptr);
    }
#endif
  }
}

void HLNodeUtils::removeImpl(HLContainerTy::iterator First,
                             HLContainerTy::iterator Last,
                             HLContainerTy *MoveContainer, bool Erase) {

  /// Even if the region becomes empty, we should not remove it. Instead, HIRCG
  /// should short-circult entry/exit bblocks to remove the dead basic blocks.
  assert(!isa<HLRegion>(First) && "Regions cannot be removed!");

  HLNode *Parent;
  HLLoop *ParentLoop;
  HLContainerTy *RemoveContainer;

  /// When removing nodes we might have to set the innermost flag if the
  /// ParentLoop becomes innermost loop. The precise condition where the flag
  /// needs updating is when there is at least one loop in [First, last) and no
  /// loop outside the range and inside ParentLoop.
  bool UpdateInnermostFlag;

  Parent = First->getParent();
  assert(Parent && "Parent is missing!");

  ParentLoop = First->getParentLoop();
  UpdateInnermostFlag = (ParentLoop && foundLoopInRange(First, Last));

  if (auto Reg = dyn_cast<HLRegion>(Parent)) {
    RemoveContainer = &Reg->Children;
  } else if (auto Loop = dyn_cast<HLLoop>(Parent)) {
    RemoveContainer = &Loop->Children;

    if (First == Loop->ChildBegin) {
      Loop->ChildBegin = Last;
    }
    if (First == Loop->PostexitBegin) {
      Loop->PostexitBegin = Last;
    }
  } else if (auto If = dyn_cast<HLIf>(Parent)) {
    RemoveContainer = &If->Children;

    if (First == If->ElseBegin) {
      If->ElseBegin = Last;
    }
  } else if (auto Switch = dyn_cast<HLSwitch>(Parent)) {
    RemoveContainer = &Switch->Children;

    for (unsigned I = 0, E = Switch->getNumCases(); I < E; ++I) {
      if (First == Switch->CaseBegin[I]) {
        Switch->CaseBegin[I] = Last;
      }
    }

  } else {
    llvm_unreachable("Unknown parent type!");
  }

  if (Erase) {
    removeInternal(*RemoveContainer, First, Last, true);
  } else if (!MoveContainer) {
    removeInternal(*RemoveContainer, First, Last, false);
  } else {
    MoveContainer->splice(MoveContainer->end(), *RemoveContainer, First, Last);
  }

  if (UpdateInnermostFlag &&
      !foundLoopInRange(ParentLoop->child_begin(), ParentLoop->child_end())) {
    ParentLoop->setInnermost(true);
  }
}

void HLNodeUtils::remove(HLNode *Node) {
  assert(Node && "Node is null!");

  HLContainerTy::iterator It(Node);
  removeImpl(It, std::next(It), nullptr);
}

void HLNodeUtils::erase(HLContainerTy::iterator First,
                        HLContainerTy::iterator Last) {
  removeImpl(First, Last, nullptr, true);
}

void HLNodeUtils::erase(HLNode *Node) {
  assert(Node && "Node is null!");

  HLContainerTy::iterator It(Node);
  erase(It, std::next(It));
}

void HLNodeUtils::moveBefore(HLNode *Pos, HLContainerTy::iterator First,
                             HLContainerTy::iterator Last) {
  assert(Pos && "Pos is null!");

  HLContainerTy TempContainer;

  removeImpl(First, Last, &TempContainer);
  insertImpl(Pos->getParent(), Pos, &TempContainer, TempContainer.begin(),
             TempContainer.end(), true, true);
}

void HLNodeUtils::moveAfter(HLNode *Pos, HLContainerTy::iterator First,
                            HLContainerTy::iterator Last) {
  assert(Pos && "Pos is null!");

  HLContainerTy TempContainer;
  HLContainerTy::iterator It(Pos);

  removeImpl(First, Last, &TempContainer);
  insertImpl(Pos->getParent(), std::next(It), &TempContainer,
             TempContainer.begin(), TempContainer.end(), false);
}

void HLNodeUtils::moveBefore(HLNode *Pos, HLNode *Node) {
  assert(Node && "Node is null!");
  HLContainerTy::iterator It(Node);

  moveBefore(Pos, It, std::next(It));
}

void HLNodeUtils::moveAfter(HLNode *Pos, HLNode *Node) {
  assert(Node && "Node is null!");
  HLContainerTy::iterator It(Node);

  moveAfter(Pos, It, std::next(It));
}

void HLNodeUtils::moveAsFirstChild(HLRegion *Reg, HLNode *Node) {
  remove(Node);
  insertAsFirstChild(Reg, Node);
}

void HLNodeUtils::moveAsLastChild(HLRegion *Reg, HLNode *Node) {
  remove(Node);
  insertAsLastChild(Reg, Node);
}

void HLNodeUtils::moveAsFirstChild(HLLoop *Loop, HLNode *Node) {
  remove(Node);
  insertAsFirstChild(Loop, Node);
}

void HLNodeUtils::moveAsLastChild(HLLoop *Loop, HLNode *Node) {
  remove(Node);
  insertAsLastChild(Loop, Node);
}

void HLNodeUtils::moveAsFirstChild(HLIf *If, HLNode *Node, bool IsThenChild) {
  remove(Node);
  insertAsFirstChild(If, Node, IsThenChild);
}

void HLNodeUtils::moveAsLastChild(HLIf *If, HLNode *Node, bool IsThenChild) {
  remove(Node);
  insertAsLastChild(If, Node, IsThenChild);
}

void HLNodeUtils::moveAsFirstDefaultChild(HLSwitch *Switch, HLNode *Node) {
  remove(Node);
  insertAsFirstDefaultChild(Switch, Node);
}

void HLNodeUtils::moveAsLastDefaultChild(HLSwitch *Switch, HLNode *Node) {
  remove(Node);
  insertAsLastDefaultChild(Switch, Node);
}

void HLNodeUtils::moveAsFirstChild(HLSwitch *Switch, HLNode *Node,
                                   unsigned CaseNum) {
  remove(Node);
  insertAsFirstChild(Switch, Node, CaseNum);
}

void HLNodeUtils::moveAsLastChild(HLSwitch *Switch, HLNode *Node,
                                  unsigned CaseNum) {
  remove(Node);
  insertAsLastChild(Switch, Node, CaseNum);
}

void HLNodeUtils::moveAsFirstPreheaderNode(HLLoop *Loop, HLNode *Node) {
  remove(Node);
  insertAsFirstPreheaderNode(Loop, Node);
}

void HLNodeUtils::moveAsLastPreheaderNode(HLLoop *Loop, HLNode *Node) {
  remove(Node);
  insertAsLastPreheaderNode(Loop, Node);
}

void HLNodeUtils::moveAsFirstPostexitNode(HLLoop *Loop, HLNode *Node) {
  remove(Node);
  insertAsFirstPostexitNode(Loop, Node);
}

void HLNodeUtils::moveAsLastPostexitNode(HLLoop *Loop, HLNode *Node) {
  remove(Node);
  insertAsLastPostexitNode(Loop, Node);
}

void HLNodeUtils::moveAsFirstChildren(HLRegion *Reg,
                                      HLContainerTy::iterator First,
                                      HLContainerTy::iterator Last) {
  HLContainerTy TempContainer;

  removeImpl(First, Last, &TempContainer);
  insertAsChildImpl(Reg, &TempContainer, TempContainer.begin(),
                    TempContainer.end(), true);
}

void HLNodeUtils::moveAsLastChildren(HLRegion *Reg,
                                     HLContainerTy::iterator First,
                                     HLContainerTy::iterator Last) {
  HLContainerTy TempContainer;

  removeImpl(First, Last, &TempContainer);
  insertAsChildImpl(Reg, &TempContainer, TempContainer.begin(),
                    TempContainer.end(), false);
}

void HLNodeUtils::moveAsFirstChildren(HLLoop *Loop,
                                      HLContainerTy::iterator First,
                                      HLContainerTy::iterator Last) {
  HLContainerTy TempContainer;

  removeImpl(First, Last, &TempContainer);
  insertAsChildImpl(Loop, &TempContainer, TempContainer.begin(),
                    TempContainer.end(), true);
}

void HLNodeUtils::moveAsLastChildren(HLLoop *Loop,
                                     HLContainerTy::iterator First,
                                     HLContainerTy::iterator Last) {
  HLContainerTy TempContainer;

  removeImpl(First, Last, &TempContainer);
  insertAsChildImpl(Loop, &TempContainer, TempContainer.begin(),
                    TempContainer.end(), false);
}

void HLNodeUtils::moveAsFirstChildren(HLIf *If, HLContainerTy::iterator First,
                                      HLContainerTy::iterator Last,
                                      bool IsThenChild) {
  HLContainerTy TempContainer;

  removeImpl(First, Last, &TempContainer);
  insertImpl(If, IsThenChild ? If->then_begin() : If->else_begin(),
             &TempContainer, TempContainer.begin(), TempContainer.end(),
             !IsThenChild);
}

void HLNodeUtils::moveAsLastChildren(HLIf *If, HLContainerTy::iterator First,
                                     HLContainerTy::iterator Last,
                                     bool IsThenChild) {
  HLContainerTy TempContainer;

  removeImpl(First, Last, &TempContainer);
  insertImpl(If, IsThenChild ? If->then_end() : If->else_end(), &TempContainer,
             TempContainer.begin(), TempContainer.end(), !IsThenChild);
}

void HLNodeUtils::moveAsFirstDefaultChildren(HLSwitch *Switch,
                                             HLContainerTy::iterator First,
                                             HLContainerTy::iterator Last) {
  HLContainerTy TempContainer;

  removeImpl(First, Last, &TempContainer);
  insertAsChildImpl(Switch, &TempContainer, TempContainer.begin(),
                    TempContainer.end(), 0, true);
}

void HLNodeUtils::moveAsLastDefaultChildren(HLSwitch *Switch,
                                            HLContainerTy::iterator First,
                                            HLContainerTy::iterator Last) {
  HLContainerTy TempContainer;

  removeImpl(First, Last, &TempContainer);
  insertAsChildImpl(Switch, &TempContainer, TempContainer.begin(),
                    TempContainer.end(), 0, false);
}

void HLNodeUtils::moveAsFirstChildren(HLSwitch *Switch,
                                      HLContainerTy::iterator First,
                                      HLContainerTy::iterator Last,
                                      unsigned CaseNum) {
  assert((CaseNum > 0) && (CaseNum <= Switch->getNumCases()) &&
         "CaseNum is out of range!");
  HLContainerTy TempContainer;

  removeImpl(First, Last, &TempContainer);
  insertAsChildImpl(Switch, &TempContainer, TempContainer.begin(),
                    TempContainer.end(), CaseNum, true);
}

void HLNodeUtils::moveAsLastChildren(HLSwitch *Switch,
                                     HLContainerTy::iterator First,
                                     HLContainerTy::iterator Last,
                                     unsigned CaseNum) {
  assert((CaseNum > 0) && (CaseNum <= Switch->getNumCases()) &&
         "CaseNum is out of range!");
  HLContainerTy TempContainer;

  removeImpl(First, Last, &TempContainer);
  insertAsChildImpl(Switch, &TempContainer, TempContainer.begin(),
                    TempContainer.end(), CaseNum, false);
}

void HLNodeUtils::moveAsFirstPreheaderNodes(HLLoop *Loop,
                                            HLContainerTy::iterator First,
                                            HLContainerTy::iterator Last) {
  HLContainerTy TempContainer;

  removeImpl(First, Last, &TempContainer);
  insertAsPreheaderPostexitImpl(Loop, &TempContainer, TempContainer.begin(),
                                TempContainer.end(), true, true);
}

void HLNodeUtils::moveAsLastPreheaderNodes(HLLoop *Loop,
                                           HLContainerTy::iterator First,
                                           HLContainerTy::iterator Last) {
  HLContainerTy TempContainer;

  removeImpl(First, Last, &TempContainer);
  insertAsPreheaderPostexitImpl(Loop, &TempContainer, TempContainer.begin(),
                                TempContainer.end(), true, false);
}

void HLNodeUtils::moveAsFirstPostexitNodes(HLLoop *Loop,
                                           HLContainerTy::iterator First,
                                           HLContainerTy::iterator Last) {
  HLContainerTy TempContainer;

  removeImpl(First, Last, &TempContainer);
  insertAsPreheaderPostexitImpl(Loop, &TempContainer, TempContainer.begin(),
                                TempContainer.end(), false, true);
}

void HLNodeUtils::moveAsLastPostexitNodes(HLLoop *Loop,
                                          HLContainerTy::iterator First,
                                          HLContainerTy::iterator Last) {
  HLContainerTy TempContainer;

  removeImpl(First, Last, &TempContainer);
  insertAsPreheaderPostexitImpl(Loop, &TempContainer, TempContainer.begin(),
                                TempContainer.end(), false, false);
}

void HLNodeUtils::replace(HLNode *OldNode, HLNode *NewNode) {
  insertBefore(OldNode, NewNode);
  remove(OldNode);
}

HLNode *HLNodeUtils::getLexicalControlFlowSuccessor(HLNode *Node) {
  assert(Node && "Node is null!");
  assert(!isa<HLRegion>(Node) && "Regions don't have control flow successors!");

  HLNode *Succ = nullptr, *TempSucc = nullptr, *Parent = Node->getParent();
  HLContainerTy::iterator Iter(Node);

  // Keep moving up the parent chain till we find a successor.
  while (Parent) {
    if (auto Reg = dyn_cast<HLRegion>(Parent)) {
      if (std::next(Iter) != Reg->Children.end()) {
        Succ = std::next(Iter);
        break;
      }

    } else if (auto If = dyn_cast<HLIf>(Parent)) {
      if (std::next(Iter) != If->Children.end()) {
        TempSucc = std::next(Iter);

        /// Check whether we are crossing separators.
        if ((TempSucc != If->ElseBegin)) {
          Succ = TempSucc;
          break;
        }
      }

    } else if (auto Switch = dyn_cast<HLSwitch>(Parent)) {

      if (std::next(Iter) != Switch->Children.end()) {
        TempSucc = std::next(Iter);

        bool IsSeparator = false;
        /// Check whether we are crossing separators.
        for (unsigned I = 0, E = Switch->getNumCases(); I < E; ++I) {
          if ((TempSucc == Switch->CaseBegin[I])) {
            IsSeparator = true;
            break;
          }
        }

        if (!IsSeparator) {
          Succ = TempSucc;
          break;
        }
      }

    } else {
      llvm_unreachable("Unexpected node parent type!");
    }

    Iter = Parent;
    Parent = Parent->getParent();
  }

  return Succ;
}

struct HLNodeUtils::TopSorter {
  unsigned TopSortNum;
  void visit(HLNode *Node) {
    if (isa<HLRegion>(Node)) {
      TopSortNum = 0;
    }
    TopSortNum += 50;
    Node->setTopSortNum(TopSortNum);
  }

  void postVisit(HLNode *) {}
  bool isDone() { return false; }
  TopSorter() : TopSortNum(0) {}
};

void HLNodeUtils::resetTopSortNum() {
  HLNodeUtils::TopSorter TS;
  HLNodeUtils::visitAll(&TS);
}

const HLLoop *HLNodeUtils::getParentLoopwithLevel(unsigned level,
                                                  const HLLoop *innermostLoop) {
  // level is at least 1
  // parentLoop is immediate parent
  // return nullptr for invalid inputs
  HLLoop *Loop;
  for (Loop = const_cast<HLLoop *>(innermostLoop); Loop != nullptr;
       Loop = Loop->getParentLoop()) {
    if (level == Loop->getNestingLevel()) {
      return Loop;
    }
  }
  return nullptr;
}
