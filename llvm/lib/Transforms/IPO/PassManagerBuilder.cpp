//===- PassManagerBuilder.cpp - Build Standard Pass -----------------------===//
// INTEL_CUSTOMIZATION
//
// INTEL CONFIDENTIAL
//
// Modifications, Copyright (C) 2021-2022 Intel Corporation
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
//
// This file defines the PassManagerBuilder class, which is used to set up a
// "standard" optimization sequence suitable for languages like C and C++.
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#include "llvm-c/Transforms/PassManagerBuilder.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/Analysis/CFLAndersAliasAnalysis.h"
#include "llvm/Analysis/CFLSteensAliasAnalysis.h"
#include "llvm/Analysis/GlobalsModRef.h"
#if INTEL_CUSTOMIZATION
#include "llvm/Analysis/Intel_Andersens.h"
#include "llvm/Analysis/Intel_StdContainerAA.h"
#include "llvm/Analysis/Intel_WP.h"
#include "llvm/Analysis/Intel_XmainOptLevelPass.h"
#include "llvm/Analysis/Intel_OptReport/OptReportOptionsPass.h"
#include "llvm/Analysis/InlineCost.h"
#include "llvm/IR/Verifier.h"
#endif // INTEL_CUSTOMIZATION
#include "llvm/Analysis/Passes.h"
#include "llvm/Analysis/ScopedNoAliasAA.h"
#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/Analysis/TypeBasedAliasAnalysis.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/PrintPasses.h" // INTEL
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/ManagedStatic.h"
#include "llvm/Target/CGPassBuilderOption.h"
#include "llvm/Transforms/AggressiveInstCombine/AggressiveInstCombine.h"
#include "llvm/Transforms/IPO.h"
#include "llvm/Transforms/IPO/Attributor.h"
#include "llvm/Transforms/IPO/ForceFunctionAttrs.h"
#include "llvm/Transforms/IPO/FunctionAttrs.h"
#include "llvm/Transforms/IPO/InferFunctionAttrs.h"
#include "llvm/Transforms/InstCombine/InstCombine.h"
#include "llvm/Transforms/Instrumentation.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Scalar/GVN.h"
#include "llvm/Transforms/Scalar/LICM.h"
#include "llvm/Transforms/Scalar/LoopUnrollPass.h"
#include "llvm/Transforms/Scalar/SimpleLoopUnswitch.h"
#include "llvm/Transforms/Utils.h"
#include "llvm/Transforms/Vectorize.h"
#if INTEL_CUSTOMIZATION
#include "llvm/Transforms/Instrumentation/Intel_FunctionSplitting.h"
#include "llvm/Transforms/Intel_LoopTransforms/Passes.h"
#include "llvm/IR/IRPrintingPasses.h"
#include "llvm/Transforms/Utils/Intel_VecClone.h"
#include "llvm/Transforms/Intel_MapIntrinToIml/MapIntrinToIml.h"
#include "llvm/Transforms/IPO/AlwaysInliner.h"
#include "llvm/Transforms/IPO/Inliner.h"
#if INTEL_FEATURE_SW_DTRANS
#include "llvm/Transforms/IPO/Intel_FoldWPIntrinsic.h"
#endif // INTEL_FEATURE_SW_DTRANS
#include "llvm/Transforms/IPO/Intel_InlineLists.h"
#include "llvm/Transforms/IPO/Intel_InlineReportEmitter.h"
#include "llvm/Transforms/IPO/Intel_InlineReportSetup.h"
#include "llvm/Transforms/IPO/Intel_MathLibrariesDeclaration.h"
#include "llvm/Transforms/IPO/Intel_OptimizeDynamicCasts.h"
#include "llvm/Transforms/Scalar/Intel_DopeVectorHoist.h"
#include "llvm/Transforms/Scalar/Intel_LoopAttrs.h"
#include "llvm/Transforms/Scalar/Intel_MultiVersioning.h"
#include "llvm/Transforms/Utils/UnifyFunctionExitNodes.h"

#if INTEL_FEATURE_SW_DTRANS
#include "Intel_DTrans/DTransCommon.h"
#include "Intel_DTrans/DTransPasses.h"
#endif // INTEL_FEATURE_SW_DTRANS
#if INTEL_FEATURE_CSA
#include "Intel_CSA/CSAIRPasses.h"
#endif  // INTEL_FEATURE_CSA
#endif //INTEL_CUSTOMIZATION

#if INTEL_COLLAB
#include "llvm/Transforms/VPO/VPOPasses.h"
#include "llvm/Transforms/VPO/Paropt/VPOParopt.h"
#endif // INTEL_COLLAB

using namespace llvm;

#if INTEL_CUSTOMIZATION

#define INTEL_LIMIT_BEGIN(METHOD, PM)                                          \
  {                                                                            \
    auto LimitedPM = METHOD(PM);                                               \
    {                                                                          \
      auto &PM = LimitedPM;
#define INTEL_LIMIT_END                                                        \
  }                                                                            \
  }

using namespace llvm::llvm_intel_wp_analysis;

extern cl::opt<unsigned> IntelInlineReportLevel;

cl::opt<bool> ConvertToSubs(
    "convert-to-subs-before-loopopt", cl::init(false), cl::ReallyHidden,
    cl::desc("Enables conversion of GEPs to subscripts before loopopt"));

cl::opt<bool>
EnableLV("enable-lv", cl::init(false), cl::Hidden,
         cl::desc("Enable community loop vectorizer"));

cl::opt<bool> EnableLoadCoalescing("enable-load-coalescing", cl::init(true),
                                   cl::Hidden, cl::ZeroOrMore,
                                   cl::desc("Enable load coalescing"));

cl::opt<bool>
    EnableSROAAfterSLP("enable-sroa-after-slp", cl::init(true), cl::Hidden,
                       cl::desc("Run SROA pass after the SLP vectorizer"));

enum class ThroughputMode { None, SingleJob, MultipleJob };
cl::opt<ThroughputMode> ThroughputModeOpt(
    "throughput-opt", cl::init(ThroughputMode::None), cl::Hidden,
    cl::ValueOptional,
    cl::desc(
        "Specifies if compiler should optimize for throughput performance"),
    cl::values(clEnumValN(ThroughputMode::None, "0", "No mode speficied"),
               clEnumValN(ThroughputMode::SingleJob, "1",
                          "Assume application will run a single copy"),
               clEnumValN(ThroughputMode::MultipleJob, "2",
                          "Assume application will run multiple copies")));

#endif // INTEL_CUSTOMIZATION

namespace llvm {
cl::opt<bool> RunPartialInlining("enable-partial-inlining", cl::Hidden,
                                 cl::desc("Run Partial inlinining pass"));

#if INTEL_CUSTOMIZATION
// Enable partial inlining during LTO
static cl::opt<bool>
    RunLTOPartialInlining("enable-lto-partial-inlining", cl::init(true),
                          cl::Hidden, cl::ZeroOrMore,
                          cl::desc("Run LTO Partial inlinining pass"));
#endif // INTEL_CUSTOMIZATION

static cl::opt<bool>
UseGVNAfterVectorization("use-gvn-after-vectorization",
  cl::init(false), cl::Hidden,
  cl::desc("Run GVN instead of Early CSE after vectorization passes"));

cl::opt<bool> ExtraVectorizerPasses(
    "extra-vectorizer-passes", cl::init(false), cl::Hidden,
    cl::desc("Run cleanup optimization passes after vectorization."));

static cl::opt<bool>
RunLoopRerolling("reroll-loops", cl::Hidden,
                 cl::desc("Run the loop rerolling pass"));

cl::opt<bool> SYCLOptimizationMode("sycl-opt", cl::init(false), cl::Hidden,
                                   cl::desc("Enable SYCL optimization mode."));

cl::opt<bool> RunNewGVN("enable-newgvn", cl::init(false), cl::Hidden,
                        cl::desc("Run the NewGVN pass"));

// Experimental option to use CFL-AA
static cl::opt<::CFLAAType>
    UseCFLAA("use-cfl-aa", cl::init(::CFLAAType::None), cl::Hidden,
             cl::desc("Enable the new, experimental CFL alias analysis"),
             cl::values(clEnumValN(::CFLAAType::None, "none", "Disable CFL-AA"),
                        clEnumValN(::CFLAAType::Steensgaard, "steens",
                                   "Enable unification-based CFL-AA"),
                        clEnumValN(::CFLAAType::Andersen, "anders",
                                   "Enable inclusion-based CFL-AA"),
                        clEnumValN(::CFLAAType::Both, "both",
                                   "Enable both variants of CFL-AA")));

cl::opt<bool> EnableLoopInterchange(
    "enable-loopinterchange", cl::init(false), cl::Hidden,
    cl::desc("Enable the experimental LoopInterchange Pass"));

cl::opt<bool> EnableUnrollAndJam("enable-unroll-and-jam", cl::init(false),
                                 cl::Hidden,
                                 cl::desc("Enable Unroll And Jam Pass"));

cl::opt<bool> EnableLoopFlatten("enable-loop-flatten", cl::init(false),
                                       cl::Hidden,
                                       cl::desc("Enable the LoopFlatten Pass"));
#if INTEL_COLLAB
enum { InvokeParoptBeforeInliner = 1, InvokeParoptAfterInliner };
cl::opt<unsigned> RunVPOOpt("vpoopt", cl::init(InvokeParoptAfterInliner),
                            cl::Hidden, cl::desc("Runs all VPO passes"));

// The user can use -mllvm -paropt=<mode> to enable various paropt
// transformations, where <mode> is a bit vector (see enum VPOParoptMode
// for a description of the bits.) For example, paropt=0x7 enables
// "ParPrepare" (0x1), "ParTrans" (0x2), and "OmpPar" (0x4).
// TODO: this does not seem to work with the new pass manager,
//       so we need to fix it soon.
cl::opt<unsigned> RunVPOParopt("paropt", cl::init(0x00000000), cl::Hidden,
                               cl::desc("Run VPO Paropt Pass"));
cl::opt<bool>
    SPIRVOptimizationMode("spirv-opt", cl::init(false), cl::Hidden,
                          cl::desc("Enable SPIR-V optimization mode."));
#endif // INTEL_COLLAB

#if INTEL_CUSTOMIZATION
cl::opt<bool> RunVPOVecopt("vecopt", cl::init(false), cl::Hidden,
                           cl::desc("Run VPO Vecopt Pass"));

// Switch to enable or disable all VPO related pre-loopopt passes
cl::opt<bool> RunPreLoopOptVPOPasses("pre-loopopt-vpo-passes", cl::init(false),
                                     cl::Hidden,
                                     cl::desc("Run VPO passes before loopot"));

// Switch to enable or disable all VPO related post-loopopt passes
cl::opt<bool> RunPostLoopOptVPOPasses("post-loopopt-vpo-passes", cl::init(true),
                                      cl::Hidden,
                                      cl::desc("Run VPO passes after loopot"));

// Set LLVM-IR VPlan driver pass to be enabled by default
cl::opt<bool> EnableVPlanDriver("vplan-driver", cl::init(true), cl::Hidden,
                                cl::desc("Enable VPlan Driver"));

cl::opt<bool> RunVecClone("enable-vec-clone", cl::init(true), cl::Hidden,
                          cl::desc("Run Vector Function Cloning"));

cl::opt<bool>
    EnableDeviceSimd("enable-device-simd", cl::init(false), cl::Hidden,
                     cl::desc("Enable VPlan vectorzer for SIMD on device"));

cl::opt<bool> EnableVPlanDriverHIR("vplan-driver-hir", cl::init(true),
                                   cl::Hidden, cl::desc("Enable VPlan Driver"));

// INTEL - HIR passes
enum class LoopOptMode { None, LightWeight, Full };
cl::opt<LoopOptMode> RunLoopOpts(
    "loopopt", cl::init(LoopOptMode::None), cl::Hidden, cl::ValueOptional,
    cl::desc("Runs loop optimization passes"),
    cl::values(clEnumValN(LoopOptMode::None, "0", "Disable loopopt passes"),
               clEnumValN(LoopOptMode::LightWeight, "1",
                          "Enable lightweight loopopt(minimal passes)"),
               clEnumValN(LoopOptMode::Full, "2", "Enable all loopopt passes"),
               // Value assumed when just -loopopt is specified.
               clEnumValN(LoopOptMode::Full, "", "")));

cl::opt<bool> RunLoopOptFrameworkOnly(
    "loopopt-framework-only", cl::init(false), cl::Hidden,
    cl::desc("Enables loopopt framework without any transformation passes"));

// register promotion for global vars at -O2 and above.
cl::opt<bool> EnableNonLTOGlobalVarOpt(
    "enable-non-lto-global-var-opt", cl::init(true), cl::Hidden,
    cl::desc("Enable register promotion for global vars outside of the LTO."));

// Std Container Optimization at -O2 and above.
cl::opt<bool>
    EnableStdContainerOpt("enable-std-container-opt", cl::init(true),
                          cl::Hidden,
                          cl::desc("Enable Std Container Optimization"));

static cl::opt<bool> EnableTbaaProp("enable-tbaa-prop", cl::init(true),
                                    cl::Hidden,
                                    cl::desc("Enable Tbaa Propagation"));

// Andersen AliasAnalysis
cl::opt<bool> EnableAndersen("enable-andersen", cl::init(true),
    cl::Hidden, cl::desc("Enable Andersen's Alias Analysis"));

// Indirect call Conv
static cl::opt<bool> EnableIndirectCallConv("enable-ind-call-conv",
    cl::init(true), cl::Hidden, cl::desc("Enable Indirect Call Conv"));

// Whole Program Analysis
static cl::opt<bool> EnableWPA("enable-whole-program-analysis",
    cl::init(true), cl::Hidden, cl::desc("Enable Whole Program Analysis"));

#if INTEL_CUSTOMIZATION
#if INTEL_FEATURE_SW_ADVANCED
// IP Cloning
static cl::opt<bool> EnableIPCloning("enable-ip-cloning",
    cl::init(true), cl::Hidden, cl::desc("Enable IP Cloning"));
#endif // INTEL_FEATURE_SW_ADVANCED
#endif // INTEL_CUSTOMIZATION

#if INTEL_FEATURE_SW_ADVANCED
// Dead Array Element Ops Elimination
static cl::opt<bool> EnableDeadArrayOpsElim(
   "enable-dead-array-ops-elim", cl::init(true), cl::Hidden,
   cl::desc("Enable Dead Array Ops Elimination"));
#endif // INTEL_FEATURE_SW_ADVANCED

// IPO Array Transpose
static cl::opt<bool> EnableIPArrayTranspose(
   "enable-ip-array-transpose", cl::init(true), cl::Hidden,
   cl::desc("Enable IPO Array Transpose"));

// Call Tree Cloning
static cl::opt<bool> EnableCallTreeCloning("enable-call-tree-cloning",
    cl::init(true), cl::Hidden, cl::desc("Enable Call Tree Cloning"));

// Inline Aggressive Analysis
static cl::opt<bool>
    EnableInlineAggAnalysis("enable-inline-aggressive-analysis",
    cl::init(true), cl::Hidden, cl::desc("Enable Inline Aggressive Analysis"));

#if INTEL_FEATURE_SW_ADVANCED
// IPO Prefetch
static cl::opt<bool>
    EnableIPOPrefetch("enable-ipo-prefetch",
  cl::init(true), cl::Hidden, cl::desc("Enable IPO Prefetch"));
#endif // INTEL_FEATURE_SW_ADVANCED

#if INTEL_FEATURE_SW_DTRANS
// DTrans optimizations -- this is a placeholder for future work.
static cl::opt<bool> EnableDTrans("enable-dtrans",
    cl::init(false), cl::Hidden,
    cl::desc("Enable DTrans optimizations"));
#endif // INTEL_FEATURE_SW_DTRANS

#if INTEL_FEATURE_SW_ADVANCED
// Partial inline simple functions
static cl::opt<bool>
    EnableIntelPI("enable-intelpi", cl::init(true), cl::Hidden,
                    cl::desc("Enable partial inlining for simple functions"));
#endif // INTEL_FEATURE_SW_ADVANCED

// PGO based function splitting
static cl::opt<bool> EnableFunctionSplitting("enable-function-splitting",
  cl::init(false), cl::Hidden,
  cl::desc("Enable function splitting optimization based on PGO data"));

// Function multi-versioning.
static cl::opt<bool> EnableMultiVersioning("enable-multiversioning",
  cl::init(false), cl::Hidden,
  cl::desc("Enable Function Multi-versioning"));

cl::opt<bool> EnableHandlePragmaVectorAligned(
    "enable-handle-pragma-vector-aligned", cl::init(true),
    cl::desc("Enable Handle Pragma Vector Aligned pass"));

#if INTEL_FEATURE_CSA
// CSA graph splitter.
static cl::opt<bool> RunCSAGraphSplitter("enable-csa-graph-splitter",
  cl::init(false), cl::Hidden, cl::ZeroOrMore,
  cl::desc("Run CSA graph splitter after late outlining."));

// Add extra passes for CSA target.
static cl::opt<bool> EnableCSAPasses("enable-csa-passes",
  cl::init(false), cl::ReallyHidden, cl::ZeroOrMore,
  cl::desc("Enable extra passes for CSA target."));
#endif  // INTEL_FEATURE_CSA

cl::opt<bool> EnableArgNoAliasProp(
    "enable-arg-noalias-prop", cl::init(true), cl::Hidden, cl::ZeroOrMore,
    cl::desc("Enable noalias propagation for function arguments."));

cl::opt<bool> EnableVPOParoptSharedPrivatization(
    "enable-vpo-paropt-shared-privatization", cl::init(true), cl::Hidden,
    cl::ZeroOrMore, cl::desc("Enable VPO Paropt Shared Privatization pass."));

cl::opt<bool> EnableVPOParoptTargetInline(
    "enable-vpo-paropt-target-inline", cl::init(false), cl::Hidden,
    cl::ZeroOrMore, cl::desc("Enable VPO Paropt Target Inline pass."));

static cl::opt<bool> EnableEarlyLSR("enable-early-lsr", cl::init(false),
                                    cl::Hidden, cl::ZeroOrMore,
                                    cl::desc("Add LSR pass before code gen."));

cl::opt<bool>
    EarlyJumpThreading("early-jump-threading", cl::init(true), cl::Hidden,
                       cl::desc("Run the early jump threading pass"));
#endif // INTEL_CUSTOMIZATION

cl::opt<bool> EnableDFAJumpThreading("enable-dfa-jump-thread",
                                     cl::desc("Enable DFA jump threading."),
                                     cl::init(false), cl::Hidden);

cl::opt<bool> EnableHotColdSplit("hot-cold-split",
                                 cl::desc("Enable hot-cold splitting pass"));

cl::opt<bool> EnableIROutliner("ir-outliner", cl::init(false), cl::Hidden,
    cl::desc("Enable ir outliner pass"));

static cl::opt<bool> UseLoopVersioningLICM(
    "enable-loop-versioning-licm", cl::init(false), cl::Hidden,
    cl::desc("Enable the experimental Loop Versioning LICM pass"));

cl::opt<bool>
    DisablePreInliner("disable-preinline", cl::init(false), cl::Hidden,
                      cl::desc("Disable pre-instrumentation inliner"));

cl::opt<int> PreInlineThreshold(
    "preinline-threshold", cl::Hidden, cl::init(75),
    cl::desc("Control the amount of inlining in pre-instrumentation inliner "
             "(default = 75)"));

cl::opt<bool>
    EnableGVNHoist("enable-gvn-hoist",
                   cl::desc("Enable the GVN hoisting pass (default = off)"));

static cl::opt<bool>
    DisableLibCallsShrinkWrap("disable-libcalls-shrinkwrap", cl::init(false),
                              cl::Hidden,
                              cl::desc("Disable shrink-wrap library calls"));

#if INTEL_CUSTOMIZATION
// The "legacy" loop unswitcher currently has better performance on
// polyhedron.
static cl::opt<bool> EnableSimpleLoopUnswitch(
    "enable-simple-loop-unswitch", cl::init(false), cl::Hidden,
    cl::desc("Enable the simple loop unswitch pass. Also enables independent "
             "cleanup passes integrated into the loop pass manager pipeline."));
#endif // INTEL_CUSTOMIZATION

cl::opt<bool>
    EnableGVNSink("enable-gvn-sink",
                  cl::desc("Enable the GVN sinking pass (default = off)"));

// This option is used in simplifying testing SampleFDO optimizations for
// profile loading.
cl::opt<bool>
    EnableCHR("enable-chr", cl::init(true), cl::Hidden,
              cl::desc("Enable control height reduction optimization (CHR)"));

cl::opt<bool> FlattenedProfileUsed(
    "flattened-profile-used", cl::init(false), cl::Hidden,
    cl::desc("Indicate the sample profile being used is flattened, i.e., "
             "no inline hierachy exists in the profile. "));

cl::opt<bool> EnableOrderFileInstrumentation(
    "enable-order-file-instrumentation", cl::init(false), cl::Hidden,
    cl::desc("Enable order file instrumentation (default = off)"));

cl::opt<bool> EnableMatrix(
    "enable-matrix", cl::init(false), cl::Hidden,
    cl::desc("Enable lowering of the matrix intrinsics"));

cl::opt<bool> EnableConstraintElimination(
    "enable-constraint-elimination", cl::init(false), cl::Hidden,
    cl::desc(
        "Enable pass to eliminate conditions based on linear constraints."));

cl::opt<bool> EnableFunctionSpecialization(
    "enable-function-specialization", cl::init(false), cl::Hidden,
    cl::desc("Enable Function Specialization pass"));

cl::opt<AttributorRunOption> AttributorRun(
    "attributor-enable", cl::Hidden, cl::init(AttributorRunOption::NONE),
    cl::desc("Enable the attributor inter-procedural deduction pass."),
    cl::values(clEnumValN(AttributorRunOption::ALL, "all",
                          "enable all attributor runs"),
               clEnumValN(AttributorRunOption::MODULE, "module",
                          "enable module-wide attributor runs"),
               clEnumValN(AttributorRunOption::CGSCC, "cgscc",
                          "enable call graph SCC attributor runs"),
               clEnumValN(AttributorRunOption::NONE, "none",
                          "disable attributor runs")));

extern cl::opt<bool> EnableKnowledgeRetention;
} // namespace llvm

PassManagerBuilder::PassManagerBuilder() {
    OptLevel = 2;
    SizeLevel = 0;
    LibraryInfo = nullptr;
    Inliner = nullptr;
    DisableUnrollLoops = false;
    SLPVectorize = false;
    LoopVectorize = true;
    LoopsInterleaved = true;
    RerollLoops = RunLoopRerolling;
    NewGVN = RunNewGVN;
    LicmMssaOptCap = SetLicmMssaOptCap;
    LicmMssaNoAccForPromotionCap = SetLicmMssaNoAccForPromotionCap;
    DisableGVNLoadPRE = false;
    ForgetAllSCEVInLoopUnroll = ForgetSCEVInLoopUnroll;
    VerifyInput = false;
    VerifyOutput = false;
    MergeFunctions = false;
    DivergentTarget = false;
#if INTEL_CUSTOMIZATION
    DisableIntelProprietaryOpts = false;
    AfterSLPVectorizer = false;
#if INTEL_FEATURE_SW_DTRANS
    DTransEnabled = EnableDTrans;
#else // INTEL_FEATURE_SW_DTRANS
    DTransEnabled = false;
#endif // INTEL_FEATURE_SW_DTRANS
#endif // INTEL_CUSTOMIZATION
    CallGraphProfile = true;
}

PassManagerBuilder::~PassManagerBuilder() {
  delete LibraryInfo;
  delete Inliner;
}

/// Set of global extensions, automatically added as part of the standard set.
static ManagedStatic<
    SmallVector<std::tuple<PassManagerBuilder::ExtensionPointTy,
                           PassManagerBuilder::ExtensionFn,
                           PassManagerBuilder::GlobalExtensionID>,
                8>>
    GlobalExtensions;
static PassManagerBuilder::GlobalExtensionID GlobalExtensionsCounter;

/// Check if GlobalExtensions is constructed and not empty.
/// Since GlobalExtensions is a managed static, calling 'empty()' will trigger
/// the construction of the object.
static bool GlobalExtensionsNotEmpty() {
  return GlobalExtensions.isConstructed() && !GlobalExtensions->empty();
}

PassManagerBuilder::GlobalExtensionID
PassManagerBuilder::addGlobalExtension(PassManagerBuilder::ExtensionPointTy Ty,
                                       PassManagerBuilder::ExtensionFn Fn) {
  auto ExtensionID = GlobalExtensionsCounter++;
  GlobalExtensions->push_back(std::make_tuple(Ty, std::move(Fn), ExtensionID));
  return ExtensionID;
}

void PassManagerBuilder::removeGlobalExtension(
    PassManagerBuilder::GlobalExtensionID ExtensionID) {
  // RegisterStandardPasses may try to call this function after GlobalExtensions
  // has already been destroyed; doing so should not generate an error.
  if (!GlobalExtensions.isConstructed())
    return;

  auto GlobalExtension =
      llvm::find_if(*GlobalExtensions, [ExtensionID](const auto &elem) {
        return std::get<2>(elem) == ExtensionID;
      });
  assert(GlobalExtension != GlobalExtensions->end() &&
         "The extension ID to be removed should always be valid.");

  GlobalExtensions->erase(GlobalExtension);
}

void PassManagerBuilder::addExtension(ExtensionPointTy Ty, ExtensionFn Fn) {
  Extensions.push_back(std::make_pair(Ty, std::move(Fn)));
}

void PassManagerBuilder::addExtensionsToPM(ExtensionPointTy ETy,
                                           legacy::PassManagerBase &PM) const {
  if (GlobalExtensionsNotEmpty()) {
    for (auto &Ext : *GlobalExtensions) {
      if (std::get<0>(Ext) == ETy)
        std::get<1>(Ext)(*this, PM);
    }
  }
  for (const auto &[PT, Fn] : Extensions)
    if (PT == ETy)
      Fn(*this, PM);
}

void PassManagerBuilder::addInitialAliasAnalysisPasses(
    legacy::PassManagerBase &PM) const {
  switch (UseCFLAA) {
  case ::CFLAAType::Steensgaard:
    PM.add(createCFLSteensAAWrapperPass());
    break;
  case ::CFLAAType::Andersen:
    PM.add(createCFLAndersAAWrapperPass());
    break;
  case ::CFLAAType::Both:
    PM.add(createCFLSteensAAWrapperPass());
    PM.add(createCFLAndersAAWrapperPass());
    break;
  default:
    break;
  }

  // Add TypeBasedAliasAnalysis before BasicAliasAnalysis so that
  // BasicAliasAnalysis wins if they disagree. This is intended to help
  // support "obvious" type-punning idioms.
  PM.add(createTypeBasedAAWrapperPass());
  PM.add(createScopedNoAliasAAWrapperPass());
#if INTEL_CUSTOMIZATION
  if (EnableStdContainerOpt)
    PM.add(createStdContainerAAWrapperPass());
#endif // INTEL_CUSTOMIZATION
}

#if INTEL_CUSTOMIZATION
void PassManagerBuilder::addInstructionCombiningPass(
    legacy::PassManagerBase &PM, bool EnableUpCasting) const {
  // Enable it when SLP Vectorizer is off or after SLP Vectorizer pass.
  bool EnableFcmpMinMaxCombine =
      (!SLPVectorize) || AfterSLPVectorizer;
  bool PreserveForDTrans = false;
  if (RunVPOParopt) {
    // CMPLRLLVM-25424: temporary workaround for cases, where
    // the instructions combining pass inserts value definitions
    // inside OpenMP regions making them live out without proper
    // handling in the OpenMP clauses.
    // VPOCFGRestructuring breaks blocks at the OpenMP regions'
    // boundaries minimizing the probability of illegal instruction
    // insertion in the instructions combining pass.
    // We have to move VPO Paropt transformations closer to FE
    // to stop fiddling with the optimization pipeline.
    PM.add(createVPOCFGRestructuringPass());
  }

  PM.add(createInstructionCombiningPass(PreserveForDTrans,
                                        false,
                                        EnableFcmpMinMaxCombine,
                                        EnableUpCasting));
}
#endif // INTEL_CUSTOMIZATION

void PassManagerBuilder::populateFunctionPassManager(
    legacy::FunctionPassManager &FPM) {
  addExtensionsToPM(EP_EarlyAsPossible, FPM);
#if INTEL_CUSTOMIZATION
  limitLoopOptOnly(FPM).add(createLoopOptMarkerLegacyPass());
  limitNoLoopOptOnly(FPM).add(createLowerSubscriptIntrinsicLegacyPass());
#endif // INTEL_CUSTOMIZATION

  // Add LibraryInfo if we have some.
  if (LibraryInfo)
    FPM.add(new TargetLibraryInfoWrapperPass(*LibraryInfo));

#if INTEL_CUSTOMIZATION
  FPM.add(createXmainOptLevelWrapperPass(OptLevel));
#endif // INTEL_CUSTOMIZATION
#if INTEL_COLLAB
  if (RunVPOOpt && RunVPOParopt) {
    FPM.add(createVPOCFGRestructuringPass());
#if INTEL_CUSTOMIZATION
    // VPOParoptConfig must be applied before any pass that
    // may change its behavior based on the clauses added
    // from the config (e.g. loop collapsing may behave differently
    // due to NUM_TEAMS clause).
    FPM.add(createVPOParoptApplyConfigPass());
#endif // INTEL_CUSTOMIZATION
    FPM.add(createVPOParoptLoopTransformPass());
    FPM.add(createVPOCFGRestructuringPass());
    FPM.add(createVPOParoptLoopCollapsePass());
    // TODO: maybe we have to make sure loop collapsing preserves
    //       the restructured CFG.
    FPM.add(createVPOCFGRestructuringPass());
    FPM.add(createVPOParoptPreparePass(RunVPOParopt));
  }
#endif // INTEL_COLLAB
  // The backends do not handle matrix intrinsics currently.
  // Make sure they are also lowered in O0.
  // FIXME: A lightweight version of the pass should run in the backend
  //        pipeline on demand.
  if (EnableMatrix && OptLevel == 0)
    FPM.add(createLowerMatrixIntrinsicsMinimalPass());

  if (OptLevel == 0) return;

  addInitialAliasAnalysisPasses(FPM);

  // Lower llvm.expect to metadata before attempting transforms.
  // Compare/branch metadata may alter the behavior of passes like SimplifyCFG.
  FPM.add(createLowerExpectIntrinsicPass());
  FPM.add(createCFGSimplificationPass());
  FPM.add(createSROAPass());
#if INTEL_CUSTOMIZATION
#if INTEL_FEATURE_SW_ADVANCED
  if (DTransEnabled)
    FPM.add(createFunctionRecognizerLegacyPass());
#endif // INTEL_FEATURE_SW_ADVANCED
#endif // INTEL_CUSTOMIZATION

  FPM.add(createEarlyCSEPass());
}

void PassManagerBuilder::addFunctionSimplificationPasses(
    legacy::PassManagerBase &MPM) {
  // Start of function pass.
#if INTEL_CUSTOMIZATION
  // Propagate TBAA information before SROA so that we can remove mid-function
  // fakeload intrinsics which would block SROA.
  if (EnableTbaaProp)
    MPM.add(createTbaaMDPropagationLegacyPass());
#endif // INTEL_CUSTOMIZATION

  // Break up aggregate allocas, using SSAUpdater.
  assert(OptLevel >= 1 && "Calling function optimizer with no optimization level!");
  MPM.add(createSROAPass());
  MPM.add(createEarlyCSEPass(true /* Enable mem-ssa. */)); // Catch trivial redundancies

  if (OptLevel > 1) {
    if (EnableGVNHoist)
      MPM.add(createGVNHoistPass());
    if (EnableGVNSink) {
      MPM.add(createGVNSinkPass());
      MPM.add(createCFGSimplificationPass(
          SimplifyCFGOptions().convertSwitchRangeToICmp(true)));
    }
  }

  if (EnableConstraintElimination)
    MPM.add(createConstraintEliminationPass());

  if (OptLevel > 1) {
    // Speculative execution if the target has divergent branches; otherwise nop.
    MPM.add(createSpeculativeExecutionIfHasBranchDivergencePass());

    MPM.add(createJumpThreadingPass());         // Thread jumps.
    MPM.add(createCorrelatedValuePropagationPass()); // Propagate conditionals
  }
  MPM.add(
      createCFGSimplificationPass(SimplifyCFGOptions().convertSwitchRangeToICmp(
          true))); // Merge & remove BBs
  // Combine silly seq's
  if (OptLevel > 2)
    MPM.add(createAggressiveInstCombinerPass());
  addInstructionCombiningPass(MPM, !DTransEnabled);  // INTEL
  if (SizeLevel == 0 && !DisableLibCallsShrinkWrap)
    MPM.add(createLibCallsShrinkWrapPass());
  addExtensionsToPM(EP_Peephole, MPM);

#if INTEL_CUSTOMIZATION
  bool SkipRecProgression = false;
  // TODO: Investigate the cost/benefit of tail call elimination on debugging.
  if (OptLevel > 1)
    MPM.add(createTailCallEliminationPass(SkipRecProgression));
                                              // Eliminate tail calls
#endif // INTEL_CUSTOMIZATION
  MPM.add(
      createCFGSimplificationPass(SimplifyCFGOptions().convertSwitchRangeToICmp(
          true)));                            // Merge & remove BBs
  // FIXME: re-association increases variables liveness and therefore register
  // pressure.
#if INTEL_COLLAB
  if (!SYCLOptimizationMode && !SPIRVOptimizationMode)
    MPM.add(createReassociatePass()); // Reassociate expressions
#else // INTEL_COLLAB
  if (!SYCLOptimizationMode)
    MPM.add(createReassociatePass()); // Reassociate expressions
#endif // INTEL_COLLAB

  // The matrix extension can introduce large vector operations early, which can
  // benefit from running vector-combine early on.
  if (EnableMatrix)
    MPM.add(createVectorCombinePass());

  // Do not run loop pass pipeline in "SYCL Optimization Mode". Loop
  // optimizations rely on TTI, which is not accurate for SPIR target.
  if (!SYCLOptimizationMode) { // broken formatting to simplify pulldown

#if INTEL_CUSTOMIZATION
    if (EnableSimpleLoopUnswitch) {
      // The simple loop unswitch pass relies on separate cleanup passes.
      // Schedule them first so when we re-process a loop they run before
      // other loop passes.
      MPM.add(createLoopInstSimplifyPass());
      MPM.add(createLoopSimplifyCFGPass());
    }
#endif // INTEL_CUSTOMIZATION

    // Try to remove as much code from the loop header as possible,
    // to reduce amount of IR that will have to be duplicated. However,
    // do not perform speculative hoisting the first time as LICM
    // will destroy metadata that may not need to be destroyed if run
    // after loop rotation.
    // TODO: Investigate promotion cap for O1.

#if INTEL_CUSTOMIZATION
    // 27770/28531: This extra pass causes high spill rates in some
    // benchmarks.
    if (!DTransEnabled)
      MPM.add(createLICMPass(LicmMssaOptCap, LicmMssaNoAccForPromotionCap,
                             /*AllowSpeculation=*/false));
#endif // INTEL_CUSTOMIZATION
    // Rotate Loop - disable header duplication at -Oz
    MPM.add(createLoopRotatePass(SizeLevel == 2 ? 0 : -1, false));
    // TODO: Investigate promotion cap for O1.
    MPM.add(createLICMPass(LicmMssaOptCap, LicmMssaNoAccForPromotionCap,
                           /*AllowSpeculation=*/true));
#if INTEL_CUSTOMIZATION
  if (EnableSimpleLoopUnswitch)
    MPM.add(createSimpleLoopUnswitchLegacyPass());
  else
    MPM.add(createLoopUnswitchPass(SizeLevel || OptLevel < 3, DivergentTarget));
#endif // INTEL_CUSTOMIZATION

    // FIXME: We break the loop pass pipeline here in order to do full
    // simplifycfg. Eventually loop-simplifycfg should be enhanced to replace
    // the need for this.
    MPM.add(createCFGSimplificationPass(
        SimplifyCFGOptions().convertSwitchRangeToICmp(true)));
    addInstructionCombiningPass(MPM, !DTransEnabled);  // INTEL
    // We resume loop passes creating a second loop pipeline here.
    if (EnableLoopFlatten) {
      MPM.add(createLoopFlattenPass()); // Flatten loops
      MPM.add(createLoopSimplifyCFGPass());
    }
    MPM.add(createLoopIdiomPass());      // Recognize idioms like memset.
    MPM.add(createIndVarSimplifyPass()); // Canonicalize indvars
    addExtensionsToPM(EP_LateLoopOptimizations, MPM);
    MPM.add(createLoopDeletionPass()); // Delete dead loops

    if (EnableLoopInterchange)
      MPM.add(createLoopInterchangePass()); // Interchange loops

    // Unroll small loops
#if INTEL_CUSTOMIZATION
    // HIR complete unroll pass replaces LLVM's simple loop unroll pass.
    limitNoLoopOptOnly(MPM).add(createSimpleLoopUnrollPass(
        OptLevel, DisableUnrollLoops, ForgetAllSCEVInLoopUnroll));
#if INTEL_FEATURE_CSA
    MPM.add(createLoopSPMDizationPass());
#endif // INTEL_FEATURE_CSA
#endif // INTEL_CUSTOMIZATION
    addExtensionsToPM(EP_LoopOptimizerEnd, MPM);
    // This ends the loop pass pipelines.

} // broken formatting on this line to simplify pulldown

  // Break up allocas that may now be splittable after loop unrolling.
  MPM.add(createSROAPass());

  if (OptLevel > 1) {
    MPM.add(createMergedLoadStoreMotionPass()); // Merge ld/st in diamonds
    MPM.add(NewGVN ? createNewGVNPass()
                   : createGVNPass(DisableGVNLoadPRE)); // Remove redundancies
  }
  MPM.add(createSCCPPass());                  // Constant prop with SCCP

  if (EnableConstraintElimination)
    MPM.add(createConstraintEliminationPass());

  // Delete dead bit computations (instcombine runs after to fold away the dead
  // computations, and then ADCE will run later to exploit any new DCE
  // opportunities that creates).
  MPM.add(createBitTrackingDCEPass());        // Delete dead bit computations

  // Run instcombine after redundancy elimination to exploit opportunities
  // opened up by them.
  addInstructionCombiningPass(MPM, !DTransEnabled);  // INTEL
  addExtensionsToPM(EP_Peephole, MPM);
  if (OptLevel > 1) {
    if (EnableDFAJumpThreading && SizeLevel == 0)
      MPM.add(createDFAJumpThreadingPass());

    MPM.add(createJumpThreadingPass());         // Thread jumps
    MPM.add(createCorrelatedValuePropagationPass());
  }
  MPM.add(createAggressiveDCEPass()); // Delete dead instructions

#if INTEL_CUSTOMIZATION
#if INTEL_FEATURE_SW_DTRANS
  // Skip MemCpyOpt when both PrepareForLTO and DTransEnabled flags are
  // true to simplify handling of memcpy/memset/memmov calls in DTrans
  // implementation.
  // TODO: Remove this customization once DTrans handled partial memcpy/
  // memset/memmov calls of struct types.
  if (!DTransEnabled)
    MPM.add(createMemCpyOptPass());           // Remove memcpy / form memset
#else // INTEL_FEATURE_SW_DTRANS
  MPM.add(createMemCpyOptPass());             // Remove memcpy / form memset
#endif // INTEL_FEATURE_SW_DTRANS
#endif // INTEL_CUSTOMIZATION
  // TODO: Investigate if this is too expensive at O1.
  if (OptLevel > 1) {
    MPM.add(createDeadStoreEliminationPass());  // Delete dead stores
    MPM.add(createLICMPass(LicmMssaOptCap, LicmMssaNoAccForPromotionCap,
                           /*AllowSpeculation=*/true));
  }

  addExtensionsToPM(EP_ScalarOptimizerLate, MPM);

  if (RerollLoops)
    MPM.add(createLoopRerollPass());

  // Merge & remove BBs and sink & hoist common instructions.
#if INTEL_CUSTOMIZATION
  // Hoisting of common instructions can result in unstructured CFG input to
  // loopopt. Loopopt has its own pass which hoists conditional loads/stores.
  if (SYCLOptimizationMode)
    MPM.add(createCFGSimplificationPass());
  else {
    limitLoopOptOnly(MPM).add(
      createCFGSimplificationPass(SimplifyCFGOptions()));
    limitNoLoopOptOnly(MPM).add(createCFGSimplificationPass(
        SimplifyCFGOptions().hoistCommonInsts(true).sinkCommonInsts(true)));
  }
#endif // INTEL_CUSTOMIZATION
  // Clean up after everything.
  addInstructionCombiningPass(MPM, !DTransEnabled); // INTEL
  addExtensionsToPM(EP_Peephole, MPM);

#if INTEL_CUSTOMIZATION
  // Transform calls to sin and cos to calls to sinpi, cospi or
  // sincospi.
  // The transformation is not launched for SYCL, because sinpi, cospi and
  // sincospi are not available in standard libraries provided by OpenCL RTs
  if (!SYCLOptimizationMode)
    MPM.add(createTransformSinAndCosCallsPass());
#endif // INTEL_CUSTOMIZATION
}

#if INTEL_CUSTOMIZATION
bool PassManagerBuilder::isLoopOptStaticallyDisabled() const {
  return DisableIntelProprietaryOpts || OptLevel < 2;
}
legacy::LoopOptLimitingPassManager
PassManagerBuilder::limitNoLoopOptOnly(legacy::PassManagerBase &PM) const {
  bool ForceRun = isLoopOptStaticallyDisabled();

  return legacy::LoopOptLimitingPassManager(PM, LoopOptLimiter::NoLoopOptOnly,
                                            false /*ForceSkip*/, ForceRun);
}
legacy::LoopOptLimitingPassManager
PassManagerBuilder::limitLoopOptOnly(legacy::PassManagerBase &PM) const {
  bool ForceSkip = isLoopOptStaticallyDisabled();
  return legacy::LoopOptLimitingPassManager(PM, LoopOptLimiter::LoopOpt,
                                            ForceSkip);
}
legacy::LoopOptLimitingPassManager
PassManagerBuilder::limitFullLoopOptOnly(legacy::PassManagerBase &PM) const {
  bool ForceSkip = isLoopOptStaticallyDisabled();
  return legacy::LoopOptLimitingPassManager(PM, LoopOptLimiter::FullLoopOptOnly,
                                            ForceSkip);
}
legacy::LoopOptLimitingPassManager
PassManagerBuilder::limitLightLoopOptOnly(legacy::PassManagerBase &PM) const {
  bool ForceSkip = isLoopOptStaticallyDisabled();
  return legacy::LoopOptLimitingPassManager(PM, LoopOptLimiter::LightLoopOptOnly,
                                            ForceSkip);
}
legacy::LoopOptLimitingPassManager
PassManagerBuilder::limitNoLoopOptOrNotPrepareForLTO(
    legacy::PassManagerBase &PM) const {
  return legacy::LoopOptLimitingPassManager(PM, LoopOptLimiter::NoLoopOptOnly,
                                            false /*ForceSkip*/, true);
}
#endif // INTEL_CUSTOMIZATION

/// FIXME: Should LTO cause any differences to this set of passes?
void PassManagerBuilder::addVectorPasses(legacy::PassManagerBase &PM,
                                         bool IsFullLTO) {
#if INTEL_CUSTOMIZATION
  if (!SYCLOptimizationMode) {
    if (!IsFullLTO) {
      if (EnableLV)
        limitNoLoopOptOrNotPrepareForLTO(PM).add(
            createLoopVectorizePass(!LoopsInterleaved, !LoopVectorize));
    } else {
      // FIXME: Needs driver cleanup at least.
      if (EnableLV)
        limitNoLoopOptOnly(PM).add(
            createLoopVectorizePass(true, !LoopVectorize));
    }
#endif // INTEL_CUSTOMIZATION

  if (IsFullLTO) {
    // The vectorizer may have significantly shortened a loop body; unroll
    // again. Unroll small loops to hide loop backedge latency and saturate any
    // parallel execution resources of an out-of-order processor. We also then
    // need to clean up redundancies and loop invariant code.
    // FIXME: It would be really good to use a loop-integrated instruction
    // combiner for cleanup here so that the unrolling and LICM can be pipelined
    // across the loop nests.
    // We do UnrollAndJam in a separate LPM to ensure it happens before unroll
    if (EnableUnrollAndJam && !DisableUnrollLoops)
      PM.add(createLoopUnrollAndJamPass(OptLevel));
    PM.add(createLoopUnrollPass(OptLevel, DisableUnrollLoops,
                                ForgetAllSCEVInLoopUnroll));
#if INTEL_CUSTOMIZATION
#if INTEL_FEATURE_CSA
    PM.add(createCSALowerParallelIntrinsicsWrapperPass());
#endif  // INTEL_FEATURE_CSA
#endif  // INTEL_CUSTOMIZATION
    PM.add(createWarnMissedTransformationsPass());
  }

  if (!IsFullLTO) {
    // Eliminate loads by forwarding stores from the previous iteration to loads
    // of the current iteration.
    PM.add(createLoopLoadEliminationPass());
  }
  // Cleanup after the loop optimization passes.
  INTEL_LIMIT_BEGIN(limitNoLoopOptOrNotPrepareForLTO, PM) // INTEL
  addInstructionCombiningPass(PM, !DTransEnabled); // INTEL

  if (OptLevel > 1 && ExtraVectorizerPasses) {
    // At higher optimization levels, try to clean up any runtime overlap and
    // alignment checks inserted by the vectorizer. We want to track correlated
    // runtime checks for two inner loops in the same outer loop, fold any
    // common computations, hoist loop-invariant aspects out of any outer loop,
    // and unswitch the runtime checks if possible. Once hoisted, we may have
    // dead (or speculatable) control flows or more combining opportunities.
    PM.add(createEarlyCSEPass());
    PM.add(createCorrelatedValuePropagationPass());
    addInstructionCombiningPass(PM, !DTransEnabled); // INTEL
    PM.add(createLICMPass(LicmMssaOptCap, LicmMssaNoAccForPromotionCap,
                          /*AllowSpeculation=*/true));
    PM.add(createSimpleLoopUnswitchLegacyPass());
    PM.add(createCFGSimplificationPass(
        SimplifyCFGOptions().convertSwitchRangeToICmp(true)));
    addInstructionCombiningPass(PM, !DTransEnabled); // INTEL
  }

#if INTEL_CUSTOMIZATION
  if (IsFullLTO) {
    // 28038: Avoid excessive hoisting as it increases register pressure and
    // select conversion without clear gains.
    // PM.add(createCFGSimplificationPass(SimplifyCFGOptions() // if-convert
    //                                       .hoistCommonInsts(true)));
    PM.add(createCFGSimplificationPass());
  } else {
#endif // INTEL_CUSTOMIZATION
  // Now that we've formed fast to execute loop structures, we do further
  // optimizations. These are run afterward as they might block doing complex
  // analyses and transforms such as what are needed for loop vectorization.

  // Cleanup after loop vectorization, etc. Simplification passes like CVP and
  // GVN, loop transforms, and others have already run, so it's now better to
  // convert to more optimized IR using more aggressive simplify CFG options.
  // The extra sinking transform can create larger basic blocks, so do this
  // before SLP vectorization.
  PM.add(createCFGSimplificationPass(SimplifyCFGOptions()
                                         .forwardSwitchCondToPhi(true)
                                         .convertSwitchRangeToICmp(true)
                                         .convertSwitchToLookupTable(true)
                                         .needCanonicalLoops(false)
                                         .hoistCommonInsts(true)
                                         .sinkCommonInsts(true)));
#if INTEL_CUSTOMIZATION
  } // IsFullLTO
#endif // INTEL_CUSTOMIZATION

  if (IsFullLTO) {
    PM.add(createSCCPPass());                 // Propagate exposed constants
    addInstructionCombiningPass(PM, true /* EnableUpCasting */); // INTEL
    PM.add(createBitTrackingDCEPass());
  }

  // Optimize parallel scalar instruction chains into SIMD instructions.
  if (SLPVectorize) {
    PM.add(createSLPVectorizerPass());
#if INTEL_CUSTOMIZATION
    AfterSLPVectorizer = true;
    if (EnableLoadCoalescing)
      PM.add(createLoadCoalescingPass());
    if (EnableSROAAfterSLP) {
      // SLP creates opportunities for SROA.
      PM.add(createSROAPass());
    }
#endif // INTEL_CUSTOMIZATION
    if (OptLevel > 1 && ExtraVectorizerPasses)
      PM.add(createEarlyCSEPass());
  }
  INTEL_LIMIT_END // INTEL

  // Enhance/cleanup vector code.
  PM.add(createVectorCombinePass());
#if INTEL_CUSTOMIZATION
  if (!IsFullLTO)
    PM.add(createEarlyCSEPass());
#endif // INTEL_CUSTOMIZATION
  } // if (!SYCLOptimizationMode) // INTEL

#if INTEL_CUSTOMIZATION
  if (!SYCLOptimizationMode)
#endif // INTEL_CUSTOMIZATION
  if (!IsFullLTO) {
    addExtensionsToPM(EP_Peephole, PM);
    addInstructionCombiningPass(PM, !DTransEnabled); // INTEL

    if (EnableUnrollAndJam && !DisableUnrollLoops) {
      // Unroll and Jam. We do this before unroll but need to be in a separate
      // loop pass manager in order for the outer loop to be processed by
      // unroll and jam before the inner loop is unrolled.
#if INTEL_CUSTOMIZATION
      // Disable unroll in LTO mode if loopopt is enabled so it only gets
      // triggered in link phase after loopopt.
      limitNoLoopOptOrNotPrepareForLTO(PM).add(createLoopUnrollAndJamPass(OptLevel));
#endif // INTEL_CUSTOMIZATION
    }

    // Unroll small loops
    INTEL_LIMIT_BEGIN(limitNoLoopOptOrNotPrepareForLTO, PM) // INTEL
    PM.add(createLoopUnrollPass(OptLevel, DisableUnrollLoops,
                                ForgetAllSCEVInLoopUnroll));
    INTEL_LIMIT_END // INTEL
#if INTEL_CUSTOMIZATION
#if INTEL_FEATURE_CSA
    if (!DisableUnrollLoops)
      limitNoLoopOptOrLTOLink(PM).add(
          createCSALowerParallelIntrinsicsWrapperPass());
#endif // INTEL_FEATURE_CSA
#endif // INTEL_CUSTOMIZATION
#if INTEL_CUSTOMIZATION
    if (!DisableUnrollLoops) {
      INTEL_LIMIT_BEGIN(limitNoLoopOptOrNotPrepareForLTO, PM) // INTEL
#if INTEL_FEATURE_SW_ADVANCED
      // Make unaligned nontemporal stores use a wrapper function instead of
      // scalarizing them.
      PM.add(createNontemporalStoreWrapperPass());
#endif // INTEL_FEATURE_SW_ADVANCED
      // LoopUnroll may generate some redundency to cleanup.
      addInstructionCombiningPass(PM, !DTransEnabled);

      // Runtime unrolling will introduce runtime check in loop prologue. If the
      // unrolled loop is a inner loop, then the prologue will be inside the
      // outer loop. LICM pass can help to promote the runtime check out if the
      // checked value is loop invariant.
      PM.add(createLICMPass(LicmMssaOptCap, LicmMssaNoAccForPromotionCap,
                            /*AllowSpeculation=*/true));
      INTEL_LIMIT_END // INTEL
    }

    // Postpone warnings to LTO link phase. Most transformations which process
    // user pragmas (like unroller & vectorizer) are triggered in LTO link phase.
    PM.add(createWarnMissedTransformationsPass());
#endif // INTEL_CUSTOMIZATION
  }

  // After vectorization and unrolling, assume intrinsics may tell us more
  // about pointer alignments.
  PM.add(createAlignmentFromAssumptionsPass());

#if INTEL_CUSTOMIZATION
  if (IsFullLTO) {
#if INTEL_FEATURE_SW_ADVANCED
    // Make unaligned nontemporal stores use a wrapper function instead of
    // scalarizing them.
    PM.add(createNontemporalStoreWrapperPass());
#endif // INTEL_FEATURE_SW_ADVANCED
    addInstructionCombiningPass(PM, true /* EnableUpCasting */);  // INTEL
  }
#endif // INTEL_CUSTOMIZATION
}

void PassManagerBuilder::populateModulePassManager(
    legacy::PassManagerBase &MPM) {
  MPM.add(createAnnotation2MetadataLegacyPass());

#if INTEL_CUSTOMIZATION
  MPM.add(createXmainOptLevelWrapperPass(OptLevel)); // INTEL
#endif // INTEL_CUSTOMIZATION

  // Allow forcing function attributes as a debugging and tuning aid.
  MPM.add(createForceFunctionAttrsLegacyPass());

  // If all optimizations are disabled, just run the always-inline pass and,
  // if enabled, the function merging pass.
  if (OptLevel == 0) {
    if (Inliner) {
      MPM.add(createInlineReportSetupPass(getMDInlineReport())); // INTEL
      MPM.add(createInlineListsPass()); // INTEL: -[no]inline-list parsing
      MPM.add(Inliner);
      Inliner = nullptr;
    }

    // FIXME: The BarrierNoopPass is a HACK! The inliner pass above implicitly
    // creates a CGSCC pass manager, but we don't want to add extensions into
    // that pass manager. To prevent this we insert a no-op module pass to reset
    // the pass manager to get the same behavior as EP_OptimizerLast in non-O0
    // builds. The function merging pass is
    if (MergeFunctions)
      MPM.add(createMergeFunctionsPass());
    else if (GlobalExtensionsNotEmpty() || !Extensions.empty())
      MPM.add(createBarrierNoopPass());

    addExtensionsToPM(EP_EnabledOnOptLevel0, MPM);

#if INTEL_COLLAB
    if (RunVPOOpt) {
      #if INTEL_CUSTOMIZATION
      if (RunVecClone && RunVPOVecopt) {
        MPM.add(createVecClonePass());
      }
      #endif // INTEL_CUSTOMIZATION
      // Process OpenMP directives at -O0
      addVPOPasses(MPM, true);
    }
#endif // INTEL_COLLAB

    MPM.add(createAnnotationRemarksLegacyPass());
    return;
  }

#if INTEL_COLLAB
  // Process OpenMP directives at -O1 and above
  if (RunVPOOpt == InvokeParoptBeforeInliner)
    addVPOPasses(MPM, false);
#endif // INTEL_COLLAB

  // Add LibraryInfo if we have some.
  if (LibraryInfo)
    MPM.add(new TargetLibraryInfoWrapperPass(*LibraryInfo));

  addInitialAliasAnalysisPasses(MPM);

  // Infer attributes about declarations if possible.
  MPM.add(createInferFunctionAttrsLegacyPass());

#if INTEL_CUSTOMIZATION
  if (Inliner) {
    MPM.add(createInlineReportSetupPass(getMDInlineReport()));
    MPM.add(createInlineListsPass()); // -[no]inline-list parsing
    if (RunVPOParopt && EnableVPOParoptTargetInline)
      MPM.add(createVPOParoptTargetInlinePass());
  }
#endif  // INTEL_CUSTOMIZATION

  // Infer attributes on declarations, call sites, arguments, etc.
  if (AttributorRun & AttributorRunOption::MODULE)
    MPM.add(createAttributorLegacyPass());

  addExtensionsToPM(EP_ModuleOptimizerEarly, MPM);

  if (OptLevel > 2)
    MPM.add(createCallSiteSplittingPass());

  // Propage constant function arguments by specializing the functions.
  if (OptLevel > 2 && EnableFunctionSpecialization)
    MPM.add(createFunctionSpecializationPass());

  MPM.add(createIPSCCPPass());          // IP SCCP

  MPM.add(createCalledValuePropagationPass());

  MPM.add(createGlobalOptimizerPass()); // Optimize out global vars
  // Promote any localized global vars.
  MPM.add(createPromoteMemoryToRegisterPass());

  MPM.add(createDeadArgEliminationPass()); // Dead argument elimination

#if INTEL_CUSTOMIZATION
  // Clean up after IPCP & DAE
  addInstructionCombiningPass(MPM, !DTransEnabled);
  addExtensionsToPM(EP_Peephole, MPM);
  // In 2019, "false" was passed to the AllowCFGSimps parameter.
  // In 2020, after the FreezeSelect arg was added, the Allow CFGSimps parm
  // was accidentally left at default (true) [e9e5aace]
  // Leaving it "true" for now as it has been 2 years without regressions.
  if (EarlyJumpThreading && !SYCLOptimizationMode)                // INTEL
    MPM.add(createJumpThreadingPass()); // INTEL
#endif // INTEL_CUSTOMIZATION
  MPM.add(
      createCFGSimplificationPass(SimplifyCFGOptions().convertSwitchRangeToICmp(
          true))); // Clean up after IPCP & DAE

#if INTEL_CUSTOMIZATION
  // Handle '#pragma vector aligned'.
  if (EnableHandlePragmaVectorAligned && OptLevel > 1)
    MPM.add(createHandlePragmaVectorAlignedPass());
#endif // INTEL_CUSTOMIZATION

  // We add a module alias analysis pass here. In part due to bugs in the
  // analysis infrastructure this "works" in that the analysis stays alive
  // for the entire SCC pass run below.
  MPM.add(createGlobalsAAWrapperPass());

  // Start of CallGraph SCC passes.
  MPM.add(createPruneEHPass()); // Remove dead EH info
  bool RunInliner = false;
  if (Inliner) {
    MPM.add(Inliner);
    Inliner = nullptr;
    RunInliner = true;
  }

#if INTEL_COLLAB
  // Process OpenMP directives at -O1 and above
  if (RunVPOOpt == InvokeParoptAfterInliner) {
    // We need to ensure that the inliner's CGSCC pipeline finishes before
    // vpo-restore-operands pass is run on any of the functions. See
    // llvm/test/Transforms/Intel_VPO/Paropt/target_fp_packed_struct.ll for
    // details.
    addVPOPasses(MPM, false, /* Simplify= */ true,
                 /* AddNoOpBarrierPassBeforeRestore= */ true);
  }
#endif // INTEL_COLLAB
  if (OptLevel > 1)
    MPM.add(createSROALegacyCGSCCAdaptorPass());

  // Infer attributes on declarations, call sites, arguments, etc. for an SCC.
  if (AttributorRun & AttributorRunOption::CGSCC)
    MPM.add(createAttributorCGSCCLegacyPass());

  // Try to perform OpenMP specific optimizations. This is a (quick!) no-op if
  // there are no OpenMP runtime calls present in the module.
  if (OptLevel > 1)
    MPM.add(createOpenMPOptCGSCCLegacyPass());

  MPM.add(createPostOrderFunctionAttrsLegacyPass());

  addExtensionsToPM(EP_CGSCCOptimizerLate, MPM);
  addFunctionSimplificationPasses(MPM);

#if INTEL_CUSTOMIZATION
  // If VPO paropt was required to run then do IP constant propagation after
  // promoting pointer arguments to values (when OptLevel > 1) and running
  // simplification passes. That will propagate constant values down to callback
  // functions which represent outlined OpenMP parallel loops where possible.
  if (RunVPOParopt && OptLevel > 1)
    MPM.add(createIPSCCPPass());

  // Propagate noalias attribute to function arguments.
  if (EnableArgNoAliasProp && OptLevel > 2)
    MPM.add(createArgNoAliasPropPass());
#endif // INTEL_CUSTOMIZATION

  // FIXME: This is a HACK! The inliner pass above implicitly creates a CGSCC
  // pass manager that we are specifically trying to avoid. To prevent this
  // we must insert a no-op module pass to reset the pass manager.
  MPM.add(createBarrierNoopPass());

  if (RunPartialInlining)
    MPM.add(createPartialInliningPass());

#if INTEL_CUSTOMIZATION
  if (EnableStdContainerOpt)
    MPM.add(createStdContainerOptPass());
  MPM.add(createCleanupFakeLoadsPass());
#endif // INTEL_CUSTOMIZATION

  if (OptLevel > 1)
    // Remove avail extern fns and globals definitions if we aren't
    // compiling an object file for later LTO. For LTO we want to preserve
    // these so they are eligible for inlining at link-time. Note if they
    // are unreferenced they will be removed by GlobalDCE later, so
    // this only impacts referenced available externally globals.
    // Eventually they will be suppressed during codegen, but eliminating
    // here enables more opportunity for GlobalDCE as it may make
    // globals referenced by available external functions dead
    // and saves running remaining passes on the eliminated functions.
    MPM.add(createEliminateAvailableExternallyPass());

  MPM.add(createReversePostOrderFunctionAttrsPass());

  // The inliner performs some kind of dead code elimination as it goes,
  // but there are cases that are not really caught by it. We might
  // at some point consider teaching the inliner about them, but it
  // is OK for now to run GlobalOpt + GlobalDCE in tandem as their
  // benefits generally outweight the cost, making the whole pipeline
  // faster.
  if (RunInliner) {
    MPM.add(createGlobalOptimizerPass());
    MPM.add(createGlobalDCEPass());
  }

  // Scheduling LoopVersioningLICM when inlining is over, because after that
  // we may see more accurate aliasing. Reason to run this late is that too
  // early versioning may prevent further inlining due to increase of code
  // size. By placing it just after inlining other optimizations which runs
  // later might get benefit of no-alias assumption in clone loop.
  if (UseLoopVersioningLICM) {
    MPM.add(createLoopVersioningLICMPass());    // Do LoopVersioningLICM
    MPM.add(createLICMPass(LicmMssaOptCap, LicmMssaNoAccForPromotionCap,
                           /*AllowSpeculation=*/true));
  }

#if INTEL_CUSTOMIZATION
  if (EnableAndersen) {
    MPM.add(createAndersensAAWrapperPass()); // Andersen's IP alias analysis
  }
  if (OptLevel >= 2 && EnableNonLTOGlobalVarOpt && EnableAndersen) {
    MPM.add(createNonLTOGlobalOptimizerPass());
    MPM.add(createPromoteMemoryToRegisterPass());
    // AggressiveDCE is invoked here to avoid -6% performance regression
    // for aifftr01@opt_speed
    MPM.add(createAggressiveDCEPass());
  }
#endif // INTEL_CUSTOMIZATION

  // We add a fresh GlobalsModRef run at this point. This is particularly
  // useful as the above will have inlined, DCE'ed, and function-attr
  // propagated everything. We should at this point have a reasonably minimal
  // and richly annotated call graph. By computing aliasing and mod/ref
  // information for all local globals here, the late loop passes and notably
  // the vectorizer will be able to use them to help recognize vectorizable
  // memory operations.
  //
  // Note that this relies on a bug in the pass manager which preserves
  // a module analysis into a function pass pipeline (and throughout it) so
  // long as the first function pass doesn't invalidate the module analysis.
  // Thus both Float2Int and LoopRotate have to preserve AliasAnalysis for
  // this to work. Fortunately, it is trivial to preserve AliasAnalysis
  // (doing nothing preserves it as it is required to be conservatively
  // correct in the face of IR changes).
  MPM.add(createGlobalsAAWrapperPass());

  MPM.add(createFloat2IntPass());
  MPM.add(createLowerConstantIntrinsicsPass());

  if (EnableMatrix) {
    MPM.add(createLowerMatrixIntrinsicsPass());
    // CSE the pointer arithmetic of the column vectors.  This allows alias
    // analysis to establish no-aliasing between loads and stores of different
    // columns of the same matrix.
    MPM.add(createEarlyCSEPass(false));
  }

  addExtensionsToPM(EP_VectorizerStart, MPM);

  if (!SYCLOptimizationMode) { // INTEL
  // Re-rotate loops in all our loop nests. These may have fallout out of
  // rotated form due to GVN or other transformations, and the vectorizer relies
  // on the rotated form. Disable header duplication at -Oz.
  MPM.add(createLoopRotatePass(SizeLevel == 2 ? 0 : -1, false));
  } // INTEL
#if INTEL_CUSTOMIZATION
  if (!SYCLOptimizationMode) {
    // In LTO mode, loopopt needs to run in link phase along with community
    // vectorizer and unroll after it until they are phased out.

    // TODO: We might need a more "broad" limiter for the auto CPU dispatch as
    // the cloning would happen during LTO stage only.
    INTEL_LIMIT_BEGIN(limitNoLoopOptOrNotPrepareForLTO, MPM) // INTEL
    addLoopOptAndAssociatedVPOPasses(MPM, false);
    INTEL_LIMIT_END // INTEL
  }
#endif // INTEL_CUSTOMIZATION
  if (!SYCLOptimizationMode) { // INTEL
  INTEL_LIMIT_BEGIN(limitNoLoopOptOrNotPrepareForLTO, MPM) // INTEL
  // Distribute loops to allow partial vectorization.  I.e. isolate dependences
  // into separate loop that would otherwise inhibit vectorization.  This is
  // currently only performed for loops marked with the metadata
  // llvm.loop.distribute=true or when -enable-loop-distribute is specified.
  MPM.add(createLoopDistributePass());
  INTEL_LIMIT_END // INTEL
  } // INTEL

  addVectorPasses(MPM, /* IsFullLTO */ false);

  // FIXME: We shouldn't bother with this anymore.
  MPM.add(createStripDeadPrototypesPass()); // Get rid of dead prototypes

  // GlobalOpt already deletes dead functions and globals, at -O2 try a
  // late pass of GlobalDCE.  It is capable of deleting dead cycles.
  if (OptLevel > 1) {
    MPM.add(createGlobalDCEPass());         // Remove dead fns and globals.
    MPM.add(createConstantMergePass());     // Merge dup global constants
  }

  // See comment in the new PM for justification of scheduling splitting at
  // this stage (\ref buildModuleSimplificationPipeline).
  if (EnableHotColdSplit)
    MPM.add(createHotColdSplittingPass());

  if (EnableIROutliner)
    MPM.add(createIROutlinerPass());

  if (MergeFunctions)
    MPM.add(createMergeFunctionsPass());

  // LoopSink pass sinks instructions hoisted by LICM, which serves as a
  // canonicalization pass that enables other optimizations. As a result,
  // LoopSink pass needs to be a very late IR pass to avoid undoing LICM
  // result too early.
  MPM.add(createLoopSinkPass());
#if INTEL_CUSTOMIZATION
  if (DisableIntelProprietaryOpts && EnableEarlyLSR)
    MPM.add(createLoopStrengthReducePass());
#endif // INTEL_CUSTOMIZATION
  // Get rid of LCSSA nodes.
  MPM.add(createInstSimplifyLegacyPass());

  // This hoists/decomposes div/rem ops. It should run after other sink/hoist
  // passes to avoid re-sinking, but before SimplifyCFG because it can allow
  // flattening of blocks.
  MPM.add(createDivRemPairsPass());

  // LoopSink (and other loop passes since the last simplifyCFG) might have
  // resulted in single-entry-single-exit or empty blocks. Clean up the CFG.
  MPM.add(createCFGSimplificationPass(
      SimplifyCFGOptions().convertSwitchRangeToICmp(true)));

  addExtensionsToPM(EP_OptimizerLast, MPM);

#if INTEL_CUSTOMIZATION
#if INTEL_FEATURE_CSA
  if (EnableCSAPasses) {
    MPM.add(createPromoteMemoryToRegisterPass(true, true));
    MPM.add(createSROAPass());
  }
#endif // INTEL_FEATURE_CSA
#endif // INTEL_CUSTOMIZATION

  MPM.add(createAnnotationRemarksLegacyPass());

#if INTEL_CUSTOMIZATION
  MPM.add(createInlineReportEmitterPass(OptLevel, SizeLevel, false));
#endif // INTEL_CUSTOMIZATION
}

void PassManagerBuilder::addLTOOptimizationPasses(legacy::PassManagerBase &PM) {
#if INTEL_CUSTOMIZATION
  if (Inliner &&
      (IntelInlineReportLevel & InlineReportOptions::CompositeReport)) {
    PM.add(createInlineReportSetupPass(getMDInlineReport()));
  }

  PM.add(createXmainOptLevelWrapperPass(OptLevel));
  // Whole Program Analysis
  if (EnableWPA) {
    // If whole-program-assume is enabled then we are going to call
    // the internalization pass.
    if (AssumeWholeProgram) {

      // The internalization pass does certain checks if a GlobalValue
      // should be internalized (e.g. is local, DLL export, etc.). The
      // pass also accepts a helper function that defines extra conditions
      // on top of the default requirements. If the function returns true
      // then it means that the GlobalValue should not be internalized, else
      // if it returns false then internalize it.
      auto PreserveSymbol = [](const GlobalValue &GV) {
        WholeProgramUtils WPUtils;

        // If GlobalValue is "main", has one definition rule (ODR) or
        // is a special symbol added by the linker then don't internalize
        // it. The ODR symbols are expected to be merged with equivalent
        // globals and then be removed. If these symbols aren't removed
        // then it could cause linking issues (e.g. undefined symbols).
        if (GV.hasWeakODRLinkage() ||
            WPUtils.isMainEntryPoint(GV.getName()) ||
            WPUtils.isLinkerAddedSymbol(GV.getName()))
          return true;

        // If the GlobalValue is an alias then we need to make sure that this
        // alias is OK to internalize.
        if (const GlobalAlias *Alias = dyn_cast<const GlobalAlias>(&GV)) {

          // Check if the alias has an aliasee and this aliasee is a
          // GlobalValue
          const GlobalValue *Glob =
            dyn_cast<const GlobalValue>(Alias->getAliasee());
          if (!Glob)
            return true;

          // Aliasee is a declaration
          if (Glob->isDeclaration())
            return true;

          // Aliasee is an external declaration
          if (Glob->hasAvailableExternallyLinkage())
            return true;

          // Aliasee is an DLL export
          if (Glob->hasDLLExportStorageClass())
            return true;

          // Aliasee is local already
          if (Glob->hasLocalLinkage())
            return true;

          // Aliasee is ODR
          if (Glob->hasWeakODRLinkage())
            return true;

          // Aliasee is mapped to a linker added symbol
          if (WPUtils.isLinkerAddedSymbol(Glob->getName()))
            return true;

          // Aliasee is mapped to main
          if (WPUtils.isMainEntryPoint(Glob->getName()))
            return true;
        }

        // OK to internalize
        return false;
      };
      PM.add(createInternalizePass(PreserveSymbol));
    }
    PM.add(createWholeProgramWrapperPassPass(WPUtils));
  }
#endif // INTEL_CUSTOMIZATION

  // Remove unused virtual tables to improve the quality of code generated by
  // whole-program devirtualization and bitset lowering.
  PM.add(createGlobalDCEPass());

#if INTEL_CUSTOMIZATION
#if INTEL_FEATURE_SW_ADVANCED
  // IPO Prefetching: make it before IPClone and Inline
  if (EnableIPOPrefetch)
    PM.add(createIntelIPOPrefetchWrapperPass());
#endif // INTEL_FEATURE_SW_ADVANCED

#if INTEL_FEATURE_SW_DTRANS
  if (EnableWPA)
    PM.add(createIntelFoldWPIntrinsicLegacyPass());
#endif // INTEL_FEATURE_SW_DTRANS

#if INTEL_FEATURE_SW_ADVANCED
  // IP Cloning
  if (EnableIPCloning) {
    // This pass is being added under DTRANS only at this point, because a
    // particular benchmark needs it to prove that the period of a recursive
    // progression is constant. We can remove the test for DTransEnabled if
    // we find IPSCCP to be generally useful here and we are willing to
    // tolerate the additional compile time.
    if (DTransEnabled)
      PM.add(createIPSCCPPass());
    PM.add(createIPCloningLegacyPass(false, true));
  }
#endif // INTEL_FEATURE_SW_ADVANCED

  // Apply dynamic_casts optimization pass.
  PM.add(createOptimizeDynamicCastsWrapperPass());
#endif // INTEL_CUSTOMIZATION

  // Provide AliasAnalysis services for optimizations.
  addInitialAliasAnalysisPasses(PM);

  // Allow forcing function attributes as a debugging and tuning aid.
  PM.add(createForceFunctionAttrsLegacyPass());

  // Infer attributes about declarations if possible.
  PM.add(createInferFunctionAttrsLegacyPass());

  if (OptLevel > 1) {
    // Split call-site with more constrained arguments.
    PM.add(createCallSiteSplittingPass());
#if INTEL_CUSTOMIZATION
    // Compute the loop attributes
    PM.add(createIntelLoopAttrsWrapperPass(DTransEnabled));
#endif // INTEL_CUSTOMIZATION

    // Propage constant function arguments by specializing the functions.
    if (EnableFunctionSpecialization && OptLevel > 2)
      PM.add(createFunctionSpecializationPass());

    // Propagate constants at call sites into the functions they call.  This
    // opens opportunities for globalopt (and inlining) by substituting function
    // pointers passed as arguments to direct uses of functions.
    PM.add(createIPSCCPPass());

    // Attach metadata to indirect call sites indicating the set of functions
    // they may target at run-time. This should follow IPSCCP.
    PM.add(createCalledValuePropagationPass());

    // Infer attributes on declarations, call sites, arguments, etc.
    if (AttributorRun & AttributorRunOption::MODULE)
      PM.add(createAttributorLegacyPass());
  }

  // Infer attributes about definitions. The readnone attribute in particular is
  // required for virtual constant propagation.
  PM.add(createPostOrderFunctionAttrsLegacyPass());
  PM.add(createReversePostOrderFunctionAttrsPass());

#if INTEL_CUSTOMIZATION
  // Simplify the graph before devirtualization
  if (OptLevel > 1) {
    PM.add(createInstSimplifyLegacyPass());
    PM.add(createCFGSimplificationPass());
  }
#endif // INTEL_CUSTOMIZATION

  // Split globals using inrange annotations on GEP indices. This can help
  // improve the quality of generated code when virtual constant propagation or
  // control flow integrity are enabled.
  PM.add(createGlobalSplitPass());

  // That's all we need at opt level 1.
  if (OptLevel == 1)
    return;

#if INTEL_CUSTOMIZATION
#if INTEL_FEATURE_SW_DTRANS
  if (DTransEnabled) {
    // This call adds the DTrans passes.
    addDTransLegacyPasses(PM);
  }
#endif // INTEL_FEATURE_SW_DTRANS
#if INTEL_FEATURE_SW_ADVANCED
  // Multiversion and mark for inlining functions for tiling
  if (DTransEnabled)
    PM.add(createTileMVInlMarkerLegacyPass());
#endif // INTEL_FEATURE_SW_ADVANCED
  PM.add(createDopeVectorConstPropLegacyPass());
#endif // INTEL_CUSTOMIZATION

  // Now that we internalized some globals, see if we can hack on them!
  PM.add(createGlobalOptimizerPass());
  // Promote any localized global vars.
  PM.add(createPromoteMemoryToRegisterPass());

  // Linking modules together can lead to duplicated global constants, only
  // keep one copy of each constant.
  PM.add(createConstantMergePass());

  // Remove unused arguments from functions.
  PM.add(createDeadArgEliminationPass());

#if INTEL_CUSTOMIZATION
#if INTEL_FEATURE_SW_DTRANS
  if (DTransEnabled) {
    addLateDTransLegacyPasses(PM);
    if (EnableIndirectCallConv)
      PM.add(createIndirectCallConvLegacyPass(false /* EnableAndersen */,
                                              true /* DTransEnabled */));
      // Indirect Call Conv
  }
#endif // INTEL_FEATURE_SW_DTRANS
#endif // INTEL_CUSTOMIZATION

  // Reduce the code after globalopt and ipsccp.  Both can open up significant
  // simplification opportunities, and both can propagate functions through
  // function pointers.  When this happens, we often have to resolve varargs
  // calls, etc, so let instcombine do this.
  if (OptLevel > 2)
    PM.add(createAggressiveInstCombinerPass());
  addInstructionCombiningPass(PM, !DTransEnabled);   // INTEL
  addExtensionsToPM(EP_Peephole, PM);

#if INTEL_CUSTOMIZATION

#if INTEL_FEATURE_SW_ADVANCED
  if (DTransEnabled) {
    // Compute the aligment of the argument
    PM.add(createIntelArgumentAlignmentLegacyPass());
    // Recognize Functions that implement qsort
    PM.add(createQsortRecognizerLegacyPass());
  }

  bool EnableIntelPartialInlining = EnableIntelPI && DTransEnabled;
  if (EnableIntelPartialInlining)
    PM.add(createIntelPartialInlineLegacyPass());
#endif // INTEL_FEATURE_SW_ADVANCED

  bool RunInliner = Inliner;
#if INTEL_CUSTOMIZATION
  if (RunInliner &&
      !(IntelInlineReportLevel & InlineReportOptions::CompositeReport))
    PM.add(createInlineReportSetupPass(getMDInlineReport()));
#endif // INTEL_CUSTOMIZATION
  if (RunInliner) {
    PM.add(createInlineListsPass()); // -[no]inline-list parsing
  }

  if (EnableAndersen) {
    // Andersen's IP alias analysis
    PM.add(createAndersensAAWrapperPass(true /* BeforeInl */));
  }
  if (EnableIndirectCallConv && EnableAndersen) {
#if INTEL_FEATURE_SW_DTRANS
    PM.add(createIndirectCallConvLegacyPass(true /* EnableAndersen */,
                                            false /* EnableDTrans */));
#else // INTEL_FEATURE_SW_DTRANS
    PM.add(createIndirectCallConvLegacyPass(true /* EnableAndersen */));
#endif // INTEL_FEATURE_SW_DTRANS
    // Indirect Call Conv
  }
  if (EnableInlineAggAnalysis) {
    PM.add(createAggInlinerLegacyPass()); // Aggressive Inline
  }
#endif // INTEL_CUSTOMIZATION

  // Inline small functions
  if (RunInliner) {
    PM.add(Inliner);
    Inliner = nullptr;
  }

  PM.add(createPruneEHPass());   // Remove dead EH info.

  // Infer attributes on declarations, call sites, arguments, etc. for an SCC.
  if (AttributorRun & AttributorRunOption::CGSCC)
    PM.add(createAttributorCGSCCLegacyPass());

  // Try to perform OpenMP specific optimizations. This is a (quick!) no-op if
  // there are no OpenMP runtime calls present in the module.
  if (OptLevel > 1)
    PM.add(createOpenMPOptCGSCCLegacyPass());

  // Optimize globals again if we ran the inliner.
  if (RunInliner) { // INTEL
#if INTEL_CUSTOMIZATION
#if INTEL_FEATURE_SW_DTRANS
    // The global optimizer pass can convert function calls to use
    // the 'fastcc' calling convention. The following pass enables more
    // functions to be converted to this calling convention. This can improve
    // performance by having arguments passed in registers, and enable more
    // cases where pointer parameters are changed to pass-by-value parameters.
    // We can remove the test for DTransEnabled if it is found to be useful
    // on other cases.
    if (DTransEnabled)
      PM.add(createIntelAdvancedFastCallWrapperPass());
#endif // INTEL_FEATURE_SW_DTRANS
#endif // INTEL_CUSTOMIZATION
    PM.add(createGlobalOptimizerPass());
  } // INTEL

#if INTEL_CUSTOMIZATION
  if (RunLTOPartialInlining)
    PM.add(createPartialInliningPass(true /*RunLTOPartialInlining*/,
#if INTEL_FEATURE_SW_DTRANS
                                     DTransEnabled /*EnableSpecialCases*/));
#else // INTEL_FEATURE_SW_DTRANS
                                     false /*EnableSpecialCases*/));
#endif // INTEL_FEATURE_SW_DTRANS

  if (
#if INTEL_FEATURE_SW_ADVANCED
      EnableIPCloning ||
#endif // INTEL_FEATURE_SW_ADVANCED
      EnableCallTreeCloning) {
#if INTEL_FEATURE_SW_ADVANCED
    if (EnableIPCloning)
      // Enable generic IPCloning after Inlining.
      PM.add(createIPCloningLegacyPass(true, DTransEnabled));
#endif // INTEL_FEATURE_SW_ADVANCED
    if (EnableCallTreeCloning)
      // Do function cloning along call trees
      PM.add(createCallTreeCloningPass());
    // Call IPCP to propagate constants
    PM.add(createIPSCCPPass());
  }
#endif // INTEL_CUSTOMIZATION
  PM.add(createGlobalDCEPass()); // Remove dead functions.

  // The IPO passes may leave cruft around.  Clean up after them.
  addInstructionCombiningPass(PM, !DTransEnabled);  // INTEL
  addExtensionsToPM(EP_Peephole, PM);
  PM.add(createJumpThreadingPass());

  // Break up allocas
  PM.add(createSROAPass());

#if INTEL_CUSTOMIZATION
  if (EnableIPArrayTranspose)
    PM.add(createIPArrayTransposeLegacyPass());

#if INTEL_FEATURE_SW_ADVANCED
  if (DTransEnabled)
    PM.add(createIPPredOptLegacyPass());
  if (EnableDeadArrayOpsElim)
    PM.add(createDeadArrayOpsEliminationLegacyPass());
#endif // INTEL_FEATURE_SW_ADVANCED

  PM.add(createCorrelatedValuePropagationPass());

  if (EnableMultiVersioning) {
    PM.add(createMultiVersioningWrapperPass());
#if INTEL_FEATURE_SW_DTRANS
    // 21914: If we ran cloning+MV+Dtrans, it is likely we have duplicate
    // code regions that need to be cleaned up. Community disabled hoisting
    // recently, we therefore need to run it explictly.
    if (DTransEnabled)
      PM.add(createCFGSimplificationPass(SimplifyCFGOptions()
                                             .hoistCommonInsts(true)));
#endif // INTEL_FEATURE_SW_DTRANS
  }
#endif // INTEL_CUSTOMIZATION
  // LTO provides additional opportunities for tailcall elimination due to
  // link-time inlining, and visibility of nocapture attribute.
  if (OptLevel > 1)
    PM.add(createTailCallEliminationPass());

#if INTEL_CUSTOMIZATION
  // Compute the loop attributes
  if (OptLevel > 1)
    PM.add(createIntelLoopAttrsWrapperPass(DTransEnabled));
#endif // INTEL_CUSTOMIZATION

  // Infer attributes on declarations, call sites, arguments, etc.
  PM.add(createPostOrderFunctionAttrsLegacyPass()); // Add nocapture.
#if INTEL_CUSTOMIZATION
  // Propagate noalias attribute to function arguments.
  if (EnableArgNoAliasProp && OptLevel > 2)
    PM.add(createArgNoAliasPropPass());

#endif // INTEL_CUSTOMIZATION
  // Run a few AA driven optimizations here and now, to cleanup the code.
  PM.add(createGlobalsAAWrapperPass()); // IP alias analysis.

#if INTEL_CUSTOMIZATION
  if (EnableAndersen)
    PM.add(createAndersensAAWrapperPass());

#if INTEL_FEATURE_SW_DTRANS
  if (DTransEnabled)
    PM.add(createDTransFieldModRefAnalysisWrapperPass());
#endif // INTEL_FEATURE_SW_DTRANS
  PM.add(createIntelIPODeadArgEliminationWrapperPass());
#endif // INTEL_CUSTOMIZATION

  PM.add(createLICMPass(LicmMssaOptCap, LicmMssaNoAccForPromotionCap,
                        /*AllowSpeculation=*/true));
  PM.add(NewGVN ? createNewGVNPass()
                : createGVNPass(DisableGVNLoadPRE)); // Remove redundancies.
  PM.add(createDopeVectorHoistWrapperPass());  // INTEL
  PM.add(createMemCpyOptPass());            // Remove dead memcpys.

  // Nuke dead stores.
  PM.add(createDeadStoreEliminationPass());
  PM.add(createMergedLoadStoreMotionPass()); // Merge ld/st in diamonds.

  // More loops are countable; try to optimize them.
  if (EnableLoopFlatten)
    PM.add(createLoopFlattenPass());
  PM.add(createIndVarSimplifyPass());
  PM.add(createLoopDeletionPass());
  if (EnableLoopInterchange)
    PM.add(createLoopInterchangePass());

  if (EnableConstraintElimination)
    PM.add(createConstraintEliminationPass());

#if INTEL_CUSTOMIZATION
  // HIR complete unroll pass replaces LLVM's simple loop unroll pass.
  limitNoLoopOptOnly(PM).add(
      createSimpleLoopUnrollPass(OptLevel,
                                 DisableUnrollLoops, // Unroll small loops
                                 ForgetAllSCEVInLoopUnroll));
  addLoopOptAndAssociatedVPOPasses(PM, true);
#endif  // INTEL_CUSTOMIZATION
  PM.add(createLoopDistributePass());

  addVectorPasses(PM, /* IsFullLTO */ true);

  addExtensionsToPM(EP_Peephole, PM);

#if INTEL_CUSTOMIZATION
  PM.add(createForcedCMOVGenerationPass()); // To help CMOV generation
#endif // INTEL_CUSTOMIZATION

#if INTEL_CUSTOMIZATION
  if (RunInliner)
    PM.add(createInlineReportEmitterPass(OptLevel, SizeLevel, false));
#endif // INTEL_CUSTOMIZATION
  PM.add(createJumpThreadingPass());
}

void PassManagerBuilder::addLateLTOOptimizationPasses(
    legacy::PassManagerBase &PM) {
  // See comment in the new PM for justification of scheduling splitting at
  // this stage (\ref buildLTODefaultPipeline).
  if (EnableHotColdSplit)
    PM.add(createHotColdSplittingPass());

  // Delete basic blocks, which optimization passes may have killed.
#if INTEL_CUSTOMIZATION
  // 28038: Avoid excessive hoisting as it increases register pressure and
  // select conversion without clear gains.
  // PM.add(
  //   createCFGSimplificationPass(SimplifyCFGOptions().hoistCommonInsts(true)));
  PM.add(createCFGSimplificationPass());
#endif // INTEL_CUSTOMIZATION

#if INTEL_CUSTOMIZATION
  // HIR complete unroll can expose opportunities for optimizing globals and
  // allocas.
  limitLoopOptOnly(PM).add(createGlobalOptimizerPass());
#endif // INTEL_CUSTOMIZATION
  // Drop bodies of available externally objects to improve GlobalDCE.
  PM.add(createEliminateAvailableExternallyPass());

  // Now that we have optimized the program, discard unreachable functions.
  PM.add(createGlobalDCEPass());

  // FIXME: this is profitable (for compiler time) to do at -O0 too, but
  // currently it damages debug info.
  if (MergeFunctions)
    PM.add(createMergeFunctionsPass());
}

#if INTEL_COLLAB
void PassManagerBuilder::addVPOPasses(legacy::PassManagerBase &PM, bool RunVec,
                                      bool Simplify,
                                      bool AddNoOpBarrierPassBeforeRestore) {
  if (!RunVPOParopt)
    return;

  if (Simplify) {
    // Optimize unnesessary alloca, loads and stores to simplify IR.
    PM.add(createSROAPass());

    // Inlining may introduce BasicBlocks without predecessors into an OpenMP
    // region. This breaks CodeExtractor when outlining the region because it
    // expects a single-entry-single-exit region. Calling CFG simplification
    // to remove unreachable BasicBlocks fixes this problem.
#if INTEL_CUSTOMIZATION
    // The inlining issue is documented in CMPLRLLVM-7516. It affects these
    // tests: ompo_kernelsCpp/aobenchan*,ribbon*,terrain*
#endif // INTEL_CUSTOMIZATION
    PM.add(createCFGSimplificationPass());
  }
  if (AddNoOpBarrierPassBeforeRestore)
    PM.add(createBarrierNoopPass());
  PM.add(createVPORestoreOperandsPass());
  PM.add(createVPOCFGRestructuringPass());
#if INTEL_CUSTOMIZATION
  if (OptLevel > 1 && EnableVPOParoptSharedPrivatization)
    // Shared privatization pass should be combined with the argument
    // promotion pass (to do a cleanup) which currently runs only at O2,
    // therefore it is limited to O2 as well.
    PM.add(createVPOParoptSharedPrivatizationPass(RunVPOParopt));
  PM.add(createVPOParoptOptimizeDataSharingPass());
  // No need to rerun VPO CFG restructuring, since
  // VPOParoptOptimizeDataSharing does not modify CFG,
  // and keeps the basic blocks with directive calls
  // consistent.
#endif  // INTEL_CUSTOMIZATION
  PM.add(createVPOParoptPass(RunVPOParopt));
#if INTEL_CUSTOMIZATION
#if INTEL_FEATURE_CSA
  if (RunCSAGraphSplitter)
    PM.add(createCSAGraphSplitterPass());
#endif // INTEL_FEATURE_CSA
  // If vectorizer was required to run then cleanup any remaining directives
  // that were not removed by vectorizer. This applies to all optimization
  // levels since this function is called with RunVec=true in both pass
  // pipelines i.e. -O0 and optlevel >= 1
  //
  // TODO: Issue a warning for any unprocessed directives. Change to
  // assetion failure as the feature matures.
  if (RunVec || EnableDeviceSimd) {
    if (EnableDeviceSimd || (OptLevel == 0 && RunVPOVecopt)) {
      if (OptLevel > 0) {
        addFunctionSimplificationPasses(PM);

        if (RunVPOOpt && EnableVPlanDriver && RunPreLoopOptVPOPasses)
          // Run LLVM-IR VPlan vectorizer before loopopt to vectorize all
          // explicit SIMD loops
          addVPlanVectorizer(PM);

        addLoopOptPasses(PM, false /*IsLTO*/);
      }

      if (RunVPOOpt && EnableVPlanDriver && RunPostLoopOptVPOPasses) {
        if (OptLevel > 0)
          PM.add(createLoopSimplifyPass());
        // Run LLVM-IR VPlan vectorizer after loopopt to vectorize all loops not
        // vectorized after createVPlanDriverHIRPass
        addVPlanVectorizer(PM);
      }
    }

    PM.add(createVPOCFGRestructuringPass());
    PM.add(createVPODirectiveCleanupPass());
  }
#endif // INTEL_CUSTOMIZATION
  // Clean-up empty blocks after OpenMP directives handling.
  PM.add(createVPOCFGSimplifyPass());
  // Paropt transform is complete and SIMD regions are identified. Insert guards
  // for memory motion of pointers (if needed). Renaming is also done to avoid
  // motion of GEPs operating on these pointers.
  PM.add(createVPOCFGRestructuringPass());
  PM.add(createVPOParoptGuardMemoryMotionPass());
  PM.add(createVPOCFGRestructuringPass());
  PM.add(createVPORenameOperandsPass());
#if INTEL_CUSTOMIZATION
  // Paropt transformation pass may produce new AlwaysInline functions.
  // Force inlining for them, if paropt pass runs after the normal inliner.
  if (RunVPOOpt == InvokeParoptAfterInliner) {
    // Run it even at -O0, because the only AlwaysInline functions
    // after paropt are the ones that it artificially created.
    // There is some interference with coroutines passes, which
    // insert some AlwaysInline functions early and expect them
    // to exist up to some other coroutine pass - this is rather
    // a problem of coroutine passes implementation that we may
    // inline those functions here. If it becomes a problem,
    // we will have to resolve that issue with coroutines.
    PM.add(createAlwaysInlinerLegacyPass());
    if (OptLevel > 0)
      // Run GlobalDCE to delete dead functions.
      PM.add(createGlobalDCEPass());
  }
#endif // INTEL_CUSTOMIZATION
}
#endif // INTEL_COLLAB

#if INTEL_CUSTOMIZATION // HIR passes

void PassManagerBuilder::addVPlanVectorizer(legacy::PassManagerBase &PM) const {
  if (OptLevel > 0) {
    PM.add(createLowerSwitchPass(true /*Only for SIMD loops*/));
    // Add LCSSA pass before VPlan driver
    PM.add(createLCSSAPass());
  }
  PM.add(createVPOCFGRestructuringPass());
  // VPO CFG restructuring pass makes sure that the directives of #pragma omp
  // simd ordered are in a separate block. For this reason,
  // VPlanPragmaOmpOrderedSimdExtract pass should run after VPO CFG
  // Restructuring.
  PM.add(createVPlanPragmaOmpOrderedSimdExtractPass());
  // Makes sure #pragma omp if clause will be reduced before VPlan pass
  PM.add(createVPlanPragmaOmpSimdIfPass());
  // Code extractor might add new instructions in the entry block. If the
  // entry block has a directive, than we have to split the entry block. VPlan
  // assumes that the directives are in single-entry single-exit basic blocks.
  PM.add(createVPOCFGRestructuringPass());

  // Create OCL sincos from sin/cos and sincos
  if (OptLevel > 0)
    PM.add(createMathLibraryFunctionsReplacementPass(false /*isOCL*/));

  PM.add(createVPlanDriverPass());

  // Split/translate scalar OCL and vector sincos
  if (OptLevel > 0)
    PM.add(createMathLibraryFunctionsReplacementPass(false /*isOCL*/));

  // The region that is outlined by #pragma omp simd ordered was extracted by
  // VPlanPragmaOmpOrderedSimdExtarct pass. Now, we need to run the inliner in
  // order to put this region back at the code.
  PM.add(createAlwaysInlinerLegacyPass());
  PM.add(createBarrierNoopPass());
  if (OptLevel > 0) {

    // Clean up any SIMD directives left behind by VPlan vectorizer
    PM.add(createVPOCFGRestructuringPass());
    PM.add(createVPODirectiveCleanupPass());
  }
}

void PassManagerBuilder::addLoopOptCleanupPasses(
    legacy::PassManagerBase &PM) const {
  // This pass removes the old (unreachable) code which has been replaced by a
  // new one by HIR.
  PM.add(createCFGSimplificationPass());

  // Cleanup llvm.intel.subscript from code not touched by LoopOpts.
  PM.add(createLowerSubscriptIntrinsicLegacyPass());

  // This is mainly for optimizing away unnecessary alloca load/stores generated
  // by HIR.
  PM.add(createSROAPass());

  // Reassociation helps eliminate redundant computation after looopopt.
  // HIR uses SCEVExpander to generate code. The order of operands in SCEV is
  // not optimal for code generation so we need reassociation to expose
  // redundancies which are then eliminated by GVN/InstCombine.
  // Experimentation showed that Nary reassociate pass is more effective than
  // the regular reassociate pass.
  // This is only run at O3 and higher as it may be compile time expensive.
  if (OptLevel > 2)
    PM.add(createNaryReassociatePass());

  PM.add(createGVNPass(DisableGVNLoadPRE));
  // GVN can perform alloca store forwarding thereby removing alloca loads. This
  // can expose dead alloca stores which can be cleaned up by SROA.
  PM.add(createSROAPass());
  addInstructionCombiningPass(PM, !DTransEnabled);  // INTEL
  PM.add(createLoopCarriedCSEPass());
  PM.add(createDeadStoreEliminationPass());

  if (OptLevel > 2) {
    // Cleanup code with AddSub reassociation.
    PM.add(createAddSubReassociatePass());
  }
}

void PassManagerBuilder::addLoopOptPasses(legacy::PassManagerBase &PM,
                                          bool IsLTO) const {
  INTEL_LIMIT_BEGIN(limitLoopOptOnly, PM)
  // Run additional cleanup passes that help to cleanup the code.
  if (IsLTO) {
    limitFullLoopOptOnly(PM).add(createCFGSimplificationPass());
    limitFullLoopOptOnly(PM).add(createAggressiveDCEPass());
  }

  // This pass "canonicalizes" loops and makes analysis easier.
  PM.add(createLoopSimplifyPass());

  // This lets us generate code for HIR regions independently without concern
  // for livouts from one reigon being livein to another region. It also
  // considerably simplifies handling of liveout values for multi-exit regions.
  PM.add(createLCSSAPass());

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  if (shouldPrintModuleBeforeLoopopt())
    PM.add(createPrintModulePass(dbgs(), ";Module Before HIR"));
#endif //! defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

  // Verify input LLVM IR before doing any HIR transformation.
  if (VerifyInput)
    PM.add(createVerifierPass());

  if (EnableVPlanDriverHIR) {
    PM.add(createVPOCFGRestructuringPass());
    PM.add(createVPlanPragmaOmpOrderedSimdExtractPass());
    PM.add(createVPlanPragmaOmpSimdIfPass());
  }

  if (ConvertToSubs)
    PM.add(createConvertGEPToSubscriptIntrinsicLegacyPass());

  PM.add(createHIRSSADeconstructionLegacyPass());
  // This is expected to be the first pass in the HIR pipeline as it cleans up
  // unnecessary temps from the HIR and doesn't invalidate any analysis. It is
  // considered a part of the framework and therefore ran unconditionally.
  PM.add(createHIRTempCleanupPass());

  if (!RunLoopOptFrameworkOnly) {
    if (vpo::UseOmpRegionsInLoopoptFlag)
      PM.add(createHIRRecognizeParLoopPass());

    PM.add(createHIRPropagateCastedIVPass());

    if (OptLevel > 2) {
      limitFullLoopOptOnly(PM).add(createHIRLoopConcatenationPass());
      limitFullLoopOptOnly(PM).add(
          createHIRPMSymbolicTripCountCompleteUnrollLegacyPass());
      PM.add(createHIRArrayTransposePass());
    }

    // TODO: refine cost model for individual transformations for code size.
    if (SizeLevel == 0) {
      // If VPO is disabled, we don't have to insert ParVec directives.
      if (RunVPOOpt)
        PM.add(createHIRParDirInsertPass());

      PM.add(createHIRConditionalTempSinkingPass());
      PM.add(createHIROptPredicatePass(OptLevel == 3, true));

      INTEL_LIMIT_BEGIN(limitFullLoopOptOnly, PM)
      if (OptLevel > 2) {
        PM.add(createHIRLMMPass(true));
        PM.add(createHIRStoreResultIntoTempArrayPass());
      }
      PM.add(createHIRAosToSoaPass());
      INTEL_LIMIT_END // limitFullLoopOptOnly

      PM.add(createHIRRuntimeDDPass());
      PM.add(createHIRMVForConstUBPass());

      INTEL_LIMIT_BEGIN(limitFullLoopOptOnly, PM)
      if (OptLevel > 2 && IsLTO) {
        PM.add(createHIRRowWiseMVPass());
        PM.add(createHIRSumWindowReusePass());
      }
      INTEL_LIMIT_END // limitFullLoopOptOnly
    }

    PM.add(createHIRSinkingForPerfectLoopnestPass());
    PM.add(createHIRNonZeroSinkingForPerfectLoopnestPass());
    PM.add(createHIRPragmaLoopBlockingPass());

    INTEL_LIMIT_BEGIN(limitFullLoopOptOnly, PM)
    PM.add(createHIRLoopDistributionForLoopNestPass());

#if INTEL_FEATURE_SW_ADVANCED
    if (OptLevel > 2 && IsLTO)
      PM.add(createHIRCrossLoopArrayContractionLegacyPass(
             ThroughputModeOpt != ThroughputMode::SingleJob));
#endif // INTEL_FEATURE_SW_ADVANCED
    PM.add(createHIRLoopInterchangePass());
    INTEL_LIMIT_END // limitFullLoopOptOnly

    PM.add(createHIRGenerateMKLCallPass());

    INTEL_LIMIT_BEGIN(limitFullLoopOptOnly, PM)
#if INTEL_FEATURE_SW_ADVANCED
    if (OptLevel > 2 && IsLTO)
      PM.add(createHIRInterLoopBlockingPass());
#endif // INTEL_FEATURE_SW_ADVANCED

    PM.add(createHIRLoopBlockingPass(ThroughputModeOpt !=
                                     ThroughputMode::SingleJob));
    INTEL_LIMIT_END // limitFullLoopOptOnly

    PM.add(createHIRUndoSinkingForPerfectLoopnestPass());
    PM.add(createHIRDeadStoreEliminationPass());
    PM.add(createHIRLoopReversalPass());
    PM.add(createHIRMinMaxRecognitionPass());
    PM.add(createHIRIdentityMatrixIdiomRecognitionPass());

    if (SizeLevel == 0) {
      PM.add(createHIRPreVecCompleteUnrollPass(OptLevel, DisableUnrollLoops));
    }

    if (ThroughputModeOpt != ThroughputMode::SingleJob)
      PM.add(createHIRConditionalLoadStoreMotionPass());

    if (SizeLevel == 0)
      PM.add(createHIRMemoryReductionSinkingPass());
    PM.add(createHIRLMMPass());
    PM.add(createHIRDeadStoreEliminationPass());
    PM.add(createHIRLastValueComputationPass());
    PM.add(createHIRLoopRerollPass());

    if (SizeLevel == 0) {
      limitFullLoopOptOnly(PM).add(createHIRLoopDistributionForMemRecPass());
    }

    PM.add(createHIRLoopRematerializePass());
    PM.add(createHIRMultiExitLoopRerollPass());
    PM.add(createHIRLoopCollapsePass());
    PM.add(createHIRIdiomRecognitionPass());
    PM.add(createHIRLoopFusionPass());
    PM.add(createHIRIfReversalPass());

    if (SizeLevel == 0) {

      INTEL_LIMIT_BEGIN(limitFullLoopOptOnly, PM)
      PM.add(createHIRUnrollAndJamPass(DisableUnrollLoops));
      PM.add(createHIRMVForVariableStridePass());
      INTEL_LIMIT_END // limitFullLoopOptOnly

      PM.add(createHIROptVarPredicatePass());
      PM.add(createHIROptPredicatePass(OptLevel == 3, false));

      if (RunVPOOpt) {
        PM.add(createHIRVecDirInsertPass(OptLevel == 3));
        if (EnableVPlanDriverHIR) {
          // Enable VPlan HIR Vectorizer
          limitFullLoopOptOnly(PM).add(
              createVPlanDriverHIRPass(false /* Use Lite CM */));
          limitLightLoopOptOnly(PM).add(
              createVPlanDriverHIRPass(true /* Use Lite CM */));
        }
      }
      PM.add(createHIRPostVecCompleteUnrollPass(OptLevel, DisableUnrollLoops));
      PM.add(createHIRGeneralUnrollPass(DisableUnrollLoops));
    }

    PM.add(createHIRScalarReplArrayPass());

    INTEL_LIMIT_BEGIN(limitFullLoopOptOnly, PM)
    if (OptLevel > 2) {
      if (ThroughputModeOpt != ThroughputMode::SingleJob)
        PM.add(createHIRNontemporalMarkingPass());

      PM.add(createHIRPrefetchingPass());
    }
    INTEL_LIMIT_END // limitFullLoopOptOnly
  }

  if (IntelOptReportEmitter == OptReportOptions::HIR)
    PM.add(createHIROptReportEmitterWrapperPass());

  PM.add(createHIRCodeGenWrapperPass());

  addLoopOptCleanupPasses(PM);

  if (EnableVPlanDriverHIR) {
    PM.add(createAlwaysInlinerLegacyPass());
    PM.add(createBarrierNoopPass());
  }
  INTEL_LIMIT_END // limitLoopOptOnly
}

void PassManagerBuilder::addLoopOptAndAssociatedVPOPasses(
    legacy::PassManagerBase &PM, bool IsLTO) {
  // Do not run loop optimization passes, if proprietary optimizations
  // are disabled. There are some mandatory clean-up actions that still need
  // to be performed.
  if (DisableIntelProprietaryOpts) {
    // CMPLRLLVM-25935: clean-up VPO directives for targets with
    //                  LLVM IR emission enabled (hence, with proprietary
    //                  optimizations disabled).
    PM.add(createVPOCFGRestructuringPass());
    PM.add(createVPODirectiveCleanupPass());
    return;
  }

  if (RunVPOOpt && RunVecClone) {
    PM.add(createVecClonePass());
    // VecClonePass can generate redundant geps/loads for vector parameters when
    // accessing elem[i] within the inserted simd loop. This makes DD testing
    // harder, so run CSE here to do some clean-up before HIR construction.
    PM.add(createEarlyCSEPass());
  }

  if (RunVPOOpt && EnableVPlanDriver && RunPreLoopOptVPOPasses)
    // Run LLVM-IR VPlan vectorizer before loopopt to vectorize all explicit
    // SIMD loops
    addVPlanVectorizer(PM);

  addLoopOptPasses(PM, IsLTO);

  if (RunVPOOpt && EnableVPlanDriver && RunPostLoopOptVPOPasses) {
    if (OptLevel > 0)
      PM.add(createLoopSimplifyPass());
    // Run LLVM-IR VPlan vectorizer after loopopt to vectorize all loops not
    // vectorized after createVPlanDriverHIRPass
    addVPlanVectorizer(PM);
  }

  // Process directives inserted by LoopOpt Autopar.
  // Call with RunVec==true (2nd argument) to cleanup any vec directives
  // that loopopt and vectorizers might have missed.
  if (RunVPOOpt)
    addVPOPasses(PM, true);

  if (IntelOptReportEmitter == OptReportOptions::IR)
    PM.add(createOptReportEmitterLegacyPass());
}


void PassManagerBuilder::populateThinLTOPassManager(
    legacy::PassManagerBase &PM) {
  if (LibraryInfo)
    PM.add(new TargetLibraryInfoWrapperPass(*LibraryInfo));

  if (VerifyInput)
    PM.add(createVerifierPass());

  populateModulePassManager(PM);

  if (VerifyOutput)
    PM.add(createVerifierPass());
}

void PassManagerBuilder::populateLTOPassManager(legacy::PassManagerBase &PM) {
  if (LibraryInfo)
    PM.add(new TargetLibraryInfoWrapperPass(*LibraryInfo));

  if (VerifyInput)
    PM.add(createVerifierPass());

  addExtensionsToPM(EP_FullLinkTimeOptimizationEarly, PM);

  if (OptLevel != 0)
    addLTOOptimizationPasses(PM);

  // Create a function that performs CFI checks for cross-DSO calls with targets
  // in the current module.
  PM.add(createCrossDSOCFIPass());

  if (OptLevel != 0)
    addLateLTOOptimizationPasses(PM);

  addExtensionsToPM(EP_FullLinkTimeOptimizationLast, PM);

  PM.add(createAnnotationRemarksLegacyPass());

  if (VerifyOutput)
    PM.add(createVerifierPass());
}
#endif // INTEL_CUSTOMIZATION

LLVMPassManagerBuilderRef LLVMPassManagerBuilderCreate() {
  PassManagerBuilder *PMB = new PassManagerBuilder();
  return wrap(PMB);
}

void LLVMPassManagerBuilderDispose(LLVMPassManagerBuilderRef PMB) {
  PassManagerBuilder *Builder = unwrap(PMB);
  delete Builder;
}

void
LLVMPassManagerBuilderSetOptLevel(LLVMPassManagerBuilderRef PMB,
                                  unsigned OptLevel) {
  PassManagerBuilder *Builder = unwrap(PMB);
  Builder->OptLevel = OptLevel;
}

void
LLVMPassManagerBuilderSetSizeLevel(LLVMPassManagerBuilderRef PMB,
                                   unsigned SizeLevel) {
  PassManagerBuilder *Builder = unwrap(PMB);
  Builder->SizeLevel = SizeLevel;
}

void
LLVMPassManagerBuilderSetDisableUnitAtATime(LLVMPassManagerBuilderRef PMB,
                                            LLVMBool Value) {
  // NOTE: The DisableUnitAtATime switch has been removed.
}

void
LLVMPassManagerBuilderSetDisableUnrollLoops(LLVMPassManagerBuilderRef PMB,
                                            LLVMBool Value) {
  PassManagerBuilder *Builder = unwrap(PMB);
  Builder->DisableUnrollLoops = Value;
}

void
LLVMPassManagerBuilderSetDisableSimplifyLibCalls(LLVMPassManagerBuilderRef PMB,
                                                 LLVMBool Value) {
  // NOTE: The simplify-libcalls pass has been removed.
}

void
LLVMPassManagerBuilderUseInlinerWithThreshold(LLVMPassManagerBuilderRef PMB,
                                              unsigned Threshold) {
  PassManagerBuilder *Builder = unwrap(PMB);
  Builder->Inliner = createFunctionInliningPass(Threshold);
}

void
LLVMPassManagerBuilderPopulateFunctionPassManager(LLVMPassManagerBuilderRef PMB,
                                                  LLVMPassManagerRef PM) {
  PassManagerBuilder *Builder = unwrap(PMB);
  legacy::FunctionPassManager *FPM = unwrap<legacy::FunctionPassManager>(PM);
  Builder->populateFunctionPassManager(*FPM);
}

void
LLVMPassManagerBuilderPopulateModulePassManager(LLVMPassManagerBuilderRef PMB,
                                                LLVMPassManagerRef PM) {
  PassManagerBuilder *Builder = unwrap(PMB);
  legacy::PassManagerBase *MPM = unwrap(PM);
  Builder->populateModulePassManager(*MPM);
}

#if INTEL_CUSTOMIZATION
void LLVMPassManagerBuilderPopulateLTOPassManager(LLVMPassManagerBuilderRef PMB,
                                                  LLVMPassManagerRef PM,
                                                  LLVMBool Internalize,
                                                  LLVMBool RunInliner) {
  PassManagerBuilder *Builder = unwrap(PMB);
  legacy::PassManagerBase *LPM = unwrap(PM);

  // A small backwards compatibility hack. populateLTOPassManager used to take
  // an RunInliner option.
  if (RunInliner && !Builder->Inliner)
    Builder->Inliner = createFunctionInliningPass();

  Builder->populateLTOPassManager(*LPM);
}
#endif // INTEL_CUSTOMIZATION
