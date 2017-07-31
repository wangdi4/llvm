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
#include "llvm/Analysis/Intel_VPO/Vecopt/VPOAvrUtils.h"
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
    Condition = LLVMCmpInst->getPredicate();
  else
    Condition = CmpInst::Predicate::BAD_ICMP_PREDICATE;

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
      AVRValueHIR *AvrVal = AVRUtilsHIR::createAVRValueHIR(
          DDRef, HLInst, this, Opcode == Instruction::Load);
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

      AVRValueHIR *AvrVal = AVRUtilsHIR::createAVRValueHIR(
          DDRef, HLInst, this, Opcode == Instruction::Store);
      this->Operands.push_back(AvrVal);
    }
    else
      DEBUG(dbgs() << "NO LHS\n"); // TODO: is this reachable (IsLHSExpr = ?)?
  }
  else
    IsLHSExpr = false;
}

AVRExpressionHIR::AVRExpressionHIR(AVRIfHIR *AIf,
                                   HLIf::const_pred_iterator& PredIt)
  : AVRExpression(AVR::AVRExpressionHIRNode, nullptr) {

  HLIf *HIf = AIf->getCompareInstruction();
  HIRNode = nullptr; // this is an HLIf predicate - no underlying HLInst.
  Condition = *PredIt;
  if (Condition <= CmpInst::Predicate::LAST_FCMP_PREDICATE)
    this->Opcode = Instruction::FCmp;
  else
    this->Opcode = Instruction::ICmp;
  this->setParent(AIf);
  this->Operation = this->Opcode;

  IsLHSExpr = false;

  RegDDRef *LHS = HIf->getPredicateOperandDDRef(PredIt, true);
  if (LHS) {

    AVRValueHIR *AvrVal = AVRUtilsHIR::createAVRValueHIR(LHS, HIf, this);
    this->Operands.push_back(AvrVal);
  }

  RegDDRef *RHS = HIf->getPredicateOperandDDRef(PredIt, false);
  assert (RHS && "Predicate's RHS is null");
  AVRValueHIR *AvrVal = AVRUtilsHIR::createAVRValueHIR(RHS, HIf, this);
  this->Operands.push_back(AvrVal);

  // Set type
  assert(!RHS->getSrcType()->isVectorTy() && "SrcType is vector type");
  assert(!RHS->getDestType()->isVectorTy() && "DstType is vector type");
  this->setType(Type::getInt1Ty(RHS->getDestType()->getContext()));
}

AVRExpressionHIR::AVRExpressionHIR(AVRExpressionHIR *LHS, AVRExpressionHIR *RHS)
    : AVRExpression(AVR::AVRExpressionHIRNode,
                    Type::getInt1Ty(RHS->getType()->getContext())) {

  assert(!LHS->getType()->isVectorTy() && "LHS has vector type");
  assert(!RHS->getType()->isVectorTy() && "RHS has vector type");

  HIRNode = nullptr; // no underlying HLInst.
  this->Opcode = Instruction::And;
  this->Operation = this->Opcode;

  IsLHSExpr = false;

  this->Operands.push_back(LHS);
  this->Operands.push_back(RHS);
}

// If we are not going to need IR specific information,
// Should we move this functionality to AVRExpression?
// AVRPredicator is doing something similar
AVRExpressionHIR::AVRExpressionHIR(AVR* LHS,
                                   AVR* RHS,
                                   Type *Ty,
                                   unsigned Opcode)
  : AVRExpression(AVR::AVRExpressionHIRNode, Ty) {

  assert(Ty && "Expression type is null");

  HIRNode = nullptr; // no underlying HLInst.
  // Why do we need Opcode and Operation?
  this->Opcode = Opcode;
  this->Operation = this->Opcode;

  // TODO
  IsLHSExpr = false;

  AVRUtils::setParent(LHS, this);
  AVRUtils::setParent(RHS, this);

  this->Operands.push_back(LHS);
  this->Operands.push_back(RHS);
}

AVRExpressionHIR *AVRExpressionHIR::clone() const {
  return nullptr;
}

std::string AVRExpressionHIR::getAvrValueName() const {
  return "";
}

//----------AVR Value for HIR Implementation----------//
AVRValueHIR::AVRValueHIR(RegDDRef *DDRef, HLNode *Node, AVR *Parent,
                         bool isMemoryOperation)
    : AVRValue(AVR::AVRValueHIRNode, nullptr), Val(DDRef), HNode(Node) {

  assert(Node && "HLNode cannot be null");

  setParent(Parent);

  Type *DataType = DDRef->getDestType();

  // We need a pointer type only if the parent AVR is a load or store (i.e.
  // isMemoryOperation is true)
  if (DDRef->hasGEPInfo() && isMemoryOperation) {
    CanonExpr *CE = DDRef->getBaseCE();
    PointerType *BaseTy = cast<PointerType>(CE->getDestType());

    // In the case of array of ints for example (int a[300]), the dest type of
    // the DDRef is i32 (int). We want i32* (pointer to int), so we build it.
    // This also works for DDRefs with trailing offsets.
    DataType = DataType->getPointerTo(BaseTy->getPointerAddressSpace());
  }

  this->setType(DataType);

  // In HIR, Metadata is considered a constant value because they do not affect
  // DDs. We do not take MD into account at AVR level by now.
  if (DDRef->isConstant() && !DDRef->isMetadata()) {
    ConstantFP *FPConst = nullptr;
    Constant *Const = nullptr;
    int64_t IntConst;

    if (DDRef->isIntConstant(&IntConst)) {
      IntegerType *IntDataType = cast<IntegerType>(DataType);
      Const = ConstantInt::getSigned(IntDataType, IntConst);
      this->setConstant(Const);
    } else if (DDRef->isFPConstant(&FPConst)) {
      this->setConstant(FPConst);
    } else if (DDRef->isConstantVector(&Const)) {
      this->setConstant(Const);
    } else if (DDRef->isNull()) {
      this->setConstant(ConstantPointerNull::get(cast<PointerType>(DataType)));
    } else {
      llvm_unreachable("CanonExpr has an unexpected constant value!");
    }
  }

  // Add IVValue info if RegDDRef is a standalone IV (1 * i3)
  if (DDRef->isStandAloneIV(false /*AllowConversion*/)) {
    assert(DDRef->isSingleCanonExpr() &&
           "Standalone IV must have a single canon expr");
    CanonExpr *CE = DDRef->getSingleCanonExpr();
    assert(CE->isStandAloneIV(false /*AllowConversion*/) &&
           "Standalone IV CanonExpr expected");

    setIVValue(new AVRValueHIR::IVValueInfo(CE, CE->getFirstIVLevel()));
  }
}

//TODO
AVRValueHIR::AVRValueHIR(BlobDDRef *DDRef, AVR *Parent)
    : AVRValue(AVR::AVRValueHIRNode, DDRef->getDestType()), Val(DDRef),
      HNode(nullptr) {
  setParent(Parent);
}

AVRValueHIR::AVRValueHIR(IVValueInfo *IVV, Type *Ty, AVR *Parent)
    : AVRValue(AVR::AVRValueHIRNode, Ty), Val(nullptr), IVVal(IVV),
      HNode(nullptr) {
  setParent(Parent);
}

AVRValueHIR *AVRValueHIR::clone() const {
  return nullptr;
}

void AVRValueHIR::print(formatted_raw_ostream &OS, unsigned Depth,
                        VerbosityLevel VLevel) const {
#if !INTEL_PRODUCT_RELEASE
  const Constant *Const = getConstant();

  // Print AVR Value Node.
  switch (VLevel) {
  case PrintCost:
  case PrintNumber:
    OS << "(" << getNumber() << ")";
  case PrintAvrDecomp: {
    AVR *DecTree = getDecompTree();
    if (DecTree != nullptr) {
      DecTree->print(OS, Depth, VLevel);
      break;
    }
  }
  case PrintAvrType:
      OS << getAvrTypeName() << "{";
  case PrintDataType: {
    if (Const == nullptr) {
      Type *ValType = getType();
      printSLEV(OS);
      OS << *ValType << " ";
    }
  }
  case PrintBase: {
    // If there is constant information, we print it and
    // ignore Val as it can be a nullptr
    if (Const != nullptr) {
      Const->print(OS, false);
    } else if (Val != nullptr) {
      Val->print(OS, false);
    } else { // IV Value
      assert(IVVal != nullptr && "IVValue is null");
      OS << "i" << IVVal->Level;
    }

    break;
  }
  default:
    llvm_unreachable("Unknown Avr Print Verbosity!");
  }

  // Close up open braces
  if (VLevel >= PrintAvrType)
    OS << "}";
#endif // !INTEL_PRODUCT_RELEASE
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
#if !INTEL_PRODUCT_RELEASE
  std::string Indent(Depth * TabLength, ' ');

  if (VLevel == PrintNumber)
    OS << "("  << getNumber() << ")";
  if (VLevel > PrintBase) {
    OS << Indent << "AVR_LABEL:";
    Instruct->print(OS, Depth);
    OS << "\n" ;
  }
#endif // !INTEL_PRODUCT_RELEASE
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
#if !INTEL_PRODUCT_RELEASE
  std::string Indent(Depth * TabLength, ' ');

  if (VLevel == PrintNumber)
    OS << "("  << getNumber() << ")";
  if (VLevel > PrintBase) {
    OS << Indent << "AVR_FBRANCH:";
    printSLEV(OS);
    Instruct->print(OS, Depth);
    OS << "\n" ;
  }
#endif // !INTEL_PRODUCT_RELEASE
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
