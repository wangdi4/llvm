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
#include "llvm/IR/Intel_LoopIR/HLIf.h"
#include "llvm/IR/Intel_LoopIR/HLLabel.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/HLNodeUtils.h"

using namespace llvm;
using namespace llvm::loopopt;

HLGoto::HLGoto(HLNodeUtils &HNU, BasicBlock *TargetBB)
    : HLNode(HNU, HLNode::HLGotoVal), TargetBBlock(TargetBB),
      TargetLabel(nullptr) {}

HLGoto::HLGoto(HLNodeUtils &HNU, HLLabel *TargetL)
    : HLNode(HNU, HLNode::HLGotoVal), TargetBBlock(nullptr),
      TargetLabel(TargetL) {}

HLGoto::HLGoto(const HLGoto &HLGotoObj)
    : HLNode(HLGotoObj), TargetBBlock(HLGotoObj.TargetBBlock),
      TargetLabel(HLGotoObj.TargetLabel) {}

HLGoto *HLGoto::cloneImpl(GotoContainerTy *GotoList, LabelMapTy *LabelMap,
                          HLNodeMapper *NodeMapper) const {

  // Call Copy constructor
  HLGoto *NewHLGoto = new HLGoto(*this);

  // Add the new goto into the list
  if (GotoList && !NewHLGoto->isExternal())
    GotoList->push_back(NewHLGoto);

  return NewHLGoto;
}

HLGoto *HLGoto::clone(HLNodeMapper *NodeMapper) const {
  return cast<HLGoto>(
      HLNode::cloneBaseImpl(this, nullptr, nullptr, NodeMapper));
}

void HLGoto::print(formatted_raw_ostream &OS, unsigned Depth,
                   bool Detailed) const {
#if !INTEL_PRODUCT_RELEASE
  auto ParentIf = dyn_cast<HLIf>(getParent());

  // ddref_begin() check is a workaround to skip the backedge check until we
  // form ddrefs in the framework otherwise we encounter an assert for null
  // stride ref.
  if (ParentIf && (*ParentIf->ddref_begin()) && isUnknownLoopBackEdge()) {
    // Print IV update for the unknown loop which will be generated during code
    // gen.
    auto Level = getParentLoop()->getNestingLevel();
    indent(OS, Depth);
    OS << "<i" << Level << " = "
       << "i" << Level << " + 1>\n";
  }
  
  indent(OS, Depth);
  OS << "goto ";

  if (TargetLabel) {
    OS << TargetLabel->getName();
  } else {
    HLLabel::printBBlockName(OS, *TargetBBlock);
  }

  OS << ";\n";
#endif // !INTEL_PRODUCT_RELEASE
}

bool HLGoto::isUnknownLoopBackEdge() const {
  auto ParentIf = dyn_cast<HLIf>(getParent());

  if (!ParentIf) {
    return false;
  }

  return ParentIf->isUnknownLoopBottomTest();
}

void HLGoto::verify() const {
  assert(((!TargetBBlock && TargetLabel) || (TargetBBlock && !TargetLabel)) &&
         "One and only one TargetBBlock or TargetLabel should be non-NULL");

  HLNode *Parent = getParent();

// TODO: try enabling this  once we start removing redundant nodes afte goto.
//  assert((this == getHLNodeUtils().getLastLexicalChild(Parent, this)) &&
//         "Dead nodes encountered!");

  if (TargetLabel) {
    assert((isUnknownLoopBackEdge() ||
            (TargetLabel->getTopSortNum() > getTopSortNum())) &&
           "backward jump encountered in HIR!");

    HLIf *IfParent = nullptr;

    while (Parent && (IfParent = dyn_cast<HLIf>(Parent))) {
      if (IfParent->isThenChild(this)) {
        assert(!IfParent->isElseChild(TargetLabel) &&
               "Jump from then to else case encountered!");
      }
      Parent = Parent->getParent();
    }
  }

  HLNode::verify();
}
