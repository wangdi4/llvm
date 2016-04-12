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
  if (isa<UnaryInstruction>(Node) && !isa<CastInst>(Node)) {
    return false;
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

  if (!Phi || RI->isHeaderPhi(Phi)) {
    return true;
  }

  // Do not involve single operand phis in SCCs.
  if (1 == Phi->getNumIncomingValues()) {
    return false;
  }

  bool IsUsedInHeaderPhi = false;
  for (auto I = Phi->user_begin(), E = Phi->user_end(); I != E; ++I) {
    auto UserPhi = dyn_cast<PHINode>(*I);

    if (!UserPhi || !RI->isHeaderPhi(UserPhi)) {
      continue;
    }

    if (!CurLoop->contains(UserPhi->getParent())) {
      continue;
    }

    IsUsedInHeaderPhi = true;
    break;
  }

  if (!IsUsedInHeaderPhi) {
    return false;
  }

  return true;
}

HIRSCCFormation::NodeTy::const_user_iterator
HIRSCCFormation::getNextSucc(const NodeTy *Node,
                             NodeTy::const_user_iterator PrevSucc) const {
  NodeTy::const_user_iterator I;

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

HIRSCCFormation::NodeTy::const_user_iterator
HIRSCCFormation::getFirstSucc(const NodeTy *Node) const {
  return getNextSucc(Node, getLastSucc(Node));
}

HIRSCCFormation::NodeTy::const_user_iterator
HIRSCCFormation::getLastSucc(const NodeTy *Node) const {
  return Node->user_end();
}

void HIRSCCFormation::removeIntermediateNodes(SCCNodesTy &CurSCC) {

  SmallVector<const NodeTy *, 4> IntermediateNodes;

  for (auto NodeIt = CurSCC.begin(), NodeEndIt = CurSCC.end();
       NodeIt != NodeEndIt; ++NodeIt) {
    if (isa<PHINode>(*NodeIt)) {
      continue;
    }

    bool IsUsedInPhi = false;
    // Check whether this non-phi instruction is used in any phi contained in
    // the SCC.
    for (auto UseIt = (*NodeIt)->user_begin(), UseEndIt = (*NodeIt)->user_end();
         UseIt != UseEndIt; ++UseIt) {
      auto Use = cast<NodeTy>(*UseIt);

      if (!isa<PHINode>(Use)) {
        continue;
      }

      if (!CurSCC.count(Use)) {
        continue;
      }

      IsUsedInPhi = true;
    }

    if (!IsUsedInPhi) {
      IntermediateNodes.push_back(*NodeIt);
    }
  }

  for (auto &I : IntermediateNodes) {
    CurSCC.erase(I);
  }
}

unsigned HIRSCCFormation::getRegionIndex(
    HIRRegionIdentification::const_iterator RegIt) const {
  return RegIt - RI->begin();
}

void HIRSCCFormation::setRegionSCCBegin() {
  if (isNewRegion) {
    // Set the index of the last RegionSCC element as the current region's first
    // SCC.
    RegionSCCBegin[getRegionIndex(CurRegIt)] = RegionSCCs.size() - 1;
    isNewRegion = false;
  }
}

void HIRSCCFormation::setRegion(HIRRegionIdentification::const_iterator RegIt) {
  CurRegIt = RegIt;
  isNewRegion = true;
}

bool HIRSCCFormation::isUsedInSCCPhi(const PHINode *Phi,
                                     const SCCNodesTy &Nodes) const {
  bool UsedInPhi = false;

  for (auto I = Phi->user_begin(), E = Phi->user_end(); I != E; ++I) {
    auto UserPhi = dyn_cast<PHINode>(*I);

    if (!UserPhi) {
      continue;
    }

    if (Nodes.count(UserPhi)) {
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

    if ((*RegIt)->containsBBlock(cast<Instruction>(*UserIt)->getParent())) {
      continue;
    }

    return true;
  }

  return false;
}

bool HIRSCCFormation::isProfitableSCC(const SCCNodesTy &Nodes) const {
  bool LiveoutValueFound = false;

  for (auto const &Inst : Nodes) {

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

bool HIRSCCFormation::isValidSCC(const SCCNodesTy &Nodes) const {
  SmallPtrSet<const BasicBlock *, 12> BBlocks;
  Type *NodeTy = nullptr;

  for (auto const &Inst : Nodes) {

    // Check whether all the nodes have the same type. There can be type
    // mismatch if we have traced through casts.
    // TODO: is it worth tracing through casts?
    if (!NodeTy) {
      NodeTy = Inst->getType();

    } else if (NodeTy != Inst->getType()) {
      return false;
    }

    auto Phi = dyn_cast<PHINode>(Inst);

    if (!Phi) {
      continue;
    }

    auto ParentBB = Inst->getParent();

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

    // No further validation needed for header phi.
    if (RI->isHeaderPhi(Phi)) {
      continue;
    }

    if (!isUsedInSCCPhi(Phi, Nodes)) {
      return false;
    }
  }

  return true;
}

unsigned HIRSCCFormation::findSCC(const NodeTy *Node) {
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
      SCCTy *NewSCC = new SCCTy(Node);
      auto &NewSCCNodes = NewSCC->Nodes;
      const NodeTy *SCCNode;
      bool isRootPhi = isa<PHINode>(Node);

      // Insert Nodes in new SCC.
      do {
        SCCNode = NodeStack.pop_back_val();
        NewSCCNodes.insert(SCCNode);

        // If the root of this SCC is not a phi, it may get eliminated as an
        // intermediate node which results in a dangling root node. To fix this
        // we set the first phi we encounter to be the root node.
        if (!isRootPhi && isa<PHINode>(SCCNode)) {
          NewSCC->Root = SCCNode;
          isRootPhi = true;
        }

        // Invalidate index so node is ignored in subsequent traverals.
        VisitedNodes[SCCNode] = 0;
      } while (SCCNode != Node);

      assert(isRootPhi && "No phi found in SCC!");

      // Remove nodes not directly associated with the phi nodes.
      removeIntermediateNodes(NewSCCNodes);

      if (isValidSCC(NewSCCNodes) && isProfitableSCC(NewSCCNodes)) {
        // Add new SCC to the list.
        RegionSCCs.push_back(NewSCC);

        // Set pointer to first SCC of region, if applicable.
        setRegionSCCBegin();

      } else {
        // Not a valid SCC.
        delete NewSCC;
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

    auto Root = DT->getNode((*RegIt)->getEntryBBlock());

    // Iterate the dominator tree of the region.
    for (df_iterator<DomTreeNode *> DomIt = df_begin(Root),
                                    DomEndIt = df_end(Root);
         DomIt != DomEndIt; ++DomIt) {
      auto BB = (*DomIt)->getBlock();

      // Skip this basic block as it isn't part of the region.
      if (!(*RegIt)->containsBBlock(BB)) {
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

  // Initialize to NO_SCC.
  RegionSCCBegin.resize(RI->getNumRegions(), NO_SCC);

  formRegionSCCs();

  return false;
}

void HIRSCCFormation::releaseMemory() {
  GlobalNodeIndex = 1;
  isNewRegion = false;

  for (auto &I : RegionSCCs) {
    delete I;
  }

  RegionSCCs.clear();
  RegionSCCBegin.clear();
  VisitedNodes.clear();
  NodeStack.clear();
}

HIRSCCFormation::const_iterator
HIRSCCFormation::begin(HIRRegionIdentification::const_iterator RegIt) const {
  unsigned Index = getRegionIndex(RegIt);
  int BeginOffset = RegionSCCBegin[Index];

  // No SCCs associated with this region, return end().
  if (BeginOffset == NO_SCC) {
    return RegionSCCs.end();
  }

  return RegionSCCs.begin() + BeginOffset;
}

HIRSCCFormation::const_iterator
HIRSCCFormation::end(HIRRegionIdentification::const_iterator RegIt) const {

  // RegionSCCBegin vector contains an offset indicating the first SCC of the
  // region in RegionSCCs vector. Index set to NO_SCC means the region has no
  // SCCs so we can simply return RegionSCCs end() iterator. Otherwise, to find
  // the last SCC associated with the region, we need to traverse the
  // RegionSCCBegin vector and find the next non - NO_SCC element. For exmaple,
  // consider the following RegionSCCBegin vector-
  //
  // [NO_SCC, 0, NO_SCC, 4]
  //
  // The above vector indicates that:
  // - First region does not contain any SCCs.
  // - Second region contains SCCs 0 to 3(4 is the end() element).
  // - Third region does not contain any SCCs.
  // - Fourth region contains all the remaining SCCs starting from 4.
  //
  unsigned Index = getRegionIndex(RegIt);
  int BeginOffset = RegionSCCBegin[Index];

  // No SCCs associated with this region, return end().
  if (BeginOffset == NO_SCC) {
    return RegionSCCs.end();
  }

  // Look for the end() for this region by looking at the next non-null index in
  // the array.
  for (++Index; Index < RegionSCCBegin.size(); ++Index) {
    int EndOffset = RegionSCCBegin[Index];

    if (EndOffset != NO_SCC) {
      assert(EndOffset > BeginOffset && "Region SCC offsets are wrong!");
      return RegionSCCs.begin() + EndOffset;
    }
  }

  // Couldn't find a non-null index, return end().
  return RegionSCCs.end();
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
    for (auto InstI = (*SCCIt)->Nodes.begin(), InstE = (*SCCIt)->Nodes.end();
         InstI != InstE; ++InstI) {
      if (InstI != (*SCCIt)->Nodes.begin()) {
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
