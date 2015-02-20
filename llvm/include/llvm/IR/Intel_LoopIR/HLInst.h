//===------------- HLInst.h - High level IR node ----------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file defines the HLInst node.
//
//===----------------------------------------------------------------------===//


#ifndef LLVM_IR_INTEL_LOOPIR_HLINST_H
#define LLVM_IR_INTEL_LOOPIR_HLINST_H

#include "llvm/IR/Instruction.h"

#include "llvm/IR/Intel_LoopIR/HLDDNode.h"


namespace llvm {

class BasicBlock;

namespace loopopt {

class RegDDRef;

/// \brief High level node representing a LLVM instruction
class HLInst : public HLDDNode {
private:
  Instruction* Inst;
  HLInst* SafeRednSucc;

protected:
  explicit HLInst(Instruction* In);
  ~HLInst() { }

  /// \brief Copy constructor used by cloning.
  HLInst(const HLInst &HLInstObj);

  friend class HLNodeUtils;

public:
  /// \brief Returns the underlying Instruction
  Instruction* getLLVMInstruction() const { return Inst; }
  /// \brief Returns true if this node is part of safe reduction chain.
  bool isSafeRedn() const { return SafeRednSucc != 0; };

  /// \brief Returns the safe reduction successor of this node in the chain.
  HLInst* getSafeRednSucc() const { return SafeRednSucc; };
  void setSafeRednSucc(HLInst* Succ) { SafeRednSucc = Succ; }

  /// \brief Returns the lval DDRef of this node.
  RegDDRef* getLvalDDRef();

  /// \brief Method for supporting type inquiry through isa, cast, and dyn_cast.
  static bool classof(const HLNode* Node) {
    return Node->getHLNodeID() == HLNode::HLInstVal;
  }

  /// clone() - Create a copy of 'this' HLInst that is identical in all
  /// ways except the following:
  ///   * The HLInst has no parent
  ///   * Safe Reduction Successor is set to nullptr
  HLInst* clone() const override;

};


} // End namespace loopopt

} // End namespace llvm

#endif
