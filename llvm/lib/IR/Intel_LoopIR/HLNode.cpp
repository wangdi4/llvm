//===-- HLNode.cpp - Implements the HLNode class ---------------------===//
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
// This file implements the HLNode class.
//
//===----------------------------------------------------------------------===//

#include "llvm/Support/Debug.h"

#include "llvm/IR/Intel_LoopIR/HLNode.h"
#include "llvm/IR/Intel_LoopIR/HLRegion.h"
#include "llvm/IR/Intel_LoopIR/HLLoop.h"
#include "llvm/IR/Intel_LoopIR/HLInst.h"

using namespace llvm;
using namespace llvm::loopopt;

HLContainerTy llvm::loopopt::HLRegions;

std::set<HLNode *> HLNode::Objs;
unsigned HLNode::GlobalNum(0);

HLNode::HLNode(unsigned SCID)
    : SubClassID(SCID), Parent(nullptr), TopSortNum(0) {
  Objs.insert(this);
  setNextNumber();
}

HLNode::HLNode(const HLNode &HLNodeObj)
    : SubClassID(HLNodeObj.SubClassID), Parent(nullptr), TopSortNum(0) {
  Objs.insert(this);
  setNextNumber();
}

void HLNode::destroy() {
  Objs.erase(this);
  delete this;
}

void HLNode::destroyAll() {

  for (auto &I : Objs) {
    delete I;
  }

  Objs.clear();

  // Reset HLNode numbering.
  GlobalNum = 0;
}

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
void HLNode::dump() const { this->dump(false); }

void HLNode::dump(bool Detailed) const {
  formatted_raw_ostream OS(dbgs());
  print(OS, 0, Detailed);
}
#endif

void HLNode::indent(formatted_raw_ostream &OS, unsigned Depth) const {

  static std::string LoopIndentString(100, ' ');
  static std::string SpaceString(IndentWidth, ' ');

  LoopIndentString.clear();

  /// Placeholder until we can get source location.
  if (!isa<HLRegion>(this)) {
    OS << "<" << Number << ">";
  }
  OS.PadToColumn(10);

  auto Parent = getParent();

  /// Don't print loop marker "|" if the node is in preheader/postexit.
  if (Parent && isa<HLLoop>(Parent) && isa<HLInst>(this) &&
      cast<HLInst>(this)->isInPreheaderOrPostexit()) {
    LoopIndentString = SpaceString + LoopIndentString;
    Depth--;
    Parent = Parent->getParent();
  }

  while ((Depth > 0) && Parent) {
    LoopIndentString = SpaceString + LoopIndentString;

    if (isa<HLLoop>(Parent)) {
      LoopIndentString = "|" + LoopIndentString;
    }

    Depth--;
    Parent = Parent->getParent();
  }

  OS.indent(IndentWidth * Depth);
  OS << LoopIndentString;
}

void HLNode::printPredicate(formatted_raw_ostream &OS,
                            const CmpInst::Predicate &Pred) {
  if (Pred == CmpInst::Predicate::FCMP_TRUE) {
    OS << " true ";
  } else if (Pred == CmpInst::Predicate::FCMP_FALSE) {
    OS << " false ";
  }
  /// TODO: Differentiate ordered/unordered and signed/unsigned.
  else if ((Pred == CmpInst::Predicate::FCMP_OEQ) ||
           (Pred == CmpInst::Predicate::FCMP_UEQ) ||
           (Pred == CmpInst::Predicate::ICMP_EQ)) {
    OS << " == ";
  } else if ((Pred == CmpInst::Predicate::FCMP_ONE) ||
             (Pred == CmpInst::Predicate::FCMP_UNE) ||
             (Pred == CmpInst::Predicate::ICMP_NE)) {
    OS << " != ";
  } else if ((Pred == CmpInst::Predicate::FCMP_OGT) ||
             (Pred == CmpInst::Predicate::FCMP_UGT) ||
             (Pred == CmpInst::Predicate::ICMP_UGT) ||
             (Pred == CmpInst::Predicate::ICMP_SGT)) {
    OS << " > ";
  } else if ((Pred == CmpInst::Predicate::FCMP_OGE) ||
             (Pred == CmpInst::Predicate::FCMP_UGE) ||
             (Pred == CmpInst::Predicate::ICMP_UGE) ||
             (Pred == CmpInst::Predicate::ICMP_SGE)) {
    OS << " >= ";
  } else if ((Pred == CmpInst::Predicate::FCMP_OLT) ||
             (Pred == CmpInst::Predicate::FCMP_ULT) ||
             (Pred == CmpInst::Predicate::ICMP_ULT) ||
             (Pred == CmpInst::Predicate::ICMP_SLT)) {
    OS << " < ";
  } else if ((Pred == CmpInst::Predicate::FCMP_OLE) ||
             (Pred == CmpInst::Predicate::FCMP_ULE) ||
             (Pred == CmpInst::Predicate::ICMP_ULE) ||
             (Pred == CmpInst::Predicate::ICMP_SLE)) {
    OS << " <= ";
  } else if (Pred == CmpInst::Predicate::FCMP_ORD) {
    OS << " ORDERED ";
  } else if (Pred == CmpInst::Predicate::FCMP_UNO) {
    OS << " UNORDERED ";
  } else {
    llvm_unreachable("Unexpected predicate!");
  }
}

void HLNode::setNextNumber() { Number = GlobalNum++; }

HLLoop *HLNode::getParentLoop() const {
  assert(!isa<HLRegion>(this) && "Region cannot have a parent!");

  HLNode *Par = getParent();

  while (Par && !isa<HLLoop>(Par)) {
    Par = Par->getParent();
  }

  return cast_or_null<HLLoop>(Par);
}

HLLoop *HLNode::getLexicalParentLoop() const {
  auto ParLoop = getParentLoop();

  if (auto HInst = dyn_cast<HLInst>(this)) {
    if (ParLoop && HInst->isInPreheaderOrPostexit()) {
      ParLoop = ParLoop->getParentLoop();
    }
  }

  return ParLoop;
}

HLRegion *HLNode::getParentRegion() const {
  assert(!isa<HLRegion>(this) && "Region cannot not have a parent!");

  HLNode *Par = getParent();

  while (Par && !isa<HLRegion>(Par)) {
    Par = Par->getParent();
  }

  return cast_or_null<HLRegion>(Par);
}
