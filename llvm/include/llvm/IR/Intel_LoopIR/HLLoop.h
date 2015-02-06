//===------------- HLLoop.h - High level IR node ----------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file defines the HLLoop node.
//
//===----------------------------------------------------------------------===//


#ifndef LLVM_IR_INTEL_LOOPIR_HLLOOP_H
#define LLVM_IR_INTEL_LOOPIR_HLLOOP_H

#include "llvm/IR/Intel_LoopIR/HLDDNode.h"
#include "llvm/IR/Intel_LoopIR/HLIf.h"
#include "llvm/IR/InstrTypes.h"

namespace llvm {

namespace loopopt {

class CanonExpr;

/// \brief High level node representing a loop
class HLLoop : public HLDDNode {
public:
  typedef HLIf::PredTy ZttPredTy;
  typedef HLIf::ConjunctionTy ZttConjunctionTy;
  typedef HLContainerTy ChildNodeTy;
  typedef ChildNodeTy PreheaderTy;
  typedef ChildNodeTy PostexitTy;

  /// Iterator to iterate over ZTT DDRefs
  typedef HLIf::ddref_iterator ztt_ddref_iterator;
  typedef HLIf::const_ddref_iterator const_ztt_ddref_iterator;
  typedef HLIf::reverse_ddref_iterator reverse_ztt_ddref_iterator;
  typedef HLIf::const_reverse_ddref_iterator const_reverse_ztt_ddref_iterator;

  /// Iterators to iterate over children nodes
  typedef ChildNodeTy::iterator child_iterator;
  typedef ChildNodeTy::const_iterator const_child_iterator;
  typedef ChildNodeTy::reverse_iterator reverse_child_iterator;
  typedef ChildNodeTy::const_reverse_iterator const_reverse_child_iterator;

  /// Iterators to iterate over preheader nodes
  typedef child_iterator pre_iterator;
  typedef const_child_iterator const_pre_iterator;
  typedef reverse_child_iterator reverse_pre_iterator;
  typedef const_reverse_child_iterator const_reverse_pre_iterator;

  /// Iterators to iterate over postexit nodes
  typedef child_iterator post_iterator;
  typedef const_child_iterator const_post_iterator;
  typedef reverse_child_iterator reverse_post_iterator;
  typedef const_reverse_child_iterator const_reverse_post_iterator;

private:
  HLIf* Ztt;
  PreheaderTy Preheader;
  PostexitTy Postexit;
  ChildNodeTy Children;
  bool isDoWhile;
  unsigned NumExits;

protected:
  HLLoop(HLNode* Par, HLIf* ZttIf, bool isDoWh, unsigned NumEx);
  ~HLLoop() { }

  /// \brief Copy constructor used by cloning.
  HLLoop(const HLLoop &HLLoopObj);

  friend class HLNodeUtils;

public:

  /// \brief Returns true if ztt is present.
  bool hasZtt() const { return Ztt != nullptr; }

  /// \brief Returns the Ztt for HLLoop
  const HLIf* getZtt() const { return Ztt; }
  HLIf* getZtt()             { return Ztt; }

  /// \brief Returns the underlying type of ZTT.
  Type* getZttLLVMType() const;
  /// \brief Returns the vector of predictes associated with this ZTT.
  ZttPredTy& getZttPredicates() { 
    assert(hasZtt() && "Ztt is absent");
    return Ztt->getPredicates(); 
  }

  /// \brief Returns the vector of conjunctions combining the predicates
  /// of this ZTT.
  ZttConjunctionTy& getZttConjunctions() { 
    assert(hasZtt() && "Ztt is absent");
    return Ztt->getConjunctions(); 
  }

  /// \brief Returns the DDRef associated with loop lower bound.
  /// The first DDRef is associated with lower bound.
  DDRef* getLowerDDRef() { return DDRefs[0]; }
  /// \brief Returns the DDRef associated with loop trip count.
  /// The second DDRef is associated with trip count.
  DDRef* getTripCountDDRef() { return DDRefs[1]; }
  /// \brief Returns the DDRef associated with loop stride.
  /// The third DDRef is associated with stride.
  DDRef* getStrideDDRef() { return DDRefs[2]; }
  /// \brief Returns the CanonExpr associated with loop lower bound.
  const CanonExpr* getLowerCanonExpr() const;
  /// \brief Returns the CanonExpr associated with loop trip count.
  const CanonExpr* getTripCountCanonExpr() const;
  /// \brief Returns the CanonExpr associated with loop stride.
  const CanonExpr* getStrideCanonExpr() const;
  /// \brief Returns the CanonExpr associated with loop upper bound.
  /// Returns a newly allocated CanonExpr as this information is not
  ///  directly stored so use with caution.
  const CanonExpr* getUpperCanonExpr() const;
  /// \brief Returns true if this is a do loop.
  bool isDoLoop() const;
  /// \brief Returns true if this is a do-while loop.
  bool isDoWhileLoop() const { return isDoWhile; }
  /// \brief Returns true if this is a do multi-exit loop.
  bool isDoMultiExitLoop() const;
  /// \brief Returns true if this is an unknown loop.
  bool isUnknownLoop() const;
  /// \brief Returns the number of exits of the loop.
  unsigned getNumExits() const { return NumExits; }

  /// Preheader iterator methods
  pre_iterator               pre_begin()        { return Preheader.begin(); }
  const_pre_iterator         pre_begin()  const { return Preheader.begin(); }
  pre_iterator               pre_end()          { return Preheader.end(); }
  const_pre_iterator         pre_end()    const { return Preheader.end(); }

  reverse_pre_iterator       pre_rbegin()       { return Preheader.rbegin();}
  const_reverse_pre_iterator pre_rbegin() const { return Preheader.rbegin();}
  reverse_pre_iterator       pre_rend()         { return Preheader.rend(); }
  const_reverse_pre_iterator pre_rend()   const { return Preheader.rend(); }


  /// Preheader acess methods
  size_t         numPreheader() const  { return Preheader.size();  }
  bool           hasPreheader() const  { return !Preheader.empty(); }

  /// Postexit iterator methods
  post_iterator               post_begin()        { return Postexit.begin(); }
  const_post_iterator         post_begin()  const { return Postexit.begin(); }
  post_iterator               post_end()          { return Postexit.end(); }
  const_post_iterator         post_end()    const { return Postexit.end(); }

  reverse_post_iterator       post_rbegin()       { return Postexit.rbegin();}
  const_reverse_post_iterator post_rbegin() const { return Postexit.rbegin();}
  reverse_post_iterator       post_rend()         { return Postexit.rend(); }
  const_reverse_post_iterator post_rend()   const { return Postexit.rend(); }


  /// Postexit acess methods
  size_t         numPostexit() const  { return Postexit.size();  }
  bool           hasPostexit() const  { return !Postexit.empty(); }


  /// Children iterator methods
  child_iterator               child_begin()        { return Children.begin(); }
  const_child_iterator         child_begin()  const { return Children.begin(); }
  child_iterator               child_end()          { return Children.end(); }
  const_child_iterator         child_end()    const { return Children.end(); }

  reverse_child_iterator       child_rbegin()       { return Children.rbegin();}
  const_reverse_child_iterator child_rbegin() const { return Children.rbegin();}
  reverse_child_iterator       child_rend()         { return Children.rend(); }
  const_reverse_child_iterator child_rend()   const { return Children.rend(); }


  /// Children acess methods
  size_t         numChildren() const   { return Children.size();  }
  bool           hasChildren() const  { return Children.empty(); }

  /// ZTT DDRef iterator methods
  ztt_ddref_iterator ztt_ddref_begin() {
    assert(hasZtt() && "Ztt is absent");
    return Ztt->ddref_begin();
  }
  const_ztt_ddref_iterator ztt_ddref_begin()  const {
    assert(hasZtt() && "Ztt is absent");
    return Ztt->ddref_begin();
  }
  ztt_ddref_iterator ztt_ddref_end() {
    assert(hasZtt() && "Ztt is absent");
    return Ztt->ddref_end();
  }
  const_ztt_ddref_iterator ztt_ddref_end() const {
    assert(hasZtt() && "Ztt is absent");
    return Ztt->ddref_end();
  }

  reverse_ztt_ddref_iterator ztt_ddref_rbegin() {
    assert(hasZtt() && "Ztt is absent");
    return Ztt->ddref_rbegin();
  }
  const_reverse_ztt_ddref_iterator ztt_ddref_rbegin() const {
    assert(hasZtt() && "Ztt is absent");
    return Ztt->ddref_rbegin();
  }
  reverse_ztt_ddref_iterator ztt_ddref_rend() {
    assert(hasZtt() && "Ztt is absent");
    return Ztt->ddref_rend();
  }
  const_reverse_ztt_ddref_iterator ztt_ddref_rend() const {
    assert(hasZtt() && "Ztt is absent");
    return Ztt->ddref_rend();
  }

  /// \brief Method for supporting type inquiry through isa, cast, and dyn_cast.
  static bool classof(const HLNode* Node) {
    return Node->getHLNodeID() == HLNode::HLLoopVal;
  }

  /// clone() - Create a copy of 'this' HLLoop that is identical in all
  /// ways except the following:
  ///   * The HLLoop has no parent
  HLLoop* clone() const override;
};


} // End namespace loopopt

} // End namespace llvm

#endif
