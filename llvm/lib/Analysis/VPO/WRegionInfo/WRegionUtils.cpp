#if INTEL_COLLAB
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
//===------ WRegionUtils.cpp - WRegionNode Utils class -----*- C++ -*------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
//   This file implements the the WRegionNode Utils class.
//
//===----------------------------------------------------------------------===//

#include "llvm/ADT/SmallSet.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/Analysis/VPO/Utils/VPOAnalysisUtils.h"
#include "llvm/Analysis/VPO/WRegionInfo/WRegionCollection.h"
#include "llvm/Analysis/VPO/WRegionInfo/WRegionUtils.h"

#define DEBUG_TYPE "WRegionUtils"

using namespace llvm;
using namespace vpo;
#if INTEL_CUSTOMIZATION
using namespace loopopt;
#endif // INTEL_CUSTOMIZATION

// Return default address space for the current target.
// It is vpo::ADDRESS_SPACE_GENERIC for SPIR-V targets, 0 - otherwise.
unsigned WRegionUtils::getDefaultAS(const Module *M) {
  return VPOAnalysisUtils::isTargetSPIRV(M) ? vpo::ADDRESS_SPACE_GENERIC : 0;
}

/// Update the graph of WRegionNodes
void WRegionUtils::updateWRGraph(IntrinsicInst *Call, WRContainerImpl *WRGraph,
                                 WRStack<WRegionNode *> &S, LoopInfo *LI,
#if INTEL_CUSTOMIZATION
                                 DominatorTree *DT, BasicBlock* BB, HLNode *H)
#else
                                 DominatorTree *DT, BasicBlock* BB)
#endif // INTEL_CUSTOMIZATION
{
  if (!Call)
    return;

  WRegionNode *W = nullptr;

  // Name of the directive represented by this intrinsic
  StringRef DirString = VPOAnalysisUtils::getDirectiveString(Call);

  LLVM_DEBUG(dbgs() << "\n=== updateWRGraph found: " << DirString << "\n");

  if (VPOAnalysisUtils::isOpenMPDirective(DirString)) {

    int DirID = VPOAnalysisUtils::getDirectiveID(DirString);

    if (WRegionUtils::skipDirFromWrnConstruction(DirID))
      // Ignore DirID, which is likely a new Dir still under development
      return;

    // If the intrinsic represents a BEGIN directive, then W points to the
    // corresponding WRN.  Otherwise, W is nullptr.
#if INTEL_CUSTOMIZATION
    if (H)
      W = WRegionUtils::createWRegionHIR(DirID, H, S.size(), Call);
    else
#endif // INTEL_CUSTOMIZATION
    W = WRegionUtils::createWRegion(DirID, BB, LI, S.size(), cast<CallInst>(Call));
    if (W) {
      // The intrinsic represents a BEGIN directive.
      // W points to the WRN created for it.

      assert((VPOAnalysisUtils::isBeginDirective(DirID) ||
              VPOAnalysisUtils::isStandAloneBeginDirective(DirID)) &&
             "An expected BEGIN directive is missing.");

      if (S.empty()) {
        // Top-level WRegionNode
        WRGraph->push_back(W);
      } else {
        WRegionNode *Parent = S.top();
        Parent->getChildren().push_back(W);
        W->setParent(Parent);
      }

      S.push(W);
      LLVM_DEBUG(dbgs() << "\n  === New WRegion. ");
      LLVM_DEBUG(dbgs() << "Stacksize after push = " << S.size() << "\n");
    } else if (VPOAnalysisUtils::isEndDirective(DirID) ||
               VPOAnalysisUtils::isStandAloneEndDirective(DirID)) {
      // The intrinsic represents the END directive for the WRN that is
      // currently on S.top().
      // TODO: verify the END directive is the expected one

      assert(!(S.empty()) &&
             "Unexpected empty WRN stack when seeing an END directive");

      W = S.top();
#if INTEL_CUSTOMIZATION
      if (H)
        W->setExitHLNode(H);
      else
#endif // INTEL_CUSTOMIZATION
      W->finalize(Call, DT); // set ExitDir, ExitBB and wrap up the WRN
      S.pop();
      LLVM_DEBUG(dbgs() << "\n  === Closed WRegion. ");
      LLVM_DEBUG(dbgs() << "Stacksize after pop = " << S.size() << "\n");
    }
  }
}

/// Lookahead for a nowait clause and return true in case it finds it.
/// Used while parsing DIR_OMP_TASKWAIT, which we parse as DIR_OMP_TASK
/// in case it has a nowait clause.
bool WRegionUtils::nowaitLookahead(BasicBlock *EntryBB) {
  assert(EntryBB && "Entry Bblock is null");
  Instruction *I = &(EntryBB->front());
  assert(VPOAnalysisUtils::isOpenMPDirective(I) &&
         "Expected an OpenMP directive");
  IntrinsicInst *Call = cast<IntrinsicInst>(I);
  unsigned i, NumOB = Call->getNumOperandBundles();

  for (i = 1; i < NumOB; ++i) {
    OperandBundleUse BU = Call->getOperandBundleAt(i);
    StringRef ClauseString = BU.getTagName();
    ClauseSpecifier ClauseInfo(ClauseString);
    if (ClauseInfo.getId() == QUAL_OMP_NOWAIT)
      return true;
  }
  return false;
}

/// Create a specialized WRN based on the DirString.
/// If the string corrensponds to a BEGIN directive, then create
/// a WRN node of WRegionNodeKind corresponding to the directive,
/// and return a pointer to it. Otherwise; return nullptr.
///
/// After creating the WRN, call W->handleOperandBundles() to extract
/// the clause info from the OperandBundles and update WRN accordingly.
WRegionNode *WRegionUtils::createWRegion(int DirID, BasicBlock *EntryBB,
                                         LoopInfo *LI, unsigned NestingLevel,
                                         CallInst *Dir) {
  WRegionNode *W = nullptr;

  switch(DirID) {
    case DIR_OMP_PARALLEL:
      W = new WRNParallelNode(EntryBB);
      break;
    case DIR_OMP_PARALLEL_LOOP:
      W = new WRNParallelLoopNode(EntryBB, LI);
      break;
    case DIR_OMP_PARALLEL_SECTIONS:
      W = new WRNParallelSectionsNode(EntryBB, LI);
      break;
    case DIR_OMP_PARALLEL_WORKSHARE:   // Fortran only
      W = new WRNParallelWorkshareNode(EntryBB, LI);
      break;
    case DIR_OMP_TEAMS:
      W = new WRNTeamsNode(EntryBB);
      break;
    case DIR_OMP_DISTRIBUTE_PARLOOP:
      W = new WRNDistributeParLoopNode(EntryBB, LI);
      break;
    case DIR_OMP_INTEROP:
      W = new WRNInteropNode(EntryBB);
      break;
    case DIR_OMP_PREFETCH:
      W = new WRNPrefetchNode(EntryBB);
      break;
    case DIR_OMP_TARGET:
      W = new WRNTargetNode(EntryBB);
      break;
    case DIR_OMP_TARGET_DATA:
      W = new WRNTargetDataNode(EntryBB);
      break;
    case DIR_OMP_TARGET_ENTER_DATA:
      W = new WRNTargetEnterDataNode(EntryBB);
      break;
    case DIR_OMP_TARGET_EXIT_DATA:
      W = new WRNTargetExitDataNode(EntryBB);
      break;
    case DIR_OMP_TARGET_UPDATE:
      W = new WRNTargetUpdateNode(EntryBB);
      break;
    case DIR_OMP_TARGET_VARIANT_DISPATCH:
      W = new WRNTargetVariantNode(EntryBB);
      break;
    case DIR_OMP_DISPATCH:
      W = new WRNDispatchNode(EntryBB);
      break;
    case DIR_OMP_TASK:
      W = new WRNTaskNode(EntryBB);
      break;
    case DIR_OMP_TASKLOOP:
      W = new WRNTaskloopNode(EntryBB, LI);
      break;
    case DIR_OMP_SIMD:
#if INTEL_CUSTOMIZATION
      W = new WRNVecLoopNode(EntryBB, LI, false /* auto vec */);
      break;
    case DIR_VPO_AUTO_VEC:
      W = new WRNVecLoopNode(EntryBB, LI, true /* auto vec */);
#else
      W = new WRNVecLoopNode(EntryBB, LI);
#endif // INTEL_CUSTOMIZATION
      break;
    case DIR_OMP_LOOP:
      W = new WRNWksLoopNode(EntryBB, LI);
      break;
    case DIR_OMP_GENERICLOOP:
      W = new WRNGenericLoopNode(EntryBB, LI);
      break;
    case DIR_OMP_SECTIONS:
      W = new WRNSectionsNode(EntryBB, LI);
      break;
#if INTEL_CUSTOMIZATION
#if INTEL_FEATURE_CSA
    case DIR_OMP_SECTION:
      W = new WRNSectionNode(EntryBB);
      break;
#endif // INTEL_FEATURE_CSA
#endif // INTEL_CUSTOMIZATION
    case DIR_OMP_WORKSHARE:   // Fortran only
      W = new WRNWorkshareNode(EntryBB, LI);
      break;
    case DIR_OMP_DISTRIBUTE:
      W = new WRNDistributeNode(EntryBB, LI);
      break;
    case DIR_OMP_ATOMIC:
      W = new WRNAtomicNode(EntryBB);
      break;
    case DIR_OMP_BARRIER:
      W = new WRNBarrierNode(EntryBB);
      break;
    case DIR_OMP_CANCEL:
      W = new WRNCancelNode(EntryBB, false);
      break;
    case DIR_OMP_CANCELLATION_POINT:
      W = new WRNCancelNode(EntryBB, true);
      break;
    case DIR_OMP_MASTER: // TODO: Remove once CFE starts generating MASKED for
                         // MASTER.
    case DIR_OMP_MASKED:
      W = new WRNMaskedNode(EntryBB);
      break;
    case DIR_OMP_ORDERED:
      W = new WRNOrderedNode(EntryBB);
      break;
    case DIR_OMP_SINGLE:
      W = new WRNSingleNode(EntryBB);
      break;
    case DIR_OMP_CRITICAL:
      W = new WRNCriticalNode(EntryBB);
      break;
    case DIR_OMP_FLUSH:
      W = new WRNFlushNode(EntryBB);
      break;
    case DIR_OMP_TASKGROUP:
      W = new WRNTaskgroupNode(EntryBB);
      break;
    case DIR_OMP_TASKWAIT:
      if (nowaitLookahead(EntryBB)) {
        // nowait on Taskwait depend is parsed as taskwait depend
        W = new WRNTaskNode(EntryBB);
        W->setIsTaskwaitNowaitTask(true);
        DirID = DIR_OMP_TASK;
        break;
      }
      W = new WRNTaskwaitNode(EntryBB);
      break;
    case DIR_OMP_TASKYIELD:
      W = new WRNTaskyieldNode(EntryBB);
      break;
    case DIR_OMP_THREADPRIVATE:
      // #pragma omp threadprivate can be a module-level directive so we
      // handle it outside of the WRN framework
      break;
    case DIR_OMP_SCOPE:
      W = new WRNScopeNode(EntryBB);
      break;
    case DIR_VPO_GUARD_MEM_MOTION:
      W = new WRNGuardMemMotionNode(EntryBB);
      break;
    case DIR_OMP_TILE:
      W = new WRNTileNode(EntryBB, LI);
      break;
    case DIR_OMP_SCAN:
      W = new WRNScanNode(EntryBB);
      break;
  }
  if (W) {
    W->setLevel(NestingLevel);
    W->setDirID(DirID);
    W->setEntryDirective(Dir);
    W->getClausesFromOperandBundles();
  }
  return W;
}

#if INTEL_CUSTOMIZATION
/// \brief Similar to createWRegion, but for HIR vectorizer support
WRegionNode *WRegionUtils::createWRegionHIR(int DirID,
                                            loopopt::HLNode *EntryHLNode,
                                            unsigned NestingLevel,
                                            IntrinsicInst *Call) {
  WRegionNode *W = nullptr;

  switch(DirID) {
    case DIR_OMP_PARALLEL_LOOP:
      W = new WRNParallelLoopNode(EntryHLNode);
      break;
    // TODO: complete the list for all WRegionNodeKinds needed
    //       to support vectorizer
    case DIR_OMP_SIMD:
      W = new WRNVecLoopNode(EntryHLNode, false /* auto vec */);
      break;
    case DIR_VPO_AUTO_VEC:
      W = new WRNVecLoopNode(EntryHLNode, true /* auto vec */);
      break;
  }
  if (W) {
    W->setLevel(NestingLevel);
    W->setDirID(DirID);
    W->getClausesFromOperandBundles(Call, cast<HLInst>(EntryHLNode));
  }
  return W;
}

/// \brief Visitor class to walk the HIR and build WRNs
/// based on HIR. Main logic is in the visit() member function
/// This visitor class is intended to be instantiated and used
/// only by WRegionUtils::buildWRGraphFromHIR().
struct HIRVisitor final : public HLNodeVisitorBase {
  WRContainerImpl *WRGraph;
  WRStack<WRegionNode*> S;
  HIRVisitor() : WRGraph(new WRContainerTy){}

  WRContainerImpl *getWRGraph() const { return WRGraph; }
  void visit(loopopt::HLNode *Node);
  void postVisit(loopopt::HLNode *) {}
};

void HIRVisitor::visit(loopopt::HLNode *Node) {
  // Node->dump();
  if (HLInst *HI = dyn_cast<HLInst>(Node)) {
    const Instruction *I = HI->getLLVMInstruction();
    Instruction *II = const_cast<Instruction *> (I);
    IntrinsicInst* Call = dyn_cast<IntrinsicInst>(II);
    if (Call)
      WRegionUtils::updateWRGraph(Call, WRGraph, S,
                                  nullptr, nullptr, nullptr, // LI, DT, BB
                                  Node);
  }
  else if (HLLoop *L = dyn_cast<HLLoop>(Node)) {
    // Found a loop L; check if there's a pending loop-type WRN such as
    // WRNVecLoop that still has empty getHLLoop() and needs updating
    if (!S.empty()) {
      WRegionNode *W = S.top();
      if (W->getIsOmpLoop() && !W->getHLLoop()) {
        W->setHLLoop(L);
        if (isa<WRNVecLoopNode>(W)) {
          // If the loop is marked with vector always pragma, mark that we
          // should ignore vectorization profitability in vectorizer cost
          // model.
          W->setHasVectorAlways(L->hasVectorizeAlwaysPragma());
          // TODO: Update to use "pragma vector vectorlength(N)" when support is
          // added in FE.
          unsigned PragmaBasedVF = L->getVectorizePragmaWidth();
          unsigned LoopOptForcedVF = L->getForcedVectorWidth();
          // VF forced via pragma takes higher precedence than LoopOpt's
          // internal forced VF.
          if (PragmaBasedVF > 0) {
            assert(!W->getSimdlen() && "Cannot overwrite SIMD length value.");
            W->setSimdlen(PragmaBasedVF);
          } else if (LoopOptForcedVF > 0) {
            assert(!W->getSimdlen() && "Cannot overwrite SIMD length value.");
            W->setSimdlen(LoopOptForcedVF);
          }
        }
      }
    }
  }
}

WRContainerImpl *WRegionUtils::buildWRGraphFromHIR(HIRFramework &HIRF)
{
  HIRVisitor Visitor;

  HIRF.getHLNodeUtils().visitAll(Visitor);
  return Visitor.getWRGraph();
}

bool WRegionUtils::supportsRegDDRefs(int ClauseID) {

  if (VPOAnalysisUtils::isReductionClause(ClauseID))
    return true;

  switch (ClauseID) {
  case QUAL_OMP_FIRSTPRIVATE:
  case QUAL_OMP_LASTPRIVATE:
  case QUAL_OMP_LINEAR:
  case QUAL_OMP_PRIVATE:
    return true;
  default:
    return false;
  }
}
#endif // INTEL_CUSTOMIZATION

// New OpenMP directives under development can be added to this routine
// to make the WRN graph builder skip them instead of asserting.
bool WRegionUtils::skipDirFromWrnConstruction(int DirID) {
  switch (DirID) {
#if INTEL_CUSTOMIZATION
#if INTEL_FEATURE_CSA
#else // INTEL_FEATURE_CSA
  case DIR_OMP_SECTION:
  case DIR_OMP_END_SECTION:
#endif // INTEL_FEATURE_CSA
#else // INTEL_CUSTOMIZATION
  case DIR_OMP_SECTION:
  case DIR_OMP_END_SECTION:
#endif // INTEL_CUSTOMIZATION
#if INTEL_CUSTOMIZATION
  case DIR_PRAGMA_IVDEP:
  case DIR_PRAGMA_END_IVDEP:
  case DIR_PRAGMA_BLOCK_LOOP:
  case DIR_PRAGMA_END_BLOCK_LOOP:
  case DIR_PRAGMA_DISTRIBUTE_POINT:
  case DIR_PRAGMA_END_DISTRIBUTE_POINT:
#endif // INTEL_CUSTOMIZATION
    return true;
  }
  // Example:
  // if (DirID == A_NEW_DIRECTIVE_UNDER_DEVELOPMENT)
  //   // remove from this routine when directive is fully implemented
  //   return true;
  return false;
}

// Clause Utilities
int WRegionUtils::getClauseIdFromAtomicKind(WRNAtomicKind Kind) {
  switch (Kind) {
  case WRNAtomicUpdate:
    return QUAL_OMP_UPDATE;
  case WRNAtomicRead:
    return QUAL_OMP_READ;
  case WRNAtomicWrite:
    return QUAL_OMP_WRITE;
  case WRNAtomicCapture:
    return QUAL_OMP_CAPTURE;
  }
  llvm_unreachable("Unsupported Atomic Kind");

}

// Get the induction variable of the OMP loop; if not found, either assert or
// return nullptr depending on AssertIfIVNotFound.
PHINode *
WRegionUtils::getOmpCanonicalInductionVariable(Loop *L,
                                               bool AssertIfIVNotFound) {
  assert(L && "getOmpCanonicalInductionVariable: null loop");
  BasicBlock *H = L->getHeader();
  assert(H && "getOmpCanonicalInductionVariable: null loop header");

  BasicBlock *Incoming = nullptr, *Backedge = nullptr;
  pred_iterator PI = pred_begin(H);
  // TBD: messages will become warnings or opt report messages.
  assert(PI != pred_end(H) &&
         "Omp loop must have at least one backedge!");
  Backedge = *PI++;
  if (PI == pred_end(H))
    llvm_unreachable("Omp loop is dead loop");
  Incoming = *PI++;
  if (PI != pred_end(H))
    llvm_unreachable("Omp loop has multiple backedges");

  if (L->contains(Incoming)) {
    if (L->contains(Backedge))
      llvm_unreachable("Omp loop cannot have both incoming and backedge BB!");
    std::swap(Incoming, Backedge);
  } else if (!L->contains(Backedge))
    llvm_unreachable("Omp loop cannot have neither incoming nor backedge BB");

  for (BasicBlock::iterator I = H->begin(); isa<PHINode>(I); ++I) {
    PHINode *PN = cast<PHINode>(I);
    if (Instruction *Inc =
            dyn_cast<Instruction>(PN->getIncomingValueForBlock(Backedge))) {
      if ((Inc->getOpcode() == Instruction::Add ||
           Inc->getOpcode() == Instruction::Sub) &&
          (Inc->getOperand(0) == PN || Inc->getOperand(1) == PN)) {
        // The compiler locates the bottom test expression of the loop
        // and tests whether the loop index is in the bottom
        // test expression.
        Instruction *TermInst = L->getLoopLatch()->getTerminator();
        BranchInst *ExitBrInst = dyn_cast<BranchInst>(TermInst);
        if (!ExitBrInst)
          continue;
        ICmpInst *CondInst = dyn_cast<ICmpInst>(ExitBrInst->getCondition());
        if (!CondInst)
          continue;
        bool IsLeft;
        if (!getLoopIndexPosInPredicate(Inc, CondInst, IsLeft))
          continue;
        return PN;
      }
    }
  }
  if (AssertIfIVNotFound)
    llvm_unreachable("Omp loop must have induction variable!");

  return nullptr;
}

// gets the loop lower bound of the OMP loop.
Value *WRegionUtils::getOmpLoopLowerBound(Loop *L) {
  PHINode *PN = getOmpCanonicalInductionVariable(L);
  assert(PN != nullptr && "Omp loop must have induction variable!");
  assert(L->getLoopPreheader() && "Omp loop must have preheader!");

  return PN->getIncomingValueForBlock(L->getLoopPreheader());
}

// gets the loop stride of the OMP loop.
Value *WRegionUtils::getOmpLoopStride(Loop *L, bool &IsNeg) {
  PHINode *PN = getOmpCanonicalInductionVariable(L);
  assert(PN != nullptr && "Omp loop must have induction variable!");

  if (Instruction *Inc =
      dyn_cast<Instruction>(PN->getIncomingValueForBlock(L->getLoopLatch())))
    if ((Inc->getOpcode() == Instruction::Add ||
         Inc->getOpcode() == Instruction::Sub) &&
        (Inc->getOperand(0) == PN || Inc->getOperand(1) == PN)) {
      if (Inc->getOpcode() == Instruction::Sub)
        IsNeg = true;
      else
        IsNeg = false;
      if (Inc->getOperand(0) == PN)
        return Inc->getOperand(1);
      else
        return Inc->getOperand(0);
    }
  llvm_unreachable("Omp loop must have stride!");
}

// Get the position of the given loop index at
// the bottom/zero trip test expression. It returns false if
// it cannot find the loop index.
bool WRegionUtils::getLoopIndexPosInPredicate(Value *LoopIndex,
                                              Instruction *CondInst,
                                              bool &IsLeft) {
  Value *Operand = CondInst->getOperand(0);
  if (isa<SExtInst>(Operand) || isa<ZExtInst>(Operand))
    Operand = cast<Instruction>(Operand)->getOperand(0);

  if (Operand == LoopIndex) {
      IsLeft = true;
      return true;
  }

  Operand = CondInst->getOperand(1);
  if (isa<SExtInst>(Operand) || isa<ZExtInst>(Operand))
    Operand = cast<Instruction>(Operand)->getOperand(0);

  if (Operand == LoopIndex) {
      IsLeft = false;
      return true;
  }
  return false;
}

// gets the loop upper bound of the OMP loop.
Value *WRegionUtils::getOmpLoopUpperBound(Loop *L) {
  ICmpInst *CondInst;
  Value *Res;

  CondInst = getOmpLoopBottomTest(L);
  PHINode *PN = getOmpCanonicalInductionVariable(L);
  Instruction *Inc =
    dyn_cast<Instruction>(PN->getIncomingValueForBlock(L->getLoopLatch()));
  bool IsLeft;
  getLoopIndexPosInPredicate(Inc, CondInst, IsLeft);
  if (IsLeft)
    Res = CondInst->getOperand(1);
  else
    Res = CondInst->getOperand(0);

  return Res;
}

// gets the zero trip test of the OMP loop if the zero trip
// test exists.
ICmpInst *WRegionUtils::getOmpLoopZeroTripTest(Loop *L, BasicBlock *EntryBB) {

  BasicBlock *PB = L->getLoopPreheader();
  if (pred_empty(PB) || std::distance(pred_begin(PB), pred_end(PB)) != 1)
    return nullptr;
  do {
    if (PB == EntryBB || pred_empty(PB))
      return nullptr;
    PB = *(pred_begin(PB));
    if (std::distance(succ_begin(PB), succ_end(PB))==2)
      break;
    // The basic block should have only one successor.
    if (std::distance(succ_begin(PB), succ_end(PB)) != 1)
      return nullptr;
  } while (PB);
  if (!PB)
    return nullptr;
  for (BasicBlock::reverse_iterator J = PB->rbegin();
       J != PB->rend(); ++J) {
    ICmpInst *CondInst = dyn_cast<ICmpInst>(&*J);
    if (CondInst && ICmpInst::isRelational(CondInst->getPredicate())) {
      bool IsLeft;
      getLoopIndexPosInPredicate(getOmpLoopLowerBound(L), CondInst, IsLeft);
      return CondInst;
    }
  }
  return nullptr;
}

// gets the bottom test of the OMP loop.
ICmpInst *WRegionUtils::getOmpLoopBottomTest(Loop *L) {
  PHINode *PN = getOmpCanonicalInductionVariable(L);
  assert(PN != nullptr && "Omp loop must have induction variable!");
  (void) PN;
  assert(L->isLoopExiting(L->getLoopLatch()) &&
         "Omp loop must have been rotated!");

  assert(isa<BranchInst>(&*L->getLoopLatch()->rbegin()) &&
         "Cannot find Exit Branch Instruction.");
  BranchInst *ExitBrInst = cast<BranchInst>(&*L->getLoopLatch()->rbegin());

  ICmpInst *CondInst = dyn_cast<ICmpInst>(ExitBrInst->getCondition());
  if (CondInst && ICmpInst::isRelational(CondInst->getPredicate()))
    return CondInst;

  llvm_unreachable("Omp loop must have bottom test!");
}

// gets the exit block of the OMP loop. The OMP loop may contain exit
// call. The existing LoopInfo returns two exit blocks. The utility
// is to handle this situation.
BasicBlock *WRegionUtils::getOmpExitBlock(Loop* L) {
  assert(isa<BranchInst>(&*L->getLoopLatch()->rbegin()) &&
         "Cannot find Exit Branch Instruction.");
  BranchInst *ExitBrInst = cast<BranchInst>(&*L->getLoopLatch()->rbegin());

  for (unsigned I = 0; I < ExitBrInst->getNumSuccessors(); I++) {
    if (ExitBrInst->getSuccessor(I) != L->getHeader())
      return ExitBrInst->getSuccessor(I);
  }
  llvm_unreachable("Omp loop must have one exit block");
}

// gets the predicate for the bottom test.
CmpInst::Predicate WRegionUtils::getOmpPredicate(Loop* L, bool& IsLeft) {
  assert(isa<BranchInst>(&*L->getLoopLatch()->rbegin()) &&
         "Null Exit Branch Instruction.");
  BranchInst *ExitBrInst = cast<BranchInst>(&*L->getLoopLatch()->rbegin());

  assert(isa<ICmpInst>(ExitBrInst->getCondition()) &&
         "Omp loop must have cmp instruction at the end!");
  ICmpInst *CondInst = cast<ICmpInst>(ExitBrInst->getCondition());

  PHINode *PN = getOmpCanonicalInductionVariable(L);
  Instruction *Inc =
    dyn_cast<Instruction>(PN->getIncomingValueForBlock(L->getLoopLatch()));

  getLoopIndexPosInPredicate(Inc, CondInst, IsLeft);

  return CondInst->getPredicate();
}

PrivateItem *WRegionUtils::wrnSeenAsPrivate(WRegionNode *W, Value *V) {
  PrivateClause &PrivClause = W->getPriv();
  return PrivClause.findOrig(V);
}

FirstprivateItem *WRegionUtils::wrnSeenAsFirstprivate(const WRegionNode *W,
                                                      const Value *V) {
  const FirstprivateClause &FprivClause = W->getFpriv();
  return FprivClause.findOrig(V);
}

LastprivateItem *WRegionUtils::wrnSeenAsLastprivate(WRegionNode *W, Value *V) {
  LastprivateClause &LprivClause = W->getLpriv();
  return LprivClause.findOrig(V);
}

ReductionItem *WRegionUtils::wrnSeenAsReduction(WRegionNode *W, Value *V) {
  ReductionClause &RedClause = W->getRed();
  return RedClause.findOrig(V);
}

MapItem *WRegionUtils::wrnSeenAsMap(WRegionNode *W, Value *V) {
  MapClause &Map = W->getMap();
  for (MapItem *I : Map.items()) {
    Value *Orig = I->getOrig();
    if (Orig != nullptr) { // scalar
      if (Orig == V)
        return I;
    } else { // aggregate
      MapChainTy &MapChain = I->getMapChain();
      for (MapAggrTy *Aggr : MapChain) {
        Value *BasePtr = Aggr->getBasePtr();
        if (BasePtr == V)
          return I;
        // break;
        // Note: OMP4.5 doesn't allow firstprivate/lastprivate of struct
        // fields or array sections, so to see if the fp/lp item is also in a
        // map clause for an aggregate, we only need to look at the head of the
        // chain (ie, the first BasePtr). For now, I'm letting it traverse the
        // entire chain, which is typically very short anyway.
      }
    }
  }
  return nullptr;
}

// The utility checks whether the given value is used at the region
// entry directive.
bool WRegionUtils::usedInRegionEntryDirective(WRegionNode *W, Value *I) {
  for (auto IB = I->user_begin(), IE = I->user_end(); IB != IE; IB++)
    if (Instruction *User = dyn_cast<Instruction>(*IB))
      if (VPOAnalysisUtils::isOpenMPDirective(User) &&
          User->getParent() == W->getEntryBBlock())
        return true;
  return false;
}

// Returns true if V is used in the WRN W. Populates UserInsts and UserExprs
// with the users if they are not null.
//
// Prerequisite: W's BBSet must be populated before calling this util.
bool WRegionUtils::findUsersInRegion(
    WRegionNode *W, Value *V, SmallVectorImpl<Instruction *> *UserInsts,
    bool ExcludeEntryDirective, SmallPtrSetImpl<ConstantExpr *> *UserExprs) {
  bool Found = false;
  for (User *U : V->users()) {
    if (Instruction *I = dyn_cast<Instruction>(U)) {
      if (ExcludeEntryDirective && I == W->getEntryDirective())
        continue;
      if (W->contains(I->getParent())) {
        // LLVM_DEBUG(dbgs() << "findUsersInRegion ("<< *V <<") in ("<< *I
        // <<")\n");
        if (UserInsts == nullptr)
          return true; // no need to find more users
        Found = true;
        UserInsts->push_back(I);
      }
    }
    else if (ConstantExpr *CE = dyn_cast<ConstantExpr>(U)) {
      if (UserExprs)
        UserExprs->insert(CE);
      // LLVM_DEBUG(dbgs() << "  ConstantExpr: " << *CE << "\n");
      //
      // The user may not be an Instruction, but a ConstantExpr used directly
      // in an instruction. Example (IR from ompoC/priv9a-4.c): the user of @u
      // is not the load instruction, but the GEP expr in the load:
      //
      //     %12 = load i32, i32* getelementptr inbounds (%struct.t_union_,
      //           %struct.t_union_* @u, i32 0, i32 0), align 4
      //
      // Recursively call findUsersInRegion() to find all Instructions in \p W
      // that use the ConstantExpr and add such Instructions to \p *Users.
      if (WRegionUtils::findUsersInRegion(W, CE, UserInsts,
                                          ExcludeEntryDirective, UserExprs)) {
        if (UserInsts == nullptr)
          return true; // no need to find more users
        Found = true;
      }
    }
    // else
    //  LLVM_DEBUG(dbgs() << "Not an Instruction or ConstantExpr:" << *U <<
    //  "\n");
  }
  return Found;
}

// The utility to create the loop and update the loopinfo.
Loop *WRegionUtils::createLoop(Loop *L, Loop *PL, LoopInfo *LI) {
  Loop *New = LI->AllocateLoop();
  if (PL)
    PL->replaceChildLoopWith(L, New);
  else
    LI->changeTopLevelLoop(L, New);

  New->addChildLoop(L);
  for (Loop::block_iterator I = L->block_begin(), E = L->block_end(); I != E;
       ++I)
    New->addBlockEntry(*I);

  return New;
}

// The utility to add the given BB into the loop.
void WRegionUtils::updateBBForLoop(BasicBlock *BB, Loop *L, Loop *PL,
                                   LoopInfo *LI) {
  if (PL)
    LI->removeBlock(BB);
  L->addBasicBlockToLoop(BB, *LI);
}

// Return true if the given loop is do-while loop.
bool WRegionUtils::isDoWhileLoop(Loop *L) {
  return getLoopType(L) == DoWhileLoop;
}

// Return true if the given loop is while loop.
bool WRegionUtils::isWhileLoop(Loop *L) {
  return getLoopType(L) == WhileLoop;
}

// Return the loop type (do-while, while loop or unknown loop) for the given
// loop.
unsigned WRegionUtils::getLoopType(Loop *L) {

  if (!L)
    return UnknownLoop;

  BasicBlock *H = L->getHeader();

  if (!H)
    return UnknownLoop;

  BasicBlock *Incoming = nullptr, *Backedge = nullptr;
  pred_iterator PI = pred_begin(H);

  if (PI == pred_end(H))
    return UnknownLoop;

  Backedge = *PI++;
  if (PI == pred_end(H))
    return UnknownLoop;

  Incoming = *PI++;
  if (PI != pred_end(H))
    return UnknownLoop;

  if (L->contains(Incoming)) {
    if (L->contains(Backedge))
      return UnknownLoop;
    std::swap(Incoming, Backedge);
  } else if (!L->contains(Backedge))
    return UnknownLoop;

  BasicBlock *LatchBB = L->getLoopLatch();

  if (LatchBB != Backedge)
    return UnknownLoop;

  if (LatchBB->getUniqueSuccessor()) {
    if (std::distance(succ_begin(H), succ_end(H)) != 2)
      return UnknownLoop;
    for (auto SI = succ_begin(H), SE = succ_end(H); SI != SE; ++SI) {
      if (!L->contains(*SI))
        return WhileLoop;
    }
    return UnknownLoop;
  }
  return DoWhileLoop;
}

// Return true if destructors are needed for privatized variables
bool WRegionUtils::needsDestructors(WRegionNode *W) {
  if (W->canHavePrivate())
    for (PrivateItem *PI : W->getPriv().items())
      if (PI->getDestructor() != nullptr)
        return true;

  if (W->canHaveFirstprivate())
    for (FirstprivateItem *FI : W->getFpriv().items())
      if (FI->getDestructor() != nullptr)
        return true;

  if (W->canHaveLastprivate())
    for (LastprivateItem *LI : W->getLpriv().items())
      if (LI->getDestructor() != nullptr)
        return true;

  if (W->canHaveReduction())
    for (ReductionItem *RI : W->getRed().items())
      if (RI->getDestructor() != nullptr)
        return true;

  return false;
}

bool WRegionUtils::hasCancelConstruct(WRegionNode *W) {
  assert(W && "hasCancelConstruct: null WRegionNode");

  if (!W->canHaveCancellationPoints())
    return false;

  if (!W->getCancellationPoints().empty())
    return true;

  for (auto *Child : W->getChildren()) {
    if (auto *CancelNode = dyn_cast<WRNCancelNode>(Child))
      if (!CancelNode->getIsCancellationPoint())
        return true;
  }
  return false;
}

// Return nullptr if W has no parent of the specified kind.
WRegionNode *WRegionUtils::getParentRegion(const WRegionNode *W,
                                           unsigned WRegionKind) {
  while (W) {
    WRegionNode *ParentRegion = W->getParent();
    if (!ParentRegion)
      break;
    if (WRegionKind == ParentRegion->getWRegionKindID())
      return ParentRegion;
    W = ParentRegion;
  }
  return nullptr;
}

WRegionNode *WRegionUtils::getParentRegion(
    const WRegionNode *W,
    std::function<bool(const WRegionNode *)> IsMatch,
    std::function<bool(const WRegionNode *)> ProcessNext) {
  while (W) {
    WRegionNode *ParentRegion = W->getParent();
    if (!ParentRegion)
      break;
    // Check if the ancestor satisfies the condition.
    if (IsMatch(ParentRegion))
      return ParentRegion;
    // Check if next ancestor needs to be processed.
    if (!ProcessNext(ParentRegion))
      break;
    W = ParentRegion;
  }
  return nullptr;
}

// Search the WRNs in the container for a Target construct.
// The container can be the top-level WRGraph or the Children of a WRN.
bool WRegionUtils::hasTargetDirective(WRContainerImpl &WrnContainer) {
  for (WRegionNode *W : WrnContainer) {
    if (isa<WRNTargetNode>(W))
      return true;
    if (hasTargetDirective(W->getChildren()))
      return true;
  }
  return false;
}

bool WRegionUtils::hasTargetDirective(WRegionInfo *WI) {
  WRContainerImpl *WRGraph = WI->getWRGraph();
  if (WRGraph)
    return hasTargetDirective(*WRGraph);
  return false;
}

bool WRegionUtils::hasLexicalParentTarget(const WRegionNode *W) {
  WRegionNode *PW = W->getParent();
  while (PW) {
    if (isa<WRNTargetNode>(PW))
      return true;

    PW = PW->getParent();
  }
  return false;
}

bool WRegionUtils::hasParentTarget(const WRegionNode *W) {
  Function *F = W->getEntryDirective()->getFunction();
  if (F->getAttributes().hasFnAttr("target.declare") ||
      F->getAttributes().hasFnAttr("openmp-target-declare"))
    return true;

  return hasLexicalParentTarget(W);
}

// Returns true iff W contains a WRN for which Predicate is true.
bool WRegionUtils::containsWRNsWith(
    WRegionNode *W, std::function<bool(WRegionNode *)> Predicate) {
  if (!W->hasChildren())
    return false;

  SmallVector<WRegionNode *, 32> ContainedWRNs{W->wrn_child_begin(),
                                               W->wrn_child_end()};
  while (!ContainedWRNs.empty()) {
    auto *W = ContainedWRNs.pop_back_val();

    if (Predicate(W))
      return true;

    if (!W->hasChildren())
      continue;

    ContainedWRNs.append(W->wrn_child_begin(), W->wrn_child_end());
  }
  return false;
}

WRNVecLoopNode *WRegionUtils::getEnclosedSimdForSameLoop(WRegionNode *W,
                                                         unsigned Idx) {
  assert(W->getIsOmpLoop() && "Expected a loop-type WRN");
  Loop *L = W->getWRNLoopInfo().getLoop(Idx);
  for (auto *Child : W->getChildren()) {
    if (WRNVecLoopNode *VecNode =  dyn_cast<WRNVecLoopNode>(Child)) {
      Loop *VecLoop = VecNode->getWRNLoopInfo().getLoop(Idx);
      if (L == VecLoop)
        return VecNode;
    }
  }
  return nullptr;
}

// Return true for WRNs representing stand-alone directives; ie, those
// that do not have an associated code region.
bool WRegionUtils::isStandAlone(WRegionNode *W) {
  assert (W && "Null WRegionNode");
  switch(W->getWRegionKindID()) {
    case WRegionNode::WRNBarrier:
    case WRegionNode::WRNCancel:
    case WRegionNode::WRNFlush:
    case WRegionNode::WRNTargetEnterData:
    case WRegionNode::WRNTargetExitData:
    case WRegionNode::WRNTargetUpdate:
    case WRegionNode::WRNTaskwait:
    case WRegionNode::WRNTaskyield:
      return true;
    case WRegionNode::WRNOrdered:
      // Ordered constructs are stand-alone if they are for doacross
      return W->getIsDoacross();
  }
  return false;
}

// For WRegions that need outlining, collect non-pointer values that
// will be used inside the outlined function once it is created.
void WRegionUtils::collectNonPointerValuesToBeUsedInOutlinedRegion(
    WRegionNode *W) {

  if (!W->needsOutlining())
    return;

  if (!isa<WRNParallelNode>(W) && !isa<WRNParallelLoopNode>(W) &&
      !isa<WRNParallelSectionsNode>(W) && !isa<WRNTargetNode>(W) &&
      !isa<WRNDistributeParLoopNode>(W))
    // TODO: Remove this to enable the function for all outlined WRNs.
    //
    // While this condition is here it should match with one in
    // VPOParoptTransform::captureAndAddCollectedNonPointerValuesToSharedClause.
    return;

  SmallSet<Value *, 16> AlreadyCollected;

  auto isNonPointer = [](Value *V) -> bool {
    return (V && !isa<PointerType>(V->getType()));
  };

  auto isNonPointerNonConstant = [](Value *V) -> bool {
    if (!V || isa<Constant>(V) || isa<PointerType>(V->getType()))
      return false;
    return true;
  };

  auto collectIfNotAlreadyCollected = [&](Value *V) {
    if (isa<WRNTargetNode>(W)) {
      // For target constructs, we need to capture the values a
      // deterministic number of times to avoid mismatch between host
      // and device compilation. Even if that means capturing the same
      // value twice. For example, we need to mark %n for collection twice
      // for the following:
      //   %vla1 = alloca i32, i64 %n
      //   %vla2 = alloca i32, i64 %n
      //   "PRIVATE"(i32* %vla1, i32* %vla2)
      W->addDirectlyUsedNonPointerValue(V);
      return;
    }

    if (AlreadyCollected.find(V) != AlreadyCollected.end())
      return;

    AlreadyCollected.insert(V);
    W->addDirectlyUsedNonPointerValue(V);
  };

  auto collectIfNonPointerNonConstant = [&](Value *V) {
    if (!isNonPointerNonConstant(V))
      return;
    collectIfNotAlreadyCollected(V);
  };

  auto collectIfNonPointer = [&](Value *V, bool CollectEvenIfConstant = false) {
    if (!CollectEvenIfConstant)
      return collectIfNonPointerNonConstant(V);
    if (!isNonPointer(V))
      return;
    collectIfNotAlreadyCollected(V);
  };

  auto collectIfUsedInRegion = [&](Value *V) {
    if (!isNonPointerNonConstant(V))
      return;

    W->populateBBSet(); // Populate BBSet if not already populated.
    if (!WRegionUtils::findUsersInRegion(W, V))
      return;

    collectIfNotAlreadyCollected(V);
  };

  auto collectSizeIfAlloca = [&](Value *V, bool CollectEvenIfConstant = false) {
    // Skip AddrSpaceCastInsts in hope to reach the underlying value.
    while (auto *ASCI = dyn_cast<AddrSpaceCastInst>(V))
      V = ASCI->getPointerOperand();

    if (AllocaInst *AI = dyn_cast<AllocaInst>(V)) {
      if (AI->isArrayAllocation())
        collectIfNonPointer(AI->getArraySize(), CollectEvenIfConstant);
      else if (CollectEvenIfConstant)
        collectIfNonPointer(
            ConstantInt::get(Type::getInt64Ty(V->getContext()), 1),
            CollectEvenIfConstant);
    }
  };

  auto collectArraySectionBounds = [&](const ArraySectionInfo &ASI) {
    const auto &ArrSecDims = ASI.getArraySectionDims();
    for (auto &Dim : ArrSecDims) {
      collectIfNonPointerNonConstant(std::get<0>(Dim)); // lb
      collectIfNonPointerNonConstant(std::get<1>(Dim)); // size
      collectIfNonPointerNonConstant(std::get<2>(Dim)); // stride
    }
  };

  auto collectEvenIfConstant = [&](Item *I) -> bool {
    // It's possible for CSE to promote some variables to constants before
    // Paropt. For target regions, we need to capture such constants to avoid
    // mismatch between host/device optimization pipelines.
    return isa<WRNTargetNode>(W) && I->getIsVarLen();
  };

  auto collectTypedNumElementsOrVlaSize = [&](Item *I) {
    bool CollectEvenIfConstant = collectEvenIfConstant(I);

    if (I->getIsTyped()) {
      collectIfNonPointer(I->getNumElements(), CollectEvenIfConstant);
      return;
    }
    // We need to capture VLA size even if it's not currently used
    // in the region because we'll use it for the allocation of
    // the private copy of the clause operand.
    collectSizeIfAlloca(I->getOrig(), CollectEvenIfConstant);
  };

  if (W->canHavePrivate()) {
    PrivateClause &PrivClause = W->getPriv();
    for (PrivateItem *PrivI : PrivClause.items())
      collectTypedNumElementsOrVlaSize(PrivI);
  }

  if (W->canHaveFirstprivate()) {
    FirstprivateClause &FprivClause = W->getFpriv();
    for (FirstprivateItem *FprivI : FprivClause.items())
      collectTypedNumElementsOrVlaSize(FprivI);
  }

  if (W->canHaveReduction()) {
    ReductionClause &RedClause = W->getRed();
    for (ReductionItem *RedI : RedClause.items()) {
      // TODO: This needs to be restructured when adding typed array-section
      // reduction support.
      if (RedI->getIsTyped())
        collectIfNonPointerNonConstant(RedI->getNumElements());
      if (RedI->getIsArraySection())
        collectArraySectionBounds(RedI->getArraySectionInfo());
      else if (RedI->getIsTyped())
        collectIfNonPointerNonConstant(RedI->getArraySectionOffset());
      else
        collectSizeIfAlloca(RedI->getOrig());
    }
  }

  if (W->canHaveLastprivate()) {
    LastprivateClause &LprivClause = W->getLpriv();
    for (LastprivateItem *LprivI : LprivClause.items())
      collectTypedNumElementsOrVlaSize(LprivI);
  }

  if (W->canHaveLinear()) {
    LinearClause &LrClause = W->getLinear();
    for (LinearItem *LrI : LrClause.items())
      collectIfNonPointerNonConstant(LrI->getStep());
  }

  if (W->canHaveMap()) {
    MapClause &MapClause = W->getMap();
    // For map clause items, there is no private allocation needed inside the
    // region for the item itself, but we may need to capture the base's VLA
    // size if it is used inside the region, for example, for private
    // initialization of a firstprivate clause on an inner parallel construct.
    // But it may or may not be used depending on how the inner construct is
    // handled on that target. So, we need to unconditionally pass it in.
    // TODO: OPAQUEPOINTER: We can remove the capturing of VLA size for map
    // operands for opaque pointers because the typed clauses required for
    // opaque pointers have explicit usage of the size value, and it is expected
    // to be explicitly captured on any outer construct by the frontends.
    for (MapItem *MapI : MapClause.items()) {
      // If the map item already has a non-typed private/fp clause, its VLA
      // size would have been collected by the private/fp clause handling above.
      if (const PrivateItem *PI = MapI->getInPrivate())
        if (!PI->getIsTyped())
          continue;

      if (const FirstprivateItem *FPI = MapI->getInFirstprivate())
        if (!FPI->getIsTyped())
          continue;

      collectSizeIfAlloca(MapI->getOrig(), collectEvenIfConstant(MapI));
    }
  }

  if (W->canHaveIf())
    collectIfUsedInRegion(W->getIf());

  // Maybe it is a better idea to request capturing the tripcounts
  // in front-ends, but we can do it here as well.
  if (W->canHaveOrderedTripCounts())
    for (Value *V : W->getOrderedTripCounts())
      collectIfNonPointerNonConstant(V);

  if (W->canHaveDistSchedule())
    collectIfNonPointerNonConstant(W->getDistSchedule().getChunkExpr());

  LLVM_DEBUG(
      const auto &CollectedVals = W->getDirectlyUsedNonPointerValues();
      if (CollectedVals.empty()) {
        dbgs() << __FUNCTION__
               << ": No non-pointer values to be passed into the outlined "
                  "region.\n";
      } else {
        dbgs()
            << __FUNCTION__
            << ": Non-pointer values to be passed into the outlined region: '";
        for (const Value *V : CollectedVals) {
          V->printAsOperand(dbgs());
          dbgs() << " ";
        }
        dbgs() << "'\n";
      });
}

AllocateItem *WRegionUtils::getAllocateItem(Item *I) {
  if (auto *PrivI = dyn_cast<PrivateItem>(I))
    return PrivI->getInAllocate();
  else if (auto *FprivI = dyn_cast<FirstprivateItem>(I))
    return FprivI->getInAllocate();
  else if (auto *LprivI = dyn_cast<LastprivateItem>(I))
    return LprivI->getInAllocate();
  else if (auto *RedI = dyn_cast<ReductionItem>(I))
    return RedI->getInAllocate();
  return nullptr;
}

bool WRegionUtils::isDistributeNode(const WRegionNode *W) {
  return isa<WRNDistributeNode>(W) ||
         (isa<WRNDistributeParLoopNode>(W) &&
          W->getTreatDistributeParLoopAsDistribute());
}

bool WRegionUtils::isDistributeParLoopNode(const WRegionNode *W) {
  return isa<WRNDistributeParLoopNode>(W) &&
         !W->getTreatDistributeParLoopAsDistribute();
}

template <typename ItemTy>
Item *WRegionUtils::getClauseItemForInscanIdx(const Clause<ItemTy> &C,
                                              uint64_t Idx) {
  auto FoundIter = llvm::find_if(
      C.items(), [Idx](ItemTy *I) { return I->getInscanIdx() == Idx; });
  if (FoundIter == C.end())
    return nullptr;

  return *FoundIter;
}

Item *WRegionUtils::getClauseItemForInscanIdx(const WRegionNode *W,
                                              uint64_t Idx) {
  if (W->canHaveReductionInscan())
    if (auto *FoundItem = getClauseItemForInscanIdx(W->getRed(), Idx))
      return FoundItem;

  if (W->canHaveInclusive())
    if (auto *FoundItem = getClauseItemForInscanIdx(W->getInclusive(), Idx))
      return FoundItem;

  if (W->canHaveExclusive())
    if (auto *FoundItem = getClauseItemForInscanIdx(W->getExclusive(), Idx))
      return FoundItem;

  return nullptr;
}

InclusiveExclusiveItemBase *
WRegionUtils::getInclusiveExclusiveItemForReductionItem(
    const WRegionNode *W, const ReductionItem *I) {

  if (!I->getIsInscan())
    return nullptr;

  auto ScanNodeIt = llvm::find_if(
      W->getChildren(), [](WRegionNode *Cur) { return isa<WRNScanNode>(Cur); });

  assert(ScanNodeIt != W->getChildren().end() &&
         "No scan directive in a region with a reduction(inscan) item.");

  Item *Res =
      WRegionUtils::getClauseItemForInscanIdx(*ScanNodeIt, I->getInscanIdx());
  assert(Res && "Reduction(inscan) operand has no matching clause on a child "
                "Scan directive.");

  assert((isa<InclusiveItem>(Res) || isa<ExclusiveItem>(Res)) &&
         "Reduction(inscan) item matches an unexpected clause on the child "
         "Scan directive.");

  InclusiveExclusiveItemBase *IEItem = dyn_cast<InclusiveItem>(Res);
  return (IEItem ? IEItem : cast<ExclusiveItem>(Res));
}

ReductionItem *WRegionUtils::getReductionItemForInclusiveExclusiveItem(
    const WRNScanNode *W, const InclusiveExclusiveItemBase *I) {

  auto *Parent = W->getParent();
  assert(Parent->canHaveReductionInscan() &&
         "scan directive should be perfectly nested in a region that supports "
         "reduction(inscan).");

  Item *Res =
      WRegionUtils::getClauseItemForInscanIdx(Parent, I->getInscanIdx());
  assert(Res && "inclusive/exclusive operand has no matching clause on the "
                "parent construct.");
  return cast<ReductionItem>(Res);
}

#endif // INTEL_COLLAB
