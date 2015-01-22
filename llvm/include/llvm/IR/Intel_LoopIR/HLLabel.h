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
class HLLabel : public HLDDNode {
private:
  BasicBlock* SrcBBlock;

protected:
  HLLabel(HLNode* Par, BasicBlock* SrcBB);
  ~HLLabel() { }

  friend class HLNodeUtils;

  HLLabel* clone_impl() const override;

public:

  /// \brief Returns the underlying LLVM BBlock.
  BasicBlock* getSrcBBlock() const { return SrcBBlock; }

  /// \brief Method for supporting type inquiry through isa, cast, and dyn_cast.
  static bool classof(const HLNode* Node) {
    return Node->getHLNodeID() == HLNode::HLLabelVal;
  }

};


} // End namespace loopopt

} // End namespace llvm

#endif
