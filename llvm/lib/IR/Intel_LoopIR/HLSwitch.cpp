//===---- HLSwitch.cpp - Implements the HLSwitch class --------------------===//
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
// This file implements the HLSwitch class.
//
//===----------------------------------------------------------------------===//

#include "llvm/Support/Debug.h"

#include "llvm/IR/Intel_LoopIR/HLSwitch.h"
#include "llvm/IR/Intel_LoopIR/RegDDRef.h"

#include "llvm/Transforms/Intel_LoopTransforms/Utils/HLNodeUtils.h"

using namespace llvm;
using namespace llvm::loopopt;

HLSwitch::HLSwitch(HLNodeUtils &HNU, RegDDRef *ConditionRef)
    : HLDDNode(HNU, HLNode::HLSwitchVal) {
  unsigned NumOp;

  DefaultCaseBegin = Children.end();

  /// This call is to get around calling virtual functions in the constructor.
  NumOp = getNumOperandsInternal();

  RegDDRefs.resize(NumOp, nullptr);

  setConditionDDRef(ConditionRef);
}

HLSwitch::HLSwitch(const HLSwitch &HLSwitchObj) : HLDDNode(HLSwitchObj) {
  const RegDDRef *TRef;
  RegDDRef *Ref;

  CaseBegin.resize(HLSwitchObj.getNumCases(), Children.end());
  DefaultCaseBegin = Children.end();
  RegDDRefs.resize(getNumOperandsInternal(), nullptr);

  /// Clone switch condition DDRef
  setConditionDDRef((TRef = HLSwitchObj.getConditionDDRef()) ? TRef->clone()
                                                             : nullptr);

  /// Clone case value RegDDRefs
  for (unsigned I = 1, E = getNumCases(); I <= E; I++) {
    Ref = (TRef = HLSwitchObj.getCaseValueDDRef(I)) ? TRef->clone() : nullptr;
    setCaseValueDDRef(Ref, I);
  }
}

HLSwitch *HLSwitch::cloneImpl(GotoContainerTy *GotoList, LabelMapTy *LabelMap,
                              HLNodeMapper *NodeMapper) const {
  // Call the Copy Constructor
  HLSwitch *NewHLSwitch = new HLSwitch(*this);

  /// Clone default case children
  for (auto It = this->default_case_child_begin(),
            EndIt = this->default_case_child_end();
       It != EndIt; It++) {
    HLNode *NewHLNode = cloneBaseImpl(&*It, GotoList, LabelMap, NodeMapper);
    getHLNodeUtils().insertAsLastDefaultChild(NewHLSwitch, NewHLNode);
  }

  /// Clone case children
  for (unsigned I = 1, E = this->getNumCases(); I <= E; I++) {
    for (auto It = this->case_child_begin(I), EndIt = this->case_child_end(I);
         It != EndIt; It++) {
      HLNode *NewHLNode = cloneBaseImpl(&*It, GotoList, LabelMap, NodeMapper);
      getHLNodeUtils().insertAsLastChild(NewHLSwitch, NewHLNode, I);
    }
  }

  return NewHLSwitch;
}

HLSwitch *HLSwitch::clone(HLNodeMapper *NodeMapper) const {
  return cast<HLSwitch>(HLNode::clone(NodeMapper));
}

void HLSwitch::print_break(formatted_raw_ostream &OS, unsigned Depth,
                           unsigned CaseNum) const {

  auto LastChild = getLastCaseChildInternal(CaseNum);

  if (!LastChild || !isa<HLGoto>(LastChild)) {
    indent(OS, Depth);
    OS.indent(IndentWidth);
    OS << "break;\n";
  }
}

void HLSwitch::print(formatted_raw_ostream &OS, unsigned Depth,
                     bool Detailed) const {

  indent(OS, Depth);

  OS << "switch(";

  auto Ref = getConditionDDRef();
  Ref ? Ref->print(OS, Detailed) : (void)(OS << Ref);

  OS << ")\n";
  indent(OS, Depth);
  OS << "{\n";

  /// Print cases
  for (unsigned I = 1, E = getNumCases(); I <= E; I++) {
    indent(OS, Depth);

    OS << "case ";
    auto Ref = getCaseValueDDRef(I);
    Ref ? Ref->print(OS, Detailed) : (void)(OS << Ref);
    OS << ":\n";

    for (auto It = case_child_begin(I), EndIt = case_child_end(I); It != EndIt;
         It++) {
      It->print(OS, Depth + 1, Detailed);
    }

    print_break(OS, Depth, I);
  }

  /// Print default case
  indent(OS, Depth);
  OS << "default:\n";

  for (auto It = default_case_child_begin(), EndIt = default_case_child_end();
       It != EndIt; It++) {
    It->print(OS, Depth + 1, Detailed);
  }

  print_break(OS, Depth, 0);

  indent(OS, Depth);
  OS << "}\n";
}

HLSwitch::case_child_iterator
HLSwitch::case_child_begin_internal(unsigned CaseNum) {
  if (CaseNum == 0) {
    return DefaultCaseBegin;
  } else {
    return CaseBegin[CaseNum - 1];
  }
}

HLSwitch::const_case_child_iterator
HLSwitch::case_child_begin_internal(unsigned CaseNum) const {
  return const_cast<HLSwitch *>(this)->case_child_begin_internal(CaseNum);
}

HLSwitch::case_child_iterator
HLSwitch::case_child_end_internal(unsigned CaseNum) {
  if (CaseNum == 0) {
    return Children.end();
  } else if (CaseNum == getNumCases()) {
    return DefaultCaseBegin;
  } else {
    return CaseBegin[CaseNum];
  }
}

HLSwitch::const_case_child_iterator
HLSwitch::case_child_end_internal(unsigned CaseNum) const {
  return const_cast<HLSwitch *>(this)->case_child_end_internal(CaseNum);
}

HLSwitch::reverse_case_child_iterator
HLSwitch::case_child_rbegin_internal(unsigned CaseNum) {
  if (CaseNum == 0) {
    return Children.rbegin();
  } else if (CaseNum == getNumCases()) {
    return DefaultCaseBegin.getReverse();
  } else {
    return CaseBegin[CaseNum].getReverse();
  }
}

HLSwitch::const_reverse_case_child_iterator
HLSwitch::case_child_rbegin_internal(unsigned CaseNum) const {
  return const_cast<HLSwitch *>(this)->case_child_rbegin_internal(CaseNum);
}

HLSwitch::reverse_case_child_iterator
HLSwitch::case_child_rend_internal(unsigned CaseNum) {
  if (CaseNum == 0) {
    return DefaultCaseBegin.getReverse();
  } else {
    return CaseBegin[CaseNum - 1].getReverse();
  }
}

HLSwitch::const_reverse_case_child_iterator
HLSwitch::case_child_rend_internal(unsigned CaseNum) const {
  return const_cast<HLSwitch *>(this)->case_child_rend_internal(CaseNum);
}

HLNode *HLSwitch::getFirstCaseChildInternal(unsigned CaseNum) {
  if (hasCaseChildrenInternal(CaseNum)) {
    return &*case_child_begin_internal(CaseNum);
  }

  return nullptr;
}

HLNode *HLSwitch::getLastCaseChildInternal(unsigned CaseNum) {
  if (hasCaseChildrenInternal(CaseNum)) {
    return &*(std::prev(case_child_end_internal(CaseNum)));
  }

  return nullptr;
}

unsigned HLSwitch::getChildCaseNum(const HLNode *Node) const {
  if (getHLNodeUtils().isInTopSortNumMaxRange(Node, getFirstDefaultCaseChild(),
                                              getLastDefaultCaseChild())) {
    return 0;
  }

  for (int I = 1, E = getNumCases(); I < E; ++I) {
    if (getHLNodeUtils().isInTopSortNumMaxRange(Node, getFirstCaseChild(I),
                                                getLastCaseChild(I))) {
      return I;
    }
  }

  llvm_unreachable("Node is not a child of the HLSwitch");
}

RegDDRef *HLSwitch::getConditionDDRef() { return getOperandDDRefImpl(0); }

const RegDDRef *HLSwitch::getConditionDDRef() const {
  return const_cast<HLSwitch *>(this)->getConditionDDRef();
}

void HLSwitch::setConditionDDRef(RegDDRef *ConditionRef) {
  setOperandDDRefImpl(ConditionRef, 0);
}

RegDDRef *HLSwitch::removeConditionDDRef() {
  auto TRef = getConditionDDRef();

  if (TRef) {
    setConditionDDRef(nullptr);
  }

  return TRef;
}

RegDDRef *HLSwitch::getCaseValueDDRef(unsigned CaseNum) {
  assert((CaseNum != 0) && "Default case does not contain DDRef!");
  assert((CaseNum <= getNumCases()) && "CaseNum is out of range!");

  return getOperandDDRefImpl(CaseNum);
}

const RegDDRef *HLSwitch::getCaseValueDDRef(unsigned CaseNum) const {
  return const_cast<HLSwitch *>(this)->getCaseValueDDRef(CaseNum);
}

void HLSwitch::setCaseValueDDRef(RegDDRef *ValueRef, unsigned CaseNum) {
  assert((CaseNum != 0) && "Default case does not contain DDRef!");
  assert((CaseNum <= getNumCases()) && "CaseNum is out of range!");

  setOperandDDRefImpl(ValueRef, CaseNum);
}

RegDDRef *HLSwitch::removeCaseValueDDRef(unsigned CaseNum) {
  auto TRef = getCaseValueDDRef(CaseNum);

  if (TRef) {
    setCaseValueDDRef(nullptr, CaseNum);
  }

  return TRef;
}

void HLSwitch::addCase(RegDDRef *ValueRef) {
  unsigned NumOp;

  CaseBegin.push_back(DefaultCaseBegin);

  NumOp = getNumOperandsInternal();
  RegDDRefs.resize(NumOp, nullptr);

  setCaseValueDDRef(ValueRef, getNumCases());
}

void HLSwitch::removeCase(unsigned CaseNum) {
  assert((CaseNum != 0) && "Default case cannot be removed!");
  assert((CaseNum <= getNumCases()) && "CaseNum is out of range!");

  /// Remove CaseNum's HLNodes
  getHLNodeUtils().remove(case_child_begin_internal(CaseNum),
                          case_child_end_internal(CaseNum));

  /// Remove the case value DDRef.
  removeCaseValueDDRef(CaseNum);
  /// Erase the DDRef slot.
  RegDDRefs.erase(RegDDRefs.begin() + CaseNum);

  /// Erase the separator for this case.
  CaseBegin.erase(CaseBegin.begin() + CaseNum - 1);
}

void HLSwitch::verify() const { HLDDNode::verify(); }
