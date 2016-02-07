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
//   VPOAvrIf.h -- Defines the Abstract Vector Representation (AVR) if node.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_ANALYSIS_VPO_AVR_IF_H
#define LLVM_ANALYSIS_VPO_AVR_IF_H

#include "llvm/Analysis/Intel_VPO/Vecopt/VPOAvr.h"
#include "llvm/Analysis/Intel_VPO/Vecopt/VPOAvrStmt.h"

namespace llvm { // LLVM Namespace
namespace vpo {  // VPO Vectorizer Namespace

/// \brief If node abstract vector representation. An AVRIf node represents an
/// if-statement found in LLVM IR or LoopOpt HIR.
class AVRIf : public AVR {
public:

  typedef AVRContainerTy IfChildrenTy; 

  /// Iterators to iterate over then/else children nodes
  typedef IfChildrenTy::iterator then_iterator;
  typedef IfChildrenTy::const_iterator const_then_iterator;
  typedef IfChildrenTy::reverse_iterator reverse_then_iterator;
  typedef IfChildrenTy::const_reverse_iterator const_reverse_then_iterator;

  typedef then_iterator else_iterator;
  typedef const_then_iterator const_else_iterator;
  typedef reverse_then_iterator reverse_else_iterator;
  typedef const_reverse_then_iterator const_reverse_else_iterator;

private:

  /// ThenChildren - Container that holds IF children in 'Then' branch.
  IfChildrenTy ThenChildren;

  /// ElseChildren - Container that holds IF children in 'Else' branch.
  IfChildrenTy ElseChildren;

protected:

  AVRIf(unsigned SCID);
  virtual ~AVRIf() override {}

  /// \brief Sets up state object.
  void initialize();

  /// Only this utility class should be used to modify/delete AVR nodes.
  friend class AVRUtils;

public:

  /// Children iterator methods
  then_iterator then_begin() { return ThenChildren.begin(); }
  const_then_iterator then_begin() const { return ThenChildren.begin(); }
  then_iterator then_end() { return ThenChildren.end(); }
  const_then_iterator then_end() const { return ThenChildren.end(); }

  else_iterator else_begin() { return ElseChildren.begin(); }
  const_else_iterator else_begin() const { return ElseChildren.begin(); }
  else_iterator else_end() { return ElseChildren.end(); }
  const_else_iterator else_end() const { return ElseChildren.end(); }

  /// Children acess methods

  //---------- Then Children----------//

  /// \brief Returns the first then child if it exists, otherwise
  /// returns null.
  AVR *getFirstThenChild();

  const AVR *getFirstThenChild() const {
    return const_cast<AVRIf *>(this)->getFirstThenChild();
  }
  /// \brief Returns the last then child if it exists, otherwise
  /// returns null.
  AVR *getLastThenChild();
  const AVR *getLastThenChild() const {
    return const_cast<AVRIf *>(this)->getLastThenChild();
  }

  /// \brief Returns the number of then children.
  unsigned getNumThenChildren() const {
    return ThenChildren.size();
  }
  /// \brief Returns true if it has then children.
  bool hasThenChildren() const { return !ThenChildren.empty(); }

  /// \brief Returns the first else child if it exists, otherwise
  /// returns null.
  AVR *getFirstElseChild();
  const AVR *getFirstElseChild() const {
    return const_cast<AVRIf *>(this)->getFirstElseChild();
  }

  //---------- Else Children----------//

  /// \brief Returns the last else child if it exists, otherwise
  /// returns null.
  AVR *getLastElseChild();
  const AVR *getLastElseChild() const {
    return const_cast<AVRIf *>(this)->getLastElseChild();
  }

  /// \brief Returns the number of else children.
  unsigned getNumElseChildren() const {
    return ElseChildren.size();
  }
  /// \brief Returns true if it has else children.
  bool hasElseChildren() const { return !ElseChildren.empty(); }

  AVRIf *clone() const override;
 
  /// \brief Method for supporting type inquiry through isa, cast, and dyn_cast.
  static bool classof(const AVR *Node) {
    return (Node->getAVRID() >= AVR::AVRIfNode &&
            Node->getAVRID() < AVR::AVRIfLastNode);
  }

  /// \brief Prints the AvrIf node.
  void print(formatted_raw_ostream &OS, unsigned Depth,
	     VerbosityLevel VLevel) const override;

  /// \brief Returns a constant StringRef for the type name of this node.
  virtual StringRef getAvrTypeName() const override;

  /// \brief Returns the value name of this node.
  virtual std::string getAvrValueName() const = 0;

};

} // End VPO Vectorizer Namespace
} // End LLVM Namespace

#endif  // LLVM_ANALYSIS_VPO_AVR_IF_H
