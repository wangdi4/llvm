//===----------- HLGoto.h - High level IR goto node -------------*- C++ -*-===//
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
// This file defines the HLGoto node.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_IR_INTEL_LOOPIR_HLGOTO_H
#define LLVM_IR_INTEL_LOOPIR_HLGOTO_H

#include "llvm/IR/Intel_LoopIR/HLDDNode.h"

namespace llvm {

class BasicBlock;

namespace loopopt {

class HLLabel;

/// \brief High level node representing an unconditional jump
class HLGoto final : public HLNode {
private:
  BasicBlock *TargetBBlock;
  HLLabel *TargetLabel;

  DebugLoc DbgLoc;

protected:
  HLGoto(HLNodeUtils &HNU, BasicBlock *TargetBB);
  HLGoto(HLNodeUtils &HNU, HLLabel *TargetL);
  virtual ~HLGoto() override {}

  /// \brief Copy constructor used by cloning.
  HLGoto(const HLGoto &HLGotoObj);

  friend class HLNodeUtils;

  /// \brief Clone Implementation
  /// This function populates the GotoList with the cloned Goto only if
  /// the target label is internal. LabelMap is ignored for this
  /// implementation. Returns the cloned Goto.
  HLGoto *cloneImpl(GotoContainerTy *GotoList, LabelMapTy *LabelMap,
                    HLNodeMapper *NodeMapper) const override;

public:
  /// \brief Prints HLGoto.
  virtual void print(formatted_raw_ostream &OS, unsigned Depth,
                     bool Detailed) const override;

  /// \brief Returns the target basic block of this goto.
  BasicBlock *getTargetBBlock() const { return TargetBBlock; }
  /// \brief Returns the target label, if one exists. It is null
  /// for external gotos.
  HLLabel *getTargetLabel() const { return TargetLabel; }
  /// \brief Sets the target label.
  void setTargetLabel(HLLabel *Label) {
    TargetLabel = Label;
    TargetBBlock = nullptr;
  }

  /// \brief Returns true if this goto jumps outside the region.
  bool isExternal() const { return (TargetLabel == nullptr); }

  /// \brief Method for supporting type inquiry through isa, cast, and dyn_cast.
  static bool classof(const HLNode *Node) {
    return Node->getHLNodeID() == HLNode::HLGotoVal;
  }

  /// clone() - Create a copy of 'this' HLGoto that is identical in all
  /// ways except the following:
  ///   * The HLGoto has no parent
  HLGoto *clone(HLNodeMapper *NodeMapper = nullptr) const override;

  /// \brief Verifies HLGoto integrity.
  virtual void verify() const override;

  const DebugLoc getDebugLoc() const override { return DbgLoc; }
  void setDebugLoc(const DebugLoc &Loc) { DbgLoc = Loc; }

  /// Returns true if this is the back edge of its parent unknown loop.
  bool isUnknownLoopBackEdge() const;
};

} // End namespace loopopt

} // End namespace llvm

#endif
