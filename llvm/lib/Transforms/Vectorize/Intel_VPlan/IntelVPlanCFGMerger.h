//===-- IntelVPlanCFGMerger.h -----------------------------------*- C++ -*-===//
//
//   Copyright (C) 2021 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation. and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file defines the VPlanCFGMerger class that is used to create
/// auxiliary loops (peel/remainder) and merge them into one flattened CFG.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTELVPLANCFGMERGER_H
#define LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTELVPLANCFGMERGER_H

#include "IntelVPlan.h"

namespace llvm {
class Loop;

namespace vpo {

// Utility class to merge CFG of VPlan.
class VPlanCFGMerger {
  VPlanVector &Plan;
  VPExternalValues &ExtVals;
  VPVectorTripCountCalculation * VectorTripCount = nullptr;
  unsigned VF;
  unsigned UF;

public:
  VPlanCFGMerger(VPlanVector &P, unsigned V, unsigned U)
      : Plan(P), ExtVals(P.getExternals()), VF(V), UF(U) {}

  //
  // Create a simple chain of vectorized loop and scalar remainder.
  // The following structure of VPBasicBlocks is created, while on input we have
  // VectorPreheader-LoopBody-ExitBB.
  //
  //           VectorTopTest
  //              /     \
  //             /    VectorPreheader
  //            /        |
  //           +      LoopBody
  //           |         |
  //           |      ExitBB
  //           |         |
  //           |      RemainderTopTest
  //           |     /   |
  //           |    /    |
  //      RemainderStart |
  //           |         |
  //       RemainderLoop +
  //              \     /
  //              FinalBB
  //
  void createSimpleVectorRemainderChain(Loop *OrigLoop);

private:
  // Create a new merge block (i.e. VPBasicBlock with VPPHINodes which have
  // non-undefined MergeId, see VPExternalUse::UndefMergeId) inserting it after
  // \p InsertAfter. If \p SplitBlock is not passed then no operands of merge
  // nodes are added, otherwise either incoming or outgoing values of VPlan,
  // depending on \p UseLiveIn, are added as incoming from \p SplitBlock to the
  // merge nodes. As an example of newly created block you can see FinalBB on
  // the diagram above, InsertAfter is equal to RemainderLoop and SplitBlock is
  // RemainderTopTest on the diagram.
  VPBasicBlock *createMergeBlock(VPBasicBlock *InsertAfter,
                                 VPBasicBlock *SplitBlock = nullptr,
                                 bool UseLiveIn = true);

  // Update merge nodes in \p MergeBlock with values coming from \p SplitBlock.
  // \p UseLiveIn define which values to use.
  void updateMergeBlockIncomings(VPBasicBlock *MergeBlock,
                                 VPBasicBlock *SplitBlock, bool UseLiveIn);

  // Create scalar remainder for original loop \p OrigLoop, inserting it after
  // the VPBasicBlock \p InsertAfter. The \p InsertAfter is supposed to be a
  // merge block (i.e. containing special VPPHINodes).
  // A block with VPReuseLoop and VPInstructions to get scalar liveouts is
  // created and linked as successor to InsertAfter block. VPReuseLoop input
  // values are linked to VPPHINodes from InsertAfter block, using VPlan scalar
  // in/out list. \p FinalBB is the block in VPlan which should be used to jump
  // from the last basic block of the loop (see FinalBB on the diagram above).
  // Returns the block created.
  VPBasicBlock *createScalarRemainder(Loop *OrigLoop, VPBasicBlock *InsertAfter,
                                      VPBasicBlock *FinalBB);

  // Create a VPBasicBlock with top test for VPlan loop. It's supposed that
  // VPVectorTripCountCalculation and VPOrigTripCountCalculation are inserted in
  // the Plan loop preheader. A new block is created, the trip count
  // instructions are moved to the new block and the top test check is created,
  // with two successors, loop preheader and \p FallThroughMergeBlock.
  VPBasicBlock *createVPlanLoopTopTest(VPBasicBlock *FallThroughMergeBlock);

  // Create a VPBasicBlock with a check for remaining iterations count after
  // vector loop. It's supposed that VPVectorTripCountCalculation and
  // VPOrigTripCountCalculation are inserted in a block before the \p Plan loop
  // preheader. There is created a new block and the check for the vector TC is
  // equal to the original TC is created with two successors, remainder
  // preheader and final merge block.
  VPBasicBlock *createRemainderTopTest(VPBasicBlock *InsertAfter,
                                       VPBasicBlock *RemPreheader,
                                       VPBasicBlock *FinalMergeBlock);

  // The \p InBlock is supposed to contain VPOrigLiveOut instructions.
  // \p BB is a merge block, and its VPPHINodes are updated with
  // incoming pairs {VPOrigValue, InBlock}.
  void updateMergeBlockByScalarLiveOuts(
    VPBasicBlock *BB, VPBasicBlock *InBlock);

  // Set operands of external uses to merge values from the \p FinalBB.
  void updateExternalUsesOperands(VPBasicBlock *FinalBB);

  // Find first non-empty VPBasicBlock in VPlan, starting from the entry.
  VPBasicBlock *findFirstNonEmptyBB() const;

  // Try to find vector trip count instruction in the chain of predecessors
  // starting from basic block \p StartBB.
  VPVectorTripCountCalculation *findVectorTCInst(VPBasicBlock *StartBB);
};

} // namespace vpo
} // namespace llvm

#endif // LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTELVPLANCFGMERGER_H
