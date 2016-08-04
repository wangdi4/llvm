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
  : AVRExpression(AVR::AVRExpressionHIRNode, nullptr) { 

  HLInst* HLInst = HLAssign->getHIRInstruction();
  HIRNode = HLInst;
  const Instruction* LLVMInstruction = HLInst->getLLVMInstruction();
  Opcode = LLVMInstruction->getOpcode();
  if (const CmpInst* LLVMCmpInst = dyn_cast<CmpInst>(LLVMInstruction))
    Predicate = LLVMCmpInst->getPredicate();
  else
    Predicate = CmpInst::Predicate::BAD_ICMP_PREDICATE;

  this->setParent(HLAssign); // Set Parent

  // Set the data type
  const StoreInst *SI = dyn_cast<StoreInst>(LLVMInstruction);
  Type *Ty = (SI ? SI->getValueOperand()->getType() : LLVMInstruction->getType());
  this->setType(Ty); 

  if (Operand == RightHand) {

    // Set Operands
    unsigned NumRvalOps = HLInst->getNumOperands();
    unsigned Idx;

    // RVAL operands start at index 1 if HLInst has LVAL and 0 otherwise
    Idx = HLInst->hasLval() ? 1 : 0;

    for ( ; Idx < NumRvalOps; ++Idx) {

      RegDDRef *DDRef =  HLInst->getOperandDDRef(Idx);
      AVRValueHIR *AvrVal = AVRUtilsHIR::createAVRValueHIR(DDRef, HLInst, this);
      this->Operands.push_back(AvrVal);

      IsLHSExpr = false;
    }

    // Set Operation Type
    this->Operation = HLInst->getLLVMInstruction()->getOpcode();
  }

  if (Operand == LeftHand) {

    IsLHSExpr = true;
    RegDDRef *DDRef = HLInst->getLvalDDRef();
    if (DDRef) {

      AVRValueHIR *AvrVal = AVRUtilsHIR::createAVRValueHIR(DDRef, HLInst, this);
      this->Operands.push_back(AvrVal);
    }
    else
      DEBUG(dbgs() << "NO LHS\n"); // TODO: is this reachable (IsLHSExpr = ?)?
  }
  else
    IsLHSExpr = false;
}

// TODO: set type
AVRExpressionHIR::AVRExpressionHIR(AVRIfHIR *AIf,
                                   HLIf::const_pred_iterator& PredIt)
  : AVRExpression(AVR::AVRExpressionHIRNode, nullptr) {

  const HLIf *HIf = AIf->getCompareInstruction();
  HIRNode = nullptr; // this is an HLIf predicate - no underlying HLInst.
  Predicate = *PredIt;
  if (Predicate <= CmpInst::Predicate::LAST_FCMP_PREDICATE)
    this->Opcode = Instruction::FCmp;
  else
    this->Opcode = Instruction::ICmp;
  this->setParent(AIf);
  this->Operation = this->Opcode;

  IsLHSExpr = false;

  RegDDRef *LHS = HIf->getPredicateOperandDDRef(PredIt, true);
  if (LHS) {

    AVRValueHIR *AvrVal = AVRUtilsHIR::createAVRValueHIR(LHS, nullptr, this);
    this->Operands.push_back(AvrVal);
  }

  RegDDRef *RHS = HIf->getPredicateOperandDDRef(PredIt, false);
  if (RHS) {

    AVRValueHIR *AvrVal = AVRUtilsHIR::createAVRValueHIR(RHS, nullptr, this);
    this->Operands.push_back(AvrVal);
  }
}

// TODO: set type
AVRExpressionHIR::AVRExpressionHIR(AVRExpressionHIR* LHS,
                                   AVRExpressionHIR* RHS)
  : AVRExpression(AVR::AVRExpressionHIRNode, nullptr) {

  HIRNode = nullptr; // no underlying HLInst.
  this->Predicate = CmpInst::Predicate::BAD_ICMP_PREDICATE;
  this->Opcode = Instruction::And;
  this->Operation = this->Opcode;

  IsLHSExpr = false;

  this->Operands.push_back(LHS);
  this->Operands.push_back(RHS);
}

AVRExpressionHIR *AVRExpressionHIR::clone() const {
  return nullptr;
}

std::string AVRExpressionHIR::getAvrValueName() const {
  return "";
}

std::string AVRExpressionHIR::getOpCodeName() const {
  return Instruction::getOpcodeName(Opcode);
}

//----------AVR Value for HIR Implementation----------//
AVRValueHIR::AVRValueHIR(RegDDRef *DDRef, HLNode *Node, AVR *Parent)
  : AVRValue(AVR::AVRValueHIRNode, nullptr), Val(DDRef), HNode(Node) {
  setParent(Parent);

  Type *DataType;
  const CanonExpr *CE = nullptr;

  if (DDRef->hasGEPInfo()) {
    CE = DDRef->getBaseCE();
    PointerType *BaseTy = cast<PointerType>(CE->getSrcType()); //CHECKME: getDestType?
    Type *ElemTy = DDRef->getSrcType(); //CHECKME: getDestType?
    // In the case of array of ints for example (int a[300]):
    // ElemTy is i32  (int)
    // BaseTy is [300 x i32]*  (pointer to array)
    // We want i32* (pointer to int), so we build it:
    DataType = ElemTy->getPointerTo(BaseTy->getPointerAddressSpace()); 
  }
  else {
    // CHECKME: DDRef->getSrcType? getDestType?
    CE = DDRef->getSingleCanonExpr();
    assert(CE && "DDRef is empty!");
    DataType = CE->getSrcType();
  }

  this->setType(DataType);
}

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
    case PrintDataType: {
      Type *ValType = getType();
      printSLEV(OS);
      OS << *ValType << " ";
    }
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

  if (VLevel == PrintNumber)
    OS << "("  << getNumber() << ")";
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

  if (VLevel == PrintNumber)
    OS << "("  << getNumber() << ")";
  if (VLevel > PrintBase) {
    OS << Indent << "AVR_FBRANCH:";
    printSLEV(OS);
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
