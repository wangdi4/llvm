//===-------- HLGoto.cpp - Implements the HLGoto class --------------------===//
//
// Copyright (C) 2015-2016 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// This file implements the HLGoto class.
//
//===----------------------------------------------------------------------===//

#include "llvm/IR/BasicBlock.h"

#include "llvm/IR/Intel_LoopIR/HLGoto.h"
#include "llvm/IR/Intel_LoopIR/HLLabel.h"

using namespace llvm;
using namespace llvm::loopopt;

HLGoto::HLGoto(BasicBlock *TargetBB)
    : HLNode(HLNode::HLGotoVal), TargetBBlock(TargetBB), TargetLabel(nullptr) {}

HLGoto::HLGoto(HLLabel *TargetL)
    : HLNode(HLNode::HLGotoVal), TargetBBlock(nullptr), TargetLabel(TargetL) {}

HLGoto::HLGoto(const HLGoto &HLGotoObj)
    : HLNode(HLGotoObj), TargetBBlock(HLGotoObj.TargetBBlock),
      TargetLabel(HLGotoObj.TargetLabel) {}

HLGoto *HLGoto::cloneImpl(GotoContainerTy *GotoList,
                          LabelMapTy *LabelMap) const {

  // Call Copy constructor
  HLGoto *NewHLGoto = new HLGoto(*this);

  // Add the new goto into the list
  if (GotoList && !NewHLGoto->isExternal())
    GotoList->push_back(NewHLGoto);

  return NewHLGoto;
}

HLGoto *HLGoto::clone() const { return cloneImpl(nullptr, nullptr); }

void HLGoto::print(formatted_raw_ostream &OS, unsigned Depth,
                   bool Detailed) const {

  indent(OS, Depth);

  OS << "goto ";

  if (TargetLabel) {
    OS << TargetLabel->getName();
  } else {
    HLLabel::printBBlockName(OS, *TargetBBlock);
  }

  OS << ";\n";
}

void HLGoto::verify() const {
  assert(((!TargetBBlock && TargetLabel) || (TargetBBlock && !TargetLabel)) &&
         "One and only one TargetBBlock or TargetLabel should be non-NULL");
  HLNode::verify();
}
