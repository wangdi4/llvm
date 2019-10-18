//===------- HLInst.h - High level IR instruction node ----*- C++ -*-------===//
//
// Copyright (C) 2015-2019 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// This file defines the HLInst node.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_IR_INTEL_LOOPIR_HLINST_H
#define LLVM_IR_INTEL_LOOPIR_HLINST_H

#include "llvm/Analysis/Directives.h"
#include "llvm/IR/Instructions.h"

#include "llvm/Analysis/Intel_LoopAnalysis/IR/HLDDNode.h"

namespace llvm {

class BasicBlock;

namespace loopopt {

/// High level node representing a LLVM instruction
class HLInst final : public HLDDNode {
private:
  // Neither the pointer nor the Instruction object pointed to can be modified
  // once HLInst has been constructed.
  const Instruction *const Inst;
  // Only used for Cmp and Select instructions.
  HLPredicate CmpOrSelectPred;
  bool IsSinked;

protected:
  explicit HLInst(HLNodeUtils &HNU, Instruction *Inst);
  virtual ~HLInst() override {}

  /// Copy constructor used by cloning.
  HLInst(const HLInst &HLInstObj);

  friend class HLNodeUtils;

  /// Implements getNumOperands() functionality.
  unsigned getNumOperandsInternal() const;

  /// Implements isInPreheader*()/isInPostexit*() functionality.
  bool isInPreheaderPostexitImpl(bool Preheader, HLLoop *ParLoop) const;

  /// Initializes some of the members to bring the object in a sane state.
  void initialize();

  /// Clone Implementation
  /// This function ignores the GotoList and LabelMap parameter.
  /// Returns cloned Inst.
  HLInst *cloneImpl(GotoContainerTy *GotoList, LabelMapTy *LabelMap,
                    HLNodeMapper *NodeMapper) const override;

  /// Returns true if there is a separator that we can print between operands of
  /// this instruction. Prints the separators if Print is true.
  bool checkSeparator(formatted_raw_ostream &OS, bool Print) const;

  /// Prints the beginning Opcode equivalent for this instruction.
  void printBeginOpcode(formatted_raw_ostream &OS, bool HasSeparator) const;

  /// Prints the ending Opcode equivalent for this instruction.
  void printEndOpcode(formatted_raw_ostream &OS) const;

  /// Checks if instruction is a max or a min based on flag: true for max, false
  /// for min
  bool checkMinMax(bool IsMin, bool IsMax) const;

  void printReductionInfo(formatted_raw_ostream &OS) const;

public:
  /// Prints HLInst.
  virtual void print(formatted_raw_ostream &OS, unsigned Depth,
                     bool Detailed = false) const override;

  /// Returns the underlying Instruction.
  const Instruction *getLLVMInstruction() const { return Inst; }

  /// Returns true if the underlying instruction has an lval.
  virtual bool hasLval() const override {
    return (!Inst->getType()->isVoidTy() || isa<StoreInst>(Inst));
  }

  /// Returns true if the underlying instruction has a single rval.
  virtual bool hasRval() const override {
    return (isa<StoreInst>(Inst) || isa<GetElementPtrInst>(Inst) ||
            (hasLval() && isa<UnaryInstruction>(Inst)));
  }

  /// Returns the lval DDRef of this node.
  virtual RegDDRef *getLvalDDRef() override;
  virtual const RegDDRef *getLvalDDRef() const override {
    // If we make this function non-virtual in HLDDNode base class the compiler
    // is not able to find it.
    return const_cast<HLInst *>(this)->getLvalDDRef();
  }

  /// Sets/replaces the lval DDRef of this node.
  virtual void setLvalDDRef(RegDDRef *RDDRef) override {
    assert(hasLval() && "This instruction does not have an lval!");
    setOperandDDRefImpl(RDDRef, 0);
  }

  /// Removes and returns the lval DDRef of this node.
  virtual RegDDRef *removeLvalDDRef() override;

  /// Returns the single rval DDRef of this node.
  virtual RegDDRef *getRvalDDRef() override;
  virtual const RegDDRef *getRvalDDRef() const override {
    // If we make this function non-virtual in HLDDNode base class the compiler
    // is not able to find it.
    return const_cast<HLInst *>(this)->getRvalDDRef();
  }

  /// Sets/replaces the single rval DDRef of this node.
  virtual void setRvalDDRef(RegDDRef *Ref) override {
    assert(hasRval() && "This instruction does not have a rval!");
    setOperandDDRefImpl(Ref, 1);
  }

  /// Removes and returns the single rval DDRef of this node.
  virtual RegDDRef *removeRvalDDRef() override;

  /// Returns true if Ref is the lval DDRef of this node.
  virtual bool isLval(const RegDDRef *Ref) const override {
    assert((this == Ref->getHLDDNode()) && "Ref does not belong to this node!");
    return ((getLvalDDRef() == Ref) || isFakeLval(Ref));
  }

  /// Method for supporting type inquiry through isa, cast, and dyn_cast.
  static bool classof(const HLNode *Node) {
    return Node->getHLNodeClassID() == HLNode::HLInstVal;
  }

  /// clone() - Create a copy of 'this' HLInst that is identical in all ways
  /// except the following:
  ///   * The HLInst has no parent
  ///   * Safe Reduction Successor is set to nullptr
  HLInst *clone(HLNodeMapper *NodeMapper = nullptr) const override;

  /// Returns the number of operands this HLInst is supposed to have.
  /// If lval is present, it becomes the 0th operand.
  unsigned getNumOperands() const override { return getNumOperandsInternal(); }

  /// Returns the number of operand bundles associated with this instruction.
  /// Returns 0 for non-call instructions.
  unsigned getNumOperandBundles() const {
    auto Call = getCallInst();
    return Call ? Call->getNumOperandBundles() : 0;
  }

  /// Returns the number of non-bundle operands.
  /// It is equivalent to getNumOperands() for non-call instructions.
  /// Note that we do not count the last function pointer operand for indirect
  /// calls.
  unsigned getNumNonBundleOperands() const {
    auto Call = getCallInst();
    return Call ? (Call->getNumArgOperands() + hasLval()) : getNumOperands();
  }

  /// Returns the number of operands in the bundle \p BundleNum which is in
  /// range [0, getNumOperandBundles()).
  unsigned getNumBundleOperands(unsigned BundleNum) const {
    assert(isCallInst() && BundleNum < getNumOperandBundles() &&
           "Invalid bundle number!");
    return cast<CallInst>(Inst)->getOperandBundleAt(BundleNum).Inputs.size();
  }

  /// Wrapper over CallInst. Useful for getting operand bundle tag.
  /// NOTE: This should NOT be used to retrieve operand values. Use
  /// bundle_op_ddref_begin()/end() instead.
  OperandBundleUse getOperandBundleAt(unsigned BundleNum) const {
    assert(isCallInst() && "Call instruction expected!");
    return cast<CallInst>(Inst)->getOperandBundleAt(BundleNum);
  }

  /// Useful for iterating over operand bundles
  /// Returns the first ddref iterator belonging to operand bundle \p BundleNum
  /// which is in range [0, getNumOperandBundles()).
  ddref_iterator bundle_op_ddref_begin(unsigned BundleNum);
  const_ddref_iterator bundle_op_ddref_begin(unsigned BundleNum) const {
    return const_cast<HLInst *>(this)->bundle_op_ddref_begin(BundleNum);
  }

  /// Returns the end ddref iterator for operand bundle \p BundleNum which is in
  /// range [0, getNumOperandBundles()).
  ddref_iterator bundle_op_ddref_end(unsigned BundleNum) {
    return bundle_op_ddref_begin(BundleNum) + getNumBundleOperands(BundleNum);
  }
  const_ddref_iterator bundle_op_ddref_end(unsigned BundleNum) const {
    return const_cast<HLInst *>(this)->bundle_op_ddref_begin(BundleNum);
  }

  /// Returns true if this is in a loop's preheader.
  /// \p User can optionally pass in the parent loop if readily available. This
  /// is only for compile time savings.
  bool isInPreheader(HLLoop *ParLoop = nullptr) const {
    return isInPreheaderPostexitImpl(true, ParLoop);
  }

  /// Returns true if this is in a loop's postexit.
  /// \p User can optionally pass in the parent loop if readily available. This
  /// is only for compile time savings.
  bool isInPostexit(HLLoop *ParLoop = nullptr) const {
    return isInPreheaderPostexitImpl(false, ParLoop);
  }

  /// Returns true if this is in a loop's preheader or postexit.
  /// \p User can optionally pass in the parent loop if readily available. This
  /// is only for compile time savings.
  bool isInPreheaderOrPostexit(HLLoop *ParLoop = nullptr) const {
    return (isInPreheader(ParLoop) || isInPostexit(ParLoop));
  }

  /// Returns predicate for select instruction.
  const HLPredicate &getPredicate() const {
    assert((isa<CmpInst>(Inst) || isa<SelectInst>(Inst)) &&
           "This instruction does not contain a predicate!");
    return CmpOrSelectPred;
  }

  /// Sets predicate for select instruction.
  void setPredicate(const HLPredicate &Pred) {
    assert((isa<CmpInst>(Inst) || isa<SelectInst>(Inst)) &&
           "This instruction does not contain a predicate!");

    CmpOrSelectPred = Pred;
  }

  /// Retuns true if this is a bitcast instruction with identical src and dest
  /// types. These are generally inserted by SSA deconstruction pass.
  bool isCopyInst() const;

  /// Returns true if this is a call instruction.
  bool isCallInst() const {
    return isa<CallInst>(Inst) && !isa<SubscriptInst>(Inst);
  }

  /// Returns CallInst pointer if this is a call instruction.
  const CallInst *getCallInst() const {
    return isCallInst() ? cast<CallInst>(Inst) : nullptr;
  }

  /// Returns true if \p Call only accesses inaccessible or arg memory.
  static bool onlyAccessesInaccessibleOrArgMemory(const CallInst *Call) {
    return Call->onlyAccessesArgMemory() ||
           Call->onlyAccessesInaccessibleMemory() ||
           Call->onlyAccessesInaccessibleMemOrArgMem();
  }

  /// Returns true if \p Call instruction has side effects.
  /// This is for transformations like redundant node removal. It prevents
  /// elimination of instruction.
  static bool hasSideEffects(const CallInst *Call) {
    assert(Call && "Inst is nullptr");
    return Call->mayHaveSideEffects();
  }

  /// Returns true if this is a call instruction with side effects.
  bool isSideEffectsCallInst() const {
    auto Call = getCallInst();
    return Call && hasSideEffects(Call);
  }

  /// Returns true if \p Call instruction has unsafe side effects.
  /// This is for transformations like interchange/fusion. It prevents aliasing
  /// as well as reordering issues. It is a stronger check than
  /// hasUnknownAliasing() below.
  static bool hasUnsafeSideEffects(const CallInst *Call) {
    assert(Call && "Inst is nullptr");
    return Call->mayThrow() ||
           (!Call->doesNotAccessMemory() && !Call->onlyAccessesArgMemory());
  }

  /// Returns true if this is a call instruction with unsafe side effects.
  bool isUnsafeSideEffectsCallInst() const {
    auto Call = getCallInst();
    return Call && hasUnsafeSideEffects(Call);
  }

  /// Returns true if \p Call instruction can alias arbitrary memory.
  /// This is for transformations like loop memory motion and scalar
  /// replacement.
  static bool hasUnknownAliasing(const CallInst *Call) {
    assert(Call && "Inst is nullptr");
    return !Call->doesNotAccessMemory() &&
           !onlyAccessesInaccessibleOrArgMemory(Call);
  }

  /// Returns true if this is a call instruction with unknown memory access.
  bool isUnknownAliasingCallInst() const {
    auto Call = getCallInst();
    return Call && hasUnknownAliasing(Call);
  }

  /// Returns true if this is an indirect call instruction.
  bool isIndirectCallInst() const {
    auto Call = getCallInst();
    return (Call && !Call->getCalledFunction());
  }

  /// Verifies HLInst integrity.
  virtual void verify() const override;

  /// Returns IntrinsicInst whether the instruction is a call to intrinsic else
  /// returns nullptr.
  const IntrinsicInst *getIntrinCall() const {
    return dyn_cast_or_null<IntrinsicInst>(getCallInst());
  }

  /// Checks whether the instruction is a call to intrinsic. If so, IntrinID is
  /// populated back.
  bool isIntrinCall(Intrinsic::ID &IntrinID) const;

  /// Checks whether the instruction is a call to a specific Directive.
  bool isDirective(int DirectiveID) const;

  /// Checks whether the instruction is a call to a omp simd directive.
  bool isSIMDDirective() const { return isDirective(DIR_OMP_SIMD); }

  /// Checks whether the instruction is a call to an auto vectorization
  /// directive.
  bool isAutoVecDirective() const;

  /// Checks if the Opcode is a reduction and returns OpCode
  bool isReductionOp(unsigned *OpCode) const;

  /// Checks if instruction is a min.
  bool isMin() const { return checkMinMax(true, false); }

  /// Checks if instruction is a max.
  bool isMax() const { return checkMinMax(false, true); }

  /// Checks if instruction is a min or a max.
  bool isMinOrMax() const { return checkMinMax(true, true); }

  /// Returns true if instruction represents an abs() operation.
  /// TODO: Extend to handle floating point abs().
  bool isAbs() const;

  /// Return the identity value corresponding to the given reduction
  /// instruction opcode and specified type.
  static Constant *getRecurrenceIdentity(unsigned RednOpCode, Type *Ty);

  /// Return true if OpCode is a valid reduction opcode.
  static bool isValidReductionOpCode(unsigned OpCode);

  /// Return true if the instruction was sinked.
  bool isSinked() const { return IsSinked; }

  void setIsSinked(bool Flag) { IsSinked = Flag; }

  const DebugLoc getDebugLoc() const override;
  void setDebugLoc(const DebugLoc &Loc);
};

/// Wraps around a HIR instruction which is an OpenMP region entry intrinsic and
/// provides access to objects and characteristics representing OpenMP
/// semantics. OpenMP clause 0 corresponds to operand bundle 1. 0'th bundle
/// represents the region directive.
class OMPRegionProxy {
public:
  /// Creates a proxy for the HIR instruction \p I.
  /// OMPRegionProxy::isOmpRegionEntry() must return \c true for this
  /// instruction.
  OMPRegionProxy(const HLInst *I) {
    Impl = (OmpDir = getOmpRegionEntryDir(I)) >= 0 ? I : nullptr;
  }

  /// \return Whether this proxy represents an OpenMP region entry.
  bool isValid() const { return Impl != nullptr; }

  /// \return The OpenMP directive describing the region kind.
  int getOmpDir() const {
    assert(isValid());
    return OmpDir;
  }

  /// \return OpenMP clause ID at 0-based index \p I.
  int getOmpClauseID(unsigned I) const;

  /// Asserts that the clause at index I has a single operand and returns the
  /// DDRef representing its value.
  const RegDDRef *getOmpClauseSingleOpnd(unsigned I) const {
    assert(Impl->getNumBundleOperands(I + 1) == 1 && "single operand expected");
    const RegDDRef *const *OpndBegI = Impl->bundle_op_ddref_begin(I + 1);
    return *OpndBegI;
  }

  /// \return The name of the \p I 'th OpenMP clause.
  StringRef getOmpClauseName(unsigned I) const {
    return Impl->getOperandBundleAt(I + 1).getTagName();
  }

  /// \return The number of OpenMP clauses this region defines
  unsigned getNumOmpClauses() const { return Impl->getNumOperandBundles() - 1; }

  /// Checks if \p I instruction is an OpenMP region entry intrinsic.
  /// \return
  ///   \c corresponding OpenMP directive ID if yes, \c -1 otherwise
  static int getOmpRegionEntryDir(const HLInst *I);

  /// Checks if \p Exit instruction is an OpenMP region exit intrinsic.
  /// If \p Entry is not \c nullptr, then additionally checks if the entry and
  /// the exit match
  /// \return
  ///   corresponding OpenMP directive ID if the check(s) are successful,
  ///   \c -1 otherwise
  static int getOmpRegionExitDir(const HLInst *Exit,
                                 const HLInst *Entry = nullptr);

private:
  /// The HIR instruction this proxy works for.
  const HLInst *Impl;

  /// Cached OpenMP directive found in the region this proxy represents.
  int OmpDir;
};

} // End namespace loopopt

template <>
struct DenseMapInfo<loopopt::HLInst *>
    : public loopopt::DenseHLNodeMapInfo<loopopt::HLInst> {};

template <>
struct DenseMapInfo<const loopopt::HLInst *>
    : public loopopt::DenseHLNodeMapInfo<const loopopt::HLInst> {};

} // End namespace llvm

#endif
