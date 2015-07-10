//===------------------------------------------------------------*- C++ -*-===//
//
//   Copyright (C) 2015 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation. and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//   Source file:
//   ------------
//   VPOAvrLoop.h -- Defines the Abstract Vector Representation (AVR) loop node.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_ANALYSIS_VPO_AVR_LOOP_H
#define LLVM_ANALYSIS_VPO_AVR_LOOP_H

#include "llvm/Analysis/VPO/Vecopt/AVR/VPOAvr.h"

namespace llvm { // LLVM Namespace

class LoopInfo;

namespace vpo {  // VPO Vectorizer Namespace

/// \brief Loop node abstract vector representation
///
/// An AVRLoop node represents a loop found in LLVM IR or LoopOpt HIR.
class AVRLoop : public AVR {

public:

  /// List Container of AVRFunction's children nodes.
  typedef AVRContainerTy ChildrenTy;

  /// Iterators to iterate over children nodes
  typedef ChildrenTy::iterator child_iterator;
  typedef ChildrenTy::const_iterator const_child_iterator;
  typedef ChildrenTy::reverse_iterator reverse_child_iterator;
  typedef ChildrenTy::const_reverse_iterator const_reverse_child_iterator;

  /// Preheader iterators
  typedef ChildrenTy::iterator pre_iterator;
  typedef ChildrenTy::const_iterator const_pre_iterator;
  typedef ChildrenTy::reverse_iterator reverse_pre_iterator;
  typedef ChildrenTy::const_reverse_iterator const_reverse_pre_iterator;

  /// Postexit iterators
  typedef ChildrenTy::iterator post_iterator;
  typedef ChildrenTy::const_iterator const_post_iterator;
  typedef ChildrenTy::reverse_iterator reverse_post_iterator;
  typedef ChildrenTy::const_reverse_iterator const_reverse_post_iterator;

private:

  /// Pointer to orignial LLVM Loop Class.
  const LoopInfo *OrigLoop;
  /// Loop nesting level.
  unsigned NestingLevel;
  /// Number of loop exits. 
  unsigned NumExits;
  /// Loop is of form Do-While.
  const bool IsDoWhile;
  /// Loop is innermost.
  bool IsInnerMost;
  /// Loop is a candidate for vectorization.
  bool IsVectorCandidate;
  /// Loop is a candidate for auto-vectorization.
  bool IsAutoVectorCandidate;
  /// Loop is a candidate for explicit-vectorization.
  bool IsExplicitVectorCandidate;

  /// Children of this AVRLoop
  ChildrenTy Children;
  /// Iterator pointing to begining of children nodes
  ChildrenTy::iterator ChildBegin;
  /// Iterator pointing to begining of preheader nodes
  ChildrenTy::iterator PreheaderBegin;
  /// Iterator pointing to begining of postexit nodes
  ChildrenTy::iterator PostexitBegin;

  /// Pointer to Loop info Contained in WRN
  // const WRN *WRNInfo;

  // TODO: PHI Node

protected:

  AVRLoop(const LoopInfo *OrigLLVMLoop, bool IsDoWhileLoop);
  AVRLoop(const AVRLoop &AVROrigLoop);
  ~AVRLoop();

  /// \brief Set the loop nesting level
  void setNestingLevel(unsigned Level) { NestingLevel = Level; }
  /// \brief Set the loop nesting level
  void setIsInnerMost(bool InnerM) { IsInnerMost = InnerM; }
  /// \brief Sets loop as a vectorization candidate.
  void setVectorCandidate(bool VectorCand) { IsVectorCandidate = VectorCand; }
  /// \brief Sets loop as an auto-vectorization candidate.
  void setAutoVectorCandidate(bool AutoVectorCand) {
    IsAutoVectorCandidate = AutoVectorCand; }
  /// \brief Sets loop as an explicit-vectorization candidate.
  void setExplicitVectorCandidate(bool ExplicitVectorCand) {
    IsExplicitVectorCandidate = ExplicitVectorCand; }

  /// Only this utility class should be use to modify/delete AVR nodes.
  friend class AVRUtils;

public:

  /// \brief Returns Original LLVM Loop
  const LoopInfo *getLLVMLoop() const {
     return OrigLoop; 
  }
  /// \brief Returns true if loop is a do loop.
  bool isDoLoop() const {
    return (!IsDoWhile && (NumExits == 1));
  }
  /// \brief Returns true is loop is of form Do-While.
  bool isDoWhileLoop() const {
    return IsDoWhile;
  }
  /// \brief Returns the number of exits of the loop.
  unsigned getNumExits() const {
    return NumExits;
  }
  /// \brief Returns loops nesting level
  unsigned getNestingLevel() const {
    return NestingLevel;
  }
  /// \brief Returns true is loop is innermost
  bool isInnerMost() const {
    return IsInnerMost;
  }

  child_iterator child_begin() { return Children.begin(); }
  const_child_iterator child_begin() const { return Children.begin(); }
  reverse_child_iterator child_rbegin() { return Children.rbegin(); }
  const_reverse_child_iterator child_rbegin() const { return Children.rbegin(); }
  child_iterator child_end() { return Children.end(); }
  const_child_iterator child_end() const { return Children.end(); }
  reverse_child_iterator child_rend() { return Children.rend(); }
  const_reverse_child_iterator child_rend() const { return Children.rend(); }

  /// Children Methods
  AVR *getFirstChild();

  /// \brief Returns the first child if it exists, otherwise returns null.
  const AVR *getFirstChild() const {
    return const_cast<AVRLoop *>(this)->getFirstChild();
  }

  /// \brief Returns the last child if it exists, otherwise returns null.
  AVR *getLastChild();

  /// \brief Returns const pointer to last child if it exisits.
  const AVR *getLastChild() const {
    return const_cast<AVRLoop *>(this)->getLastChild();
  }

  /// \brief Returns the number of children.
  unsigned getNumChildren() const { return Children.size(); }

  /// \brief Returns true if it has children.
  bool hasChildren() const { return !Children.empty(); }

  /// \brief Method for supporting type inquiry through isa, cast, and dyn_cast.
  static bool classof(const AVR *Node) {
    return Node->getAVRID() == AVR::AVRLoopNode;
  }

  void print() const override;
  void dump() const override;
  AVRLoop *clone() const override; 

  /// \brief Code generation for AVR loop.
  void codeGen() override;
};


} // End VPO Vectorizer Namespace
} // End LLVM Namespace

#endif  // LLVM_ANALYSIS_VPO_AVR_LOOP_H
