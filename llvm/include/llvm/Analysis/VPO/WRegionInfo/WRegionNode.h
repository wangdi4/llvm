//===--------- WRegionNode.h - W-Region Graph Node --------------*- C++ -*-===//
//
//   Copyright (C) 2015 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation. and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//
//
//   This file defines the W-Region Graph node.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_ANALYSIS_VPO_WREGIONNODE_H
#define LLVM_ANALYSIS_VPO_WREGIONNODE_H

#include "llvm/ADT/ilist.h"
#include "llvm/ADT/ilist_node.h"

#include "llvm/IR/Dominators.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/FormattedStream.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/Debug.h"

#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/BasicBlock.h"

#include <set>

namespace llvm {

namespace vpo {

typedef std::vector<const BasicBlock *> WRegionBSetTy;

class WRegionNode;

typedef iplist<WRegionNode> WRContainerTy;

/// \brief WRegion Node base class
class WRegionNode : public ilist_node<WRegionNode> {
private:
  /// \brief Make class uncopyable.
  void operator=(const WRegionNode &) = delete;

  /// Unique number associated with this WRegionNode.
  unsigned Number;

  /// ID to differentitate between concrete subclasses.
  const unsigned SubClassID;

  /// Entry and Exit BBs of this WRN
  BasicBlock    *EntryBBlock;
  BasicBlock    *ExitBBlock;

  /// Set containing all the BBs in this WRN
  WRegionBSetTy *BBlockSet;

  /// Enclosing parent of WRegionNode in CFG.
  WRegionNode *Parent;

  /// True if the WRN came from HIR; false otherwise
  bool IsFromHIR;

  /// Counter used for assigning unique numbers to WRegionNodes.
  static unsigned UniqueNum;

  /// \brief Sets the unique number associated with this WRegionNode.
  void setNextNumber() { Number = ++UniqueNum; }

  /// \brief Sets the flag to indicate if WRN came from HIR
  void setIsFromHIR(bool flag) { IsFromHIR = flag; }

  /// \brief Destroys all objects of this class. Should only be
  /// called after code gen.
  static void destroyAll();

protected:

  /// \brief constructors
  WRegionNode(unsigned SCID);
  WRegionNode(WRegionNode *W);

  // copy constructor not needed (at least for now)
  // WRegionNode(const WRegionNode &WRegionNodeObj);

  /// \brief Destroys the object.
  void destroy();

  /// \brief Sets the entry(first) bblock of this region.
  void setEntryBBlock(BasicBlock *EntryBB) { EntryBBlock = EntryBB; }

  /// \brief Sets the exit(last) bblock of this region.
  void setExitBBlock(BasicBlock *ExitBB) { ExitBBlock = ExitBB; }

  /// \brief generates BB set in sub CFG for a given WRegionNode.
  //  Consider moving this to WRegionUtil
  void doPreOrderSubCFGVisit(BasicBlock *BB, BasicBlock *ExitBB,
                             SmallPtrSetImpl<BasicBlock*> *PreOrderTreeVisited);

  /// \brief Populates BBlockSet with BBs in the WRN from EntryBB to ExitBB.
  void populateBBlockSet();

  /// \brief Sets the set of bblocks that constitute this region.
  void setBBlockSet(WRegionBSetTy *BBSet) { BBlockSet = BBSet; }

  /// \brief Sets the graph parent of this WRegionNode.
  void setParent(WRegionNode *P) { Parent = P; }

  /// Only these classes are allowed to create/modify/delete WRegionNode.
  friend class WRegionUtils;
  friend class WRegionCollection;  //temporary

public:
  
#if 0
  // WRegionNodes are destroyed in bulk using
  // WRegionUtils::destroyAll(). iplist<> tries to
  // access and destroy the nodes if we don't clear them out here.
  virtual ~WRegionNode() { Children.clearAndLeakNodesUnsafely(); }
#else
  virtual ~WRegionNode() {}
#endif

  // Virtual Clone Method
  // virtual WRegionNode *clone() const = 0;

  /// \brief Returns the unique number associated with this WRegionNode.
  unsigned getNumber() const { return Number; }

  /// \brief Returns the flag that indicates if WRN came from HIR
  bool getIsFromHIR() const { return IsFromHIR; }

  /// \brief Dumps WRegionNode.
  void dump() const;

  /// \brief Prints WRegionNode.
  //  Actual code from derived class only
  virtual void print(formatted_raw_ostream &OS, unsigned Depth) const = 0;

  /// \brief Prints WRegionNode children.
  void printChildren(formatted_raw_ostream &OS, unsigned Depth) const;

  /// \brief Returns the entry(first) bblock of this region.
  BasicBlock *getEntryBBlock() const { return EntryBBlock; }

  /// \brief Returns the exit(last) bblock of this region.
  BasicBlock *getExitBBlock() const { return ExitBBlock; }

  /// \brief Returns the set of bblocks that constitute this region.
  /// BBlockset must be populated by calling computeBBlockSet() first.
  WRegionBSetTy *getBBlockSet() const { return BBlockSet; }

  /// \brief Returns the predecessor bblock of this region.
  BasicBlock *getPredBBlock() const;

  /// \brief Returns the successor bblock of this region.
  BasicBlock *getSuccBBlock() const;

  /// \brief Returns the immediate enclosing parent of the WRegionNode.
  WRegionNode *getParent() const { return Parent; }

  /// Children acess methods

  /// \brief Returns true if it has children.
  bool hasChildren() const ;

  /// \brief Returns the number of children.
  unsigned getNumChildren() const ;

  /// \brief Return address of the Children container 
  WRContainerTy &getChildren() ;

  /// \brief Returns the first child if it exists, otherwise returns null.
  WRegionNode *getFirstChild();

  /// \brief Returns the last child if it exists, otherwise returns null.
  WRegionNode *getLastChild();

  /// \brief Return an ID for the concrete type of this object.
  ///
  /// This is used to implement the classof checks in LLVM and should't
  /// be used for any other purpose.
  unsigned getWRegionKindID() const { return SubClassID; }

  /// \brief An enumeration to keep track of the concrete subclasses of 
  /// WRegionNode
  enum WRegionNodeKind{
    // These require outlining:
    WRNParallel,
    WRNParallelLoop,
    WRNParallelSections,
    WRNTask,

    // These don't require outlining:
    WRNVecLoop,
    WRNWksLoop,
    WRNWksSections,
    WRNSection,
    WRNSingle,
    WRNMaster,
    WRNAtomic,
    WRNBarrier,
    WRNCancel,
    WRNCritical,
    WRNFlush,
    WRNOrdered,
    WRNTaskgroup
  };
}; // class WRegionNode

} // End vpo namespace


/// \brief traits for iplist<WRegionNode>
///
/// Refer to ilist_traits<Instruction> in BasicBlock.h for explanation.
template <>
struct ilist_traits<vpo::WRegionNode>
    : public ilist_default_traits<vpo::WRegionNode> {

  vpo::WRegionNode *createSentinel() const {
    return static_cast<vpo::WRegionNode *>(&Sentinel);
  }

  static void destroySentinel(vpo::WRegionNode *) {}

  vpo::WRegionNode *provideInitialHead() const { return createSentinel(); }
  vpo::WRegionNode *ensureHead(vpo::WRegionNode *) const {
    return createSentinel();
  }
  static void noteHead(vpo::WRegionNode *, vpo::WRegionNode *) {}

  static vpo::WRegionNode *createWRegionNode(const vpo::WRegionNode &) {
    llvm_unreachable("WRegionNodes should be explicitly created via" 
                     "WRegionUtils class");
    return nullptr;
  }
  static void deleteWRegionNode(vpo::WRegionNode *) {}

private:
  mutable ilist_half_node<vpo::WRegionNode> Sentinel;
};

} // End llvm namespace

#endif
