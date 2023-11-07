//===-- DriverHIR.h -------------------------------------------------------===//
//
//   INTEL CONFIDENTIAL
//
//   Copyright (C) 2019 Intel Corporation
//
//   This software and the related documents are Intel copyrighted materials,
//   and your use of them is governed by the express license under which they
//   were provided to you ("License").  Unless the License provides otherwise,
//   you may not use, modify, copy, publish, distribute, disclose or treansmit
//   this software or the related documents without Intel's prior written
//   permission.
//
//   This software and the related documents are provided as is, with no
//   express or implied warranties, other than those that are expressly
//   stated in the License.
//
//===----------------------------------------------------------------------===//
//
/// \file DriverHIR.h
/// This file defines the VPlan vectorizer driver pass for HIR.
///
/// Split from Driver.h on 2023-10-26.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_HIR_DRIVERHIR_H
#define LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_HIR_DRIVERHIR_H

#include "../Driver.h"

namespace llvm {
namespace vpo {

class DriverHIRImpl : public DriverImpl {
  friend DriverImpl;

private:
  loopopt::HIRFramework *HIRF;
  loopopt::HIRLoopStatistics *HIRLoopStats;
  loopopt::HIRDDAnalysis *DDA;
  loopopt::HIRSafeReductionAnalysis *SafeRedAnalysis;
  bool LightWeightMode;
  bool WillRunLLVMIRVPlan;

  bool processLoop(loopopt::HLLoop *Lp, Function &Fn, WRNVecLoopNode *WRLp);
  bool isSupported(loopopt::HLLoop *Lp, WRNVecLoopNode *WRLp);
  void collectAllLoops(SmallVectorImpl<loopopt::HLLoop *> &Loops);
  bool isVPlanCandidate(Function &Fn, loopopt::HLLoop *Lp);
  // Delete intel intrinsic directives before/after the given loop.
  void eraseLoopIntrins(loopopt::HLLoop *Lp, WRNVecLoopNode *WRLp);
  bool bailout(VPlanOptReportBuilder &VPORBuilder, loopopt::HLLoop *Lp,
               WRNVecLoopNode *WRLp, VPlanBailoutRemark RemarkData);
  OptRemark prependHIR(OptRemark R);

  // Add remarks to the VPlan loop opt-report for loops created by codegen
  void addOptReportRemarks(WRNVecLoopNode *WRLp,
                           VPlanOptReportBuilder &VPORBuilder,
                           VPOCodeGenHIR *VCodeGen);

public:
  bool runImpl(Function &F, loopopt::HIRFramework *HIRF,
               loopopt::HIRLoopStatistics *HIRLoopStats,
               loopopt::HIRDDAnalysis *DDA,
               loopopt::HIRSafeReductionAnalysis *SafeRedAnalysis,
               OptReportVerbosity::Level Verbosity, WRegionInfo *WR,
               TargetTransformInfo *TTI, TargetLibraryInfo *TLI,
               AssumptionCache *AC, DominatorTree *DT);

  DriverHIRImpl(bool LightWeightMode, bool WillRunLLVMIRVPlan)
      : DriverImpl(), HIRF(nullptr), HIRLoopStats(nullptr), DDA(nullptr),
        SafeRedAnalysis(nullptr), LightWeightMode(LightWeightMode),
        WillRunLLVMIRVPlan(WillRunLLVMIRVPlan){};

  bool lightWeightMode() { return LightWeightMode; }
  bool willRunLLVMIRVPlan() { return WillRunLLVMIRVPlan; }
};

} // namespace vpo
} // namespace llvm

#endif // LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_HIR_DRIVERHIR_H
