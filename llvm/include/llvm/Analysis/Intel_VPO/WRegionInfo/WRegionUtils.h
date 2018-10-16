#if INTEL_COLLAB // -*- C++ -*-
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
#if INTEL_CUSTOMIZATION
#include "llvm/Analysis/Intel_LoopAnalysis/Framework/HIRFramework.h"
#endif // INTEL_CUSTOMIZATION
#include "llvm/Analysis/Intel_VPO/WRegionInfo/WRegionCollection.h"
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
  bool forwardVisit(WRContainerImpl *C);

  /// \brief Visits WRegionNodes in the WRContainerTy in the backward direction.
  /// Returns true to indicate that early termination has occurred.
  bool backwardVisit(WRContainerImpl *C);
};

template <typename WV> bool WRNVisitor<WV>::forwardVisit(WRContainerImpl *C) {
  for (auto I = C->begin(), Next = I, E = C->end(); I != E; I = Next) {
    ++Next;
    if (visit(*I, true))
      return true;
  }
  return false;
}

template <typename WV> bool WRNVisitor<WV>::backwardVisit(WRContainerImpl *C) {
  for (auto RI = C->rbegin(), RNext = RI, RE = C->rend(); RI != RE;
       RI = RNext) {
    ++RNext;
    if (visit(*RI, false))
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
  enum { UnknownLoop, DoWhileLoop, WhileLoop };

public:
  /// \brief Enumeration for types of WRegionNode Graph insert/update
  enum OpType { FirstChild, LastChild, Append, Prepend };

  friend class WRegionNode;

  /// \brief Visit all WRN nodes in the Graph in the forward direction
  template <typename WV>
  static void forwardVisit(WV &Visitor, WRContainerImpl *Graph) {
    WRNVisitor<WV> V(Visitor);
    V.forwardVisit(Graph);
  }

  /// \brief Visit all WRN nodes in the Graph in the backward direction
  template <typename WV>
  static void backwardVisit(WV &Visitor, WRContainerImpl *Graph) {
    WRNVisitor<WV> V(Visitor);
    V.backwardVisit(Graph);
  }

  /// It contains functions which are used to create, modify, and destroy
  /// WRegionNode.

  /// Creation Utilities

  /// \brief Returns a new node derived from WRegionNode node that
  /// matches the construct type based on DirID.
  static WRegionNode *createWRegion(int DirID, BasicBlock *EntryBB,
                                    LoopInfo *LI, unsigned NestingLevel,
                                    bool IsRegionIntrinsic);

#if INTEL_CUSTOMIZATION
  /// \brief Similar to createWRegion, but for HIR vectorizer support
  static WRegionNode *createWRegionHIR(int DirID,
                                       loopopt::HLNode *EntryHLNode,
                                       unsigned NestingLevel);

  /// \brief Update WRGraph from processing intrinsic calls extracted
  /// from HIR.  This is needed to support vectorizer in HIR.
  ///   Call: the call instruction with the intrinsic
  ///   IntrinId: the intrinsic id (eg intel_directive/_qual, etc.)
  ///   WRGraph: points to the WRN graph being built
  ///   S: stack of pending WRN nodes
  ///   H: The HLNode containing the intrinsic call
  static void updateWRGraphFromHIR(IntrinsicInst *Call,
                                   Intrinsic::ID IntrinId,
                                   WRContainerImpl *WRGraph,
                                   WRStack<WRegionNode*> &S,
                                   loopopt::HLNode *H);

  /// \brief Driver routine to build WRGraph based on HIR representation
  static WRContainerImpl *buildWRGraphFromHIR(loopopt::HIRFramework &HIRF);
#endif // INTEL_CUSTOMIZATION

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

  /// Return nullptr if W has no parent of the specified kind.
  static WRegionNode *getParentRegion(WRegionNode *W, unsigned WRegionKind);

  ///\name Clause related Utilities
  /// @{

  /// \brief get the Clause Id for the WRNAtomicKind \p kind.
  static int getClauseIdFromAtomicKind(WRNAtomicKind Kind);

  /// \brief Extract the operands for a list-type clause.
  /// This is called by WRegionNode::handleQualOpndList()
  template <typename ClauseTy>
  static void extractQualOpndList(const Use *Args, unsigned NumArgs,
                                  int ClauseID, ClauseTy &C);
  template <typename ClauseItemTy>
  static void extractQualOpndListNonPod(const Use *Args, unsigned NumArgs,
                                  const ClauseSpecifier &ClauseInfo,
                                  Clause<ClauseItemTy> &C);

  /// \brief Extract operands from a map clause
  static void extractMapOpndList(const Use *Args, unsigned NumArgs,
                                 const ClauseSpecifier &ClauseInfo,
                                 MapClause &C, unsigned MapKind);

  /// \brief Extract operands from a depend clause
  static void extractDependOpndList(const Use *Args, unsigned NumArgs,
                                    const ClauseSpecifier &ClauseInfo,
                                    DependClause &C, bool IsIn);

  /// \brief Extract operands from a linear clause
  static void extractLinearOpndList(const Use *Args, unsigned NumArgs,
                                    LinearClause &C);

  /// \brief Extract operands from a reduction clause
  static void extractReductionOpndList(const Use *Args, unsigned NumArgs,
                                      const ClauseSpecifier &ClauseInfo,
                                      ReductionClause &C, int ReductionKind,
                                      bool IsInreduction);
  /// \brief Extract operands from a schedule clause
  static void extractScheduleOpndList(ScheduleClause & Sched,
                                      const Use *Args,
                                      const ClauseSpecifier &ClauseInfo,
                                      WRNScheduleKind Kind);
  /// @}

  /// \brief Get the induction variable of the OMP loop.
  static PHINode *getOmpCanonicalInductionVariable(Loop *L);

  /// \brief Get the loop lower bound of the OMP loop.
  static Value *getOmpLoopLowerBound(Loop *L);

  /// \brief Get the loop stride of the OMP loop.
  static Value *getOmpLoopStride(Loop *L, bool &IsNeg);

  /// \brief Get the loop upper bound of the OMP loop.
  static Value *getOmpLoopUpperBound(Loop *L);

  /// \brief Get the exit block of the OMP loop.
  static BasicBlock *getOmpExitBlock(Loop *L);

  /// \brief Get the predicate for the bottom test.
  static CmpInst::Predicate getOmpPredicate(Loop *L, bool& IsLeft);

  /// \brief Get the bottom test of the OMP loop.
  static ICmpInst* getOmpLoopBottomTest(Loop *L);

  /// \brief Get the zero trip test of the OMP loop if the zero trip
  ///  test exists.
  static ICmpInst *getOmpLoopZeroTripTest(Loop *L, BasicBlock *EntryBB);

  /// \brief Get the position of the given loop index at
  /// the bottom/zero trip test expression. It returns false if
  /// it cannot find the loop index.
  static bool getLoopIndexPosInPredicate(Value *LoopIndex,
                                         Instruction *CondInst, bool &IsLeft);
  static FirstprivateItem *wrnSeenAsFirstPrivate(WRegionNode *W, Value *V);
  static LastprivateItem *wrnSeenAsLastPrivate(WRegionNode *W, Value *V);
  static MapItem *wrnSeenAsMap(WRegionNode *W, Value *V);

  /// \brief The utility checks whether the given value is used
  /// at the region entry directive.
  static bool usedInRegionEntryDirective(WRegionNode *W, Value *I);

  /// \brief Return true if the value \p V is used in the WRN \p W.
  /// If \p Users is not null, then find all users of \p V in \p W and put them
  /// in \p *Users.
  /// If \p ExcludeDirective is true, then ignore the instructions for which
  /// isIntelDirectiveOrClause() is true.
  ///
  /// Prerequisite: W's BBSet must be populated before calling this util.
  static bool findUsersInRegion(WRegionNode *W, Value *V,
                                SmallVectorImpl<Instruction *> *Users=nullptr,
                                bool ExcludeDirective = true);

  /// \brief The utility to create the loop and update the loopinfo.
  static Loop *createLoop(Loop *L, Loop *PL, LoopInfo *LI);

  /// \brief The utility to add the given BB into the loop.
  static void updateBBForLoop(BasicBlock *BB, Loop *L, Loop *PL, LoopInfo *LI);

  /// \brief Return true if the given loop is do-while loop.
  static bool isDoWhileLoop(Loop *L);

  /// \brief Return true if the given loop is while loop.
  static bool isWhileLoop(Loop *L);

  /// \brief Return the loop type (do-while, while loop or unknown loop) for the
  /// given loop.
  static unsigned getLoopType(Loop *L);

  /// \brief Return true if destructors are needed for privatized variables
  static bool needsDestructors(WRegionNode *W);

  /// Returns \b true if \p W contains a `\#pragma omp cancel` construct
  /// directly inside its region; \b false otherwise.
  static bool hasCancelConstruct(WRegionNode *W);
};

} // End VPO Namespace

} // End LLVM Namespace

#endif // LLVM_ANAYSIS_VPO_WREGIONUTILS_H
#endif // INTEL_COLLAB
