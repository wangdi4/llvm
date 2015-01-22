//===----------- HLSwitch.h - High level IR node ----------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file defines the HLSwitch node.
//
//===----------------------------------------------------------------------===//


#ifndef LLVM_IR_INTEL_LOOPIR_HLSWITCH_H
#define LLVM_IR_INTEL_LOOPIR_HLSWITCH_H

#include "llvm/IR/Intel_LoopIR/HLDDNode.h"

namespace llvm {

namespace loopopt {

/// \brief High level node representing a switch statement
/// TODO: will be implemented later.
class HLSwitch : public HLDDNode {
protected:
  explicit HLSwitch(HLNode* Par);
  ~HLSwitch() { }

  friend class HLNodeUtils;

  HLSwitch* clone_impl() const override;

public:
  /// \brief Method for supporting type inquiry through isa, cast, and dyn_cast.
  static bool classof(const HLNode* Node) {
    return Node->getHLNodeID() == HLNode::HLSwitchVal;
  }

};

} // End namespace loopopt

} // End namespace llvm

#endif
