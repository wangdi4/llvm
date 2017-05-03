//===------ HLLabel.cpp - Implements the HLLabel class --------------------===//
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
// This file implements the HLLabel class.
//
//===----------------------------------------------------------------------===//

#include "llvm/IR/BasicBlock.h"

#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"

#include "llvm/IR/Intel_LoopIR/HLLabel.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/HLNodeUtils.h"

using namespace llvm;
using namespace llvm::loopopt;


void HLLabel::makeNameUnique() {
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  bool is_uniq = (SrcBBlock != nullptr); // Treat BasicBlock name as unique
  do {
    if (!is_uniq) {
      raw_svector_ostream(Name) << "." << getNumber();
    }

    is_uniq = getHLNodeUtils().LabelNames.insert(Name).second;
  } while (!is_uniq);
#endif
}

HLLabel::HLLabel(HLNodeUtils &HNU, BasicBlock *SrcBB)
    : HLNode(HNU, HLNode::HLLabelVal), SrcBBlock(SrcBB) {
  assert(SrcBB != nullptr && "SrcBB must not be NULL");

  {
    raw_svector_ostream OS(this->Name);
    printBBlockName(OS, *SrcBB);
  }

  makeNameUnique();
}

HLLabel::HLLabel(HLNodeUtils &HNU, const Twine &Name)
    : HLNode(HNU, HLNode::HLLabelVal), SrcBBlock(nullptr) {

  {
    raw_svector_ostream OS(this->Name);
    Name.print(OS);
  }

  makeNameUnique();
}

HLLabel::HLLabel(const HLLabel &LabelObj)
    : HLNode(LabelObj), SrcBBlock(nullptr), Name(LabelObj.Name) {

  makeNameUnique();
}

HLLabel::~HLLabel() {
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  getHLNodeUtils().LabelNames.erase(getName());
#endif
}

HLLabel *HLLabel::cloneImpl(GotoContainerTy *GotoList, LabelMapTy *LabelMap,
                            HLNodeMapper *NodeMapper) const {
  // Call Copy constructor
  HLLabel *NewHLLabel = new HLLabel(*this);

  if (LabelMap) {
    LabelMap->insert(std::pair<const HLLabel *, HLLabel *>(this, NewHLLabel));
  }

  return NewHLLabel;
}

HLLabel *HLLabel::clone(HLNodeMapper *NodeMapper) const {
  return cast<HLLabel>(cloneBaseImpl(this, nullptr, nullptr, NodeMapper));
}

void HLLabel::print(formatted_raw_ostream &OS, unsigned Depth,
                    bool Detailed) const {

  auto ParentLoop = dyn_cast<HLLoop>(getParent());

  // ddref_begin() check is a workaround to skip the backedge check until we
  // form ddrefs in the framework otherwise we encounter an assert for null
  // stride ref.
  if (ParentLoop && (*ParentLoop->ddref_begin()) && isUnknownLoopHeaderLabel()) {
    // Print IV update for the unknown loop which will be generated during code
    // gen.
    auto Level = ParentLoop->getNestingLevel();
    indent(OS, Depth);
    OS << "<i" << Level << " = 0>\n";
  }
 
  indent(OS, Depth);
  OS << getName() << ":\n";
}

void HLLabel::printBBlockName(raw_ostream &OS, const BasicBlock &BB) {
  if (BB.hasName()) {
    OS << BB.getName();
  } else {
    BB.printAsOperand(OS, false);
  }
}

bool HLLabel::isUnknownLoopHeaderLabel() const {
  auto ParentLoop = dyn_cast<HLLoop>(getParent());
  return (ParentLoop && (ParentLoop->getHeaderLabel() == this));
}

