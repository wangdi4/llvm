//===------ HIRSCCFormation.cpp - Identifies SCC in IRRegions -------------===//
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
// This file implements the SCC formation pass.
//
//===----------------------------------------------------------------------===//

#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"

#include "llvm/Analysis/Intel_LoopAnalysis/IR/IRRegion.h"
#include "llvm/IR/CFG.h"
#include "llvm/IR/Dominators.h"

#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"

#include "llvm/Analysis/Intel_LoopAnalysis/Framework/HIRRegionIdentification.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Framework/HIRSCCFormation.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Passes.h"

using namespace llvm;
using namespace llvm::loopopt;

#define DEBUG_TYPE "hir-scc-formation"

AnalysisKey HIRSCCFormationAnalysis::Key;

HIRSCCFormation HIRSCCFormationAnalysis::run(Function &F,
                                             FunctionAnalysisManager &AM) {
  // All the real work is done in the constructor for the HIRSCCFormation.
  return HIRSCCFormation(AM.getResult<LoopAnalysis>(F),
                         AM.getResult<DominatorTreeAnalysis>(F),
                         AM.getResult<ScalarEvolutionAnalysis>(F),
                         AM.getResult<HIRRegionIdentificationAnalysis>(F));
}

INITIALIZE_PASS_BEGIN(HIRSCCFormationWrapperPass, "hir-scc-formation",
                      "HIR SCC Formation", false, true)
INITIALIZE_PASS_DEPENDENCY(DominatorTreeWrapperPass)
INITIALIZE_PASS_DEPENDENCY(LoopInfoWrapperPass)
INITIALIZE_PASS_DEPENDENCY(ScalarEvolutionWrapperPass)
INITIALIZE_PASS_DEPENDENCY(HIRRegionIdentificationWrapperPass)
INITIALIZE_PASS_END(HIRSCCFormationWrapperPass, "hir-scc-formation",
                    "HIR SCC Formation", false, true)

char HIRSCCFormationWrapperPass::ID = 0;

FunctionPass *llvm::createHIRSCCFormationWrapperPass() {
  return new HIRSCCFormationWrapperPass();
}

bool HIRSCCFormationWrapperPass::runOnFunction(Function &) {
  // All the real work is done in the constructor for the HIRSCCFormation.
  SCCF.reset(new HIRSCCFormation(
      getAnalysis<LoopInfoWrapperPass>().getLoopInfo(),
      getAnalysis<DominatorTreeWrapperPass>().getDomTree(),
      getAnalysis<ScalarEvolutionWrapperPass>().getSE(),
      getAnalysis<HIRRegionIdentificationWrapperPass>().getRI()));
  return false;
}

HIRSCCFormationWrapperPass::HIRSCCFormationWrapperPass() : FunctionPass(ID) {
  initializeHIRSCCFormationWrapperPassPass(*PassRegistry::getPassRegistry());
}

void HIRSCCFormationWrapperPass::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.setPreservesAll();
  AU.addRequiredTransitive<DominatorTreeWrapperPass>();
  AU.addRequiredTransitive<LoopInfoWrapperPass>();
  AU.addRequiredTransitive<ScalarEvolutionWrapperPass>();
  AU.addRequiredTransitive<HIRRegionIdentificationWrapperPass>();
}

bool HIRSCCFormation::isConsideredLinear(const NodeTy *Node) const {

  if (!SE.isSCEVable(Node->getType())) {
    return false;
  }

  auto SC = SE.getSCEV(const_cast<NodeTy *>(Node));
  auto AddRecSCEV = dyn_cast<SCEVAddRecExpr>(SC);

  if (!AddRecSCEV || !AddRecSCEV->isAffine()) {
    return false;
  }

  if (!Node->getType()->isPointerTy()) {
    return true;
  }

  auto Phi = dyn_cast<PHINode>(Node);

  // Header phis can be handled by the parser.
  if (!Phi || RI.isHeaderPhi(Phi)) {
    return true;
  }

  // Check if there is a type mismatch in the primary element type for pointer
  // types.
  if (RI.getPrimaryElementType(Phi->getType()) !=
      RI.getPrimaryElementType(SC->getType())) {
    return false;
  }

  return true;
}

bool HIRSCCFormation::isCandidateRootNode(const NodeTy *Node) const {
  assert(isa<PHINode>(Node) && "Instruction is not a phi!");

  // Already visited?
  if (VisitedNodes.find(Node) != VisitedNodes.end()) {
    return false;
  }

  // Do not form SCC for IVs as the live range issues caused when IV is parsed
  // as a blob cannot be handled very cleanly. We will let the parser clean up
  // IVs.
  if (isConsideredLinear(Node)) {
    return false;
  }

  return true;
}

bool HIRSCCFormation::isLoopLiveOut(const Instruction *Inst) const {
  auto Lp = LI.getLoopFor(Inst->getParent());

  assert(Lp && "Loop is null!");

  for (auto I = Inst->user_begin(), E = Inst->user_end(); I != E; ++I) {
    auto UserInst = cast<Instruction>(*I);

    // If HIRSCCFormation is recomputed after SSA deconstruction (during testing
    // using opt, for example) we will see a liveout copy which is used outside
    // the loop instead of a direct liveout use. This check is to make sure we
    // form identical SCCs irrespective of when this is called.
    if (SE.getHIRMetadata(UserInst, ScalarEvolution::HIRLiveKind::LiveOut)) {
      return isLoopLiveOut(UserInst);
    }

    if (!Lp->contains(UserInst->getParent())) {
      return true;
    }
  }

  return false;
}

bool HIRSCCFormation::usedInHeaderPhi(const PHINode *Phi) const {
  assert(!RI.isHeaderPhi(Phi) && "Header phi not expected!");

  for (auto I = Phi->user_begin(), E = Phi->user_end(); I != E; ++I) {
    auto UserPhi = dyn_cast<PHINode>(*I);

    if (!UserPhi || !RI.isHeaderPhi(UserPhi)) {
      continue;
    }

    if (!CurLoop->contains(UserPhi->getParent())) {
      continue;
    }

    return true;
  }

  return false;
}

class BasicBlockPhiFinder {
  const PHINode *BBPhi;
  bool Found;

public:
  BasicBlockPhiFinder(const PHINode *BBPhi) : BBPhi(BBPhi), Found(false) {}

  bool follow(const SCEV *SC) {
    if (auto Blob = dyn_cast<SCEVUnknown>(SC)) {
      auto Phi = dyn_cast<PHINode>(Blob->getValue());

      if (Phi && (Phi != BBPhi) && (Phi->getParent() == BBPhi->getParent())) {
        Found = true;
      }
    }

    return true;
  }

  bool isDone() { return found(); }

  bool found() { return Found; }
};

bool HIRSCCFormation::dependsOnSameBasicBlockPhi(const PHINode *Phi) const {
  assert(RI.isHeaderPhi(Phi) && "Header phi expected!");

  if (isConsideredLinear(Phi)) {
    return false;
  }

  auto PhiBB = Phi->getParent();
  bool IsSCEVable = SE.isSCEVable(Phi->getType());

  BasicBlockPhiFinder BBPF(Phi);

  for (unsigned I = 0, E = Phi->getNumIncomingValues(); I != E; ++I) {

    auto InstOp = dyn_cast<Instruction>(Phi->getIncomingValue(I));

    if (!InstOp) {
      continue;
    }

    // Check if operand is a phi defined in the same bblock.
    if (isa<PHINode>(InstOp) && (InstOp->getParent() == PhiBB)) {
      return true;
    }

    if (!IsSCEVable) {
      continue;
    }

    // Check SCEV form of operand.
    auto SC = SE.getSCEV(const_cast<Instruction *>(InstOp));
    visitAll(SC, BBPF);

    if (BBPF.found()) {
      return true;
    }
  }

  return false;
}

bool HIRSCCFormation::hasEarlyExitPredecessor(const PHINode *Phi) const {

  // Phis in innermost loops cannot have early exit predecessors.
  if (CurLoop->empty()) {
    return false;
  }

  auto PhiLp = LI.getLoopFor(Phi->getParent());

  for (unsigned I = 0, E = Phi->getNumIncomingValues(); I < E; ++I) {
    auto PredBB = Phi->getIncomingBlock(I);

    auto PredLp = LI.getLoopFor(PredBB);

    if ((PredLp != PhiLp) && (PredBB != PredLp->getLoopLatch())) {
      return true;
    }
  }

  return false;
}

bool HIRSCCFormation::isCandidateNode(const NodeTy *Node) const {

  // Use is outside the loop bring processed.
  if (!CurLoop->contains(Node->getParent())) {
    return false;
  }

  // Phi SCCs do not have anything to do with control flow.
  if (Node->isTerminator()) {
    return false;
  }

  // Unary instruction types are alloca, cast, extractvalue, load and vaarg.
  if (isa<UnaryInstruction>(Node)) {
    // Only allow single use non-(liveout copy) instructions as they can be
    // removed as intermediate temps from the SCC and do not cause type mismatch
    // issues.
    if (!isa<CastInst>(Node) ||
        (!SE.getHIRMetadata(Node, ScalarEvolution::HIRLiveKind::LiveOut) &&
         !Node->hasOneUse())) {
      return false;
    }
  }

  // Phi SCCs do not have anything to do with memory.
  if (isa<StoreInst>(Node) || isa<AtomicCmpXchgInst>(Node) ||
      isa<AtomicRMWInst>(Node)) {
    return false;
  }

  // Phi SCCs do not have anything to do with calls.
  if (isa<CallInst>(Node) && !isa<SubscriptInst>(Node)) {
    return false;
  }

  // Phi SCCs do not have anything to do with exception handling.
  if (isa<LandingPadInst>(Node) || isa<CatchPadInst>(Node) ||
      isa<CleanupPadInst>(Node)) {
    return false;
  }

  // Ignore linear uses.
  if (isConsideredLinear(Node)) {
    return false;
  }

  // In a valid phi SCC, all phis are either header phis or used in header phis.
  auto Phi = dyn_cast<PHINode>(Node);

  if (!Phi) {
    return true;
  }

  if (RI.isHeaderPhi(Phi)) {
    return (!isLoopLiveOut(Phi) || !dependsOnSameBasicBlockPhi(Phi));
  }

  // If phi has a predecessor which is an early exit from an inner loop, then it
  // gets complicated to preserve loop simplify form if we want to split this
  // edge during SSA deconstruction so we suppress the SCC formation.
  return (usedInHeaderPhi(Phi) && !hasEarlyExitPredecessor(Phi));
}

HIRSCCFormation::NodeTy::user_iterator
HIRSCCFormation::getNextSucc(NodeTy *Node,
                             NodeTy::user_iterator PrevSucc) const {
  NodeTy::user_iterator I;

  // Called from getFirstSucc().
  if (PrevSucc == getLastSucc(Node)) {
    I = Node->user_begin();
  } else {
    I = std::next(PrevSucc);
  }

  for (auto E = getLastSucc(Node); I != E; ++I) {
    assert(isa<NodeTy>(*I) && "Use is not an instruction!");

    if (isCandidateNode(cast<NodeTy>(*I))) {
      break;
    }
  }

  return I;
}

HIRSCCFormation::NodeTy::user_iterator
HIRSCCFormation::getFirstSucc(NodeTy *Node) const {
  return getNextSucc(Node, getLastSucc(Node));
}

HIRSCCFormation::NodeTy::user_iterator
HIRSCCFormation::getLastSucc(NodeTy *Node) const {
  return Node->user_end();
}

void HIRSCCFormation::removeIntermediateNodes(SCC &CurSCC) const {

  SmallVector<NodeTy *, 8> IntermediateNodes;

  Type *RootTy = CurSCC.getRoot()->getType();
  bool IsSCEVable = SE.isSCEVable(RootTy);

  // Collect all the intermediate nodes of the SCC for removal afterwards.
  for (auto Node : CurSCC) {

    if (isa<PHINode>(Node)) {
      continue;
    }

    if (Node->getType() != RootTy) {
      assert((!isa<CastInst>(Node) || Node->hasOneUse()) &&
             "Unexpected SCC node!");
      IntermediateNodes.push_back(Node);
      continue;

    } else if (SE.getHIRMetadata(Node, ScalarEvolution::HIRLiveKind::LiveOut)) {
      // Liveout copies added by SSA deconstruction should be removed as
      // intermediate nodes.
      IntermediateNodes.push_back(Node);
      continue;
    }

    // We can remove nodes which only have non-phi uses inside the SCC or use
    // outside the region as they cannot cause live-range issues.
    // TODO : Consider eliminating this logic.
    bool IsIntermediateNode = true;
    for (auto UserIt = Node->user_begin(), E = Node->user_end(); UserIt != E;
         ++UserIt) {
      auto UserNode = cast<NodeTy>(*UserIt);

      if (CurSCC.contains(UserNode)) {
        if (isa<PHINode>(UserNode)) {
          IsIntermediateNode = false;
          break;
        }
      } else if (IsSCEVable &&
                 CurRegIt->containsBBlock(UserNode->getParent())) {
        IsIntermediateNode = false;
        break;
      }
    }

    if (IsIntermediateNode) {
      IntermediateNodes.push_back(Node);
    }
  }

  for (auto InterNode : IntermediateNodes) {
    auto NodeIt = std::find(CurSCC.begin(), CurSCC.end(), InterNode);
    assert((NodeIt != CurSCC.end()) && "SCC node not found!");
    CurSCC.remove(NodeIt);
  }
}

unsigned HIRSCCFormation::getRegionIndex(
    HIRRegionIdentification::const_iterator RegIt) const {
  return RegIt - RI.begin();
}

void HIRSCCFormation::setRegionSCCBegin() {
  if (IsNewRegion) {
    // Set begin index of current region's SCCs.
    RegionSCCBegin[getRegionIndex(CurRegIt)].first = RegionSCCs.size() - 1;

    // Set end index of last region with SCCs.
    if (LastSCCRegIt != RI.end()) {
      RegionSCCBegin[getRegionIndex(LastSCCRegIt)].second =
          RegionSCCs.size() - 1;
    }

    // Set the current region as the last region with SCCs.
    LastSCCRegIt = CurRegIt;
    IsNewRegion = false;
  }
}

void HIRSCCFormation::setRegion(HIRRegionIdentification::const_iterator RegIt) {
  CurRegIt = RegIt;
  IsNewRegion = true;
}

bool HIRSCCFormation::isUsedInSCCPhi(PHINode *Phi, const SCC &CurSCC) {
  bool UsedInPhi = false;

  for (auto I = Phi->user_begin(), E = Phi->user_end(); I != E; ++I) {
    auto UserPhi = dyn_cast<PHINode>(*I);

    if (!UserPhi) {
      continue;
    }

    if (CurSCC.contains(UserPhi)) {
      UsedInPhi = true;
      break;
    }
  }

  if (!UsedInPhi) {
    return false;
  }

  return true;
}

bool HIRSCCFormation::isRegionLiveOut(
    HIRRegionIdentification::const_iterator RegIt, const Instruction *Inst) {
  for (auto UserIt = Inst->user_begin(), EndIt = Inst->user_end();
       UserIt != EndIt; ++UserIt) {
    assert(isa<Instruction>(*UserIt) && "Use is not an instruction!");

    if (RegIt->containsBBlock(cast<Instruction>(*UserIt)->getParent())) {
      continue;
    }

    return true;
  }

  return false;
}

bool HIRSCCFormation::isMulByConstRecurrence(const SCC &CurSCC) const {
  // Looks for this pattern-
  //
  // L:
  //  t1 = phi (t2, init)
  //  ...
  //  t2 = shl t1, constant; OR t2 = mul t1, constant;
  //  ...
  //  if () {
  //    goto L:
  //  }
  //
  // -where both t1 and t2 are NOT live out of the loop.
  //
  // Suppressing such recurrences results in cleaner HIR code. When the phi is
  // deconstructed without SCC, the recurrence appears at the end of the loop so
  // it cannot cause live range issues as opposed to keeping the original
  // mul/shift instruction which can appear anywhere inside the loop. For
  // example, in the case below multiplication appears at the beginning of the
  // loop and both the old and new values are being used inside the loop.
  //
  // DO i1
  //  t.out = t
  //  t = t * 2
  //
  //    = t
  //    = t.out
  // END DO
  //
  // If we use non-SCC deconstruction, the code will look like the following
  // without the copy which is cleaner.
  //
  // DO i1
  //     = 2 * t
  //     = t
  //   t = t * 2
  // END DO
  //
  // Perhaps it would have helped if SCEV had a concept of SCEVMulRecExpr
  // similar to SCEVAddRecExpr.
  //
  // TODO: make the suppression logic more generic.

  // Recurrences (reductions of interest) in innermost loops will most likely be
  // live out of the loop which makes the suppression non-profitable.
  if (CurLoop->empty()) {
    return false;
  }

  // We only expect two instructions, one phi and one mul/shl inst.
  if (CurSCC.size() != 2) {
    return false;
  }

  auto Phi = cast<PHINode>(CurSCC.getRoot());

  auto InstIt = CurSCC.begin();

  const Instruction *MulInst = (*InstIt == Phi) ? *std::next(InstIt) : *InstIt;

  if ((MulInst->getOpcode() != Instruction::Shl) &&
      (MulInst->getOpcode() != Instruction::Mul)) {
    return false;
  }

  if (!isa<ConstantInt>(MulInst->getOperand(0)) &&
      !isa<ConstantInt>(MulInst->getOperand(1))) {
    return false;
  }

  return (!isLoopLiveOut(Phi) && !isLoopLiveOut(MulInst));
}

bool HIRSCCFormation::isProfitableSCC(const SCC &CurSCC) const {
  bool LiveoutValueFound = false;

  for (auto const &Inst : CurSCC) {

    if (isRegionLiveOut(CurRegIt, Inst)) {
      // We skip SCC formation if multiple values are live outside the region.
      // Technically, we can handle this case but it requires insertion of a
      // liveout copy during SSA deconstruction for each liveout value in the
      // SCC and it it not clear whether forming the SCC and then inserting
      // liveout copies makes HIR any cleaner than not forming the SCC at all.
      // Thus, this is more of a cost-model decision.
      if (LiveoutValueFound) {
        return false;
      }

      LiveoutValueFound = true;
    }
  }

  return !isMulByConstRecurrence(CurSCC);
}

bool HIRSCCFormation::isCmpAndSelectPattern(Instruction *Inst1,
                                            Instruction *Inst2) {
  auto CInst = Inst1;
  auto SelInst = Inst2;

  if (!isa<CmpInst>(CInst)) {
    std::swap(CInst, SelInst);
  }

  if (!isa<CmpInst>(CInst) || !isa<SelectInst>(SelInst)) {
    return false;
  }

  return (CInst->hasOneUse() && (*(CInst->user_begin()) == SelInst));
}

bool HIRSCCFormation::foundIntermediateSCCNode(
    const BasicBlock *CurBB, const Instruction *EndNode,
    const Instruction *TargetNode, const SCC &CurSCC,
    SmallPtrSet<const BasicBlock *, 8> &VisitedBBs) const {
  bool IsTargetBB = (CurBB == TargetNode->getParent());
  bool FullBBScope = (!IsTargetBB && !EndNode);

  SmallPtrSet<const NodeTy *, 4> CurBBSCCNodes;

  if (VisitedBBs.count(CurBB)) {
    return false;
  }

  // This check attempts to save compile time by traversing (likely fewer) SCC
  // instructions rather than (probably many) bblock instructions. If none of
  // the SCC nodes are defined in the current bblock, we can skip traversing it.
  for (auto Node : CurSCC) {
    if ((Node != TargetNode) && (Node != EndNode) &&
        (Node->getParent() == CurBB)) {
      if (FullBBScope) {
        return true;
      } else {
        CurBBSCCNodes.insert(Node);
      }
    }
  }

  if (!CurBBSCCNodes.empty()) {
    // Check whether any SCC nodes lie within the node range in CurBB.
    BasicBlock::const_iterator It =
        IsTargetBB ? std::next(TargetNode->getIterator()) : CurBB->begin();
    BasicBlock::const_iterator EndIt =
        EndNode ? EndNode->getIterator() : CurBB->end();

    for (; It != EndIt; ++It) {
      if (CurBBSCCNodes.count(&*It)) {
        return true;
      }
    }
  }

  if (FullBBScope) {
    VisitedBBs.insert(CurBB);
  }

  if (!IsTargetBB) {

    // Recurse on predecessors until we reach target bblock.
    for (auto PredIt = pred_begin(CurBB), E = pred_end(CurBB); PredIt != E;
         ++PredIt) {
      // Skip backedges
      if (DT.dominates(CurBB, *PredIt)) {
        continue;
      }

      if (foundIntermediateSCCNode(*PredIt, nullptr, TargetNode, CurSCC,
                                   VisitedBBs)) {
        return true;
      }
    }
  }

  return false;
}

bool HIRSCCFormation::hasLiveRangeOverlap(const NodeTy *Node,
                                          const SCC &CurSCC) const {

  auto ParentBB = Node->getParent();
  // Maintain a set of visited bblocks to cache explored paths from uses to
  // Node.
  SmallPtrSet<const BasicBlock *, 8> VisitedBBs;

  for (auto I = Node->user_begin(), E = Node->user_end(); I != E; ++I) {
    auto UserInst = cast<Instruction>(*I);

    if (!CurSCC.contains(UserInst)) {
      continue;
    }

    // Found an SCC def-use edge. Now check if another SCC node lies between the
    // def and use sites. This indicates live range violation.

    if (auto UserPhi = dyn_cast<PHINode>(UserInst)) {
      // Skip backedges
      if (DT.dominates(UserPhi->getParent(), ParentBB)) {
        continue;
      }

      // There can be multiple uses of Node in phi therefore we need to check
      // each incoming value.
      for (unsigned I = 0, E = UserPhi->getNumIncomingValues(); I < E; ++I) {
        if (UserPhi->getIncomingValue(I) != Node) {
          continue;
        }

        if (foundIntermediateSCCNode(UserPhi->getIncomingBlock(I), nullptr,
                                     Node, CurSCC, VisitedBBs)) {
          return true;
        }
      }

    } else if (foundIntermediateSCCNode(UserInst->getParent(), UserInst, Node,
                                        CurSCC, VisitedBBs)) {
      return true;
    }
  }
  return false;
}

bool HIRSCCFormation::hasLoopLiveoutUseInSCC(const Instruction *Inst,
                                             const SCC &CurSCC) const {

  auto Lp = LI.getLoopFor(Inst->getParent());

  assert(Lp && "Loop is null!");

  for (auto I = Inst->user_begin(), E = Inst->user_end(); I != E; ++I) {
    auto UserInst = cast<Instruction>(*I);

    // If HIRSCCFormation is recomputed after SSA deconstruction (during testing
    // using opt, for example) we will see a liveout copy which is used outside
    // the loop instead of a direct liveout use. This check is to make sure we
    // form identical SCCs irrespective of when this is called.
    if (SE.getHIRMetadata(UserInst, ScalarEvolution::HIRLiveKind::LiveOut)) {
      return hasLoopLiveoutUseInSCC(UserInst, CurSCC);
    }

    // Loop liveout uses of the phi need to be deconstructured using liveout
    // copies because the value going outside the loop is coming from the
    // second-last iteration of the loop. This is done correctly for uses
    // outside the SCC but not for uses inside the SCC. Instead of adding logic
    // for ssa deconstruction to take care of this, we just invalidate the SCC.
    if (CurSCC.contains(UserInst) && !Lp->contains(UserInst->getParent())) {
      return true;
    }
  }

  return false;
}

bool HIRSCCFormation::isValidSCCRootNode(const NodeTy *Root,
                                         const SCC &CurSCC) const {

  if (!Root->getType()->isIntegerTy()) {
    return true;
  }

  auto SC = SE.getSCEV(const_cast<NodeTy *>(Root));

  auto AddRec = dyn_cast<SCEVAddRecExpr>(SC);
  // Suppress SCC formation for polynomial AddRecs. The issue is that
  // ScalarEvolution is very conservative with propagating nuw/nsw flags for
  // them so they never contain any refined range information but after they get
  // converted to SCEVUnknowns by SSA deconstruction, they might contain refined
  // range information. This is because ScalaEvolution uses
  // ValueTracking::computeKnownBits() for SCEVUnknowns which can come up with
  // better info. This behavior is counterintuitive from the client's
  // perspective as forming a more conservative SCEV leads to better range
  // information. SclarEvolution has very limited support for polynomial
  // AddRecs. I suspect they are ignored by all ScalarEvolution clients.
  // TODO: Suppress creation of polynomial AddRecs in HIR mode and use HIR mode
  // for getSCEV() calls in LoopOpt codebase.
  if (AddRec && !AddRec->isAffine()) {
    return false;
  }

  // Do not form SCCs where root nodes have range info which doesn't match other
  // nodes' range info. This allows ScalarEvolution to optimize closed form
  // expressions. For example if a 32 bit value is within i8 range [0,256),
  // zext.i8.i32(trunc.i32.i8(t)) can be simplified to t. This is problematic
  // for parser which wants to substitute all occurences of temps in the SCC
  // with the base/root temp. If such simplification occurs during substitution,
  // we will form incorrect HIR.
  //
  // TODO: This seems like an artificial limitation. Can we get rid of it by
  // creating new temps like we do in HLNodeUtils::createTemp() during SSA
  // deconstruction?

  auto UnsignedRange = SE.getUnsignedRange(SC);

  if (!UnsignedRange.isFullSet()) {
    for (auto *Node : CurSCC) {
      if (Node != Root &&
          (SE.getUnsignedRange(SE.getSCEV(Node)) != UnsignedRange)) {
        return false;
      }
    }
  }

  auto SignedRange = SE.getSignedRange(SC);

  if (!SignedRange.isFullSet()) {
    for (auto *Node : CurSCC) {
      if (Node != Root &&
          (SE.getSignedRange(SE.getSCEV(Node)) != SignedRange)) {
        return false;
      }
    }
  }

  return true;
}

bool HIRSCCFormation::isValidSCC(const SCC &CurSCC) const {
  SmallPtrSet<BasicBlock *, 12> BBlocks;
  auto Root = CurSCC.getRoot();

  if (!isValidSCCRootNode(Root, CurSCC)) {
    return false;
  }

  Type *RootTy = Root->getType();
  unsigned NodeCount = CurSCC.size();

  for (auto Node : CurSCC) {

    if ((NodeCount > 2) && hasLiveRangeOverlap(Node, CurSCC)) {
      return false;
    }

    auto Phi = dyn_cast<PHINode>(Node);
    if (!Phi) {
      continue;
    }

    // Check whether all phis have the same type. There can be type mismatch if
    // we have traced through casts.
    // TODO: is it worth tracing through casts?
    if (Phi->getType() != RootTy) {
      return false;
    }

    auto ParentBB = Node->getParent();

    if (BBlocks.count(ParentBB)) {
      // If any two phis in the SCC have the same bblock parent then we
      // cannot assign the same symbase to them because they are live inside
      // the bblock at the same time, hence we invalidate the SCC. This can
      // happen in circular wrap cases. The following example generates a
      // single SCC out of a, b and c.
      //
      // for(i=0; i<n; i++) {
      //   A[i] = a;
      //   t = a;
      //   a = b;
      //   b = c;
      //   c = t;
      // }
      //
      // IR-
      //
      // for.body:
      //   %a.addr.010 = phi i32 [ %b.addr.07, %for.body ], [ %a, %entry ]
      //   %c.addr.08 = phi i32 [ %a.addr.010, %for.body ], [ %c, %entry ]
      //   %b.addr.07 = phi i32 [ %c.addr.08, %for.body ], [ %b, %entry ]
      // ...
      //
      return false;
    }

    BBlocks.insert(ParentBB);

    if (RI.isHeaderPhi(Phi)) {
      if (hasLoopLiveoutUseInSCC(Phi, CurSCC)) {
        return false;
      } else {
        continue;
      }
    }

    if (!isUsedInSCCPhi(Phi, CurSCC)) {
      return false;
    }
  }

  return true;
}

void HIRSCCFormation::updateRoot(SCC &CurSCC, NodeTy *NewRoot) const {

  if (!isa<PHINode>(NewRoot)) {
    return;
  }

  // Update blindly if NewRoot is a phi and old root is not. This avoids loop
  // lookup for single phi SCCs.
  if (!isa<PHINode>(CurSCC.getRoot())) {
    CurSCC.setRoot(NewRoot);
    return;
  }

  auto ParentBB = NewRoot->getParent();
  auto NewLp = LI.getLoopFor(ParentBB);

  // Return if NewRoot isn't a header phi.
  if (ParentBB != NewLp->getHeader()) {
    return;
  }

  auto OldLp = LI.getLoopFor(CurSCC.getRoot()->getParent());

  // If new loop contains old loop, we have found an outer loop header phi.
  if (NewLp->contains(OldLp)) {
    CurSCC.setRoot(NewRoot);
  }
}

unsigned HIRSCCFormation::findSCC(NodeTy *Node) {
  unsigned Index = GlobalNodeIndex++;
  unsigned LowLink = Index;

  // Push onto stack.
  NodeStack.push_back(Node);

  // Mark it as visited.
  auto Ret = VisitedNodes.insert(std::make_pair(Node, Index));
  (void)Ret;
  assert((Ret.second == true) && "Node has already been visited!");

  for (auto SuccIter = getFirstSucc(Node); SuccIter != getLastSucc(Node);
       SuccIter = getNextSucc(Node, SuccIter)) {
    assert(isa<NodeTy>(*SuccIter) && "Successor is not an instruction!");

    auto SuccNode = cast<NodeTy>(*SuccIter);
    auto Iter = VisitedNodes.find(SuccNode);

    // Successor hasn't been visited yet.
    if (Iter == VisitedNodes.end()) {
      // Recurse on the successor.
      auto SuccLowlink = findSCC(SuccNode);

      LowLink = std::min(LowLink, SuccLowlink);
    }
    // If this node has been visited already and has non-zero index, it belongs
    // to the current SCC.
    else if (Iter->second) {
      LowLink = std::min(LowLink, Iter->second);
    }
  }

  // This is the root of a new SCC.
  if (LowLink == Index) {

    // Ignore trivial single node SCC.
    if (Node == NodeStack.back()) {
      NodeStack.pop_back();

      // Invalidate index so node is ignored in subsequent traversals.
      VisitedNodes[Node] = 0;
    } else {
      // Create new SCC.
      SCC NewSCC(Node);
      NodeTy *SCCNode;

      // Insert Nodes in new SCC.
      do {
        SCCNode = NodeStack.pop_back_val();
        NewSCC.add(SCCNode);

        updateRoot(NewSCC, SCCNode);

        // Invalidate index so node is ignored in subsequent traverals.
        VisitedNodes[SCCNode] = 0;
      } while (SCCNode != Node);

      assert(isa<PHINode>(NewSCC.getRoot()) &&
             RI.isHeaderPhi(cast<PHINode>(NewSCC.getRoot())) &&
             "No phi found in SCC!");

      removeIntermediateNodes(NewSCC);

      if (isValidSCC(NewSCC) && isProfitableSCC(NewSCC)) {
        // Add new SCC to the list.
        RegionSCCs.push_back(std::move(NewSCC));

        // Set pointer to first SCC of region, if applicable.
        setRegionSCCBegin();
      }
    }
  }

  return LowLink;
}

void HIRSCCFormation::runImpl() {

  // Iterate through the regions.
  for (auto RegIt = RI.begin(), RegionEndIt = RI.end(); RegIt != RegionEndIt;
       ++RegIt) {

    if (RegIt->isLoopMaterializationCandidate()) {
      continue;
    }

    setRegion(RegIt);
    VisitedNodes.clear();

    auto Root = DT.getNode(RegIt->getEntryBBlock());

    // Iterate the dominator tree of the region.
    for (df_iterator<DomTreeNode *> DomIt = df_begin(Root),
                                    DomEndIt = df_end(Root);
         DomIt != DomEndIt; ++DomIt) {
      auto BB = (*DomIt)->getBlock();

      // Skip this basic block as it isn't part of the region.
      if (!RegIt->containsBBlock(BB)) {
        continue;
      }

      // We only care about loop headers as the phi cycle starts there.
      if (!LI.isLoopHeader(BB)) {
        continue;
      }

      CurLoop = LI.getLoopFor(BB);

      // Iterate through the phi nodes in the header.
      for (auto I = BB->begin(); isa<PHINode>(I); ++I) {

        if (isCandidateRootNode(&*I)) {
          findSCC(&*I);
          assert(NodeStack.empty() && "NodeStack isn't empty!");
        }
      }
    }
  }
}

HIRSCCFormation::HIRSCCFormation(LoopInfo &LI, DominatorTree &DT,
                                 ScalarEvolution &SE,
                                 HIRRegionIdentification &RI)
    : LI(LI), DT(DT), SE(SE), RI(RI), GlobalNodeIndex(1) {
  // Initialize members to default values.
  RegionSCCBegin.resize(RI.getNumRegions(), std::make_pair(NO_SCC, NO_SCC));
  LastSCCRegIt = RI.end();

  runImpl();
}

HIRSCCFormation::HIRSCCFormation(HIRSCCFormation &&SCCF)
    : LI(SCCF.LI), DT(SCCF.DT), SE(SCCF.SE), RI(SCCF.RI),
      RegionSCCs(std::move(SCCF.RegionSCCs)),
      RegionSCCBegin(std::move(SCCF.RegionSCCBegin)),
      VisitedNodes(std::move(SCCF.VisitedNodes)),
      NodeStack(std::move(SCCF.NodeStack)), CurRegIt(SCCF.CurRegIt),
      LastSCCRegIt(SCCF.LastSCCRegIt), CurLoop(SCCF.CurLoop),
      GlobalNodeIndex(SCCF.GlobalNodeIndex), IsNewRegion(SCCF.IsNewRegion) {}

HIRSCCFormation::const_iterator
HIRSCCFormation::begin(HIRRegionIdentification::const_iterator RegIt) const {
  unsigned Index = getRegionIndex(RegIt);
  int BeginOffset = RegionSCCBegin[Index].first;

  // No SCCs associated with this region, return end().
  if (BeginOffset == NO_SCC) {
    return RegionSCCs.end();
  }

  return RegionSCCs.begin() + BeginOffset;
}

HIRSCCFormation::const_iterator
HIRSCCFormation::end(HIRRegionIdentification::const_iterator RegIt) const {

  // If this is the absolute last region, blindly return end() iterator.
  if (std::next(RegIt) == RI.end()) {
    return RegionSCCs.end();
  }

  unsigned Index = getRegionIndex(RegIt);
  int EndOffset = RegionSCCBegin[Index].second;

  // No SCCs associated with this region, return end().
  if (EndOffset == NO_SCC) {
    return RegionSCCs.end();
  }

  return RegionSCCs.begin() + EndOffset;
}

void HIRSCCFormation::print(
    raw_ostream &OS, HIRRegionIdentification::const_iterator RegIt) const {
#if !INTEL_PRODUCT_RELEASE
  unsigned Count = 1;
  bool FirstSCC = true;
  auto RegBegin = RI.begin();

  for (auto SCCIt = begin(RegIt), SCCEndIt = end(RegIt); SCCIt != SCCEndIt;
       ++SCCIt, ++Count) {
    if (FirstSCC) {
      OS << "\nRegion " << RegIt - RegBegin + 1;
      FirstSCC = false;
    }

    OS << "\n   SCC" << Count << ": ";
    for (auto InstI = SCCIt->begin(), InstE = SCCIt->end(); InstI != InstE;
         ++InstI) {
      if (InstI != SCCIt->begin()) {
        OS << " -> ";
      }
      (*InstI)->printAsOperand(OS, false);
    }
  }
  // Add a newline only if we printed anything.
  if (!FirstSCC) {
    OS << "\n";
  }
#endif // !INTEL_PRODUCT_RELEASE
}

void HIRSCCFormation::print(raw_ostream &OS) const {
#if !INTEL_PRODUCT_RELEASE
  for (auto RegIt = RI.begin(), RegEndIt = RI.end(); RegIt != RegEndIt;
       ++RegIt) {
    print(OS, RegIt);
  }
#endif // !INTEL_PRODUCT_RELEASE
}
