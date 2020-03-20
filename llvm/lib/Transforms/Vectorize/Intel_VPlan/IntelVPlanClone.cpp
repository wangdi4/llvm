//===------------------- IntelVPlanClone.cpp - Cloning functions for VPlan -==//
//
// Copyright (C) 2019 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//

#include "IntelVPlanClone.h"

namespace llvm {
namespace vpo {

/// Remap all operands from cloned \p Inst instruction.
void VPValueMapper::remapInstruction(VPInstruction *Inst) {
  for (unsigned I = 0, E = Inst->getNumOperands(); I != E; ++I) {
    Inst->setOperand(I, remapValue(Value2ValueMap, Inst->getOperand(I)));
  }

  if (auto Phi = dyn_cast<VPPHINode>(Inst)) {
    for (auto &B : Phi->blocks()) {
      B = remapValue(Block2BlockMap, B);
    }
  }
}

/// Clone \p Block and its instructions.
/// Remapping happens later by VPValueMapper which must be called by user.
VPBasicBlock *VPCloneUtils::cloneBasicBlock(VPBasicBlock *Block,
                                            std::string Prefix,
                                            Block2BlockMapTy &BlockMap,
                                            Value2ValueMapTy &ValueMap,
                                            VPlanDivergenceAnalysis *DA) {
  Prefix = Prefix == "" ? "cloned." : Prefix;
  std::string Name = VPlanUtils::createUniqueName((Prefix + Block->getName()));
  VPBasicBlock *ClonedBlock = new VPBasicBlock(Prefix);
  ClonedBlock->setName(Name);
  VPlan *Plan = Block->getParent();
  ClonedBlock->setParent(Plan);
  Plan->setSize(Plan->getSize() + 1);

  for (auto &Inst : *Block) {
    auto ClonedInst = Inst.clone();
    ClonedBlock->appendInstruction(ClonedInst);
    ValueMap.insert(std::make_pair(&Inst, ClonedInst));
    if (DA) {
      if (DA->isDivergent(Inst))
        DA->markDivergent(*ClonedInst);
      DA->updateVectorShape(ClonedInst, DA->getVectorShape(&Inst));
    }
  }

  BlockMap.insert({Block, ClonedBlock});
  if (auto CondBit = Block->getCondBit()) {
    VPValue *ClonedCondBit = CondBit;
    if (auto CondBitInst = dyn_cast<VPInstruction>(CondBit)) {
      ClonedCondBit = CondBitInst->clone();
      // Parent of the cloned condition bit will be updated later by
      // VPValueMapper.
      cast<VPInstruction>(ClonedCondBit)->Parent = CondBitInst->getParent();
    }
    ClonedBlock->setCondBit(ClonedCondBit);
    ValueMap.insert({CondBit, ClonedCondBit});
  }

  return ClonedBlock;
}

/// Clone given blocks from Begin to End
/// Remapping happens later by VPValueMapper which must be called by user.
VPBasicBlock *VPCloneUtils::cloneBlocksRange(
    VPBasicBlock *Begin, VPBasicBlock *End, Block2BlockMapTy &BlockMap,
    Value2ValueMapTy &ValueMap, VPlanDivergenceAnalysis *DA, Twine Prefix) {
  if (Prefix.isTriviallyEmpty())
    Prefix.concat("cloned.");

  bool EndReached = false;
  std::function<VPBasicBlock *(VPBasicBlock *)> Cloning =
      [&](VPBasicBlock *Block) -> VPBasicBlock * {
    auto It = BlockMap.find(Block);
    if (It != BlockMap.end())
      return It->second;

    // Do not clone any new blocks if the end block was reached and we are
    // finalizing the cloning process.
    if (EndReached)
      return nullptr;

    VPBasicBlock *ClonedBlock =
        cloneBasicBlock(Block, Prefix.str(), BlockMap, ValueMap, DA);
    BlockMap.insert({Block, ClonedBlock});

    if (Block == End)
      EndReached = true;

    for (auto &Succ : Block->getSuccessors())
      if (VPBasicBlock *ClonedSucc = Cloning(Succ)) {
        ClonedBlock->appendSuccessor(ClonedSucc);
        ClonedSucc->appendPredecessor(ClonedBlock);
      }

    return ClonedBlock;
  };

  VPBasicBlock *Clone = Cloning(Begin);
  assert(EndReached && "End block expected to be reached");
  return Clone;
}

} // namespace vpo
} // namespace llvm
