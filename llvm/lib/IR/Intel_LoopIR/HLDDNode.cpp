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
    : HLNode(HNU, SCID), MaskDDRef(nullptr) {}

/// DDRefs are taken care of in the derived classes.
HLDDNode::HLDDNode(const HLDDNode &HLDDNodeObj)
    : HLNode(HLDDNodeObj), MaskDDRef(nullptr) {}

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

  for (auto I = fake_ddref_begin(), E = fake_ddref_end(); I != E; ++I) {

    if ((*I) == Ref) {
      return true;
    }
  }

  return false;
}

void HLDDNode::setMaskDDRef(RegDDRef *Ref) {
  if (MaskDDRef) {
    removeFakeDDRef(MaskDDRef);
  }

  MaskDDRef = Ref;

  if (Ref) {
    addFakeDDRef(Ref);
  }
}

void HLDDNode::addFakeDDRef(RegDDRef *RDDRef) {
  assert(RDDRef && "Cannot add null fake DDRef!");
  assert(isa<HLInst>(this) && "Fake DDRef can only be attached to a HLInst!");
  assert(!RDDRef->getHLDDNode() && "DDRef attached to some other node, please "
                                   "remove it first!");

  RegDDRefs.push_back(RDDRef);
  setNode(RDDRef, this);
}

void HLDDNode::removeFakeDDRef(RegDDRef *RDDRef) {
  assert(RDDRef && "Cannot remove null fake DDRef!");
  assert(RDDRef->isFake() && "RDDRef is not a fake DDRef!");
  assert(isa<HLInst>(this) && "Fake DDRef can only be attached to a HLInst!");
  assert((this == RDDRef->getHLDDNode()) &&
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
  assert(NewRef && "NewRef is null!");

  removeFakeDDRef(ExistingRef);
  addFakeDDRef(NewRef);
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
#if !INTEL_PRODUCT_RELEASE
  if (Detailed) {
    printDDRefs(OS, Depth);
  }
#endif // !INTEL_PRODUCT_RELEASE
}

void HLDDNode::printDDRefs(formatted_raw_ostream &OS, unsigned Depth) const {
#if !INTEL_PRODUCT_RELEASE
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
#endif // !INTEL_PRODUCT_RELEASE
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

