//===-- IntelVPlanDriver.h ----------------------------------------------===//
//
//   Copyright (C) 2019 Intel Corporation. All rights reserved.
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

#include "IntelVPOLoopAdapters.h"
#include "IntelVPlanHCFGBuilder.h"

namespace llvm {
class FunctionPass;
class TargetTransformInfo;
class TargetLibraryInfo;
class LoopAccessLegacyAnalysis;
class DemandedBits;
class OptimizationRemarkEmitter;
#if INTEL_CUSTOMIZATION
namespace loopopt {
class HIRFramework;
class HIRLoopStatistics;
class HIRDDAnalysis;
} // namespace loopopt
#endif //INTEL_CUSTOMIZATION
} // namespace llvm

namespace llvm {
namespace vpo {

class WRegionInfo;

class VPlanDriver : public FunctionPass {

private:
  LoopInfo *LI;
  ScalarEvolution *SE;
  DominatorTree *DT;
  AssumptionCache *AC;
  AliasAnalysis *AA;
  DemandedBits *DB;
  LoopAccessLegacyAnalysis *LAA;
  OptimizationRemarkEmitter *ORE;
  LoopOptReportBuilder LORBuilder;
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

  /// Handle to Target Information
  TargetTransformInfo *TTI;
  TargetLibraryInfo *TLI;
  const DataLayout *DL;

#if INTEL_CUSTOMIZATION
  // VPlanDriverHIR inherits from VPlanDriver.
  // The VPlanDriver object is created by DirverHIR following empty
  // contructor is invoked and doesn't initialize Driver's data members
  VPlanDriver(char &ID) : FunctionPass(ID){};
#endif //INTEL_CUSTOMIZATION

  void getAnalysisUsage(AnalysisUsage &AU) const override;
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
  void addOptReportRemarks(VPlanOptReportBuilder &VPORBuilder,
                           VPOCodeGenType *VCodeGen);
public:
  static char ID; // Pass identification, replacement for typeid

  VPlanDriver() : FunctionPass(ID) {
    initializeVPlanDriverPass(*PassRegistry::getPassRegistry());
  }

  bool runOnFunction(Function &Fn) override;
};

#if INTEL_CUSTOMIZATION
class VPlanDriverHIR : public VPlanDriver {
  friend VPlanDriver;

private:
  HIRFramework *HIRF;
  HIRLoopStatistics *HIRLoopStats;
  HIRDDAnalysis *DDA;
  LoopOptReportBuilder LORBuilder;
  bool processLoop(HLLoop *Lp, Function &Fn, WRNVecLoopNode *WRLp);
  bool isSupported(HLLoop *Lp);
  void collectAllLoops(SmallVectorImpl<HLLoop *> &Loops);
  bool isVPlanCandidate(Function &Fn, HLLoop *Lp);

public:
  static char ID; // Pass identification, replacement for typeid

  VPlanDriverHIR() : VPlanDriver(ID) {
    initializeVPlanDriverHIRPass(*PassRegistry::getPassRegistry());
  }

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
