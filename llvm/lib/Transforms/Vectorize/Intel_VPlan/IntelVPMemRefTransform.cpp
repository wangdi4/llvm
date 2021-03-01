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

/// Do appropriate transforms on SOA GEPs.
// For SOARnd and SOAStr-shaped GEPs, we add an extra argument, a const-vector
// <0, 1, .., VF-1>. This extra argument is represented as VPInstruction, which
// is not materialized, but, during CG, is tranformed into a const-vector. This
// const-vector aids in computation of address of the elements to be accessed
// within each private-copy of the array.
void VPMemRefTransform::transformSOAGEPs(unsigned VF) {

  auto *DA = cast<VPlanDivergenceAnalysis>(Plan.getVPlanDA());
  // Return true if this is a non unit-stride SOA-access GEPs, where atleast one
  // user is a load/store instruction.
  auto IsNonUnitStrideSOAAccessGEP = [&](const VPInstruction *I) {
    return isa<VPGEPInstruction>(I) &&
           (DA->isSOAShape(I) && !DA->isSOAUnitStride(I)) &&
           any_of(I->users(), [](const VPValue *User) {
             return isa<VPLoadStoreInst>(User);
           });
  };

  // Determine if the GEP should be cloned. We decide to clone the GEPs only if
  // any of it's users is a non load/store instruction.
  auto HasNonLoadStoreUser = [](const VPInstruction *GEP) {
    return any_of(GEP->users(),
                  [](const auto *User) { return !isa<VPLoadStoreInst>(User); });
  };

  // Algorithm:
  // 1) Iterate through the VPlan and look for non unit-stride SOA GEPs and make
  // sure that they are used in a load/store.
  // 2) For every such GEP, check if there is a non load/store user for that GEP.
  // If such GEP exists, create a clone of the GEP and then replace all the uses
  // of the original GEP in those instructions (typically other GEPs), with the
  // new cloned GEPs.
  // 3) Create a const-step-vector instruction and add it as an operand to the
  // original GEP. These would continue to be used for load/store operation.

  VPBuilder Builder;
  bool ResetSVA = false;

  for (auto &VPBB : Plan) {
    for (auto &I : VPBB) {
      if (IsNonUnitStrideSOAAccessGEP(&I)) {
        Builder.setInsertPoint(&I);

        // Clone the GEP instruction only if there is a non load/store user of
        // this GEP.
        if (HasNonLoadStoreUser(&I)) {
          // Clone the GEP.
          auto *ClonedGEP = I.clone();

          // Copy over the shape from the previous instruction.
          DA->updateVectorShape(ClonedGEP, DA->getVectorShape(I));

          // Use this GEP when we encounter the non-load/store user of this GEP.
          Builder.insert(ClonedGEP);

          // Replace all non load/store uses of original GEP in the user
          // instruction with the cloned GEP.
          I.replaceUsesWithIf(ClonedGEP, [](VPUser *User) {
            return !isa<VPLoadStoreInst>(User);
          });
        }
        VPInstruction *ConstVectorStepInst = Builder.create<VPConstStepVector>(
            "const.step", Type::getInt64Ty(*(Plan.getLLVMContext())), 0, 1, VF);
        I.addOperand(ConstVectorStepInst);
        assert(DA->isDivergent(I) && "Expect the GEP to be divergent.");
        DA->markDivergent(*ConstVectorStepInst);
        ResetSVA = true;
      }
    }
  }

  if (ResetSVA)
    Plan.invalidateAnalyses({VPAnalysisID::SVA});

  VPLAN_DUMP(SOAGEPsDumpsControl, Plan);
}

} // namespace vpo.
} // namespace llvm.
