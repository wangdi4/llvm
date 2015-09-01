//===----------------------------------------------------------------------===//
//
//   Copyright (C) 2015 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation. and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//   Source file:
//   ------------
//   VPOAvrUtils.cpp -- Implements the Abstract Vector Representation (AVR)
//   utilities.
//
//===----------------------------------------------------------------------===//

#include "llvm/Analysis/VPO/Vecopt/AVR/VPOAvrUtils.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"

#define DEBUG_TYPE "avr-utilities"

using namespace llvm;
using namespace llvm::vpo;

// AVR Creation Utilities
AVRFunction *AVRUtils::createAVRFunction(Function *OrigF, const LoopInfo *LpInfo) {
  return new AVRFunction(OrigF, LpInfo);
}

AVRLoop *AVRUtils::createAVRLoop(const Loop *Lp) {
  return new AVRLoop(Lp);
}

AVRWrn *AVRUtils::createAVRWrn(WRNVecLoopNode *WrnSimdNode) {
  assert(WrnSimdNode && "WrnSimdNode is empty!");
  return new AVRWrn(WrnSimdNode);
}

AVRAssign *AVRUtils::createAVRAssign(Instruction *Inst) {
  return new AVRAssign(Inst);
}

AVRLabel *AVRUtils::createAVRLabel(BasicBlock *Block) {
  return new AVRLabel(Block);
}

AVRPhi *AVRUtils::createAVRPhi(Instruction *Inst) {
  return new AVRPhi(Inst);
}

AVRCall *AVRUtils::createAVRCall(Instruction *Inst) {
  return new AVRCall(Inst);
}

AVRFBranch *AVRUtils::createAVRFBranch(Instruction *Inst) {
  return new AVRFBranch(Inst);
}

AVRBackEdge *AVRUtils::createAVRBackEdge(Instruction *Inst) {
  return new AVRBackEdge(Inst);
}

AVREntry *AVRUtils::createAVREntry(Instruction *Inst) {
  return new AVREntry(Inst);
}

AVRReturn *AVRUtils::createAVRReturn(Instruction *Inst) {
  return new AVRReturn(Inst);
}

AVRIf *AVRUtils::createAVRIf(Instruction *Inst) {
  return new AVRIf(Inst);
}


// Insertion Utilities

void AVRUtils::insertFirstChildAVR(AVR *Parent, AvrItr NewAvr) {
  insertAVR(Parent, nullptr, NewAvr, FirstChild);
}

void AVRUtils::insertLastChildAVR(AVR *Parent, AvrItr NewAvr) {
  insertAVR(Parent, nullptr, NewAvr, LastChild);
}

void AVRUtils::insertAVRAfter(AvrItr InsertionPos, AVR *NewAvr) {
  assert(InsertionPos && "InsertionPos is Null");
  insertAVR(InsertionPos->getParent(), InsertionPos, NewAvr, Append);
}

void AVRUtils::insertAVRBefore(AvrItr InsertionPos, AVR *NewAvr) {
  assert(InsertionPos && "InsertionPos is Null");
  insertAVR(InsertionPos->getParent(), InsertionPos, NewAvr, Prepend);
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

void AVRUtils::insertAVR(AVR *Parent, AvrItr Pos, AvrItr NewAvr, InsertType Itype) {

  assert(Parent && "Parent is null.");

  AvrItr InsertionPos;
  AVRContainerTy  *Children = nullptr;

  if (AVRFunction *AFunc = dyn_cast<AVRFunction>(Parent)) {
    Children = &(AFunc->Children);
  }
  else if (AVRLoop *ALoop = dyn_cast<AVRLoop>(Parent)) {
    Children = &(ALoop->Children);
  }
  else if (AVRIf *AIf = dyn_cast<AVRIf>(Parent)) {
    llvm_unreachable("VPO: AVR_SPLIT Insertion not supported.\n");
    // TODO: Split Support
    Children = &(AIf->ThenChildren);
    Children = &(AIf->ElseChildren);
  } 
  else if (AVRWrn *AWrn = dyn_cast<AVRWrn>(Parent)) {
    Children = &(AWrn->Children);
  }
  else {
    llvm_unreachable("VPO: Unsupported AVR Insertion\n");
  }

  assert(Children && "Children container ptr is null.");

  // Set insertion point
  switch (Itype) {
    case FirstChild:
      InsertionPos = Children->begin();
      break;
    case LastChild:
      InsertionPos = Children->end();
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
  Children->insert(InsertionPos, NewAvr);

  return;
}

void AVRUtils::moveAfter(AvrItr InsertionPos, AVR *Node) {

  remove(Node);
  insertAVRAfter(InsertionPos, Node);
}

void AVRUtils::moveAsFirstChildren(AVRLoop *ALoop, AvrItr First, AvrItr Last) {

  AVR *Parent = First->getParent();
  AVRContainerTy TempContainer, 
                 *OrigContainer = AVRUtils::getAvrChildren(Parent);

  removeInternal(OrigContainer, First, Last, &TempContainer, false);
  insertAVRSeq(ALoop, ALoop->Children, ALoop->Children.begin(),
               &TempContainer, TempContainer.begin(),
               TempContainer.end(), FirstChild);
}


// Removal Utilities

void AVRUtils::destroy(AVR *Avr) {

  Avr->destroy();
}

AVRContainerTy *AVRUtils::removeInternal(AVRContainerTy *OrigContainer,
                                         AvrItr Begin, AvrItr End,
                                         AVRContainerTy *MoveContainer,
                                         bool Delete) {

  assert(OrigContainer && "Container missing for node removal!");

  // Removal of Avr or Avr sequence doenst require move to new location.
  if (!MoveContainer) {

    for (auto I = Begin, Next = I, E = End; I != E; I = Next) {
   
      Next++;
      AVR *Node = OrigContainer->remove(I);

      if (Delete)
        destroy(Node);
    }
  }
  else {
    // Removal of AVR sequence is being moved to a new container.
    MoveContainer->splice(MoveContainer->end(), *OrigContainer, Begin, ++End);
  }

  return MoveContainer;
}


// Remove singleton AVR
void AVRUtils::remove(AVR* Node) {

  assert (Node && "Missing AVR Node!");

  AVR *Parent = Node->getParent();
  AVRContainerTy *PChildren = AVRUtils::getAvrChildren(Parent);

  removeInternal(PChildren, Node, nullptr, nullptr, false);
}

// Remove sequence of AVRs
void AVRUtils::remove(AvrItr Begin, AvrItr End) {

  AVR *BParent = Begin->getParent();
  AVR *EParent = End->getParent();

  if (BParent != EParent) {
    assert (0 &&
       "Removal of AVR sequnece without a common parent is not supported!");
  }

  AVRContainerTy *PChildren = AVRUtils::getAvrChildren(BParent);

  removeInternal(PChildren, Begin, End, nullptr, false);
}

// Search Utilities

AVRLabel *AVRUtils::getAvrLabelForBB(BasicBlock *BB, AVR *ParentNode) {

  assert(BB && "Missing Basic Block!");
  assert(ParentNode && "Missing Avr Node!");

  AVRLabel *AvrLabelNode = nullptr;
  AVRContainerTy *Children = getAvrChildren(ParentNode);

  if (Children) {
    for (auto I = Children->begin(), E = Children->end(); I != E; ++I) {
      AvrLabelNode = AVRUtils::getAvrLabelForBB(BB, I);
      if (AvrLabelNode)
	return AvrLabelNode; // Found Node
    }
  }
  else {
    if (AVRLabel *AvrLb = dyn_cast<AVRLabel>(ParentNode)) {
      if (AvrLb->getSourceBBlock() == BB) {
        AvrLabelNode = AvrLb;
      }
    }
  }

  return AvrLabelNode;
}

AVRFBranch *AVRUtils::getAvrBranchForTerm(Instruction *Terminator, AVR *ParentNode) {

  assert(Terminator && "Missing Basic Block terminator!");
  assert(ParentNode && "Missing Avr Node!");

  AVRFBranch *AvrBNode = nullptr;
  AVRContainerTy *Children = getAvrChildren(ParentNode);

  if (Children) {
    for (auto I = Children->begin(), E = Children->end(); I != E; ++I) {
      AvrBNode = AVRUtils::getAvrBranchForTerm(Terminator, I);
      if (AvrBNode)
	return AvrBNode; // Found Node
    }
  }
  else {
    if (AVRFBranch *AvrB = dyn_cast<AVRFBranch>(ParentNode)) {
      if (AvrB->getLLVMInstruction() == Terminator) {
        AvrBNode = AvrB;
      }
    }
  }

  return AvrBNode;
}

AVRContainerTy *AVRUtils::getAvrChildren(AVR *Avr) {

  AVRContainerTy *Children = nullptr;

  if (AVRFunction *AFunc = dyn_cast<AVRFunction>(Avr)) {
    Children = &(AFunc->Children);
  }
  else if (AVRLoop *ALoop = dyn_cast<AVRLoop>(Avr)) {
    Children = &(ALoop->Children);
  }
  else if (AVRIf *AIf = dyn_cast<AVRIf>(Avr)) {
     // TODO: Split Support incorrect.
    Children = &(AIf->ThenChildren);
    Children = &(AIf->ElseChildren);
  } 
  else if (AVRWrn *AWrn = dyn_cast<AVRWrn>(Avr)) {
    Children = &(AWrn->Children);
  }

  return Children;
}
