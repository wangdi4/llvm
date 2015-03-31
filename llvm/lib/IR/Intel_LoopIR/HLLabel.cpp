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

#include "llvm/IR/BasicBlock.h"

#include "llvm/IR/Intel_LoopIR/HLLabel.h"

using namespace llvm;
using namespace llvm::loopopt;

HLLabel::HLLabel(BasicBlock *SrcBB)
    : HLNode(HLNode::HLLabelVal), SrcBBlock(SrcBB) {}

HLLabel::HLLabel(const HLLabel &LabelObj)
    : HLNode(LabelObj), SrcBBlock(LabelObj.SrcBBlock) {}

HLLabel *HLLabel::clone() const {

  /// Call Copy constructor
  HLLabel *NewHLLabel = new HLLabel(*this);

  return NewHLLabel;
}

void HLLabel::print(formatted_raw_ostream &OS, unsigned Depth) const {
  indent(OS, Depth);
  OS << SrcBBlock->getName() << ":\n";
}
