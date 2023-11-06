//===-- DriverLLVM.h ------------------------------------------------------===//
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
/// \file DriverLLVM.h
/// This file defines the VPlan vectorizer driver pass for LLVM IR.
///
/// Split from Driver.h on 2023-10-26.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_LLVM_DRIVERLLVM_H
#define LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_LLVM_DRIVERLLVM_H

#include "../Driver.h"

namespace llvm {
namespace vpo {

class DriverLLVMImpl : public DriverImpl {
  friend DriverImpl;

private:
  LoopInfo *LI;
  ScalarEvolution *SE;
  AliasAnalysis *AA;
  DemandedBits *DB;
  BlockFrequencyInfo *BFI;
  LoopAccessInfoManager *LAIs;
  OptimizationRemarkEmitter *ORE;

  // PSI is only used by the CG stress testing code that relies on
  // community legality testing.  We have to keep it for now.
  ProfileSummaryInfo *PSI;

  bool processLoop(Loop *Lp, Function &Fn, WRNVecLoopNode *WRLp);
  bool isSupported(Loop *Lp, WRNVecLoopNode *WRLp);
  void collectAllLoops(SmallVectorImpl<Loop *> &Loops);
  bool isVPlanCandidate(Function &Fn, Loop *Lp);
  Loop *adjustLoopIfNeeded(Loop *Lp, BasicBlock *Header);

  // Helper functions for isSupportedLLVM().
  bool hasDedicatedAndUniqueExits(Loop *Lp, WRNVecLoopNode *WRLp);
  bool isSupportedRec(Loop *Lp, WRNVecLoopNode *WRLp);

  bool bailout(VPlanOptReportBuilder &ORBuilder, Loop *Lp, WRNVecLoopNode *WRLp,
               VPlanBailoutRemark RemarkData);

  bool formLCSSAIfNeeded(Loop *Lp);

public:
  bool runImpl(Function &F, LoopInfo *LI, ScalarEvolution *SE,
               DominatorTree *DT, AssumptionCache *AC, AliasAnalysis *AA,
               DemandedBits *DB, LoopAccessInfoManager *LAIs,
               OptimizationRemarkEmitter *ORE,
               OptReportVerbosity::Level Verbosity, WRegionInfo *WR,
               TargetTransformInfo *TTI, TargetLibraryInfo *TLI,
               BlockFrequencyInfo *BFI, ProfileSummaryInfo *PSI,
               VecErrorHandlerTy VecErrorHandler);
};

} // namespace vpo
} // namespace llvm

#endif // LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_LLVM_DRIVERLLVM_H
