//===-------- DDUtils.cpp - Implements DD Utilities -----------------------===//
//
// Copyright (C) 2015-2018 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// This file implements DD Utilities
//
//===----------------------------------------------------------------------===//

#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRSafeReductionAnalysis.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Framework/HIRFramework.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/DDUtils.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HLNodeUtils.h"
#include "llvm/Support/Debug.h"

using namespace llvm;
using namespace llvm::loopopt;

#define DEBUG_TYPE "dd-utils"

/// Returns true if any incoming/outgoing edge into Loop for a DDRef.
/// Exceptions are dd edges both source and sink are preheader/postexit ddrefs.
/// This function is to be used for sinking instructions in preheader and
/// postexit just using the same logic with which instructions in
/// enclosing(parent) loop body are sinked.
bool DDUtils::anyEdgeToLoop(DDGraph DDG, const DDRef *Ref, HLLoop *Loop) {

  // Current logic matches one loop  only.
  // It can be extended later with an argument to match all
  // the containing ParaentLoops

  DDRef *DDref = const_cast<DDRef *>(Ref);
  for (auto I1 = DDG.outgoing_edges_begin(DDref),
            E1 = DDG.outgoing_edges_end(DDref);
       I1 != E1; ++I1) {

    DDRef *DDRefSink = (*I1)->getSink();
    HLLoop *ParentLoop = DDRefSink->getLexicalParentLoop();
    if (ParentLoop == Loop) {
      return true;
    }
  }

  for (auto I1 = DDG.incoming_edges_begin(DDref),
            E1 = DDG.incoming_edges_end(DDref);
       I1 != E1; ++I1) {

    DDRef *DDRefSrc = (*I1)->getSrc();
    HLLoop *ParentLoop = DDRefSrc->getLexicalParentLoop();
    if (ParentLoop == Loop) {
      return true;
    }
  }
  return false;
}

namespace {

///  Return true if a load can move into the loop.
///  t0 = A[i1]; loop { };
///  In some case, moving a load into a loop requires a corresponding store
///  A[i1] = t0 to be moved into the loop also as in the case of sum reduction
///  t0 = A[i]  ( LRef = RRef )
///   RRef      (a)  No edge to the innermost loop
///   LRef:     (b1) Multiple uses in innermost loop
///         or  (b2) 1 def and 1 use only
///                  Same store for A[x] is needed in PostLoop stmts
///  Note:  More restrictive conditions are put here just to
///         avoid unnessary sinking
///  For Post Loop Stmts, which are all stores, we can only move inside the
///  loop if it is of the form A[i] = t0 or LHS has no DD edges
///
///  StoreNode returns the store after the loop that needs to be moved in also
///  in the case of sum reduction:
bool canMoveLoadIntoLoop(const DDRef *Lref, const DDRef *Rref,
                         HLLoop *InnermostLoop,
                         SmallVectorImpl<HLInst *> &PostLoopInsts,
                         HLInst **StoreInst, DDGraph DDG) {

  DDRef *LRef = const_cast<DDRef *>(Lref);
  DDRef *RRef = const_cast<DDRef *>(Rref);
  HLNode *StoreNode1 = nullptr;
  HLNode *StoreNode2 = nullptr;
  const DDEdge *AntiEdge = nullptr;
  const DDEdge *FlowEdge = nullptr;
  HLNode *Node;

  if (DDUtils::anyEdgeToLoop(DDG, RRef, InnermostLoop)) {
    // (a) no edge into innermost Loop
    return false;
  }

  for (auto I1 = DDG.outgoing_edges_begin(RRef),
            E1 = DDG.outgoing_edges_end(RRef);
       I1 != E1; ++I1) {
    //  ..  = A[i]  is  RRef
    const DDEdge *Edge = *I1;
    DDRef *DDRefSink = Edge->getSink();

    Node = DDRefSink->getHLDDNode();
    if (Node->getLexicalParentLoop() == InnermostLoop) {
      // TODO: remove this check as it is redundant with one in anyEdgeToLoop
      //       above
      return false;
    }
    if (Edge->isANTIdep()) {
      AntiEdge = Edge;
      StoreNode1 = Node;
    }
  }

  unsigned Defs = 0;
  for (auto I1 = DDG.outgoing_edges_begin(LRef),
            E1 = DDG.outgoing_edges_end(LRef);
       I1 != E1; ++I1) {
    //   t0  = ..    ; t0 is LRef
    const DDEdge *Edge = *I1;
    DDRef *DDRefSink = Edge->getSink();
    RegDDRef *RegRef = dyn_cast<RegDDRef>(DDRefSink);
    if (!RegRef) {
      // TODO: Handles blobs later
      return false;
    }
    Node = DDRefSink->getHLDDNode();
    if (Edge->isOUTPUTdep() && Node->getParentLoop() != InnermostLoop) {
      return false;
    }
    if (Edge->isOUTPUTdep()) {
      if (++Defs > 1) {
        return false;
      }
    } else if (Edge->isFLOWdep()) {
      const HLLoop *ParentLoop = Node->getLexicalParentLoop();
      if (ParentLoop == InnermostLoop->getParentLoop()) {
        FlowEdge = Edge;
        StoreNode2 = Node;
      }
    }
  }

  // Defs is either 0 or 1
  // FlowEdge and AntiEdges are all between inst in
  // right outside the innermostloop.
  // TODO: I did not find logic for (1 use in innermost loop
  //       if def is 1), which is stated in the comment of this function.
  if (Defs == 1 && (StoreNode1 != StoreNode2)) {
    return false;
  }
  if (Defs == 1) {
    if (!FlowEdge || !AntiEdge) {
      // This is a case that the load goes through 2 copy stmts
      // Need some forwardSub cleanup. Bail out now.
      return false;
    }
    unsigned Level = InnermostLoop->getNestingLevel() - 1;
    if (FlowEdge->getDVAtLevel(Level) != DVKind::EQ) {
      return false;
    }
    if (AntiEdge->getDVAtLevel(Level) != DVKind::EQ) {
      return false;
    }
  }
  if (StoreNode1) {
    *StoreInst = cast<HLInst>(StoreNode1);
  }
  return true;
}

///  Search for the corresponding load - in the case when Forward Sub
///  is not done well.
///
///  This function specifically looks for the following pattern:
///     t0 = A[] // --(1) a load inst
///
///     t1 = t0  // --(2)
///  Given (2), (1) should be found.
///  Note that (1) is appearing before (2). Thus, PreLoopInsts is searched
///  For (1).
bool findLoadInst(const DDRef *RRef, SmallVectorImpl<HLInst *> &PreLoopInsts,
                  DDGraph DDG) {

  for (auto &Inst : PreLoopInsts) {
    const DDRef *LRef = Inst->getLvalDDRef();
    if (RRef->getSymbase() == LRef->getSymbase()) {
      // (A)
      // t0 = a[i1];     LRef = ...    (Inst)
      // ...
      // t2 = t0;         ... = RRef
      // Make sure t0 is not liveout and 1 single use
      if (LRef->isLiveOutOfRegion() || !DDG.singleEdgeGoingOut(LRef)) {
        return false;
      }
      return true;
    }
  }
  return false;
}

// TODO: Remove CopyStmt related logic if possible. Now HIRTempCleanup
//       takes care of those.
template <bool IsPreHeader = false>
bool gatherPreloopInsts(HLInst *Inst, HLLoop *InnermostLoop, DDGraph DDG,
                        SmallVectorImpl<HLInst *> &PreLoopInsts,
                        SmallVectorImpl<HLInst *> &ForwardSubInsts) {

  if (!Inst) {
    // pre(post)loop HLNode might have not be HLInst (e.g HLIf)
    return false;
  }
  const Instruction *LLVMInst = Inst->getLLVMInstruction();
  bool IsCopyInst = Inst->isCopyInst();
  if (!isa<LoadInst>(LLVMInst) && !IsCopyInst) {
    return false;
  }

  bool CopyStmt = false;
  if (IsCopyInst) {
    if (!findLoadInst(Inst->getRvalDDRef(), PreLoopInsts, DDG)) {
      return false;
    }
    CopyStmt = true;
  }

  if (!IsPreHeader && InnermostLoop->hasZtt()) {
    unsigned LvalSymbase = Inst->getLvalDDRef()->getSymbase();
    if (Inst->getParentLoop()->isLiveOut(LvalSymbase)) {
      return false;
    }
  }

  if (!CopyStmt &&
      DDUtils::anyEdgeToLoop(DDG, Inst->getRvalDDRef(), InnermostLoop)) {
    return false;
  }

  PreLoopInsts.push_back(Inst);
  if (CopyStmt) {
    ForwardSubInsts.push_back(Inst);
  }

  return true;
}

template <bool IsPostexit = false>
bool gatherPostloopInsts(HLInst *Inst, const HLLoop *InnermostLoop,
                         SmallVectorImpl<HLInst *> &PostLoopInsts) {
  if (!Inst) {
    // pre(post)loop HLNode might have not be HLInst (e.g HLIf)
    return false;
  }

  const Instruction *LLVMInst = Inst->getLLVMInstruction();
  // Allow only Store in PostLoop Nodes
  if (!isa<StoreInst>(LLVMInst)) {
    return false;
  }

  if (!IsPostexit && InnermostLoop->hasZtt()) {
    return false;
  }

  PostLoopInsts.push_back(Inst);
  return true;
}

/// Gather Pre / Post Nodes in Vectors
/// Called from EnablePerfectLoopNest Util
/// Return false if unwanted nodes are encountered (e.g  if)
/// Traverse the Nodes outside the InnermostLoop and put them in
/// Pre / Post Vectors for later processing
bool enablePerfectLPGatherPrePostInsts(
    HLLoop *InnermostLoop, DDGraph DDG, SmallVectorImpl<HLInst *> &PreLoopInsts,
    SmallVectorImpl<HLInst *> &PostLoopInsts,
    SmallVectorImpl<HLInst *> &ForwardSubInsts) {

  HLLoop *ParentLoop = InnermostLoop->getParentLoop();
  unsigned NumLoops = 0;

  for (auto I1 = ParentLoop->child_begin(), E1 = ParentLoop->child_end();
       I1 != E1; ++I1) {

    if (isa<HLLoop>(I1)) {
      if (++NumLoops > 1) {
        return false;
      }
      continue;
    }

    if (NumLoops == 0) {
      if (!gatherPreloopInsts(dyn_cast<HLInst>(I1), InnermostLoop, DDG,
                              PreLoopInsts, ForwardSubInsts)) {
        return false;
      }
    } else {
      if (!gatherPostloopInsts(dyn_cast<HLInst>(I1), InnermostLoop,
                               PostLoopInsts)) {
        return false;
      }
    }
  }

  // Scan preheader insts
  for (auto I = InnermostLoop->pre_begin(), E = InnermostLoop->pre_end();
       I != E; ++I) {
    if (!gatherPreloopInsts<true>(cast<HLInst>(I), InnermostLoop, DDG,
                                  PreLoopInsts, ForwardSubInsts)) {
      return false;
    }
  }

  // Scan postexit insts
  for (auto I = InnermostLoop->post_begin(), E = InnermostLoop->post_end();
       I != E; ++I) {
    if (!gatherPostloopInsts<true>(cast<HLInst>(I), InnermostLoop,
                                   PostLoopInsts)) {
      return false;
    }
  }

  return true;
}

/// Find node receiving the load
/// e.g.   t0 = a[i] ;
///         ...
///        t1 = t0
///  returns t1 = t0
///
/// t0 = a[i1];     LRef =
///  ...
/// t1  = t0        Node
/// Looking for Node (assuming  forward sub is not done)
HLInst *findForwardSubInst(const DDRef *LRef,
                           SmallVectorImpl<HLInst *> &ForwardSubInsts) {

  for (auto &Inst : ForwardSubInsts) {
    const RegDDRef *RRef = Inst->getRvalDDRef();
    if (RRef->getSymbase() == LRef->getSymbase()) {
      return Inst;
    }
  }
  return nullptr;
}

// Legality Check for nodes before Loop.
// Return true if legal
// TODO: Remove CopyStmt/ForwardSubInsts related logic if possible. Now
// HIRTempCleanup
//       takes care of those.
bool enablePerfectLPLegalityCheckPre(
    HLLoop *InnermostLoop, DDGraph DDG, SmallVectorImpl<HLInst *> &PreLoopInsts,
    SmallVectorImpl<HLInst *> &PostLoopInsts,
    SmallVectorImpl<HLInst *> &ForwardSubInsts,
    SmallVectorImpl<HLInst *> &ValidatedStores) {

  const DDRef *LRef, *RRef;

  for (auto &Inst : PreLoopInsts) {
    const Instruction *LLVMInst;

    if (std::find(ForwardSubInsts.begin(), ForwardSubInsts.end(), Inst) !=
        ForwardSubInsts.end()) {
      // if copy skip.
      continue;
    }

    // Restrict to a small subset for compile time consideration
    LLVMInst = Inst->getLLVMInstruction();
    if (!isa<LoadInst>(LLVMInst)) {
      return false;
    }

    // Two cases to consider:
    //  a. t0 = a[x]
    //  b. t0 = a[x]
    //     ....
    //     t2 = t0
    //
    //  t = A[x]  ( LRef = RRef )
    //   LRef  t0 for a. else  t2
    //   Rref     No edge to the innermost loop
    //   LRef:    if there is  output edge to the innermost loop,
    //            the same store for A[x] is needed in PostLoop stmts
    LRef = Inst->getLvalDDRef();
    RRef = Inst->getRvalDDRef();
    HLInst *ForwardSInst = findForwardSubInst(LRef, ForwardSubInsts);
    LRef = ForwardSInst ? ForwardSInst->getLvalDDRef() : Inst->getLvalDDRef();
    HLInst *StoreInst = nullptr;
    if (!canMoveLoadIntoLoop(LRef, RRef, InnermostLoop, PostLoopInsts,
                             &StoreInst, DDG)) {
      LLVM_DEBUG(dbgs() << "\n Fails at canMoveLoadIntoLoop \n");
      LLVM_DEBUG(Inst->dump());
      return false;
    }
    if (StoreInst) {
      ValidatedStores.push_back(StoreInst);
    }
  }
  return true;
}

// Legality Check for nodes after Loop.
// Called from EnablePerfectLoopNest Util.
// Return true if legal
bool enablePerfectLPLegalityCheckPost(
    HLLoop *InnermostLoop, DDGraph DDG,
    SmallVectorImpl<HLInst *> &PostLoopInsts,
    SmallVectorImpl<HLInst *> &ValidatedStores) {

  for (auto &Inst : PostLoopInsts) {
    //  A[i] = t0
    // a. Node is in Validated Store, Ok
    // b. No in/out edge of A[i] to InnermostLoop

    if (std::find(ValidatedStores.begin(), ValidatedStores.end(), Inst) !=
        ValidatedStores.end()) {
      continue;
    }
    const DDRef *LRef = Inst->getLvalDDRef();
    if (DDUtils::anyEdgeToLoop(DDG, LRef, InnermostLoop)) {
      return false;
    }
  }

  return true;
}

} // namespace

/// Updates DDRef linearity.
//  Input is a vector of Nodes that have been moved into the innermost loop.
/// Current code will take care of assignment stmt moved into the innermost
/// loop.
/// TODO: safe reduction chain needs to be removed. Currently there are a
/// few copy stmts spanning that chain. Need to wait until Safe reduction
/// is done
void DDUtils::updateDDRefsLinearity(SmallVectorImpl<HLInst *> &HLInsts,
                                    DDGraph DDG) {

  const Instruction *LLVMInst;
  RegDDRef *LRef;

  for (auto &Inst : HLInsts) {
    auto Node1 = cast<HLNode>(Inst);
    HLNode *Node = Node1;
    HLLoop *ParentLoop = Node->getParentLoop();
    (void)ParentLoop;
    assert(ParentLoop && ParentLoop->isInnermost() &&
           "Only handles for stmts in the innermost loop now");
    LLVMInst = Inst->getLLVMInstruction();

    // Stores are only move from post loop stmts.
    // Only need to clear safe reduction chain (TODO)
    if (isa<StoreInst>(LLVMInst)) {
      continue;
    }

    LRef = Inst->getLvalDDRef();
    assert(LRef->isTerminalRef() && "Unexpected memrefs");
    assert(LRef->getSingleCanonExpr()->isNonLinear() &&
           "Unexpected linear temps");

    // Make uses as non-linear
    for (auto I2 = DDG.outgoing_edges_begin(LRef),
              E2 = DDG.outgoing_edges_end(LRef);
         I2 != E2; ++I2) {

      const DDEdge *Edge = *I2;
      if (!Edge->isFLOWdep()) {
        continue;
      }

      DDRef *DDRefSink = Edge->getSink();
      HLDDNode *SinkDDNode = DDRefSink->getHLDDNode();
      HLLoop *ParentLoop = SinkDDNode->getParentLoop();
      (void)ParentLoop;
      assert(ParentLoop && ParentLoop->isInnermost() &&
             "Unexpected stmt outside loop");
      RegDDRef *RegRef = dyn_cast<RegDDRef>(DDRefSink);
      CanonExpr *SinkCE = nullptr;

      if (RegRef) {
        assert(RegRef->isTerminalRef() && "Unexpected memrefs");
        SinkCE = RegRef->getSingleCanonExpr();
      } else {
        SinkCE = cast<BlobDDRef>(DDRefSink)->getMutableSingleCanonExpr();
      }
      // There might be defs which are non-linear encountered here,
      // update it anyway
      SinkCE->setNonLinear();
    }
  }
}

// \p IsPreLoop indicates whether the sinked instruction appeared before or
// after the loop before sinking.
static void updateSinkedRvalLiveinsLiveouts(unsigned RvalSymbase,
                                            HLLoop *InnermostLoop,
                                            bool IsPreLoop) {
  auto ParentLoop = InnermostLoop->getParentLoop();

  if (ParentLoop->isLiveIn(RvalSymbase)) {
    InnermostLoop->addLiveInTemp(RvalSymbase);
  }

  // If rval symbase is not liveout of parent loop, remove it as a liveout
  // from the innermost loop.
  // DO i2 =
  // END DO
  //   A[i1] = t1; << sinked instruction removes t1 as liveout.
  if (!IsPreLoop && !ParentLoop->isLiveOut(RvalSymbase)) {
    InnermostLoop->removeLiveOutTemp(RvalSymbase);
  }
}

// \p IsPreLoop indicates whether the sinked instruction appeared before or
// after the loop before sinking.
static void updateLiveinsLiveoutsForSinkedInst(HLLoop *InnermostLoop,
                                               HLInst *SinkedInst,
                                               bool IsPreLoop) {

  // Mark lval terminal ref in the sinked instruction as liveout to the
  // innermost loop if it is liveout of the parent loop. Here's an example where
  // t1 will become liveout of i2 loop- DO i1
  //     t1 = A[i1]
  //   DO i2
  //   END DO
  // END DO
  //   A[5] = t1
  //
  // Mark all blob refs in the sinked instruction as livein to the innermost
  // loop if it is livein to the parent loop. Here's an example where t1 will
  // become livein to i2 loop-
  //   t1 =
  // DO i1
  //     t2 = A[t1]
  //   DO i2
  //   END DO
  // END DO
  for (auto I = SinkedInst->op_ddref_begin(), E = SinkedInst->op_ddref_end();
       I != E; ++I) {
    auto Ref = *I;

    if (Ref->isLval() && Ref->isTerminalRef()) {
      auto ParentLoop = InnermostLoop->getParentLoop();
      unsigned Symbase = Ref->getSymbase();

      if (ParentLoop->isLiveOut(Symbase)) {
        InnermostLoop->addLiveOutTemp(Symbase);
      }

      // If lval symbase is not livein to parent loop, remove it as a livein
      // from the innermost loop.
      //  t1 = A[i1]; << sinked instruction removes t1 as livein.
      // DO i2 =
      if (IsPreLoop && !ParentLoop->isLiveIn(Symbase)) {
        InnermostLoop->removeLiveInTemp(Symbase);
      }
    } else if (Ref->isSelfBlob()) {
      updateSinkedRvalLiveinsLiveouts(Ref->getSymbase(), InnermostLoop,
                                      IsPreLoop);
    } else {
      for (auto BIt = Ref->blob_cbegin(), End = Ref->blob_cend(); BIt != End;
           ++BIt) {
        updateSinkedRvalLiveinsLiveouts((*BIt)->getSymbase(), InnermostLoop,
                                        IsPreLoop);
      }
    }
  }
}

static void gatherTempRegDDRefSymbases(
    const SmallVectorImpl<HLInst *> &Insts,
    InterchangeIgnorableSymbasesTy &SinkedTempDDRefSymbases) {

  for (auto &I : Insts) {
    const HLDDNode *DDNode = dyn_cast<HLDDNode>(I);
    for (const DDRef *PrePostDDRef :
         llvm::make_range(DDNode->op_ddref_begin(), DDNode->op_ddref_end())) {
      if (PrePostDDRef->isTerminalRef()) {
        SinkedTempDDRefSymbases.insert(PrePostDDRef->getSymbase());
      }
    }
  }
}

///  Enables Perfect Loop Nests
///  Only takes care loops of this form:
///  Invariants are just those right outside the innermost loop
///   do i1
///    do i3
///      do i3
///        s1; s2;
///        do i4
///        end do
///        s3 ; s4;
///      enddo
///   enddo
///  s1, ... s4 are siblings of the innermost loop
///
///  Stmts are load / store / copy  (2 operands)
///  We need to prove that the load/copy in the PreLoop stmts are
///  using the same temp as in the Stores in the PostLoop stmts.
///  These cases are fine:
///
///  (1) t0 = p[i]
///      do;  .. enddo;
///      p[i] = t0
///  (2) t0 = p[i]
///      do;  .. enddo;   (No Def of t0 inside loop)
///  (3) do ; .. enddo;
///      p[i] = t0
///      if this stmt is not part of (1), then no Dep edge of p or p[i] allowed
///
bool DDUtils::enablePerfectLoopNest(
    HLLoop *InnermostLoop, DDGraph DDG,
    InterchangeIgnorableSymbasesTy &SinkedTempDDRefSymbases) {
  assert(InnermostLoop->getParentLoop() && "Parent Loop must not be nullptr");

  SmallVector<HLInst *, 8> PreLoopInsts;
  SmallVector<HLInst *, 8> PostLoopInsts;
  SmallVector<HLInst *, 8> ForwardSubInsts;
  SmallVector<HLInst *, 8> ValidatedStores;

  //  Allow copy stmts before the innermostt loop, but not inside the innermost.
  //  The copy stmts need special handling for legality check
  //
  //  t0 = a[x]
  //  t2 = t0
  //  if t0 is not live out and t0 has 1 use, treat it as t2 = a[x]
  //  No need to do forward sub.
  //  (1) Gather  PreLoop / PostLoop Nodes in Vector.
  //  ForwarSub nodes are  copy stmts of the for  t2 = t0
  if (!enablePerfectLPGatherPrePostInsts(InnermostLoop, DDG, PreLoopInsts,
                                         PostLoopInsts, ForwardSubInsts)) {
    LLVM_DEBUG(dbgs() << "\n Fails in gatherprepost stmts \n");
    return false;
  }

  // (2) Perform legality  check of PreLoop nodes
  if (!enablePerfectLPLegalityCheckPre(InnermostLoop, DDG, PreLoopInsts,
                                       PostLoopInsts, ForwardSubInsts,
                                       ValidatedStores)) {
    LLVM_DEBUG(dbgs() << "\n Fails in legality pre stmt check \n");
    return false;
  }

  // (3) Perform legality check of PostLoop nodes
  if (!enablePerfectLPLegalityCheckPost(InnermostLoop, DDG, PostLoopInsts,
                                        ValidatedStores)) {
    LLVM_DEBUG(dbgs() << "\n Fails in legality post stmt check \n");
    return false;
  }

  // (4) Move Stmts into Innermost Loop
  for (auto I = PreLoopInsts.rbegin(), E = PreLoopInsts.rend(); I != E; ++I) {
    HLNodeUtils::moveAsFirstChild(InnermostLoop, *I);
    updateLiveinsLiveoutsForSinkedInst(InnermostLoop, *I, true);
  }

  for (auto &I : PostLoopInsts) {
    HLNodeUtils::moveAsLastChild(InnermostLoop, I);
    updateLiveinsLiveoutsForSinkedInst(InnermostLoop, I, false);
  }

  // Call Util to update the temp DDRefs from linear-at-level to non-linear
  updateDDRefsLinearity(PreLoopInsts, DDG);
  updateDDRefsLinearity(PostLoopInsts, DDG);

  // Gather temp DDRef's symbase in sinked instruction
  // Loop Interchange needs this information
  gatherTempRegDDRefSymbases(PreLoopInsts, SinkedTempDDRefSymbases);
  gatherTempRegDDRefSymbases(PostLoopInsts, SinkedTempDDRefSymbases);

  return true;
}

///  How many uses of LvalRef in loop?  Counts blobs as well

bool DDUtils::maxUsesInLoop(const RegDDRef *LvalRef, const HLLoop *Loop,
                            DDGraph DDG, const unsigned Threshold) {

  assert(LvalRef && LvalRef->isLval() && "DDRef must be lval");
  assert(Loop && "Loop  must be supplied");

  unsigned NumUse = 0;
  for (auto I1 = DDG.outgoing_edges_begin(LvalRef),
            E1 = DDG.outgoing_edges_end(LvalRef);
       I1 != E1; ++I1) {

    DDRef *DDRefSink = (*I1)->getSink();

    // Skip Sink outside loop, including prehdr/postexit
    if (!(HLNodeUtils::contains(Loop, DDRefSink->getHLDDNode()))) {
      continue;
    }

    RegDDRef *RefSink = dyn_cast<RegDDRef>(DDRefSink);
    if ((!RefSink || RefSink->isRval()) && ++NumUse > Threshold) {
      return false;
    }
  }

  return true;
}

///  Is single use of LvalRef in loop?  Counts blobs as well

bool DDUtils::singleUseInLoop(const RegDDRef *LvalRef, const HLLoop *Loop,
                              DDGraph DDG) {
  return maxUsesInLoop(LvalRef, Loop, DDG, 1);
}

/// Checks if a DDRef is part of a reduction. Must match with input Symbase
///
bool DDUtils::isValidReductionDDRef(RegDDRef *RRef, HLLoop *Loop,
                                    unsigned FirstSymbase,
                                    bool *LastReductionInst, DDGraph DDG) {
  *LastReductionInst = false;
  CanonExpr *CE = RRef->getSingleCanonExpr();
  if (!CE->isNonLinear()) {
    return false;
  }
  if (RRef->isLval()) {
    if (!singleUseInLoop(RRef, Loop, DDG)) {
      return false;
    }
    if (RRef->getSymbase() == FirstSymbase) {
      *LastReductionInst = true;
      return true;
    }
  }

  return true;
}

namespace {
bool isLegalToShiftLoop(unsigned DstLevel, unsigned SrcLevel,
                        unsigned OutmostNestingLevel,
                        SmallVectorImpl<DirectionVector> &DVs) {

  unsigned SmallerLevel;

  //  Trying to move Loop from Srclevel to DstLevel
  //  The loop can either shift inwards or outwards depending if SrcLevel <
  //  DstLevel
  //  It would become illegal if the leftmost non-equal dv is  ">".
  //  for each DV
  // (1)
  //  First scan if there is any leading dv "<" outside the range of movement
  //  e.g.  ( <  < = >)  if we are moving only last 3 levels then it is always
  //  legal
  //  because it will end up as ( < > = <)
  // (2) Moving < inwards: Once we hit <, then it is legal , if we hit > or *
  //     return illegal
  // (3) Moving > outwards: okay to shift * to left as long as it does
  //     not hit <
  //

  //  Adjust DstLevel based on OutmostNestingLevel
  //  because DV are based on actual loop level, input Dst/Src level
  //  are relative to 1
  DstLevel += OutmostNestingLevel - 1;
  SrcLevel += OutmostNestingLevel - 1;
  SmallerLevel = std::min(SrcLevel, DstLevel);

  for (auto &II : DVs) {
    bool Ok = false;
    const DirectionVector &WorkDV = II;
    // (1)
    for (unsigned KK = OutmostNestingLevel; KK < SmallerLevel; ++KK) {
      if (WorkDV[KK - 1] == DVKind::LT) {
        Ok = true;
        break;
      }
    }
    if (Ok) {
      continue;
    }
    // (2)
    if (DstLevel > SrcLevel) {
      if (WorkDV[SrcLevel - 1] & DVKind::LT) {
        for (unsigned JJ = SrcLevel + 1; JJ <= DstLevel; ++JJ) {
          if (WorkDV[JJ - 1] == DVKind::LT || WorkDV[JJ - 1] == DVKind::LE) {
            break;
          }
          if (WorkDV[JJ - 1] & DVKind::GT) {
            return false;
          }
        }
      }
    } else {
      // (3)
      // (= = *)  Okay to shift * to left as long as it does not hit <
      if (WorkDV[SrcLevel - 1] & DVKind::GT) {
        for (unsigned JJ = SrcLevel - 1; JJ >= DstLevel; --JJ) {
          if (WorkDV[JJ - 1] & DVKind::LT) {
            return false;
          }
        }
      }
    }
  }
  return true;
}
} // namespace

bool DDUtils::isLegalForPermutation(unsigned DstLevel, unsigned SrcLevel,
                                    unsigned OutmostNestingLevel,
                                    SmallVectorImpl<DirectionVector> &DVs) {
  if (SrcLevel == DstLevel) {
    return true;
  }
  return isLegalToShiftLoop(DstLevel, SrcLevel, OutmostNestingLevel, DVs);
}

namespace {
///  1. Ignore all  (= = ..)
///  2. for temps, ignore  anti (< ..)
///     If there is a loop carried flow for scalars, the DV will not
///     be all =
///     (check no longer needed because of change in DD for compile time
///     saving)
///  3. Safe reduction (already excluded out in collectDDInfo)
bool ignoreEdgeForPermute(const DDEdge *Edge, const HLLoop *CandidateLoop,
                          DirectionVector *RefinedDV = nullptr) {

  const DirectionVector *DV = RefinedDV;
  if (DV == nullptr) {
    DV = &Edge->getDV();
  }
  if (DV->isEQ()) {
    return true;
  }

  if (DV->isIndepFromLevel(CandidateLoop->getNestingLevel())) {
    return true;
  }

  // t1 =
  //    = t1
  // Anti dep (<) for LoopIndepDepTemp is no longer generated
  // no need to check

  return false;
}

///  Scan presence of  < ... >
///  If none, return true, which  means DV can be dropped for
///  Interchange legality checking
bool ignoreDVWithNoLTGTForPermute(const DirectionVector &DV,
                                  unsigned OutmostNestingLevel,
                                  unsigned InnermostNestingLevel) {

  bool DVhasLT = false;
  unsigned LTLevel = 0;

  for (unsigned II = OutmostNestingLevel; II <= InnermostNestingLevel; ++II) {
    if (DVhasLT && (DV[II - 1] & DVKind::GT)) {
      if (II != LTLevel) {
        return false;
      }
    }

    if (!DVhasLT && (DV[II - 1] & DVKind::LT)) {
      DVhasLT = true;
      LTLevel = II;
    }
  }
  return true;
}

// Collect DVs to be examined for permuting loops
// For a candidate loop, DVs of its Edges are collected.
// Some Edges are ignored since does not affect the legality of interchange
// This visitor assumes SRA is computed already
struct CollectDDInfoForPermute final : public HLNodeVisitorBase {

  const HLLoop *CandidateLoop;
  const unsigned OutermostLevel;
  const unsigned InnermostLevel;
  HIRDDAnalysis &DDA;
  DDGraph &DDG;
  HIRSafeReductionAnalysis &SRA;
  InterchangeIgnorableSymbasesTy *IgnorableSymBases;

  // Indicates if we need to call Demand Driven DD to refine DV
  bool RefineDV;

  // Outputs of this visitor
  SmallVectorImpl<DirectionVector> &DVs;

  InterchangeIgnorableSymbasesTy EmptyIgnorableSBs;

  CollectDDInfoForPermute(const HLLoop *CandidateLoop, unsigned OutermostLevel,
                          unsigned InnermostLevel, HIRDDAnalysis &DDA,
                          DDGraph &DDG, HIRSafeReductionAnalysis &SRA,
                          InterchangeIgnorableSymbasesTy *Ignores,
                          bool RefineDV, SmallVectorImpl<DirectionVector> &DVs);

  void visit(const HLDDNode *DDNode);

  void visit(const HLNode *Node) {}
  void postVisit(const HLNode *Node) {}
  void postVisit(const HLDDNode *Node) {}
};
} // namespace

CollectDDInfoForPermute::CollectDDInfoForPermute(
    const HLLoop *CandidateLoop, unsigned OutermostLevel,
    unsigned InnermostLevel, HIRDDAnalysis &DDA, DDGraph &DDG,
    HIRSafeReductionAnalysis &SRA, InterchangeIgnorableSymbasesTy *Ignores,
    bool RefineDV, SmallVectorImpl<DirectionVector> &DVs)
    : CandidateLoop(CandidateLoop), OutermostLevel(OutermostLevel),
      InnermostLevel(InnermostLevel), DDA(DDA), DDG(DDG), SRA(SRA),
      IgnorableSymBases(Ignores), RefineDV(RefineDV), DVs(DVs) {
  DVs.clear();
  if (!IgnorableSymBases) {
    IgnorableSymBases = &EmptyIgnorableSBs;
  }
}

void CollectDDInfoForPermute::visit(const HLDDNode *DDNode) {

  const HLInst *Inst = dyn_cast<HLInst>(DDNode);
  if (Inst && SRA.isSafeReduction(Inst)) {
    LLVM_DEBUG(dbgs() << "\n\tIs Safe Red");
    return;
  }

  for (auto I = DDNode->ddref_begin(), E = DDNode->ddref_end(); I != E; ++I) {

    // Ignorable symbases are symbases of temps originally were
    // in pre(post)loop or preheader/postexit.
    // Those were legally sinked into the innermost loop.
    // The fact allows us to ignore DDs related to those temps.
    if ((*I)->isTerminalRef() && IgnorableSymBases->count((*I)->getSymbase())) {
      continue;
    }

    for (auto II = DDG.outgoing_edges_begin(*I),
              EE = DDG.outgoing_edges_end(*I);
         II != EE; ++II) {
      // Examining outoging edges is sufficent
      const DDEdge *Edge = *II;
      DDRef *DDref = Edge->getSink();

      if (ignoreEdgeForPermute(Edge, CandidateLoop)) {

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
        LLVM_DEBUG(dbgs() << "\n\t<Edge dropped>");
        LLVM_DEBUG(dbgs() << "\t"; Edge->print(dbgs()));
#endif
        continue;
      }
      const DirectionVector *TempDV = &Edge->getDV();

      // Calling Demand Driven DD to refine DV
      RefinedDependence RefinedDep;

      if (RefineDV) {
        DDRef *SrcDDRef = Edge->getSrc();
        DDRef *DstDDRef = DDref;

        // Refine works only for non-terminal refs
        RefinedDep = DDA.refineDV(SrcDDRef, DstDDRef, OutermostLevel,
                                  InnermostLevel, false);

        if (RefinedDep.isIndependent()) {
          LLVM_DEBUG(dbgs() << "\n\t<Edge dropped with DDTest Indep>");
          LLVM_DEBUG(dbgs() << "\t"; Edge->print(dbgs()));
          continue;
        }

        if (RefinedDep.isRefined()) {
          LLVM_DEBUG(dbgs() << "\n\t<Edge with refined DV>");
          LLVM_DEBUG(dbgs() << "\t"; Edge->print(dbgs()));
          if (ignoreEdgeForPermute(Edge, CandidateLoop, &RefinedDep.getDV())) {
            LLVM_DEBUG(dbgs() << "\n\t<Edge dropped with refined DV Ignore>");
            LLVM_DEBUG(dbgs() << "\t"; Edge->print(dbgs()));
            continue;
          }

          TempDV = &RefinedDep.getDV();
        }
      }
      if (ignoreDVWithNoLTGTForPermute(*TempDV, OutermostLevel,
                                       InnermostLevel)) {
        LLVM_DEBUG(dbgs() << "\n\t<Edge dropped with NoLTGT>");
        LLVM_DEBUG(dbgs() << "\t"; Edge->print(dbgs()));
        continue;
      }

      //  Save the DV in an array which will be used later
      DVs.push_back(*TempDV);

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
      LLVM_DEBUG(dbgs() << "\n\t<Edge selected>");
      LLVM_DEBUG(dbgs() << "\t"; Edge->print(dbgs()));
#endif
    }
  }
}

void DDUtils::computeDVsForPermute(
    SmallVectorImpl<DirectionVector> &DVs, const HLLoop *OutermostLoop,
    unsigned InnermostNestingLevel, HIRDDAnalysis &DDA,
    HIRSafeReductionAnalysis &SRA, bool RefineDV,
    InterchangeIgnorableSymbasesTy *IgnorableSBs) {

  DDGraph DDG = DDA.getGraph(OutermostLoop);
  CollectDDInfoForPermute CDD(OutermostLoop, OutermostLoop->getNestingLevel(),
                              InnermostNestingLevel, DDA, DDG, SRA,
                              IgnorableSBs, RefineDV, DVs);

  HLNodeUtils::visit(CDD, OutermostLoop);
}
