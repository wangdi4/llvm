//===--- HLDDNode.cpp - Implements the HLDDNode class ---------------------===//
//
// Copyright (C) 2015 Intel Corporation. All rights reserved.
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

#include "llvm/Transforms/Intel_LoopTransforms/Utils/HLNodeUtils.h"
#include "llvm/IR/Intel_LoopIR/DDRef.h"
#include "llvm/IR/Intel_LoopIR/RegDDRef.h"
#include "llvm/IR/Intel_LoopIR/BlobDDRef.h"

using namespace llvm;
using namespace llvm::loopopt;

/// DDRefs are taken care of in the derived classes.
HLDDNode::HLDDNode(unsigned SCID) : HLNode(SCID) {}

/// DDRefs are taken care of in the derived classes.
HLDDNode::HLDDNode(const HLDDNode &HLDDNodeObj) : HLNode(HLDDNodeObj) {}

void HLDDNode::setNode(RegDDRef *Ref, HLDDNode *HNode) {
  Ref->setHLDDNode(HNode);
}

HLDDNode::ddref_iterator HLDDNode::ddref_begin() {
  HLLoop *HLoop;

  /// Skip null DDRefs for unknown loops
  if ((HLoop = dyn_cast<HLLoop>(this)) && HLoop->isUnknown()) {
    return RegDDRefs.end();
  }
  return RegDDRefs.begin();
}

HLDDNode::const_ddref_iterator HLDDNode::ddref_begin() const {
  return const_cast<HLDDNode *>(this)->ddref_begin();
}

HLDDNode::ddref_iterator HLDDNode::ddref_end() { return RegDDRefs.end(); }

HLDDNode::const_ddref_iterator HLDDNode::ddref_end() const {
  return const_cast<HLDDNode *>(this)->ddref_end();
}

HLDDNode::reverse_ddref_iterator HLDDNode::ddref_rbegin() {
  HLLoop *HLoop;

  /// Skip null DDRefs for unknown loops
  if ((HLoop = dyn_cast<HLLoop>(this)) && HLoop->isUnknown()) {
    return RegDDRefs.rend();
  }
  return RegDDRefs.rbegin();
}

HLDDNode::const_reverse_ddref_iterator HLDDNode::ddref_rbegin() const {
  return const_cast<HLDDNode *>(this)->ddref_rbegin();
}

HLDDNode::reverse_ddref_iterator HLDDNode::ddref_rend() {
  return RegDDRefs.rend();
}

HLDDNode::const_reverse_ddref_iterator HLDDNode::ddref_rend() const {
  return const_cast<HLDDNode *>(this)->ddref_rend();
}

RegDDRef *HLDDNode::getOperandDDRefImpl(unsigned OperandNum) const {
  return RegDDRefs[OperandNum];
}

void HLDDNode::setOperandDDRefImpl(RegDDRef *Ref, unsigned OperandNum) {

#ifndef NDEBUG
  /// Reset HLDDNode of a previous DDRef, if any. We can catch more errors
  /// this way.
  if (auto TRef = RegDDRefs[OperandNum]) {
    setNode(TRef, nullptr);
  }
#endif

  if (Ref) {
    assert(!Ref->getHLDDNode() && "DDRef attached to some other node, please "
                                  "remove it first!");
    setNode(Ref, this);
  }

  RegDDRefs[OperandNum] = Ref;
}

void HLDDNode::print(formatted_raw_ostream &OS, unsigned Depth,
                     bool Detailed) const {
  if (Detailed) {
    printDDRefs(OS, Depth);
  }
}

void HLDDNode::printDDRefs(formatted_raw_ostream &OS, unsigned Depth) const {
  bool printed = false;
  bool isLoop = false;

  // DD refs attached to Loop nodes require additional
  // "|" symbol to make listing nice
  if (isa<HLLoop>(this)) {
    isLoop = true;
  }

  for (auto I = ddref_begin(), E = ddref_end(); I != E; ++I) {
    if ((*I) == nullptr || (*I)->isConstant()) {
      continue;
    }

    indent(OS, Depth);

    if (isLoop) {
      OS << "| ";
    }
    OS << "<REG> ";
    (*I)->print(OS, true);
    OS << "\n";
    for (auto B = (*I)->blob_cbegin(), BE = (*I)->blob_cend(); B != BE; ++B) {
      indent(OS, Depth);
      if (isLoop) {
        OS << "| ";
      }
      OS << "<BLOB> ";
      (*B)->print(OS, true);
      OS << "\n";
    }

    printed = true;
  }

  if (printed) {
    indent(OS, Depth);
    if (isLoop) {
      OS << "| ";
    }
    OS << "\n";
  }
}

void HLDDNode::verify() const {
  for (auto I = ddref_begin(), E = ddref_end(); I != E; ++I) {
    if (!*I) {
      continue;
    }

    assert((*I)->getHLDDNode() == this &&
           "DDRef is attached to a different node");
    (*I)->verify();
  }

  HLNode::verify();
}
