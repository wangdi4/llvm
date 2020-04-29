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

/// Clone \p Block and its instructions for the right VPlan (current VPlan or
/// New VPlan). Remapping happens later by VPValueMapper which must be called by
/// user.
VPBasicBlock *VPCloneUtils::cloneBasicBlock(VPBasicBlock *Block,
                                            const Twine &Prefix,
                                            Value2ValueMapTy &ValueMap,
                                            VPlan::iterator InsertBefore,
                                            VPlanDivergenceAnalysis *DA,
                                            VPlan *NewVPlan) {

  // The following makes sure that the cloned basic block is assigned to the
  // correct VPlan. In loop unrolling, the basic block is cloned inside the same
  // VPlan. Hence, the Parent is the current VPlan. In peel/remainder, the
  // cloned basic block is assigned to the new VPlan. Thus, the Parent is the
  // NewVPlan.
  VPlan *Parent = NewVPlan ? NewVPlan : Block->getParent();
  // InsertBefore indicates where the cloned basic block should be placed inside
  // the ilist. In case of loop unrolling, the new basic block is inserted
  // before the basic block that is indicated by InsertBefore. In case of
  // peel/remainder, the new basic block is appended at the back of NewVPlan's
  // ilist.
  InsertBefore = NewVPlan ? NewVPlan->end() : InsertBefore;

  VPBasicBlock *ClonedBlock = new VPBasicBlock(Prefix, Parent);
  std::string Name = VPlanUtils::createUniqueName((Prefix + Block->getName()));
  ClonedBlock->setName(Name);
  Parent->insertBefore(ClonedBlock, InsertBefore);

  for (auto &Inst : *Block) {
    auto ClonedInst = Inst.clone();
    ClonedBlock->insert(ClonedInst, ClonedBlock->end());
    ValueMap.insert(std::make_pair(&Inst, ClonedInst));
    if (DA)
      DA->updateVectorShape(ClonedInst, DA->getVectorShape(&Inst));
  }

  ValueMap.insert({Block, ClonedBlock});

  return ClonedBlock;
}

/// Clone given blocks from Begin to End
/// Remapping happens later by VPValueMapper which must be called by user.
VPBasicBlock *VPCloneUtils::cloneBlocksRange(
    VPBasicBlock *Begin, VPBasicBlock *End, Value2ValueMapTy &ValueMap,
    VPlanDivergenceAnalysis *DA, const Twine &Prefix, VPlan *NewVPlan) {
  for (auto *BB : sese_depth_first(Begin, End))
    cloneBasicBlock(BB, Prefix.str(), ValueMap, ++End->getIterator(), DA,
                    NewVPlan);

  return cast<VPBasicBlock>(ValueMap[Begin]);
}

} // namespace vpo
} // namespace llvm
