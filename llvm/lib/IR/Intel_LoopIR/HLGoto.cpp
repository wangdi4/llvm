//===-------- HLGoto.cpp - Implements the HLGoto class --------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements the HLGoto class.
//
//===----------------------------------------------------------------------===//

#include "llvm/IR/Intel_LoopIR/HLGoto.h"

using namespace llvm;
using namespace llvm::loopopt;

HLGoto::HLGoto(HLNode* Par, BasicBlock* TargetBB,
  HLLabel* TargetL)
  : HLDDNode(HLNode::HLGotoVal, Par), TargetBBlock(TargetBB)
  , TargetLabel(TargetL) { }

HLGoto::HLGoto(const HLGoto &HLGotoObj)
  : HLDDNode(HLGotoObj), TargetBBlock(HLGotoObj.TargetBBlock),
    TargetLabel(HLGotoObj.TargetLabel){ }

HLGoto* HLGoto::clone() const {

  /// Check for 'this' as null
  assert(this && " HLGoto cannot be null");

  /// Call Copy constructor
  HLGoto *NewHLGoto = new HLGoto(*this);

  return NewHLGoto;
}

