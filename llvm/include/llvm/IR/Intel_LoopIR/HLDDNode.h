//===----------- HLDDNode.h - High level IR DD node -------------*- C++ -*-===//
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
// This file defines the HLDDnode node.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_IR_INTEL_LOOPIR_HLDDNODE_H
#define LLVM_IR_INTEL_LOOPIR_HLDDNODE_H

#include "llvm/ADT/SmallVector.h"
#include "llvm/IR/Intel_LoopIR/HLNode.h"
#include "llvm/IR/Intel_LoopIR/HLRegion.h"
#include "llvm/IR/Intel_LoopIR/RegDDRef.h"
#include <iterator>

namespace llvm {

class BasicBlock;

namespace loopopt {

/// Base class for high level nodes which can contain DDRefs.
class HLDDNode : public HLNode {
public:
  /// SmallVector of 5 should be enough for most HLDDNodes.
  /// HLLoop usually requires 5 ddrefs(lower, tripcount, stride, 2 ztt ddrefs).
  /// Most instructions are covered except some vector instructions which are
  /// a minority.
  typedef SmallVector<RegDDRef *, 5> RegDDRefTy;

  /// Iterators to iterate over RegDDRefs
  typedef RegDDRefTy::iterator ddref_iterator;
  typedef RegDDRefTy::const_iterator const_ddref_iterator;
  typedef RegDDRefTy::reverse_iterator reverse_ddref_iterator;
  typedef RegDDRefTy::const_reverse_iterator const_reverse_ddref_iterator;

protected:
  HLDDNode(HLNodeUtils &HNU, unsigned SCID);
  virtual ~HLDDNode() override{};

  friend class HLNodeUtils;

  /// Copy Constructor
  HLDDNode(const HLDDNode &HLDDNodeObj);

  /// The mask DDRef for this node - defaults to null and set by vectorizer
  /// when the corresponding HLDDNode is masked.
  RegDDRef *MaskDDRef;

  /// The DDRef indices correspond to the operand number in the instruction
  /// with the first DDRef being for lval, if applicable.
  RegDDRefTy RegDDRefs;

  /// Number of fake lvals in the node. This is used to separate fake lvals and
  /// rvals.
  unsigned NumFakeLvals;

  /// Sets HLDDNode for Ref.
  static void setNode(RegDDRef *Ref, HLDDNode *HNode);

  /// Implements get*OperandDDRef() functionality.
  RegDDRef *getOperandDDRefImpl(unsigned OperandNum) const;

  /// Implements set*OperandDDRef() functionality.
  void setOperandDDRefImpl(RegDDRef *Ref, unsigned OperandNum);

public:
  /// Prints HLInst.
  virtual void print(formatted_raw_ostream &OS, unsigned Depth,
                     bool Detailed = false) const override;
  /// Prints list of attached Reg and Blob DD Refs
  void printDDRefs(formatted_raw_ostream &OS, unsigned Depth) const;

  /// DDRef iterator methods. This traversal includes fake DDRefs as well.
  ddref_iterator ddref_begin() { return RegDDRefs.begin(); }
  const_ddref_iterator ddref_begin() const {
    return const_cast<HLDDNode *>(this)->ddref_begin();
  }

  ddref_iterator ddref_end() { return RegDDRefs.end(); }
  const_ddref_iterator ddref_end() const {
    return const_cast<HLDDNode *>(this)->ddref_end();
  }

  reverse_ddref_iterator ddref_rbegin() { return RegDDRefs.rbegin(); }
  const_reverse_ddref_iterator ddref_rbegin() const {
    return const_cast<HLDDNode *>(this)->ddref_rbegin();
  }

  reverse_ddref_iterator ddref_rend() { return RegDDRefs.rend(); }
  const_reverse_ddref_iterator ddref_rend() const {
    return const_cast<HLDDNode *>(this)->ddref_rend();
  }

  /// Operand DDRef iterator methods
  ddref_iterator op_ddref_begin() { return ddref_begin(); }
  const_ddref_iterator op_ddref_begin() const {
    return const_cast<HLDDNode *>(this)->op_ddref_begin();
  }
  ddref_iterator op_ddref_end() { return ddref_begin() + getNumOperands(); }
  const_ddref_iterator op_ddref_end() const {
    return const_cast<HLDDNode *>(this)->op_ddref_end();
  }

  reverse_ddref_iterator op_ddref_rbegin() {
    return ddref_rend() - getNumOperands();
  }
  const_reverse_ddref_iterator op_ddref_rbegin() const {
    return const_cast<HLDDNode *>(this)->op_ddref_rbegin();
  }
  reverse_ddref_iterator op_ddref_rend() { return ddref_rend(); }
  const_reverse_ddref_iterator op_ddref_rend() const {
    return const_cast<HLDDNode *>(this)->op_ddref_rend();
  }

  /// Fake DDRef iterator methods
  ddref_iterator fake_ddref_begin() { return op_ddref_end(); }
  const_ddref_iterator fake_ddref_begin() const {
    return const_cast<HLDDNode *>(this)->fake_ddref_begin();
  }
  ddref_iterator fake_ddref_end() { return ddref_end(); }
  const_ddref_iterator fake_ddref_end() const {
    return const_cast<HLDDNode *>(this)->fake_ddref_end();
  }

  reverse_ddref_iterator fake_ddref_rbegin() { return ddref_rbegin(); }
  const_reverse_ddref_iterator fake_ddref_rbegin() const {
    return const_cast<HLDDNode *>(this)->fake_ddref_rbegin();
  }
  reverse_ddref_iterator fake_ddref_rend() { return op_ddref_rbegin(); }
  const_reverse_ddref_iterator fake_ddref_rend() const {
    return const_cast<HLDDNode *>(this)->fake_ddref_rend();
  }

  // Fake lval DDRefs
  ddref_iterator lval_fake_ddref_begin() { return fake_ddref_begin(); }
  const_ddref_iterator lval_fake_ddref_begin() const {
    return const_cast<HLDDNode *>(this)->lval_fake_ddref_begin();
  }
  ddref_iterator lval_fake_ddref_end() {
    return fake_ddref_begin() + NumFakeLvals;
  }
  const_ddref_iterator lval_fake_ddref_end() const {
    return const_cast<HLDDNode *>(this)->lval_fake_ddref_end();
  }

  // Fake rval DDRefs
  ddref_iterator rval_fake_ddref_begin() { return lval_fake_ddref_end(); }
  const_ddref_iterator rval_fake_ddref_begin() const {
    return const_cast<HLDDNode *>(this)->rval_fake_ddref_begin();
  }
  ddref_iterator rval_fake_ddref_end() { return fake_ddref_end(); }
  const_ddref_iterator rval_fake_ddref_end() const {
    return const_cast<HLDDNode *>(this)->rval_fake_ddref_end();
  }

  /// Method for supporting type inquiry through isa, cast, and dyn_cast.
  static bool classof(const HLNode *Node) {
    return (Node->getHLNodeID() == HLNode::HLLoopVal) ||
           (Node->getHLNodeID() == HLNode::HLIfVal) ||
           (Node->getHLNodeID() == HLNode::HLInstVal) ||
           (Node->getHLNodeID() == HLNode::HLSwitchVal);
  }

  /// Rval operand DDRef iterator methods
  ddref_iterator rval_op_ddref_begin() {
    return hasLval() ? op_ddref_begin() + 1 : op_ddref_begin();
  }
  const_ddref_iterator rval_op_ddref_begin() const {
    return const_cast<HLDDNode *>(this)->rval_op_ddref_begin();
  }
  ddref_iterator rval_op_ddref_end() { return op_ddref_end(); }
  const_ddref_iterator rval_op_ddref_end() const {
    return const_cast<HLDDNode *>(this)->rval_op_ddref_end();
  }

  reverse_ddref_iterator rval_op_ddref_rbegin() { return op_ddref_rbegin(); }
  const_reverse_ddref_iterator rval_op_ddref_rbegin() const {
    return const_cast<HLDDNode *>(this)->rval_op_ddref_rbegin();
  }
  reverse_ddref_iterator rval_op_ddref_rend() {
    return hasLval() ? op_ddref_rend() - 1 : op_ddref_rend();
  }
  const_reverse_ddref_iterator rval_op_ddref_rend() const {
    return const_cast<HLDDNode *>(this)->rval_op_ddref_rend();
  }

  /// DDRef acess methods

  /// Returns total number of DDRefs attached to this node (including fake
  /// DDRefs).
  unsigned getNumDDRefs() const { return RegDDRefs.size(); }

  /// Returns the number of operands (and lval, if applicable) this node is
  /// supposed to have.
  virtual unsigned getNumOperands() const = 0;

  /// Verifies DDRefs attached to the node.
  virtual void verify() const override;

  /// Returns true if node has an lval.
  virtual bool hasLval() const { return false; }

  /// Returns true if the node has a single rval.
  virtual bool hasRval() const { return false; }

  /// Returns true if Ref is the lval DDRef of this node.
  /// Default implementation returns false as only HLInst can contain lval
  /// DDRef.
  virtual bool isLval(const RegDDRef *Ref) const {
    assert((this == Ref->getHLDDNode()) && "Ref does not belong to this node!");
    return false;
  }

  /// Returns true if Ref is a rval DDRef of this node. MaskDDRef is treated
  /// as a rval DDRef
  virtual bool isRval(const RegDDRef *Ref) const { return !isLval(Ref); }

  /// Returns true if Ref is a fake DDRef attached to this node.
  bool isFake(const RegDDRef *Ref) const;

  /// Returns true if Ref is a fake lval DDRef attached to this node.
  bool isFakeLval(const RegDDRef *Ref) const;

  /// Returns true if Ref is a fake rval DDRef attached to this node.
  bool isFakeRval(const RegDDRef *Ref) const;

  /// Returns true if node has fake DDRefs.
  bool hasFakeDDRefs() const { return fake_ddref_begin() != fake_ddref_end(); }

  /// Returns true if node has fake lval DDRefs.
  bool hasFakeLvalDDRefs() const {
    return lval_fake_ddref_begin() != lval_fake_ddref_end();
  }

  /// Returns true if node has fake Rval DDRefs.
  bool hasFakeRvalDDRefs() const {
    return rval_fake_ddref_begin() != rval_fake_ddref_end();
  }

  /// Returns the DDRef associated with the Nth operand (starting with 0).
  RegDDRef *getOperandDDRef(unsigned OperandNum);
  const RegDDRef *getOperandDDRef(unsigned OperandNum) const;
  /// Sets/replaces the DDRef associated with the Nth operand (starting with 0).
  void setOperandDDRef(RegDDRef *Ref, unsigned OperandNum);

  /// Replaces existing operand DDRef with \p NewRef.
  void replaceOperandDDRef(RegDDRef *ExistingRef, RegDDRef *NewRef);
  /// Removes and returns the DDRef associated with the Nth operand (starting
  /// with 0).
  RegDDRef *removeOperandDDRef(unsigned OperandNum);

  /// Returns the operand number of \p OpRef.
  unsigned getOperandNum(RegDDRef *OpRef) const;

  /// Returns the lval DDRef of this node. Returns null if it doesn't exist.
  virtual RegDDRef *getLvalDDRef() { return nullptr; }
  virtual const RegDDRef *getLvalDDRef() const { return nullptr; }

  /// Sets/replaces the lval DDRef of this node.
  virtual void setLvalDDRef(RegDDRef *RDDRef) {
    llvm_unreachable("Node doesn't have an lval!");
  }
  /// Removes and returns the lval DDRef of this node.
  virtual RegDDRef *removeLvalDDRef() {
    llvm_unreachable("Node doesn't have an lval!");
  }

  /// Returns the single rval DDRef of this node. Returns null if it doesn't
  /// exist.
  virtual RegDDRef *getRvalDDRef() { return nullptr; }
  virtual const RegDDRef *getRvalDDRef() const { return nullptr; }

  /// Sets/replaces the single rval DDRef of this node.
  virtual void setRvalDDRef(RegDDRef *Ref) {
    llvm_unreachable("Node doesn't have an rval!");
  }
  /// Removes and returns the single rval DDRef of this node.
  virtual RegDDRef *removeRvalDDRef() {
    llvm_unreachable("Node doesn't have an rval!");
  }

  /// Returns the mask DDRef associated with this node.
  RegDDRef *getMaskDDRef() { return MaskDDRef; }
  const RegDDRef *getMaskDDRef() const {
    return const_cast<HLDDNode *>(this)->getMaskDDRef();
  }

  /// Sets the mask DDRef associated with this node. This gets used to generate
  /// masked loads/stores when needed.
  void setMaskDDRef(RegDDRef *Ref);

  /// Adds an extra lval RegDDRef which does not correspond to lval or any
  /// operand.
  /// This DDRef might be used for exposing DD edges.
  /// This applies to call instructions with pointer arguments. Since the
  /// arguments can be derefenerenced and read/written to inside the call, we
  /// need to add corresponding fake refs for them.
  /// The mask DDRef is also added as a fake DDRef for exposing DD edges.
  void addFakeLvalDDRef(RegDDRef *RDDRef);
  void addFakeRvalDDRef(RegDDRef *RDDRef);

  /// Removes a previously inserted fake DDRef.
  void removeFakeDDRef(RegDDRef *RDDRef);

  /// Replaces existing fake DDRef with \p NewRef.
  void replaceFakeDDRef(RegDDRef *ExistingRef, RegDDRef *NewRef);

  /// Replaces existing operand/fake DDRef with \p NewRef.
  void replaceOperandOrFakeDDRef(RegDDRef *ExistingRef, RegDDRef *NewRef);

  /// Returns true if symbase is live out of region.
  bool isLiveOutOfRegion(unsigned SB) const {
    return getParentRegion()->isLiveOut(SB);
  }

  /// Returns true if symbase is live into parent loop.
  bool isLiveIntoParentLoop(unsigned SB) const;

  /// Returns true if symbase is live out of parent loop.
  bool isLiveOutOfParentLoop(unsigned SB) const;

  /// Shifts all ddrefs in the Node by \p Amount at level \p LoopLevel.
  void shift(unsigned LoopLevel, int64_t Amount) {
    for (auto RefIt = ddref_begin(), End = ddref_end(); RefIt != End; ++RefIt) {
      (*RefIt)->shift(LoopLevel, Amount);
    }
  }
};

} // End namespace loopopt

} // End namespace llvm

#endif
