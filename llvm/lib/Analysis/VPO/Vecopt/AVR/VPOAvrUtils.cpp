//===------- VPOAvrUtils.cpp - Implements AVRUtils class --------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements the the AVRUtils class.
//
//===----------------------------------------------------------------------===//

#include "llvm/Analysis/VPO/Vecopt/AVR/VPOAvrUtils.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"

#define DEBUG_TYPE "avr"


using namespace llvm;
using namespace intel;

// AVR Creatation Utilities
AVRFunction *AVRUtils::createAVRFunction(Function *OrigF) {
  return new AVRFunction(OrigF);
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

  assert(Parent && "Parent is Null");

  // TODO: Clean this up - Only Supports Function Parents right now.
  // TODO: Add AVRLoop, AVRIf support.

  if (isa<AVRFunction>(Parent)) {

    AVRFunction *Func = cast<AVRFunction>(Parent);
    AVRContainerTy &AVRContainer = Func->Children;
    AvrItr InsertionPos;

    switch (Itype) {
      case FirstChild:
        InsertionPos = Func->child_begin();
        break;
      case LastChild:
        InsertionPos = Func->getLastChild();
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

    NewAvr->setParent(Parent);
    AVRContainer.insert(InsertionPos, NewAvr);
  }
  else {
    // TODO: Missing Support
    DEBUG(dbgs() << "Missing Full Insertion Support\n");
  }

  return;
}

