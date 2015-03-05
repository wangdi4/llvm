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
  HLSwitch();
  ~HLSwitch() {}

  /// \brief Copy constructor used by cloning.
  HLSwitch(const HLSwitch &HLSwitchObj);

  friend class HLNodeUtils;

public:
  /// \brief Method for supporting type inquiry through isa, cast, and dyn_cast.
  static bool classof(const HLNode *Node) {
    return Node->getHLNodeID() == HLNode::HLSwitchVal;
  }

  /// clone() - Create a copy of 'this' switch that is identical in all
  /// ways except the following:
  ///   * The Switch has no parent
  ///   * TODO : Implement other cloning members later
  HLSwitch *clone() const;

  /// \brief Returns the number of operands this node is supposed to have.
  /// TODO : Implement later
  unsigned getNumOperands() const override { return 1; }
};

} // End namespace loopopt

} // End namespace llvm

#endif
