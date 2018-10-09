//===- IntelVPlan.cpp - Vectorizer Plan -----------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This is the LLVM vectorization plan. It represents a candidate for
// vectorization, allowing to plan and optimize how to vectorize a given loop
// before generating LLVM-IR.
// The vectorizer uses vectorization plans to estimate the costs of potential
// candidates and if profitable to execute the desired plan, generating vector
// LLVM-IR code.
//
//===----------------------------------------------------------------------===//

#if INTEL_CUSTOMIZATION
#include "IntelVPlan.h"
#include "IntelLoopVectorizationCodeGen.h"
#include "IntelVPlanDominatorTree.h"
#include "IntelVPlanVLSAnalysis.h"
#include "VPlanHIR/IntelVPOCodeGenHIR.h"
#else
#include "VPlan.h"
#endif //INTEL_CUSTOMIZATION

#include "llvm/ADT/PostOrderIterator.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Dominators.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/GraphWriter.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"

#if INTEL_CUSTOMIZATION
#define DEBUG_TYPE "intel-vplan"
#else
#define DEBUG_TYPE "vplan"
#endif

using namespace llvm;
using namespace llvm::vpo;

std::atomic<unsigned> VPlanUtils::NextOrdinal{1};
#if INTEL_CUSTOMIZATION
// Replace dot print output with plain print.
static cl::opt<bool>
    DumpPlainVPlanIR("vplan-plain-dump", cl::init(false), cl::Hidden,
                       cl::desc("Print plain VPlan IR"));

raw_ostream &operator<<(raw_ostream &OS, const VPValue &V) {
  if (const VPInstruction *I = dyn_cast<VPInstruction>(&V))
    I->dump(OS);
  else
    V.dump(OS);
  return OS;
}
#endif

/// \return the VPBasicBlock that is the entry of Block, possibly indirectly.
const VPBasicBlock *VPBlockBase::getEntryBasicBlock() const {
  const VPBlockBase *Block = this;
  while (const VPRegionBlock *Region = dyn_cast<VPRegionBlock>(Block))
    Block = Region->getEntry();
  return cast<VPBasicBlock>(Block);
}

/// \return the VPBasicBlock that is the exit of Block, possibly indirectly.
const VPBasicBlock *VPBlockBase::getExitBasicBlock() const {
  const VPBlockBase *Block = this;
  while (const VPRegionBlock *Region = dyn_cast<VPRegionBlock>(Block))
    Block = Region->getExit();
  return cast<VPBasicBlock>(Block);
}

VPBasicBlock *VPBlockBase::getExitBasicBlock() {
  VPBlockBase *Block = this;
  while (VPRegionBlock *Region = dyn_cast<VPRegionBlock>(Block))
    Block = Region->getExit();
  return cast<VPBasicBlock>(Block);
}

#if INTEL_CUSTOMIZATION
// It turns A->B into A->NewSucc->B and updates VPLoopInfo, DomTree and
// PostDomTree accordingly.
VPBasicBlock *VPBlockUtils::splitBlock(VPBlockBase *Block,
                                       VPLoopInfo *VPLInfo,
                                       VPDominatorTree &DomTree,
                                       VPPostDominatorTree &PostDomTree,
                                       VPlan *Plan) {
  VPBasicBlock *NewBlock = new VPBasicBlock(VPlanUtils::createUniqueName("BB"));
  insertBlockAfter(NewBlock, Block, Plan);

  // Add NewBlock to VPLoopInfo
  if (VPLoop *Loop = VPLInfo->getLoopFor(Block)) {
    Loop->addBasicBlockToLoop(NewBlock, *VPLInfo);
  }

  // Update dom information

  VPDomTreeNode *BlockDT = DomTree.getNode(Block);
  assert(BlockDT && "Expected node in dom tree!");
  SmallVector<VPDomTreeNode *, 2> BlockDTChildren(BlockDT->begin(),
                                                  BlockDT->end());
  // Block is NewBlock's idom.
  VPDomTreeNode *NewBlockDT = DomTree.addNewBlock(NewBlock, Block /*IDom*/);

  // NewBlock dominates all other nodes dominated by Block.
  for (VPDomTreeNode *Child : BlockDTChildren)
    DomTree.changeImmediateDominator(Child, NewBlockDT);

  // Update postdom information

  VPDomTreeNode *NewBlockPDT;
  if (VPBlockBase *NewBlockSucc = NewBlock->getSingleSuccessor()) {
    // NewBlock has a single successor. That successor is NewBlock's ipostdom.
    NewBlockPDT = PostDomTree.addNewBlock(NewBlock, NewBlockSucc /*IDom*/);
  } else {
    // NewBlock has multiple successors. NewBlock's ipostdom is the nearest
    // common post-dominator of both successors.

    // TODO: getSuccessor(0)
    auto &Successors = NewBlock->getSuccessors();
    VPBlockBase *Succ1 = *Successors.begin();
    VPBlockBase *Succ2 = *std::next(Successors.begin());

    NewBlockPDT = PostDomTree.addNewBlock(
        NewBlock, PostDomTree.findNearestCommonDominator(Succ1, Succ2));
  }

  VPDomTreeNode *BlockPDT = PostDomTree.getNode(Block);
  assert(BlockPDT && "Expected node in post-dom tree!");

  // TODO: remove getBlock?
  if (BlockPDT->getIDom()->getBlock() == NewBlockPDT->getIDom()->getBlock()) {
    // Block's old ipostdom is the same as NewBlock's ipostdom. Block's new
    // ipostdom is NewBlock
    PostDomTree.changeImmediateDominator(BlockPDT, NewBlockPDT);

  } else {
    // Otherwise, Block's new ipostdom is the nearest common post-dominator of
    // NewBlock and Block's old ipostdom
    PostDomTree.changeImmediateDominator(
        BlockPDT, PostDomTree.getNode(PostDomTree.findNearestCommonDominator(
                      NewBlock, BlockPDT->getIDom()->getBlock())));
  }

  return NewBlock;
}
#endif

/// Returns the closest ancestor, starting from "this", which has successors.
/// Returns the root ancestor if all ancestors have no successors.
VPBlockBase *VPBlockBase::getAncestorWithSuccessors() {
  if (!Successors.empty() || !Parent)
    return this;
  assert(Parent->getExit() == this &&
         "Block w/o successors not the exit of its parent.");
  return Parent->getAncestorWithSuccessors();
}

/// Returns the closest ancestor, starting from "this", which has
/// predecessors.
/// Returns the root ancestor if all ancestors have no predecessors.
VPBlockBase *VPBlockBase::getAncestorWithPredecessors() {
  if (!Predecessors.empty() || !Parent)
    return this;
  assert(Parent->getEntry() == this &&
         "Block w/o predecessors not the entry of its parent.");
  return Parent->getAncestorWithPredecessors();
}

void VPBlockBase::deleteCFG(VPBlockBase *Entry) {
  SmallVector<VPBlockBase *, 8> Blocks;
  for (VPBlockBase *Block : depth_first(Entry))
    Blocks.push_back(Block);

  for (VPBlockBase *Block : Blocks)
    delete Block;
}

#if INTEL_CUSTOMIZATION
void VPBlockBase::setCondBit(VPValue *CB, VPlan *Plan) {
  CondBit = CB;
  if (CB)
    Plan->setCondBitUser(CB, this);
}
#endif

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
  const SmallVectorImpl<VPBlockBase *> &Preds = getHierarchicalPredecessors();
  for (VPBlockBase *PredVPBlock : make_range(Preds.rbegin(), Preds.rend())) {

    VPBasicBlock *PredVPBB = PredVPBlock->getExitBasicBlock();
    if (!CFG.VPBB2IRBB.count(PredVPBB)) {
      // Back edge from inner loop
      CFG.EdgesToFix[PredVPBB] = NewBB;
    } else {
      BasicBlock *PredBB = CFG.VPBB2IRBB[PredVPBB];
      LLVM_DEBUG(dbgs() << "LV: draw edge from" << PredBB->getName() << '\n');
      if (isa<UnreachableInst>(PredBB->getTerminator())) {
        PredBB->getTerminator()->eraseFromParent();
        BranchInst::Create(NewBB, PredBB);
      } else {
        // Replace old unconditional branch with new conditional branch.
        // Note: we rely on traversing the successors in order.
        BasicBlock *FirstSuccBB;
        BasicBlock *SecondSuccBB;

        assert(PredBB->getSingleSuccessor() && "Unexpected null successor");
        if (PredVPBB->getSuccessors()[0] == this) {
          FirstSuccBB = NewBB;
          SecondSuccBB = PredBB->getSingleSuccessor();
        } else {
          FirstSuccBB = PredBB->getSingleSuccessor();
          SecondSuccBB = NewBB;
        }

        PredBB->getTerminator()->eraseFromParent();
        VPValue *CBV = PredVPBlock->getCondBit();
        assert(CBV && "Expected condition bit!");
        Value *Bit = nullptr;
        if (State->CBVToConditionBitMap.count(CBV)) {
          Bit = State->CBVToConditionBitMap[CBV];
        } else {
          Bit = CBV->getUnderlyingValue();
        }
        assert(Bit && "Cannot create conditional branch with empty bit.");
        assert(!Bit->getType()->isVectorTy() && "Should be 1-bit scalar");
        BranchInst::Create(FirstSuccBB, SecondSuccBB, Bit, PredBB);
      }
    }
  }
#else
  // Hook up the new basic block to its predecessors.
  for (VPBlockBase *PredVPBlock : getHierarchicalPredecessors()) {
    VPBasicBlock *PredVPBB = PredVPBlock->getExitBasicBlock();
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

void VPBasicBlock::execute(VPTransformState *State) {

#if INTEL_CUSTOMIZATION
  // The community version and 'vpo' version of "execute" for VPBB diverge
  // considerably. Instead of having INTEL_CUSTOMIZATIONS every few lines of
  // code and make the code unreadable, we decided to seperate both the
  // versions with a single INTEL_CUSTOMIZATION

  bool Replica = State->Instance &&
                 !(State->Instance->Part == 0 && State->Instance->Lane == 0);
  VPBasicBlock *PrevVPBB = State->CFG.PrevVPBB;
  VPBlockBase *SingleHPred = nullptr;
  BasicBlock *NewBB = State->CFG.PrevBB; // Reuse it if possible.
  VPLoopRegion *ParentLoop = nullptr;

  // 1. Create an IR basic block, or reuse one already available if possible.
  // The last IR basic block is reused in four cases:
  // A. the first VPBB reuses the pre-header BB - when PrevVPBB is null;
  // B. the second VPBB reuses the header BB;
  // C. when the current VPBB has a single (hierarchical) predecessor which
  //    is PrevVPBB and the latter has a single (hierarchical) successor; and
  // D. when the current VPBB is an entry of a region replica - where PrevVPBB
  //    is the exit of this region from a previous instance.
  // TODO: We currently cannot use VPLoopRegion class here
  if (PrevVPBB && (ParentLoop = dyn_cast<VPLoopRegion>(this->getParent())) &&
      ParentLoop->getEntry() == PrevVPBB /* B */ &&
      // TODO: We need to properly support outer-loop vectorization scenarios.
      // Latches for inner loops are not removed and we don't have VPlan,
      // VPLoopInfo, etc. accessible from State. Temporal fix: outermost loop's
      // parent is TopRegion. TopRegion's parent is null.
      !ParentLoop->getParent()->getParent()) {

    // Set NewBB to loop H basic block
    BasicBlock *LoopPH = State->CFG.PrevBB;
    NewBB = LoopPH->getSingleSuccessor();
    assert(NewBB && "Expected single successor from loop pre-header");
    State->Builder.SetInsertPoint(NewBB->getTerminator());
    State->CFG.PrevBB = NewBB;
  } else if (PrevVPBB /* A */ &&
             !((SingleHPred = getSingleHierarchicalPredecessor()) &&
               SingleHPred->getExitBasicBlock() == PrevVPBB &&
               PrevVPBB->getSingleHierarchicalSuccessor()) && /* C */
             !(Replica && getPredecessors().empty())) {       /* D */

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

  for (VPRecipeBase &Recipe : Recipes)
    Recipe.execute(*State);

  // ILV's MaskValue is set when we find a BlockPredicateRecipe in
  // VPBasicBlock's list of recipes. After generating code for all the
  // VPBasicBlock's instructions, we have to reset MaskValue in order not to
  // propagate its value to the next VPBasicBlock.
  State->ILV->setMaskValue(nullptr);

  LLVM_DEBUG(dbgs() << "LV: filled BB:" << *NewBB);

#else

  bool Replica = State->Instance &&
                 !(State->Instance->Part == 0 && State->Instance->Lane == 0);
  VPBasicBlock *PrevVPBB = State->CFG.PrevVPBB;
  VPBlockBase *SingleHPred = nullptr;
  BasicBlock *NewBB = State->CFG.PrevBB; // Reuse it if possible.

  // 1. Create an IR basic block, or reuse the last one if possible.
  // The last IR basic block is reused, as an optimization, in three cases:
  // A. the first VPBB reuses the loop header BB - when PrevVPBB is null;
  // B. when the current VPBB has a single (hierarchical) predecessor which
  //    is PrevVPBB and the latter has a single (hierarchical) successor; and
  // C. when the current VPBB is an entry of a region replica - where PrevVPBB
  //    is the exit of this region from a previous instance, or the predecessor
  //    of this region.
  if (PrevVPBB && /* A */
      !((SingleHPred = getSingleHierarchicalPredecessor()) &&
        SingleHPred->getExitBasicBlock() == PrevVPBB &&
        PrevVPBB->getSingleHierarchicalSuccessor()) && /* B */
      !(Replica && getPredecessors().empty())) {       /* C */
    NewBB = createEmptyBasicBlock(State->CFG);
    State->Builder.SetInsertPoint(NewBB);
    // Temporarily terminate with unreachable until CFG is rewired.
    UnreachableInst *Terminator = State->Builder.CreateUnreachable();
    State->Builder.SetInsertPoint(Terminator);
    // Register NewBB in its loop. In innermost loops its the same for all BB's.
    Loop *L = State->LI->getLoopFor(State->CFG.LastBB);
    L->addBasicBlockToLoop(NewBB, *State->LI);
    State->CFG.PrevBB = NewBB;
  }

  // 2. Fill the IR basic block with IR instructions.
  LLVM_DEBUG(dbgs() << "LV: vectorizing VPBB:" << getName()
                    << " in BB:" << NewBB->getName() << '\n');

  State->CFG.VPBB2IRBB[this] = NewBB;
  State->CFG.PrevVPBB = this;

  for (VPRecipeBase &Recipe : Recipes)
    Recipe.execute(*State);

  LLVM_DEBUG(dbgs() << "LV: filled BB:" << *NewBB);
#endif
}

VPRegionBlock::~VPRegionBlock() {
  if (Entry)
    deleteCFG(Entry);
#if INTEL_CUSTOMIZATION
  if (RegionDT)
    delete RegionDT;
  if (RegionPDT)
    delete RegionPDT;
#endif
}

void VPRegionBlock::execute(VPTransformState *State) {
  ReversePostOrderTraversal<VPBlockBase *> RPOT(Entry);

  if (!isReplicator()) {
    // Visit the VPBlocks connected to "this", starting from it.
    for (VPBlockBase *Block : RPOT) {
      LLVM_DEBUG(dbgs() << "LV: VPBlock in RPO " << Block->getName() << '\n');
      Block->execute(State);
    }
    return;
  }

  assert(!State->Instance && "Replicating a Region with non-null instance.");

  // Enter replicating mode.
  State->Instance = {0, 0};

  for (unsigned Part = 0, UF = State->UF; Part < UF; ++Part) {
    State->Instance->Part = Part;
    for (unsigned Lane = 0, VF = State->VF; Lane < VF; ++Lane) {
      State->Instance->Lane = Lane;
      // Visit the VPBlocks connected to \p this, starting from it.
      for (VPBlockBase *Block : RPOT) {
        LLVM_DEBUG(dbgs() << "LV: VPBlock in RPO " << Block->getName() << '\n');
        Block->execute(State);
      }
    }
  }

  // Exit replicating mode.
  State->Instance.reset();
}

#if INTEL_CUSTOMIZATION
// TODO: Please, remove this interface once C/T/F blocks have been removed.
void VPBasicBlock::moveConditionalEOBTo(VPBasicBlock *ToBB, VPlan *Plan) {
  // Set CondBit in NewBlock. Note that we are only setting the
  // successor selector pointer. The CondBit is kept in its
  // original VPBB recipe list.
  if (getNumSuccessors() > 1) {
    assert(getCondBit() && "Missing CondBit");
    ToBB->setCondBit(getCondBit(), Plan);
    ToBB->setCBlock(CBlock);
    ToBB->setTBlock(TBlock);
    ToBB->setFBlock(FBlock);
    setCondBit(nullptr, Plan);
    CBlock = TBlock = FBlock = nullptr;
  }
}

void VPRegionBlock::recomputeSize() {
  Size = std::distance(df_iterator<const VPBlockBase *>::begin(Entry),
                       df_iterator<const VPBlockBase *>::end(Exit));
}

void VPBasicBlock::dump(raw_ostream &OS, unsigned Indent) const {
  std::string StrIndent = std::string(2 * Indent, ' ');
  // Print name and predicate
  OS << StrIndent << getName() << " (BP: ";
  if (getPredicateRecipe())
    OS << *getPredicateRecipe();
  else
    OS << "NULL";
  OS << ") :\n";

  // Print block body
  if (empty()) {
    OS << StrIndent << " <Empty Block>\n";
  } else {
    for (const VPRecipeBase &Recipe : *this) {
      OS << StrIndent << " " << Recipe;
    }
  }
  const VPValue *CB = getCondBit();
  if (CB) {
    const VPInstruction *CBI = dyn_cast<VPInstruction>(CB);
    if (CBI && CBI->getNumOperands()) {
      if (CBI->getParent() != this) {
        OS << StrIndent << " Condition(" << CBI->getParent()->getName()
           << "): " << *CBI;
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
  if (Successors.empty()) {
    OS << StrIndent << "END Block - no SUCCESSORS\n";
    return;
  }
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
  OS << "\n\n";
}

void VPBasicBlock::dump() const {
  dump(errs(), 1);
}

void VPBasicBlock::executeHIR(VPOCodeGenHIR *CG) {
  CG->setCurMaskValue(nullptr);
  for (VPRecipeBase &Recipe : Recipes)
    Recipe.executeHIR(CG);
}

void VPRegionBlock::computeDT(void) {
  assert(!RegionDT && "Null expected");
  RegionDT = new VPDominatorTree();
  RegionDT->recalculate(*this);
}

void VPRegionBlock::computePDT(void) {
  assert(!RegionPDT && "Null expected");
  RegionPDT = new VPPostDominatorTree();
  RegionPDT->recalculate(*this);
}

/// Get a list of the basic blocks which make up this region.
void VPRegionBlock::getOrderedBlocks(std::vector<const VPBlockBase *> &Blocks) const {
  ReversePostOrderTraversal<VPBlockBase *> RPOT(Entry);
  for (VPBlockBase *Block : RPOT)
    Blocks.push_back(Block);
}

void VPRegionBlock::dump(raw_ostream &OS, unsigned Indent) const {
  SetVector<const VPBlockBase *> Printed;
  SetVector<const VPBlockBase *> SuccList;
  std::vector<const VPBlockBase *> Blocks;
  getOrderedBlocks(Blocks);

  std::string StrIndent = std::string(2 * Indent, ' ');
  // Print name and predicate
  OS << StrIndent << "REGION: " << getName() << " (BP: ";
  if (getPredicateRecipe())
    OS << *getPredicateRecipe();
  else
    OS << "NULL";
  OS << ")\n";

  SuccList.insert(Entry);
  // Main loop for printing VPRegion blocks.
  //                  Indent
  //        BB1         +0
  //       /   \
  //     BB2  BB6       +1
  //    /  \   |
  //  BB3  BB4 |        +2
  //    \  /   |
  //    BB5   /         +1
  //      \  /
  //       BB7          +0
  for (const VPBlockBase *BB : Blocks) {
    BB->dump(OS, Indent + SuccList.size() - 1);
    Printed.insert(BB);
    SuccList.remove(BB);
    for (auto *Succ : BB->getSuccessors())
      // Do not increase Indent for back edges
      if (!Printed.count(Succ))
        SuccList.insert(Succ);
  }
  if (const VPBlockBase *Successor = getSingleSuccessor())
    OS << StrIndent << "SUCCESSORS(1):" << Successor->getName() << "\n";
  OS << StrIndent << "END Region(" << getName() << ")\n";
  OS << "\n";
}

void VPRegionBlock::dump() const {
  dump(errs(), 1);
}

void VPRegionBlock::executeHIR(VPOCodeGenHIR *CG) {
  ReversePostOrderTraversal<VPBlockBase *> RPOT(Entry);

  // Visit the VPBlocks connected to "this", starting from it.
  for (VPBlockBase *Block : RPOT) {
    LLVM_DEBUG(dbgs() << "HIRV: VPBlock in RPO " << Block->getName() << '\n');
    Block->executeHIR(CG);
  }
}
#endif //INTEL_CUSTOMIZATION

void VPInstruction::generateInstruction(VPTransformState &State,
                                        unsigned Part) {
#if INTEL_CUSTOMIZATION
  assert(getInstruction() && "There is no underlying Instruction.");
  State.ILV->vectorizeInstruction(getInstruction());
  return;
#endif
  IRBuilder<> &Builder = State.Builder;

  switch (getOpcode()) {
  case Instruction::UDiv:
  case Instruction::SDiv:
  case Instruction::SRem:
  case Instruction::URem:
  case Instruction::Add:
  case Instruction::FAdd:
  case Instruction::Sub:
  case Instruction::FSub:
  case Instruction::Mul:
  case Instruction::FMul:
  case Instruction::FDiv:
  case Instruction::FRem:
  case Instruction::Shl:
  case Instruction::LShr:
  case Instruction::AShr:
  case Instruction::And:
  case Instruction::Or:
  case Instruction::Xor: {
    Value *A = State.get(getOperand(0), Part);
    Value *B = State.get(getOperand(1), Part);
    Value *V = Builder.CreateBinOp((Instruction::BinaryOps)getOpcode(), A, B);
    State.set(this, V, Part);
    break;
  }
  case VPInstruction::Not: {
    Value *A = State.get(getOperand(0), Part);
    Value *V = Builder.CreateNot(A);
    State.set(this, V, Part);
    break;
  }
  default:
    llvm_unreachable("Unsupported opcode for instruction");
  }
}

#if INTEL_CUSTOMIZATION
void VPInstruction::executeHIR(VPOCodeGenHIR *CG) {
  // TODO: For the reuse/invalidation of the underlying HIR to be working
  // properly, we need to do an independent traversal of all the VPInstructions
  // in the CFG and invalidate their HIR when:
  //  1) there are nested blobs that need to be widened,
  //  2) the decomposed VPInstructions don't precede their master VPInstruction
  //     (rule to be refined), and
  //  3) the decomposed VPInstructions have more than one use.

  HLInst *WInst;

  // TBD - see if anything special is needed for semiphis.
  if (Opcode == SemiPhi)
    return;

  if (HIR.isDecomposed() && HIR.isValid()) {
    // Skip decomposed VPInstruction with valid HIR. They will be codegen'ed by
    // its master VPInstruction.
    LLVM_DEBUG(dbgs() << "Skipping decomposed VPInstruction with valid HIR:"
                      << *this << "\n");
    return;
  }

  if (auto Branch = dyn_cast<VPBranchInst>(this)) {
    assert(Branch->getHLGoto() && "For HIR VPBranchInst must have HLGoto.");
    const HLGoto *HGoto = Branch->getHLGoto();
    assert(CG->isSearchLoop() && HGoto->isEarlyExit(CG->getOrigLoop()) &&
           "Only early exit gotos expected!");
    // FIXME: Temporary support for last value computation of live-outs in the
    // early exit branch. 'createNonLinearLiveOutsForEE' introduces the last
    // value computation instructions before the goto instruction for the
    // reaching definitions of the live-outs.
    CG->handleNonLinearEarlyExitLiveOuts(HGoto);

    CG->addInst(HGoto->clone(), nullptr);
    return;
  }

  if (HIR.isValid()) {
    // Master VPInstruction with valid HIR.
    assert(HIR.isMaster() && "VPInstruction with valid HIR must be a Master "
                             "VPInstruction at this point.");
    HLNode *HNode = HIR.getUnderlyingNode();
    if (auto *Inst = dyn_cast<HLInst>(HNode)) {
      unsigned Opcode = getOpcode();
      const OVLSGroup *Group = nullptr;
      const HLInst *GrpStartInst = nullptr;
      int64_t InterleaveFactor = 0, InterleaveIndex = 0;

      if (Opcode == Instruction::Load || Opcode == Instruction::Store) {
        VPlanVLSAnalysis *VLSA = CG->getVLS();
        const VPlan *Plan = CG->getPlan();
        int64_t GroupStride = 0;
        int32_t GrpSize = 0;

        // Get OPTVLS group for current load/store instruction
        Group = VLSA->getGroupsFor(Plan, this);

        // Only handle strided OptVLS Groups with no access gaps for now.
        if (Group && Group->hasAConstStride(GroupStride)) {
          uint64_t AllAccessMask = ~(UINT64_MAX << GroupStride);
          GrpSize = Group->size();

          // Access mask is currently 64 bits, skip groups with group stride >
          // 64 and access gaps.
          if ((GroupStride > 64) ||
              (Group->getNByteAccessMask() != AllAccessMask))
            Group = nullptr;
        } else
          Group = nullptr;

        if (Group) {
          VPVLSClientMemrefHIR *FirstMemref = nullptr;
          const RegDDRef *FirstMemrefDD = nullptr;
          uint64_t RefSizeInBytes = 0;

          // Check that all members of the group have same type. Currently we
          // do not handle groups such as a[i].i, a[i].d for
          //   struct {int i; double d} a[100]
          // Also setup GrpStartInst, FirstMemref, FirstMemrefDD, and
          // RefSizeInBytes.
          bool TypesMatch = true;
          for (int64_t Index = 0; Index < Group->size(); ++Index) {
            auto *Memref = cast<VPVLSClientMemrefHIR>(Group->getMemref(Index));
            auto *VPInst = Memref->getInstruction();
            const HLInst *HInst;
            const RegDDRef *MemrefDD;
            assert(VPInst->HIR.isValid() && VPInst->HIR.isMaster() &&
                   "Expected valid master HIR instruction to start group");
            HInst = dyn_cast<HLInst>(VPInst->HIR.getUnderlyingNode());
            assert(HInst && "Expected non-null group start instruction");

            MemrefDD = Memref->getRegDDRef();
            if (Index == 0) {
              auto DL = MemrefDD->getDDRefUtils().getDataLayout();

              FirstMemref = Memref;
              FirstMemrefDD = MemrefDD;
              GrpStartInst = HInst;
              RefSizeInBytes =
                  DL.getTypeSizeInBits(FirstMemrefDD->getDestType()) >> 3;
            } else if (MemrefDD->getDestType() != FirstMemrefDD->getDestType())
              TypesMatch = false;

            // Setup the interleave index of the current instruction within the
            // VLS group.
            if (Memref->getInstruction() == this) {
              int64_t Dist = 0;
              Memref->isAConstDistanceFrom(*FirstMemref, &Dist);
              InterleaveIndex = Dist / RefSizeInBytes;
            }
          }

          // Compute interleave factor based on the distance of the last memref
          // in the group from the first memref. This may not be the same as
          // the group size as we may see duplicate accesses like:
          //     a[2*i]
          //     a[2*i+1]
          //     a[2*i]
          auto *LastMemref =
              cast<VPVLSClientMemrefHIR>(Group->getMemref(GrpSize - 1));
          int64_t LastDist = 0;
          LastMemref->isAConstDistanceFrom(*FirstMemref, &LastDist);
          InterleaveFactor = LastDist / RefSizeInBytes + 1;

          // If interleave factor is less than 2, nothing special needs to be
          // done. Similarly, we do not handle the case where all memrefs in the
          // group are not of the same type.
          if (InterleaveFactor < 2 || !TypesMatch)
            Group = nullptr;
        }
      }

      CG->widenNode(Inst, nullptr, Group, InterleaveFactor, InterleaveIndex,
                    GrpStartInst);
      return;
    }
    if (auto *HIf = dyn_cast<HLIf>(HNode)) {
      // We generate a compare instruction from the IF predicate. The VPValue
      // corresponding to this instruction gets used as the condition bit
      // value for the conditional branch. We need a mapping between this
      // VPValue and the widened value so that we can generate code for the
      // predicate recipes.
      WInst = CG->widenIfPred(HIf, nullptr);
      CG->addVPValueWideRefMapping(this, WInst->getOperandDDRef(0));
      return;
    }
    if (isa<HLLoop>(HNode)) {
      // Master VPInstructions with an attached HLLoop are IV-related or bottom
      // test instructions that don't have explicit instruction representation
      // in HIR. This information will be updated directly when processing the
      // HLLoop construct.
      return;
    }

    llvm_unreachable("Master VPInstruction with unexpected HLDDNode.");
  }

  // VPInstruction with invalid or no HIR (new).
  LLVM_DEBUG(
      dbgs()
      << "TODO: Generate new HIR for VPInstruction with invalid or no HIR: "
      << *this << "\n");
  llvm_unreachable("VPInstruction with invalid HIR shouldn't be generated at "
                   "this point in VPlan.");
}
#endif

void VPInstruction::execute(VPTransformState &State) {
#if INTEL_CUSTOMIZATION
  // TODO: Remove this block of code. Its purpose is to emulate the execute()
  //       of the conditionbit recipies that have now been removed.
  if (State.UniformCBVs->count(this)) {
    Value *ScConditionBit = getUnderlyingValue();
    State.ILV->serializeInstruction(cast<Instruction>(ScConditionBit));
    Value *ConditionBit = State.ILV->getScalarValue(ScConditionBit, 0);
    assert(!ConditionBit->getType()->isVectorTy() && "Bit should be scalar");
    State.CBVToConditionBitMap[this] = ConditionBit;
    return;
  }
#endif

  assert(!State.Instance && "VPInstruction executing an Instance");
  for (unsigned Part = 0; Part < State.UF; ++Part)
    generateInstruction(State, Part);
}

void VPInstruction::print(raw_ostream &O, const Twine &Indent) const {
  O << " +\n" << Indent << "\"EMIT ";
  print(O);
  O << "\\l\"";
}

#if INTEL_CUSTOMIZATION
void VPInstruction::dump(raw_ostream &O) const {
  print(O);
  O << "\n";
}
#endif /* INTEL_CUSTOMIZATION */

void VPInstruction::print(raw_ostream &O) const {
#if INTEL_CUSTOMIZATION
  if (getOpcode() != Instruction::Store && !isa<VPBranchInst>(this)) {
    printAsOperand(O);
    O << " = ";
  }
#else
  printAsOperand(O);
  O << " = ";
#endif /* INTEL_CUSTOMIZATION */

  switch (getOpcode()) {
  case VPInstruction::Not:
    O << "not";
    break;
#if INTEL_CUSTOMIZATION
  case VPInstruction::SemiPhi:
    O << "semi-phi";
    break;
  case VPInstruction::SMax:
    O << "smax";
    break;
  case VPInstruction::UMax:
    O << "umax";
    break;
  case Instruction::Br:
    cast<VPBranchInst>(this)->print(O);
    return;
#endif
  default:
    O << Instruction::getOpcodeName(getOpcode());
  }

#if INTEL_CUSTOMIZATION
  // TODO: print type when this information will be available.
  // So far don't print anything, because PHI may not have Instruction
  if (auto *Phi = dyn_cast<const VPPHINode>(this)) {
    auto PrintValueWithBB = [&](const unsigned i) {
      O << " ";
      O << " [ ";
      Phi->getIncomingValue(i)->printAsOperand(O);
      O << ", ";
      O << Phi->getIncomingBlock(i)->getName();
      O << " ]";
    };
    const unsigned size = Phi->getNumIncomingValues();
    for (unsigned i = 0; i < size - 1; ++i) {
      PrintValueWithBB(i);
      O << ",";
    }
    PrintValueWithBB(size-1);
  }
  else
#endif // INTEL_CUSTOMIZATION
    for (const VPValue *Operand : operands()) {
      O << " ";
      Operand->printAsOperand(O);
    }
}

// Generate the code inside the body of the vectorized loop. Assumes a single
// LoopVectorBody basic block was created for this; introduces additional
// basic blocks as needed, and fills them all.
void VPlan::execute(VPTransformState *State) {

#if INTEL_CUSTOMIZATION
  // The community version and "vpo" version of "execute" for VPlan, diverge
  // considerably. Instead of having INTEL_CUSTOMIZATION for every few lines
  // of code, we decided to seperate both versions with a single
  // INTEL_CUSTOMIZATION
  BasicBlock *VectorPreHeaderBB = State->CFG.PrevBB;
  BasicBlock *VectorHeaderBB = VectorPreHeaderBB->getSingleSuccessor();
  assert(VectorHeaderBB && "Loop preheader does not have a single successor.");
  BasicBlock *VectorLatchBB = VectorHeaderBB;
  auto *HTerm = VectorHeaderBB->getTerminator();
  assert(HTerm->getNumSuccessors() == 2 &&
         "Unexpected vector loop header successors");
  unsigned MidBlockSuccNum = HTerm->getSuccessor(0) == VectorHeaderBB ? 1 : 0;
  BasicBlock *MiddleBlock = HTerm->getSuccessor(MidBlockSuccNum);
  auto CurrIP = State->Builder.saveIP();

  // 1. Make room to generate basic blocks inside loop body if needed.
  VectorLatchBB = VectorHeaderBB->splitBasicBlock(
      VectorHeaderBB->getFirstInsertionPt(), "vector.body.latch");
  Loop *L = State->LI->getLoopFor(VectorHeaderBB);
  assert(L && "Unexpected null loop for Vector Header");
  L->addBasicBlockToLoop(VectorLatchBB, *State->LI);
  // Remove the edge between Header and Latch to allow other connections.
  // Temporarily terminate with unreachable until CFG is rewired.
  // Note: this asserts xform code's assumption that getFirstInsertionPt()
  // can be dereferenced into an Instruction.
  VectorHeaderBB->getTerminator()->eraseFromParent();
  State->Builder.SetInsertPoint(VectorHeaderBB);
  State->Builder.CreateUnreachable();
  // Set insertion point to vector loop PH
  State->Builder.SetInsertPoint(VectorPreHeaderBB->getTerminator());

  // 2. Generate code in loop body of vectorized version.
  State->CFG.PrevVPBB = nullptr;
  State->CFG.PrevBB = VectorPreHeaderBB;
  State->CFG.LastBB = VectorLatchBB;

  for (VPBlockBase *CurrentBlock = Entry; CurrentBlock != nullptr;
       CurrentBlock = CurrentBlock->getSingleSuccessor()) {
    assert(CurrentBlock->getSuccessors().size() <= 1 &&
           "Multiple successors at top level.");
    CurrentBlock->execute(State);
  }

  // 3. Fix the back edges
  for (auto Edge : State->CFG.EdgesToFix) {
    VPBasicBlock *FromVPBB = Edge.first;
    assert(State->CFG.VPBB2IRBB.count(FromVPBB) &&
           "The IR basic block should be ready at this moment");
    BasicBlock *FromBB = State->CFG.VPBB2IRBB[FromVPBB];
    BasicBlock *ToBB = Edge.second;

    // We should have conditional branch from FromBB to ToBB. Conditional branch
    // is 2 edges - forward edge and backward edge.
    // The forward edge should be in-place, we are fixing the backward
    // edge only.
    assert(!isa<UnreachableInst>(FromBB->getTerminator()) &&
           "One edge should be in-place");

    BasicBlock *FirstSuccBB = FromBB->getSingleSuccessor();
    assert(FirstSuccBB && "Unexpected null successor");
    FromBB->getTerminator()->eraseFromParent();
    VPValue *CBV = FromVPBB->getCondBit();
    assert(State->CBVToConditionBitMap.count(CBV) && "Must be in map.");
    Value *NCondBit = State->CBVToConditionBitMap[CBV];
    assert(NCondBit && "Null scalar value for condition bit.");
    BranchInst::Create(FirstSuccBB, ToBB, NCondBit, FromBB);
  }

  // 4. Merge the temporary latch created with the last basic block filled.
  BasicBlock *LastBB = State->CFG.PrevBB;
  assert(isa<UnreachableInst>(LastBB->getTerminator()) &&
         "Expected VPlan CFG to terminate with unreachable");

  // LastBB will be the outermost loop exit block. Get the latch BB.
  BasicBlock *LatchBB = LastBB->getSinglePredecessor();
  assert(LatchBB && "Unexpected null latch BB");

  // Merge LatchBB with Latch.
  LatchBB->getTerminator()->eraseFromParent();
  BranchInst::Create(VectorLatchBB, LatchBB);

  bool merged = MergeBlockIntoPredecessor(VectorLatchBB, nullptr, State->LI);
  assert(merged && "Could not merge last basic block with latch.");
  (void) merged;
  VectorLatchBB = LatchBB;

  // Insert LastBB between LatchBB and MiddleBlock. TODO - currently we
  // assume MiddleBlock and LastBB do not have any PHIs. This will need
  // to be addressed if this changes.
  assert(!isa<PHINode>(MiddleBlock->begin()) &&
         "Middle block starts with a PHI");
  assert(!isa<PHINode>(LastBB->begin()) && "LastBB starts with a PHI");

  LatchBB->getTerminator()->setSuccessor(MidBlockSuccNum, LastBB);
  LastBB->getTerminator()->eraseFromParent();
  BranchInst::Create(MiddleBlock, LastBB);

  // Do no try to update dominator tree as we may be generating vector loops
  // with inner loops. Right now we are not marking any analyses as
  // preserved - so this should be ok.
  // updateDominatorTree(State->DT, VectorPreHeaderBB, VectorLatchBB);
  State->Builder.restoreIP(CurrIP);

#else
  // 0. Set the reverse mapping from VPValues to Values for code generation.
  for (auto &Entry : Value2VPValue)
    State->VPValue2Value[Entry.second] = Entry.first;

  BasicBlock *VectorPreHeaderBB = State->CFG.PrevBB;
  BasicBlock *VectorHeaderBB = VectorPreHeaderBB->getSingleSuccessor();
  assert(VectorHeaderBB && "Loop preheader does not have a single successor.");
  BasicBlock *VectorLatchBB = VectorHeaderBB;

  // 1. Make room to generate basic-blocks inside loop body if needed.
  VectorLatchBB = VectorHeaderBB->splitBasicBlock(
      VectorHeaderBB->getFirstInsertionPt(), "vector.body.latch");
  Loop *L = State->LI->getLoopFor(VectorHeaderBB);
  L->addBasicBlockToLoop(VectorLatchBB, *State->LI);
  // Remove the edge between Header and Latch to allow other connections.
  // Temporarily terminate with unreachable until CFG is rewired.
  // Note: this asserts the generated code's assumption that
  // getFirstInsertionPt() can be dereferenced into an Instruction.
  VectorHeaderBB->getTerminator()->eraseFromParent();
  State->Builder.SetInsertPoint(VectorHeaderBB);
  UnreachableInst *Terminator = State->Builder.CreateUnreachable();
  State->Builder.SetInsertPoint(Terminator);

  // 2. Generate code in loop body.
  State->CFG.PrevVPBB = nullptr;
  State->CFG.PrevBB = VectorHeaderBB;
  State->CFG.LastBB = VectorLatchBB;

  for (VPBlockBase *Block : depth_first(Entry))
    Block->execute(State);

  // 3. Merge the temporary latch created with the last basic-block filled.
  BasicBlock *LastBB = State->CFG.PrevBB;
  // Connect LastBB to VectorLatchBB to facilitate their merge.
  assert(isa<UnreachableInst>(LastBB->getTerminator()) &&
         "Expected VPlan CFG to terminate with unreachable");
  LastBB->getTerminator()->eraseFromParent();
  BranchInst::Create(VectorLatchBB, LastBB);

  // Merge LastBB with Latch.
  bool Merged = MergeBlockIntoPredecessor(VectorLatchBB, nullptr, State->LI);
  (void)Merged;
  assert(Merged && "Could not merge last basic block with latch.");
  VectorLatchBB = LastBB;

  updateDominatorTree(State->DT, VectorPreHeaderBB, VectorLatchBB);
#endif
}

#if INTEL_CUSTOMIZATION
void VPlan::executeHIR(VPOCodeGenHIR *CG) {
  assert(isa<VPRegionBlock>(Entry) && Entry->getNumPredecessors() == 0 &&
         Entry->getNumSuccessors() == 0 && "Invalid VPlan entry");
  Entry->executeHIR(CG);
}
#endif

void VPlan::updateDominatorTree(DominatorTree *DT, BasicBlock *LoopPreHeaderBB,
                                BasicBlock *LoopLatchBB) {
  BasicBlock *LoopHeaderBB = LoopPreHeaderBB->getSingleSuccessor();
  assert(LoopHeaderBB && "Loop preheader does not have a single successor.");
  DT->addNewBlock(LoopHeaderBB, LoopPreHeaderBB);
  // The vector body may be more than a single basic block by this point.
  // Update the dominator tree information inside the vector body by
  // propagating
  // it from header to latch, expecting only triangular control-flow, if any.
  BasicBlock *PostDomSucc = nullptr;
  for (auto *BB = LoopHeaderBB; BB != LoopLatchBB; BB = PostDomSucc) {
    // Get the list of successors of this block.
    std::vector<BasicBlock *> Succs(succ_begin(BB), succ_end(BB));
    assert(Succs.size() <= 2 &&
           "Basic block in vector loop has more than 2 successors.");
    PostDomSucc = Succs[0];
    if (Succs.size() == 1) {
      assert(PostDomSucc->getSinglePredecessor() &&
             "PostDom successor has more than one predecessor.");
      DT->addNewBlock(PostDomSucc, BB);
      continue;
    }
    BasicBlock *InterimSucc = Succs[1];
    if (PostDomSucc->getSingleSuccessor() == InterimSucc) {
      PostDomSucc = Succs[1];
      InterimSucc = Succs[0];
    }
    assert(InterimSucc->getSingleSuccessor() == PostDomSucc &&
           "One successor of a basic block does not lead to the other.");
    assert(InterimSucc->getSinglePredecessor() &&
           "Interim successor has more than one predecessor.");
    assert(std::distance(pred_begin(PostDomSucc), pred_end(PostDomSucc)) == 2 &&
           "PostDom successor has more than two predecessors.");
    DT->addNewBlock(InterimSucc, BB);
    DT->addNewBlock(PostDomSucc, BB);
  }
}

const Twine VPlanPrinter::getUID(const VPBlockBase *Block) {
  return (isa<VPRegionBlock>(Block) ? "cluster_N" : "N") +
    Twine(getOrCreateBID(Block));
}

const Twine VPlanPrinter::getOrCreateName(const VPBlockBase *Block) {
  const std::string &Name = Block->getName();
  if (!Name.empty())
    return Name;
  return "VPB" + Twine(getOrCreateBID(Block));
}

#if INTEL_CUSTOMIZATION
void VPlan::dump(raw_ostream &OS) const {
  if (!getName().empty())
    OS << "VPlan IR for: " << getName() << "\n";
  getEntry()->dump(OS, 1);
}
void VPlan::dump() const {
  if (!getName().empty())
    errs() << "VPlan IR for: " << getName() << "\n";
  getEntry()->dump(errs(), 1);
}
#endif /* INTEL_CUSTOMIZATION */

void VPlanPrinter::dump() {
#if INTEL_CUSTOMIZATION
  if (DumpPlainVPlanIR) {
    Plan.dump(OS);
    return;
  }
#endif /* INTEL_CUSTOMIZATION */

  Depth = 1;
  bumpIndent(0);
  OS << "digraph VPlan {\n";
  OS << "graph [labelloc=t, fontsize=30; label=\"Vectorization Plan";
  if (!Plan.getName().empty())
    OS << "\\n" << DOT::EscapeString(Plan.getName());
#if !INTEL_CUSTOMIZATION
  if (!Plan.Value2VPValue.empty()) {
    OS << ", where:";
    for (auto Entry : Plan.Value2VPValue) {
      OS << "\\n" << *Entry.second;
      OS << DOT::EscapeString(" := ");
      Entry.first->printAsOperand(OS, false);
    }
  }
#endif
  OS << "\"]\n";
  OS << "node [shape=rect, fontname=Courier, fontsize=30]\n";
  OS << "edge [fontname=Courier, fontsize=30]\n";
  OS << "compound=true\n";

  for (const VPBlockBase *Block : depth_first(Plan.getEntry()))
    dumpBlock(Block);

  OS << "}\n";
}

void VPlanPrinter::dumpBlock(const VPBlockBase *Block) {
  if (const VPBasicBlock *BasicBlock = dyn_cast<VPBasicBlock>(Block))
    dumpBasicBlock(BasicBlock);
  else if (const VPRegionBlock *Region = dyn_cast<VPRegionBlock>(Block))
    dumpRegion(Region);
  else
    llvm_unreachable("Unsupported kind of VPBlock.");
}

void VPlanPrinter::drawEdge(const VPBlockBase *From, const VPBlockBase *To,
                            bool Hidden, const Twine &Label) {
  // Due to "dot" we print an edge between two regions as an edge between the
  // exit basic block and the entry basic of the respective regions.
  const VPBlockBase *Tail = From->getExitBasicBlock();
  const VPBlockBase *Head = To->getEntryBasicBlock();
  OS << Indent << getUID(Tail) << " -> " << getUID(Head);
  OS << " [ label=\"" << Label << '\"';
  if (Tail != From)
    OS << " ltail=" << getUID(From);
  if (Head != To)
    OS << " lhead=" << getUID(To);
  if (Hidden)
    OS << "; splines=none";
  OS << "]\n";
}

void VPlanPrinter::dumpEdges(const VPBlockBase *Block) {
  auto &Successors = Block->getSuccessors();
  if (Successors.size() == 1)
    drawEdge(Block, Successors.front(), false, "");
  else if (Successors.size() == 2) {
    drawEdge(Block, Successors.front(), false, "T");
    drawEdge(Block, Successors.back(), false, "F");
  } else {
    unsigned SuccessorNumber = 0;
    for (auto *Successor : Successors)
      drawEdge(Block, Successor, false, Twine(SuccessorNumber++));
  }
}

void VPlanPrinter::dumpBasicBlock(const VPBasicBlock *BasicBlock) {
  OS << Indent << getUID(BasicBlock) << " [label =\n";
  bumpIndent(1);
  OS << Indent << "\"" << DOT::EscapeString(BasicBlock->getName()) << ":\\n\"";
  bumpIndent(1);
  for (const VPRecipeBase &Recipe : *BasicBlock)
    Recipe.print(OS, Indent);
#if INTEL_CUSTOMIZATION
  const VPValue *CBV = BasicBlock->getCondBit();
  // Dump the CondBit
  if (CBV) {
    OS << " +\n" << Indent << " \"CondBit: ";
    if (const VPInstruction *CBI = dyn_cast<VPInstruction>(CBV)) {
      CBI->printAsOperand(OS);
      OS << " (" << DOT::EscapeString(CBI->getParent()->getName()) << ")\\l\"";
    } else {
      CBV->printAsOperand(OS);
    }
  }
#endif
  bumpIndent(-2);
  OS << "\n" << Indent << "]\n";
  dumpEdges(BasicBlock);
}

void VPlanPrinter::dumpRegion(const VPRegionBlock *Region) {
  OS << Indent << "subgraph " << getUID(Region) << " {\n";
  bumpIndent(1);
  OS << Indent << "fontname=Courier\n"
     << Indent << "label=\""
     << DOT::EscapeString(Region->isReplicator() ? "<xVFxUF> " : "<x1> ")
#if INTEL_CUSTOMIZATION
     << DOT::EscapeString(Region->getName()) << " Size=" << Region->getSize()
     << "\"\n";
#else
     << DOT::EscapeString(Region->getName()) << "\"\n";
#endif

  // Dump the blocks of the region.
  assert(Region->getEntry() && "Region contains no inner blocks.");
  for (const VPBlockBase *Block : depth_first(Region->getEntry()))
    dumpBlock(Block);
  bumpIndent(-1);
  OS << Indent << "}\n";
  dumpEdges(Region);
}

void VPlanPrinter::printAsIngredient(raw_ostream &O, Value *V) {
  std::string IngredientString;
  raw_string_ostream RSO(IngredientString);
  if (auto *Inst = dyn_cast<Instruction>(V)) {
    if (!Inst->getType()->isVoidTy()) {
      Inst->printAsOperand(RSO, false);
      RSO << " = ";
    }
    RSO << Inst->getOpcodeName() << " ";
    unsigned E = Inst->getNumOperands();
    if (E > 0) {
      Inst->getOperand(0)->printAsOperand(RSO, false);
      for (unsigned I = 1; I < E; ++I)
        Inst->getOperand(I)->printAsOperand(RSO << ", ", false);
    }
  } else // !Inst
    V->printAsOperand(RSO, false);
  RSO.flush();
  O << DOT::EscapeString(IngredientString);
}

#if INTEL_CUSTOMIZATION
void VPlan::printInst2Recipe() {
  DenseMap<Instruction *, VPRecipeBase *>::iterator It, End;
  for (It = Inst2Recipe.begin(), End = Inst2Recipe.end(); It != End; ++It) {
    LLVM_DEBUG(errs() << "Instruction: " << *It->first << "\n");
    std::string RecipeString;
    raw_string_ostream RSO(RecipeString);
    VPRecipeBase *Recipe = It->second;
    Recipe->print(RSO, Twine()); // TODO: Twine
    LLVM_DEBUG(errs() << "Recipe: " << RSO.str() << "\n");
  }
}
#endif

#if INTEL_CUSTOMIZATION
void VPBlockPredicateRecipe::executeHIR(VPOCodeGenHIR *CG) {
  const auto &IncomingPredicates = getIncomingPredicates();
  unsigned NumIncoming = IncomingPredicates.size();
  unsigned UF = 1;

  for (unsigned UnrIndex = 0; UnrIndex < UF; ++UnrIndex) {
    RegDDRef *PredDDRef = nullptr;

    for (unsigned CurIncoming = 0; CurIncoming < NumIncoming; ++CurIncoming) {
      auto IncomingPredRecipe = IncomingPredicates[CurIncoming];

      if (IncomingPredRecipe->getVectorizedPredicateHIR().size() == 0)
        IncomingPredRecipe->executeHIR(CG);

      RegDDRef *CurIncomingPredDDRef =
          IncomingPredRecipe->getVectorizedPredicateHIR()[UnrIndex];
      if (!PredDDRef)
        PredDDRef = CurIncomingPredDDRef;
      else {
        auto Inst = PredDDRef->getHLDDNode()->getHLNodeUtils().createOr(
            PredDDRef->clone(), CurIncomingPredDDRef->clone(), "IncOr");
        CG->addInstUnmasked(Inst);
        PredDDRef = Inst->getOperandDDRef(0);
      }
    }
    VectorizedPredicateHIR.push_back(PredDDRef);
  }

  // Set mask value to use to mask instructions in the block
  CG->setCurMaskValue(VectorizedPredicateHIR[0]);
}
void VPBlockPredicateRecipe::dump(raw_ostream &OS) const {
  print(OS, "");
  OS << "\n";
}
#endif

void VPBlockPredicateRecipe::execute(VPTransformState &State) {
  const auto &IncomingPredicates = getIncomingPredicates();
  auto NumIncoming = IncomingPredicates.size();
  auto UF = State.UF;

  for (unsigned UnrIndex = 0; UnrIndex < UF; ++UnrIndex) {
    Value *PredValue = nullptr;

    for (unsigned CurIncoming = 0; CurIncoming < NumIncoming; ++CurIncoming) {
      auto IncomingPredRecipe = IncomingPredicates[CurIncoming];

      if (IncomingPredRecipe->getVectorizedPredicate().size() == 0)
        IncomingPredRecipe->execute(State);

      auto CurIncomingPredVal =
          IncomingPredRecipe->getVectorizedPredicate()[UnrIndex];
      if (!PredValue)
        PredValue = CurIncomingPredVal;
      else
        PredValue =
            State.Builder.CreateOr(PredValue, CurIncomingPredVal, "IncOr");
    }
    VectorizedPredicate.push_back(PredValue);
  }

  // Set mask value to use to mask instructions in the block
  State.ILV->setMaskValue(VectorizedPredicate[0]);
}

void VPBlockPredicateRecipe::print(raw_ostream &OS, const Twine &Indent) const {
  OS << Name << " = ";
  // Predicate Inputs
  if (!getIncomingPredicates().empty()) {
    VPPredicateRecipeBase *LastPred = getIncomingPredicates().back();
    for (VPPredicateRecipeBase *inputPredicate : getIncomingPredicates()) {
      OS << inputPredicate->getName();
      if (inputPredicate != LastPred) {
        OS << " || ";
      }
    }
  }
}

#if INTEL_CUSTOMIZATION
void VPIfTruePredicateRecipe::executeHIR(VPOCodeGenHIR *CG) {
  RegDDRef *PredMask =
      PredecessorPredicate
          ? PredecessorPredicate->getVectorizedPredicateHIR()[0]
          : nullptr;

  // Get the vector mask value of the branch condition
  RegDDRef *VecCondMask = CG->getWideRefForVPVal(ConditionValue);
  assert(VecCondMask && "ConditionValue is expected to be widened by now");

  // Combine with the predecessor block mask if needed - a null predecessor
  // mask implies allones(predecessor is active for all lanes).
  RegDDRef *EdgeMask;
  if (PredMask) {
    auto Inst = VecCondMask->getHLDDNode()->getHLNodeUtils().createAnd(
        VecCondMask->clone(), PredMask->clone(), "IfTPred");
    CG->addInstUnmasked(Inst);
    EdgeMask = Inst->getOperandDDRef(0);
  } else
    EdgeMask = VecCondMask;

  VectorizedPredicateHIR.push_back(EdgeMask);
}

void VPIfTruePredicateRecipe::dump(raw_ostream &OS) const {
  print(OS, "");
  OS << "\n";
}
#endif

void VPIfTruePredicateRecipe::execute(VPTransformState &State) {
  Value *PredMask = PredecessorPredicate
                        ? PredecessorPredicate->getVectorizedPredicate()[0]
                        : nullptr;

  // Get the vector mask value of the branch condition
  auto VecCondMask =
      State.ILV->getVectorValue(ConditionValue->getUnderlyingValue());

  // Combine with the predecessor block mask if needed - a null predecessor
  // mask
  // implies allones(predecessor is active for all lanes).
  Value *EdgeMask;
  if (PredMask)
    EdgeMask = State.Builder.CreateAnd(VecCondMask, PredMask);
  else
    EdgeMask = VecCondMask;

  VectorizedPredicate.push_back(EdgeMask);
  // Register the Edge with mask in CG
  State.ILV->setEdgeMask(FromBB, ToBB, EdgeMask);
}

#if INTEL_CUSTOMIZATION
void VPEdgePredicateRecipe::executeHIR(VPOCodeGenHIR *CG) {
  RegDDRef *PredMask =
      PredecessorPredicate
          ? PredecessorPredicate->getVectorizedPredicateHIR()[0]
          : nullptr;
  CG->setCurMaskValue(PredMask);
}

void VPEdgePredicateRecipe::dump(raw_ostream &OS) const {
  if (PredecessorPredicate)
    OS << Name << " = " << PredecessorPredicate->getName() << "\n";
}
#endif

void VPEdgePredicateRecipe::execute(VPTransformState &State) {
  // This recipe does not produce any code. It propagates an already
  // calculated mask value to CG.
  Value *PredMask = PredecessorPredicate
                        ? PredecessorPredicate->getVectorizedPredicate()[0]
                        : nullptr;
  State.ILV->setEdgeMask(FromBB, ToBB, PredMask);
}

void VPEdgePredicateRecipe::print(raw_ostream &OS, const Twine &Indent) const {
  OS << " +\n" << Indent << "\"" << Name << " = ";
  if (PredecessorPredicate)
    OS << PredecessorPredicate->getName();
  OS << "\\l\"";
}

void VPIfTruePredicateRecipe::print(raw_ostream &OS,
                                    const Twine &Indent) const {
  OS << Name;
  OS << " = ";
  if (PredecessorPredicate)
    OS << PredecessorPredicate->getName() << " && ";

  ConditionValue->printAsOperand(OS);
}

#if INTEL_CUSTOMIZATION
void VPIfFalsePredicateRecipe::executeHIR(VPOCodeGenHIR *CG) {
  RegDDRef *PredMask =
      PredecessorPredicate
          ? PredecessorPredicate->getVectorizedPredicateHIR()[0]
          : nullptr;

  // Get the vector mask value of the branch condition
  RegDDRef *VecCondMask = CG->getWideRefForVPVal(ConditionValue);
  assert(VecCondMask && "ConditionValue is expected to be widened by now");
  auto Inst = VecCondMask->getHLDDNode()->getHLNodeUtils().createNot(
      VecCondMask->clone(), "IfFPred");
  CG->addInstUnmasked(Inst);
  VecCondMask = Inst->getOperandDDRef(0);

  // Combine with the predecessor block mask if needed - a null predecessor
  // mask implies allones(predecessor is active for all lanes).
  RegDDRef *EdgeMask;
  if (PredMask) {
    auto Inst = VecCondMask->getHLDDNode()->getHLNodeUtils().createAnd(
        VecCondMask->clone(), PredMask->clone(), "IfFPred");
    CG->addInstUnmasked(Inst);
    EdgeMask = Inst->getOperandDDRef(0);
  } else
    EdgeMask = VecCondMask;

  VectorizedPredicateHIR.push_back(EdgeMask);
}

void VPIfFalsePredicateRecipe::dump(raw_ostream &OS) const {
  print(OS, "");
  OS << "\n";
}
#endif

void VPIfFalsePredicateRecipe::execute(VPTransformState &State) {
  Value *PredMask = PredecessorPredicate
                        ? PredecessorPredicate->getVectorizedPredicate()[0]
                        : nullptr;

  // Get the vector mask value of the branch condition - since this
  // edge is taken if the mask value is false we compute the negation
  // of this mask value.
  auto VecCondMask =
      State.ILV->getVectorValue(ConditionValue->getUnderlyingValue());
  VecCondMask = State.Builder.CreateNot(VecCondMask);

  // Combine with the predecessor block mask if needed - a null predecessor
  // mask
  // implies allones(predecessor is active for all lanes).
  Value *EdgeMask;
  if (PredMask)
    EdgeMask = State.Builder.CreateAnd(VecCondMask, PredMask);
  else
    EdgeMask = VecCondMask;

  VectorizedPredicate.push_back(EdgeMask);
  // Register the Edge with mask in CG
  State.ILV->setEdgeMask(FromBB, ToBB, EdgeMask);
}

void VPIfFalsePredicateRecipe::print(raw_ostream &OS,
                                     const Twine &Indent) const {
  OS << Name;
  OS << " = ";
  if (PredecessorPredicate)
    OS << PredecessorPredicate->getName() << " && ";

  OS << "!";
  ConditionValue->printAsOperand(OS);
}

#if INTEL_CUSTOMIZATION
void VPBranchInst::print(raw_ostream &O) const {
  O << "br ";
  const BasicBlock *BB = getTargetBlock();
  if (BB)
    O << BB->getName();
  else
    // FIXME: Call HGoto print.
    O << "<External Basic Block>";
}
#endif // INTEL_CUSTOMIZATION

#if INTEL_CUSTOMIZATION
using VPDomTree = DomTreeBase<VPBlockBase>;
template void DomTreeBuilder::Calculate<VPDomTree>(VPDomTree &DT);

using VPPostDomTree = PostDomTreeBase<VPBlockBase>;
template void DomTreeBuilder::Calculate<VPPostDomTree>(VPPostDomTree &PDT);
#endif
