//===------------------------------------------------------------*- C++ -*-===//
//
//   Copyright (C) 2015-2016 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation. and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This header file defines the Abstract Vector Representation (AVR) loop node.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_ANALYSIS_VPO_AVR_LOOP_H
#define LLVM_ANALYSIS_VPO_AVR_LOOP_H

#include "llvm/Analysis/Intel_VPO/Vecopt/VPOAvr.h"
#include "llvm/Analysis/Intel_VPO/WRegionInfo/WRegion.h"

namespace llvm { // LLVM Namespace

class Loop;

namespace vpo { // VPO Vectorizer Namespace

class AVRIf;

/// \brief Loop node abstract vector representation
///
/// An AVRLoop node represents a loop found in LLVM IR or LoopOpt HIR.
class AVRLoop : public AVR {

public:
  /// List Container of AVRFunction's children nodes.
  typedef AVRContainerTy ChildNodeTy;
  typedef ChildNodeTy PreheaderTy;
  typedef ChildNodeTy PostexitTy;

  /// Iterators to iterate over children nodes
  typedef ChildNodeTy::iterator child_iterator;
  typedef ChildNodeTy::const_iterator const_child_iterator;
  typedef ChildNodeTy::reverse_iterator reverse_child_iterator;
  typedef ChildNodeTy::const_reverse_iterator const_reverse_child_iterator;

  /// Preheader iterators
  typedef child_iterator pre_iterator;
  typedef const_child_iterator const_pre_iterator;
  typedef reverse_child_iterator reverse_pre_iterator;
  typedef const_reverse_child_iterator const_reverse_pre_iterator;

  /// Postexit iterators
  typedef child_iterator post_iterator;
  typedef const_child_iterator const_post_iterator;
  typedef reverse_child_iterator reverse_post_iterator;
  typedef const_reverse_child_iterator const_reverse_post_iterator;

private:
  /// WRNVecLoop Node.
  WRNVecLoopNode *WrnLoopNode;
  /// Zero trip test.
  AVRIf *Ztt;
  /// Loop nesting level.
  unsigned NestingLevel;
  /// Number of loop exits.
  unsigned NumberOfExits;
  /// Loop is of form Do-While.
  bool IsDoWhile;
  /// Loop is innermost.
  bool IsInnerMost;
  /// Loop is a candidate for vectorization.
  bool IsVectorCandidate;
  /// Loop is a candidate for auto-vectorization.
  bool IsAutoVectorCandidate;
  /// Loop is a candidate for explicit-vectorization.
  bool IsExplicitVectorCandidate;
  /// Children - Contains the children nodes of this loop.
  ChildNodeTy Children;
  /// PreheaderChildren - Contains the preheader nodes of this loop.
  ChildNodeTy PreheaderChildren;
  /// PostexitChildren - Contains the postexit nodes of this loop.
  ChildNodeTy PostexitChildren;

  // TODO: PHI Node

protected:
  AVRLoop(unsigned SCID);
  virtual ~AVRLoop() override {}

  /// \brief Set the loop nesting level
  void setNestingLevel(unsigned Level) { NestingLevel = Level; }

  /// \brief Set the number of exits in loop
  void setNumberOfExits(unsigned NumExits) { NumberOfExits = NumExits; }

  /// \brief Set IsDoWhileLoop
  void setIsDoWhileLoop(bool IsDoW) { IsDoWhile = IsDoW; }

  /// \brief Set the loop nesting level
  void setIsInnerMost(bool InnerM) { IsInnerMost = InnerM; }

  /// \brief Sets loop as a vectorization candidate.
  void setVectorCandidate(bool VectorCand) { IsVectorCandidate = VectorCand; }

  /// \brief Sets loop as an auto-vectorization candidate.
  void setAutoVectorCandidate(bool AutoVectorCand) {
    IsAutoVectorCandidate = AutoVectorCand;
  }

  /// \brief Sets loop as an explicit-vectorization candidate.
  void setExplicitVectorCandidate(bool ExplicitVectorCand) {
    IsExplicitVectorCandidate = ExplicitVectorCand;
  }

  /// \brief Moves preheader nodes before the loop. Ztt is extracted first, if
  /// present.
  // TODO
  void extractPreheader();

  /// \brief Moves postexit nodes after the loop. Ztt is extracted first, if
  /// present.
  // TODO
  void extractPostexit();

  /// \brief Moves preheader nodes before the loop and postexit nodes after the
  /// loop. Ztt is extracted first, if present.
  // TODO
  void extractPreheaderAndPostexit();

  /// \brief Sets the loop's zero trip test.
  void setZeroTripTest(AVRIf *IfZtt) { Ztt = IfZtt; }

  /// \brief Removes this loops ZTT and returns the AVRIf if ztt exists,
  /// otherwise
  /// returns nullptr.
  AVRIf *removeZeroTripTest();

  /// Only this utility class should be use to modify/delete AVR nodes.
  friend class AVRUtils;

public:
  /// \brief Set WRNVecLoopNode
  void setWrnVecLoopNode(WRNVecLoopNode *WRN) { WrnLoopNode = WRN; }

  /// \brief Returns WRNVecLoop node.
  WRNVecLoopNode *getWrnVecLoopNode() { return WrnLoopNode; }

  /// \brief Returns Loop nesting level
  unsigned getNestingLevel() const { return NestingLevel; }

  /// \brief Returns Loop Ztt.
  AVRIf *getZeroTripTest() { return Ztt; }

  /// \brief Returns true if loop is a do loop.
  bool isDoLoop() const { return (!IsDoWhile && (NumberOfExits == 1)); }

  /// \brief Returns true is loop is of form Do-While.
  bool isDoWhileLoop() const { return IsDoWhile; }

  /// \brief Returns the number of exits of the loop.
  unsigned getNumExits() const { return NumberOfExits; }

  /// \brief Returns true is loop is innermost
  bool isInnerMost() const { return IsInnerMost; }

  /// \brief Returns true is loop is a candidate for vectorization.
  bool isVectorCandidate() const { return IsVectorCandidate; }

  /// \brief Returns true is loop is a candidate for auto-vectorization.
  bool isAutoVectorCandidate() const { return IsAutoVectorCandidate; }

  /// \brief Returns true is loop is a candidate for explicit vectorization.
  bool isExplicitVectorCandidate() const { return IsExplicitVectorCandidate; }

  /// \brief Returns true if loop contains a zero trip test.
  bool hasZeroTripTest() const { return Ztt != nullptr; }

  /// Preheader iterator methods

  pre_iterator pre_begin() { return PreheaderChildren.begin(); }
  const_pre_iterator pre_begin() const { return PreheaderChildren.begin(); }
  pre_iterator pre_end() { return PreheaderChildren.end(); }
  const_pre_iterator pre_end() const { return PreheaderChildren.end(); }

  reverse_pre_iterator pre_rbegin() { return PreheaderChildren.rbegin(); }
  const_reverse_pre_iterator pre_rbegin() const {
    return PreheaderChildren.rbegin();
  }
  reverse_pre_iterator pre_rend() { return PreheaderChildren.rend(); }
  const_reverse_pre_iterator pre_rend() const {
    return PreheaderChildren.rend();
  }

  /// Preheader access methods

  /// \brief Returns the first preheader node if it exists, otherwise returns
  /// null.
  AVR *getFirstPreheaderNode();
  const AVR *getFirstPreheaderNode() const {
    return const_cast<AVRLoop *>(this)->getFirstPreheaderNode();
  }

  /// \brief Returns the last preheader node if it exists, otherwise returns
  /// null.
  AVR *getLastPreheaderNode();
  const AVR *getLastPreheaderNode() const {
    return const_cast<AVRLoop *>(this)->getLastPreheaderNode();
  }

  /// \brief Returns the number of preheader nodes.
  unsigned getNumPreheader() const {
    return std::distance(pre_begin(), pre_end());
  }

  /// \brief Returns true if preheader is not empty.
  bool hasPreheader() const { return !PreheaderChildren.empty(); }

  /// \brief Returns true if PreheaderChildren contains Node.
  bool isPreheaderChild(AVR *Node) const;

  /// Loop Children Iterators

  child_iterator child_begin() { return Children.begin(); }
  const_child_iterator child_begin() const { return Children.begin(); }
  reverse_child_iterator child_rbegin() { return Children.rbegin(); }
  const_reverse_child_iterator child_rbegin() const {
    return Children.rbegin();
  }

  child_iterator child_end() { return Children.end(); }
  const_child_iterator child_end() const { return Children.end(); }
  reverse_child_iterator child_rend() { return Children.rend(); }
  const_reverse_child_iterator child_rend() const { return Children.rend(); }

  typedef iterator_range<child_iterator> LoopNodesRange;

  LoopNodesRange nodes() { return LoopNodesRange(child_begin(), child_end()); }

  /// Children access Methods

  /// \brief Returns the first child if it exists, otherwise returns null.
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

  /// \brief Returns true if Children contains Node.
  bool isChild(AVR *Node) const;

  /// Postexit iterator methods

  post_iterator post_begin() { return PostexitChildren.begin(); }
  const_post_iterator post_begin() const { return PostexitChildren.begin(); }
  post_iterator post_end() { return PostexitChildren.end(); }
  const_post_iterator post_end() const { return PostexitChildren.end(); }

  reverse_post_iterator post_rbegin() { return PostexitChildren.rbegin(); }
  const_reverse_post_iterator post_rbegin() const {
    return PostexitChildren.rbegin();
  }
  reverse_post_iterator post_rend() { return PostexitChildren.rend(); }
  const_reverse_post_iterator post_rend() const {
    return PostexitChildren.rend();
  }

  /// Postexit access methods

  /// \brief Returns the first postexit node if it exists, otherwise returns
  /// null.
  AVR *getFirstPostexitNode();
  const AVR *getFirstPostexitNode() const {
    return const_cast<AVRLoop *>(this)->getFirstPostexitNode();
  }
  /// \brief Returns the last postexit node if it exists, otherwise returns
  /// null.
  AVR *getLastPostexitNode();
  const AVR *getLastPostexitNode() const {
    return const_cast<AVRLoop *>(this)->getLastPostexitNode();
  }

  /// \brief Returns the number of postexit nodes.
  unsigned getNumPostexit() const {
    return std::distance(post_begin(), post_end());
  }
  /// \brief Returns true if postexit is not empty.
  bool hasPostexit() const { return !PostexitChildren.empty(); }

  /// \brief Returns true if Children contains Node.
  bool isPostexitChild(AVR *Node) const;

  /// \brief Method for supporting type inquiry through isa, cast, and dyn_cast.
  static bool classof(const AVR *Node) {
    return (Node->getAVRID() >= AVR::AVRLoopNode &&
            Node->getAVRID() < AVR::AVRLoopLastNode);
  }

  /// \brief Prints the AvrLoop node.
  void print(formatted_raw_ostream &OS, unsigned Depth,
             VerbosityLevel VLevel) const override;

  /// \brief Shallow-prints the AvrLoop node.
  void shallowPrint(formatted_raw_ostream &OS) const override;

  /// \brief Returns a constant StringRef for the type name of this node.
  virtual StringRef getAvrTypeName() const override;

  /// \brief Returns the value name of this node.
  virtual std::string getAvrValueName() const override;

  AVRLoop *clone() const override;

  /// \brief Code generation for AVR loop.
  void codeGen() override;
};

} // End VPO Vectorizer Namespace
} // End LLVM Namespace

#endif // LLVM_ANALYSIS_VPO_AVR_LOOP_H
