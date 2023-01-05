//===--------------- HLIf.h - High level IR if node -------------*- C++ -*-===//
//
// Copyright (C) 2015-2020 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// This file defines the HLIf node.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_IR_INTEL_LOOPIR_HLIF_H
#define LLVM_IR_INTEL_LOOPIR_HLIF_H

#include "llvm/Analysis/Intel_LoopAnalysis/IR/HLDDNode.h"
#include "llvm/IR/InstrTypes.h"

namespace llvm {

namespace loopopt {

class HLLoop;

/// High level node representing a conditional branch
///
/// Sample HLIf-
/// If( (Op1 Pred1 Op2) AND (Op3 Pred2 Op4) )
class HLIf final : public HLDDNode {
public:
  typedef SmallVector<HLPredicate, 2> PredicateContainerTy;
  typedef HLContainerTy ChildNodeTy;

  /// Iterators to iterate over predicates
  typedef PredicateContainerTy::iterator pred_iterator;
  typedef PredicateContainerTy::const_iterator const_pred_iterator;
  typedef PredicateContainerTy::reverse_iterator reverse_pred_iterator;
  typedef PredicateContainerTy::const_reverse_iterator
      const_reverse_pred_iterator;

  /// Iterators to iterate over then/else children nodes
  typedef ChildNodeTy::iterator then_iterator;
  typedef ChildNodeTy::const_iterator const_then_iterator;
  typedef ChildNodeTy::reverse_iterator reverse_then_iterator;
  typedef ChildNodeTy::const_reverse_iterator const_reverse_then_iterator;

  typedef then_iterator else_iterator;
  typedef const_then_iterator const_else_iterator;
  typedef reverse_then_iterator reverse_else_iterator;
  typedef const_reverse_then_iterator const_reverse_else_iterator;

private:
  /// HLIf should contain two operands(DDRefs) per predicate unless the
  /// predicate is FCMP_TRUE or FCMP_FALSE in which case they can be null.
  /// It should contain at least one predicate. Predicates are joined using the
  /// implicit AND conjunction.
  PredicateContainerTy Predicates;

  /// Contains both then and else children, in that order.
  /// Having a single container allows for more efficient and cleaner
  /// implementation of insert(Before/After) and remove(Before/After).
  ChildNodeTy Children;
  /// Iterator pointing to the begining of else children.
  ChildNodeTy::iterator ElseBegin;

  /// Disables the predicate optimization.
  bool UnswitchDisabled;

  // Branch debug location.
  DebugLoc BranchDbgLoc;

  // Tag applied to Ifs created for runtime DD.
  unsigned MVTag;

protected:
  HLIf(HLNodeUtils &HNU, const HLPredicate &FirstPred, RegDDRef *Ref1,
       RegDDRef *Ref2);

  /// Copy constructor used by cloning.
  HLIf(const HLIf &HLIfObj);

  friend class HLNodeUtils;
  /// Required to access setParent() for loop's ztt.
  friend class HLLoop;

  /// Implements getNumOperands() functionality.
  unsigned getNumOperandsInternal() const;

  /// Initializes some of the members to bring the object in a sane
  /// state.
  void initialize();

  /// Returns the offset of the LHS/RHS DDRef associated with this
  /// predicate.
  unsigned getPredicateOperandDDRefOffset(const_pred_iterator CPredI,
                                          bool IsLHS) const;

  /// Returns non-const iterator version of const_pred_iterator.
  pred_iterator getNonConstPredIterator(const_pred_iterator CPredI);

  /// Clone Implementation
  /// This function populates the GotoList with Goto branching within the
  /// cloned If and LabelMap with Old and New Labels. Returns a cloned If.
  HLIf *cloneImpl(GotoContainerTy *GotoList, LabelMapTy *LabelMap,
                  HLNodeMapper *NodeMapper) const override;

  /// Implements print*Header() functionality. Loop parameter tells
  /// whether we are printing a ZTT or a regular HLIf.
  void printHeaderImpl(formatted_raw_ostream &OS, unsigned Depth,
                       const HLLoop *Loop, bool Detailed) const;

  /// Prints this HLIf as a ZTT of Loop.
  void printZttHeader(formatted_raw_ostream &OS, const HLLoop *Loop) const;

  /// Returns the LHS/RHS operand of the predicate based on the IsLHS flag.
  RegDDRef *getPredicateOperandDDRef(const_pred_iterator CPredI,
                                     bool IsLHS) const;

  /// Sets the LHS/RHS operand of the predicate based on the IsLHS flag.
  void setPredicateOperandDDRef(RegDDRef *Ref, const_pred_iterator CPredI,
                                bool IsLHS);

  /// Removes and returns the LHS/RHS operand of the predicate based on the
  /// IsLHS flag.
  RegDDRef *removePredicateOperandDDRef(const_pred_iterator CPredI, bool IsLHS);

public:
  /// Predicate iterator methods
  const_pred_iterator pred_begin() const { return Predicates.begin(); }
  const_pred_iterator pred_end() const { return Predicates.end(); }

  const_reverse_pred_iterator pred_rbegin() const {
    return Predicates.rbegin();
  }
  const_reverse_pred_iterator pred_rend() const { return Predicates.rend(); }

  /// Returns the number of predicates associated with this if.
  unsigned getNumPredicates() const { return Predicates.size(); }

  /// Children iterator methods
  ChildNodeTy::iterator child_begin() { return Children.begin(); }
  ChildNodeTy::iterator child_end() { return Children.end(); }

  ChildNodeTy::const_iterator child_begin() const { return Children.begin(); }
  ChildNodeTy::const_iterator child_end() const { return Children.end(); }

  then_iterator then_begin() { return Children.begin(); }
  const_then_iterator then_begin() const { return Children.begin(); }
  then_iterator then_end() { return ElseBegin; }
  const_then_iterator then_end() const { return ElseBegin; }

  reverse_then_iterator then_rbegin() { return ++ElseBegin.getReverse(); }
  const_reverse_then_iterator then_rbegin() const {
    return ++ElseBegin.getReverse();
  }
  reverse_then_iterator then_rend() { return Children.rend(); }
  const_reverse_then_iterator then_rend() const { return Children.rend(); }

  else_iterator else_begin() { return then_end(); }
  const_else_iterator else_begin() const { return then_end(); }
  else_iterator else_end() { return Children.end(); }
  const_else_iterator else_end() const { return Children.end(); }

  reverse_else_iterator else_rbegin() { return Children.rbegin(); }
  const_reverse_else_iterator else_rbegin() const { return Children.rbegin(); }

  reverse_else_iterator else_rend() { return then_rbegin(); }
  const_reverse_else_iterator else_rend() const { return then_rbegin(); }

  /// Children acess methods

  /// Returns the first then child if it exists, otherwise
  /// returns null.
  HLNode *getFirstThenChild();
  const HLNode *getFirstThenChild() const {
    return const_cast<HLIf *>(this)->getFirstThenChild();
  }
  /// Returns the last then child if it exists, otherwise
  /// returns null.
  HLNode *getLastThenChild();
  const HLNode *getLastThenChild() const {
    return const_cast<HLIf *>(this)->getLastThenChild();
  }

  /// Returns the number of then children.
  unsigned getNumThenChildren() const {
    return std::distance(then_begin(), then_end());
  }
  /// Returns true if it has then children.
  bool hasThenChildren() const { return (then_begin() != then_end()); }

  /// Returns the first else child if it exists, otherwise returns null.
  HLNode *getFirstElseChild();
  const HLNode *getFirstElseChild() const {
    return const_cast<HLIf *>(this)->getFirstElseChild();
  }
  /// Returns the last else child if it exists, otherwise returns null.
  HLNode *getLastElseChild();
  const HLNode *getLastElseChild() const {
    return const_cast<HLIf *>(this)->getLastElseChild();
  }

  /// Returns the number of else children.
  unsigned getNumElseChildren() const {
    return std::distance(else_begin(), else_end());
  }
  /// Returns true if it has else children.
  bool hasElseChildren() const { return (else_begin() != else_end()); }

  /// Method for supporting type inquiry through isa, cast, and dyn_cast.
  static bool classof(const HLNode *Node) {
    return Node->getHLNodeClassID() == HLNode::HLIfVal;
  }

  /// Creates a clone without children nodes.
  HLIf *cloneEmpty() const;

  /// clone() - Create a copy of 'this' HLIf that is identical in all
  /// ways except the following:
  ///   * The HLIf has no parent
  /// This method will automatically update the goto branches with new labels
  /// inside the cloned If.
  HLIf *clone(HLNodeMapper *NodeMapper = nullptr) const override;

  /// Prints HLIf header only: if (...condition...)
  void printHeader(formatted_raw_ostream &OS, unsigned Depth,
                   bool Detailed = false) const;

  /// Prints HLIf.
  virtual void print(formatted_raw_ostream &OS, unsigned Depth,
                     bool Detailed = false) const override;

  /// Returns the number of operands this HLIf is supposed to have.
  unsigned getNumOperands() const override { return getNumOperandsInternal(); }

  /// Adds new predicate in HLIf.
  void addPredicate(const HLPredicate &Pred, RegDDRef *Ref1, RegDDRef *Ref2);

  /// Removes the associated predicate and operand DDRefs(not destroyed).
  /// Example-
  /// Before:
  /// If((Op1 Pred1 Op2) AND (Op3 Pred2 Op4) AND (Op5 Pred3 Op6))
  ///
  /// removePredicate(SecondPredIter);
  ///
  /// After:
  /// If((Op1 Pred1 Op2) AND (Op5 Pred3 Op6))
  void removePredicate(const_pred_iterator CPredI);

  /// Replaces existing predicate pointed to by CPredI, by NewPred.
  void replacePredicate(const_pred_iterator CPredI, const HLPredicate &NewPred);

  /// Replaces existing PredicateTy in CPredI, by NewPred.
  void replacePredicate(const_pred_iterator CPredI, PredicateTy NewPred);

  /// Inverts PredicateTy in CPredI.
  void invertPredicate(const_pred_iterator CPredI);

  /// Returns the LHS operand of the predicate.
  RegDDRef *getLHSPredicateOperandDDRef(const_pred_iterator CPredI) const {
    return getPredicateOperandDDRef(CPredI, true);
  }

  /// Returns the RHS operand of the predicate.
  RegDDRef *getRHSPredicateOperandDDRef(const_pred_iterator CPredI) const {
    return getPredicateOperandDDRef(CPredI, false);
  }

  /// Sets the LHS operand of the predicate.
  void setLHSPredicateOperandDDRef(RegDDRef *Ref, const_pred_iterator CPredI) {
    setPredicateOperandDDRef(Ref, CPredI, true);
  }

  /// Sets the RHS operand of the predicate.
  void setRHSPredicateOperandDDRef(RegDDRef *Ref, const_pred_iterator CPredI) {
    setPredicateOperandDDRef(Ref, CPredI, false);
  }

  /// Removes and returns the LHS operand of the predicate.
  RegDDRef *removeLHSPredicateOperandDDRef(const_pred_iterator CPredI) {
    return removePredicateOperandDDRef(CPredI, true);
  }

  /// Removes and returns the RHS operand DDRef of the predicate.
  RegDDRef *removeRHSPredicateOperandDDRef(const_pred_iterator CPredI) {
    return removePredicateOperandDDRef(CPredI, false);
  }

  /// Returns true if \p Node is contained inside *then* or *else* branch
  /// of the HLIf.
  bool isThenChild(const HLNode *Node) const;
  bool isElseChild(const HLNode *Node) const;

  /// Verifies HLIf integrity.
  virtual void verify() const override;

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  LLVM_DUMP_METHOD
  void dumpHeader() const;
#endif

  /// Returns true if the HLIf is known to always compute to the specific
  /// result, which is returned to the \p IsTrue.
  bool isKnownPredicate(bool *IsTrue = nullptr) const;

  const DebugLoc getDebugLoc() const override { return BranchDbgLoc; }
  void setDebugLoc(const DebugLoc &Loc) { BranchDbgLoc = Loc; }

  /// Returns true if this is the bottom test of its parent unknown loop.
  bool isUnknownLoopBottomTest() const;

  bool isUnswitchDisabled() const { return UnswitchDisabled; }
  void setUnswitchDisabled(bool Disabled = true) {
    UnswitchDisabled = Disabled;
  }

  /// Inverts the predicate and swaps the then-path with else-path,
  /// changing lexical order, but preserving logic.
  void invertPredAndReverse();

  /// Getter/Setter for MVTag set by Runtime DD multiversioning.
  unsigned getMVTag() const { return MVTag; }
  void setMVTag(unsigned Tag) { MVTag = Tag; }
};

} // End namespace loopopt

} // End namespace llvm

#endif
