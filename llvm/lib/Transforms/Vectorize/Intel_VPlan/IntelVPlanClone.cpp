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
                                            VPlan::iterator InsertBefore,
                                            VPlanDivergenceAnalysis *DA) {
  Prefix = Prefix == "" ? "cloned." : Prefix;
  std::string Name = VPlanUtils::createUniqueName((Prefix + Block->getName()));
  VPBasicBlock *ClonedBlock = new VPBasicBlock(Prefix);
  ClonedBlock->setName(Name);
  Block->getParent()->insertBefore(ClonedBlock, InsertBefore);

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

  auto Iter = df_begin(Begin);
  auto EndIter = df_end(Begin);
  while (Iter != EndIter) {
    cloneBasicBlock(*Iter, Prefix.str(), BlockMap, ValueMap,
                    ++End->getIterator(), DA);
    if (*Iter == End) {
      // Don't go outside of SESE region. It does move the iterator, so avoid
      // usual increment.
      Iter.skipChildren();
      continue;
    }
    // Go to the next block in ordinary way.
    ++Iter;
  }

  // Remap successors *inside* SESE region. Once CFG is represented through
  // terminator VPInstruction that won't be needed here and would be done as
  // part of ordinary remap.
  //
  // Can't iterate over BlockMap directly because the order won't be stable
  // resulting in unstable predecessors order.
  Iter = df_begin(Begin);
  while (Iter != EndIter) {
    if (*Iter == End){
      // Don't remap successor of the last block.
      Iter.skipChildren();
      continue;
    }

    VPBasicBlock *Orig = *Iter;
    VPBasicBlock *Clone = BlockMap[Orig];
    for (VPBasicBlock *OrigSucc : Orig->getSuccessors()) {
      VPBasicBlock *CloneSucc = BlockMap[OrigSucc];
      Clone->appendSuccessor(CloneSucc);
      CloneSucc->appendPredecessor(Clone);
    }
    ++Iter;
  }

  return BlockMap[Begin];
}

} // namespace vpo
} // namespace llvm
