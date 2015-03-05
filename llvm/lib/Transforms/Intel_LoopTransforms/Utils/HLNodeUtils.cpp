//===------- HLNodeUtils.cpp - Implements HLNodeUtils class ------*- C++
//-*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements HLNodeUtils class.
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/Intel_LoopTransforms/Utils/HLNodeUtils.h"

using namespace llvm;
using namespace loopopt;

HLRegion *HLNodeUtils::createHLRegion(std::set<BasicBlock *> &OrigBBs,
                                      BasicBlock *EntryBB, BasicBlock *ExitBB) {
  return new HLRegion(OrigBBs, EntryBB, ExitBB);
}

HLSwitch *HLNodeUtils::createHLSwitch() { return new HLSwitch(); }

HLLabel *HLNodeUtils::createHLLabel(BasicBlock *SrcBB) {
  return new HLLabel(SrcBB);
}

HLGoto *HLNodeUtils::createHLGoto(BasicBlock *TargetBB, HLLabel *TargetL) {
  return new HLGoto(TargetBB, TargetL);
}

HLInst *HLNodeUtils::createHLInst(Instruction *In) { return new HLInst(In); }

HLIf *HLNodeUtils::createHLIf(CmpInst::Predicate FirstPred, DDRef *Ref1,
                              DDRef *Ref2) {
  return new HLIf(FirstPred, Ref1, Ref2);
}

HLLoop *HLNodeUtils::createHLLoop(HLIf *ZttIf, DDRef *LowerDDRef,
                                  DDRef *TripCountDDRef, DDRef *StrideDDRef,
                                  bool isDoWh, unsigned NumEx) {
  return new HLLoop(ZttIf, LowerDDRef, TripCountDDRef, StrideDDRef, isDoWh,
                    NumEx);
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

  void visit(HLRegion *Reg) {
    llvm_unreachable("Found a region inside another!");
  }
  void postVisit(HLRegion *Reg) {
    llvm_unreachable("Found a region inside another!");
  }

  void visit(HLLoop *Loop) {
    if (FinderMode) {
      FoundLoop = true;
    } else {
      HLNodeUtils::updateLoopInfo(Loop);
    }
  }
  void postVisit(HLLoop *Loop) {}

  void visit(HLIf *If) {}
  void postVisit(HLIf *If) {}

  void visit(HLInst *Inst) {}
  void visit(HLLabel *Label) {}
  void visit(HLGoto *Goto) {}
  void visit(HLSwitch *Switch) {}

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
                             bool PostExitSeparator) {

  assert(!isa<HLRegion>(First) && "Transformations should not add/reorder "
                                  "regions!");

  assert(Parent && "Parent is missing!");
  unsigned I = 0, Distance = 1;

  if (RemoveContainer) {
    Distance = std::distance(First, Last);
  }

  if (isa<HLRegion>(Parent)) {
    HLRegion *Reg = cast<HLRegion>(Parent);
    insertInternal(Reg->Children, Pos, RemoveContainer, First, Last);
  } else if (isa<HLLoop>(Parent)) {
    HLLoop *Loop = cast<HLLoop>(Parent);
    insertInternal(Loop->Children, Pos, RemoveContainer, First, Last);

    if (UpdateSeparator) {
      if (Pos == Loop->ChildBegin) {
        Loop->ChildBegin = std::prev(Pos, Distance);
      }
      if (PostExitSeparator && Pos == Loop->PostexitBegin) {
        Loop->PostexitBegin = std::prev(Pos, Distance);
      }
    }

  } else if (isa<HLIf>(Parent)) {
    HLIf *If = cast<HLIf>(Parent);
    insertInternal(If->Children, Pos, RemoveContainer, First, Last);

    if (UpdateSeparator && (Pos == If->ElseBegin)) {
      If->ElseBegin = std::prev(Pos, Distance);
    }
  } else {
    llvm_unreachable("Unknown parent type!");
  }

  for (auto It = Pos; I < Distance; I++, It--) {
    std::prev(It)->setParent(Parent);
  }

  updateLoopInfoRecursively(std::prev(Pos, Distance), Pos);
}

void HLNodeUtils::insertBefore(HLNode *Pos, HLNode *Node) {
  assert(Pos && "Pos is null!");
  assert(Node && "Node is null!");
  insertImpl(Pos->getParent(), Pos, nullptr, Node, Node, true, true);
}

void HLNodeUtils::insertAfter(HLNode *Pos, HLNode *Node) {
  assert(Pos && "Pos is null!");
  assert(Node && "Node is null!");
  HLContainerTy::iterator It(Pos);

  insertImpl(Pos->getParent(), std::next(It), nullptr, Node, Node, false);
}

void HLNodeUtils::insertAsFirstIfChild(HLIf *If, HLNode *Node,
                                       bool IsThenChild) {
  assert(If && "If is null!");
  assert(Node && "Node is null!");

  insertImpl(If, IsThenChild ? If->then_begin() : If->else_begin(), nullptr,
             Node, Node, !IsThenChild);
}

void HLNodeUtils::insertAsLastIfChild(HLIf *If, HLNode *Node,
                                      bool IsThenChild) {
  assert(If && "If is null!");
  assert(Node && "Node is null!");

  insertImpl(If, IsThenChild ? If->then_end() : If->else_end(), nullptr, Node,
             Node, !IsThenChild);
}

void HLNodeUtils::insertAsChildImpl(HLNode *Parent,
                                    HLContainerTy *RemoveContainer,
                                    HLContainerTy::iterator First,
                                    HLContainerTy::iterator Last,
                                    bool IsFirstChild) {

  if (isa<HLRegion>(Parent)) {
    HLRegion *Reg = cast<HLRegion>(Parent);
    insertImpl(Reg, IsFirstChild ? Reg->child_begin() : Reg->child_end(),
               RemoveContainer, First, Last, false);
  } else if (isa<HLLoop>(Parent)) {
    HLLoop *Loop = cast<HLLoop>(Parent);
    insertImpl(Loop, IsFirstChild ? Loop->child_begin() : Loop->child_end(),
               RemoveContainer, First, Last, true, false);
  } else {
    llvm_unreachable("Parent can only be region or loop!");
  }
}

void HLNodeUtils::insertAsFirstChild(HLNode *Parent, HLNode *Node) {
  assert(Node && "Node is null!");
  insertAsChildImpl(Parent, nullptr, Node, Node, true);
}

void HLNodeUtils::insertAsLastChild(HLNode *Parent, HLNode *Node) {
  assert(Node && "Node is null!");
  insertAsChildImpl(Parent, nullptr, Node, Node, false);
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

bool HLNodeUtils::IsInPreheaderPostexitImpl(const HLLoop *Loop,
                                            const HLNode *Node,
                                            bool Preheader) {
  assert(Loop && "Loop is null!");
  assert(Node && "Node is null!");

  /// Shouldn't have any other type here
  if (!isa<HLInst>(Node)) {
    return false;
  }

  if (Node->getParent() != Loop) {
    return false;
  }

  auto I = Preheader ? Loop->pre_begin() : Loop->post_begin();
  auto E = Preheader ? Loop->pre_end() : Loop->post_end();

  for (; I != E; I++) {
    if (cast<HLNode>(I) == Node) {
      return true;
    }
  }

  return false;
}

bool HLNodeUtils::IsInPreheader(const HLLoop *Loop, const HLNode *Node) {
  return IsInPreheaderPostexitImpl(Loop, Node, true);
}

bool HLNodeUtils::IsInPostexit(const HLLoop *Loop, const HLNode *Node) {
  return IsInPreheaderPostexitImpl(Loop, Node, false);
}

bool HLNodeUtils::IsInPreheaderOrPostexit(const HLLoop *Loop,
                                          const HLNode *Node) {
  return (IsInPreheader(Loop, Node) || IsInPostexit(Loop, Node));
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
  }
}

void HLNodeUtils::removeImpl(HLContainerTy::iterator First,
                             HLContainerTy::iterator Last,
                             HLContainerTy *MoveContainer, bool Erase) {

  HLNode *Parent;
  HLLoop *ParentLoop;
  HLContainerTy *RemoveContainer;

  /// When removing nodes we might have to set the innermost flag if the
  /// ParentLoop becomes innermost loop. The precise condition where the flag
  /// needs updating is when there is at least one loop in [First, last) and no
  /// loop outside the range and inside ParentLoop.
  bool UpdateInnermostFlag;

  assert(!isa<HLRegion>(First) && "Use removeRegion() for removing regions!");

  Parent = First->getParent();
  assert(Parent && "Parent is missing!");

  ParentLoop = First->getParentLoop();
  UpdateInnermostFlag = (ParentLoop && foundLoopInRange(First, Last));

  if (isa<HLRegion>(Parent)) {
    HLRegion *Reg = cast<HLRegion>(Parent);
    RemoveContainer = &Reg->Children;
  } else if (isa<HLLoop>(Parent)) {
    HLLoop *Loop = cast<HLLoop>(Parent);
    RemoveContainer = &Loop->Children;

    if (First == Loop->ChildBegin) {
      Loop->ChildBegin = Last;
    } else if (First == Loop->PostexitBegin) {
      Loop->PostexitBegin = Last;
    }
  } else if (isa<HLIf>(Parent)) {
    HLIf *If = cast<HLIf>(Parent);
    RemoveContainer = &If->Children;

    if (First == If->ElseBegin) {
      If->ElseBegin = Last;
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

void HLNodeUtils::moveAsFirstIfChild(HLIf *If, HLNode *Node, bool IsThenChild) {
  remove(Node);
  insertAsFirstIfChild(If, Node, IsThenChild);
}

void HLNodeUtils::moveAsLastIfChild(HLIf *If, HLNode *Node, bool IsThenChild) {
  remove(Node);
  insertAsLastIfChild(If, Node, IsThenChild);
}

void HLNodeUtils::moveAsFirstChild(HLNode *Parent, HLNode *Node) {
  remove(Node);
  insertAsFirstChild(Parent, Node);
}

void HLNodeUtils::moveAsLastChild(HLNode *Parent, HLNode *Node) {
  remove(Node);
  insertAsLastChild(Parent, Node);
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

void HLNodeUtils::moveAsFirstIfChildren(HLIf *If, HLContainerTy::iterator First,
                                        HLContainerTy::iterator Last,
                                        bool IsThenChild) {
  HLContainerTy TempContainer;

  removeImpl(First, Last, &TempContainer);
  insertImpl(If, IsThenChild ? If->then_begin() : If->else_begin(),
             &TempContainer, TempContainer.begin(), TempContainer.end(),
             !IsThenChild);
}

void HLNodeUtils::moveAsLastIfChildren(HLIf *If, HLContainerTy::iterator First,
                                       HLContainerTy::iterator Last,
                                       bool IsThenChild) {
  HLContainerTy TempContainer;

  removeImpl(First, Last, &TempContainer);
  insertImpl(If, IsThenChild ? If->then_end() : If->else_end(), &TempContainer,
             TempContainer.begin(), TempContainer.end(), !IsThenChild);
}

void HLNodeUtils::moveAsFirstChildren(HLNode *Parent,
                                      HLContainerTy::iterator First,
                                      HLContainerTy::iterator Last) {
  HLContainerTy TempContainer;

  removeImpl(First, Last, &TempContainer);
  insertAsChildImpl(Parent, &TempContainer, TempContainer.begin(),
                    TempContainer.end(), true);
}

void HLNodeUtils::moveAsLastChildren(HLNode *Parent,
                                     HLContainerTy::iterator First,
                                     HLContainerTy::iterator Last) {
  HLContainerTy TempContainer;

  removeImpl(First, Last, &TempContainer);
  insertAsChildImpl(Parent, &TempContainer, TempContainer.begin(),
                    TempContainer.end(), false);
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
