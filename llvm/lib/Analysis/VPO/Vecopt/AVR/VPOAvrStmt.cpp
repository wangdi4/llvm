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
//   VPOAvrFunction.cpp -- Implements the Abstract Vector Representation (AVR)
//   statement nodes.
//
//===----------------------------------------------------------------------===//

#include "llvm/Support/ErrorHandling.h"
#include "llvm/IR/CFG.h"
#include "llvm/Analysis/VPO/Vecopt/AVR/VPOAvrStmt.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"

using namespace llvm;
using namespace llvm::vpo;

#define DEBUG_TYPE "avr"

// TODO: Properly define print routines.


//----------AVR Assign Implementation----------//
AVRAssign::AVRAssign(Instruction * Inst)
  : AVR(AVR::AVRAssignNode), Instruct(Inst) {}

AVRAssign *AVRAssign::clone() const {
  return nullptr;
}

void AVRAssign::print() const {
  DEBUG(dbgs() <<"AVR_ASSIGN: ");
  DEBUG(Instruct->dump());
}

void AVRAssign::dump() const {
  print();
}

void AVRAssign::codeGen() {
  Instruction *inst;

  DEBUG(Instruct->dump());
  inst = Instruct->clone();

  if (!inst->getType()->isVoidTy())
    inst->setName(Instruct->getName() + 
                  ".VPOClone");

  ReplaceInstWithInst(Instruct, inst);
  DEBUG(inst->dump());
}

//----------AVR Label Implementation----------//
AVRLabel::AVRLabel(BasicBlock *SourceB)
  : AVR(AVR::AVRLabelNode), SourceBlock(SourceB) {}

AVRLabel *AVRLabel::clone() const {
  return nullptr;
}

void AVRLabel::print() const {
  DEBUG(dbgs() <<"\nAVR_LABEL:    " <<
    this->SourceBlock->getName() << "\n");
}

void AVRLabel::dump() const {
  print();
}
void AVRLabel::codeGen() {
}

//----------AVR Phi Implementation----------//
AVRPhi::AVRPhi(Instruction * Inst)
  : AVR(AVR::AVRPhiNode), Instruct(Inst) {}

AVRPhi *AVRPhi::clone() const {
  return nullptr;
}

void AVRPhi::print() const {
  DEBUG(dbgs() <<"AVR_PHI:    ");
  DEBUG(Instruct->dump());
}

void AVRPhi::dump() const {
  print();
}

void AVRPhi::codeGen() {
  Instruction *inst;

  DEBUG(Instruct->dump());
  inst = Instruct->clone();

  if (!inst->getType()->isVoidTy())
    inst->setName(Instruct->getName() + 
                  ".VPOClone");

  ReplaceInstWithInst(Instruct, inst);
  DEBUG(inst->dump());
}

//----------AVR Call Implementation----------//
AVRCall::AVRCall(Instruction * Inst)
  : AVR(AVR::AVRCallNode), Instruct(Inst) {}

AVRCall *AVRCall::clone() const {
  return nullptr;
}

void AVRCall::print() const {
  DEBUG(dbgs() <<"AVR_CALL:   ");
  DEBUG(Instruct->dump());
}

void AVRCall::dump() const {
  print();
}

void AVRCall::codeGen() {
  Instruction *inst;

  DEBUG(Instruct->dump());
  inst = Instruct->clone();

  if (!inst->getType()->isVoidTy())
    inst->setName(Instruct->getName() + 
                  ".VPOClone");

  ReplaceInstWithInst(Instruct, inst);
  DEBUG(inst->dump());
}

//----------AVR FBranch Implementation----------//
AVRFBranch::AVRFBranch(Instruction * Inst)
  : AVR(AVR::AVRFBranchNode), Instruct(Inst) {}

AVRFBranch *AVRFBranch::clone() const {
  return nullptr;
}

void AVRFBranch::print() const {
  DEBUG(dbgs() <<"AVR_FBRANCH:");
  DEBUG(Instruct->dump());
}

void AVRFBranch::dump() const {
  print();
}

void AVRFBranch::codeGen() {
  Instruction *inst;

  DEBUG(Instruct->dump());
  inst = Instruct->clone();

  if (!inst->getType()->isVoidTy())
    inst->setName(Instruct->getName() + 
                  ".VPOClone");

  ReplaceInstWithInst(Instruct, inst);
  DEBUG(inst->dump());
}

//----------AVR BackEdge Implementation----------//
AVRBackEdge::AVRBackEdge(Instruction * Inst)
  : AVR(AVR::AVRFBranchNode), Instruct(Inst) {}

AVRBackEdge *AVRBackEdge::clone() const {
  return nullptr;
}

void AVRBackEdge::print() const {
  DEBUG(dbgs() <<"AVR_BACKEDGE:");
  DEBUG(Instruct->dump());
}

void AVRBackEdge::dump() const {
  print();
}

void AVRBackEdge::codeGen() {
  Instruction *inst;

  DEBUG(Instruct->dump());
  inst = Instruct->clone();

  if (!inst->getType()->isVoidTy())
    inst->setName(Instruct->getName() + 
                  ".VPOClone");

  ReplaceInstWithInst(Instruct, inst);
  DEBUG(inst->dump());
}

//----------AVR Entry Implementation----------//
AVREntry::AVREntry(Instruction * Inst)
  : AVR(AVR::AVREntryNode), Instruct(Inst) {}

AVREntry *AVREntry::clone() const {
  return nullptr;
}

void AVREntry::print() const {
  DEBUG(dbgs() <<"AVR_ENTRY: ");
  DEBUG(Instruct->dump());
}

void AVREntry::dump() const {
  print();
}

void AVREntry::codeGen() {
  Instruction *inst;

  DEBUG(Instruct->dump());
  inst = Instruct->clone();

  if (!inst->getType()->isVoidTy())
    inst->setName(Instruct->getName() + 
                  ".VPOClone");

  ReplaceInstWithInst(Instruct, inst);
  DEBUG(inst->dump());
}


//----------AVR Return Implementation----------//
AVRReturn::AVRReturn(Instruction * Inst)
  : AVR(AVR::AVRReturnNode), Instruct(Inst) {}

AVRReturn *AVRReturn::clone() const {
  return nullptr;
}

void AVRReturn::print() const {
  DEBUG(dbgs() <<"AVR_RETURN: ");
  DEBUG(Instruct->dump());
}

void AVRReturn::dump() const {
  print();
}

void AVRReturn::codeGen() {
  Instruction *inst;

  DEBUG(Instruct->dump());
  inst = Instruct->clone();

  if (!inst->getType()->isVoidTy())
    inst->setName(Instruct->getName() + 
                  ".VPOClone");

  ReplaceInstWithInst(Instruct, inst);
  DEBUG(inst->dump());
}

