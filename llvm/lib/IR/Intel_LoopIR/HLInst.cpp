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

#include "llvm/IR/Intel_LoopIR/HLInst.h"

using namespace llvm;
using namespace llvm::loopopt;


HLInst::HLInst(HLNode* Par, Instruction* In)
  : HLDDNode(HLNode::HLInstVal, Par), Inst(In)
  , SafeRednSucc(nullptr) { }


HLInst* HLInst::clone_impl() const {
  // TODO: placeholder, implement later
  return nullptr;
}
