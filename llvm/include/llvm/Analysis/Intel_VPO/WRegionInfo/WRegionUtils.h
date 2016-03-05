//===----- WRegionNodeUtils.h - Utilities for WRegionNodeNode class -----*- C++
//-*-===//
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
/// This file defines the utilities for W-Region Node class.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_ANAYSIS_VPO_WREGIONUTILS_H
#define LLVM_ANAYSIS_VPO_WREGIONUTILS_H

#include "llvm/Support/Compiler.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/HLNodeUtils.h"
#include "llvm/Analysis/Intel_VPO/WRegionInfo/WRegion.h"

namespace llvm {

namespace vpo {


/// \brief This class is used to visit WRegionNode or WRContainerTy
/// recursively.
///
/// The forward/backward traversal works even if the current iterator is
/// invalidated (removed/replaced) as the next/prev iterator is saved so it
/// should work for most transformations. Specialized traversals might be needed
/// otherwise.
///
/// Visitor (template class WV) needs to implement:
///
/// 1) void preVisit() function for action done to each WRegionNode
///    before visiting its children.
/// 2) void postVisit() functions for action done to each WRegionNode
///    after visiting its children.
/// 3) bool quitVisit() for early termination of the traversal.
///
/// Sample visitor class:
///
/// struct Visitor {
///   void preVisit(WRegionNode* W)  { errs() << "preVisit node "
///                                           << W->getNumber() << "\n"; }
///   void postVisit(WRegionNode* W) { errs() << "postVisit node "
///                                           << W->getNumber() << "\n"; }
///   bool quitVisit(WRegionNode* W) { return false; }
/// };
///
template <typename WV> class WRNVisitor {
private:
  WV &Visitor;

public:
  WRNVisitor(WV &V) : Visitor(V) {}

  /// \brief Visits WRegionNode recursively.
  /// Returns true to indicate that early termination has occurred.
  /// Forward==true:  call forwardVisit()  to recurse into children.
  /// Forward==false: call backwardVisit() to recurse into children.
  bool visit(WRegionNode *W, bool Forward);

  /// \brief Visits WRegionNodes in the WRContainerTy in the forward direction.
  /// Returns true to indicate that early termination has occurred.
  bool forwardVisit(WRContainerTy *C);

  /// \brief Visits WRegionNodes in the WRContainerTy in the backward direction.
  /// Returns true to indicate that early termination has occurred.
  bool backwardVisit(WRContainerTy *C);
};

template <typename WV> bool WRNVisitor<WV>::forwardVisit(WRContainerTy *C) {
  for (auto I = C->begin(), Next = I, E = C->end(); I != E; I = Next) {
    ++Next;
    if (visit(I, true))
      return true;
  }
  return false;
}

template <typename WV> bool WRNVisitor<WV>::backwardVisit(WRContainerTy *C) {
  for (auto RI = C->rbegin(), RNext = RI, RE = C->rend(); RI != RE;
       RI = RNext) {
    ++RNext;
    if (visit(&(*RI), false))
      return true;
  }
  return false;
}

template <typename WV>
bool WRNVisitor<WV>::visit(WRegionNode *W, bool Forward) {

  // Execute user-defined action on W before visiting W's children
  Visitor.preVisit(W);

  if (Visitor.quitVisit(W))
    return true; // early exit

  if (W->hasChildren()) {
    bool Ret;
    if (Forward) {
      Ret = forwardVisit(&(W->getChildren()));
    } else {
      Ret = backwardVisit(&(W->getChildren()));
    }
    if (Ret)
      return true;
  }

  // Execute user-defined action on W after visiting W's children
  Visitor.postVisit(W);

  /*
  if (Visitor.quitVisit(W))
    return true;  // early exit
  */

  return false;
}

/// \brief This class defines the utilies for WRegionNode nodes.
class WRegionUtils {

  typedef WRContainerTy::iterator WrnIter;

private:
  /// \brief Do not allow instantiation.
  // WRegionNodeUtils() LLVM_DELETED_FUNCTION;

  /// \brief Destroys all nodes
  static void destroyAll();

public:
  /// \brief Enumeration for types of WRegionNode Graph insert/update
  enum OpType { FirstChild, LastChild, Append, Prepend };

  friend class WRegionNode;

  /// \brief Visit all WRN nodes in the Graph in the forward direction
  template <typename WV>
  static void forwardVisit(WV &Visitor, WRContainerTy *Graph) {
    WRNVisitor<WV> V(Visitor);
    V.forwardVisit(Graph);
  }
  
  /// \brief Visit all WRN nodes in the Graph in the backward direction
  template <typename WV>
  static void backwardVisit(WV &Visitor, WRContainerTy *Graph) {
    WRNVisitor<WV> V(Visitor);
    V.backwardVisit(Graph);
  }
  
 

  /// It contains functions which are used to create, modify, and destroy
  /// WRegionNode.

  /// Insertion Utilities -- To Do: Define More Utilities

  /// \brief Standard Insert Utility
  static void insertWRegionNode(WRegionNode *Parent, WrnIter Pos, WrnIter W,
                                OpType Op);

  /// \brief Inserts new wrn as the first child in Parent wrn.
  static void insertFirstChild(WRegionNode *Parent, WrnIter W);

  /// \brief Inserts new wrn as the last child in Parent wrn.
  static void insertLastChild(WRegionNode *Parent, WrnIter W);

  /// \brief Inserts an unlinked WRegion Node after pos in WRegion Node list.
  static void insertAfter(WrnIter Pos, WRegionNode *W);

  /// \brief Inserts an unlinked WRegion Node before pos in WRegion Node list.
  static void insertBefore(WrnIter Pos, WRegionNode *W);

  /// Creation Utilities

  /// \brief Returns a new node derived from WRegionNode node that
  /// matches the construct type based on DirString.
  //  (eg create a WRNParRegion node if DirString is "dir.parallel")
  static WRegionNode *createWRegion(StringRef DirString, BasicBlock *EntryBB,
                                    LoopInfo *LI, unsigned NestingLevel);

  /// \brief Similar to createWRegion, but for HIR vectorizer support
  static WRegionNode *createWRegionHIR(StringRef DirString,
                                       loopopt::HLNode *EntryHLNode,
                                       unsigned NestingLevel);

  /// \brief Update WRGraph from processing intrinsic calls extracted
  /// from HIR.  This is needed to support vectorizer in HIR.
  ///   Call: the call instruction with the intrinsic
  ///   IntrinId: the intrinsic id (eg intel_directive/_qual, etc.)
  ///   WRGraph: points to the WRN graph being built
  ///   CurrentWRN: points to the current pending WRN node. If not null, and
  ///               if the intrinsic is for a clause, then the CurrentWRN is
  ///               updated with the clause info
  ///   H: The HLNode containing the intrinsic call
  ///
  /// If it creates a WRN, then it returns a pointer to it. Otherwise,
  /// it just returns CurrentWRN.
  static WRegionNode *updateWRGraphFromHIR(IntrinsicInst *Call,
                                           Intrinsic::ID IntrinId,
                                           WRContainerTy *WRGraph,
                                           WRegionNode *CurrentWRN,
                                           loopopt::HLNode *H);

  /// \brief Driver routine to build WRGraph based on HIR representation
  static WRContainerTy *buildWRGraphFromHIR();

  /// \brief Extract the operands for a list-type clause.
  /// This is called by WRegionNode::handleQualOpndList()
  template <typename ClauseTy>
  static ClauseTy *extractQualOpndList(IntrinsicInst *Call, ClauseTy *C);
  static ReductionClause *extractReductionOpndList(IntrinsicInst *Call,
                                                   ReductionClause *C,
                                                   int ReductionKind);

  /// Removal Utilities

  /// \brief Destroys the passed in WRegion node.
  static void destroy(WRegionNode *wrn);

  /// \brief Unlinks WRegion node from avr list.
  static void remove(WRegionNode *wrn);

  /// \brief Unlinks wrn node from wrn list and destroys it.
  static void erase(WRegionNode *wrn);

  /// \brief Unlinks [First, Last) from WRegionNode list and destroys them.
  static void erase(WrnIter First, WrnIter Last);

  /// \brief Replaces OldNode by an unlinked NewNode.
  static void replace(WRegionNode *OldW, WRegionNode *NewW);
};


} // End VPO Namespace

} // End LLVM Namespace

#endif // LLVM_ANAYSIS_VPO_WREGIONUTILS_H
