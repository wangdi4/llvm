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

/// \brief High level node representing a LLVM instruction
class HLInst : public HLDDNode {
private:
  // Neither the pointer nor the Instruction object pointed to can be modified
  // once HLInst has been constructed.
  const Instruction *const Inst;
  HLInst *SafeRednSucc;
  // Only used for Cmp and Select instructions.
  PredicateTy CmpOrSelectPred;

protected:
  explicit HLInst(Instruction *In);
  virtual ~HLInst() override {}

  /// \brief Copy constructor used by cloning.
  HLInst(const HLInst &HLInstObj);

  friend class HLNodeUtils;

  /// \brief Implements getNumOperands() functionality.
  unsigned getNumOperandsInternal() const;

  /// \brief Implements isInPreheader*()/isInPostexit*() functionality.
  bool isInPreheaderPostexitImpl(bool Preheader) const;

  /// \brief Initializes some of the members to bring the object in a sane
  /// state.
  void initialize();

  /// \brief Clone Implementation
  /// This function ignores the GotoList and LabelMap parameter.
  /// Returns cloned Inst.
  HLInst *cloneImpl(GotoContainerTy *GotoList,
                    LabelMapTy *LabelMap) const override;

  /// \brief Returns true if there is a separator that we can print between
  /// operands of this instruction. Prints the separators if Print is true.
  bool checkSeparator(formatted_raw_ostream &OS, bool Print) const;

  /// \brief Prints the beginning Opcode equivalent for this instruction.
  void printBeginOpcode(formatted_raw_ostream &OS, bool HasSeparator) const;

  /// \brief Prints the ending Opcode equivalent for this instruction.
  void printEndOpcode(formatted_raw_ostream &OS) const;

public:
  /// \brief Prints HLInst.
  virtual void print(formatted_raw_ostream &OS, unsigned Depth,
                     bool Detailed = false) const override;

  /// \brief Returns the underlying Instruction.
  const Instruction *getLLVMInstruction() const { return Inst; }
  /// \brief Returns true if this node is part of safe reduction chain.
  bool isSafeRedn() const { return SafeRednSucc != 0; };

  /// \brief Returns the safe reduction successor of this node in the chain.
  HLInst *getSafeRednSucc() const { return SafeRednSucc; };
  void setSafeRednSucc(HLInst *Succ) { SafeRednSucc = Succ; }

  /// \brief Returns true if the underlying instruction has an lval.
  bool hasLval() const;
  /// \brief Returns true if the underlying instruction has a single rval.
  bool hasRval() const;

  const Value *getOperandValue(unsigned OperandNum);

  /// \brief Returns the DDRef associated with the Nth operand (starting with
  /// 0).
  RegDDRef *getOperandDDRef(unsigned OperandNum);
  const RegDDRef *getOperandDDRef(unsigned OperandNum) const;
  /// \brief Sets the DDRef associated with the Nth operand (starting with 0).
  void setOperandDDRef(RegDDRef *Ref, unsigned OperandNum);
  /// \brief Removes and returns the DDRef associated with the Nth operand
  /// (starting with 0).
  RegDDRef *removeOperandDDRef(unsigned OperandNum);

  /// \brief Returns the lval DDRef of this node.
  RegDDRef *getLvalDDRef();
  const RegDDRef *getLvalDDRef() const;
  /// \brief Sets the lval DDRef of this node.
  void setLvalDDRef(RegDDRef *RDDRef);
  /// \brief Removes and returns the lval DDRef of this node.
  RegDDRef *removeLvalDDRef();

  /// \brief Returns the single rval DDRef of this node.
  RegDDRef *getRvalDDRef();
  const RegDDRef *getRvalDDRef() const;
  /// \brief Sets the single rval DDRef of this node.
  void setRvalDDRef(RegDDRef *Ref);
  /// \brief Removes and returns the single rval DDRef of this node.
  RegDDRef *removeRvalDDRef();

  /// \brief Adds an extra RegDDRef which does not correspond to lval or any
  /// operand. This DDRef is not used for code generation but might be used for
  /// exposing DD edges. TODO: more on this later...
  void addFakeDDRef(RegDDRef *RDDRef);

  /// \brief Removes a previously inserted fake DDRef.
  void removeFakeDDRef(RegDDRef *RDDRef);

  /// Operand DDRef iterator methods
  ddref_iterator op_ddref_begin() { return RegDDRefs.begin(); }
  const_ddref_iterator op_ddref_begin() const { return RegDDRefs.begin(); }
  ddref_iterator op_ddref_end() { return RegDDRefs.begin() + getNumOperands(); }
  const_ddref_iterator op_ddref_end() const {
    return RegDDRefs.begin() + getNumOperands();
  }

  reverse_ddref_iterator op_ddref_rbegin() {
    return RegDDRefs.rend() - getNumOperands();
  }
  const_reverse_ddref_iterator op_ddref_rbegin() const {
    return RegDDRefs.rend() - getNumOperands();
  }
  reverse_ddref_iterator op_ddref_rend() { return RegDDRefs.rend(); }
  const_reverse_ddref_iterator op_ddref_rend() const {
    return RegDDRefs.rend();
  }

  /// Fake DDRef iterator methods
  ddref_iterator fake_ddref_begin() { return op_ddref_end(); }
  const_ddref_iterator fake_ddref_begin() const { return op_ddref_end(); }
  ddref_iterator fake_ddref_end() { return RegDDRefs.end(); }
  const_ddref_iterator fake_ddref_end() const { return RegDDRefs.end(); }

  reverse_ddref_iterator fake_ddref_rbegin() { return RegDDRefs.rbegin(); }
  const_reverse_ddref_iterator fake_ddref_rbegin() const {
    return RegDDRefs.rbegin();
  }
  reverse_ddref_iterator fake_ddref_rend() { return op_ddref_rbegin(); }
  const_reverse_ddref_iterator fake_ddref_rend() const {
    return op_ddref_rbegin();
  }

  /// \brief Method for supporting type inquiry through isa, cast, and dyn_cast.
  static bool classof(const HLNode *Node) {
    return Node->getHLNodeID() == HLNode::HLInstVal;
  }

  /// clone() - Create a copy of 'this' HLInst that is identical in all
  /// ways except the following:
  ///   * The HLInst has no parent
  ///   * Safe Reduction Successor is set to nullptr
  HLInst *clone() const override;

  /// \brief Returns the number of operands this HLInst is supposed to have.
  /// If lval is present, it becomes the 0th operand.
  unsigned getNumOperands() const override;

  /// \brief Returns true if this is in a loop's preheader.
  bool isInPreheader() const;
  /// \brief Returns true if this is in a loop's postexit.
  bool isInPostexit() const;
  /// \brief Returns true if this is in a loop's preheader or postexit.
  bool isInPreheaderOrPostexit() const;

  /// \brief Returns predicate for select instruction.
  PredicateTy getPredicate() const {
    assert((isa<CmpInst>(Inst) || isa<SelectInst>(Inst)) &&
           "This instruction does not contain a predicate!");
    return CmpOrSelectPred;
  }

  /// \brief Sets predicate for select instruction.
  void setPredicate(PredicateTy Pred) {
    assert((isa<CmpInst>(Inst) || isa<SelectInst>(Inst)) &&
           "This instruction does not contain a predicate!");
    CmpOrSelectPred = Pred;
  }

  /// \brief Retuns true if this is a bitcast instruction with identical src and
  /// dest types. These are generally inserted by SSA deconstruction pass.
  bool isCopyInst() const;

  /// \brief Returns true if this is a call instruction.
  bool isCallInst() const;

  /// \brief Verifies HLInst integrity.
  virtual void verify() const override;

  /// \brief Checks whether the instruction is a call to intrinsic
  /// If so, IntrinID is populated back.
  bool isIntrinCall(Intrinsic::ID &IntrinID) const;

  /// \brief Checks whether the instruction is a call to SIMD Directive,
  /// i.e., intel_directive call with the right metadata.
  bool isSIMDDirective() const;
};

} // End namespace loopopt

} // End namespace llvm

#endif
