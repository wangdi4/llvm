//===--- HLRegion.cpp - Implements the HLRegion class ---------------------===//
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
// This file implements the HLRegion class.
//
//===----------------------------------------------------------------------===//

#include "llvm/Support/ErrorHandling.h"
#include "llvm/IR/Intel_LoopIR/HLRegion.h"

using namespace llvm;
using namespace llvm::loopopt;

HLRegion::HLRegion(IRRegion &IReg)
    : HLNode(HLNode::HLRegionVal), GenCode(false), IRReg(IReg) {}

HLRegion *HLRegion::cloneImpl(GotoContainerTy *GotoList,
                              LabelMapTy *LabelMap) const {
  llvm_unreachable("Do not support HLRegion cloning.");
  return nullptr;
}

HLRegion *HLRegion::clone() const {

  llvm_unreachable("Do not support HLRegion cloning.");

  return nullptr;
}

void HLRegion::print(formatted_raw_ostream &OS, unsigned Depth,
                     bool Detailed) const {
  print(OS, Depth, false, Detailed);
}

void HLRegion::print(formatted_raw_ostream &OS, unsigned Depth,
                     bool PrintIRRegion, bool Detailed) const {
  indent(OS, Depth);

  OS << "BEGIN REGION";

  OS << " {";
  if (shouldGenCode()) {
    OS << " modified";
  }
  OS << " }";
  OS << "\n";

  if (PrintIRRegion) {
    OS << "\n";
    IRReg.print(OS, Depth);
    OS << "\n";
  }

  for (auto I = child_begin(), E = child_end(); I != E; I++) {
    I->print(OS, Depth + 1, Detailed);
  }

  indent(OS, Depth);

  OS << "END REGION\n";
}

HLNode *HLRegion::getFirstChild() {
  if (hasChildren()) {
    return &*child_begin();
  }

  return nullptr;
}

HLNode *HLRegion::getLastChild() {
  if (hasChildren()) {
    return &*(std::prev(child_end()));
  }

  return nullptr;
}

void HLRegion::verify() const {
  assert(getParent() == nullptr && "HLRegion should be a root node");
  HLNode::verify();
}
