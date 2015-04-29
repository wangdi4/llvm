//===----------- VectorAVRFunction.h - AVR Loop Node-------------*- C++ -*-===//
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

#ifndef LLVM_ANALYSIS_VPO_AVR_FUNCTION_H
#define LLVM_ANALYSIS_VPO_AVR_LUNCTION_H

#include "llvm/IR/Function.h"
#include "llvm/Analysis/VPO/Vecopt/AVR/VPOAvr.h"

namespace intel { // VPO Vectorizer Namespace

using namespace llvm;

/// \brief AVR Function node abstract vector representation
///
/// An AVRFunction node represents a function found in LLVM IR or LoopOpt HIR.
class AVRFunction : public AVR {
public:

  /// List Container of AVRFunction's children nodes.
  typedef AVRContainerTy ChildrenTy;
  typedef iplist<BasicBlock> BasicBlockListTy;


  /// Iterators to iterate over children nodes
  typedef ChildrenTy::iterator child_iterator;
  typedef ChildrenTy::const_iterator const_child_iterator;
  typedef ChildrenTy::reverse_iterator reverse_child_iterator;
  typedef ChildrenTy::const_reverse_iterator const_reverse_child_iterator;

private:

  /// Pointer to orignial LLVM Loop
  Function *OriginalFunction;
  /// Function is void.
  bool IsVoidFunction;
  /// Function is a candidate for explicit vectorization
  bool IsExplicitVectorCandidate;

  // To Do: Parameters
  // To Do: Returns
  

public:

  AVRFunction(Function *OrigF);
  ~AVRFunction();

  // TODO: Make this private
  ChildrenTy Children;

  /// \brief Sets pointer to original LLVM function.
  void setOriginalFunction(Function *OrigF) { OriginalFunction = OrigF; }

  /// \brief Sets this function as void.
  void setIsVoidFunction(bool IsVoid) { IsVoidFunction = IsVoid; }

  /// \brief Sets this function as explicit vectorization candidate
  void setVectorCandidate(bool VectorF) { IsExplicitVectorCandidate = VectorF; }

  /// \brief Returns pointer to original LLVM function.
  Function *getOrigFunction() const { return OriginalFunction; }

  /// \brief Returns the entry(first) bblock of this function.
  BasicBlock *getEntryBasicBlock() const;

  /// \brief Returns the first bblock of this function.
  BasicBlock *getFirstBasicBlock() const;

  /// \brief Returns the exit(last) bblock of this function.
  BasicBlock *getLastBasicBlock() const;

  BasicBlockListTy *getBasicBlockList() const{
    return &OriginalFunction->getBasicBlockList();
  }
  /// \brief Returns true if function is void.
  bool isVoidFunction() const { return IsVoidFunction; }

  /// \brief Returns true if function is void.
  bool isVectorCandidate() const { return IsExplicitVectorCandidate; }

  /// \brief Returns true is function is an explicit vector candidate
  bool isExplicitVectorCandidate() const { return isVectorCandidate(); }

  /// \brief Returns true is function is an explicit vector candidate
  bool isAutoVectorCandidate() const { return false; }

  AVRFunction *clone() const override;

  /// Children Iterators
  child_iterator child_begin() {return Children.begin();}
  const_child_iterator child_begin() const { return Children.begin(); }
  child_iterator child_end() { return Children.end(); }
  const_child_iterator child_end() const { return Children.end(); }

  /// Children Methods
  AVR *getFirstChild();

  /// \brief Returns the first child if it exists, otherwise returns null.
  const AVR *getFirstChild() const {
    return const_cast<AVRFunction *>(this)->getFirstChild();
  }

  /// \brief Returns the last child if it exists, otherwise returns null.
  AVR *getLastChild();

  /// \bried TODO
  const AVR *getLastChild() const {
    return const_cast<AVRFunction *>(this)->getLastChild();
  }

  /// \brief Returns the number of children.
  unsigned getNumChildren() const { return Children.size(); }

  /// \brief Returns true if it has children.
  bool hasChildren() const { return !Children.empty(); }

  /// \brief Method for supporting type inquiry through isa, cast, and dyn_cast.
  static bool classof(const AVR *Node) {
    return Node->getAVRID() == AVR::AVRFunctionNode;
  }

  /// \brief Print Method for AVR Function.
  void print() const override;

  /// \brief Dump AVR Function Node and its children.
  void dump() const override;

};

}  // End VPO Vectorizer namespace

#endif  // LLVM_ANALYSIS_VPO_AVR_FUNCTION_H
