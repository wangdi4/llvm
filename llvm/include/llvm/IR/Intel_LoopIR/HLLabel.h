//===------------ HLLabel.h - High level IR node ----------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file defines the HLLabel node.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_IR_INTEL_LOOPIR_HLLABEL_H
#define LLVM_IR_INTEL_LOOPIR_HLLABEL_H

#include "llvm/IR/Intel_LoopIR/HLDDNode.h"

namespace llvm {

class BasicBlock;

namespace loopopt {

/// \brief High level node representing a label.
class HLLabel : public HLNode {
private:
  BasicBlock *SrcBBlock;

protected:
  explicit HLLabel(BasicBlock *SrcBB);
  ~HLLabel() {}

  /// \brief Copy constructor used by cloning.
  HLLabel(const HLLabel &LabelObj);

  friend class HLNodeUtils;

public:
  /// \brief Returns the underlying LLVM BBlock.
  BasicBlock *getSrcBBlock() const { return SrcBBlock; }

  /// \brief Method for supporting type inquiry through isa, cast, and dyn_cast.
  static bool classof(const HLNode *Node) {
    return Node->getHLNodeID() == HLNode::HLLabelVal;
  }

  /// clone() - Create a copy of 'this' HLLabel that is identical in all
  /// ways except the following:
  ///   * The HLLabel has no parent
  HLLabel *clone() const override;
};

} // End namespace loopopt

} // End namespace llvm

#endif
