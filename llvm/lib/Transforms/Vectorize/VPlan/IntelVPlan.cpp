
#include "IntelVPlan.h"
#include "LoopVectorizationCodeGen.h"

using namespace llvm;
using namespace llvm::vpo;

//Replicated from LoopVectorize.cpp

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

VPBranchIfNotAllZeroRecipe *
IntelVPlanUtils::createBranchIfNotAllZeroRecipe(Instruction *Cond) {
  return new VPBranchIfNotAllZeroRecipe(Cond, Plan);
}

VPMaskGenerationRecipe *
IntelVPlanUtils::createMaskGenerationRecipe(const Value *Pred,
                                            const Value *Backedge) {
  return new VPMaskGenerationRecipe(Pred, Backedge);
}

VPNonUniformConditionBitRecipe *
IntelVPlanUtils::createNonUniformConditionBitRecipe(
  const VPMaskGenerationRecipe *MaskRecipe) {
  return new VPNonUniformConditionBitRecipe(MaskRecipe);
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
