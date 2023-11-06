//===-- Driver.h --------------------------------------------------------===//
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
/// \file Driver.h
/// This file defines the VPlan vectorizer driver pass.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_DRIVER_H
#define LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_DRIVER_H

#include "IntelVPlan.h"
#include "llvm/Transforms/Vectorize.h"

namespace llvm {
namespace vpo {

class WRegionInfo;
class LoopVectorizationPlanner;
class WRNVecLoopNode;
class CfgMergerPlanDescr;

extern bool DisableCodeGen;
extern bool ReportLoopNumber;
extern bool EnableOuterLoopHIR;
extern bool VPlanConstrStressTest;
extern unsigned VPlanVectCand;
extern bool VPlanEnablePeelingOpt;
extern bool VPlanEnablePeelingHIROpt;
extern bool VPlanEnableGeneralPeelingOpt;
extern bool VPlanEnableGeneralPeelingHIROpt;
extern bool EmitDebugOptRemarks;
extern bool VPlanPrintInit;
extern bool VPlanPrintAfterSingleTripCountOpt;

#ifndef NDEBUG
extern bool DebugErrHandler;
#endif

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
extern bool PrintHIRBeforeVPlan;
#endif

// Data to be passed to VPlanOptReportBuilder::addRemark().
struct VPlanBailoutRemark {
  OptReportVerbosity::Level BailoutLevel = OptReportVerbosity::High;
  OptRemark BailoutRemark;
};

class DriverImpl {
private:
  template <class Loop>
  bool processLoop(Loop *Lp, Function &Fn, WRNVecLoopNode *WRLp);

  template <class Loop> bool isSupported(Loop *Lp, WRNVecLoopNode *WRLp);

  template <class Loop> void collectAllLoops(SmallVectorImpl<Loop *> &Loops);

  /// Return true if the given loop is a candidate for VPlan vectorization.
  // Currently this function is used in the LLVM IR path to generate VPlan
  // candidates using LoopVectorizationLegality for stress testing the LLVM IR
  // VPlan implementation.
  template <class Loop> bool isVPlanCandidate(Function &Fn, Loop *Lp);

  template <class Loop>
  bool bailout(VPlanOptReportBuilder &ORBuilder, Loop *Lp, WRNVecLoopNode *WRLp,
               VPlanBailoutRemark RemarkData);

  template <class Loop> Loop *adjustLoopIfNeeded(Loop *Lp, BasicBlock *Header);

  template <class Loop> bool formLCSSAIfNeeded(Loop *Lp);

protected:
  DominatorTree *DT;
  AssumptionCache *AC;

  // Hold information regarding explicit vectorization in LLVM-IR.
  WRegionInfo *WR;
  // true if runStandardMode was used for current processFunction and should
  // emit  kernel remarks in this case
  bool isEmitKernelOptRemarks;

  /// Handle to Target Information
  TargetTransformInfo *TTI;
  TargetLibraryInfo *TLI;
  const DataLayout *DL;

  /// Error handler. Is invoked (if set) on bailouts and is passed to CG
  /// to react on unrecoverable errors.
  VecErrorHandlerTy VecErrorHandler;

  OptReportBuilder ORBuilder;
  VPlanBailoutRemark BR;

  template <typename Loop> bool processFunction(Function &Fn);

  // VPlan Driver running modes
  template <typename Loop> bool runStandardMode(Function &Fn);

  template <typename Loop> bool runCGStressTestMode(Function &Fn);

  template <typename Loop> bool runConstructStressTestMode(Function &Fn);

  // Utility to add remarks related to VPlan containing the main vectorized
  // loop.
  void addOptReportRemarksForMainPlan(WRNVecLoopNode *WRLp,
                                      const CfgMergerPlanDescr &MainPlanDescr);

  // Utility to add remarks related to VPlan containing vectorized remainder
  // loop.
  void addOptReportRemarksForVecRemainder(const CfgMergerPlanDescr &PlanDescr);

  // Utility to add remarks related to VPlan containing scalar remainder loop.
  void addOptReportRemarksForScalRemainder(const CfgMergerPlanDescr &PlanDescr);

  // Utility to add remarks related to VPlan containing vectorized peel loop.
  void addOptReportRemarksForVecPeel(const CfgMergerPlanDescr &PlanDescr,
                                     const VPlanPeelingVariant *Variant);

  // Utility to add remarks related to VPlan containing scalar peel loop.
  void addOptReportRemarksForScalPeel(const CfgMergerPlanDescr &PlanDescr,
                                      const VPlanPeelingVariant *Variant);

  // Helper utility to populate all needed analyses in VPlans using the provided
  // factory object.
  void populateVPlanAnalyses(LoopVectorizationPlanner &LVP,
                             VPAnalysesFactoryBase &VPAF);

  // Helper utility to create masked version of VPlans.
  void generateMaskedModeVPlans(LoopVectorizationPlanner *LVP,
                                VPAnalysesFactoryBase *VPAF);

  // Expand f90-dv-buffer-init.
  void preprocessDopeVectorInstructions(VPlanVector *Plan);

  // Expand various private-final instructions.
  void preprocessPrivateFinalCondInstructions(VPlanVector *Plan);

  // Increment vectorized loop count for stress testing.
  void incrementCandLoopsVectorized();

protected:
  AssumptionCache *getAC() const { return AC; }
  void setAC(AssumptionCache *NewAC) { AC = NewAC; }

  DominatorTree *getDT() const { return DT; }
  void setDT(DominatorTree *NewDT) { DT = NewDT; }

  void clearBailoutRemark() { BR.BailoutRemark = OptRemark(); }

  // Store a variadic remark indicating the reason for not vectorizing a loop.
  // Clients should pass string constants as std::string to avoid extra
  // instantiations of this template function.
  template <typename... Args>
  void setBailoutRemark(OptReportVerbosity::Level BailoutLevel,
                        OptRemarkID BailoutID, Args &&...BailoutArgs) {
    BR.BailoutLevel = BailoutLevel;
    BR.BailoutRemark = OptRemark::get(ORBuilder.getContext(), BailoutID,
                                      std::forward<Args>(BailoutArgs)...);
  }

public:
  /// Utility functions for adding/constructing debug remarks.
  template <typename RemarkRecordT, typename... ArgsT>
  static RemarkRecordT getDebugRemark(LLVMContext &C, ArgsT &&...Args) {
    std::string Remark;
    ((llvm::raw_string_ostream{Remark} << std::forward<ArgsT>(Args)), ...);
    return RemarkRecordT{C, OptRemarkID::GenericDebug, Remark};
  }

  template <typename... ArgsT, typename RemarkRecordT>
  static void addDebugRemark(SmallVectorImpl<RemarkRecordT> &Remarks,
                             LLVMContext &C, ArgsT &&...Args) {
    Remarks.push_back(
        getDebugRemark<RemarkRecordT>(C, std::forward<ArgsT>(Args)...));
  }
};

} // namespace vpo
} // namespace llvm

#endif // LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_DRIVER_H
