//===-- IntelVPlanVerifier.h ------------------------------------*- C++ -*-===//
//
//   Copyright (C) 2016 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation. and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file declares VPlanVerifier class that is used to verify that several
/// aspects of a VPlan are correct.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTELVPLANVERIFIER_H
#define LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTELVPLANVERIFIER_H

#include "IntelVPlan.h"
#include "llvm/IR/InstrTypes.h"

namespace llvm {
namespace vpo {

class VPlanVerifier {
private:
  // Outermost LLVM-IR loop to be vectorized.
  const Loop *TheLoop;

  // DataLayout of verifying size-related properties.
  const DataLayout &DL;

  // VPLoopInfo analysis information.
  const VPLoopInfo *VPLInfo = nullptr;

  // Stored flags for checks to run or skip
  unsigned int Flags = 0;

  // Verify VPPHINode instruction.
  void verifyPHINode(const VPPHINode *Phi) const;

  // Verify VPGEPInstruction instruction.
  void verifyGEPInstruction(const VPGEPInstruction *GEP) const;

  // Verify VPSubscriptInst instruction.
  void verifySubscriptInst(const VPSubscriptInst *SI) const;

  // Main functions driving the verification of instructions, blocks,
  // loops and regions.

  /// Verify Specific VPInstructions.
  void verifySpecificInstruction(const VPInstruction *Inst) const;

  /// Verify VPInstructions.
  void verifyAbsInst(const VPInstruction *I) const;
  void verifyICmpInst(const VPInstruction *I) const;
  void verifyFCmpInst(const VPInstruction *I) const;
  void verifyZExtInst(const VPInstruction *I) const;
  void verifySExtInst(const VPInstruction *I) const;
  void verifyFPExtInst(const VPInstruction *I) const;
  void verifyTruncInst(const VPInstruction *I) const;
  void verifyFPTruncInst(const VPInstruction *I) const;
  void verifyFPToUIInst(const VPInstruction *I) const;
  void verifyFPToSIInst(const VPInstruction *I) const;
  void verifyUIToFPInst(const VPInstruction *I) const;
  void verifySIToFPInst(const VPInstruction *I) const;
  void verifyIntToPtrInst(const VPInstruction *I) const;
  void verifyPtrToIntInst(const VPInstruction *I) const;
  void verifyBitCastInst(const VPInstruction *I) const;
  void verifyBinaryOperator(const VPInstruction *BI) const;

  void verifyInstruction(const VPInstruction *Inst,
                         const VPBasicBlock *Block) const;

  /// Verify a block.
  void verifyBlock(const VPBasicBlock *Block) const;
  void verifyNumLoops(void) const;

  // Count the number of loops in the underlying IR.
  virtual unsigned countLoopsInUnderlyingIR() const;

  // Separate verifyOperands/verifyUsers below are needed because we don't fully
  // represent each use with an explicit edge (unlike llvm::Value hierarchy).
  // Still, better than nothing.

  /// Verify that:
  /// 1. No operands are null, and
  /// 2. Every operand has \p U in its users.
  static void verifyOperands(const VPUser *U);

  /// Verify that each user of \p Def has \p Def as an operand.
  static void verifyUsers(const VPValue *Def);

  /// Verify the CFG of the loop.
  static void verifyCFGExternals(const VPlan *Plan);

  // Verify that all of the Phi and Blend statements are at the start
  // of the block.
  void verifyPhiBlendPlacement(const VPBasicBlock *Block) const;

  // Verify that an instruction dominates all of its uses
  void verifySSA(const VPInstruction *I, const VPDominatorTree *DT) const;

  // Verify using the DA verify function
  void verifyDA(const VPlan *Plan) const;

  // Recalculate DA shapes of Inst and compare with the stored shape
  // Only valid before predicator, after which the shapes can't reliably
  // be recomputed.
  void verifyDAShape(const VPInstruction *VPI) const;

  // Verify the LiveIn/Out lists attached to the VPlan
  // Checks that there is a corresponding VPExternalUse for every live-in
  // and live-out, and that the index of each live-in/out in the list
  // matches the merge ID of the external use.
  void verifyLiveInOut(const VPlanVector *Plan) const;

  // Verify that the header of a loop and its exits have the same predicate,
  // or that none are predicated.
  void verifyHeaderExitPredicates(const VPLoop *Lp) const;

  // Helper functions to hide the underlying enum check
  bool shouldSkipLoopInfo() const { return Flags & SkipLoopInfo; }

  bool shouldSkipExternals() const { return Flags & SkipExternals; }

  bool shouldSkipInnerMultiExit() const { return Flags & SkipInnerMultiExit; }

  bool shouldSkipNumLoops() const { return !(Flags & CheckNumLoops); }

  bool shouldSkipDA() const { return Flags & SkipDA; }

  bool shouldSkipDAShapes() const { return !(Flags & CheckDAShapes); }

public:
  // Enum for holding the flags to be given to verifyVPlan to skip
  // parts of the verification.
  enum {
    SkipLoopInfo = 1 << 0,
    SkipExternals = 1 << 1,
    SkipInnerMultiExit = 1 << 2,
    CheckNumLoops = 1 << 3,
    SkipDA = 1 << 4,
    CheckDAShapes = 1 << 5
  };

  VPlanVerifier(const Loop *Lp, const DataLayout &DLObj)
      : TheLoop(Lp), DL(DLObj) {}

  VPlanVerifier(const DataLayout &DLObj) : TheLoop(nullptr), DL(DLObj) {}

  virtual ~VPlanVerifier() {}

  /// Set VPLoopInfo analysis. This information will be used in some
  /// verification steps, if available.
  void setVPLoopInfo(const VPLoopInfo *VPLI) { VPLInfo = VPLI; }

  /// Verify CFG externals, VPLoopInfo, VPLoop and number of loops in loop nest
  void verifyVPlan(const VPlanVector *Plan, unsigned int CheckFlags = 0);

  // Interface for calling the verifier more easily
  // Constructs a VPlanVerifier instance, then calls verifyVPlan on it
  static void verify(const VPlanVector *Plan, const Loop *Lp = nullptr,
                     unsigned int CheckFlags = 0);
};
} // namespace vpo
} // namespace llvm

#endif // LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTELVPLANVERIFIER_H
