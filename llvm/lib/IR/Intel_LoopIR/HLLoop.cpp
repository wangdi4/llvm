//===-------- HLLoop.cpp - Implements the HLLoop class --------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements the HLLoop class.
//
//===----------------------------------------------------------------------===//

#include "llvm/IR/Intel_LoopIR/HLLoop.h"

using namespace llvm;
using namespace llvm::loopopt;

HLLoop::HLLoop(HLNode* Par, HLIf* ZttIf, bool isDoWh, unsigned NumEx)
  : HLDDNode(HLNode::HLLoopVal, Par), Ztt(ZttIf), isDoWhile(isDoWh)
  , NumExits(NumEx) { }


HLLoop* HLLoop::clone_impl() const {
  // TODO: placeholder, implement later
  return nullptr;
}

