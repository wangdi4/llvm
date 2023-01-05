//===- IntelVPlanVLSAnalysisHIR.h - ---------------------------------------===/
//
//   Copyright (C) 2018-2020 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation. and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This class implements VPlanVLSAnalysis is supposed to work on HIR.
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORM_VECTORIZE_INTEL_VPLAN_INTELVPLANVLSANALYSISHIR_H
#define LLVM_TRANSFORM_VECTORIZE_INTEL_VPLAN_INTELVPLANVLSANALYSISHIR_H

#include "IntelVPlanVLSClientHIR.h"
#include "Intel_VPlan/IntelVPlanCostModel.h"
#include "Intel_VPlan/IntelVPlanVLSAnalysis.h"

namespace llvm {

namespace loopopt {
class HIRDDAnalysis;
class RegDDRef;
} // namespace loopopt

namespace vpo {

class VPlanVLSAnalysisHIR : public VPlanVLSAnalysis {
friend class VPlanVLSAnalysis;

private:
  /// HIRDDAnalysis is used to answer whether one memory reference can be moved
  /// to another one.
  HIRDDAnalysis *DDA;

  // Decomposition doesn't create/copy RegDDRefs, so need to dig into HLDDNode
  // to extract first unmet RegDDRef.
  // This container is used to keep all visited RegDDRefs.
  mutable DenseMap<const HLDDNode *, DenseSet<const RegDDRef *>>
      DDNodeRefs;

  OVLSMemref *createVLSMemref(const VPLoadStoreInst *Inst, const unsigned VF,
                              const VPlanScalarEvolution *VPSE) final override;

  static MemAccessTy getAccessType(const RegDDRef *Ref, const unsigned Level,
                                   int64_t *Stride);

  const HLLoop *TheLoop = nullptr;

public:
  VPlanVLSAnalysisHIR(HIRDDAnalysis *DDA, LLVMContext &Context,
                      const DataLayout &DL, TargetTransformInfo *TTI,
                      const HLLoop *Loop)
      : VPlanVLSAnalysis(/*MainLoop=*/nullptr, Context, DL, TTI), DDA(DDA),
        TheLoop(Loop) {}

  ~VPlanVLSAnalysisHIR() {}
};

} // namespace vpo
} // namespace llvm

#endif // LLVM_TRANSFORM_VECTORIZE_INTEL_VPLAN_INTELVLSANALYSIS_H
