//===- Inliner.h - Inliner pass and infrastructure --------------*- C++ -*-===//
// INTEL_CUSTOMIZATION
//
// INTEL CONFIDENTIAL
//
// Modifications, Copyright (C) 2021 Intel Corporation
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you ("License"). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.
//
// end INTEL_CUSTOMIZATION
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_IPO_INLINER_H
#define LLVM_TRANSFORMS_IPO_INLINER_H

#include "llvm/Analysis/CGSCCPassManager.h"
#include "llvm/Analysis/CallGraphSCCPass.h"
#include "llvm/Analysis/InlineAdvisor.h"
#include "llvm/Analysis/InlineCost.h"
#include "llvm/Analysis/LazyCallGraph.h"
#include "llvm/Analysis/Utils/ImportedFunctionsInliningStatistics.h"
#include "llvm/ADT/SmallSet.h"    // INTEL
#include "llvm/IR/PassManager.h"
#include "llvm/Transforms/IPO/Intel_InlineReport.h" // INTEL
#include "llvm/Transforms/IPO/Intel_MDInlineReport.h" // INTEL

namespace llvm {

class AssumptionCacheTracker;
class CallGraph;
class ProfileSummaryInfo;

/// This class contains all of the helper code which is used to perform the
/// inlining operations that do not depend on the policy. It contains the core
/// bottom-up inlining infrastructure that specific inliner passes use.
struct LegacyInlinerBase : public CallGraphSCCPass {
  explicit LegacyInlinerBase(char &ID);
  explicit LegacyInlinerBase(char &ID, bool InsertLifetime);

  /// For this class, we declare that we require and preserve the call graph.
  /// If the derived class implements this method, it should always explicitly
  /// call the implementation here.
  void getAnalysisUsage(AnalysisUsage &Info) const override;

  using llvm::Pass::doInitialization;

  bool doInitialization(CallGraph &CG) override;

  /// Main run interface method, this implements the interface required by the
  /// Pass class.
  bool runOnSCC(CallGraphSCC &SCC) override;

  using llvm::Pass::doFinalization;

  /// Remove now-dead linkonce functions at the end of processing to avoid
  /// breaking the SCC traversal.
  bool doFinalization(CallGraph &CG) override;

  /// This method must be implemented by the subclass to determine the cost of
  /// inlining the specified call site.  If the cost returned is greater than
  /// the current inline threshold, the call site is not inlined.
  virtual InlineCost getInlineCost(CallBase &CB) = 0;

  /// Remove dead functions.
  ///
  /// This also includes a hack in the form of the 'AlwaysInlineOnly' flag
  /// which restricts it to deleting functions with an 'AlwaysInline'
  /// attribute. This is useful for the InlineAlways pass that only wants to
  /// deal with that subset of the functions.
  bool removeDeadFunctions(CallGraph &CG, bool AlwaysInlineOnly = false);

  InlineReport* getReport() { return Report; } // INTEL
  InlineReportBuilder* getMDReport() { return MDReport; } // INTEL

  /// This function performs the main work of the pass.  The default of
  /// Inlinter::runOnSCC() calls skipSCC() before calling this method, but
  /// derived classes which cannot be skipped can override that method and call
  /// this function unconditionally.
  bool inlineCalls(CallGraphSCC &SCC);

private:
  // Insert @llvm.lifetime intrinsics.
  bool InsertLifetime = true;

  // INTEL The inline report
  InlineReport *Report; // INTEL
  InlineReportBuilder *MDReport; // INTEL

protected:
#if INTEL_CUSTOMIZATION
  virtual const InlineParams *getInlineParams() const { return nullptr; }
#endif // INTEL_CUSTOMIZATION
  AssumptionCacheTracker *ACT;
  InliningLoopInfoCache *ILIC; // INTEL
  ProfileSummaryInfo *PSI;
  WholeProgramInfo *WPI;       // INTEL
  std::function<const TargetLibraryInfo &(Function &)> GetTLI;
  ImportedFunctionsInliningStatistics ImportedFunctionsStats;
};

/// The inliner pass for the new pass manager.
///
/// This pass wires together the inlining utilities and the inline cost
/// analysis into a CGSCC pass. It considers every call in every function in
/// the SCC and tries to inline if profitable. It can be tuned with a number of
/// parameters to control what cost model is used and what tradeoffs are made
/// when making the decision.
///
/// It should be noted that the legacy inliners do considerably more than this
/// inliner pass does. They provide logic for manually merging allocas, and
/// doing considerable DCE including the DCE of dead functions. This pass makes
/// every attempt to be simpler. DCE of functions requires complex reasoning
/// about comdat groups, etc. Instead, it is expected that other more focused
/// passes be composed to achieve the same end result.
class InlinerPass : public PassInfoMixin<InlinerPass> {
public:
#if INTEL_CUSTOMIZATION
  InlinerPass(bool OnlyMandatory = false,
              ThinOrFullLTOPhase LTOPhase = ThinOrFullLTOPhase::None);
  ~InlinerPass();
  InlinerPass(InlinerPass &&Arg)
      : OnlyMandatory(Arg.OnlyMandatory), LTOPhase(std::move(Arg.LTOPhase)),
        Report(std::move(Arg.Report)), MDReport(std::move(Arg.MDReport)) {}
#endif // INTEL_CUSTOMIZATION

  PreservedAnalyses run(LazyCallGraph::SCC &C, CGSCCAnalysisManager &AM,
                        LazyCallGraph &CG, CGSCCUpdateResult &UR);
#if INTEL_CUSTOMIZATION
  InlineReport *getReport() { return Report; }
  InlineReportBuilder *getMDReport() { return MDReport; }
  bool getIsAlwaysInline() { return IsAlwaysInline; }
  void setIsAlwaysInline(bool AlwaysInline) { IsAlwaysInline = AlwaysInline; }
#endif // INTEL_CUSTOMIZATION

  void printPipeline(raw_ostream &OS,
                     function_ref<StringRef(StringRef)> MapClassName2PassName);

private:
  InlineAdvisor &getAdvisor(const ModuleAnalysisManagerCGSCCProxy::Result &MAM,
                            FunctionAnalysisManager &FAM, Module &M);
  std::unique_ptr<InlineAdvisor> OwnedAdvisor;
  const bool OnlyMandatory;
  const ThinOrFullLTOPhase LTOPhase;

  // INTEL The inline report
  InlineReport *Report; // INTEL
  InlineReportBuilder *MDReport; // INTEL
  bool IsAlwaysInline; // INTEL
};

/// Module pass, wrapping the inliner pass. This works in conjunction with the
/// InlineAdvisorAnalysis to facilitate inlining decisions taking into account
/// module-wide state, that need to keep track of inter-inliner pass runs, for
/// a given module. An InlineAdvisor is configured and kept alive for the
/// duration of the ModuleInlinerWrapperPass::run.
class ModuleInlinerWrapperPass
    : public PassInfoMixin<ModuleInlinerWrapperPass> {
public:
  ModuleInlinerWrapperPass(
      InlineParams Params = getInlineParams(), bool MandatoryFirst = true,
      InlineContext IC = {},
      InliningAdvisorMode Mode = InliningAdvisorMode::Default,
      unsigned MaxDevirtIterations = 0);
  ModuleInlinerWrapperPass(ModuleInlinerWrapperPass &&Arg) = default;

  PreservedAnalyses run(Module &, ModuleAnalysisManager &);

  /// Allow adding more CGSCC passes, besides inlining. This should be called
  /// before run is called, as part of pass pipeline building.
  CGSCCPassManager &getPM() { return PM; }

  /// Add a module pass that runs before the CGSCC passes.
  template <class T> void addModulePass(T Pass) {
    MPM.addPass(std::move(Pass));
  }

  /// Add a module pass that runs after the CGSCC passes.
  template <class T> void addLateModulePass(T Pass) {
    AfterCGMPM.addPass(std::move(Pass));
  }

  void printPipeline(raw_ostream &OS,
                     function_ref<StringRef(StringRef)> MapClassName2PassName);

private:
  const InlineParams Params;
  const InlineContext IC;
  const InliningAdvisorMode Mode;
  const unsigned MaxDevirtIterations;
  // TODO: Clean this up so we only have one ModulePassManager.
  CGSCCPassManager PM;
  ModulePassManager MPM;
  ModulePassManager AfterCGMPM;
};
} // end namespace llvm

#endif // LLVM_TRANSFORMS_IPO_INLINER_H
