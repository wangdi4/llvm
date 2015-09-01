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

#define DEBUG_TYPE "avr-stmt-node"

// TODO: Properly define print routines.


//----------AVR Assign Implementation----------//
AVRAssign::AVRAssign(Instruction * Inst)
  : AVR(AVR::AVRAssignNode), Instruct(Inst) {}

AVRAssign *AVRAssign::clone() const {
  return nullptr;
}

void AVRAssign::print(formatted_raw_ostream &OS, unsigned Depth,
                      unsigned VerbosityLevel) const {
  std::string Indent(Depth * TabLength, ' ');

  if (VerbosityLevel > 0) {
    OS << Indent << "AVR_ASSIGN: ";
    Instruct->print(OS); 
    OS << "\n" ;
  }
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

void AVRLabel::print(formatted_raw_ostream &OS, unsigned Depth,
                     unsigned VerbosityLevel) const {
  std::string Indent(Depth * TabLength, ' ');

  if (VerbosityLevel > 0) {
    OS << Indent <<"AVR_LABEL:    " << this->SourceBlock->getName() << "\n";
  }
}

void AVRLabel::codeGen() {
}

//----------AVR Phi Implementation----------//
AVRPhi::AVRPhi(Instruction * Inst)
  : AVR(AVR::AVRPhiNode), Instruct(Inst) {}

AVRPhi *AVRPhi::clone() const {
  return nullptr;
}

void AVRPhi::print(formatted_raw_ostream &OS, unsigned Depth,
                     unsigned VerbosityLevel) const {
  std::string Indent(Depth * TabLength, ' ');

  if (VerbosityLevel > 0) {
    OS << Indent << "AVR_PHI:    ";
    Instruct->print(OS);
    OS << "\n" ;
  }
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

void AVRCall::print(formatted_raw_ostream &OS, unsigned Depth,
                    unsigned VerbosityLevel) const {
  std::string Indent(Depth * TabLength, ' ');

  if (VerbosityLevel > 0) {
    OS << Indent <<"AVR_CALL:   ";
    Instruct->print(OS);
    OS << "\n" ;
  }
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

void AVRFBranch::print(formatted_raw_ostream &OS, unsigned Depth,
                       unsigned VerbosityLevel) const {
  std::string Indent(Depth * TabLength, ' ');

  if (VerbosityLevel > 0) {
    OS << Indent << "AVR_FBRANCH:";
    Instruct->print(OS);
    OS << "\n" ;
  }
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

void AVRBackEdge::print(formatted_raw_ostream &OS, unsigned Depth,
                        unsigned VerbosityLevel) const {
  std::string Indent(Depth * TabLength, ' ');

  if (VerbosityLevel > 0) {
    OS << Indent << "AVR_BACKEDGE:";
    Instruct->print(OS);
    OS << "\n" ;
  }
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

void AVREntry::print(formatted_raw_ostream &OS, unsigned Depth,
                     unsigned VerbosityLevel) const {
  std::string Indent(Depth * TabLength, ' ');

  if (VerbosityLevel > 0 ) {
    OS << Indent <<"AVR_ENTRY: ";
    Instruct->print(OS);
    OS << "\n" ;
  }
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

void AVRReturn::print(formatted_raw_ostream &OS, unsigned Depth,
                      unsigned VerbosityLevel) const {
  std::string Indent(Depth * TabLength, ' ');

  if (VerbosityLevel > 0) {
    OS << Indent << "AVR_RETURN: ";
    Instruct->print(OS);
    OS << "\n" ;
  }
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

//----------------------------------------------------------------------------//
// AVR Wrn Node
//----------------------------------------------------------------------------//
AVRWrn::AVRWrn(WRNVecLoopNode *WrnSimdNode)
  : AVR(AVR::AVRWrnNode), WRegionSimdNode(WrnSimdNode) {}

AVRWrn *AVRWrn::clone() const {
  return nullptr;
}


void AVRWrn::print(formatted_raw_ostream &OS, unsigned Depth,
                   unsigned VerbosityLevel) const {

  std::string Indent(Depth * TabLength, ' ');

  if (VerbosityLevel > 0) {

    OS << Indent <<"AVR_WRN\n";

    Depth++;
    for (auto Itr = child_begin(), E = child_end(); Itr != E; ++Itr) { 
      Itr->print(OS, Depth, VerbosityLevel);
    }
  }
}

void AVRWrn::codeGen() {
  for (auto Itr = child_begin(), E = child_end(); Itr != E; ++Itr) { 
    Itr->codeGen();
  }
}

