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
#include "IntelVPBasicBlock.h"
#include "IntelVPlanUtils.h"

namespace llvm {
namespace vpo {

/// Remap all operands from cloned \p Inst instruction.
void VPValueMapper::remapInstruction(VPInstruction *Inst) {
  for (unsigned I = 0, E = Inst->getNumOperands(); I != E; ++I) {
    Inst->setOperand(I, remapValue(Value2ValueMap, Inst->getOperand(I)));
  }

  if (auto Phi = dyn_cast<VPPHINode>(Inst)) {
    for (auto &B : Phi->blocks()) {
      B = cast<VPBasicBlock>(remapValue(Value2ValueMap, B));
    }
  }
}

void VPValueMapper::remapOperands(
    VPBasicBlock *OrigVPBB, std::function<void(VPInstruction &)> UpdateFunc) {
  auto *ClonedVPBB = cast<VPBasicBlock>(Value2ValueMap[OrigVPBB]);

  for (VPInstruction &Inst : *ClonedVPBB) {
    remapInstruction(&Inst);
    UpdateFunc(Inst);
  }

  // Fix cond predicate.
  if (VPInstruction *BlockPredicate = OrigVPBB->getBlockPredicate())
    ClonedVPBB->setBlockPredicate(
        cast<VPInstruction>(remapValue(Value2ValueMap, BlockPredicate)));
}

/// Clone \p Block and its instructions.
/// Remapping happens later by VPValueMapper which must be called by user.
VPBasicBlock *VPCloneUtils::cloneBasicBlock(VPBasicBlock *Block,
                                            const Twine &Prefix,
                                            Value2ValueMapTy &ValueMap,
                                            VPlan::iterator InsertBefore,
                                            VPlanDivergenceAnalysis *DA) {
  VPBasicBlock *ClonedBlock = new VPBasicBlock(Prefix, Block->getParent());
  std::string Name = VPlanUtils::createUniqueName((Prefix + Block->getName()));
  ClonedBlock->setName(Name);
  Block->getParent()->insertBefore(ClonedBlock, InsertBefore);

  for (auto &Inst : *Block) {
    auto ClonedInst = Inst.clone();
    ClonedBlock->appendInstruction(ClonedInst);
    ValueMap.insert(std::make_pair(&Inst, ClonedInst));
    if (DA)
      DA->updateVectorShape(ClonedInst, DA->getVectorShape(&Inst));
  }

  // Remove unnecessary terminator added by VPBasicBlock constructor
  // FIXME: this is a temporary workaround which should be removed
  ClonedBlock->removeInstruction(ClonedBlock->getTerminator());

  ValueMap.insert({Block, ClonedBlock});

  return ClonedBlock;
}

/// Clone given blocks from Begin to End
/// Remapping happens later by VPValueMapper which must be called by user.
VPBasicBlock *VPCloneUtils::cloneBlocksRange(VPBasicBlock *Begin,
                                             VPBasicBlock *End,
                                             Value2ValueMapTy &ValueMap,
                                             VPlanDivergenceAnalysis *DA,
                                             const Twine &Prefix) {
  for (auto *BB : sese_depth_first(Begin, End))
    cloneBasicBlock(BB, Prefix.str(), ValueMap, ++End->getIterator(), DA);

  return cast<VPBasicBlock>(ValueMap[Begin]);
}

} // namespace vpo
} // namespace llvm
