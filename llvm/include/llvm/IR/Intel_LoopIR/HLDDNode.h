//===----------- HLDDNode.h - High level IR node ----------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file defines the HLDDnode node.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_IR_INTEL_LOOPIR_HLDDNODE_H
#define LLVM_IR_INTEL_LOOPIR_HLDDNODE_H

#include "llvm/IR/Intel_LoopIR/HLNode.h"
#include "llvm/ADT/SmallVector.h"
#include <iterator>

namespace llvm {

class BasicBlock;

namespace loopopt {

class DDRef;

/// \brief Base class for high level nodes which can contain DDRefs.
class HLDDNode : public HLNode {
public:
  /// SmallVector of 5 should be enough for most HLDDNodes.
  /// HLLoop usually requires 5 ddrefs(lower, tripcount, stride, 2 ztt ddrefs).
  /// Most instructions are covered except some vector instructions which are
  /// a minority.
  typedef SmallVector<DDRef *, 5> DDRefTy;

  /// Iterators to iterate over DDRefs
  typedef DDRefTy::iterator ddref_iterator;
  typedef DDRefTy::const_iterator const_ddref_iterator;
  typedef DDRefTy::reverse_iterator reverse_ddref_iterator;
  typedef DDRefTy::const_reverse_iterator const_reverse_ddref_iterator;

protected:
  HLDDNode(unsigned SCID);
  virtual ~HLDDNode(){};

  friend class HLNodeUtils;

  /// \brief Copy Constructor
  HLDDNode(const HLDDNode &HLDDNodeObj);

  /// The DDRef indices correspond to the operand number in the instruction
  /// with the first DDRef being for lval, if applicable.
  DDRefTy DDRefs;

  /// \brief Resize DDRefs to match number of operands in the Node.
  virtual void resizeDDRefsToNumOperands();

  /// \brief Sets HLDDNode for Ref.
  static void setNode(DDRef *Ref, HLDDNode *HNode);

  /// \brief Implements get*OperandDDRef() functionality.
  DDRef *getOperandDDRefImpl(unsigned OperandNum) const;
  /// \brief Implements set*OperandDDRef() functionality.
  void setOperandDDRefImpl(DDRef *Ref, unsigned OperandNum);

public:
  /// DDRef iterator methods
  ddref_iterator ddref_begin() { return DDRefs.begin(); }
  const_ddref_iterator ddref_begin() const { return DDRefs.begin(); }
  ddref_iterator ddref_end() { return DDRefs.end(); }
  const_ddref_iterator ddref_end() const { return DDRefs.end(); }

  reverse_ddref_iterator ddref_rbegin() { return DDRefs.rbegin(); }
  const_reverse_ddref_iterator ddref_rbegin() const { return DDRefs.rbegin(); }
  reverse_ddref_iterator ddref_rend() { return DDRefs.rend(); }
  const_reverse_ddref_iterator ddref_rend() const { return DDRefs.rend(); }

  /// DDRef acess methods
  unsigned getNumDDRefs() const { return DDRefs.size(); }

  /// Virtual Clone method
  virtual HLDDNode *clone() const = 0;

  /// \brief Returns the DDRef associated with the Nth operand (starting with
  /// 0).
  DDRef *getOperandDDRef(unsigned OperandNum);
  const DDRef *getOperandDDRef(unsigned OperandNum) const;
  /// \brief Sets the DDRef associated with the Nth operand (starting with 0).
  void setOperandDDRef(DDRef *Ref, unsigned OperandNum);
  /// \brief Removes and returns the DDRef associated with the Nth operand
  /// (starting with 0).
  DDRef *removeOperandDDRef(unsigned OperandNum);

  /// \brief Returns the number of operands (and lval if applicable) this node
  /// is supposed to have.
  virtual unsigned getNumOperands() const = 0;
};

} // End namespace loopopt

} // End namespace llvm

#endif
