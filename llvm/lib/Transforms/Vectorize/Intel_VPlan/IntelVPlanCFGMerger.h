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
extern bool EmitPushPopVF;

class SingleLoopVecScenario;
class LoopVectorizationPlanner;
class CfgMergerPlanDescr;

// Utility class to merge CFG of VPlan.
class VPlanCFGMerger {
  VPlanVector &Plan;
  VPExternalValues &ExtVals;
  // Cache for VPVectorTripCountCalculation* instructions by VPlans. Is
  // implicitly used by findVectorUB()
  mutable DenseMap<VPlanNonMasked*, VPVectorTripCountCalculation *> VectorUBs;
  unsigned MainVF;
  unsigned MainUF;

  // Instruction that contain PeelCount. It's assigned during VPlan CG for peel
  // count calculation and used in the follow up VPlan CG for various checks.
  VPValue *PeelCount = nullptr;

  // OrigUB is filled in at the beginning of VPlan CFG creation as it's used in
  // many trip count checks. It's taken from existing VPlan code.
  VPValue* OrigUB = nullptr;

public:
  VPlanCFGMerger(VPlanVector &P, unsigned V, unsigned U)
      : Plan(P), ExtVals(P.getExternals()), MainVF(V), MainUF(U) {}

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

  // Create a list of VPlan descriptors (along with the needed VPlans) by
  // vectorization scenario \p Scen.
  static void createPlans(LoopVectorizationPlanner &Planner,
                          const SingleLoopVecScenario &Scen,
                          std::list<CfgMergerPlanDescr> &Plans, Loop *OrigLoop,
                          VPlan &MainPlan,
                          VPAnalysesFactory &VPAF);

  // The first pass of the universal CFG merger. Creates merged CFG by scenario
  // \p Scen and the prepared list of VPlans \p Plans. It goes through the list
  // of VPlans, creates placeholder basic blocks for them, inserts merging
  // blocks, and emits the needed trip count checks.
  void createMergedCFG(SingleLoopVecScenario &Scen,
                       std::list<CfgMergerPlanDescr> &Plans);

private:
  using PlanDescr = CfgMergerPlanDescr;

  // Create a new merge block with merge phis inserting it after \p InsertAfter.
  // As an example of newly created block you can see FinalBB on the diagram
  // above, InsertAfter is equal to RemainderLoop and SplitBlock is
  // RemainderTopTest on the diagram.
  VPBasicBlock *createMergeBlock(VPBasicBlock *InsertAfter,
                                 VPBasicBlock *SplitBlock = nullptr,
                                 bool UseLiveIn = true);

  // Create merge phis (i.e. VPPHINodes which have MergeId not equal to
  // VPExternalUse::UndefMergeId). The phis are created for each liveout of
  // Plan. If \p SplitBlock is not passed then no operands of merge phis are
  // added, otherwise either incoming or outgoing values of Plan, depending on
  // \p UseLiveIn, are added as incoming from \p SplitBlock to the merge nodes.
  VPBasicBlock *createMergePhis(VPBasicBlock *MergeBlock,
                                VPBasicBlock *SplitBlock = nullptr,
                                bool UseLiveIn = true);

  // Create a merge block, with merge phis, and insert it before \p
  // InsertBefore.
  VPBasicBlock *createMergeBlockBefore(VPBasicBlock *InsertBefore);

  // Update merge nodes in \p MergeBlock with values coming from \p SplitBlock.
  // \p UseLiveIn define which values to use: either original incoming ones
  // or liveouts from VPlan.
  void updateMergeBlockIncomings(VPlan &P, VPBasicBlock *MergeBlock,
                                 VPBasicBlock *SplitBlock, bool UseLiveIn);

  // Update merge nodes in \p MergeBlock with values coming from \p SplitBlock.
  // \p UseLiveIn define which values to use: either original incoming ones
  // or liveouts from VPlan. In case \p Descr points to non-main VPlan the
  // incoming values are set to VPlanAdapter of this VPlan, to avoid having
  // cross VPlan use-def chain. The VPlanAdapter is replaced by real values
  // during further stages of VPlans merging.
  void updateMergeBlockIncomings(PlanDescr &Descr, VPBasicBlock *MergeBlock,
                                 VPBasicBlock *SplitBlock, bool UseLiveIn);

  // Create scalar remainder for original loop \p OrigLoop, inserting it after
  // the VPBasicBlock \p InsertAfter. The \p InsertAfter is supposed to be a
  // merge block (i.e. containing special VPPHINodes).
  // A block with VPScalarRemainder and VPInstructions to get scalar liveouts is
  // created and linked as successor to InsertAfter block. VPScalarRemainder
  // input values are linked to VPPHINodes from InsertAfter block, using VPlan
  // scalar in/out list. \p FinalBB is the block in VPlan which should be used
  // to jump from the last basic block of the loop (see FinalBB on the diagram
  // above). Returns the block created.
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
  // starting from basic block \p BB.
  VPVectorTripCountCalculation *findVectorTCInst(VPBasicBlock *BB) const;

  // Create VPBasicBlock with VPlanAdapter VPInstruction that encapsulates VPlan
  // described by \p Descr. The new VPBasicBlock is inserted in the parent
  // list before \p InsertBefore and linked as a predecessor to Succ. \p
  // InsertBefore and \p Succ can be different in some cases. E.g. for masked
  // mode loop when we have a scalar remainder, in this case we need to "jump
  // over" scalar remainder to the final merge block. The \p Descr is updated
  // setting its FirstBB and LastBB into the newly created block.
  void createAdapterBB(PlanDescr &Descr, VPBasicBlock *InsertBefore,
                                VPBasicBlock *Succ);

  // Find VPVectorTripCountCalculation instruction which calculates upper
  // bound for VPlan \p P. VPlan should be VPlanNonMasked.
  VPVectorTripCountCalculation *findVectorUB(VPlan &P) const;

  // Create a skeleton of CFG, adding in the main Plan placeholders for VPlans
  // from the passed list. Each VPlan is represented by a VPlanAdapter. The
  // sequence of basic blocks with VPlanAdpaters, merge blocks, and the needed
  // trip count checks are generated. See example of CFG in the *.cpp file.
  void emitSkeleton(std::list<PlanDescr> &Plans);

  // Create a check for whether the loop described by \p PrevDescr should be
  // executed after the loop described by \p Descr. I.e. the check for the upper
  // bound of the loop from \p Descr is not equal to the upper bound of the loop
  // from \p PrevDescr. If the upper bounds are not equal then the previous loop
  // has some iterations to execute. Here "previous" is about order of
  // processing, not about placement in CFG.
  void createTCCheckAfter(PlanDescr &Descr, PlanDescr &PrevDescr);

  // Create a top test, i.e. the sequence of instructions (placed in a separate
  // basic block)
  //    %c = icmp eq 0, <vector loop upper bound>
  //    br %c, label %SuccEq, label %SuccNe
  //
  // where:
  //  both Succ* are parameters of the routine, <vector loop upper bound> is
  //  the upper bound of the loop described by VecPlan.
  //
  // The generated top test block is inserted before \p InsertBefore.
  VPBasicBlock *createTopTest(VPlan *VecPlan, VPBasicBlock *InsertBefore,
                              VPBasicBlock *SuccEq, VPBasicBlock *SuccNe,
                              unsigned VF);

  // The routine creates one, two or none top test checks. The arguments
  // \p MainDescr and \p PRemDescr represent descriptors of main and
  // remainder loop (one following the main loop) respectively.
  // Each check (if generated) is inserted into a new block placed before
  // MainDescr.FirstBB.
  // The top test checks are generated by the following rules:
  // - if the main loop is masked mode or there is no remainder loop (i.e
  //   PRemDescr is null)
  //      then no checks are generated
  // - if the remainder loop is:
  //    * masked mode or scalar
  //      then generated check is
  //         whether upper bound of main vector loop is 0
  //    * non-masked mode
  //      then two checks generated:
  //         whether upper bound of the remainder vector loop is 0
  //      followed by
  //         whether upper bound of main vector loop is 0
  void createTCCheckBeforeMain(PlanDescr &MainDescr, PlanDescr *PRemDescr);

  // If original upper bound is an VPInstruction then move it to the first
  // non-empty basic block. This is needed due to all trip count checks
  // use it. The first non-empty basic block dominates all other blocks in
  // VPlan, including inner loops.
  void moveOrigUBToBegin();

  // Insert \p VectorUB at the end of \p BB, guarding it by PushVF/PopVF with
  // vector factor \p VF and unroll factor \p UF. If we insert \p VectorUB for
  // main VPlan we don't need push/pop. That is indicated by \p IsMain.
  void insertVectorUBInst(VPVectorTripCountCalculation *VectorUB,
                          VPBasicBlock *BB, unsigned VF, bool IsMain);

  // Read the original upper bound from VPlan of main loop and update
  // corresponding field.
  void updateOrigUB();
};

} // namespace vpo
} // namespace llvm

#endif // LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTELVPLANCFGMERGER_H
