//===-------- HLInst.cpp - Implements the HLInst class --------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements the HLInst class.
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/Intel_LoopTransforms/Utils/HLNodeUtils.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/DDRefUtils.h"
#include "llvm/IR/Instructions.h"

using namespace llvm;
using namespace llvm::loopopt;

HLInst::HLInst(Instruction *In)
    : HLDDNode(HLNode::HLInstVal), Inst(In), SafeRednSucc(nullptr) {
  assert(Inst && "LLVM Instruction for HLInst cannot be null!");

  /// This call is to get around calling virtual functions in the constructor.
  unsigned NumOp = getNumOperandsInternal();

  /// Number of operands stays the same over the lifetime of HLInst so make
  /// that the min size.
  if (NumOp > DDRefs.size()) {
    DDRefs.resize(getNumOperands(), nullptr);
  }
}

HLInst::HLInst(const HLInst &HLInstObj)
    : HLDDNode(HLInstObj), SafeRednSucc(nullptr) {

  unsigned NumOp, Count = 0;

  /// Clone the LLVM Instruction
  Inst = HLInstObj.Inst->clone();
  NumOp = getNumOperands();

  /// Clone DDRefs
  for (auto I = HLInstObj.ddref_begin(), E = HLInstObj.ddref_end(); I != E;
       I++, Count++) {
    if (Count < NumOp) {
      setOperandDDRef((*I)->clone(), Count);
    } else {
      addFakeDDRef(cast<RegDDRef>((*I)->clone()));
    }
  }
}

HLInst *HLInst::clone() const {

  /// Call the Copy Constructor
  HLInst *NewHLInst = new HLInst(*this);

  return NewHLInst;
}

bool HLInst::hasLval() const {
  return (Inst->hasName() || isa<StoreInst>(Inst));
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

  if (TRef) {
    setOperandDDRefImpl(nullptr, 0);
  }

  return TRef;
}

void HLInst::addFakeDDRef(RegDDRef *RDDRef) {
  assert(RDDRef && "Cannot add null fake DDRef!");

  DDRefs.push_back(RDDRef);
  setNode(RDDRef, this);
}

void HLInst::removeFakeDDRef(RegDDRef *RDDRef) {
  HLDDNode *Node;

  assert(RDDRef && "Cannot remove null fake DDRef!");
  assert(RDDRef->isFake() && "RDDRef is not a fake DDRef!");
  assert((Node = RDDRef->getHLDDNode()) && isa<HLInst>(Node) &&
         (cast<HLInst>(Node) == this) &&
         "RDDRef does not belong to this HLInst!");

  for (auto I = fake_ddref_begin(), E = fake_ddref_end(); I != E; I++) {
    if ((*I) == RDDRef) {
      setNode(RDDRef, nullptr);
      DDRefs.erase(I);
      return;
    }
  }

  llvm_unreachable("Unexpected condition!");
}

unsigned HLInst::getNumOperands() const { return getNumOperandsInternal(); }

unsigned HLInst::getNumOperandsInternal() const {
  unsigned NumOp = Inst->getNumOperands();

  if (hasLval() && !isa<StoreInst>(Inst)) {
    NumOp++;
  }

  return NumOp;
}
