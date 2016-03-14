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
#include <iterator>

namespace llvm {

class BasicBlock;

namespace loopopt {

class RegDDRef;

/// \brief Base class for high level nodes which can contain DDRefs.
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
  HLDDNode(unsigned SCID);
  virtual ~HLDDNode() override{};

  friend class HLNodeUtils;

  /// \brief Copy Constructor
  HLDDNode(const HLDDNode &HLDDNodeObj);

  /// The DDRef indices correspond to the operand number in the instruction
  /// with the first DDRef being for lval, if applicable.
  RegDDRefTy RegDDRefs;

  /// \brief Sets HLDDNode for Ref.
  static void setNode(RegDDRef *Ref, HLDDNode *HNode);

  /// \brief Implements get*OperandDDRef() functionality.
  RegDDRef *getOperandDDRefImpl(unsigned OperandNum) const;
  /// \brief Implements set*OperandDDRef() functionality.
  void setOperandDDRefImpl(RegDDRef *Ref, unsigned OperandNum);

  /// \brief Virtual Clone Implementation
  /// This function populates the GotoList with Goto branches
  /// and LabelMap with Old and New Labels.
  virtual HLDDNode *cloneImpl(GotoContainerTy *GotoList,
                              LabelMapTy *LabelMap) const override = 0;

public:
  /// \brief Prints HLInst.
  virtual void print(formatted_raw_ostream &OS, unsigned Depth,
                     bool Detailed = false) const override;
  /// \brief Prints list of attached Reg and Blob DD Refs
  void printDDRefs(formatted_raw_ostream &OS, unsigned Depth) const;

  /// DDRef iterator methods
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

  /// \brief Method for supporting type inquiry through isa, cast, and dyn_cast.
  static bool classof(const HLNode *Node) {
    return (Node->getHLNodeID() == HLNode::HLLoopVal) ||
           (Node->getHLNodeID() == HLNode::HLIfVal) ||
           (Node->getHLNodeID() == HLNode::HLInstVal) ||
           (Node->getHLNodeID() == HLNode::HLSwitchVal);
  }

  /// DDRef acess methods
  unsigned getNumDDRefs() const { return RegDDRefs.size(); }

  /// \brief Virtual Clone method
  virtual HLDDNode *clone() const override = 0;

  /// \brief Returns the number of operands (and lval, if applicable) this node
  /// is supposed to have.
  virtual unsigned getNumOperands() const = 0;

  /// \brief Verifies DDRefs attached to the node.
  virtual void verify() const override;

  /// \brief Returns true if Ref is the lval DDRef of this node.
  bool isLval(const RegDDRef *Ref) const;

  /// \brief Returns true if Ref is a rval DDRef of this node.
  bool isRval(const RegDDRef *Ref) const;

  /// \brief Returns true if Ref is a fake DDRef attached to this node.
  bool isFake(const RegDDRef *Ref) const;
};

} // End namespace loopopt

} // End namespace llvm

#endif
