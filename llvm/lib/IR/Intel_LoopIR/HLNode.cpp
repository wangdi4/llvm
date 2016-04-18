//===-- HLNode.cpp - Implements the HLNode class ---------------------===//
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
// This file implements the HLNode class.
//
//===----------------------------------------------------------------------===//

#include "llvm/Support/Debug.h"
#include "llvm/Support/CommandLine.h"

#include "llvm/IR/Intel_LoopIR/HLNode.h"
#include "llvm/IR/Intel_LoopIR/HLRegion.h"
#include "llvm/IR/Intel_LoopIR/HLLoop.h"
#include "llvm/IR/Intel_LoopIR/HLInst.h"

using namespace llvm;
using namespace llvm::loopopt;

static cl::opt<bool> PrintTopSortNum("hir-details-topsort", cl::init(false),
                                     cl::Hidden,
                                     cl::desc("Print HLNode TopSort numbers"));

std::set<HLNode *> HLNode::Objs;
unsigned HLNode::GlobalNum(0);

HLNode::HLNode(unsigned SCID)
    : SubClassID(SCID), Parent(nullptr), TopSortNum(0), MaxTopSortNum(0) {
  Objs.insert(this);
  setNextNumber();
}

HLNode::HLNode(const HLNode &HLNodeObj)
    : SubClassID(HLNodeObj.SubClassID), Parent(nullptr), TopSortNum(0),
      MaxTopSortNum(0) {
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
    OS << "<" << Number;
    if (PrintTopSortNum) {
      OS << ":" << TopSortNum << "(" << MaxTopSortNum << ")";
    }
    OS << ">";
  }
  OS.PadToColumn(10);

  auto Parent = getParent();

  /// Don't print loop marker "|" if the node is in preheader/postexit.
  if ((Depth > 0) && Parent && isa<HLLoop>(Parent) && isa<HLInst>(this) &&
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

void HLNode::printPredicate(formatted_raw_ostream &OS, PredicateTy Pred) {
  if (Pred == PredicateTy::FCMP_TRUE) {
    OS << " true ";
  } else if (Pred == PredicateTy::FCMP_FALSE) {
    OS << " false ";
  }
  /// TODO: Differentiate ordered/unordered and signed/unsigned.
  else if ((Pred == PredicateTy::FCMP_OEQ) || (Pred == PredicateTy::FCMP_UEQ) ||
           (Pred == PredicateTy::ICMP_EQ)) {
    OS << " == ";
  } else if ((Pred == PredicateTy::FCMP_ONE) ||
             (Pred == PredicateTy::FCMP_UNE) ||
             (Pred == PredicateTy::ICMP_NE)) {
    OS << " != ";
  } else if ((Pred == PredicateTy::FCMP_OGT) ||
             (Pred == PredicateTy::FCMP_UGT) ||
             (Pred == PredicateTy::ICMP_UGT) ||
             (Pred == PredicateTy::ICMP_SGT)) {
    OS << " > ";
  } else if ((Pred == PredicateTy::FCMP_OGE) ||
             (Pred == PredicateTy::FCMP_UGE) ||
             (Pred == PredicateTy::ICMP_UGE) ||
             (Pred == PredicateTy::ICMP_SGE)) {
    OS << " >= ";
  } else if ((Pred == PredicateTy::FCMP_OLT) ||
             (Pred == PredicateTy::FCMP_ULT) ||
             (Pred == PredicateTy::ICMP_ULT) ||
             (Pred == PredicateTy::ICMP_SLT)) {
    OS << " < ";
  } else if ((Pred == PredicateTy::FCMP_OLE) ||
             (Pred == PredicateTy::FCMP_ULE) ||
             (Pred == PredicateTy::ICMP_ULE) ||
             (Pred == PredicateTy::ICMP_SLE)) {
    OS << " <= ";
  } else if (Pred == PredicateTy::FCMP_ORD) {
    OS << " ORDERED ";
  } else if (Pred == PredicateTy::FCMP_UNO) {
    OS << " UNORDERED ";
  } else if (Pred == UNDEFINED_PREDICATE) {
    OS << " #UNDEF# ";
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

HLLoop *HLNode::getOutermostParentLoop() const {
  auto TempLoop = getParentLoop();
  HLLoop *ParLoop = nullptr;

  while (TempLoop) {
    ParLoop = TempLoop;
    TempLoop = TempLoop->getParentLoop();
  }

  return ParLoop;
}

unsigned HLNode::getNodeLevel() const {

  assert(getParentRegion() && " Node should be connected to a HLRegion");

  if (auto CurrentLoop = dyn_cast<HLLoop>(this)) {
    return CurrentLoop->getNestingLevel();
  }

  HLLoop *Loop = getLexicalParentLoop();
  unsigned Level = Loop ? Loop->getNestingLevel() : 0;
  return Level;
}

HLRegion *HLNode::getParentRegion() const {
  assert(!isa<HLRegion>(this) && "Region cannot not have a parent!");

  HLNode *Par = getParent();

  while (Par && !isa<HLRegion>(Par)) {
    Par = Par->getParent();
  }

  return cast_or_null<HLRegion>(Par);
}

void HLNode::verify() const {
  assert((isa<HLRegion>(this) || getParent() != nullptr) &&
         "Non-Region HLNode should have a parent node");
}
