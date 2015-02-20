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
  /// Contains preheader, child and postexit nodes, in that order.
  /// Having a single container allows for more efficient and cleaner 
  /// implementation of insert(Before/After) and remove(Before/After).
  ChildNodeTy Children;
  /// Iterator pointing to beginning of children nodes.
  ChildNodeTy::iterator ChildBegin;
  /// Iterator pointing to beginning of postexit nodes.
  ChildNodeTy::iterator PostexitBegin;
  bool IsDoWhile;
  unsigned NumExits;
  unsigned NestingLevel;
  bool IsInnermost;

protected:
  HLLoop(HLIf* ZttIf, bool IsDoWh, unsigned NumEx);

  ~HLLoop() { }

  /// \brief Copy constructor used by cloning.
  HLLoop(const HLLoop &HLLoopObj);

  friend class HLNodeUtils;

  void setNestingLevel(unsigned Level) { NestingLevel = Level; }
  void setInnermost(bool IsInnermst) { IsInnermost = IsInnermst; }

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

  int computeNestingLevel() {
    int CNestingLevel=1;
    HLNode *curPar = this->getParent();
    while((curPar)) {
      if(isa<HLLoop>(curPar)) CNestingLevel++;
      curPar = curPar->getParent();
    }
    return CNestingLevel;
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
  bool isDoWhileLoop() const { return IsDoWhile; }
  /// \brief Returns true if this is a do multi-exit loop.
  bool isDoMultiExitLoop() const;
  /// \brief Returns true if this is an unknown loop.
  bool isUnknownLoop() const;
  /// \brief Returns the number of exits of the loop.
  unsigned getNumExits() const { return NumExits; }
  /// \brief Returns the nesting level of the loop.
  unsigned getNestingLevel() const { return NestingLevel; }
  /// \brief Returns true if this is the innermost loop in the loopnest.
  bool isInnermost() const { return IsInnermost; }

  /// Preheader iterator methods
  pre_iterator               pre_begin()        { return Children.begin(); }
  const_pre_iterator         pre_begin()  const { return Children.begin(); }
  pre_iterator               pre_end()          { return ChildBegin; }
  const_pre_iterator         pre_end()    const { return ChildBegin; }

  reverse_pre_iterator       pre_rbegin()       { 
    return ChildNodeTy::reverse_iterator(ChildBegin);
  }
  const_reverse_pre_iterator pre_rbegin() const { 
    return ChildNodeTy::const_reverse_iterator(ChildBegin);
  }

  reverse_pre_iterator       pre_rend()         { return Children.rend(); }
  const_reverse_pre_iterator pre_rend()   const { return Children.rend(); }


  /// Preheader acess methods
  size_t         numPreheader() const  { 
    return std::distance(pre_begin(), pre_end());  
  }
  bool           hasPreheader() const  { 
    return (pre_begin() != pre_end()); 
  }

  /// Postexit iterator methods
  post_iterator               post_begin()        { return PostexitBegin; }
  const_post_iterator         post_begin()  const { return PostexitBegin; }
  post_iterator               post_end()          { return Children.end(); }
  const_post_iterator         post_end()    const { return Children.end(); }

  reverse_post_iterator       post_rbegin()       { return Children.rbegin(); }
  const_reverse_post_iterator post_rbegin() const { return Children.rbegin(); }
  reverse_post_iterator       post_rend()         { 
    return ChildNodeTy::reverse_iterator(PostexitBegin); 
  }
  const_reverse_post_iterator post_rend()   const { 
    return ChildNodeTy::const_reverse_iterator(PostexitBegin); 
  }


  /// Postexit acess methods
  size_t         numPostexit() const  { 
    return std::distance(post_begin(), post_end());  
  }
  bool           hasPostexit() const  { 
    return (post_begin() != post_end()); 
  }


  /// Children iterator methods
  child_iterator               child_begin()        { return pre_end(); }
  const_child_iterator         child_begin()  const { return pre_end(); }
  child_iterator               child_end()          { return post_begin(); }
  const_child_iterator         child_end()    const { return post_begin(); }

  reverse_child_iterator       child_rbegin()       { return post_rend(); }
  const_reverse_child_iterator child_rbegin() const { return post_rend(); }
  reverse_child_iterator       child_rend()         { return pre_rbegin(); }
  const_reverse_child_iterator child_rend()   const { return pre_rbegin(); }


  /// Children acess methods
  size_t         numChildren() const   { 
    return std::distance(child_begin(), child_end());  
  }
  bool           hasChildren() const  { 
    return (child_begin() != child_end()); 
  }

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
  ///   * Data members that depend on where the cloned loop lives in HIR (like
  ///     parent, nesting level) are not copied. They will be updated by HLNode
  ///     insertion/removal utilities.
  HLLoop* clone() const override;
};


} // End namespace loopopt

} // End namespace llvm

#endif
