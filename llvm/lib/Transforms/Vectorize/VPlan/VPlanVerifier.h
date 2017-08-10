//===-- VPlanVerifier.h --------------------------------------*- C++ -*-===//
//
//   Copyright (C) 2015-2017 Intel Corporation. All rights reserved.
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
/// aspect of a VPlan are correct.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_VECTORIZE_VPLAN_VPLANVERIFIER_H
#define LLVM_TRANSFORMS_VECTORIZE_VPLAN_VPLANVERIFIER_H

#include "IntelVPlan.h"

namespace llvm {
namespace vpo {

class VPlanVerifierBase {

protected:
  // VPLoopInfo analysis information.
  const VPLoopInfo *VPLInfo;

private:
  // Main functions driving the verification of regions and loops.
  void verifyRegions(const VPRegionBlock *Region) const;
  void verifyLoops(const VPRegionBlock *TopRegion) const;

  // Auxiliary functions for loop verification.
  void verifyVPLoopInfo(const VPLoopRegion *LoopRegion) const;
  void verifyContainerLoop(const VPBlockBase *Block,
                           const VPLoopRegion *ParentLoopR) const;
  void verifyLoopRegions(const VPRegionBlock *TopRegion) const;
  void verifyNumLoops(const VPRegionBlock *TopRegion) const;
  // Count the number of loops in the underlying IR. 
  virtual unsigned countLoopsInUnderlyingIR() const = 0;
  // Perform IR-specific checks for IR-specific VPLoopRegion.
  virtual void
  verifyIRSpecificLoopRegion(const VPRegionBlock *Region) const = 0;

public:
  VPlanVerifierBase() : VPLInfo(nullptr) {}
  virtual ~VPlanVerifierBase() {}

  /// Set VPLoopInfo analysis. This information will be used in some
  /// verification steps, if available.
  void setVPLoopInfo(const VPLoopInfo *VPLI) { VPLInfo = VPLI; }

  /// Verify that H-CFG is well-formed starting from TopRegion. If \p VPLInfo
  /// and \p LI are not nullptr, it also checks that loop related information in
  /// H-CFG is consistent with information in VPLInfo and LoopInfo. The
  /// verification process comprises two main phases:
  ///
  /// 1. verifyLoops: A first global verification step checks that the number
  /// of VPLoopRegion's (H-CFG), VPLoop's (VPLoopInfo) and Loop's (LoopInfo)
  /// match. In a second step, it checks that the following invariants are met
  /// in every VPLoopRegion:
  ///   - VPLoopRegion has VPLoop attached.
  ///   - Entry is loop preheader
  ///   - Loop preheader has a single successor (loop header)
  ///   - VPLoopInfo returns the expected VPLoop from loop preheader/header
  ///   - VPLoop preheader and exits are contained in VPLoopRegion's parent
  ///     VPLoop (if any)
  ///   - Blocks in loop SCC are contained in VPLoop
  ///
  /// 2. verifyRegions: It checks that the following invariants are met in
  /// every VPRegionBlock:
  ///   - Entry/Exit is not another region.
  ///   - Entry/Exit has no predecessors/successors, repectively.
  ///   - Non-loop region's Entry (Exit) must have more than two successors
  ///     (predecessors).
  ///   - Size is correct.
  ///   - Blocks' parent is correct.
  ///   - Blocks with multiple successors have a ConditionBitRecipe set.
  ///   - Linked blocks have a bi-directional link (successor/predecessor).
  ///   - All predecessors/successors are inside the region.
  ///   - Blocks have no duplicated successor/predecessor (TODO: switch)
  ///
  void verifyHierarchicalCFG(const VPRegionBlock *TopRegion) const;
};

/// Specialization of VPlanVerifierBase for LLVM-IR. It uses LLVM-IR specific
/// information such as Loop and LoopInfo.
class VPlanVerifier : public VPlanVerifierBase {

private:
  // Outermost LLVM-IR loop to be vectorized.
  const Loop *TheLoop;

  // VPlan-incoming LoopInfo analysis.
  const LoopInfo *LInfo;

  unsigned countLoopsInUnderlyingIR() const;
  void verifyIRSpecificLoopRegion(const VPRegionBlock *Region) const {};

public:
  VPlanVerifier(const Loop *Lp, const LoopInfo *LInfo)
      : VPlanVerifierBase(), TheLoop(Lp), LInfo(LInfo) {}
};

/// Specialization of VPlanVerifierBase for HIR. It uses HIR specific
/// information such as HLLoop.
class VPlanVerifierHIR : public VPlanVerifierBase {

private:
  // Outermost HIR loop to be vectorized.
  const HLLoop *TheLoop;

  unsigned countLoopsInUnderlyingIR() const;
  void verifyIRSpecificLoopRegion(const VPRegionBlock *Region) const;

public:
  VPlanVerifierHIR(const HLLoop *HLLp) : VPlanVerifierBase(), TheLoop(HLLp) {}
};

} // End vpo namespace
} // end llvm namespace

#endif //LLVM_TRANSFORMS_VECTORIZE_VPLAN_VPLANVERIFIER_H
