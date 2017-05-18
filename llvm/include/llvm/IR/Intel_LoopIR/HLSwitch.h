//===-------- HLSwitch.h - High level IR switch node ------------*- C++ -*-===//
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
// This file defines the HLSwitch node.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_IR_INTEL_LOOPIR_HLSWITCH_H
#define LLVM_IR_INTEL_LOOPIR_HLSWITCH_H

#include "llvm/IR/Intel_LoopIR/HLDDNode.h"

namespace llvm {

namespace loopopt {

class RegDDRef;

/// \brief High level node representing a switch statement
class HLSwitch final : public HLDDNode {
public:
  typedef HLContainerTy ChildNodeTy;

  /// Iterators to iterate over children nodes
  typedef ChildNodeTy::iterator case_child_iterator;
  typedef ChildNodeTy::const_iterator const_case_child_iterator;
  typedef ChildNodeTy::reverse_iterator reverse_case_child_iterator;
  typedef ChildNodeTy::const_reverse_iterator const_reverse_case_child_iterator;

private:
  /// Contains all the switch cases' children. The default case is always the
  /// last one.
  ChildNodeTy Children;
  /// Iterators pointing to beginning of switch cases.
  SmallVector<case_child_iterator, 5> CaseBegin;
  case_child_iterator DefaultCaseBegin;

  DebugLoc DbgLoc;

  /// ConditionRef represents the switch conditon.
  HLSwitch(HLNodeUtils &HNU, RegDDRef *ConditionRef);

  /// \brief Copy constructor used by cloning.
  HLSwitch(const HLSwitch &HLSwitchObj);

  friend class HLNodeUtils;

  /// Implements getNumOperands() functionality.
  unsigned getNumOperandsInternal() const { return getNumCases() + 1; }

  /// Implements case child iterator functionality.
  case_child_iterator case_child_begin_internal(unsigned CaseNum);
  const_case_child_iterator case_child_begin_internal(unsigned CaseNum) const;
  case_child_iterator case_child_end_internal(unsigned CaseNum);
  const_case_child_iterator case_child_end_internal(unsigned CaseNum) const;

  reverse_case_child_iterator case_child_rbegin_internal(unsigned CaseNum);
  const_reverse_case_child_iterator
  case_child_rbegin_internal(unsigned CaseNum) const;
  reverse_case_child_iterator case_child_rend_internal(unsigned CaseNum);
  const_reverse_case_child_iterator
  case_child_rend_internal(unsigned CaseNum) const;

  /// \brief Implements hasCaseChildren() functionality.
  bool hasCaseChildrenInternal(unsigned CaseNum) const {
    return (case_child_begin_internal(CaseNum) !=
            case_child_end_internal(CaseNum));
  }

  /// \brief Implements getFirstCaseChild() functionality.
  HLNode *getFirstCaseChildInternal(unsigned CaseNum);
  const HLNode *getFirstCaseChildInternal(unsigned CaseNum) const {
    return const_cast<HLSwitch *>(this)->getFirstCaseChildInternal(CaseNum);
  }

  /// \brief Implements getLastCaseChild() functionality.
  HLNode *getLastCaseChildInternal(unsigned CaseNum);
  const HLNode *getLastCaseChildInternal(unsigned CaseNum) const {
    return const_cast<HLSwitch *>(this)->getLastCaseChildInternal(CaseNum);
  }

  /// \brief Used by print() to print breaks after switch cases.
  void print_break(formatted_raw_ostream &OS, unsigned Depth,
                   unsigned CaseNum) const;

  /// \brief Clone Implementation
  /// This function populates the GotoList with Goto branching within the
  /// cloned Switch and LabelMap with Old and New Labels. Returns cloned Switch.
  HLSwitch *cloneImpl(GotoContainerTy *GotoList, LabelMapTy *LabelMap,
                      HLNodeMapper *NodeMapper) const override;

public:
  /// \brief Method for supporting type inquiry through isa, cast, and dyn_cast.
  static bool classof(const HLNode *Node) {
    return Node->getHLNodeID() == HLNode::HLSwitchVal;
  }

  /// \brief Prints HLSwitch.
  virtual void print(formatted_raw_ostream &OS, unsigned Depth,
                     bool Detailed = false) const override;

  /// clone() - Create a copy of 'this' switch that is identical in all
  /// ways except the following:
  ///   * The Switch has no parent
  /// This method will automatically update the goto branches with new labels
  /// inside the cloned Switch.
  HLSwitch *clone(HLNodeMapper *NodeMapper = nullptr) const override;

  /// \brief Returns the number of operands this node is supposed to have.
  unsigned getNumOperands() const override { return getNumOperandsInternal(); }

  /// \brief Returns the number of switch cases excluding the default case.
  unsigned getNumCases() const { return CaseBegin.size(); }

  /// Default Case iterator methods
  case_child_iterator default_case_child_begin() {
    return case_child_begin_internal(0);
  }
  const_case_child_iterator default_case_child_begin() const {
    return const_cast<HLSwitch *>(this)->default_case_child_begin();
  }
  case_child_iterator default_case_child_end() {
    return case_child_end_internal(0);
  }
  const_case_child_iterator default_case_child_end() const {
    return const_cast<HLSwitch *>(this)->default_case_child_end();
  }

  reverse_case_child_iterator default_case_child_rbegin() {
    return case_child_rbegin_internal(0);
  }
  const_reverse_case_child_iterator default_case_child_rbegin() const {
    return const_cast<HLSwitch *>(this)->default_case_child_rbegin();
  }
  reverse_case_child_iterator default_case_child_rend() {
    return case_child_rend_internal(0);
  }
  const_reverse_case_child_iterator default_case_child_rend() const {
    return const_cast<HLSwitch *>(this)->default_case_child_rend();
  }

  /// \brief Returns true if default case has at least one child.
  bool hasDefaultCaseChildren() const { return hasCaseChildrenInternal(0); }

  /// \brief Returns the number of default case children.
  unsigned getNumDefaultCaseChildren() const {
    return std::distance(default_case_child_begin(), default_case_child_end());
  }

  /// Case iterator methods
  /// Range of CaseNum is [1, getNumCases()].
  case_child_iterator case_child_begin(unsigned CaseNum) {
    assert((CaseNum > 0) && (CaseNum <= getNumCases()) &&
           "CaseNum is out of range!");
    return case_child_begin_internal(CaseNum);
  }
  const_case_child_iterator case_child_begin(unsigned CaseNum) const {
    return const_cast<HLSwitch *>(this)->case_child_begin(CaseNum);
  }
  case_child_iterator case_child_end(unsigned CaseNum) {
    assert((CaseNum > 0) && (CaseNum <= getNumCases()) &&
           "CaseNum is out of range!");
    return case_child_end_internal(CaseNum);
  }
  const_case_child_iterator case_child_end(unsigned CaseNum) const {
    return const_cast<HLSwitch *>(this)->case_child_end(CaseNum);
  }

  reverse_case_child_iterator case_child_rbegin(unsigned CaseNum) {
    assert((CaseNum > 0) && (CaseNum <= getNumCases()) &&
           "CaseNum is out of range!");
    return case_child_rbegin_internal(CaseNum);
  }
  const_reverse_case_child_iterator case_child_rbegin(unsigned CaseNum) const {
    return const_cast<HLSwitch *>(this)->case_child_rbegin(CaseNum);
  }
  reverse_case_child_iterator case_child_rend(unsigned CaseNum) {
    assert((CaseNum > 0) && (CaseNum <= getNumCases()) &&
           "CaseNum is out of range!");
    return case_child_rend_internal(CaseNum);
  }
  const_reverse_case_child_iterator case_child_rend(unsigned CaseNum) const {
    return const_cast<HLSwitch *>(this)->case_child_rend(CaseNum);
  }

  /// \brief Returns true if this case has at least one child.
  /// Range of CaseNum is [1, getNumCases()].
  bool hasCaseChildren(unsigned CaseNum) const {
    assert((CaseNum > 0) && (CaseNum <= getNumCases()) &&
           "CaseNum is out of range!");
    return hasCaseChildrenInternal(CaseNum);
  }

  /// \brief Returns the number of case children.
  /// Range of CaseNum is [1, getNumCases()].
  unsigned getNumCaseChildren(unsigned CaseNum) const {
    return std::distance(case_child_begin(CaseNum), case_child_end(CaseNum));
  }

  /// \brief Returns the first default case child if it exists, otherwise
  /// returns null.
  HLNode *getFirstDefaultCaseChild() { return getFirstCaseChildInternal(0); }
  const HLNode *getFirstDefaultCaseChild() const {
    return const_cast<HLSwitch *>(this)->getFirstDefaultCaseChild();
  }
  /// \brief Returns the last default case child if it exists, otherwise returns
  /// null.
  HLNode *getLastDefaultCaseChild() { return getLastCaseChildInternal(0); }
  const HLNode *getLastDefaultCaseChild() const {
    return const_cast<HLSwitch *>(this)->getLastDefaultCaseChild();
  }

  /// \brief Returns the first case child if it exists, otherwise returns null.
  /// Range of CaseNum is [1, getNumCases()].
  HLNode *getFirstCaseChild(unsigned CaseNum) {
    assert((CaseNum > 0) && (CaseNum <= getNumCases()) &&
           "CaseNum is out of range!");
    return getFirstCaseChildInternal(CaseNum);
  }
  const HLNode *getFirstCaseChild(unsigned CaseNum) const {
    return const_cast<HLSwitch *>(this)->getFirstCaseChild(CaseNum);
  }
  /// \brief Returns the last case child if it exists, otherwise returns null.
  /// Range of CaseNum is [1, getNumCases()].
  HLNode *getLastCaseChild(unsigned CaseNum) {
    assert((CaseNum > 0) && (CaseNum <= getNumCases()) &&
           "CaseNum is out of range!");
    return getLastCaseChildInternal(CaseNum);
  }
  const HLNode *getLastCaseChild(unsigned CaseNum) const {
    return const_cast<HLSwitch *>(this)->getLastCaseChild(CaseNum);
  }

  /// Returns the CaseNum for a child \p Node. The method may return zero,
  /// meaning the default case.
  unsigned getChildCaseNum(const HLNode *Node) const;

  /// \brief Returns the DDRef which represents the switch condition.
  RegDDRef *getConditionDDRef();
  const RegDDRef *getConditionDDRef() const;
  /// \brief Sets the DDRef which represents the switch condition.
  void setConditionDDRef(RegDDRef *ConditionRef);
  /// \brief Removes and returns the DDRef which represents the switch
  /// condition.
  RegDDRef *removeConditionDDRef();

  /// \brief Returns the DDRef which represents this case's value.
  /// Range of CaseNum is [1, getNumCases()].
  RegDDRef *getCaseValueDDRef(unsigned CaseNum);
  const RegDDRef *getCaseValueDDRef(unsigned CaseNum) const;
  /// \brief Sets the DDRef which represents this case's value.
  void setCaseValueDDRef(RegDDRef *ValueRef, unsigned CaseNum);
  /// \brief Removes and returns the DDRef which represents this case's value.
  RegDDRef *removeCaseValueDDRef(unsigned CaseNum);

  /// Returns constant value associated with the case.
  int64_t getConstCaseValue(unsigned CaseNum) const {
    int64_t ConstValue;
    bool Result = getCaseValueDDRef(CaseNum)->isIntConstant(&ConstValue);
    assert(Result && "Non-constant HLSwitch case value found");
    (void)Result;
    return ConstValue;
  }

  /// \brief Adds a new case in the switch. This invalidates the case_child
  /// iterators.
  void addCase(RegDDRef *ValueRef);
  /// \brief Removes case CaseNum from the switch. This invalidates the
  /// case_child iterators.
  /// Default case cannot be removed.
  void removeCase(unsigned CaseNum);

  /// \brief Verifies HLSwitch integrity.
  virtual void verify() const override;

  const DebugLoc getDebugLoc() const override { return DbgLoc; }
  void setDebugLoc(const DebugLoc &Loc) { DbgLoc = Loc; }
};

} // End namespace loopopt

} // End namespace llvm

#endif
