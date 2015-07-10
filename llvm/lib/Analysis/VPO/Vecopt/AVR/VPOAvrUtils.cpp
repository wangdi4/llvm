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

#define DEBUG_TYPE "avr"


using namespace llvm;
using namespace llvm::vpo;

// AVR Creatation Utilities
AVRFunction *AVRUtils::createAVRFunction(Function *OrigF) {
  return new AVRFunction(OrigF);
}

AVRLoop *AVRUtils::createAVRLoop(const LoopInfo *OrigL, bool isDW) {
  return new AVRLoop(OrigL, isDW);
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

