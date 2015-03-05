//===------------- HLLoop.h - High level IR node ----------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file defines the HLLoop node.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_IR_INTEL_LOOPIR_HLLOOP_H
#define LLVM_IR_INTEL_LOOPIR_HLLOOP_H

#include "llvm/IR/Intel_LoopIR/HLDDNode.h"
#include "llvm/IR/Intel_LoopIR/HLIf.h"
#include "llvm/IR/InstrTypes.h"

namespace llvm {

namespace loopopt {

class CanonExpr;

/// \brief High level node representing a loop
class HLLoop : public HLDDNode {
public:
  typedef HLIf::PredTy ZttPredTy;
  typedef HLIf::ConjunctionTy ZttConjunctionTy;
  typedef HLContainerTy ChildNodeTy;
  typedef ChildNodeTy PreheaderTy;
  typedef ChildNodeTy PostexitTy;

  /// Iterator to iterate over ZTT DDRefs
  typedef HLIf::ddref_iterator ztt_ddref_iterator;
  typedef HLIf::const_ddref_iterator const_ztt_ddref_iterator;
  typedef HLIf::reverse_ddref_iterator reverse_ztt_ddref_iterator;
  typedef HLIf::const_reverse_ddref_iterator const_reverse_ztt_ddref_iterator;

  /// Iterators to iterate over children nodes
  typedef ChildNodeTy::iterator child_iterator;
  typedef ChildNodeTy::const_iterator const_child_iterator;
  typedef ChildNodeTy::reverse_iterator reverse_child_iterator;
  typedef ChildNodeTy::const_reverse_iterator const_reverse_child_iterator;

  /// Iterators to iterate over preheader nodes
  typedef child_iterator pre_iterator;
  typedef const_child_iterator const_pre_iterator;
  typedef reverse_child_iterator reverse_pre_iterator;
  typedef const_reverse_child_iterator const_reverse_pre_iterator;

  /// Iterators to iterate over postexit nodes
  typedef child_iterator post_iterator;
  typedef const_child_iterator const_post_iterator;
  typedef reverse_child_iterator reverse_post_iterator;
  typedef const_reverse_child_iterator const_reverse_post_iterator;

private:
  HLIf *Ztt;
  /// Contains preheader, child and postexit nodes, in that order.
  /// Having a single container allows for more efficient and cleaner
  /// implementation of insert(Before/After) and remove(Before/After).
  ChildNodeTy Children;
  /// Iterator pointing to beginning of children nodes.
  ChildNodeTy::iterator ChildBegin;
  /// Iterator pointing to beginning of postexit nodes.
  ChildNodeTy::iterator PostexitBegin;
  /// This flag shouldn't change during the lifetime of the loop.
  const bool IsDoWhile;
  unsigned NumExits;
  unsigned NestingLevel;
  bool IsInnermost;

protected:
  HLLoop(HLIf *ZttIf, DDRef *LowerDDRef, DDRef *TripCountDDRef,
         DDRef *StrideDDRef, bool IsDoWh, unsigned NumEx);

  ~HLLoop() {}

  /// \brief Copy constructor used by cloning.
  HLLoop(const HLLoop &HLLoopObj);

  friend class HLNodeUtils;

  void setNestingLevel(unsigned Level) { NestingLevel = Level; }
  void setInnermost(bool IsInnermst) { IsInnermost = IsInnermst; }

  /// \brief Hides HLDDNode's getOperandDDref(). Users are expected to use
  /// loop specific functions.
  DDRef *getOperandDDref(unsigned OperandNum);
  const DDRef *getOperandDDref(unsigned OperandNum) const;
  /// \brief Hides HLDDNode's setOperandDDref(). Users are expected to use
  /// loop specific functions.
  void setOperandDDRef(DDRef *, unsigned OperandNum);
  /// \brief Hides HLDDNode's removeOperandDDref(). Users are expected to use
  /// loop specific functions.
  DDRef *removeOperandDDref(unsigned OperandNum);

  /// \brief Returns the number of DDRefs associated with only the loop
  /// without the ztt.
  /// Comes down to DDRefs for lower, tripcount and stride.
  unsigned getNumLoopDDRefs() const { return 3; }

  /// \brief Returns the number of operands this loop is supposed to have.
  unsigned getNumOperands() const override;

  /// \brief Sets DDRefs' size to getNumLoopDDRefs().
  void resizeToNumLoopDDRefs();

  /// \brief Used to implement get*CanonExpr() functionality.
  const CanonExpr *getLoopCanonExpr(const DDRef *Ref) const;

  /// Implements getNumOperands() functionality.
  unsigned getNumOperandsInternal() const;

public:
  /// \brief Returns true if ztt is present.
  bool hasZtt() const { return Ztt != nullptr; }

  /// \brief sets the Ztt for HLLoop.
  void setZtt(HLIf *ZttIf);
  /// \brief Removes and returns the Ztt for HLLoop.
  HLIf *removeZtt();

  /// \brief Returns the underlying type of ZTT.
  Type *getZttLLVMType() const;
  /// \brief Returns the vector of predictes associated with this ZTT.
  /// TODO: make this consistent with HLIf's interface.
  const ZttPredTy &getZttPredicates() const {
    assert(hasZtt() && "Ztt is absent");
    return Ztt->getPredicates();
  }

  /// \brief Returns the vector of conjunctions combining the predicates
  /// of this ZTT.
  /// TODO: make this consistent with HLIf's interface.
  const ZttConjunctionTy &getZttConjunctions() const {
    assert(hasZtt() && "Ztt is absent");
    return Ztt->getConjunctions();
  }

  /// \brief Returns the DDRef associated with loop lower bound.
  /// The first DDRef is associated with lower bound.
  DDRef *getLowerDDRef();
  const DDRef *getLowerDDRef() const;
  /// \brief Sets the DDRef associated with loop lower bound.
  void setLowerDDRef(DDRef *Ref);
  /// \brief Removes and returns the DDRef associated with loop lower bound.
  DDRef *removeLowerDDRef();

  /// \brief Returns the DDRef associated with loop trip count.
  /// The second DDRef is associated with trip count.
  DDRef *getTripCountDDRef();
  const DDRef *getTripCountDDRef() const;
  /// \brief Sets the DDRef associated with loop trip count.
  void setTripCountDDRef(DDRef *Ref);
  /// \brief Removes and returns the DDRef associated with loop trip count.
  DDRef *removeTripCountDDRef();

  /// \brief Returns the DDRef associated with loop stride.
  /// The third DDRef is associated with stride.
  DDRef *getStrideDDRef();
  const DDRef *getStrideDDRef() const;
  /// \brief Sets the DDRef associated with loop stride.
  void setStrideDDRef(DDRef *Ref);
  /// \brief Removes and returns the DDRef associated with loop stride.
  DDRef *removeStrideDDRef();

  /// \brief Returns the DDRef associated with the Nth ZTT operand (starting
  /// with 0) of Loop.
  DDRef *getZttOperandDDRef(unsigned OperandNum);
  const DDRef *getZttOperandDDRef(unsigned OperandNum) const;
  /// \brief Sets the DDRef associated with the Nth ZTT operand (starting with
  /// 0) of Loop.
  void setZttOperandDDRef(DDRef *Ref, unsigned OperandNum);
  /// \brief Removes and returns the DDRef associated with the Nth ZTT operand
  /// (starting with 0) of Loop.
  DDRef *removeZttOperandDDRef(unsigned OperandNum);

  /// \brief Returns the CanonExpr associated with loop lower bound.
  const CanonExpr *getLowerCanonExpr() const;

  /// \brief Returns the CanonExpr associated with loop trip count.
  const CanonExpr *getTripCountCanonExpr() const;

  /// \brief Returns the CanonExpr associated with loop stride.
  const CanonExpr *getStrideCanonExpr() const;

  /// \brief Returns the CanonExpr associated with loop upper bound.
  /// Returns a newly allocated CanonExpr as this information is not
  ///  directly stored so use with caution.
  const CanonExpr *getUpperCanonExpr() const;

  /// \brief Returns true if this is a do loop.
  bool isDoLoop() const {
    return (!IsDoWhile && (NumExits == 1) && getTripCountDDRef());
  }

  /// \brief Returns true if this is a do-while loop.
  bool isDoWhileLoop() const { return IsDoWhile; }

  /// \brief Returns true if this is a do multi-exit loop.
  bool isDoMultiExitLoop() const {
    return (!IsDoWhile && (NumExits > 1) && getTripCountDDRef());
  }

  /// \brief Returns true if this is an unknown loop.
  bool isUnknownLoop() const { return !getTripCountDDRef(); }

  /// \brief Returns the number of exits of the loop.
  unsigned getNumExits() const { return NumExits; }
  /// \brief Sets the number of exits of the loop.
  void setNumExits(unsigned NumEx);

  /// \brief Returns the nesting level of the loop.
  unsigned getNestingLevel() const { return NestingLevel; }
  /// \brief Returns true if this is the innermost loop in the loopnest.
  bool isInnermost() const { return IsInnermost; }

  /// Preheader iterator methods
  pre_iterator pre_begin() { return Children.begin(); }
  const_pre_iterator pre_begin() const { return Children.begin(); }
  pre_iterator pre_end() { return ChildBegin; }
  const_pre_iterator pre_end() const { return ChildBegin; }

  reverse_pre_iterator pre_rbegin() {
    return ChildNodeTy::reverse_iterator(ChildBegin);
  }
  const_reverse_pre_iterator pre_rbegin() const {
    return ChildNodeTy::const_reverse_iterator(ChildBegin);
  }

  reverse_pre_iterator pre_rend() { return Children.rend(); }
  const_reverse_pre_iterator pre_rend() const { return Children.rend(); }

  /// Preheader acess methods
  unsigned getNumPreheader() const {
    return std::distance(pre_begin(), pre_end());
  }
  bool hasPreheader() const { return (pre_begin() != pre_end()); }

  /// Postexit iterator methods
  post_iterator post_begin() { return PostexitBegin; }
  const_post_iterator post_begin() const { return PostexitBegin; }
  post_iterator post_end() { return Children.end(); }
  const_post_iterator post_end() const { return Children.end(); }

  reverse_post_iterator post_rbegin() { return Children.rbegin(); }
  const_reverse_post_iterator post_rbegin() const { return Children.rbegin(); }
  reverse_post_iterator post_rend() {
    return ChildNodeTy::reverse_iterator(PostexitBegin);
  }
  const_reverse_post_iterator post_rend() const {
    return ChildNodeTy::const_reverse_iterator(PostexitBegin);
  }

  /// Postexit acess methods
  unsigned getNumPostexit() const {
    return std::distance(post_begin(), post_end());
  }
  bool hasPostexit() const { return (post_begin() != post_end()); }

  /// Children iterator methods
  child_iterator child_begin() { return pre_end(); }
  const_child_iterator child_begin() const { return pre_end(); }
  child_iterator child_end() { return post_begin(); }
  const_child_iterator child_end() const { return post_begin(); }

  reverse_child_iterator child_rbegin() { return post_rend(); }
  const_reverse_child_iterator child_rbegin() const { return post_rend(); }
  reverse_child_iterator child_rend() { return pre_rbegin(); }
  const_reverse_child_iterator child_rend() const { return pre_rbegin(); }

  /// Children acess methods
  unsigned getNumChildren() const {
    return std::distance(child_begin(), child_end());
  }
  bool hasChildren() const { return (child_begin() != child_end()); }

  /// ZTT DDRef iterator methods
  ztt_ddref_iterator ztt_ddref_begin() {
    if (!hasZtt()) {
      return ddref_end();
    }
    return ddref_begin() + getNumLoopDDRefs();
  }
  const_ztt_ddref_iterator ztt_ddref_begin() const {
    if (!hasZtt()) {
      return ddref_end();
    }
    return ddref_begin() + getNumLoopDDRefs();
  }
  ztt_ddref_iterator ztt_ddref_end() { return ddref_end(); }
  const_ztt_ddref_iterator ztt_ddref_end() const { return ddref_end(); }

  reverse_ztt_ddref_iterator ztt_ddref_rbegin() { return ddref_rbegin(); }
  const_reverse_ztt_ddref_iterator ztt_ddref_rbegin() const {
    return ddref_rbegin();
  }
  reverse_ztt_ddref_iterator ztt_ddref_rend() {
    if (!hasZtt()) {
      return ddref_rend();
    }
    return ddref_rend() - getNumLoopDDRefs();
  }
  const_reverse_ztt_ddref_iterator ztt_ddref_rend() const {
    if (!hasZtt()) {
      return ddref_rend();
    }
    return ddref_rend() - getNumLoopDDRefs();
  }

  /// \brief Method for supporting type inquiry through isa, cast, and dyn_cast.
  static bool classof(const HLNode *Node) {
    return Node->getHLNodeID() == HLNode::HLLoopVal;
  }

  /// clone() - Create a copy of 'this' HLLoop that is identical in all
  /// ways except the following:
  ///   * Data members that depend on where the cloned loop lives in HIR (like
  ///     parent, nesting level) are not copied. They will be updated by HLNode
  ///     insertion/removal utilities.
  HLLoop *clone() const override;

  /// \brief Returns the number of operands associated with the loop ztt.
  unsigned getNumZttOperands() const;
};

} // End namespace loopopt

} // End namespace llvm

#endif
