//===- IntelVPBasicBlock.cpp ----------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "IntelVPBasicBlock.h"
#include "IntelVPLoopAnalysis.h"
#include "IntelVPOCodeGen.h"
#include "IntelVPlan.h"
#include "IntelVPlanDominatorTree.h"
#include "IntelVPlanVLSAnalysis.h" // We might need to add in in VPlanHIR/IntelVPOCodeGenHIR.h
#include "VPlanHIR/IntelVPOCodeGenHIR.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"

#define DEBUG_TYPE "VPBasicBlock"

using namespace llvm;
using namespace llvm::vpo;

void VPBlockUtils::insertBlockBefore(VPBasicBlock *NewBB,
                                     VPBasicBlock *BlockPtr) {
  insertBlockBefore(NewBB, BlockPtr, BlockPtr->getPredecessors());
}

void VPBlockUtils::insertBlockBefore(VPBasicBlock *NewBB,
                                     VPBasicBlock *BlockPtr,
                                     SmallVectorImpl<VPBasicBlock *> &Preds) {
  VPlan *CurPlan = BlockPtr->getParent();
  movePredecessors(BlockPtr, NewBB, Preds);
  NewBB->setParent(CurPlan);
  connectBlocks(NewBB, BlockPtr);
  CurPlan->setSize(CurPlan->getSize() + 1);
  // If BlockPtr is Plan's entry, set NewBlock as Plan's entry.
  if (CurPlan->getEntryBlock() == BlockPtr)
    CurPlan->setEntryBlock(NewBB);
}

void VPBlockUtils::insertBlockAfter(VPBasicBlock *NewBB,
                                    VPBasicBlock *BlockPtr) {
  VPlan *CurPlan = BlockPtr->getParent();
  moveSuccessors(BlockPtr, NewBB);
  NewBB->setParent(CurPlan);
  connectBlocks(BlockPtr, NewBB);
  CurPlan->setSize(CurPlan->getSize() + 1);
  // If BlockPtr is Plan's exit, set NewBlock as Plan's exit.
  if (CurPlan->getExitBlock() == BlockPtr)
    CurPlan->setExitBlock(NewBB);
}

void VPBlockUtils::connectBlocks(VPBasicBlock *From, VPBasicBlock *To) {
  assert((From->getParent() == To->getParent()) &&
         "Can't connect two block with different parents");
  assert(From->getNumSuccessors() < 2 &&
         "Blocks can't have more than two successors.");
  From->appendSuccessor(To);
  To->appendPredecessor(From);
}

void VPBlockUtils::connectBlocks(VPBasicBlock *From, VPValue *ConditionV,
                                 VPBasicBlock *IfTrue, VPBasicBlock *IfFalse) {
  From->setTwoSuccessors(ConditionV, IfTrue, IfFalse);
  IfTrue->appendPredecessor(From);
  IfFalse->appendPredecessor(From);
}

void VPBlockUtils::disconnectBlocks(VPBasicBlock *From, VPBasicBlock *To) {
  assert(To && "Successor to disconnect is null.");
  From->removeSuccessor(To);
  To->removePredecessor(From);
}

void VPBlockUtils::replaceBlockSuccessor(VPBasicBlock *BB,
                                         VPBasicBlock *OldSuccessor,
                                         VPBasicBlock *NewSuccessor) {
  // Replace successor
  // TODO: Add VPBasicBlock::replaceSuccessor. Let's not modify VPlan.h too
  // much by now
  auto &Successors = BB->getSuccessors();
  auto SuccIt = std::find(Successors.begin(), Successors.end(), OldSuccessor);
  assert(SuccIt != Successors.end() && "Successor not found");
  SuccIt = Successors.erase(SuccIt);
  Successors.insert(SuccIt, NewSuccessor);
}

void VPBlockUtils::replaceBlockPredecessor(VPBasicBlock *BB,
                                           VPBasicBlock *OldPredecessor,
                                           VPBasicBlock *NewPredecessor) {
  // Replace predecessor
  // TODO: Add VPBasicBlock::replacePredecessor. Let's not modify VPlan.h too
  // much by now
  auto &Predecessors = BB->getPredecessors();
  auto PredIt =
      std::find(Predecessors.begin(), Predecessors.end(), OldPredecessor);
  assert(PredIt != Predecessors.end() && "Predecessor not found");
  PredIt = Predecessors.erase(PredIt);
  Predecessors.insert(PredIt, NewPredecessor);
}

void VPBlockUtils::movePredecessor(VPBasicBlock *Pred, VPBasicBlock *From,
                                   VPBasicBlock *To) {
  replaceBlockSuccessor(Pred, From /*OldSuccessor*/, To /*NewSuccessor*/);
  To->appendPredecessor(Pred);
  From->removePredecessor(Pred);
}

void VPBlockUtils::movePredecessors(
    VPBasicBlock *From, VPBasicBlock *To,
    SmallVectorImpl<VPBasicBlock *> &Predecessors) {
  for (auto &Pred : Predecessors) {
    replaceBlockSuccessor(Pred, From, To);
    To->appendPredecessor(Pred);
    From->removePredecessor(Pred);
  }
}

void VPBlockUtils::movePredecessors(VPBasicBlock *From, VPBasicBlock *To) {
  movePredecessors(From, To, From->getPredecessors());
}

void VPBlockUtils::moveSuccessors(VPBasicBlock *From, VPBasicBlock *To) {
  auto &Successors = From->getSuccessors();

  for (auto &Succ : Successors) {
    replaceBlockPredecessor(Succ, From, To);
    To->appendSuccessor(Succ);
  }

  // Remove successors from From
  Successors.clear();
}

bool VPBlockUtils::isBackEdge(const VPBasicBlock *FromBB,
                              const VPBasicBlock *ToBB,
                              const VPLoopInfo *VPLI) {

  assert(FromBB->getParent() == ToBB->getParent() &&
         FromBB->getParent() != nullptr && "Must be in same region");
  const VPLoop *FromLoop = VPLI->getLoopFor(FromBB);
  const VPLoop *ToLoop = VPLI->getLoopFor(ToBB);
  if (FromLoop == nullptr || ToLoop == nullptr || FromLoop != ToLoop) {
    return false;
  }
  // A back-edge is latch->header
  return (ToBB == ToLoop->getHeader() && ToLoop->isLoopLatch(FromBB));
}

bool VPBlockUtils::blockIsLoopLatch(const VPBasicBlock *BB,
                                    const VPLoopInfo *VPLInfo) {

  if (const VPLoop *ParentVPL = VPLInfo->getLoopFor(BB)) {
    return ParentVPL->isLoopLatch(BB);
  }

  return false;
}

// It turns A->B into A->NewSucc->B and updates VPLoopInfo, DomTree and
// PostDomTree accordingly.
VPBasicBlock *VPBlockUtils::splitBlock(VPBasicBlock *BB,
                                       VPBasicBlock::iterator BeforeIt,
                                       VPLoopInfo *VPLInfo,
                                       VPDominatorTree *DomTree,
                                       VPPostDominatorTree *PostDomTree) {
  VPBasicBlock *NewBB = BB->splitBlock(BeforeIt);

  // Add NewBB to VPLoopInfo
  if (VPLoop *Loop = VPLInfo->getLoopFor(BB)) {
    Loop->addBasicBlockToLoop(NewBB, *VPLInfo);
  }
  if (DomTree) {
    // Update dom information
    VPDomTreeNode *BBDT = DomTree->getNode(BB);
    assert(BBDT && "Expected node in dom tree!");
    SmallVector<VPDomTreeNode *, 2> BlockDTChildren(BBDT->begin(), BBDT->end());
    // BB is NewBB's idom.
    VPDomTreeNode *NewBBDT = DomTree->addNewBlock(NewBB, BB /*IDom*/);

    // NewBB dominates all other nodes dominated by BB.
    for (VPDomTreeNode *Child : BlockDTChildren)
      DomTree->changeImmediateDominator(Child, NewBBDT);
  }

  if (PostDomTree) {
    // Update postdom information
    VPDomTreeNode *NewBBPDT;
    if (VPBasicBlock *NewBBSucc = NewBB->getSingleSuccessor()) {
      // NewBB has a single successor. That successor is NewBB's ipostdom.
      NewBBPDT = PostDomTree->addNewBlock(NewBB, NewBBSucc /*IDom*/);
    } else {
      // NewBB has multiple successors. NewBB's ipostdom is the nearest
      // common post-dominator of both successors.

      // TODO: getSuccessor(0)
      auto &Successors = NewBB->getSuccessors();
      VPBasicBlock *Succ1 = *Successors.begin();
      VPBasicBlock *Succ2 = *std::next(Successors.begin());

      NewBBPDT = PostDomTree->addNewBlock(
          NewBB, PostDomTree->findNearestCommonDominator(Succ1, Succ2));
    }

    VPDomTreeNode *BBPDT = PostDomTree->getNode(BB);
    assert(BBPDT && "Expected node in post-dom tree!");

    // TODO: remove getBlock?
    if (BBPDT->getIDom()->getBlock() == NewBBPDT->getIDom()->getBlock()) {
      // BB's old ipostdom is the same as NewBB's ipostdom. BB's new
      // ipostdom is NewBB
      PostDomTree->changeImmediateDominator(BBPDT, NewBBPDT);

    } else {
      // Otherwise, BB's new ipostdom is the nearest common post-dominator of
      // NewBB and BB's old ipostdom
      PostDomTree->changeImmediateDominator(
          BBPDT, PostDomTree->getNode(PostDomTree->findNearestCommonDominator(
                     NewBB, BBPDT->getIDom()->getBlock())));
    }
  }

  return NewBB;
}

VPBasicBlock *VPBlockUtils::splitBlockBegin(VPBasicBlock *BB,
                                            VPLoopInfo *VPLInfo,
                                            VPDominatorTree *DomTree,
                                            VPPostDominatorTree *PostDomTree) {
  return splitBlock(BB, BB->begin(), VPLInfo, DomTree, PostDomTree);
}

VPBasicBlock *VPBlockUtils::splitBlockEnd(VPBasicBlock *BB, VPLoopInfo *VPLInfo,
                                          VPDominatorTree *DomTree,
                                          VPPostDominatorTree *PostDomTree) {
  // TODO: Once terminators are implemented, we should split before the
  // terminator, not before the end iterator.
  return splitBlock(BB, BB->end(), VPLInfo, DomTree, PostDomTree);
}

void VPBasicBlock::appendSuccessor(VPBasicBlock *Successor) {
  assert(Successor && "Cannot add nullptr successor!");
  Successors.push_back(Successor);
}

void VPBasicBlock::appendPredecessor(VPBasicBlock *Predecessor) {
  assert(Predecessor && "Cannot add nullptr predecessor!");
  Predecessors.push_back(Predecessor);
}

void VPBasicBlock::removePredecessor(VPBasicBlock *Predecessor) {
  auto Pos = std::find(Predecessors.begin(), Predecessors.end(), Predecessor);
  assert(Pos && "Predecessor does not exist");
  Predecessors.erase(Pos);
}

void VPBasicBlock::removeSuccessor(VPBasicBlock *Successor) {
  auto Pos = std::find(Successors.begin(), Successors.end(), Successor);
  assert(Pos && "Successor does not exist");
  Successors.erase(Pos);
}

const VPBasicBlock *VPBasicBlock::getUniquePredecessor() const {
  auto PI = Predecessors.begin();
  auto E = Predecessors.end();
  if (PI == E)
    return nullptr; // No preds.
  const VPBasicBlock *PredBB = *PI;
  ++PI;
  for (; PI != E; ++PI) {
    if (*PI != PredBB)
      return nullptr;
    // The same predecessor appears multiple times in the predecessor list.
    // This is OK.
  }
  return PredBB;
}

void VPBasicBlock::setOneSuccessor(VPBasicBlock *Successor) {
  assert(Successors.empty() && "Setting one successor when others exist.");
  appendSuccessor(Successor);
}

void VPBasicBlock::setTwoSuccessors(VPValue *ConditionV, VPBasicBlock *IfTrue,
                                    VPBasicBlock *IfFalse) {
  assert(Successors.empty() && "Setting two successors when others exist.");
  setCondBit(ConditionV);
  appendSuccessor(IfTrue);
  appendSuccessor(IfFalse);
}

void VPBasicBlock::setPredecessors(ArrayRef<VPBasicBlock *> NewPreds) {
  assert(Predecessors.empty() && "Block predecessors already set.");
  for (auto *Pred : NewPreds)
    appendPredecessor(Pred);
}

void VPBasicBlock::insert(VPInstruction *Instruction, iterator InsertPt) {
  assert(Instruction && "No instruction to append.");
  assert(!Instruction->Parent && "Instruction already in VPlan");
  Instruction->Parent = this;
  Instructions.insert(InsertPt, Instruction);
}

void VPBasicBlock::appendInstruction(VPInstruction *Instruction) {
  insert(Instruction, end());
}

void VPBasicBlock::addInstructionAfter(VPInstruction *Instruction,
                                       VPInstruction *After) {
  Instruction->Parent = this;
  if (!After) {
    Instructions.insert(Instructions.begin(), Instruction);
  } else {
    Instructions.insertAfter(After->getIterator(), Instruction);
  }
}

void VPBasicBlock::addInstruction(VPInstruction *Instruction,
                                  VPInstruction *Before) {
  Instruction->Parent = this;
  if (!Before) {
    Instructions.insert(Instructions.end(), Instruction);
  } else {
    assert(Before->Parent == this &&
           "Insertion before point not in this basic block.");
    Instructions.insert(Before->getIterator(), Instruction);
  }
}

// TODO: Please, remove this interface once C/T/F blocks have been removed.
void VPBasicBlock::moveConditionalEOBTo(VPBasicBlock *ToBB) {
  // Set CondBit in NewBlock. Note that we are only setting the
  // successor selector pointer. The CondBit is kept in its
  // original VPBB instructions list.
  if (getNumSuccessors() > 1) {
    assert(getCondBit() && "Missing CondBit");
    ToBB->setCondBit(getCondBit());
    ToBB->setCBlock(CBlock);
    ToBB->setTBlock(TBlock);
    ToBB->setFBlock(FBlock);
    setCondBit(nullptr);
    CBlock = TBlock = FBlock = nullptr;
  }
}

void VPBasicBlock::eraseInstruction(VPInstruction *Instruction) {
  // We need to remove all instruction operands before erasing the
  // VPInstruction. Else this breaks the use-def chains.
  while (Instruction->getNumOperands())
    Instruction->removeOperand(0);

  Instructions.erase(Instruction);
}

void VPBasicBlock::execute(VPTransformState *State) {
  bool Replica = State->Instance &&
                 !(State->Instance->Part == 0 && State->Instance->Lane == 0);
  VPBasicBlock *PrevVPBB = State->CFG.PrevVPBB;
  VPBasicBlock *SinglePred = nullptr;
  BasicBlock *NewBB = State->CFG.PrevBB; // Reuse it if possible.

  VPLoopInfo *VPLI = State->VPLI;

  // TODO: Won't take place with explicit peel/reminder. But we'd need much more
  // fixes to support CG for such case anyway.
  assert(std::distance(VPLI->begin(), VPLI->end()) == 1 &&
         "Expected single outermost loop!");

  VPLoop *OuterMostVPLoop = *VPLI->begin();

  // 1. Create an IR basic block, or reuse one already available if possible.
  // The last IR basic block is reused in four cases:
  // A. the first VPBB reuses the pre-header BB - when PrevVPBB is null;
  // B. the second VPBB reuses the header BB;
  // C. when the current VPBB has a single (hierarchical) predecessor which
  //    is PrevVPBB and the latter has a single (hierarchical) successor; and
  // D. when the current VPBB is an entry of a region replica - where PrevVPBB
  //    is the exit of this region from a previous instance.
  if (OuterMostVPLoop->getHeader() == this) {
    // Set NewBB to loop H basic block
    BasicBlock *LoopPH = State->CFG.PrevBB;
    NewBB = LoopPH->getSingleSuccessor();
    assert(NewBB && "Expected single successor from loop pre-header");
    State->Builder.SetInsertPoint(NewBB->getTerminator());
    State->CFG.PrevBB = NewBB;
  } else if (PrevVPBB /* A */ &&
             !((SinglePred = getSinglePredecessor()) &&
               SinglePred == PrevVPBB &&
               PrevVPBB->getSingleSuccessor()) &&       /* C */
             !(Replica && getPredecessors().empty())) { /* D */

    NewBB = createEmptyBasicBlock(State);
    State->Builder.SetInsertPoint(NewBB);
    // Temporarily terminate with unreachable until CFG is rewired.
    UnreachableInst *Terminator = State->Builder.CreateUnreachable();
    State->Builder.SetInsertPoint(Terminator);
    // Register NewBB in its loop. In innermost loops its the same for all
    // BB's.
    Loop *L = State->LI->getLoopFor(State->CFG.LastBB);
    assert(L && "Unexpected null loop for Last BasicBlock");
    L->addBasicBlockToLoop(NewBB, *State->LI);
    State->CFG.PrevBB = NewBB;
  }

  // 2. Fill the IR basic block with IR instructions.
  LLVM_DEBUG(dbgs() << "LV: vectorizing VPBB:" << getName()
                    << " in BB:" << NewBB->getName() << '\n');

  State->CFG.VPBB2IRBB[this] = NewBB;
  State->CFG.PrevVPBB = this;

  for (VPInstruction &Inst : Instructions)
    Inst.execute(*State);

  // ILV's MaskValue is set when we find a Pred Instruction in
  // VPBasicBlock's list of instructions. After generating code for all the
  // VPBasicBlock's instructions, we have to reset MaskValue in order not to
  // propagate its value to the next VPBasicBlock.
  State->ILV->setMaskValue(nullptr);

  if (auto *CBV = getCondBit()) {
    // Condition bit value in a VPBasicBlock is used as the branch selector. All
    // branches that remain are uniform - we generate a branch instruction using
    // the condition value from vector lane 0 and dummy successors. The
    // successors are fixed later when the successor blocks are visited.
    Value *NewCond = State->ILV->getScalarValue(CBV, 0);

    // Replace the temporary unreachable terminator with the new conditional
    // branch.
    auto *CurrentTerminator = NewBB->getTerminator();
    assert(isa<UnreachableInst>(CurrentTerminator) &&
           "Expected to replace unreachable terminator with conditional "
           "branch.");
    auto *CondBr = BranchInst::Create(NewBB, nullptr, NewCond);
    CondBr->setSuccessor(0, nullptr);
    ReplaceInstWithInst(CurrentTerminator, CondBr);
  }

  LLVM_DEBUG(dbgs() << "LV: filled BB:" << *NewBB);
}

/// Return true if \p Inst is a VPInstruction in a single-use chain of
/// instructions ending in block-predicate instruction that is the last
/// instruction in the block (and thus not really masking anything).
static bool isDeadPredicateInst(VPInstruction &Inst) {
  unsigned Opcode = Inst.getOpcode();
  if (Opcode == VPInstruction::Pred) {
    auto *BB = Inst.getParent();
    return ++(Inst.getIterator()) == BB->end();
  }

  if (Opcode != VPInstruction::Not && Opcode != Instruction::And)
    return false;

  if (Inst.getNumUsers() != 1)
    return false;

  return isDeadPredicateInst(*cast<VPInstruction>(*Inst.user_begin()));
}

void VPBasicBlock::executeHIR(VPOCodeGenHIR *CG) {
  CG->setCurMaskValue(nullptr);

  // Emit block start label. This is done only if we see uniform control flow
  // and only for blocks inside the loop being vectorized.
  CG->emitBlockLabel(this);

  for (VPInstruction &Inst : Instructions) {
    if (isDeadPredicateInst(Inst))
      // This is not just emitted code clean-up, but something required to
      // support our hacky search loop CG that crashes trying to emit code for
      // "and/not" instructions that use "icmp" decomposed from the HLLoop.
      continue;
    Inst.executeHIR(CG);
  }

  // Emit block terminator. This can be either an if/else or a simple goto
  // depending on the block's successors. This is done only if we see uniform
  // control flow and only for non-latch blocks.
  CG->emitBlockTerminator(this);
}

iterator_range<VPBasicBlock::const_iterator>
VPBasicBlock::getNonPredicateInstructions() const {
  // New predicator uses VPInstructions to generate the block predicate.
  // Skip instructions until block-predicate instruction is seen if the block
  // has a predicate.

  // No predicate instruction, return immediately.
  if (getPredicate() == nullptr)
    return make_range(begin(), end());

  auto It = begin();
  auto ItEnd = end();

  assert(It != ItEnd &&
         "VPBasicBlock without VPInstructions can't have a predicate!");

  // Skip until predicate.
  while (It->getOpcode() != VPInstruction::Pred) {
    assert(It != ItEnd && "Predicate VPInstruction not found!");
    ++It;
  }

  // Skip the predicate itself.
  ++It;

  return make_range(It, ItEnd);
}

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
void VPBasicBlock::print(raw_ostream &OS, unsigned Indent,
                         const VPlanDivergenceAnalysis *DA,
                         const Twine &NamePrefix) const {
  std::string StrIndent = std::string(2 * Indent, ' ');
  OS << StrIndent << NamePrefix << getName() << ":\n";

  // Print block body
  if (empty()) {
    OS << StrIndent << " <Empty Block>\n";
  } else {
    for (const VPInstruction &Inst : *this) {
      OS << StrIndent << " ";
      Inst.dump(OS, DA);
    }
  }
  const VPValue *CB = getCondBit();
  if (CB) {
    const VPInstruction *CBI = dyn_cast<VPInstruction>(CB);
    if (CBI && CBI->getNumOperands()) {
      if (CBI->getParent() != this) {
        OS << StrIndent << " Condition(";
        if (CBI->getParent()) {
          OS << CBI->getParent()->getName();
        }
        OS << "): ";
        CBI->dump(OS, DA);
      }
    } else {
      // We fall here if VPInstruction has no operands or Value is
      // constant - both match external defenition.
      OS << StrIndent << " Condition(external): ";
      CB->printAsOperand(OS);
      OS << "\n";
    }
  }
  auto &Successors = getSuccessors();
  if (Successors.empty())
    OS << StrIndent << "no SUCCESSORS";
  else {
    OS << StrIndent << "SUCCESSORS(" << Successors.size() << "):";
    if (Successors.size() == 1) {
      OS << Successors.front()->getName();
    } else if (Successors.size() == 2) {
      if (CB) {
        OS << Successors.front()->getName() << "(";
        CB->printAsOperand(OS);
        OS << "), " << Successors.back()->getName() << "(!";
        CB->printAsOperand(OS);
        OS << ")";
      } else {
        OS << Successors.front()->getName() << "(<undef>), "
           << Successors.back()->getName() << "(!<undef>)";
      }
    } else {
      assert("More than 2 successors in basic block are not supported!");
    }
  }
  OS << "\n";
  auto &Predecessors = getPredecessors();
  if (Predecessors.empty()) {
    OS << StrIndent << "no PREDECESSORS";
  } else {
    OS << StrIndent << "PREDECESSORS(" << Predecessors.size() << "):";
    for (auto BB : Predecessors)
      OS << " " << BB->getName();
  }
  OS << "\n\n";
}
#endif // !NDEBUG || LLVM_ENABLE_DUMP

BasicBlock *
#if INTEL_CUSTOMIZATION
VPBasicBlock::createEmptyBasicBlock(VPTransformState *State)
#else
VPBasicBlock::createEmptyBasicBlock(VPTransformState::CFGState &CFG)
#endif
{
#if INTEL_CUSTOMIZATION
  VPTransformState::CFGState &CFG = State->CFG;
#endif
  // BB stands for IR BasicBlocks. VPBB stands for VPlan VPBasicBlocks.
  // Pred stands for Predecessor. Prev stands for Previous, last
  // visited/created.
  BasicBlock *PrevBB = CFG.PrevBB;
  BasicBlock *NewBB = BasicBlock::Create(PrevBB->getContext(), "VPlannedBB",
                                         PrevBB->getParent(), CFG.LastBB);
  LLVM_DEBUG(dbgs() << "LV: created " << NewBB->getName() << '\n');

#if INTEL_CUSTOMIZATION
  // Hook up the new basic block to its predecessors. New predecessors that
  // result from creating new BranchInsts are prepended instead of appended to
  // the predecessor list. In order to preserve original CFG and original
  // predecessors order, we have to process them in reverse order.
  for (VPBasicBlock *PredVPBB : reverse(getPredecessors())) {
    auto &PredVPSuccessors = PredVPBB->getSuccessors();

    // In order to keep the hookup code simple, we delay fixing up blocks
    // with two successors to the end of code generation when all blocks
    // have been visited.
    if (PredVPSuccessors.size() == 2) {
      CFG.VPBBsToFix.push_back(PredVPBB);
      continue;
    }

    BasicBlock *PredBB = CFG.VPBB2IRBB[PredVPBB];
    auto *PredBBTerminator = PredBB->getTerminator();
    LLVM_DEBUG(dbgs() << "LV: draw edge from" << PredBB->getName() << '\n');
    if (isa<UnreachableInst>(PredBB->getTerminator())) {
      PredBBTerminator->eraseFromParent();
      BranchInst::Create(NewBB, PredBB);
    } else {
      llvm_unreachable("Predecessor with two successors unexpected here");
    }
  }
#else
  // Hook up the new basic block to its predecessors.
  for (VPBasicBlock *PredVPBB : getPredecessors()) {
    auto &PredVPSuccessors = PredVPBB->getSuccessors();
    BasicBlock *PredBB = CFG.VPBB2IRBB[PredVPBB];
    assert(PredBB && "Predecessor basic-block not found building successor.");
    auto *PredBBTerminator = PredBB->getTerminator();
    LLVM_DEBUG(dbgs() << "LV: draw edge from" << PredBB->getName() << '\n');
    if (isa<UnreachableInst>(PredBBTerminator)) {
      assert(PredVPSuccessors.size() == 1 &&
             "Predecessor ending w/o branch must have single successor.");
      PredBBTerminator->eraseFromParent();
      BranchInst::Create(NewBB, PredBB);
    } else {
      assert(PredVPSuccessors.size() == 2 &&
             "Predecessor ending with branch must have two successors.");
      unsigned idx = PredVPSuccessors.front() == this ? 0 : 1;
      assert(!PredBBTerminator->getSuccessor(idx) &&
             "Trying to reset an existing successor block.");
      PredBBTerminator->setSuccessor(idx, NewBB);
    }
  }
#endif
  return NewBB;
}

VPBasicBlock *VPBasicBlock::splitBlock(iterator I, const Twine &NewBBName) {
  std::string NewName = NewBBName.str();
  if (NewName.empty())
    NewName = VPlanUtils::createUniqueName("BB");

  VPBasicBlock *NewBB = new VPBasicBlock(NewName);
  moveConditionalEOBTo(NewBB);
  NewBB->moveTripCountInfoFrom(this);
  VPBlockUtils::insertBlockAfter(NewBB, this);

  auto End = end();

  while (I != End && isa<VPPHINode>(*I))
    ++I;

  NewBB->Instructions.splice(NewBB->end(), Instructions, I, end());
  // TODO: Make it automatical via splice above.
  for (VPInstruction &VPInst : *NewBB)
    VPInst.setParent(NewBB);

  // Once instructions have been moved, determine which block has a
  // block-predicate instruction now.
  VPValue *OrigPredicate = getPredicate();

  auto IsBlockPredicateForBlock = [](const VPUser *U,
                                     const VPBasicBlock *BB) -> bool {
    auto *Inst = dyn_cast<VPInstruction>(U);
    if (!Inst)
      return false;
    return Inst->getOpcode() == VPInstruction::Pred && Inst->getParent() == BB;
  };
  auto HasBlockPredicate = [=](const VPBasicBlock *BB) {
    return OrigPredicate != nullptr &&
           llvm::any_of(OrigPredicate->users(), [=](const VPUser *U) {
             return IsBlockPredicateForBlock(U, BB);
           });
  };

  bool NewBlockHasPredicate = HasBlockPredicate(NewBB);
  if (NewBlockHasPredicate) {
    setPredicate(nullptr);
    NewBB->setPredicate(OrigPredicate);
  }
  assert(
      (!OrigPredicate || HasBlockPredicate(this) || HasBlockPredicate(NewBB)) &&
      "Block predicate was lost!");

  // Update incoming block of VPPHINodes in successors, if any
  for (auto &Successor : NewBB->getSuccessors()) {
    // Iterate over all VPPHINodes in Successor. Successor can be a region so
    // we should take its entry.
    for (VPPHINode &VPN : Successor->getVPPhis()) {
      if (VPN.getBlend() && !NewBlockHasPredicate)
        // Don't update incoming blocks for a blend. They are needed for their
        // predicates only, and the predicate wasn't copied into the new
        // block.
        continue;

      // Transform the VPBBUsers vector of the PHI node by replacing any
      // occurrence of BB with NewBB
      llvm::transform(VPN.blocks(), VPN.block_begin(),
                      [this, NewBB](VPBasicBlock *A) -> VPBasicBlock * {
                        if (A == this)
                          return NewBB;
                        return A;
                      });
    }
  }

  return NewBB;
}
