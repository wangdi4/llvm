//===--- HLDDNode.cpp - Implements the HLDDNode class ---------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements the HLDDNode class.
//
//===----------------------------------------------------------------------===//

#include "llvm/Support/Debug.h"

#include "llvm/Transforms/Intel_LoopTransforms/Utils/HLNodeUtils.h"
#include "llvm/IR/Intel_LoopIR/DDRef.h"
#include "llvm/IR/Intel_LoopIR/BlobDDRef.h"

using namespace llvm;
using namespace llvm::loopopt;

/// DDRefs are taken care of in the derived classes.
HLDDNode::HLDDNode(unsigned SCID) : HLNode(SCID) {}

/// DDRefs are taken care of in the derived classes.
HLDDNode::HLDDNode(const HLDDNode &HLDDNodeObj) : HLNode(HLDDNodeObj) {}

void HLDDNode::resizeDDRefsToNumOperands() {
  DDRefs.resize(getNumOperands(), nullptr);
}

void HLDDNode::setNode(DDRef *Ref, HLDDNode *HNode) { Ref->setHLDDNode(HNode); }

HLDDNode::ddref_iterator HLDDNode::ddref_begin() {
  HLLoop *HLoop;

  /// Skip null DDRefs for unknown loops
  if ((HLoop = dyn_cast<HLLoop>(this)) && HLoop->isUnknownLoop()) {
    return DDRefs.end();
  }
  return DDRefs.begin();
}

HLDDNode::const_ddref_iterator HLDDNode::ddref_begin() const {
  return const_cast<HLDDNode *>(this)->ddref_begin();
}

HLDDNode::ddref_iterator HLDDNode::ddref_end() { return DDRefs.end(); }

HLDDNode::const_ddref_iterator HLDDNode::ddref_end() const {
  return const_cast<HLDDNode *>(this)->ddref_end();
}

HLDDNode::reverse_ddref_iterator HLDDNode::ddref_rbegin() {
  HLLoop *HLoop;

  /// Skip null DDRefs for unknown loops
  if ((HLoop = dyn_cast<HLLoop>(this)) && HLoop->isUnknownLoop()) {
    return DDRefs.rend();
  }
  return DDRefs.rbegin();
}

HLDDNode::const_reverse_ddref_iterator HLDDNode::ddref_rbegin() const {
  return const_cast<HLDDNode *>(this)->ddref_rbegin();
}

HLDDNode::reverse_ddref_iterator HLDDNode::ddref_rend() {
  return DDRefs.rend();
}

HLDDNode::const_reverse_ddref_iterator HLDDNode::ddref_rend() const {
  return const_cast<HLDDNode *>(this)->ddref_rend();
}

DDRef *HLDDNode::getOperandDDRefImpl(unsigned OperandNum) const {
  return DDRefs[OperandNum];
}

DDRef *HLDDNode::getOperandDDRef(unsigned OperandNum) {
  assert(OperandNum < getNumOperands() && "Operand is out of range!");
  assert(!isa<HLLoop>(this) && "Please use loop specific"
                               " utility!");

  return getOperandDDRefImpl(OperandNum);
}

const DDRef *HLDDNode::getOperandDDRef(unsigned OperandNum) const {
  return const_cast<HLDDNode *>(this)->getOperandDDRef(OperandNum);
}

void HLDDNode::setOperandDDRefImpl(DDRef *Ref, unsigned OperandNum) {
  assert((!Ref || !isa<BlobDDRef>(Ref)) && "Cannot associate blob DDRef with "
                                           "operand!");
  /// Reset HLDDNode of a previous DDRef, if any. We can catch more errors
  /// this way.
  /// TODO: Do this only in debug mode to save compile time.
  if (auto TRef = DDRefs[OperandNum]) {
    setNode(TRef, nullptr);
  }

  if (Ref) {
    assert(!Ref->getHLDDNode() && "DDRef attached to some other node, please "
                                  "remove it first!");
    setNode(Ref, this);
  }

  DDRefs[OperandNum] = Ref;
}

void HLDDNode::setOperandDDRef(DDRef *Ref, unsigned OperandNum) {
  assert(OperandNum < getNumOperands() && "Operand is out of range!");
  assert(!isa<HLLoop>(this) && "Please use loop specific utility!");

  setOperandDDRefImpl(Ref, OperandNum);
}

DDRef *HLDDNode::removeOperandDDRef(unsigned OperandNum) {
  auto TRef = getOperandDDRef(OperandNum);

  if (TRef) {
    setOperandDDRef(nullptr, OperandNum);
  }

  return TRef;
}
