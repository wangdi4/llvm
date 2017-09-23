//   Copyright (C) 2015-2016 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation. and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//   Source file:
//   ------------
//   VPOPredicator.cpp -- Implements the VPO Predication pass.
//
//===----------------------------------------------------------------------===//

#include "llvm/Analysis/Intel_VPO/Vecopt/VPOPredicator.h"
#include "llvm/Analysis/Intel_VPO/Vecopt/VPOSIMDLaneEvolution.h"
#include "llvm/ADT/MapVector.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/Analysis/Intel_VPO/Vecopt/Passes.h"

#define DEBUG_TYPE "avr-predicate"
using namespace llvm;
using namespace llvm::vpo;

namespace llvm {

namespace vpo {

class SESERegion {

private:

  AVR* ContainingNode;
  bool IsUniform = true;
  SmallVector<SESERegion*, 2> SubRegions;

  AvrBasicBlock* Entry;
  AvrBasicBlock* Exit;

  void print(formatted_raw_ostream &O, unsigned Depth) const {
    std::string Indent((Depth * 3), ' ');
    O << Indent << "SESE Region at ";
    ContainingNode->shallowPrint(O);
    O << "\n";
    O << Indent << Indent << "Uniform: " << (IsUniform ? "Yes" : "No") << "\n";
    O << Indent << Indent << "Entry BB: ";
    Entry->print(O, false);
    O << "\n";
    O << Indent << Indent << "Exit BB: ";
    Exit->print(O, false);
    O << "\n";
    O << Indent << Indent << "Subregions:\n";
    for (SESERegion* SubRegion : SubRegions)
      SubRegion->print(O, Depth + 2);
  }

public:

  SESERegion(AVR* ANode, AvrBasicBlock* Dom, AvrBasicBlock* PostDom)
      : ContainingNode(ANode), Entry(Dom), Exit(PostDom) {}
  virtual ~SESERegion() {
    for (SESERegion *SubRegion : SubRegions)
      delete SubRegion;
  }
  void setDivergent() { IsUniform = false; }
  void addSubRegion(SESERegion *SubRegion) { SubRegions.push_back(SubRegion); }
  AvrBasicBlock* getEntry() const { return Entry; }
  AvrBasicBlock* getExit() const { return Exit; }
  bool isUniform() const { return IsUniform; }
  AVR* getContainingNode() const { return ContainingNode; }
  const SmallVectorImpl<SESERegion*>& getSubRegions() const {
    return SubRegions;
  }

  void print(formatted_raw_ostream &O) const { print (O, 0); }

};

class OrderLoops {
public:
  OrderLoops() {}
  virtual ~OrderLoops() {}

  /// Visit Functions
  void visit(AVR* ANode) {}
  void postVisit(AVR* ANode) {}
  bool isDone() { return false; }
  bool skipRecursion(AVR *ANode) { return false; }
  void postVisit(AVRLoop* ALoop) {
    Loops.push_back(ALoop);
  }

  SmallVector<AVRLoop*, 3> Loops;
};

class ConstructSESERegions {

private:

  AVRLoop* ALoop;
  AvrCFGBase& CFG;
  AvrDominatorTree &DominatorTree;
  AvrPostDominatorTree &PostDominatorTree;
  SESERegion *Root;
  std::stack<SESERegion*> RegionStack;

  void processControlFlow(AVR* ANode) {

    // Check if this AVR node contains a SESE region. If so, create it as a
    // sub-region of the one it is in.

    if (!CFG.isBranchCondition(ANode))
      return;

    AvrBasicBlock* BasicBlock = CFG.getBasicBlock(ANode);
    assert(BasicBlock && "AVR node not in CFG?");
    AvrBasicBlock* PostDom =
      PostDominatorTree.getNode(BasicBlock)->getIDom()->getBlock();
    AvrBasicBlock* Dom = DominatorTree.getNode(PostDom)->getIDom()->getBlock();
    
    if (Dom == BasicBlock) {

      SESERegion *SubRegion = new SESERegion(ANode, Dom, PostDom);
      RegionStack.top()->addSubRegion(SubRegion);
      RegionStack.push(SubRegion);
    }
    // If this AVR node is divergent, it marks the SESE region it affects for
    // deconstruction (either the one it contains or the one it resides in).
    // TODO: Enable SLEV 
    //if (!ANode->getSLEV().isUniform()) 
      RegionStack.top()->setDivergent();
  }

public:
  ConstructSESERegions(AVRLoop *AL, AvrCFGBase &C, AvrDominatorTree &DT,
                       AvrPostDominatorTree &PDT)
      : ALoop(AL), CFG(C), DominatorTree(DT), PostDominatorTree(PDT) {

    AvrBasicBlock* BasicBlock = CFG.getBasicBlock(&*ALoop->child_begin());
    assert(BasicBlock && "Loop node not in CFG?");
    auto* PostDomNode = PostDominatorTree.getNode(BasicBlock)->getIDom();
    if (PostDomNode) {
      AvrBasicBlock* PostDom = PostDomNode->getBlock();
      AvrBasicBlock* Dom = DominatorTree.getNode(PostDom)->getIDom()->getBlock();
      Root = new SESERegion(ALoop, Dom, PostDom);
    }
    else {
      // This CFG is a singleton.
      Root = new SESERegion(ALoop, BasicBlock, BasicBlock);
    }

    RegionStack.push(Root);
  }
  
  virtual ~ConstructSESERegions() {
    delete Root;
  }

  const SESERegion *getRoot() { return Root; }

  /// Visit Functions
  void visit(AVR* ANode) {}
  void postVisit(AVR* ANode) {}
  bool isDone() { return false; }
  bool skipRecursion(AVR *ANode) {
    return !(ANode == ALoop || isa<AVRIf>(ANode) || isa<AVRSwitch>(ANode));
  }
  void visit(AVRIf* AIf) { processControlFlow(AIf); }
  void postVisit(AVRIf* AIf) {
    if (RegionStack.top()->getContainingNode() == AIf)
      RegionStack.pop();
  }
  void visit(AVRSwitch* ASwitch) { processControlFlow(ASwitch); }
  void postVisit(AVRSwitch* ASwitch) {
    if (RegionStack.top()->getContainingNode() == ASwitch)
      RegionStack.pop();
  }
};

class CollectLexicalLinks {

public:
  
  typedef std::set<std::pair<AVR*, AVR*> > ConstraintsTy;

private:
  
  ConstraintsTy Constraints;

public:

  CollectLexicalLinks() {}
  ~CollectLexicalLinks() {}

  const ConstraintsTy& getConstraints() const {
    return Constraints;
  }

  /// Visit Functions
  void visit(AVR* ANode) {}
  void postVisit(AVR* ANode) {}
  bool isDone() { return false; }
  bool skipRecursion(AVR *ANode) {
    return isa<AVRLoop>(ANode);
  }
  void visit(AVRIf* AIf) {
    AVR *LastThenChild = AIf->getLastThenChild();
    if (!LastThenChild)
      return;
    AVR *FirstElseChild = AIf->getFirstElseChild();
    if (!FirstElseChild)
      return;
    Constraints.insert(std::make_pair(FirstElseChild, LastThenChild));
  }
  void visit(AVRSwitch* ASwitch) {
    AVR *FirstDefaultChild = nullptr;
    if (ASwitch->hasDefaultCaseChildren())
      FirstDefaultChild = ASwitch->getDefaultCaseFirstChild();
    
    for (unsigned Case = 1; Case <= ASwitch->getNumCases(); ++Case) {
      if (!ASwitch->hasCaseChildren(Case))
        continue;
      
      if (FirstDefaultChild)
        Constraints.insert(std::make_pair(FirstDefaultChild,
                                          ASwitch->getCaseLastChild(Case)));

      for (unsigned PrevCase = 1; PrevCase < Case; ++PrevCase) {
        if (!ASwitch->hasCaseChildren(PrevCase))
          continue;
        Constraints.insert(std::make_pair(ASwitch->getFirstCaseChild(Case),
                                          ASwitch->getCaseLastChild(PrevCase)));
      }
    }
  }
};

} // End namespace vpo

template <> struct GraphTraits<vpo::AVRBlock*> {
  typedef vpo::AVRBlock NodeType;
  typedef vpo::AVRBlock *NodeRef;
  typedef SmallVectorImpl<AVRBlock*>::iterator ChildIteratorType;
  typedef standard_df_iterator2<vpo::AVRBlock *> nodes_iterator;

  static NodeType *getEntryNode(vpo::AVRBlock *N) {
    return N;
  }

  static inline ChildIteratorType child_begin(NodeType *N) {
    return N->succ_begin();
  }
  static inline ChildIteratorType child_end(NodeType *N) {
    return N->succ_end();
  }

  static nodes_iterator nodes_begin(vpo::AVRBlock *N) {
    return nodes_iterator(N, true);
  }

  static nodes_iterator nodes_end(vpo::AVRBlock *N) {
    return nodes_iterator(N, false);
  }
};

template <> struct GraphTraits<Inverse<vpo::AVRBlock*> > {
  typedef vpo::AVRBlock NodeType;
  typedef vpo::AVRBlock *NodeRef;
  typedef SmallVectorImpl<AVRBlock*>::iterator ChildIteratorType;
  typedef standard_df_iterator2<vpo::AVRBlock *> nodes_iterator;

  static NodeType *getEntryNode(Inverse<vpo::AVRBlock *> G) {
    return G.Graph;
  }

  static inline ChildIteratorType child_begin(NodeType *N) {
    return N->pred_begin();
  }

  static inline ChildIteratorType child_end(NodeType *N) {
    return N->pred_end();
  }

  static nodes_iterator nodes_begin(vpo::AVRBlock *N) {
    return nodes_iterator(N, true);
  }

  static nodes_iterator nodes_end(vpo::AVRBlock *N) {
    return nodes_iterator(N, false);
  }
};

template <> struct GraphTraits<const vpo::AVRBlock*> {
  typedef const vpo::AVRBlock NodeType;
  typedef const vpo::AVRBlock *NodeRef;
  typedef SmallVectorImpl<AVRBlock*>::const_iterator ChildIteratorType;
  typedef standard_df_iterator2<const vpo::AVRBlock *> nodes_iterator;

  static NodeType *getEntryNode(const vpo::AVRBlock *N) {
    return N;
  }

  static inline ChildIteratorType child_begin(NodeType *N) {
    return N->succ_begin();
  }
  static inline ChildIteratorType child_end(NodeType *N) {
    return N->succ_end();
  }

  static nodes_iterator nodes_begin(const vpo::AVRBlock *N) {
    return nodes_iterator(N, true);
  }

  static nodes_iterator nodes_end(const vpo::AVRBlock *N) {
    return nodes_iterator(N, false);
  }
};

template <> struct GraphTraits<Inverse<const vpo::AVRBlock*> > {
  typedef const vpo::AVRBlock NodeType;
  typedef const vpo::AVRBlock *NodeRef;
  typedef SmallVectorImpl<AVRBlock*>::const_iterator ChildIteratorType;
  typedef standard_df_iterator2<const vpo::AVRBlock *> nodes_iterator;

  static NodeType *getEntryNode(Inverse<const vpo::AVRBlock *> G) {
    return G.Graph;
  }

  static inline ChildIteratorType child_begin(NodeType *N) {
    return N->pred_begin();
  }

  static inline ChildIteratorType child_end(NodeType *N) {
    return N->pred_end();
  }

  static nodes_iterator nodes_begin(const vpo::AVRBlock *N) {
    return nodes_iterator(N, true);
  }

  static nodes_iterator nodes_end(const vpo::AVRBlock *N) {
    return nodes_iterator(N, false);
  }
};

} // End namespace llvm

bool VPOPredicatorBase::runOnFunction(Function &F) {

  // TODO: Enable SLEV
  //SE = &getAnalysis<ScalarEvolutionWrapperPass>().getSE();
  DEBUG(dbgs() << "AVR Predicator Pass\n");

  for (auto AVRItr = AVRG->begin(), AVREnd = AVRG->end(); AVRItr != AVREnd;
       ++AVRItr) {

    // Run on loops within the working region
    if (AVRWrn *Wrn = dyn_cast<AVRWrn>(AVRItr)) {
      for (auto WrnItr = Wrn->child_begin(), WrnEnd = Wrn->child_end();
           WrnItr != WrnEnd; ++WrnItr) {
        if (AVRLoop *ALoop = dyn_cast<AVRLoop>(WrnItr)) {
          runOnAvr(ALoop);
        }
      }
    }
  }

  return false;
}

void VPOPredicatorBase::runOnAvr(AVRLoop* ALoop) {

  DEBUG(formatted_raw_ostream FOS(dbgs());
        FOS << "Predicator running on:\n";
        ALoop->print(FOS, 0, PrintNumber);
        );

  // Make all loops uniform.

  // Predicate each (now uniform) loop
  OrderLoops OrderedLoops;
  AVRVisitor<OrderLoops> LoopVisitor(OrderedLoops);
  LoopVisitor.visit(ALoop, true, true, false /*RecursiveInsideValues*/, true);

  for (AVRLoop* ALoop : OrderedLoops.Loops)
    predicateLoop(ALoop);

  DEBUG(formatted_raw_ostream FOS(dbgs());
        FOS << "Predicator finished:\n";
        ALoop->print(FOS, 0, PrintNumber);
        );
}

void VPOPredicatorBase::predicateLoop(AVRLoop* ALoop) {
  DEBUG(formatted_raw_ostream FOS(dbgs());
        FOS << "Predicating loop ";
        ALoop->shallowPrint(FOS);
        FOS << "\n");

  AvrCFGBase CFG(ALoop->child_begin(),
                 ALoop->child_end(),
                 "Shallow CFG for loop",
                 false,
                 true);

  DEBUG(formatted_raw_ostream FOS(dbgs());
        FOS << "Predicating DAG:\n";
        CFG.print(FOS));

  AvrDominatorTree DominatorTree;
  DominatorTree.recalculate(CFG);
  AvrPostDominatorTree PostDominatorTree;
  PostDominatorTree.recalculate(CFG);

  DEBUG(dbgs() << "Dominator Tree:\n"; DominatorTree.print(dbgs()));
  DEBUG(dbgs() << "PostDominator Tree:\n"; PostDominatorTree.print(dbgs()));

  ConstructSESERegions CSR(ALoop, CFG, DominatorTree, PostDominatorTree);
  AVRVisitor<ConstructSESERegions> SESERegionsVisitor(CSR);
  SESERegionsVisitor.visit(ALoop, true, true, false /*RecursiveInsideValues*/,
                           true);

  DEBUG(formatted_raw_ostream FOS(dbgs());
        FOS << "SESE regions:\n";
        CSR.getRoot()->print(FOS));

  // Recursively handle the SESE regions
  handleSESERegion(CSR.getRoot(), &CFG);
}

void VPOPredicatorBase::handleSESERegion(const SESERegion *Region, AvrCFGBase* CFG) {

  DEBUG(formatted_raw_ostream FOS(dbgs());
        FOS << "Handling "
        << (Region->isUniform() ? "UNIFORM" : "DIVERGENT")
        << " SESE region:\n";
        Region->getContainingNode()->print(FOS, 1, PrintNumber));

  if (Region->isUniform()) {

    // Just handle all subregions recursively
    for (SESERegion* SubRegion : Region->getSubRegions())
      handleSESERegion(SubRegion, CFG);
    return;
  }

  // This region is divergent: deconstruct into AVRBlocks (but keep uniform
  // sub-regions intact).

  // Collect lexical links scheduling constraints.
  CollectLexicalLinks CLL;
  AVRVisitor<CollectLexicalLinks> CollectLexicalLinksVisitor(CLL);
  CollectLexicalLinksVisitor.visit(Region->getContainingNode(), true, true,
                                   false /*RecursiveInsideValues*/, true);

  DenseMap<AVR*, SESERegion*> DoNotDeconstruct;

  // Handle all uniform subregions
  for (SESERegion* SubRegion : Region->getSubRegions())
    if (SubRegion->isUniform()) {
      DEBUG(formatted_raw_ostream FOS(dbgs());
            FOS << "Preserving ";
	    SubRegion->getContainingNode()->shallowPrint(FOS);
	    FOS << "\n");
      DoNotDeconstruct[SubRegion->getContainingNode()] = SubRegion;
      handleSESERegion(SubRegion, CFG);
    }

  // Deconstruct this region into basic blocks

  // Construct an AVRBlock for each basic block in the CFG and place them
  // as the first children of the SESE containing node.

  DenseMap<AvrBasicBlock*, AVRBlock*> BlocksMap;
  MapVector<AVRBlock *, AvrBasicBlock *> BlockSuccessorsDictatorMap;
  SmallVector<AvrBasicBlock*, 3> Worklist;
  SmallPtrSet<AvrBasicBlock*, 8> Visited;
  AvrBasicBlock* RegionEntryBB = Region->getEntry();
  AvrBasicBlock* RegionExitBB = Region->getExit();
  Worklist.push_back(RegionEntryBB);

  // The following blocks get special treatment. 
  AVRBlock* EntryBlock = nullptr;
  AVRBlock* ExitBlock = nullptr;

  while (!Worklist.empty()) {

    AvrBasicBlock* Current = Worklist.back();
    Worklist.pop_back();
    Visited.insert(Current);

    AVRBlock* Block = AVRUtils::createAVRBlock();
    BlocksMap[Current] = Block;
    BlockSuccessorsDictatorMap[Block] = Current;
    DEBUG(formatted_raw_ostream FOS(dbgs());
	  FOS << "Mapping ";
	  Current->print(FOS, false);
	  FOS << " to AVRBlock " << Block->getNumber() << "\n");

    if (Current == RegionEntryBB) {
      EntryBlock = Block;
    }

    if (Current == RegionExitBB) {
      ExitBlock = Block;
      continue;
    }

    // If this block is the entry of a SESE region we skip the inner blocks of
    // the SESE region and continue from its exit block.
    AVR* Terminator = *Current->getInstructions().rbegin();
    assert(Terminator && "Expected block to have a terminator");
    if (DoNotDeconstruct.count(Terminator)) {
      Current = DoNotDeconstruct[Terminator]->getExit(); // Jump to exit BB.
      BlocksMap[Current] = Block; // Exit BB map to the same block as entry.
      DEBUG(formatted_raw_ostream FOS(dbgs());
	    FOS << "Mapping ";
	    Current->print(FOS, false);
	    FOS << " to AVRBlock " << Block->getNumber() << "\n");
      BlockSuccessorsDictatorMap[Block] = Current; // Take successors from exit.
    }

    for (AvrBasicBlock* Successor : Current->getSuccessors()) {
      if (!Visited.count(Successor))
        Worklist.push_back(Successor);
    }
  }

  assert(EntryBlock && "Entry basic block not mapped to any AVRBlock");
  assert(ExitBlock && "Exit basic block not mapped to any AVRBlock");

  // Exit BB should have no successors.
  BlockSuccessorsDictatorMap.erase(ExitBlock);

  // Now that all AVRBlocks exist, set their successors
  for (auto It : BlockSuccessorsDictatorMap) {

    for (AvrBasicBlock* Successor : It.second->getSuccessors())
      AVRUtils::addSuccessor(It.first, BlocksMap[Successor]);
  }

  // Schedule the entry block after the insertion point (which may itself move)
  // as an anchor for the block's scheduling process.

  AVR* ContainingNode = Region->getContainingNode();

  if (AVRLoop* ALoop = dyn_cast<AVRLoop>(ContainingNode))
    AVRUtils::insertAfter(AvrItr(ALoop->child_end()), EntryBlock);
  else
    AVRUtils::insertAfter(AvrItr(ContainingNode), EntryBlock);
  AVRBlock* NextBlockInsertionPos = EntryBlock;

  // Move the AVR nodes residing in the AvrBasicBlocks to their designated
  // AVRBlocks.
  for (auto It : BlocksMap) {

    if (It.second == ExitBlock)
      continue; // Do not do this for the AVR nodes of the exit BB

    const AvrBasicBlock::InstructionsTy& BBInstructions
      = It.first->getInstructions();
    AvrItr First;
    AvrItr Last;
    if (It.second == EntryBlock) {
      // Move into the AVRBlock only the diverging AVR node which is the entry
      // of the SESE region.
      AVR* Singleton = *BBInstructions.rbegin();
      AVRUtils::remove(Singleton);
      AVRUtils::insertFirstChild(It.second, Singleton);
    }
    else {
      // Move all instructions in the basic block into the AVR Block
      First = AvrItr(*BBInstructions.begin());
      Last = AvrItr(*BBInstructions.rbegin());
      AVRUtils::moveAsLastChildren(It.second, First, Last);
    }
  }

  // Add lexical links scheduling constraints e.g. then before else.
  for (auto& Pair : CLL.getConstraints()) {
    AVRBlock* FirstBlock = dyn_cast<AVRBlock>(Pair.first->getParent());
    if (!FirstBlock)
      continue;
    AVRBlock* SecondBlock = dyn_cast<AVRBlock>(Pair.second->getParent());
    if (!SecondBlock)
      continue;
    DEBUG(formatted_raw_ostream FOS(dbgs());
          FOS << "Block " << FirstBlock->getNumber()
	  << " lexically depends on block "
	  << SecondBlock->getNumber() << "\n");
    AVRUtils::addSchedulingConstraint(FirstBlock, SecondBlock);
  }

  // Insert the AVRBlocks into the parent of the region's entry. We use DFS
  // topological sort in order to get valid scheduling that preserves blocks
  // locality to improve the chances of later zero-bypass installations.
  SmallPtrSet<AVRBlock*, 16> ScheduledBlocks;
  std::stack<AVRBlock*> BlockStack;

  ScheduledBlocks.insert(EntryBlock); // already scheduled.
  BlockStack.push(ExitBlock);
  while (!BlockStack.empty()) {

    // Examine top of stack: push into stack any constrainting block which has
    // not yet been scheduled. If there are no such blocks, schedule this block.
    AVRBlock* Top = BlockStack.top();
    DEBUG(formatted_raw_ostream FOS(dbgs());
          FOS << "Top AVRBlock now " << Top->getNumber() << " \n");
    const SmallVectorImpl<AVRBlock *> &SchedConstraints =
        Top->getSchedConstraints();
    bool CanSchedule = true;
    for (AVRBlock* Dependency : SchedConstraints) {
      if (!ScheduledBlocks.count(Dependency)) {
	DEBUG(formatted_raw_ostream FOS(dbgs());
	      FOS << "Pushing dependency AVRBlock " << Dependency->getNumber()
	      << "\n");
        BlockStack.push(Dependency);
        CanSchedule = false;
      }
    }
    if (!CanSchedule)
      continue; // at least one scheduling constraint has not been met yet

    BlockStack.pop();

    if (ScheduledBlocks.count(Top))
      continue;

    DEBUG(formatted_raw_ostream FOS(dbgs());
          FOS << "Scheduling AVRBlock " << Top->getNumber() << " after AVRBlock "
	  << NextBlockInsertionPos->getNumber() << "\n");

    AVRUtils::insertAfter(AvrItr(NextBlockInsertionPos), Top);
    NextBlockInsertionPos = Top;
    ScheduledBlocks.insert(Top);
  }

  DEBUG(formatted_raw_ostream FOS(dbgs());
        FOS << "After liearization:\n";
        EntryBlock->getParent()->getParent()->print(FOS, 0, PrintNumber));

  predicate(EntryBlock);

  DEBUG(formatted_raw_ostream FOS(dbgs());
        FOS << "After predication:\n";
        EntryBlock->getParent()->getParent()->print(FOS, 0, PrintNumber));

  removeCFG(EntryBlock);

  DEBUG(formatted_raw_ostream FOS(dbgs());
        FOS << "After removing CFG:\n";
        ContainingNode->getParent()->getParent()->print(FOS, 0, PrintNumber));
}

void VPOPredicatorBase::predicate(AVRBlock* Entry) {

  for (auto It = df_iterator<AVRBlock*>::begin(Entry),
         End = df_iterator<AVRBlock*>::end(Entry); It != End; ++It) {

    AVRBlock* ABlock = *It;

    // Every block except the entry must update its mask. We create an assign
    // statement to compute the block's mask of all incoming masks.
    AVRPredicate *APredicate = AVRUtils::createAVRPredicate();
    AVRUtils::insertBefore(AvrItr(ABlock), APredicate);
    AVRUtils::setPredicate(ABlock, APredicate);
    DEBUG(formatted_raw_ostream FOS(dbgs());
          FOS << "Created predicate " << APredicate->getNumber() << " for block "
	  << ABlock->getNumber() << "\n");
  }

  for (auto It = df_iterator<AVRBlock*>::begin(Entry),
         End = df_iterator<AVRBlock*>::end(Entry); It != End; ++It) {

    AVRBlock* ABlock = *It;

    DEBUG(formatted_raw_ostream FOS(dbgs());
          FOS << "Predicating block " << ABlock->getNumber() << "\n");

    for (AVRBlock* Predecessor : ABlock->getPredecessors()) {

      unsigned Ordinal = Predecessor->getSuccessorOrdinal(ABlock);
      DEBUG(formatted_raw_ostream FOS(dbgs());
            FOS << "Handling predecessor block " << Predecessor->getNumber()
	    << " with ordinal " << Ordinal << "\n");
      SmallVector<AVR*, 2> Conditions;
      AVR* Terminator = &*Predecessor->child_rbegin();
      assert(Terminator && "Predecessor has no terminator");

      if (AVRIf *AIf = dyn_cast<AVRIf>(Terminator)) {

        AVR *Condition = AIf->getCondition();
        Type *ConstTy;
        SmallVector<AVR*, 2> Operands;
        if (AVRExpression *AExpr = dyn_cast<AVRExpression>(Condition)) {
          ConstTy = AExpr->getType();
          Operands.push_back(AVRUtils::createAVRValue(AExpr));
        }
        else if (AVRValue *AValue = dyn_cast<AVRValue>(Condition)) {
          ConstTy = AValue->getType();
          Operands.push_back(AValue); // TODO - clone to keep AVR a tree.
        }
        else if (AVRCompareIR *ACompareIR = dyn_cast<AVRCompareIR>(Condition)) {
          // TODO - create a value when compare is an assign (generically for
          // AVRCompare rather than AVRCompareIR).
          Instruction *Inst = ACompareIR->getLLVMInstruction();
          ConstTy = Inst->getType();
          Operands.push_back(ACompareIR);
        }
        else
          llvm_unreachable("Unknown AVR condition");

        Constant *CompareTo = ConstantInt::get(ConstTy, 1 - Ordinal);
        Operands.push_back(AVRUtils::createAVRValue(CompareTo));

        AVRExpression *IncomingCondition = AVRUtils::createAVRExpression(
            Operands, Instruction::ICmp, ConstTy, CmpInst::ICMP_EQ);
        AVRUtils::addAVRPredicateIncoming(ABlock->getPredicate(),
                                          Predecessor->getPredicate(),
                                          IncomingCondition);
      }
      else if (AVRSwitch *ASwitch = dyn_cast<AVRSwitch>(Terminator)) {

        AVRValue* SwitchCondition = ASwitch->getCondition();
        Type *ConstTy = SwitchCondition->getType();
        unsigned NumCases = ASwitch->getNumCases();

        if (Ordinal == 0 && NumCases < Predecessor->getSuccessors().size()) {

          // This block handles the switch's default case. Predicate on the
          // condition 'C != case1 && ... & C != caseN'
          AVRExpression* DefaultCondition = nullptr;
          for (unsigned Case = 1; Case <= NumCases; ++Case) {
            SmallVector<AVR*, 2> Operands;
            Operands.push_back(SwitchCondition); // TODO - clone to keep AVR a tree.

            Constant* CaseConst = ConstantInt::get(ConstTy, Case);
            Operands.push_back(AVRUtils::createAVRValue(CaseConst));

            AVRExpression *IncomingCondition = AVRUtils::createAVRExpression(
                Operands, Instruction::ICmp, ConstTy, CmpInst::ICMP_NE);

            if (DefaultCondition) {
              SmallVector<AVR*, 2> Operands;
              Operands.push_back(DefaultCondition);
              Operands.push_back(IncomingCondition);
              DefaultCondition = AVRUtils::createAVRExpression(Operands,
                                                               Instruction::And,
                                                               ConstTy);
            }
            else {
              DefaultCondition = IncomingCondition;
            }
          }
          AVRUtils::addAVRPredicateIncoming(ABlock->getPredicate(),
                                            Predecessor->getPredicate(),
                                            DefaultCondition);
        }
        else {

          // This block handles one of the switch's cases. Predicate on the
          // condition 'C == caseX'
          SmallVector<AVR*, 2> Operands;
          Operands.push_back(SwitchCondition); // TODO - clone to keep AVR a tree.

          Constant* CaseConst = ConstantInt::get(ConstTy, Ordinal);
          Operands.push_back(AVRUtils::createAVRValue(CaseConst));

          AVRExpression *IncomingCondition = AVRUtils::createAVRExpression(
              Operands, Instruction::ICmp, ConstTy, CmpInst::ICMP_EQ);
          AVRUtils::addAVRPredicateIncoming(ABlock->getPredicate(),
                                            Predecessor->getPredicate(),
                                            IncomingCondition);
        }
      }
      else {
        if (isa<AVRBranch>(Terminator)) {
          
          AVRBranch *ab = cast<AVRBranch>(Terminator);
          errs() << "isConditional?: " << ab->isConditional() << "\ni";

          assert(!cast<AVRBranch>(Terminator)->isConditional() &&
                 "Did not expect conditional branches at this point");
        }
        else 
          // In HIR, there aren't explicit unconditional AVRBranch instructions.
          // By now, we assume that if the terminator is not AVRIf or AVRSwitch,
          // then it has to be an AVRAssig (and meaning unconditional branch).
          assert(isa<AVRAssign>(Terminator) &&
                 "Did not expect an AVR other than an AVRAssign at this point");

        AVRUtils::addAVRPredicateIncoming(ABlock->getPredicate(),
                                          Predecessor->getPredicate(),
                                          nullptr);
      }
    }
  }
}

void VPOPredicatorBase::removeCFG(AVRBlock* Entry) {

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


// LLVM-IR Pass

INITIALIZE_PASS_BEGIN(VPOPredicator, "avr-predicate",
                      "AVR Predicator for LLVM-IR", false, true)
INITIALIZE_PASS_DEPENDENCY(AVRGenerate)
INITIALIZE_PASS_END(VPOPredicator, "avr-predicate",
                    "AVR Predicator for LLVM-IR", false, true)

char VPOPredicator::ID = 0;

FunctionPass *llvm::createVPOPredicatorPass() { return new VPOPredicator(); }

VPOPredicator::VPOPredicator() : FunctionPass(ID) {
  llvm::initializeVPOPredicatorPass(*PassRegistry::getPassRegistry());
}

void VPOPredicator::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.setPreservesAll();
  AU.addRequired<AVRGenerate>();
}

bool VPOPredicator::runOnFunction(Function &F) {

  AVRG = &getAnalysis<AVRGenerate>();
  return VPOPredicatorBase::runOnFunction(F);
}


// HIR Pass

INITIALIZE_PASS_BEGIN(VPOPredicatorHIR, "hir-avr-predicate",
                      "AVR Predicator for HIR", false, true)
INITIALIZE_PASS_DEPENDENCY(AVRGenerateHIR)
INITIALIZE_PASS_END(VPOPredicatorHIR, "hir-avr-predicate",
                    "AVR Predicator for HIR", false, true)

char VPOPredicatorHIR::ID = 0;

FunctionPass *llvm::createVPOPredicatorHIRPass() {
  return new VPOPredicatorHIR();
}

VPOPredicatorHIR::VPOPredicatorHIR() : FunctionPass(ID) {
  llvm::initializeVPOPredicatorHIRPass(*PassRegistry::getPassRegistry());
}

void VPOPredicatorHIR::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.setPreservesAll();
  AU.addRequired<AVRGenerateHIR>();
}

bool VPOPredicatorHIR::runOnFunction(Function &F) {

  AVRG = &getAnalysis<AVRGenerateHIR>();
  return VPOPredicatorBase::runOnFunction(F);
}

