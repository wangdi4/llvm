//===------ HIRSCCFormation.cpp - Identifies SCC in IRRegions -------------===//
//
// Copyright (C) 2015-2016 Intel Corporation. All rights reserved.
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

#include "llvm/Pass.h"

#include "llvm/IR/CFG.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/Intel_LoopIR/IRRegion.h"

#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"

#include "llvm/Analysis/Intel_LoopAnalysis/HIRRegionIdentification.h"
#include "llvm/Analysis/Intel_LoopAnalysis/HIRSCCFormation.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Passes.h"

using namespace llvm;
using namespace llvm::loopopt;

#define DEBUG_TYPE "hir-scc-formation"

INITIALIZE_PASS_BEGIN(HIRSCCFormation, "hir-scc-formation", "HIR SCC Formation",
                      false, true)
INITIALIZE_PASS_DEPENDENCY(DominatorTreeWrapperPass)
INITIALIZE_PASS_DEPENDENCY(LoopInfoWrapperPass)
INITIALIZE_PASS_DEPENDENCY(ScalarEvolutionWrapperPass)
INITIALIZE_PASS_DEPENDENCY(HIRRegionIdentification)
INITIALIZE_PASS_END(HIRSCCFormation, "hir-scc-formation", "HIR SCC Formation",
                    false, true)

char HIRSCCFormation::ID = 0;

FunctionPass *llvm::createHIRSCCFormationPass() {
  return new HIRSCCFormation();
}

HIRSCCFormation::HIRSCCFormation() : FunctionPass(ID), GlobalNodeIndex(1) {
  initializeHIRSCCFormationPass(*PassRegistry::getPassRegistry());
}

void HIRSCCFormation::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.setPreservesAll();
  AU.addRequiredTransitive<DominatorTreeWrapperPass>();
  AU.addRequiredTransitive<LoopInfoWrapperPass>();
  AU.addRequiredTransitive<ScalarEvolutionWrapperPass>();
  AU.addRequiredTransitive<HIRRegionIdentification>();
}

bool HIRSCCFormation::isConsideredLinear(const NodeTy *Node) const {

  if (!SE->isSCEVable(Node->getType())) {
    return false;
  }

  auto SC = SE->getSCEV(const_cast<NodeTy *>(Node));
  auto AddRecSCEV = dyn_cast<SCEVAddRecExpr>(SC);

  if (!AddRecSCEV || !AddRecSCEV->isAffine()) {
    return false;
  }

  if (!Node->getType()->isPointerTy()) {
    return true;
  }

  auto Phi = dyn_cast<PHINode>(Node);

  // Header phis can be handled by the parser.
  if (!Phi || RI->isHeaderPhi(Phi)) {
    return true;
  }

  // Check if there is a type mismatch in the primary element type for pointer
  // types.
  if (RI->getPrimaryElementType(Phi->getType()) !=
      RI->getPrimaryElementType(SC->getType())) {
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
  auto Lp = LI->getLoopFor(Inst->getParent());

  assert(Lp && "Loop is null!");

  for (auto I = Inst->user_begin(), E = Inst->user_end(); I != E; ++I) {
    auto UserInst = cast<Instruction>(*I);

    // If HIRSCCFormation is recomputed after SSA deconstruction (during testing
    // using opt, for example) we will see a liveout copy which is used outside
    // the loop instead of a direct liveout use. This check is to make sure we
    // form identical SCCs irrespective of when this is called.
    if (SE->getHIRMetadata(UserInst, ScalarEvolution::HIRLiveKind::LiveOut)) {
      return isLoopLiveOut(UserInst);
    }

    if (!Lp->contains(UserInst->getParent())) {
      return true;
    }
  }

  return false;
}

bool HIRSCCFormation::usedInHeaderPhi(const PHINode *Phi) const {
  assert(!RI->isHeaderPhi(Phi) && "Header phi not expected!");

  for (auto I = Phi->user_begin(), E = Phi->user_end(); I != E; ++I) {
    auto UserPhi = dyn_cast<PHINode>(*I);

    if (!UserPhi || !RI->isHeaderPhi(UserPhi)) {
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
  assert(RI->isHeaderPhi(Phi) && "Header phi expected!");

  if (isConsideredLinear(Phi)) {
    return false;
  }

  auto PhiBB = Phi->getParent();
  bool IsSCEVable = SE->isSCEVable(Phi->getType());

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
    auto SC = SE->getSCEV(const_cast<Instruction *>(InstOp));
    visitAll(SC, BBPF);

    if (BBPF.found()) {
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
  if (isa<TerminatorInst>(Node)) {
    return false;
  }

  // Unary instruction types are alloca, cast, extractvalue, load and vaarg.
  if (isa<UnaryInstruction>(Node)) {
    // Only allow single use non-(liveout copy) instructions as they can be
    // removed as intermediate temps from the SCC and do not cause type mismatch
    // issues.
    if (!isa<CastInst>(Node) ||
        (!SE->getHIRMetadata(Node, ScalarEvolution::HIRLiveKind::LiveOut) &&
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
  if (isa<CallInst>(Node)) {
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

  if (RI->isHeaderPhi(Phi)) {
    return (!isLoopLiveOut(Phi) || !dependsOnSameBasicBlockPhi(Phi));
  }

  return usedInHeaderPhi(Phi);
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
  bool IsSCEVable = SE->isSCEVable(RootTy);

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

    } else if (SE->getHIRMetadata(Node,
                                  ScalarEvolution::HIRLiveKind::LiveOut)) {
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
  return RegIt - RI->begin();
}

void HIRSCCFormation::setRegionSCCBegin() {
  if (isNewRegion) {
    // Set begin index of current region's SCCs.
    RegionSCCBegin[getRegionIndex(CurRegIt)].first = RegionSCCs.size() - 1;

    // Set end index of last region with SCCs.
    if (LastSCCRegIt != RI->end()) {
      RegionSCCBegin[getRegionIndex(LastSCCRegIt)].second =
          RegionSCCs.size() - 1;
    }

    // Set the current region as the last region with SCCs.
    LastSCCRegIt = CurRegIt;
    isNewRegion = false;
  }
}

void HIRSCCFormation::setRegion(HIRRegionIdentification::const_iterator RegIt) {
  CurRegIt = RegIt;
  isNewRegion = true;
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

  return true;
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

bool HIRSCCFormation::dominatesInSameBB(const Instruction *Inst1,
                                        const Instruction *Inst2,
                                        const Instruction *EndInst) {
  auto ParentBB = Inst1->getParent();

  assert((Inst1 != Inst2) &&
         "Inst1 and Inst2 should be different instructions!");
  assert((ParentBB == Inst2->getParent()) &&
         "Inst1 and Inst2 are not in same bblock!");

  for (auto It = std::next(Inst1->getIterator()), EndIt = ParentBB->end();
       It != EndIt; ++It) {
    if (&*It == EndInst) {
      break;
    }

    if (&*It == Inst2) {
      return true;
    }
  }

  return false;
}

bool HIRSCCFormation::hasLiveRangeOverlap(const NodeTy *Node,
                                          const SCC &CurSCC) const {

  auto ParentBB = Node->getParent();

  for (auto I = Node->user_begin(), E = Node->user_end(); I != E; ++I) {
    auto UserInst = cast<Instruction>(*I);

    if (!CurSCC.contains(UserInst)) {
      continue;
    }

    // Found an SCC def-use edge. Now check if another SCC node lies between the
    // def and use sites. This indicates live range violation.
    auto UserParentBB = UserInst->getParent();
    for (auto SCCNode : CurSCC) {

      // Ignore def and use nodes.
      if ((SCCNode == Node) || (SCCNode == UserInst)) {
        continue;
      }

      auto NodeBB = SCCNode->getParent();

      if (NodeBB == ParentBB) {
        // Does SCCNode lie between def and use in the def node bblock?
        if (dominatesInSameBB(Node, SCCNode, UserInst)) {
          return true;
        }
      } else if (NodeBB == UserParentBB) {
        // Does SCCNode lie between def and use in the use node bblock?
        if (dominatesInSameBB(SCCNode, UserInst, nullptr)) {
          return true;
        }
      }
    }
  }
  return false;
}

bool HIRSCCFormation::hasLoopLiveoutUseInSCC(const Instruction *Inst,
                                             const SCC &CurSCC) const {

  auto Lp = LI->getLoopFor(Inst->getParent());

  assert(Lp && "Loop is null!");

  for (auto I = Inst->user_begin(), E = Inst->user_end(); I != E; ++I) {
    auto UserInst = cast<Instruction>(*I);

    // If HIRSCCFormation is recomputed after SSA deconstruction (during testing
    // using opt, for example) we will see a liveout copy which is used outside
    // the loop instead of a direct liveout use. This check is to make sure we
    // form identical SCCs irrespective of when this is called.
    if (SE->getHIRMetadata(UserInst, ScalarEvolution::HIRLiveKind::LiveOut)) {
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

bool HIRSCCFormation::isValidSCC(const SCC &CurSCC) const {
  SmallPtrSet<BasicBlock *, 12> BBlocks;
  Type *RootTy = CurSCC.getRoot()->getType();
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

    if (RI->isHeaderPhi(Phi)) {
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
  auto NewLp = LI->getLoopFor(ParentBB);

  // Return if NewRoot isn't a header phi.
  if (ParentBB != NewLp->getHeader()) {
    return;
  }

  auto OldLp = LI->getLoopFor(CurSCC.getRoot()->getParent());

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
             RI->isHeaderPhi(cast<PHINode>(NewSCC.getRoot())) &&
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

void HIRSCCFormation::formRegionSCCs() {

  // Iterate through the regions.
  for (auto RegIt = RI->begin(), RegionEndIt = RI->end(); RegIt != RegionEndIt;
       ++RegIt) {

    setRegion(RegIt);
    VisitedNodes.clear();

    auto Root = DT->getNode(RegIt->getEntryBBlock());

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
      if (!LI->isLoopHeader(BB)) {
        continue;
      }

      CurLoop = LI->getLoopFor(BB);

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

bool HIRSCCFormation::runOnFunction(Function &F) {

  LI = &getAnalysis<LoopInfoWrapperPass>().getLoopInfo();
  DT = &getAnalysis<DominatorTreeWrapperPass>().getDomTree();
  SE = &getAnalysis<ScalarEvolutionWrapperPass>().getSE();
  RI = &getAnalysis<HIRRegionIdentification>();

  // Initialize members to defualt values.
  RegionSCCBegin.resize(RI->getNumRegions(), std::make_pair(NO_SCC, NO_SCC));
  LastSCCRegIt = RI->end();

  formRegionSCCs();

  return false;
}

void HIRSCCFormation::releaseMemory() {
  GlobalNodeIndex = 1;
  isNewRegion = false;

  RegionSCCs.clear();
  RegionSCCBegin.clear();
  VisitedNodes.clear();
  NodeStack.clear();
}

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
  if (std::next(RegIt) == RI->end()) {
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
  unsigned Count = 1;
  bool FirstSCC = true;
  auto RegBegin = RI->begin();

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
}

void HIRSCCFormation::print(raw_ostream &OS, const Module *M) const {

  for (auto RegIt = RI->begin(), RegEndIt = RI->end(); RegIt != RegEndIt;
       ++RegIt) {
    print(OS, RegIt);
  }
}

void HIRSCCFormation::verifyAnalysis() const {
  // TODO: implement later
}
