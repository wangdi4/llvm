//===-- IntelVPlanVerifier.h ------------------------------------*- C++ -*-===//
//
//   Copyright (C) 2016-2019 Intel Corporation. All rights reserved.
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

#if INTEL_CUSTOMIZATION
  /// Verify the CFG of the loop.
  static void verifyCFGExternals(const VPlan *Plan);
#endif
public:
  VPlanVerifier(const Loop *Lp, const DataLayout &DLObj)
      : TheLoop(Lp), DL(DLObj) {}

  VPlanVerifier(const DataLayout &DLObj) : TheLoop(nullptr), DL(DLObj) {}

  virtual ~VPlanVerifier() {}

  /// Set VPLoopInfo analysis. This information will be used in some
  /// verification steps, if available.
  void setVPLoopInfo(const VPLoopInfo *VPLI) { VPLInfo = VPLI; }

  /// Verify CFG externals, VPLoopInfo, VPLoop and number of loops in loop nest
  void verifyLoops(const VPlan *Plan, const VPDominatorTree &VPDomTree,
                   VPLoopInfo *VPLInfo);
};
} // namespace vpo
} // namespace llvm

#endif //LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTELVPLANVERIFIER_H
