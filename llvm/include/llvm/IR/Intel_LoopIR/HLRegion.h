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
#include "llvm/Analysis/Intel_LoopAnalysis/RegionIdentification.h"
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
  typedef HLContainerTy ChildNodeTy;

  /// Iterators to iterate over children nodes
  typedef ChildNodeTy::iterator child_iterator;
  typedef ChildNodeTy::const_iterator const_child_iterator;
  typedef ChildNodeTy::reverse_iterator reverse_child_iterator;
  typedef ChildNodeTy::const_reverse_iterator const_reverse_child_iterator;

protected:
  HLRegion(RegionIdentification::RegionBBlocksTy &OrigBB, BasicBlock *EntryBB,
           BasicBlock *ExitBB);

  /// HLNodes are destroyed in bulk using HLNodeUtils::destroyAll(). iplist<>
  /// tries to
  /// access and destroy the nodes if we don't clear them out here.
  ~HLRegion() { Children.clearAndLeakNodesUnsafely(); }

  friend class HLNodeUtils;
  friend class HIRCreation;

  /// \brief Sets the entry(first) bblock of this region.
  void setEntryBBlock(BasicBlock *EntryBB) { EntryBBlock = EntryBB; }

  /// \brief Sets the exit(last) bblock of this region.
  void setExitBBlock(BasicBlock *ExitBB) { ExitBBlock = ExitBB; }

private:
  RegionIdentification::RegionBBlocksTy &OrigBBlocks;
  BasicBlock *EntryBBlock;
  BasicBlock *ExitBBlock;

  bool GenCode;
  ChildNodeTy Children;

public:
  /// \brief Prints HLRegion.
  virtual void print(formatted_raw_ostream &OS, unsigned Depth) const override;

  /// \brief Returns the set of basic blocks which constitute this region.
  const RegionIdentification::RegionBBlocksTy &getOrigBBlocks() const {
    return OrigBBlocks;
  }

  /// \brief Returns the entry(first) bblock of this region.
  BasicBlock *getEntryBBlock() const { return EntryBBlock; }
  /// \brief Returns the exit(last) bblock of this region.
  BasicBlock *getExitBBlock() const { return ExitBBlock; }

  /// \brief Returns the predecessor bblock of this region.
  BasicBlock *getPredBBlock() const;
  /// \brief Returns the successor bblock of this region.
  BasicBlock *getSuccBBlock() const;

  /// \brief Returns true if we need to generate code for this region.
  bool shouldGenCode() const { return GenCode; }
  void setGenCode(bool GC = true) { GenCode = GC; }

  /// Children iterator methods
  child_iterator child_begin() { return Children.begin(); }
  const_child_iterator child_begin() const { return Children.begin(); }
  child_iterator child_end() { return Children.end(); }
  const_child_iterator child_end() const { return Children.end(); }

  reverse_child_iterator child_rbegin() { return Children.rbegin(); }
  const_reverse_child_iterator child_rbegin() const {
    return Children.rbegin();
  }
  reverse_child_iterator child_rend() { return Children.rend(); }
  const_reverse_child_iterator child_rend() const { return Children.rend(); }

  /// Children acess methods

  /// \brief Returns the first child if it exists, otherwise returns null.
  HLNode *getFirstChild();
  const HLNode *getFirstChild() const {
    return const_cast<HLRegion *>(this)->getFirstChild();
  }
  /// \brief Returns the last child if it exists, otherwise returns null.
  HLNode *getLastChild();
  const HLNode *getLastChild() const {
    return const_cast<HLRegion *>(this)->getLastChild();
  }

  /// \brief Returns the number of children.
  unsigned getNumChildren() const { return Children.size(); }
  /// \brief Returns true if it has children.
  bool hasChildren() const { return !Children.empty(); }

  /// \brief Method for supporting type inquiry through isa, cast, and dyn_cast.
  static bool classof(const HLNode *Node) {
    return Node->getHLNodeID() == HLNode::HLRegionVal;
  }

  /// clone() - Do not support Cloning of Region.
  /// This is LLVM Unreachable code.
  HLRegion *clone() const override;
};

} // End namespace loopopt

} // End namespace llvm

#endif
