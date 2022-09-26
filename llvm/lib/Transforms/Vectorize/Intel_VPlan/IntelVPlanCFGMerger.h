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
class VPInvSCEVWrapper;

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

  // Boolean flag to indicate if the scenario is a main vector loop and a scalar
  // remainder for a constant trip count original loop. If true, we can avoid
  // unnecessary checks to see if we should exercise main and scalar loops.
  // This also allows setting a constant lower bound for the scalar remainder
  // which is important for downstream HIR optimizations.
  // TODO - this flag is only set for HIR path for now. Revisit for the LLVM
  // IR path if needed later.
  bool IsSimpleConstTCScenario = false;

public:
  VPlanCFGMerger(VPlanVector &P, unsigned V, unsigned U)
      : Plan(P), ExtVals(P.getExternals()), MainVF(V), MainUF(U) {}

  // Create a list of VPlan descriptors (along with the needed VPlans) by
  // vectorization scenario \p Scen.
  template <class LoopTy>
  static void createPlans(LoopVectorizationPlanner &Planner,
                          const SingleLoopVecScenario &Scen,
                          std::list<CfgMergerPlanDescr> &Plans,
                          LoopTy *OrigLoop, VPlan &MainPlan,
                          VPAnalysesFactoryBase &VPAF);

  // The first pass of the universal CFG merger. Creates merged CFG by scenario
  // \p Scen and the prepared list of VPlans \p Plans. It goes through the list
  // of VPlans, creates placeholder basic blocks for them, inserts merging
  // blocks, and emits the needed trip count checks.
  template <class LoopTy>
  void createMergedCFG(SingleLoopVecScenario &Scen,
                       std::list<CfgMergerPlanDescr> &Plans, LoopTy *OrigLoop);

  void mergeVPlans(std::list<CfgMergerPlanDescr> &Plans);

  void setIsSimpleConstTCScenario(bool Val) { IsSimpleConstTCScenario = Val; }

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

  // Create a VPBasicBlock with a check for remaining iterations count after
  // vector loop. It's supposed that VPVectorTripCountCalculation and
  // VPOrigTripCountCalculation are inserted in a block before the \p Plan loop
  // preheader. There is created a new block and the check for the vector TC is
  // equal to the original TC is created with two successors, remainder
  // preheader and final merge block.
  VPBasicBlock *createRemainderTopTest(VPBasicBlock *InsertAfter,
                                       VPBasicBlock *RemPreheader,
                                       VPBasicBlock *FinalMergeBlock);

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
  template <class LoopTy>
  void emitSkeleton(std::list<PlanDescr> &Plans, LoopTy *OrigLoop);

  // Create a check for whether the loop described by \p PrevDescr should be
  // executed after the loop described by \p Descr. I.e. the check for the upper
  // bound of the loop from \p Descr is not equal to the upper bound of the loop
  // from \p PrevDescr. If the upper bounds are not equal then the previous loop
  // has some iterations to execute. Here "previous" is about order of
  // processing, not about placement in CFG.
  void createTCCheckAfter(PlanDescr &Descr, PlanDescr &PrevDescr);

  // Create a sequence of instructions to compare (PeelCount + \p VF) with
  // \p UB (PeelCount is a class member initialized during corresponding
  // instructions insertion). The comparison is created with predicate \p Pred.
  // The \p Builder's insertion point should be setup before the call.
  VPCmpInst *createPeelCntVFCheck(VPValue *UB, VPBuilder &Builder,
                                  CmpInst::Predicate Pred, unsigned VF);

  // Create a top test, i.e. the sequence of instructions (placed in a separate
  // basic block)
  //    %c = icmp pred LHS, <vector loop upper bound>
  //    br %c, label %SuccEq, label %SuccNe
  //
  // where:
  //  both Succ* are parameters of the routine, <vector loop upper bound> is
  //  the upper bound of the loop described by VecPlan, and,
  //  in case when \p Peel is not null,
  //     LHS = PeelCount + MainVF
  //     pred = ugt
  //  For the case w/o peel:
  //     LHS = 0
  //     pred = eq
  //
  // The generated top test block is inserted before \p InsertBefore.
  VPBasicBlock *createTopTest(VPlan *VecPlan, VPBasicBlock *InsertBefore,
                              VPBasicBlock *SuccEq, VPBasicBlock *SuccNe,
                              VPlan *Peel, unsigned VF);

  // The routine creates one, two or none top test checks. The arguments
  // \p MainDescr, \p PRemDescr, \p PPrevDescr, and \p Peel represent
  // descriptors of main, remainder (one following the main loop), second
  // remainder (one following remainder after main) and peel loop respectively.
  // The \p PPrevDescr is not null only in case when \p PRemDescr describes
  // non-masked VPlan and there is one more remainder created. Each check (if
  // generated) is inserted into a new block placed before MainDescr.FirstBB.
  // The top test checks are generated by the following rules:
  // - if the main loop is masked mode or there is no remainder loop (i.e
  //   PRemDescr is null) then no checks are generated
  // - if the remainder loop is:
  //    * masked mode or scalar
  //      then generated check is
  //        if no peel: whether upper bound of main vector loop is 0
  //        else      : whether upper bound of main loop is less than
  //                    (peel_count + mainVF*MainUF)
  //        \p MainDescr->PrevMerge in this case is used as "true" target
  //        for jump and MainDescr->FirstBB as "false" target.
  //    * non-masked mode
  //      then two checks generated:
  //        if no peel: whether upper bound of vector remainder loop is 0
  //        else      : whether upper bound of vector remainder loop is less
  //                    than (peel_count + vector_remainder_VF)
  //        if \p PPrevDescr is passed (meaning there is one more remainder)
  //        then \pPPrevDescr->MergeBefore otherwise PRemDescr->PrevMerge is
  //        used as "true" target. And the next check is used as false target.
  //      followed by
  //        if no peel: whether upper bound of main vector loop is 0
  //        else      : whether upper bound of main loop is less than
  //                    (peel_count + mainVF*MainUF)
  //        \p MainDescr->PrevMerge in this case is used as "true" target
  //        for jump and MainDescr->FirstBB as "false" target.
  //
  void createTCCheckBeforeMain(PlanDescr *Peel, PlanDescr &MainDescr,
                               PlanDescr *PRemDescr, PlanDescr *PPrevDescr);

  // Before the peel loop creates PeelCount instruction and the following
  // checks:
  //  - for static peel no checks is inserted
  //  - for dynamic peel we have
  //    - check for peeled pointer has zero low bits (if not we can't peel)
  //      jumping to the next check or to the \p RemainderMerge block which
  //      is entry for the remainder/main loop.
  //    - check for peel count is not 0, jumping to the next check or to the
  //      merge block after the peel loop.
  //    - check for (PeelCount + MainVF*MainUF) is less than main loop upper
  //      bound, jumping to the remainder or to the peel loop.
  template <class LoopTy>
  void insertPeelCntAndChecks(PlanDescr &PeelDescr,
                              VPBasicBlock *FinalRemainderMerge,
                              VPBasicBlock *RemainderMerge, LoopTy *OrigLoop);

  // Generate the following sequence to check whether lower bits of the pointer
  // for \p Peeling are non-zero. If they are non-zero we can't align the
  // pointer by peeling.
  //  %ptr = Peeling->getMemRef()->getPointerOperand();
  //  %ptr2int = ptrtoint i64 %ptr
  //  %and = and %ptr2int, (Peeling->getTargetAlignment().value()-1)
  //  %cmp = icmp eq %and, 0
  //  br %cmp, %InsertBefore, %NonZeroMerge
  //
  //  \p InsertBefore is block before which we insert the check and where we jump
  //  to in case the bits are 0. \p NonZeroMerge block can be merge block either
  //  before scalar remainder or before main loop. That depends on whether we
  //  can execute the main loop w/o peeling. E.g. in case of a search loop with
  //  speculative loads the alignment is required and we should go to the scalar
  //  remainder.
  template <class LoopTy>
  void createPeelPtrCheck(VPlanDynamicPeeling &Peeling,
                          VPBasicBlock *InsertBefore,
                          VPBasicBlock *NonZeroMerge, VPlan &P,
                          VPValue *&PeelBasePtr, LoopTy *OrigLoop);

  // Predicate whether we need peel for safety, e.g. in the search loop.
  bool needPeelForSafety() const;

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

  // Set operands of VPlanAdapter from \p AdapterBB to the phis that are
  // in the \p MergeBB.
  void updateAdapterOperands(VPBasicBlock *AdapterBB, VPBasicBlock *MergeBB);

  // Emit the sequence of instructions to calculate peel count by the following
  // formula.
  //   Quotient = BasePtr / DP.RequiredAlignment;
  //   Divisor = DP.TargetAlignment / DP.RequiredAlignment;
  //   PeelCount = (Quotient * Multiplier) % Divisor;
  template <class LoopTy>
  VPValue *emitDynamicPeelCount(VPlanDynamicPeeling &DP, VPValue *BasePtr,
                                VPBuilder &Builder, LoopTy *OrigLoop);

  // Create VPInvSCEVWrapper for \p Peeling->invariantBase at the \p Builder's
  // insertion point.
  template <class LoopTy>
  VPInvSCEVWrapper *emitPeelBasePtr(VPlanDynamicPeeling &Peeling,
                                    VPBuilder &Builder, LoopTy *OrigLoop);

  // Insert VPPushVF/VPPopVF in the VPlan body. VPPushVF is inserted at the
  // beginning of the first VPBasicBlock of VPlan and VPPopVF is inserted in the
  // end of the last VPBasicBlock of VPlan.
  static void insertPushPopVF(VPlan &P, unsigned VF, unsigned UF);

  // Copy DA data from all VPlans in the list to the main VPlan DA.
  void copyDA(std::list<PlanDescr> &Plans);

  // The utility function to update incoming values of VPlans from list by
  // corresponding values from merged CFG skeleton. In skeleton, we have
  // VPlanAdapter instructions inserted in the corresponding blocks. The
  // incoming values of each VPlan (except the main VPlan) are replaced by
  // operands of VPlanAdapter. The incoming values of the main VPlan are
  // replaced using phi nodes of the merge block before main VPlan if that block
  // exists. It may be absent in case when we have no peel.
  void updateVPlansIncomings(std::list<PlanDescr> &Plans);

  // Replace all uses of \p Adapter with corresponding outgoing values of VPlan.
  // Each use is expected to be a VPPHINode with non-undefined merge id. Then we
  // get a corresponding VPLiveOut from VPlan and replace \p Adapter uses by
  // VPLiveOut operand.
  void replaceAdapterUses(VPlanAdapter *Adapter, VPlan &P);

  // Merge all basic blocks in VPlans from the list into main VPlan.
  void mergeVPlanBodies(std::list<PlanDescr> &Plans);

  // Copy LoopInfo from VPlan \p P to the main VPlan. The loops are added
  // at the same level as they exist in \p P.
  void mergeLoopInfo(VPlanVector &P);
};

} // namespace vpo
} // namespace llvm

#endif // LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTELVPLANCFGMERGER_H
