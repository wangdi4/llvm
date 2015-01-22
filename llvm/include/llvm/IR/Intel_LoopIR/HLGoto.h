//===------------- HLGoto.h - High level IR node ----------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file defines the HLGoto node.
//
//===----------------------------------------------------------------------===//


#ifndef LLVM_IR_INTEL_LOOPIR_HLGOTO_H
#define LLVM_IR_INTEL_LOOPIR_HLGOTO_H

#include "llvm/IR/Intel_LoopIR/HLDDNode.h"


namespace llvm {

class BasicBlock;

namespace loopopt {

/// \brief High level node represening an unconditional jump
class HLGoto : public HLDDNode {
private:
  BasicBlock* TargetBBlock;
  HLLabel* TargetLabel;

protected:
  HLGoto(HLNode* Par, BasicBlock* TargetBB, HLLabel* TargetL);
  ~HLGoto() { }

  friend class HLNodeUtils;

  HLGoto* clone_impl() const override;

public:
  /// \brief Returns the target basic block of this goto.
  BasicBlock* getTargetBBlock() const { return TargetBBlock; }
  /// \brief Returns the target label, if one exists. It is null
  /// for external gotos.
  HLLabel* getTargetLabel() const { return TargetLabel; }

  /// \brief Returns true if this goto jumps outside the region.
  bool isExternal() const;

  /// \brief Method for supporting type inquiry through isa, cast, and dyn_cast.
  static bool classof(const HLNode* Node) {
    return Node->getHLNodeID() == HLNode::HLGotoVal;
  }

};

} // End namespace loopopt

} // End namespace llvm

#endif
