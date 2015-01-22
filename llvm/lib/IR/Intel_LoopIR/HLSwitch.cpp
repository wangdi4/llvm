//===---- HLSwitch.cpp - Implements the HLSwitch class --------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements the HLSwitch class.
//
//===----------------------------------------------------------------------===//

#include "llvm/IR/Intel_LoopIR/HLSwitch.h"

using namespace llvm;
using namespace llvm::loopopt;


HLSwitch::HLSwitch(HLNode* Par)
  : HLDDNode(HLNode::HLSwitchVal, Par) { }


HLSwitch* HLSwitch::clone_impl() const {
  // TODO: placeholder, implement later
  return nullptr;
}

