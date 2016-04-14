//===----------- HLLoop.h - High level IR loop node -------------*- C++ -*-===//
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
// This file defines the HLLoop node.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_IR_INTEL_LOOPIR_HLLOOP_H
#define LLVM_IR_INTEL_LOOPIR_HLLOOP_H

#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/Intel_LoopIR/HLDDNode.h"
#include "llvm/IR/Intel_LoopIR/HLIf.h"
#include "llvm/IR/Intel_LoopIR/RegDDRef.h"

namespace llvm {

class Loop;

namespace loopopt {

class CanonExpr;
class RegDDRef;

/// \brief High level node representing a loop
class HLLoop : public HLDDNode {
public:
  typedef HLContainerTy ChildNodeTy;
  typedef ChildNodeTy PreheaderTy;
  typedef ChildNodeTy PostexitTy;

  /// Iterators to iterate over ZTT predicates
  typedef HLIf::pred_iterator ztt_pred_iterator;
  typedef HLIf::const_pred_iterator const_ztt_pred_iterator;
  typedef HLIf::reverse_pred_iterator reverse_ztt_pred_iterator;
  typedef HLIf::const_reverse_pred_iterator const_reverse_ztt_pred_iterator;

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
  const Loop *OrigLoop;
  HLIf *Ztt;
  /// Contains preheader, child and postexit nodes, in that order.
  /// Having a single container allows for more efficient and cleaner
  /// implementation of insert(Before/After) and remove(Before/After).
  ChildNodeTy Children;
  /// Iterator pointing to beginning of children nodes.
  ChildNodeTy::iterator ChildBegin;
  /// Iterator pointing to beginning of postexit nodes.
  ChildNodeTy::iterator PostexitBegin;
  unsigned NumExits;
  unsigned NestingLevel;
  // This flag indicates if the loop is innermost or not. All loops are created
  // and cloned as innermost. Insert/remove utilities are updating this flag
  // assuming that it's true in a fresh HLLoop.
  bool IsInnermost;
  Type *IVType;

  // Indicates whether loop's IV is in signed range.
  bool IsNSW;

  // Temporary tag to mark loop as multiversioned.
  unsigned MVTag = 0;

protected:
  HLLoop(const Loop *LLVMLoop);
  HLLoop(HLIf *ZttIf, RegDDRef *LowerDDRef, RegDDRef *UpperDDRef,
         RegDDRef *StrideDDRef, unsigned NumEx);

  /// HLNodes are destroyed in bulk using HLNodeUtils::destroyAll(). iplist<>
  /// tries to access and destroy the nodes if we don't clear them out here.
  virtual ~HLLoop() override { Children.clearAndLeakNodesUnsafely(); }

  /// \brief Copy constructor used by cloning.
  /// CloneChildren parameter denotes if we want to clone
  /// children and preheader/postexit.
  HLLoop(const HLLoop &HLLoopObj, GotoContainerTy *GotoList,
         LabelMapTy *LabelMap, bool CloneChildren);

  friend class HLNodeUtils;
  friend class HIRParser; // accesses ZTT

  /// \brief Initializes some of the members to bring the loop in a sane state.
  void initialize();

  /// \brief Returns the Ztt of the loop.
  HLIf *getZtt() { return Ztt; }

  /// \brief Sets the nesting level of the loop.
  void setNestingLevel(unsigned Level) { NestingLevel = Level; }
  /// \brief Sets the Innermost flag to indicate if it is innermost loop.
  void setInnermost(bool IsInnermost) { this->IsInnermost = IsInnermost; }

  /// \brief Returns the number of DDRefs associated with only the loop
  /// without the ztt.
  /// Comes down to DDRefs for lower, tripcount and stride.
  unsigned getNumLoopDDRefs() const { return 3; }

  /// \brief Returns the number of operands this loop is supposed to have.
  unsigned getNumOperands() const override;

  /// \brief Sets DDRefs' size to getNumLoopDDRefs().
  void resizeToNumLoopDDRefs();

  /// \brief Used to implement get*CanonExpr() functionality.
  CanonExpr *getLoopCanonExpr(RegDDRef *Ref);
  const CanonExpr *getLoopCanonExpr(const RegDDRef *Ref) const;

  /// \brief Implements getNumOperands() functionality.
  unsigned getNumOperandsInternal() const;

  /// \brief Returns the DDRef offset of a ztt predicate.
  unsigned getZttPredicateOperandDDRefOffset(const_ztt_pred_iterator CPredI,
                                             bool IsLHS) const;

  /// \brief Clone Implementation
  /// This function populates the GotoList with Goto branches within the
  /// cloned loop and LabelMap with Old and New Labels. Returns a cloned loop.
  HLLoop *cloneImpl(GotoContainerTy *GotoList,
                    LabelMapTy *LabelMap) const override;

  /// \brief Used to print members of the loop which are otherwise hidden in
  /// pretty print like ztt, innermost flag etc.
  void printDetails(formatted_raw_ostream &OS, unsigned Depth) const;

public:
  /// \brief Prints HLLoop.
  virtual void print(formatted_raw_ostream &OS, unsigned Depth,
                     bool Detailed = false) const override;

  /// \brief Returns underlying LLVM loop.
  const Loop *getLLVMLoop() const { return OrigLoop; }

  /// \brief Returns the underlying type of the loop IV.
  Type *getIVType() const { return IVType; }
  /// \brief Sets the underlying type of the loop IV.
  void setIVType(Type *Ty) { IVType = Ty; }

  /// \brief Returns true if loop's IV is in signed range.
  bool isNSW() const { return IsNSW; }
  /// \brief Sets/resets IsNSW flag.
  void setNSW(bool IsNSW) { this->IsNSW = IsNSW; }

  /// \brief Returns true if ztt is present.
  bool hasZtt() const { return Ztt != nullptr; }

  /// \brief sets the Ztt for HLLoop.
  void setZtt(HLIf *ZttIf);

  /// \brief Removes and returns the Ztt for HLLoop if it exists, else returns
  /// nullptr.
  HLIf *removeZtt();

  /// \brief Removes and returns the OrigLoop
  const Loop *removeLLVMLoop();

  /// \brief Creates a Ztt for HLLoop. IsOverwrite flag
  /// indicates to overwrite existing Ztt or not.
  void createZtt(bool IsOverwrite = true);

  /// \brief Hoists the Ztt out of the loop. It returns a handle to the Ztt or
  /// nullptr if it doesn't exist.
  HLIf *extractZtt();

  /// ZTT Predicate iterator methods
  const_ztt_pred_iterator ztt_pred_begin() const {
    assert(hasZtt() && "Ztt is absent!");
    return Ztt->pred_begin();
  }
  const_ztt_pred_iterator ztt_pred_end() const {
    assert(hasZtt() && "Ztt is absent!");
    return Ztt->pred_end();
  }

  const_reverse_ztt_pred_iterator ztt_pred_rbegin() const {
    assert(hasZtt() && "Ztt is absent!");
    return Ztt->pred_rbegin();
  }
  const_reverse_ztt_pred_iterator ztt_pred_rend() const {
    assert(hasZtt() && "Ztt is absent!");
    return Ztt->pred_rend();
  }

  /// \brief Returns the number of predicates associated with this ZTT.
  unsigned getNumZttPredicates() const {
    assert(hasZtt() && "Ztt is absent!");
    return Ztt->getNumPredicates();
  }

  /// \brief Adds new predicate in ZTT.
  void addZttPredicate(PredicateTy Pred, RegDDRef *Ref1, RegDDRef *Ref2);

  /// \brief Removes the associated predicate and operand DDRefs(not destroyed).
  void removeZttPredicate(const_ztt_pred_iterator CPredI);

  /// \brief Replaces existing ztt predicate pointed to by CPredI, by NewPred.
  void replaceZttPredicate(const_ztt_pred_iterator CPredI, PredicateTy NewPred);

  /// \brief Returns the LHS/RHS operand DDRef of the predicate based on the
  /// IsLHS flag.
  RegDDRef *getZttPredicateOperandDDRef(const_ztt_pred_iterator CPredI,
                                        bool IsLHS) const;

  /// \brief Sets the LHS/RHS operand DDRef of the predicate based on the IsLHS
  /// flag.
  void setZttPredicateOperandDDRef(RegDDRef *Ref,
                                   const_ztt_pred_iterator CPredI, bool IsLHS);

  /// \brief Removes and returns the LHS/RHS operand DDRef of the predicate
  /// based on the IsLHS flag.
  RegDDRef *removeZttPredicateOperandDDRef(const_ztt_pred_iterator CPredI,
                                           bool IsLHS);

  /// \brief Returns true if this Ref belongs to ztt.
  bool isZttOperandDDRef(const RegDDRef *Ref) const;

  /// \brief Returns the DDRef associated with loop lower bound.
  /// The first DDRef is associated with lower bound.
  RegDDRef *getLowerDDRef();
  const RegDDRef *getLowerDDRef() const;
  /// \brief Sets the DDRef associated with loop lower bound.
  void setLowerDDRef(RegDDRef *Ref);
  /// \brief Removes and returns the DDRef associated with loop lower bound.
  RegDDRef *removeLowerDDRef();

  /// \brief Returns the DDRef associated with loop upper bound.
  /// The second DDRef is associated with upper bound.
  RegDDRef *getUpperDDRef();
  const RegDDRef *getUpperDDRef() const;

  /// \brief Sets the DDRef associated with loop upper bound.
  void setUpperDDRef(RegDDRef *Ref);

  /// \brief Removes and returns the DDRef associated with loop upper bound.
  RegDDRef *removeUpperDDRef();

  /// \brief Returns the DDRef associated with loop stride.
  /// The third DDRef is associated with stride.
  RegDDRef *getStrideDDRef();
  const RegDDRef *getStrideDDRef() const;

  /// \brief Sets the LLVM OrigLoop
  void setLLVMLoop(const Loop *OrigLoop);

  /// \brief Sets the DDRef associated with loop stride.
  void setStrideDDRef(RegDDRef *Ref);

  /// \brief Removes and returns the DDRef associated with loop stride.
  RegDDRef *removeStrideDDRef();

  /// \brief Returns the CanonExpr associated with loop lower bound.
  CanonExpr *getLowerCanonExpr();
  const CanonExpr *getLowerCanonExpr() const;

  /// \brief Returns the CanonExpr associated with loop upper bound.
  CanonExpr *getUpperCanonExpr();
  const CanonExpr *getUpperCanonExpr() const;

  /// \brief Returns the CanonExpr associated with loop stride.
  CanonExpr *getStrideCanonExpr();
  const CanonExpr *getStrideCanonExpr() const;

  /// \brief Returns the CanonExpr associated with loop trip count.
  /// Returns a newly allocated CanonExpr as this information is not
  /// directly stored so use with caution.
  /// This routine can return nullptr if the trip count cannot be computed.
  CanonExpr *getTripCountCanonExpr() const;

  /// \brief Returns Trip Count DDRef of this loop.
  /// Note, this will create a new DDRef in each call.
  /// NestingLevel argument indicates the level at which this DDRef will be
  /// attached to HIR. The default is: (loop nesting nevel - 1).
  /// This routine can return nullptr if the trip count cannot be computed.
  RegDDRef *getTripCountDDRef(unsigned NestingLevel = (MaxLoopNestLevel +
                                                       1)) const;

  /// \brief Returns true if this is a constant trip count loop and sets the
  /// trip count in TripCnt parameter only if the loop is constant trip loop.
  bool isConstTripLoop(int64_t *TripCnt = nullptr) const;

  /// \brief Returns true if this is a do loop.
  bool isDo() const {
    auto UpperDDRef = getUpperDDRef();
    assert(UpperDDRef && "UpperDDRef may not be null");
    return ((NumExits == 1) && !UpperDDRef->containsUndef());
  }

  /// \brief Returns true if this is a do multi-exit loop.
  bool isDoMultiExit() const {
    auto UpperDDRef = getUpperDDRef();
    assert(UpperDDRef && "UpperDDRef may not be null");
    return ((NumExits > 1) && !UpperDDRef->containsUndef());
  }

  /// \brief Returns true if this is an unknown loop.
  bool isUnknown() const {
    auto UpperDDRef = getUpperDDRef();
    assert(UpperDDRef && "UpperDDRef may not be null");
    return UpperDDRef->containsUndef();
  }

  /// \brief Returns true if loop is normalized.
  /// This method checks if LB = 0 and StrideRef = 1. UB can be a DDRef or
  // constant.
  bool isNormalized() const;

  /// \brief Returns the number of exits of the loop.
  unsigned getNumExits() const { return NumExits; }
  /// \brief Sets the number of exits of the loop.
  void setNumExits(unsigned NumEx);

  /// \brief Returns the nesting level of the loop.
  unsigned getNestingLevel() const { return NestingLevel; }
  /// \brief Returns true if this is the innermost loop in the loop nest.
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

  /// Preheader access methods

  /// \brief Returns the first preheader node if it exists, otherwise returns
  /// null.
  HLNode *getFirstPreheaderNode();
  const HLNode *getFirstPreheaderNode() const {
    return const_cast<HLLoop *>(this)->getFirstPreheaderNode();
  }
  /// \brief Returns the last preheader node if it exists, otherwise returns
  /// null.
  HLNode *getLastPreheaderNode();
  const HLNode *getLastPreheaderNode() const {
    return const_cast<HLLoop *>(this)->getLastPreheaderNode();
  }

  /// \brief Returns the number of preheader nodes.
  unsigned getNumPreheader() const {
    return std::distance(pre_begin(), pre_end());
  }
  /// \brief Returns true if preheader is not empty.
  bool hasPreheader() const { return (pre_begin() != pre_end()); }

  /// \brief Moves preheader nodes before the loop. Ztt is extracted first, if
  /// present.
  void extractPreheader();

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

  /// \brief Returns the first postexit node if it exists, otherwise returns
  /// null.
  HLNode *getFirstPostexitNode();
  const HLNode *getFirstPostexitNode() const {
    return const_cast<HLLoop *>(this)->getFirstPostexitNode();
  }
  /// \brief Returns the last postexit node if it exists, otherwise returns
  /// null.
  HLNode *getLastPostexitNode();
  const HLNode *getLastPostexitNode() const {
    return const_cast<HLLoop *>(this)->getLastPostexitNode();
  }

  /// \brief Returns the number of postexit nodes.
  unsigned getNumPostexit() const {
    return std::distance(post_begin(), post_end());
  }
  /// \brief Returns true if postexit is not empty.
  bool hasPostexit() const { return (post_begin() != post_end()); }

  /// \brief Moves postexit nodes after the loop. Ztt is extracted first, if
  /// present.
  void extractPostexit();

  /// \brief Moves preheader nodes before the loop and postexit nodes after the
  /// loop. Ztt is extracted first, if present.
  void extractPreheaderAndPostexit();

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

  /// \brief Returns the first child if it exists, otherwise returns null.
  HLNode *getFirstChild();
  const HLNode *getFirstChild() const {
    return const_cast<HLLoop *>(this)->getFirstChild();
  }
  /// \brief Returns the last child if it exists, otherwise returns null.
  HLNode *getLastChild();
  const HLNode *getLastChild() const {
    return const_cast<HLLoop *>(this)->getLastChild();
  }

  /// \brief Returns the number of children.
  unsigned getNumChildren() const {
    return std::distance(child_begin(), child_end());
  }
  /// \brief Returns true if it has children.
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
  /// This method will automatically update the goto branches with new labels
  /// inside the cloned loop.
  HLLoop *clone() const override;

  /// \brief - Clones the original loop without any of the children, preheader
  /// and postexit nodes. This routines copies all the original loop properties
  /// such as exits, ub, lb, etc. Data members that depend on where the cloned
  /// loop lives in HIR (like parent, nesting level) are not copied. They will
  /// be updated by HLNode insertion/removal utilities.
  HLLoop *cloneEmptyLoop() const;

  /// \brief - Clones the original loop without any of the children, preheader
  /// and postexit nodes. This routines copies all the original loop properties
  /// such as exits, ub, lb, strides, level. Data members that depend on where
  //  the cloned loop lives in HIR (like parent, etc) are not copied. They will
  /// be updated by HLNode insertion/removal utilities.
  HLLoop *cloneCompleteEmptyLoop() const;

  /// \brief Returns the number of operands associated with the loop ztt.
  unsigned getNumZttOperands() const;

  /// \brief Verifies HLLoop integrity.
  virtual void verify() const override;

  /// \brief Checks whether SIMD directive is attached to the loop.
  bool isSIMD() const;

  unsigned getMVTag() { return MVTag; }

  void setMVTag(unsigned Tag) { MVTag = Tag; }
};

} // End namespace loopopt

} // End namespace llvm

#endif
