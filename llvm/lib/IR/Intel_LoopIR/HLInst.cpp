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

HLInst::HLInst(const HLInst &HLInstObj)
  : HLDDNode(HLInstObj), SafeRednSucc(nullptr) {

  /// Clone the LLVM Instruction
  assert(HLInstObj.Inst && " LLVM Instruction for HLInst cannot be null");
  Inst = HLInstObj.Inst->clone();
}

HLInst* HLInst::clone() const {

  /// Call the Copy Constructor
  HLInst *NewHLInst = new HLInst(*this);

  return NewHLInst;
}
