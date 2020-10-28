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

// When a VPinstruction is added in a basic block list, the following method
// updates the parent.
void ilist_traits<VPInstruction>::addNodeToList(VPInstruction *VPInst) {
  assert(!VPInst->getParent() && "VPInstruction already in a basic block");
  VPInst->setParent(getListOwner<VPBasicBlock, VPInstruction>(this));
}

// When we remove a VPInstruction from a basic block list, we update its parent
// pointer.
void ilist_traits<VPInstruction>::removeNodeFromList(VPInstruction *VPInst) {
  assert(VPInst->getParent() && "VPInstruction not in a basic block");
  VPInst->setParent(nullptr);
}

// When moving a range of VPInstructions from one basic blocks list to another,
// we need to update the parent.
void ilist_traits<VPInstruction>::transferNodesFromList(ilist_traits &FromList,
                                                        instr_iterator First,
                                                        instr_iterator Last) {
  // If it's within the same BB, there's nothing to do.
  if (this == &FromList)
    return;

  VPBasicBlock *CurBB = getListOwner<VPBasicBlock, VPInstruction>(this);
  VPBasicBlock *FromBB = getListOwner<VPBasicBlock, VPInstruction>(&FromList);
  assert(CurBB != FromBB && "Two lists have the same parent?");
  (void)CurBB;
  (void)FromBB;

  // If splicing between two blocks,then update the parent pointers.
  for (; First != Last; ++First)
    First->setParent(getListOwner<VPBasicBlock, VPInstruction>(this));
}

void ilist_traits<VPInstruction>::deleteNode(VPInstruction *VPInst) {
  assert(!VPInst->getParent() && "VPInstruction is still in a block!");
  delete VPInst;
}

void VPBlockUtils::insertBlockBefore(VPBasicBlock *NewBB,
                                     VPBasicBlock *BlockPtr) {
  for (auto Pred : BlockPtr->getPredecessors())
    Pred->replaceSuccessor(BlockPtr, NewBB);

  NewBB->insertBefore(BlockPtr);
  NewBB->setTerminator(BlockPtr);
}

void VPBlockUtils::insertBlockAfter(VPBasicBlock *NewBB,
                                    VPBasicBlock *BlockPtr) {
  NewBB->insertAfter(BlockPtr);

  if (BlockPtr->getNumSuccessors() == 1)
    NewBB->setTerminator(BlockPtr->getSuccessor(0));
  else if (BlockPtr->getNumSuccessors() == 2)
    NewBB->setTerminator(BlockPtr->getSuccessor(0), BlockPtr->getSuccessor(1),
                         BlockPtr->getCondBit());
  else
    NewBB->setTerminator();

  BlockPtr->setTerminator(NewBB);
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
      VPBasicBlock *Succ1 = NewBB->getSuccessor(0);
      VPBasicBlock *Succ2 = NewBB->getSuccessor(1);

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

VPBasicBlock *VPBlockUtils::splitBlockHead(VPBasicBlock *BB,
                                           VPBasicBlock::iterator BeforeIt,
                                           VPLoopInfo *VPLInfo,
                                           const Twine &Name,
                                           VPDominatorTree *DomTree,
                                           VPPostDominatorTree *PostDomTree) {
  VPBasicBlock *NewBB = new VPBasicBlock(Name, BB->getParent());
  NewBB->insertBefore(BB);

  SmallVector<VPBasicBlock *, 4> Preds(BB->getPredecessors());
  for (auto *Pred : Preds)
    Pred->replaceSuccessor(BB, NewBB);

  NewBB->setTerminator(BB);

  // Add NewBB to VPLoopInfo
  if (VPLoop *Loop = VPLInfo->getLoopFor(BB)) {
    Loop->addBasicBlockToLoop(NewBB, *VPLInfo);
  }

  auto End = BB->end();

  VPBasicBlock::iterator I = BeforeIt;
  while (I != End && (isa<VPPHINode>(*I) || isa<VPBlendInst>(*I)))
    ++I;

  NewBB->Instructions.splice(NewBB->terminator(), BB->Instructions, BB->begin(),
                             I);

  // TODO: Make updates incremental.
  if (DomTree)
    DomTree->recalculate(*BB->getParent());
  if (PostDomTree)
    PostDomTree->recalculate(*BB->getParent());

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
  return splitBlock(BB, BB->terminator(), VPLInfo, DomTree, PostDomTree);
}

VPBasicBlock *VPBlockUtils::splitBlockAtPredicate(
    VPBasicBlock *BB,
    VPLoopInfo *VPLInfo,
    VPDominatorTree *DomTree,
    VPPostDominatorTree *PostDomTree) {

  VPBasicBlock::iterator SplitPt = BB->end();
  for (auto &VPInst : *BB) {
    if (VPInst.getOpcode() == VPInstruction::Pred) {
      SplitPt = VPInst.getIterator();
      break;
    }
  }
  assert(SplitPt != BB->end() && "Block predicate not found.");

  return splitBlock(BB, SplitPt, VPLInfo, DomTree, PostDomTree);
}

VPBasicBlock::VPBasicBlock(const Twine &Name, VPlan *Plan)
    : VPValue(VPBasicBlockSC, Type::getLabelTy(*Plan->getLLVMContext())),
      // We don't insert the VPBB into the Plan's VPBasicBlockList here,
      // so do NOT set parent.
      Parent(nullptr) {
  setName(Name);
}

VPBasicBlock::~VPBasicBlock() { dropAllReferences(); }

void VPBasicBlock::dropTerminatorIfExists() {
  if (!empty() && isa<VPBranchInst>(std::prev(end())))
    eraseInstruction(getTerminator());
}

void VPBasicBlock::setTerminator() {
  dropTerminatorIfExists();
  VPBranchInst *Instr = new VPBranchInst(Type::getVoidTy(getType()->getContext()));
  insert(Instr, end());
  if (Parent)
    if (VPlanDivergenceAnalysis *DA = Parent->getVPlanDA())
      DA->updateDivergence(*Instr);
}

void VPBasicBlock::setTerminator(VPBasicBlock *Succ) {
  dropTerminatorIfExists();
  VPBranchInst *Instr = new VPBranchInst(Succ);
  insert(Instr, end());
  if (Parent)
    if (VPlanDivergenceAnalysis *DA = Parent->getVPlanDA())
      DA->updateDivergence(*Instr);
}

void VPBasicBlock::setTerminator(VPBasicBlock *IfTrue, VPBasicBlock *IfFalse,
                                 VPValue *Cond) {
  dropTerminatorIfExists();
  VPBranchInst *Instr = new VPBranchInst(IfTrue, IfFalse, Cond);
  insert(Instr, end());
  if (Parent)
    if (VPlanDivergenceAnalysis *DA = Parent->getVPlanDA())
      DA->updateDivergence(*Instr);
}

void VPBasicBlock::replaceSuccessor(VPBasicBlock *OldSuccessor,
                                    VPBasicBlock *NewSuccessor) {
  getTerminator()->replaceUsesOfWith(OldSuccessor, NewSuccessor);
}

VPBasicBlock * VPBasicBlock::getSuccessor(unsigned idx) const {
  return getTerminator()->getSuccessor(idx);
}

void VPBasicBlock::setSuccessor(unsigned idx, VPBasicBlock *NewSucc) {
  getTerminator()->setSuccessor(idx, NewSucc);
}

VPBasicBlock *VPBasicBlock::getSinglePredecessor() const {
  return getNumPredecessors() == 1 ? *getPredecessors().begin() : nullptr;
}

const VPBasicBlock *VPBasicBlock::getUniquePredecessor() const {
  auto Predecessors = getPredecessors();
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

VPUser::const_operand_range VPBasicBlock::successors() const {
  return getTerminator()->successors();
}

size_t VPBasicBlock::getNumSuccessors() const {
  return getTerminator()->getNumSuccessors();
}

VPBasicBlock *VPBasicBlock::getSingleSuccessor() const {
  return getNumSuccessors() == 1 ? *getSuccessors().begin() : nullptr;
}

VPValue *VPBasicBlock::getCondBit() {
  const VPBranchInst *T = getTerminator();
  return T->isConditional() ? T->getCondition() : nullptr;
}

const VPValue *VPBasicBlock::getCondBit() const {
  return const_cast<VPBasicBlock *>(this)->getCondBit();
}

void VPBasicBlock::setCondBit(VPValue *CB) {
  getTerminator()->setCondition(CB);
}

void VPBasicBlock::insertBefore(VPBasicBlock *InsertPos) {
  VPlan *CurPlan = InsertPos->getParent();
  CurPlan->insertBefore(this, InsertPos);
}

void VPBasicBlock::insertAfter(VPBasicBlock *InsertPos) {
  VPlan *CurPlan = InsertPos->getParent();
  CurPlan->insertAfter(this, InsertPos);
}

VPValue *VPBasicBlock::getPredicate() {
  if (!BlockPredicate)
    return nullptr;
  assert(BlockPredicate->getParent() == this &&
         "Block predicate doesn't belong to this VPBasicBlock!");
  return BlockPredicate->getOperand(0);
}
const VPValue *VPBasicBlock::getPredicate() const {
  if (!BlockPredicate)
    return nullptr;
  assert(BlockPredicate->getParent() == this &&
         "Block predicate doesn't belong to this VPBasicBlock!");
  return BlockPredicate->getOperand(0);
}
void VPBasicBlock::setBlockPredicate(VPInstruction *BlockPredicate) {
  assert(
      (!BlockPredicate || BlockPredicate->getOpcode() == VPInstruction::Pred) &&
      "VPBlockPredicate is expected!");
  this->BlockPredicate = BlockPredicate;
}

void VPBasicBlock::insert(VPInstruction *Instruction, iterator InsertPt) {
  assert(Instruction && "No instruction to append.");
  assert(!Instruction->Parent && "Instruction already in VPlan");
  Instructions.insert(InsertPt, Instruction);
}

void VPBasicBlock::appendInstruction(VPInstruction *Instruction) {
  insert(Instruction, terminator());
}

void VPBasicBlock::addInstructionAfter(VPInstruction *Instruction,
                                       VPInstruction *After) {
  if (!After) {
    Instructions.insert(Instructions.begin(), Instruction);
  } else {
    Instructions.insertAfter(After->getIterator(), Instruction);
  }
}

void VPBasicBlock::addInstruction(VPInstruction *Instruction,
                                  VPInstruction *Before) {
  if (!Before) {
    Instructions.insert(terminator(), Instruction);
  } else {
    assert(Before->Parent == this &&
           "Insertion before point not in this basic block.");
    Instructions.insert(Before->getIterator(), Instruction);
  }
}

// TODO: Please, remove this interface once C/T/F blocks have been removed.
void VPBasicBlock::moveConditionalEOBTo(VPBasicBlock *ToBB) {
  if (getNumSuccessors() > 1) {
    ToBB->setCBlock(CBlock);
    ToBB->setTBlock(TBlock);
    ToBB->setFBlock(FBlock);
    CBlock = TBlock = FBlock = nullptr;
  }
}

// Unlinks the instruction from VPBasicBlock's instructions and adds in
// UnlinkedVPInsns vector. The VPInstructions in UnlinkedVPInsns are deleted
// when VPlan's destructor is called.
void VPBasicBlock::eraseInstruction(VPInstruction *Instruction) {
  // We need to remove all instruction operands before erasing the
  // VPInstruction. Else this breaks the use-def chains.
  Instruction->dropAllReferences();

  removeInstruction(Instruction);

  if (VPlan *CurVPlan = getParent())
    CurVPlan->addUnlinkedVPInst(Instruction);
  else
    delete Instruction;
}

void VPBasicBlock::dropAllReferences() {
  for (auto &Inst : *this)
    Inst.dropAllReferences();
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

  State->CFG.VPBB2IREndBB[this] = State->CFG.PrevBB;
  if (auto *CBV = getCondBit()) {
    // Condition bit value in a VPBasicBlock is used as the branch selector. All
    // branches that remain are uniform - we generate a branch instruction using
    // the condition value from vector lane 0 and dummy successors. The
    // successors are fixed later when the successor blocks are visited.
    Value *NewCond = State->ILV->getScalarValue(CBV, 0);
    BasicBlock *EndBB = State->CFG.VPBB2IREndBB[this];

    // Replace the temporary unreachable terminator with the new conditional
    // branch.
    auto *CurrentTerminator = EndBB->getTerminator();
    assert(isa<UnreachableInst>(CurrentTerminator) &&
           "Expected to replace unreachable terminator with conditional "
           "branch.");
    auto *CondBr = BranchInst::Create(EndBB, nullptr, NewCond);
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
    auto NextIt = ++Inst.getIterator();
    return NextIt == BB->terminator() &&
           !cast<VPBranchInst>(NextIt)->getHLGoto();
  }

  if (Opcode != VPInstruction::Not && Opcode != Instruction::And)
    return false;

  if (Inst.getNumUsers() != 1)
    return false;

  auto *InstUser = *Inst.user_begin();

  if (!isa<VPInstruction>(InstUser))
    // Instruction has a single non-instruction user.
    return false;

  return isDeadPredicateInst(*cast<VPInstruction>(InstUser));
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
                         const Twine &NamePrefix) const {
  std::string StrIndent = std::string(2 * Indent, ' ');
  OS << StrIndent << NamePrefix << getName() << ": # preds: ";
  auto Predecessors = getPredecessors();
  for (auto It = Predecessors.begin(); It != Predecessors.end(); It++) {
    if (It != Predecessors.begin())
      OS << ", ";
    OS << (*It)->getName();
  }
  OS << "\n";

  // Print block body
  if (empty())
    OS << StrIndent << " <Empty Block>\n";
  else {
    for (const VPInstruction &Inst : Instructions) {
      OS << StrIndent << " ";
      Inst.print(OS);
      OS << '\n';
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
        CBI->print(OS);
        OS << '\n';
      }
    } else {
      // We fall here if VPInstruction has no operands or Value is
      // constant - both match external defenition.
      OS << StrIndent << " Condition(external): ";
      CB->printAsOperand(OS);
      OS << "\n";
    }
  }
  OS << "\n";
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
                                         PrevBB->getParent(), CFG.InsertBefore);
  LLVM_DEBUG(dbgs() << "LV: created " << NewBB->getName() << '\n');

#if INTEL_CUSTOMIZATION
  // Hook up the new basic block to its predecessors.
  for (VPBasicBlock *PredVPBB : getPredecessors()) {
    // In order to keep the hookup code simple, we delay fixing up blocks
    // with two successors to the end of code generation when all blocks
    // have been visited.
    if (PredVPBB->getNumSuccessors() == 2) {
      CFG.VPBBsToFix.push_back(PredVPBB);
      continue;
    }

    BasicBlock *PredBB = CFG.VPBB2IREndBB[PredVPBB];
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
    auto PredVPSuccessors = PredVPBB->getSuccessors();
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

  VPBasicBlock *NewBB = new VPBasicBlock(NewName, Parent);
  NewBB->setTerminator();
  moveConditionalEOBTo(NewBB);
  NewBB->moveTripCountInfoFrom(this);

  auto End = end();

  while (I != End && (isa<VPPHINode>(*I) || isa<VPBlendInst>(*I)))
    ++I;

  NewBB->Instructions.splice(NewBB->terminator(), Instructions, I,
                             terminator());

  VPBlockUtils::insertBlockAfter(NewBB, this);

  // Once instructions have been moved, determine which block has a
  // block-predicate instruction now.
  VPInstruction *BlockPredicate = getBlockPredicate();
  setBlockPredicate(nullptr);
  if (BlockPredicate)
    BlockPredicate->getParent()->setBlockPredicate(BlockPredicate);

  // Update incoming block of VPPHINodes in successors, if any
  for (auto Successor : NewBB->getSuccessors()) {
    // Iterate over all VPPHINodes in Successor. Successor can be a region so
    // we should take its entry.
    for (VPPHINode &VPN : Successor->getVPPhis()) {
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

VPBasicBlock *VPBlockUtils::splitEdge(VPBasicBlock *From, VPBasicBlock *To,
                                      const Twine &Name, VPLoopInfo *VPLInfo,
                                      VPDominatorTree *DomTree,
                                      VPPostDominatorTree *PostDomTree) {
  assert(is_contained(From->getSuccessors(), To) &&
         "From and To do not form an edge!");
  auto *NewBB = new VPBasicBlock(Name, From->getParent());
  NewBB->setTerminator(To);
  NewBB->insertAfter(From);
  From->replaceSuccessor(To, NewBB);

  for (VPPHINode &VPN : To->getVPPhis()) {
    // Transform the VPBBUsers vector of the PHI node by replacing any
    // occurrence of BB with NewBB
    llvm::transform(VPN.blocks(), VPN.block_begin(),
                    [From, NewBB](VPBasicBlock *A) -> VPBasicBlock * {
                      if (A == From)
                        return NewBB;
                      return A;
                    });
  }

  // FIXME: VPLInfo update.
  if (VPLInfo) {
    auto *FromLoop = VPLInfo->getLoopFor(From);
    auto *ToLoop = VPLInfo->getLoopFor(To);
    if (FromLoop) {
      if (FromLoop == ToLoop) {
        FromLoop->addBasicBlockToLoop(NewBB, *VPLInfo);
      } else {
        assert(false && "Not implemented!");
      }
    }
  }
  if (DomTree)
    DomTree->recalculate(*From->getParent());
  if (PostDomTree)
    PostDomTree->recalculate(*From->getParent());
  return NewBB;
}

VPBasicBlock::iterator VPBasicBlock::terminator() {
  assert(!empty() && "At least one VPBranchInst block is expected.");
  iterator It = std::prev(end());
  assert(isa<VPBranchInst>(It) &&
         "The last VPInstruction is expected to be VPBranchInst instruction.");
  return It;
}

VPBasicBlock::const_iterator VPBasicBlock::terminator() const {
  return const_cast<VPBasicBlock *>(this)->terminator();
}

VPBranchInst *VPBasicBlock::getTerminator() {
  auto It = terminator();
  return cast<VPBranchInst>(It);
}

const VPBranchInst *VPBasicBlock::getTerminator() const {
  return const_cast<VPBasicBlock *>(this)->getTerminator();
}

VPBasicBlock *VPBasicBlock::getVPUserParent(VPUser *User) {
  return cast<VPBranchInst>(User)->getParent();
}
