//===----------- HLGoto.h - High level IR goto node -------------*- C++ -*-===//
//
// Copyright (C) 2015-2018 Intel Corporation. All rights reserved.
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

#include "llvm/Analysis/Intel_LoopAnalysis/IR/HLDDNode.h"

namespace llvm {

class BasicBlock;

namespace loopopt {

class HLLabel;

/// High level node representing an unconditional jump
class HLGoto final : public HLNode {
private:
  // Storing both src and dest bblock tells us the exact edge that this goto
  // represents. This helps us handle liveout values during code gen as we can
  // map old edge to new ones. The observation here is that HIR transformations
  // can either optimize away external jumps or replicate them. In both these
  // cases, the relationship between liveout values -> old liveout edge does not
  // change.
  BasicBlock *SrcBBlock;
  BasicBlock *TargetBBlock;
  HLLabel *TargetLabel;

  DebugLoc DbgLoc;

protected:
  HLGoto(HLNodeUtils &HNU, BasicBlock *SrcBBlock, BasicBlock *TargetBB);
  HLGoto(HLNodeUtils &HNU, HLLabel *TargetL);
  virtual ~HLGoto() override {}

  /// Copy constructor used by cloning.
  HLGoto(const HLGoto &HLGotoObj);

  friend class HLNodeUtils;

  /// Clone Implementation
  /// This function populates the GotoList with the cloned Goto only if
  /// the target label is internal. LabelMap is ignored for this
  /// implementation. Returns the cloned Goto.
  HLGoto *cloneImpl(GotoContainerTy *GotoList, LabelMapTy *LabelMap,
                    HLNodeMapper *NodeMapper) const override;

public:
  /// Prints HLGoto.
  virtual void print(formatted_raw_ostream &OS, unsigned Depth,
                     bool Detailed) const override;

  /// Returns the src basic block of this goto. This is only relevant for
  /// external jumps.
  BasicBlock *getSrcBBlock() const { return SrcBBlock; }

  /// Returns the target basic block of this goto.
  BasicBlock *getTargetBBlock() const { return TargetBBlock; }

  /// Returns the target label, if one exists. It is null
  /// for external gotos.
  HLLabel *getTargetLabel() const { return TargetLabel; }

  /// Sets the target label.
  void setTargetLabel(HLLabel *Label) {
    TargetLabel = Label;
    SrcBBlock = nullptr;
    TargetBBlock = nullptr;
  }

  /// Returns true if this goto jumps outside the region.
  bool isExternal() const { return (TargetLabel == nullptr); }

  /// Returns true if this goto is an early exit of \p Loop.
  bool isEarlyExit(HLLoop *Loop) const;

  /// Method for supporting type inquiry through isa, cast, and dyn_cast.
  static bool classof(const HLNode *Node) {
    return Node->getHLNodeClassID() == HLNode::HLGotoVal;
  }

  /// clone() - Create a copy of 'this' HLGoto that is identical in all
  /// ways except the following:
  ///   * The HLGoto has no parent
  HLGoto *clone(HLNodeMapper *NodeMapper = nullptr) const override;

  /// Verifies HLGoto integrity.
  virtual void verify() const override;

  const DebugLoc getDebugLoc() const override { return DbgLoc; }
  void setDebugLoc(const DebugLoc &Loc) { DbgLoc = Loc; }

  /// Returns true if this is the back edge of its parent unknown loop.
  bool isUnknownLoopBackEdge() const;
};

} // End namespace loopopt

template <>
struct DenseMapInfo<loopopt::HLGoto *>
    : public loopopt::DenseHLNodeMapInfo<loopopt::HLGoto> {};

template <>
struct DenseMapInfo<const loopopt::HLGoto *>
    : public loopopt::DenseHLNodeMapInfo<const loopopt::HLGoto> {};

} // End namespace llvm

#endif
