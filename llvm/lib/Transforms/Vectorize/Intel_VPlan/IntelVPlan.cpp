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
static cl::opt<int>
    DumpVPlanLiveness("vplan-dump-liveness", cl::init(0), cl::Hidden,
                       cl::desc("Print VPlan instructions' liveness info"));

static cl::opt<bool> EnableNames(
    "vplan-enable-names", cl::init(false), cl::Hidden,
    cl::desc("Print VP Operands using VPValue's Name member."));

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
raw_ostream &llvm::vpo::operator<<(raw_ostream &OS, const VPValue &V) {
  if (const VPInstruction *I = dyn_cast<VPInstruction>(&V))
    I->dump(OS);
  else
    V.dump(OS);
  return OS;
}
#endif // !NDEBUG || LLVM_ENABLE_DUMP
#endif

/// \return the VPBasicBlock that is the entry of Block, possibly indirectly.
const VPBasicBlock *VPBlockBase::getEntryBasicBlock() const {
  const VPBlockBase *Block = this;
  while (const VPRegionBlock *Region = dyn_cast<VPRegionBlock>(Block))
    Block = Region->getEntry();
  return cast<VPBasicBlock>(Block);
}

VPBasicBlock *VPBlockBase::getEntryBasicBlock() {
  VPBlockBase *Block = this;
  while (VPRegionBlock *Region = dyn_cast<VPRegionBlock>(Block))
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
VPBasicBlock *VPBlockUtils::splitExitBlock(VPBlockBase *Block,
                                           VPLoopInfo *VPLInfo,
                                           VPDominatorTree &DomTree) {
  VPBasicBlock *BB = Block->getExitBasicBlock();
  VPBasicBlock *NewBlock = new VPBasicBlock(VPlanUtils::createUniqueName("BB"));
  BB->moveConditionalEOBTo(NewBlock);
  NewBlock->moveTripCountInfoFrom(BB);
  insertBlockAfter(NewBlock, BB);

  // Add NewBlock to VPLoopInfo
  if (VPLoop *Loop = VPLInfo->getLoopFor(BB)) {
    Loop->addBasicBlockToLoop(NewBlock, *VPLInfo);
  }


  // Update incoming block of VPPHINodes in successors, if any
  for (auto &Successor : NewBlock->getHierarchicalSuccessors()) {
    // Iterate over all VPPHINodes in Successor. Successor can be a region so
    // we should take its entry.
    // NOTE: Here we assume that all VPPHINodes are always placed at the top of
    // its parent VPBasicBlock
    for (VPPHINode &VPN : Successor->getEntryBasicBlock()->getVPPhis()) {
      if (VPN.getBlend())
        continue;

      // Transform the VPBBUsers vector of the PHI node by replacing any
      // occurrence of Block with NewBlock
      llvm::transform(VPN.blocks(), VPN.block_begin(),
                      [BB, NewBlock](VPBasicBlock *A) -> VPBasicBlock * {
                        if (A == BB)
                          return NewBlock;
                        return A;
                      });
    }
  }

  if (Block != BB)
    // No DomTree updates because it's not recursive.
    return NewBlock;

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

  return NewBlock;
}

// It turns A->B into A->NewSucc->B and updates VPLoopInfo, DomTree and
// PostDomTree accordingly.
VPBasicBlock *VPBlockUtils::splitBlock(VPBlockBase *Block,
                                       VPLoopInfo *VPLInfo,
                                       VPDominatorTree &DomTree,
                                       VPPostDominatorTree &PostDomTree) {
  VPBasicBlock *NewBlock = new VPBasicBlock(VPlanUtils::createUniqueName("BB"));
  if (auto *BB = dyn_cast<VPBasicBlock>(Block)) {
    BB->moveConditionalEOBTo(NewBlock);
    NewBlock->moveTripCountInfoFrom(BB);
  }
  insertBlockAfter(NewBlock, Block);

  // Add NewBlock to VPLoopInfo
  if (VPLoop *Loop = VPLInfo->getLoopFor(Block)) {
    Loop->addBasicBlockToLoop(NewBlock, *VPLInfo);
  }

  // Update incoming block of VPPHINodes in successors, if any
  for (auto &Successor : NewBlock->getSuccessors()) {
    // Iterate over all VPPHINodes in Successor. Successor can be a region so
    // we should take its entry.
    // NOTE: Here we assume that all VPPHINodes are always placed at the top of
    // its parent VPBasicBlock
    for (VPPHINode &VPN : Successor->getEntryBasicBlock()->getVPPhis()) {
      // Transform the VPBBUsers vector of the PHI node by replacing any
      // occurrence of Block with NewBlock
      llvm::transform(VPN.blocks(), VPN.block_begin(),
                      [Block, NewBlock](VPBasicBlock *A) -> VPBasicBlock * {
                        if (A == cast<VPBasicBlock>(Block))
                          return NewBlock;
                        return A;
                      });
    }
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

void VPBlockBase::setTwoSuccessors(VPValue *ConditionV, VPBlockBase *IfTrue,
                                   VPBlockBase *IfFalse) {
  assert(Successors.empty() && "Setting two successors when others exist.");
  setCondBit(ConditionV);
  appendSuccessor(IfTrue);
  appendSuccessor(IfFalse);
}

void VPBlockBase::deleteCFG(VPBlockBase *Entry) {
  SmallVector<VPBlockBase *, 8> Blocks;
  for (VPBlockBase *Block : depth_first(Entry))
    Blocks.push_back(Block);

  for (VPBlockBase *Block : Blocks)
    delete Block;
}

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
void VPBlockBase::print(raw_ostream &OS, unsigned Depth,
                        const VPlanDivergenceAnalysis *DA,
                        const Twine &NamePrefix) const {
  formatted_raw_ostream FOS(OS);
  if (auto *Region = dyn_cast<VPRegionBlock>(this)) {
    Region->print(FOS, Depth, DA, NamePrefix);
    return;
  }

  auto *BB = cast<VPBasicBlock>(this);
  BB->print(FOS, Depth, DA, NamePrefix);
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
  const SmallVectorImpl<VPBlockBase *> &Preds = getHierarchicalPredecessors();
  for (VPBlockBase *PredVPBlock : make_range(Preds.rbegin(), Preds.rend())) {

    VPBasicBlock *PredVPBB = PredVPBlock->getExitBasicBlock();
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

  VPLoopInfo *VPLI = State->VPLI;

  // TODO: Won't take place with explicit peel/reminder. But we'd need much more
  // fixes to support CG for such case anyway.
  assert(std::distance(VPLI->begin(), VPLI->end()) == 1
         && "Expected single outermost loop!");

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
void VPBasicBlock::moveConditionalEOBTo(VPBasicBlock *ToBB) {
  // Set CondBit in NewBlock. Note that we are only setting the
  // successor selector pointer. The CondBit is kept in its
  // original VPBB recipe list.
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

void VPRegionBlock::recomputeSize() {
  Size = std::distance(df_iterator<const VPBlockBase *>::begin(Entry),
                       df_iterator<const VPBlockBase *>::end(Exit));
}

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
void VPBasicBlock::print(raw_ostream &OS, unsigned Indent,
                         const VPlanDivergenceAnalysis *DA,
                         const Twine &NamePrefix) const {
  std::string StrIndent = std::string(2 * Indent, ' ');
  // Print name and predicate
  OS << StrIndent << NamePrefix << getName() << " (BP: ";
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
      if (auto *Inst = dyn_cast<VPInstruction>(&Recipe)) {
        OS << StrIndent << " ";
        Inst->dump(OS, DA);
        continue;
      }
      OS << StrIndent << " " << Recipe;
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
    for (auto Block : Predecessors)
      OS << " " << Block->getName();
  }
  OS << "\n\n";
}
#endif // !NDEBUG || LLVM_ENABLE_DUMP

/// Return true if \p Recipe is a VPInstruction in a single-use chain of
/// instructions ending in block-predicate instruction that is the last
/// instruction in the block (and thus not really masking anything).
static bool isDeadPredicateInst(VPRecipeBase &Recipe) {
  auto *Inst = dyn_cast<VPInstruction>(&Recipe);
  if (!Inst)
    return false;
  unsigned Opcode = Inst->getOpcode();
  if (Opcode == VPInstruction::Pred) {
    auto *BB = Inst->getParent();
    return ++(Inst->getIterator()) == BB->end();
  }

  if (Opcode != VPInstruction::Not && Opcode != Instruction::And)
    return false;

  if (Inst->getNumUsers() != 1)
    return false;

  return isDeadPredicateInst(*cast<VPInstruction>(*Inst->user_begin()));
}


void VPBasicBlock::executeHIR(VPOCodeGenHIR *CG) {
  CG->setCurMaskValue(nullptr);
  for (VPRecipeBase &Recipe : Recipes) {
    if (isDeadPredicateInst(Recipe))
      // This is not just emitted code clean-up, but something required to
      // support our hacky search loop CG that crashes trying to emit code for
      // "and/not" instructions that use "icmp" decomposed from the HLLoop.
      continue;
    Recipe.executeHIR(CG);
  }
}

void VPRegionBlock::computeDT(void) {
  if (!RegionDT)
    RegionDT = new VPDominatorTree();
  RegionDT->recalculate(*this);
}

void VPRegionBlock::computePDT(void) {
  if (!RegionPDT)
    RegionPDT = new VPPostDominatorTree();
  RegionPDT->recalculate(*this);
}

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
void VPRegionBlock::print(raw_ostream &OS, unsigned Indent,
                          const VPlanDivergenceAnalysis *DA,
                          const Twine &NamePrefix) const {
  SetVector<const VPBlockBase *> Printed;
  SetVector<const VPBlockBase *> SuccList;

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
  ReversePostOrderTraversal<VPBlockBase *> RPOT(Entry);
  for (const VPBlockBase *BB : RPOT) {
    BB->print(OS, Indent + SuccList.size() - 1, DA);
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
#endif // !NDEBUG || LLVM_ENABLE_DUMP

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
  State.ILV->vectorizeInstruction(this);
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

  const OVLSGroup *Group = nullptr;
  const HLInst *GrpStartInst = nullptr;
  int64_t InterleaveFactor = 0, InterleaveIndex = 0;

  // Compute group information if we have a valid master instruction
  if (HIR.isMaster() && isUnderlyingIRValid()) {
    HLNode *HNode = HIR.getUnderlyingNode();
    if (isa<HLInst>(HNode)) {
      unsigned Opcode = getOpcode();

      if (Opcode == Instruction::Load || Opcode == Instruction::Store) {
        VPlanVLSAnalysis *VLSA = CG->getVLS();
        const VPlan *Plan = CG->getPlan();
        int32_t GrpSize = 0;

        // Get OPTVLS group for current load/store instruction
        Group = VLSA->getGroupsFor(Plan, this);
        Optional<int64_t> GroupStride = Group ? Group->getConstStride() : None;

        // Only handle strided OptVLS Groups with no access gaps for now.
        if (GroupStride) {
          GrpSize = Group->size();
          APInt AccessMask = Group->computeByteAccessMask();

          // Access mask is currently 64 bits, skip groups with group stride >
          // 64 and access gaps.
          if (*GroupStride > 64 || !AccessMask.isAllOnesValue() ||
              AccessMask.getBitWidth() != *GroupStride)
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
            const HLInst *HInst;
            const RegDDRef *MemrefDD;

            // Members of the group can be memrefs which are not master
            // VPInstructions (due to HIR Temp cleanup). Assertions for validity
            // and to check if underlying HLInst is master VPI is not needed
            // anymore. We directly obtain the HInst from memref's RegDDRef.
            MemrefDD = Memref->getRegDDRef();
            HInst = cast<HLInst>(MemrefDD->getHLDDNode());
            if (Index == 0) {
              auto DL = MemrefDD->getDDRefUtils().getDataLayout();

              FirstMemref = Memref;
              FirstMemrefDD = MemrefDD;
              GrpStartInst = HInst;
              RefSizeInBytes =
                  DL.getTypeAllocSize(FirstMemrefDD->getDestType());
            } else if (MemrefDD->getDestType() != FirstMemrefDD->getDestType())
              TypesMatch = false;

            // Setup the interleave index of the current instruction within the
            // VLS group.
            if (Memref->getInstruction() == this) {
              auto DistOrNone = Memref->getConstDistanceFrom(*FirstMemref);
              InterleaveIndex = DistOrNone.getValueOr(0) / RefSizeInBytes;
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
          auto LastDistOrNone = LastMemref->getConstDistanceFrom(*FirstMemref);
          InterleaveFactor = LastDistOrNone.getValueOr(0) / RefSizeInBytes + 1;

          // If interleave factor is less than 2, nothing special needs to be
          // done. Similarly, we do not handle the case where all memrefs in the
          // group are not of the same type.
          if (InterleaveFactor < 2 || !TypesMatch)
            Group = nullptr;
        }
      }
    }
  }

  CG->widenNode(this, nullptr, Group, InterleaveFactor, InterleaveIndex,
                GrpStartInst);
}
#endif

void VPInstruction::execute(VPTransformState &State) {
  assert(!State.Instance && "VPInstruction executing an Instance");
  for (unsigned Part = 0; Part < State.UF; ++Part)
    generateInstruction(State, Part);
}

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
const char *VPInstruction::getOpcodeName(unsigned Opcode) {
  switch (Opcode) {
  case VPInstruction::Not:
    return "not";
#if INTEL_CUSTOMIZATION
  case VPInstruction::AllZeroCheck:
    return "all-zero-check";
  case VPInstruction::Pred:
    return "block-predicate";
  case VPInstruction::SMax:
    return "smax";
  case VPInstruction::UMax:
    return "umax";
  case VPInstruction::SMin:
    return "smin";
  case VPInstruction::UMin:
    return "umin";
  case VPInstruction::FMax:
    return "fmax";
  case VPInstruction::FMin:
    return "fmin";
  case VPInstruction::InductionInit:
    return "induction-init";
  case VPInstruction::InductionInitStep:
    return "induction-init-step";
  case VPInstruction::InductionFinal:
    return "induction-final";
  case VPInstruction::ReductionInit:
    return "reduction-init";
  case VPInstruction::ReductionFinal:
    return "reduction-final";
  case VPInstruction::AllocatePrivate:
    return "allocate-priv";
#endif
  default:
    return Instruction::getOpcodeName(Opcode);
  }
}

void VPInstruction::print(raw_ostream &O, const Twine &Indent) const {
  O << " +\n" << Indent << "\"EMIT ";
  print(O);
  O << "\\l\"";
}

#if INTEL_CUSTOMIZATION
void VPInstruction::dump(raw_ostream &O,
                         const VPlanDivergenceAnalysis *DA) const {
  print(O, DA);
  O << "\n";
}
#endif /* INTEL_CUSTOMIZATION */

void VPInstruction::print(raw_ostream &O,
                          const VPlanDivergenceAnalysis *DA) const {
#if INTEL_CUSTOMIZATION
  if (DA) {
    if (DA->isDivergent(*this))
      O << "[DA: Divergent] ";
    else
      O << "[DA: Uniform]   ";
  }

  if (getOpcode() != Instruction::Store && !isa<VPBranchInst>(this)) {
    printAsOperand(O);
    O << " = ";
  }
#else
  printAsOperand(O);
  O << " = ";
#endif /* INTEL_CUSTOMIZATION */

  switch (getOpcode()) {
#if INTEL_CUSTOMIZATION
  case Instruction::Br:
    cast<VPBranchInst>(this)->print(O);
    return;
  case Instruction::GetElementPtr:
    O << getOpcodeName(getOpcode());
    if (auto *VPGEP = dyn_cast<const VPGEPInstruction>(this)) {
      if (VPGEP->isInBounds()) {
        O << " inbounds";
      }
    }
    break;
  case VPInstruction::InductionInit:
    O << getOpcodeName(getOpcode()) << "{"
      << getOpcodeName(cast<const VPInductionInit>(this)->getBinOpcode())
      << "}";
    break;
  case VPInstruction::InductionInitStep:
    O << getOpcodeName(getOpcode()) << "{"
      << getOpcodeName(cast<const VPInductionInitStep>(this)->getBinOpcode())
      << "}";
    break;
  case VPInstruction::InductionFinal: {
    O << getOpcodeName(getOpcode());
    const VPInductionFinal *Ind = cast<const VPInductionFinal>(this);
    if (Ind->getBinOpcode() != Instruction::BinaryOpsEnd)
      O << "{" << getOpcodeName(Ind->getBinOpcode()) << "}";
    break;
  }
  case VPInstruction::ReductionFinal: {
    O << getOpcodeName(getOpcode()) << "{";
    Type *Ty = getType();
    if (Ty->isIntegerTy()) {
      O << (cast<const VPReductionFinal>(this)->isSigned() ? "s_" : "u_");
    }
    O << getOpcodeName(cast<const VPReductionFinal>(this)->getBinOpcode())
      << "}";
    break;
  }
#endif
  default:
    O << getOpcodeName(getOpcode());
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
    for (unsigned i = 0; i < size; ++i) {
      if (i > 0)
        O << ",";
      PrintValueWithBB(i);
    }
  } else {
    if (getOpcode() == VPInstruction::AllocatePrivate) {
      O << " ";
      getType()->print(O);
    }
#endif // INTEL_CUSTOMIZATION
    for (const VPValue *Operand : operands()) {
      O << " ";
      Operand->printAsOperand(O);
    }
#if INTEL_CUSTOMIZATION
    switch (getOpcode()) {
    case Instruction::ZExt:
    case Instruction::SExt:
    case Instruction::FPToUI:
    case Instruction::FPToSI:
    case Instruction::FPExt:
    case Instruction::PtrToInt:
    case Instruction::IntToPtr:
    case Instruction::SIToFP:
    case Instruction::UIToFP:
    case Instruction::Trunc:
    case Instruction::FPTrunc:
      O << " to ";
      getType()->print(O);
    }
  }
#endif // INTEL_CUSTOMIZATION
}
#endif // !NDEBUG || LLVM_ENABLE_DUMP

// Generate the code inside the body of the vectorized loop. Assumes a single
// LoopVectorBody basic block was created for this; introduces additional
// basic blocks as needed, and fills them all.
void VPlan::execute(VPTransformState *State) {
#if INTEL_CUSTOMIZATION
  // The community version and "vpo" version of "execute" for VPlan, diverge
  // considerably. Instead of having INTEL_CUSTOMIZATION for every few lines
  // of code, we decided to seperate both versions with a single
  // INTEL_CUSTOMIZATION
  assert(std::distance(VPLInfo->begin(), VPLInfo->end()) == 1 &&
         "Expected single outermost loop!");
  VPLoop *VLoop = *VPLInfo->begin();
  State->ILV->setVPlan(this, getLoopEntities(VLoop));
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

  for (VPBlockBase *CurrentBlock = Entry.get(); CurrentBlock != nullptr;
       CurrentBlock = CurrentBlock->getSingleSuccessor()) {
    assert(CurrentBlock->getSuccessors().size() <= 1 &&
           "Multiple successors at top level.");
    CurrentBlock->execute(State);
  }

  // 3. Fix the edges for blocks in VPBBsToFix list.
  for (auto VPBB : State->CFG.VPBBsToFix) {
    BasicBlock *BB = State->CFG.VPBB2IRBB[VPBB];
    assert(BB && "Unexpected null basic block for VPBB");

    unsigned Idx = 0;
    auto *BBTerminator = BB->getTerminator();

    for (VPBlockBase *SuccVPBlock : VPBB->getHierarchicalSuccessors()) {
      VPBasicBlock *SuccVPBB = SuccVPBlock->getEntryBasicBlock();
      BBTerminator->setSuccessor(Idx, State->CFG.VPBB2IRBB[SuccVPBB]);
      ++Idx;
    }
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
  assert(isa<VPRegionBlock>(Entry.get()) && Entry->getNumPredecessors() == 0 &&
         Entry->getNumSuccessors() == 0 && "Invalid VPlan entry");
  CG->createAndMapLoopEntityRefs();
  Entry->executeHIR(CG);
}

void VPlan::verifyVPConstants() const {
  SmallPtrSet<const Constant *, 16> ConstantSet;
  for (const auto &Pair : VPConstants) {
    const Constant *KeyConst = Pair.first;
    assert(KeyConst == Pair.second->getConstant() &&
           "Value key and VPConstant's underlying Constant must be the same!");
    // Checking that an element is repeated in a map is unnecessary but it
    // will catch bugs if the data structure is changed in the future.
    assert(!ConstantSet.count(KeyConst) && "Repeated VPConstant!");
    ConstantSet.insert(KeyConst);
  }
}

void VPlan::verifyVPExternalDefs() const {
  SmallPtrSet<const Value *, 16> ValueSet;
  for (const auto &Def : VPExternalDefs) {
    const Value *KeyVal = Def->getUnderlyingValue();
    assert(!ValueSet.count(KeyVal) && "Repeated VPExternalDef!");
    ValueSet.insert(KeyVal);
  }
}

void VPlan::verifyVPExternalDefsHIR() const {
  SmallSet<unsigned, 16> SymbaseSet;
  SmallSet<unsigned, 16> IVLevelSet;
  for (const auto &ExtDef : VPExternalDefsHIR) {
    const VPOperandHIR *HIROperand = ExtDef.getOperandHIR();

    // Deeper verification depending on the kind of the underlying HIR operand.
    if (const auto *Blob = dyn_cast<VPBlob>(HIROperand)) {
      // For blobs we check that the symbases are unique.
      unsigned Symbase = Blob->getBlob()->getSymbase();
      assert(!SymbaseSet.count(Symbase) && "Repeated blob VPExternalDef!");
      SymbaseSet.insert(Symbase);
    } else {
      // For IVs we check that the IV levels are unique.
      const auto *IV = cast<VPIndVar>(HIROperand);
      unsigned IVLevel = IV->getIVLevel();
      assert(!IVLevelSet.count(IVLevel) && "Repeated IV VPExternalDef!");
      IVLevelSet.insert(IVLevel);
    }
  }
}

void VPlan::verifyVPMetadataAsValues() const {
  SmallPtrSet<const MetadataAsValue *, 16> MDAsValueSet;
  for (const auto &Pair : VPMetadataAsValues) {
    const MetadataAsValue *KeyMD = Pair.first;
    assert(KeyMD == Pair.second->getMetadataAsValue() &&
           "Value key and VPMetadataAsValue's underlying MetadataAsValue must "
           "be the same!");
    // Checking that an element is repeated in a map is unnecessary but it
    // will catch bugs if the data structure is changed in the future.
    assert(!MDAsValueSet.count(KeyMD) && "Repeated MetadataAsValue!");
    MDAsValueSet.insert(KeyMD);
  }
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

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
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
void VPlan::dump(raw_ostream &OS, bool DumpDA) const {
  if (!getName().empty())
    OS << "VPlan IR for: " << getName() << "\n";
  for (auto EIter = LoopEntities.begin(), End = LoopEntities.end();
       EIter != End; ++EIter) {
    VPLoopEntityList *E = EIter->second.get();
    E->dump(OS, EIter->first->getHeader());
  }
  const VPBlockBase *Entry = getEntry();
  Entry->print(OS, 1, DumpDA ? getVPlanDA() : nullptr);
  for (auto &Succ : Entry->getSuccessors()) {
    Succ->print(OS, 1, DumpDA ? getVPlanDA() : nullptr);
  }
  if (!VPExternalUses.empty()) {
    OS << "External Uses:\n";
    for (auto &ExtUse : VPExternalUses) {
      ExtUse.second->dump(OS);
      OS << "\n";
    }
  }
}

void VPlan::dump() const {
  dump(dbgs(), true);
}

void VPlan::dumpLivenessInfo(raw_ostream &OS) const {
  if (DumpVPlanLiveness == 0 || !VPLInfo)
    return;
  OS << "Live-in and Live-out info:\n";
  if (!VPExternalDefs.empty()) {
    OS << "External defs:\n";
    for (auto DI = VPExternalDefs.begin(); DI != VPExternalDefs.end(); DI++)
      OS << *(*DI) << "\n";
  }
  if (!VPExternalDefsHIR.empty()) {
    OS << "External defs:\n";
    for (auto DI = VPExternalDefsHIR.begin(); DI != VPExternalDefsHIR.end();
         DI++)
      OS << *DI << "\n";
  }
  if (!VPExternalUses.empty()) {
    OS << "Used externally:\n";
    for (auto UI = VPExternalUses.begin(); UI != VPExternalUses.end(); UI++) {
      const VPValue *Op = UI->second->getOperand(0);
      Op->printAsOperand(OS);
      if (DumpVPlanLiveness > 1 && UI->second->getUnderlyingValue())
        OS << " (used by " << *(UI->second->getUnderlyingValue()) << ")\n";
      else
        OS << "\n";
    }
  }
  if (!VPExternalUsesHIR.empty()) {
    OS << "Used externally:\n";
    for (auto UI = VPExternalUsesHIR.begin(); UI != VPExternalUsesHIR.end();
         UI++) {
      const VPValue *Op = UI->getOperand(0);
      Op->printAsOperand(OS);
      if (DumpVPlanLiveness > 1 && UI->getUnderlyingValue())
        OS << " (used by " << *(UI->getUnderlyingValue()) << ")\n";
      else
        OS << "\n";
    }
  }
  std::function<void(const VPBasicBlock *)> dumpBlockLiveness =
      [&](const VPBasicBlock *Block) {
        // For each live-in or live-out instruction in the Block print the
        // corresponding comment
        const VPLoop *Loop = VPLInfo->getLoopFor(Block);
        if (DumpVPlanLiveness > 2)
          OS << "Liveness for BBlock: " << Block->getName() << "\n";
        if (Loop == nullptr) {
          if (DumpVPlanLiveness > 2)
            OS << "no loop found\n";
          return;
        }
        for (const VPRecipeBase &Recipe : *Block) {
          const auto *VPInst = dyn_cast<VPInstruction>(&Recipe);
          if (!VPInst)
            continue;
          if (DumpVPlanLiveness > 2)
            OS << "Instruction: " << *VPInst << "\n";
          for (const VPValue *Op : VPInst->operands()) {
            SmallVector<const VPLoop *, 4> LoopList;
            const VPLoop *ParentLoop = Loop;
            while (ParentLoop) {
              if (!ParentLoop->isLiveIn(Op))
                break;
              LoopList.push_back(ParentLoop);
              ParentLoop = ParentLoop->getParentLoop();
            }
            if (LoopList.size()) {
              Op->printAsOperand(OS);
              OS << " livein in the loops: ";
              for (const VPLoop *L : LoopList)
                OS << " " << L->getLoopPreheader()->getName();
              OS << "\n";
            }
          }
          if (Loop->isLiveOut(VPInst)) {
            VPInst->printAsOperand(OS);
            OS << " liveout in the loop: "
               << Loop->getLoopPreheader()->getName() << "\n";
          }
        }
      };
  std::function<void(const VPBlockBase *)> dumpLiveness =
      [&](const VPBlockBase *VPBlock) {
        // Print liveness information for a basic block or for a region
        if (auto Region = dyn_cast<VPRegionBlock>(VPBlock)) {
          for (const VPBlockBase *Block : depth_first(Region->getEntry()))
            dumpLiveness(Block);
        } else {
          const auto *VPBB = cast<VPBasicBlock>(VPBlock);
          dumpBlockLiveness(VPBB);
        }
      };

  dumpLiveness(getEntry());
  OS << "Live-in and Live-out info end\n";
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
#endif // !NDEBUG || LLVM_ENABLE_DUMP

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
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
void VPBlockPredicateRecipe::dump(raw_ostream &OS) const {
  print(OS, "");
  OS << "\n";
}
#endif // !NDEBUG || LLVM_ENABLE_DUMP
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

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
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
#endif // !NDEBUG || LLVM_ENABLE_DUMP

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

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
void VPIfTruePredicateRecipe::dump(raw_ostream &OS) const {
  print(OS, "");
  OS << "\n";
}
#endif // !NDEBUG || LLVM_ENABLE_DUMP
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

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
void VPEdgePredicateRecipe::dump(raw_ostream &OS) const {
  if (PredecessorPredicate)
    OS << Name << " = " << PredecessorPredicate->getName() << "\n";
}
#endif // !NDEBUG || LLVM_ENABLE_DUMP
#endif

void VPEdgePredicateRecipe::execute(VPTransformState &State) {
  // This recipe does not produce any code. It propagates an already
  // calculated mask value to CG.
  Value *PredMask = PredecessorPredicate
                        ? PredecessorPredicate->getVectorizedPredicate()[0]
                        : nullptr;
  State.ILV->setEdgeMask(FromBB, ToBB, PredMask);
}

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
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
#endif // !NDEBUG || LLVM_ENABLE_DUMP

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

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
void VPIfFalsePredicateRecipe::dump(raw_ostream &OS) const {
  print(OS, "");
  OS << "\n";
}
#endif // !NDEBUG || LLVM_ENABLE_DUMP
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

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
void VPIfFalsePredicateRecipe::print(raw_ostream &OS,
                                     const Twine &Indent) const {
  OS << Name;
  OS << " = ";
  if (PredecessorPredicate)
    OS << PredecessorPredicate->getName() << " && ";

  OS << "!";
  ConditionValue->printAsOperand(OS);
}
#endif // !NDEBUG || LLVM_ENABLE_DUMP

#if INTEL_CUSTOMIZATION
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
void VPBranchInst::print(raw_ostream &O) const {
  O << "br ";
  const BasicBlock *BB = getTargetBlock();
  if (BB)
    O << BB->getName();
  else
    // FIXME: Call HGoto print.
    O << "<External Basic Block>";
}
#endif // !NDEBUG || LLVM_ENABLE_DUMP

void VPValue::replaceAllUsesWithImpl(VPValue *NewVal, VPLoop *Loop,
                                     VPBasicBlock *VPBB, bool InvalidateIR) {
  assert(NewVal && "Can't replace uses with null value");
  assert(getType() == NewVal->getType() && "Incompatible data types");
  assert(!(Loop && VPBB) && "Cannot have both Loop and VPBB to be non-null.");
  unsigned Cnt = 0;
  while (getNumUsers() > Cnt) {
    if (Loop) {
      if (auto Instr = dyn_cast<VPInstruction>(Users[Cnt]))
        if (!Loop->contains(cast<VPBlockBase>(Instr->getParent()))) {
          ++Cnt;
          continue;
        }
    } else if (VPBB) {
      if (auto Instr = dyn_cast<VPInstruction>(Users[Cnt]))
        if (Instr->getParent() != VPBB) {
          ++Cnt;
          continue;
        }
    }
    Users[Cnt]->replaceUsesOfWith(this, NewVal, InvalidateIR);
  }
}

bool VPValue::isUnderlyingIRValid() const {
  if (auto *VPI = dyn_cast<VPInstruction>(this))
    return IsUnderlyingValueValid || VPI->HIR.isValid();
  else {
    // Non VPInstruction values can never be invalidated.
    return true;
  }
}

void VPValue::invalidateUnderlyingIR() {
  // Non VPInstruction values should not be invalidated since they represent
  // entities outside the loop being vectorized and it is illegal to modify
  // their underlying IR. Hence invalidation is strictly limited to
  // VPInstructions only.
  if (auto *VPI = dyn_cast<VPInstruction>(this)) {
    IsUnderlyingValueValid = false;
    // Temporary hook-up to ignore loop induction related instructions during CG
    // by not invalidating them.
    // TODO: Remove this code after VPInduction support is added to HIR CG.
    const HLNode *HNode = VPI->HIR.getUnderlyingNode();
    if (HNode && isa<HLLoop>(HNode))
      return;
    VPI->HIR.invalidate();

    // At this point, we don't have a use-case where invalidation of users of
    // instruction is needed. This is because VPInstructions mostly represent
    // r-value of a HLInst and modifying the r-value should not affect l-value
    // temp refs. For stores this was never an issue since store instructions
    // cannot have users. In case of LLVM-IR invalidating an instruction might
    // mean that the underlying bit value is different. If this is the case then
    // invalidation should be propagated to users as well.
  }
}

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
void VPValue::printAsOperand(raw_ostream &OS) const {
  if (EnableNames && !Name.empty())
    // There is no interface to enforce uniqueness of the names, so continue
    // using the pointer-based name for the suffix.
    OS << *getType() << " %" << Name << "."
       << (unsigned short)(unsigned long long)this;
  else
    OS << *getType() << " %vp" << (unsigned short)(unsigned long long)this;
}
#endif // !NDEBUG || LLVM_ENABLE_DUMP

void VPBlockUtils::setParentRegionForBody(VPRegionBlock *Region) {
  for (VPBlockBase *Block :
       make_range(df_iterator<VPBlockBase *>::begin(Region->getEntry()),
                  df_iterator<VPBlockBase *>::end(Region->getExit()))) {
    Block->setParent(Region);
  }
}

const VPLoopRegion *VPlanUtils::findNthLoopDFS(const VPlan *Plan, unsigned N) {
  std::function<const VPLoopRegion *(const VPBlockBase *)> Dfs =
      [&](const VPBlockBase *Block) -> const VPLoopRegion * {
    if (const auto Loop = dyn_cast<const VPLoopRegion>(Block)) {
      --N;
      if (N == 0) {
        return Loop;
      }
    }

    if (const auto Region = dyn_cast<const VPRegionBlock>(Block))
      for (const VPBlockBase *Block : depth_first(Region->getEntry()))
        if (const VPLoopRegion *Loop = Dfs(Block))
          return Loop;

    return nullptr;
  };

  return Dfs(Plan->getEntry());
}

using VPDomTree = DomTreeBase<VPBlockBase>;
template void DomTreeBuilder::Calculate<VPDomTree>(VPDomTree &DT);

using VPPostDomTree = PostDomTreeBase<VPBlockBase>;
template void DomTreeBuilder::Calculate<VPPostDomTree>(VPPostDomTree &PDT);
#endif
