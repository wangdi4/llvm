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
//   VPOAvrStmtIR.cpp -- Implements the Abstract Vector Representation (AVR)
//   statement nodes for LLVM IR.
//
//===----------------------------------------------------------------------===//

#include "llvm/IR/CFG.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/Analysis/VPO/Vecopt/AVR/VPOAvrStmtIR.h"

using namespace llvm;
using namespace llvm::vpo;

#define DEBUG_TYPE "avr-stmt-node"

//----------AVR Assign for LLVM IR Implementation----------//
AVRAssignIR::AVRAssignIR(Instruction * Inst)
  : AVRAssign(AVR::AVRAssignIRNode), Instruct(Inst) {}

AVRAssignIR *AVRAssignIR::clone() const {
  return nullptr;
}

void AVRAssignIR::print(formatted_raw_ostream &OS, unsigned Depth,
                      unsigned VerbosityLevel) const {
  std::string Indent(Depth * TabLength, ' ');

  if (VerbosityLevel > 0) {
    OS << Indent << "AVR_ASSIGN: ";
    Instruct->print(OS); 
    OS << "\n" ;
  }
}

void AVRAssignIR::codeGen() {
  Instruction *inst;

  DEBUG(Instruct->dump());
  inst = Instruct->clone();

  if (!inst->getType()->isVoidTy())
    inst->setName(Instruct->getName() + 
                  ".VPOClone");

  ReplaceInstWithInst(Instruct, inst);
  DEBUG(inst->dump());
}

//----------AVR Label for LLVM IR Implementation----------//
AVRLabelIR::AVRLabelIR(BasicBlock *SourceB)
  : AVRLabel(AVR::AVRLabelIRNode), SourceBlock(SourceB) {}

AVRLabelIR *AVRLabelIR::clone() const {
  return nullptr;
}

void AVRLabelIR::print(formatted_raw_ostream &OS, unsigned Depth,
                     unsigned VerbosityLevel) const {
  std::string Indent(Depth * TabLength, ' ');

  if (VerbosityLevel > 0) {
    OS << Indent <<"AVR_LABEL:    " << this->SourceBlock->getName() << "\n";
  }
}

void AVRLabelIR::codeGen() {
}

//----------AVR Phi for LLVM IR Implementation----------//
AVRPhiIR::AVRPhiIR(Instruction * Inst)
  : AVRPhi(AVR::AVRPhiIRNode), Instruct(Inst) {}

AVRPhiIR *AVRPhiIR::clone() const {
  return nullptr;
}

void AVRPhiIR::print(formatted_raw_ostream &OS, unsigned Depth,
                     unsigned VerbosityLevel) const {
  std::string Indent(Depth * TabLength, ' ');

  if (VerbosityLevel > 0) {
    OS << Indent << "AVR_PHI:    ";
    Instruct->print(OS);
    OS << "\n" ;
  }
}

void AVRPhiIR::codeGen() {
  Instruction *inst;

  DEBUG(Instruct->dump());
  inst = Instruct->clone();

  if (!inst->getType()->isVoidTy())
    inst->setName(Instruct->getName() + 
                  ".VPOClone");

  ReplaceInstWithInst(Instruct, inst);
  DEBUG(inst->dump());
}

//----------AVR Call for LLVM IR Implementation----------//
AVRCallIR::AVRCallIR(Instruction * Inst)
  : AVRCall(AVR::AVRCallIRNode), Instruct(Inst) {}

AVRCallIR *AVRCallIR::clone() const {
  return nullptr;
}

void AVRCallIR::print(formatted_raw_ostream &OS, unsigned Depth,
                    unsigned VerbosityLevel) const {
  std::string Indent(Depth * TabLength, ' ');

  if (VerbosityLevel > 0) {
    OS << Indent <<"AVR_CALL:   ";
    Instruct->print(OS);
    OS << "\n" ;
  }
}

void AVRCallIR::codeGen() {
  Instruction *inst;

  DEBUG(Instruct->dump());
  inst = Instruct->clone();

  if (!inst->getType()->isVoidTy())
    inst->setName(Instruct->getName() + 
                  ".VPOClone");

  ReplaceInstWithInst(Instruct, inst);
  DEBUG(inst->dump());
}

//----------AVR FBranch for LLVM IR Implementation----------//
AVRFBranchIR::AVRFBranchIR(Instruction * Inst)
  : AVRFBranch(AVR::AVRFBranchIRNode), Instruct(Inst) {}

AVRFBranchIR *AVRFBranchIR::clone() const {
  return nullptr;
}

void AVRFBranchIR::print(formatted_raw_ostream &OS, unsigned Depth,
                       unsigned VerbosityLevel) const {
  std::string Indent(Depth * TabLength, ' ');

  if (VerbosityLevel > 0) {
    OS << Indent << "AVR_FBRANCH:";
    Instruct->print(OS);
    OS << "\n" ;
  }
}

void AVRFBranchIR::codeGen() {
  Instruction *inst;

  DEBUG(Instruct->dump());
  inst = Instruct->clone();

  if (!inst->getType()->isVoidTy())
    inst->setName(Instruct->getName() + 
                  ".VPOClone");

  ReplaceInstWithInst(Instruct, inst);
  DEBUG(inst->dump());
}

//----------AVR BackEdge for LLVM IR Implementation----------//
AVRBackEdgeIR::AVRBackEdgeIR(Instruction * Inst)
  : AVRBackEdge(AVR::AVRBackEdgeIRNode), Instruct(Inst) {}

AVRBackEdgeIR *AVRBackEdgeIR::clone() const {
  return nullptr;
}

void AVRBackEdgeIR::print(formatted_raw_ostream &OS, unsigned Depth,
                        unsigned VerbosityLevel) const {
  std::string Indent(Depth * TabLength, ' ');

  if (VerbosityLevel > 0) {
    OS << Indent << "AVR_BACKEDGE:";
    Instruct->print(OS);
    OS << "\n" ;
  }
}

void AVRBackEdgeIR::codeGen() {
  Instruction *inst;

  DEBUG(Instruct->dump());
  inst = Instruct->clone();

  if (!inst->getType()->isVoidTy())
    inst->setName(Instruct->getName() + 
                  ".VPOClone");

  ReplaceInstWithInst(Instruct, inst);
  DEBUG(inst->dump());
}

//----------AVR Entry for LLVM IR Implementation----------//
AVREntryIR::AVREntryIR(Instruction * Inst)
  : AVREntry(AVR::AVREntryIRNode), Instruct(Inst) {}

AVREntryIR *AVREntryIR::clone() const {
  return nullptr;
}

void AVREntryIR::print(formatted_raw_ostream &OS, unsigned Depth,
                     unsigned VerbosityLevel) const {
  std::string Indent(Depth * TabLength, ' ');

  if (VerbosityLevel > 0 ) {
    OS << Indent <<"AVR_ENTRY: ";
    Instruct->print(OS);
    OS << "\n" ;
  }
}

void AVREntryIR::codeGen() {
  Instruction *inst;

  DEBUG(Instruct->dump());
  inst = Instruct->clone();

  if (!inst->getType()->isVoidTy())
    inst->setName(Instruct->getName() + 
                  ".VPOClone");

  ReplaceInstWithInst(Instruct, inst);
  DEBUG(inst->dump());
}

//----------AVR Return for LLVM IR Implementation----------//
AVRReturnIR::AVRReturnIR(Instruction * Inst)
  : AVRReturn(AVR::AVRReturnIRNode), Instruct(Inst) {}

AVRReturnIR *AVRReturnIR::clone() const {
  return nullptr;
}

void AVRReturnIR::print(formatted_raw_ostream &OS, unsigned Depth,
                      unsigned VerbosityLevel) const {
  std::string Indent(Depth * TabLength, ' ');

  if (VerbosityLevel > 0) {
    OS << Indent << "AVR_RETURN: ";
    Instruct->print(OS);
    OS << "\n" ;
  }
}

void AVRReturnIR::codeGen() {
  Instruction *inst;

  DEBUG(Instruct->dump());
  inst = Instruct->clone();

  if (!inst->getType()->isVoidTy())
    inst->setName(Instruct->getName() + 
                  ".VPOClone");

  ReplaceInstWithInst(Instruct, inst);
  DEBUG(inst->dump());
}

