//   Copyright (C) 2015-2016 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation. and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//   Source file:
//   ------------
//   VPOCFG.cpp -- Implements the AVR-level Control Flow Graph.
//
//===----------------------------------------------------------------------===//

#include "llvm/Analysis/Intel_VPO/Vecopt/VPOCFG.h"
#include "llvm/Analysis/Intel_VPO/Vecopt/Passes.h"

#include "llvm/PassSupport.h"
#include "llvm/InitializePasses.h"
#include "llvm/IR/PassManager.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Support/GenericDomTreeConstruction.h"
#include "llvm/Support/GraphWriter.h"
#include "llvm/Support/FileSystem.h"

#define DEBUG_TYPE "vpo-cfg"

using namespace llvm;
using namespace vpo;

const AvrCFGBase::NodeSetTy AvrCFGBase::EmptyNodeSet;

template void llvm::Calculate<AvrCFGBase, AvrBasicBlock *>(
    DominatorTreeBase<GraphTraits<AvrBasicBlock *>::NodeType> &DT, AvrCFGBase &F);
template void llvm::Calculate<AvrCFGBase, Inverse<AvrBasicBlock *>>(
    DominatorTreeBase<GraphTraits<Inverse<AvrBasicBlock *>>::NodeType> &DT,
    AvrCFGBase &F);

unsigned long long AvrBasicBlock::NextId = 0;

AvrCFGBase::BuilderBase::BuilderBase(AvrCFGBase& C,
                             AvrBasicBlock* CurrentPredecessor) : CFG(C) {

  if (CurrentPredecessor)
    setCurrentPredecessor(CurrentPredecessor);
}

void AvrCFGBase::print(raw_ostream &OS, const PathTy& Path) const {
#if !INTEL_PRODUCT_RELEASE

  bool First = true;

  for (AvrBasicBlock* Node : Path) {

    if (!First)
      OS << " -> ";

    if (Node == nullptr)
      OS << "...";
    else
      OS << Node->getId();

    First = false;
  }
#endif // !INTEL_PRODUCT_RELEASE
}

AvrCFGBase::PathTy
AvrCFGBase::findSimplePath(const PathTy& Schema,
                           bool SkipFixedPointSiblings) const {

  std::set<PathTy> Result = findSimplePaths(Schema, true, SkipFixedPointSiblings);
  if (Result.empty())
    return PathTy();
  else
    return *Result.begin();
}

std::set<AvrCFGBase::PathTy>
AvrCFGBase::findSimplePaths(const PathTy& Schema,
                            bool JustOne,
                            bool SkipFixedPointSiblings) const {
  assert(Schema.size() >= 2 && "Looking for a path with less than 2 nodes?");

#if 0
  DEBUG(dbgs() << "Find simple paths:\n";
        dbgs() << "  For: "; print(dbgs(), Schema); dbgs() << "\n";
        dbgs() << "  Just one: " << (JustOne ? "yes" : "no") << "\n";
        dbgs() << "  Skip fixed points siblings: "
               << (SkipFixedPointSiblings ? "yes" : "no") << "\n");
#endif

  std::set<PathTy> Result;
  NodeSetTy InPath;

  // Mark all fixed points in the path as disallowed. Since the fixed points are
  // to appear in the result path in a specific order, we handle them
  // specifically and eliminate paths in advance if the next step is to visit a
  // fixed point that is not the next fixed point according to path schema.
  for (AvrBasicBlock* BB : Schema)
    if (BB != nullptr) {
      if (InPath.count(BB)) {
        // Support the corner case where the fixed points themselves
        // contain a circle.
        return Result;
      }
      InPath.insert(BB);
    }

  PathTy PathSoFar; // Initialize to empty path.
  findSimplePathsImpl(Schema, 0, JustOne, SkipFixedPointSiblings, InPath,
                      PathSoFar, Result);

#if 0
  DEBUG(dbgs() << "Find simple paths results:\n";
        dbgs() << "  For: "; print(dbgs(), Schema); dbgs() << "\n";
        if (Result.empty())
          dbgs() << "  Got: {}\n";
        else {
          dbgs() << "  Got: {\n";
          for (const PathTy& P : Result) {
            dbgs() << "    "; print(dbgs(), P); dbgs() << "\n";
          }
        });
#endif

  return Result;
}

bool AvrCFGBase::findSimplePathsImpl(const PathTy& Schema,
                                     unsigned Index,
                                     bool FindJustOne,
                                     bool SkipFixedPointSiblings,
                                     NodeSetTy& InPath,
                                     PathTy& PathSoFar,
                                     std::set<PathTy>& Result) const {

  AvrBasicBlock* CurrentNode;

#if 0
  DEBUG(dbgs() << "Finding simple path:\n";
        dbgs() << "|-Path: "; print(dbgs(), Path); dbgs() << "\n";
        dbgs() << "|-Index: " << Index << "\n";
        dbgs() << "|-Disallowed:";
        for (auto V : InPath) dbgs() << " " << V->getId();
        dbgs() << "\n";
        dbgs() << "|-Path So Far: "; print(dbgs(), PathSoFar); dbgs() << "\n");
#endif

  // Add to route every fixed point from Index onwards until we run out of
  // fixed points or reach a gap, e.g.:
  //        from here                         to here
  //            |                                |
  // ... ->    BB1      ->     BB2       ->    nullptr    -> ... ->
  // ... -> Path[Index] -> Path[Index+1] -> Path[Index+2] -> ... -> 

  for (; Index < Schema.size(); ++Index) {

    CurrentNode = Schema[Index];
    if (CurrentNode == nullptr)
      break;

    // Sanity check: make sure current node is indeed a successor of the previous
    // node in this consecutive segment of the path.
    assert((Index == 0 ||
            Schema[Index - 1] == nullptr ||
            CurrentNode->isSuccessorOf(Schema[Index - 1])) &&
            "No edge between consecutive nodes in path");

#if 0
    dbgs() << "--- Adding fixed point " << CurrentNode->getId() << "\n";
#endif
    PathSoFar.push_back(CurrentNode);
  }

  // Ran out of fixed points and gaps - all done.
  if (Index == Schema.size()) {
#if 0
    dbgs() << "--- Ran out of fixed points and gaps, all done\n";
#endif
    Result.insert(PathSoFar);
    return true;
  }

  assert(CurrentNode == nullptr && "Expected to be in a gap");

  // We are in a gap. Continue from the current end of the path being
  // constructed.

  assert(PathSoFar.rbegin() != PathSoFar.rend() && "In a gap, but result is empty");

  AvrBasicBlock* CurrentEndOfPath = *PathSoFar.rbegin();
  AvrBasicBlock* NextFixedPoint = Schema[Index + 1];
#if 0
  dbgs() << "--- In gap from " << CurrentEndOfPath->getId()
         << " to " << NextFixedPoint->getId() << "\n";
#endif

  // Try every successor in turn as the next node en route to next fixed
  // point, i.e.          CurrentEndOfPath       NextFixedPoint
  //                               |                  |
  //                               |     {S1}         |
  // ... ->     BB1     -> ... -> BBi -> {..} ->     BB2
  //                                     {Sn}
  // ... ->     BB1     ->      nullptr        ->    BB2        -> ... ->
  // ... -> Path[Index] -> Path[Index+1]       -> Path[Index+2] -> ... ->

  for (AvrBasicBlock* Successor : CurrentEndOfPath->getSuccessors()) {

    if (NextFixedPoint->isSuccessorOf(CurrentEndOfPath)) {

      // We've closed the gap to the next fixed point - just continue from
      // there, i.e.         CurrentEndOfPath  NextFixedPoint
      //                               |             |
      // ... ->     BB1     -> ... -> BBi    ->     BB2
      // ... ->     BB1     ->    nullptr    ->     BB2       -> ... ->
      // ... -> Path[Index] -> Path[Index+1] -> Path[Index+2] -> ... -> 

#if 0
      dbgs() << "--- Closed gap, moving on to fixed point "
             << NextFixedPoint->getId() << "\n";
#endif
      PathTy PathSoFarCopy = PathSoFar;

      bool Found = (findSimplePathsImpl(Schema, Index + 1,
                                        FindJustOne, SkipFixedPointSiblings,
                                        InPath, PathSoFarCopy, Result));
      if (Found && (FindJustOne || SkipFixedPointSiblings))
        return true; // We're done
      else
        continue; // to next successor for other/more alternatives.
    }

    if (InPath.count(Successor))
      continue;

#if 0
    dbgs() << "--- Trying successor " << Successor->getId() << "\n";
#endif
    PathTy PathSoFarCopy = PathSoFar;
    PathSoFarCopy.push_back(Successor);
    InPath.insert(Successor);

    // TODO: avoid the recursion for single successors.
    bool Found = findSimplePathsImpl(Schema, Index, FindJustOne,
                                     SkipFixedPointSiblings, InPath,
                                     PathSoFarCopy, Result);
    if (Found && FindJustOne)
      return true; // We're done

    // Rollback the successor we just tried.
#if 0
    dbgs() << "--- Rolling back successor "
           << Successor->getId() << "\n";
#endif
    InPath.erase(Successor);
  }

  // Recursive calls failed to complete the path. Clear our addition to the
  // path and return.
#if 0
  dbgs() << "--- Didn't find a path down this road\n";
#endif
  return false;
}

AvrBasicBlock* AvrCFGBase::BuilderBase::getOrCreateInstruction(AVR* A) {

  auto It = CFG.BasicBlocks.find(A);
  if (It != CFG.BasicBlocks.end())
    return It->second;

  AvrBasicBlock* BB = CFG.createBasicBlock(A);
  return BB;
}

void AvrCFGBase::BuilderBase::linkWithCurrentPredecessors(AvrBasicBlock* VI) {
  for (auto It : CurrentPredecessors)
    AvrBasicBlock::link(*It, *VI);
}

void AvrCFGBase::BuilderBase::setNextInstruction(AvrBasicBlock* VI) {
  linkWithCurrentPredecessors(VI);
  setCurrentPredecessor(VI);
}

void AvrCFGBase::BuilderBase::construct(AVRCall *ACall) {

  setNextInstruction(getOrCreateInstruction(ACall));
}

void AvrCFGBase::BuilderBase::construct(AVRBackEdge *ABackEdge) {
  llvm_unreachable("AVRBackEdge nodes are never actually constructed");
}

void AvrCFGBase::BuilderBase::construct(AVREntry *AEntry) {
  llvm_unreachable("AVREntry nodes are never actually constructed");
}

void AvrCFGBase::BuilderBase::construct(AVRReturn *AReturn) {

  // Just link with any current predecessor and then clear the predecessors set.
  linkWithCurrentPredecessors(getOrCreateInstruction(AReturn));
  clearCurrentPredecessors();
}

void AvrCFGBase::BuilderBase::construct(AVRSelect *ASelect) {

  setNextInstruction(getOrCreateInstruction(ASelect));
}

void AvrCFGBase::BuilderBase::construct(AVRCompare *ACompare) {

  setNextInstruction(getOrCreateInstruction(ACompare));
}

void AvrCFGBase::BuilderBase::construct(AVRLabel* ALabel) {

  setNextInstruction(getOrCreateInstruction(ALabel));
}

void AvrCFGBase::BuilderBase::construct(AVRBranch* ABranch) {

  AvrBasicBlock* BranchInst = getOrCreateInstruction(ABranch);

  // Just link with any current predecessor and then clear the predecessors set.
  linkWithCurrentPredecessors(BranchInst);
  clearCurrentPredecessors();

  // Explicitly link to all successors.
  for (AVRLabel* ALabel : ABranch->getSuccessors()) {
    AvrBasicBlock* SuccessorInst = getOrCreateInstruction(ALabel);
    AvrBasicBlock::link(*BranchInst, *SuccessorInst);
  }
}

void AvrCFGBase::DeepBuilder::visit(AVRValue* AValue) {

  AVRExpression* TopLevelExpr = nullptr;
  AVR* Parent;
  for (Parent = AValue->getParent();
       Parent != nullptr && isa<AVRExpression>(Parent);
       Parent = Parent->getParent()) {

    assert(isa<AVRExpression>(Parent) && "Expected expression as value parent");
    TopLevelExpr = cast<AVRExpression>(Parent);
  }

  assert(TopLevelExpr && "Couldn't find a top-level expression");

  if (!TopLevelExpr->isLHSExpr()) {

    // RHS values belong in the CFG.
    setNextInstruction(getOrCreateInstruction(AValue));
  }
  else {

    // LHS values go into the CFG only if the RHS is a STORE (the LHS in this
    // case is the address.
    if (Parent) {

      AVRAssign *AAssign = dyn_cast<AVRAssign>(Parent);
      if (AAssign && AAssign->getRHS()) {

        // TODO: address calculation should go before the store, not after.
        AVRExpression *RHS = dyn_cast<AVRExpression>(AAssign->getRHS());
        if (RHS && RHS->getOperation() == Instruction::Store)
          setNextInstruction(getOrCreateInstruction(AValue));
      }
    }
  }
}

void AvrCFGBase::DeepBuilder::postVisit(AVRExpression* AExpr) {

  if (!AExpr->isLHSExpr())
    setNextInstruction(getOrCreateInstruction(AExpr));
}

void AvrCFGBase::DeepBuilder::visit(AVRPhi* APhi) {

  // Handle the incoming values.
  auto& IncomingValues = APhi->getIncomingValues();
  unsigned IncomingNum = IncomingValues.size();
  for (unsigned Ind = 0; Ind < IncomingNum; ++Ind) {

    auto& Incoming = IncomingValues[Ind];
    AVRValue *IncomingValue = Incoming.first;
    if (IncomingValue->isConstant()) {

      // constant values need to go into the corresponding predecessor in order
      // to represent their Def correctly w.r.t control flow.
      AVRLabel *Label = Incoming.second;
      AvrBasicBlock* IncomingBB = getOrCreateInstruction(IncomingValue);
      CFG.PendingIncomingPHIValues[Label].insert(IncomingBB);
    }
    else {

      // Other values are handled as usual.
      setNextInstruction(getOrCreateInstruction(IncomingValue));
    }
  }

  setNextInstruction(getOrCreateInstruction(APhi));
}

void AvrCFGBase::DeepBuilder::visit(AVRIf* AIf) {

  AvrBasicBlock* CurrentPredecessor = nullptr;

  if (isa<AVRIfHIR>(AIf)) {

    // In HIR the condition is only accesible via the AVRIf construct. We visit
    // it now to get the CFG to "execute" it before the 'then' and 'else'
    // blocks. Note that unlike AVRIfIR, AVRHIRIf conditions are explicitly
    // visited by Visitor logic, but not in the order we need (and we stop
    // recursion on AVRIfs anyhow).

    CurrentPredecessor = *CurrentPredecessors.begin();
    AvrCFGBase::DeepBuilder ConditionCFGDeepBuilder(CFG, CurrentPredecessor);
    AVRVisitor<AvrCFGBase::DeepBuilder> ConditionVisitor(ConditionCFGDeepBuilder);
    AVR* Condition = AIf->getCondition();
    ConditionVisitor.visit(Condition, true, true, false /*RecurseInsideValues*/,
                           true);
    clearCurrentPredecessors();
    assert(ConditionCFGDeepBuilder.CurrentPredecessors.size() == 1 &&
           "Expected a single predecessor from the condition visit");
    CurrentPredecessors.insert(*ConditionCFGDeepBuilder.CurrentPredecessors.begin());
  }
  else {

    // In AVR-IR the condition is a member of some container, typically (but
    // not necessarily) immediately before the AVR-IF which only uses its
    // computed value. We therefore must add the AVR-IF itself to the CFG.

    assert(isa<AVRIfIR>(AIf) && "Not an AVRIfIR?");
    setNextInstruction(getOrCreateInstruction(AIf));
  }

  assert(CurrentPredecessors.size() == 1 &&
         "Expected exactly 1 predecessor for AVR-IF");
  CurrentPredecessor = *CurrentPredecessors.begin();
  clearCurrentPredecessors();

  // Recurse into 'then' block.
  AvrCFGBase::DeepBuilder ThenCFGDeepBuilder(CFG, CurrentPredecessor);
  AVRVisitor<AvrCFGBase::DeepBuilder> ThenVisitor(ThenCFGDeepBuilder);
  ThenVisitor.forwardVisit(AIf->then_begin(), AIf->then_end(), true, true,
                           false /*RecurseInsideValues*/);
  // Set any dangling predecessors of the 'then' block as predecessors.
  CurrentPredecessors.insert(ThenCFGDeepBuilder.CurrentPredecessors.begin(),
                             ThenCFGDeepBuilder.CurrentPredecessors.end());

  // Recurse into 'else' block.
  if (AIf->else_begin() != AIf->else_end()) {
    // This is an if-then-else.
    AvrCFGBase::DeepBuilder ElseCFGDeepBuilder(CFG, CurrentPredecessor);
    AVRVisitor<AvrCFGBase::DeepBuilder> ElseVisitor(ElseCFGDeepBuilder);
    ElseVisitor.forwardVisit(AIf->else_begin(), AIf->else_end(), true, true,
                             false /*RecurseInsideValues*/);
    // Set any dangling predecessors of the 'else' block as predecessors.
    CurrentPredecessors.insert(ElseCFGDeepBuilder.CurrentPredecessors.begin(),
                               ElseCFGDeepBuilder.CurrentPredecessors.end());
  }
  else {
    // This is an if-then.
    // Re-insert the condition as a predecessor.
    CurrentPredecessors.insert(CurrentPredecessor);
  }
}

void AvrCFGBase::DeepBuilder::visit(AVRSwitch* ASwitch) {

  AvrBasicBlock* CurrentPredecessor = nullptr;

  setNextInstruction(getOrCreateInstruction(ASwitch->getCondition()));

  CurrentPredecessor = *CurrentPredecessors.begin();
  clearCurrentPredecessors();

  // Recurse into default case first to order it 1st among switch's successors.
  AvrCFGBase::DeepBuilder DefaultCFGDeepBuilder(CFG, CurrentPredecessor);
  AVRVisitor<AvrCFGBase::DeepBuilder> DefaultVisitor(DefaultCFGDeepBuilder);
  DefaultVisitor.forwardVisit(ASwitch->default_case_child_begin(),
                              ASwitch->default_case_child_end(), true, true,
                              false /*RecurseInsideValues*/);
  // Set any dangling predecessors of the case as predecessors.
  CurrentPredecessors.insert(DefaultCFGDeepBuilder.CurrentPredecessors.begin(),
                             DefaultCFGDeepBuilder.CurrentPredecessors.end());

  unsigned NumCases = ASwitch->getNumCases();
  for (unsigned I = 1; I <= NumCases; ++I) {

    // Recurse into case.
    AvrCFGBase::DeepBuilder CaseCFGDeepBuilder(CFG, CurrentPredecessor);
    AVRVisitor<AvrCFGBase::DeepBuilder> CaseVisitor(CaseCFGDeepBuilder);
    CaseVisitor.forwardVisit(ASwitch->case_child_begin(I),
                             ASwitch->case_child_end(I), true, true,
                             false /*RecurseInsideValues*/);
    // Set any dangling predecessors of the case as predecessors.
    CurrentPredecessors.insert(CaseCFGDeepBuilder.CurrentPredecessors.begin(),
                               CaseCFGDeepBuilder.CurrentPredecessors.end());
  }
}

void AvrCFGBase::DeepBuilder::visit(AVRLoopHIR *ALoopHIR) {

  // Create an empty basic block with as a temporary header.
  AvrBasicBlock* TempHeader = CFG.createBasicBlock();
  setNextInstruction(TempHeader);

  // Recurse into loop body.
  AvrCFGBase::DeepBuilder LoopCFGDeepBuilder(CFG, TempHeader);
  AVRVisitor<AvrCFGBase::DeepBuilder> LoopVisitor(LoopCFGDeepBuilder);
  LoopVisitor.forwardVisit(ALoopHIR->child_begin(), ALoopHIR->child_end(), true,
                           true, false /*RecurseInsideVisitor*/);

  // Create an empty basic block as the latch, link it to the temporary header
  // and then remove the temporary header.
  AvrBasicBlock* Latch = CFG.createBasicBlock();
  AvrBasicBlock::link(*Latch, *TempHeader);
  assert(TempHeader->Successors.size() == 1 && "Loop body with two entries?");
  CFG.deleteBasicBlock(TempHeader);

  // Set any dangling predecessors of loop body as predecessors of the latch.
  for (AvrBasicBlock* DanglingBB : LoopCFGDeepBuilder.CurrentPredecessors)
    AvrBasicBlock::link(*DanglingBB, *Latch);

  // Set the latch as the current predecessor of the CFG.
  setCurrentPredecessor(Latch);
}

void AvrCFGBase::ShallowBuilder::visit(AVRAssign* AAssign) {
  setNextInstruction(getOrCreateInstruction(AAssign));
}

void AvrCFGBase::ShallowBuilder::visit(AVRPhi* APhi) {
  setNextInstruction(getOrCreateInstruction(APhi));
}

void AvrCFGBase::ShallowBuilder::visit(AVRIf* AIf) {

  AvrBasicBlock* CurrentPredecessor = nullptr;

  setNextInstruction(getOrCreateInstruction(AIf));

  CurrentPredecessor = *CurrentPredecessors.begin();
  clearCurrentPredecessors();

  // Recurse into 'then' block.
  AvrCFGBase::ShallowBuilder ThenCFGShallowBuilder(CFG, CurrentPredecessor);
  AVRVisitor<AvrCFGBase::ShallowBuilder> ThenVisitor(ThenCFGShallowBuilder);
  ThenVisitor.forwardVisit(AIf->then_begin(), AIf->then_end(), true, true,
                           false /*RecurseInsideValues*/);
  // Set any dangling predecessors of the 'then' block as predecessors.
  CurrentPredecessors.insert(ThenCFGShallowBuilder.CurrentPredecessors.begin(),
                             ThenCFGShallowBuilder.CurrentPredecessors.end());

  // Recurse into 'else' block.
  if (AIf->else_begin() != AIf->else_end()) {
    // This is an if-then-else.
    AvrCFGBase::ShallowBuilder ElseCFGShallowBuilder(CFG, CurrentPredecessor);
    AVRVisitor<AvrCFGBase::ShallowBuilder> ElseVisitor(ElseCFGShallowBuilder);
    ElseVisitor.forwardVisit(AIf->else_begin(), AIf->else_end(), true, true,
                             false /*RecurseInsideValues*/);
    // Set any dangling predecessors of the 'else' block as predecessors.
    CurrentPredecessors.insert(ElseCFGShallowBuilder.CurrentPredecessors.begin(),
                               ElseCFGShallowBuilder.CurrentPredecessors.end());
  }
  else {
    // This is an if-then.
    // Re-insert the AVRIf as a predecessor.
    CurrentPredecessors.insert(CurrentPredecessor);
  }
}

void AvrCFGBase::ShallowBuilder::visit(AVRSwitch* ASwitch) {

  AvrBasicBlock* CurrentPredecessor = nullptr;

  setNextInstruction(getOrCreateInstruction(ASwitch));

  CurrentPredecessor = *CurrentPredecessors.begin();
  clearCurrentPredecessors();

  // Recurse into default case first to order it 1st among switch's successors.
  AvrCFGBase::ShallowBuilder DefaultCFGShallowBuilder(CFG, CurrentPredecessor);
  AVRVisitor<AvrCFGBase::ShallowBuilder> DefaultVisitor(DefaultCFGShallowBuilder);
  DefaultVisitor.forwardVisit(ASwitch->default_case_child_begin(),
                              ASwitch->default_case_child_end(), true, true,
                              false /*RecurseInsideValues*/);
  // Set any dangling predecessors of the case as predecessors.
  CurrentPredecessors.insert(DefaultCFGShallowBuilder.CurrentPredecessors.begin(),
                             DefaultCFGShallowBuilder.CurrentPredecessors.end());

  unsigned NumCases = ASwitch->getNumCases();
  for (unsigned I = 1; I <= NumCases; ++I) {

    // Recurse into case.
    AvrCFGBase::ShallowBuilder CaseCFGShallowBuilder(CFG, CurrentPredecessor);
    AVRVisitor<AvrCFGBase::ShallowBuilder> CaseVisitor(CaseCFGShallowBuilder);
    CaseVisitor.forwardVisit(ASwitch->case_child_begin(I),
                             ASwitch->case_child_end(I), true, true,
                             false /*RecurseInsideValues*/);
    // Set any dangling predecessors of the case as predecessors.
    CurrentPredecessors.insert(CaseCFGShallowBuilder.CurrentPredecessors.begin(),
                               CaseCFGShallowBuilder.CurrentPredecessors.end());
  }
}

void AvrCFGBase::ShallowBuilder::visit(AVRLoop *ALoop) {
  setNextInstruction(getOrCreateInstruction(ALoop));
}

AvrCFGBase::AvrCFGBase(AvrItr Begin, AvrItr End,
                       const std::string& T,
                       bool Deep,
                       bool Compress) : Title(T) {
  
  Entry = nullptr;
  Exit = nullptr;
  Size = 0;

  runOnAvr(Begin, End, Deep, Compress);
}

AvrCFGBase::~AvrCFGBase() {

  // Delete any existing basic block in the CFG.
  SmallVector<AvrBasicBlock*, 8> Worklist;
  assert(Entry && "No entry for last constructed CFG");
  Worklist.push_back(Entry);
  while (!Worklist.empty()) {
    AvrBasicBlock* BB = Worklist.back();
    Worklist.pop_back();
    for (AvrBasicBlock* Successor : BB->Successors)
      if (Successor != BB) {
        Worklist.push_back(Successor);
      }
    deleteBasicBlock(BB, false);
  }
}

void AvrCFGBase::runOnAvr(AvrItr Begin, AvrItr End, bool Deep, bool Compress) {

  // Create a temporary entry node. We remove this node after construction
  // if it is empty and its only successor has no other predecessors.
  Entry = createBasicBlock();

  BuilderBase* Builder;

  // Walk down the AVR tree and generate the CFG.
  if (Deep) {
    DeepBuilder &CFGDeepBuilder = *new DeepBuilder(*this, Entry);
    AVRVisitor<DeepBuilder> AVisitor(CFGDeepBuilder);
    AVisitor.forwardVisit(Begin, End, true, true,
                          false /*RecurseInsideValues*/);
    Builder = &CFGDeepBuilder;
  }
  else {
    ShallowBuilder &CFGShallowBuilder = * new ShallowBuilder(*this, Entry);
    AVRVisitor<ShallowBuilder> AVisitor(CFGShallowBuilder);
    AVisitor.forwardVisit(Begin, End, true, true,
                          false /*RecurseInsideValues*/);
    Builder = &CFGShallowBuilder;
  }

  // Add all pending phi-incoming values to the CFG right after the designated
  // label.
  for (auto& It : PendingIncomingPHIValues) {

    assert(BasicBlocks.count(It.first) && "Phi-incoming label not in CFG");
    AvrBasicBlock* LabelBB = BasicBlocks[It.first];
    for (AvrBasicBlock* IncomingBB : It.second)
      insertAfter(LabelBB, IncomingBB);
  }
  PendingIncomingPHIValues.clear();

  // If the current entry basic block is empty and its successor has no other
  // predecessors we can set its successor as the entry node.
  assert(Entry->Successors.size() == 1 &&
         "Entry node expected to have exactly one successor");
  if (Entry->Instructions.empty() &&
      (*Entry->Successors.begin())->Predecessors.size() == 1) {
    AvrBasicBlock* DummyEntry = Entry;
    Entry = *Entry->getSuccessors().begin();
    deleteBasicBlock(DummyEntry);
  }

  // Find the exit node of the CFG. If there isn't one, create a dummy node
  // (that points to no AVR node) and place it as the exit node to complete the
  // CFG. The latter can happen either since:
  // - There are multiple nodes with no successors
  // - The visitor terminated with more than a single basic blocks waiting for
  //   its successors. This can happen when some branching construct was the
  //   last instruction and its fall-through was left dangling (e.g. an if-then
  //   which is the last node in the AVR tree still has the condition waiting
  //   for its second successor (note that the dangling last instruction from
  //   the 'then' block will be detected as a 'dead-end' which needs to be
  //   linked to the exit node).

  const SmallPtrSetImpl<AvrBasicBlock*>& LastBasicBlocks =
    Builder->getPredecessors();

  SmallPtrSet<AvrBasicBlock*, 1> deadEnds;
  for (AvrBasicBlock* CFGInst : depth_first(Entry))
    if (CFGInst->getSuccessorsNum() == 0 || LastBasicBlocks.count(CFGInst)) {

      deadEnds.insert(CFGInst);
    }

  if (deadEnds.size() == 1 && (*deadEnds.begin())->Successors.size() == 0) {

    // If the only basic block we collected has no successors it can function as
    // the exit node.
    Exit = *deadEnds.begin();
  }
  else {

    // Otherwise - create a dedicated exit node.
    Exit = createBasicBlock();
    for (AvrBasicBlock* deadEnd : deadEnds)
      AvrBasicBlock::link(*deadEnd, *Exit);
  }

  if (Compress)
    compress();

  delete Builder;
}

void AvrCFGBase::compress() {

  // Compress the CFG: repeatedly merge basic block B to A if A is B's single
  // predecessor and B is A's single successor.
  SmallVector<AvrBasicBlock*, 8> Worklist;
  SmallPtrSet<AvrBasicBlock*, 32> Visited;
  Worklist.push_back(Entry);

  while (!Worklist.empty()) {
    AvrBasicBlock* BB = Worklist.back();
    Worklist.pop_back();

    if (Visited.count(BB))
      continue;
    Visited.insert(BB);

    while (mergeWithDominatedSuccessor(BB));
    for (AvrBasicBlock* Successor : BB->Successors)
      Worklist.push_back(Successor);
  }
}

void AvrCFGBase::print(raw_ostream &OS) const {
#if !INTEL_PRODUCT_RELEASE

  static std::string Indent(TabLength, ' ');

  if (!getEntry()) {
    OS << "AVR CFG for " << Title << " is empty\n";
    return;
  }

  formatted_raw_ostream FOS(OS);
  FOS << "AVR CFG for " << Title << ":\n"
      << "  Entry: " << Entry->Id << "\n"
      << "  Exit: " << Exit->Id << "\n";
  for (auto It = df_begin(this), E = df_end(this); It != E; ++It) {
    const AvrBasicBlock* BB = *It;
    FOS << Indent << "BasicBlock " << BB->Id << ":\n"
        << Indent << Indent << "Instructions:\n";
    for (AVR* I : BB->Instructions) {
      FOS << Indent << Indent << Indent;
      I->shallowPrint(FOS);
      FOS << "\n";
    }
    FOS << Indent << Indent << "Predecessors:";
    for (AvrBasicBlock* Pred : BB->Predecessors)
      FOS << " " << Pred->Id;
    FOS << "\n" << Indent << Indent << "Successors:";
    for (AvrBasicBlock* Succ : BB->Successors)
      FOS << " " << Succ->Id;
    FOS << "\n";
  }
#endif // !INTEL_PRODUCT_RELEASE
}

void AvrCFGBase::printDot(raw_ostream &O, bool ShortNames) {
#if !INTEL_PRODUCT_RELEASE

  static std::string TitleHeader("AVR Control Flow Graph for ");

  if (!getEntry()) {
    O << "AVR CFG is empty";
    return;
  }

  WriteGraph<AvrBasicBlock*>(O, getEntry(), ShortNames, TitleHeader + Title);
#endif // !INTEL_PRODUCT_RELEASE
}

void AvrCFGBase::insertAfter(AvrBasicBlock* BB, AvrBasicBlock* NewSuccessor) {

  assert(NewSuccessor->Predecessors.empty() &&
         NewSuccessor->Successors.empty() &&
         "New successor is already in the CFG");

  for (AvrBasicBlock* Successor : BB->Successors) {
    AvrBasicBlock::link(*NewSuccessor, *Successor);
    AvrBasicBlock::unlink(*BB, *Successor);
  }

  AvrBasicBlock::link(*BB, *NewSuccessor);
}

void AvrCFGBase::deleteBasicBlock(AvrBasicBlock* BB,
                                  bool ConnectPredecessorsToSuccessors) {

  AvrBasicBlock::CFGEdgesTy Predecessors;
  AvrBasicBlock::CFGEdgesTy Successors;

  if (ConnectPredecessorsToSuccessors) {
    Predecessors = BB->Predecessors;
    Successors = BB->Successors;
  }

  AvrBasicBlock::unlink(*BB);

  delete BB;
  Size--;

  if (ConnectPredecessorsToSuccessors)
    for (AvrBasicBlock* Predecessor : Predecessors)
      for (AvrBasicBlock* Successor : Successors)
        AvrBasicBlock::link(*Predecessor, *Successor);
}

bool AvrCFGBase::mergeWithDominatedSuccessor(AvrBasicBlock* BB) {

  if (BB->getSuccessorsNum() != 1 ||
      (*BB->Successors.begin())->getPredecessorsNum() != 1)
    return false;

  // Do not merge basic blocks that terminate with an explicit BRANCH.
  // Although such a BRANCH is necessarily unconditional (or we would have
  // failed the above single-successor check) which makes the BRANCH/LABEL pair
  // meaningless w.r.t the CFG, we keep the blocks separate since phi nodes may
  // explicitly refer to the LABEL.
  if (!BB->Instructions.empty() && isa<AVRBranch>(BB->Instructions.back()))
    return false;

  AvrBasicBlock* Successor = *(BB->Successors.begin());

  // Shouldn't happen - it's a single basic block pointing to
  // itself.
  assert(Successor != BB && "Single BB infinite loop detected");

#if 0
  // Do not merge the succesor if it is a branch condition.
  // While we definitely can merge, this will complicate user's code involving
  // finding simple paths between instructions which would need to take into
  // account cases where some of the instructions along the path share a basic
  // block. By keeping the instructions that affect control-flow in their own
  // basic blocks we allow the user to assume that these instructions (which
  // are often part of the path of interest) do not share a basic block with any
  // other instruction.

  if (!Successor->Instructions.empty() &&
      isBranchCondition(Successor->Instructions.front()))
    return false;
#endif

  // Append Successors's Instructions to BB's
  for (AVR* MergedInstruction : Successor->Instructions) {
    BB->Instructions.push_back(MergedInstruction);
    BasicBlocks[MergedInstruction] = BB;
  }

  // If Successor is the exit node, set BB as the new exit.
  if (Exit == Successor)
    Exit = BB;

  // Add Successor's successors as BB's successors.
  for (auto It : Successor->Successors) {
    AvrBasicBlock::link(*BB, *It);
  }

  // Delete Successor.
  deleteBasicBlock(Successor, false);

  return true;
}

INITIALIZE_PASS_BEGIN(AvrCFG, "avr-cfg",
                      "VPO AVR Control Flow Graph",
                      false, true)
INITIALIZE_PASS_DEPENDENCY(AVRGenerate)
INITIALIZE_PASS_END(AvrCFG, "avr-cfg",
                    "VPO AVR Control Flow Graph",
                    false, true)

char AvrCFG::ID = 0;

AvrCFG::AvrCFG() : FunctionPass(ID) {
}

AvrCFG::~AvrCFG() {
}

bool AvrCFG::runOnFunction(Function &F) {

  AVRGenerate& AV = getAnalysis<AVRGenerate>();

  if (AV.isAbstractLayerEmpty())
    return false;

  CFG = new AvrCFGBase(AV.begin(), AV.end(), F.getName(), true , true);

  return false;
}

void AvrCFG::print(raw_ostream &OS, const Module *M) const {
#if !INTEL_PRODUCT_RELEASE

  if (!CFG) {
    OS << "AVR CFG is empty\n";
    return;
  }

  CFG->print(OS);
#endif // !INTEL_PRODUCT_RELEASE
}

FunctionPass *llvm::createAvrCFGPass() {
  return new AvrCFG();
}

INITIALIZE_PASS_BEGIN(AvrCFGHIR, "avr-cfg-hir",
                      "VPO AVR-HIR Control Flow Graph",
                      false, true)
INITIALIZE_PASS_DEPENDENCY(AVRGenerateHIR)
INITIALIZE_PASS_END(AvrCFGHIR, "avr-cfg-hir",
                    "VPO AVR-HIR Control Flow Graph",
                    false, true)

char AvrCFGHIR::ID = 0;

AvrCFGHIR::AvrCFGHIR() : FunctionPass(ID) {
}

AvrCFGHIR::~AvrCFGHIR() {
}

bool AvrCFGHIR::runOnFunction(Function &F) {

  AVRGenerateHIR& AV = getAnalysis<AVRGenerateHIR>();

  if (AV.isAbstractLayerEmpty())
    return false;

  CFG = new AvrCFGBase(AV.begin(), AV.end(), F.getName(), true, true);

  return false;
}

void AvrCFGHIR::print(raw_ostream &OS, const Module *M) const {
#if !INTEL_PRODUCT_RELEASE

  if (!CFG) {
    OS << "AVR CFG is empty\n";
    return;
  }

  CFG->print(OS);
#endif // !INTEL_PRODUCT_RELEASE
}

FunctionPass *llvm::createAvrCFGHIRPass() {
  return new AvrCFGHIR();
}
