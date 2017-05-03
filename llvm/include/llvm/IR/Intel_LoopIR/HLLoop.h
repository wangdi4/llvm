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

#include "llvm/ADT/SmallVector.h"

#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/Intel_LoopIR/HLDDNode.h"
#include "llvm/IR/Intel_LoopIR/HLIf.h"
#include "llvm/IR/Intel_LoopIR/RegDDRef.h"

namespace llvm {

class Loop;

namespace loopopt {

class CanonExpr;
class RegDDRef;

/// High level node representing a loop
class HLLoop final : public HLDDNode {
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

  typedef SmallVector<unsigned, 8> LiveInSetTy;
  typedef LiveInSetTy LiveOutSetTy;

  typedef LiveInSetTy::const_iterator const_live_in_iterator;
  typedef LiveOutSetTy::const_iterator const_live_out_iterator;

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

  // Set of temp symbases live into the loop.
  LiveInSetTy LiveInSet;
  // Set of temp symbases live out of the loop.
  LiveOutSetTy LiveOutSet;

  // Associated !llvm.loop metadata.
  MDNode *LoopMetadata;

  uint64_t MaxTripCountEstimate;

  // Bottom test debug location.
  DebugLoc CmpDbgLoc;

  // Back-edge branch debug location.
  DebugLoc BranchDbgLoc;

protected:
  HLLoop(HLNodeUtils &HNU, const Loop *LLVMLoop);
  HLLoop(HLNodeUtils &HNU, HLIf *ZttIf, RegDDRef *LowerDDRef,
         RegDDRef *UpperDDRef, RegDDRef *StrideDDRef, unsigned NumEx);

  /// Copy constructor used by cloning.
  /// CloneChildren parameter denotes if we want to clone
  /// children and preheader/postexit.
  HLLoop(const HLLoop &HLLoopObj);

  /// Move assignment operator used by HLNodeUtils::permuteLoopNests() to move
  /// loop properties from Lp to this loop.
  HLLoop &operator=(HLLoop &&Lp);

  friend class HLNodeUtils;
  friend class HIRParser;         // accesses ZTT
  friend class HIRTransformUtils; // For compile-time, allow permuteLoopNests()
                                  // to modify HLLoop internals.

  /// Initializes some of the members to bring the loop in a sane state.
  void initialize();

  /// Returns the Ztt of the loop.
  HLIf *getZtt() { return Ztt; }

  /// Sets the nesting level of the loop.
  void setNestingLevel(unsigned Level) { NestingLevel = Level; }

  /// Sets the Innermost flag to indicate if it is innermost loop.
  void setInnermost(bool IsInnermost) { this->IsInnermost = IsInnermost; }

  /// Returns the number of DDRefs associated with only the loop
  /// without the ztt.
  /// Comes down to DDRefs for lower, tripcount and stride.
  unsigned getNumLoopDDRefs() const { return 3; }

  /// Returns the number of operands this loop is supposed to have.
  unsigned getNumOperands() const override;

  /// Sets DDRefs' size to getNumLoopDDRefs().
  void resizeToNumLoopDDRefs();

  /// Used to implement get*CanonExpr() functionality.
  CanonExpr *getLoopCanonExpr(RegDDRef *Ref);
  const CanonExpr *getLoopCanonExpr(const RegDDRef *Ref) const;

  /// Implements getNumOperands() functionality.
  unsigned getNumOperandsInternal() const;

  /// Returns the DDRef offset of a ztt predicate.
  unsigned getZttPredicateOperandDDRefOffset(const_ztt_pred_iterator CPredI,
                                             bool IsLHS) const;

  /// Clone Implementation
  /// This function populates the GotoList with Goto branches within the
  /// cloned loop and LabelMap with Old and New Labels. Returns a cloned loop.
  HLLoop *cloneImpl(GotoContainerTy *GotoList, LabelMapTy *LabelMap,
                    HLNodeMapper *NodeMapper) const override;

  /// Used to print members of the loop which are otherwise hidden in
  /// pretty print like ztt, innermost flag etc.
  void printDetails(formatted_raw_ostream &OS, unsigned Depth,
                    bool Detailed) const;

  void addRemoveLoopMetadataImpl(ArrayRef<MDNode *> MDs, StringRef *RemoveID);

public:
  /// Prints preheader of loop.
  void printPreheader(formatted_raw_ostream &OS, unsigned Depth,
                      bool Detailed) const;

  /// Prints header of loop.
  void printHeader(formatted_raw_ostream &OS, unsigned Depth,
                   bool Detailed) const;

  /// Prints body of loop.
  void printBody(formatted_raw_ostream &OS, unsigned Depth,
                 bool Detailed) const;

  /// Prints footer of loop.
  void printFooter(formatted_raw_ostream &OS, unsigned Depth) const;

  /// Prints postexit of loop.
  void printPostexit(formatted_raw_ostream &OS, unsigned Depth,
                     bool Detailed) const;

  /// Prints HLLoop.
  virtual void print(formatted_raw_ostream &OS, unsigned Depth,
                     bool Detailed = false) const override;

  /// Returns underlying LLVM loop.
  const Loop *getLLVMLoop() const { return OrigLoop; }

  /// Returns the underlying type of the loop IV.
  Type *getIVType() const { return IVType; }
  /// Sets the underlying type of the loop IV.
  void setIVType(Type *Ty) { IVType = Ty; }

  /// Returns true if loop's IV is in signed range.
  bool isNSW() const { return IsNSW; }
  /// Sets/resets IsNSW flag.
  void setNSW(bool IsNSW) { this->IsNSW = IsNSW; }

  /// Returns true if ztt is present.
  bool hasZtt() const { return Ztt != nullptr; }

  /// sets the Ztt for HLLoop.
  void setZtt(HLIf *ZttIf);

  /// Removes and returns the Ztt for HLLoop if it exists, else returns
  /// nullptr.
  HLIf *removeZtt();

  /// Removes and returns the OrigLoop
  const Loop *removeLLVMLoop();

  /// Creates a Ztt for HLLoop. IsOverwrite flag
  /// indicates to overwrite existing Ztt or not.
  void createZtt(bool IsOverwrite = true, bool IsSigned = false);

  /// Creates a Ztt for HLLoop. IsOverwrite flag indicates to overwrite existing
  /// Ztt or not. The loop becomes an owner of incoming DDRefs.
  void createZtt(RegDDRef *LHS, PredicateTy Pred, RegDDRef *RHS,
                 bool IsOverwrite = true);

  /// Hoists the Ztt out of the loop. It returns a handle to the Ztt or
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

  /// Returns the number of predicates associated with this ZTT.
  unsigned getNumZttPredicates() const {
    assert(hasZtt() && "Ztt is absent!");
    return Ztt->getNumPredicates();
  }

  /// Adds new predicate in ZTT.
  void addZttPredicate(const HLPredicate &Pred, RegDDRef *Ref1, RegDDRef *Ref2);

  /// Removes the associated predicate and operand DDRefs(not destroyed).
  void removeZttPredicate(const_ztt_pred_iterator CPredI);

  /// Replaces existing ztt predicate pointed to by CPredI, by NewPred.
  void replaceZttPredicate(const_ztt_pred_iterator CPredI,
                           const HLPredicate &NewPred);

  /// Replaces PredicateTy in CPredI by NewPred.
  void replaceZttPredicate(const_ztt_pred_iterator CPredI, PredicateTy NewPred);

  /// Inverts PredicateTy in CPredI.
  void invertZttPredicate(const_ztt_pred_iterator CPredI);

  /// Returns the LHS/RHS operand DDRef of the predicate based on the
  /// IsLHS flag.
  RegDDRef *getZttPredicateOperandDDRef(const_ztt_pred_iterator CPredI,
                                        bool IsLHS) const;

  /// Sets the LHS/RHS operand DDRef of the predicate based on the IsLHS
  /// flag.
  void setZttPredicateOperandDDRef(RegDDRef *Ref,
                                   const_ztt_pred_iterator CPredI, bool IsLHS);

  /// Removes and returns the LHS/RHS operand DDRef of the predicate
  /// based on the IsLHS flag.
  RegDDRef *removeZttPredicateOperandDDRef(const_ztt_pred_iterator CPredI,
                                           bool IsLHS);

  /// Returns true if this Ref belongs to ztt.
  bool isZttOperandDDRef(const RegDDRef *Ref) const;

  /// Returns the DDRef associated with loop lower bound.
  /// The first DDRef is associated with lower bound.
  RegDDRef *getLowerDDRef() { return getOperandDDRefImpl(0); }
  const RegDDRef *getLowerDDRef() const {
    return const_cast<HLLoop *>(this)->getLowerDDRef();
  }

  /// Sets the DDRef associated with loop lower bound.
  void setLowerDDRef(RegDDRef *Ref) {
    assert((!Ref || Ref->isTerminalRef()) && "Invalid LowerDDRef!");
    setOperandDDRefImpl(Ref, 0);
  }

  /// Removes and returns the DDRef associated with loop lower bound.
  RegDDRef *removeLowerDDRef();

  /// Returns the DDRef associated with loop upper bound.
  /// The second DDRef is associated with upper bound.
  RegDDRef *getUpperDDRef() { return getOperandDDRefImpl(1); }
  const RegDDRef *getUpperDDRef() const {
    return const_cast<HLLoop *>(this)->getUpperDDRef();
  }

  /// Sets the DDRef associated with loop upper bound.
  void setUpperDDRef(RegDDRef *Ref) {
    assert((!Ref || Ref->isTerminalRef()) && "Invalid UpperDDRef!");
    setOperandDDRefImpl(Ref, 1);
  }

  /// Removes and returns the DDRef associated with loop upper bound.
  RegDDRef *removeUpperDDRef();

  /// Returns the DDRef associated with loop stride.
  /// The third DDRef is associated with stride.
  RegDDRef *getStrideDDRef() { return getOperandDDRefImpl(2); }
  const RegDDRef *getStrideDDRef() const {
    return const_cast<HLLoop *>(this)->getStrideDDRef();
  }

  /// Sets the underlying LLVM Loop.
  void setLLVMLoop(const Loop *LLVMLoop) { OrigLoop = LLVMLoop; }

  /// Sets the DDRef associated with loop stride.
  void setStrideDDRef(RegDDRef *Ref) {
    assert((!Ref || Ref->isTerminalRef()) && "Invalid StrideDDRef!");
    setOperandDDRefImpl(Ref, 2);
  }

  /// Removes and returns the DDRef associated with loop stride.
  RegDDRef *removeStrideDDRef();

  /// Returns the CanonExpr associated with loop lower bound.
  CanonExpr *getLowerCanonExpr();
  const CanonExpr *getLowerCanonExpr() const;

  /// Returns the CanonExpr associated with loop upper bound.
  CanonExpr *getUpperCanonExpr();
  const CanonExpr *getUpperCanonExpr() const;

  /// Returns the CanonExpr associated with loop stride.
  // TODO: Should we keep an uint64_t stride if it is always constant?
  CanonExpr *getStrideCanonExpr();
  const CanonExpr *getStrideCanonExpr() const;

  /// Returns the CanonExpr associated with loop trip count.
  /// Returns a newly allocated CanonExpr as this information is not
  /// directly stored so use with caution.
  /// This routine can return nullptr if the trip count cannot be computed.
  CanonExpr *getTripCountCanonExpr() const;

  /// Returns Trip Count DDRef of this loop.
  /// Note, this will create a new DDRef in each call.
  /// NestingLevel argument indicates the level at which this DDRef will be
  /// attached to HIR. The default is: (loop nesting nevel - 1).
  /// This routine can return nullptr if the trip count cannot be computed.
  RegDDRef *getTripCountDDRef(unsigned NestingLevel = (MaxLoopNestLevel +
                                                       1)) const;

  /// Returns true if this is a constant trip count loop and sets the
  /// trip count in TripCnt parameter only if the loop is constant trip loop.
  bool isConstTripLoop(uint64_t *TripCnt = nullptr,
                       bool AllowZeroTripCount = false) const;

  /// Returns true if this is an unknown loop.
  bool isUnknown() const {
    auto StrideRef = getStrideDDRef();
    assert(StrideRef && "Stride ref is null!");
    int64_t Val;

    // Stride is 0 for unknown loops.
    return (StrideRef->isIntConstant(&Val) && (Val == 0));
  }

  /// Returns true if this is a do loop.
  bool isDo() const { return ((NumExits == 1) && !isUnknown()); }

  /// Returns true if this is a do multi-exit loop.
  bool isDoMultiExit() const { return ((NumExits > 1) && !isUnknown()); }

  /// Returns true if loop is normalized.
  /// This method checks if LB = 0 and StrideRef = 1. UB can be a DDRef or
  // constant.
  bool isNormalized() const;

  /// Returns the number of exits of the loop.
  unsigned getNumExits() const { return NumExits; }
  /// Sets the number of exits of the loop.
  void setNumExits(unsigned NumEx);

  /// Returns the nesting level of the loop.
  unsigned getNestingLevel() const {
    assert(getParentRegion() && "getNestingLevel() invoked on detached loop!");
    return NestingLevel;
  }

  /// Returns true if this is the innermost loop in the loop nest.
  bool isInnermost() const {
    assert(getParentRegion() && "isInnermost() invoked on detached loop!");
    return IsInnermost;
  }

  /// Preheader iterator methods
  pre_iterator pre_begin() { return Children.begin(); }
  const_pre_iterator pre_begin() const { return Children.begin(); }
  pre_iterator pre_end() { return ChildBegin; }
  const_pre_iterator pre_end() const { return ChildBegin; }

  reverse_pre_iterator pre_rbegin() { return ChildBegin.getReverse(); }
  const_reverse_pre_iterator pre_rbegin() const {
    return ChildBegin.getReverse();
  }

  reverse_pre_iterator pre_rend() { return Children.rend(); }
  const_reverse_pre_iterator pre_rend() const { return Children.rend(); }

  /// Preheader access methods

  /// Returns the first preheader node if it exists, otherwise returns
  /// null.
  HLNode *getFirstPreheaderNode();
  const HLNode *getFirstPreheaderNode() const {
    return const_cast<HLLoop *>(this)->getFirstPreheaderNode();
  }
  /// Returns the last preheader node if it exists, otherwise returns
  /// null.
  HLNode *getLastPreheaderNode();
  const HLNode *getLastPreheaderNode() const {
    return const_cast<HLLoop *>(this)->getLastPreheaderNode();
  }

  /// Returns the number of preheader nodes.
  unsigned getNumPreheader() const {
    return std::distance(pre_begin(), pre_end());
  }
  /// Returns true if preheader is not empty.
  bool hasPreheader() const { return (pre_begin() != pre_end()); }

  /// Moves preheader nodes before the loop. Ztt is extracted first, if
  /// present.
  void extractPreheader();

  /// Removes loop preheader nodes.
  void removePreheader();

  /// Postexit iterator methods
  post_iterator post_begin() { return PostexitBegin; }
  const_post_iterator post_begin() const { return PostexitBegin; }
  post_iterator post_end() { return Children.end(); }
  const_post_iterator post_end() const { return Children.end(); }

  reverse_post_iterator post_rbegin() { return Children.rbegin(); }
  const_reverse_post_iterator post_rbegin() const { return Children.rbegin(); }
  reverse_post_iterator post_rend() { return PostexitBegin.getReverse(); }
  const_reverse_post_iterator post_rend() const {
    return PostexitBegin.getReverse();
  }

  /// Postexit acess methods

  /// Returns the first postexit node if it exists, otherwise returns
  /// null.
  HLNode *getFirstPostexitNode();
  const HLNode *getFirstPostexitNode() const {
    return const_cast<HLLoop *>(this)->getFirstPostexitNode();
  }
  /// Returns the last postexit node if it exists, otherwise returns
  /// null.
  HLNode *getLastPostexitNode();
  const HLNode *getLastPostexitNode() const {
    return const_cast<HLLoop *>(this)->getLastPostexitNode();
  }

  /// Returns the number of postexit nodes.
  unsigned getNumPostexit() const {
    return std::distance(post_begin(), post_end());
  }
  /// Returns true if postexit is not empty.
  bool hasPostexit() const { return (post_begin() != post_end()); }

  /// Moves postexit nodes after the loop. Ztt is extracted first, if
  /// present.
  void extractPostexit();

  /// Moves preheader nodes before the loop and postexit nodes after the
  /// loop. Ztt is extracted first, if present.
  void extractPreheaderAndPostexit();

  /// Removes loop postexit nodes.
  void removePostexit();

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

  /// Returns the first child if it exists, otherwise returns null.
  HLNode *getFirstChild();
  const HLNode *getFirstChild() const {
    return const_cast<HLLoop *>(this)->getFirstChild();
  }
  /// Returns the last child if it exists, otherwise returns null.
  HLNode *getLastChild();
  const HLNode *getLastChild() const {
    return const_cast<HLLoop *>(this)->getLastChild();
  }

  /// Returns the number of children.
  unsigned getNumChildren() const {
    return std::distance(child_begin(), child_end());
  }
  /// Returns true if it has children.
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

  /// Method for supporting type inquiry through isa, cast, and dyn_cast.
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
  HLLoop *clone(HLNodeMapper *NodeMapper = nullptr) const override;

  /// - Clones the original loop without any of the children, preheader
  /// and postexit nodes. This routines copies all the original loop properties
  /// such as exits, ub, lb, etc. Data members that depend on where the cloned
  /// loop lives in HIR (like parent, nesting level) are not copied. They will
  /// be updated by HLNode insertion/removal utilities.
  HLLoop *cloneEmptyLoop() const;

  /// Returns the number of operands associated with the loop ztt.
  unsigned getNumZttOperands() const;

  /// Verifies HLLoop integrity.
  virtual void verify() const override;

  /// Checks whether SIMD directive is attached to the loop.
  bool isSIMD() const;

  unsigned getMVTag() const { return MVTag; }

  void setMVTag(unsigned Tag) { MVTag = Tag; }

  /// return true if Triangular Loop
  bool isTriangularLoop() const;

  const_live_in_iterator live_in_begin() const { return LiveInSet.begin(); }
  const_live_in_iterator live_in_end() const { return LiveInSet.end(); }

  const_live_out_iterator live_out_begin() const { return LiveOutSet.begin(); }
  const_live_out_iterator live_out_end() const { return LiveOutSet.end(); }

  /// Returns true if loop has livein temps.
  bool hasLiveInTemps() const { return !LiveInSet.empty(); }

  /// Returns true if loop has liveout temps.
  bool hasLiveOutTemps() const { return !LiveOutSet.empty(); }

  /// Returns true if this symbase is live in to this loop.
  bool isLiveIn(unsigned Symbase) const {
    return std::binary_search(live_in_begin(), live_in_end(), Symbase);
  }

  /// Returns true if this symbase is live out of this loop.
  bool isLiveOut(unsigned Symbase) const {
    return std::binary_search(live_out_begin(), live_out_end(), Symbase);
  }

  /// Adds symbase as live into the loop.
  void addLiveInTemp(unsigned Symbase) {
    auto It = std::lower_bound(LiveInSet.begin(), LiveInSet.end(), Symbase);

    if ((It == LiveInSet.end()) || (*It != Symbase)) {
      LiveInSet.insert(It, Symbase);
    }
  }

  /// Adds symbase as live out of the loop.
  void addLiveOutTemp(unsigned Symbase) {
    auto It = std::lower_bound(LiveOutSet.begin(), LiveOutSet.end(), Symbase);

    if ((It == LiveOutSet.end()) || (*It != Symbase)) {
      LiveOutSet.insert(It, Symbase);
    }
  }

  /// Removes symbase from livein set.
  void removeLiveInTemp(unsigned Symbase) {
    auto It = std::lower_bound(LiveInSet.begin(), LiveInSet.end(), Symbase);

    if ((It != LiveInSet.end()) && (*It == Symbase)) {
      LiveInSet.erase(It);
    }
  }

  /// Removes symbase from liveout set.
  void removeLiveOutTemp(unsigned Symbase) {
    auto It = std::lower_bound(LiveOutSet.begin(), LiveOutSet.end(), Symbase);

    if ((It != LiveOutSet.end()) && (*It == Symbase)) {
      LiveOutSet.erase(It);
    }
  }

  void replaceLiveOutTemp(unsigned OldSymbase, unsigned NewSymbase) {
    assert(isLiveOut(OldSymbase) && "OldSymbase is not liveout!");
    removeLiveOutTemp(OldSymbase);
    addLiveOutTemp(NewSymbase);
  }

  /// Set or replace !llvm.loop metadata.
  void setLoopMetadata(MDNode *MD) { LoopMetadata = MD; }

  /// Returns !llvm.loop metadata associated with the Loop.
  MDNode *getLoopMetadata() const { return LoopMetadata; }

  /// Add a list of metadata \p MDs to loops !llvm.loop MDNode.
  ///
  /// The MDNode should have the format !{!"string-identifier", Args...}
  void addLoopMetadata(ArrayRef<MDNode *> MDs) {
    addRemoveLoopMetadataImpl(MDs, nullptr);
  }

  /// Remove !llvm.loop metadata that starts with \p ID.
  void removeLoopMetadata(StringRef ID) { addRemoveLoopMetadataImpl({}, &ID); }

  /// Clear all metadata from !llvm.loop MDNode.
  void clearLoopMetadata() { setLoopMetadata(nullptr); }

  uint64_t getMaxTripCountEstimate() const { return MaxTripCountEstimate; }

  void setMaxTripCountEstimate(uint64_t MaxTC) { MaxTripCountEstimate = MaxTC; }

  /// Marks loop to do not vectorize.
  void markDoNotVectorize();

  bool canNormalize() const;
  bool normalize();

  const DebugLoc &getCmpDebugLoc() const { return CmpDbgLoc; }
  void setCmpTestDebugLoc(const DebugLoc &Loc) { CmpDbgLoc = Loc; }

  const DebugLoc &getBranchDebugLoc() const { return BranchDbgLoc; }
  void setBranchDebugLoc(const DebugLoc &Loc) { BranchDbgLoc = Loc; }

  const DebugLoc getDebugLoc() const override { return getBranchDebugLoc(); }

  /// Returns the bottom test node for the loop. It is null for non-unknown
  /// loops.
  HLIf *getBottomTest();
  const HLIf *getBottomTest() const {
    return const_cast<HLLoop *>(this)->getBottomTest();
  }

  /// Returns the header label for the loop. It is null for non-unknown
  /// loops.
  HLLabel *getHeaderLabel();
  const HLLabel *getHeaderLabel() const {
    return const_cast<HLLoop *>(this)->getHeaderLabel();
  }
};

} // End namespace loopopt

} // End namespace llvm

#endif
