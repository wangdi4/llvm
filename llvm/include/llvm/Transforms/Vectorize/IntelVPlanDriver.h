//===-- IntelVPlanDriver.h ----------------------------------------------===//
//
//   Copyright (C) 2019-2021 Intel Corporation. All rights reserved.
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

#include "llvm/Analysis/Intel_LoopAnalysis/Framework/HIRFramework.h" // INTEL
#include "llvm/Analysis/VectorUtils.h"
#include "llvm/Transforms/Intel_LoopTransforms/HIRTransformPass.h" // INTEL
#include "llvm/Transforms/Vectorize.h" // INTEL

namespace llvm {
namespace vpo {

class WRegionInfo;
class VPlanOptReportBuilder;
class VPAnalysesFactoryBase;
class LoopVectorizationPlanner;
class WRNVecLoopNode;
class VPLoop;
class VPLoopInfo;
class CfgMergerPlanDescr;

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
  OptReportBuilder ORBuilder;
  FatalErrorHandlerTy FatalErrorHandler;

#if INTEL_CUSTOMIZATION
  template <class Loop>
#endif // INTEL_CUSTOMIZATION
  bool processLoop(Loop *Lp, Function &Fn, WRNVecLoopNode *WRLp);

#if INTEL_CUSTOMIZATION
  template <class Loop>
#endif // INTEL_CUSTOMIZATION
  bool isSupported(Loop *Lp);

#if INTEL_CUSTOMIZATION
  template <class Loop>
#endif // INTEL_CUSTOMIZATION
  void collectAllLoops(SmallVectorImpl<Loop *> &Loops);

  /// Return true if the given loop is a candidate for VPlan vectorization.
#if INTEL_CUSTOMIZATION
  // Currently this function is used in the LLVM IR path to generate VPlan
  // candidates using LoopVectorizationLegality for stress testing the LLVM IR
  // VPlan implementation.
  template <class Loop>
#endif // INTEL_CUSTOMIZATION
  bool isVPlanCandidate(Function &Fn, Loop *Lp);

#if INTEL_CUSTOMIZATION
protected:
  // Hold information regarding explicit vectorization in LLVM-IR.
#endif //INTEL_CUSTOMIZATION
  WRegionInfo *WR;
  // true if runStandardMode was used for current processFunction and should
  // emit  kernel remarks in this case
  bool isEmitKernelOptRemarks;

  /// Handle to Target Information
  TargetTransformInfo *TTI;
  TargetLibraryInfo *TLI;
  const DataLayout *DL;

#if INTEL_CUSTOMIZATION
  template <typename Loop = llvm::Loop>
  bool processFunction(Function &Fn);

  // VPlan Driver running modes
  template <typename Loop = llvm::Loop>
  bool runStandardMode(Function &Fn);

  template <typename Loop = Loop>
  bool runCGStressTestMode(Function &Fn);

  template <typename Loop = Loop>
  bool runConstructStressTestMode(Function &Fn);

#else
  bool processFunction(Function &Fn);
  // VPlan Driver running modes
  bool runStandardMode(Function &Fn);
  bool runCGStressTestMode(Function &Fn);
  bool runConstructStressTestMode(Function &Fn);
#endif // INTEL_CUSTOMIZATION

  // Add remarks to the VPlan loop opt-report for loops created by codegen
#if INTEL_CUSTOMIZATION
  template <class VPOCodeGenType, typename Loop = Loop>
#else
  template <class VPOCodeGenType>
#endif //INTEL_CUSTOMIZATION
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
  void addOptReportRemarksForVecPeel(const CfgMergerPlanDescr &PlanDescr);

  // Utility to add remarks related to VPlan containing scalar peel loop.
  void addOptReportRemarksForScalPeel(const CfgMergerPlanDescr &PlanDescr);

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

public:
  bool runImpl(Function &F, LoopInfo *LI, ScalarEvolution *SE,
               DominatorTree *DT, AssumptionCache *AC, AliasAnalysis *AA,
               DemandedBits *DB,
               LoopAccessInfoManager *LAIs,
               OptimizationRemarkEmitter *ORE,
               OptReportVerbosity::Level Verbosity, WRegionInfo *WR,
               TargetTransformInfo *TTI, TargetLibraryInfo *TLI,
               BlockFrequencyInfo *BFI, ProfileSummaryInfo *PSI,
               FatalErrorHandlerTy FatalErrorHandler);
};

class VPlanDriverPass : public PassInfoMixin<VPlanDriverPass> {
  VPlanDriverImpl Impl;

public:
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &AM);
};

class VPlanDriver : public FunctionPass {
  VPlanDriverImpl Impl;
  FatalErrorHandlerTy FatalErrorHandler; // INTEL

protected:
  void getAnalysisUsage(AnalysisUsage &AU) const override;

public:
  static char ID; // Pass identification, replacement for typeid
  VPlanDriver(FatalErrorHandlerTy FatalErrorHandler = nullptr); // INTEL

  bool runOnFunction(Function &Fn) override;
  bool skipFunction(const Function &F) const override;
};

#if INTEL_CUSTOMIZATION
class VPlanDriverHIRImpl : public VPlanDriverImpl {
  friend VPlanDriverImpl;

private:
  loopopt::HIRFramework *HIRF;
  loopopt::HIRLoopStatistics *HIRLoopStats;
  loopopt::HIRDDAnalysis *DDA;
  loopopt::HIRSafeReductionAnalysis *SafeRedAnalysis;
  OptReportBuilder ORBuilder;
  bool LightWeightMode;

  bool processLoop(loopopt::HLLoop *Lp, Function &Fn, WRNVecLoopNode *WRLp);
  bool isSupported(loopopt::HLLoop *Lp);
  void collectAllLoops(SmallVectorImpl<loopopt::HLLoop *> &Loops);
  bool isVPlanCandidate(Function &Fn, loopopt::HLLoop *Lp);
  // Delete intel intrinsic directives before/after the given loop.
  void eraseLoopIntrins(loopopt::HLLoop *Lp, WRNVecLoopNode *WRLp);

public:
  bool runImpl(Function &F, loopopt::HIRFramework *HIRF,
               loopopt::HIRLoopStatistics *HIRLoopStats,
               loopopt::HIRDDAnalysis *DDA,
               loopopt::HIRSafeReductionAnalysis *SafeRedAnalysis,
               OptReportVerbosity::Level Verbosity, WRegionInfo *WR,
               TargetTransformInfo *TTI, TargetLibraryInfo *TLI,
               AssumptionCache *AC, DominatorTree *DT,
               FatalErrorHandlerTy FatalErrorHandler);

  VPlanDriverHIRImpl(bool LightWeightMode)
      : VPlanDriverImpl(), HIRF(nullptr), HIRLoopStats(nullptr),
        DDA(nullptr), SafeRedAnalysis(nullptr), LightWeightMode(LightWeightMode){};
};

class VPlanDriverHIRPass
    : public loopopt::HIRPassInfoMixin<VPlanDriverHIRPass> {
  VPlanDriverHIRImpl Impl;

public:
  static constexpr auto PassName = "hir-vplan-vec";
  PreservedAnalyses runImpl(Function &F, FunctionAnalysisManager &AM,
                            loopopt::HIRFramework &);
  VPlanDriverHIRPass(bool LightWeightMode) : Impl(LightWeightMode) {};
};

class VPlanDriverHIR : public FunctionPass {
  VPlanDriverHIRImpl Impl;

public:
  static char ID; // Pass identification, replacement for typeid

  VPlanDriverHIR(bool LightWeightMode = false);

  void getAnalysisUsage(AnalysisUsage &AU) const override;

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  /// \brief Overrides FunctionPass's printer pass to return one which prints
  /// HIR instead of LLVM IR.
  FunctionPass *createPrinterPass(raw_ostream &OS,
                                  const std::string &Banner) const override;
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

  bool runOnFunction(Function &Fn) override;
};
#endif //INTEL_CUSTOMIZATION

} // namespace vpo
} // namespace llvm

#endif
