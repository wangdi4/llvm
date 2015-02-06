//===----- HLiLabel.cpp - Implements the HLLabel class --------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements the HLLabel class.
//
//===----------------------------------------------------------------------===//

#include "llvm/IR/Intel_LoopIR/HLLabel.h"

using namespace llvm;
using namespace llvm::loopopt;


HLLabel::HLLabel(HLNode* Par, BasicBlock* SrcBB)
  : HLDDNode(HLNode::HLLabelVal, Par), SrcBBlock(SrcBB) { }

HLLabel::HLLabel(const HLLabel &LabelObj)
  : HLDDNode(LabelObj), SrcBBlock(LabelObj.SrcBBlock) { }

HLLabel* HLLabel::clone() const {

  /// Check for 'this' as null
  assert(this && " HLLabel cannot be null");

  /// Call Copy constructor
  HLLabel *NewHLLabel = new HLLabel(*this);

  return NewHLLabel;
}


