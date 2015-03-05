//===--------------- HLIf.h - High level IR node ----------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file defines the HLIf node.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_IR_INTEL_LOOPIR_HLIF_H
#define LLVM_IR_INTEL_LOOPIR_HLIF_H

#include "llvm/IR/Intel_LoopIR/HLDDNode.h"
#include "llvm/IR/InstrTypes.h"

namespace llvm {

namespace loopopt {

class HLLoop;

/// \brief High level node representing a conditional branch
///
/// Sample HLIf-
/// If( (Op1 Pred1 Op2) Conj1 (Op3 Pred2 Op4) )
class HLIf : public HLDDNode {
public:
  typedef std::vector<CmpInst::Predicate> PredTy;
  typedef std::vector<unsigned> ConjunctionTy;
  typedef HLContainerTy ChildNodeTy;

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
  /// It should contain at least one predicate.
  PredTy Preds;
  /// HLIf should contain two predicates per conjunction.
  ConjunctionTy Conjunctions;
  /// Contains both then and else children, in that order.
  /// Having a single container allows for more efficient and cleaner
  /// implementation of insert(Before/After) and remove(Before/After).
  ChildNodeTy Children;
  /// Iterator pointing to the begining of else children.
  ChildNodeTy::iterator ElseBegin;

protected:
  HLIf(CmpInst::Predicate FirstPred, DDRef *Ref1, DDRef *Ref2);
  ~HLIf() {}

  /// \brief Copy constructor used by cloning.
  HLIf(const HLIf &HLIfObj);

  friend class HLNodeUtils;
  /// Required to access setParent() for loop's ztt.
  friend class HLLoop;

  /// Implements getNumOperands() functionality.
  unsigned getNumOperandsInternal() const;

public:
  /// \brief Returns the underlying type of if.
  Type *getLLVMType() const;
  /// \brief Returns the vector of predicates associated with this if.
  const PredTy &getPredicates() const { return Preds; }

  /// \brief Returns the vector of conjunctions combining the predicates
  /// of this if.
  const ConjunctionTy &getConjunctions() const { return Conjunctions; }

  /// Children iterator methods
  then_iterator then_begin() { return Children.begin(); }
  const_then_iterator then_begin() const { return Children.begin(); }
  then_iterator then_end() { return ElseBegin; }
  const_then_iterator then_end() const { return ElseBegin; }

  reverse_then_iterator then_rbegin() {
    return ChildNodeTy::reverse_iterator(ElseBegin);
  }
  const_reverse_then_iterator then_rbegin() const {
    return ChildNodeTy::const_reverse_iterator(ElseBegin);
  }
  reverse_then_iterator then_rend() { return Children.rend(); }
  const_reverse_then_iterator then_rend() const { return Children.rend(); }

  else_iterator else_begin() { return ElseBegin; }
  const_else_iterator else_begin() const { return ElseBegin; }
  else_iterator else_end() { return Children.end(); }
  const_else_iterator else_end() const { return Children.end(); }

  reverse_else_iterator else_rbegin() { return Children.rbegin(); }
  const_reverse_else_iterator else_rbegin() const { return Children.rbegin(); }

  reverse_else_iterator else_rend() {
    return ChildNodeTy::reverse_iterator(ElseBegin);
  }
  const_reverse_else_iterator else_rend() const {
    return ChildNodeTy::const_reverse_iterator(ElseBegin);
  }

  /// Children acess methods
  unsigned getNumThenChildren() const {
    return std::distance(then_begin(), then_end());
  }
  bool isThenEmpty() const { return (then_begin() != then_end()); }
  unsigned getNumElseChildren() const {
    return std::distance(else_begin(), else_end());
  }
  bool isElseEmpty() const { return (else_begin() != else_end()); }

  /// \brief Method for supporting type inquiry through isa, cast, and dyn_cast.
  static bool classof(const HLNode *Node) {
    return Node->getHLNodeID() == HLNode::HLIfVal;
  }

  /// clone() - Create a copy of 'this' HLIf that is identical in all
  /// ways except the following:
  ///   * The HLIf has no parent
  HLIf *clone() const override;

  /// \brief Returns the number of operands this HLIf is supposed to have.
  unsigned getNumOperands() const override;

  /// TODO: Complete interface for adding/removing conjunctions. This should
  /// also resize DDRefs.
  /// \brief Add new conjunction and predicate into If.
  void addConjunction(unsigned Conj, CmpInst::Predicate Pred, DDRef *Ref1,
                      DDRef *Ref2);
};

} // End namespace loopopt

} // End namespace llvm

#endif
