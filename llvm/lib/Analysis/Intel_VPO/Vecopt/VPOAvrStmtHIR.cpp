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
//   VPOAvrStmtHIR.cpp -- Implements the Abstract Vector Representation (AVR)
//   statement nodes for HIR.
//
//===----------------------------------------------------------------------===//

#include "llvm/Analysis/Intel_VPO/Vecopt/VPOAvrStmtHIR.h"
#include "llvm/Analysis/Intel_VPO/Vecopt/VPOAvrUtilsHIR.h"

using namespace llvm;
using namespace llvm::vpo;

#define DEBUG_TYPE "avr-stmt-node"

//----------AVR Assign for HIR Implementation----------//
AVRAssignHIR::AVRAssignHIR(HLInst * Inst)
  : AVRAssign(AVR::AVRAssignHIRNode), Instruct(Inst) {}

AVRAssignHIR *AVRAssignHIR::clone() const {
  return nullptr;
}

std::string AVRAssignHIR::getAvrValueName() const {
  std::string IString;
  llvm::raw_string_ostream RSO(IString);

  formatted_raw_ostream FOS(RSO);
  Instruct->print(FOS,1, false);

  // TODO: Need proper clean up of the printing of HLInst for pretty print.

  return IString;
}

//----------AVR Expression for HIR Implementation----------//
AVRExpressionHIR::AVRExpressionHIR(AVRAssignHIR *HLAssign, AssignOperand Operand)
  : AVRExpression(AVR::AVRExpressionHIRNode) {

 HIRNode = HLAssign->getHIRInstruction();

  if (Operand == RightHand) {

    // Set Operands
    unsigned NumRvalOps = HIRNode->getNumOperands();
    for (unsigned Idx = 1; Idx < NumRvalOps; ++Idx) {

      RegDDRef *DDRef =  HIRNode->getOperandDDRef(Idx);
      AVRValueHIR *AvrVal = AVRUtilsHIR::createAVRValueHIR(DDRef, HIRNode);
      this->Operands.push_back(AvrVal);

      IsLHSExpr = false;
    }

    // Set Operation Type
    this->Operation = HIRNode->getLLVMInstruction()->getOpcode();
  }

  if (Operand == LeftHand) {

    RegDDRef *DDRef = HIRNode->getLvalDDRef();
    if (DDRef) {

      AVRValueHIR *AvrVal = AVRUtilsHIR::createAVRValueHIR(DDRef, HIRNode);
      this->Operands.push_back(AvrVal);
      IsLHSExpr = true;
    }
    else
      DEBUG(dbgs() << "NO LHS\n");
  }
}

AVRExpressionHIR *AVRExpressionHIR::clone() const {
  return nullptr;
}

std::string AVRExpressionHIR::getAvrValueName() const {
  return "";
}

std::string AVRExpressionHIR::getOpCodeName() const {
  auto NodeOpCode = HIRNode->getLLVMInstruction()->getOpcode();
  assert(NodeOpCode && "Missing HIR node opcode!");
  return Instruction::getOpcodeName(NodeOpCode);
}

//----------AVR Value for HIR Implementation----------//
AVRValueHIR::AVRValueHIR(RegDDRef *DDRef, HLInst *Inst)
  : AVRValue(AVR::AVRValueHIRNode), Val(DDRef), HLInstruct(Inst) {}

AVRValueHIR *AVRValueHIR::clone() const {
  return nullptr;
}

void AVRValueHIR::print(formatted_raw_ostream &OS, unsigned Depth,
                        VerbosityLevel VLevel) const {

  // Print AVR Value Node.
  switch (VLevel) {
    case PrintNumber:
      OS << "("  << getNumber() << ")";
    case PrintAvrType:
      OS << getAvrTypeName() << "{";
    case PrintDataType:
      //OS << *ValType << " ";
    case PrintBase:
      Val->print(OS,false);
      break;
  default:
    llvm_unreachable("Unknown Avr Print Verbosity!");
  }

  // Close up open braces
  if (VLevel >= PrintAvrType)
    OS << "}";
}

std::string AVRValueHIR::getAvrValueName() const {
  return "";
}

//----------AVR Label for HIR Implementation----------//
AVRLabelHIR::AVRLabelHIR(HLNode *Inst)
  : AVRLabel(AVR::AVRLabelHIRNode), Instruct(Inst) {}

AVRLabelHIR *AVRLabelHIR::clone() const {
  return nullptr;
}

void AVRLabelHIR::print(formatted_raw_ostream &OS, unsigned Depth,
                     VerbosityLevel VLevel) const {
  std::string Indent(Depth * TabLength, ' ');

  if (VLevel > PrintBase) {
    OS << Indent << "AVR_LABEL:";
    Instruct->print(OS, Depth);
    OS << "\n" ;
  }
}

std::string AVRLabelHIR::getAvrValueName() const {
  return "";
}

//----------AVR Branch for HIR Implementation----------//
AVRBranchHIR::AVRBranchHIR(HLNode * Inst)
  : AVRBranch(AVR::AVRBranchHIRNode), Instruct(Inst) {}

AVRBranchHIR *AVRBranchHIR::clone() const {
  return nullptr;
}

void AVRBranchHIR::print(formatted_raw_ostream &OS, unsigned Depth,
                       VerbosityLevel VLevel) const {
  std::string Indent(Depth * TabLength, ' ');

  if (VLevel > PrintBase) {
    OS << Indent << "AVR_FBRANCH:";
    Instruct->print(OS, Depth);
    OS << "\n" ;
  }
}

std::string AVRBranchHIR::getAvrValueName() const {
  return "";
}

//----------AVR Unreachable for HIR Implementation----------//
AVRUnreachableHIR::AVRUnreachableHIR(HLNode *Inst)
  : AVRUnreachable(AVR::AVRUnreachableHIRNode), Instruct(Inst) {}

AVRUnreachableHIR *AVRUnreachableHIR::clone() const {
  return nullptr;
}
