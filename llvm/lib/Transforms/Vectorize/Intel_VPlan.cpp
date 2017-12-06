//===- Intel_VPlan.cpp - Vectorizer Plan ----------------------------------===//
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

#include "Intel_VPlan.h"
#include "./Intel_VPlan/LoopVectorizationCodeGen.h"
#include "llvm/ADT/PostOrderIterator.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Dominators.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/GraphWriter.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"

#define DEBUG_TYPE "vplan"

namespace llvm {
namespace vpo {

unsigned VPlanUtils::NextOrdinal = 1;

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

#if INTEL_CUSTOMIZATION
void VPBlockBase::setCondBitVPVal(VPValue *CV, VPlan *Plan) {
  CondBitVPVal = CV;
  if (CV)
    Plan->setCondBitVPValUser(CV, this);
}
#else
void VPBlockBase::setConditionBitRecipe(VPConditionBitRecipeBase *R,
                                        VPlan *Plan) {
  ConditionBitRecipe = R;
  if (R)
    Plan->setConditionBitRecipeUser(R, this);
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
  DEBUG(dbgs() << "LV: created " << NewBB->getName() << '\n');

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
        VPValue *CBV = PredVPBlock->getCondBitVPVal();
        assert(CBV && "Expected CondBitVPVal");
        Value *Bit = nullptr;
        if (State->CBVToConditionBitMap.count(CBV)) {
          Bit = State->CBVToConditionBitMap[CBV];
        } else {
          Bit = CBV->getValue();
        }
        assert(Bit && "Cannot create conditional branch with empty bit.");
        assert(!Bit->getType()->isVectorTy() && "Should be 1-bit scalar");
        BranchInst::Create(FirstSuccBB, SecondSuccBB, Bit, PredBB);
      }
    }
  }
  return NewBB;
}

#if !INTEL_CUSTOMIZATION
#if INTEL_CUSTOMIZATION
// Eventually, we will remove the ifndef macro above, when we use separate
// libraries for opensource VPlan and VPO VPlan. In the meantime, this function
// cannot be shared by both implementations. This implementation is for
// opensource VPlan, currently disabled in XMAIN. Implementation for VPO is in
// IntelVPlan.cpp
#endif
void VPBasicBlock::execute(VPTransformState *State) {
  VPIteration *I = State->Instance;
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
    Recipe.execute(*State);

  DEBUG(dbgs() << "LV: filled BB:" << *NewBB);
}
#endif // INTEL_CUSTOMIZATION

#if INTEL_CUSTOMIZATION
void VPBasicBlock::moveConditionalEOBTo(VPBasicBlock *ToBB, VPlan *Plan) {
  // Set CondBitVPVal in NewBlock. Note that we are only setting the
  // successor selector pointer. The CondBitVPVal is kept in its
  // original VPBB recipe list.
  if (getNumSuccessors() > 1) {
    assert(getCondBitVPVal() && "Missing CondBitVPVal");
    ToBB->setCondBitVPVal(getCondBitVPVal(), Plan);
    ToBB->setCBlock(CBlock);
    ToBB->setTBlock(TBlock);
    ToBB->setFBlock(FBlock);
    setCondBitVPVal(nullptr, Plan);
    CBlock = TBlock = FBlock = nullptr;
  }
}

void VPRegionBlock::recomputeSize() {
  Size = std::distance(df_iterator<const VPBlockBase *>::begin(Entry),
                       df_iterator<const VPBlockBase *>::end(Exit));
}
#endif

void VPRegionBlock::execute(VPTransformState *State) {
  ReversePostOrderTraversal<VPBlockBase *> RPOT(Entry);

  if (!isReplicator()) {
    // Visit the VPBlocks connected to "this", starting from it.
    for (VPBlockBase *Block : RPOT) {
      DEBUG(dbgs() << "LV: VPBlock in RPO " << Block->getName() << '\n');
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
        DEBUG(dbgs() << "LV: VPBlock in RPO " << Block->getName() << '\n');
        Block->execute(State);
      }
    }
  }

  // Exit replicating mode.
  State->Instance.reset();
}

void VPInstruction::generateInstruction(VPTransformState &State,
                                        unsigned Part) {
#if INTEL_CUSTOMIZATION
  assert(Inst && "There is no underlying Instruction.");
  State.ILV->vectorizeInstruction(Inst);
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

void VPInstruction::execute(VPTransformState &State) {
  // TODO: Remove this block of code. Its purpose is to emulate the execute()
  //       of the conditionbit recipies that have now been removed.
  if (State.UniformCBVs->count(this)) {
    Value *ScConditionBit = getValue();
    State.ILV->serializeInstruction(cast<Instruction>(ScConditionBit));
    Value *ConditionBit = State.ILV->getScalarValue(ScConditionBit, 0);
    assert(!ConditionBit->getType()->isVectorTy() && "Bit should be scalar");
    State.CBVToConditionBitMap[this] = ConditionBit;
    return;
  }

  assert(!State.Instance && "VPInstruction executing an Instance");
  for (unsigned Part = 0; Part < State.UF; ++Part)
    generateInstruction(State, Part);
}

void VPInstruction::print(raw_ostream &O, const Twine &Indent) const {
  O << " +\n" << Indent << "\"EMIT ";
  print(O);
  O << "\\l\"";
}

void VPInstruction::print(raw_ostream &O) const {
  printAsOperand(O);
  O << " = ";

  switch (getOpcode()) {
  case VPInstruction::Not:
    O << "not";
    break;
#if INTEL_CUSTOMIZATION
  case VPInstruction::SemiPhi:
    O << "semi-phi";
    break;
#endif
  default:
    O << Instruction::getOpcodeName(getOpcode());
  }

  for (const VPValue *Operand : operands()) {
    O << " ";
    Operand->printAsOperand(O);
  }
}

/// Generate the code inside the body of the vectorized loop. Assumes a single
/// LoopVectorBody basic block was created for this; introduces additional
/// basic blocks as needed, and fills them all.
void VPlan::execute(VPTransformState *State) {
#if INTEL_CUSTOMIZATION
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
    FromBB->getTerminator()->eraseFromParent();
    Value *Bit = FromVPBB->getCondBitVPVal()->getValue();
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
  (void)merged;
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

void VPlanPrinter::dump() {
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
  const VPValue *CBV = BasicBlock->getCondBitVPVal();
  // Dump the CondBitVPVal
  if (CBV) {
    OS << " +\n"
       << Indent << "\" ---- CondBitVPVal ---- \\l\" ";
    OS << " +\n" << Indent << "\" ";
    if (const VPInstruction *CBI = dyn_cast<VPInstruction>(CBV)) {
      CBI->print(OS);
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

void VPlan::printInst2Recipe() {
  DenseMap<Instruction *, VPRecipeBase *>::iterator It, End;
  for (It = Inst2Recipe.begin(), End = Inst2Recipe.end(); It != End; ++It) {
    DEBUG(errs() << "Instruction: " << *It->first << "\n");
    std::string RecipeString;
    raw_string_ostream RSO(RecipeString);
    VPRecipeBase *Recipe = It->second;
    Recipe->print(RSO, Twine()); // TODO: Twine
    DEBUG(errs() << "Recipe: " << RSO.str() << "\n");
  }
}

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

void VPIfTruePredicateRecipe::execute(VPTransformState &State) {
  Value *PredMask = PredecessorPredicate
                        ? PredecessorPredicate->getVectorizedPredicate()[0]
                        : nullptr;

  // Get the vector mask value of the branch condition
  auto VecCondMask = State.ILV->getVectorValue(ConditionValue->getValue());

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

void VPIfFalsePredicateRecipe::execute(VPTransformState &State) {
  Value *PredMask = PredecessorPredicate
                        ? PredecessorPredicate->getVectorizedPredicate()[0]
                        : nullptr;

  // Get the vector mask value of the branch condition - since this
  // edge is taken if the mask value is false we compute the negation
  // of this mask value.
  auto VecCondMask = State.ILV->getVectorValue(ConditionValue->getValue());
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
} // namespace vpo
} // namespace llvm
