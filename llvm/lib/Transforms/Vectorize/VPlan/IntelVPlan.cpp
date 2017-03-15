
#include "IntelVPlan.h"
#include "LoopVectorizationCodeGen.h"

using namespace llvm;
using namespace llvm::vpo;

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
  if (isa<Instruction>(ScConditionBit)) {
    State.ILV->serializeInstruction(cast<Instruction>(ScConditionBit));
    ConditionBit = State.ILV->getScalarValue(ScConditionBit, 0);
  }
}
