//===- PassManagerBuilder.cpp - Build Standard Pass -----------------------===//
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
#include "llvm/ADT/SmallVector.h"
#include "llvm/Analysis/BasicAliasAnalysis.h"
#include "llvm/Analysis/CFLAndersAliasAnalysis.h"
#include "llvm/Analysis/CFLSteensAliasAnalysis.h"
#include "llvm/Analysis/GlobalsModRef.h"
#include "llvm/Analysis/InlineCost.h"
#if INTEL_CUSTOMIZATION
#include "llvm/Analysis/Intel_AggInline.h"
#include "llvm/Analysis/Intel_Andersens.h"
#include "llvm/Analysis/Intel_StdContainerAA.h"
#include "llvm/Analysis/Intel_WP.h"
#include "llvm/Analysis/Intel_XmainOptLevelPass.h"
#include "llvm/Analysis/Intel_OptReport/OptReportOptionsPass.h"
#endif // INTEL_CUSTOMIZATION
#include "llvm/Analysis/Passes.h"
#include "llvm/Analysis/ScopedNoAliasAA.h"
#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/Analysis/TypeBasedAliasAnalysis.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/ManagedStatic.h"
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
#include "llvm/Transforms/Scalar/InstSimplifyPass.h"
#include "llvm/Transforms/Scalar/LICM.h"
#include "llvm/Transforms/Scalar/LoopUnrollPass.h"
#include "llvm/Transforms/Scalar/SimpleLoopUnswitch.h"
#include "llvm/Transforms/Utils.h"
#include "llvm/Transforms/Vectorize.h"
#include "llvm/Transforms/Vectorize/LoopVectorize.h"
#include "llvm/Transforms/Vectorize/SLPVectorizer.h"
#if INTEL_CUSTOMIZATION
#include "llvm/Transforms/Instrumentation/Intel_FunctionSplitting.h"
#include "llvm/Transforms/Intel_LoopTransforms/Passes.h"
#include "llvm/IR/IRPrintingPasses.h"
#include "llvm/Transforms/Utils/Intel_VecClone.h"
#include "llvm/Transforms/Intel_MapIntrinToIml/MapIntrinToIml.h"
#include "llvm/Transforms/IPO/Inliner.h"
#include "llvm/Transforms/IPO/Intel_InlineLists.h"
#include "llvm/Transforms/IPO/Intel_InlineReportEmitter.h"
#include "llvm/Transforms/IPO/Intel_InlineReportSetup.h"
#include "llvm/Transforms/IPO/Intel_OptimizeDynamicCasts.h"
#include "llvm/Transforms/Scalar/Intel_MultiVersioning.h"

#if INTEL_INCLUDE_DTRANS
#include "Intel_DTrans/DTransCommon.h"
#endif // INTEL_INCLUDE_DTRANS
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

using namespace llvm::llvm_intel_wp_analysis;

extern cl::opt<unsigned> IntelInlineReportLevel;

static cl::opt<bool> ConvertToSubs(
    "convert-to-subs-before-loopopt", cl::init(false), cl::ReallyHidden,
    cl::desc("Enables conversion of GEPs to subscripts before loopopt"));

static cl::opt<bool>
EarlyJumpThreading("early-jump-threading", cl::init(true), cl::Hidden,
                   cl::desc("Run the early jump threading pass"));
static cl::opt<bool>
EnableLV("enable-lv", cl::init(false), cl::Hidden,
         cl::desc("Enable community loop vectorizer"));

static cl::opt<bool> EnableLoadCoalescing("enable-load-coalescing",
                                          cl::init(true), cl::Hidden,
                                          cl::ZeroOrMore,
                                          cl::desc("Enable load coalescing"));

static cl::opt<bool>
    EnableSROAAfterSLP("enable-sroa-after-slp", cl::init(true), cl::Hidden,
                       cl::desc("Run SROA pass after the SLP vectorizer"));

#endif // INTEL_CUSTOMIZATION
static cl::opt<bool>
    RunPartialInlining("enable-partial-inlining", cl::init(false), cl::Hidden,
                       cl::ZeroOrMore, cl::desc("Run Partial inlinining pass"));

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

static cl::opt<bool> ExtraVectorizerPasses(
    "extra-vectorizer-passes", cl::init(false), cl::Hidden,
    cl::desc("Run cleanup optimization passes after vectorization."));

static cl::opt<bool>
RunLoopRerolling("reroll-loops", cl::Hidden,
                 cl::desc("Run the loop rerolling pass"));

static cl::opt<bool> RunNewGVN("enable-newgvn", cl::init(false), cl::Hidden,
                               cl::desc("Run the NewGVN pass"));

// Experimental option to use CFL-AA
enum class CFLAAType { None, Steensgaard, Andersen, Both };
static cl::opt<CFLAAType>
    UseCFLAA("use-cfl-aa", cl::init(CFLAAType::None), cl::Hidden,
             cl::desc("Enable the new, experimental CFL alias analysis"),
             cl::values(clEnumValN(CFLAAType::None, "none", "Disable CFL-AA"),
                        clEnumValN(CFLAAType::Steensgaard, "steens",
                                   "Enable unification-based CFL-AA"),
                        clEnumValN(CFLAAType::Andersen, "anders",
                                   "Enable inclusion-based CFL-AA"),
                        clEnumValN(CFLAAType::Both, "both",
                                   "Enable both variants of CFL-AA")));

static cl::opt<bool> EnableLoopInterchange(
    "enable-loopinterchange", cl::init(false), cl::Hidden,
    cl::desc("Enable the new, experimental LoopInterchange Pass"));

static cl::opt<bool> EnableUnrollAndJam("enable-unroll-and-jam",
                                        cl::init(false), cl::Hidden,
                                        cl::desc("Enable Unroll And Jam Pass"));

#if INTEL_COLLAB
enum { InvokeParoptBeforeInliner = 1, InvokeParoptAfterInliner };
static cl::opt<unsigned> RunVPOOpt("vpoopt", cl::init(InvokeParoptAfterInliner),
                                   cl::Hidden, cl::desc("Runs all VPO passes"));

// The user can use -mllvm -paropt=<mode> to enable various paropt
// transformations, where <mode> is a bit vector (see enum VPOParoptMode
// for a description of the bits.) For example, paropt=0x7 enables
// "ParPrepare" (0x1), "ParTrans" (0x2), and "OmpPar" (0x4).
static cl::opt<unsigned> RunVPOParopt("paropt",
  cl::init(0x00000000), cl::Hidden,
  cl::desc("Run VPO Paropt Pass"));
#endif // INTEL_COLLAB

#if INTEL_CUSTOMIZATION
static cl::opt<bool> RunVPOVecopt("vecopt",
  cl::init(false), cl::Hidden,
  cl::desc("Run VPO Vecopt Pass"));

// Switch to enable or disable all VPO related pre-loopopt passes
static cl::opt<bool>
    RunPreLoopOptVPOPasses("pre-loopopt-vpo-passes", cl::init(true), cl::Hidden,
                           cl::desc("Run VPO passes before loopot"));

// Set LLVM-IR VPlan driver pass to be enabled by default
static cl::opt<bool> EnableVPlanDriver("vplan-driver", cl::init(true),
                                       cl::Hidden,
                                       cl::desc("Enable VPlan Driver"));

static cl::opt<bool> RunVecClone("enable-vec-clone",
  cl::init(false), cl::Hidden,
  cl::desc("Run Vector Function Cloning"));

static cl::opt<bool> EnableVPlanDriverHIR("vplan-driver-hir", cl::init(true),
                                       cl::Hidden,
                                       cl::desc("Enable VPlan Driver"));
// INTEL - HIR passes
enum class LoopOptMode { None, LightWeight, Full };
static cl::opt<LoopOptMode> RunLoopOpts(
    "loopopt", cl::init(LoopOptMode::None), cl::Hidden, cl::ValueOptional,
    cl::desc("Runs loop optimization passes"),
    cl::values(clEnumValN(LoopOptMode::None, "0", "Disable loopopt passes"),
               clEnumValN(LoopOptMode::LightWeight, "1",
                          "Enable lightweight loopopt(minimal passes)"),
               clEnumValN(LoopOptMode::Full, "2", "Enable all loopopt passes"),
               // Value assumed when just -loopopt is specified.
               clEnumValN(LoopOptMode::Full, "", "")));

static cl::opt<bool> RunLoopOptFrameworkOnly("loopopt-framework-only",
    cl::init(false), cl::Hidden,
    cl::desc("Enables loopopt framework without any transformation passes"));

static cl::opt<bool> PrintModuleBeforeLoopopt(
    "print-module-before-loopopt", cl::init(false), cl::Hidden,
    cl::desc("Prints LLVM module to dbgs() before first HIR transform(HIR SSA "
             "deconstruction)"));

// register promotion for global vars at -O2 and above.
static cl::opt<bool> EnableNonLTOGlobalVarOpt(
    "enable-non-lto-global-var-opt", cl::init(true), cl::Hidden,
    cl::desc("Enable register promotion for global vars outside of the LTO."));

// Std Container Optimization at -O2 and above.
static cl::opt<bool> EnableStdContainerOpt("enable-std-container-opt",
                                           cl::init(true), cl::Hidden,
                                           cl::desc("Enable Std Container Optimization"));

static cl::opt<bool> EnableTbaaProp("enable-tbaa-prop", cl::init(true),
                                    cl::Hidden,
                                    cl::desc("Enable Tbaa Propagation"));

// Andersen AliasAnalysis
static cl::opt<bool> EnableAndersen("enable-andersen", cl::init(true),
    cl::Hidden, cl::desc("Enable Andersen's Alias Analysis"));

// Indirect call Conv
static cl::opt<bool> EnableIndirectCallConv("enable-ind-call-conv",
    cl::init(true), cl::Hidden, cl::desc("Enable Indirect Call Conv"));

// Whole Program Analysis
static cl::opt<bool> EnableWPA("enable-whole-program-analysis",
    cl::init(true), cl::Hidden, cl::desc("Enable Whole Program Analysis"));

// IP Cloning
static cl::opt<bool> EnableIPCloning("enable-ip-cloning",
    cl::init(true), cl::Hidden, cl::desc("Enable IP Cloning"));

// Call Tree Cloning
static cl::opt<bool> EnableCallTreeCloning("enable-call-tree-cloning",
    cl::init(true), cl::Hidden, cl::desc("Enable Call Tree Cloning"));

// Inline Aggressive Analysis
static cl::opt<bool>
    EnableInlineAggAnalysis("enable-inline-aggressive-analysis",
    cl::init(true), cl::Hidden, cl::desc("Enable Inline Aggressive Analysis"));

#if INTEL_INCLUDE_DTRANS
// DTrans optimizations -- this is a placeholder for future work.
static cl::opt<bool> EnableDTrans("enable-dtrans",
    cl::init(false), cl::Hidden,
    cl::desc("Enable DTrans optimizations"));

// Partial inline simple functions
static cl::opt<bool>
    EnableIntelPI("enable-intelpi", cl::init(true), cl::Hidden,
                    cl::desc("Enable partial inlining for simple functions"));
#endif // INTEL_INCLUDE_DTRANS

// PGO based function splitting
static cl::opt<bool> EnableFunctionSplitting("enable-function-splitting",
  cl::init(false), cl::Hidden,
  cl::desc("Enable function splitting optimization based on PGO data"));

// Function multi-versioning.
static cl::opt<bool> EnableMultiVersioning("enable-multiversioning",
  cl::init(false), cl::Hidden,
  cl::desc("Enable Function Multi-versioning"));

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
#endif // INTEL_CUSTOMIZATION

static cl::opt<bool>
    EnablePrepareForThinLTO("prepare-for-thinlto", cl::init(false), cl::Hidden,
                            cl::desc("Enable preparation for ThinLTO."));

static cl::opt<bool>
    EnablePerformThinLTO("perform-thinlto", cl::init(false), cl::Hidden,
                         cl::desc("Enable performing ThinLTO."));

cl::opt<bool> EnableHotColdSplit("hot-cold-split", cl::init(false), cl::Hidden,
    cl::desc("Enable hot-cold splitting pass"));

static cl::opt<bool> UseLoopVersioningLICM(
    "enable-loop-versioning-licm", cl::init(false), cl::Hidden,
    cl::desc("Enable the experimental Loop Versioning LICM pass"));

static cl::opt<bool>
    DisablePreInliner("disable-preinline", cl::init(false), cl::Hidden,
                      cl::desc("Disable pre-instrumentation inliner"));

static cl::opt<int> PreInlineThreshold(
    "preinline-threshold", cl::Hidden, cl::init(75), cl::ZeroOrMore,
    cl::desc("Control the amount of inlining in pre-instrumentation inliner "
             "(default = 75)"));

static cl::opt<bool> EnableGVNHoist(
    "enable-gvn-hoist", cl::init(false), cl::Hidden,
    cl::desc("Enable the GVN hoisting pass (default = off)"));

static cl::opt<bool>
    DisableLibCallsShrinkWrap("disable-libcalls-shrinkwrap", cl::init(false),
                              cl::Hidden,
                              cl::desc("Disable shrink-wrap library calls"));

static cl::opt<bool> EnableSimpleLoopUnswitch(
    "enable-simple-loop-unswitch", cl::init(false), cl::Hidden,
    cl::desc("Enable the simple loop unswitch pass. Also enables independent "
             "cleanup passes integrated into the loop pass manager pipeline."));

static cl::opt<bool> EnableGVNSink(
    "enable-gvn-sink", cl::init(false), cl::Hidden,
    cl::desc("Enable the GVN sinking pass (default = off)"));

// This option is used in simplifying testing SampleFDO optimizations for
// profile loading.
static cl::opt<bool>
    EnableCHR("enable-chr", cl::init(true), cl::Hidden,
              cl::desc("Enable control height reduction optimization (CHR)"));

cl::opt<bool> FlattenedProfileUsed(
    "flattened-profile-used", cl::init(false), cl::Hidden,
    cl::desc("Indicate the sample profile being used is flattened, i.e., "
             "no inline hierachy exists in the profile. "));

cl::opt<bool> EnableOrderFileInstrumentation(
    "enable-order-file-instrumentation", cl::init(false), cl::Hidden,
    cl::desc("Enable order file instrumentation (default = off)"));

PassManagerBuilder::PassManagerBuilder() {
    OptLevel = 2;
    SizeLevel = 0;
    LibraryInfo = nullptr;
    Inliner = nullptr;
    DisableUnrollLoops = false;
    SLPVectorize = RunSLPVectorization;
    LoopVectorize = EnableLoopVectorization;
    LoopsInterleaved = EnableLoopInterleaving;
    RerollLoops = RunLoopRerolling;
    NewGVN = RunNewGVN;
    LicmMssaOptCap = SetLicmMssaOptCap;
    LicmMssaNoAccForPromotionCap = SetLicmMssaNoAccForPromotionCap;
    DisableGVNLoadPRE = false;
    ForgetAllSCEVInLoopUnroll = ForgetSCEVInLoopUnroll;
    VerifyInput = false;
    VerifyOutput = false;
    MergeFunctions = false;
    PrepareForLTO = false;
    EnablePGOInstrGen = false;
    EnablePGOCSInstrGen = false;
    EnablePGOCSInstrUse = false;
    PGOInstrGen = "";
    PGOInstrUse = "";
    PGOSampleUse = "";
    PrepareForThinLTO = EnablePrepareForThinLTO;
    PerformThinLTO = EnablePerformThinLTO;
    DivergentTarget = false;
#if INTEL_CUSTOMIZATION
    DisableIntelProprietaryOpts = false;
#endif // INTEL_CUSTOMIZATION
}

PassManagerBuilder::~PassManagerBuilder() {
  delete LibraryInfo;
  delete Inliner;
}

/// Set of global extensions, automatically added as part of the standard set.
static ManagedStatic<SmallVector<std::pair<PassManagerBuilder::ExtensionPointTy,
   PassManagerBuilder::ExtensionFn>, 8> > GlobalExtensions;

/// Check if GlobalExtensions is constructed and not empty.
/// Since GlobalExtensions is a managed static, calling 'empty()' will trigger
/// the construction of the object.
static bool GlobalExtensionsNotEmpty() {
  return GlobalExtensions.isConstructed() && !GlobalExtensions->empty();
}

void PassManagerBuilder::addGlobalExtension(
    PassManagerBuilder::ExtensionPointTy Ty,
    PassManagerBuilder::ExtensionFn Fn) {
  GlobalExtensions->push_back(std::make_pair(Ty, std::move(Fn)));
}

void PassManagerBuilder::addExtension(ExtensionPointTy Ty, ExtensionFn Fn) {
  Extensions.push_back(std::make_pair(Ty, std::move(Fn)));
}

void PassManagerBuilder::addExtensionsToPM(ExtensionPointTy ETy,
                                           legacy::PassManagerBase &PM) const {
  if (GlobalExtensionsNotEmpty()) {
    for (auto &Ext : *GlobalExtensions) {
      if (Ext.first == ETy)
        Ext.second(*this, PM);
    }
  }
  for (unsigned i = 0, e = Extensions.size(); i != e; ++i)
    if (Extensions[i].first == ETy)
      Extensions[i].second(*this, PM);
}

void PassManagerBuilder::addInitialAliasAnalysisPasses(
    legacy::PassManagerBase &PM) const {
  switch (UseCFLAA) {
  case CFLAAType::Steensgaard:
    PM.add(createCFLSteensAAWrapperPass());
    break;
  case CFLAAType::Andersen:
    PM.add(createCFLAndersAAWrapperPass());
    break;
  case CFLAAType::Both:
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

void PassManagerBuilder::addInstructionCombiningPass(
    legacy::PassManagerBase &PM) const {
  bool ExpensiveCombines = OptLevel > 2;
#if INTEL_CUSTOMIZATION
#if INTEL_INCLUDE_DTRANS
  bool GEPInstOptimizations = !(PrepareForLTO && EnableDTrans);
#else
  bool GEPInstOptimizations = true;
#endif // INTEL_INCLUDE_DTRANS
  PM.add(
      createInstructionCombiningPass(ExpensiveCombines,
                                     GEPInstOptimizations));
#endif // INTEL_CUSTOMIZATION
}

void PassManagerBuilder::populateFunctionPassManager(
    legacy::FunctionPassManager &FPM) {
  addExtensionsToPM(EP_EarlyAsPossible, FPM);
#if INTEL_CUSTOMIZATION
  if (isLoopOptEnabled())
    FPM.add(createLoopOptMarkerLegacyPass());
  else
    FPM.add(createLowerSubscriptIntrinsicLegacyPass());
#endif // INTEL_CUSTOMIZATION
  FPM.add(createEntryExitInstrumenterPass());

  // Add LibraryInfo if we have some.
  if (LibraryInfo)
    FPM.add(new TargetLibraryInfoWrapperPass(*LibraryInfo));

#if INTEL_CUSTOMIZATION
  FPM.add(createXmainOptLevelWrapperPass(OptLevel));
#endif // INTEL_CUSTOMIZATION
#if INTEL_COLLAB
  if (RunVPOOpt && RunVPOParopt) {
    FPM.add(createVPOCFGRestructuringPass());
    FPM.add(createVPOParoptPreparePass(RunVPOParopt));
    if (OptLevel == 0) {
      // OpenMP also needs CFGSimplify at -O0. For some loops which are proven
      // to have only one iteration the FE may skip the BB doing loop increment
      // and exit the loop directly, leaving the increment BB without any
      // predecessor, while its successor is still part of the WRN. This causes
      // code extractor later to assert ("No blocks in this region may have
      // entries from outside the region").
      // CFGSimplify removes the dead code in the increment BB, fixing this.
      // NOTE: It is important to do this after VPOParoptPrepare. Otherwise,
      // CFGSimplify could modify the IR and prevent codegen of Openmp
      // constructs transformed in the Prepare pass, such as ATOMIC.
#if INTEL_CUSTOMIZATION
      // [This affects ompoC/fmt7bc-1.c.]
#endif // INTEL_CUSTOMIZATION
      FPM.add(createCFGSimplificationPass());
    }
  }
#endif // INTEL_COLLAB

  if (OptLevel == 0) return;

  addInitialAliasAnalysisPasses(FPM);

  FPM.add(createCFGSimplificationPass());
  FPM.add(createSROAPass());
  FPM.add(createEarlyCSEPass());
  FPM.add(createLowerExpectIntrinsicPass());
}

// Do PGO instrumentation generation or use pass as the option specified.
void PassManagerBuilder::addPGOInstrPasses(legacy::PassManagerBase &MPM,
                                           bool IsCS = false) {
  if (IsCS) {
    if (!EnablePGOCSInstrGen && !EnablePGOCSInstrUse)
      return;
  } else if (!EnablePGOInstrGen && PGOInstrUse.empty() && PGOSampleUse.empty())
    return;

  // Perform the preinline and cleanup passes for O1 and above.
  // And avoid doing them if optimizing for size.
  // We will not do this inline for context sensitive PGO (when IsCS is true).
  if (OptLevel > 0 && SizeLevel == 0 && !DisablePreInliner &&
      PGOSampleUse.empty() && !IsCS) {
    // Create preinline pass. We construct an InlineParams object and specify
    // the threshold here to avoid the command line options of the regular
    // inliner to influence pre-inlining. The only fields of InlineParams we
    // care about are DefaultThreshold and HintThreshold.
    InlineParams IP;
    IP.DefaultThreshold = PreInlineThreshold;
    // FIXME: The hint threshold has the same value used by the regular inliner.
    // This should probably be lowered after performance testing.
    IP.HintThreshold = 325;
    IP.PrepareForLTO = PrepareForLTO; // INTEL

    MPM.add(createFunctionInliningPass(IP));
    MPM.add(createSROAPass());
    MPM.add(createEarlyCSEPass());             // Catch trivial redundancies
    MPM.add(createCFGSimplificationPass());    // Merge & remove BBs
#if INTEL_CUSTOMIZATION
#if INTEL_INCLUDE_DTRANS
    // Configure the instruction combining pass to avoid some transformations
    // that lose type information for DTrans.
    bool GEPInstOptimizations = !(PrepareForLTO && EnableDTrans);
#else
    bool GEPInstOptimizations = true;
#endif // INTEL_INCLUDE_DTRANS
    MPM.add(createInstructionCombiningPass(
        /*ExpensiveCombines=default value*/ true,
        GEPInstOptimizations)); // Combine silly seq's
#endif                          // INTEL_CUSTOMIZATION
    addExtensionsToPM(EP_Peephole, MPM);
  }
  if ((EnablePGOInstrGen && !IsCS) || (EnablePGOCSInstrGen && IsCS)) {
    MPM.add(createPGOInstrumentationGenLegacyPass(IsCS));
    // Add the profile lowering pass.
    InstrProfOptions Options;
    if (!PGOInstrGen.empty())
      Options.InstrProfileOutput = PGOInstrGen;
    Options.DoCounterPromotion = true;
    Options.UseBFIInPromotion = IsCS;
    MPM.add(createLoopRotatePass());
    MPM.add(createInstrProfilingLegacyPass(Options, IsCS));
  }
  if (!PGOInstrUse.empty())
    MPM.add(createPGOInstrumentationUseLegacyPass(PGOInstrUse, IsCS));
  // Indirect call promotion that promotes intra-module targets only.
  // For ThinLTO this is done earlier due to interactions with globalopt
  // for imported functions. We don't run this at -O0.
  if (OptLevel > 0 && !IsCS)
    MPM.add(
        createPGOIndirectCallPromotionLegacyPass(false, !PGOSampleUse.empty()));

#if INTEL_CUSTOMIZATION
  // The function splitting pass uses the PGO frequency info, and only
  // makes sense to run during profile feedback.
  if (EnableFunctionSplitting &&
    (!PGOInstrUse.empty() || !PGOSampleUse.empty())) {
    MPM.add(createFunctionSplittingWrapperPass());
  }
#endif // INTEL_CUSTOMIZATION
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
  MPM.add(createSROAPass());
  MPM.add(createEarlyCSEPass(true /* Enable mem-ssa. */)); // Catch trivial redundancies
  if (EnableGVNHoist)
    MPM.add(createGVNHoistPass());
  if (EnableGVNSink) {
    MPM.add(createGVNSinkPass());
    MPM.add(createCFGSimplificationPass());
  }

  // Speculative execution if the target has divergent branches; otherwise nop.
  MPM.add(createSpeculativeExecutionIfHasBranchDivergencePass());
  MPM.add(createJumpThreadingPass());         // Thread jumps.
  MPM.add(createCorrelatedValuePropagationPass()); // Propagate conditionals
  MPM.add(createCFGSimplificationPass());     // Merge & remove BBs
  // Combine silly seq's
  if (OptLevel > 2)
    MPM.add(createAggressiveInstCombinerPass());
  addInstructionCombiningPass(MPM);
  if (SizeLevel == 0 && !DisableLibCallsShrinkWrap)
    MPM.add(createLibCallsShrinkWrapPass());
  addExtensionsToPM(EP_Peephole, MPM);

  // Optimize memory intrinsic calls based on the profiled size information.
  if (SizeLevel == 0)
    MPM.add(createPGOMemOPSizeOptLegacyPass());

#if INTEL_CUSTOMIZATION
#if INTEL_INCLUDE_DTRANS
  bool SkipRecProgression = PrepareForLTO && EnableDTrans;
#else
  bool SkipRecProgression = false;
#endif // INTEL_INCLUDE_DTRANS
  MPM.add(createTailCallEliminationPass(SkipRecProgression));
                                              // Eliminate tail calls
#endif // INTEL_CUSTOMIZATION
  MPM.add(createCFGSimplificationPass());     // Merge & remove BBs
  MPM.add(createReassociatePass());           // Reassociate expressions

  // Begin the loop pass pipeline.
  if (EnableSimpleLoopUnswitch) {
    // The simple loop unswitch pass relies on separate cleanup passes. Schedule
    // them first so when we re-process a loop they run before other loop
    // passes.
    MPM.add(createLoopInstSimplifyPass());
    MPM.add(createLoopSimplifyCFGPass());
  }
  // Rotate Loop - disable header duplication at -Oz
  MPM.add(createLoopRotatePass(SizeLevel == 2 ? 0 : -1));
  MPM.add(createLICMPass(LicmMssaOptCap, LicmMssaNoAccForPromotionCap));
  if (EnableSimpleLoopUnswitch)
    MPM.add(createSimpleLoopUnswitchLegacyPass());
  else
    MPM.add(createLoopUnswitchPass(SizeLevel || OptLevel < 3, DivergentTarget));
  // FIXME: We break the loop pass pipeline here in order to do full
  // simplify-cfg. Eventually loop-simplifycfg should be enhanced to replace the
  // need for this.
  MPM.add(createCFGSimplificationPass());
  addInstructionCombiningPass(MPM);
  // We resume loop passes creating a second loop pipeline here.
  MPM.add(createIndVarSimplifyPass());        // Canonicalize indvars
  MPM.add(createLoopIdiomPass());             // Recognize idioms like memset.
  addExtensionsToPM(EP_LateLoopOptimizations, MPM);
  MPM.add(createLoopDeletionPass());          // Delete dead loops

  if (EnableLoopInterchange)
    MPM.add(createLoopInterchangePass()); // Interchange loops

  // INTEL - HIR complete unroll pass replaces LLVM's simple loop unroll pass.
  if (!isLoopOptEnabled()) // INTEL
    MPM.add(createSimpleLoopUnrollPass(OptLevel,  // INTEL
                                     DisableUnrollLoops, // Unroll small loops
                                     ForgetAllSCEVInLoopUnroll));
#if INTEL_CUSTOMIZATION
#if INTEL_FEATURE_CSA
  MPM.add(createLoopSPMDizationPass());
#endif  // INTEL_FEATURE_CSA
#endif  // INTEL_CUSTOMIZATION
  addExtensionsToPM(EP_LoopOptimizerEnd, MPM);
  // This ends the loop pass pipelines.

  if (OptLevel > 1) {
    MPM.add(createMergedLoadStoreMotionPass()); // Merge ld/st in diamonds
    MPM.add(NewGVN ? createNewGVNPass()
                   : createGVNPass(DisableGVNLoadPRE)); // Remove redundancies
  }
#if INTEL_CUSTOMIZATION
#if INTEL_INCLUDE_DTRANS
  // Skip MemCpyOpt when both PrepareForLTO and EnableDTrans flags are
  // true to simplify handling of memcpy/memset/memmov calls in DTrans
  // implementation.
  // TODO: Remove this customization once DTrans handled partial memcpy/
  // memset/memmov calls of struct types.
  if (!PrepareForLTO || !EnableDTrans)
    MPM.add(createMemCpyOptPass());           // Remove memcpy / form memset
#else
  MPM.add(createMemCpyOptPass());             // Remove memcpy / form memset
#endif // INTEL_INCLUDE_DTRANS
#endif // INTEL_CUSTOMIZATION
  MPM.add(createSCCPPass());                  // Constant prop with SCCP

  // Delete dead bit computations (instcombine runs after to fold away the dead
  // computations, and then ADCE will run later to exploit any new DCE
  // opportunities that creates).
  MPM.add(createBitTrackingDCEPass());        // Delete dead bit computations

  // Run instcombine after redundancy elimination to exploit opportunities
  // opened up by them.
  addInstructionCombiningPass(MPM);
  addExtensionsToPM(EP_Peephole, MPM);
  MPM.add(createJumpThreadingPass());         // Thread jumps
  MPM.add(createCorrelatedValuePropagationPass());
  MPM.add(createDeadStoreEliminationPass());  // Delete dead stores
  MPM.add(createLICMPass(LicmMssaOptCap, LicmMssaNoAccForPromotionCap));

  addExtensionsToPM(EP_ScalarOptimizerLate, MPM);

  if (RerollLoops)
    MPM.add(createLoopRerollPass());

  MPM.add(createAggressiveDCEPass());         // Delete dead instructions
  MPM.add(createCFGSimplificationPass()); // Merge & remove BBs
  // Clean up after everything.
  addInstructionCombiningPass(MPM);
  addExtensionsToPM(EP_Peephole, MPM);

  if (EnableCHR && OptLevel >= 3 &&
      (!PGOInstrUse.empty() || !PGOSampleUse.empty() || EnablePGOCSInstrGen))
    MPM.add(createControlHeightReductionLegacyPass());
}

void PassManagerBuilder::populateModulePassManager(
    legacy::PassManagerBase &MPM) {
  // Whether this is a default or *LTO pre-link pipeline. The FullLTO post-link
  // is handled separately, so just check this is not the ThinLTO post-link.
  bool DefaultOrPreLinkPipeline = !PerformThinLTO;

  MPM.add(createXmainOptLevelWrapperPass(OptLevel)); // INTEL
  if (!PGOSampleUse.empty()) {
    MPM.add(createPruneEHPass());
    // In ThinLTO mode, when flattened profile is used, all the available
    // profile information will be annotated in PreLink phase so there is
    // no need to load the profile again in PostLink.
    if (!(FlattenedProfileUsed && PerformThinLTO))
      MPM.add(createSampleProfileLoaderPass(PGOSampleUse));
  }

  // Allow forcing function attributes as a debugging and tuning aid.
  MPM.add(createForceFunctionAttrsLegacyPass());

  // If all optimizations are disabled, just run the always-inline pass and,
  // if enabled, the function merging pass.
  if (OptLevel == 0) {
    addPGOInstrPasses(MPM);
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

    if (PerformThinLTO) {
      // Drop available_externally and unreferenced globals. This is necessary
      // with ThinLTO in order to avoid leaving undefined references to dead
      // globals in the object file.
      MPM.add(createEliminateAvailableExternallyPass());
      MPM.add(createGlobalDCEPass());
    }

    addExtensionsToPM(EP_EnabledOnOptLevel0, MPM);

    if (PrepareForLTO || PrepareForThinLTO) {
      MPM.add(createCanonicalizeAliasesPass());
      // Rename anon globals to be able to export them in the summary.
      // This has to be done after we add the extensions to the pass manager
      // as there could be passes (e.g. Adddress sanitizer) which introduce
      // new unnamed globals.
      MPM.add(createNameAnonGlobalPass());
    }
#if INTEL_COLLAB
    if (RunVPOOpt) {
      #if INTEL_CUSTOMIZATION
      if (RunVecClone) {
        MPM.add(createVecClonePass());
      }
      #endif // INTEL_CUSTOMIZATION
      // Process OpenMP directives at -O0
      addVPOPasses(MPM, true);
    }
#endif // INTEL_COLLAB
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

  // For ThinLTO there are two passes of indirect call promotion. The
  // first is during the compile phase when PerformThinLTO=false and
  // intra-module indirect call targets are promoted. The second is during
  // the ThinLTO backend when PerformThinLTO=true, when we promote imported
  // inter-module indirect calls. For that we perform indirect call promotion
  // earlier in the pass pipeline, here before globalopt. Otherwise imported
  // available_externally functions look unreferenced and are removed.
  if (PerformThinLTO)
    MPM.add(createPGOIndirectCallPromotionLegacyPass(/*InLTO = */ true,
                                                     !PGOSampleUse.empty()));

  // For SamplePGO in ThinLTO compile phase, we do not want to unroll loops
  // as it will change the CFG too much to make the 2nd profile annotation
  // in backend more difficult.
  bool PrepareForThinLTOUsingPGOSampleProfile =
      PrepareForThinLTO && !PGOSampleUse.empty();
  if (PrepareForThinLTOUsingPGOSampleProfile)
    DisableUnrollLoops = true;

  // Infer attributes about declarations if possible.
  MPM.add(createInferFunctionAttrsLegacyPass());

  addExtensionsToPM(EP_ModuleOptimizerEarly, MPM);

  if (OptLevel > 2)
    MPM.add(createCallSiteSplittingPass());

  MPM.add(createIPSCCPPass());          // IP SCCP
  MPM.add(createCalledValuePropagationPass());

  // Infer attributes on declarations, call sites, arguments, etc.
  MPM.add(createAttributorLegacyPass());

  MPM.add(createGlobalOptimizerPass()); // Optimize out global vars
  // Promote any localized global vars.
  MPM.add(createPromoteMemoryToRegisterPass());

  MPM.add(createDeadArgEliminationPass()); // Dead argument elimination

  addInstructionCombiningPass(MPM); // Clean up after IPCP & DAE
  addExtensionsToPM(EP_Peephole, MPM);
  if (EarlyJumpThreading)                         // INTEL
    MPM.add(createJumpThreadingPass(-1, false));  // INTEL
  MPM.add(createCFGSimplificationPass()); // Clean up after IPCP & DAE

  // For SamplePGO in ThinLTO compile phase, we do not want to do indirect
  // call promotion as it will change the CFG too much to make the 2nd
  // profile annotation in backend more difficult.
  // PGO instrumentation is added during the compile phase for ThinLTO, do
  // not run it a second time
  if (DefaultOrPreLinkPipeline && !PrepareForThinLTOUsingPGOSampleProfile)
    addPGOInstrPasses(MPM);

  // Create profile COMDAT variables. Lld linker wants to see all variables
  // before the LTO/ThinLTO link since it needs to resolve symbols/comdats.
  if (!PerformThinLTO && EnablePGOCSInstrGen)
    MPM.add(createPGOInstrumentationGenCreateVarLegacyPass(PGOInstrGen));

  // We add a module alias analysis pass here. In part due to bugs in the
  // analysis infrastructure this "works" in that the analysis stays alive
  // for the entire SCC pass run below.
  MPM.add(createGlobalsAAWrapperPass());

#if INTEL_CUSTOMIZATION
  if (Inliner) {
    MPM.add(createInlineReportSetupPass(getMDInlineReport()));
    MPM.add(createInlineListsPass()); // -[no]inline-list parsing
  }
#endif  // INTEL_CUSTOMIZATION

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
    addVPOPasses(MPM, false, /* Simplify= */ true);
  }
#endif // INTEL_COLLAB
  MPM.add(createPostOrderFunctionAttrsLegacyPass());
  if (OptLevel > 2)
    MPM.add(createArgumentPromotionPass()); // Scalarize uninlined fn args

  addExtensionsToPM(EP_CGSCCOptimizerLate, MPM);
  addFunctionSimplificationPasses(MPM);

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

  if (OptLevel > 1 && !PrepareForLTO && !PrepareForThinLTO)
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

  // CSFDO instrumentation and use pass. Don't invoke this for Prepare pass
  // for LTO and ThinLTO -- The actual pass will be called after all inlines
  // are performed.
  // Need to do this after COMDAT variables have been eliminated,
  // (i.e. after EliminateAvailableExternallyPass).
  if (!(PrepareForLTO || PrepareForThinLTO))
    addPGOInstrPasses(MPM, /* IsCS */ true);

  if (EnableOrderFileInstrumentation)
    MPM.add(createInstrOrderFilePass());

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

  // If we are planning to perform ThinLTO later, let's not bloat the code with
  // unrolling/vectorization/... now. We'll first run the inliner + CGSCC passes
  // during ThinLTO and perform the rest of the optimizations afterward.
  if (PrepareForThinLTO) {
    // Ensure we perform any last passes, but do so before renaming anonymous
    // globals in case the passes add any.
    addExtensionsToPM(EP_OptimizerLast, MPM);
    MPM.add(createCanonicalizeAliasesPass());
    // Rename anon globals to be able to export them in the summary.
    MPM.add(createNameAnonGlobalPass());
    return;
  }

  if (PerformThinLTO)
    // Optimize globals now when performing ThinLTO, this enables more
    // optimizations later.
    MPM.add(createGlobalOptimizerPass());

  // Scheduling LoopVersioningLICM when inlining is over, because after that
  // we may see more accurate aliasing. Reason to run this late is that too
  // early versioning may prevent further inlining due to increase of code
  // size. By placing it just after inlining other optimizations which runs
  // later might get benefit of no-alias assumption in clone loop.
  if (UseLoopVersioningLICM) {
    MPM.add(createLoopVersioningLICMPass());    // Do LoopVersioningLICM
    MPM.add(createLICMPass(LicmMssaOptCap, LicmMssaNoAccForPromotionCap));
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

  addExtensionsToPM(EP_VectorizerStart, MPM);

  // Re-rotate loops in all our loop nests. These may have fallout out of
  // rotated form due to GVN or other transformations, and the vectorizer relies
  // on the rotated form. Disable header duplication at -Oz.
  MPM.add(createLoopRotatePass(SizeLevel == 2 ? 0 : -1));

#if INTEL_CUSTOMIZATION
  // In LTO mode, loopopt needs to run in link phase along with community
  // vectorizer and unroll after it until they are phased out.
  if (!PrepareForLTO || !isLoopOptEnabled()) {
    addLoopOptAndAssociatedVPOPasses(MPM, false);
#endif // INTEL_CUSTOMIZATION

  // Distribute loops to allow partial vectorization.  I.e. isolate dependences
  // into separate loop that would otherwise inhibit vectorization.  This is
  // currently only performed for loops marked with the metadata
  // llvm.loop.distribute=true or when -enable-loop-distribute is specified.
  MPM.add(createLoopDistributePass());

#if INTEL_CUSTOMIZATION
  if (EnableLV)
    MPM.add(createLoopVectorizePass(!LoopsInterleaved, !LoopVectorize));
  }
#endif  // INTEL_CUSTOMIZATION
  // Eliminate loads by forwarding stores from the previous iteration to loads
  // of the current iteration.
  MPM.add(createLoopLoadEliminationPass());

#if INTEL_CUSTOMIZATION
  // No need to run cleanup passes in LTO mode when loopopt is enabled as
  // vectorization is moved to link phase.
  if (!PrepareForLTO || !isLoopOptEnabled()) {
#endif // INTEL_CUSTOMIZATION
  // FIXME: Because of #pragma vectorize enable, the passes below are always
  // inserted in the pipeline, even when the vectorizer doesn't run (ex. when
  // on -O1 and no #pragma is found). Would be good to have these two passes
  // as function calls, so that we can only pass them when the vectorizer
  // changed the code.
  addInstructionCombiningPass(MPM);
  if (OptLevel > 1 && ExtraVectorizerPasses) {
    // At higher optimization levels, try to clean up any runtime overlap and
    // alignment checks inserted by the vectorizer. We want to track correllated
    // runtime checks for two inner loops in the same outer loop, fold any
    // common computations, hoist loop-invariant aspects out of any outer loop,
    // and unswitch the runtime checks if possible. Once hoisted, we may have
    // dead (or speculatable) control flows or more combining opportunities.
    MPM.add(createEarlyCSEPass());
    MPM.add(createCorrelatedValuePropagationPass());
    addInstructionCombiningPass(MPM);
    MPM.add(createLICMPass(LicmMssaOptCap, LicmMssaNoAccForPromotionCap));
    MPM.add(createLoopUnswitchPass(SizeLevel || OptLevel < 3, DivergentTarget));
    MPM.add(createCFGSimplificationPass());
    addInstructionCombiningPass(MPM);
  }

  // Cleanup after loop vectorization, etc. Simplification passes like CVP and
  // GVN, loop transforms, and others have already run, so it's now better to
  // convert to more optimized IR using more aggressive simplify CFG options.
  // The extra sinking transform can create larger basic blocks, so do this
  // before SLP vectorization.
  MPM.add(createCFGSimplificationPass(1, true, true, false, true));

  if (SLPVectorize) {
    MPM.add(createSLPVectorizerPass()); // Vectorize parallel scalar chains.
#if INTEL_CUSTOMIZATION
    if (EnableLoadCoalescing)
      MPM.add(createLoadCoalescingPass());
    if (EnableSROAAfterSLP)
      // SLP creates opportunities for SROA.
      MPM.add(createSROAPass());
#endif // INTEL_CUSTOMIZATION
    if (OptLevel > 1 && ExtraVectorizerPasses) {
      MPM.add(createEarlyCSEPass());
    }
  }
  } // INTEL

  addExtensionsToPM(EP_Peephole, MPM);
  addInstructionCombiningPass(MPM);

#if INTEL_CUSTOMIZATION
  // Disable unroll in LTO mode if loopopt is enabled so it only gets triggered
  // in link phase after loopopt.
  if (EnableUnrollAndJam && !DisableUnrollLoops &&
      (!PrepareForLTO || !isLoopOptEnabled())) {
#endif // INTEL_CUSTOMIZATION
    // Unroll and Jam. We do this before unroll but need to be in a separate
    // loop pass manager in order for the outer loop to be processed by
    // unroll and jam before the inner loop is unrolled.
    MPM.add(createLoopUnrollAndJamPass(OptLevel));
  }

  if (!PrepareForLTO || !isLoopOptEnabled()) // INTEL
  MPM.add(createLoopUnrollPass(OptLevel,
                               DisableUnrollLoops, // Unroll small loops
                               ForgetAllSCEVInLoopUnroll));

#if INTEL_CUSTOMIZATION
#if INTEL_FEATURE_CSA
  if (!DisableUnrollLoops && (!PrepareForLTO || !isLoopOptEnabled()))
    MPM.add(createCSALowerParallelIntrinsicsWrapperPass());
#endif  // INTEL_FEATURE_CSA
#endif  // INTEL_CUSTOMIZATION

  if (!DisableUnrollLoops && (!PrepareForLTO || !isLoopOptEnabled())) { // INTEL
    // LoopUnroll may generate some redundency to cleanup.
    addInstructionCombiningPass(MPM);

    // Runtime unrolling will introduce runtime check in loop prologue. If the
    // unrolled loop is a inner loop, then the prologue will be inside the
    // outer loop. LICM pass can help to promote the runtime check out if the
    // checked value is loop invariant.
    MPM.add(createLICMPass(LicmMssaOptCap, LicmMssaNoAccForPromotionCap));
  }

  MPM.add(createWarnMissedTransformationsPass());

  // After vectorization and unrolling, assume intrinsics may tell us more
  // about pointer alignments.
  MPM.add(createAlignmentFromAssumptionsPass());

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
  if (EnableHotColdSplit && !(PrepareForLTO || PrepareForThinLTO))
    MPM.add(createHotColdSplittingPass());

  if (MergeFunctions)
    MPM.add(createMergeFunctionsPass());

  // LoopSink pass sinks instructions hoisted by LICM, which serves as a
  // canonicalization pass that enables other optimizations. As a result,
  // LoopSink pass needs to be a very late IR pass to avoid undoing LICM
  // result too early.
  MPM.add(createLoopSinkPass());
  // Get rid of LCSSA nodes.
  MPM.add(createInstSimplifyLegacyPass());

  // This hoists/decomposes div/rem ops. It should run after other sink/hoist
  // passes to avoid re-sinking, but before SimplifyCFG because it can allow
  // flattening of blocks.
  MPM.add(createDivRemPairsPass());

  // LoopSink (and other loop passes since the last simplifyCFG) might have
  // resulted in single-entry-single-exit or empty blocks. Clean up the CFG.
  MPM.add(createCFGSimplificationPass());

  addExtensionsToPM(EP_OptimizerLast, MPM);

  if (PrepareForLTO) {
    MPM.add(createCanonicalizeAliasesPass());
    // Rename anon globals to be able to handle them in the summary
    MPM.add(createNameAnonGlobalPass());
  }

#if INTEL_CUSTOMIZATION
#if INTEL_FEATURE_CSA
  if (EnableCSAPasses) {
    MPM.add(createPromoteMemoryToRegisterPass(true, true));
    MPM.add(createSROAPass());
  }
#endif // INTEL_FEATURE_CSA
  MPM.add(createInlineReportEmitterPass(OptLevel, SizeLevel,
                                        PrepareForLTO || PrepareForThinLTO));
#endif // INTEL_CUSTOMIZATION
}

void PassManagerBuilder::addLTOOptimizationPasses(legacy::PassManagerBase &PM) {
#if INTEL_CUSTOMIZATION
  if (Inliner &&
      (IntelInlineReportLevel & InlineReportOptions::CompositeReport)) {
    PM.add(createInlineReportSetupPass(getMDInlineReport()));
  }
#endif // INTEL_CUSTOMIZATION
  // Load sample profile before running the LTO optimization pipeline.
  if (!PGOSampleUse.empty()) {
    PM.add(createPruneEHPass());
    PM.add(createSampleProfileLoaderPass(PGOSampleUse));
  }

  PM.add(createXmainOptLevelWrapperPass(OptLevel)); // INTEL

  // Remove unused virtual tables to improve the quality of code generated by
  // whole-program devirtualization and bitset lowering.
  PM.add(createGlobalDCEPass());

#if INTEL_CUSTOMIZATION
  // Whole Program Analysis
  if (EnableWPA) {
    PM.add(createWholeProgramWrapperPassPass());
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
  }

  // IP Cloning
  if (EnableIPCloning) {
#if INTEL_INCLUDE_DTRANS
    // This pass is being added under DTRANS only at this point, because a
    // particular benchmark needs it to prove that the period of a recursive
    // progression is constant. We can remove the test for EnableDTrans if
    // we find IPSCCP to be generally useful here and we are willing to
    // tolerate the additional compile time.
    if (EnableDTrans)
      PM.add(createIPSCCPPass());
#endif // INTEL_INCLUDE_DTRANS
    PM.add(createIPCloningLegacyPass(false, true));
  }

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

    // Indirect call promotion. This should promote all the targets that are
    // left by the earlier promotion pass that promotes intra-module targets.
    // This two-step promotion is to save the compile time. For LTO, it should
    // produce the same result as if we only do promotion here.
    PM.add(
        createPGOIndirectCallPromotionLegacyPass(true, !PGOSampleUse.empty()));

    // Propagate constants at call sites into the functions they call.  This
    // opens opportunities for globalopt (and inlining) by substituting function
    // pointers passed as arguments to direct uses of functions.
    PM.add(createIPSCCPPass());

    // Attach metadata to indirect call sites indicating the set of functions
    // they may target at run-time. This should follow IPSCCP.
    PM.add(createCalledValuePropagationPass());

    // Infer attributes on declarations, call sites, arguments, etc.
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

  // Apply whole-program devirtualization and virtual constant propagation.
  PM.add(createWholeProgramDevirtPass(ExportSummary, nullptr));

  // That's all we need at opt level 1.
  if (OptLevel == 1)
    return;

#if INTEL_CUSTOMIZATION
#if INTEL_INCLUDE_DTRANS
  if (EnableDTrans) {
    // This call adds the DTrans passes.
    addDTransLegacyPasses(PM);
  }
#endif // INTEL_INCLUDE_DTRANS
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
#if INTEL_INCLUDE_DTRANS
  if (EnableDTrans) {
    addLateDTransLegacyPasses(PM);
    if (EnableIndirectCallConv)
      PM.add(createIndirectCallConvLegacyPass(false /* EnableAndersen */,
                                              true /* EnableDTrans */));
      // Indirect Call Conv
  }
#endif // INTEL_INCLUDE_DTRANS
#endif // INTEL_CUSTOMIZATION

  // Reduce the code after globalopt and ipsccp.  Both can open up significant
  // simplification opportunities, and both can propagate functions through
  // function pointers.  When this happens, we often have to resolve varargs
  // calls, etc, so let instcombine do this.
  if (OptLevel > 2)
    PM.add(createAggressiveInstCombinerPass());
  addInstructionCombiningPass(PM);
  addExtensionsToPM(EP_Peephole, PM);

#if INTEL_CUSTOMIZATION

#if INTEL_INCLUDE_DTRANS
  bool EnableIntelPartialInlining = EnableIntelPI && EnableDTrans;
#else
  bool EnableIntelPartialInlining = false;
#endif // INTEL_INCLUDE_DTRANS

  // Partial inlining for simple functions
  if (EnableIntelPartialInlining)
    PM.add(createIntelPartialInlineLegacyPass());

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
    PM.add(createAndersensAAWrapperPass()); // Andersen's IP alias analysis
  }
  if (EnableIndirectCallConv && EnableAndersen) {
    PM.add(createIndirectCallConvLegacyPass(true /* EnableAndersen */,
                                            false /* EnableDTrans */));
    // Indirect Call Conv
  }
  if (EnableInlineAggAnalysis) {
    PM.add(createInlineAggressiveWrapperPassPass()); // Aggressive Inline
  }
#endif // INTEL_CUSTOMIZATION

  // Inline small functions
  if (RunInliner) {
    PM.add(Inliner);
    Inliner = nullptr;
  }

  PM.add(createPruneEHPass());   // Remove dead EH info.

  // CSFDO instrumentation and use pass.
  addPGOInstrPasses(PM, /* IsCS */ true);

  // Optimize globals again if we ran the inliner.
  if (RunInliner) { // INTEL
#if INTEL_CUSTOMIZATION
#if INTEL_INCLUDE_DTRANS
    // The global optimizer pass can convert function calls to use
    // the 'fastcc' calling convention. The following pass enables more
    // functions to be converted to this calling convention. This can improve
    // performance by having arguments passed in registers, and enable more
    // cases where pointer parameters are changed to pass-by-value parameters.
    // We can remove the test for EnableDTrans if it is found to be useful
    // on other cases.
    if (EnableDTrans)
      PM.add(createIntelAdvancedFastCallWrapperPass());
#endif // INTEL_INCLUDE_DTRANS
#endif // INTEL_CUSTOMIZATION
    PM.add(createGlobalOptimizerPass());
  } // INTEL

#if INTEL_CUSTOMIZATION
  if (RunLTOPartialInlining)
    PM.add(createPartialInliningPass(true /*RunLTOPartialInlining*/,
#if INTEL_INCLUDE_DTRANS
                                     EnableDTrans /*EnableSpecialCases*/));
#else
                                     false /*EnableSpecialCases*/));
#endif // INTEL_INCLUDE_DTRANS

  if (EnableIPCloning || EnableCallTreeCloning) {
    if (EnableIPCloning)
      // Enable generic IPCloning after Inlining.
#if INTEL_INCLUDE_DTRANS
      PM.add(createIPCloningLegacyPass(true, EnableDTrans));
#else
      PM.add(createIPCloningLegacyPass(true, false));
#endif // INTEL_INCLUDE_DTRANS
    if (EnableCallTreeCloning)
      // Do function cloning along call trees
      PM.add(createCallTreeCloningPass());
    // Call IPCP to propagate constants
    PM.add(createIPSCCPPass());
  }
#endif // INTEL_CUSTOMIZATION
  PM.add(createGlobalDCEPass()); // Remove dead functions.

  // If we didn't decide to inline a function, check to see if we can
  // transform it to pass arguments by value instead of by reference.
  PM.add(createArgumentPromotionPass());

  // The IPO passes may leave cruft around.  Clean up after them.
  addInstructionCombiningPass(PM);
  addExtensionsToPM(EP_Peephole, PM);
  PM.add(createJumpThreadingPass());

  // Break up allocas
  PM.add(createSROAPass());

#if INTEL_CUSTOMIZATION
  PM.add(createCorrelatedValuePropagationPass());

  if (EnableMultiVersioning)
    PM.add(createMultiVersioningWrapperPass());
#endif // INTEL_CUSTOMIZATION
  // LTO provides additional opportunities for tailcall elimination due to
  // link-time inlining, and visibility of nocapture attribute.
  PM.add(createTailCallEliminationPass());

  // Infer attributes on declarations, call sites, arguments, etc.
  PM.add(createPostOrderFunctionAttrsLegacyPass()); // Add nocapture.
  // Run a few AA driven optimizations here and now, to cleanup the code.
  PM.add(createGlobalsAAWrapperPass()); // IP alias analysis.

#if INTEL_CUSTOMIZATION
  if (EnableAndersen)
    PM.add(createAndersensAAWrapperPass());
#endif // INTEL_CUSTOMIZATION

  PM.add(createLICMPass(LicmMssaOptCap, LicmMssaNoAccForPromotionCap));
  PM.add(createMergedLoadStoreMotionPass()); // Merge ld/st in diamonds.
  PM.add(NewGVN ? createNewGVNPass()
                : createGVNPass(DisableGVNLoadPRE)); // Remove redundancies.
  PM.add(createMemCpyOptPass());            // Remove dead memcpys.

  // Nuke dead stores.
  PM.add(createDeadStoreEliminationPass());

  // More loops are countable; try to optimize them.
  PM.add(createIndVarSimplifyPass());
  PM.add(createLoopDeletionPass());
  if (EnableLoopInterchange)
    PM.add(createLoopInterchangePass());

#if INTEL_CUSTOMIZATION
  // HIR complete unroll pass replaces LLVM's simple loop unroll pass.
  if (!isLoopOptEnabled())
    PM.add(createSimpleLoopUnrollPass(OptLevel,
                                    DisableUnrollLoops, // Unroll small loops
                                    ForgetAllSCEVInLoopUnroll));
  addLoopOptAndAssociatedVPOPasses(PM, true);
  if (EnableLV)
    PM.add(createLoopVectorizePass(true, !LoopVectorize));
#endif  // INTEL_CUSTOMIZATION

  // The vectorizer may have significantly shortened a loop body; unroll again.
  PM.add(createLoopUnrollPass(OptLevel, DisableUnrollLoops,
                              ForgetAllSCEVInLoopUnroll));

#if INTEL_CUSTOMIZATION
#if INTEL_FEATURE_CSA
  PM.add(createCSALowerParallelIntrinsicsWrapperPass());
#endif  // INTEL_FEATURE_CSA
#endif  // INTEL_CUSTOMIZATION

  PM.add(createWarnMissedTransformationsPass());

  // Now that we've optimized loops (in particular loop induction variables),
  // we may have exposed more scalar opportunities. Run parts of the scalar
  // optimizer again at this point.
  addInstructionCombiningPass(PM); // Initial cleanup
  PM.add(createCFGSimplificationPass()); // if-convert
  PM.add(createSCCPPass()); // Propagate exposed constants
  addInstructionCombiningPass(PM); // Clean up again
  PM.add(createBitTrackingDCEPass());

  // More scalar chains could be vectorized due to more alias information
#if INTEL_CUSTOMIZATION
  if (SLPVectorize) {
    PM.add(createSLPVectorizerPass()); // Vectorize parallel scalar chains.
    if (EnableLoadCoalescing)
      PM.add(createLoadCoalescingPass());
    if (EnableSROAAfterSLP)
      // SLP creates opportunities for SROA.
      PM.add(createSROAPass());
  }
#endif // INTEL_CUSTOMIZATION

  // After vectorization, assume intrinsics may tell us more about pointer
  // alignments.
  PM.add(createAlignmentFromAssumptionsPass());

  // Cleanup and simplify the code after the scalar optimizations.
  addInstructionCombiningPass(PM);
  addExtensionsToPM(EP_Peephole, PM);

  PM.add(createJumpThreadingPass());

#if INTEL_CUSTOMIZATION
  if (RunInliner)
    PM.add(createInlineReportEmitterPass(OptLevel, SizeLevel, false));
#endif // INTEL_CUSTOMIZATION
}

void PassManagerBuilder::addLateLTOOptimizationPasses(
    legacy::PassManagerBase &PM) {
  // See comment in the new PM for justification of scheduling splitting at
  // this stage (\ref buildLTODefaultPipeline).
  if (EnableHotColdSplit)
    PM.add(createHotColdSplittingPass());

  // Delete basic blocks, which optimization passes may have killed.
  PM.add(createCFGSimplificationPass());

#if INTEL_CUSTOMIZATION
  // HIR complete unroll can expose opportunities for optimizing globals and
  // allocas.
  if (isLoopOptEnabled()) {
    PM.add(createGlobalOptimizerPass());
  }
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
                                      bool Simplify) const {
  if (RunVPOParopt) {
    if (Simplify) {
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
    PM.add(createVPORestoreOperandsPass());
    PM.add(createVPOCFGRestructuringPass());
    PM.add(createVPOParoptPass(RunVPOParopt, OptLevel));
#if INTEL_CUSTOMIZATION
#if INTEL_FEATURE_CSA
    if (RunCSAGraphSplitter)
      PM.add(createCSAGraphSplitterPass());
#endif // INTEL_FEATURE_CSA
#endif // INTEL_CUSTOMIZATION
  }
  #if INTEL_CUSTOMIZATION
  // If vectorizer was required to run then cleanup any remaining directives
  // that were not removed by vectorizer. This applies to all optimization
  // levels since this function is called with RunVec=true in both pass
  // pipelines i.e. -O0 and optlevel >= 1
  //
  // TODO: Issue a warning for any unprocessed directives. Change to
  // assetion failure as the feature matures.
  if (RunVPOParopt && RunVec)
    PM.add(createVPODirectiveCleanupPass());
  #endif // INTEL_CUSTOMIZATION
}
#endif // INTEL_COLLAB

#if INTEL_CUSTOMIZATION // HIR passes

void PassManagerBuilder::addVPOPassesPreLoopOpt(
    legacy::PassManagerBase &PM) const {
  PM.add(createLowerSwitchPass(true /*Only for SIMD loops*/));
  // Add LCSSA pass before VPlan driver
  PM.add(createLCSSAPass());
  PM.add(createVPOCFGRestructuringPass());
  PM.add(createVPlanDriverPass());
  // Clean up any SIMD directives left behind by VPlan vectorizer
  PM.add(createVPODirectiveCleanupPass());
}

bool PassManagerBuilder::isLoopOptEnabled() const {
  if (!DisableIntelProprietaryOpts &&
      ((RunLoopOpts != LoopOptMode::None) || RunLoopOptFrameworkOnly) &&
      (OptLevel >= 2) && !PerformThinLTO)
    return true;

  return false;
}

llvm::InlineReportBuilder *PassManagerBuilder::getMDInlineReport() const {
  if (!Inliner)
    return nullptr;
  return &(static_cast<LegacyInlinerBase *>(Inliner)->getMDReport());
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
  addInstructionCombiningPass(PM);
  PM.add(createLoopCarriedCSEPass());
  PM.add(createDeadStoreEliminationPass());

  if (OptLevel > 2) {
    // Cleanup code with AddSub reassociation.
    PM.add(createAddSubReassociatePass());
  }
}

void PassManagerBuilder::addLoopOptPasses(legacy::PassManagerBase &PM,
                                          bool IsLTO) const {

  if (!isLoopOptEnabled())
    return;

  // Run additional cleanup passes that help to cleanup the code.
  if (IsLTO && RunLoopOpts == LoopOptMode::Full) {
    PM.add(createCFGSimplificationPass());
    PM.add(createAggressiveDCEPass());
  }

  // This pass "canonicalizes" loops and makes analysis easier.
  PM.add(createLoopSimplifyPass());

  // This lets us generate code for HIR regions independently without concern
  // for livouts from one reigon being livein to another region. It also
  // considerably simplifies handling of liveout values for multi-exit regions.
  PM.add(createLCSSAPass());

  if (PrintModuleBeforeLoopopt)
    PM.add(createPrintModulePass(dbgs(), ";Module Before HIR" ));

  // Verify input LLVM IR before doing any HIR transformation.
  if (VerifyInput)
    PM.add(createVerifierPass());

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
      if (RunLoopOpts == LoopOptMode::Full) {
        PM.add(createHIRLoopConcatenationPass());
        PM.add(createHIRSymbolicTripCountCompleteUnrollLegacyPass());
      }
      PM.add(createHIRArrayTransposePass());
    }

    if (RunLoopOpts == LoopOptMode::Full) {

      // TODO: refine cost model for individual transformations for code size.
      if (SizeLevel == 0) {
        // If VPO is disabled, we don't have to insert ParVec directives.
        if (RunVPOOpt)
          PM.add(createHIRParDirInsertPass());

        PM.add(createHIRConditionalTempSinkingPass());
        PM.add(createHIROptPredicatePass(OptLevel == 3, true));
        PM.add(createHIRRuntimeDDPass());
        PM.add(createHIRMVForConstUBPass());
      }

      PM.add(createHIRSinkingForPerfectLoopnestPass());
      PM.add(createHIRLoopDistributionForLoopNestPass());
      PM.add(createHIRLoopInterchangePass());
      PM.add(createHIRDeadStoreEliminationPass());
      PM.add(createHIRGenerateMKLCallPass());
      PM.add(createHIRLoopBlockingPass());
      PM.add(createHIRLoopReversalPass());
      PM.add(createHIRIdentityMatrixIdiomRecognitionPass());
    }

    if (SizeLevel == 0) {
      PM.add(createHIRPreVecCompleteUnrollPass(OptLevel, DisableUnrollLoops));
    }

    if (RunLoopOpts == LoopOptMode::Full) {
      PM.add(createHIRLMMPass());
      if (SizeLevel == 0)
        PM.add(createHIRMemoryReductionSinkingPass());
    }

    PM.add(createHIRLastValueComputationPass());

    if (RunLoopOpts == LoopOptMode::Full) {
      PM.add(createHIRLoopRerollPass());

      if (SizeLevel == 0) {
        PM.add(createHIRLoopDistributionForMemRecPass());
      }

      PM.add(createHIRLoopRematerializePass());
      PM.add(createHIRMultiExitLoopRerollPass());
      PM.add(createHIRLoopCollapsePass());
      PM.add(createHIRIdiomRecognitionPass());
      PM.add(createHIRLoopFusionPass());
    }

    if (SizeLevel == 0) {
      if (RunLoopOpts == LoopOptMode::Full) {
        PM.add(createHIRUnrollAndJamPass(DisableUnrollLoops));
        PM.add(createHIROptVarPredicatePass());
        PM.add(createHIROptPredicatePass(OptLevel == 3, false));
      }
      if (RunVPOOpt) {
        PM.add(createHIRVecDirInsertPass(OptLevel == 3));
        if (EnableVPlanDriverHIR) {
          // Enable VPlan HIR Vectorizer
          PM.add(createVPlanDriverHIRPass());
        }
      }
      PM.add(createHIRPostVecCompleteUnrollPass(OptLevel, DisableUnrollLoops));
      PM.add(createHIRGeneralUnrollPass(DisableUnrollLoops));
    }

    if (RunLoopOpts == LoopOptMode::Full) {
      PM.add(createHIRScalarReplArrayPass());
      if (OptLevel > 2)
        PM.add(createHIRPrefetchingPass());
    }
  }

  if (IntelOptReportEmitter == OptReportOptions::HIR)
    PM.add(createHIROptReportEmitterWrapperPass());

  PM.add(createHIRCodeGenWrapperPass());

  addLoopOptCleanupPasses(PM);
}

void PassManagerBuilder::addLoopOptAndAssociatedVPOPasses(
    legacy::PassManagerBase &PM, bool IsLTO) const {
  // We should never get here if proprietary options are disabled,
  // but it's a release-mode feature so we can't just assert.
  if (DisableIntelProprietaryOpts)
    return;

  if (RunVPOOpt && RunVecClone) {
    PM.add(createVecClonePass());
    // VecClonePass can generate redundant geps/loads for vector parameters when
    // accessing elem[i] within the inserted simd loop. This makes DD testing
    // harder, so run CSE here to do some clean-up before HIR construction.
    PM.add(createEarlyCSEPass());
  }

  // Run LLVM-IR VPlan vectorizer before loopopt to vectorize all explicit SIMD
  // loops
  if (RunVPOOpt && RunPreLoopOptVPOPasses && EnableVPlanDriver)
    addVPOPassesPreLoopOpt(PM);

  addLoopOptPasses(PM, IsLTO);

  // Process directives inserted by LoopOpt Autopar.
  // Call with RunVec==true (2nd argument) to cleanup any vec directives
  // that loopopt and vectorizers might have missed.
  if (RunVPOOpt)
    addVPOPasses(PM, true);

  if (IntelOptReportEmitter == OptReportOptions::IR)
    PM.add(createLoopOptReportEmitterLegacyPass());
}

#endif // INTEL_CUSTOMIZATION

void PassManagerBuilder::populateThinLTOPassManager(
    legacy::PassManagerBase &PM) {
  PerformThinLTO = true;
  if (LibraryInfo)
    PM.add(new TargetLibraryInfoWrapperPass(*LibraryInfo));

  if (VerifyInput)
    PM.add(createVerifierPass());

  if (ImportSummary) {
    // These passes import type identifier resolutions for whole-program
    // devirtualization and CFI. They must run early because other passes may
    // disturb the specific instruction patterns that these passes look for,
    // creating dependencies on resolutions that may not appear in the summary.
    //
    // For example, GVN may transform the pattern assume(type.test) appearing in
    // two basic blocks into assume(phi(type.test, type.test)), which would
    // transform a dependency on a WPD resolution into a dependency on a type
    // identifier resolution for CFI.
    //
    // Also, WPD has access to more precise information than ICP and can
    // devirtualize more effectively, so it should operate on the IR first.
    PM.add(createWholeProgramDevirtPass(nullptr, ImportSummary));
    PM.add(createLowerTypeTestsPass(nullptr, ImportSummary));
  }

  populateModulePassManager(PM);

  if (VerifyOutput)
    PM.add(createVerifierPass());
  PerformThinLTO = false;
}

void PassManagerBuilder::populateLTOPassManager(legacy::PassManagerBase &PM) {
  if (LibraryInfo)
    PM.add(new TargetLibraryInfoWrapperPass(*LibraryInfo));

  if (VerifyInput)
    PM.add(createVerifierPass());

  addExtensionsToPM(EP_FullLinkTimeOptimizationEarly, PM);

  if (OptLevel != 0)
    addLTOOptimizationPasses(PM);
  else {
    // The whole-program-devirt pass needs to run at -O0 because only it knows
    // about the llvm.type.checked.load intrinsic: it needs to both lower the
    // intrinsic itself and handle it in the summary.
    PM.add(createWholeProgramDevirtPass(ExportSummary, nullptr));
  }

  // Create a function that performs CFI checks for cross-DSO calls with targets
  // in the current module.
  PM.add(createCrossDSOCFIPass());

  // Lower type metadata and the type.test intrinsic. This pass supports Clang's
  // control flow integrity mechanisms (-fsanitize=cfi*) and needs to run at
  // link time if CFI is enabled. The pass does nothing if CFI is disabled.
  PM.add(createLowerTypeTestsPass(ExportSummary, nullptr));

  if (OptLevel != 0)
    addLateLTOOptimizationPasses(PM);

  addExtensionsToPM(EP_FullLinkTimeOptimizationLast, PM);

  if (VerifyOutput)
    PM.add(createVerifierPass());
}

inline PassManagerBuilder *unwrap(LLVMPassManagerBuilderRef P) {
    return reinterpret_cast<PassManagerBuilder*>(P);
}

inline LLVMPassManagerBuilderRef wrap(PassManagerBuilder *P) {
  return reinterpret_cast<LLVMPassManagerBuilderRef>(P);
}

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
