//===------ ConstDDRef.h - Data dependency node in HIR ----------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file defines the ConstDDRef node in high level IR.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_IR_INTEL_LOOPIR_CONSTDDREF_H
#define LLVM_IR_INTEL_LOOPIR_CONSTDDREF_H

#include "llvm/IR/Intel_LoopIR/DDRef.h"
#include "llvm/Support/Casting.h"

namespace llvm {

namespace loopopt {

class CanonExpr;
class HLDDNode;

/// \brief Represents a numerical constant value
///
/// Objects of this class cannot cause data dependencies and are only used to
/// capture the modifications made to the High Level IR without needing to
/// change
/// the LLVM IR.
class ConstDDRef : public DDRef {
private:
  CanonExpr *CExpr;
  HLDDNode *Node;

protected:
  ConstDDRef(CanonExpr *CE);
  ~ConstDDRef() {}

  /// \brief Copy constructor used by cloning.
  ConstDDRef(const ConstDDRef &ConstDDRefObj);

  friend class DDRefUtils;

  /// \brief Sets the HLDDNode of this ConstDDRef
  void setHLDDNode(HLDDNode *HNode) override { Node = HNode; }

public:
  /// \brief Returns CanonExpr representing this ref.
  CanonExpr *getCanonExpr() { return CExpr; }
  const CanonExpr *getCanonExpr() const { return CExpr; }

  /// \brief Returns HLDDNode this DDRef is attached to.
  HLDDNode *getHLDDNode() const override { return Node; }

  /// TODO implementation
  Value *getLLVMValue() const override { return nullptr; }

  /// \brief Method for supporting type inquiry through isa, cast, and dyn_cast.
  static bool classof(const DDRef *Ref) {
    return Ref->getDDRefID() == DDRef::ConstDDRefVal;
  }

  /// clone() - Create a copy of 'this' ConstDDRef that is identical in all
  /// ways except the following:
  ///   * The HLDDNode needs to be set explicitly
  ConstDDRef *clone() const override;
};

} // End namespace loopopt

} // End namespace llvm

#endif
