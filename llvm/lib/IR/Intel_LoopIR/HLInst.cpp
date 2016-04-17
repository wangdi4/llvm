//===-------- HLInst.cpp - Implements the HLInst class --------------------===//
//
// Copyright (C) 2015-2016 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// This file implements the HLInst class.
//
//===----------------------------------------------------------------------===//

#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/DDRefUtils.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/HLNodeUtils.h"
#include "llvm/Transforms/Intel_VPO/Utils/VPOUtils.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IntrinsicInst.h"

using namespace llvm;
using namespace llvm::loopopt;

void HLInst::initialize() {
  /// This call is to get around calling virtual functions in the constructor.
  unsigned NumOp = getNumOperandsInternal();

  /// Number of operands stays the same over the lifetime of HLInst so make
  /// that the min size.
  RegDDRefs.resize(NumOp, nullptr);
}

HLInst::HLInst(Instruction *In)
    : HLDDNode(HLNode::HLInstVal), Inst(In), SafeRednSucc(nullptr),
      CmpOrSelectPred(PredicateTy::FCMP_TRUE) {
  assert(Inst && "LLVM Instruction for HLInst cannot be null!");
  initialize();
}

HLInst::HLInst(const HLInst &HLInstObj)
    : HLDDNode(HLInstObj), Inst(HLInstObj.Inst), SafeRednSucc(nullptr),
      CmpOrSelectPred(HLInstObj.CmpOrSelectPred) {

  unsigned NumOp, Count = 0;

  initialize();
  NumOp = getNumOperandsInternal();

  /// Clone DDRefs
  for (auto I = HLInstObj.ddref_begin(), E = HLInstObj.ddref_end(); I != E;
       I++, Count++) {
    if (Count < NumOp) {
      setOperandDDRef((*I)->clone(), Count);
    } else {
      addFakeDDRef((*I)->clone());
    }
  }
}

HLInst *HLInst::cloneImpl(GotoContainerTy *GotoList,
                          LabelMapTy *LabelMap) const {
  // Call the Copy Constructor
  HLInst *NewHLInst = new HLInst(*this);

  return NewHLInst;
}

HLInst *HLInst::clone() const {

  // Call the clone implementation.
  return cloneImpl(nullptr, nullptr);
}

bool HLInst::isCallInst() const { return (isa<CallInst>(Inst)); }

bool HLInst::isCopyInst() const {

  if (auto BCInst = dyn_cast<BitCastInst>(Inst)) {
    if (BCInst->getSrcTy() == BCInst->getDestTy()) {
      return true;
    }
  }

  return false;
}

/// TODO: Add beautification logic based on instruction types.
bool HLInst::checkSeparator(formatted_raw_ostream &OS, bool Print) const {

  bool Ret = true;
  unsigned Opcode = Inst->getOpcode();

  if (isa<BinaryOperator>(Inst)) {
    if (Print) {
      if ((Opcode == Instruction::Add) || (Opcode == Instruction::FAdd)) {
        OS << "  +  ";
      } else if ((Opcode == Instruction::Sub) ||
                 (Opcode == Instruction::FSub)) {
        OS << "  -  ";
      } else if ((Opcode == Instruction::Mul) ||
                 (Opcode == Instruction::FMul)) {
        OS << "  *  ";
      } else if ((Opcode == Instruction::UDiv) ||
                 (Opcode == Instruction::SDiv) ||
                 (Opcode == Instruction::FDiv)) {
        OS << "  /  ";
      } else if ((Opcode == Instruction::URem) ||
                 (Opcode == Instruction::SRem) ||
                 (Opcode == Instruction::FRem)) {
        OS << "  %  ";
      } else if (Opcode == Instruction::Shl) {
        OS << "  <<  ";
      } else if ((Opcode == Instruction::LShr) ||
                 (Opcode == Instruction::AShr)) {
        OS << "  >>  ";
      } else if (Opcode == Instruction::And) {
        OS << "  &&  ";
      } else if (Opcode == Instruction::Or) {
        OS << "  ||  ";
      } else if (Opcode == Instruction::Xor) {
        OS << "  ^  ";
      } else {
        llvm_unreachable("Unexpected binary operator!");
      }
    }
  } else if (isa<CmpInst>(Inst)) {
    if (Print) {
      printPredicate(OS, CmpOrSelectPred);
    }
  } else {
    if (!isa<CallInst>(Inst)) {
      Ret = false;
    }
    if (Print && !isa<SelectInst>(Inst)) {
      OS << ",  ";
    }
  }

  return Ret;
}

void HLInst::printBeginOpcode(formatted_raw_ostream &OS,
                              bool HasSeparator) const {

  if (auto CInst = dyn_cast<CastInst>(Inst)) {
    if (!isCopyInst()) {
      OS << CInst->getOpcodeName() << ".";
      OS << *(CInst->getSrcTy());
      OS << ".";
      OS << *(CInst->getDestTy());
      OS << "(";
    }
  } else if (auto FInst = dyn_cast<CallInst>(Inst)) {
    FInst->getCalledValue()->printAsOperand(OS, false);
    OS << "(";
  } else if (isa<SelectInst>(Inst)) {
    OS << "(";
  } else if (!HasSeparator && !isa<LoadInst>(Inst) && !isa<StoreInst>(Inst) &&
             !isa<GetElementPtrInst>(Inst) && !isa<CmpInst>(Inst)) {
    OS << Inst->getOpcodeName() << " ";
  }
}

void HLInst::printEndOpcode(formatted_raw_ostream &OS) const {
  if (isa<CallInst>(Inst) || (isa<CastInst>(Inst) && !isCopyInst())) {
    OS << ")";
  }
}

void HLInst::print(formatted_raw_ostream &OS, unsigned Depth,
                   bool Detailed) const {
  unsigned Count = 0;
  bool HasSeparator = checkSeparator(OS, false);

  indent(OS, Depth);

  for (auto I = op_ddref_begin(), E = op_ddref_end(); I != E; ++I, ++Count) {
    if ((Count > 1) || (!hasLval() && (Count > 0))) {
      checkSeparator(OS, true);
    }

    if (Count == 0) {
      if (hasLval()) {
        *I ? (*I)->print(OS, false) : (void)(OS << *I);

        OS << " = ";
        printBeginOpcode(OS, HasSeparator);

      } else {
        printBeginOpcode(OS, HasSeparator);

        *I ? (*I)->print(OS, false) : (void)(OS << *I);
      }
    } else {
      *I ? (*I)->print(OS, false) : (void)(OS << *I);

      if (isa<SelectInst>(Inst)) {
        if (Count == 1) {
          printPredicate(OS, CmpOrSelectPred);
        } else if (Count == 2) {
          OS << ") ? ";
        } else if (Count == 3) {
          OS << " : ";
        }
      }
    }
  }

  // Print function which no operands (no arguments or return value).
  if (!Count) {
    printBeginOpcode(OS, HasSeparator);
  }

  printEndOpcode(OS);

  OS << ";\n";

  HLDDNode::print(OS, Depth, Detailed);
}

bool HLInst::hasLval() const {
  /// The following logic is copied from AssemblyWriter::printInstruction().
  /// TODO: Is there a better way to determine this, probably by checking
  /// non-zero uses?
  return (Inst->hasName() || !Inst->getType()->isVoidTy() ||
          isa<StoreInst>(Inst));
}

RegDDRef *HLInst::getOperandDDRef(unsigned OperandNum) {
  assert(OperandNum < getNumOperands() && "Operand is out of range!");
  return getOperandDDRefImpl(OperandNum);
}

const RegDDRef *HLInst::getOperandDDRef(unsigned OperandNum) const {
  return const_cast<HLInst *>(this)->getOperandDDRef(OperandNum);
}

void HLInst::setOperandDDRef(RegDDRef *Ref, unsigned OperandNum) {
  assert(OperandNum < getNumOperands() && "Operand is out of range!");
  setOperandDDRefImpl(Ref, OperandNum);
}

RegDDRef *HLInst::removeOperandDDRef(unsigned OperandNum) {
  auto TRef = getOperandDDRef(OperandNum);

  if (TRef) {
    setOperandDDRef(nullptr, OperandNum);
  }

  return TRef;
}

RegDDRef *HLInst::getLvalDDRef() {
  if (hasLval()) {
    return cast<RegDDRef>(getOperandDDRefImpl(0));
  }

  return nullptr;
}

const RegDDRef *HLInst::getLvalDDRef() const {
  return const_cast<HLInst *>(this)->getLvalDDRef();
}

void HLInst::setLvalDDRef(RegDDRef *RDDRef) {
  assert(hasLval() && "This instruction does not have an lval!");

  setOperandDDRefImpl(RDDRef, 0);
}

RegDDRef *HLInst::removeLvalDDRef() {
  auto TRef = getLvalDDRef();

  setLvalDDRef(nullptr);

  return TRef;
}

bool HLInst::hasRval() const {
  return (isa<StoreInst>(Inst) || isa<GetElementPtrInst>(Inst) ||
          (hasLval() && isa<UnaryInstruction>(Inst)));
}

RegDDRef *HLInst::getRvalDDRef() {
  if (hasRval()) {
    return getOperandDDRefImpl(1);
  }

  return nullptr;
}

const RegDDRef *HLInst::getRvalDDRef() const {
  return const_cast<HLInst *>(this)->getRvalDDRef();
}

void HLInst::setRvalDDRef(RegDDRef *Ref) {
  assert(hasRval() && "This instruction does not have a rval!");

  setOperandDDRefImpl(Ref, 1);
}

RegDDRef *HLInst::removeRvalDDRef() {
  auto TRef = getRvalDDRef();

  setRvalDDRef(nullptr);

  return TRef;
}

void HLInst::addFakeDDRef(RegDDRef *RDDRef) {
  assert(RDDRef && "Cannot add null fake DDRef!");

  RegDDRefs.push_back(RDDRef);
  setNode(RDDRef, this);
}

void HLInst::removeFakeDDRef(RegDDRef *RDDRef) {
  HLDDNode *Node;

  (void)Node;
  assert(RDDRef && "Cannot remove null fake DDRef!");
  assert(RDDRef->isFake() && "RDDRef is not a fake DDRef!");
  assert((Node = RDDRef->getHLDDNode()) && isa<HLInst>(Node) &&
         (cast<HLInst>(Node) == this) &&
         "RDDRef does not belong to this HLInst!");

  for (auto I = fake_ddref_begin(), E = fake_ddref_end(); I != E; I++) {
    if ((*I) == RDDRef) {
      setNode(RDDRef, nullptr);
      RegDDRefs.erase(I);
      return;
    }
  }

  llvm_unreachable("Unexpected condition!");
}

unsigned HLInst::getNumOperands() const { return getNumOperandsInternal(); }

unsigned HLInst::getNumOperandsInternal() const {
  unsigned NumOp = 0;

  if (isa<GetElementPtrInst>(Inst)) {
    // GEP is represented as an assignment of address: %t = &A[i];
    // TODO: GEP accessing structure elements
    NumOp = 1;
  } else if (auto CInst = dyn_cast<CallInst>(Inst)) {
    NumOp = CInst->getNumArgOperands();
  } else {
    NumOp = Inst->getNumOperands();
  }

  if (hasLval() && !isa<StoreInst>(Inst)) {
    NumOp++;
  }
  // Select instruction gains an extra operand due to inclusion of the
  // predicate.
  if (isa<SelectInst>(Inst)) {
    NumOp++;
  }

  return NumOp;
}

bool HLInst::isInPreheaderPostexitImpl(bool Preheader) const {
  auto HLoop = getParentLoop();

  if (!HLoop) {
    return false;
  }

  auto I = Preheader ? HLoop->pre_begin() : HLoop->post_begin();
  auto E = Preheader ? HLoop->pre_end() : HLoop->post_end();

  for (; I != E; I++) {
    if (cast<HLInst>(I) == this) {
      return true;
    }
  }

  return false;
}

bool HLInst::isInPreheader() const { return isInPreheaderPostexitImpl(true); }

bool HLInst::isInPostexit() const { return isInPreheaderPostexitImpl(false); }

bool HLInst::isInPreheaderOrPostexit() const {
  return (isInPreheader() || isInPostexit());
}

void HLInst::verify() const {
  bool IsSelectCmpTrueFalse = (isa<SelectInst>(Inst) || isa<CmpInst>(Inst)) &&
                              isPredicateTrueOrFalse(CmpOrSelectPred);

  assert((CmpInst::isFPPredicate(CmpOrSelectPred) ||
          CmpInst::isIntPredicate(CmpOrSelectPred) ||
          CmpOrSelectPred == UNDEFINED_PREDICATE) &&
         "Invalid predicate value, should be one of PredicateTy");

  HLDDNode::verify();

  if (IsSelectCmpTrueFalse) {
    assert(getOperandDDRef(1)->containsUndef() &&
           getOperandDDRef(2)->containsUndef() &&
           "DDRefs for Select or Cmp Instruction with "
           "True or False predicate must be undefined");
  }

  if (isa<LoadInst>(Inst)) {
    assert(getRvalDDRef()->isMemRef() &&
           "Rval of load instruction is not a memref!");
  }

  if (isa<StoreInst>(Inst)) {
    assert(getLvalDDRef()->isMemRef() &&
           "Lval of store instruction is not a memref!");
  }

  if (isa<GetElementPtrInst>(Inst)) {
    assert(getRvalDDRef()->isAddressOf() &&
           "Rval of GEP instruction is not an AddressOf ref!");
  }
}

bool HLInst::isIntrinCall(Intrinsic::ID &IntrinID) const {
  auto Call = dyn_cast<IntrinsicInst>(getLLVMInstruction());
  if (!Call) {
    return false;
  }
  IntrinID = Call->getIntrinsicID();
  return true;
}

bool HLInst::isSIMDDirective() const {
  Intrinsic::ID IntrinID;
  if (!isIntrinCall(IntrinID) || !vpo::VPOUtils::isIntelDirective(IntrinID)) {
    return false;
  }
  // TODO: check metadata
  return true;
}

