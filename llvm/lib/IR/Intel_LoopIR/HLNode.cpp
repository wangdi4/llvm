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
#include "llvm/IR/Intel_LoopIR/HLRegion.h"
#include "llvm/IR/Intel_LoopIR/HLLoop.h"

using namespace llvm;
using namespace llvm::loopopt;

//using HLRegions() is considered another declaration. very odd
HLContainerTy llvm::loopopt::HLRegions;

std::set< HLNode* >HLNode::Objs;

HLNode::HLNode(unsigned SCID)
  : SubClassID(SCID), Parent(nullptr) {

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

HLLoop* HLNode::getParentLoop() const {
  assert( !isa<HLRegion>(this) && "Region cannot have a parent loop");

  HLNode* Par = getParent();

  while(Par && !isa<HLLoop>(Par)) {
    Par = Par->getParent();
  } 

  return cast_or_null<HLLoop>(Par);
}
