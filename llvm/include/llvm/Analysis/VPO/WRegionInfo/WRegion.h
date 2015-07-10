//===----------------- WRegion.h - W-Region node ----------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file defines the WRegion node.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_ANALYSIS_VPO_WREGION_H
#define LLVM_ANALYSIS_VPO_WREGION_H

#include "llvm/Analysis/VPO/WRegionInfo/WRegionNode.h"
//#include "llvm/Analysis/VPO/WRegionInfo/WRegionCollection.h"
#include <set>
#include <iterator>

namespace llvm {

class BasicBlock;
class LoopInfo;

namespace vpo {

typedef std::set<const BasicBlock *> WRegionBSetTy;
typedef std::vector<WRegion *>       WRegionsTy;

/// \brief Top level W-Region node in W-Region Graph 
///
/// A High level region is a section of CFG which can be analyzed and
/// transformed independently of other sections of CFG. 
class WRegion : public WRegionNode {
public:
  /// List of children nodes inside region
  typedef WRContainerTy ChildNodeTy;

  /// Iterators to iterate over children nodes
  typedef ChildNodeTy::iterator wrn_child_iterator;
  typedef ChildNodeTy::const_iterator wrn_const_child_iterator;
  typedef ChildNodeTy::reverse_iterator wrn_reverse_child_iterator;
  typedef ChildNodeTy::const_reverse_iterator wrn_const_reverse_child_iterator;

private:
  BasicBlock    *EntryBBlock;
  BasicBlock    *ExitBBlock;
  const WRegionBSetTy &BBlockSet;
  const LoopInfo *LI;


public:

  /// Children list of WRegions
  ChildNodeTy   Children;

  /// \brief Sets the entry(first) bblock of this region.
  void setEntryBBlock(BasicBlock *EntryBB) { EntryBBlock = EntryBB; }

  /// \brief Sets the exit(last) bblock of this region.
  void setExitBBlock(BasicBlock *ExitBB) { ExitBBlock = ExitBB; }

  void addChildWRegion(WRegion *Parent, WRegion *Child) const {
    //Children.pack_back(Child);
    //const_cast<WRegion *>(this)->getLastChild();
  }

  WRegion(BasicBlock *EntryBB, BasicBlock *ExitBB, const WRegionBSetTy &BBlockSet,
          const LoopInfo *LoopI);

  /// WRegionNodes are destroyed in bulk using 
  /// WRegionUtils::destroyAll(). iplist<> tries to
  /// access and destroy the nodes if we don't clear them out here.
  ~WRegion() { Children.clearAndLeakNodesUnsafely(); }

  /// WRegions - Vector of WRegions.
  WRegionsTy    WRegions;

  /// \brief Prints WRegion.
  virtual void print(formatted_raw_ostream &OS, unsigned Depth) const override;

  /// \brief Returns the set of basic blocks which constitute this region.
  const WRegionBSetTy &getBBlockSet() const {
    return BBlockSet;
  }

  /// \brief Returns the entry(first) bblock of this region.
  BasicBlock *getEntryBBlock() const { return EntryBBlock; }

  /// \brief Returns the exit(last) bblock of this region.
  BasicBlock *getExitBBlock() const { return ExitBBlock; }

  /// \brief Returns the predecessor bblock of this region.
  BasicBlock *getPredBBlock() const;

  /// \brief Returns the successor bblock of this region.
  BasicBlock *getSuccBBlock() const;

  /// Children iterator methods
  wrn_child_iterator wrn_child_begin() { return Children.begin(); }
  wrn_const_child_iterator wrn_child_begin() const { return Children.begin(); }
  wrn_child_iterator wrn_child_end() { return Children.end(); }
  wrn_const_child_iterator wrn_child_end() const { return Children.end(); }

  wrn_reverse_child_iterator wrn_child_rbegin() { return Children.rbegin(); }
  wrn_const_reverse_child_iterator wrn_child_rbegin() const {
    return Children.rbegin();
  }

  wrn_reverse_child_iterator wrn_child_rend() { return Children.rend(); }
  wrn_const_reverse_child_iterator wrn_child_rend() const { 
    return Children.rend(); 
  }

  /// Children acess methods

  /// \brief Returns the first child if it exists, otherwise returns null.
  WRegionNode *getFirstChild();

  const WRegionNode *getFirstChild() const {
    return const_cast<WRegion *>(this)->getFirstChild();
  }
  /// \brief Returns the last child if it exists, otherwise returns null.
  WRegionNode *getLastChild();

  const WRegionNode *getLastChild() const {
    return const_cast<WRegion *>(this)->getLastChild();
  }

  /// \brief Returns the number of children.
  unsigned getNumChildren() const { return Children.size(); }

  /// \brief Returns the Loop Info for this Work Region
  const LoopInfo * getLoopInfo() const { return LI; }

  /// \brief Returns true if it has children.
  bool hasChildren() const { return !Children.empty(); }

  /// \brief Method for supporting type inquiry through isa, cast, and dyn_cast.
  static bool classof(const WRegionNode *W) {
    return W->getWRegionKindID() == WRegionNode::VPO_PAR_REGION;
  }

  /// clone() - Do not support Cloning of Region.
  /// This is LLVM Unreachable code.
  WRegion *clone() const override;
};

} // End namespace vpo

} // End namespace llvm

#endif
