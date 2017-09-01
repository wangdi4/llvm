//   Copyright (C) 2015-2016 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation. and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//   Source file:
//   ------------
//   VectorGraphPredicator.cpp -- Implements the Vector Graph Predication pass.
//
//===----------------------------------------------------------------------===//

#include "llvm/Analysis/Intel_VPO/Vecopt/VectorGraphPredicator.h"
#include "llvm/Analysis/Intel_VPO/Vecopt/VectorGraphInfo.h"
#include "llvm/Analysis/Intel_VPO/Vecopt/VectorGraph.h"
#include "llvm/Analysis/Intel_VPO/Vecopt/VectorGraphUtils.h"
#include "llvm/IR/Dominators.h"
#include "llvm/Analysis/PostDominators.h"
#include "llvm/IR/CFG.h"
#include "llvm/Analysis/Intel_VPO/Vecopt/Passes.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/Support/GenericDomTreeConstruction.h"
#include <stack>

#define DEBUG_TYPE "vector-graph-predicator"

using namespace llvm;

namespace llvm {

class VGSESERegion {

private:

  VGNode* ContainingNode;
  bool IsUniform = true;
  SmallVector<VGSESERegion*, 2> SubRegions;

  VGBlock* Entry;
  VGBlock* Exit;

  void print(formatted_raw_ostream &O, unsigned Depth) const {
    std::string Indent((Depth * 3), ' ');
    O << Indent << "SESE Region at ";
    ContainingNode->print(O, 0);
    O << "\n";
    O << Indent << Indent << "Uniform: " << (IsUniform ? "Yes" : "No") << "\n";
    O << Indent << Indent << "Entry BB: ";
    Entry->print(O, false);
    O << "\n";
    O << Indent << Indent << "Exit BB: ";
    Exit->print(O, false);
    O << "\n";
    O << Indent << Indent << "Subregions:\n";
    for (VGSESERegion* SubRegion : SubRegions)
      SubRegion->print(O, Depth + 2);
  }

public:

  VGSESERegion(VGNode* ANode, VGBlock* Dom, VGBlock* PostDom)
      : ContainingNode(ANode), Entry(Dom), Exit(PostDom) {}
  virtual ~VGSESERegion() {
    for (VGSESERegion *SubRegion : SubRegions)
      delete SubRegion;
  }
  void setDivergent() { IsUniform = false; }
  void addSubRegion(VGSESERegion *SubRegion) { SubRegions.push_back(SubRegion); }
  VGBlock* getEntry() const { return Entry; }
  VGBlock* getExit() const { return Exit; }
  bool isUniform() const { return IsUniform; }
  VGNode* getContainingNode() const { return ContainingNode; }
  const SmallVectorImpl<VGSESERegion*>& getSubRegions() const {
    return SubRegions;
  }

  void print(formatted_raw_ostream &O) const { print (O, 0); }

};

class ConstructVGSESERegions {

private:

  VGLoop* ALoop;
  VGDominatorTree& DominatorTree;
  VGDominatorTree& PostDominatorTree;
  VGSESERegion *Root;
  std::stack<VGSESERegion*> RegionStack;

public:
  ConstructVGSESERegions(VGLoop* VGL, VGDominatorTree &VGDT,
                                      VGDominatorTree &VGPDT) :
    ALoop(VGL), DominatorTree(VGDT), PostDominatorTree(VGPDT) {

    VGBlock *FirstChild = cast<VGBlock>(VGL->getFirstChild());
    auto *PostDomNode = PostDominatorTree.getNode(FirstChild)->getIDom();
    if (PostDomNode) {
      VGBlock *PostDom = PostDomNode->getBlock();
      VGBlock *Dom = DominatorTree.getNode(PostDom)->getIDom()->getBlock();
      Root = new VGSESERegion(VGL, Dom, PostDom);
    } else {
      Root = new VGSESERegion(VGL, FirstChild, FirstChild);
    }

    RegionStack.push(Root);
  }
  
  virtual ~ConstructVGSESERegions() {
    //delete Root;
  }

  const VGSESERegion *getRoot() { return Root; }

  void processControlFlow(VGNode *VNode, ScalarEvolution *SE) {

    // Check if this VGNode contains a SESE region. If so, create it as a
    // sub-region of the one it is in.

    VGBlock *VBlock = cast<VGBlock>(VNode);
    if (!VBlock->hasBranchCondition())
      return;

    VGBlock *PostDom = PostDominatorTree.getNode(VBlock)->getIDom()->getBlock();
    VGBlock *Dom = DominatorTree.getNode(PostDom)->getIDom()->getBlock();

    if (Dom == VBlock) {
      VGSESERegion *SubRegion = new VGSESERegion(VNode, Dom, PostDom);
      RegionStack.top()->addSubRegion(SubRegion);
      RegionStack.push(SubRegion);
    }

    // For now, it is assumed we're dealing exclusively with innermost loop
    // vectorization. Mark the SESE region as divergent if the condition of the
    // branch is non-uniform with respect to this loop.
    //Value *Cmp = VBlock->getBranchCondition();
    //const SCEV *CmpSCEV = SE->getSCEV(Cmp);
    //if (!SE->isLoopInvariant(CmpSCEV, ALoop->getLoop())) {
    //  RegionStack.top()->setDivergent();
    //}
    // TODO: LoopVectorize doesn't know how to deal with uniform regions
    // We make all regions divergent
    RegionStack.top()->setDivergent();
  }
};

//class CollectLexicalLinks {
//
//public:
//  
//  typedef std::set<std::pair<VGBlock*, VGBlock*> > ConstraintsTy;
//
//private:
//  
//  ConstraintsTy Constraints;
//
//public:
//
//  CollectLexicalLinks() {}
//  ~CollectLexicalLinks() {}
//
//  const ConstraintsTy& getConstraints() const {
//    return Constraints;
//  }
//
//  void setConstraint(VGBlock* VBlock1, VGBlock* VBlock2) {
//    Constraints.insert(std::make_pair(VBlock1, VBlock2));
//  }
//};

} // End namespace llvm

using VGBBDomTree = DominatorTreeBase<VGBlock>;
template void
llvm::DomTreeBuilder::Calculate<VGBBDomTree, VGLoop>(VGBBDomTree &DT,
                                                     VGLoop &VGL);

INITIALIZE_PASS_BEGIN(VectorGraphPredicator, "vec-graph-predicator", "Vector Graph Predicator", false, true)
INITIALIZE_PASS_DEPENDENCY(VectorGraphInfo)
INITIALIZE_PASS_DEPENDENCY(DominatorTreeWrapperPass)
INITIALIZE_PASS_DEPENDENCY(PostDominatorTreeWrapperPass)
INITIALIZE_PASS_DEPENDENCY(ScalarEvolutionWrapperPass)
INITIALIZE_PASS_END(VectorGraphPredicator, "vec-graph-predicator", "Vector Graph Predicator", false, true)

char VectorGraphPredicator::ID = 0;

static cl::opt<bool>
    VectorGraphPred("vector-graph-predicator", cl::init(true),
         cl::desc("Inserts predicates on the vector graph."));

FunctionPass *llvm::createVectorGraphPredicatorPass() { return new VectorGraphPredicator(); }

VectorGraphPredicator::VectorGraphPredicator()
    : FunctionPass(ID) {
  llvm::initializeVectorGraphPredicatorPass(*PassRegistry::getPassRegistry());
}

VectorGraphPredicator::VectorGraphPredicator(ScalarEvolution *SE)
    : FunctionPass(ID), SE(SE) {}

void VectorGraphPredicator::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.setPreservesAll();
  AU.addRequired<VectorGraphInfo>();
  AU.addRequired<DominatorTreeWrapperPass>();
  AU.addRequired<PostDominatorTreeWrapperPass>();
  AU.addRequired<ScalarEvolutionWrapperPass>();
}

bool VectorGraphPredicator::runOnFunction(Function &F) {

  VGI = &getAnalysis<VectorGraphInfo>();
  SE = &getAnalysis<ScalarEvolutionWrapperPass>().getSE();

  VectorGraphInfo::iterator VGIItr = VGI->begin();
  VectorGraphInfo::iterator VGIEnd = VGI->end();
  for (; VGIItr != VGIEnd; ++VGIItr) {
    if (VGLoop *ALoop = dyn_cast<VGLoop>(VGIItr)) {
      runOnAvr(ALoop);
    }
  }
//errs() << "Entering runOnFuncion for Predicator ...\n";

  return false;
}

void VectorGraphPredicator::runOnAvr(VGLoop* ALoop) {

  DEBUG(formatted_raw_ostream FOS(dbgs());
        FOS << "Predicator running on:\n";
        ALoop->print(FOS, 0);
        );

  predicateLoop(ALoop);

  DEBUG(formatted_raw_ostream FOS(dbgs());
        FOS << "Predicator finished:\n";
        ALoop->print(FOS, 0);
        );
}

void VectorGraphPredicator::predicateLoop(VGLoop* ALoop) {

  DEBUG(formatted_raw_ostream FOS(dbgs());
        FOS << "Predicating loop ";
        ALoop->print(FOS, 0);
        FOS << "\n");

  VGDominatorTree DominatorTree(false /* DominatorTree */);
  DominatorTree.recalculate(*ALoop);
  VGDominatorTree PostDominatorTree(true /* Post-Dominator Tree */);
  PostDominatorTree.recalculate(*ALoop);

  DEBUG(dbgs() << "Dominator Tree:\n"; DominatorTree.print(dbgs()));
  DEBUG(dbgs() << "PostDominator Tree:\n"; PostDominatorTree.print(dbgs()));

  ConstructVGSESERegions CSR(ALoop, DominatorTree, PostDominatorTree);

  // For now, just iterate over the VGBlocks in the VGLoop. This will later be
  // replaced by visitors.
  for (auto Itr = ALoop->child_begin(), End = ALoop->child_end(); Itr != End;
       ++Itr) {
    CSR.processControlFlow(&*Itr, SE);
  }

  DEBUG(formatted_raw_ostream FOS(dbgs());
        FOS << "SESE regions:\n";
        CSR.getRoot()->print(FOS));

  // Recursively handle the SESE regions
  handleVGSESERegion(CSR.getRoot(), ALoop, DominatorTree);
}

void VectorGraphPredicator::handleVGSESERegion(const VGSESERegion *Region,
                                               VGLoop *Loop,
                                               const VGDominatorTree &DomTree) {

  DEBUG(formatted_raw_ostream FOS(dbgs());
        FOS << "Handling DIVERGENT"
            << " SESE region:\n";
        Region->getContainingNode()->print(FOS, 1));
  //      << (Region->isUniform() ? "UNIFORM" : "DIVERGENT")

  //if (Region->isUniform()) {

  //  // Just handle all subregions recursively
  //  for (VGSESERegion* SubRegion : Region->getSubRegions())
  //    handleVGSESERegion(SubRegion, Loop, DomTree);
  //  return;
  //}

  // This region is divergent: deconstruct into AVRBlocks (but keep uniform
  // sub-regions intact).

//  CollectLexicalLinks CLL;
//
//  for (auto Itr = Loop->child_begin(), End = Loop->child_end(); Itr != End;
//       ++Itr) {
//    VGBlock *VBlock = cast<VGBlock>(Itr);
//    if (VBlock->hasBranchCondition()) {
//      BasicBlock *BB = VBlock->getBasicBlock();
//      TerminatorInst *TermInst = BB->getTerminator();
//errs() << "TerminatorInst: " << *TermInst << "\n";
//      BasicBlock *IfBlock = TermInst->getSuccessor(0);
//      BasicBlock *ElseBlock = TermInst->getSuccessor(1);
//      VGBlock *IfVBlock = Loop->BlockMap[IfBlock];
//      VGBlock *ElseVBlock = Loop->BlockMap[ElseBlock];
//      // Successor could be outside of the VGLoop
//      if (!IfBlock || !ElseVBlock) continue;
//      CLL.setConstraint(ElseVBlock, IfVBlock);
//    }
//  }
//
//  DenseMap<VGNode*, VGSESERegion*> DoNotDeconstruct;

  // Handle all uniform subregions
  //for (VGSESERegion* SubRegion : Region->getSubRegions())
  //  if (SubRegion->isUniform()) {
  //    DEBUG(formatted_raw_ostream FOS(dbgs());
  //          FOS << "Preserving ";
  //      SubRegion->getContainingNode()->print(FOS, 0);
  //      FOS << "\n");
  //    DoNotDeconstruct[SubRegion->getContainingNode()] = SubRegion;
  //    handleVGSESERegion(SubRegion, Loop, DomTree);
  //  }

  // Add lexical links scheduling constraints e.g. then before else.
//  for (auto& Pair : CLL.getConstraints()) {
//    VGBlock* FirstBlock = dyn_cast<VGBlock>(Pair.first->getParent());
//    if (!FirstBlock)
//      continue;
//    VGBlock* SecondBlock = dyn_cast<VGBlock>(Pair.second->getParent());
//    if (!SecondBlock)
//      continue;
//    DEBUG(formatted_raw_ostream FOS(dbgs());
//          FOS << "Block " << FirstBlock->getNumber()
//	  << " lexically depends on block "
//	  << SecondBlock->getNumber() << "\n");
//    VectorGraphUtils::addSchedulingConstraint(FirstBlock, SecondBlock);
//  }

  VGBlock* EntryBlock = Region->getEntry();
//  VGBlock* ExitBlock = Region->getExit();
//  VGBlock* NextBlockInsertionPos = EntryBlock;

  // Insert the AVRBlocks into the parent of the region's entry. We use DFS
  // topological sort in order to get valid scheduling that preserves blocks
  // locality to improve the chances of later zero-bypass installations.
//  SmallPtrSet<VGBlock*, 16> ScheduledBlocks;
//  SmallVector<VGBlock*, 16> LinearizedBlocks;
//  std::stack<VGBlock*> BlockStack;
//
//  ScheduledBlocks.insert(EntryBlock); // already scheduled.
//  LinearizedBlocks.push_back(EntryBlock);
//  BlockStack.push(ExitBlock);
//  while (!BlockStack.empty()) {
//
//    // Examine top of stack: push into stack any constrainting block which has
//    // not yet been scheduled. If there are no such blocks, schedule this block.
//    VGBlock* Top = BlockStack.top();
//    DEBUG(formatted_raw_ostream FOS(dbgs());
//          FOS << "Top VGBlock now " << Top->getNumber() << " \n");
//    const SmallPtrSetImpl<VGBlock*>& SchedConstraints
//      = Top->getSchedConstraints();
//    bool CanSchedule = true;
//    for (VGBlock* Dependency : SchedConstraints) {
//      if (!ScheduledBlocks.count(Dependency)) {
//	DEBUG(formatted_raw_ostream FOS(dbgs());
//	      FOS << "Pushing dependency AVRBlock " << Dependency->getNumber()
//	      << "\n");
//        BlockStack.push(Dependency);
//        CanSchedule = false;
//      }
//    }
//    if (!CanSchedule)
//      continue; // at least one scheduling constraint has not been met yet
//
//    BlockStack.pop();
//
//    if (ScheduledBlocks.count(Top))
//      continue;
//
//    DEBUG(formatted_raw_ostream FOS(dbgs());
//          FOS << "Scheduling VGBlock " << Top->getNumber() << " after VGBlock "
//	  << NextBlockInsertionPos->getNumber() << "\n");
//
//    //AVRUtils::insertAfter(AvrItr(NextBlockInsertionPos), Top);
//    NextBlockInsertionPos = Top;
//    ScheduledBlocks.insert(Top);
//    LinearizedBlocks.push_back(Top);
//  }
//
//  DEBUG(formatted_raw_ostream FOS(dbgs());
//        FOS << "Linearized Block Sequence:\n";
//        for (unsigned i = 0; i < LinearizedBlocks.size(); i++)
//          FOS << "Block #: " << LinearizedBlocks[i]->getNumber() << "\n";);
  
  predicate(EntryBlock, Loop, DomTree);

  // Remove the initial VGBlock ordering within the VGLoop and replace with the
  // linearized order of VGBlocks.
//  SmallVector<VGBlock*, 4> ToRemove;
//  for (auto Itr = Loop->child_begin(), End = Loop->child_end(); Itr != End;
//       ++Itr) {
//    VGBlock *VBlock = cast<VGBlock>(Itr);
//    ToRemove.push_back(VBlock);
//  }
//
//  for (unsigned i = 0; i < ToRemove.size(); i++) {
//    VectorGraphUtils::remove(ToRemove[i]);
//  }
//
//  for (unsigned i = 0; i < LinearizedBlocks.size(); i++) {
//    VectorGraphUtils::insertLastChild(Loop, LinearizedBlocks[i]);
//  }
//
//errs() << "Dumping Linearized VGLoop blocks:\n";
//for (auto Itr = Loop->child_begin(), End = Loop->child_end(); Itr != End;
//     ++Itr) {
//  VGBlock *VBlock = cast<VGBlock>(Itr);
//  VBlock->dump();
//}

/*
  DEBUG(formatted_raw_ostream FOS(dbgs());
        FOS << "After predication:\n";
        EntryBlock->getParent()->getParent()->print(FOS, 0, PrintNumber));

  removeCFG(EntryBlock);

  DEBUG(formatted_raw_ostream FOS(dbgs());
        FOS << "After removing CFG:\n";
        ContainingNode->getParent()->getParent()->print(FOS, 0, PrintNumber));
*/
}

void VectorGraphPredicator::predicate(VGBlock *Entry,
                                      VGLoop *Loop,
                                      const VGDominatorTree &DomTree) {
  // Add an empty VGPredicate object to each VGBlock.
  for (auto It = df_iterator<VGBlock*>::begin(Entry),
       End = df_iterator<VGBlock*>::end(Entry); It != End; ++It) {

    VGBlock* VBlock = *It;
    VGPredicate *Pred = VectorGraphUtils::createVGPredicate();
    VectorGraphUtils::setPredicate(VBlock, Pred);
  }

  // Propagate all incoming conditions from prececessor blocks to this
  // block.
  for (auto It = df_iterator<VGBlock*>::begin(Entry),
       End = df_iterator<VGBlock*>::end(Entry); It != End; ++It) {

    VGBlock *VBlock = *It;

    // The loop entry doesn't need predication in supported
    // innermost loop vectorization scenarios.
    if (VBlock == Loop->getEntry()) continue;

    // This won't work with the latest changes in the algorithm
    // TODO: Optimization assuming innermostloop vectorization without
    // breaks/continues, etc.
    // if (VBlock->getPredecessors().size() > 1) {
    //    bool PostDomAllPredecs = true;
    //    for (P : VBlock->getPredecessors()) {
    //      if (!PostDomTree.dominates(VBlock, P)) {
    //         PostDomAllPredecs = false;
    //         break;
    //      }
    //    }
    //    // Propagate predicate from immediate dominator
    //    if (PostDomAllPredecs) {
    //      VGBlock *IDom = DomTree.getImmediateDominator(VBlock);
    //      VectorGraphUtils::setPredicate(VBlock, IDom->getPredicate()); 
    //
    //      DEBUG(formatted_raw_ostream FOS(dbgs());
    //      FOS << "Propagating predicate from Block " << IDom->getNumber()
    //          << " to Block " << VBlock->getNumber() << "\n";
    //    }
    //
    //    continue;
    // }

    DEBUG(formatted_raw_ostream FOS(dbgs());
          FOS << "Predicating block " << VBlock->getNumber() << "\n";
          VBlock->dump();
          FOS << "\n");

    for (VGBlock *Predecessor : VBlock->getPredecessors()) {

      // Ordinal tells us whether VBlock lies on the true or false branch taken
      // by the predecessor (0 = true, 1 = false)
      unsigned Ordinal = Predecessor->getSuccessorOrdinal(VBlock);
      bool CondNeedsNegation = Ordinal == 1;

      DEBUG(formatted_raw_ostream FOS(dbgs());
            FOS << "Handling predecessor block " << Predecessor->getNumber()
                << " with ordinal " << Ordinal << "\n");

      Value *PredCondition = Predecessor->getBranchCondition();
      if (PredCondition) {
        DEBUG(formatted_raw_ostream FOS(dbgs());
              FOS << "Incoming Condition: " << *PredCondition << "\n");

        VectorGraphUtils::addVGPredicateIncoming(
            VBlock->getPredicate(), Predecessor->getPredicate(), PredCondition,
            CondNeedsNegation);
      } else { // Unconditional branch
        assert(
            !cast<BranchInst>(Predecessor->getTerminator())->isConditional() &&
            "Did not expect conditional branches at this point");
        DEBUG(formatted_raw_ostream FOS(dbgs());
              FOS << "Unconditional Branch\n");

        VectorGraphUtils::addVGPredicateIncoming(VBlock->getPredicate(),
                                                 Predecessor->getPredicate(),
                                                 nullptr, CondNeedsNegation);
      }
    }

    // By now, we only can ensure that predicates from VBlocks that dominate the
    // loop latch will be all ones in supported innermost loop vectorization
    // scenarios. At least, loop entry, loop latch and loop exit will not hit
    // here.
    if (!DomTree.dominates(VBlock, Loop->getLoopLatch())) {
      DEBUG(formatted_raw_ostream FOS(dbgs());
            FOS << "Block " << VBlock->getNumber()
                << " needs predication. Removing isAllOnes flag.\n";
            VBlock->dump(); FOS << "\n");

      VectorGraphUtils::setAllOnes(VBlock->getPredicate(), false);
    } 

    //Value *Condition = VBlock->getBranchCondition();
    //if (Condition) {
    //  errs() << "VBlock Condition (incoming to successor blocks): "
    //         << *Condition << "\n";
    //}
  }

  for (auto It = df_iterator<VGBlock*>::begin(Entry),
       End = df_iterator<VGBlock*>::end(Entry); It != End; ++It) {

    VGBlock* VBlock = *It;
    //errs() << "Incoming Conditions for block:\n";
    //VBlock->dump();
    //errs() << "\n";
    VGPredicate *Pred = VBlock->getPredicate();
    const SmallVectorImpl<VGPredicate::IncomingTy> &IncomingPredicates =
        Pred->getIncoming();
    for (unsigned i = 0; i < IncomingPredicates.size(); i++) {
    //  errs() << "  Condition: ";
    //  Value *Cond = IncomingPredicates[i].second.first;
    //  bool CondNeedsNegation = IncomingPredicates[i].second.second;

    //  if (Cond) {
    //    if (CondNeedsNegation) {
    //      errs() << "!";
    //    }
    //    errs() << *Cond << "\n";
    //  } else {
    //    errs() << "-\n";
    //  }
    }
  }
}

/*
void VectorGraphPredicator::removeCFG(AVRBlock* Entry) {

  for (auto It = df_iterator<AVRBlock*>::begin(Entry),
         End = df_iterator<AVRBlock*>::end(Entry); It != End; ++It) {

    AVRBlock* ABlock = *It;
    DEBUG(formatted_raw_ostream FOS(dbgs());
          FOS << "Extracting nodes from block " << ABlock->getNumber() << "\n");

    for (auto It = ABlock->child_rbegin();
         It != ABlock->child_rend();
         It = ABlock->child_rbegin() ) {

      AVR* Node = &*It;

      if (isa<AVRBranch>(Node)) {

        // Delete branches, which have no use in a linearized sequence of nodes.
        AVRUtils::remove(Node);
        AVRUtils::destroy(Node);
        continue;
      }

      // Propagate the block's predicate to this node.
      AVRUtils::setPredicate(Node, ABlock->getPredicate());

      // Move this node past the block (reverse iterator maintains the order).
      AVRUtils::moveAfter(AvrItr(ABlock), Node);
    }
  }

  SmallVector<AVRBlock*, 3> Worklist;
  SmallPtrSet<AVRBlock*, 8> Visited;
  Worklist.push_back(Entry);
  while (!Worklist.empty()) {

    AVRBlock* CurrentBlock = Worklist.back();
    Worklist.pop_back();
    Visited.insert(CurrentBlock);

    for (AVRBlock *Successor : CurrentBlock->getSuccessors())
      if (!Visited.count(Successor))
        Worklist.push_back(Successor);

    AVRUtils::remove(CurrentBlock);
    AVRUtils::destroy(CurrentBlock);
  }
}
*/
