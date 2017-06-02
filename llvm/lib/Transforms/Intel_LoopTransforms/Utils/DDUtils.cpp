//===-------- DDUtils.cpp - Implements DD Utilities -----------------------===//
//
// Copyright (C) 2015 Intel Corporation. All rights reserved.
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

#include "llvm/Transforms/Intel_LoopTransforms/Utils/DDUtils.h"
#include "llvm/Analysis/Intel_LoopAnalysis/HIRFramework.h"
#include "llvm/Support/Debug.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/HLNodeUtils.h"

using namespace llvm;
using namespace llvm::loopopt;

#define DEBUG_TYPE "dd-utils"

/// Returns true if any incoming/outgoing edge into Loop for a DDRef.
bool DDUtils::anyEdgeToLoop(DDGraph DDG, const DDRef *Ref, HLLoop *Loop) {

  // Current logic matches one loop  only.
  // It can be extended later with an argument to match all
  // the containing ParaentLoops

  DDRef *DDref = const_cast<DDRef *>(Ref);
  for (auto I1 = DDG.outgoing_edges_begin(DDref),
            E1 = DDG.outgoing_edges_end(DDref);
       I1 != E1; ++I1) {

    DDRef *DDRefSink = (*I1)->getSink();
    HLLoop *ParentLoop = DDRefSink->getParentLoop();
    if (ParentLoop == Loop) {
      return true;
    }
  }

  for (auto I1 = DDG.incoming_edges_begin(DDref),
            E1 = DDG.incoming_edges_end(DDref);
       I1 != E1; ++I1) {

    DDRef *DDRefSrc = (*I1)->getSrc();
    HLLoop *ParentLoop = DDRefSrc->getParentLoop();
    if (ParentLoop == Loop) {
      return true;
    }
  }
  return false;
}

///  t0 = A[i]  ( LRef = RRef )
///   RRef      (a)  No edge to the innermost loop
///   LRef:     (b1) Multiple uses in innermost loop
///         or  (b2) 1 def and 1 use only
///                  Same store for A[x] is needed in PostLoop stmts
///   Note:  More restrictive conditions are put here just to
///          avoid unnessary sinking
///   For Post Loop Stmts, which are all stores, we can only move inside the
///   loop if it is of the form A[i] = t0 or LHS has no DD edges
///
///   StoreNode returns the store after the loop that needs to be moved in also
///   in the case of sum reduction:

bool DDUtils::canMoveLoadIntoLoop(const DDRef *Lref, const DDRef *Rref,
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
    if (Node->getParentLoop() == InnermostLoop) {
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
      // Handles blobs later
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
      const HLLoop *ParentLoop = InnermostLoop->getParentLoop();
      if (Node->getParentLoop() == ParentLoop) {
        FlowEdge = Edge;
        StoreNode2 = Node;
      }
    }
  }
  if (Defs == 1 && (StoreNode1 != StoreNode2)) {
    return false;
  }

  if (Defs == 1) {
    if (!FlowEdge || !AntiEdge) {
      // This is a case that the load goes through 2 copy stmts
      // Need some forwardSub cleanup. Bail out now.
      return false;
    }
    // Get the level for ParentLoop
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
///  is not done well
bool DDUtils::findLoadInst(const DDRef *RRef,
                           SmallVectorImpl<HLInst *> &PreLoopInsts,
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

///  Traverse the Nodes outside the InnermostLoop and put them in
///  Pre / Post Vectors for later processing
bool DDUtils::enablePerfectLPGatherPrePostInsts(
    HLLoop *InnermostLoop, DDGraph DDG, SmallVectorImpl<HLInst *> &PreLoopInsts,
    SmallVectorImpl<HLInst *> &PostLoopInsts,
    SmallVectorImpl<HLInst *> &ForwardSubInsts) {

  HLLoop *ParentLoop = InnermostLoop->getParentLoop();
  unsigned NumLoops = 0;
  const DDRef *RRef;

  for (auto I1 = ParentLoop->child_begin(), E1 = ParentLoop->child_end();
       I1 != E1; ++I1) {

    if (isa<HLLoop>(I1)) {
      if (++NumLoops > 1) {
        return false;
      }
      continue;
    }

    // Save stmts PreLoop / PostLoop vector
    // Allow only Load in PreLoop and Store in PostLoop

    Instruction *LLVMInst;
    HLInst *Inst = dyn_cast<HLInst>(I1);
    if (!Inst || Inst->getNumOperands() != 2) {
      // will not move Split, etc.
      return false;
    }

    LLVMInst = const_cast<Instruction *>(Inst->getLLVMInstruction());
    assert(LLVMInst && "LLVMInst cannot be null");

    const RegDDRef *RegLRef = Inst->getLvalDDRef();
    const RegDDRef *RegRRef = Inst->getRvalDDRef();
    bool CopyStmt = false;

    if (!RegLRef) {
      return false;
    }

    if (Inst->isCopyInst()) {
      if (!findLoadInst(RegRRef, PreLoopInsts, DDG)) {
        return false;
      }
      CopyStmt = true;
    }

    if (NumLoops == 0) {
      // Allow only Load or Copy stmt in PreLoop Nodes
      if (!isa<LoadInst>(LLVMInst) && !CopyStmt) {
        return false;
      }
      PreLoopInsts.push_back(Inst);
    } else {
      // Allow only Store in PostLoop Nodes
      if (!isa<StoreInst>(LLVMInst)) {
        return false;
      }
      PostLoopInsts.push_back(Inst);
    }

    if (CopyStmt) {
      // (A)
      // t0 = a[i1];    ...     = RRef
      // ...
      // t2 = t0;       RegLRef = RegRRef     Inst
      // Save Node:  t2 = t0;
      ForwardSubInsts.push_back(Inst);
    } else if (isa<LoadInst>(LLVMInst)) {
      // (B)
      // t0 = a[i];       Inst
      RRef = Inst->getRvalDDRef();
      if (DDUtils::anyEdgeToLoop(DDG, RRef, InnermostLoop)) {
        return false;
      }
    }
  }

  return true;
}

bool DDUtils::enablePerfectLPLegalityCheckPre(
    HLLoop *InnermostLoop, DDGraph DDG, SmallVectorImpl<HLInst *> &PreLoopInsts,
    SmallVectorImpl<HLInst *> &PostLoopInsts,
    SmallVectorImpl<HLInst *> &ForwardSubInsts,
    SmallVectorImpl<HLInst *> &ValidatedStores) {

  const DDRef *LRef, *RRef;
  auto &HNU = InnermostLoop->getHLNodeUtils();

  for (auto &Inst : PreLoopInsts) {
    const Instruction *LLVMInst;

    if (std::find(ForwardSubInsts.begin(), ForwardSubInsts.end(), Inst) !=
        ForwardSubInsts.end()) {
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
    //  t0 = A[i]  ( LRef = RRef )
    //   LRef  t0 for a. else  t2
    //   Rref     No edge to the innermost loop
    //   LRef:    if there is  output edge to the innermost loop,
    //            the same store for A[x] is needed in PostLoop stmts
    LRef = Inst->getLvalDDRef();
    RRef = Inst->getRvalDDRef();
    HLInst *ForwardSInst = HNU.findForwardSubInst(LRef, ForwardSubInsts);
    LRef = ForwardSInst ? ForwardSInst->getLvalDDRef() : Inst->getLvalDDRef();
    HLInst *StoreInst = nullptr;
    if (!canMoveLoadIntoLoop(LRef, RRef, InnermostLoop, PostLoopInsts,
                             &StoreInst, DDG)) {
      return false;
    }
    if (StoreInst) {
      ValidatedStores.push_back(StoreInst);
    }
  }

  return true;
}

bool DDUtils::enablePerfectLPLegalityCheckPost(
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
    CanonExpr *CE = LRef->getSingleCanonExpr();
    assert(CE->isNonLinear() && "Unexpected linear temps");

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
      assert(ParentLoop && ParentLoop->isInnermost() &&
             "Unexpected stmt outside loop");
      RegDDRef *RegRef = dyn_cast<RegDDRef>(DDRefSink);
      CanonExpr *SinkCE = nullptr;

      if (RegRef) {
        assert(RegRef->isTerminalRef() && "Unexpected memrefs");
        SinkCE = RegRef->getSingleCanonExpr();
      } else {
        SinkCE = cast<BlobDDRef>(DDRefSink)->getMutableCanonExpr();
      }
      // There might be defs which are non-linear encountered here,
      // update it anyway
      SinkCE->setNonLinear();
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
bool DDUtils::enablePerfectLoopNest(HLLoop *InnermostLoop, DDGraph DDG) {

  HLLoop *ParentLoop = InnermostLoop->getParentLoop();
  assert(ParentLoop && "Parent Loop must not be nullptr");

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

  // Handle preheader / postexit later
  if (InnermostLoop->hasPreheader() || InnermostLoop->hasPostexit()) {
    DEBUG(dbgs() << "\n prehdr/postexit present");
    return false;
  }

  //  (1) Gather  PreLoop / PostLoop Nodes in Vector.
  //  ForwarSub nodes are  copy stmts of the for  t2 = t0
  if (!enablePerfectLPGatherPrePostInsts(InnermostLoop, DDG, PreLoopInsts,
                                         PostLoopInsts, ForwardSubInsts)) {
    DEBUG(dbgs() << "\n Fails in gatherprepost stmts");
    return false;
  }

  // (2) Perform legality  check of PreLoop nodes
  if (!enablePerfectLPLegalityCheckPre(InnermostLoop, DDG, PreLoopInsts,
                                       PostLoopInsts, ForwardSubInsts,
                                       ValidatedStores)) {
    DEBUG(dbgs() << "\n Fails in legality pre stmt check");
    return false;
  }

  // (3) Perform legality check of PostLoop nodes
  if (!enablePerfectLPLegalityCheckPost(InnermostLoop, DDG, PostLoopInsts,
                                        ValidatedStores)) {
    DEBUG(dbgs() << "\n Fails in legality post stmt check");
    return false;
  }

  auto &HNU = InnermostLoop->getHLNodeUtils();

  // (4) Move Stmts into Innermost Loop
  for (auto I = PreLoopInsts.rbegin(), E = PreLoopInsts.rend(); I != E; ++I) {
    HLNode *Node = cast<HLNode>(*I);
    HNU.moveAsFirstChild(InnermostLoop, Node);
  }
  for (auto &I : PostLoopInsts) {
    HLNode *Node = cast<HLNode>(I);
    HNU.moveAsLastChild(InnermostLoop, Node);
  }

  // Call Util to update the temp DDRefs from linear-at-level to non-linear
  updateDDRefsLinearity(PreLoopInsts, DDG);
  updateDDRefsLinearity(PostLoopInsts, DDG);

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
