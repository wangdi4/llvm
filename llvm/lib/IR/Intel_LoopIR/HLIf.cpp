//===------------ HLIf.cpp - Implements the HLIf class --------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements the HLIf class.
//
//===----------------------------------------------------------------------===//

#include "llvm/IR/Intel_LoopIR/HLIf.h"

using namespace llvm;
using namespace llvm::loopopt;

HLIf::HLIf(HLNode* Par)
  : HLDDNode(HLNode::HLIfVal, Par) { }


HLIf* HLIf::clone_impl() const {
  // TODO: placeholder, implement later
  return nullptr;
}

