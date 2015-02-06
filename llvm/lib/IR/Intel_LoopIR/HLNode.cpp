//===-- HLNode.cpp - Implements the HLNode class ---------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements the HLNode class.
//
//===----------------------------------------------------------------------===//

#include "llvm/IR/Intel_LoopIR/HLNode.h"
using namespace llvm;
using namespace llvm::loopopt;

//using HLRegions() is considered another declaration. very odd
HLContainerTy llvm::loopopt::HLRegions;

std::set< HLNode* >HLNode::Objs;

HLNode::HLNode(unsigned SCID, HLNode* Par)
  : SubClassID(SCID), Parent(Par) {

  Objs.insert(this);
}

HLNode::HLNode(const HLNode &HLNodeObj)
  : SubClassID(HLNodeObj.SubClassID), Parent(nullptr) {

  Objs.insert(this);
}

void HLNode::destroy() {
  Objs.erase(this);
  delete this;
}

void HLNode::destroyAll() {

  for (auto &I : Objs) {
      delete I;
  }

  Objs.clear();
}

