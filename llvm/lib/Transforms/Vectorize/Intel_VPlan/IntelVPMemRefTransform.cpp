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

void VPMemRefTransform::transformSOAGEPs(unsigned InVF) {

  // Algorithm:
  // Iterate through the VPlan, identify loads/stores with SOAStr/SOARnd pointer
  // operands, and transform the pointers as follows:
  // - create a new GEP using original pointer and adding zero and a
  //   const-step-vector as indexes. This changes the GEP-type from
  //   <VF x Ty>* into <VF x Ty*>.
  //   After transformation we mark these GEPs as SOAConverted to easily
  //   identify them during CG.
  // - replace the original pointer in load/store with the new GEP.
  // The const-vector <0, 1, .., VF-1> extra index is represented as
  // VPInstruction, which is not materialized, but, during CG, is transformed
  // into a constant vector. This constant vector aids in computation of address
  // of the elements to be accessed within each private-copy of the array.

  VF = InVF;
  SmallSet<VPInstruction *, 16> InstructionsToProcess;

  for (auto &I : vpinstructions(&Plan)) {
    auto *LdSt = dyn_cast<VPLoadStoreInst>(&I);
    if (!LdSt)
      continue;
    VPValue *Ptr = LdSt->getPointerOperand();
    auto Shape = DA.getVectorShape(*Ptr);
    if (!Shape.isSOAShape())
      continue;
    if (Shape.isSOAUnitStride())
      continue;
    InstructionsToProcess.insert(cast<VPInstruction>(Ptr));
  }

  if (InstructionsToProcess.empty()) {
    VPLAN_DUMP(SOAGEPsDumpsControl, Plan);
    return;
  }

  for (auto *Ptr : InstructionsToProcess) {
    if (isa<VPPHINode>(Ptr) || isa<VPBlendInst>(Ptr))
      Builder.setInsertPointAfterBlends(Ptr->getParent());
    else
      Builder.setInsertPoint(Ptr->getParent(), std::next(Ptr->getIterator()));

    VPInstruction *ConstVectorStepInst = Builder.create<VPConstStepVector>(
        "const.step", Type::getInt32Ty(*(Plan.getLLVMContext())), 0, 1, VF);
    DA.markDivergent(*ConstVectorStepInst);
    auto Zero =
        Plan.getVPConstant(ConstantInt::get(ConstVectorStepInst->getType(), 0));

    VPLoadStoreInst *LdSt = cast<VPLoadStoreInst>(*llvm::find_if(
        Ptr->users(), [](const VPUser *U) { return isa<VPLoadStoreInst>(U); }));

    Type *LdStTy = LdSt->getValueType();
    VPGEPInstruction *GEP = Builder.createGEP(
        LdStTy, LdStTy, Ptr, {Zero, ConstVectorStepInst}, nullptr);
    Ptr->replaceUsesWithIf(
        GEP, [GEP](VPUser *U) { return U != GEP && isa<VPLoadStoreInst>(U); });

    // We mark this GEP as SOACvt to record the completion of this SOA
    // transformation.
    // This helps us identify the SOA-transformed GEPs in CG.
    //
    // *** GEP with SOAStr/SOARnd shape ***
    // [DA: [Shape: SOA Random]] i64* %vp4136 =
    //         getelementptr inbounds i64* %vp48688 i64 %vp48720
    // [DA: [Shape: Random]] store i64 %some_val, i64* %vp4136
    //
    // After Transformation:
    // *** Transformed Pointer with added index ***
    // [DA: [Shape: SOA Random]] i64* %vp4136 =
    //         getelementptr inbounds i64* %vp48688 i64 %vp48720
    //
    // [DA: [Shape: Random]] i64 %vp55978 =
    //         const-step-vector: { Start:0, Step:1, NumSteps:2}
    // [DA: [Shape: SOA Converted]] i64* %vp4136.n =
    //         = getelementptr inbounds i64* %vp4136 i64 0 i64 %vp55978
    //
    // [DA: [Shape: Random]] store i64 %some_val, i64* %vp4136.n
    //
    DA.updateVectorShape(GEP, VPVectorShape::SOACvt);
  }
  Plan.invalidateAnalyses({VPAnalysisID::SVA});

  VPLAN_DUMP(SOAGEPsDumpsControl, Plan);
}

} // namespace vpo.
} // namespace llvm.
