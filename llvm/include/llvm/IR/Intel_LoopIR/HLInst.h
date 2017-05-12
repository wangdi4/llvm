//===------- HLInst.h - High level IR instruction node ----*- C++ -*-------===//
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
// This file defines the HLInst node.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_IR_INTEL_LOOPIR_HLINST_H
#define LLVM_IR_INTEL_LOOPIR_HLINST_H

#include "llvm/IR/Instructions.h"

#include "llvm/IR/Intel_LoopIR/HLDDNode.h"

namespace llvm {

class BasicBlock;

namespace loopopt {

class RegDDRef;

/// High level node representing a LLVM instruction
class HLInst final : public HLDDNode {
private:
  // Neither the pointer nor the Instruction object pointed to can be modified
  // once HLInst has been constructed.
  const Instruction *const Inst;
  // Only used for Cmp and Select instructions.
  HLPredicate CmpOrSelectPred;

protected:
  explicit HLInst(HLNodeUtils &HNU, Instruction *Inst);
  virtual ~HLInst() override {}

  /// Copy constructor used by cloning.
  HLInst(const HLInst &HLInstObj);

  friend class HLNodeUtils;

  /// Implements getNumOperands() functionality.
  unsigned getNumOperandsInternal() const;

  /// Implements isInPreheader*()/isInPostexit*() functionality.
  bool isInPreheaderPostexitImpl(bool Preheader) const;

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

public:
  /// Prints HLInst.
  virtual void print(formatted_raw_ostream &OS, unsigned Depth,
                     bool Detailed = false) const override;

  /// Returns the underlying Instruction.
  const Instruction *getLLVMInstruction() const { return Inst; }

  /// Returns true if the underlying instruction has an lval.
  virtual bool hasLval() const override;
  /// Returns true if the underlying instruction has a single rval.
  virtual bool hasRval() const override;

  /// Returns the lval DDRef of this node.
  virtual RegDDRef *getLvalDDRef() override;
  virtual const RegDDRef *getLvalDDRef() const override;
  /// Sets/replaces the lval DDRef of this node.
  virtual void setLvalDDRef(RegDDRef *RDDRef) override;
  /// Removes and returns the lval DDRef of this node.
  virtual RegDDRef *removeLvalDDRef() override;

  /// Returns the single rval DDRef of this node.
  virtual RegDDRef *getRvalDDRef() override;
  virtual const RegDDRef *getRvalDDRef() const override;
  /// Sets/replaces the single rval DDRef of this node.
  virtual void setRvalDDRef(RegDDRef *Ref) override;
  /// Removes and returns the single rval DDRef of this node.
  virtual RegDDRef *removeRvalDDRef() override;

  /// Returns true if Ref is the lval DDRef of this node.
  virtual bool isLval(const RegDDRef *Ref) const override;

  /// Returns true if Ref is a rval DDRef of this node.
  virtual bool isRval(const RegDDRef *Ref) const override;

  /// Method for supporting type inquiry through isa, cast, and dyn_cast.
  static bool classof(const HLNode *Node) {
    return Node->getHLNodeID() == HLNode::HLInstVal;
  }

  /// clone() - Create a copy of 'this' HLInst that is identical in all ways
  /// except the following:
  ///   * The HLInst has no parent
  ///   * Safe Reduction Successor is set to nullptr
  HLInst *clone(HLNodeMapper *NodeMapper = nullptr) const override;

  /// Returns the number of operands this HLInst is supposed to have.
  /// If lval is present, it becomes the 0th operand.
  unsigned getNumOperands() const override;

  /// Returns true if this is in a loop's preheader.
  bool isInPreheader() const;
  /// Returns true if this is in a loop's postexit.
  bool isInPostexit() const;
  /// Returns true if this is in a loop's preheader or postexit.
  bool isInPreheaderOrPostexit() const;

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
  bool isCallInst() const { return isa<CallInst>(Inst); }

  /// Verifies HLInst integrity.
  virtual void verify() const override;

  /// Checks whether the instruction is a call to intrinsic If so, IntrinID is
  /// populated back.
  bool isIntrinCall(Intrinsic::ID &IntrinID) const;

  /// Checks whether the instruction is a call to a specific Intel Directive,
  /// i.e. the intel_directive call with the right metadata.
  bool isIntelDirective(int DirectiveID) const;

  /// Checks whether the instruction is a call to a omp simd directive.
  bool isSIMDDirective() const;

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

  const DebugLoc getDebugLoc() const override;
  void setDebugLoc(const DebugLoc &Loc);
};

} // End namespace loopopt

} // End namespace llvm

#endif
