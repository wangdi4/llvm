//===-- VPOAvrFunction.h ----------------------------------------*- C++ -*-===//
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
/// This file defines the Abstract Vector Representation (AVR)
/// function node.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_ANALYSIS_VPO_AVR_FUNCTION_H
#define LLVM_ANALYSIS_VPO_AVR_FUNCTION_H

#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/CFG.h"

#include "llvm/Analysis/Intel_VPO/Vecopt/VPOAvr.h"

namespace llvm { // LLVM Namespace

class LoopInfo;

namespace vpo { // VPO Vectorizer Namespace

/// \brief AVR Function node abstract vector representation
///
/// An AVRFunction node represents a function found in LLVM IR or LoopOpt HIR.
/// This is a temporary AVR Node type used for stress testing. Loop and function
/// vectorization do not use this node type.
///
class AVRFunction : public AVR {
public:
  /// List Container of AVRFunction's children nodes.
  typedef AVRContainerTy ChildrenTy;
  //  typedef iplist<BasicBlock> BasicBlockListTy;
  typedef SymbolTableList<BasicBlock> BasicBlockListTy;

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

  /// Contains the children AVR nodes of this Function
  ChildrenTy Children;

  /// Loop info for the original LLVM function.
  const LoopInfo *LI;

  // TODO: Parameters
  // TODO: Returns

  /// \brief Sets Loop Info
  void setLoopInfo(const LoopInfo *LpIn) { LI = LpIn; }

protected:
  AVRFunction(Function *OrigF, const LoopInfo *LpIn);
  virtual ~AVRFunction() override {}

  /// \brief Sets pointer to original LLVM function.
  void setOriginalFunction(Function *OrigF) { OriginalFunction = OrigF; }

  /// \brief Sets this function as void.
  void setIsVoidFunction(bool IsVoid) { IsVoidFunction = IsVoid; }

  /// \brief Sets this function as explicit vectorization candidate
  void setVectorCandidate(bool VectorF) { IsExplicitVectorCandidate = VectorF; }

  /// All AVR objects should only be created/destroyed/modified through
  /// utility class.
  friend class AVRUtils;

public:
  /// \brief Returns pointer to original LLVM function.
  Function *getOrigFunction() const { return OriginalFunction; }

  /// \brief Returns the entry(first) bblock of this function.
  BasicBlock *getEntryBBlock() const;

  /// \brief Returns the first bblock of this function.
  BasicBlock *getFirstBBlock() const;

  /// \brief Returns the exit(last) bblock of this function.
  BasicBlock *getLastBBlock() const;

  /// \brief Returns Loop Info.
  const LoopInfo *getLoopInfo() { return LI; }

  BasicBlockListTy *getBasicBlockList() const {
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
  child_iterator child_begin() { return Children.begin(); }
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

  /// \brief Returns const pointer to last child if it exisits.
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

  /// \brief Prints Avr Function
  virtual void print(formatted_raw_ostream &OS, unsigned Depth,
                     VerbosityLevel VLevel) const override;

  /// \brief Returns a constant StringRef for the type name of this node.
  virtual StringRef getAvrTypeName() const override;

  /// \brief Returns the value name of this node.
  virtual std::string getAvrValueName() const override;

  /// \brief Code generation for AVR Function Node and its children.
  // We have this under analysis for now. Clients call this from a
  // transform pass. This will change and will move into transforms
  // once we have AVR visitors. Only does a simple cloning of the
  // underlying LLVM instruction in AVRs for now.
  void codeGen() override;
};

} // End VPO Vectorizer Namespace
} // End LLVM Namespace

#endif // LLVM_ANALYSIS_VPO_AVR_FUNCTION_H
