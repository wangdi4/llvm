
#include "IntelVPlan.h"
#include "LoopVectorizationCodeGen.h"

using namespace llvm;
using namespace llvm::vpo;

// Replicated from class Loop with minor changes (successors/predecessors iterators)
// TODO: Would it possible to move this to LoopBase?
void VPLoop::getUniqueExitBlocks(
    SmallVectorImpl<VPBlockBase *> &ExitBlocks) const {
  //TODO: This method is in class Loop.
  //assert(hasDedicatedExits() &&
  //       "getUniqueExitBlocks assumes the loop has canonical form exits!");

  SmallVector<VPBlockBase *, 32> SwitchExitBlocks;
  for (VPBlockBase *BB : this->blocks()) {
    SwitchExitBlocks.clear();
    for (VPBlockBase *Successor : BB->getSuccessors()) {
      // If block is inside the loop then it is not an exit block.
      if (contains(Successor))
        continue;

      auto PI = Successor->getPredecessors().begin();
      VPBlockBase *FirstPred = *PI;

      // If current basic block is this exit block's first predecessor
      // then only insert exit block in to the output ExitBlocks vector.
      // This ensures that same exit block is not inserted twice into
      // ExitBlocks vector.
      if (BB != FirstPred)
        continue;

      // If a terminator has more then two successors, for example SwitchInst,
      // then it is possible that there are multiple edges from current block
      // to one exit block.
      if (std::distance(BB->getSuccessors().begin(),
                        BB->getSuccessors().end()) <= 2) {
        ExitBlocks.push_back(Successor);
        continue;
      }

      // In case of multiple edges from current block to exit block, collect
      // only one edge in ExitBlocks. Use switchExitBlocks to keep track of
      // duplicate edges.
      if (!is_contained(SwitchExitBlocks, Successor)) {
        SwitchExitBlocks.push_back(Successor);
        ExitBlocks.push_back(Successor);
      }
    }
  }
}

VPBlockBase *VPLoop::getUniqueExitBlock() const {
  SmallVector<VPBlockBase *, 8> UniqueExitBlocks;
  getUniqueExitBlocks(UniqueExitBlocks);
  if (UniqueExitBlocks.size() == 1)
    return UniqueExitBlocks[0];
  return nullptr;
}

//Replicated from LoopVectorize.cpp

/// VPVectorizeOneByOneRecipe is a VPOneByOneRecipeBase which transforms by
/// vectorizing each Instruction in itsingredients independently, in order.
/// This recipe covers most of the traditional vectorization cases where
/// each ingredient produces  a vectorized version of itself.
class VPVectorizeOneByOneRecipe : public VPOneByOneRecipeBase {
  friend class VPlanUtilsLoopVectorizer;

private:
  /// Do the actual code generation for a single instruction.
  void transformIRInstruction(Instruction *I, VPTransformState &State) override;

public:
  VPVectorizeOneByOneRecipe(const BasicBlock::iterator B,
                            const BasicBlock::iterator E, VPlan *Plan)
      : VPOneByOneRecipeBase(VPVectorizeOneByOneSC, B, E, Plan) {}

  ~VPVectorizeOneByOneRecipe() {}

  /// Method to support type inquiry through isa, cast, and dyn_cast.
  static inline bool classof(const VPRecipeBase *V) {
    return V->getVPRecipeID() == VPRecipeBase::VPVectorizeOneByOneSC;
  }

  /// Print the recipe.
  void print(raw_ostream &O) const override {
    O << "Vectorize:";
    for (auto It = Begin; It != End; ++It) {
      O << '\n' << *It;
      if (willAlsoPackOrUnpack(&*It))
        O << " (S->V)";
    }
  }
};

void VPVectorizeOneByOneRecipe::transformIRInstruction(
    Instruction *I, VPTransformState &State) {
  assert(I && "No instruction to vectorize.");
  State.ILV->vectorizeInstruction(I);
  // if (willAlsoPackOrUnpack(I)) { // Unpack instruction
  //  for (unsigned Part = 0; Part < State.UF; ++Part)
  //    for (unsigned Lane = 0; Lane < State.VF; ++Lane)
  //      State.ILV->getScalarValue(I, Part, Lane);
  // }
}

class VPVectorizeOneByOneIRRecipe : public VPOneByOneIRRecipeBase {
  friend class VPlanUtilsLoopVectorizer;

private:
  /// Do the actual code generation for a single instruction.
  void transformIRInstruction(Instruction *I, VPTransformState &State) override;

public:
  VPVectorizeOneByOneIRRecipe(const BasicBlock::iterator B,
                              const BasicBlock::iterator E, VPlan *Plan)
      : VPOneByOneIRRecipeBase(VPVectorizeOneByOneSC, B, E, Plan) {}

  ~VPVectorizeOneByOneIRRecipe() {}

  /// Method to support type inquiry through isa, cast, and dyn_cast.
  static inline bool classof(const VPRecipeBase *V) {
    return V->getVPRecipeID() == VPRecipeBase::VPVectorizeOneByOneSC;
  }

  /// Print the recipe.
  void print(raw_ostream &O) const override {
    auto It = begin();
    auto End = end();

    O << "Vectorize VPInstIR:";
    for (; It != End; ++It) {
      auto Inst = cast<VPInstructionIR>(&*It);
      auto IRInst = Inst->getInstruction();
      O << '\n' << *IRInst;
      if (willAlsoPackOrUnpack(IRInst))
        O << " (S->V)";
    }
  }
};

void VPVectorizeOneByOneIRRecipe::transformIRInstruction(
    Instruction *I, VPTransformState &State) {
  assert(I && "No instruction to vectorize.");
  State.ILV->vectorizeInstruction(I);
  // if (willAlsoPackOrUnpack(I)) { // Unpack instruction
  //  for (unsigned Part = 0; Part < State.UF; ++Part)
  //    for (unsigned Lane = 0; Lane < State.VF; ++Lane)
  //      State.ILV->getScalarValue(I, Part, Lane);
  // }
}

VPOneByOneIRRecipeBase *
IntelVPlanUtils::createOneByOneRecipe(const BasicBlock::iterator B,
                                      const BasicBlock::iterator E,
                                      // VPlan *Plan,
                                      bool isScalarizing) {
  // TODO
  // if (isScalarizing)
  //  return new VPScalarizeOneByOneRecipe(B, E, Plan);
  return new VPVectorizeOneByOneIRRecipe(B, E, Plan);
}

template void llvm::Calculate<VPRegionBlock, VPBlockBase *>(
    DominatorTreeBase<GraphTraits<VPBlockBase *>::NodeType> &DT,
    VPRegionBlock &VPR);
//template void llvm::Calculate<VPRegionBlock, Inverse<VPBlockBase *>>(
//    DominatorTreeBase<GraphTraits<Inverse<VPBlockBase *>>::NodeType> &DT,
//    VPRegionBlock &VPR);

void VPUniformConditionBitRecipe::vectorize(VPTransformState &State) {
  if (isa<Instruction>(ConditionBit)) {
    State.ILV->serializeInstruction(cast<Instruction>(ScConditionBit));
    ConditionBit = State.ILV->getScalarValue(ScConditionBit, 0);
  }
}
