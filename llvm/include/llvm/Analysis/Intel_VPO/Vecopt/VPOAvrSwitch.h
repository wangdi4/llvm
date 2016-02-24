//===-- VPOAvrSwitch.h-------------------------------------------*- C++ -*-===//
//
//   Copyright (C) 2016 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file defines the Abstract Vector Representation (AVR) switch node.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_ANALYSIS_VPO_AVR_SWITCH_H
#define LLVM_ANALYSIS_VPO_AVR_SWITCH_H

#include "llvm/Analysis/Intel_VPO/Vecopt/VPOAvr.h"

namespace llvm { // LLVM Namespace
namespace vpo {  // VPO Vectorizer Namespace

/// \brief This avr node represents a switch found in IR. There are two derived
/// classes for this object. AVRSwitchIR for switches found in LLVM IR and
/// AVRSwitchHIR for switches found in HIR.
class AVRSwitch : public AVR {

  typedef AVRContainerTy ChildrenTy;

  // Iterators for switch cases.
  typedef ChildrenTy::iterator case_child_iterator;
  typedef ChildrenTy::const_iterator const_case_child_iterator;
  typedef ChildrenTy::reverse_iterator reverse_case_child_iterator;
  typedef ChildrenTy::const_reverse_iterator const_reverse_case_child_iterator;

private:
  /// Children - Container for all of the switches case's children. This single
  /// single container holds all cases with the default case first.
  ChildrenTy Children;

  // ToDo: Need member data to obtain label/condition of switch

protected:
  /// CaseBegin - Iterators pointing to the begining of each switch case,
  /// excluding
  /// the default case.
  SmallVector<case_child_iterator, 5> CaseBegin;

  ///\brief Object constructor, AVR switch should not be instantiated at this
  /// level. Use derived classes to create an Avr Switch.
  AVRSwitch(unsigned SCID);

  /// \brief Destructor for this object.
  virtual ~AVRSwitch() override {}

  ///\brief Adds a case to the switch.
  void addCase();

  ///\brief Removes the nth (CaseNumber) case from the switch.
  void removeCase(unsigned CaseNumber);

  /// Internal iterator methods for accessing switch cases.
  case_child_iterator case_child_begin_internal(unsigned CaseNumber);
  const_case_child_iterator
  case_child_begin_internal(unsigned CaseNumber) const;
  case_child_iterator case_child_end_internal(unsigned CaseNumber);
  const_case_child_iterator case_child_end_internal(unsigned CaseNumber) const;
  reverse_case_child_iterator case_child_rbegin_internal(unsigned CaseNumber);
  const_reverse_case_child_iterator
  case_child_rbegin_internal(unsigned CaseNumber) const;
  reverse_case_child_iterator case_child_rend_internal(unsigned CaseNumber);
  const_reverse_case_child_iterator
  case_child_rend_internal(unsigned CaseNumber) const;

  /// \brief Internal implementation, will return the first child of
  /// the given CaseNumber case.
  AVR *getCaseFirstChildInternal(unsigned CaseNumber);

  const AVR *getCaseFirstChildInternal(unsigned CaseNumber) const {
    return const_cast<AVRSwitch *>(this)->getCaseFirstChildInternal(CaseNumber);
  }

  /// \brief Implements getLastCaseChild() functionality.
  AVR *getCaseLastChildInternal(unsigned CaseNumber);
  const AVR *getCaseLastChildInternal(unsigned CaseNumber) const {
    return const_cast<AVRSwitch *>(this)->getCaseLastChildInternal(CaseNumber);
  }

  /// \brief Implements hasCaseChildren() functionality.
  bool hasCaseChildrenInternal(unsigned CaseNumber) const {
    return (case_child_begin_internal(CaseNumber) !=
            case_child_end_internal(CaseNumber));
  }

  // Only this utility class should be used to modify/delete AVR nodes.
  friend class AVRUtils;

public:
  /// \brief Returns the number of cases switch contains excluding the default
  /// case.
  unsigned getNumCases() const { return CaseBegin.size(); }

  /// \brief Returns true if default case has at least one child.
  bool hasDefaultCaseChildren() const { return hasCaseChildrenInternal(0); }

  /// \brief Returns the number of default case children.
  unsigned getNumDefaultCaseChildren() const {
    return std::distance(default_case_child_begin(), default_case_child_end());
  }

  // Default case iterators

  case_child_iterator default_case_child_begin() {
    return case_child_begin_internal(0);
  }

  case_child_iterator default_case_child_end() {
    return case_child_end_internal(0);
  }

  const_case_child_iterator default_case_child_begin() const {
    return const_cast<AVRSwitch *>(this)->default_case_child_begin();
  }

  const_case_child_iterator default_case_child_end() const {
    return const_cast<AVRSwitch *>(this)->default_case_child_end();
  }

  reverse_case_child_iterator default_case_child_rbegin() {
    return case_child_rbegin_internal(0);
  }

  reverse_case_child_iterator default_case_child_rend() {
    return case_child_rend_internal(0);
  }

  const_reverse_case_child_iterator default_case_child_rbegin() const {
    return const_cast<AVRSwitch *>(this)->default_case_child_rbegin();
  }

  const_reverse_case_child_iterator default_case_child_rend() const {
    return const_cast<AVRSwitch *>(this)->default_case_child_rend();
  }

  // Switch case iterators

  // First case in case children container is default case. Use the methods
  // to access CaseNumber range of [1, Number of Cases]
  case_child_iterator case_child_begin(unsigned CaseNumber) {
    assert((CaseNumber > 0) && (CaseNumber <= getNumCases()) &&
           "Switch case number is out of range!");
    return case_child_begin_internal(CaseNumber);
  }

  case_child_iterator case_child_end(unsigned CaseNumber) {
    assert((CaseNumber > 0) && (CaseNumber <= getNumCases()) &&
           "Switch case number is out of range!");
    return case_child_end_internal(CaseNumber);
  }

  const_case_child_iterator case_child_begin(unsigned CaseNumber) const {
    return const_cast<AVRSwitch *>(this)->case_child_begin(CaseNumber);
  }

  const_case_child_iterator case_child_end(unsigned CaseNumber) const {
    return const_cast<AVRSwitch *>(this)->case_child_end(CaseNumber);
  }

  reverse_case_child_iterator case_child_rbegin(unsigned CaseNumber) {
    assert((CaseNumber > 0) && (CaseNumber <= getNumCases()) &&
           "Switch case number is out of range!");
    return case_child_rbegin_internal(CaseNumber);
  }

  reverse_case_child_iterator case_child_rend(unsigned CaseNumber) {
    assert((CaseNumber > 0) && (CaseNumber <= getNumCases()) &&
           "Switch case number is out of range!");
    return case_child_rend_internal(CaseNumber);
  }

  const_reverse_case_child_iterator child_rbegin(unsigned CaseNumber) const {
    return const_cast<AVRSwitch *>(this)->case_child_rbegin(CaseNumber);
  }

  const_reverse_case_child_iterator case_child_rend(unsigned CaseNumber) const {
    return const_cast<AVRSwitch *>(this)->case_child_rend(CaseNumber);
  }

  /// \brief Returns true if this case has at least one child.
  /// Range of CaseNum is [1, getNumCases()]. 0th case is reserved for default.
  bool hasCaseChildren(unsigned CaseNumber) const {
    assert((CaseNumber > 0) && (CaseNumber <= getNumCases()) &&
           "Switch case number is out of range!");
    return hasCaseChildrenInternal(CaseNumber);
  }

  /// \brief Returns the number of case children.
  /// Range of CaseNum is [1, getNumCases()]. 0th case is reserved for default.
  unsigned getNumCaseChildren(unsigned CaseNumber) const {
    return std::distance(case_child_begin(CaseNumber),
                         case_child_end(CaseNumber));
  }

  /// \brief Returns the first default case child if it exists, otherwise
  /// returns null.
  AVR *getDefaultCaseFirstChild() { return getCaseFirstChildInternal(0); }

  const AVR *getDefaultFirstCaseChild() const {
    return const_cast<AVRSwitch *>(this)->getDefaultCaseFirstChild();
  }

  /// \brief Returns the last default case child if it exists, otherwise returns
  /// null.
  AVR *getDefaultCaseLastChild() { return getCaseLastChildInternal(0); }

  const AVR *getDefaultCaseLastChild() const {
    return const_cast<AVRSwitch *>(this)->getDefaultCaseLastChild();
  }

  /// \brief Returns the first case child if it exists, otherwise returns null.
  /// Range of CaseNum is [1, getNumCases()].
  AVR *getFirstCaseChild(unsigned CaseNumber) {
    assert((CaseNumber > 0) && (CaseNumber <= getNumCases()) &&
           "Switch case number is out of range!");
    return getCaseFirstChildInternal(CaseNumber);
  }

  const AVR *getFirstCaseChild(unsigned CaseNumber) const {
    return const_cast<AVRSwitch *>(this)->getFirstCaseChild(CaseNumber);
  }

  /// \brief Returns the last case child if it exists, otherwise returns null.
  /// Range of CaseNum is [1, getNumCases()].
  AVR *getCaseLastChild(unsigned CaseNumber) {
    assert((CaseNumber > 0) && (CaseNumber <= getNumCases()) &&
           "Switch case number is out of range!");
    return getCaseLastChildInternal(CaseNumber);
  }

  const AVR *getCaseLastChild(unsigned CaseNumber) const {
    return const_cast<AVRSwitch *>(this)->getCaseLastChild(CaseNumber);
  }

  /// \brief Clone method for AVR Switch
  AVRSwitch *clone() const override;

  /// \brief Method for supporting type inquiry through isa, cast, and dyn_cast.
  static bool classof(const AVR *Node) {
    return (Node->getAVRID() >= AVR::AVRSwitchNode &&
            Node->getAVRID() < AVR::AVRSwitchLastNode);
  }

  /// \brief Prints break statement for given case if case breaks
  void printBreak(formatted_raw_ostream &OS, unsigned Depth,
                  unsigned CaseNum) const;

  /// \brief Prints the AvrSwitch node.
  void print(formatted_raw_ostream &OS, unsigned Depth,
             VerbosityLevel VLevel) const override;

  /// \brief Returns a constant StringRef for the codition of the switch.
  virtual void printConditionValueName(formatted_raw_ostream &OS) const = 0;

  /// \brief Returns a constant StringRef for the type name of this node.
  virtual StringRef getAvrTypeName() const override;

  /// \brief Returns the value name of this node.
  virtual std::string getAvrValueName() const = 0;
};

} // End VPO Vectorizer Namespace
} // End LLVM Namespace

#endif // LLVM_ANALYSIS_VPO_AVR_SWITCH_H
