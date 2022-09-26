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
#include "IntelVPlanDominatorTree.h"
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

// Validates an induction variable by asserting on following conditions -
// 1. "next" value dominates latch
// 2. "next" value has "correct" compute (same opcode as induction init, uniform
//    2nd operand)
static void validateInductionPHI(VPPHINode *Phi, VPLoop *VLp,
                                 VPlanVector &Plan) {
  unsigned InitOpNum = isa<VPInductionInit>(Phi->getOperand(0)) ? 0 : 1;
  unsigned NextOpNum = 1 - InitOpNum;
  auto *IVInit = cast<VPInductionInit>(Phi->getOperand(InitOpNum));
  auto *IVNext = cast<VPInstruction>(Phi->getOperand(NextOpNum));
  (void)IVInit;
  (void)IVNext;

  assert(Plan.getDT()->dominates(IVNext->getParent(), VLp->getLoopLatch()) &&
         "IV's next instruction should dominate loop latch.");
  assert(IVNext->getOpcode() == IVInit->getBinOpcode() &&
         "Opcode of next and induction-init should match.");
  assert(Plan.getVPlanDA()->isUniform(*IVNext->getOperand(1)) &&
         "Second operand of IV next instruction should be uniform.");
}

void VPlanSSADeconstruction::run() {
  VPLoop *VLoop = *(Plan.getVPLoopInfo()->begin());
  // Can't handle search loops.
  if (VLoop->getUniqueExitBlock() == nullptr)
    return;
  VPBuilderHIR Builder;
  unsigned DeconstructedPhiId = 0;
  bool ResetSVA = false;

  SmallVector<VPPHINode *, 4> PhisToErase;
  for (VPBasicBlock &VPBB : Plan) {
    auto *CurrVLoop = Plan.getVPLoopInfo()->getLoopFor(&VPBB);
    for (VPPHINode &Phi : VPBB.getVPPhis()) {
      if (CurrVLoop && &VPBB == CurrVLoop->getHeader()) {
        // If loop header PHI is an induction then copies are not needed for
        // them since it represents main loop IV which is processed specially in
        // HIR codegen.
        if (isa<VPInductionInit>(Phi.getOperand(0)) ||
            isa<VPInductionInit>(Phi.getOperand(1))) {
          VPInductionInit *IndInit =
              isa<VPInductionInit>(Phi.getOperand(0))
                  ? cast<VPInductionInit>(Phi.getOperand(0))
                  : cast<VPInductionInit>(Phi.getOperand(1));
          if (IndInit->isMainLoopIV()) {
            validateInductionPHI(&Phi, CurrVLoop, Plan);
            continue;
          }
        }
      }

      if (Phi.getNumIncomingValues() == 1) {
        unsigned Idx = 0;
        LLVM_DEBUG(dbgs() << "[SSADecons] Single value Phi replacing uses with "
                             "incoming value for: ";
                   Phi.dump());
        Phi.replaceAllUsesWith(Phi.getIncomingValue(Idx));
        PhisToErase.push_back(&Phi);
        continue;
      }

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
        // NOTE: The code below assumes that DA is not recomputed between SSA
        // deconstruction and CG. Consider the alternate solution if that's
        // needed in future - capture the deconstructed PHI's property in
        // HIRCopyInst and use that in DA to determine divergence/uniformity.
        if (Plan.getVPlanDA()->isDivergent(Phi))
          Plan.getVPlanDA()->markDivergent(*CopyInst);
        else
          Plan.getVPlanDA()->markUniform(*CopyInst);

        PhiInValUpdates[IncomingBlock] = CopyInst;
      }

      // Update incoming values of PHI to the newly inserted copy instructions.
      for (auto &UpdateKey : PhiInValUpdates)
        Phi.setIncomingValue(UpdateKey.first, UpdateKey.second);

      ResetSVA = true;
    }
  }

  for (auto *Phi : PhisToErase)
    Phi->getParent()->eraseInstruction(Phi);

  // Invalidate SVA results as VPlan has been changed.
  if (ResetSVA)
    Plan.invalidateAnalyses({VPAnalysisID::SVA});
}
