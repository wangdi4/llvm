//===-- IntelVPlanSSADeconstruction.cpp -------------------------*- C++ -*-===//
//
//   Copyright (C) 2020 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//

#include "IntelVPlanSSADeconstruction.h"
#include "VPlanHIR/IntelVPOCodeGenHIR.h"
#include "VPlanHIR/IntelVPlanBuilderHIR.h"
#include "llvm/Support/CommandLine.h"

#define DEBUG_TYPE "VPlanSSADeconstruction"

using namespace llvm::vpo;

static cl::opt<bool, true> PrintAfterSSADeconstructionOpt(
    "vplan-print-after-ssa-deconstruction", cl::Hidden,
    cl::location(PrintAfterSSADeconstruction),
    cl::desc("Print VPlan after deconstructing PHIs as copies."));

namespace llvm {
namespace vpo {
bool PrintAfterSSADeconstruction = false;
} // namespace vpo
} // namespace llvm

void VPlanSSADeconstruction::run() {
  assert(Plan.getVPLoopInfo()->size() == 1 && "Expected one loop");
  VPLoop *VLoop = *(Plan.getVPLoopInfo()->begin());
  VPBuilderHIR Builder;
  unsigned DeconstructedPhiId = 0;

  for (VPBasicBlock &VPBB : Plan) {
    // Outermost loop header PHIs are either inductions or reductions (loop
    // entities). Copies are not needed for them since they are processed
    // specially in VPOCodeGenHIR::createAndMapLoopEntityRefs. TODO: Consider
    // using this copy-based implementation for loop entities too.
    if (&VPBB == VLoop->getHeader())
      continue;

    for (VPPHINode &Phi : VPBB.getVPPhis()) {
      LLVM_DEBUG(dbgs() << "[SSADecons] Inserting copies for: "; Phi.dump());
      // Create a new ID to tag the copies generated for the PHI.
      unsigned PhiId = DeconstructedPhiId++;

      // Insert a copy for each incoming value at the end of corresponding
      // incoming block, and update the PHI to use the copies as corresponding
      // incoming values. For example -
      // BB0:
      //   %0 = ...
      //   ...
      //
      // BB1:
      //   %1 = ...
      //   ...
      //
      // BB2:
      //   %phi = [%0, BB0], [%1, BB1]
      //
      // is transformed to -
      // BB0:
      //   %0 = ...
      //   ...
      //   %copy.BB0 = hir-copy %0
      //
      // BB1:
      //   %1 = ...
      //   ...
      //   %copy.BB1 = hir-copy %1
      //
      // BB2:
      //   %phi = [%copy.BB0, BB0], [%copy.BB1, BB1]
      //
      //
      SmallDenseMap<VPBasicBlock *, VPValue *, 4> PhiInValUpdates;
      for (auto *IncomingBlock : Phi.blocks()) {
        VPValue *IncomingValue = Phi.getIncomingValue(IncomingBlock);
        // Add the copy at the end of incoming block.
        Builder.setInsertPoint(IncomingBlock);
        auto *CopyInst = Builder.createHIRCopy(IncomingValue);
        // Tag the copy instruction with the deconstructed PHI ID it was created
        // for. This is used by CG to identify all copies that correspond to
        // same PHI.
        CopyInst->setOriginPhiId(PhiId);
        LLVM_DEBUG(dbgs() << "[SSADecons] CopyInst: "; CopyInst->dump();
                   dbgs() << " is mapped to the origin PHI ID: " << PhiId
                          << "\n");
        // TODO: Update SVA of new instruction. Should be same as IncomingValue.
        if (!Plan.getVPlanDA()->isDivergent(*IncomingValue))
          Plan.getVPlanDA()->markUniform(*CopyInst);

        PhiInValUpdates[IncomingBlock] = CopyInst;
      }

      // Update incoming values of PHI to the newly inserted copy instructions.
      for (auto &UpdateKey : PhiInValUpdates)
        Phi.setIncomingValue(UpdateKey.first, UpdateKey.second);
    }
  }
}
