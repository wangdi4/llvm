//===------ HLLabel.cpp - Implements the HLLabel class --------------------===//
//
// Copyright (C) 2015 Intel Corporation. All rights reserved.
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

#include "llvm/Analysis/Intel_LoopAnalysis/IR/HLLabel.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HLNodeUtils.h"

using namespace llvm;
using namespace llvm::loopopt;

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
void HLLabel::makeNameUnique() {
  bool IsUnique = (SrcBBlock != nullptr); // Treat BasicBlock name as unique
  do {
    if (!IsUnique) {
      raw_svector_ostream(DebugName) << "." << getNumber();
    }

    IsUnique = getHLNodeUtils().LabelNames.insert(DebugName).second;
  } while (!IsUnique);
}

void HLLabel::setDebugName(StringRef Name, const BasicBlock *SrcBB) {
  raw_svector_ostream OS(DebugName);
  if (Name.empty()) {
    printBBlockName(OS, *SrcBB);
  } else {
    OS << Name;
  }
  makeNameUnique();
}
#endif

StringRef HLLabel::getDebugName() const {
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  if (SrcBBlock && DebugName.empty()) {
    const_cast<HLLabel *>(this)->setDebugName({}, SrcBBlock);
  }

  return DebugName;
#else
  return {};
#endif
}

HLLabel::HLLabel(HLNodeUtils &HNU, BasicBlock *SrcBB)
    : HLNode(HNU, HLNode::HLLabelVal), SrcBBlock(SrcBB) {
  assert(SrcBB != nullptr && "SrcBB must not be NULL");
  // Do not call setDebugName() here as it may be expensive.
  // The name for BasicBlock labels will be initialized in getDebugName().
}

HLLabel::HLLabel(HLNodeUtils &HNU, const Twine &Name)
    : HLNode(HNU, HLNode::HLLabelVal), SrcBBlock(nullptr) {
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  setDebugName(Name.str(), nullptr);
#endif
}

HLLabel::HLLabel(const HLLabel &LabelObj)
    : HLNode(LabelObj), SrcBBlock(nullptr) {
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  setDebugName(LabelObj.DebugName, LabelObj.SrcBBlock);
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
#if !INTEL_PRODUCT_RELEASE
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
  OS << getDebugName() << ":\n";
#endif // !INTEL_PRODUCT_RELEASE
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

