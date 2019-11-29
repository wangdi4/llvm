//===-------- DDUtils.cpp - Implements DD Utilities -----------------------===//
//
// Copyright (C) 2015-2019 Intel Corporation. All rights reserved.
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

#include "llvm/Analysis/Intel_LoopAnalysis/Utils/DDUtils.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRSafeReductionAnalysis.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Framework/HIRFramework.h"
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

inline void reportFail(const char *Banner) {
  LLVM_DEBUG(dbgs() << "Can't canMoveLoadIntoLoop: " << Banner << "\n");
}

///  Return true if a load can move into the loop.
///  t0 = A[i1]; loop { };
///  In some case, moving a load into a loop requires a corresponding store
///  A[i1] = t0 to be moved into the loop also as in the case of sum reduction
///  t0 = A[i]  ( TempRef = LoadRef )
///   LoadRef      (a)  No edge to the innermost loop
///   TempRef:     (b1) Multiple uses in innermost loop
///         or  (b2) 1 def and 1 use only
///                  Same store for A[x] is needed in PostLoop stmts
///  Another case is
///  A[i1] = 0.0
///  tmp = 0.0
///  Do
///    tmp = ...
///  Enddo
///  A[i1] = tmp
///  we can sink the load tmp = A[i] to enable a perfect loop nest
///  Note:  More restrictive conditions are put here just to
///         avoid unnessary sinking
///  For Post Loop Stmts, which are all stores, we can only move inside the
///  loop if it is of the form A[i] = t0 or LHS has no DD edges
///
///  StoreNode returns the store after the loop that needs to be moved in also
///  in the case of sum reduction:
bool canMoveLoadIntoLoop(const DDRef *TempRef, const DDRef *MemRef,
                         HLLoop *InnermostLoop, DDGraph DDG,
                         const SmallVectorImpl<HLInst *> &PostLoopInsts,
                         HLInst **StoreInstPtr) {

  HLNode *StoreNode1 = nullptr;
  const DDEdge *AntiOrOutputEdge = nullptr;

  if (DDUtils::anyEdgeToLoop(DDG, MemRef, InnermostLoop)) {
    // (a) no edge into innermost Loop
    reportFail("F1");
    return false;
  }

  for (auto I1 = DDG.outgoing_edges_begin(MemRef),
            E1 = DDG.outgoing_edges_end(MemRef);
       I1 != E1; ++I1) {
    const DDEdge *Edge = *I1;
    DDRef *DDRefSink = Edge->getSink();

    auto *SinkNode = DDRefSink->getHLDDNode();
    if (SinkNode->getLexicalParentLoop() == InnermostLoop) {
      // TODO: remove this check as it is redundant with one in anyEdgeToLoop
      //       above
      reportFail("F2");
      return false;
    }

    if (MemRef->isRval() ? Edge->isAnti() : Edge->isOutput()) {
      AntiOrOutputEdge = Edge;
      StoreNode1 = SinkNode;
    }
  }

  unsigned Defs = 0;
  const DDEdge *FlowEdge = nullptr;
  HLNode *StoreNode2 = nullptr;
  const HLLoop *ParentOfInnermost = InnermostLoop->getParentLoop();
  for (auto I1 = DDG.outgoing_edges_begin(TempRef),
            E1 = DDG.outgoing_edges_end(TempRef);
       I1 != E1; ++I1) {
    const DDEdge *Edge = *I1;
    DDRef *DDRefSink = Edge->getSink();
    auto *SinkNode = DDRefSink->getHLDDNode();
    if (SinkNode == InnermostLoop) {
      // Added because of blobs
      // Could be a blob ddref in UB/LB/Stride of a loop
      LLVM_DEBUG(dbgs() << "SinkNode == InnermostLoop\n");
      LLVM_DEBUG(TempRef->dump());

      return false;
    }
    if (Edge->isOutput()) {
      if (SinkNode->getParentLoop() != InnermostLoop) {
        reportFail("F3");
        return false;
      }
      if (++Defs > 1) {
        reportFail("F4");
        return false;
      }
    } else if (Edge->isFlow()) {
      const HLLoop *ParentLoop = SinkNode->getLexicalParentLoop();
      if (ParentLoop == ParentOfInnermost) {
        FlowEdge = Edge;
        StoreNode2 = SinkNode;
      }
    }
  }

  // Defs is either 0 or 1
  // Defs == 0 means no defs within the innermost loop of this load's Lval
  // Defs == 1 means 1 def within the innermost loop of this load's Lval
  // FlowEdge and AntiOrOutputEdges are all between inst in
  // right outside the innermostloop.
  if (Defs == 1) {
    // Definition of temp inside the loop killed flow edges to post-loop
    // instructions. We will have to look for it in collected PostLoopInsts.
    if (!StoreNode2) {
      // Also handle non-self blob temps case
      auto TempBlobIndex = TempRef->isSelfBlob()
                               ? TempRef->getSelfBlobIndex()
                               : TempRef->getBlobUtils().findTempBlobIndex(
                                     TempRef->getSymbase());

      if (TempBlobIndex == InvalidBlobIndex) {
        return false;
      }

      for (auto *PostInst : PostLoopInsts) {
        assert(isa<StoreInst>(PostInst->getLLVMInstruction()) &&
               "Only stores expected in post-loop insts!");

        // Do not allow store such as B[t0] = 1 as t0 is changing inside the
        // loop.
        if (PostInst->getLvalDDRef()->usesTempBlob(TempBlobIndex)) {
          reportFail("F5");
          return false;
        }

        // We can allow multiple stores like in the example below as long as
        // there is one matching store but for now we just pick the last store
        // from post loop insts and match it with the load.
        // A[i1][i2] = t0;
        // B[i1][i2] = t0;
        if (PostInst->getRvalDDRef()->usesTempBlob(TempBlobIndex)) {
          StoreNode2 = PostInst;
        }
      }
    }

    if (StoreNode1 != StoreNode2) {
      reportFail("F6");
      // not-equal-load-store-invalid.ll is sifted here.
      // FlowDep from Load to Store exists, but no AntiDep from Store to Load
      // found.
      return false;
    }
    if (!AntiOrOutputEdge) {
      // This is a case that the load goes through 2 copy stmts
      // Need some forwardSub cleanup. Bail out now.
      reportFail("F7");
      return false;
    }

    unsigned Level = InnermostLoop->getNestingLevel() - 1;
    if (FlowEdge && FlowEdge->getDVAtLevel(Level) != DVKind::EQ) {
      reportFail("F8");
      return false;
    }
    if (AntiOrOutputEdge->getDVAtLevel(Level) != DVKind::EQ) {
      // invalid-sink.ll is sifted here.
      reportFail("F9");
      return false;
    }
  }

  if (StoreNode1) {
    *StoreInstPtr = cast<HLInst>(StoreNode1);
    // Check if this store's TempRef has a flow dependence to
    // another node other than Node (current source node)
    // See invalid-sink-2.ll
    //
    // <28>  + DO i1 = 0, sext.i32.i64(%M) + -1, 1
    // <2>   |   %0 = (@A)[0][1];
    // <5>   |   %c.030 = (@A)[0][i1 + 1];
    // <29>  |
    // <29>  |   + DO i2 = 0, sext.i32.i64(%M) + -1, 1
    // <14>  |   |   %c.030 = %0 + %c.030  +  (@B)[0][i2][i1];
    // <29>  |   + END LOOP
    // <29>  |
    // <22>  |   (@A)[0][i1 + 1] = %c.030;
    //
    //
    // DDG's==
    //     5:14 %c.030 --> %c.030 OUTPUT (*) (?)
    //     5:14 %c.030 --> %c.030 FLOW (=) (0)
    //     5:22 %c.030 --> %c.030 FLOW (=) (0)
    //     14:14 %c.030 --> %c.030 FLOW (<= *) (? ?)
    //     14:22 %c.030 --> %c.030 FLOW (*) (?)
    //     14:14 %c.030 --> %c.030 ANTI (= =) (0 0)
    //     22:14 %c.030 --> %c.030 ANTI (*) (?)
    //     2:14 %0 --> %0 FLOW (=) (0)
    //     2:22 (@A)[0][1] --> (@A)[0][i1 + 1] ANTI (=) (0)
    //     5:22 (@A)[0][i1 + 1] --> (@A)[0][i1 + 1] ANTI (=) (0)
    //     22:2 (@A)[0][i1 + 1] --> (@A)[0][1] FLOW (<) (?)
    //
    // TODO: See if the existing logic above could be reused without
    //       this special-casing. Notice FLOW edge 22:2 with (<).
    if (!DDRefUtils::areEqual((*StoreInstPtr)->getLvalDDRef(), MemRef)) {
      // invalid-sink-2.ll is sifted here.

      LLVM_DEBUG(dbgs() << "Load instruction being examined: ");
      LLVM_DEBUG(TempRef->getHLDDNode()->dump());
      LLVM_DEBUG(dbgs() << "Instruction's matching store: ");
      LLVM_DEBUG((*StoreInstPtr)->dump());

      reportFail("F10");
      return false;
    }

    // Temps in <953> (load) should match against RvalRef of <975> (store)
    // <1941> + DO i1 = 0, 15, 1
    // <953>  |   %1166 = (%s)[0].32[-1 * i1 + 15];
    // <1942> |
    // <1942> |   + DO i2 = 0, 15, 1
    // <963>  |   |   (%s)[0].31[-16 * i1 + -1 * i2 + 4095] =
    //                    (%s)[0].31[-1 * i2 + sext.i32.i64(%1166) + 15];
    // <1942> |   + END LOOP
    // <1942> |
    // <975>  |   (%s)[0].32[-1 * i1 + 15] = -16 * i1 + 4080;
    // <1941> + END LOOP
    // Comparing symbases to get integer temp case work, like %t = 0, where the
    // canon-expr of tmp is 0
    if (TempRef->getSymbase() !=
        (*StoreInstPtr)->getRvalDDRef()->getSymbase()) {
      reportFail("F11");
      return false;
    }
  }

  return true;
}

// TODO: Remove CopyStmt related logic if possible. Now HIRTempCleanup
//       takes care of those.
template <bool IsPreHeader = false>
bool gatherPreloopInsts(HLInst *Inst, HLLoop *InnermostLoop, DDGraph DDG,
                        SmallVectorImpl<HLInst *> &PreLoopInsts,
                        HLInst *&PreLoopStoreInst,
                        HLInst *&TmpInitializationInst,
                        bool AllowNonPerfectSinking) {

  if (!Inst) {
    // pre(post)loop HLNode might have not be HLInst (e.g HLIf)
    return false;
  }

  const Instruction *LLVMInst = Inst->getLLVMInstruction();
  RegDDRef *LRef = Inst->getLvalDDRef();

  // Collect store insts into a vector if non-perfect sinking is allowed
  if (AllowNonPerfectSinking && !PreLoopStoreInst && isa<StoreInst>(LLVMInst)) {

    if (DDUtils::anyEdgeToLoop(DDG, LRef, InnermostLoop)) {
      return false;
    }

    PreLoopStoreInst = Inst;

    return true;
  }

  RegDDRef *RRef = Inst->getRvalDDRef();

  if (Inst->isCopyInst()) {
    // Once copy inst has been found, compare the rval with the store inst's
    // rval. If there are equal, the copy inst will be the candidate
    // TmpInitializationInst
    if (PreLoopStoreInst &&
        DDRefUtils::areEqual(PreLoopStoreInst->getRvalDDRef(), RRef)) {
      TmpInitializationInst = Inst;
      PreLoopInsts.push_back(Inst);
      return true;
    }

    return false;
  }

  if (!isa<LoadInst>(LLVMInst)) {
    return false;
  }

  if (!IsPreHeader && InnermostLoop->hasZtt()) {
    unsigned LvalSymbase = LRef->getSymbase();
    if (Inst->getParentLoop()->isLiveOut(LvalSymbase)) {
      return false;
    }
  }

  if (DDUtils::anyEdgeToLoop(DDG, RRef, InnermostLoop)) {
    return false;
  }

  PreLoopInsts.push_back(Inst);

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
bool enablePerfectLPGatherPrePostInsts(HLLoop *InnermostLoop, DDGraph DDG,
                                       SmallVectorImpl<HLInst *> &PreLoopInsts,
                                       SmallVectorImpl<HLInst *> &PostLoopInsts,
                                       HLInst *&PreLoopStoreInst,
                                       HLInst *&TmpInitializationInst,
                                       bool AllowNonPerfectSinking) {

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
                              PreLoopInsts, PreLoopStoreInst,
                              TmpInitializationInst, AllowNonPerfectSinking)) {
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
    if (!gatherPreloopInsts<true>(
            cast<HLInst>(I), InnermostLoop, DDG, PreLoopInsts, PreLoopStoreInst,
            TmpInitializationInst, AllowNonPerfectSinking)) {
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

// Legality Check for nodes before Loop.
// Return true if legal
bool enablePerfectLPLegalityCheckPre(
    HLLoop *InnermostLoop, DDGraph DDG, SmallVectorImpl<HLInst *> &PreLoopInsts,
    const SmallVectorImpl<HLInst *> &PostLoopInsts,
    SmallVectorImpl<HLInst *> &ValidatedStores, HLInst *PreLoopStoreInst,
    HLInst *TmpInitializationInst) {

  RegDDRef *LRef, *RRef;

  for (auto &Inst : PreLoopInsts) {

    //  t = A[x]  ( LRef = RRef )
    //   RRef:     No edge to the innermost loop
    //   LRef:    if there is  output edge to the innermost loop,
    //            the same store for A[x] is needed in PostLoop stmts
    LRef = Inst->getLvalDDRef();
    RRef = Inst->getRvalDDRef();

    if (Inst == TmpInitializationInst) {
      RRef = PreLoopStoreInst->getLvalDDRef();
    } else {
      assert(isa<LoadInst>(Inst->getLLVMInstruction()) &&
             "Unexpected preloop inst!");
    }

    HLInst *StoreInst = nullptr;

    if (!canMoveLoadIntoLoop(LRef, RRef, InnermostLoop, DDG, PostLoopInsts,
                             &StoreInst)) {
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

    // If we returned false right here,
    // not-equal-load-store-valid.ll fails.
    const DDRef *LRef = Inst->getLvalDDRef();
    if (DDUtils::anyEdgeToLoop(DDG, LRef, InnermostLoop)) {
      return false;
    }

    // Profitablity check:
    // Check for the case as follows:
    //
    //   + DO i1 = 0, 9, 1   <DO_LOOP>
    //   |   + DO i2 = 0, 9, 1   <DO_LOOP>
    //   |   |   + DO i3 = 0, 9, 1   <DO_LOOP>
    //   |   |   |   %s.087 = (@a)[0][i3][i1]  +  %s.087;
    //   |   |   + END LOOP
    //   |   |
    //   |   |   (@b)[0][i2][i1] = %s.087;
    //   |   + END LOOP
    //   + END LOOP
    //
    // Sinking the store into the innermost loop is correct.
    // However, sinking only postloop insts when
    // matching preloop insts do not present,
    // usually not profitable because loop interchange and blocking won't be
    // valid anyway in most such cases.
    // Choose not to enable a perfect loop nest in such case.
    const DDRef *RRef = Inst->getRvalDDRef();
    for (auto I = DDG.incoming_edges_begin(RRef),
              E = DDG.incoming_edges_end(RRef);
         I != E; ++I) {
      const DDEdge *Edge = *I;
      DDRef *DDRefSrc = Edge->getSrc();
      if (DDRefSrc->getLexicalParentLoop() == InnermostLoop &&
          Edge->isFlow()) {
        return false;
      }
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
      if (!Edge->isFlow()) {
        continue;
      }

      DDRef *DDRefSink = Edge->getSink();
      HLDDNode *SinkDDNode = DDRefSink->getHLDDNode();
      HLLoop *ParentLoop = SinkDDNode->getParentLoop();
      (void)ParentLoop;
      assert(ParentLoop && ParentLoop->isInnermost() &&
             "Unexpected stmt outside loop");
      RegDDRef *RegRef = dyn_cast<RegDDRef>(DDRefSink);
      (void)RegRef;
      assert(!RegRef || (RegRef->isTerminalRef() && "Unexpected memrefs"));
      auto *SinkCE = DDRefSink->getSingleCanonExpr();

      // There might be defs which are non-linear encountered here,
      // update it anyway
      SinkCE->setNonLinear();
      // If DDRefSink was a blob ddref, linearity information should be
      // propagated to reg ddrefs.
      if (!RegRef) {
        unsigned InnermostLevel = ParentLoop->getNestingLevel();
        (cast<BlobDDRef>(DDRefSink))
            ->getParentDDRef()
            ->updateDefLevel(InnermostLevel);
      }
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
      for (auto BIt = Ref->blob_begin(), End = Ref->blob_end(); BIt != End;
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
    InterchangeIgnorableSymbasesTy &SinkedTempDDRefSymbases,
    bool AllowNonPerfectSinking) {
  assert(InnermostLoop->getParentLoop() && "Parent Loop must not be nullptr");

  SmallVector<HLInst *, 8> PreLoopInsts;
  SmallVector<HLInst *, 8> PostLoopInsts;
  SmallVector<HLInst *, 8> ValidatedStores;
  HLInst *PreLoopStoreInst = nullptr;
  HLInst *TmpInitializationInst = nullptr;

  //  (1) Gather  PreLoop / PostLoop Nodes in Vector.
  if (!enablePerfectLPGatherPrePostInsts(
          InnermostLoop, DDG, PreLoopInsts, PostLoopInsts, PreLoopStoreInst,
          TmpInitializationInst, AllowNonPerfectSinking)) {
    LLVM_DEBUG(dbgs() << "\n Fails in gatherprepost stmts \n");
    return false;
  }

  // (2) Perform legality  check of PreLoop nodes
  if (!enablePerfectLPLegalityCheckPre(
          InnermostLoop, DDG, PreLoopInsts, PostLoopInsts, ValidatedStores,
          PreLoopStoreInst, TmpInitializationInst)) {
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
    if (TmpInitializationInst == *I) {
      RegDDRef *TmpRef = TmpInitializationInst->getLvalDDRef();
      RegDDRef *MemRef = PreLoopStoreInst->getLvalDDRef()->clone();

      // make TmpRef a self blob to take care of the integer case %t = 0, where
      // the cannon-expr of tmp is 0
      TmpRef = TmpRef->clone();
      TmpRef->makeSelfBlob(true);
      HLInst *LoadInst =
          InnermostLoop->getHLNodeUtils().createLoad(MemRef, "Load", TmpRef);
      HLNodeUtils::insertAsFirstChild(InnermostLoop, LoadInst);

      HLNodeUtils::remove(*I);
      *I = LoadInst;

      updateLiveinsLiveoutsForSinkedInst(InnermostLoop, LoadInst, true);
    } else {
      HLNodeUtils::moveAsFirstChild(InnermostLoop, *I);
      updateLiveinsLiveoutsForSinkedInst(InnermostLoop, *I, true);
    }

    (*I)->setIsSinked(true);
  }

  for (auto &I : PostLoopInsts) {
    HLNodeUtils::moveAsLastChild(InnermostLoop, I);
    I->setIsSinked(true);
    updateLiveinsLiveoutsForSinkedInst(InnermostLoop, I, false);
  }

  // Call Util to update the temp DDRefs from linear-at-level to non-linear
  updateDDRefsLinearity(PreLoopInsts, DDG);
  updateDDRefsLinearity(PostLoopInsts, DDG);

  // Gather temp DDRef's symbase in sinked instruction
  // Loop Interchange needs this information
  gatherTempRegDDRefSymbases(PreLoopInsts, SinkedTempDDRefSymbases);
  gatherTempRegDDRefSymbases(PostLoopInsts, SinkedTempDDRefSymbases);

  InnermostLoop->setIsUndoSinkingCandidate(true);

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
  DDGraph DDG;
  HIRSafeReductionAnalysis &SRA;
  const SpecialSymbasesTy *SpecialSymbases;
  bool IgnoreSpecialSymbases;

  // Indicates if we need to call Demand Driven DD to refine DV
  bool RefineDV;

  // Outputs of this visitor
  SmallVectorImpl<DirectionVector> &DVs;

  CollectDDInfoForPermute(const HLLoop *CandidateLoop, unsigned InnermostLevel,
                          HIRDDAnalysis &DDA, HIRSafeReductionAnalysis &SRA,
                          const SpecialSymbasesTy *SpecialSBs,
                          bool IgnoreSpecialSymbases, bool RefineDV,
                          SmallVectorImpl<DirectionVector> &DVs);

  void visit(const HLDDNode *DDNode);

  void visit(const HLNode *Node) {}
  void postVisit(const HLNode *Node) {}
  void postVisit(const HLDDNode *Node) {}
};
} // namespace

CollectDDInfoForPermute::CollectDDInfoForPermute(
    const HLLoop *CandidateLoop, unsigned InnermostLevel, HIRDDAnalysis &DDA,
    HIRSafeReductionAnalysis &SRA, const SpecialSymbasesTy *SpecialSBs,
    bool IgnoreSpecialSymbases, bool RefineDV,
    SmallVectorImpl<DirectionVector> &DVs)
    : CandidateLoop(CandidateLoop),
      OutermostLevel(CandidateLoop->getNestingLevel()),
      InnermostLevel(InnermostLevel), DDA(DDA),
      DDG(DDA.getGraph(CandidateLoop)), SRA(SRA), SpecialSymbases(SpecialSBs),
      IgnoreSpecialSymbases(IgnoreSpecialSymbases), RefineDV(RefineDV),
      DVs(DVs) {
  DVs.clear();
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
    if ((*I)->isTerminalRef() && SpecialSymbases) {
      if (!IgnoreSpecialSymbases &&
          !(SpecialSymbases->count((*I)->getSymbase()))) {
        // Consider only these symbases
        continue;
      }
      if (IgnoreSpecialSymbases &&
          (SpecialSymbases->count((*I)->getSymbase()))) {
        // Ignore this symbase
        continue;
      }
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

void DDUtils::computeDVsForPermute(SmallVectorImpl<DirectionVector> &DVs,
                                   const HLLoop *OutermostLoop,
                                   unsigned InnermostNestingLevel,
                                   HIRDDAnalysis &DDA,
                                   HIRSafeReductionAnalysis &SRA,
                                   bool RefineDV) {

  CollectDDInfoForPermute CDD(OutermostLoop, InnermostNestingLevel, DDA, SRA,
                              nullptr, true, RefineDV, DVs);

  HLNodeUtils::visit(CDD, OutermostLoop);
}

void DDUtils::computeDVsForPermuteWithSBs(SmallVectorImpl<DirectionVector> &DVs,
                                          const HLLoop *OutermostLoop,
                                          unsigned InnermostNestingLevel,
                                          HIRDDAnalysis &DDA,
                                          HIRSafeReductionAnalysis &SRA,
                                          bool RefineDV,
                                          const SpecialSymbasesTy *SpecialSBs) {

  CollectDDInfoForPermute CDD(OutermostLoop, InnermostNestingLevel, DDA, SRA,
                              SpecialSBs, false, RefineDV, DVs);

  HLNodeUtils::visit(CDD, OutermostLoop);
}

void DDUtils::computeDVsForPermuteIgnoringSBs(
    SmallVectorImpl<DirectionVector> &DVs, const HLLoop *OutermostLoop,
    unsigned InnermostNestingLevel, HIRDDAnalysis &DDA,
    HIRSafeReductionAnalysis &SRA, bool RefineDV,
    const SpecialSymbasesTy *SpecialSBs) {

  CollectDDInfoForPermute CDD(OutermostLoop, InnermostNestingLevel, DDA, SRA,
                              SpecialSBs, true, RefineDV, DVs);

  HLNodeUtils::visit(CDD, OutermostLoop);
}
