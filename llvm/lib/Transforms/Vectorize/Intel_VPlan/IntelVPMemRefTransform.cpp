//===- IntelVPMemRefTransform.cpp -------------------------------*- C++ -*-===//
//
//   Copyright (C) 2020 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file implements the VPMemRefTransform class.
//===---------------------------------------------------------------------===//

#include "IntelVPMemRefTransform.h"
#include "IntelVPlan.h"
#include "IntelVPlanBuilder.h"
#include "IntelVPlanDivergenceAnalysis.h"
#include "IntelVPlanUtils.h"

#define DEBUG_TYPE "vp-memref-transform"

using namespace llvm;
using namespace llvm::vpo;

static LoopVPlanDumpControl SOAGEPsDumpsControl("transformed-soa-geps",
                                                "Dump Transformed SOA GEPs");

namespace llvm {
namespace vpo {

// Return true if this is a non unit-stride SOA GEP.
bool VPMemRefTransform::isSOANonUnitStridedGEP(VPInstruction *I) {
  return isa<VPGEPInstruction>(I) && DA.isSOAShape(I) && !DA.isSOAUnitStride(I);
}

// Return \true if the given GEP is a SOA-unit-stride GEP.
bool VPMemRefTransform::isSOAUnitStridedGEP(VPInstruction *I) {
  return isa<VPGEPInstruction>(I) && DA.isSOAUnitStride(I);
}

// Method which clones the instruction if predicate \p Pred is true for its
// users. The cloned instruction is replaced in all users for which \p Pred
// is true.
void VPMemRefTransform::cloneAndReplaceUses(
    VPInstruction *I, std::function<bool(const VPUser *)> Pred) {

  if (none_of(I->users(), [&](const VPUser *User) { return Pred(User); }))
    return;

  Builder.setInsertPoint(I);
  // Clone the GEP.
  auto *ClonedInst = I->clone();

  // Copy over the shape from the previous instruction.
  DA.updateVectorShape(ClonedInst, DA.getVectorShape(*I));

  // Use this GEP when we encounter the non-load/store user of this GEP.
  Builder.insert(ClonedInst);

  // Replace original GEP by cloned one in filtered users.
  I->replaceUsesWithIf(ClonedInst,
                       [&](const VPUser *User) { return Pred(User); });
}

// Helper method to update dependent PHI instructions.
void VPMemRefTransform::updateDependentPHIs(VPInstruction *I) {
  // Calling DA.updateDivergence(*I) does not quite work as
  // the call marks the GEP as SOA unit-stride again, and as a
  // consequence, shape of the PHIs are not updated.
  for (auto *User : I->users())
    if (auto *Phi = dyn_cast<VPPHINode>(User))
      DA.updateDivergence(*Phi);
}

// Transform SOA-unitstrided GEPs to GEPs which return a vector of pointers to
// the base-address of each element.
void VPMemRefTransform::transformSOAUnitStrideGEPs(VPGEPInstruction *GEP) {
  Builder.setInsertPoint(GEP->getParent(), std::next(GEP->getIterator(), 1));
  Type *IndexTy = Type::getInt64Ty(*(Plan.getLLVMContext()));
  VPInstruction *ConstVectorStepInst =
      Builder.create<VPConstStepVector>("const.step", IndexTy, 0, 1, VF);
  VPInstruction *BaseAddrGEP = Builder.createGEP(
      GEP->getResultElementType(),
       // FIXME: This is totally wrong, but consistent with a pre-existing bug
       // in the VPGEPInstruction's ctor...
      GEP->getResultElementType(),
      GEP,
      {Plan.getVPConstant(ConstantInt::get(IndexTy, 0)), ConstVectorStepInst},
      nullptr);
  GEP->replaceUsesWithIf(BaseAddrGEP, [&](VPUser *User) {
    return isa<VPPHINode>(User) || isa<VPBlendInst>(User);
  });
  DA.markDivergent(*ConstVectorStepInst);
  // We mark this GEP as SOACvt to record the completion of this SOA
  // transformation.
  // This prevents a re-processing the GEPs and helps us identify the
  // SOA-transformed GEPs in CG.
  // *** Pointer to SOA private with SOASeq shape ***
  // [DA: [Shape: SOA Unit Stride, Stride: i64 8]] i64* %vp53916
  //               = getelementptr inbounds [1024 x i64]* %vp54568 i64 0 i64 1
  // …
  // BB25: # preds: BB24, BB23
  // [DA: [Shape: SOA Random], SVA: ( V )] i64* %vp55654 = phi  [ i64* %vp53916,
  //                     BB24 ],  [ i64* %vp53364, BB23 ] (SVAOpBits 0->V 1->V )
  //
  // After transformation:
  // *** Pointer to SOA private with SOASeq shape ***
  // [DA: [Shape: SOA Unit Stride, Stride: i64 8]] i64* %vp53916
  //               = getelementptr inbounds [1024 x i64]* %vp54568 i64 0 i64 1
  // *** Transformed pointer with added index ***
  // [DA: [Shape: Random]] i64 %vp48614 = const-step-vector: { Start:0, Step:1,
  //                                                           NumSteps:2}
  // [DA: [Shape: Random]] i64* %vp25688 = getelementptr i64*
  //                                               %vp53916 i64 0 i64 %vp48614
  // …
  // BB25: # preds: BB24, BB23
  // [DA: [Shape: Random]] i64* %vp55654 = phi  [ i64* %vp25688, BB24 ],  [ i64*
  //                                                            %vp53364, BB23 ]
  DA.updateVectorShape(BaseAddrGEP, VPVectorShape::SOACvt);
  //  Call updateDependentPHIs() to update the shape of dependent instructions.
  updateDependentPHIs(BaseAddrGEP);
}

// Transform SOA-non-unitstride GEPs to GEPs which return a vector of pointers
// to the base-address of each element.
void VPMemRefTransform::transformSOANonUnitStrideGEPs(VPGEPInstruction *GEP) {
  Builder.setInsertPoint(GEP);
  VPInstruction *ConstVectorStepInst = Builder.create<VPConstStepVector>(
      "const.step", Type::getInt64Ty(*(Plan.getLLVMContext())), 0, 1, VF);
  GEP->addOperand(ConstVectorStepInst);
  assert(DA.isDivergent(*GEP) && "Expect the GEP to be divergent.");
  DA.markDivergent(*ConstVectorStepInst);
  // We mark this GEP as SOACvt to record the completion of this SOA
  // transformation.
  // This prevents a re-processing the GEPs and helps us identify the
  // SOA-transformed GEPs in CG.
  // *** GEP with SOAStr/SOARnd shape ***
  // [DA: [Shape: SOA Random], SVA: ( V )] i64* %vp4136
  //  = getelementptr inbounds i64* %vp48688 i64 %vp48720 (SVAOpBits 0->F 1->V )
  //
  // After Transformation:
  // *** Transformed Pointer with added index ***
  // [DA: [Shape: Random], SVA: ( V )] i64 %vp55978
  //  = const-step-vector: { Start:0, Step:1, NumSteps:2} (SVAOpBits )
  // [DA: [Shape: Random], SVA: ( V )] i64* %vp4136
  //  = getelementptr inbounds i64* %vp48688 i64 %vp48720 i64
  //      %vp55978 (SVAOpBits 0->F 1->V 2->V )
  //
  DA.updateVectorShape(GEP, VPVectorShape::SOACvt);
  //  Call updateDependentPHIs() to update the shape of dependent instructions.
  //  NOTE: Blends are always random in the scenario. So, now special handling
  //  is required in the callee.
  updateDependentPHIs(GEP);
}

/// Do appropriate transforms on SOA GEPs.
// For SOARnd and SOAStr-shaped GEPs, we add an extra argument, a const-vector
// <0, 1, .., VF-1>. This extra argument is represented as VPInstruction, which
// is not materialized, but, during CG, is tranformed into a const-vector. This
// const-vector aids in computation of address of the elements to be accessed
// within each private-copy of the array.
// For SOASeq instruction, if the user happens to be a PHI(which merges SOA
// non-unitstrided pointers)/blend, we just get the base-address of the
// individual elements and pass the resultant GEP to the PHI. We also clone the
// original instruction, if there are non-PHI users for the GEP.
void VPMemRefTransform::transformSOAGEPs(unsigned InVF) {

  // Algorithm:
  // Iterate through the VPlan and,
  // 1) For SOAStr/SOARnd GEPs,
  //   i) If there is a non load/store/PHI/Blend user for the GEP, clone the GEP
  //   and then replace all those uses of the original GEP with the cloned GEP.
  //   ii) For load/store/Phi/Blend uses, create a const-step-vector instruction
  //   and add it as an operand to the original GEP.
  // 2) For SOASeq GEP,
  //   i) If there is a load/store/GEP or a PHI(which merges only SOAAeq ptrs),
  //   user of that GEP, we want to maintain the SOASeq property of the GEP, so
  //   that we generate wide load/store(s). Clone the GEP and replace all the
  //   load/store uses with the cloned GEP.
  //   ii) For PHI/Blend users, add a
  //   ZeroInitializer and a const-step-vector as arguments to the original GEP.
  //   This changes the GEP-type from <VF x Ty>*
  //   -> <VF x Ty*>. Also, mark this new GEP as 'Random' shape and then invoke
  //   updateDivergence() on dependent PHIs. This changes the shape of the PHIs
  //   from SOARandom to just Random.

  VF = InVF;
  bool ResetSVA = false;
  SmallVector<VPInstruction *, 50> InstructionsToProcess;

  auto SOANonUnitStridedPHIUsersToProcess =
      [=](const VPUser *U) -> bool {
    return isa<VPLoadStoreInst>(U) || isa<VPPHINode>(U) || isa<VPBlendInst>(U);
  };

  auto SOAUnitStridedPHIUsersToProcess = [&](const VPUser *U,
                                             Type *Ty) -> bool {
    return (isa<VPPHINode>(U) && !DA.isUnitStridePtr(U, Ty)) ||
           isa<VPBlendInst>(U);
  };

  for (auto &VPBB : Plan) {
    for (auto &I : VPBB) {
      if (isSOANonUnitStridedGEP(&I)) {

        // We want to transform GEP only if it has one of the following users.
        if (any_of(I.users(), [&](const VPUser *User) {
              return SOANonUnitStridedPHIUsersToProcess(User);
            }))
          InstructionsToProcess.push_back(&I);
      }
      if (isSOAUnitStridedGEP(&I)) {
        // if:
        //   %gep = getelementptr ..., %soa.ptr, i64 1 (SOA Unit-stride GEP.)
        //   %ld  = load i64, %gep
        //   ...
        // merge.block:
        //   %phi = phi [ [%gep, %if], [%some.gep, %else]]
        //
        // for VF = 2, is transformed to,
        //
        // if:
        //   %gep.clone = getelementptr ..., %soa.ptr
        //   %gep = getelementptr ..., %soa.ptr, <i64 0, i64 0> <i64 0, i64 1>
        //   %ld  = load i64, %gep.clone
        // ...
        // merge.block:
        //   %phi = phi [ [%gep, %if], [%some.gep, %else]]
        //   %blend.res = blend [ [%gep, %if], [%some.gep, %else]]
        //   ...

        // We want to transform SOA unit-strided GEP only if it has the
        // following users and constraints.
        Type *PointedToTy = cast<VPGEPInstruction>(&I)->getResultElementType();
        if (any_of(I.users(), [&](const VPUser *U) {
              return SOAUnitStridedPHIUsersToProcess(U, PointedToTy);
            }))
          InstructionsToProcess.push_back(&I);
      }
    }
  }

  // We are definitely transforming some instructions, so reset SVA.
  if (!InstructionsToProcess.empty())
    ResetSVA = true;

  for (auto *I : InstructionsToProcess) {
    if (isSOANonUnitStridedGEP(I)) {
      // Clone the GEP instruction only if there is a non load/store/PHI/Blend
      // user of this GEP.
      cloneAndReplaceUses(
          I, [SOANonUnitStridedPHIUsersToProcess](const VPUser *U) {
            return !SOANonUnitStridedPHIUsersToProcess(U);
          });
      transformSOANonUnitStrideGEPs(cast<VPGEPInstruction>(I));
    }

    if (isSOAUnitStridedGEP(I)) {
      // Clone the GEP instruction only if there is a non SOA-unitstrided
      // PHI/Blend user of this GEP.
      Type *PointedToTy = cast<VPGEPInstruction>(I)->getResultElementType();
      cloneAndReplaceUses(
          I, [SOAUnitStridedPHIUsersToProcess, &PointedToTy](const VPUser *U) {
            return !SOAUnitStridedPHIUsersToProcess(U, PointedToTy);
          });
      transformSOAUnitStrideGEPs(cast<VPGEPInstruction>(I));
    }
  }

  if (ResetSVA)
    Plan.invalidateAnalyses({VPAnalysisID::SVA});

  VPLAN_DUMP(SOAGEPsDumpsControl, Plan);
}

} // namespace vpo.
} // namespace llvm.
