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
    if (DA) {
      if (DA->isDivergent(Inst))
        DA->markDivergent(*ClonedInst);
      DA->updateVectorShape(ClonedInst, DA->getVectorShape(&Inst));
    }
  }

  ValueMap.insert({Block, ClonedBlock});
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
VPBasicBlock *VPCloneUtils::cloneBlocksRange(VPBasicBlock *Begin,
                                             VPBasicBlock *End,
                                             Value2ValueMapTy &ValueMap,
                                             VPlanDivergenceAnalysis *DA,
                                             const Twine &Prefix) {

  for (auto *BB : sese_depth_first(Begin, End))
    cloneBasicBlock(BB, Prefix.str(), ValueMap, ++End->getIterator(), DA);

  // Remap successors *inside* SESE region. Once CFG is represented through
  // terminator VPInstruction that won't be needed here and would be done as
  // part of ordinary remap.
  //
  // Can't iterate over BlockMap directly because the order won't be stable
  // resulting in unstable predecessors order.
  for (auto *BB : sese_depth_first(Begin, End)) {
    // Skip last basic block.
    if (BB == End)
      continue;
    auto *Clone = cast<VPBasicBlock>(ValueMap[BB]);
    for (VPBasicBlock *OrigSucc : BB->getSuccessors()) {
      auto *CloneSucc = cast<VPBasicBlock>(ValueMap[OrigSucc]);
      Clone->appendSuccessor(CloneSucc);
    }
  }

  return cast<VPBasicBlock>(ValueMap[Begin]);
}

} // namespace vpo
} // namespace llvm
