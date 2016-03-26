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

AVRLoop *AVRUtils::createAVRLoop(const Loop *Lp) { return new AVRLoop(Lp); }

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

AVRNOP *AVRUtils::createAVRNOP() {
  return new AVRNOP();
}

// Modification Utilities

void AVRUtils::setAVRAssignLHS(AVRAssign *AvrAssign, AVR *Node) {
  AvrAssign->setLHS(Node);
}

void AVRUtils::setAVRAssignRHS(AVRAssign *AvrAssign, AVR *Node) {
  AvrAssign->setRHS(Node);
}

void AVRUtils::addCase(AVRSwitch *ASwitch) {
  ASwitch->addCase();
}

// Insertion Utilities

void AVRUtils::insertAVR(AVR *Parent, AvrItr Pos, AvrItr NewAvr,
                         InsertType Itype, SplitType SType, 
                         unsigned CaseNumber) {

  assert(Parent && "Parent is null.");

  AvrItr InsertionPos;
  AVRContainerTy *Children = nullptr;
  AVRSwitch *ASwitch = nullptr;

  if (AVRFunction *AFunc = dyn_cast<AVRFunction>(Parent)) {
    Children = &(AFunc->Children);
  } else if (AVRLoop *ALoop = dyn_cast<AVRLoop>(Parent)) {
    Children = &(ALoop->Children);
  } else if (AVRIf *AIf = dyn_cast<AVRIf>(Parent)) {

    if (SType == ThenChild) {
      Children = &(AIf->ThenChildren);
    } else if (SType == ElseChild) {
      Children = &(AIf->ElseChildren);
    } else {
      // It's possible that insert is called with an AvrIf parent and unknown
      // child container. Resolve children container with a quick look up.
      if (AVRUtils::containsAvr(AIf->ThenChildren, &*Pos)) {
        Children = &(AIf->ThenChildren);
      } else if (AVRUtils::containsAvr(AIf->ElseChildren, &*Pos)) {
        Children = &(AIf->ElseChildren);
      } else {
        llvm_unreachable("Malformed AVRIf insertion!");
      }
    }
  } else if (AVRWrn *AWrn = dyn_cast<AVRWrn>(Parent)) {
    Children = &(AWrn->Children);
  } else if (isa<AVRSwitch>(Parent)) {
    ASwitch = cast<AVRSwitch>(Parent);

    assert((SType == None) && " Invalid insertion into switch avr node!");
    assert((CaseNumber <= ASwitch->getNumCases()) &&
           "Switch case number is out of range!");

    Children = &(ASwitch->Children);
  } else {
    llvm_unreachable("VPO: Unsupported AVR Insertion\n");
  }

  assert(Children && "Children container ptr is null.");

  // Set insertion point
  switch (Itype) {
    case FirstChild:
      // FirstChild insertion for Switch case statements requires special
      // handling. A single children container holds all switch cases. Update
      // insertion iterator to point to correct switch case FirstChild.
      if (ASwitch) {
        if (CaseNumber == 0) {
          InsertionPos = ASwitch->default_case_child_begin();
	}
        else {
          InsertionPos = ASwitch->case_child_begin(CaseNumber);
	}
      } 
      else {
        InsertionPos = Children->begin();
      }
      break;
    case LastChild:
      // LastChild insertion for Switch case statements requires special
      // handling. A single children container holds all switch cases. Update
      // insertion iterator to point to correct switch case LastChild.
      if (ASwitch) {
        if (CaseNumber == 0)
          InsertionPos = ASwitch->default_case_child_end();
        else
          InsertionPos = ASwitch->case_child_end(CaseNumber);
      }
      else {
        InsertionPos = Children->end();
      }
      break;
    case Append:
      InsertionPos = std::next(Pos);
      break;
    case Prepend:
      InsertionPos = Pos;
      break;
    default:
      llvm_unreachable("VPO: Unknown AVR Insertion Type");
  }

  // Insert new avr.
  NewAvr->setParent(Parent);
  Children->insert(InsertionPos, &*NewAvr);

  // Because the Avr Switch node uses a single linked-list container to store 
  // all the cases of the switch, we must update internal seperators which
  // keep track of where each case begins in the linked-list. 
  if ((Itype == FirstChild) && (ASwitch) && (CaseNumber != 0)) {
    ASwitch->CaseBegin[CaseNumber - 1] = std::prev(InsertionPos);
  }

  return;
}

void AVRUtils::insertFirstChildAVR(AVR *Parent, AvrItr NewAvr) {
  insertAVR(Parent, AvrItr(nullptr), NewAvr, FirstChild);
}

void AVRUtils::insertFirstThenChild(AVRIf *AvrIf, AvrItr NewAvr) {
  insertAVR(AvrIf, AvrItr(nullptr), NewAvr, FirstChild, ThenChild);
}

void AVRUtils::insertFirstElseChild(AVRIf *AvrIf, AvrItr NewAvr) {
  insertAVR(AvrIf, AvrItr(nullptr), NewAvr, FirstChild, ElseChild);
}

void AVRUtils::insertLastChildAVR(AVR *Parent, AvrItr NewAvr) {
  insertAVR(Parent, AvrItr(nullptr), NewAvr, LastChild);
}

void AVRUtils::insertLastThenChild(AVRIf *AvrIf, AvrItr NewAvr) {
  insertAVR(AvrIf, AvrItr(nullptr), NewAvr, LastChild, ThenChild);
}

void AVRUtils::insertLastElseChild(AVRIf *AvrIf, AvrItr NewAvr) {
  insertAVR(AvrIf, AvrItr(nullptr), NewAvr, LastChild, ElseChild);
}

void AVRUtils::insertAVRAfter(AvrItr InsertionPos, AVR *NewAvr) {
  assert(&*InsertionPos && "InsertionPos is Null");
  insertAVR(InsertionPos->getParent(), InsertionPos, AvrItr(NewAvr), Append);
}

void AVRUtils::insertAVRBefore(AvrItr InsertionPos, AVR *NewAvr) {
  assert(&*InsertionPos && "InsertionPos is Null");
  insertAVR(InsertionPos->getParent(), InsertionPos, AvrItr(NewAvr), Prepend);
}

void AVRUtils::insertFirstDefaultChild(AVRSwitch *ASwitch, AVR *NewAvr) {
  insertAVR(ASwitch, AvrItr(nullptr), AvrItr(NewAvr), FirstChild, None, 0);
}

void AVRUtils::insertLastDefaultChild(AVRSwitch *ASwitch, AVR *NewAvr) {
  insertAVR(ASwitch, AvrItr(nullptr), AvrItr(NewAvr), LastChild, None, 0);
}

void AVRUtils::insertFirstChild(AVRSwitch *ASwitch, AVR *NewAvr,
                                  unsigned CaseNum) {
  insertAVR(ASwitch, AvrItr(nullptr), AvrItr(NewAvr),
            FirstChild, None, CaseNum);
}

void AVRUtils::insertLastChild(AVRSwitch *ASwitch, AVR *NewAvr,
                                 unsigned CaseNum) {
  insertAVR(ASwitch, AvrItr(nullptr), AvrItr(NewAvr), LastChild, None, CaseNum);
}

void AVRUtils::insertAVRSeq(AVR *NewParent, AVRContainerTy &ToContainer,
                            AvrItr InsertionPos, AVRContainerTy *FromContainer,
                            AvrItr Begin, AvrItr End, InsertType Itype) {

  unsigned Distance = std::distance(Begin, End), I = 0;

  // Set insertion point
  switch (Itype) {
  case FirstChild:
    InsertionPos = ToContainer.begin();
    break;
  case LastChild:
    InsertionPos = ToContainer.end();
    break;
  case Append:
    InsertionPos = std::next(InsertionPos);
    break;
  case Prepend:
    // No change to InsertionPos will prepend sequence.
    InsertionPos = InsertionPos;
    break;
  default:
    llvm_unreachable("VPO: Unknown AVR Insertion Type");
  }

  ToContainer.splice(InsertionPos, *FromContainer, Begin, End);

  // Update parent of topmost nodes. Inner nodes' parent remains the same.
  for (auto It = InsertionPos; I < Distance; ++I, It--) {
    std::prev(It)->setParent(NewParent);
  }
}

// Move Utilities

void AVRUtils::moveAfter(AvrItr InsertionPos, AVR *Node) {

  remove(Node);
  insertAVRAfter(InsertionPos, Node);
}

void AVRUtils::moveAsFirstChildren(AVRLoop *ALoop, AvrItr First, AvrItr Last) {

  AVRContainerTy TempContainer;
  AVR *Begin = &*First, *End = &*Last;

  if (First->getParent() != Last->getParent()) {

    if(!resolveCommonLexicalParent(Begin, &End))
      llvm_unreachable("Coudlnt resolve common lexical parent for avr sequence!");
  }

  removeInternal(AvrItr(Begin), AvrItr(End), &TempContainer, false);
  insertAVRSeq(ALoop, ALoop->Children, ALoop->Children.begin(), &TempContainer,
               TempContainer.begin(), TempContainer.end(), FirstChild);
}

void AVRUtils::moveAsFirstThenChildren(AVRIf *AIf, AvrItr First, AvrItr Last) {

  assert(AIf && "Missing AvrIf for insertion!");

  AVRContainerTy TempContainer;
  AVR *Begin = &*First, *End = &*Last;

  if (First->getParent() != Last->getParent()) {

    if(!resolveCommonLexicalParent(Begin, &End))
      llvm_unreachable("Coudlnt resolve common lexical parent for avr sequence!");
  }

  removeInternal(AvrItr(Begin), AvrItr(End), &TempContainer, false);
  insertAVRSeq(AIf, AIf->ThenChildren, AIf->ThenChildren.begin(),
               &TempContainer, TempContainer.begin(), TempContainer.end(),
               FirstChild);
}

void AVRUtils::moveAsFirstElseChildren(AVRIf *AIf, AvrItr First, AvrItr Last) {

  assert(AIf && "Missing AvrIf for insertion!");

  AVRContainerTy TempContainer;
  AVR *Begin = &*First, *End = &*Last;

  if (First->getParent() != Last->getParent()) {

    if(!resolveCommonLexicalParent(Begin, &End))
      llvm_unreachable("Coudlnt resolve common lexical parent for avr sequence!");
  }

  removeInternal(AvrItr(Begin), AvrItr(End), &TempContainer, false);
  insertAVRSeq(AIf, AIf->ElseChildren, AIf->ElseChildren.begin(),
               &TempContainer, TempContainer.begin(), TempContainer.end(),
               FirstChild);
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
        for (unsigned Itr = 0, End = ASwitch->getNumCases(); Itr < End;
             ++Itr) {

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
  }
  else if(AVRSwitch *ASwitch = dyn_cast<AVRSwitch>(Parent)){
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

