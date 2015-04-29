//===--------------- VectorAVRLoop.h - AVR Loop Node-------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file defines the Vectorizer's AVR Loop node.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_ANALYSIS_VPO_AVR_LOOP_H
#define LLVM_ANALYSIS_VPO_AVR_LOOP_H

#include "llvm/Analysis/VPO/Vecopt/AVR/VPOAvr.h"


namespace intel { // Change namespace

using namespace llvm;

class Loop;
class WRN; // Replace With Corresponding PAR WRN

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
  const Loop *OrigLoop;
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

  // To Do: PHI Node

public:
  AVRLoop(const Loop *OrigLLVMLoop, bool IsDoWhileLoop);
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


  // Eric : Do we want the ability to clone?
  AVRLoop (const AVRLoop &AVROrigLoop);

  /// Pointer to Loop info Contained in WRN
  const WRN *WRNInfo;
  /// \brief Returns Original LLVM Loop
  const Loop *getLLVMLoop() const {
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

  void print() const override;
  void dump() const override;
};


}  // End VPO Vectorizer namespace


#endif  // LLVM_ANALYSIS_VPO_AVR_LOOP_H
