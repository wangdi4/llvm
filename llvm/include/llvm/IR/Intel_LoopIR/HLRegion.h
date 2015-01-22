//===----------- HLRegion.h - High level IR node ----------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file defines the HLRegion node.
//
//===----------------------------------------------------------------------===//


#ifndef LLVM_IR_INTEL_LOOPIR_HLREGION_H
#define LLVM_IR_INTEL_LOOPIR_HLREGION_H

#include "llvm/IR/Intel_LoopIR/HLNode.h"
#include <set>
#include <iterator>

namespace llvm {

class BasicBlock;

namespace loopopt {

/// \brief Top level node in High level IR
///
/// A High level region is a section of CFG which can be analyzed and
/// transformed independently of other sections of CFG. It typically consists
/// of a single loopnest.
class HLRegion : public HLNode {
public:
  /// List of children nodes inside region
  typedef iplist<HLNode> ChildNodeTy;

  /// Iterators to iterate over children nodes
  typedef ChildNodeTy::iterator child_iterator;
  typedef ChildNodeTy::const_iterator const_child_iterator;
  typedef ChildNodeTy::reverse_iterator reverse_child_iterator;
  typedef ChildNodeTy::const_reverse_iterator const_reverse_child_iterator;

protected:
  HLRegion(std::set< BasicBlock* >& OrigBB, BasicBlock* PredBB,
    BasicBlock* SuccBB);
  ~HLRegion() { }

  friend class HLNodeUtils;

  HLRegion* clone_impl() const override;

private:
  std::set< BasicBlock* >& OrigBBlocks;
  BasicBlock* PredBBlock;
  BasicBlock* SuccBBlock;

  bool GenCode;
  ChildNodeTy Children;

public:
  /// \brief Returns the set of basic blocks which constitute this region.
  const std::set< BasicBlock* >& getOrigBBlocks() const { return OrigBBlocks; }

  /// \brief Returns the predecessor bblock of this region.
  BasicBlock* getPredBBlock() const { return PredBBlock; }
  void setPredBBlock(BasicBlock* PredBB) { PredBBlock = PredBB; }

  /// \brief Returns the successor bblock of this region.
  BasicBlock* getSuccBBlock() const { return SuccBBlock; }
  void setSuccBBlock(BasicBlock* SuccBB) { SuccBBlock = SuccBB; }

  /// \brief Returns true if we need to generate code for this region.
  bool shouldGenCode() const { return GenCode; }
  void setGenCode(bool GC = true) { GenCode = GC; }

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
  bool           empty() const  { return Children.empty(); }

  /// \brief Method for supporting type inquiry through isa, cast, and dyn_cast.
  static bool classof(const HLNode* Node) {
    return Node->getHLNodeID() == HLNode::HLRegionVal;
  }

};

} // End namespace loopopt

} // End namespace llvm

#endif
