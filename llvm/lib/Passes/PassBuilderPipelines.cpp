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
//===- Construction of pass pipelines -------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
/// \file
///
/// This file provides the implementation of the PassBuilder based on our
/// static pass registry as well as related functionality. It also provides
/// helpers to aid in analyzing, debugging, and testing passes and pass
/// pipelines.
///
//===----------------------------------------------------------------------===//

#include "llvm/ADT/ScopeExit.h" // INTEL
#include "llvm/Analysis/AliasAnalysis.h"
#include "llvm/Analysis/BasicAliasAnalysis.h"
#include "llvm/Analysis/CGSCCPassManager.h"
#include "llvm/Analysis/GlobalsModRef.h"
#if INTEL_CUSTOMIZATION
#include "llvm/Analysis/Intel_Andersens.h"
#include "llvm/Analysis/Intel_ArrayUseAnalysis.h"
#include "llvm/Analysis/Intel_StdContainerAA.h"
#include "llvm/Analysis/Intel_WP.h"
#endif // INTEL_CUSTOMIZATION
#include "llvm/Analysis/InlineAdvisor.h"
#include "llvm/Analysis/OptimizationRemarkEmitter.h"
#include "llvm/Analysis/ProfileSummaryInfo.h"
#include "llvm/Analysis/ScopedNoAliasAA.h"
#include "llvm/Analysis/TypeBasedAliasAnalysis.h"
#include "llvm/IR/IRPrintingPasses.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Passes/OptimizationLevel.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/PGOOptions.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Transforms/AggressiveInstCombine/AggressiveInstCombine.h"
#include "llvm/Transforms/Coroutines/CoroCleanup.h"
#include "llvm/Transforms/Coroutines/CoroConditionalWrapper.h"
#include "llvm/Transforms/Coroutines/CoroEarly.h"
#include "llvm/Transforms/Coroutines/CoroElide.h"
#include "llvm/Transforms/Coroutines/CoroSplit.h"
#include "llvm/Transforms/Instrumentation/Intel_FunctionSplitting.h" // INTEL
#include "llvm/Transforms/IPO/AlwaysInliner.h"
#include "llvm/Transforms/IPO/Annotation2Metadata.h"
#include "llvm/Transforms/IPO/ArgumentPromotion.h"
#include "llvm/Transforms/IPO/Attributor.h"
#include "llvm/Transforms/IPO/CalledValuePropagation.h"
#include "llvm/Transforms/IPO/ConstantMerge.h"
#include "llvm/Transforms/IPO/CrossDSOCFI.h"
#include "llvm/Transforms/IPO/DeadArgumentElimination.h"
#include "llvm/Transforms/IPO/ElimAvailExtern.h"
#include "llvm/Transforms/IPO/ForceFunctionAttrs.h"
#include "llvm/Transforms/IPO/FunctionAttrs.h"
#include "llvm/Transforms/IPO/GlobalDCE.h"
#include "llvm/Transforms/IPO/GlobalOpt.h"
#include "llvm/Transforms/IPO/GlobalSplit.h"
#include "llvm/Transforms/IPO/HotColdSplitting.h"
#include "llvm/Transforms/IPO/IROutliner.h"
#include "llvm/Transforms/IPO/InferFunctionAttrs.h"
#include "llvm/Transforms/IPO/Inliner.h"
#if INTEL_CUSTOMIZATION
#include "llvm/Transforms/IPO/Intel_AdvancedFastCall.h"
#include "llvm/Transforms/IPO/Intel_AggInliner.h"
#include "llvm/Transforms/IPO/Intel_ArgNoAliasProp.h"
#include "llvm/Transforms/IPO/Intel_ArgumentAlignment.h"
#include "llvm/Transforms/IPO/Intel_AutoCPUClone.h"
#include "llvm/Transforms/IPO/Intel_CallTreeCloning.h"
#if INTEL_FEATURE_SW_ADVANCED
#include "llvm/Transforms/IPO/Intel_DeadArrayOpsElimination.h"
#endif // INTEL_FEATURE_SW_ADVANCED
#include "llvm/Transforms/IPO/Intel_DopeVectorConstProp.h"
#if INTEL_FEATURE_SW_DTRANS
#include "llvm/Transforms/IPO/Intel_FoldWPIntrinsic.h"
#endif // INTEL_FEATURE_SW_DTRANS
#include "llvm/Transforms/IPO/Intel_InlineLists.h"
#include "llvm/Transforms/IPO/Intel_InlineReportEmitter.h"
#include "llvm/Transforms/IPO/Intel_InlineReportSetup.h"
#include "llvm/Transforms/IPO/Intel_IPArrayTranspose.h"
#include "llvm/Transforms/IPO/Intel_IPODeadArgElimination.h"
#if INTEL_FEATURE_SW_ADVANCED
#include "llvm/Transforms/IPO/Intel_IPCloning.h"
#include "llvm/Transforms/IPO/Intel_IPOPrefetch.h"
#include "llvm/Transforms/IPO/Intel_IPPredOpt.h"
#endif // INTEL_FEATURE_SW_ADVANCED
#include "llvm/Transforms/IPO/Intel_MathLibrariesDeclaration.h"
#include "llvm/Transforms/IPO/Intel_OptimizeDynamicCasts.h"
#if INTEL_FEATURE_SW_ADVANCED
#include "llvm/Transforms/IPO/Intel_PartialInline.h"
#include "llvm/Transforms/IPO/Intel_QsortRecognizer.h"
#include "llvm/Transforms/IPO/Intel_TileMVInlMarker.h"
#endif // INTEL_FEATURE_SW_ADVANCED
#include "llvm/Transforms/IPO/Intel_VTableFixup.h"
#endif // INTEL_CUSTOMIZATION
#include "llvm/Transforms/IPO/Internalize.h"
#include "llvm/Transforms/IPO/LowerTypeTests.h"
#include "llvm/Transforms/IPO/MergeFunctions.h"
#include "llvm/Transforms/IPO/ModuleInliner.h"
#include "llvm/Transforms/IPO/OpenMPOpt.h"
#include "llvm/Transforms/IPO/PartialInlining.h"
#include "llvm/Transforms/IPO/SCCP.h"
#include "llvm/Transforms/IPO/SampleProfile.h"
#include "llvm/Transforms/IPO/SampleProfileProbe.h"
#include "llvm/Transforms/IPO/SyntheticCountsPropagation.h"
#include "llvm/Transforms/IPO/WholeProgramDevirt.h"
#include "llvm/Transforms/InstCombine/InstCombine.h"
#include "llvm/Transforms/Instrumentation/CGProfile.h"
#include "llvm/Transforms/Instrumentation/ControlHeightReduction.h"
#include "llvm/Transforms/Instrumentation/InstrOrderFile.h"
#include "llvm/Transforms/Instrumentation/InstrProfiling.h"
#include "llvm/Transforms/Instrumentation/MemProfiler.h"
#include "llvm/Transforms/Instrumentation/PGOInstrumentation.h"
#include "llvm/Transforms/Intel_OpenCLTransforms/FMASplitter.h" // INTEL
#include "llvm/Transforms/Scalar/ADCE.h"
#include "llvm/Transforms/Scalar/Intel_IVSplit.h" // INTEL
#include "llvm/Transforms/Scalar/AlignmentFromAssumptions.h"
#include "llvm/Transforms/Scalar/AnnotationRemarks.h"
#include "llvm/Transforms/Scalar/BDCE.h"
#include "llvm/Transforms/Scalar/CallSiteSplitting.h"
#include "llvm/Transforms/Scalar/ConstraintElimination.h"
#include "llvm/Transforms/Scalar/CorrelatedValuePropagation.h"
#include "llvm/Transforms/Scalar/DFAJumpThreading.h"
#include "llvm/Transforms/Scalar/DeadStoreElimination.h"
#include "llvm/Transforms/Scalar/DivRemPairs.h"
#include "llvm/Transforms/Scalar/EarlyCSE.h"
#include "llvm/Transforms/Scalar/Float2Int.h"
#include "llvm/Transforms/Scalar/GVN.h"
#include "llvm/Transforms/Scalar/IndVarSimplify.h"
#if INTEL_CUSTOMIZATION
#if INTEL_FEATURE_SW_ADVANCED
#include "llvm/Transforms/Scalar/Intel_FunctionRecognizer.h"
#endif // INTEL_FEATURE_SW_ADVANCED
#endif // INTEL_CUSTOMIZATION
#include "llvm/Transforms/Scalar/Intel_GlobalOpt.h"         // INTEL
#include "llvm/Transforms/Scalar/Intel_IndirectCallConv.h"  // INTEL
#include "llvm/Transforms/Scalar/Intel_LoopAttrs.h"         // INTEL
#include "llvm/Transforms/Scalar/Intel_LoopOptMarker.h" // INTEL
#include "llvm/Transforms/Scalar/Intel_LowerSubscriptIntrinsic.h" // INTEL
#include "llvm/Transforms/Scalar/Intel_StdContainerOpt.h" // INTEL
#include "llvm/Transforms/Scalar/Intel_TbaaMDPropagation.h" // INTEL
#include "llvm/Transforms/Scalar/InductiveRangeCheckElimination.h"
#if !INTEL_COLLAB
#include "llvm/Transforms/Scalar/InferAddressSpaces.h"
#endif // ! INTEL_COLLAB
#include "llvm/Transforms/Scalar/InstSimplifyPass.h"
#include "llvm/Transforms/Scalar/JumpThreading.h"
#include "llvm/Transforms/Scalar/LICM.h"
#include "llvm/Transforms/Scalar/LoopDeletion.h"
#include "llvm/Transforms/Scalar/LoopDistribute.h"
#include "llvm/Transforms/Scalar/LoopFlatten.h"
#include "llvm/Transforms/Scalar/LoopIdiomRecognize.h"
#include "llvm/Transforms/Scalar/LoopInstSimplify.h"
#include "llvm/Transforms/Scalar/LoopInterchange.h"
#include "llvm/Transforms/Scalar/LoopLoadElimination.h"
#include "llvm/Transforms/Scalar/LoopPassManager.h"
#include "llvm/Transforms/Scalar/LoopRotation.h"
#include "llvm/Transforms/Scalar/LoopSimplifyCFG.h"
#include "llvm/Transforms/Scalar/LoopSink.h"
#include "llvm/Transforms/Scalar/LoopUnrollAndJamPass.h"
#include "llvm/Transforms/Scalar/LoopUnrollPass.h"
#include "llvm/Transforms/Scalar/LowerConstantIntrinsics.h"
#include "llvm/Transforms/Scalar/LowerExpectIntrinsic.h"
#include "llvm/Transforms/Scalar/LowerMatrixIntrinsics.h"
#include "llvm/Transforms/Scalar/MemCpyOptimizer.h"
#include "llvm/Transforms/Scalar/MergedLoadStoreMotion.h"
#include "llvm/Transforms/Scalar/NaryReassociate.h"
#include "llvm/Transforms/Scalar/NewGVN.h"
#include "llvm/Transforms/Scalar/Reassociate.h"
#include "llvm/Transforms/Scalar/SCCP.h"
#include "llvm/Transforms/Scalar/SROA.h"
#include "llvm/Transforms/Scalar/SimpleLoopUnswitch.h"
#include "llvm/Transforms/Scalar/SimplifyCFG.h"
#include "llvm/Transforms/Scalar/SpeculativeExecution.h"
#include "llvm/Transforms/Scalar/TailRecursionElimination.h"
#include "llvm/Transforms/Scalar/WarnMissedTransforms.h"
#include "llvm/Transforms/Utils/AddDiscriminators.h"
#include "llvm/Transforms/Utils/AssumeBundleBuilder.h"
#include "llvm/Transforms/Utils/CanonicalizeAliases.h"
#include "llvm/Transforms/Utils/InjectTLIMappings.h"
#include "llvm/Transforms/Utils/LibCallsShrinkWrap.h"
#include "llvm/Transforms/Utils/LowerSwitch.h"
#include "llvm/Transforms/Utils/Mem2Reg.h"
#include "llvm/Transforms/Utils/NameAnonGlobals.h"
#include "llvm/Transforms/Utils/RelLookupTableConverter.h"
#include "llvm/Transforms/Utils/SimplifyCFGOptions.h"
#include "llvm/Transforms/Vectorize/LoopVectorize.h"
#include "llvm/Transforms/Vectorize/SLPVectorizer.h"
#if INTEL_CUSTOMIZATION
#include "llvm/Analysis/Intel_OptReport/OptReportOptionsPass.h"
#include "llvm/Analysis/Intel_XmainOptLevelPass.h"
#include "llvm/Transforms/Scalar/Intel_AddSubReassociate.h"
#include "llvm/Transforms/Scalar/Intel_DopeVectorHoist.h"
#include "llvm/Transforms/Scalar/Intel_ForcedCMOVGeneration.h"
#include "llvm/Transforms/Scalar/Intel_HandlePragmaVectorAligned.h"
#include "llvm/Transforms/Scalar/Intel_LoopCarriedCSE.h"
#include "llvm/Transforms/Scalar/Intel_MultiVersioning.h"
#include "llvm/Transforms/Scalar/Intel_OptReportEmitter.h"
#if INTEL_FEATURE_SW_ADVANCED
#include "llvm/Transforms/Scalar/Intel_NontemporalStore.h"
#endif // INTEL_FEATURE_SW_ADVANCED
#include "llvm/Transforms/Scalar/Intel_TransformSinAndCosCalls.h"
#include "llvm/Transforms/Vectorize/Intel_LoadCoalescing.h"
#include "llvm/Transforms/Vectorize/IntelMFReplacement.h"
#include "llvm/Transforms/Utils/Intel_VecClone.h"

// DPC++ CPU Kernel Transformation passes
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/Passes.h"

// Intel Loop Optimization framework
// Framework passes
#include "llvm/Analysis/Intel_LoopAnalysis/Framework/HIRFramework.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Framework/HIRRegionIdentification.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Framework/HIRSCCFormation.h"
#include "llvm/Transforms/Intel_LoopTransforms/HIRCodeGenPass.h"
#include "llvm/Transforms/Intel_LoopTransforms/HIROptReportEmitterPass.h"
#include "llvm/Transforms/Intel_LoopTransforms/HIRSSADeconstructionPass.h"

// VPlan Vectorizer passes
#include "llvm/Transforms/Intel_MapIntrinToIml/MapIntrinToIml.h"
#include "llvm/Transforms/Intel_VPO/VPODirectiveCleanup.h"
#include "llvm/Transforms/Vectorize/IntelVPlanDriver.h"
#include "llvm/Transforms/Vectorize/IntelVPlanFunctionVectorizer.h"
#include "llvm/Transforms/Vectorize/IntelVPlanPragmaOmpOrderedSimdExtract.h"
#include "llvm/Transforms/Vectorize/IntelVPlanPragmaOmpSimdIf.h"

// Analysis passes
#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRDDAnalysis.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRLocalityAnalysis.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRLoopResource.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRLoopStatistics.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRParVecAnalysis.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRSafeReductionAnalysis.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRSparseArrayReductionAnalysis.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRArraySectionAnalysis.h"

// Transformation passes
#include "llvm/Transforms/Intel_LoopTransforms/HIRAosToSoaPass.h"
#include "llvm/Transforms/Intel_LoopTransforms/HIRArrayScalarizationTestLauncherPass.h"
#include "llvm/Transforms/Intel_LoopTransforms/HIRArrayTransposePass.h"
#include "llvm/Transforms/Intel_LoopTransforms/HIRConditionalLoadStoreMotion.h"
#include "llvm/Transforms/Intel_LoopTransforms/HIRConditionalTempSinkingPass.h"
#include "llvm/Transforms/Intel_LoopTransforms/HIRDeadStoreEliminationPass.h"
#include "llvm/Transforms/Intel_LoopTransforms/HIRGeneralUnrollPass.h"
#include "llvm/Transforms/Intel_LoopTransforms/HIRGenerateMKLCallPass.h"
#include "llvm/Transforms/Intel_LoopTransforms/HIRMinMaxRecognitionPass.h"
#include "llvm/Transforms/Intel_LoopTransforms/HIRIdentityMatrixIdiomRecognitionPass.h"
#include "llvm/Transforms/Intel_LoopTransforms/HIRIdentityMatrixSubstitution.h"
#include "llvm/Transforms/Intel_LoopTransforms/HIRIdiomRecognitionPass.h"
#include "llvm/Transforms/Intel_LoopTransforms/HIRIfReversalPass.h"
#include "llvm/Transforms/Intel_LoopTransforms/HIRLMMPass.h"
#include "llvm/Transforms/Intel_LoopTransforms/HIRLastValueComputationPass.h"
#include "llvm/Transforms/Intel_LoopTransforms/HIRLoopBlockingPass.h"
#include "llvm/Transforms/Intel_LoopTransforms/HIRLoopCollapsePass.h"
#include "llvm/Transforms/Intel_LoopTransforms/HIRLoopConcatenationPass.h"
#include "llvm/Transforms/Intel_LoopTransforms/HIRLoopDistributionForLoopNestPass.h"
#include "llvm/Transforms/Intel_LoopTransforms/HIRLoopDistributionForMemRecPass.h"
#include "llvm/Transforms/Intel_LoopTransforms/HIRLoopFusionPass.h"
#include "llvm/Transforms/Intel_LoopTransforms/HIRLoopInterchangePass.h"
#include "llvm/Transforms/Intel_LoopTransforms/HIRLoopRematerializePass.h"
#include "llvm/Transforms/Intel_LoopTransforms/HIRLoopRerollPass.h"
#include "llvm/Transforms/Intel_LoopTransforms/HIRLoopReversalPass.h"
#include "llvm/Transforms/Intel_LoopTransforms/HIRMVForConstUBPass.h"
#include "llvm/Transforms/Intel_LoopTransforms/HIRMVForVariableStridePass.h"
#include "llvm/Transforms/Intel_LoopTransforms/HIRMemoryReductionSinkingPass.h"
#include "llvm/Transforms/Intel_LoopTransforms/HIRMultiExitLoopRerollPass.h"
#include "llvm/Transforms/Intel_LoopTransforms/HIRNonZeroSinkingForPerfectLoopnest.h"
#include "llvm/Transforms/Intel_LoopTransforms/HIRNontemporalMarking.h"
#include "llvm/Transforms/Intel_LoopTransforms/HIROptPredicatePass.h"
#include "llvm/Transforms/Intel_LoopTransforms/HIROptVarPredicatePass.h"
#include "llvm/Transforms/Intel_LoopTransforms/HIRPMSymbolicTripCountCompleteUnrollPass.h"
#include "llvm/Transforms/Intel_LoopTransforms/HIRPostVecCompleteUnrollPass.h"
#include "llvm/Transforms/Intel_LoopTransforms/HIRPreVecCompleteUnrollPass.h"
#include "llvm/Transforms/Intel_LoopTransforms/HIRPrefetchingPass.h"
#include "llvm/Transforms/Intel_LoopTransforms/HIRPropagateCastedIVPass.h"
#include "llvm/Transforms/Intel_LoopTransforms/HIRRecognizeParLoopPass.h"
#include "llvm/Transforms/Intel_LoopTransforms/HIRRowWiseMVPass.h"
#include "llvm/Transforms/Intel_LoopTransforms/HIRRuntimeDDPass.h"
#include "llvm/Transforms/Intel_LoopTransforms/HIRScalarReplArrayPass.h"
#include "llvm/Transforms/Intel_LoopTransforms/HIRSinkingForPerfectLoopnestPass.h"
#include "llvm/Transforms/Intel_LoopTransforms/HIRStoreResultIntoTempArray.h"
#include "llvm/Transforms/Intel_LoopTransforms/HIRSumWindowReuse.h"
#include "llvm/Transforms/Intel_LoopTransforms/HIRTempCleanupPass.h"
#include "llvm/Transforms/Intel_LoopTransforms/HIRUndoSinkingForPerfectLoopnestPass.h"
#include "llvm/Transforms/Intel_LoopTransforms/HIRUnrollAndJamPass.h"
#include "llvm/Transforms/Intel_LoopTransforms/HIRVecDirInsertPass.h"
#if INTEL_FEATURE_SW_ADVANCED
#include "llvm/Transforms/Intel_LoopTransforms/HIRCrossLoopArrayContraction.h"
#include "llvm/Transforms/Intel_LoopTransforms/HIRInterLoopBlockingPass.h"
#endif // INTEL_FEATURE_SW_ADVANCED

#if INTEL_FEATURE_SW_DTRANS
#include "Intel_DTrans/DTransCommon.h"
#include "Intel_DTrans/DTransPasses.h"
#endif // INTEL_FEATURE_SW_DTRANS
#include "llvm/Transforms/VPO/Paropt/Intel_VPOParoptOptimizeDataSharing.h"
#include "llvm/Transforms/VPO/Paropt/Intel_VPOParoptSharedPrivatization.h"
#include "llvm/Transforms/VPO/Paropt/Intel_VPOParoptTargetInline.h"
#include "llvm/Transforms/VPO/Paropt/Intel_VPOParoptApplyConfig.h"
#endif // INTEL_CUSTOMIZATION
#if INTEL_COLLAB
// VPO
#include "llvm/Analysis/VPO/WRegionInfo/WRegionCollection.h"
#include "llvm/Analysis/VPO/WRegionInfo/WRegionInfo.h"
#include "llvm/Transforms/VPO/Paropt/VPOParopt.h"
#include "llvm/Transforms/VPO/Paropt/VPOParoptLoopCollapse.h"
#include "llvm/Transforms/VPO/Paropt/VPOParoptLoopTransform.h"
#include "llvm/Transforms/VPO/Paropt/VPOParoptPrepare.h"
#include "llvm/Transforms/VPO/Paropt/VPOParoptTpv.h"
#include "llvm/Transforms/VPO/Utils/CFGRestructuring.h"
#include "llvm/Transforms/VPO/Utils/VPORestoreOperands.h"
#include "llvm/Transforms/VPO/Utils/CFGSimplify.h"
#endif // INTEL_COLLAB
#include "llvm/Transforms/Vectorize/VectorCombine.h"

using namespace llvm;
using namespace llvm::loopopt;                 // INTEL
using namespace llvm::llvm_intel_wp_analysis;  // INTEL

#if INTEL_CUSTOMIZATION
// Enable the partial inlining during LTO
static cl::opt<bool>
    RunLTOPartialInlining("enable-npm-lto-partial-inlining", cl::init(true),
                       cl::Hidden, cl::ZeroOrMore,
                       cl::desc("Run LTO Partial inlinining pass"));
#endif // INTEL_CUSTOMIZATION

#if INTEL_CUSTOMIZATION
#if INTEL_FEATURE_SW_ADVANCED
// Partial inlining for simple functions
static cl::opt<bool>
    EnableIntelPI("enable-npm-intel-pi", cl::init(true), cl::Hidden,
                cl::desc("Enable the partial inlining for simple functions"));
#endif // INTEL_FEATURE_SW_ADVANCED
#endif // INTEL_CUSTOMIZATION

static cl::opt<InliningAdvisorMode> UseInlineAdvisor(
    "enable-ml-inliner", cl::init(InliningAdvisorMode::Default), cl::Hidden,
    cl::desc("Enable ML policy for inliner. Currently trained for -Oz only"),
    cl::values(clEnumValN(InliningAdvisorMode::Default, "default",
                          "Heuristics-based inliner version."),
               clEnumValN(InliningAdvisorMode::Development, "development",
                          "Use development mode (runtime-loadable model)."),
               clEnumValN(InliningAdvisorMode::Release, "release",
                          "Use release mode (AOT-compiled model).")));

static cl::opt<bool> EnableSyntheticCounts(
    "enable-npm-synthetic-counts", cl::Hidden,
    cl::desc("Run synthetic function entry count generation "
             "pass"));
#if INTEL_CUSTOMIZATION
// Inline Aggressive Analysis
static cl::opt<bool> EnableInlineAggAnalysis(
   "enable-npm-inline-aggressive-analysis", cl::init(true), cl::Hidden,
   cl::desc("Enable Inline Aggressive Analysis for the new PM (default = on)"));

#if INTEL_FEATURE_SW_ADVANCED
// IP Cloning
static cl::opt<bool> EnableIPCloning(
    "enable-npm-ip-cloning", cl::init(true), cl::Hidden,
    cl::desc("Enable IP Cloning for the new PM (default = on)"));
#endif // INTEL_FEATURE_SW_ADVANCED
static cl::opt<bool> EnableCallTreeCloning(
    "enable-npm-call-tree-cloning", cl::init(true), cl::Hidden,
    cl::desc("Enable Call Tree Cloning for the new PM (default = on)"));

// IPO Array Transpose
static cl::opt<bool> EnableIPArrayTranspose(
   "enable-npm-ip-array-transpose", cl::init(true), cl::Hidden,
   cl::desc("Enable IPO Array Transpose for the new PM (default = on)"));

#if INTEL_FEATURE_SW_ADVANCED
// Dead Array Element Ops Elimination
static cl::opt<bool> EnableDeadArrayOpsElim(
   "enable-npm-dead-array-ops-elim", cl::init(true), cl::Hidden,
   cl::desc("Enable Dead Array Ops Elimination for the new PM (default = on)"));

// IPO Prefetch
static cl::opt<bool> EnableIPOPrefetch(
    "enable-npm-ipo-prefetch", cl::init(true), cl::Hidden,
    cl::desc("Enable IPO Prefetch"));
#endif // INTEL_FEATURE_SW_ADVANCED

// Indirect call Conv
static cl::opt<bool> EnableIndirectCallConv("enable-npm-ind-call-conv",
    cl::init(true), cl::Hidden,
    cl::desc("Enable Indirect Call Conv for the new PM (default = on)"));

// Function multi-versioning.
static cl::opt<bool> EnableMultiVersioning("enable-npm-multiversioning",
  cl::init(true), cl::ReallyHidden,
  cl::desc("Enable Function Multi-versioning in the new PM"));

// Enable Auto Cpu Dispatch.
static cl::opt<bool> EnableAX("enable-ax",
  cl::init(false), cl::ReallyHidden,
  cl::desc("Enable Auto CPU Dispatch"));

// Enable whole program analysis
static cl::opt<bool> EnableWPA("enable-npm-whole-program-analysis",
  cl::init(true), cl::ReallyHidden,
  cl::desc("Enable Whole Program analysis in the new pass manager"));
#endif // INTEL_CUSTOMIZATION

/// Flag to enable inline deferral during PGO.
static cl::opt<bool>
    EnablePGOInlineDeferral("enable-npm-pgo-inline-deferral", cl::init(true),
                            cl::Hidden,
                            cl::desc("Enable inline deferral during PGO"));

static cl::opt<bool> EnableMemProfiler("enable-mem-prof", cl::Hidden,
                                       cl::desc("Enable memory profiler"));

static cl::opt<bool> EnableModuleInliner("enable-module-inliner",
                                         cl::init(false), cl::Hidden,
                                         cl::desc("Enable module inliner"));

static cl::opt<bool> PerformMandatoryInliningsFirst(
    "mandatory-inlining-first", cl::init(true), cl::Hidden,
    cl::desc("Perform mandatory inlinings module-wide, before performing "
             "inlining."));

static cl::opt<bool> EnableO3NonTrivialUnswitching(
    "enable-npm-O3-nontrivial-unswitch", cl::init(true), cl::Hidden,
    cl::desc("Enable non-trivial loop unswitching for -O3"));

static cl::opt<bool> EnableEagerlyInvalidateAnalyses(
    "eagerly-invalidate-analyses", cl::init(true), cl::Hidden,
    cl::desc("Eagerly invalidate more analyses in default pipelines"));

static cl::opt<bool> EnableNoRerunSimplificationPipeline(
    "enable-no-rerun-simplification-pipeline", cl::init(true), cl::Hidden,
    cl::desc(
        "Prevent running the simplification pipeline on a function more "
        "than once in the case that SCC mutations cause a function to be "
        "visited multiple times as long as the function has not been changed"));

static cl::opt<bool> EnableMergeFunctions(
    "enable-merge-functions", cl::init(false), cl::Hidden,
    cl::desc("Enable function merging as part of the optimization pipeline"));

PipelineTuningOptions::PipelineTuningOptions() {
  LoopInterleaving = true;
  LoopVectorization = true;
  SLPVectorization = false;
  LoopUnrolling = true;
  ForgetAllSCEVInLoopUnroll = ForgetSCEVInLoopUnroll;
  LicmMssaOptCap = SetLicmMssaOptCap;
  LicmMssaNoAccForPromotionCap = SetLicmMssaNoAccForPromotionCap;
  CallGraphProfile = true;
  MergeFunctions = EnableMergeFunctions;
  EagerlyInvalidateAnalyses = EnableEagerlyInvalidateAnalyses;
  DisableIntelProprietaryOpts = false; // INTEL
  EnableAutoCPUDispatch = EnableAX; // INTEL
}
#if INTEL_CUSTOMIZATION
extern cl::opt<bool> ConvertToSubs;
extern cl::opt<bool> EnableLV;
enum class ThroughputMode { None, SingleJob, MultipleJob };
extern cl::opt<ThroughputMode> ThroughputModeOpt;
extern cl::opt<bool> EnableLoadCoalescing;
extern cl::opt<bool> EnableSROAAfterSLP;
#endif // INTEL_CUSTOMIZATION
namespace llvm {
#if INTEL_CUSTOMIZATION
extern cl::opt<bool> EnableHandlePragmaVectorAligned;
// Andersen AliasAnalysis
extern cl::opt<bool> EnableAndersen;
extern cl::opt<bool> EnableArgNoAliasProp;
extern cl::opt<bool> RunLoopOptFrameworkOnly;
enum class LoopOptMode { None, LightWeight, Full };
extern cl::opt<LoopOptMode> RunLoopOpts;
#endif // INTEL_CUSTOMIZATION
extern cl::opt<bool> ExtraVectorizerPasses;

extern cl::opt<unsigned> MaxDevirtIterations;
extern cl::opt<bool> EnableConstraintElimination;
extern cl::opt<bool> EnableFunctionSpecialization;
extern cl::opt<bool> EnableGVNHoist;
extern cl::opt<bool> EnableGVNSink;
extern cl::opt<bool> EnableHotColdSplit;
extern cl::opt<bool> EnableIROutliner;
extern cl::opt<bool> EnableOrderFileInstrumentation;
extern cl::opt<bool> EnableCHR;
extern cl::opt<bool> EnableLoopInterchange;
extern cl::opt<bool> EnableUnrollAndJam;
extern cl::opt<bool> EnableLoopFlatten;
extern cl::opt<bool> EnableDFAJumpThreading;
extern cl::opt<bool> RunNewGVN;
extern cl::opt<bool> RunPartialInlining;
extern cl::opt<bool> ExtraVectorizerPasses;

extern cl::opt<bool> FlattenedProfileUsed;

extern cl::opt<AttributorRunOption> AttributorRun;
extern cl::opt<bool> EnableKnowledgeRetention;

extern cl::opt<bool> EnableMatrix;

extern cl::opt<bool> DisablePreInliner;
extern cl::opt<int> PreInlineThreshold;
#if INTEL_COLLAB
// TODO: Change this to an enum class in PassManagerBuilder.cpp
enum { InvokeParoptBeforeInliner = 1, InvokeParoptAfterInliner };
extern cl::opt<unsigned> RunVPOOpt;
extern cl::opt<unsigned> RunVPOParopt;
extern cl::opt<bool> SPIRVOptimizationMode;
#endif // INTEL_COLLAB
#if INTEL_CUSTOMIZATION
extern cl::opt<bool> EnableVPlanDriver;
extern cl::opt<bool> RunVecClone;
extern cl::opt<bool> EnableDeviceSimd;
extern cl::opt<bool> EnableVPlanDriverHIR;
extern cl::opt<bool> RunVPOVecopt;
extern cl::opt<bool> RunPreLoopOptVPOPasses;
extern cl::opt<bool> RunPostLoopOptVPOPasses;
extern cl::opt<bool> EnableVPOParoptSharedPrivatization;
extern cl::opt<bool> EnableVPOParoptTargetInline;
#endif // INTEL_CUSTOMIZATION

#if INTEL_CUSTOMIZATION
extern cl::opt<bool> EnableStdContainerOpt;
extern cl::opt<bool> EnableNonLTOGlobalVarOpt;
extern cl::opt<bool> EarlyJumpThreading;
#endif // INTEL_CUSTOMIZATION

extern cl::opt<bool> SYCLOptimizationMode;
} // namespace llvm

void PassBuilder::invokePeepholeEPCallbacks(FunctionPassManager &FPM,
                                            OptimizationLevel Level) {
  for (auto &C : PeepholeEPCallbacks)
    C(FPM, Level);
}

// Helper to add AnnotationRemarksPass.
static void addAnnotationRemarksPass(ModulePassManager &MPM) {
  MPM.addPass(createModuleToFunctionPassAdaptor(AnnotationRemarksPass()));
}

#if INTEL_CUSTOMIZATION
void PassBuilder::addInstCombinePass(FunctionPassManager &FPM,
                                     bool EnableUpCasting) const {
  // Enable it when SLP Vectorizer is off or after SLP Vectorizer pass.
  bool EnableFcmpMinMaxCombine =
      (!PrepareForLTO && !PTO.SLPVectorization) || AfterSLPVectorizer;
#if INTEL_FEATURE_SW_DTRANS
  // Configure the instruction combining pass to avoid some transformations
  // that lose type information for DTrans.
  bool PreserveForDTrans = (PrepareForLTO && DTransEnabled);
#else // INTEL_FEATURE_SW_DTRANS
  bool PreserveForDTrans = false;
#endif // INTEL_FEATURE_SW_DTRANS
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
    FPM.addPass(VPOCFGRestructuringPass());
  }
  FPM.addPass(InstCombinePass(PreserveForDTrans,
                              PrepareForLTO && EnableIPArrayTranspose,
                              EnableFcmpMinMaxCombine, EnableUpCasting));
}
#endif // INTEL_CUSTOMIZATION

// Helper to check if the current compilation phase is preparing for LTO
static bool isLTOPreLink(ThinOrFullLTOPhase Phase) {
  return Phase == ThinOrFullLTOPhase::ThinLTOPreLink ||
         Phase == ThinOrFullLTOPhase::FullLTOPreLink;
}

// TODO: Investigate the cost/benefit of tail call elimination on debugging.
FunctionPassManager
PassBuilder::buildO1FunctionSimplificationPipeline(OptimizationLevel Level,
                                                   ThinOrFullLTOPhase Phase) {

  FunctionPassManager FPM;

#if INTEL_CUSTOMIZATION
  // Propagate TBAA information before SROA so that we can remove mid-function
  // fakeload intrinsics which would block SROA.
  FPM.addPass(TbaaMDPropagationPass());
  // Run OptReportOptionsPass early so that it is available to all users.
  FPM.addPass(RequireAnalysisPass<OptReportOptionsAnalysis, Function>());
#endif // INTEL_CUSTOMIZATION

  // Form SSA out of local memory accesses after breaking apart aggregates into
  // scalars.
  FPM.addPass(SROAPass());

  // Catch trivial redundancies
  FPM.addPass(EarlyCSEPass(true /* Enable mem-ssa. */));

  // Hoisting of scalars and load expressions.
  FPM.addPass(
      SimplifyCFGPass(SimplifyCFGOptions().convertSwitchRangeToICmp(true)));
  addInstCombinePass(FPM, !DTransEnabled); //INTEL

  FPM.addPass(LibCallsShrinkWrapPass());

  invokePeepholeEPCallbacks(FPM, Level);

  FPM.addPass(
      SimplifyCFGPass(SimplifyCFGOptions().convertSwitchRangeToICmp(true)));

  // Form canonically associated expression trees, and simplify the trees using
  // basic mathematical properties. For example, this will form (nearly)
  // minimal multiplication trees.
  if (!SYCLOptimizationMode) {
  // FIXME: re-association increases variables liveness and therefore register
  // pressure.
#if INTEL_COLLAB
    if (!SPIRVOptimizationMode)
      FPM.addPass(ReassociatePass());
#else // INTEL_COLLAB
    FPM.addPass(ReassociatePass());
#endif // INTEL_COLLAB

    // Do not run loop pass pipeline in "SYCL Optimization Mode". Loop
    // optimizations rely on TTI, which is not accurate for SPIR target.

    // Add the primary loop simplification pipeline.
    // FIXME: Currently this is split into two loop pass pipelines because we
    // run some function passes in between them. These can and should be removed
    // and/or replaced by scheduling the loop pass equivalents in the correct
    // positions. But those equivalent passes aren't powerful enough yet.
    // Specifically, `SimplifyCFGPass` and `InstCombinePass` are currently still
    // used. We have `LoopSimplifyCFGPass` which isn't yet powerful enough yet
    // to fully replace `SimplifyCFGPass`, and the closest to the other we have
    // is `LoopInstSimplify`.
    LoopPassManager LPM1, LPM2;

    // Simplify the loop body. We do this initially to clean up after other loop
    // passes run, either when iterating on a loop or on inner loops with
    // implications on the outer loop.
    LPM1.addPass(LoopInstSimplifyPass());
    LPM1.addPass(LoopSimplifyCFGPass());

    // Try to remove as much code from the loop header as possible,
    // to reduce amount of IR that will have to be duplicated. However,
    // do not perform speculative hoisting the first time as LICM
    // will destroy metadata that may not need to be destroyed if run
    // after loop rotation.
    // TODO: Investigate promotion cap for O1.
#if INTEL_CUSTOMIZATION
    // 27770/28531: This extra pass causes high spill rates in several
    // benchmarks.
    if (!DTransEnabled)
      LPM1.addPass(
          LICMPass(PTO.LicmMssaOptCap, PTO.LicmMssaNoAccForPromotionCap,
                   /*AllowSpeculation=*/false));
#else // INTEL_CUSTOMIZATION
    LPM1.addPass(LICMPass(PTO.LicmMssaOptCap, PTO.LicmMssaNoAccForPromotionCap,
                          /*AllowSpeculation=*/false));
#endif // INTEL_CUSTOMIZATION

    LPM1.addPass(LoopRotatePass(/* Disable header duplication */ true,
                                isLTOPreLink(Phase)));
    // TODO: Investigate promotion cap for O1.
    LPM1.addPass(LICMPass(PTO.LicmMssaOptCap, PTO.LicmMssaNoAccForPromotionCap,
                          /*AllowSpeculation=*/true));
    LPM1.addPass(SimpleLoopUnswitchPass());
    if (EnableLoopFlatten)
      LPM1.addPass(LoopFlattenPass());

    LPM2.addPass(LoopIdiomRecognizePass());
#if INTEL_COLLAB
    if (!SPIRVOptimizationMode)
      LPM2.addPass(IndVarSimplifyPass());
    else
      LPM2.addPass(IndVarSimplifyPass(false /* WidenIndVars */));
#else // INTEL_COLLAB
    LPM2.addPass(IndVarSimplifyPass());
#endif // INTEL_COLLAB

    for (auto &C : LateLoopOptimizationsEPCallbacks)
      C(LPM2, Level);

    LPM2.addPass(LoopDeletionPass());

    if (EnableLoopInterchange)
      LPM2.addPass(LoopInterchangePass());

    // Do not enable unrolling in PreLinkThinLTO phase during sample PGO
    // because it changes IR to makes profile annotation in back compile
    // inaccurate. The normal unroller doesn't pay attention to forced full
    // unroll attributes so we need to make sure and allow the full unroll pass
    // to pay attention to it.
    if (Phase != ThinOrFullLTOPhase::ThinLTOPreLink || !PGOOpt ||
        PGOOpt->Action != PGOOptions::SampleUse)
      LPM2.addPass(LoopFullUnrollPass(Level.getSpeedupLevel(),
                                      /* OnlyWhenForced= */ !PTO.LoopUnrolling,
                                      PTO.ForgetAllSCEVInLoopUnroll));

    for (auto &C : LoopOptimizerEndEPCallbacks)
      C(LPM2, Level);

    // We provide the opt remark emitter pass for LICM to use. We only need to
    // do this once as it is immutable.
    FPM.addPass(
        RequireAnalysisPass<OptimizationRemarkEmitterAnalysis, Function>());
    FPM.addPass(
        createFunctionToLoopPassAdaptor(std::move(LPM1),
                                        /*UseMemorySSA=*/true,
                                        /*UseBlockFrequencyInfo=*/true));
    FPM.addPass(
        SimplifyCFGPass(SimplifyCFGOptions().convertSwitchRangeToICmp(true)));
    addInstCombinePass(FPM, !DTransEnabled); // INTEL
    // The loop passes in LPM2 (LoopFullUnrollPass) do not preserve MemorySSA.
    // *All* loop passes must preserve it, in order to be able to use it.
    FPM.addPass(
        createFunctionToLoopPassAdaptor(std::move(LPM2),
                                        /*UseMemorySSA=*/false,
                                        /*UseBlockFrequencyInfo=*/false));
  }
  // Delete small array after loop unroll.
  FPM.addPass(SROAPass());

  // Specially optimize memory movement as it doesn't look like dataflow in SSA.
  FPM.addPass(MemCpyOptPass());

  // Sparse conditional constant propagation.
  // FIXME: It isn't clear why we do this *after* loop passes rather than
  // before...
  FPM.addPass(SCCPPass());

  // Delete dead bit computations (instcombine runs after to fold away the dead
  // computations, and then ADCE will run later to exploit any new DCE
  // opportunities that creates).
  FPM.addPass(BDCEPass());

  // Run instcombine after redundancy and dead bit elimination to exploit
  // opportunities opened up by them.
  addInstCombinePass(FPM, !DTransEnabled); // INTEL
  invokePeepholeEPCallbacks(FPM, Level);

  FPM.addPass(CoroElidePass());

  for (auto &C : ScalarOptimizerLateEPCallbacks)
    C(FPM, Level);

  // Finally, do an expensive DCE pass to catch all the dead code exposed by
  // the simplifications and basic cleanup after all the simplifications.
  // TODO: Investigate if this is too expensive.
  FPM.addPass(ADCEPass());
  FPM.addPass(
      SimplifyCFGPass(SimplifyCFGOptions().convertSwitchRangeToICmp(true)));
  addInstCombinePass(FPM, !DTransEnabled); // INTEL
  invokePeepholeEPCallbacks(FPM, Level);

#if INTEL_CUSTOMIZATION
  if (!SYCLOptimizationMode)
    FPM.addPass(TransformSinAndCosCallsPass());
#endif // INTEL_CUSTOMIZATION

  return FPM;
}
#if INTEL_CUSTOMIZATION
bool PassBuilder::isLoopOptEnabled(OptimizationLevel Level) {
  if (!PTO.DisableIntelProprietaryOpts &&
      ((RunLoopOpts != LoopOptMode::None) || RunLoopOptFrameworkOnly) &&
      (Level.getSpeedupLevel() >= 2))
    return true;

  return false;
}
#endif // INTEL_CUSTOMIZATION

FunctionPassManager
PassBuilder::buildFunctionSimplificationPipeline(OptimizationLevel Level,
                                                 ThinOrFullLTOPhase Phase) {
  assert(Level != OptimizationLevel::O0 && "Must request optimizations!");

  // The O1 pipeline has a separate pipeline creation function to simplify
  // construction readability.
  if (Level.getSpeedupLevel() == 1)
    return buildO1FunctionSimplificationPipeline(Level, Phase);

  FunctionPassManager FPM;

#if INTEL_CUSTOMIZATION
  // Propagate TBAA information before SROA so that we can remove mid-function
  // fakeload intrinsics which would block SROA.
  FPM.addPass(TbaaMDPropagationPass());
  // Run OptReportOptionsPass early so that it is available to all users.
  FPM.addPass(RequireAnalysisPass<OptReportOptionsAnalysis, Function>());
#endif // INTEL_CUSTOMIZATION

  // Form SSA out of local memory accesses after breaking apart aggregates into
  // scalars.
  FPM.addPass(SROAPass());

  // Catch trivial redundancies
  FPM.addPass(EarlyCSEPass(true /* Enable mem-ssa. */));
  if (EnableKnowledgeRetention)
    FPM.addPass(AssumeSimplifyPass());

  // Hoisting of scalars and load expressions.
  if (EnableGVNHoist)
    FPM.addPass(GVNHoistPass());

  // Global value numbering based sinking.
  if (EnableGVNSink) {
    FPM.addPass(GVNSinkPass());
    FPM.addPass(
        SimplifyCFGPass(SimplifyCFGOptions().convertSwitchRangeToICmp(true)));
  }

  if (EnableConstraintElimination)
    FPM.addPass(ConstraintEliminationPass());

  // Speculative execution if the target has divergent branches; otherwise nop.
  FPM.addPass(SpeculativeExecutionPass(/* OnlyIfDivergentTarget =*/true));

  // Optimize based on known information about branches, and cleanup afterward.
  FPM.addPass(JumpThreadingPass());
  FPM.addPass(CorrelatedValuePropagationPass());

  FPM.addPass(
      SimplifyCFGPass(SimplifyCFGOptions().convertSwitchRangeToICmp(true)));
#if INTEL_CUSTOMIZATION
  // Combine silly sequences. Set PreserveAddrCompute to true in LTO phase 1 if
  // IP ArrayTranspose is enabled.
  addInstCombinePass(FPM, !DTransEnabled);
#endif // INTEL_CUSTOMIZATION
  if (Level == OptimizationLevel::O3)
    FPM.addPass(AggressiveInstCombinePass());

  if (!Level.isOptimizingForSize())
    FPM.addPass(LibCallsShrinkWrapPass());

  invokePeepholeEPCallbacks(FPM, Level);

  // For PGO use pipeline, try to optimize memory intrinsics such as memcpy
  // using the size value profile. Don't perform this when optimizing for size.
  if (PGOOpt && PGOOpt->Action == PGOOptions::IRUse &&
      !Level.isOptimizingForSize())
    FPM.addPass(PGOMemOPSizeOpt());

#if INTEL_CUSTOMIZATION
#if INTEL_FEATURE_SW_DTRANS
  bool SkipRecProgression = PrepareForLTO && DTransEnabled;
#else // INTEL_FEATURE_SW_DTRANS
  bool SkipRecProgression = false;
#endif // INTEL_FEATURE_SW_DTRANS
  // TODO: Investigate the cost/benefit of tail call elimination on debugging.
  FPM.addPass(TailCallElimPass(SkipRecProgression));
#endif // INTEL_CUSTOMIZATION
  FPM.addPass(
      SimplifyCFGPass(SimplifyCFGOptions().convertSwitchRangeToICmp(true)));

  // Form canonically associated expression trees, and simplify the trees using
  // basic mathematical properties. For example, this will form (nearly)
  // minimal multiplication trees.
  if (!SYCLOptimizationMode) {
    // FIXME: re-association increases variables liveness and therefore register
    // pressure.
#if INTEL_COLLAB
    if (!SPIRVOptimizationMode)
      FPM.addPass(ReassociatePass());
#else // INTEL_COLLAB
    FPM.addPass(ReassociatePass());
#endif // INTEL_COLLAB

    // Do not run loop pass pipeline in "SYCL Optimization Mode". Loop
    // optimizations rely on TTI, which is not accurate for SPIR target.

    // Add the primary loop simplification pipeline.
    // FIXME: Currently this is split into two loop pass pipelines because we
    // run some function passes in between them. These can and should be removed
    // and/or replaced by scheduling the loop pass equivalents in the correct
    // positions. But those equivalent passes aren't powerful enough yet.
    // Specifically, `SimplifyCFGPass` and `InstCombinePass` are currently still
    // used. We have `LoopSimplifyCFGPass` which isn't yet powerful enough yet
    // to fully replace `SimplifyCFGPass`, and the closest to the other we have
    // is `LoopInstSimplify`.
    LoopPassManager LPM1, LPM2;

    // Simplify the loop body. We do this initially to clean up after other loop
    // passes run, either when iterating on a loop or on inner loops with
    // implications on the outer loop.
    LPM1.addPass(LoopInstSimplifyPass());
    LPM1.addPass(LoopSimplifyCFGPass());

    // Try to remove as much code from the loop header as possible,
    // to reduce amount of IR that will have to be duplicated. However,
    // do not perform speculative hoisting the first time as LICM
    // will destroy metadata that may not need to be destroyed if run
    // after loop rotation.
    // TODO: Investigate promotion cap for O1.
#if INTEL_CUSTOMIZATION
  // 27770/28531: This extra pass causes high spill rates in several
  // benchmarks.
    if (!DTransEnabled)
      LPM1.addPass(
          LICMPass(PTO.LicmMssaOptCap, PTO.LicmMssaNoAccForPromotionCap,
                   /*AllowSpeculation=*/false));
#else // INTEL_CUSTOMIZATION
    LPM1.addPass(LICMPass(PTO.LicmMssaOptCap, PTO.LicmMssaNoAccForPromotionCap,
                          /*AllowSpeculation=*/false));
#endif // INTEL_CUSTOMIZATION

    // Disable header duplication in loop rotation at -Oz.
    LPM1.addPass(
        LoopRotatePass(Level != OptimizationLevel::Oz, isLTOPreLink(Phase)));
    // TODO: Investigate promotion cap for O1.
    LPM1.addPass(LICMPass(PTO.LicmMssaOptCap, PTO.LicmMssaNoAccForPromotionCap,
                          /*AllowSpeculation=*/true));
    LPM1.addPass(SimpleLoopUnswitchPass(/* NonTrivial */ Level ==
                                            OptimizationLevel::O3 &&
                                        EnableO3NonTrivialUnswitching));
    if (EnableLoopFlatten)
      LPM1.addPass(LoopFlattenPass());

    LPM2.addPass(LoopIdiomRecognizePass());
#if INTEL_COLLAB
    if (!SPIRVOptimizationMode)
      LPM2.addPass(IndVarSimplifyPass());
    else
      LPM2.addPass(IndVarSimplifyPass(false /* WidenIndVars */));
#else // INTEL_COLLAB
    LPM2.addPass(IndVarSimplifyPass());
#endif // INTEL_COLLAB

    for (auto &C : LateLoopOptimizationsEPCallbacks)
      C(LPM2, Level);

    LPM2.addPass(LoopDeletionPass());

    if (EnableLoopInterchange)
      LPM2.addPass(LoopInterchangePass());

    // Do not enable unrolling in PreLinkThinLTO phase during sample PGO
    // because it changes IR to makes profile annotation in back compile
    // inaccurate. The normal unroller doesn't pay attention to forced full
    // unroll attributes so we need to make sure and allow the full unroll pass
    // to pay attention to it.
#if INTEL_CUSTOMIZATION
    // HIR complete unroll pass replaces LLVM's full loop unroll pass.
    if ((Phase != ThinOrFullLTOPhase::ThinLTOPreLink || !PGOOpt ||
        PGOOpt->Action != PGOOptions::SampleUse) && !isLoopOptEnabled(Level))
#endif // INTEL_CUSTOMIZATION
      LPM2.addPass(LoopFullUnrollPass(Level.getSpeedupLevel(),
                                      /* OnlyWhenForced= */ !PTO.LoopUnrolling,
                                      PTO.ForgetAllSCEVInLoopUnroll));

    for (auto &C : LoopOptimizerEndEPCallbacks)
      C(LPM2, Level);

    // We provide the opt remark emitter pass for LICM to use. We only need to
    // do this once as it is immutable.
    FPM.addPass(
        RequireAnalysisPass<OptimizationRemarkEmitterAnalysis, Function>());
    FPM.addPass(
        createFunctionToLoopPassAdaptor(std::move(LPM1),
                                        /*UseMemorySSA=*/true,
                                        /*UseBlockFrequencyInfo=*/true));
    FPM.addPass(
        SimplifyCFGPass(SimplifyCFGOptions().convertSwitchRangeToICmp(true)));
#if INTEL_CUSTOMIZATION
    // Combine silly sequences. Set PreserveAddrCompute to true in LTO phase 1 if
    // IP ArrayTranspose is enabled.
    addInstCombinePass(FPM, !DTransEnabled);
#else // INTEL_CUSTOMIZATION
    FPM.addPass(InstCombinePass());
#endif // INTEL_CUSTOMIZATION
    // The loop passes in LPM2 (LoopIdiomRecognizePass, IndVarSimplifyPass,
    // LoopDeletionPass and LoopFullUnrollPass) do not preserve MemorySSA.
    // *All* loop passes must preserve it, in order to be able to use it.
    FPM.addPass(
        createFunctionToLoopPassAdaptor(std::move(LPM2),
                                        /*UseMemorySSA=*/false,
                                        /*UseBlockFrequencyInfo=*/false));
  }

  // Delete small array after loop unroll.
  FPM.addPass(SROAPass());

  // The matrix extension can introduce large vector operations early, which can
  // benefit from running vector-combine early on.
  if (EnableMatrix)
    FPM.addPass(VectorCombinePass(/*ScalarizationOnly=*/true));

  // Eliminate redundancies.
  FPM.addPass(MergedLoadStoreMotionPass());
  if (RunNewGVN)
    FPM.addPass(NewGVNPass());
  else
    FPM.addPass(GVNPass());

  // Sparse conditional constant propagation.
  // FIXME: It isn't clear why we do this *after* loop passes rather than
  // before...
  FPM.addPass(SCCPPass());

  // Delete dead bit computations (instcombine runs after to fold away the dead
  // computations, and then ADCE will run later to exploit any new DCE
  // opportunities that creates).
  FPM.addPass(BDCEPass());

  // Run instcombine after redundancy and dead bit elimination to exploit
  // opportunities opened up by them.
#if INTEL_CUSTOMIZATION
  // Combine silly sequences. Set PreserveAddrCompute to true in LTO phase 1 if
  // IP ArrayTranspose is enabled.
  addInstCombinePass(FPM, !DTransEnabled);
#endif // INTEL_CUSTOMIZATION
  invokePeepholeEPCallbacks(FPM, Level);

  // Re-consider control flow based optimizations after redundancy elimination,
  // redo DCE, etc.
  if (EnableDFAJumpThreading && Level.getSizeLevel() == 0)
    FPM.addPass(DFAJumpThreadingPass());

  FPM.addPass(JumpThreadingPass());
  FPM.addPass(CorrelatedValuePropagationPass());

  // Finally, do an expensive DCE pass to catch all the dead code exposed by
  // the simplifications and basic cleanup after all the simplifications.
  // TODO: Investigate if this is too expensive.
  FPM.addPass(ADCEPass());

  // Specially optimize memory movement as it doesn't look like dataflow in SSA.
#if INTEL_CUSTOMIZATION
#if INTEL_FEATURE_SW_DTRANS
  // Skip MemCpyOpt when both PrepareForLTO and DTransEnabled flags are
  // true to simplify handling of memcpy/memset/memmov calls in DTrans
  // implementation.
  // TODO: Remove this customization once DTrans handled partial memcpy/
  // memset/memmov calls of struct types.
  if (!PrepareForLTO || !DTransEnabled)
    FPM.addPass(MemCpyOptPass());
#else // INTEL_FEATURE_SW_DTRANS
  FPM.addPass(MemCpyOptPass());
#endif // INTEL_FEATURE_SW_DTRANS
#endif // INTEL_CUSTOMIZATION

  FPM.addPass(DSEPass());
  FPM.addPass(createFunctionToLoopPassAdaptor(
      LICMPass(PTO.LicmMssaOptCap, PTO.LicmMssaNoAccForPromotionCap,
               /*AllowSpeculation=*/true),
      /*UseMemorySSA=*/true, /*UseBlockFrequencyInfo=*/true));

  FPM.addPass(CoroElidePass());

  for (auto &C : ScalarOptimizerLateEPCallbacks)
    C(FPM, Level);

#if INTEL_CUSTOMIZATION
  // Hoisting of common instructions can result in unstructured CFG input to
  // loopopt. Loopopt has its own pass which hoists conditional loads/stores.
  if (isLoopOptEnabled(Level)) {
    FPM.addPass(SimplifyCFGPass(SimplifyCFGOptions()));
  } else {
    if (SYCLOptimizationMode)
      FPM.addPass(SimplifyCFGPass());
    else
      FPM.addPass(SimplifyCFGPass(SimplifyCFGOptions()
                                      .convertSwitchRangeToICmp(true)
                                      .hoistCommonInsts(true)
                                      .sinkCommonInsts(true)));
  }
  // Combine silly sequences. Set PreserveAddrCompute to true in LTO phase 1 if
  // IP ArrayTranspose is enabled.
  addInstCombinePass(FPM, !DTransEnabled);
#endif // INTEL_CUSTOMIZATION
  invokePeepholeEPCallbacks(FPM, Level);

  if (EnableCHR && Level == OptimizationLevel::O3 && PGOOpt &&
      (PGOOpt->Action == PGOOptions::IRUse ||
       PGOOpt->Action == PGOOptions::SampleUse))
    FPM.addPass(ControlHeightReductionPass());
#if INTEL_CUSTOMIZATION
  if (!SYCLOptimizationMode)
    FPM.addPass(TransformSinAndCosCallsPass());
#endif // INTEL_CUSTOMIZATION

  return FPM;
}

void PassBuilder::addRequiredLTOPreLinkPasses(ModulePassManager &MPM) {
  MPM.addPass(CanonicalizeAliasesPass());
  MPM.addPass(NameAnonGlobalPass());
}

void PassBuilder::addPGOInstrPasses(ModulePassManager &MPM,
                                    OptimizationLevel Level, bool RunProfileGen,
                                    bool IsCS, std::string ProfileFile,
                                    std::string ProfileRemappingFile,
                                    ThinOrFullLTOPhase LTOPhase) {
  assert(Level != OptimizationLevel::O0 && "Not expecting O0 here!");
  if (!IsCS && !DisablePreInliner) {
    InlineParams IP;

    IP.DefaultThreshold = PreInlineThreshold;

    // FIXME: The hint threshold has the same value used by the regular inliner
    // when not optimzing for size. This should probably be lowered after
    // performance testing.
    // FIXME: this comment is cargo culted from the old pass manager, revisit).
    IP.HintThreshold = Level.isOptimizingForSize() ? PreInlineThreshold : 325;
    IP.PrepareForLTO = PrepareForLTO; // INTEL

#if INTEL_CUSTOMIZATION
    // Parse -[no]inline-list option and set corresponding attributes.
    MPM.addPass(InlineListsPass());
#endif //INTEL_CUSTOMIZATION

    ModuleInlinerWrapperPass MIWP(
        IP, /* MandatoryFirst */ true,
        InlineContext{LTOPhase, InlinePass::EarlyInliner});
    CGSCCPassManager &CGPipeline = MIWP.getPM();

    FunctionPassManager FPM;
    FPM.addPass(SROAPass());
    FPM.addPass(EarlyCSEPass());    // Catch trivial redundancies.
    FPM.addPass(SimplifyCFGPass(SimplifyCFGOptions().convertSwitchRangeToICmp(
        true)));                    // Merge & remove basic blocks.
#if INTEL_CUSTOMIZATION
    // Combine silly sequences. Set PreserveAddrCompute to true in LTO phase 1
    // if IP ArrayTranspose is enabled.
    addInstCombinePass(FPM, !DTransEnabled);
#endif // INTEL_CUSTOMIZATION
    invokePeepholeEPCallbacks(FPM, Level);

    CGPipeline.addPass(createCGSCCToFunctionPassAdaptor(
        std::move(FPM), PTO.EagerlyInvalidateAnalyses));

    MPM.addPass(std::move(MIWP));

    // Delete anything that is now dead to make sure that we don't instrument
    // dead code. Instrumentation can end up keeping dead code around and
    // dramatically increase code size.
    MPM.addPass(GlobalDCEPass());
  }

  if (!RunProfileGen) {
    assert(!ProfileFile.empty() && "Profile use expecting a profile file!");
    MPM.addPass(PGOInstrumentationUse(ProfileFile, ProfileRemappingFile, IsCS));
    // Cache ProfileSummaryAnalysis once to avoid the potential need to insert
    // RequireAnalysisPass for PSI before subsequent non-module passes.
    MPM.addPass(RequireAnalysisPass<ProfileSummaryAnalysis, Module>());
    return;
  }

  // Perform PGO instrumentation.
  MPM.addPass(PGOInstrumentationGen(IsCS));

  // Disable header duplication in loop rotation at -Oz.
  MPM.addPass(createModuleToFunctionPassAdaptor(
      createFunctionToLoopPassAdaptor(
          LoopRotatePass(Level != OptimizationLevel::Oz),
          /*UseMemorySSA=*/false,
          /*UseBlockFrequencyInfo=*/false),
      PTO.EagerlyInvalidateAnalyses));

  // Add the profile lowering pass.
  InstrProfOptions Options;
  if (!ProfileFile.empty())
    Options.InstrProfileOutput = ProfileFile;
  // Do counter promotion at Level greater than O0.
  Options.DoCounterPromotion = true;
  Options.UseBFIInPromotion = IsCS;
  MPM.addPass(InstrProfiling(Options, IsCS));
}

void PassBuilder::addPGOInstrPassesForO0(ModulePassManager &MPM,
                                         bool RunProfileGen, bool IsCS,
                                         std::string ProfileFile,
                                         std::string ProfileRemappingFile) {
  if (!RunProfileGen) {
    assert(!ProfileFile.empty() && "Profile use expecting a profile file!");
    MPM.addPass(PGOInstrumentationUse(ProfileFile, ProfileRemappingFile, IsCS));
    // Cache ProfileSummaryAnalysis once to avoid the potential need to insert
    // RequireAnalysisPass for PSI before subsequent non-module passes.
    MPM.addPass(RequireAnalysisPass<ProfileSummaryAnalysis, Module>());
    return;
  }

  // Perform PGO instrumentation.
  MPM.addPass(PGOInstrumentationGen(IsCS));
  // Add the profile lowering pass.
  InstrProfOptions Options;
  if (!ProfileFile.empty())
    Options.InstrProfileOutput = ProfileFile;
  // Do not do counter promotion at O0.
  Options.DoCounterPromotion = false;
  Options.UseBFIInPromotion = IsCS;
  MPM.addPass(InstrProfiling(Options, IsCS));
}

static InlineParams getInlineParamsFromOptLevel(OptimizationLevel Level,
#if INTEL_CUSTOMIZATION
                                                bool PrepareForLTO,
                                                bool LinkForLTO,
                                                bool SYCLOptimizationMode) {
  return getInlineParams(Level.getSpeedupLevel(), Level.getSizeLevel(),
                         PrepareForLTO, LinkForLTO, SYCLOptimizationMode);
#endif // INTEL_CUSTOMIZATION
}

ModuleInlinerWrapperPass
PassBuilder::buildInlinerPipeline(OptimizationLevel Level,
#if INTEL_COLLAB
                                  ThinOrFullLTOPhase Phase,
                                  ModulePassManager *MPM) {
#else
                                  ThinOrFullLTOPhase Phase) {
#endif // INTEL_COLLAB
#if INTEL_CUSTOMIZATION
  InlineParams IP = getInlineParamsFromOptLevel(Level, PrepareForLTO,
      LinkForLTO, SYCLOptimizationMode);
#endif // INTEL_CUSTOMIZATION
  // For PreLinkThinLTO + SamplePGO, set hot-caller threshold to 0 to
  // disable hot callsite inline (as much as possible [1]) because it makes
  // profile annotation in the backend inaccurate.
  //
  // [1] Note the cost of a function could be below zero due to erased
  // prologue / epilogue.
  if (Phase == ThinOrFullLTOPhase::ThinLTOPreLink && PGOOpt &&
      PGOOpt->Action == PGOOptions::SampleUse)
    IP.HotCallSiteThreshold = 0;

  if (PGOOpt)
    IP.EnableDeferral = EnablePGOInlineDeferral;

  ModuleInlinerWrapperPass MIWP(
      IP, PerformMandatoryInliningsFirst,
      InlineContext{Phase, InlinePass::CGSCCInliner},
      UseInlineAdvisor, MaxDevirtIterations);

#if INTEL_COLLAB
  auto AddPreCGSCCModulePasses = [&](ModuleInlinerWrapperPass& MIWP) {
#endif // INTEL_COLLAB

  // Require the GlobalsAA analysis for the module so we can query it within
  // the CGSCC pipeline.
  MIWP.addModulePass(RequireAnalysisPass<GlobalsAA, Module>());
  // Invalidate AAManager so it can be recreated and pick up the newly available
  // GlobalsAA.
  MIWP.addModulePass(
      createModuleToFunctionPassAdaptor(InvalidateAnalysisPass<AAManager>()));

  // Require the ProfileSummaryAnalysis for the module so we can query it within
  // the inliner pass.
  MIWP.addModulePass(RequireAnalysisPass<ProfileSummaryAnalysis, Module>());

#if INTEL_COLLAB
  }; // AddPreCGSCCModulePasses

  AddPreCGSCCModulePasses(MIWP);
#endif // INTEL_COLLAB
  // Now begin the main postorder CGSCC pipeline.
  // FIXME: The current CGSCC pipeline has its origins in the legacy pass
  // manager and trying to emulate its precise behavior. Much of this doesn't
  // make a lot of sense and we should revisit the core CGSCC structure.
  CGSCCPassManager &MainCGPipeline = MIWP.getPM();

  // Note: historically, the PruneEH pass was run first to deduce nounwind and
  // generally clean up exception handling overhead. It isn't clear this is
  // valuable as the inliner doesn't currently care whether it is inlining an
  // invoke or a call.
#if INTEL_COLLAB
  if (RunVPOParopt && RunVPOOpt == InvokeParoptAfterInliner) {
    assert(MPM && "Need MPM to insert inliner + paropt before the full inliner "
                  "+ cgscc pass pipeline");
    // Run Inliner once before Paropt to align with the legacy pass manager, and
    // leave the full inliner+CGSCC pipeline unbroken for a subsequent run after
    // Paropt.
    ModuleInlinerWrapperPass PMIWP(
        IP,
        // This can be set to false if always inliner
        // is not needed before Paropt.
        PerformMandatoryInliningsFirst,
        InlineContext{Phase, InlinePass::CGSCCInliner}, UseInlineAdvisor,
        // Don't use DevirtSCCRepeatedPass to track
        // indirect -> direct call conversions.
        /*MaxDevirtIterations=*/0);
    // Process OpenMP directives at -O1 and above.
    AddPreCGSCCModulePasses(PMIWP);
    MPM->addPass(std::move(PMIWP));
    FunctionPassManager FPM;
    addVPOPasses(*MPM, FPM, Level, /*RunVec=*/false, /*Simplify=*/true);
    if (!FPM.isEmpty())
      MPM->addPass(createModuleToFunctionPassAdaptor(std::move(FPM)));
#if INTEL_CUSTOMIZATION
    // Propagate noalias attribute to function arguments.
    // Run ArgNoAliasProp module pass here to avoid breaking
    // Inliner+CGSCC pipeline later.
    if (EnableArgNoAliasProp && Level.getSpeedupLevel() > 2) {
      // Running ArgumentPromotion & SROA before ArgNoAliasProp will
      // create more noalias attribute propagation opportunities.
      MPM->addPass(createModuleToPostOrderCGSCCPassAdaptor(
                              ArgumentPromotionPass(true)));
      MPM->addPass(createModuleToFunctionPassAdaptor(SROAPass()));
      MPM->addPass(ArgNoAliasPropPass());
    }
#endif // INTEL_CUSTOMIZATION
  }
#endif // INTEL_COLLAB

#if INTEL_CUSTOMIZATION
  // Argument promotion pass was originally added after passes which compute
  // attribues for functions and arguments, but such ordering is not good
  // because argument promotion changes function arguments. As a result
  // promoted arguments do not get any attributes. Reordering argument
  // promotion pass and the passes computing attributes fixes this problem.
  // Additionally adding SROA after the argument promotion to cleanup allocas
  // allows to get more accurate attributes for the promoted arguments.
  if (Level.getSpeedupLevel() > 1) {
    MainCGPipeline.addPass(ArgumentPromotionPass(true));
    MainCGPipeline.addPass(createCGSCCToFunctionPassAdaptor(SROAPass()));
  }
#endif // INTEL_CUSTOMIZATION

  if (AttributorRun & AttributorRunOption::CGSCC)
    MainCGPipeline.addPass(AttributorCGSCCPass());

  // Now deduce any function attributes based in the current code.
  MainCGPipeline.addPass(PostOrderFunctionAttrsPass());

  // Try to perform OpenMP specific optimizations. This is a (quick!) no-op if
  // there are no OpenMP runtime calls present in the module.
  if (Level == OptimizationLevel::O2 || Level == OptimizationLevel::O3)
    MainCGPipeline.addPass(OpenMPOptCGSCCPass());

  for (auto &C : CGSCCOptimizerLateEPCallbacks)
    C(MainCGPipeline, Level);

  // Lastly, add the core function simplification pipeline nested inside the
  // CGSCC walk.
  MainCGPipeline.addPass(createCGSCCToFunctionPassAdaptor(
      buildFunctionSimplificationPipeline(Level, Phase),
      PTO.EagerlyInvalidateAnalyses, EnableNoRerunSimplificationPipeline));

  MainCGPipeline.addPass(CoroSplitPass(Level != OptimizationLevel::O0));

  if (EnableNoRerunSimplificationPipeline)
    MIWP.addLateModulePass(createModuleToFunctionPassAdaptor(
        InvalidateAnalysisPass<ShouldNotRunFunctionPassesAnalysis>()));

  return MIWP;
}

ModulePassManager
PassBuilder::buildModuleInlinerPipeline(OptimizationLevel Level,
                                        ThinOrFullLTOPhase Phase) {
  ModulePassManager MPM;

#if INTEL_CUSTOMIZATION
  InlineParams IP = getInlineParamsFromOptLevel(Level, PrepareForLTO,
     LinkForLTO, SYCLOptimizationMode);
#endif // INTEL_CUSTOMIZATION
  // For PreLinkThinLTO + SamplePGO, set hot-caller threshold to 0 to
  // disable hot callsite inline (as much as possible [1]) because it makes
  // profile annotation in the backend inaccurate.
  //
  // [1] Note the cost of a function could be below zero due to erased
  // prologue / epilogue.
  if (Phase == ThinOrFullLTOPhase::ThinLTOPreLink && PGOOpt &&
      PGOOpt->Action == PGOOptions::SampleUse)
    IP.HotCallSiteThreshold = 0;

  if (PGOOpt)
    IP.EnableDeferral = EnablePGOInlineDeferral;

  // The inline deferral logic is used to avoid losing some
  // inlining chance in future. It is helpful in SCC inliner, in which
  // inlining is processed in bottom-up order.
  // While in module inliner, the inlining order is a priority-based order
  // by default. The inline deferral is unnecessary there. So we disable the
  // inline deferral logic in module inliner.
  IP.EnableDeferral = false;

  MPM.addPass(ModuleInlinerPass(IP, UseInlineAdvisor, Phase));

  MPM.addPass(createModuleToFunctionPassAdaptor(
      buildFunctionSimplificationPipeline(Level, Phase),
      PTO.EagerlyInvalidateAnalyses));

  MPM.addPass(createModuleToPostOrderCGSCCPassAdaptor(
      CoroSplitPass(Level != OptimizationLevel::O0)));

  return MPM;
}

ModulePassManager
PassBuilder::buildModuleSimplificationPipeline(OptimizationLevel Level,
                                               ThinOrFullLTOPhase Phase) {
  ModulePassManager MPM;

  // Place pseudo probe instrumentation as the first pass of the pipeline to
  // minimize the impact of optimization changes.
  if (PGOOpt && PGOOpt->PseudoProbeForProfiling &&
      Phase != ThinOrFullLTOPhase::ThinLTOPostLink)
    MPM.addPass(SampleProfileProbePass(TM));

  bool HasSampleProfile = PGOOpt && (PGOOpt->Action == PGOOptions::SampleUse);

  // In ThinLTO mode, when flattened profile is used, all the available
  // profile information will be annotated in PreLink phase so there is
  // no need to load the profile again in PostLink.
  bool LoadSampleProfile =
      HasSampleProfile &&
      !(FlattenedProfileUsed && Phase == ThinOrFullLTOPhase::ThinLTOPostLink);

#if INTEL_CUSTOMIZATION
  InlineParams IP = getInlineParamsFromOptLevel(Level, PrepareForLTO,
      LinkForLTO, SYCLOptimizationMode);
  if (Phase == ThinOrFullLTOPhase::ThinLTOPreLink && PGOOpt &&
      PGOOpt->Action == PGOOptions::SampleUse)
    IP.HotCallSiteThreshold = 0;
#endif // INTEL_CUSTOMIZATION

  // During the ThinLTO backend phase we perform early indirect call promotion
  // here, before globalopt. Otherwise imported available_externally functions
  // look unreferenced and are removed. If we are going to load the sample
  // profile then defer until later.
  // TODO: See if we can move later and consolidate with the location where
  // we perform ICP when we are loading a sample profile.
  // TODO: We pass HasSampleProfile (whether there was a sample profile file
  // passed to the compile) to the SamplePGO flag of ICP. This is used to
  // determine whether the new direct calls are annotated with prof metadata.
  // Ideally this should be determined from whether the IR is annotated with
  // sample profile, and not whether the a sample profile was provided on the
  // command line. E.g. for flattened profiles where we will not be reloading
  // the sample profile in the ThinLTO backend, we ideally shouldn't have to
  // provide the sample profile file.
  if (Phase == ThinOrFullLTOPhase::ThinLTOPostLink && !LoadSampleProfile)
    MPM.addPass(PGOIndirectCallPromotion(true /* InLTO */, HasSampleProfile));

  // Do basic inference of function attributes from known properties of system
  // libraries and other oracles.
  MPM.addPass(InferFunctionAttrsPass());
#if INTEL_CUSTOMIZATION
  if (RunVPOOpt && RunVPOParopt)
    MPM.addPass(RequireAnalysisPass<VPOParoptConfigAnalysis, Module>());
#endif // INTEL_CUSTOMIZATION

#if INTEL_CUSTOMIZATION
  // Parse -[no]inline-list option and set corresponding attributes.
  InlinerPass InlPass;
  MPM.addPass(InlineReportSetupPass(InlPass.getMDReport()));
  MPM.addPass(InlineListsPass());
  if (RunVPOParopt && EnableVPOParoptTargetInline)
    MPM.addPass(VPOParoptTargetInlinePass());
#if INTEL_FEATURE_SW_DTRANS
  if (PrepareForLTO && DTransEnabled) {
    MPM.addPass(dtrans::DTransForceInlinePass());
    MPM.addPass(dtransOP::DTransForceInlineOPPass());
  }
#endif // INTEL_FEATURE_SW_DTRANS
#endif // INTEL_CUSTOMIZATION
  MPM.addPass(CoroEarlyPass());

  // Create an early function pass manager to cleanup the output of the
  // frontend.
  FunctionPassManager EarlyFPM;
#if INTEL_CUSTOMIZATION
  if (isLoopOptEnabled(Level))
    EarlyFPM.addPass(LoopOptMarkerPass());
  else
    EarlyFPM.addPass(LowerSubscriptIntrinsicPass());
#endif // INTEL_CUSTOMIZATION
#if INTEL_COLLAB
  if (RunVPOOpt && RunVPOParopt)
    addVPOPreparePasses(EarlyFPM);
#endif //INTEL_COLLAB
  // Lower llvm.expect to metadata before attempting transforms.
  // Compare/branch metadata may alter the behavior of passes like SimplifyCFG.
  EarlyFPM.addPass(LowerExpectIntrinsicPass());
  EarlyFPM.addPass(SimplifyCFGPass());
  EarlyFPM.addPass(SROAPass());
#if INTEL_CUSTOMIZATION
#if INTEL_FEATURE_SW_ADVANCED
  if (DTransEnabled && PrepareForLTO)
    EarlyFPM.addPass(FunctionRecognizerPass());
#endif // INTEL_FEATURE_SW_ADVANCED
#endif // INTEL_CUSTOMIZATION
  EarlyFPM.addPass(EarlyCSEPass());
#if INTEL_COLLAB

  // Process OpenMP directives at -O1 and above
  if (RunVPOParopt && RunVPOOpt == InvokeParoptBeforeInliner) {
    // CallSiteSplitting and InstCombine are run after the pre-inliner Paropt
    // in the legacy pass manager. For now we can stick to the same with NPM as
    // well, even though that would break the FPM pipeline here.
    addVPOPasses(MPM, EarlyFPM, Level, /*RunVec=*/false);
  }
#endif // INTEL_COLLAB
  if (Level == OptimizationLevel::O3)
    EarlyFPM.addPass(CallSiteSplittingPass());

  // In SamplePGO ThinLTO backend, we need instcombine before profile annotation
  // to convert bitcast to direct calls so that they can be inlined during the
  // profile annotation prepration step.
  // More details about SamplePGO design can be found in:
  // https://research.google.com/pubs/pub45290.html
  // FIXME: revisit how SampleProfileLoad/Inliner/ICP is structured.
  if (LoadSampleProfile)
    addInstCombinePass(EarlyFPM, !DTransEnabled); // INTEL
  MPM.addPass(createModuleToFunctionPassAdaptor(std::move(EarlyFPM),
                                                PTO.EagerlyInvalidateAnalyses));

  if (LoadSampleProfile) {
    // Annotate sample profile right after early FPM to ensure freshness of
    // the debug info.
    MPM.addPass(SampleProfileLoaderPass(PGOOpt->ProfileFile,
                                        PGOOpt->ProfileRemappingFile, Phase));
    // Cache ProfileSummaryAnalysis once to avoid the potential need to insert
    // RequireAnalysisPass for PSI before subsequent non-module passes.
    MPM.addPass(RequireAnalysisPass<ProfileSummaryAnalysis, Module>());
    // Do not invoke ICP in the LTOPrelink phase as it makes it hard
    // for the profile annotation to be accurate in the LTO backend.
    if (Phase != ThinOrFullLTOPhase::ThinLTOPreLink &&
        Phase != ThinOrFullLTOPhase::FullLTOPreLink)
      // We perform early indirect call promotion here, before globalopt.
      // This is important for the ThinLTO backend phase because otherwise
      // imported available_externally functions look unreferenced and are
      // removed.
      MPM.addPass(
          PGOIndirectCallPromotion(true /* IsInLTO */, true /* SamplePGO */));
  }

  // Try to perform OpenMP specific optimizations on the module. This is a
  // (quick!) no-op if there are no OpenMP runtime calls present in the module.
#if INTEL_COLLAB
  // If OpenMP codegen is to be done by Paropt after the inliner,
  // then OpenMPOpt cannot be run here, before the inliner.
  if (Level != OptimizationLevel::O0 &&
      (!RunVPOParopt || RunVPOOpt == InvokeParoptBeforeInliner))
#else // INTEL_COLLAB
  if (Level != OptimizationLevel::O0)
#endif // INTEL_COLLAB
    MPM.addPass(OpenMPOptPass());

  if (AttributorRun & AttributorRunOption::MODULE)
    MPM.addPass(AttributorPass());

  // Lower type metadata and the type.test intrinsic in the ThinLTO
  // post link pipeline after ICP. This is to enable usage of the type
  // tests in ICP sequences.
  if (Phase == ThinOrFullLTOPhase::ThinLTOPostLink)
    MPM.addPass(LowerTypeTestsPass(nullptr, nullptr, true));

  for (auto &C : PipelineEarlySimplificationEPCallbacks)
    C(MPM, Level);

  // Specialize functions with IPSCCP.
  if (EnableFunctionSpecialization && Level == OptimizationLevel::O3)
    MPM.addPass(FunctionSpecializationPass());

  // Interprocedural constant propagation now that basic cleanup has occurred
  // and prior to optimizing globals.
  // FIXME: This position in the pipeline hasn't been carefully considered in
  // years, it should be re-analyzed.
  MPM.addPass(IPSCCPPass());

  // Attach metadata to indirect call sites indicating the set of functions
  // they may target at run-time. This should follow IPSCCP.
  MPM.addPass(CalledValuePropagationPass());

  // Optimize globals to try and fold them into constants.
  MPM.addPass(GlobalOptPass());

  // Promote any localized globals to SSA registers.
  // FIXME: Should this instead by a run of SROA?
  // FIXME: We should probably run instcombine and simplifycfg afterward to
  // delete control flows that are dead once globals have been folded to
  // constants.
  MPM.addPass(createModuleToFunctionPassAdaptor(PromotePass()));

  // Remove any dead arguments exposed by cleanups and constant folding
  // globals.
  MPM.addPass(DeadArgumentEliminationPass());

  // Create a small function pass pipeline to cleanup after all the global
  // optimizations.
  FunctionPassManager GlobalCleanupPM;
#if INTEL_CUSTOMIZATION
  // Combine silly sequences. Set PreserveAddrCompute to true in LTO phase 1 if
  // IP ArrayTranspose is enabled.
  addInstCombinePass(GlobalCleanupPM, !DTransEnabled);
  // We may consider passing "false" to the AllowCFGSimps arg here,
  // as originally intended. See IPO/PassManagerBuilder.cpp for details.
  if (EarlyJumpThreading && !SYCLOptimizationMode)
    GlobalCleanupPM.addPass(JumpThreadingPass());
#endif // INTEL_CUSTOMIZATION
  invokePeepholeEPCallbacks(GlobalCleanupPM, Level);

  GlobalCleanupPM.addPass(
      SimplifyCFGPass(SimplifyCFGOptions().convertSwitchRangeToICmp(true)));
  MPM.addPass(createModuleToFunctionPassAdaptor(std::move(GlobalCleanupPM),
                                                PTO.EagerlyInvalidateAnalyses));

  // Add all the requested passes for instrumentation PGO, if requested.
  if (PGOOpt && Phase != ThinOrFullLTOPhase::ThinLTOPostLink &&
      (PGOOpt->Action == PGOOptions::IRInstr ||
       PGOOpt->Action == PGOOptions::IRUse)) {
    addPGOInstrPasses(MPM, Level,
                      /* RunProfileGen */ PGOOpt->Action == PGOOptions::IRInstr,
                      /* IsCS */ false, PGOOpt->ProfileFile,
                      PGOOpt->ProfileRemappingFile, Phase);
    MPM.addPass(PGOIndirectCallPromotion(false, false));
  }
  if (PGOOpt && Phase != ThinOrFullLTOPhase::ThinLTOPostLink &&
      PGOOpt->CSAction == PGOOptions::CSIRInstr)
    MPM.addPass(PGOInstrumentationGenCreateVar(PGOOpt->CSProfileGenFile));

  // Synthesize function entry counts for non-PGO compilation.
  if (EnableSyntheticCounts && !PGOOpt)
    MPM.addPass(SyntheticCountsPropagation());

#if INTEL_COLLAB
// FIXME: addVPOPasses needs to be called if EnableModuleInliner is true. This
// needs to be fixed before the flag is made true by default.
#endif // INTEL_COLLAB
  if (EnableModuleInliner)
    MPM.addPass(buildModuleInlinerPipeline(Level, Phase));
  else
#if INTEL_COLLAB
    MPM.addPass(buildInlinerPipeline(Level, Phase, &MPM));
#else // INTEL_COLLAB
    MPM.addPass(buildInlinerPipeline(Level, Phase));
#endif // INTEL_COLLAB
#if INTEL_CUSTOMIZATION

  // If VPO paropt was required to run then do IP constant propagation after
  // promoting pointer arguments to values (when OptLevel > 1) and running
  // simplification passes. That will propagate constant values down to callback
  // functions which represent outlined OpenMP parallel loops where possible.
  if (RunVPOParopt && Level.getSpeedupLevel() > 1)
    MPM.addPass(IPSCCPPass());
#endif // INTEL_CUSTOMIZATION

  MPM.addPass(CoroCleanupPass());

  if (EnableMemProfiler && Phase != ThinOrFullLTOPhase::ThinLTOPreLink) {
    MPM.addPass(createModuleToFunctionPassAdaptor(MemProfilerPass()));
    MPM.addPass(ModuleMemProfilerPass());
  }

  return MPM;
}

/// TODO: Should LTO cause any differences to this set of passes?
void PassBuilder::addVectorPasses(OptimizationLevel Level,
                                  FunctionPassManager &FPM, bool IsFullLTO) {
#if INTEL_CUSTOMIZATION
  // In LTO mode, loopopt runs in link phase along with community vectorizer
  // after it.
  // Call InjectTLIMappings only when the community loop vectorizer or SLP
  // vectorizer is run. Running this can add symbols to @llvm.compile.used,
  // which will inhibit whole program detection in the link step of a -flto
  // compilation.
  if (!PrepareForLTO || !isLoopOptEnabled(Level)) {
    if (EnableLV) {
      FPM.addPass(InjectTLIMappings());
      FPM.addPass(LoopVectorizePass(
          LoopVectorizeOptions(!PTO.LoopInterleaving, !PTO.LoopVectorization)));
    } else if (PTO.SLPVectorization) {
      FPM.addPass(InjectTLIMappings());
    }
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
    if (EnableUnrollAndJam && PTO.LoopUnrolling)
      FPM.addPass(createFunctionToLoopPassAdaptor(
          LoopUnrollAndJamPass(Level.getSpeedupLevel())));
    FPM.addPass(LoopUnrollPass(LoopUnrollOptions(
        Level.getSpeedupLevel(), /*OnlyWhenForced=*/!PTO.LoopUnrolling,
        PTO.ForgetAllSCEVInLoopUnroll)));
    FPM.addPass(WarnMissedTransformationsPass());
  }

  if (!IsFullLTO) {
    // Eliminate loads by forwarding stores from the previous iteration to loads
    // of the current iteration.
    FPM.addPass(LoopLoadEliminationPass());
  }
  // Cleanup after the loop optimization passes.
  addInstCombinePass(FPM, !DTransEnabled); // INTEL

#if INTEL_CUSTOMIZATION
  // In LTO mode, loopopt runs in link phase along with community vectorizer
  // after it.
  if (!PrepareForLTO || !isLoopOptEnabled(Level)) {
#endif // INTEL_CUSTOMIZATION
  if (Level.getSpeedupLevel() > 1 && ExtraVectorizerPasses) {
    ExtraVectorPassManager ExtraPasses;
    // At higher optimization levels, try to clean up any runtime overlap and
    // alignment checks inserted by the vectorizer. We want to track correlated
    // runtime checks for two inner loops in the same outer loop, fold any
    // common computations, hoist loop-invariant aspects out of any outer loop,
    // and unswitch the runtime checks if possible. Once hoisted, we may have
    // dead (or speculatable) control flows or more combining opportunities.
    ExtraPasses.addPass(EarlyCSEPass());
    ExtraPasses.addPass(CorrelatedValuePropagationPass());
    ExtraPasses.addPass(InstCombinePass());
    LoopPassManager LPM;
    LPM.addPass(LICMPass(PTO.LicmMssaOptCap, PTO.LicmMssaNoAccForPromotionCap,
                         /*AllowSpeculation=*/true));
    LPM.addPass(SimpleLoopUnswitchPass(/* NonTrivial */ Level ==
                                       OptimizationLevel::O3));
    ExtraPasses.addPass(
        RequireAnalysisPass<OptimizationRemarkEmitterAnalysis, Function>());
    ExtraPasses.addPass(
        createFunctionToLoopPassAdaptor(std::move(LPM), /*UseMemorySSA=*/true,
                                        /*UseBlockFrequencyInfo=*/true));
    ExtraPasses.addPass(
        SimplifyCFGPass(SimplifyCFGOptions().convertSwitchRangeToICmp(true)));
    ExtraPasses.addPass(InstCombinePass());
    FPM.addPass(std::move(ExtraPasses));
  }

#if INTEL_CUSTOMIZATION
  if (IsFullLTO) {
    // 28038: Avoid excessive hoisting as it increases register pressure and
    // select conversion without clear gains.
    // FPM.addPass(SimplifyCFGPass(SimplifyCFGOptions().hoistCommonInsts(true)));
    FPM.addPass(SimplifyCFGPass());
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
  FPM.addPass(SimplifyCFGPass(SimplifyCFGOptions()
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
    FPM.addPass(SCCPPass());
#if INTEL_CUSTOMIZATION
    // This IC instance must be made loop-aware to avoid sinking expensive
    // insts into loops. Make sure loops were not invalidated by above passes.
    FPM.addPass(RequireAnalysisPass<LoopAnalysis, Function>());
    addInstCombinePass(FPM, !DTransEnabled);
#endif // INTEL_CUSTOMIZATION
    FPM.addPass(BDCEPass());
  }

  // Optimize parallel scalar instruction chains into SIMD instructions.
  if (PTO.SLPVectorization) {
    FPM.addPass(SLPVectorizerPass());
#if INTEL_CUSTOMIZATION
    AfterSLPVectorizer = !PrepareForLTO;
    if (EnableLoadCoalescing)
      FPM.addPass(LoadCoalescingPass());
    if (EnableSROAAfterSLP) {
      // SLP creates opportunities for SROA.
      FPM.addPass(SROAPass());
    }
#endif // INTEL_CUSTOMIZATION
    if (Level.getSpeedupLevel() > 1 && ExtraVectorizerPasses) {
      FPM.addPass(EarlyCSEPass());
    }
  }
  } // INTEL
#if INTEL_CUSTOMIZATION
  if (!IsFullLTO)
    AfterSLPVectorizer = true;
#endif // INTEL_CUSTOMIZATION
  // Enhance/cleanup vector code.
  FPM.addPass(VectorCombinePass());

  if (!IsFullLTO) {
#if INTEL_CUSTOMIZATION
    FPM.addPass(EarlyCSEPass());
    // Combine silly sequences. Set PreserveAddrCompute to true in LTO phase 1
    // if IP ArrayTranspose is enabled.
    addInstCombinePass(FPM, !DTransEnabled);
#endif // INTEL_CUSTOMIZATION
    // Unroll small loops to hide loop backedge latency and saturate any
    // parallel execution resources of an out-of-order processor. We also then
    // need to clean up redundancies and loop invariant code.
    // FIXME: It would be really good to use a loop-integrated instruction
    // combiner for cleanup here so that the unrolling and LICM can be pipelined
    // across the loop nests.
    // We do UnrollAndJam in a separate LPM to ensure it happens before unroll
#if INTEL_CUSTOMIZATION
  // In LTO mode, loopopt runs in link phase along with community unroller
  // after it.
  if (!PrepareForLTO || !isLoopOptEnabled(Level)) {
    // Unroll passes, same as llorg.
    if (EnableUnrollAndJam && PTO.LoopUnrolling) {
      FPM.addPass(createFunctionToLoopPassAdaptor(
          LoopUnrollAndJamPass(Level.getSpeedupLevel())));
    }
    FPM.addPass(LoopUnrollPass(LoopUnrollOptions(
        Level.getSpeedupLevel(), /*OnlyWhenForced=*/!PTO.LoopUnrolling,
        PTO.ForgetAllSCEVInLoopUnroll)));
    // We add SROA here, because unroll may convert GEPs with variable
    // indices to constant indices, which are registerizable.
    FPM.addPass(SROAPass()); // INTEL
  }

#if INTEL_FEATURE_SW_ADVANCED
  // Make unaligned nontemporal stores use a wrapper function instead of
  // scalarizing them.
  if (!PrepareForLTO && isLoopOptEnabled(Level))
    FPM.addPass(NontemporalStorePass());
#endif // INTEL_FEATURE_SW_ADVANCED

  // Postpone warnings to LTO link phase. Most transformations which process
  // user pragmas (like unroller & vectorizer) are triggered in LTO link phase.
  if (!PrepareForLTO)
    FPM.addPass(WarnMissedTransformationsPass());
  // Combine silly sequences. Set PreserveAddrCompute to true in LTO phase 1
  // if IP ArrayTranspose is enabled.
  addInstCombinePass(FPM, !DTransEnabled);
#endif // INTEL_CUSTOMIZATION
    FPM.addPass(
        RequireAnalysisPass<OptimizationRemarkEmitterAnalysis, Function>());
    FPM.addPass(createFunctionToLoopPassAdaptor(
        LICMPass(PTO.LicmMssaOptCap, PTO.LicmMssaNoAccForPromotionCap,
                 /*AllowSpeculation=*/true),
        /*UseMemorySSA=*/true, /*UseBlockFrequencyInfo=*/true));
  }

  // Now that we've vectorized and unrolled loops, we may have more refined
  // alignment information, try to re-derive it here.
  FPM.addPass(AlignmentFromAssumptionsPass());

#if INTEL_CUSTOMIZATION
  if (IsFullLTO) {
#if INTEL_FEATURE_SW_ADVANCED
    // Make unaligned nontemporal stores use a wrapper function instead of
    // scalarizing them.
    if (isLoopOptEnabled(Level))
      FPM.addPass(NontemporalStorePass());
#endif // INTEL_FEATURE_SW_ADVANCED
    addInstCombinePass(FPM, true /* EnableUpCasting */);
  }
#endif // INTEL_CUSTOMIZATOIN
}

#if INTEL_COLLAB

void PassBuilder::addVPOPreparePasses(FunctionPassManager &FPM) {
  FPM.addPass(VPOCFGRestructuringPass());
#if INTEL_CUSTOMIZATION
  // VPOParoptConfig must be applied before any pass that
  // may change its behavior based on the clauses added
  // from the config (e.g. loop collapsing may behave differently
  // due to NUM_TEAMS clause).
  FPM.addPass(VPOParoptApplyConfigPass());
#endif // INTEL_CUSTOMIZATION
  FPM.addPass(VPOParoptLoopTransformPass());
  FPM.addPass(VPOCFGRestructuringPass());
  FPM.addPass(VPOParoptLoopCollapsePass());
  // TODO: maybe we have to make sure loop collapsing preserves
  //       the restructured CFG.
  FPM.addPass(VPOCFGRestructuringPass());
  FPM.addPass(LoopSimplifyUnskippablePass());
  unsigned Mode = RunVPOParopt & (vpo::ParPrepare | vpo::OmpOffload);
  FPM.addPass(VPOParoptPreparePass(Mode));
}

void PassBuilder::addVPOPasses(ModulePassManager &MPM, FunctionPassManager &FPM,
                               OptimizationLevel Level, bool RunVec,
                               bool Simplify) {
  if (!RunVPOParopt)
    return;

  unsigned OptLevel = Level.getSpeedupLevel();

  if (Simplify) {
    // Optimize unnesessary alloca, loads and stores to simplify IR.
    FPM.addPass(SROAPass());

    // Inlining may introduce BasicBlocks without predecessors into an OpenMP
    // region. This breaks CodeExtractor when outlining the region because it
    // expects a single-entry-single-exit region. Calling CFG simplification
    // to remove unreachable BasicBlocks fixes this problem.
#if INTEL_CUSTOMIZATION
    // The inlining issue is documented in CMPLRLLVM-7516. It affects these
    // tests: ompo_kernelsCpp/aobenchan*,ribbon*,terrain*
#endif // INTEL_CUSTOMIZATION
    FPM.addPass(SimplifyCFGPass());
  }

  FPM.addPass(VPORestoreOperandsPass());
  FPM.addPass(VPOCFGRestructuringPass());
#if INTEL_CUSTOMIZATION
  if (OptLevel > 1 && EnableVPOParoptSharedPrivatization) {
    // Shared privatization pass should be combined with the argument
    // promotion pass (to do a cleanup) which currently runs only at O2,
    // therefore it is limited to O2 as well.
    unsigned Mode = RunVPOParopt & vpo::OmpOffload;
    FPM.addPass(VPOParoptSharedPrivatizationPass(Mode));
  }
  FPM.addPass(VPOParoptOptimizeDataSharingPass());
  // No need to rerun VPO CFG restructuring, since
  // VPOParoptOptimizeDataSharing does not modify CFG,
  // and keeps the basic blocks with directive calls
  // consistent.
#endif // INTEL_CUSTOMIZATION
  FPM.addPass(LoopSimplifyUnskippablePass());
  MPM.addPass(createModuleToFunctionPassAdaptor(std::move(FPM)));
  unsigned Mode = RunVPOParopt & (vpo::ParTrans | vpo::OmpPar | vpo::OmpVec |
                                  vpo::OmpTpv | vpo::OmpOffload | vpo::OmpTbb);
  MPM.addPass(VPOParoptPass(Mode));

#if INTEL_CUSTOMIZATION
#if INTEL_FEATURE_CSA
  if (RunCSAGraphSplitter)
    MPM.addPass(CSAGraphSplitterPass());
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
      // Makes sure #pragma omp if clause will be reduced before VPlan pass
      FPM.addPass(VPlanPragmaOmpSimdIfPass());
      // LegacyPM calls an equivalent addFunctionSimplificationPasses,
      // which internally asserts that OptLevel is >= 1. With New PM,
      // the same assertion happens inside buildFunctionSimplificationPipeline.
      if (OptLevel > 0) {
        // FIXME: Check if FullLTOPreLink is the correct enum for the call.
        FPM.addPass(buildFunctionSimplificationPipeline(
            Level, ThinOrFullLTOPhase::FullLTOPreLink));
        if (RunVPOOpt && EnableVPlanDriver && RunPreLoopOptVPOPasses)
          // Run LLVM-IR VPlan vectorizer before loopopt to vectorize all
          // explicit SIMD loops
          addVPlanVectorizer(MPM, FPM, Level);

        addLoopOptPasses(MPM, FPM, Level, /*IsLTO=*/false);
      }

      if (RunVPOOpt && EnableVPlanDriver && RunPostLoopOptVPOPasses) {
        // Run LLVM-IR VPlan vectorizer after loopopt to vectorize all loops not
        // vectorized after createVPlanDriverHIRPass
        if (OptLevel > 0)
          FPM.addPass(createFunctionToLoopPassAdaptor(LoopSimplifyCFGPass()));
        addVPlanVectorizer(MPM, FPM, Level);
      }
    }

    FPM.addPass(VPODirectiveCleanupPass());
  }
#endif // INTEL_CUSTOMIZATION
  // Clean-up empty blocks after OpenMP directives handling.
  FPM.addPass(VPOCFGSimplifyPass());
  if (RunVPOOpt == InvokeParoptAfterInliner) {
#if INTEL_CUSTOMIZATION
    // Paropt transformation pass may produce new AlwaysInline functions.
    // Force inlining for them, if paropt pass runs after the normal inliner.
    // Run it even at -O0, because the only AlwaysInline functions
    // after paropt are the ones that it artificially created.
    // There is some interference with coroutines passes, which
    // insert some AlwaysInline functions early and expect them
    // to exist up to some other coroutine pass - this is rather
    // a problem of coroutine passes implementation that we may
    // inline those functions here. If it becomes a problem,
    // we will have to resolve that issue with coroutines.
    // TODO: This may be redundant since Inliner is also run after Paropt
    // in the new PM. Remove it in the future if it's not needed.
    MPM.addPass(createModuleToFunctionPassAdaptor(std::move(FPM)));
    MPM.addPass(AlwaysInlinerPass(
        /*InsertLifetimeIntrinsics=*/false));

#endif // INTEL_CUSTOMIZATION
    // If Paropt is run after the inliner, we have to delay the OpenMPOpt module
    // pass as well (which otherwise runs before the inliner).
    if (!RunVec && OptLevel > 0) {
#if INTEL_CUSTOMIZATION
#else // INTEL_CUSTOMIZATION
      MPM.addPass(createModuleToFunctionPassAdaptor(std::move(FPM)));
#endif // INTEL_CUSTOMIZATION
      MPM.addPass(OpenMPOptPass());
    }

#if INTEL_CUSTOMIZATION
    // Run GlobalDCE to delete dead functions.
    if (OptLevel > 0)
      MPM.addPass(GlobalDCEPass());
#endif // INTEL_CUSTOMIZATION
  }
}
#endif // INTEL_COLLAB
#if INTEL_CUSTOMIZATION

void PassBuilder::addVPlanVectorizer(ModulePassManager &MPM,
                                     FunctionPassManager &FPM,
                                     OptimizationLevel Level) {
  unsigned OptLevel = Level.getSpeedupLevel();

  if (OptLevel > 0) {
    FPM.addPass(LowerSwitchPass(true /*Only for SIMD loops*/));
    // Add LCSSA pass before VPlan driver
    FPM.addPass(LCSSAPass());
  }

  FPM.addPass(VPOCFGRestructuringPass());
  // VPO CFG restructuring pass makes sure that the directives of #pragma omp
  // simd ordered are in a separate block. For this reason,
  // VPlanPragmaOmpOrderedSimdExtract pass should run after VPO CFG
  // Restructuring.

  // Before proceeding, consume the existing FPM pipeline before resetting
  // the pass manager, since next pass is a module pass.
  MPM.addPass(createModuleToFunctionPassAdaptor(std::move(FPM)));

  // FIXME: We should try to avoid breaking the FPM pipeline by making
  // VPlanPragmaOmpOrderedSimdExtractPass a Function pass.
  MPM.addPass(VPlanPragmaOmpOrderedSimdExtractPass());

  // Code extractor might add new instructions in the entry block. If the entry
  // block has a directive, than we have to split the entry block. VPlan assumes
  // that the directives are in single-entry single-exit basic blocks.
  FPM.addPass(VPOCFGRestructuringPass());

  // Create OCL sincos from sin/cos and sincos
  if (OptLevel > 0)
    FPM.addPass(MathLibraryFunctionsReplacementPass(false /*isOCL*/));

  FPM.addPass(vpo::VPlanDriverPass());

  // Split/translate scalar OCL and vector sincos
  if (OptLevel > 0)
    FPM.addPass(MathLibraryFunctionsReplacementPass(false /*isOCL*/));

  // Consume the function pass manager FPM1 before adding Module passes.
  MPM.addPass(createModuleToFunctionPassAdaptor(std::move(FPM)));

  // The region that is outlined by #pragma omp simd ordered was extracted by
  // VPlanPragmaOmpOrderedSimdExtarct pass. Now, we need to run the inliner in
  // order to put this region back at the code.
  MPM.addPass(AlwaysInlinerPass(
      /*InsertLifetimeIntrinsics=*/false));

  // Clean up any SIMD directives left behind by VPlan vectorizer
  if (OptLevel > 0)
    FPM.addPass(VPODirectiveCleanupPass());
}

void PassBuilder::addLoopOptPasses(ModulePassManager &MPM,
                                   FunctionPassManager &FPM,
                                   OptimizationLevel Level, bool IsLTO) {
  if (!isLoopOptEnabled(Level))
    return;

  if (IsLTO && RunLoopOpts == LoopOptMode::Full) {
    FPM.addPass(SimplifyCFGPass());
    FPM.addPass(ADCEPass());
  }

  FPM.addPass(createFunctionToLoopPassAdaptor(
      LoopSimplifyCFGPass(), /*UseMemorySSA=*/false,
      /*UseBlockFrequencyInfo=*/false));

  FPM.addPass(LCSSAPass());
  // Leaving comments for stuff which need to be added to match legacy pass
  // manager.
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  if (shouldPrintModuleBeforeLoopopt()) {
    // We need to 'flush' current function pipeline to MPM to get latest module dump with the printer.
    MPM.addPass(createModuleToFunctionPassAdaptor(std::move(FPM)));
    MPM.addPass(PrintModulePass(dbgs(), ";Module Before HIR"));
  }
#endif //! defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

  if (EnableVPlanDriverHIR) {
    FPM.addPass(VPOCFGRestructuringPass());
    MPM.addPass(createModuleToFunctionPassAdaptor(std::move(FPM)));
    MPM.addPass(VPlanPragmaOmpOrderedSimdExtractPass());
    FPM.addPass(VPlanPragmaOmpSimdIfPass());
  }

  if (ConvertToSubs)
    FPM.addPass(ConvertGEPToSubscriptIntrinsicPass());

  FPM.addPass(HIRSSADeconstructionPass());
  FPM.addPass(HIRTempCleanupPass());

  if (!RunLoopOptFrameworkOnly) {
    // if (vpo::UseOmpRegionsInLoopoptFlag)
    //   FPM.addPass(HIRRecognizeParLoopPass());

    FPM.addPass(HIRPropagateCastedIVPass());

    if (Level.getSpeedupLevel() > 2) {
      if (RunLoopOpts == LoopOptMode::Full) {
        FPM.addPass(HIRLoopConcatenationPass());
        FPM.addPass(HIRPMSymbolicTripCountCompleteUnrollPass());
      }
      FPM.addPass(HIRArrayTransposePass());
    }

    if (Level.getSizeLevel() == 0) {
      // if (RunVPOOpt)
      // FPM.add(createHIRParDirInsertPass());
      FPM.addPass(HIRConditionalTempSinkingPass());
      FPM.addPass(HIROptPredicatePass(Level.getSpeedupLevel() == 3, true));

      if (RunLoopOpts == LoopOptMode::Full) {
        if (Level.getSpeedupLevel() > 2) {
          FPM.addPass(HIRLMMPass(true));
          FPM.addPass(HIRStoreResultIntoTempArrayPass());
        }
        FPM.addPass(HIRAosToSoaPass());
      } // END LoopOptMode::Full

      FPM.addPass(HIRRuntimeDDPass());
      FPM.addPass(HIRMVForConstUBPass());

      if (RunLoopOpts == LoopOptMode::Full && Level.getSpeedupLevel() > 2 &&
          IsLTO) {
        FPM.addPass(HIRRowWiseMVPass());
        FPM.addPass(HIRSumWindowReusePass());
      }
    }

    FPM.addPass(HIRSinkingForPerfectLoopnestPass());
    FPM.addPass(HIRNonZeroSinkingForPerfectLoopnestPass());
    FPM.addPass(HIRPragmaLoopBlockingPass());

    if (RunLoopOpts == LoopOptMode::Full) {
      FPM.addPass(HIRLoopDistributionForLoopNestPass());

#if INTEL_FEATURE_SW_ADVANCED
      if (Level.getSpeedupLevel() > 2 && IsLTO)
        FPM.addPass(HIRCrossLoopArrayContractionPass(
            ThroughputModeOpt != ThroughputMode::SingleJob));
#endif // INTEL_FEATURE_SW_ADVANCED
      FPM.addPass(HIRLoopInterchangePass());
    } // END LoopOptMode::Full

    FPM.addPass(HIRGenerateMKLCallPass());

    if (RunLoopOpts == LoopOptMode::Full) {
#if INTEL_FEATURE_SW_ADVANCED
      if (Level.getSpeedupLevel() > 2 && IsLTO)
        FPM.addPass(HIRInterLoopBlockingPass());
#endif // INTEL_FEATURE_SW_ADVANCED

      FPM.addPass(
          HIRLoopBlockingPass(ThroughputModeOpt != ThroughputMode::SingleJob));
    } // END LoopOptMode::Full

    FPM.addPass(HIRUndoSinkingForPerfectLoopnestPass());
    FPM.addPass(HIRDeadStoreEliminationPass());
    FPM.addPass(HIRMinMaxRecognitionPass());
    FPM.addPass(HIRIdentityMatrixIdiomRecognitionPass());

    if (Level.getSizeLevel() == 0)
      FPM.addPass(HIRPreVecCompleteUnrollPass(Level.getSpeedupLevel(),
                                              !PTO.LoopUnrolling));

    if (ThroughputModeOpt != ThroughputMode::SingleJob)
      FPM.addPass(HIRConditionalLoadStoreMotionPass());

    if (Level.getSizeLevel() == 0)
      FPM.addPass(HIRMemoryReductionSinkingPass());

    FPM.addPass(HIRLMMPass());
    FPM.addPass(HIRDeadStoreEliminationPass());
    FPM.addPass(HIRLastValueComputationPass());

    FPM.addPass(HIRLoopRerollPass());

    if (RunLoopOpts == LoopOptMode::Full && Level.getSizeLevel() == 0)
      FPM.addPass(HIRLoopDistributionForMemRecPass());

    FPM.addPass(HIRLoopReversalPass());
    FPM.addPass(HIRLoopRematerializePass());
    FPM.addPass(HIRMultiExitLoopRerollPass());
    FPM.addPass(HIRLoopCollapsePass());
    FPM.addPass(HIRIdiomRecognitionPass());
    FPM.addPass(HIRLoopFusionPass());
    FPM.addPass(HIRIfReversalPass());

    if (Level.getSizeLevel() == 0) {
      if (RunLoopOpts == LoopOptMode::Full) {
        FPM.addPass(HIRUnrollAndJamPass(!PTO.LoopUnrolling));
        FPM.addPass(HIRMVForVariableStridePass());
      }

      FPM.addPass(HIROptVarPredicatePass());
      FPM.addPass(HIROptPredicatePass(Level.getSpeedupLevel() == 3, false));
      FPM.addPass(HIRLMMPass());
      if (RunVPOOpt) {
        FPM.addPass(HIRVecDirInsertPass(Level.getSpeedupLevel() == 3));
        if (EnableVPlanDriverHIR) {
          FPM.addPass(vpo::VPlanDriverHIRPass(
            RunLoopOpts == LoopOptMode::LightWeight));
        }
      }
      FPM.addPass(HIRPostVecCompleteUnrollPass(Level.getSpeedupLevel(),
                                               !PTO.LoopUnrolling));
      FPM.addPass(HIRGeneralUnrollPass(!PTO.LoopUnrolling));
    }

    FPM.addPass(HIRScalarReplArrayPass());

    if (RunLoopOpts == LoopOptMode::Full) {
      if (Level.getSpeedupLevel() > 2) {
        if (ThroughputModeOpt != ThroughputMode::SingleJob)
          FPM.addPass(HIRNontemporalMarkingPass());

        FPM.addPass(HIRPrefetchingPass());
      }
    }

  } // RunLoopOptFrameworkOnly

  if (IntelOptReportEmitter == OptReportOptions::HIR)
    FPM.addPass(HIROptReportEmitterPass());

  FPM.addPass(HIRCodeGenPass());

  addLoopOptCleanupPasses(FPM, Level);

  // if (EnableVPlanDriverHIR) {
  //  PM.add(createAlwaysInlinerLegacyPass());
  //  PM.add(createBarrierNoopPass());
  // }
}

void PassBuilder::addLoopOptCleanupPasses(FunctionPassManager &FPM,
                                          OptimizationLevel Level) {

  FPM.addPass(SimplifyCFGPass());
  FPM.addPass(LowerSubscriptIntrinsicPass());
  FPM.addPass(SROAPass());

  if (Level.getSpeedupLevel() > 2)
    FPM.addPass(NaryReassociatePass());

  FPM.addPass(GVNPass());

  FPM.addPass(SROAPass());

  addInstCombinePass(FPM, !DTransEnabled);

  FPM.addPass(LoopCarriedCSEPass());

  FPM.addPass(DSEPass());

  if (Level.getSpeedupLevel() > 2)
    FPM.addPass(AddSubReassociatePass());
}

void PassBuilder::addLoopOptAndAssociatedVPOPasses(ModulePassManager &MPM,
                                                   FunctionPassManager &FPM,
                                                   OptimizationLevel Level,
                                                   bool IsLTO) {
  // Do not run loop optimization passes, if proprietary optimizations
  // are disabled (for instance during spir64 compilation for OpenMP offload).
  // There are some mandatory clean-up actions that still need/ to be performed.
  if (PTO.DisableIntelProprietaryOpts) {
    // CMPLRLLVM-25935: clean-up VPO directives for targets with
    //                  LLVM IR emission enabled (hence, with proprietary
    //                  optimizations disabled).
    FPM.addPass(VPODirectiveCleanupPass());
    return;
  }

  if (RunVPOOpt && RunVecClone) {
    if (!FPM.isEmpty())
      MPM.addPass(createModuleToFunctionPassAdaptor(std::move(FPM)));
    MPM.addPass(VecClonePass());
    // VecClonePass can generate redundant geps/loads for vector parameters when
    // accessing elem[i] within the inserted simd loop. This makes DD testing
    // harder, so run CSE here to do some clean-up before HIR construction.
    FPM.addPass(EarlyCSEPass());
  }

  // Enable VPlanPragmaOmpSimdIfPass pass only in case VPlan pass will be
  // enabled after that
  if (RunVPOOpt && EnableVPlanDriver)
    // Makes sure #pragma omp if clause will be reduced before VPlan pass
    FPM.addPass(VPlanPragmaOmpSimdIfPass());

  if (RunVPOOpt && EnableVPlanDriver && RunPreLoopOptVPOPasses) {
    // Run LLVM-IR VPlan vectorizer before loopopt to vectorize all explicit
    // SIMD loops
    addVPlanVectorizer(MPM, FPM, Level);
  }

  addLoopOptPasses(MPM, FPM, Level, IsLTO);

  if (RunVPOOpt && EnableVPlanDriver && RunPostLoopOptVPOPasses) {
    if (Level.getSpeedupLevel() > 0)
      FPM.addPass(LoopSimplifyPass());
    // Run LLVM-IR VPlan vectorizer after loopopt to vectorize all loops not
    // vectorized after createVPlanDriverHIRPass
    addVPlanVectorizer(MPM, FPM, Level);
  }

  // Process directives inserted by LoopOpt Autopar.
  // Call with RunVec==true (2nd argument) to cleanup any vec directives
  // that loopopt and vectorizer might have missed.
  if (RunVPOOpt)
    addVPOPasses(MPM, FPM, Level, /*RunVec=*/true, /*Simplify=*/true);

  if (IntelOptReportEmitter == OptReportOptions::IR)
    FPM.addPass(OptReportEmitterPass());
}

#endif // INTEL_CUSTOMIZATION

ModulePassManager
PassBuilder::buildModuleOptimizationPipeline(OptimizationLevel Level,
                                             ThinOrFullLTOPhase LTOPhase) {
  const bool LTOPreLink = (LTOPhase == ThinOrFullLTOPhase::ThinLTOPreLink ||
                           LTOPhase == ThinOrFullLTOPhase::FullLTOPreLink);
  ModulePassManager MPM;

  // Optimize globals now that the module is fully simplified.
  MPM.addPass(GlobalOptPass());
  MPM.addPass(GlobalDCEPass());

  // Run partial inlining pass to partially inline functions that have
  // large bodies.
  if (RunPartialInlining)
    MPM.addPass(PartialInlinerPass());

#if INTEL_CUSTOMIZATION
  // Propagate noalias attribute to function arguments.
  if (EnableArgNoAliasProp && Level.getSpeedupLevel() > 2)
    MPM.addPass(ArgNoAliasPropPass());

  FunctionPassManager FakeLoadFPM;
  if (EnableStdContainerOpt)
    FakeLoadFPM.addPass(StdContainerOptPass());
  FakeLoadFPM.addPass(CleanupFakeLoadsPass());
  MPM.addPass(createModuleToFunctionPassAdaptor(std::move(FakeLoadFPM)));
#endif // INTEL_CUSTOMIZATION

  // Remove avail extern fns and globals definitions since we aren't compiling
  // an object file for later LTO. For LTO we want to preserve these so they
  // are eligible for inlining at link-time. Note if they are unreferenced they
  // will be removed by GlobalDCE later, so this only impacts referenced
  // available externally globals. Eventually they will be suppressed during
  // codegen, but eliminating here enables more opportunity for GlobalDCE as it
  // may make globals referenced by available external functions dead and saves
  // running remaining passes on the eliminated functions. These should be
  // preserved during prelinking for link-time inlining decisions.
  if (!LTOPreLink)
    MPM.addPass(EliminateAvailableExternallyPass());

  if (EnableOrderFileInstrumentation)
    MPM.addPass(InstrOrderFilePass());

  // Do RPO function attribute inference across the module to forward-propagate
  // attributes where applicable.
  // FIXME: Is this really an optimization rather than a canonicalization?
  MPM.addPass(ReversePostOrderFunctionAttrsPass());

  // Do a post inline PGO instrumentation and use pass. This is a context
  // sensitive PGO pass. We don't want to do this in LTOPreLink phrase as
  // cross-module inline has not been done yet. The context sensitive
  // instrumentation is after all the inlines are done.
  if (!LTOPreLink && PGOOpt) {
    if (PGOOpt->CSAction == PGOOptions::CSIRInstr)
      addPGOInstrPasses(MPM, Level, /* RunProfileGen */ true,
                        /* IsCS */ true, PGOOpt->CSProfileGenFile,
                        PGOOpt->ProfileRemappingFile, LTOPhase);
    else if (PGOOpt->CSAction == PGOOptions::CSIRUse)
      addPGOInstrPasses(MPM, Level, /* RunProfileGen */ false,
                        /* IsCS */ true, PGOOpt->ProfileFile,
                        PGOOpt->ProfileRemappingFile, LTOPhase);
  }

#if INTEL_CUSTOMIZATION
  if (!PTO.DisableIntelProprietaryOpts && PTO.EnableAutoCPUDispatch &&
      Level.getSpeedupLevel() > 1)
    MPM.addPass(AutoCPUClonePass());

  if (EnableAndersen) {
    // Andersen's IP alias analysis
    MPM.addPass(RequireAnalysisPass<AndersensAA, Module>());
    // Global var opt that is outside LTO. Can still run with LTO.
    if (EnableNonLTOGlobalVarOpt && Level.getSpeedupLevel() > 1) {
      FunctionPassManager GlobalOptFPM;
      GlobalOptFPM.addPass(NonLTOGlobalOptPass());
      GlobalOptFPM.addPass(PromotePass());
      // ADCE avoids a regression in aifftr01@opt_speed.
      GlobalOptFPM.addPass(ADCEPass());
      MPM.addPass(createModuleToFunctionPassAdaptor(std::move(GlobalOptFPM)));
    }
  }
#endif // INTEL_CUSTOMIZATION
  // Re-compute GlobalsAA here prior to function passes. This is particularly
  // useful as the above will have inlined, DCE'ed, and function-attr
  // propagated everything. We should at this point have a reasonably minimal
  // and richly annotated call graph. By computing aliasing and mod/ref
  // information for all local globals here, the late loop passes and notably
  // the vectorizer will be able to use them to help recognize vectorizable
  // memory operations.
  MPM.addPass(RecomputeGlobalsAAPass());

  for (auto &C : OptimizerEarlyEPCallbacks)
    C(MPM, Level);

  FunctionPassManager OptimizePM;
  OptimizePM.addPass(Float2IntPass());
  OptimizePM.addPass(LowerConstantIntrinsicsPass());

  if (EnableMatrix) {
    OptimizePM.addPass(LowerMatrixIntrinsicsPass());
    OptimizePM.addPass(EarlyCSEPass());
  }

  // FIXME: We need to run some loop optimizations to re-rotate loops after
  // simplifycfg and others undo their rotation.

  // Optimize the loop execution. These passes operate on entire loop nests
  // rather than on each loop in an inside-out manner, and so they are actually
  // function passes.

  for (auto &C : VectorizerStartEPCallbacks)
    C(OptimizePM, Level);

  if (!SYCLOptimizationMode) {
    LoopPassManager LPM;
    // First rotate loops that may have been un-rotated by prior passes.
    // Disable header duplication at -Oz.
    LPM.addPass(LoopRotatePass(Level != OptimizationLevel::Oz, LTOPreLink));
    // Some loops may have become dead by now. Try to delete them.
    // FIXME: see discussion in https://reviews.llvm.org/D112851,
    //        this may need to be revisited once we run GVN before loop deletion
    //        in the simplification pipeline.
    LPM.addPass(LoopDeletionPass());
    OptimizePM.addPass(
        createFunctionToLoopPassAdaptor(std::move(LPM), /*UseMemorySSA=*/false,
                                        /*UseBlockFrequencyInfo=*/false));
#if INTEL_CUSTOMIZATION
    if (!PrepareForLTO)
      addLoopOptAndAssociatedVPOPasses(MPM, OptimizePM, Level, false);
#endif // INTEL_CUSTOMIZATION

    // Distribute loops to allow partial vectorization. I.e. isolate dependences
    // into separate loop that would otherwise inhibit vectorization. This is
    // currently only performed for loops marked with the metadata
    // llvm.loop.distribute=true or when -enable-loop-distribute is specified.
    OptimizePM.addPass(LoopDistributePass());

    // Populates the VFABI attribute with the scalar-to-vector mappings
    // from the TargetLibraryInfo.
#if INTEL_CUSTOMIZATION
    // Adding InjectTLIMappings is sunken into addVectorPasses
#endif // INTEL_CUSTOMIZATION

    addVectorPasses(Level, OptimizePM, /* IsFullLTO */ false);
  }

  // LoopSink pass sinks instructions hoisted by LICM, which serves as a
  // canonicalization pass that enables other optimizations. As a result,
  // LoopSink pass needs to be a very late IR pass to avoid undoing LICM
  // result too early.
  OptimizePM.addPass(LoopSinkPass());

  // And finally clean up LCSSA form before generating code.
  OptimizePM.addPass(InstSimplifyPass());

  // This hoists/decomposes div/rem ops. It should run after other sink/hoist
  // passes to avoid re-sinking, but before SimplifyCFG because it can allow
  // flattening of blocks.
  OptimizePM.addPass(DivRemPairsPass());

  // LoopSink (and other loop passes since the last simplifyCFG) might have
  // resulted in single-entry-single-exit or empty blocks. Clean up the CFG.
  OptimizePM.addPass(
      SimplifyCFGPass(SimplifyCFGOptions().convertSwitchRangeToICmp(true)));

  // Add the core optimizing pipeline.
  MPM.addPass(createModuleToFunctionPassAdaptor(std::move(OptimizePM),
                                                PTO.EagerlyInvalidateAnalyses));

  for (auto &C : OptimizerLastEPCallbacks)
    C(MPM, Level);

  // Split out cold code. Splitting is done late to avoid hiding context from
  // other optimizations and inadvertently regressing performance. The tradeoff
  // is that this has a higher code size cost than splitting early.
  if (EnableHotColdSplit && !LTOPreLink)
    MPM.addPass(HotColdSplittingPass());

  // Search the code for similar regions of code. If enough similar regions can
  // be found where extracting the regions into their own function will decrease
  // the size of the program, we extract the regions, a deduplicate the
  // structurally similar regions.
  if (EnableIROutliner)
    MPM.addPass(IROutlinerPass());

  // Merge functions if requested.
  if (PTO.MergeFunctions)
    MPM.addPass(MergeFunctionsPass());

  // Now we need to do some global optimization transforms.
  // FIXME: It would seem like these should come first in the optimization
  // pipeline and maybe be the bottom of the canonicalization pipeline? Weird
  // ordering here.
  MPM.addPass(GlobalDCEPass());
  MPM.addPass(ConstantMergePass());

  if (PTO.CallGraphProfile && !LTOPreLink)
    MPM.addPass(CGProfilePass());

  // TODO: Relative look table converter pass caused an issue when full lto is
  // enabled. See https://reviews.llvm.org/D94355 for more details.
  // Until the issue fixed, disable this pass during pre-linking phase.
  if (!LTOPreLink)
    MPM.addPass(RelLookupTableConverterPass());

#if INTEL_CUSTOMIZATION
  // If LTO is enabled, then check if the declaration for math libraries are
  // needed
  if (PrepareForLTO)
    MPM.addPass(IntelMathLibrariesDeclarationPass());
  MPM.addPass(InlineReportEmitterPass(Level.getSpeedupLevel(),
                                      Level.getSizeLevel(), LTOPreLink));
#endif // INTEL_CUSTOMIZATION

  return MPM;
}

ModulePassManager
PassBuilder::buildPerModuleDefaultPipeline(OptimizationLevel Level,
                                           bool LTOPreLink) {
  assert(Level != OptimizationLevel::O0 &&
         "Must request optimizations for the default pipeline!");

  ModulePassManager MPM;
#if INTEL_CUSTOMIZATION
  MPM.addPass(XmainOptLevelAnalysisInit(Level.getSpeedupLevel()));
#endif // INTEL_CUSTOMIZATION

  // Convert @llvm.global.annotations to !annotation metadata.
  MPM.addPass(Annotation2MetadataPass());

  // Force any function attributes we want the rest of the pipeline to observe.
  MPM.addPass(ForceFunctionAttrsPass());

  // Apply module pipeline start EP callback.
  for (auto &C : PipelineStartEPCallbacks)
    C(MPM, Level);

  if (PGOOpt && PGOOpt->DebugInfoForProfiling)
    MPM.addPass(createModuleToFunctionPassAdaptor(AddDiscriminatorsPass()));

  const ThinOrFullLTOPhase LTOPhase = LTOPreLink
                                          ? ThinOrFullLTOPhase::FullLTOPreLink
                                          : ThinOrFullLTOPhase::None;
  // Add the core simplification pipeline.
  MPM.addPass(buildModuleSimplificationPipeline(Level, LTOPhase));

  // Now add the optimization pipeline.
  MPM.addPass(buildModuleOptimizationPipeline(Level, LTOPhase));

  if (PGOOpt && PGOOpt->PseudoProbeForProfiling &&
      PGOOpt->Action == PGOOptions::SampleUse)
    MPM.addPass(PseudoProbeUpdatePass());

  // Emit annotation remarks.
  addAnnotationRemarksPass(MPM);

  if (LTOPreLink)
    addRequiredLTOPreLinkPasses(MPM);

  return MPM;
}

ModulePassManager
PassBuilder::buildThinLTOPreLinkDefaultPipeline(OptimizationLevel Level) {
  assert(Level != OptimizationLevel::O0 &&
         "Must request optimizations for the default pipeline!");

  ModulePassManager MPM;

  // Convert @llvm.global.annotations to !annotation metadata.
  MPM.addPass(Annotation2MetadataPass());

  // Force any function attributes we want the rest of the pipeline to observe.
  MPM.addPass(ForceFunctionAttrsPass());

  if (PGOOpt && PGOOpt->DebugInfoForProfiling)
    MPM.addPass(createModuleToFunctionPassAdaptor(AddDiscriminatorsPass()));

  // Apply module pipeline start EP callback.
  for (auto &C : PipelineStartEPCallbacks)
    C(MPM, Level);

  // If we are planning to perform ThinLTO later, we don't bloat the code with
  // unrolling/vectorization/... now. Just simplify the module as much as we
  // can.
  MPM.addPass(buildModuleSimplificationPipeline(
      Level, ThinOrFullLTOPhase::ThinLTOPreLink));

  // Run partial inlining pass to partially inline functions that have
  // large bodies.
  // FIXME: It isn't clear whether this is really the right place to run this
  // in ThinLTO. Because there is another canonicalization and simplification
  // phase that will run after the thin link, running this here ends up with
  // less information than will be available later and it may grow functions in
  // ways that aren't beneficial.
  if (RunPartialInlining)
    MPM.addPass(PartialInlinerPass());

  // Reduce the size of the IR as much as possible.
  MPM.addPass(GlobalOptPass());

  if (PGOOpt && PGOOpt->PseudoProbeForProfiling &&
      PGOOpt->Action == PGOOptions::SampleUse)
    MPM.addPass(PseudoProbeUpdatePass());

  // Handle OptimizerLastEPCallbacks added by clang on PreLink. Actual
  // optimization is going to be done in PostLink stage, but clang can't
  // add callbacks there in case of in-process ThinLTO called by linker.
  for (auto &C : OptimizerLastEPCallbacks)
    C(MPM, Level);

  // Emit annotation remarks.
  addAnnotationRemarksPass(MPM);

  addRequiredLTOPreLinkPasses(MPM);

  return MPM;
}

ModulePassManager PassBuilder::buildThinLTODefaultPipeline(
    OptimizationLevel Level, const ModuleSummaryIndex *ImportSummary) {
  ModulePassManager MPM;

  // Convert @llvm.global.annotations to !annotation metadata.
  MPM.addPass(Annotation2MetadataPass());

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
    //
    // The WPD and LowerTypeTest passes need to run at -O0 to lower type
    // metadata and intrinsics.
    MPM.addPass(WholeProgramDevirtPass(nullptr, ImportSummary));
    MPM.addPass(LowerTypeTestsPass(nullptr, ImportSummary));
  }

  if (Level == OptimizationLevel::O0) {
    // Run a second time to clean up any type tests left behind by WPD for use
    // in ICP.
    MPM.addPass(LowerTypeTestsPass(nullptr, nullptr, true));
    // Drop available_externally and unreferenced globals. This is necessary
    // with ThinLTO in order to avoid leaving undefined references to dead
    // globals in the object file.
    MPM.addPass(EliminateAvailableExternallyPass());
    MPM.addPass(GlobalDCEPass());
    return MPM;
  }

  // Force any function attributes we want the rest of the pipeline to observe.
  MPM.addPass(ForceFunctionAttrsPass());

  // Add the core simplification pipeline.
  MPM.addPass(buildModuleSimplificationPipeline(
      Level, ThinOrFullLTOPhase::ThinLTOPostLink));

  // Now add the optimization pipeline.
  MPM.addPass(buildModuleOptimizationPipeline(
      Level, ThinOrFullLTOPhase::ThinLTOPostLink));

  // Emit annotation remarks.
  addAnnotationRemarksPass(MPM);

  return MPM;
}

ModulePassManager
PassBuilder::buildLTOPreLinkDefaultPipeline(OptimizationLevel Level) {
  assert(Level != OptimizationLevel::O0 &&
         "Must request optimizations for the default pipeline!");
  // FIXME: We should use a customized pre-link pipeline!
#if INTEL_CUSTOMIZATION
  PrepareForLTO = true;
  auto Guard = llvm::make_scope_exit([&]() { PrepareForLTO = false; });
#endif // INTEL_CUSTOMIZATION
  return buildPerModuleDefaultPipeline(Level,
                                       /* LTOPreLink */ true);
}

ModulePassManager
PassBuilder::buildLTODefaultPipeline(OptimizationLevel Level,
                                     ModuleSummaryIndex *ExportSummary) {
  ModulePassManager MPM;

#if INTEL_CUSTOMIZATION
  LinkForLTO = true;
#endif // INTEL_CUSTOMIZATION
  // Convert @llvm.global.annotations to !annotation metadata.
  MPM.addPass(Annotation2MetadataPass());

  for (auto &C : FullLinkTimeOptimizationEarlyEPCallbacks)
    C(MPM, Level);

  // Create a function that performs CFI checks for cross-DSO calls with targets
  // in the current module.
  MPM.addPass(CrossDSOCFIPass());

  if (Level == OptimizationLevel::O0) {
#if INTEL_CUSTOMIZATION
    if (EnableWPA) {
      // Set the optimization level
      MPM.addPass(XmainOptLevelAnalysisInit(Level.getSpeedupLevel()));
      MPM.addPass(RequireAnalysisPass<WholeProgramAnalysis, Module>());
#if INTEL_FEATURE_SW_DTRANS
      MPM.addPass(IntelFoldWPIntrinsicPass());
#endif // INTEL_FEATURE_SW_DTRANS
    }
#endif // INTEL_CUSTOMIZATION
    // The WPD and LowerTypeTest passes need to run at -O0 to lower type
    // metadata and intrinsics.
    MPM.addPass(WholeProgramDevirtPass(ExportSummary, nullptr));
    MPM.addPass(LowerTypeTestsPass(ExportSummary, nullptr));
    // Run a second time to clean up any type tests left behind by WPD for use
    // in ICP.
    MPM.addPass(LowerTypeTestsPass(nullptr, nullptr, true));

    for (auto &C : FullLinkTimeOptimizationLastEPCallbacks)
      C(MPM, Level);

    // Emit annotation remarks.
    addAnnotationRemarksPass(MPM);

    return MPM;
  }

#if INTEL_CUSTOMIZATION
  InlinerPass InlPass;
  MPM.addPass(InlineReportSetupPass(InlPass.getMDReport()));
#endif // INTEL_CUSTOMIZATION
  if (PGOOpt && PGOOpt->Action == PGOOptions::SampleUse) {
    // Load sample profile before running the LTO optimization pipeline.
    MPM.addPass(SampleProfileLoaderPass(PGOOpt->ProfileFile,
                                        PGOOpt->ProfileRemappingFile,
                                        ThinOrFullLTOPhase::FullLTOPostLink));
    // Cache ProfileSummaryAnalysis once to avoid the potential need to insert
    // RequireAnalysisPass for PSI before subsequent non-module passes.
    MPM.addPass(RequireAnalysisPass<ProfileSummaryAnalysis, Module>());
  }

#if INTEL_CUSTOMIZATION
  // Set the optimization level
  MPM.addPass(XmainOptLevelAnalysisInit(Level.getSpeedupLevel()));
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
      MPM.addPass(InternalizePass(PreserveSymbol));
    }
    MPM.addPass(RequireAnalysisPass<WholeProgramAnalysis, Module>());
  }
#endif // INTEL_CUSTOMIZATION

  // Try to run OpenMP optimizations, quick no-op if no OpenMP metadata present.
  MPM.addPass(OpenMPOptPass());

  // Remove unused virtual tables to improve the quality of code generated by
  // whole-program devirtualization and bitset lowering.
  MPM.addPass(GlobalDCEPass());

#if INTEL_CUSTOMIZATION
#if INTEL_FEATURE_SW_ADVANCED
  // IPO-based prefetch
  if (EnableIPOPrefetch)
    MPM.addPass(IntelIPOPrefetchPass());
#endif // INTEL_FEATURE_SW_ADVANCED

#if INTEL_FEATURE_SW_DTRANS
  if (EnableWPA)
    MPM.addPass(IntelFoldWPIntrinsicPass());
#endif // INTEL_FEATURE_SW_DTRANS

#if INTEL_FEATURE_SW_ADVANCED
  if (EnableIPCloning) {
    // This pass is being added under DTRANS only at this point, because a
    // particular benchmark needs it to prove that the period of a recursive
    // progression is constant. We can remove the test for DTransEnabled if
    // we find IPSCCP to be generally useful here and we are willing to
    // tolerate the additional compile time.
    if (DTransEnabled)
      MPM.addPass(IPSCCPPass());
    MPM.addPass(IPCloningPass(/*AfterInl*/ false,
                              /*IFSwitchHeuristic*/ true));
  }
#endif // INTEL_FEATURE_SW_ADVANCED
#endif // INTEL_CUSTOMIZATION

  // Force any function attributes we want the rest of the pipeline to observe.
  MPM.addPass(ForceFunctionAttrsPass());

  // Do basic inference of function attributes from known properties of system
  // libraries and other oracles.
  MPM.addPass(InferFunctionAttrsPass());

  if (Level.getSpeedupLevel() > 1) {
#if INTEL_CUSTOMIZATION
    FunctionPassManager EarlyFPM;
    EarlyFPM.addPass(CallSiteSplittingPass());
    // Collect the information from the loops and insert the attributes
    EarlyFPM.addPass(IntelLoopAttrsPass(DTransEnabled));
    MPM.addPass(createModuleToFunctionPassAdaptor(
            std::move(EarlyFPM), PTO.EagerlyInvalidateAnalyses));
#endif // INTEL_CUSTOMIZATION

    // Indirect call promotion. This should promote all the targets that are
    // left by the earlier promotion pass that promotes intra-module targets.
    // This two-step promotion is to save the compile time. For LTO, it should
    // produce the same result as if we only do promotion here.
    MPM.addPass(PGOIndirectCallPromotion(
        true /* InLTO */, PGOOpt && PGOOpt->Action == PGOOptions::SampleUse));

    if (EnableFunctionSpecialization && Level == OptimizationLevel::O3)
      MPM.addPass(FunctionSpecializationPass());
    // Propagate constants at call sites into the functions they call.  This
    // opens opportunities for globalopt (and inlining) by substituting function
    // pointers passed as arguments to direct uses of functions.
    MPM.addPass(IPSCCPPass());

    // Attach metadata to indirect call sites indicating the set of functions
    // they may target at run-time. This should follow IPSCCP.
    MPM.addPass(CalledValuePropagationPass());
  }

  // Now deduce any function attributes based in the current code.
  MPM.addPass(createModuleToPostOrderCGSCCPassAdaptor(
              PostOrderFunctionAttrsPass()));

  // Do RPO function attribute inference across the module to forward-propagate
  // attributes where applicable.
  // FIXME: Is this really an optimization rather than a canonicalization?
  MPM.addPass(ReversePostOrderFunctionAttrsPass());

#if INTEL_CUSTOMIZATION
  // Optimize some dynamic_cast calls.
  MPM.addPass(OptimizeDynamicCastsPass());
  if (Level.getSpeedupLevel() > 1) {
    // Run the instruction simplify and CFG simplify passes before
    // devirtualization to clean the IR after lowering the
    // llvm.intel.wholeprogramsafe intrinsic.
    MPM.addPass(createModuleToFunctionPassAdaptor(InstSimplifyPass()));
    MPM.addPass(createModuleToFunctionPassAdaptor(SimplifyCFGPass()));
  }
#endif // INTEL_CUSTOMIZATION

  // Use in-range annotations on GEP indices to split globals where beneficial.
  MPM.addPass(GlobalSplitPass());

  // Run whole program optimization of virtual call when the list of callees
  // is fixed.
  MPM.addPass(WholeProgramDevirtPass(ExportSummary, nullptr));

  // Stop here at -O1.
  if (Level == OptimizationLevel::O1) {
#if INTEL_CUSTOMIZATION
    // Adding VPO Passes at O1 optimization level. Note: addLoopOptPasses() is
    // guarded under isLoopOptEnabled() so loopopt will not be invoked at O1.
    FunctionPassManager FPM;
    addLoopOptAndAssociatedVPOPasses(MPM, FPM, Level, true);
    MPM.addPass(createModuleToFunctionPassAdaptor(
        std::move(FPM), PTO.EagerlyInvalidateAnalyses));
#endif // INTEL_CUSTOMIZATION
    // The LowerTypeTestsPass needs to run to lower type metadata and the
    // type.test intrinsics. The pass does nothing if CFI is disabled.
    MPM.addPass(LowerTypeTestsPass(ExportSummary, nullptr));
    // Run a second time to clean up any type tests left behind by WPD for use
    // in ICP (which is performed earlier than this in the regular LTO
    // pipeline).
    MPM.addPass(LowerTypeTestsPass(nullptr, nullptr, true));

    for (auto &C : FullLinkTimeOptimizationLastEPCallbacks)
      C(MPM, Level);

    // Emit annotation remarks.
    addAnnotationRemarksPass(MPM);

    return MPM;
  }

#if INTEL_CUSTOMIZATION
#if INTEL_FEATURE_SW_DTRANS
  if (DTransEnabled)
    // This call adds the DTrans passes.
    addDTransPasses(MPM);
#endif // INTEL_FEATURE_SW_DTRANS
#if INTEL_FEATURE_SW_ADVANCED
  if (DTransEnabled)
    MPM.addPass(TileMVInlMarkerPass());
#endif // INTEL_FEATURE_SW_ADVANCED
  MPM.addPass(DopeVectorConstPropPass());
  MPM.addPass(createModuleToPostOrderCGSCCPassAdaptor(
              ArgumentPromotionPass()));
#endif // INTEL_CUSTOMIZATION

  // Optimize globals to try and fold them into constants.
  MPM.addPass(GlobalOptPass());

  // Promote any localized globals to SSA registers.
  MPM.addPass(createModuleToFunctionPassAdaptor(PromotePass()));

  // Linking modules together can lead to duplicate global constant, only
  // keep one copy of each constant.
  MPM.addPass(ConstantMergePass());

  // Remove unused arguments from functions.
  MPM.addPass(DeadArgumentEliminationPass());

#if INTEL_CUSTOMIZATION
#if INTEL_FEATURE_SW_DTRANS
  if (DTransEnabled) {
    addLateDTransPasses(MPM);
    if (EnableIndirectCallConv)
       MPM.addPass(IndirectCallConvPass(false /* EnableAndersen */,
                                       true /* DTransEnabled */));
  }
#endif // INTEL_FEATURE_SW_DTRANS
#endif // INTEL_CUSTOMIZATION

  // Reduce the code after globalopt and ipsccp.  Both can open up significant
  // simplification opportunities, and both can propagate functions through
  // function pointers.  When this happens, we often have to resolve varargs
  // calls, etc, so let instcombine do this.
  FunctionPassManager PeepholeFPM;
  addInstCombinePass(PeepholeFPM, !DTransEnabled); // INTEL
  if (Level == OptimizationLevel::O3)
    PeepholeFPM.addPass(AggressiveInstCombinePass());
  invokePeepholeEPCallbacks(PeepholeFPM, Level);

  MPM.addPass(createModuleToFunctionPassAdaptor(std::move(PeepholeFPM),
                                                PTO.EagerlyInvalidateAnalyses));

#if INTEL_CUSTOMIZATION

#if INTEL_FEATURE_SW_ADVANCED
  if (DTransEnabled) {
    MPM.addPass(IntelArgumentAlignmentPass());
    MPM.addPass(QsortRecognizerPass());
  }

  bool EnableIntelPartialInlining = EnableIntelPI && DTransEnabled;
  // Partially inline small functions
  if (EnableIntelPartialInlining)
    MPM.addPass(IntelPartialInlinePass());
#endif // INTEL_FEATURE_SW_ADVANCED

  // Parse -[no]inline-list option and set corresponding attributes.
  MPM.addPass(InlineListsPass());
  if (EnableAndersen) {
    MPM.addPass(RequireAnalysisPass<AndersensAA, Module>());
  }
  // Indirect to direct call conversion.
#if INTEL_FEATURE_SW_DTRANS
  if (EnableIndirectCallConv && EnableAndersen)
    MPM.addPass(IndirectCallConvPass(true /* EnableAndersen */,
                                     false /* EnableDTrans */));
#else // INTEL_FEATURE_SW_DTRANS
  if (EnableIndirectCallConv && EnableAndersen)
    MPM.addPass(IndirectCallConvPass(true /* EnableAndersen */));
#endif // INTEL_FEATURE_SW_DTRANS

  // Require the InlineAggAnalysis for the module so we can query it within
  // the inliner.
  if (EnableInlineAggAnalysis) {
    MPM.addPass(AggInlinerPass());
  }

  // Note: historically, the PruneEH pass was run first to deduce nounwind and
  // generally clean up exception handling overhead. It isn't clear this is
  // valuable as the inliner doesn't currently care whether it is inlining an
  // invoke or a call.
  // Run the inliner now.
  MPM.addPass(ModuleInlinerWrapperPass(
      getInlineParamsFromOptLevel(Level, PrepareForLTO, LinkForLTO,
      SYCLOptimizationMode), /* MandatoryFirst */ true,
      InlineContext{ThinOrFullLTOPhase::FullLTOPostLink,
                          InlinePass::CGSCCInliner}));

#if INTEL_FEATURE_SW_DTRANS
  // The global optimizer pass can convert function calls to use
  // the 'fastcc' calling convention. The following pass enables more
  // functions to be converted to this calling convention. This can improve
  // performance by having arguments passed in registers, and enable more
  // cases where pointer parameters are changed to pass-by-value parameters. We
  // can remove the test for DTransEnabled if it is found to be useful on other
  // cases.
  if (DTransEnabled)
    MPM.addPass(IntelAdvancedFastCallPass());

#endif // INTEL_FEATURE_SW_DTRANS
#endif // INTEL_CUSTOMIZATION

  // Optimize globals again after we ran the inliner.
  MPM.addPass(GlobalOptPass());

#if INTEL_CUSTOMIZATION
  if (RunLTOPartialInlining)
    MPM.addPass(PartialInlinerPass(true /*RunLTOPartialInline*/,
                                   false /*EnableSpecialCases*/));
  if (
#if INTEL_FEATURE_SW_ADVANCED
      EnableIPCloning ||
#endif // INTEL_FEATURE_SW_ADVANCED
      EnableCallTreeCloning) {
#if INTEL_FEATURE_SW_ADVANCED
    if (EnableIPCloning) {
      // Enable generic IPCloning after Inlining.
      MPM.addPass(IPCloningPass(/*AfterInl*/ true,
                              /*IFSwitchHeuristic*/ DTransEnabled));
    }
#endif // INTEL_FEATURE_SW_ADVANCED
    if (EnableCallTreeCloning) {
      // Do function cloning along call trees
      MPM.addPass(CallTreeCloningPass());
    }
    // Call IPCP to propagate constants
    MPM.addPass(IPSCCPPass());
  }
#endif // INTEL_CUSTOMIZATION

  // Garbage collect dead functions.
  MPM.addPass(GlobalDCEPass());

  // If we didn't decide to inline a function, check to see if we can
  // transform it to pass arguments by value instead of by reference.
  MPM.addPass(createModuleToPostOrderCGSCCPassAdaptor(ArgumentPromotionPass()));

  FunctionPassManager FPM;
  // The IPO Passes may leave cruft around. Clean up after them.
  addInstCombinePass(FPM, !DTransEnabled); // INTEL
  invokePeepholeEPCallbacks(FPM, Level);

  FPM.addPass(JumpThreadingPass());

#if INTEL_CUSTOMIZATION
  // Handle '#pragma vector aligned'.
  if (EnableHandlePragmaVectorAligned && Level.getSpeedupLevel() > 1)
    FPM.addPass(HandlePragmaVectorAlignedPass());
#endif // INTEL_CUSTOMIZATION

  // Do a post inline PGO instrumentation and use pass. This is a context
  // sensitive PGO pass.
  if (PGOOpt) {
    if (PGOOpt->CSAction == PGOOptions::CSIRInstr)
      addPGOInstrPasses(MPM, Level, /* RunProfileGen */ true,
                        /* IsCS */ true, PGOOpt->CSProfileGenFile,
                        PGOOpt->ProfileRemappingFile,
                        ThinOrFullLTOPhase::FullLTOPostLink);
    else if (PGOOpt->CSAction == PGOOptions::CSIRUse)
      addPGOInstrPasses(MPM, Level, /* RunProfileGen */ false,
                        /* IsCS */ true, PGOOpt->ProfileFile,
                        PGOOpt->ProfileRemappingFile,
                        ThinOrFullLTOPhase::FullLTOPostLink);
  }

  // Break up allocas
  FPM.addPass(SROAPass());

#if INTEL_CUSTOMIZATION
  FPM.addPass(CorrelatedValuePropagationPass());
  if (EnableMultiVersioning) {
    FPM.addPass(MultiVersioningPass());

#if INTEL_FEATURE_SW_DTRANS
    if (DTransEnabled)
      FPM.addPass(SimplifyCFGPass(SimplifyCFGOptions().hoistCommonInsts(true)));
#endif // INTEL_FEATURE_SW_DTRANS
  }
#endif // INTEL_CUSTOMIZATION
  // LTO provides additional opportunities for tailcall elimination due to
  // link-time inlining, and visibility of nocapture attribute.
  FPM.addPass(TailCallElimPass());

#if INTEL_CUSTOMIZATION
  // Collect the information from the loops and insert the attributes
  if (Level.getSpeedupLevel() > 1)
    FPM.addPass(IntelLoopAttrsPass(DTransEnabled));
#endif // INTEL_CUSTOMIZATION

  // Run a few AA driver optimizations here and now to cleanup the code.
  MPM.addPass(createModuleToFunctionPassAdaptor(std::move(FPM),
                                                PTO.EagerlyInvalidateAnalyses));

  MPM.addPass(
      createModuleToPostOrderCGSCCPassAdaptor(PostOrderFunctionAttrsPass()));

#if INTEL_CUSTOMIZATION
  if (EnableIPArrayTranspose)
    MPM.addPass(IPArrayTransposePass());

#if INTEL_FEATURE_SW_ADVANCED
  if (DTransEnabled)
    MPM.addPass(IPPredOptPass());
  if (EnableDeadArrayOpsElim)
    MPM.addPass(DeadArrayOpsEliminationPass());
#endif // INTEL_FEATURE_SW_ADVANCED

  // Propagate noalias attribute to function arguments.
  if (EnableArgNoAliasProp && Level.getSpeedupLevel() > 2)
    MPM.addPass(ArgNoAliasPropPass());

  if (EnableAndersen) {
    // Andersen's IP alias analysis
    // AndersensAA is stateless analysis that is not invalided by any passes.
    // So, first invalidate it and then recompute AndersensAA again due
    // to many changes in IR.
    MPM.addPass(InvalidateAnalysisPass<AndersensAA>());
    MPM.addPass(RequireAnalysisPass<AndersensAA, Module>());
  }
#endif // INTEL_CUSTOMIZATION
  // Require the GlobalsAA analysis for the module so we can query it within
  // MainFPM.
  MPM.addPass(RequireAnalysisPass<GlobalsAA, Module>());
  // Invalidate AAManager so it can be recreated and pick up the newly available
  // GlobalsAA.
  MPM.addPass(
      createModuleToFunctionPassAdaptor(InvalidateAnalysisPass<AAManager>()));
#if INTEL_CUSTOMIZATION
#if INTEL_FEATURE_SW_DTRANS
  // Loop transformations need information about structure fields that are
  // modified/referenced during function calls.
  if (DTransEnabled)
    MPM.addPass(RequireAnalysisPass<DTransFieldModRefAnalysis, Module>());
#endif // INTEL_FEATURE_SW_DTRANS
  MPM.addPass(IntelIPODeadArgEliminationPass());
#endif // INTEL_CUSTOMIZATION

  FunctionPassManager MainFPM;
  MainFPM.addPass(createFunctionToLoopPassAdaptor(
      LICMPass(PTO.LicmMssaOptCap, PTO.LicmMssaNoAccForPromotionCap,
               /*AllowSpeculation=*/true),
      /*USeMemorySSA=*/true, /*UseBlockFrequencyInfo=*/true));

  if (RunNewGVN)
    MainFPM.addPass(NewGVNPass());
  else
    MainFPM.addPass(GVNPass());

  MainFPM.addPass(DopeVectorHoistPass()); // INTEL

  // Remove dead memcpy()'s.
  MainFPM.addPass(MemCpyOptPass());

  // Nuke dead stores.
  MainFPM.addPass(DSEPass());
  MainFPM.addPass(MergedLoadStoreMotionPass());


  if (EnableConstraintElimination)
    MainFPM.addPass(ConstraintEliminationPass());

  LoopPassManager LPM;
  if (EnableLoopFlatten && Level.getSpeedupLevel() > 1)
    LPM.addPass(LoopFlattenPass());
#if INTEL_COLLAB
  if (!SPIRVOptimizationMode)
    LPM.addPass(IndVarSimplifyPass());
  else
    LPM.addPass(IndVarSimplifyPass(false /* WidenIndVars */));
#else // INTEL_COLLAB
  LPM.addPass(IndVarSimplifyPass());
#endif // INTEL_COLLAB
  LPM.addPass(LoopDeletionPass());
  // FIXME: Add loop interchange.

#if INTEL_CUSTOMIZATION
  // HIR complete unroll pass replaces LLVM's full loop unroll pass.
  if (!isLoopOptEnabled(Level))
#endif // INTEL_CUSTOMIZATION
  // Unroll small loops and perform peeling.
  LPM.addPass(LoopFullUnrollPass(Level.getSpeedupLevel(),
                                 /* OnlyWhenForced= */ !PTO.LoopUnrolling,
                                 PTO.ForgetAllSCEVInLoopUnroll));
  // The loop passes in LPM (LoopFullUnrollPass) do not preserve MemorySSA.
  // *All* loop passes must preserve it, in order to be able to use it.
  MainFPM.addPass(createFunctionToLoopPassAdaptor(
      std::move(LPM), /*UseMemorySSA=*/false, /*UseBlockFrequencyInfo=*/true));

#if INTEL_CUSTOMIZATION
  addLoopOptAndAssociatedVPOPasses(MPM, MainFPM, Level, true);
#endif // INTEL_CUSTOMIZATION
  MainFPM.addPass(LoopDistributePass());

  addVectorPasses(Level, MainFPM, /* IsFullLTO */ true);

  // Run the OpenMPOpt CGSCC pass again late.
  MPM.addPass(
      createModuleToPostOrderCGSCCPassAdaptor(OpenMPOptCGSCCPass()));

  invokePeepholeEPCallbacks(MainFPM, Level);

  MainFPM.addPass(JumpThreadingPass());
#if INTEL_CUSTOMIZATION
  MainFPM.addPass(ForcedCMOVGenerationPass()); // To help CMOV generation
#endif // INTEL_CUSTOMIZATION

  MPM.addPass(createModuleToFunctionPassAdaptor(std::move(MainFPM),
                                                PTO.EagerlyInvalidateAnalyses));

  // Lower type metadata and the type.test intrinsic. This pass supports
  // clang's control flow integrity mechanisms (-fsanitize=cfi*) and needs
  // to be run at link time if CFI is enabled. This pass does nothing if
  // CFI is disabled.
  MPM.addPass(LowerTypeTestsPass(ExportSummary, nullptr));
  // Run a second time to clean up any type tests left behind by WPD for use
  // in ICP (which is performed earlier than this in the regular LTO pipeline).
  MPM.addPass(LowerTypeTestsPass(nullptr, nullptr, true));

  // Enable splitting late in the FullLTO post-link pipeline.
  if (EnableHotColdSplit)
    MPM.addPass(HotColdSplittingPass());

  // Add late LTO optimization passes.
  // Delete basic blocks, which optimization passes may have killed.
#if INTEL_CUSTOMIZATION
  // 28038: Avoid excessive hoisting as it increases register pressure and
  // select conversion without clear gains.
  // MPM.addPass(createModuleToFunctionPassAdaptor(
  //   SimplifyCFGPass(SimplifyCFGOptions().hoistCommonInsts(true))));
  MPM.addPass(createModuleToFunctionPassAdaptor(SimplifyCFGPass(
      SimplifyCFGOptions().convertSwitchRangeToICmp(true).hoistCommonInsts(
          true))));
#endif // INTEL_CUSTOMIZATION

#if INTEL_CUSTOMIZATION
  // HIR complete unroll can expose opportunities for optimizing globals and
  // allocas.
  if (isLoopOptEnabled(Level))
    MPM.addPass(GlobalOptPass());
#endif // INTEL_CUSTOMIZATION

  // Drop bodies of available eternally objects to improve GlobalDCE.
  MPM.addPass(EliminateAvailableExternallyPass());

  // Now that we have optimized the program, discard unreachable functions.
  MPM.addPass(GlobalDCEPass());

  if (PTO.MergeFunctions)
    MPM.addPass(MergeFunctionsPass());

  if (PTO.CallGraphProfile)
    MPM.addPass(CGProfilePass());

  for (auto &C : FullLinkTimeOptimizationLastEPCallbacks)
    C(MPM, Level);

  // Emit annotation remarks.
  addAnnotationRemarksPass(MPM);

#if INTEL_CUSTOMIZATION
  MPM.addPass(InlineReportEmitterPass(Level.getSpeedupLevel(),
                                      Level.getSizeLevel(), false));
#endif // INTEL_CUSTOMIZATION

  return MPM;
}

ModulePassManager PassBuilder::buildO0DefaultPipeline(OptimizationLevel Level,
                                                      bool LTOPreLink) {
  assert(Level == OptimizationLevel::O0 &&
         "buildO0DefaultPipeline should only be used with O0");

  ModulePassManager MPM;

  // Perform pseudo probe instrumentation in O0 mode. This is for the
  // consistency between different build modes. For example, a LTO build can be
  // mixed with an O0 prelink and an O2 postlink. Loading a sample profile in
  // the postlink will require pseudo probe instrumentation in the prelink.
  if (PGOOpt && PGOOpt->PseudoProbeForProfiling)
    MPM.addPass(SampleProfileProbePass(TM));

  if (PGOOpt && (PGOOpt->Action == PGOOptions::IRInstr ||
                 PGOOpt->Action == PGOOptions::IRUse))
    addPGOInstrPassesForO0(
        MPM,
        /* RunProfileGen */ (PGOOpt->Action == PGOOptions::IRInstr),
        /* IsCS */ false, PGOOpt->ProfileFile, PGOOpt->ProfileRemappingFile);

  for (auto &C : PipelineStartEPCallbacks)
    C(MPM, Level);

  if (PGOOpt && PGOOpt->DebugInfoForProfiling)
    MPM.addPass(createModuleToFunctionPassAdaptor(AddDiscriminatorsPass()));

  for (auto &C : PipelineEarlySimplificationEPCallbacks)
    C(MPM, Level);

#if INTEL_CUSTOMIZATION
  if (RunVPOOpt && RunVPOParopt) {
    // Paropt passes and BasicAA (one of Paropt's dependencies), use
    // XmainOptLevelPass.
    MPM.addPass(XmainOptLevelAnalysisInit(Level.getSpeedupLevel()));
    MPM.addPass(RequireAnalysisPass<VPOParoptConfigAnalysis, Module>());
  }

#endif // INTEL_CUSTOMIZATION
#if INTEL_COLLAB
  FunctionPassManager FPM;
#if INTEL_CUSTOMIZATION
  FPM.addPass(LowerSubscriptIntrinsicPass());
#endif // INTEL_CUSTOMIZATION
  if (RunVPOOpt && RunVPOParopt)
    addVPOPreparePasses(FPM);
  if (!FPM.isEmpty())
    MPM.addPass(createModuleToFunctionPassAdaptor(std::move(FPM)));

#endif // INTEL_COLLAB
  // Build a minimal pipeline based on the semantics required by LLVM,
  // which is just that always inlining occurs. Further, disable generating
  // lifetime intrinsics to avoid enabling further optimizations during
  // code generation.
  MPM.addPass(InlineListsPass()); // INTEL
  MPM.addPass(AlwaysInlinerPass(
      /*InsertLifetimeIntrinsics=*/false));

  if (PTO.MergeFunctions)
    MPM.addPass(MergeFunctionsPass());

  if (EnableMatrix)
    MPM.addPass(
        createModuleToFunctionPassAdaptor(LowerMatrixIntrinsicsPass(true)));

#if INTEL_COLLAB
  if (RunVPOOpt) {
#if INTEL_CUSTOMIZATION
    if (RunVecClone && RunVPOVecopt)
      MPM.addPass(VecClonePass());
#endif // INTEL_CUSTOMIZATION
    // Add VPO transform and vec passes.
    FunctionPassManager FPM;
    addVPOPasses(MPM, FPM, Level, /*RunVec=*/true);
    if (!FPM.isEmpty())
      MPM.addPass(createModuleToFunctionPassAdaptor(std::move(FPM)));
  }

#endif // INTEL_COLLAB
  if (!CGSCCOptimizerLateEPCallbacks.empty()) {
    CGSCCPassManager CGPM;
    for (auto &C : CGSCCOptimizerLateEPCallbacks)
      C(CGPM, Level);
    if (!CGPM.isEmpty())
      MPM.addPass(createModuleToPostOrderCGSCCPassAdaptor(std::move(CGPM)));
  }
  if (!LateLoopOptimizationsEPCallbacks.empty()) {
    LoopPassManager LPM;
    for (auto &C : LateLoopOptimizationsEPCallbacks)
      C(LPM, Level);
    if (!LPM.isEmpty()) {
      MPM.addPass(createModuleToFunctionPassAdaptor(
          createFunctionToLoopPassAdaptor(std::move(LPM))));
    }
  }
  if (!LoopOptimizerEndEPCallbacks.empty()) {
    LoopPassManager LPM;
    for (auto &C : LoopOptimizerEndEPCallbacks)
      C(LPM, Level);
    if (!LPM.isEmpty()) {
      MPM.addPass(createModuleToFunctionPassAdaptor(
          createFunctionToLoopPassAdaptor(std::move(LPM))));
    }
  }
  if (!ScalarOptimizerLateEPCallbacks.empty()) {
    FunctionPassManager FPM;
    for (auto &C : ScalarOptimizerLateEPCallbacks)
      C(FPM, Level);
    if (!FPM.isEmpty())
      MPM.addPass(createModuleToFunctionPassAdaptor(std::move(FPM)));
  }

  for (auto &C : OptimizerEarlyEPCallbacks)
    C(MPM, Level);

  if (!VectorizerStartEPCallbacks.empty()) {
    FunctionPassManager FPM;
    for (auto &C : VectorizerStartEPCallbacks)
      C(FPM, Level);
    if (!FPM.isEmpty())
      MPM.addPass(createModuleToFunctionPassAdaptor(std::move(FPM)));
  }

  ModulePassManager CoroPM;
  CoroPM.addPass(CoroEarlyPass());
  CGSCCPassManager CGPM;
  CGPM.addPass(CoroSplitPass());
  CoroPM.addPass(createModuleToPostOrderCGSCCPassAdaptor(std::move(CGPM)));
  CoroPM.addPass(CoroCleanupPass());
  CoroPM.addPass(GlobalDCEPass());
  MPM.addPass(CoroConditionalWrapper(std::move(CoroPM)));

  for (auto &C : OptimizerLastEPCallbacks)
    C(MPM, Level);

  if (LTOPreLink)
    addRequiredLTOPreLinkPasses(MPM);

  MPM.addPass(createModuleToFunctionPassAdaptor(AnnotationRemarksPass()));

  return MPM;
}

AAManager PassBuilder::buildDefaultAAPipeline() {
  AAManager AA;

  // The order in which these are registered determines their priority when
  // being queried.

  // First we register the basic alias analysis that provides the majority of
  // per-function local AA logic. This is a stateless, on-demand local set of
  // AA techniques.
  AA.registerFunctionAnalysis<BasicAA>();

  // Next we query fast, specialized alias analyses that wrap IR-embedded
  // information about aliasing.
  AA.registerFunctionAnalysis<ScopedNoAliasAA>();
  AA.registerFunctionAnalysis<TypeBasedAA>();

#if INTEL_CUSTOMIZATION
  AA.registerFunctionAnalysis<StdContainerAA>();
  if (EnableAndersen)
    AA.registerModuleAnalysis<AndersensAA>();
#endif // INTEL_CUSTOMIZATION

  // Add support for querying global aliasing information when available.
  // Because the `AAManager` is a function analysis and `GlobalsAA` is a module
  // analysis, all that the `AAManager` can do is query for any *cached*
  // results from `GlobalsAA` through a readonly proxy.
  AA.registerModuleAnalysis<GlobalsAA>();

  // Add target-specific alias analyses.
  if (TM)
    TM->registerDefaultAliasAnalyses(AA);

  return AA;
}

