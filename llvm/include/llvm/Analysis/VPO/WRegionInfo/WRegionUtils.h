#if INTEL_COLLAB // -*- C++ -*-
//
// INTEL CONFIDENTIAL
//
// Modifications, Copyright (C) 2021 Intel Corporation
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you ("License"). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.
//
//===--- WRegionUtils.h - Utilities for WRegionNode class -----*- C++ -*---===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file defines the utilities for WRegion Node class.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_ANAYSIS_VPO_WREGIONUTILS_H
#define LLVM_ANAYSIS_VPO_WREGIONUTILS_H

#include "llvm/Support/Compiler.h"
#include "llvm/IR/IntrinsicInst.h"
#if INTEL_CUSTOMIZATION
#include "llvm/Analysis/Intel_LoopAnalysis/Framework/HIRFramework.h"
#endif // INTEL_CUSTOMIZATION
#include "llvm/Analysis/VPO/WRegionInfo/WRegionCollection.h"
#include "llvm/Analysis/VPO/WRegionInfo/WRegionInfo.h"
#include "llvm/Analysis/VPO/WRegionInfo/WRegion.h"
#include <functional>

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
  /// \name Deleted constructor and destructor to disable instantiation.
  /// @{
  WRegionUtils() = delete;
  ~WRegionUtils() = delete;
  /// @}

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

  /// Update WRGraph from processing intrinsic calls representing directives.
  /// @param[in]     Call     The call instruction with the intrinsic
  /// @param[in,out] WRGraph  The WRN graph being updated
  /// @param[in,out] S        Stack of pending WRN nodes
  /// @param[in]     LI       LoopInfo, needed to call createWRegion()
  /// @param[in]     DT       DomTree, needed to call finalize()
  /// @param[in]     BB       The BasicBlock containing the intrinsic call
#if INTEL_CUSTOMIZATION
  /// @param[in]     H        The HLNode containing the intrinsic call
  ///                         If \p H != null, then HIR is assumed; otherwise
  ///                         LLVM IR CFG is assumed (using BB, DT, LI)
#endif // INTEL_CUSTOMIZATION
  static void updateWRGraph(IntrinsicInst *Call, WRContainerImpl *WRGraph,
                            WRStack<WRegionNode *> &S, LoopInfo *LI,
                            DominatorTree *DT,
#if INTEL_CUSTOMIZATION
                            BasicBlock *BB, loopopt::HLNode *H = nullptr);
#else
                            BasicBlock *BB);
#endif // INTEL_CUSTOMIZATION

  /// \brief Returns a new node derived from WRegionNode node that
  /// matches the construct type based on DirID.
  static WRegionNode *createWRegion(int DirID, BasicBlock *EntryBB,
                                    LoopInfo *LI, unsigned NestingLevel,
                                    CallInst *Dir);

#if INTEL_CUSTOMIZATION
  /// \brief Similar to createWRegion, but for HIR vectorizer support
  static WRegionNode *createWRegionHIR(int DirID, loopopt::HLNode *EntryHLNode,
                                       unsigned NestingLevel,
                                       IntrinsicInst *Call);

  /// \brief Driver routine to build WRGraph based on HIR representation
  static WRContainerImpl *buildWRGraphFromHIR(loopopt::HIRFramework &HIRF);

  /// Does the WRegion infrastructure support having RegDDRefs as operands
  /// for the clause with ID \p ClauseID.
  static bool supportsRegDDRefs(int ClauseID);
#endif // INTEL_CUSTOMIZATION

  /// New OpenMP directives under development can be added to this routine
  /// to make the WRN graph builder skip them instead of asserting.
  static bool skipDirFromWrnConstruction(int DirID);

  /// Lookahead for a nowait clause and return true in case it finds it
  static bool nowaitLookahead(BasicBlock *EntryBB);

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
  static WRegionNode *getParentRegion(const WRegionNode *W,
                                      unsigned WRegionKind);

  /// Traverse ancestors of \p W and return the one, for which
  /// \p IsMatch predicate is true. If \p ProcessNext returns
  /// true for an ancestor, then the traversal stops, but
  /// \p IsMatch is still evaluated for this last ancestor.
  /// \p IsMatch is evaluated at least for the parent of \p W,
  /// if it has one.
  /// If there is no processed ancestor, for which \p IsMatch
  /// is true, then return nullptr.
  static WRegionNode *getParentRegion(
      const WRegionNode *W,
      std::function<bool(const WRegionNode *)> IsMatch,
      std::function<bool(const WRegionNode *)> ProcessNext);

  /// Get the Clause Id for the WRNAtomicKind \p kind.
  static int getClauseIdFromAtomicKind(WRNAtomicKind Kind);

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
  static PrivateItem *wrnSeenAsPrivate(WRegionNode *W, Value *V);
  static FirstprivateItem *wrnSeenAsFirstprivate(const WRegionNode *W,
                                                 const Value *V);
  static LastprivateItem *wrnSeenAsLastprivate(WRegionNode *W, Value *V);
  static ReductionItem *wrnSeenAsReduction(WRegionNode *W, Value *V);
  static MapItem *wrnSeenAsMap(WRegionNode *W, Value *V);

  /// \brief The utility checks whether the given value is used
  /// at the region entry directive.
  static bool usedInRegionEntryDirective(WRegionNode *W, Value *I);

  /// Return true if the value \p V is used in the WRN \p W.
  /// If \p UserInsts is not null, then find all users of \p V in \p W and put
  /// them in \p *UserInsts.
  /// If \p ExcludeEntryDirective is true, then ignore the region.entry
  /// directive of \p W.
  /// If \p UserExprs is not null, then add all ConstantExpr users of \p V in
  /// it.
  ///
  /// Prerequisite: W's BBSet must be populated before calling this util.
  static bool
  findUsersInRegion(WRegionNode *W, Value *V,
                    SmallVectorImpl<Instruction *> *UserInsts = nullptr,
                    bool ExcludeEntryDirective = true,
                    SmallPtrSetImpl<ConstantExpr *> *UserExprs = nullptr);

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

  /// Return \b true if the WRGraph contains OMP target construct(s).
  /// This is used by offloading codegen to exclude routines with
  /// target regions from the target code after outlining is done.
  static bool hasTargetDirective(WRContainerImpl &WRC);
  static bool hasTargetDirective(WRegionInfo *WI);

  /// Returns \b true if one of \p W's ancestors is an OMP target region.
  static bool hasLexicalParentTarget(const WRegionNode *W);

  /// Returns \b true if one of \p W's ancestors is an OMP target region,
  /// or the function where \p W lies in has target declare attribute.
  static bool hasParentTarget(const WRegionNode *W);

  /// \returns \b true iff \p W contains a WRN for which \p Predicate is true.
  static bool containsWRNsWith(WRegionNode *W,
                               std::function<bool(WRegionNode *)> Predicate);

  /// Given a loop-type WRN \p W, if there is an enclosed SIMD construct bound
  /// to the same loop as W, then return the pointer to the WRNVecLoopNode
  /// corresponding to the SIMD construct. Otherwise, return nullptr.
  static WRNVecLoopNode *getEnclosedSimdForSameLoop(WRegionNode *W,
                                                    unsigned Idx = 0);

  /// Return true if \p W represents a stand-alone directive
  static bool isStandAlone(WRegionNode *W);

  /// Add \p V as a new ClauseItem in the Clause \p C.
  template <typename T> static void addToClause(Clause<T> &C, Value *V) {
    C.add(V);
  }

  /// Collect non-pointer, non-constant values that will be used directly
  /// inside the outlined region created for \p W. These include:
  ///
  /// a) Linear clause step:
  /// \code
  ///   %step.val = load i32, i32* %step
  ///   "DIR.OMP.PARALLEL.LOOP" ..."QUAL.OMP.LINEAR" (i32* %y, i32 %step.val)
  /// \endcode
  ///
  /// b) Non-constant size of an array alloca (like C99 VLAs):
  /// \code
  ///   %vla.size = load i32, i32* %size
  ///   %y.vla = alloca i32, i32 %vla.size
  ///   "DIR.OMP.PARALLEL" ..."QUAL.OMP.PRIVATE" (i32* %y.vla)
  /// \endcode
  ///
  /// c) Array section bound expressions:
  /// \code
  ///   "DIR.OMP.PARALLEL.LOOP"(), "QUAL.OMP.REDUCTION.ADD:ARRSECT"(
  ///     [10 x i32]* %y, i64 1, i64 %lb.val, i64 %size.val, i64 1)
  /// \endcode
  ///
  /// The values `%step.val`, `%vla.size`, `%lb.val` and `%size.val` are of
  /// non-pointer types (i32/i64 here), and will be used directly inside the
  /// outlined function created for the construct \p W. However, the OpenMP
  /// runtime only accepts outline functions with pointer arguments. So,
  /// these values need to be collected and passed into the outlined function
  /// by reference.
  ///
  /// This function collects all these values and puts them in a vector in \p W.
  ///
  /// VPOParoptTransform::captureAndAddCollectedNonPointerValuesToSharedClause()
  /// is then used to capture these collected values into pointers, and mark the
  /// pointers as shared, which causes them to be passed in to the outlined
  /// function by reference.
  static void collectNonPointerValuesToBeUsedInOutlinedRegion(WRegionNode *W);

  /// If \p Item is from clause whose getOrig() is also in an allocate clause,
  /// then return the AllocateItem associated with \p Item.
  /// Otherwise, return null.
  static AllocateItem *getAllocateItem(Item *I);

  /// \returns true if W is a WRNDistributeNode, or if it's a
  /// WRNDistributeParLoopNode with its TreatDistributeParLoopAsDistribute flag
  /// set to true; \b false otherwise.
  static bool isDistributeNode(const WRegionNode *W);

  /// \returns true if W is a WRNDistributeParLoopNode with
  /// its TreatDistributeParLoopAsDistribute flag set to false; \b false
  /// otherwise.
  static bool isDistributeParLoopNode(const WRegionNode *W);

  /// {@ Utilities for scan inclusive/exclusive + reduction(inscan).

  /// Find and return the clause item in \p C that matches the Inscan index \p
  /// Idx. Returns null if no such item is found.
  template <typename ItemTy>
  static Item *getClauseItemForInscanIdx(const Clause<ItemTy> &C, uint64_t Idx);

  /// Find and return the clause item in \p W that matches the Inscan index \p
  /// Idx. Returns null if no such item is found.
  static Item *getClauseItemForInscanIdx(const WRegionNode *W, uint64_t Idx);

  /// For a reduction(inscan) item \p I on \p W, returns the corresponding
  /// inclusive/exclusive item from the inner scan directive.
  static InclusiveExclusiveItemBase *
  getInclusiveExclusiveItemForReductionItem(const WRegionNode *W,
                                            const ReductionItem *I);

  /// For an inclusive/exclusive item \p I on \p W, returns the corresponding
  /// reduction(inscan) item from the outer loop/simd directive.
  static ReductionItem *getReductionItemForInclusiveExclusiveItem(
      const WRNScanNode *W, const InclusiveExclusiveItemBase *I);
  /// @}
};

} // End VPO Namespace

} // End LLVM Namespace

#endif // LLVM_ANAYSIS_VPO_WREGIONUTILS_H
#endif // INTEL_COLLAB
