//===--- VPOAvrUtils.cpp ----------------------------------------*- C++ -*-===//
//
//   Copyright (C) 2015-2016 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation. and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file implements the Abstract Vector Representation (AVR)
/// utilities.
///
//===----------------------------------------------------------------------===//

#include "llvm/Analysis/Intel_VPO/Vecopt/VPOAvrUtils.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"

#define DEBUG_TYPE "avr-utilities"

using namespace llvm;
using namespace llvm::vpo;

// AVR Creation Utilities

AVRFunction *AVRUtils::createAVRFunction(Function *OrigF,
                                         const LoopInfo *LpInfo) {
  return new AVRFunction(OrigF, LpInfo);
}

AVRWrn *AVRUtils::createAVRWrn(WRNVecLoopNode *WrnSimdNode) {
  // TODO - for now we create an AVRWrn for every HIR region node by
  // passing a null WrnSimdNode. This assert is suppressed until we
  // figure how WRN analysis works with HIR.
  // assert(WrnSimdNode && "WrnSimdNode is empty!");

  return new AVRWrn(WrnSimdNode);
}

AVRBranch *AVRUtils::createAVRBranch(AVRLabel *Successor) {
  return new AVRBranch(Successor);
}

AVRNOP *AVRUtils::createAVRNOP() { return new AVRNOP(); }

// Modification Utilities

void AVRUtils::setAVRAssignLHS(AVRAssign *AvrAssign, AVR *Node) {
  AvrAssign->setLHS(Node);
}

void AVRUtils::setAVRAssignRHS(AVRAssign *AvrAssign, AVR *Node) {
  AvrAssign->setRHS(Node);
}

void AVRUtils::setAVRPhiLHS(AVRPhi *APhi, AVRValue *AValue) {
  APhi->setLHS(AValue);
}

void AVRUtils::addAVRPhiIncoming(AVRPhi *APhi, AVRValue *AValue,
                                 AVRLabel *ALabel) {
  APhi->addIncoming(AValue, ALabel);
}

void AVRUtils::addCase(AVRSwitch *ASwitch) { ASwitch->addCase(); }

void AVRUtils::setZeroTripTest(AVRLoop *ALoop, AVRIf *IfZtt) {
  llvm_unreachable("Zero trip test support not implemented yet!");
  ALoop->setZeroTripTest(IfZtt);
}

void AVRUtils::setSLEV(AVR *Avr, const SLEV& Slev) {
  Avr->Slev.copyValue(Slev);
}

// Insertion Utilities

void AVRUtils::insertSingleton(AVR *DestinationParent,
                               AVRContainerTy &DestinationContainer,
                               AvrItr InsertionPosition, AVR *NewNode) {

  assert(DestinationParent && "Parent node is null!");
  NewNode->setParent(DestinationParent);
  DestinationContainer.insert(InsertionPosition, NewNode);
}

void AVRUtils::insertSequence(AVR *DestinationParent,
                              AVRContainerTy &DestinationContainer,
                              AVRContainerTy *SourceContainer,
                              AvrItr InsertionPosition) {

  AvrItr SourceBegin = SourceContainer->begin();
  AvrItr SourceEnd = SourceContainer->end();
  unsigned Distance = std::distance(SourceBegin, SourceEnd), I = 0;

  DestinationContainer.splice(InsertionPosition, *SourceContainer, SourceBegin,
                              SourceEnd);

  // Update the parents of top-most nodes.
  for (auto Itr = InsertionPosition; I < Distance; ++I, Itr--) {
    std::prev(Itr)->setParent(DestinationParent);
  }
}

void AVRUtils::insertAfter(AvrItr InsertionPosition, AVR *Node) {
  AVR *Parent = InsertionPosition->getParent();
  AVRContainerTy *Children = getChildrenContainer(Parent, InsertionPosition);
  insertSingleton(Parent, *Children, std::next(InsertionPosition), Node);
}

void AVRUtils::insertBefore(AvrItr InsertionPosition, AVR *Node) {
  AVR *Parent = InsertionPosition->getParent();
  AVRContainerTy *Children = getChildrenContainer(Parent, InsertionPosition);
  insertSingleton(Parent, *Children, InsertionPosition, Node);
}

void AVRUtils::insertAfter(AvrItr InsertionPosition,
                           AVRContainerTy *NodeContainer) {
  AVR *Parent = InsertionPosition->getParent();
  AVRContainerTy *Children = getChildrenContainer(Parent, InsertionPosition);
  insertSequence(Parent, *Children, NodeContainer,
                 std::next(InsertionPosition));
}

void AVRUtils::insertBefore(AvrItr InsertionPosition,
                            AVRContainerTy *NodeContainer) {
  AVR *Parent = InsertionPosition->getParent();
  AVRContainerTy *Children = getChildrenContainer(Parent, InsertionPosition);
  insertSequence(Parent, *Children, NodeContainer, InsertionPosition);
}

void AVRUtils::insertAfter(AvrItr InsertionPosition,
                           AVRContainerTy &DestinationContainer, AVR *Node) {
  AVR *Parent = InsertionPosition->getParent();
  insertSingleton(Parent, DestinationContainer, std::next(InsertionPosition),
                  Node);
}

void AVRUtils::insertBefore(AvrItr InsertionPosition,
                            AVRContainerTy &DestinationContainer, AVR *Node) {
  AVR *Parent = InsertionPosition->getParent();
  insertSingleton(Parent, DestinationContainer, InsertionPosition, Node);
}

void AVRUtils::insertAfter(AvrItr InsertionPosition,
                           AVRContainerTy &DestinationContainer,
                           AVRContainerTy *SourceContainer) {
  AVR *Parent = InsertionPosition->getParent();
  insertSequence(Parent, DestinationContainer, SourceContainer,
                 std::next(InsertionPosition));
}

void AVRUtils::insertBefore(AvrItr InsertionPosition,
                            AVRContainerTy &DestinationContainer,
                            AVRContainerTy *SourceContainer) {
  AVR *Parent = InsertionPosition->getParent();
  insertSequence(Parent, DestinationContainer, SourceContainer,
                 InsertionPosition);
}

void AVRUtils::insertFirstChild(AVR *Parent, AVR *Node) {
  if (AVRLoop *LpNode = dyn_cast<AVRLoop>(Parent))
    return insertFirstChild(LpNode, Node);
  else if (AVRFunction *FunNode = dyn_cast<AVRFunction>(Parent))
    return insertFirstChild(FunNode, Node);
  else if (AVRWrn *WrNode = dyn_cast<AVRWrn>(Parent))
    return insertFirstChild(WrNode, Node);
  llvm_unreachable("Unable to resolve Parent node!");
}

void AVRUtils::insertLastChild(AVR *Parent, AVR *Node) {
  if (AVRLoop *LpNode = dyn_cast<AVRLoop>(Parent))
    return insertLastChild(LpNode, Node);
  else if (AVRFunction *FunNode = dyn_cast<AVRFunction>(Parent))
    return insertLastChild(FunNode, Node);
  else if (AVRWrn *WrNode = dyn_cast<AVRWrn>(Parent))
    return insertLastChild(WrNode, Node);
  llvm_unreachable("Unable to resolve Parent node!");
}

void AVRUtils::insertFirstChild(AVRLoop *Parent, AVR *Node) {
  insertSingleton(Parent, Parent->Children, Parent->child_begin(), Node);
}

void AVRUtils::insertLastChild(AVRLoop *Parent, AVR *Node) {
  insertSingleton(Parent, Parent->Children, Parent->child_end(), Node);
}

void AVRUtils::insertFirstChildren(AVRLoop *Parent,
                                   AVRContainerTy *NodeContainer) {
  insertSequence(Parent, Parent->Children, NodeContainer,
                 Parent->child_begin());
}

void AVRUtils::insertLastChildren(AVRLoop *Parent,
                                  AVRContainerTy *NodeContainer) {
  insertSequence(Parent, Parent->Children, NodeContainer, Parent->child_end());
}

void AVRUtils::insertFirstChild(AVRFunction *Parent, AVR *Node) {
  insertSingleton(Parent, Parent->Children, Parent->child_begin(), Node);
}

void AVRUtils::insertLastChild(AVRFunction *Parent, AVR *Node) {
  insertSingleton(Parent, Parent->Children, Parent->child_end(), Node);
}

void AVRUtils::insertFirstChildren(AVRFunction *Parent,
                                   AVRContainerTy *NodeContainer) {
  insertSequence(Parent, Parent->Children, NodeContainer,
                 Parent->child_begin());
}

void AVRUtils::insertLastChildren(AVRFunction *Parent,
                                  AVRContainerTy *NodeContainer) {
  insertSequence(Parent, Parent->Children, NodeContainer, Parent->child_end());
}

void AVRUtils::insertFirstChild(AVRWrn *Parent, AVR *Node) {
  insertSingleton(Parent, Parent->Children, Parent->child_begin(), Node);
}

void AVRUtils::insertLastChild(AVRWrn *Parent, AVR *Node) {
  insertSingleton(Parent, Parent->Children, Parent->child_end(), Node);
}

void AVRUtils::insertFirstChildren(AVRWrn *Parent,
                                   AVRContainerTy *NodeContainer) {
  insertSequence(Parent, Parent->Children, NodeContainer,
                 Parent->child_begin());
}

void AVRUtils::insertLastChildren(AVRWrn *Parent,
                                  AVRContainerTy *NodeContainer) {
  insertSequence(Parent, Parent->Children, NodeContainer, Parent->child_end());
}

void AVRUtils::insertFirstThenChild(AVRIf *Parent, AVR *Node) {
  insertSingleton(Parent, Parent->ThenChildren, Parent->then_begin(), Node);
}

void AVRUtils::insertLastThenChild(AVRIf *Parent, AVR *Node) {
  insertSingleton(Parent, Parent->ThenChildren, Parent->then_end(), Node);
}

void AVRUtils::insertFirstThenChildren(AVRIf *Parent,
                                       AVRContainerTy *NodeContainer) {
  insertSequence(Parent, Parent->ThenChildren, NodeContainer,
                 Parent->then_begin());
}

void AVRUtils::insertLastThenChildren(AVRIf *Parent,
                                      AVRContainerTy *NodeContainer) {
  insertSequence(Parent, Parent->ThenChildren, NodeContainer,
                 Parent->then_end());
}

void AVRUtils::insertFirstElseChild(AVRIf *Parent, AVR *Node) {
  insertSingleton(Parent, Parent->ElseChildren, Parent->else_begin(), Node);
}

void AVRUtils::insertLastElseChild(AVRIf *Parent, AVR *Node) {
  insertSingleton(Parent, Parent->ElseChildren, Parent->else_end(), Node);
}

void AVRUtils::insertFirstElseChildren(AVRIf *Parent,
                                       AVRContainerTy *NodeContainer) {
  insertSequence(Parent, Parent->ElseChildren, NodeContainer,
                 Parent->else_begin());
}

void AVRUtils::insertLastElseChildren(AVRIf *Parent,
                                      AVRContainerTy *NodeContainer) {
  insertSequence(Parent, Parent->ElseChildren, NodeContainer,
                 Parent->else_end());
}

void AVRUtils::insertFirstDefaultChild(AVRSwitch *Parent, AVR *Node) {
  insertSingleton(Parent, Parent->Children, Parent->default_case_child_begin(),
                  Node);
}

void AVRUtils::insertLastDefaultChild(AVRSwitch *Parent, AVR *Node) {
  insertSingleton(Parent, Parent->Children, Parent->default_case_child_end(),
                  Node);
}

void AVRUtils::insertFirstDefaultChildren(AVRSwitch *Parent,
                                          AVRContainerTy *NodeContainer) {
  insertSequence(Parent, Parent->Children, NodeContainer,
                 Parent->default_case_child_begin());
}

void AVRUtils::insertLastDefaultChildren(AVRSwitch *Parent,
                                         AVRContainerTy *NodeContainer) {
  insertSequence(Parent, Parent->Children, NodeContainer,
                 Parent->default_case_child_end());
}

void AVRUtils::insertFirstChild(AVRSwitch *Parent, AVR *Node,
                                unsigned CaseNum) {

  assert((CaseNum <= Parent->getNumCases()) && "Switch case is out of range!");
  AvrItr InsertionPosition = Parent->case_child_begin(CaseNum);
  insertSingleton(Parent, Parent->Children, InsertionPosition, Node);

  // Because the switch node uses a single linked-list container to store
  // all the cases of the switch, we must update internal seperators which
  // keep track of where each case begins in the linked-list.
  Parent->CaseBegin[CaseNum - 1] = std::prev(InsertionPosition);
}

void AVRUtils::insertLastChild(AVRSwitch *Parent, AVR *Node, unsigned CaseNum) {

  assert((CaseNum <= Parent->getNumCases()) && "Switch case is out of range!");
  insertSingleton(Parent, Parent->Children, Parent->case_child_end(CaseNum),
                  Node);
}

void AVRUtils::insertFirstChildren(AVRSwitch *Parent,
                                   AVRContainerTy *NodeContainer,
                                   unsigned CaseNum) {

  assert((CaseNum <= Parent->getNumCases()) && "Switch case is out of range!");
  AvrItr InsertionPosition = Parent->case_child_begin(CaseNum);
  insertSequence(Parent, Parent->Children, NodeContainer, InsertionPosition);

  // Because the switch node uses a single linked-list container to store
  // all the cases of the switch, we must update internal seperators which
  // keep track of where each case begins in the linked-list.
  Parent->CaseBegin[CaseNum - 1] = std::prev(InsertionPosition);
}

void AVRUtils::insertLastChildren(AVRSwitch *Parent,
                                  AVRContainerTy *NodeContainer,
                                  unsigned CaseNum) {
  assert((CaseNum <= Parent->getNumCases()) && "Switch case is out of range!");
  insertSequence(Parent, Parent->Children, NodeContainer,
                 Parent->case_child_end(CaseNum));
}

void AVRUtils::insertFirstPreheaderChild(AVRLoop *Parent, AVR *Node) {
  insertSingleton(Parent, Parent->PreheaderChildren, Parent->pre_begin(), Node);
}

void AVRUtils::insertLastPreheaderChild(AVRLoop *Parent, AVR *Node) {
  insertSingleton(Parent, Parent->PreheaderChildren, Parent->pre_end(), Node);
}

void AVRUtils::insertFirstPreheaderChildren(AVRLoop *Parent,
                                            AVRContainerTy *NodeContainer) {
  insertSequence(Parent, Parent->PreheaderChildren, NodeContainer,
                 Parent->pre_begin());
}

void AVRUtils::insertLastPreheaderChildren(AVRLoop *Parent,
                                           AVRContainerTy *NodeContainer) {
  insertSequence(Parent, Parent->PreheaderChildren, NodeContainer,
                 Parent->pre_end());
}

void AVRUtils::insertFirstPostexitChild(AVRLoop *Parent, AVR *Node) {
  insertSingleton(Parent, Parent->PostexitChildren, Parent->post_begin(), Node);
}

void AVRUtils::insertLastPostexitChild(AVRLoop *Parent, AVR *Node) {
  insertSingleton(Parent, Parent->PostexitChildren, Parent->post_end(), Node);
}

void AVRUtils::insertFirstPostexitChildren(AVRLoop *Parent,
                                           AVRContainerTy *NodeContainer) {
  insertSequence(Parent, Parent->PostexitChildren, NodeContainer,
                 Parent->post_begin());
}

void AVRUtils::insertLastPostexitChildren(AVRLoop *Parent,
                                          AVRContainerTy *NodeContainer) {
  insertSequence(Parent, Parent->PostexitChildren, NodeContainer,
                 Parent->post_end());
}

// Move Utilities

void AVRUtils::moveAfter(AvrItr InsertionPos, AVR *Node) {
  remove(Node);
  insertAfter(InsertionPos, Node);
}

void AVRUtils::moveAsFirstChildren(AVRLoop *ALoop, AvrItr First, AvrItr Last) {

  AVRContainerTy TempContainer;
  AVR *Begin = &*First, *End = &*Last;
  AvrItr InsertionPosition = ALoop->child_begin();

  if (First->getParent() != Last->getParent()) {

    if (!resolveCommonLexicalParent(Begin, &End))
      llvm_unreachable(
          "Coudln't resolve common lexical parent for avr sequence!");
  }

  removeInternal(AvrItr(Begin), AvrItr(End), &TempContainer, false);
  insertSequence(ALoop, ALoop->Children, &TempContainer, InsertionPosition);
}

void AVRUtils::moveAsFirstChildren(AVRSwitch *ASwitch, AvrItr First,
                                   AvrItr Last, unsigned CaseNumber) {

  AVRContainerTy TempContainer;
  AVR *Begin = &*First, *End = &*Last;
  AvrItr InsertionPosition = ASwitch->case_child_begin(CaseNumber);

  if (First->getParent() != Last->getParent()) {

    if (!resolveCommonLexicalParent(Begin, &End))
      llvm_unreachable(
          "Coudln't resolve common lexical parent for avr sequence!");
  }

  removeInternal(AvrItr(Begin), AvrItr(End), &TempContainer, false);
  insertSequence(ASwitch, ASwitch->Children, &TempContainer, InsertionPosition);
}

void AVRUtils::moveAsLastChildren(AVRLoop *ALoop, AvrItr First, AvrItr Last) {

  AVRContainerTy TempContainer;
  AVR *Begin = &*First, *End = &*Last;
  AvrItr InsertionPosition = ALoop->child_end();

  if (First->getParent() != Last->getParent()) {

    if (!resolveCommonLexicalParent(Begin, &End))
      llvm_unreachable(
          "Coudln't resolve common lexical parent for avr sequence!");
  }

  removeInternal(AvrItr(Begin), AvrItr(End), &TempContainer, false);
  insertSequence(ALoop, ALoop->Children, &TempContainer, InsertionPosition);
}

void AVRUtils::moveAsLastChildren(AVRSwitch *ASwitch, AvrItr First, AvrItr Last,
                                  unsigned CaseNumber) {
  AVRContainerTy TempContainer;
  AVR *Begin = &*First, *End = &*Last;
  AvrItr InsertionPosition = ASwitch->case_child_end(CaseNumber);

  if (First->getParent() != Last->getParent()) {

    if (!resolveCommonLexicalParent(Begin, &End))
      llvm_unreachable(
          "Coudln't resolve common lexical parent for avr sequence!");
  }

  removeInternal(AvrItr(Begin), AvrItr(End), &TempContainer, false);
  insertSequence(ASwitch, ASwitch->Children, &TempContainer, InsertionPosition);
}

void AVRUtils::moveAsFirstThenChildren(AVRIf *AIf, AvrItr First, AvrItr Last) {

  assert(AIf && "Missing AvrIf for insertion!");

  AVRContainerTy TempContainer;
  AVR *Begin = &*First, *End = &*Last;
  AvrItr InsertionPosition = AIf->ThenChildren.begin();

  if (First->getParent() != Last->getParent()) {

    if (!resolveCommonLexicalParent(Begin, &End))
      llvm_unreachable(
          "Couldn't resolve common lexical parent for avr sequence!");
  }

  removeInternal(AvrItr(Begin), AvrItr(End), &TempContainer, false);
  insertSequence(AIf, AIf->ThenChildren, &TempContainer, InsertionPosition);
}

void AVRUtils::moveAsFirstElseChildren(AVRIf *AIf, AvrItr First, AvrItr Last) {

  assert(AIf && "Missing AvrIf for insertion!");

  AVRContainerTy TempContainer;
  AVR *Begin = &*First, *End = &*Last;
  AvrItr InsertionPosition = AIf->ElseChildren.begin();

  if (First->getParent() != Last->getParent()) {

    if (!resolveCommonLexicalParent(Begin, &End))
      llvm_unreachable(
          "Coudlnt resolve common lexical parent for avr sequence!");
  }

  removeInternal(AvrItr(Begin), AvrItr(End), &TempContainer, false);
  insertSequence(AIf, AIf->ElseChildren, &TempContainer, InsertionPosition);
}

void AVRUtils::moveAsFirstDefaultChildren(AVRSwitch *ASwitch, AvrItr First,
                                          AvrItr Last) {
  AVRContainerTy TempContainer;
  AVR *Begin = &*First, *End = &*Last;
  AvrItr InsertionPosition = ASwitch->default_case_child_begin();

  if (First->getParent() != Last->getParent()) {

    if (!resolveCommonLexicalParent(Begin, &End))
      llvm_unreachable(
          "Coudln't resolve common lexical parent for avr sequence!");
  }

  removeInternal(AvrItr(Begin), AvrItr(End), &TempContainer, false);
  insertSequence(ASwitch, ASwitch->Children, &TempContainer, InsertionPosition);
}

void AVRUtils::moveAsLastDefaultChildren(AVRSwitch *ASwitch, AvrItr First,
                                         AvrItr Last) {
  AVRContainerTy TempContainer;
  AVR *Begin = &*First, *End = &*Last;
  AvrItr InsertionPosition = ASwitch->default_case_child_end();

  if (First->getParent() != Last->getParent()) {

    if (!resolveCommonLexicalParent(Begin, &End))
      llvm_unreachable(
          "Coudln't resolve common lexical parent for avr sequence!");
  }

  removeInternal(AvrItr(Begin), AvrItr(End), &TempContainer, false);
  insertSequence(ASwitch, ASwitch->Children, &TempContainer, InsertionPosition);
}

void AVRUtils::moveAsFirstPreheaderChildren(AVRLoop *ALoop, AvrItr First,
                                            AvrItr Last) {
  AVRContainerTy TempContainer;
  AVR *Begin = &*First, *End = &*Last;
  AvrItr InsertionPosition = ALoop->pre_begin();

  if (First->getParent() != Last->getParent()) {

    if (!resolveCommonLexicalParent(Begin, &End))
      llvm_unreachable(
          "Coudln't resolve common lexical parent for avr sequence!");
  }

  removeInternal(AvrItr(Begin), AvrItr(End), &TempContainer, false);
  insertSequence(ALoop, ALoop->Children, &TempContainer, InsertionPosition);
}

void AVRUtils::moveAsFirstPostexitChildren(AVRLoop *ALoop, AvrItr First,
                                           AvrItr Last) {
  AVRContainerTy TempContainer;
  AVR *Begin = &*First, *End = &*Last;
  AvrItr InsertionPosition = ALoop->post_begin();

  if (First->getParent() != Last->getParent()) {

    if (!resolveCommonLexicalParent(Begin, &End))
      llvm_unreachable(
          "Coudln't resolve common lexical parent for avr sequence!");
  }

  removeInternal(AvrItr(Begin), AvrItr(End), &TempContainer, false);
  insertSequence(ALoop, ALoop->Children, &TempContainer, InsertionPosition);
}

void AVRUtils::moveAsLastPreheaderChildren(AVRLoop *ALoop, AvrItr First,
                                           AvrItr Last) {
  AVRContainerTy TempContainer;
  AVR *Begin = &*First, *End = &*Last;
  AvrItr InsertionPosition = ALoop->pre_end();

  if (First->getParent() != Last->getParent()) {

    if (!resolveCommonLexicalParent(Begin, &End))
      llvm_unreachable(
          "Coudln't resolve common lexical parent for avr sequence!");
  }

  removeInternal(AvrItr(Begin), AvrItr(End), &TempContainer, false);
  insertSequence(ALoop, ALoop->Children, &TempContainer, InsertionPosition);
}

void AVRUtils::moveAsLastPostexitChildren(AVRLoop *ALoop, AvrItr First,
                                          AvrItr Last) {
  AVRContainerTy TempContainer;
  AVR *Begin = &*First, *End = &*Last;
  AvrItr InsertionPosition = ALoop->post_end();

  if (First->getParent() != Last->getParent()) {

    if (!resolveCommonLexicalParent(Begin, &End))
      llvm_unreachable(
          "Coudln't resolve common lexical parent for avr sequence!");
  }

  removeInternal(AvrItr(Begin), AvrItr(End), &TempContainer, false);
  insertSequence(ALoop, ALoop->Children, &TempContainer, InsertionPosition);
}

// Removal Utilities

void AVRUtils::destroy(AVR *Avr) { Avr->destroy(); }

AVRContainerTy *AVRUtils::removeInternal(AvrItr Begin, AvrItr End,
                                         AVRContainerTy *MoveContainer,
                                         bool Delete) {

  // Find the current container which holds Node.
  AVRContainerTy *OrigContainer = AVRUtils::getAvrContainer(&*Begin);
  assert(OrigContainer && "Container missing for node removal!");

  // Removal of Avr or Avr sequence doenst require move to new location.
  if (!MoveContainer) {

    // Remove Singleton
    if (Begin == End) {

      // Removing avr nodes with switch parents requires the updating of
      // internal separators used to specify switch cases within the
      // children container.
      if (AVRSwitch *ASwitch = dyn_cast<AVRSwitch>(Begin->getParent())) {
        for (unsigned Itr = 0, End = ASwitch->getNumCases(); Itr < End; ++Itr) {

          if (ASwitch->CaseBegin[Itr] == Begin) {
            ASwitch->CaseBegin[Itr] = std::next(Begin);
            break;
          }
        }
      }

      OrigContainer->remove(Begin);

      if (Delete)
        destroy(&*Begin);

      return nullptr;
    }

    // Remove Sequence
    for (auto I = Begin, Next = I, E = End; I != E; I = Next) {

      Next++;
      AVR *Node = OrigContainer->remove(I);

      if (Delete)
        destroy(Node);
    }
  } else {
    MoveContainer->splice(MoveContainer->end(), *OrigContainer, Begin, ++End);
  }

  return MoveContainer;
}

// Remove singleton AVR
void AVRUtils::remove(AVR *Node) {

  assert(Node && "Missing AVR Node!");
  removeInternal(AvrItr(Node), AvrItr(Node), nullptr, false);
}

// Remove sequence of AVRs
void AVRUtils::remove(AvrItr Begin, AvrItr End) {

  assert(Begin->getParent() == End->getParent() &&
         "Candidate avr move sequence do not share common parent!");

  removeInternal(Begin, End, nullptr, false);
}

// Search Utilities

bool AVRUtils::containsAvr(AVRContainerTy &Children, AVR *Node) {

  assert(Node && "Avr misssing for child search!");

  for (auto I = Children.begin(), E = Children.end(); I != E; ++I) {

    if (&(*I) == Node) {
      return true;
    }
  }
  return false;
}

AVRContainerTy *AVRUtils::getAvrContainer(AVR *Node) {

  AVR *Parent = Node->getParent();
  assert(Parent && "Avr node missing parent!");

  if (AVRFunction *AFunc = dyn_cast<AVRFunction>(Parent)) {
    return &(AFunc->Children);
  }

  if (AVRLoop *ALoop = dyn_cast<AVRLoop>(Parent)) {
    return &(ALoop->Children);
  }

  if (AVRIf *AIf = dyn_cast<AVRIf>(Parent)) {

    if (AVRUtils::containsAvr(AIf->ThenChildren, Node)) {
      return &(AIf->ThenChildren);
    }

    if (AVRUtils::containsAvr(AIf->ElseChildren, Node)) {
      return &(AIf->ElseChildren);
    }
  } else if (AVRWrn *AWrn = dyn_cast<AVRWrn>(Parent)) {
    return &(AWrn->Children);
  } else if (AVRSwitch *ASwitch = dyn_cast<AVRSwitch>(Parent)) {
    return &(ASwitch->Children);
  }

  llvm_unreachable("VPO: Avr node missing parent container!");
}

// Helper Functions

bool AVRUtils::resolveCommonLexicalParent(AVR *First, AVR **Last) {

  // We only traverse up the lexical tree for Last. The thought is that
  // Last could be the last avr of some child container and that the
  // parent of this child container is at the same lexical nesting level
  // of First. This function will update Last to point to that parent.

  AVR *TargetParent = First->getParent();
  AVR *TestParent = (*Last)->getParent();
  AVR *NewLast = *Last;

  assert(TargetParent && "Couldn't resolve First's parent node!");
  assert(TestParent && "Couldn't resolve Last's parent node!");

  do {

    if (TargetParent == TestParent) {
      *Last = NewLast;
      return true;
    }

    NewLast = TestParent;
    TestParent = TestParent->getParent();

  } while (TestParent);

  return false;
}

AVRContainerTy *AVRUtils::getChildrenContainer(AVR *Parent, AvrItr Child) {

  // This implementation is costly. Callers of this function are corner cases
  // that should be rarely encountered. This function should not be called
  // as a generic utility.

  // TODO: Unify the children containers of IF, LOOP parent nodes to eliminate
  // the search overhead.

  AVR *Node = &(*Child);

  if (AVRFunction *FuncNode = dyn_cast<AVRFunction>(Parent)) {
    return &FuncNode->Children;
  } else if (AVRWrn *WrnNode = dyn_cast<AVRWrn>(Parent)) {
    return &WrnNode->Children;
  } else if (AVRSwitch *SwitchNode = dyn_cast<AVRSwitch>(Parent)) {
    return &SwitchNode->Children;
  } else if (AVRIf *IfNode = dyn_cast<AVRIf>(Parent)) {

    if (IfNode->isThenChild(Node))
      return &IfNode->ThenChildren;
    else if (IfNode->isElseChild(Node))
      return &IfNode->ElseChildren;

    llvm_unreachable("Vector graph node parent unresolvable!");
  } else if (AVRLoop *LoopNode = dyn_cast<AVRLoop>(Parent)) {

    if (LoopNode->isPreheaderChild(Node))
      return &LoopNode->PreheaderChildren;
    else if (LoopNode->isChild(Node))
      return &LoopNode->Children;
    else if (LoopNode->isPostexitChild(Node))
      return &LoopNode->PostexitChildren;

    llvm_unreachable("Vector graph node parent unresolvable!");
  }

  llvm_unreachable("Unknown vector graph node parent!");
  return nullptr;
}
