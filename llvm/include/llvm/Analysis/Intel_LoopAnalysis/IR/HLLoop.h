//===----------- HLLoop.h - High level IR loop node -------------*- C++ -*-===//
//
// Copyright (C) 2015-2021 Intel Corporation. All rights reserved.
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

#include "llvm/Analysis/Directives.h"
#include "llvm/Analysis/Intel_LoopAnalysis/IR/HLDDNode.h"
#include "llvm/Analysis/Intel_LoopAnalysis/IR/HLIf.h"
#include "llvm/Analysis/Intel_LoopAnalysis/IR/RegDDRef.h"
#include "llvm/IR/InstrTypes.h"

#include "llvm/Analysis/Intel_OptReport/OptReport.h"
#include "llvm/Analysis/Intel_OptReport/OptReportBuilder.h"
#include <functional>

namespace llvm {

class Loop;

namespace loopopt {

class CanonExpr;
class RegDDRef;
class HLLoopParallelTraits;
class HLInst;

typedef std::pair<int, RegDDRef *> LevelAndFactorPairTy;

struct BlockingPragmaInfo {
  SmallVector<LevelAndFactorPairTy, MaxLoopNestLevel> LevelsAndFactors;
  SmallVector<RegDDRef *, 4> Privates;
};

// Prefetching pragma info data structure:
// Var is null when omitted in the directive
// Hint is -1 when omitted in the directive
// Distance is -1 when omitted in the directive
// Distance of 0 indicates prefetching is disabled
struct PrefetchingPragmaInfo {
  const RegDDRef *Var;
  int Hint;
  int Dist;

  PrefetchingPragmaInfo(const RegDDRef *Var, int Hint, int Dist)
      : Var(Var), Hint(Hint), Dist(Dist) {}
};

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

  /// Scenarios that can be used to tag a given vectorized HLLoop. These
  /// tags are meant to be used as a debugging aid and should not be used
  /// to tune optimization passes.
  enum class VecTagTy { NONE, AUTOVEC, SIMD, PEEL, REMAINDER };

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
  bool HasSignedIV;

  // Tag to mark loop as multiversioned.
  unsigned MVTag = 0;

  // List of temp blobs that is proven to be delinearizable within the loop.
  // Note: changing the loopnest bounds may invalidate the list.
  SmallVector<unsigned, 0> MVDelinearizableBlobIndices;

  // Set of temp symbases live into the loop.
  LiveInSetTy LiveInSet;
  // Set of temp symbases live out of the loop.
  LiveOutSetTy LiveOutSet;

  // Loop distributed for Memory recurrence
  bool DistributedForMemRec;
  // Associated !llvm.loop metadata.
  MDNode *LoopMetadata;

  /// This loop's characteristics related to parallelism.
  std::unique_ptr<HLLoopParallelTraits> ParTraits;

  uint64_t LegalMaxTripCount;

  uint64_t MaxTripCountEstimate;
  // This flag is set for stripmined loops so DD can override the Upper Bound to
  // derive a more refined DV.
  bool MaxTCIsUsefulForDD;

  // Bottom test debug location.
  DebugLoc CmpDbgLoc;

  // Back-edge branch debug location.
  DebugLoc BranchDbgLoc;

  // Optimization report for the loop.
  OptReport OR;

  bool HasDistributePoint;

  bool IsUndoSinkingCandidate;

  bool IsBlocked;

  // Special field to force VF for a loop inside LoopOpt.
  unsigned ForcedVectorWidth;
  // Special field to force vector UF for a loop inside LoopOpt.
  unsigned ForcedVectorUnrollFactor;

  // Tag to mark loop as being auto/simd-vectorized. Default is none of the
  // scenarios.
  VecTagTy VecTag;

  // Contains info specified in blocking pragma.
  std::unique_ptr<BlockingPragmaInfo> BlockingInfo;

  // Contains info specified in prefetching pragma.
  SmallVector<PrefetchingPragmaInfo, 0> PrefetchingInfoVec;

protected:
  HLLoop(HLNodeUtils &HNU, const Loop *LLVMLoop);
  HLLoop(HLNodeUtils &HNU, HLIf *ZttIf, RegDDRef *LowerDDRef,
         RegDDRef *UpperDDRef, RegDDRef *StrideDDRef, unsigned NumEx);

  /// Copy constructor used by cloning.
  HLLoop(const HLLoop &HLLoopObj);

  /// Move assignment operator used by HLNodeUtils::permuteLoopNests() to move
  /// loop properties from Lp to this loop.
  HLLoop &operator=(HLLoop &&Lp);

  friend class HLNodeUtils;
  friend class HIRParser;         // accesses ZTT
  friend class HIRLoopFormation;  // set ZTT and IVType
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

  /// Sets DDRefs' size to getNumLoopDDRefs().
  void resizeToNumLoopDDRefs() {
    RegDDRefs.resize(getNumLoopDDRefs(), nullptr);
  }

  /// Implements getNumOperands() functionality.
  unsigned getNumOperandsInternal() const {
    return getNumLoopDDRefs() + getNumZttOperands();
  }

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

  void printDirectives(formatted_raw_ostream &OS, unsigned Depth) const;

  void addRemoveLoopMetadataImpl(ArrayRef<MDNode *> MDs, StringRef RemoveID,
                                 MDNode **ExternalLoopMetadata);

  /// Return true if the specified directive is attached to the loop.
  const HLInst *getDirective(int DirectiveID) const;

  /// Returns underlying LLVM loop.
  const Loop *getLLVMLoop() const { return OrigLoop; }

  /// Adds a pair of {level-factor} info specified in the blocking pragma for
  /// this loop.
  void addBlockingPragmaLevelAndFactor(int Level, RegDDRef *Factor) {
    if (!BlockingInfo) {
      BlockingInfo.reset(new BlockingPragmaInfo);
    }
    BlockingInfo->LevelsAndFactors.push_back(std::make_pair(Level, Factor));
  }

  /// Adds a new private specified in the blocking pragma for this loop.
  void addBlockingPragmaPrivate(RegDDRef *Private) {
    if (!BlockingInfo) {
      BlockingInfo.reset(new BlockingPragmaInfo);
    }
    BlockingInfo->Privates.push_back(Private);
  }

  /// Adds prefetch pragma info specified in the prefetching pragma vector for
  /// this loop.
  void addPrefetchingPragmaInfo(const RegDDRef *Var, int Hint, int Dist) {
    PrefetchingInfoVec.emplace_back(Var, Hint, Dist);
  }

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

  void dumpOptReport() const;

  /// Returns the underlying type of the loop IV.
  Type *getIVType() const { return IVType; }
  /// Sets the underlying type of the loop IV.
  void setIVType(Type *Ty) { IVType = Ty; }

  /// Returns true if loop's IV is in signed range.
  bool hasSignedIV() const { return HasSignedIV; }
  /// Sets/resets HasSignedIV flag.
  void setHasSignedIV(bool IsSigned) { HasSignedIV = IsSigned; }

  /// Returns true if ztt is present.
  bool hasZtt() const { return Ztt != nullptr; }

  /// sets the Ztt for HLLoop.
  void setZtt(HLIf *ZttIf);

  /// Removes and returns the Ztt for HLLoop if it exists, else returns
  /// nullptr.
  HLIf *removeZtt();

  /// Creates a Ztt for HLLoop. IsOverwrite flag
  /// indicates to overwrite existing Ztt or not.
  void createZtt(bool IsOverwrite = true, bool IsSigned = false);

  /// Creates a Ztt for HLLoop. IsOverwrite flag indicates to overwrite existing
  /// Ztt or not. The loop becomes an owner of incoming DDRefs.
  void createZtt(RegDDRef *LHS, PredicateTy Pred, RegDDRef *RHS,
                 bool IsOverwrite = true);

  /// Hoists the Ztt out of the loop. It returns a handle to the Ztt or
  /// nullptr if it doesn't exist.
  /// \p NewNestingLevel indicates the nesting level of the (possibly detached)
  /// loop. If it is ommited, nesting level is obtained from attachement level
  /// of the loop.
  HLIf *extractZtt(unsigned NewNestingLevel = NonLinearLevel);

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

  /// Returns the LHS operand DDRef of the predicate.
  RegDDRef *
  getLHSZttPredicateOperandDDRef(const_ztt_pred_iterator CPredI) const {
    return getZttPredicateOperandDDRef(CPredI, true);
  }

  /// Returns the RHS operand DDRef of the predicate.
  RegDDRef *
  getRHSZttPredicateOperandDDRef(const_ztt_pred_iterator CPredI) const {
    return getZttPredicateOperandDDRef(CPredI, false);
  }

  /// Sets the LHS operand DDRef of the predicate.
  void setLHSZttPredicateOperandDDRef(RegDDRef *Ref,
                                      const_ztt_pred_iterator CPredI) {
    setZttPredicateOperandDDRef(Ref, CPredI, true);
  }

  /// Sets the RHS operand DDRef of the predicate.
  void setRHSZttPredicateOperandDDRef(RegDDRef *Ref,
                                      const_ztt_pred_iterator CPredI) {
    setZttPredicateOperandDDRef(Ref, CPredI, false);
  }

  /// Removes and returns the LHS operand DDRef of the predicate.
  RegDDRef *removeLHSZttPredicateOperandDDRef(const_ztt_pred_iterator CPredI) {
    return removeZttPredicateOperandDDRef(CPredI, true);
  }

  /// Removes and returns the RHS operand DDRef of the predicate.
  RegDDRef *removeRHSZttPredicateOperandDDRef(const_ztt_pred_iterator CPredI) {
    return removeZttPredicateOperandDDRef(CPredI, false);
  }

  /// Returns true if this Ref belongs to ztt.
  bool isZttOperandDDRef(const RegDDRef *Ref) const;

  /// Returns the number of operands this loop is supposed to have.
  unsigned getNumOperands() const override { return getNumOperandsInternal(); }

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
  /// Asserts on unknown loop unless \p AllowUnknownLoop is set to true.
  RegDDRef *getUpperDDRef(bool AllowUnknownLoop = false) {
    assert((AllowUnknownLoop || !isUnknown()) &&
           "Invalid access to unknown loop's upper bound!");
    return getOperandDDRefImpl(1);
  }

  const RegDDRef *getUpperDDRef(bool AllowUnknownLoop = false) const {
    return const_cast<HLLoop *>(this)->getUpperDDRef(AllowUnknownLoop);
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
  CanonExpr *getLowerCanonExpr() {
    return getLowerDDRef()->getSingleCanonExpr();
  }
  const CanonExpr *getLowerCanonExpr() const {
    return getLowerDDRef()->getSingleCanonExpr();
  }

  /// Returns the CanonExpr associated with loop upper bound.
  CanonExpr *getUpperCanonExpr() {
    return getUpperDDRef()->getSingleCanonExpr();
  }
  const CanonExpr *getUpperCanonExpr() const {
    return getUpperDDRef()->getSingleCanonExpr();
  }

  /// Returns the CanonExpr associated with loop stride.
  // TODO: Should we keep an uint64_t stride if it is always constant?
  CanonExpr *getStrideCanonExpr() {
    return getStrideDDRef()->getSingleCanonExpr();
  }
  const CanonExpr *getStrideCanonExpr() const {
    return getStrideDDRef()->getSingleCanonExpr();
  }

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

  // Returns true if loop has ZTT and it may be proven to always evaluate to the
  // same result, that is stored to \p IsTrue argument.
  // Note, this will return false if there is no ZTT.
  bool isKnownZttPredicate(bool *IsTrue = nullptr) const;

  /// Returns true if this is a constant trip count loop and sets the
  /// trip count in TripCnt parameter only if the loop is constant trip loop.
  bool isConstTripLoop(uint64_t *TripCnt = nullptr) const;

  /// Returns true if this is an unknown loop.
  bool isUnknown() const {
    auto StrideRef = getStrideDDRef();
    int64_t Val;

    // Stride is 0 for unknown loops.
    return (!StrideRef || (StrideRef->isIntConstant(&Val) && (Val == 0)));
  }

  /// Returns true if this is a do loop.
  bool isDo() const { return (!isMultiExit() && !isUnknown()); }

  /// Returns true if this is a do multi-exit loop.
  bool isDoMultiExit() const { return (isMultiExit() && !isUnknown()); }

  /// Returns true if this is a multi-exit loop.
  bool isMultiExit() const { return NumExits > 1; }

  /// Returns true if loop is normalized.
  /// This method checks if LB = 0 and StrideRef = 1. UB can be a DDRef or
  // constant.
  bool isNormalized() const;

  /// Returns the number of exits of the loop.
  unsigned getNumExits() const { return NumExits; }
  /// Sets the number of exits of the loop.
  void setNumExits(unsigned NumEx) {
    assert(NumEx && "Number of exits cannot be zero!");
    NumExits = NumEx;
  }

  /// Returns the nesting level of the loop.
  unsigned getNestingLevel() const {
    assert(getParentRegion() && "getNestingLevel() invoked on detached loop!");
    return NestingLevel;
  }

  /// Returns loop depth of the underlying LLVM loop if it exists, otherwise
  /// returns 0. This can be useful for cost-modelling.
  unsigned getLLVMLoopDepth() const {
    return OrigLoop ? OrigLoop->getLoopDepth() : 0;
  }

  /// Returns true if this is the innermost loop in the loop nest.
  bool isInnermost() const { return IsInnermost; }

  /// Preheader iterator methods
  pre_iterator pre_begin() { return Children.begin(); }
  const_pre_iterator pre_begin() const { return Children.begin(); }
  pre_iterator pre_end() { return ChildBegin; }
  const_pre_iterator pre_end() const { return ChildBegin; }
  auto preheaderNodes() { return make_range(pre_begin(), pre_end());}
  auto preheaderNodes() const { return make_range(pre_begin(), pre_end()); }

  reverse_pre_iterator pre_rbegin() { return ++ChildBegin.getReverse(); }
  const_reverse_pre_iterator pre_rbegin() const {
    return const_cast<HLLoop *>(this)->pre_rbegin();
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
  auto postExitNodes() { return make_range(post_begin(), post_end());}
  auto postExitNodes() const { return make_range(post_begin(), post_end());}

  reverse_post_iterator post_rbegin() { return Children.rbegin(); }
  const_reverse_post_iterator post_rbegin() const { return Children.rbegin(); }
  reverse_post_iterator post_rend() { return ++PostexitBegin.getReverse(); }
  const_reverse_post_iterator post_rend() const {
    return const_cast<HLLoop *>(this)->post_rend();
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
  void extractPreheaderAndPostexit() {
    extractPreheader();
    extractPostexit();
  }

  /// Extracts any ZTT, preheader nodes, and postexit nodes that are present in
  /// this loop.
  ///
  /// This is similar to \ref extractPreheaderAndPostexit, but extracts the
  /// loop's ZTT even if no preheader or postexit nodes are present.
  void extractZttPreheaderAndPostexit() {
    extractZtt();
    extractPreheaderAndPostexit();
  }

  /// Removes loop postexit nodes.
  void removePostexit();

  /// Replaces the loop with its body and IVs with the lower bound.
  void replaceByFirstIteration(bool ExtractPostexit = true);

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
    return Node->getHLNodeClassID() == HLNode::HLLoopVal;
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
  HLLoop *cloneEmpty() const;

  /// Returns the number of operands associated with the loop ztt.
  unsigned getNumZttOperands() const;

  /// Verifies HLLoop integrity.
  virtual void verify() const override;

  /// Checks whether SIMD directive is attached to the loop.
  bool isSIMD() const { return getDirective(DIR_OMP_SIMD); }

  /// Return SIMD Entry intrinsic
  const HLInst* getSIMDEntryIntrinsic() const {
    return getDirective(DIR_OMP_SIMD);
  }

  /// Checks whether SIMD directive is attached to the loop or its parents.
  bool isInSIMDRegion() const {
    const HLLoop *OuterLp = this;
    while (OuterLp) {
      if (OuterLp->isSIMD())
        return true;
      OuterLp = OuterLp->getParentLoop();
    }
    return false;
  }

  unsigned getMVTag() const { return MVTag; }

  SmallVectorImpl<unsigned> &getMVDelinearizableBlobIndices() {
    return MVDelinearizableBlobIndices;
  }

  const SmallVectorImpl<unsigned> &getMVDelinearizableBlobIndices() const {
    return MVDelinearizableBlobIndices;
  }

  bool isDistributedForMemRec() const { return DistributedForMemRec; }

  void setMVTag(unsigned Tag) { MVTag = Tag; }

  void setDistributedForMemRec() { DistributedForMemRec = true; }

  /// return true if Triangular Loop
  bool isTriangularLoop() const;

  const_live_in_iterator live_in_begin() const { return LiveInSet.begin(); }
  const_live_in_iterator live_in_end() const { return LiveInSet.end(); }

  const_live_out_iterator live_out_begin() const { return LiveOutSet.begin(); }
  const_live_out_iterator live_out_end() const { return LiveOutSet.end(); }

  /// Returns true if loop has livein temps.
  bool hasLiveInTemps() const { return !LiveInSet.empty(); }

  /// Returns number of livein temps.
  unsigned getNumLiveInTemps() const { return LiveInSet.size(); }

  /// Returns true if loop has liveout temps.
  bool hasLiveOutTemps() const { return !LiveOutSet.empty(); }

  /// Returns number of liveout temps.
  unsigned getNumLiveOutTemps() const { return LiveOutSet.size(); }

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

  void addLiveInTemp(ArrayRef<unsigned> Symbases) {
    for (auto Symbase : Symbases)
      addLiveInTemp(Symbase);
  }

  /// Adds all symbases attached at Ref as live into the loop
  void addLiveInTemp(const RegDDRef *Ref) {
    if (Ref->isSelfBlob()) {
      addLiveInTemp(Ref->getSymbase());
    }

    for (auto DRef : make_range(Ref->blob_begin(), Ref->blob_end())) {
      addLiveInTemp(DRef->getSymbase());
    }
  }

  /// Adds all symbases attached at Ref as live out to the loop
  void addLiveOutTemp(const RegDDRef *Ref) {
    if (Ref->isSelfBlob()) {
      addLiveOutTemp(Ref->getSymbase());
    }

    for (auto DRef : make_range(Ref->blob_begin(), Ref->blob_end())) {
      addLiveOutTemp(DRef->getSymbase());
    }
  }

  // TODO: const
  void addLiveInTemp(ArrayRef<RegDDRef *> Refs) {
    for (auto Ref : Refs)
      addLiveInTemp(Ref);
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

  void clearLiveInTemp() { LiveInSet.clear(); }

  void clearLiveOutTemp() { LiveOutSet.clear(); }

  void replaceLiveInTemp(unsigned OldSymbase, unsigned NewSymbase) {
    assert(isLiveIn(OldSymbase) && "OldSymbase is not liveout!");
    removeLiveInTemp(OldSymbase);
    addLiveInTemp(NewSymbase);
  }

  void replaceLiveOutTemp(unsigned OldSymbase, unsigned NewSymbase) {
    assert(isLiveOut(OldSymbase) && "OldSymbase is not liveout!");
    removeLiveOutTemp(OldSymbase);
    addLiveOutTemp(NewSymbase);
  }

  /// Set or replace !llvm.loop metadata.
  void setLoopMetadata(MDNode *MD) {
    assert(MD && "MD is null, use clearLoopMetadata() instead!");
    LoopMetadata = MD;
  }

  /// Returns !llvm.loop metadata associated with the Loop.
  MDNode *getLoopMetadata() const { return LoopMetadata; }

  /// Clear all metadata from !llvm.loop MDNode.
  void clearLoopMetadata() { LoopMetadata = nullptr; }

  /// Add a list of metadata \p MDs to loops !llvm.loop MDNode.
  ///
  /// The MDNode should have the format !{!"string-identifier", Args...}
  /// Function operates on \p ExternalLoopMetadata, if provided.
  void addLoopMetadata(ArrayRef<MDNode *> MDs,
                       MDNode **ExternalLoopMetadata = nullptr) {
    addRemoveLoopMetadataImpl(MDs, "", ExternalLoopMetadata);
  }

  /// Function adds !{!ID, Value} metadata to the !llvm.loop MDNode.
  void addInt32LoopMetadata(StringRef ID, unsigned Value);

  /// Remove !llvm.loop metadata that starts with \p ID.
  /// Function operates on \p ExternalLoopMetadata, if provided.
  void removeLoopMetadata(StringRef ID,
                          MDNode **ExternalLoopMetadata = nullptr) {
    addRemoveLoopMetadataImpl({}, ID, ExternalLoopMetadata);
  }

  /// Returns loop metadata corresponding to \p Name. Returns null if not found.
  MDNode *getLoopStringMetadata(StringRef Name) const;

  /// Documentation for clang loop pragmas-
  /// http://clang.llvm.org/docs/LanguageExtensions.html#extensions-for-loop-hint-optimizations

  /// Returns true if loop has pragma to enable complete or general unrolling.
  bool hasUnrollEnablingPragma() const {
    return hasCompleteUnrollEnablingPragma() ||
           hasGeneralUnrollEnablingPragma();
  }

  /// Returns true if loop has pragma to disable complete or general unrolling.
  bool hasUnrollDisablingPragma() const {
    return hasCompleteUnrollDisablingPragma() ||
           hasGeneralUnrollDisablingPragma();
  }

  /// Returns true if loop has pragma to enable complete unrolling.
  bool hasCompleteUnrollEnablingPragma() const;

  /// Returns true if loop has pragma to disable complete unrolling.
  bool hasCompleteUnrollDisablingPragma() const;

  /// Returns true if loop has pragma to enable general unrolling.
  bool hasGeneralUnrollEnablingPragma() const {
    if (getLoopStringMetadata("llvm.loop.unroll.enable")) {
      return true;
    }

    unsigned PragmaCount = getUnrollPragmaCount();

    return (PragmaCount > 1);
  }

  /// Returns true if loop has pragma to disable general unrolling.
  bool hasGeneralUnrollDisablingPragma() const;

  /// Returns unroll count specified through pragma, otherwise returns 0.
  unsigned getUnrollPragmaCount() const {
    auto *MD = getLoopStringMetadata("llvm.loop.unroll.count");

    if (!MD) {
      return 0;
    }

    return mdconst::extract<ConstantInt>(MD->getOperand(1))->getZExtValue();
  }

  unsigned getInterleavePragmaCount() const {
    auto *MD = getLoopStringMetadata("llvm.loop.interleave.count");

    if (!MD) {
      return 0;
    }

    return mdconst::extract<ConstantInt>(MD->getOperand(1))->getZExtValue();
  }

  /// Returns true if loop has pragma to enable unroll & jam.
  bool hasUnrollAndJamEnablingPragma() const {
    if (getLoopStringMetadata("llvm.loop.unroll_and_jam.enable")) {
      return true;
    }

    unsigned PragmaCount = getUnrollAndJamPragmaCount();

    return (PragmaCount > 1);
  }

  /// Returns true if loop has pragma to disable unroll & jam.
  bool hasUnrollAndJamDisablingPragma() const {
    if (getLoopStringMetadata("llvm.loop.unroll_and_jam.disable")) {
      return true;
    }

    unsigned PragmaCount = getUnrollAndJamPragmaCount();

    return (PragmaCount == 1);
  }

  /// Returns unroll & jam count specified through pragma, otherwise returns 0.
  unsigned getUnrollAndJamPragmaCount() const {
    auto *MD = getLoopStringMetadata("llvm.loop.unroll_and_jam.count");

    if (!MD) {
      return 0;
    }

    return mdconst::extract<ConstantInt>(MD->getOperand(1))->getZExtValue();
  }

  /// Returns true if loop has pragma to enable distribution.
  bool hasDistributionEnablingPragma() const {
    auto *MD = getLoopStringMetadata("llvm.loop.distribute.enable");

    // first operand is i1 type.
    return MD && mdconst::extract<ConstantInt>(MD->getOperand(1))->isOne();
  }

  /// Returns true if loop has pragma to disable distribution.
  bool hasDistributionDisablingPragma() const {
    auto *MD = getLoopStringMetadata("llvm.loop.distribute.enable");

    // first operand is i1 type.
    return MD && mdconst::extract<ConstantInt>(MD->getOperand(1))->isZero();
  }

  /// Returns true if loop has pragma to enable vectorization.
  bool hasVectorizeEnablingPragma() const;

  /// Returns true if loop has pragma to disable vectorization.
  bool hasVectorizeDisablingPragma() const;

  /// Returns true if loop has ivdep loop pragma.
  bool hasVectorizeIVDepLoopPragma() const {
    return getLoopStringMetadata("llvm.loop.vectorize.ivdep_loop");
  }
  /// Returns true if loop has ivdep back pragma
  bool hasVectorizeIVDepBackPragma() const {
    return getLoopStringMetadata("llvm.loop.vectorize.ivdep_back");
  }

  /// Returns true if loop has ivdep back/loop pragma or assumeIVDEP option
  bool hasVectorizeIVDepPragma() const;

  /// Returns true if loop has vectorize always pragma.
  bool hasVectorizeAlwaysPragma() const {
    if (getLoopStringMetadata("llvm.loop.vectorize.ignore_profitability")) {
      assert(hasVectorizeEnablingPragma() &&
             "llvm.loop.vectorize.ignore_profitability metadata found without "
             "llvm.loop.vectorize.enable metadata!");
      return true;
    }

    return false;
  }

  /// Returns the vector width specified through pragma, otherwise returns 0.
  unsigned getVectorizePragmaWidth() const {
    auto *MD = getLoopStringMetadata("llvm.loop.vectorize.width");

    if (!MD) {
      return 0;
    }

    return mdconst::extract<ConstantInt>(MD->getOperand(1))->getZExtValue();
  }

  /// Returns the VF for this loop forced inside LoopOpt.
  unsigned getForcedVectorWidth() const { return ForcedVectorWidth; }
  /// Sets the forced VF that should be used during vectorization.
  void setForcedVectorWidth(unsigned VF) { ForcedVectorWidth = VF; }
  /// Returns the vector UF for this loop forced inside LoopOpt.
  unsigned getForcedVectorUnrollFactor() const {
    return ForcedVectorUnrollFactor;
  }
  /// Sets the forced vector UF that should be used during vectorization.
  void setForcedVectorUnrollFactor(unsigned UF) {
    ForcedVectorUnrollFactor = UF;
  }

  /// Returns the vectorization tag attached to this loop.
  VecTagTy getVecTag() const { return VecTag; }
  /// Tags the loop with given vectorization scenario.
  void setVecTag(VecTagTy VT) { VecTag = VT; }

  /// Returns true if minimum trip count of loop is specified using pragma and
  /// returns the value in \p MinTripCount.
  bool getPragmaBasedMinimumTripCount(unsigned &MinTripCount) const {
    auto *MD = getLoopStringMetadata("llvm.loop.intel.loopcount_minimum");

    if (!MD) {
      return false;
    }

    MinTripCount =
        mdconst::extract<ConstantInt>(MD->getOperand(1))->getZExtValue();
    return true;
  }

  /// Sets the pragma based minimum trip count of the loop to \p MinTripCount.
  void setPragmaBasedMinimumTripCount(unsigned MinTripCount);

  /// Removes pragma based minimum trip count of the loop.
  void removePragmaBasedMinimumTripCount() {
    removeLoopMetadata("llvm.loop.intel.loopcount_minimum");
  }

  /// Returns true if maximum trip count of loop is specified using pragma and
  /// returns the value in \p MaxTripCount.
  bool getPragmaBasedMaximumTripCount(unsigned &MaxTripCount) const {
    auto *MD = getLoopStringMetadata("llvm.loop.intel.loopcount_maximum");

    if (!MD) {
      return false;
    }

    MaxTripCount =
        mdconst::extract<ConstantInt>(MD->getOperand(1))->getZExtValue();
    return true;
  }

  /// Returns true if legal maximum trip count of loop is specified using pragma
  /// and returns the value in \p MaxTripCount.
  bool getPragmaBasedLegalMaxTripCount(unsigned &LegalMaxTripCount) const {
    auto *MD = getLoopStringMetadata("llvm.loop.intel.max.trip_count");

    if (!MD) {
      return false;
    }

    LegalMaxTripCount =
        mdconst::extract<ConstantInt>(MD->getOperand(1))->getZExtValue();
    return true;
  }

  /// Sets the pragma based maximum trip count of the loop to \p MaxTripCount.
  void setPragmaBasedMaximumTripCount(unsigned MaxTripCount);

  /// Removes pragma based minimum trip count of the loop.
  void removePragmaBasedMaximumTripCount() {
    removeLoopMetadata("llvm.loop.intel.loopcount_maximum");
  }

  /// Returns true if average trip count of loop is specified using pragma and
  /// returns the value in \p AvgTripCount.
  bool getPragmaBasedAverageTripCount(unsigned &AvgTripCount) const {
    auto *MD = getLoopStringMetadata("llvm.loop.intel.loopcount_average");

    if (!MD) {
      return false;
    }

    AvgTripCount =
        mdconst::extract<ConstantInt>(MD->getOperand(1))->getZExtValue();
    return true;
  }

  /// Sets the pragma based average trip count of the loop to \p AvgTripCount.
  void setPragmaBasedAverageTripCount(unsigned AvgTripCount);

  /// Removes pragma based minimum trip count of the loop.
  void removePragmaBasedAverageTripCount() {
    removeLoopMetadata("llvm.loop.intel.loopcount_average");
  }

  /// Divides any existing pragma based min/max/avg trip count by \p Factor.
  void dividePragmaBasedTripCount(unsigned Factor);

  /// Returns true if likely trip counts of loop are specified using pragma and
  /// returns the values in \p TripCounts.
  bool
  getPragmaBasedLikelyTripCounts(SmallVectorImpl<unsigned> &TripCounts) const {
    auto *MD = getLoopStringMetadata("llvm.loop.intel.loopcount");

    if (!MD) {
      return false;
    }

    for (unsigned I = 1, E = MD->getNumOperands(); I < E; ++I) {
      TripCounts.push_back(
          mdconst::extract<ConstantInt>(MD->getOperand(I))->getZExtValue());
    }

    return true;
  }

  /// Returns true if loop has pragma to enable fusion.
  bool hasFusionEnablingPragma() const;

  /// Returns true if loop has pragma to disable fusion.
  bool hasFusionDisablingPragma() const;

  // This info can be used for legality checks as opposed to
  // MaxTripCountEstimate which can only be used for profitability checks. 0
  // means no info.
  uint64_t getLegalMaxTripCount() const { return LegalMaxTripCount; }

  void setLegalMaxTripCount(uint64_t LegalMaxTC) {
    LegalMaxTripCount = LegalMaxTC;
  }

  // 0 means no estimate.
  uint64_t getMaxTripCountEstimate() const { return MaxTripCountEstimate; }

  // \p IsUsefulForDD indicates that the max trip count estimate can be used for
  // DD analysis.
  void setMaxTripCountEstimate(uint64_t MaxTC, bool IsUsefulForDD = false) {
    MaxTripCountEstimate = MaxTC;
    MaxTCIsUsefulForDD = IsUsefulForDD;
  }

  // Returns true if max trip count estimate can be used for DD analysis.
  bool isMaxTripCountEstimateUsefulForDD() const { return MaxTCIsUsefulForDD; }

  void setMaxTripCountEstimateUsefulForDD(bool Flag) {
    MaxTCIsUsefulForDD = Flag;
  }

  /// Marks loop to do not vectorize.
  void markDoNotVectorize();

  /// Marks loop to do not unroll.
  void markDoNotUnroll();

  // Add unroll disabling metadata to underlying LLVM loop.
  void markLLVMLoopDoNotUnroll();

  /// Marks loop to do not unroll & jam.
  void markDoNotUnrollAndJam();

  /// Marks loop not to block.
  void markDoNotBlock();

  /// Caller can supply loop lower bound in \p LowerCE to be used for
  /// normalization instead of the one in the Loop. \p AllowExplicitBoundInst
  /// allows the normalization utility to create an explicit instruction
  /// representing the loop lower bound in the preheader, if necessary.
  /// Normalization always succeeeds if this flag is true.
  bool canNormalize(const CanonExpr *LowerCE = nullptr,
                    bool AllowExplicitBoundInst = false) const;

  /// \p AllowExplicitBoundInst allows the utility to create an explicit
  /// instruction representing the loop lower bound in the preheader, if
  /// necessary. Normalization always succeeeds if this flag is true.
  bool normalize(bool AllowExplicitBoundInst = false);

  /// Return false if loop cannot be stripmined - some stripmined
  /// loop cannot be normalized. When \p AllowExplicitBoundInst
  /// is set, normalization will always succeed.
  bool canStripmine(unsigned StripmineSize,
                    bool AllowExplicitBoundInst = false) const;

  /// Stripmine is not required for trip counts <= \p StripmineSize.
  bool isStripmineRequired(unsigned StripmineSize) const;

  const DebugLoc &getCmpDebugLoc() const { return CmpDbgLoc; }
  void setCmpTestDebugLoc(const DebugLoc &Loc) { CmpDbgLoc = Loc; }

  const DebugLoc &getBranchDebugLoc() const { return BranchDbgLoc; }
  void setBranchDebugLoc(const DebugLoc &Loc) { BranchDbgLoc = Loc; }

  const DebugLoc getDebugLoc() const override { return getBranchDebugLoc(); }

  OptReport getOptReport() const { return OR; }
  void setOptReport(OptReport R) { OR = R; }
  void eraseOptReport() { OR = nullptr; }

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

  bool hasDistributePoint() const { return HasDistributePoint; }
  void setHasDistributePoint(bool Flag) { HasDistributePoint = Flag; }

  /// The flag is set by SinkingForPerfectLoopnest pass to indicate to later
  /// UndoSinking pass that sinking was performed and needs to be undone.
  /// The flag is reset by intermediate interchange or blocking passes if they
  /// trigger on the loopnest. If loop interchange or loop blocking is not triggered
  /// with sinked insts, we need to set the loop as an undo sinking candidate
  /// and undo sinking later.
  bool isUndoSinkingCandidate() const { return IsUndoSinkingCandidate; }
  void setIsUndoSinkingCandidate(bool Flag) { IsUndoSinkingCandidate = Flag; }

  bool isBlocked() const { return IsBlocked; }
  void setIsBlocked(bool Flag) { IsBlocked = Flag; }

  /// Shifts by \p Amount all the RegDDRefs in the body of this loop.
  void shiftLoopBodyRegDDRefs(int64_t Amount);

  /// Returns true if the first iteration of the loop can be peeled.
  bool canPeelFirstIteration() const;

  /// Peels the first iteration of the loop and inserts the peel loop before
  /// this loop. Ztt, loop preheader and postexit are extracted before cloning
  /// this loop to generate the peel loop. If \p UpdateMainLoop is true, this
  /// loop's UB and DDRefs are updated so that peeled iterations are not
  /// executed again. Otherwise, this loop's UB and DDRefs won't be updated and
  /// the loop will redundantly execute the iterations executed by the peel
  /// loop. This method requires that the loop is in normalized form.
  //
  /// NOTE: Peeling can fail for unknown loops in which case it returns nullptr
  /// and leaves the original loop unchanged.
  HLLoop *peelFirstIteration(bool UpdateMainLoop = true);

  // Initializes unconditional liveout only (not livein) temps with undef before
  // the loop like this-
  //
  // t = undef << undef initialization
  // DO
  //   t = A[i];
  // END DO
  //    = t  << use of t in postexit
  //
  // This is needed because when we create peel/main/remainder loop setup with
  // ZTTs for loops as CFG analysis alone is not able to tell that at least one
  // of the loops will definitely execute and hence dominates the temp uses.
  // This results in the creation of extra phis for these temps in the outer
  // loop headers. This affects privatization analysis of outer SIMD loops which
  // are handled after loopopt but it can cause other issues like increasing
  // register pressure.
  void undefInitializeUnconditionalLiveoutTemps();

  /// Peels the current loop to align memory accesses to the memref \p
  /// PeelArrayRef. Alignment is determined based on \p VF being used for the
  /// main vector loop, and this in-turn determines the number of iterations to
  /// peel at runtime. For a given loop, this utility transforms it to -
  // <instructions to compute alignment & peel iterations>
  // if (%peel_iters ! = 0) {
  //   + DO i1 = 0, %peel_iters + -1, 1
  //   +    <scalar code>
  //   + END LOOP
  // }
  //
  // + DO i1 = 0, %UB - %peel_iters + -1, 1
  // +    <normalized scalar code>
  // + END LOOP
  HLLoop *generatePeelLoop(const RegDDRef *PeelArrayRef, unsigned VF);

  // Collects all HLGotos which exit the loop.
  void populateEarlyExits(SmallVectorImpl<HLGoto *> &Gotos);

  HLLoopParallelTraits *getParallelTraits() const { return ParTraits.get(); }
  void setParallelTraits(HLLoopParallelTraits *CT) {
    assert(ParTraits == nullptr && "parallel traits already set");
    ParTraits.reset(CT);
  }

  bool isMVFallBack() const {
    unsigned MVTag = getMVTag();
    return (MVTag && (MVTag != getNumber()));
  }

  /// Returns all (level-factor) pairs specified in the blocking pragma for this
  /// loop. Returns empty ArrayRef if no info exists.
  ArrayRef<LevelAndFactorPairTy> getBlockingPragmaLevelAndFactors() const {
    if (BlockingInfo) {
      return BlockingInfo->LevelsAndFactors;
    }
    return {};
  }

  /// Returns all privates specified in the blocking pragma for this loop.
  /// Returns empty ArrayRef if no info exists.
  ArrayRef<RegDDRef *> getBlockingPragmaPrivates() const {
    if (BlockingInfo) {
      return BlockingInfo->Privates;
    }
    return {};
  }

  void promoteNestingLevel(unsigned StartLevel);

  /// Returns the prefetching pragma vector
  ArrayRef<PrefetchingPragmaInfo> getPrefetchingPragmaInfo() const {
    return PrefetchingInfoVec;
  }

  /// Clears the prefetching pragma vector
  void clearPrefetchingPragmaInfo() { PrefetchingInfoVec.clear(); }

  /// Some countable loops can be written in a way that their trip count is
  /// equal to type's range size which is 1 more than unsigned max value. There
  /// was an assumption in loopopt framework that this does not happen. However,
  /// HIR CG is able to generate the right code for the loops. Some
  /// transformations like general unroll and vectorization may require special
  /// code generation to handle this case.
  ///
  /// As an example, following loop has a tip count equal to 2 ^ 32 if n is
  /// equal to 5-
  ///
  /// unsigned i = n;
  ///
  /// do {
  ///   i++;
  /// } while (i != 5);
  bool canTripCountEqualIVTypeRangeSize() const;
};

/// Loop information related to its parallel characteristics, such as
/// OpenMP caluses (private variables, reductions, number of threads etc.).
/// The source of this information can be incoming LLVM IR or
/// HIRParVecAnalysis.
/// TODO support other clauses - private variables, reductions,...
class HLLoopParallelTraits {
public:
  HLLoopParallelTraits() : NumThreads(nullptr) {}
  HLLoopParallelTraits(const HLLoopParallelTraits &O) = delete;

  void setNumThreads(const RegDDRef *N) { NumThreads = N; }
  const RegDDRef *getNumThreads() { return NumThreads; }

private:
  // Number of threads as found in the num_threads clause
  const RegDDRef *NumThreads;
};

} // End namespace loopopt

// Traits of HLLoop for OptReportBuilder.
template <> struct OptReportTraits<loopopt::HLLoop> {
  using ObjectHandleTy = loopopt::HLLoop &;
  using ChildNodeTy = loopopt::HLLoop;

  static OptReport getOptReport(const loopopt::HLLoop &Loop) {
    return Loop.getOptReport();
  }

  static void setOptReport(loopopt::HLLoop &Loop, OptReport OR) {
    Loop.setOptReport(OR);
  }

  static void eraseOptReport(loopopt::HLLoop &Loop) { Loop.eraseOptReport(); }

  static DebugLoc getDebugLoc(const loopopt::HLLoop &Loop) {
    return Loop.getDebugLoc();
  }

  static Optional<std::string> getOptReportTitle(const loopopt::HLLoop &Loop) {
    return None;
  }

  static OptReport getOrCreatePrevOptReport(loopopt::HLLoop &Loop,
                                            const OptReportBuilder &Builder);

  static OptReport getOrCreateParentOptReport(loopopt::HLLoop &Loop,
                                              const OptReportBuilder &Builder);

  using NodeVisitorTy = std::function<void(loopopt::HLLoop &)>;
  static void traverseChildNodesBackward(loopopt::HLLoop &Loop,
                                         NodeVisitorTy Func);
};

} // End namespace llvm

#endif
