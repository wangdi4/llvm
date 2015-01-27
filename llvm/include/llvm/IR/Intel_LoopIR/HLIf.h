//===--------------- HLIf.h - High level IR node ----------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file defines the HLIf node.
//
//===----------------------------------------------------------------------===//


#ifndef LLVM_IR_INTEL_LOOPIR_HLIF_H
#define LLVM_IR_INTEL_LOOPIR_HLIF_H

#include "llvm/IR/Intel_LoopIR/HLDDNode.h"
#include "llvm/IR/InstrTypes.h"

namespace llvm {

namespace loopopt {

/// \brief High level node representing a conditional branch
class HLIf : public HLDDNode {
public:
  typedef std::vector< CmpInst::Predicate* > PredTy;
  typedef std::vector<unsigned> ConjunctionTy;
  typedef HLContainerTy ChildNodeTy;

  /// Iterators to iterate over then/else children nodes
  typedef ChildNodeTy::iterator then_iterator;
  typedef ChildNodeTy::const_iterator const_then_iterator;
  typedef ChildNodeTy::reverse_iterator reverse_then_iterator;
  typedef ChildNodeTy::const_reverse_iterator const_reverse_then_iterator;

  typedef then_iterator else_iterator;
  typedef const_then_iterator const_else_iterator;
  typedef reverse_then_iterator reverse_else_iterator;
  typedef const_reverse_then_iterator const_reverse_else_iterator;

private:
  PredTy Preds;
  ConjunctionTy Conjunctions;
  ChildNodeTy ThenChildren;
  ChildNodeTy ElseChildren;

protected:
  explicit HLIf(HLNode* Par);
  ~HLIf() { }

  friend class HLNodeUtils;

  HLIf* clone_impl() const override;

public:
  /// \brief Returns the underlying type of if.
  Type* getLLVMType() const;
  /// \brief Returns the vector of predictes associated with this if.
  PredTy& getPredicates();
  /// \brief Returns the vector of conjunctions combining the predicates
  /// of this if.
  ConjunctionTy& getConjunctions();


  /// Children iterator methods
  then_iterator               then_begin()     { return ThenChildren.begin(); }
  const_then_iterator         then_begin() const { return ThenChildren.begin();}
  then_iterator               then_end()          { return ThenChildren.end(); }
  const_then_iterator         then_end()   const { return ThenChildren.end(); }

  reverse_then_iterator       then_rbegin()    { return ThenChildren.rbegin(); }
  const_reverse_then_iterator then_rbegin() const {
    return ThenChildren.rbegin();
  }
  reverse_then_iterator       then_rend()        { return ThenChildren.rend(); }
  const_reverse_then_iterator then_rend()  const { return ThenChildren.rend(); }


  else_iterator               else_begin()      { return ElseChildren.begin(); }
  const_else_iterator         else_begin()  const{ return ElseChildren.begin();}
  else_iterator               else_end()          { return ElseChildren.end(); }
  const_else_iterator         else_end()    const { return ElseChildren.end(); }

  reverse_else_iterator       else_rbegin()    { return ElseChildren.rbegin(); }
  const_reverse_else_iterator else_rbegin() const {
    return ElseChildren.rbegin();
  }
  reverse_else_iterator       else_rend()        { return ElseChildren.rend(); }
  const_reverse_else_iterator else_rend()  const { return ElseChildren.rend(); }

  /// Children acess methods
  size_t         numThenChildren() const   { return ThenChildren.size();  }
  bool           isThenEmpty() const  { return ThenChildren.empty(); }

  size_t         numElseChildren() const   { return ElseChildren.size();  }
  bool           isElseEmpty() const  { return ElseChildren.empty(); }

  /// \brief Method for supporting type inquiry through isa, cast, and dyn_cast.
  static bool classof(const HLNode* Node) {
    return Node->getHLNodeID() == HLNode::HLIfVal;
  }

};


} // End namespace loopopt

} // End namespace llvm

#endif
