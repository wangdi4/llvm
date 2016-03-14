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

using namespace llvm;
using namespace llvm::loopopt;

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
StringSet<> HLLabel::LabelNames;
#endif

void HLLabel::makeNameUnique() {
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  bool is_uniq = (SrcBBlock != nullptr); // Treat BasicBlock name as unique
  do {
    if (!is_uniq) {
      raw_svector_ostream(Name) << "." << getNumber();
    }

    is_uniq = LabelNames.insert(Name).second;
  } while (!is_uniq);
#endif
}

HLLabel::HLLabel(BasicBlock *SrcBB)
    : HLNode(HLNode::HLLabelVal), SrcBBlock(SrcBB) {
  assert(SrcBB != nullptr && "SrcBB must not be NULL");

  {
    raw_svector_ostream OS(this->Name);
    printBBlockName(OS, *SrcBB);
  }

  makeNameUnique();
}

HLLabel::HLLabel(const Twine &Name)
    : HLNode(HLNode::HLLabelVal), SrcBBlock(nullptr) {

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
  LabelNames.erase(getName());
#endif
}

HLLabel *HLLabel::cloneImpl(GotoContainerTy *GotoList,
                            LabelMapTy *LabelMap) const {
  // Call Copy constructor
  HLLabel *NewHLLabel = new HLLabel(*this);

  if (LabelMap) {
    LabelMap->insert(std::pair<const HLLabel *, HLLabel *>(this, NewHLLabel));
  }

  return NewHLLabel;
}

HLLabel *HLLabel::clone() const { return cloneImpl(nullptr, nullptr); }

void HLLabel::print(formatted_raw_ostream &OS, unsigned Depth,
                    bool Detailed) const {
  indent(OS, Depth);
  OS << getName() << ":\n";
}

void HLLabel::printBBlockName(raw_ostream &O, const BasicBlock &BB) {
  if (BB.hasName()) {
    O << BB.getName();
  } else {
    BB.printAsOperand(O, false);
  }
}
