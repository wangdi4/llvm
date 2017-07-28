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

#include "llvm/IR/Intel_LoopIR/HLRegion.h"
#include "llvm/IR/Intel_LoopIR/HLInst.h"

#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/FormattedStream.h"

using namespace llvm;
using namespace llvm::loopopt;

HLRegion::HLRegion(HLNodeUtils &HNU, IRRegion &IReg)
    : HLNode(HNU, HLNode::HLRegionVal), GenCode(false), IRReg(IReg) {
  IRReg.ParentRegion = this;
}

HLRegion *HLRegion::cloneImpl(GotoContainerTy *GotoList, LabelMapTy *LabelMap,
                              HLNodeMapper *NodeMapper) const {
  llvm_unreachable("Do not support HLRegion cloning.");
  return nullptr;
}

HLRegion *HLRegion::clone(HLNodeMapper *NodeMapper) const {

  llvm_unreachable("Do not support HLRegion cloning.");

  return nullptr;
}

void HLRegion::print(formatted_raw_ostream &OS, unsigned Depth,
                     bool Detailed) const {
  print(OS, Depth, false, Detailed);
}

void HLRegion::printHeader(formatted_raw_ostream &OS, unsigned Depth,
                           bool PrintIRRegion, bool Detailed) const {
#if !INTEL_PRODUCT_RELEASE
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
#endif // !INTEL_PRODUCT_RELEASE
}

void HLRegion::printBody(formatted_raw_ostream &OS, unsigned Depth,
                         bool Detailed) const {
#if !INTEL_PRODUCT_RELEASE
  for (auto I = child_begin(), E = child_end(); I != E; I++) {
    I->print(OS, Depth + 1, Detailed);
  }
#endif // !INTEL_PRODUCT_RELEASE
}

void HLRegion::printFooter(formatted_raw_ostream &OS, unsigned Depth) const {
#if !INTEL_PRODUCT_RELEASE
  indent(OS, Depth);

  OS << "END REGION\n";
#endif // !INTEL_PRODUCT_RELEASE
}

void HLRegion::print(formatted_raw_ostream &OS, unsigned Depth,
                     bool PrintIRRegion, bool Detailed) const {
#if !INTEL_PRODUCT_RELEASE
  printHeader(OS, Depth, PrintIRRegion, Detailed);

  printBody(OS, Depth + 1, Detailed);

  printFooter(OS, Depth);
#endif // !INTEL_PRODUCT_RELEASE
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

bool HLRegion::exitsFunction() const {
  auto LastChild = getLastChild();

  if (!LastChild) {
    return false;
  }

  auto HInst = dyn_cast<HLInst>(LastChild);

  if (!HInst) {
    return false;
  }

  return isa<ReturnInst>(HInst->getLLVMInstruction());
}
