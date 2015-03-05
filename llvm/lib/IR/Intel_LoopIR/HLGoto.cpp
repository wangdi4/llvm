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

HLGoto::HLGoto(BasicBlock *TargetBB, HLLabel *TargetL)
    : HLNode(HLNode::HLGotoVal), TargetBBlock(TargetBB), TargetLabel(TargetL) {}

HLGoto::HLGoto(const HLGoto &HLGotoObj)
    : HLNode(HLGotoObj), TargetBBlock(HLGotoObj.TargetBBlock),
      TargetLabel(HLGotoObj.TargetLabel) {}

HLGoto *HLGoto::clone() const {

  /// Call Copy constructor
  HLGoto *NewHLGoto = new HLGoto(*this);

  return NewHLGoto;
}
