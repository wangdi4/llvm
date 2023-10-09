//===-- IntelVPlanHCFGBuilder.h ---------------------------------*- C++ -*-===//
//
//   Copyright (C) 2015 Intel Corporation. All rights reserved.
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

namespace llvm {
class ScalarEvolution;
class Loop;

namespace vpo {

extern bool VPlanPrintLegality;

class LegalityLLVM;
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

  /// Block frequency info for CFG build
  BlockFrequencyInfo *BFI;

protected:
  /// Hold WRegion information for TheLoop, if available.
  const WRNVecLoopNode *const WRLp;

  VPlanVector *Plan = nullptr;

  /// VPlan verifier utility.
  std::unique_ptr<VPlanVerifier> Verifier;

  // TODO: Only used to determine if a condition is uniform. Decouple from
  // Legality.
  // TODO: This must be a reference. Using pointer to support temporal nullptr
  // from HIR.
  /// The legality analysis.
  LegalityLLVM *Legal;

  ScalarEvolution *SE;

  const DominatorTree &DT;

  AssumptionCache &AC;

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

  virtual bool buildPlainCFG(VPLoopEntityConverterList &Cvts);

  /// Translate loop metadata from underlying IR to VPLoop data structure.
  virtual void populateVPLoopMetadata(VPLoopInfo *VPLInfo);

  virtual void passEntitiesToVPlan(VPLoopEntityConverterList &Cvts);

public:
  VPlanHCFGBuilder(Loop *Lp, LoopInfo *LI, const DataLayout &DL,
                   const WRNVecLoopNode *WRL, VPlanVector *Plan,
                   LegalityLLVM *Legal, AssumptionCache &AC,
                   const DominatorTree &DT, ScalarEvolution *SE = nullptr,
                   BlockFrequencyInfo *BFI = nullptr);

  virtual ~VPlanHCFGBuilder();

  /// Build hierarchical CFG for TheLoop. Update Plan with the resulting H-CFG.
  bool buildHierarchicalCFG();
};

} // namespace vpo
} // namespace llvm

#endif // LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTELVPLANHCFGBUILDER_H
