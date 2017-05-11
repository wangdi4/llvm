//===- VPlan.cpp - Vectorizer Plan ----------------------------------------===//
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

#include "VPlan.h"
#include "./VPlan/LoopVectorizationCodeGen.h"
#include "llvm/ADT/PostOrderIterator.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Dominators.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/GraphWriter.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"

using namespace llvm;

#define DEBUG_TYPE "vplan"

unsigned VPlanUtils::NextOrdinal = 1;

VPOneByOneRecipeBase::VPOneByOneRecipeBase(unsigned char SC,
                                           const BasicBlock::iterator B,
                                           const BasicBlock::iterator E,
                                           class VPlan *Plan)
    : VPRecipeBase(SC), Begin(B), End(E) {
  for (auto It = B; It != E; ++It)
    Plan->setInst2Recipe(&*It, this);
}

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

void VPBlockBase::setConditionBitRecipe(VPConditionBitRecipeBase *R,
                                        VPlan *Plan) {
  ConditionBitRecipe = R;
  if (R)
    Plan->setConditionBitRecipeUser(R, this);
}

BasicBlock *
VPBasicBlock::createEmptyBasicBlock(VPTransformState::CFGState &CFG) {
  // BB stands for IR BasicBlocks. VPBB stands for VPlan VPBasicBlocks.
  // Pred stands for Predecessor. Prev stands for Previous, last
  // visited/created.
  BasicBlock *PrevBB = CFG.PrevBB;
  BasicBlock *NewBB = BasicBlock::Create(PrevBB->getContext(), "VPlannedBB",
                                         PrevBB->getParent(), CFG.LastBB);
  DEBUG(dbgs() << "LV: created " << NewBB->getName() << '\n');

  // Hook up the new basic block to its predecessors.
  for (VPBlockBase *PredVPBlock : getHierarchicalPredecessors()) {
    VPBasicBlock *PredVPBB = PredVPBlock->getExitBasicBlock();
    if (!CFG.VPBB2IRBB.count(PredVPBB)) {
      // Back edge from inner loop
      CFG.EdgesToFix[PredVPBB] = NewBB;
    } else {
      BasicBlock *PredBB = CFG.VPBB2IRBB[PredVPBB];
      DEBUG(dbgs() << "LV: draw edge from" << PredBB->getName() << '\n');
      if (isa<UnreachableInst>(PredBB->getTerminator())) {
        PredBB->getTerminator()->eraseFromParent();
        BranchInst::Create(NewBB, PredBB);
      } else {
        // Replace old unconditional branch with new conditional branch.
        // Note: we rely on traversing the successors in order.
        BasicBlock *FirstSuccBB;
        BasicBlock *SecondSuccBB;

        if (PredVPBB->getSuccessors()[0] == this) {
          FirstSuccBB = NewBB;
          SecondSuccBB = PredBB->getSingleSuccessor();
        } else {
          FirstSuccBB = PredBB->getSingleSuccessor();
          SecondSuccBB = NewBB;
        }

        PredBB->getTerminator()->eraseFromParent();
        assert(PredVPBlock->getConditionBitRecipe() &&
               "Expected ConditionBitRecipe.");
        Value *Bit = PredVPBlock->getConditionBitRecipe()->getConditionBit();
        assert(Bit && "Cannot create conditional branch with empty bit.");
        BranchInst::Create(FirstSuccBB, SecondSuccBB, Bit, PredBB);
      }
    }
  }
  return NewBB;
}

#ifndef INTEL_CUSTOMIZATION
#ifdef INTEL_CUSTOMIZATION
// Eventually, we will remove the ifndef macro above, when we use separate
// libraries for opensource VPlan and VPO VPlan. In the meantime, this function
// cannot be shared by both implementations. This implementation is for
// opensource VPlan, currently disabled in XMAIN. Implementation for VPO is in
// IntelVPlan.cpp
#endif
void VPBasicBlock::vectorize(VPTransformState *State) {
  VPIterationInstance *I = State->Instance;
  bool Replica = I && !(I->Part == 0 && I->Lane == 0);
  VPBasicBlock *PrevVPBB = State->CFG.PrevVPBB;
  VPBlockBase *SingleHPred = nullptr;
  BasicBlock *NewBB = State->CFG.PrevBB; // Reuse it if possible.

  // 1. Create an IR basic block, or reuse the last one if possible.
  // The last IR basic block is reused in three cases:
  // A. the first VPBB reuses the header BB - when PrevVPBB is null;
  // B. when the current VPBB has a single (hierarchical) predecessor which
  //    is PrevVPBB and the latter has a single (hierarchical) successor; and
  // C. when the current VPBB is an entry of a region replica - where PrevVPBB
  //    is the exit of this region from a previous instance.
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
  DEBUG(dbgs() << "LV: vectorizing VPBB:" << getName()
               << " in BB:" << NewBB->getName() << '\n');

  State->CFG.VPBB2IRBB[this] = NewBB;
  State->CFG.PrevVPBB = this;

  for (VPRecipeBase &Recipe : Recipes)
    Recipe.vectorize(*State);

  DEBUG(dbgs() << "LV: filled BB:" << *NewBB);
}
#endif // INTEL_CUSTOMIZATION

#ifdef INTEL_CUSTOMIZATION
void VPBasicBlock::moveConditionalEOBTo(VPBasicBlock *ToBB, VPlan *Plan) {
  // Set ConditionBitRecipe in NewBlock. Note that we are only setting the
  // successor selector pointer. The ConditionBitRecipe is kept in its
  // original VPBB recipe list.
  if (getNumSuccessors() > 1) {
    assert(getConditionBitRecipe() && "Missing ConditionBitRecipe");
    ToBB->setConditionBitRecipe(getConditionBitRecipe(), Plan);
    ToBB->setCBlock(CBlock);
    ToBB->setTBlock(TBlock);
    ToBB->setFBlock(FBlock);
    setConditionBitRecipe(nullptr, Plan);
    CBlock = TBlock = FBlock = nullptr;
  }
}

void VPRegionBlock::recomputeSize() {
  Size = std::distance(df_iterator<const VPBlockBase *>::begin(Entry),
                       df_iterator<const VPBlockBase *>::end(Exit));
}
#endif

void VPRegionBlock::vectorize(VPTransformState *State) {
  ReversePostOrderTraversal<VPBlockBase *> RPOT(Entry);
  typedef std::vector<VPBlockBase *>::reverse_iterator rpo_iterator;

  if (!isReplicator()) {
    // Visit the VPBlocks connected to \p this, starting from it.
    for (rpo_iterator I = RPOT.begin(); I != RPOT.end(); ++I) {
      DEBUG(dbgs() << "LV: VPBlock in RPO " << (*I)->getName() << '\n');
      (*I)->vectorize(State);
    }
    return;
  }

  assert(!State->Instance &&
         "Replicating a Region only in null context instance.");
  VPIterationInstance I;
  State->Instance = &I;

  for (I.Part = 0; I.Part < State->UF; ++I.Part)
    for (I.Lane = 0; I.Lane < State->VF; ++I.Lane)
      // Visit the VPBlocks connected to \p this, starting from it.
      for (rpo_iterator I = RPOT.begin(); I != RPOT.end(); ++I) {
        DEBUG(dbgs() << "LV: VPBlock in RPO " << (*I)->getName() << '\n');
        (*I)->vectorize(State);
      }

  State->Instance = nullptr;
}

/// Generate the code inside the body of the vectorized loop. Assumes a single
/// LoopVectorBody basic block was created for this; introduces additional
/// basic blocks as needed, and fills them all.
void VPlan::vectorize(VPTransformState *State) {

#ifdef INTEL_CUSTOMIZATION
// NOTE: This function is not used in VPO vectorizer. IntelVPlan::vectorize is
// used instead.
#endif

  BasicBlock *VectorPreHeaderBB = State->CFG.PrevBB;
  BasicBlock *VectorHeaderBB = VectorPreHeaderBB->getSingleSuccessor();
  assert(VectorHeaderBB && "Loop preheader does not have a single successor.");
  BasicBlock *VectorLatchBB = VectorHeaderBB;
  auto CurrIP = State->Builder.saveIP();

  // 1. Make room to generate basic blocks inside loop body if needed.
  VectorLatchBB = VectorHeaderBB->splitBasicBlock(
      VectorHeaderBB->getFirstInsertionPt(), "vector.body.latch");
  Loop *L = State->LI->getLoopFor(VectorHeaderBB);
  L->addBasicBlockToLoop(VectorLatchBB, *State->LI);
  // Remove the edge between Header and Latch to allow other connections.
  // Temporarily terminate with unreachable until CFG is rewired.
  // Note: this asserts xform code's assumption that getFirstInsertionPt()
  // can be dereferenced into an Instruction.
  VectorHeaderBB->getTerminator()->eraseFromParent();
  State->Builder.SetInsertPoint(VectorHeaderBB);
  UnreachableInst *Terminator = State->Builder.CreateUnreachable();
  State->Builder.SetInsertPoint(Terminator);

  // 2. Generate code in loop body of vectorized version.
  State->CFG.PrevVPBB = nullptr;
  State->CFG.PrevBB = VectorHeaderBB;
  State->CFG.LastBB = VectorLatchBB;

  for (VPBlockBase *CurrentBlock = Entry; CurrentBlock != nullptr;
       CurrentBlock = CurrentBlock->getSingleSuccessor()) {
    assert(CurrentBlock->getSuccessors().size() <= 1 &&
           "Multiple successors at top level.");
    CurrentBlock->vectorize(State);
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
    FromBB->getTerminator()->eraseFromParent();
    Value *Bit = FromVPBB->getConditionBitRecipe()->getConditionBit();
    assert(Bit && "Cannot create conditional branch with empty bit.");
    BranchInst::Create(FirstSuccBB, ToBB, Bit, FromBB);
  }

  // 3. Merge the temporary latch created with the last basic block filled.
  BasicBlock *LastBB = State->CFG.PrevBB;
  // Connect LastBB to VectorLatchBB to facilitate their merge.
  assert(isa<UnreachableInst>(LastBB->getTerminator()) &&
         "Expected VPlan CFG to terminate with unreachable");
  LastBB->getTerminator()->eraseFromParent();
  BranchInst::Create(VectorLatchBB, LastBB);

  // Merge LastBB with Latch.
  bool merged = MergeBlockIntoPredecessor(VectorLatchBB, nullptr, State->LI);
  assert(merged && "Could not merge last basic block with latch.");
  VectorLatchBB = LastBB;

  updateDominatorTree(State->DT, VectorPreHeaderBB, VectorLatchBB);
  State->Builder.restoreIP(CurrIP);
}

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

const char *VPlanPrinter::getNodePrefix(const VPBlockBase *Block) {
  if (isa<VPBasicBlock>(Block))
    return "";
  assert(isa<VPRegionBlock>(Block) && "Unsupported kind of VPBlock.");
  return "cluster_";
}

const std::string &
VPlanPrinter::getReplicatorString(const VPRegionBlock *Region) {
  static std::string ReplicatorString(DOT::EscapeString("<xVFxUF>"));
  static std::string NonReplicatorString(DOT::EscapeString("<x1>"));
  return Region->isReplicator() ? ReplicatorString : NonReplicatorString;
}

void VPlanPrinter::dump(const std::string &Title) {
  resetDepth();
  OS << "digraph VPlan {\n";
  OS << "graph [labelloc=t, fontsize=30, splines=spline; "
        "label=\"Vectorization "
        "Plan";
  if (!Title.empty())
    OS << "\\n" << DOT::EscapeString(Title);
  OS << "\"]\n";
  OS << "node [shape=record]\n";
  OS << "compound=true\n";

  for (const VPBlockBase *CurrentBlock = Plan.getEntry();
       CurrentBlock != nullptr;
       CurrentBlock = CurrentBlock->getSingleSuccessor())
    dumpBlock(CurrentBlock);

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

/// Print the information related to a CFG edge between two VPBlockBases.
void VPlanPrinter::drawEdge(const VPBlockBase *From, const VPBlockBase *To,
                            bool Hidden, const Twine &Label) {
  // Due to "dot" we print an edge between two regions as an edge between the
  // exit basic block and the entry basic of the respective regions.
  const VPBlockBase *Tail = From->getExitBasicBlock();
  const VPBlockBase *Head = To->getEntryBasicBlock();

  OS << Indent << getNodePrefix(Tail) << DOT::EscapeString(Tail->getName())
     << " -> " << getNodePrefix(Head) << DOT::EscapeString(Head->getName());
  OS << " [ label=\"" << Label << '\"';
  if (Tail != From)
    OS << " ltail=" << getNodePrefix(From)
       << DOT::EscapeString(From->getName());
  if (Head != To)
    OS << " lhead=" << getNodePrefix(To) << DOT::EscapeString(To->getName());
  if (Hidden)
    OS << "; splines=none";
  OS << "]\n";
}

/// Print the information related to the CFG edges going out of a given
/// \p Block, followed by printing the successor blocks themselves.
void VPlanPrinter::dumpEdges(const VPBlockBase *Block) {
  std::string Cond = "";
  if (auto *ConditionBitRecipe = Block->getConditionBitRecipe())
    Cond = ConditionBitRecipe->getName().str();
  unsigned SuccessorNumber = 1;
  for (auto *Successor : Block->getSuccessors()) {
    drawEdge(Block, Successor, false,
             Twine() + (SuccessorNumber == 2 ? "!" : "") + Twine(Cond));
    ++SuccessorNumber;
  }
}

/// Print a VPBasicBlock, including its VPRecipes, followed by printing its
/// successor blocks.
void VPlanPrinter::dumpBasicBlock(const VPBasicBlock *BasicBlock) {
  std::string Indent(Depth * TabLength, ' ');
  OS << Indent << getNodePrefix(BasicBlock)
     << DOT::EscapeString(BasicBlock->getName()) << " [label = \"{"
     << DOT::EscapeString(BasicBlock->getName());

  for (const VPRecipeBase &Recipe : BasicBlock->getRecipes()) {
    OS << " | ";
    std::string RecipeString;
    raw_string_ostream RSO(RecipeString);
    Recipe.print(RSO);
    OS << DOT::EscapeString(RSO.str());
  }

  OS << "}\"]\n";
  dumpEdges(BasicBlock);
}

/// Print a given \p Region of the VPlan.
void VPlanPrinter::dumpRegion(const VPRegionBlock *Region) {
  OS << Indent << "subgraph " << getNodePrefix(Region)
     << DOT::EscapeString(Region->getName()) << " {\n";
  increaseDepth();
  OS << Indent;
  OS << "label = \"" << getReplicatorString(Region) << " "
#ifdef INTEL_CUSTOMIZATION
     << DOT::EscapeString(Region->getName()) << " Size=" << Region->getSize()
     << "\"\n\n";
#else
     << DOT::EscapeString(Region->getName()) << "\"\n\n";
#endif

  // Dump the blocks of the region.
  assert(Region->getEntry() && "Region contains no inner blocks.");

  for (const VPBlockBase *Block : depth_first(Region->getEntry()))
    dumpBlock(Block);

  decreaseDepth();
  OS << Indent << "}\n";
  dumpEdges(Region);
}

void VPlan::printInst2Recipe() {
  DenseMap<Instruction *, VPRecipeBase *>::iterator It, End;
  for (It = Inst2Recipe.begin(), End = Inst2Recipe.end(); It != End; ++It) {
    DEBUG(errs() << "Instruction: " << *It->first << "\n");
    std::string RecipeString;
    raw_string_ostream RSO(RecipeString);
    VPRecipeBase *Recipe = It->second;
    Recipe->print(RSO);
    DEBUG(errs() << "Recipe: " << RSO.str() << "\n");
  }
}

void VPAllOnesPredicateRecipe::vectorize(VPTransformState &State) {
  // Nothing to do for AllOnesPredicate case - push null to indicate
  // that we do not need a mask value for this case.
  for (unsigned Index = 0; Index < State.UF; ++Index)
    VectorizedPredicate.push_back(nullptr);
}

void VPAllOnesPredicateRecipe::print(raw_ostream &O) const { O << Name; }

void VPBlockPredicateRecipe::vectorize(VPTransformState &State) {
  const auto &IncomingPredicates = getIncomingPredicates();
  auto NumIncoming = IncomingPredicates.size();
  auto UF = State.UF;

  for (unsigned UnrIndex = 0; UnrIndex < UF; ++UnrIndex) {
    Value *PredValue = nullptr;

    for (unsigned CurIncoming = 0; CurIncoming < NumIncoming; ++CurIncoming) {
      auto IncomingPredRecipe = IncomingPredicates[CurIncoming];

      if (IncomingPredRecipe->getVectorizedPredicate().size() == 0)
        IncomingPredRecipe->vectorize(State);

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

void VPBlockPredicateRecipe::print(raw_ostream &O) const {
  O << Name;
  // Predicate Inputs
  if (!getIncomingPredicates().empty()) {
    O << " = ";
    for (VPPredicateRecipeBase *inputPredicate : getIncomingPredicates()) {
      O << inputPredicate->getName();
      if (inputPredicate != getIncomingPredicates().back()) {
        O << " || ";
      }
    }
  }
}

void VPIfTruePredicateRecipe::vectorize(VPTransformState &State) {
  Value *PredMask = PredecessorPredicate
                        ? PredecessorPredicate->getVectorizedPredicate()[0]
                        : nullptr;

  // Get the vector mask value of the branch condition
  auto VecCondMask =
      State.ILV->getVectorValue(ConditionRecipe->getConditionValue());

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

void VPEdgePredicateRecipe::vectorize(VPTransformState &State) {
  // This recipe does not produce any code. It propagates an already
  // calculated mask value to CG.
  Value *PredMask = PredecessorPredicate
                        ? PredecessorPredicate->getVectorizedPredicate()[0]
                        : nullptr;
  State.ILV->setEdgeMask(FromBB, ToBB, PredMask);
}

void VPEdgePredicateRecipe::print(raw_ostream &O) const {
  O << Name;
  O << " = ";
  if (PredecessorPredicate)
    O << PredecessorPredicate->getName();
  else
    O << "NULL";
}

void VPIfTruePredicateRecipe::print(raw_ostream &O) const {
  O << Name;
  O << " = ";
  if (PredecessorPredicate) {
    O << PredecessorPredicate->getName() << " && ";
  }

  O << ConditionRecipe->getName();
}

void VPIfFalsePredicateRecipe::vectorize(VPTransformState &State) {
  Value *PredMask = PredecessorPredicate
                        ? PredecessorPredicate->getVectorizedPredicate()[0]
                        : nullptr;

  // Get the vector mask value of the branch condition - since this
  // edge is taken if the mask value is false we compute the negation
  // of this mask value.
  auto VecCondMask =
      State.ILV->getVectorValue(ConditionRecipe->getConditionValue());
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

void VPIfFalsePredicateRecipe::print(raw_ostream &O) const {
  O << Name;
  O << " = ";
  if (PredecessorPredicate) {
    O << PredecessorPredicate->getName() << " && ";
  }
  
  O << "!" <<  ConditionRecipe->getName();
}

void VPBooleanRecipe::print(raw_ostream &O) const {
  O << Name << " ";
  if (ConditionValue) {
    O << *ConditionValue;
  } else {
    O << "NULL";
  }
}

void VPVectorizeBooleanRecipe::vectorize(VPTransformState &State) {
  State.ILV->vectorizeInstruction(cast<Instruction>(ConditionValue));
}
