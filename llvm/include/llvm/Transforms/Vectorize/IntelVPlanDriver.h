//===-- IntelVPlanDriver.h ----------------------------------------------===//
//
//   Copyright (C) 2019-2023 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//
//
// This file defines VPlan vectorizer driver pass.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTELVPLANDRIVER_H
#define LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTELVPLANDRIVER_H

#include "llvm/Analysis/Intel_LoopAnalysis/Framework/HIRFramework.h"
#include "llvm/Analysis/Intel_OptReport/OptReport.h"
#include "llvm/Analysis/VectorUtils.h"
#include "llvm/Transforms/Intel_LoopTransforms/HIRTransformPass.h"
#include "llvm/Transforms/Vectorize.h"

namespace llvm {
namespace vpo {

class WRegionInfo;
class VPlanOptReportBuilder;
class VPAnalysesFactoryBase;
class LoopVectorizationPlanner;
class WRNVecLoopNode;
class VPlanPeelingVariant;
class VPLoop;
class VPLoopInfo;
class CfgMergerPlanDescr;

// Data to be passed to VPlanOptReportBuilder::addRemark().
struct VPlanBailoutRemark {
  OptReportVerbosity::Level BailoutLevel = OptReportVerbosity::High;
  OptRemark BailoutRemark;
};

class VPlanDriverImpl {
private:
  LoopInfo *LI;
  ScalarEvolution *SE;
  DominatorTree *DT;
  AssumptionCache *AC;
  AliasAnalysis *AA;
  DemandedBits *DB;
  BlockFrequencyInfo *BFI;
  ProfileSummaryInfo *PSI;
  LoopAccessInfoManager *LAIs;
  OptimizationRemarkEmitter *ORE;
  FatalErrorHandlerTy FatalErrorHandler;

  template <class Loop>
  bool processLoop(Loop *Lp, Function &Fn, WRNVecLoopNode *WRLp);

  template <class Loop>
  bool isSupported(Loop *Lp, WRNVecLoopNode *WRLp);

  template <class Loop>
  void collectAllLoops(SmallVectorImpl<Loop *> &Loops);

  /// Return true if the given loop is a candidate for VPlan vectorization.
  // Currently this function is used in the LLVM IR path to generate VPlan
  // candidates using LoopVectorizationLegality for stress testing the LLVM IR
  // VPlan implementation.
  template <class Loop>
  bool isVPlanCandidate(Function &Fn, Loop *Lp);

  template <class Loop>
  bool bailout(VPlanOptReportBuilder &ORBuilder, Loop *Lp, WRNVecLoopNode *WRLp,
               VPlanBailoutRemark RemarkData);

  // Helper functions for isSupported().
  bool hasDedicatedAndUniqueExits(Loop *Lp, WRNVecLoopNode *WRLp);
  bool isSupportedRec(Loop *Lp, WRNVecLoopNode *WRLp);

protected:
  // Hold information regarding explicit vectorization in LLVM-IR.
  WRegionInfo *WR;
  // true if runStandardMode was used for current processFunction and should
  // emit  kernel remarks in this case
  bool isEmitKernelOptRemarks;

  /// Handle to Target Information
  TargetTransformInfo *TTI;
  TargetLibraryInfo *TLI;
  const DataLayout *DL;

  OptReportBuilder ORBuilder;
  VPlanBailoutRemark BR;

  template <typename Loop = llvm::Loop>
  bool processFunction(Function &Fn);

  // VPlan Driver running modes
  template <typename Loop = llvm::Loop>
  bool runStandardMode(Function &Fn);

  template <typename Loop = Loop>
  bool runCGStressTestMode(Function &Fn);

  template <typename Loop = Loop>
  bool runConstructStressTestMode(Function &Fn);

  // Add remarks to the VPlan loop opt-report for loops created by codegen
  template <class VPOCodeGenType, typename Loop = Loop>
  void addOptReportRemarks(WRNVecLoopNode *WRLp,
                           VPlanOptReportBuilder &VPORBuilder,
                           VPOCodeGenType *VCodeGen);

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
                        unsigned BailoutID, Args &&...BailoutArgs) {
    BR.BailoutLevel = BailoutLevel;
    BR.BailoutRemark = OptRemark::get(ORBuilder.getContext(), BailoutID,
                                      OptReportDiag::getMsg(BailoutID),
                                      std::forward<Args>(BailoutArgs)...);
  }

public:
  bool runImpl(Function &F, LoopInfo *LI, ScalarEvolution *SE,
               DominatorTree *DT, AssumptionCache *AC, AliasAnalysis *AA,
               DemandedBits *DB, LoopAccessInfoManager *LAIs,
               OptimizationRemarkEmitter *ORE,
               OptReportVerbosity::Level Verbosity, WRegionInfo *WR,
               TargetTransformInfo *TTI, TargetLibraryInfo *TLI,
               BlockFrequencyInfo *BFI, ProfileSummaryInfo *PSI,
               FatalErrorHandlerTy FatalErrorHandler);

  /// Whether to emit debug remarks into the opt report.
  static inline bool EmitDebugOptRemarks = false;

  // Remark IDs are defined in lib/Analysis/Intel_OptReport/Diag.cpp:

  /// 15305: vectorization support: vector length %s
  static constexpr unsigned VectorLengthRemarkID = 15305;

  /// 15313: %s was not vectorized: unsupported data type
  static constexpr unsigned BadTypeRemarkID = 15313;

  /// 15315: %s was not vectorized: low trip count
  static constexpr unsigned LowTripCountRemarkID = 15315;

  /// 15330: %s was not vectorized: the reduction operator is not supported yet
  static constexpr unsigned BadRednRemarkID = 15330;

  /// 15332: %s was not vectorized: loop is not within user-defined range
  static constexpr unsigned OutOfRangeRemarkID = 15332;

  /// 15335: %s was not vectorized: vectorization possible but seems
  ///        inefficient. Use vector always directive or -vec-threshold0 to
  ///        override
  static constexpr unsigned NoProfitRemarkID = 15335;

  /// 15353: loop was not vectorized: loop is not in canonical form from
  ///        OpenMP specification, may be as a result of previous
  ///        optimization(s)
  static constexpr unsigned BadSimdRemarkID = 15353;

  /// 15407: vectorization support: type complex float is not supported for
  ///        operation %s
  static constexpr unsigned CmplxFltRemarkID = 15407;

  /// 15408: vectorization support: type complex double is not supported for
  ///        operation %s
  static constexpr unsigned CmplxDblRemarkID = 15408;

  /// 15436: loop was not vectorized: %s
  static constexpr unsigned BailoutRemarkID = 15436;

  /// 15437: peel loop was vectorized
  static constexpr unsigned PeelLoopWasVectorizedRemarkID = 15437;

  /// 15439: remainder loop was vectorized (unmasked)
  static constexpr unsigned RemainderLoopVectorizedUnmaskedRemarkID = 15439;

  /// 15440: remainder loop was vectorized (masked)
  static constexpr unsigned RemainderLoopVectorizedMaskedRemarkID = 15440;

  /// 15520: %s was not vectorized: loop with multiple exits cannot be
  ///        vectorized unless it meets search loop idiom criteria
  static constexpr unsigned BadSearchRemarkID = 15520;

  /// 15521: %s was not vectorized: loop control variable was not identified.
  ///        Explicitly compute the iteration count before executing the loop
  ///        or try using canonical loop form from OpenMP specification%s
  static constexpr unsigned LoopIVRemarkID = 15521;

  /// 15522: %s was not vectorized: loop control flow is too complex. Try
  ///        using canonical loop form from OpenMP specification%s
  static constexpr unsigned ComplexFlowRemarkID = 15522;

  /// 15535: %s was not vectorized: loop contains switch statement. Consider
  ///        using if-else statement.
  static constexpr unsigned SwitchRemarkID = 15535;

  /// 15560: Indirect call cannot be vectorized
  static constexpr unsigned IndCallRemarkID = 15560;

  /// 15571: %s was not vectorized: loop contains a recurrent computation that
  ///        could not be identified as an induction or reduction.  Try using
  ///        #pragma omp simd reduction/linear/private to clarify recurrence.
  static constexpr unsigned BadRecurPhiRemarkID = 15571;

  /// 15572: %s was not vectorized: loop contains a live-out value that could
  ///        not be identified as an induction or reduction.  Try using #pragma
  ///        omp simd reduction/linear/private to clarify recurrence.
  static constexpr unsigned BadLiveOutRemarkID = 15572;

  /// 15573: %s was not vectorized: a reduction or induction of a vector type
  ///        is not supported.
  static constexpr unsigned VecTypeRednRemarkID = 15573;

  /// 15574: %s was not vectorized: unsupported nested OpenMP (simd) loop or
  ///        region.
  static constexpr unsigned NestedSimdRemarkID = 15574;

  /// 15575: peel loop is static
  static constexpr unsigned PeelLoopIsStaticRemarkID = 15575;

  /// 15576: peel loop is dynamic
  static constexpr unsigned PeelLoopIsDynamicRemarkID = 15576;

  /// 15577: estimated number of scalar loop iterations peeled: %s
  static constexpr unsigned EstimatedPeelCountRemarkID = 15577;

  /// 15578: DEBUG: %s
  static constexpr unsigned DebugRemarkID = 15578;

  /// 25518: Peel loop for vectorization
  static constexpr unsigned PeelLoopForVectorizationRemarkID = 25518;

  /// 25519: Remainder loop for vectorization
  static constexpr unsigned RemainderLoopForVectorizationRemarkID = 25519;

  /// Utility functions for adding/constructing debug remarks.
  template <typename RemarkRecordT, typename... ArgsT>
  static RemarkRecordT getDebugRemark(ArgsT &&...Args) {
    std::string Remark;
    ((llvm::raw_string_ostream{Remark} << std::forward<ArgsT>(Args)), ...);
    return RemarkRecordT{VPlanDriverImpl::DebugRemarkID, std::move(Remark)};
  }

  template <typename... ArgsT, typename RemarkRecordT>
  static void addDebugRemark(SmallVectorImpl<RemarkRecordT> &Remarks,
                             ArgsT &&...Args) {
    Remarks.push_back(
        getDebugRemark<RemarkRecordT>(std::forward<ArgsT>(Args)...));
  }
};

class VPlanDriverPass : public PassInfoMixin<VPlanDriverPass> {
  VPlanDriverImpl Impl;
  static bool RunForSycl;
  static bool RunForO0;

public:
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &AM);
  static void setRunForSycl(bool isSycl) { RunForSycl = isSycl; }
  static void setRunForO0(bool isO0Vec) { RunForO0 = isO0Vec; }
  static bool isRequired() {
    return (RunForSycl || RunForO0);
    }
};

class VPlanDriver : public FunctionPass {
  VPlanDriverImpl Impl;
  FatalErrorHandlerTy FatalErrorHandler;

protected:
  void getAnalysisUsage(AnalysisUsage &AU) const override;

public:
  static char ID; // Pass identification, replacement for typeid
  VPlanDriver(FatalErrorHandlerTy FatalErrorHandler = nullptr);

  bool runOnFunction(Function &Fn) override;
  bool skipFunction(const Function &F) const override;
};

class VPlanDriverHIRImpl : public VPlanDriverImpl {
  friend VPlanDriverImpl;

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

public:
  bool runImpl(Function &F, loopopt::HIRFramework *HIRF,
               loopopt::HIRLoopStatistics *HIRLoopStats,
               loopopt::HIRDDAnalysis *DDA,
               loopopt::HIRSafeReductionAnalysis *SafeRedAnalysis,
               OptReportVerbosity::Level Verbosity, WRegionInfo *WR,
               TargetTransformInfo *TTI, TargetLibraryInfo *TLI,
               AssumptionCache *AC, DominatorTree *DT,
               FatalErrorHandlerTy FatalErrorHandler);

  VPlanDriverHIRImpl(bool LightWeightMode, bool WillRunLLVMIRVPlan)
      : VPlanDriverImpl(), HIRF(nullptr), HIRLoopStats(nullptr), DDA(nullptr),
        SafeRedAnalysis(nullptr), LightWeightMode(LightWeightMode),
        WillRunLLVMIRVPlan(WillRunLLVMIRVPlan){};
};

class VPlanDriverHIRPass
    : public loopopt::HIRPassInfoMixin<VPlanDriverHIRPass> {
  VPlanDriverHIRImpl Impl;

public:
  static constexpr auto PassName = "hir-vplan-vec";
  PreservedAnalyses runImpl(Function &F, FunctionAnalysisManager &AM,
                            loopopt::HIRFramework &);
  VPlanDriverHIRPass(bool LightWeightMode, bool WillRunLLVMIRVPlan)
      : Impl(LightWeightMode, WillRunLLVMIRVPlan){};
};

class VPlanDriverHIR : public FunctionPass {
  VPlanDriverHIRImpl Impl;

public:
  static char ID; // Pass identification, replacement for typeid

  VPlanDriverHIR(bool LightWeightMode = false, bool WillRunLLVMIRVPlan = true);

  void getAnalysisUsage(AnalysisUsage &AU) const override;

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  /// \brief Overrides FunctionPass's printer pass to return one which prints
  /// HIR instead of LLVM IR.
  FunctionPass *createPrinterPass(raw_ostream &OS,
                                  const std::string &Banner) const override;
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

  bool runOnFunction(Function &Fn) override;
};

} // namespace vpo
} // namespace llvm

#endif
