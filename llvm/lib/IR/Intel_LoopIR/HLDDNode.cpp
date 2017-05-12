//===--- HLDDNode.cpp - Implements the HLDDNode class ---------------------===//
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
// This file implements the HLDDNode class.
//
//===----------------------------------------------------------------------===//

#include "llvm/Support/Debug.h"

#include "llvm/IR/Intel_LoopIR/BlobDDRef.h"
#include "llvm/IR/Intel_LoopIR/DDRef.h"
#include "llvm/IR/Intel_LoopIR/HLLoop.h"
#include "llvm/IR/Intel_LoopIR/RegDDRef.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/HLNodeUtils.h"

using namespace llvm;
using namespace llvm::loopopt;

static cl::opt<bool>
    PrintConstDDRefs("hir-details-constants", cl::init(false), cl::Hidden,
                     cl::desc("Print constant DDRefs in detailed print"));

/// DDRefs are taken care of in the derived classes.
HLDDNode::HLDDNode(HLNodeUtils &HNU, unsigned SCID)
    : HLNode(HNU, SCID), MaskDDRef(nullptr), NumFakeLvals(0) {}

/// DDRefs are taken care of in the derived classes.
HLDDNode::HLDDNode(const HLDDNode &HLDDNodeObj)
    : HLNode(HLDDNodeObj), MaskDDRef(nullptr),
      NumFakeLvals(HLDDNodeObj.NumFakeLvals) {}

void HLDDNode::setNode(RegDDRef *Ref, HLDDNode *HNode) {
  Ref->setHLDDNode(HNode);
}

RegDDRef *HLDDNode::getOperandDDRefImpl(unsigned OperandNum) const {
  return RegDDRefs[OperandNum];
}

void HLDDNode::setOperandDDRefImpl(RegDDRef *Ref, unsigned OperandNum) {

  /// Reset HLDDNode of a previous DDRef, if any. We can catch more errors
  /// this way.
  if (auto TRef = RegDDRefs[OperandNum]) {
    setNode(TRef, nullptr);
  }

  if (Ref) {
    assert(!Ref->getHLDDNode() && "DDRef attached to some other node, please "
                                  "remove it first!");
    setNode(Ref, this);
  }

  RegDDRefs[OperandNum] = Ref;
}

RegDDRef *HLDDNode::getOperandDDRef(unsigned OperandNum) {
  assert(OperandNum < getNumOperands() && "Operand is out of range!");
  return getOperandDDRefImpl(OperandNum);
}

const RegDDRef *HLDDNode::getOperandDDRef(unsigned OperandNum) const {
  return const_cast<HLDDNode *>(this)->getOperandDDRef(OperandNum);
}

void HLDDNode::setOperandDDRef(RegDDRef *Ref, unsigned OperandNum) {
  assert(OperandNum < getNumOperands() && "Operand is out of range!");
  setOperandDDRefImpl(Ref, OperandNum);
}

RegDDRef *HLDDNode::removeOperandDDRef(unsigned OperandNum) {
  auto TRef = getOperandDDRef(OperandNum);

  if (TRef) {
    setOperandDDRef(nullptr, OperandNum);
  }

  return TRef;
}

unsigned HLDDNode::getOperandNum(RegDDRef *OpRef) const {
  assert(OpRef && "OpRef is null!");
  assert((this == OpRef->getHLDDNode()) &&
         "OpRef does not belong to this HLDDNode!");
  assert(!OpRef->isFake() && "OpRef is a fake DDRef!");

  unsigned OpNum = 0;

  for (auto It = op_ddref_begin(), EndIt = op_ddref_end(); It != EndIt;
       ++It, ++OpNum) {
    if (*It == OpRef) {
      return OpNum;
    }
  }

  llvm_unreachable("Did not find OpRef in the operands!");
}

bool HLDDNode::isFake(const RegDDRef *Ref) const {
  assert((this == Ref->getHLDDNode()) && "Ref does not belong to this node!");

  auto It = std::find(fake_ddref_begin(), fake_ddref_end(), Ref);

  return (It != fake_ddref_end());
}

bool HLDDNode::isFakeLval(const RegDDRef *Ref) const {
  assert((this == Ref->getHLDDNode()) && "Ref does not belong to this node!");

  auto It = std::find(lval_fake_ddref_begin(), lval_fake_ddref_end(), Ref);

  return (It != lval_fake_ddref_end());
}

bool HLDDNode::isFakeRval(const RegDDRef *Ref) const {
  assert((this == Ref->getHLDDNode()) && "Ref does not belong to this node!");

  auto It = std::find(rval_fake_ddref_begin(), rval_fake_ddref_end(), Ref);

  return (It != rval_fake_ddref_end());
}

void HLDDNode::setMaskDDRef(RegDDRef *Ref) {
  if (MaskDDRef) {
    removeFakeDDRef(MaskDDRef);
  }

  MaskDDRef = Ref;

  if (Ref) {
    addFakeRvalDDRef(Ref);
  }
}

void HLDDNode::addFakeLvalDDRef(RegDDRef *Ref) {
  assert(Ref && "Cannot add null fake DDRef!");
  assert(isa<HLInst>(this) && "Fake DDRef can only be attached to a HLInst!");
  assert(!Ref->getHLDDNode() && "DDRef attached to some other node, please "
                                "remove it first!");

  if (hasFakeRvalDDRefs()) {
    // Push the first fake rval ref to the back and move Ref in its place
    // essentially swapping them out.
    RegDDRefs.push_back(*rval_fake_ddref_begin());
    *rval_fake_ddref_begin() = Ref;
  } else {
    RegDDRefs.push_back(Ref);
  }

  ++NumFakeLvals;
  setNode(Ref, this);
}

void HLDDNode::addFakeRvalDDRef(RegDDRef *Ref) {
  assert(Ref && "Cannot add null fake DDRef!");
  assert(isa<HLInst>(this) && "Fake DDRef can only be attached to a HLInst!");
  assert(!Ref->getHLDDNode() && "DDRef attached to some other node, please "
                                "remove it first!");

  RegDDRefs.push_back(Ref);
  setNode(Ref, this);
}

void HLDDNode::removeFakeDDRef(RegDDRef *Ref) {
  assert(Ref && "Cannot remove null fake DDRef!");
  assert(Ref->isFake() && "RDDRef is not a fake DDRef!");
  assert(isa<HLInst>(this) && "Fake DDRef can only be attached to a HLInst!");
  assert((this == Ref->getHLDDNode()) &&
         "RDDRef does not belong to this HLInst!");

  bool IsLval = isFakeLval(Ref);

  for (auto I = fake_ddref_begin(), E = fake_ddref_end(); I != E; I++) {
    if ((*I) == Ref) {
      setNode(Ref, nullptr);
      RegDDRefs.erase(I);
      if (IsLval) {
        --NumFakeLvals;
      }
      return;
    }
  }

  llvm_unreachable("Unexpected condition!");
}

void HLDDNode::replaceOperandDDRef(RegDDRef *ExistingRef, RegDDRef *NewRef) {
  assert(ExistingRef && "ExistingRef is null!");
  assert(NewRef && "NewRef is null!");
  unsigned OpNum = 0;

  for (auto RefIt = op_ddref_begin(), EndIt = op_ddref_end(); RefIt != EndIt;
       ++RefIt, ++OpNum) {
    if (*RefIt == ExistingRef) {
      setOperandDDRef(NewRef, OpNum);
      return;
    }
  }

  llvm_unreachable("ExistingRef not found!");
}

void HLDDNode::replaceFakeDDRef(RegDDRef *ExistingRef, RegDDRef *NewRef) {
  assert(ExistingRef && "ExistingRef is null!");
  assert(ExistingRef->isFake() && "ExistingRef is not a fake DDRef!");
  assert(NewRef && "NewRef is null!");
  assert(!NewRef->getHLDDNode() && "DDRef attached to some other node, please "
                                   "remove it first!");

  auto It = std::find(fake_ddref_begin(), fake_ddref_end(), ExistingRef);
  assert(It != fake_ddref_end() && "ExistingRef not found!");

  setNode(ExistingRef, nullptr);

  *It = NewRef;
}

void HLDDNode::replaceOperandOrFakeDDRef(RegDDRef *ExistingRef,
                                         RegDDRef *NewRef) {
  assert(ExistingRef && "ExistingRef is null!");
  assert(NewRef && "NewRef is null!");

  if (ExistingRef->isFake()) {
    replaceFakeDDRef(ExistingRef, NewRef);
  } else {
    replaceOperandDDRef(ExistingRef, NewRef);
  }
}

void HLDDNode::print(formatted_raw_ostream &OS, unsigned Depth,
                     bool Detailed) const {
  if (Detailed) {
    printDDRefs(OS, Depth);
  }
}

void HLDDNode::printDDRefs(formatted_raw_ostream &OS, unsigned Depth) const {
  bool Printed = false;
  bool IsLoop = false;

  // DD refs attached to Loop nodes require additional
  // "|" symbol to make listing nice
  if (isa<HLLoop>(this)) {
    IsLoop = true;
  }

  for (auto I = ddref_begin(), E = ddref_end(); I != E; ++I) {
    // Simply checking for isConstant() also filters out lval DDRefs whose
    // canonical representation is a constant. We should print out lval DDRefs
    // regardless.
    if ((*I) && !PrintConstDDRefs && ((*I)->getSymbase() == ConstantSymbase)) {
      continue;
    }

    bool IsZttDDRef = false;
    indent(OS, Depth);

    if (IsLoop) {
      OS << "| ";

      IsZttDDRef = *I ? cast<HLLoop>(this)->isZttOperandDDRef(*I) : false;
    }

    if (IsZttDDRef) {
      OS << "<ZTT-REG> ";
    } else {
      bool IsFake = false;
      bool IsLval = false;

      if (*I) {
        IsFake = isFake(*I);
        IsLval = isLval(*I);
      }

      OS << "<" << (IsFake ? "FAKE-" : "") << (IsLval ? "LVAL" : "RVAL")
         << "-REG> ";
    }

    (*I) ? (*I)->print(OS, true) : (void)(OS << *I);

    OS << "\n";

    if (*I) {
      for (auto B = (*I)->blob_cbegin(), BE = (*I)->blob_cend(); B != BE; ++B) {
        indent(OS, Depth);
        if (IsLoop) {
          OS << "| ";
        }

        // Add extra indentation for blob ddrefs.
        OS.indent(IndentWidth);

        OS << "<BLOB> ";
        (*B)->print(OS, true);
        OS << "\n";
      }
    }

    Printed = true;
  }

  if (Printed) {
    indent(OS, Depth);
    if (IsLoop) {
      OS << "| ";
    }
    OS << "\n";
  }
}

void HLDDNode::verify() const {
  for (auto I = ddref_begin(), E = ddref_end(); I != E; ++I) {
    assert((*I) != nullptr && "null ddref found in the list");
    assert((*I)->getHLDDNode() == this &&
           "DDRef is attached to a different node");
    (*I)->verify();
  }

  HLNode::verify();
}

bool HLDDNode::isLiveIntoParentLoop(unsigned SB) const {
  return getLexicalParentLoop()->isLiveIn(SB);
}

bool HLDDNode::isLiveOutOfParentLoop(unsigned SB) const {
  return getLexicalParentLoop()->isLiveOut(SB);
}

