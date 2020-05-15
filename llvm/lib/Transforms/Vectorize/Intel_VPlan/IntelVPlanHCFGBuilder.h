//===-- IntelVPlanHCFGBuilder.h ---------------------------------*- C++ -*-===//
//
//   Copyright (C) 2015-2020 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation. and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file defines VPlanHCFGBuilder class that is used to build a
/// hierarchical CFG in VPlan. Further documentation can be found in document
/// 'VPlan Hierarchical CFG Builder'.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTELVPLANHCFGBUILDER_H
#define LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTELVPLANHCFGBUILDER_H

#include "IntelVPlan.h"
#include "IntelVPlanDominatorTree.h"
#include "llvm/ADT/SmallVector.h"

extern bool LoopMassagingEnabled;

namespace llvm {
class ScalarEvolution;
class Loop;

namespace vpo {

  class VPOVectorizationLegality;
  class VPlanVerifier;
  class WRNVecLoopNode;
  class VPlan;

class VPlanHCFGBuilder {
public:
  typedef SmallVector<std::unique_ptr<VPLoopEntitiesConverterBase>, 2>
      VPLoopEntityConverterList;
private:
  /// The outermost loop to be vectorized.
  Loop *TheLoop;

  /// Loop Info analysis.
  LoopInfo *LI;

protected:
  /// Hold WRegion information for TheLoop, if available.
  const WRNVecLoopNode *const WRLp;

  VPlan *Plan = nullptr;

  /// VPlan verifier utility.
  std::unique_ptr<VPlanVerifier> Verifier;

  // TODO: Only used to determine if a condition is uniform. Decouple from
  // Legality.
  // TODO: This must be a reference. Using pointer to support temporal nullptr
  // from HIR.
  /// The legality analysis.
  VPOVectorizationLegality *Legal;

  // Holds instructions from the original loop that we predicated. Such
  // instructions reside in their own conditioned VPBasicBlock and represent
  // an optimization opportunity for sinking their scalarized operands thus
  // reducing their cost by the predicate's probability.
  // SmallPtrSet<Instruction *, 4> PredicatedInstructions;

  // Holds instructions from the original loop whose counterparts in the
  // vectorized loop would be trivially dead if generated. For example,
  // original induction update instructions can become dead because we
  // separately emit induction "steps" when generating code for the new loop.
  // Similarly, we create a new latch condition when setting up the structure
  // of the new loop, so the old one can become dead.
  // SmallPtrSet<Instruction *, 4> DeadInstructions;

  virtual void buildPlainCFG(VPLoopEntityConverterList &Cvts);

  /// Translate loop metadata from underlying IR to VPLoop data structure.
  virtual void populateVPLoopMetadata(VPLoopInfo *VPLInfo);

  virtual void passEntitiesToVPlan(VPLoopEntityConverterList &Cvts);

  void simplifyPlainCFG();
  void splitLoopsPreheader(VPLoop *VPLp);
  void splitLoopsExit(VPLoop *VPLp);

public:
  VPlanHCFGBuilder(Loop *Lp, LoopInfo *LI, const DataLayout &DL,
                   const WRNVecLoopNode *WRL, VPlan *Plan,
                   VPOVectorizationLegality *Legal);

  virtual ~VPlanHCFGBuilder();

  /// Build hierarchical CFG for TheLoop. Update Plan with the resulting H-CFG.
  void buildHierarchicalCFG();

  // So far only emits explict uniform Vector loop iv, but is expected to be
  // extended to include all the peeling/main vector/remainder CFG/phis.
  virtual void emitVecSpecifics();

  // Emit uniform IV for the vector loop and rewrite backedge condition to use
  // it:
  //
  //   header:
  //     %iv = phi [ 0, preheader ], [ iv.next, latch ]
  //
  //   latch:
  //     %iv.next = %iv + VF
  //     %cond = %iv.next `icmp` TripCount
  //     br i1 %cond
  //
  // The order of latch's successors isn't changed which is ensured by selecting
  // proper icmp predicate (eq/ne). Original latch's CondBit is erased if there
  // are no remaining uses of it after the transformation above.
  void emitVectorLoopIV(VPValue *TripCount, VPValue *VF);
};

} // namespace vpo
} // namespace llvm

#endif // LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTELVPLANHCFGBUILDER_H
