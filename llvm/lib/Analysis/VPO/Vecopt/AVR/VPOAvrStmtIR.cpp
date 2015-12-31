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
                        VerbosityLevel VLevel) const {

  std::string Indent((Depth * TabLength), ' ');

  OS << Indent;

  switch (VLevel) {
    case PrintNumber:
      OS << "(" << getNumber() << ") ";
    case PrintType:
      OS << getAvrTypeName();
    case PrintBase:
      OS << "{" << getAvrValueName() << "}\n";
      break;
    default:
      llvm_unreachable("Unknown Avr Print Verbosity!");
  }
}

std::string AVRAssignIR::getAvrValueName() const {
  std::string IString;
  llvm::raw_string_ostream RSO(IString);
  Instruct->print(RSO);

  return IString;
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
                     VerbosityLevel VLevel) const {

  std::string Indent((Depth * TabLength), ' ');

  OS << Indent;

  switch (VLevel) {
    case PrintNumber:
      OS << "(" << getNumber() << ") ";
    case PrintType:
      OS << getAvrTypeName();
    case PrintBase:
      OS << "{" << getAvrValueName() << "}\n";
      break;
    default:
      llvm_unreachable("Unknown Avr Print Verbosity!");
  }
}

std::string AVRLabelIR::getAvrValueName() const {
  return getSourceBBlock()->getName();
}

void AVRLabelIR::codeGen() {}

//----------AVR Phi for LLVM IR Implementation----------//
AVRPhiIR::AVRPhiIR(Instruction * Inst)
  : AVRPhi(AVR::AVRPhiIRNode), Instruct(Inst) {}

AVRPhiIR *AVRPhiIR::clone() const {
  return nullptr;
}

void AVRPhiIR::print(formatted_raw_ostream &OS, unsigned Depth,
                     VerbosityLevel VLevel) const {

  std::string Indent((Depth * TabLength), ' ');

  OS << Indent;

  switch (VLevel) {
    case PrintNumber:
      OS << "(" << getNumber() << ") ";
    case PrintType:
      OS << getAvrTypeName();
    case PrintBase:
      OS << "{" << getAvrValueName() << "}\n";
      break;
    default:
      llvm_unreachable("Unknown Avr Print Verbosity!");
  }
}

std::string AVRPhiIR::getAvrValueName() const {
  std::string IString;
  llvm::raw_string_ostream RSO(IString);
  Instruct->print(RSO);

  return IString;
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
                      VerbosityLevel VLevel) const {

  std::string Indent((Depth * TabLength), ' ');

  OS << Indent;

  switch (VLevel) {
    case PrintNumber:
      OS << "(" << getNumber() << ") ";
    case PrintType:
      OS << getAvrTypeName();
    case PrintBase:
      OS << "{" << getAvrValueName() << "}\n";
      break;
    default:
      llvm_unreachable("Unknown Avr Print Verbosity!");
  }
}

std::string AVRCallIR::getAvrValueName() const {
  std::string IString;
  llvm::raw_string_ostream RSO(IString);
  Instruct->print(RSO);

  return IString;
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

//----------AVR Branch for LLVM IR Implementation----------//
AVRBranchIR::AVRBranchIR(Instruction *In, AVR *Cond)
  : AVRBranch(AVR::AVRBranchIRNode, false, Cond), Instruct(In) { 

  if (BranchInst *BI = dyn_cast<BranchInst>(In)) {

    if (BI->isConditional()) {
      setIsConditional(true);
      ThenBBlock = BI->getSuccessor(0);
      ElseBBlock = BI->getSuccessor(1);
    }
    else {
      setIsConditional(false);
    }
  }
}

AVRBranchIR *AVRBranchIR::clone() const {
  return nullptr;
}

void AVRBranchIR::print(formatted_raw_ostream &OS, unsigned Depth,
                        VerbosityLevel VLevel) const {

  std::string Indent((Depth * TabLength), ' ');

  OS << Indent;

  switch (VLevel) {
    case PrintNumber:
      OS << "(" << getNumber() << ") ";
    case PrintType:
      OS << getAvrTypeName();
    case PrintBase:
      OS << "{" << getAvrValueName() << "}\n";
      break;
    default:
      llvm_unreachable("Unknown Avr Print Verbosity!");
  }
}

std::string AVRBranchIR::getAvrValueName() const {
  std::string IString;
  llvm::raw_string_ostream RSO(IString);

  Instruct->print(RSO);
 
  return IString;
}

void AVRBranchIR::codeGen() {
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
                          VerbosityLevel VLevel) const {

  std::string Indent((Depth * TabLength), ' ');

  OS << Indent;

  switch (VLevel) {
    case PrintNumber:
      OS << "(" << getNumber() << ") ";
    case PrintType:
      OS << getAvrTypeName();
    case PrintBase:
      OS << "{" << getAvrValueName() << "}\n";
      break;
    default:
      llvm_unreachable("Unknown Avr Print Verbosity!");
  }
}

std::string AVRBackEdgeIR::getAvrValueName() const {
  std::string IString;
  llvm::raw_string_ostream RSO(IString);
  Instruct->print(RSO);

  return IString;
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
                       VerbosityLevel VLevel) const {

  std::string Indent((Depth * TabLength), ' ');

  OS << Indent;

  switch (VLevel) {
    case PrintNumber:
      OS << "(" << getNumber() << ") ";
    case PrintType:
      OS << getAvrTypeName();
    case PrintBase:
      OS << "{" << getAvrValueName() << "}\n";
      break;
    default:
      llvm_unreachable("Unknown Avr Print Verbosity!");
  }
}

std::string AVREntryIR::getAvrValueName() const {
  std::string IString;
  llvm::raw_string_ostream RSO(IString);
  Instruct->print(RSO);

  return IString;
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
                        VerbosityLevel VLevel) const {

  std::string Indent((Depth * TabLength), ' ');

  OS << Indent;

  switch (VLevel) {
    case PrintNumber:
      OS << "(" << getNumber() << ") ";
    case PrintType:
      OS << getAvrTypeName();
    case PrintBase:
      OS << "{" << getAvrValueName() << "}\n";
      break;
    default:
      llvm_unreachable("Unknown Avr Print Verbosity!");
  }
}

std::string AVRReturnIR::getAvrValueName() const {
  std::string IString;
  llvm::raw_string_ostream RSO(IString);
  Instruct->print(RSO);

  return IString;
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

//----------------------------------------------------------------------------//
// AVR Select Node for LLVM IR
//----------------------------------------------------------------------------//
AVRSelectIR::AVRSelectIR(Instruction * Inst, AVRCompare *AComp)
  : AVRSelect(AVR::AVRSelectIRNode, AComp), Instruct(Inst) {}

AVRSelectIR *AVRSelectIR::clone() const {
  return nullptr;
}

void AVRSelectIR::print(formatted_raw_ostream &OS, unsigned Depth,
                        VerbosityLevel VLevel) const {

  std::string Indent((Depth * TabLength), ' ');

  OS << Indent;

  switch (VLevel) {
    case PrintNumber:
      OS << "(" << getNumber() << ") ";
    case PrintType:
      OS << getAvrTypeName();
    case PrintBase:
      OS << "{" << getAvrValueName() << "}\n";
      break;
    default:
      llvm_unreachable("Unknown Avr Print Verbosity!");
  }
}

std::string AVRSelectIR::getAvrValueName() const {
  std::string IString;
  llvm::raw_string_ostream RSO(IString);
  Instruct->print(RSO);

  return IString;
}

void AVRSelectIR::codeGen() {
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
// AVR Compare Node for LLVM IR
//----------------------------------------------------------------------------//
AVRCompareIR::AVRCompareIR(Instruction *Inst)
  : AVRCompare(AVR::AVRCompareIRNode), Instruct(Inst) {

  setIsCompareSelect(false);
  setSelect(nullptr);
  setBranch(nullptr);
}

AVRCompareIR *AVRCompareIR::clone() const {
  return nullptr;
}

void AVRCompareIR::print(formatted_raw_ostream &OS, unsigned Depth,
                         VerbosityLevel VLevel) const {

  std::string Indent((Depth * TabLength), ' ');

  OS << Indent;

  switch (VLevel) {
    case PrintNumber:
      OS << "(" << getNumber() << ") ";
    case PrintType:
      OS << getAvrTypeName();
    case PrintBase:
      OS << "{" << getAvrValueName() << "}\n";
      break;
    default:
      llvm_unreachable("Unknown Avr Print Verbosity!");
  }
}

std::string AVRCompareIR::getAvrValueName() const {
  std::string IString;
  llvm::raw_string_ostream RSO(IString);
  Instruct->print(RSO);

  return IString;
}

void AVRCompareIR::codeGen() {
  Instruction *inst;

  DEBUG(Instruct->dump());
  inst = Instruct->clone();

  if (!inst->getType()->isVoidTy())
    inst->setName(Instruct->getName() + 
                  ".VPOClone");

  ReplaceInstWithInst(Instruct, inst);
  DEBUG(inst->dump());
}
