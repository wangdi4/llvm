//===--------- HLLabel.h - High level IR label node -------------*- C++ -*-===//
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
// This file defines the HLLabel node.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_IR_INTEL_LOOPIR_HLLABEL_H
#define LLVM_IR_INTEL_LOOPIR_HLLABEL_H

#include "llvm/ADT/SmallString.h"
#include "llvm/ADT/StringSet.h"

#include "llvm/IR/Intel_LoopIR/HLDDNode.h"

namespace llvm {

class BasicBlock;

namespace loopopt {

/// \brief High level node representing a label.
class HLLabel : public HLNode {
private:
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  static StringSet<> LabelNames;
#endif

  BasicBlock *SrcBBlock;
  SmallString<32> Name;

  /// \brief Updates Name to make it unique across HLLabels
  void makeNameUnique();

protected:
  explicit HLLabel(BasicBlock *SrcBB);
  explicit HLLabel(const Twine &Name);
  virtual ~HLLabel() override;

  /// \brief Copy constructor used by cloning.
  HLLabel(const HLLabel &LabelObj);

  friend class HLNodeUtils;

  /// \brief Clone Implementation
  /// This function populates the LabelMap with the old Label (Before cloning)
  /// and new Label (After cloning).
  /// GotoList is ignored for this implementation. Returns the cloned Label.
  HLLabel *cloneImpl(GotoContainerTy *GotoList,
                     LabelMapTy *LabelMap) const override;

public:
  /// \brief Prints HLLabel.
  virtual void print(formatted_raw_ostream &OS, unsigned Depth,
                     bool Detailed = false) const override;

  /// \brief Prints a basic block name, even for unnamed blocks.
  static void printBBlockName(raw_ostream &O, const BasicBlock &BB);

  /// \brief Returns the underlying LLVM BBlock.
  BasicBlock *getSrcBBlock() const { return SrcBBlock; }

  /// \brief Method for supporting type inquiry through isa, cast, and dyn_cast.
  static bool classof(const HLNode *Node) {
    return Node->getHLNodeID() == HLNode::HLLabelVal;
  }

  // \brief Returns Unique Label name.
  StringRef getName() const { return StringRef(Name); }

  /// clone() - Create a copy of 'this' HLLabel that is identical in all
  /// ways except the following:
  ///   * The HLLabel has no parent
  HLLabel *clone() const override;
};

} // End namespace loopopt

} // End namespace llvm

#endif
