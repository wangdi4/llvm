//===----------------------------------------------------------------------===//
//
//   Copyright (C) 2015-2016 Intel Corporation. All rights reserved.
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
#include "llvm/Analysis/Intel_VPO/Vecopt/VPOAvrStmtIR.h"
#include "llvm/Analysis/Intel_VPO/Vecopt/VPOAvrUtilsIR.h"
#include <cctype>

using namespace llvm;
using namespace llvm::vpo;

#define DEBUG_TYPE "avr-stmt-node"

// Helper function to trim leading white spaces in value printing. Improved
// pretty print support
static inline std::string &trimLeadingWhiteSpace(std::string &MyString) {
  // Trim leading white spaces for pretty print.
  MyString.erase(MyString.begin(),
                 std::find_if(MyString.begin(), MyString.end(),
                              std::not1(std::ptr_fun<int, int>(isspace))));
  return MyString;
}

//----------AVR Assign for LLVM IR Implementation----------//
AVRAssignIR::AVRAssignIR(Instruction *Inst)
    : AVRAssign(AVR::AVRAssignIRNode), Instruct(Inst) {}

AVRAssignIR *AVRAssignIR::clone() const { return nullptr; }

std::string AVRAssignIR::getAvrValueName() const {
  std::string IString;
  llvm::raw_string_ostream RSO(IString);
  Instruct->print(RSO);

  return IString;
}

void AVRAssignIR::codeGen() {
  Instruction *Inst;

  DEBUG(Instruct->dump());
  Inst = Instruct->clone();

  if (!Inst->getType()->isVoidTy())
    Inst->setName(Instruct->getName() + ".VPOClone");

  ReplaceInstWithInst(Instruct, Inst);
  DEBUG(Inst->dump());
}

//----------AVR Expression for LLVM IR Implementation----------//
AVRExpressionIR::AVRExpressionIR(AVRAssignIR *Assign, AssignOperand Operand)
    : AVRExpression(AVR::AVRExpressionIRNode) {

  AVRValueIR *AvrVal = nullptr;
  const Value *OpValue = nullptr;

  Instruct = Assign->getLLVMInstruction(); // Set LLVM Instuction
  this->Operation = Instruct->getOpcode(); // Set Operation Type

  // Create RHS Expression
  if (Operand == RightHand) {

    IsLHSExpr = false;

    if (auto StoreInstruct = dyn_cast<StoreInst>(Instruct)) {

      // Set RHS Expr to value operamd
      AvrVal = AVRUtilsIR::createAVRValueIR(StoreInstruct->getValueOperand(),
                                            Instruct);
      this->Operands.push_back(AvrVal);
    } else {
      for (auto Itr = Instruct->op_begin(), End = Instruct->op_end();
           Itr != End; ++Itr) {

        OpValue = *Itr;
        AvrVal = AVRUtilsIR::createAVRValueIR(OpValue, Instruct);
        this->Operands.push_back(AvrVal);
      }
    }
  }

  // Create LHS Expression
  if (Operand == LeftHand) {

    IsLHSExpr = true;

    if (auto StoreInstruct = dyn_cast<StoreInst>(Instruct)) {

      // Set LHS Expr to pointer operand
      AvrVal = AVRUtilsIR::createAVRValueIR(StoreInstruct->getPointerOperand(),
                                            Instruct);
    } else {

      OpValue = cast<Value>(Instruct);
      AvrVal = AVRUtilsIR::createAVRValueIR(OpValue, Instruct);
    }

    this->Operands.push_back(AvrVal);
  }
}

AVRExpressionIR *AVRExpressionIR::clone() const { return nullptr; }

std::string AVRExpressionIR::getAvrValueName() const { return ""; }

std::string AVRExpressionIR::getOpCodeName() const {
  return Instruct->getOpcodeName();
}

//----------AVR Value for LLVM IR Implementation----------//
AVRValueIR::AVRValueIR(const Value *V, const Instruction *Inst)
    : AVRValue(AVR::AVRValueIRNode), Val(V), Instruct(Inst) {

  ValType = Val->getType();
}

AVRValueIR *AVRValueIR::clone() const { return nullptr; }

void AVRValueIR::print(formatted_raw_ostream &OS, unsigned Depth,
                       VerbosityLevel VLevel) const {

  // Print AVR Value Node.
  switch (VLevel) {
  case PrintNumber:
    OS << "(" << getNumber() << ")";
  case PrintAvrType:
    OS << getAvrTypeName() << "{";
  case PrintDataType:
    OS << *ValType << " ";
  case PrintBase:
    Val->printAsOperand(OS, false);
    break;
  default:
    llvm_unreachable("Unknown Avr Print Verbosity!");
  }

  // Close up open braces
  if (VLevel >= PrintAvrType)
    OS << "}";
}

std::string AVRValueIR::getAvrValueName() const { return ""; }

//----------AVR Label for LLVM IR Implementation----------//
AVRLabelIR::AVRLabelIR(BasicBlock *SourceB)
    : AVRLabel(AVR::AVRLabelIRNode), SourceBlock(SourceB) {}

AVRLabelIR *AVRLabelIR::clone() const { return nullptr; }

void AVRLabelIR::print(formatted_raw_ostream &OS, unsigned Depth,
                       VerbosityLevel VLevel) const {

  std::string Indent((Depth * TabLength), ' ');
  OS << Indent;

  // Print Avr Label Node.
  switch (VLevel) {
  case PrintNumber:
    OS << "(" << getNumber() << ") ";
  case PrintAvrType:
    OS << getAvrTypeName() << "{";
  case PrintDataType:
  case PrintBase:
    OS << getAvrValueName() << ":";
    break;
  default:
    llvm_unreachable("Unknown Avr Print Verbosity!");
  }

  // Close up open braces
  if (VLevel >= PrintAvrType)
    OS << "}";

  OS << "\n";
}

std::string AVRLabelIR::getAvrValueName() const {
  return getSourceBBlock()->getName();
}

void AVRLabelIR::codeGen() {}

//----------AVR Phi for LLVM IR Implementation----------//
AVRPhiIR::AVRPhiIR(Instruction *Inst)
    : AVRPhi(AVR::AVRPhiIRNode), Instruct(Inst) {}

AVRPhiIR *AVRPhiIR::clone() const { return nullptr; }

void AVRPhiIR::print(formatted_raw_ostream &OS, unsigned Depth,
                     VerbosityLevel VLevel) const {

  std::string Indent((Depth * TabLength), ' ');
  OS << Indent;

  // Print Avr Phi Node.
  switch (VLevel) {
  case PrintNumber:
    OS << "(" << getNumber() << ") ";
  case PrintAvrType:
    OS << getAvrTypeName() << "{";
  case PrintDataType:
  case PrintBase:
    OS << getAvrValueName();
    break;
  default:
    llvm_unreachable("Unknown Avr Print Verbosity!");
  }

  // Close up open braces
  if (VLevel >= PrintAvrType)
    OS << "}";

  OS << "\n";
}

std::string AVRPhiIR::getAvrValueName() const {
  std::string IString;
  llvm::raw_string_ostream RSO(IString);

  Instruct->print(RSO);
  IString = trimLeadingWhiteSpace(IString);

  return IString;
}

void AVRPhiIR::codeGen() {
  Instruction *Inst;

  DEBUG(Instruct->dump());
  Inst = Instruct->clone();

  if (!Inst->getType()->isVoidTy())
    Inst->setName(Instruct->getName() + ".VPOClone");

  ReplaceInstWithInst(Instruct, Inst);
  DEBUG(Inst->dump());
}

//----------AVR Call for LLVM IR Implementation----------//
AVRCallIR::AVRCallIR(Instruction *Inst)
    : AVRCall(AVR::AVRCallIRNode), Instruct(Inst) {}

AVRCallIR *AVRCallIR::clone() const { return nullptr; }

void AVRCallIR::print(formatted_raw_ostream &OS, unsigned Depth,
                      VerbosityLevel VLevel) const {

  std::string Indent((Depth * TabLength), ' ');
  OS << Indent;

  // Print Avr Call Node.
  switch (VLevel) {
  case PrintNumber:
    OS << "(" << getNumber() << ") ";
  case PrintAvrType:
    OS << getAvrTypeName() << "{";
  case PrintDataType:
  case PrintBase:
    OS << getAvrValueName();
    break;
  default:
    llvm_unreachable("Unknown Avr Print Verbosity!");
  }

  // Close up open braces
  if (VLevel >= PrintAvrType)
    OS << "}";

  OS << "\n";
}

std::string AVRCallIR::getAvrValueName() const {
  std::string IString;
  llvm::raw_string_ostream RSO(IString);

  Instruct->print(RSO);
  IString = trimLeadingWhiteSpace(IString);

  return IString;
}

void AVRCallIR::codeGen() {
  Instruction *Inst;

  DEBUG(Instruct->dump());
  Inst = Instruct->clone();

  if (!Inst->getType()->isVoidTy())
    Inst->setName(Instruct->getName() + ".VPOClone");

  ReplaceInstWithInst(Instruct, Inst);
  DEBUG(Inst->dump());
}

//----------AVR Branch for LLVM IR Implementation----------//
AVRBranchIR::AVRBranchIR(Instruction *In, AVR *Cond)
    : AVRBranch(AVR::AVRBranchIRNode, false, Cond), Instruct(In) {

  if (BranchInst *BI = dyn_cast<BranchInst>(In)) {

    if (BI->isConditional()) {
      setIsConditional(true);
      ThenBBlock = BI->getSuccessor(0);
      ElseBBlock = BI->getSuccessor(1);
    } else {
      setIsConditional(false);
    }
  }
}

AVRBranchIR *AVRBranchIR::clone() const { return nullptr; }

void AVRBranchIR::print(formatted_raw_ostream &OS, unsigned Depth,
                        VerbosityLevel VLevel) const {

  std::string Indent((Depth * TabLength), ' ');

  OS << Indent;

  // Print Avr Branch Node.
  switch (VLevel) {
  case PrintNumber:
    OS << "(" << getNumber() << ") ";
  case PrintAvrType:
    OS << getAvrTypeName() << "{";
  case PrintDataType:
  case PrintBase:
    OS << getAvrValueName();
    break;
  default:
    llvm_unreachable("Unknown Avr Print Verbosity!");
  }

  // Close up open braces
  if (VLevel >= PrintAvrType)
    OS << "}";

  OS << "\n";
}

std::string AVRBranchIR::getAvrValueName() const {
  std::string IString;
  llvm::raw_string_ostream RSO(IString);

  Instruct->print(RSO);
  IString = trimLeadingWhiteSpace(IString);

  return IString;
}

void AVRBranchIR::codeGen() {
  Instruction *Inst;

  DEBUG(Instruct->dump());
  Inst = Instruct->clone();

  if (!Inst->getType()->isVoidTy())
    Inst->setName(Instruct->getName() + ".VPOClone");

  ReplaceInstWithInst(Instruct, Inst);
  DEBUG(Inst->dump());
}

//----------AVR BackEdge for LLVM IR Implementation----------//
AVRBackEdgeIR::AVRBackEdgeIR(Instruction *Inst)
    : AVRBackEdge(AVR::AVRBackEdgeIRNode), Instruct(Inst) {}

AVRBackEdgeIR *AVRBackEdgeIR::clone() const { return nullptr; }

void AVRBackEdgeIR::print(formatted_raw_ostream &OS, unsigned Depth,
                          VerbosityLevel VLevel) const {

  std::string Indent((Depth * TabLength), ' ');
  OS << Indent;

  // Print Avr BackEdge Node.
  switch (VLevel) {
  case PrintNumber:
    OS << "(" << getNumber() << ") ";
  case PrintAvrType:
    OS << getAvrTypeName() << "{";
  case PrintDataType:
  case PrintBase:
    OS << getAvrValueName();
    break;
  default:
    llvm_unreachable("Unknown Avr Print Verbosity!");
  }

  // Close up open braces
  if (VLevel >= PrintAvrType)
    OS << "}";
}

std::string AVRBackEdgeIR::getAvrValueName() const {
  std::string IString;
  llvm::raw_string_ostream RSO(IString);
  Instruct->print(RSO);

  return IString;
}

void AVRBackEdgeIR::codeGen() {
  Instruction *Inst;

  DEBUG(Instruct->dump());
  Inst = Instruct->clone();

  if (!Inst->getType()->isVoidTy())
    Inst->setName(Instruct->getName() + ".VPOClone");

  ReplaceInstWithInst(Instruct, Inst);
  DEBUG(Inst->dump());
}

//----------AVR Entry for LLVM IR Implementation----------//
AVREntryIR::AVREntryIR(Instruction *Inst)
    : AVREntry(AVR::AVREntryIRNode), Instruct(Inst) {}

AVREntryIR *AVREntryIR::clone() const { return nullptr; }

void AVREntryIR::print(formatted_raw_ostream &OS, unsigned Depth,
                       VerbosityLevel VLevel) const {

  std::string Indent((Depth * TabLength), ' ');
  OS << Indent;

  // Print Avr Entry Node.
  switch (VLevel) {
  case PrintNumber:
    OS << "(" << getNumber() << ") ";
  case PrintAvrType:
    OS << getAvrTypeName() << "{";
  case PrintDataType:
  case PrintBase:
    OS << getAvrValueName();
    break;
  default:
    llvm_unreachable("Unknown Avr Print Verbosity!");
  }

  // Close up open braces
  if (VLevel >= PrintAvrType)
    OS << "}";
}

std::string AVREntryIR::getAvrValueName() const {
  std::string IString;
  llvm::raw_string_ostream RSO(IString);

  Instruct->print(RSO);
  IString = trimLeadingWhiteSpace(IString);

  return IString;
}

void AVREntryIR::codeGen() {
  Instruction *Inst;

  DEBUG(Instruct->dump());
  Inst = Instruct->clone();

  if (!Inst->getType()->isVoidTy())
    Inst->setName(Instruct->getName() + ".VPOClone");

  ReplaceInstWithInst(Instruct, Inst);
  DEBUG(Inst->dump());
}

//----------AVR Return for LLVM IR Implementation----------//
AVRReturnIR::AVRReturnIR(Instruction *Inst)
    : AVRReturn(AVR::AVRReturnIRNode), Instruct(Inst) {}

AVRReturnIR *AVRReturnIR::clone() const { return nullptr; }

void AVRReturnIR::print(formatted_raw_ostream &OS, unsigned Depth,
                        VerbosityLevel VLevel) const {

  std::string Indent((Depth * TabLength), ' ');
  OS << Indent;

  // Print Avr Return Node.
  switch (VLevel) {
  case PrintNumber:
    OS << "(" << getNumber() << ") ";
  case PrintAvrType:
    OS << getAvrTypeName() << "{";
  case PrintDataType:
  case PrintBase:
    OS << getAvrValueName();
    break;
  default:
    llvm_unreachable("Unknown Avr Print Verbosity!");
  }

  // Close up open braces
  if (VLevel >= PrintAvrType)
    OS << "}";

  OS << "\n";
}

std::string AVRReturnIR::getAvrValueName() const {
  std::string IString;
  llvm::raw_string_ostream RSO(IString);

  Instruct->print(RSO);
  IString = trimLeadingWhiteSpace(IString);

  return IString;
}

void AVRReturnIR::codeGen() {
  Instruction *Inst;

  DEBUG(Instruct->dump());
  Inst = Instruct->clone();

  if (!Inst->getType()->isVoidTy())
    Inst->setName(Instruct->getName() + ".VPOClone");

  ReplaceInstWithInst(Instruct, Inst);
  DEBUG(Inst->dump());
}

//----------------------------------------------------------------------------//
// AVR Select Node for LLVM IR
//----------------------------------------------------------------------------//
AVRSelectIR::AVRSelectIR(Instruction *Inst, AVR *ACondition)
  : AVRSelect(AVR::AVRSelectIRNode), Instruct(Inst) {

  this->setCondition(ACondition);
}


AVRSelectIR *AVRSelectIR::clone() const { return nullptr; }

void AVRSelectIR::print(formatted_raw_ostream &OS, unsigned Depth,
                        VerbosityLevel VLevel) const {

  std::string Indent((Depth * TabLength), ' ');
  OS << Indent;

  // Print Avr Select Node.
  switch (VLevel) {
  case PrintNumber:
    OS << "(" << getNumber() << ") ";
  case PrintAvrType:
    OS << getAvrTypeName() << "{";
  case PrintDataType:
  case PrintBase:
    OS << getAvrValueName();
    break;
  default:
    llvm_unreachable("Unknown Avr Print Verbosity!");
  }

  // Close up open braces
  if (VLevel >= PrintAvrType)
    OS << "}";

  OS << "\n";
}

std::string AVRSelectIR::getAvrValueName() const {
  std::string IString;
  llvm::raw_string_ostream RSO(IString);
  Instruct->print(RSO);

  return IString;
}

void AVRSelectIR::codeGen() {
  Instruction *Inst;

  DEBUG(Instruct->dump());
  Inst = Instruct->clone();

  if (!Inst->getType()->isVoidTy())
    Inst->setName(Instruct->getName() + ".VPOClone");

  ReplaceInstWithInst(Instruct, Inst);
  DEBUG(Inst->dump());
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

AVRCompareIR *AVRCompareIR::clone() const { return nullptr; }

void AVRCompareIR::print(formatted_raw_ostream &OS, unsigned Depth,
                         VerbosityLevel VLevel) const {

  std::string Indent((Depth * TabLength), ' ');
  OS << Indent;

  // Print Avr Compare Node.
  switch (VLevel) {
  case PrintNumber:
    OS << "(" << getNumber() << ") ";
  case PrintAvrType:
    OS << getAvrTypeName() << "{";
  case PrintDataType:
  case PrintBase:
    OS << getAvrValueName();
    break;
  default:
    llvm_unreachable("Unknown Avr Print Verbosity!");
  }

  // Close up open braces
  if (VLevel >= PrintAvrType)
    OS << "}";

  OS << "\n";
}

std::string AVRCompareIR::getAvrValueName() const {
  std::string IString;
  llvm::raw_string_ostream RSO(IString);

  Instruct->print(RSO);
  IString = trimLeadingWhiteSpace(IString);

  return IString;
}

void AVRCompareIR::codeGen() {
  Instruction *Inst;

  DEBUG(Instruct->dump());
  Inst = Instruct->clone();

  if (!Inst->getType()->isVoidTy())
    Inst->setName(Instruct->getName() + ".VPOClone");

  ReplaceInstWithInst(Instruct, Inst);
  DEBUG(Inst->dump());
}


//----------AVR Unreachable for LLVM IR Implementation----------//
AVRUnreachableIR::AVRUnreachableIR(Instruction *Inst)
    : AVRUnreachable(AVR::AVRUnreachableIRNode), Instruct(Inst) {}

AVRUnreachableIR *AVRUnreachableIR::clone() const { return nullptr; }

