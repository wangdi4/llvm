//===- llvm/InitializePasses.h - Initialize All Passes ----------*- C++ -*-===//
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
//
// This file contains the declarations for the pass initialization routines
// for the entire LLVM project.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_INITIALIZEPASSES_H
#define LLVM_INITIALIZEPASSES_H

namespace llvm {

class PassRegistry;

/// Initialize all passes linked into the TransformUtils library.
void initializeCore(PassRegistry&);

/// Initialize all passes linked into the TransformUtils library.
void initializeTransformUtils(PassRegistry&);

/// Initialize all passes linked into the ScalarOpts library.
void initializeScalarOpts(PassRegistry&);

/// Initialize all passes linked into the ObjCARCOpts library.
void initializeObjCARCOpts(PassRegistry&);

/// Initialize all passes linked into the Vectorize library.
void initializeVectorization(PassRegistry&);

/// Initialize all passes linked into the InstCombine library.
void initializeInstCombine(PassRegistry&);

/// Initialize all passes linked into the AggressiveInstCombine library.
void initializeAggressiveInstCombine(PassRegistry&);

/// Initialize all passes linked into the IPO library.
void initializeIPO(PassRegistry&);

/// Initialize all passes linked into the Instrumentation library.
void initializeInstrumentation(PassRegistry&);

/// Initialize all passes linked into the Analysis library.
void initializeAnalysis(PassRegistry&);

/// Initialize all passes linked into the CodeGen library.
void initializeCodeGen(PassRegistry&);

/// Initialize all passes linked into the GlobalISel library.
void initializeGlobalISel(PassRegistry&);

/// Initialize all passes linked into the CodeGen library.
void initializeTarget(PassRegistry&);

#if INTEL_CUSTOMIZATION

/// initializeIntel_LoopAnalysis - Initialize all passes linked into the
/// Intel_LoopAnalysis library.
void initializeIntel_LoopAnalysis(PassRegistry&);

/// initializeIntel_LoopTransforms - Initialize all passes linked into the
/// Intel_LoopTransforms library.
void initializeIntel_LoopTransforms(PassRegistry&);

// initializeIntel_OpenCLTransforms - Initialize all passes linked into the
// Intel_OpenCLTransforms library
void initializeIntel_OpenCLTransforms(PassRegistry&);

// initializeIntel_DPCPPKernelTransforms - Initialize all passes linked into the
// Intel_DPCPPKernelTransforms
void initializeIntel_DPCPPKernelTransforms(PassRegistry&);

void initializeInlineListsPass(PassRegistry&);
void initializeInlineReportSetupPass(PassRegistry&);
void initializeInlineReportEmitterPass(PassRegistry&);
#endif // INTEL_CUSTOMIZATION

#if INTEL_COLLAB
// initializeVPOAnaylsis - Initialize all passes linked into the
// VPOAnalysis library
void initializeVPOAnalysis(PassRegistry&);

// initializeVPOTransforms - Initialize all passes linked into the
// VPOTransforms library
void initializeVPOTransforms(PassRegistry&);
#endif // INTEL_COLLAB

void initializeAAEvalLegacyPassPass(PassRegistry&);
void initializeAAResultsWrapperPassPass(PassRegistry&);
void initializeADCELegacyPassPass(PassRegistry&);
void initializeAddDiscriminatorsLegacyPassPass(PassRegistry&);
void initializeAddFSDiscriminatorsPass(PassRegistry &);
void initializeAggressiveInstCombinerLegacyPassPass(PassRegistry&);
void initializeAliasSetPrinterPass(PassRegistry&);
void initializeAlignmentFromAssumptionsPass(PassRegistry&);
void initializeAlwaysInlinerLegacyPassPass(PassRegistry&);
void initializeAndersensAAWrapperPassPass(PassRegistry&); // INTEL
void initializeAssumeSimplifyPassLegacyPassPass(PassRegistry &);
void initializeAssumeBuilderPassLegacyPassPass(PassRegistry &);
void initializeAnnotation2MetadataLegacyPass(PassRegistry &);
void initializeAnnotationRemarksLegacyPass(PassRegistry &);
void initializeOpenMPOptCGSCCLegacyPassPass(PassRegistry &);
void initializeArgNoAliasPropPass(PassRegistry &); // INTEL
void initializeArrayUseWrapperPassPass(PassRegistry&); // INTEL
void initializeAssumptionCacheTrackerPass(PassRegistry&);
void initializeAtomicExpandPass(PassRegistry&);
void initializeAttributorLegacyPassPass(PassRegistry&);
void initializeAttributorCGSCCLegacyPassPass(PassRegistry &);
void initializeBasicBlockSectionsProfileReaderPass(PassRegistry &);
void initializeBasicBlockSectionsPass(PassRegistry &);
void initializeBDCELegacyPassPass(PassRegistry&);
void initializeBarrierNoopPass(PassRegistry&);
void initializeBasicAAWrapperPassPass(PassRegistry&);
void initializeBlockExtractorLegacyPassPass(PassRegistry &);
void initializeBlockFrequencyInfoWrapperPassPass(PassRegistry&);
void initializeBoundsCheckingLegacyPassPass(PassRegistry&);
void initializeBranchFolderPassPass(PassRegistry&);
void initializeBranchProbabilityInfoWrapperPassPass(PassRegistry&);
void initializeBranchRelaxationPass(PassRegistry&);
void initializeBreakCriticalEdgesPass(PassRegistry&);
void initializeBreakFalseDepsPass(PassRegistry&);
void initializeCanonicalizeFreezeInLoopsPass(PassRegistry &);
void initializeCFGOnlyPrinterLegacyPassPass(PassRegistry&);
void initializeCFGOnlyViewerLegacyPassPass(PassRegistry&);
void initializeCFGPrinterLegacyPassPass(PassRegistry&);
void initializeCFGSimplifyPassPass(PassRegistry&);
void initializeCFGuardPass(PassRegistry&);
void initializeCFGuardLongjmpPass(PassRegistry&);
void initializeCFGViewerLegacyPassPass(PassRegistry&);
void initializeCFIFixupPass(PassRegistry&);
void initializeCFIInstrInserterPass(PassRegistry&);
void initializeCFLAndersAAWrapperPassPass(PassRegistry&);
void initializeCFLSteensAAWrapperPassPass(PassRegistry&);
void initializeCallGraphDOTPrinterPass(PassRegistry&);
void initializeCallGraphPrinterLegacyPassPass(PassRegistry&);
void initializeCallGraphViewerPass(PassRegistry&);
void initializeCallGraphWrapperPassPass(PassRegistry&);
void initializeCallSiteSplittingLegacyPassPass(PassRegistry&);
void initializeCalledValuePropagationLegacyPassPass(PassRegistry &);
void initializeCheckDebugMachineModulePass(PassRegistry &);
void initializeCodeGenPreparePass(PassRegistry&);
void initializeConstantHoistingLegacyPassPass(PassRegistry&);
void initializeConstantMergeLegacyPassPass(PassRegistry&);
void initializeConstraintEliminationPass(PassRegistry &);
void initializeConvertGEPToSubscriptIntrinsicLegacyPassPass(PassRegistry &); // INTEL
void initializeGCOVProfilerLegacyPassPass(PassRegistry&); // INTEL
void initializeCorrelatedValuePropagationPass(PassRegistry&);
void initializeCostModelAnalysisPass(PassRegistry&);
void initializeCrossDSOCFIPass(PassRegistry&);
void initializeCycleInfoWrapperPassPass(PassRegistry &);
void initializeDAEPass(PassRegistry&);
void initializeDAHPass(PassRegistry&);
void initializeDAESYCLPass(PassRegistry&);
void initializeDCELegacyPassPass(PassRegistry&);
void initializeDFAJumpThreadingLegacyPassPass(PassRegistry &);
void initializeDSELegacyPassPass(PassRegistry&);
void initializeDataFlowSanitizerLegacyPassPass(PassRegistry &);
#if INTEL_CUSTOMIZATION
#if INTEL_FEATURE_SW_ADVANCED
void initializeDeadArrayOpsEliminationLegacyPassPass(PassRegistry&);
#endif // INTEL_FEATURE_SW_ADVANCED
#endif // INTEL_CUSTOMIZATION
void initializeDeadMachineInstructionElimPass(PassRegistry&);
void initializeDebugifyMachineModulePass(PassRegistry &);
void initializeDelinearizationPass(PassRegistry&);
void initializeDemandedBitsWrapperPassPass(PassRegistry&);
void initializeDependenceAnalysisPass(PassRegistry&);
void initializeDependenceAnalysisWrapperPassPass(PassRegistry&);
void initializeDopeVectorConstPropLegacyPassPass(PassRegistry&);   // INTEL
void initializeDetectDeadLanesPass(PassRegistry&);
void initializeDivRemPairsLegacyPassPass(PassRegistry&);
void initializeDomOnlyPrinterWrapperPassPass(PassRegistry &);
void initializeDomOnlyViewerWrapperPassPass(PassRegistry &);
void initializeDomPrinterWrapperPassPass(PassRegistry &);
void initializeDomViewerWrapperPassPass(PassRegistry &);
void initializeDominanceFrontierWrapperPassPass(PassRegistry&);
void initializeDominatorTreeWrapperPassPass(PassRegistry&);
void initializeDwarfEHPrepareLegacyPassPass(PassRegistry &);
void initializeEarlyCSELegacyPassPass(PassRegistry&);
void initializeEarlyCSEMemSSALegacyPassPass(PassRegistry&);
void initializeEarlyIfConverterPass(PassRegistry&);
void initializeEarlyIfPredicatorPass(PassRegistry &);
void initializeEarlyMachineLICMPass(PassRegistry&);
void initializeEarlyTailDuplicatePass(PassRegistry&);
void initializeEdgeBundlesPass(PassRegistry&);
void initializeEHContGuardCatchretPass(PassRegistry &);
void initializeEliminateAvailableExternallyLegacyPassPass(PassRegistry&);
void initializeExpandComplexPass(PassRegistry&); // INTEL
void initializeExpandMemCmpPassPass(PassRegistry&);
void initializeExpandPostRAPass(PassRegistry&);
void initializeExpandReductionsPass(PassRegistry&);
void initializeExpandVectorPredicationPass(PassRegistry &);
void initializeMakeGuardsExplicitLegacyPassPass(PassRegistry&);
void initializeExternalAAWrapperPassPass(PassRegistry&);
void initializeFEntryInserterPass(PassRegistry&);
void initializeFinalizeISelPass(PassRegistry&);
void initializeFinalizeMachineBundlesPass(PassRegistry&);
void initializeFixIrreduciblePass(PassRegistry &);
void initializeFixupStatepointCallerSavedPass(PassRegistry&);
void initializeFlattenCFGLegacyPassPass(PassRegistry &);
void initializeFloat128ExpandPass(PassRegistry&); // INTEL
void initializeFloat2IntLegacyPassPass(PassRegistry&);
void initializeHeteroArchOptPass(PassRegistry&); // INTEL
void initializeFoldLoadsToGatherPass(PassRegistry&); // INTEL
void initializeForceFunctionAttrsLegacyPassPass(PassRegistry&);
void initializeForwardControlFlowIntegrityPass(PassRegistry&);
void initializeFuncletLayoutPass(PassRegistry&);
void initializeFunctionSpecializationLegacyPassPass(PassRegistry &);
void initializeFunctionSplittingWrapperPass(PassRegistry&);        // INTEL
void initializeGCMachineCodeAnalysisPass(PassRegistry&);
void initializeGCModuleInfoPass(PassRegistry&);
void initializeGVNHoistLegacyPassPass(PassRegistry&);
void initializeGVNLegacyPassPass(PassRegistry&);
void initializeGVNSinkLegacyPassPass(PassRegistry&);
void initializeGlobalDCELegacyPassPass(PassRegistry&);
void initializeGlobalMergePass(PassRegistry&);
void initializeGlobalOptLegacyPassPass(PassRegistry&);
void initializeGlobalSplitPass(PassRegistry&);
void initializeGlobalsAAWrapperPassPass(PassRegistry&);
void initializeGuardWideningLegacyPassPass(PassRegistry&);
void initializeHardwareLoopsPass(PassRegistry&);
void initializeMIRProfileLoaderPassPass(PassRegistry &);
void initializeMemProfilerLegacyPassPass(PassRegistry &);
void initializeHotColdSplittingLegacyPassPass(PassRegistry&);
void initializeIPSCCPLegacyPassPass(PassRegistry&);
void initializeIRCELegacyPassPass(PassRegistry&);
void initializeIROutlinerLegacyPassPass(PassRegistry&);
void initializeIRSimilarityIdentifierWrapperPassPass(PassRegistry&);
void initializeIRTranslatorPass(PassRegistry&);
#if INTEL_CUSTOMIZATION
void initializeIPArrayTransposeLegacyPassPass(PassRegistry &);
#if INTEL_FEATURE_SW_ADVANCED
void initializeIPCloningLegacyPassPass(PassRegistry&);
void initializeIPPredOptLegacyPassPass(PassRegistry &);
#endif // INTEL_FEATURE_SW_ADVANCED
#endif // INTEL_CUSTOMIZATION
void initializeCallTreeCloningLegacyPassPass(PassRegistry &);      // INTEL
void initializeIVUsersWrapperPassPass(PassRegistry&);
void initializeIfConverterPass(PassRegistry&);
void initializeImmutableModuleSummaryIndexWrapperPassPass(PassRegistry&);
void initializeImplicitNullChecksPass(PassRegistry&);
void initializeIndVarSimplifyLegacyPassPass(PassRegistry&);
void initializeIndirectBrExpandPassPass(PassRegistry&);
void initializeInferAddressSpacesPass(PassRegistry&);
void initializeInferFunctionAttrsLegacyPassPass(PassRegistry&);
void initializeInjectTLIMappingsLegacyPass(PassRegistry &);
void initializeInlineAggressiveWrapperPassPass(PassRegistry&); // INTEL
void initializeInlineCostAnalysisPass(PassRegistry&);
void initializeInstCountLegacyPassPass(PassRegistry &);
void initializeInstNamerPass(PassRegistry&);
void initializeInstSimplifyLegacyPassPass(PassRegistry &);
void initializeInstructionCombiningPassPass(PassRegistry&);
void initializeInstructionSelectPass(PassRegistry&);
void initializeIntelAdvancedFastCallWrapperPassPass(PassRegistry &); // INTEL
#if INTEL_CUSTOMIZATION
#if INTEL_FEATURE_SW_ADVANCED
void initializeIntelIPOPrefetchWrapperPassPass(PassRegistry &);
void initializeIntelPartialInlineLegacyPassPass(PassRegistry &);
#endif // INTEL_FEATURE_SW_ADVANCED
#if INTEL_FEATURE_SW_DTRANS
void initializeIntelFoldWPIntrinsicLegacyPassPass(PassRegistry &);
#endif // INTEL_FEATURE_SW_DTRANS
#endif // INTEL_CUSTOMIZATION
void initializeIntelArgumentAlignmentLegacyPassPass(PassRegistry &); // INTEL
void initializeInterleavedAccessPass(PassRegistry&);
void initializeInterleavedLoadCombinePass(PassRegistry &);
void initializeInternalizeLegacyPassPass(PassRegistry&);
void initializeIntervalPartitionPass(PassRegistry&);
void initializeJMCInstrumenterPass(PassRegistry&);
void initializeJumpThreadingPass(PassRegistry&);
void initializeIVSplitLegacyPassPass(PassRegistry&); // INTEL
void initializeLCSSAVerificationPassPass(PassRegistry&);
void initializeLCSSAWrapperPassPass(PassRegistry&);
void initializeLazyBlockFrequencyInfoPassPass(PassRegistry&);
void initializeLazyBranchProbabilityInfoPassPass(PassRegistry&);
void initializeLazyMachineBlockFrequencyInfoPassPass(PassRegistry&);
void initializeLazyValueInfoPrinterPass(PassRegistry&);
void initializeLazyValueInfoWrapperPassPass(PassRegistry&);
void initializeLegacyDivergenceAnalysisPass(PassRegistry&);
void initializeLegacyLICMPassPass(PassRegistry&);
void initializeLegacyLoopSinkPassPass(PassRegistry&);
void initializeLegalizerPass(PassRegistry&);
void initializeMemorySanitizerLegacyPassPass(PassRegistry&); // INTEL
void initializeGISelCSEAnalysisWrapperPassPass(PassRegistry &);
void initializeGISelKnownBitsAnalysisPass(PassRegistry &);
void initializeLibCallsShrinkWrapLegacyPassPass(PassRegistry&);
void initializeLintLegacyPassPass(PassRegistry &);
void initializeLiveDebugValuesPass(PassRegistry&);
void initializeLiveDebugVariablesPass(PassRegistry&);
void initializeLiveIntervalsPass(PassRegistry&);
void initializeLiveRangeShrinkPass(PassRegistry&);
void initializeLiveRegMatrixPass(PassRegistry&);
void initializeLiveStacksPass(PassRegistry&);
void initializeLiveVariablesPass(PassRegistry &);
void initializeLoadStoreOptPass(PassRegistry &);
void initializeLoadStoreVectorizerLegacyPassPass(PassRegistry&);
void initializeLoaderPassPass(PassRegistry&);
void initializeLocalStackSlotPassPass(PassRegistry&);
void initializeLocalizerPass(PassRegistry&);
void initializeLoopAccessLegacyAnalysisPass(PassRegistry&);
void initializeLoopDataPrefetchLegacyPassPass(PassRegistry&);
void initializeLoopDeletionLegacyPassPass(PassRegistry&);
void initializeLoopDistributeLegacyPass(PassRegistry&);
void initializeLoopExtractorLegacyPassPass(PassRegistry &);
void initializeLoopGuardWideningLegacyPassPass(PassRegistry&);
void initializeLoopFuseLegacyPass(PassRegistry&);
void initializeLoopIdiomRecognizeLegacyPassPass(PassRegistry&);
void initializeLoopInfoWrapperPassPass(PassRegistry&);
void initializeLoopInstSimplifyLegacyPassPass(PassRegistry&);
void initializeLoopInterchangeLegacyPassPass(PassRegistry &);
void initializeLoopFlattenLegacyPassPass(PassRegistry&);
void initializeLoopLoadEliminationPass(PassRegistry&);
void initializeLoopPassPass(PassRegistry&);
void initializeLoopPredicationLegacyPassPass(PassRegistry&);
void initializeLoopRerollLegacyPassPass(PassRegistry &);
void initializeLoopRotateLegacyPassPass(PassRegistry&);
void initializeLoopSimplifyCFGLegacyPassPass(PassRegistry&);
void initializeLoopSimplifyPass(PassRegistry&);
void initializeLoopStrengthReducePass(PassRegistry&);
void initializeLoopUnrollAndJamPass(PassRegistry&);
void initializeLoopUnrollPass(PassRegistry&);
void initializeLoopUnswitchPass(PassRegistry&);
void initializeLoopVectorizePass(PassRegistry&);
void initializeLoopVersioningLICMLegacyPassPass(PassRegistry &);
void initializeLoopVersioningLegacyPassPass(PassRegistry &);
void initializeLowerAtomicLegacyPassPass(PassRegistry&);
void initializeLowerConstantIntrinsicsPass(PassRegistry&);
void initializeLowerEmuTLSPass(PassRegistry&);
void initializeLowerExpectIntrinsicPass(PassRegistry&);
void initializeLowerGlobalDtorsLegacyPassPass(PassRegistry &);
void initializeLowerGuardIntrinsicLegacyPassPass(PassRegistry&);
void initializeLowerWidenableConditionLegacyPassPass(PassRegistry&);
void initializeLowerSubscriptIntrinsicLegacyPassPass(PassRegistry&); // INTEL
void initializeLowerIntrinsicsPass(PassRegistry&);
void initializeLowerInvokeLegacyPassPass(PassRegistry&);
void initializeLowerSwitchLegacyPassPass(PassRegistry &);
void initializeLowerMatrixIntrinsicsLegacyPassPass(PassRegistry &);
void initializeLowerMatrixIntrinsicsMinimalLegacyPassPass(PassRegistry &);
void initializeMIRAddFSDiscriminatorsPass(PassRegistry &);
void initializeMIRCanonicalizerPass(PassRegistry &);
void initializeMIRNamerPass(PassRegistry &);
void initializeMIRPrintingPassPass(PassRegistry&);
void initializeMachineBlockFrequencyInfoPass(PassRegistry&);
void initializeMachineBlockPlacementPass(PassRegistry&);
void initializeMachineBlockPlacementStatsPass(PassRegistry&);
void initializeMachineBranchProbabilityInfoPass(PassRegistry&);
void initializeMachineCSEPass(PassRegistry&);
void initializeMachineCombinerPass(PassRegistry&);
void initializeMachineCopyPropagationPass(PassRegistry&);
void initializeMachineCycleInfoPrinterPassPass(PassRegistry &);
void initializeMachineCycleInfoWrapperPassPass(PassRegistry &);
void initializeMachineDominanceFrontierPass(PassRegistry&);
void initializeMachineDominatorTreePass(PassRegistry&);
void initializeMachineFunctionPrinterPassPass(PassRegistry&);
void initializeMachineFunctionSplitterPass(PassRegistry &);
void initializeMachineLICMPass(PassRegistry&);
void initializeMachineLoopInfoPass(PassRegistry&);
void initializeMachineModuleInfoWrapperPassPass(PassRegistry &);
void initializeMachineOptimizationRemarkEmitterPassPass(PassRegistry&);
void initializeMachineOutlinerPass(PassRegistry&);
void initializeMachinePipelinerPass(PassRegistry&);
void initializeMachinePostDominatorTreePass(PassRegistry&);
void initializeMachineRegionInfoPassPass(PassRegistry&);
void initializeMachineSchedulerPass(PassRegistry&);
void initializeMachineSinkingPass(PassRegistry&);
void initializeMachineTraceMetricsPass(PassRegistry&);
void initializeMachineVerifierPassPass(PassRegistry&);
void initializeMemCpyOptLegacyPassPass(PassRegistry&);
void initializeMemDepPrinterPass(PassRegistry&);
void initializeMemDerefPrinterPass(PassRegistry&);
void initializeMemoryDependenceWrapperPassPass(PassRegistry&);
void initializeMemorySSAPrinterLegacyPassPass(PassRegistry&);
void initializeMemorySSAWrapperPassPass(PassRegistry&);
void initializeMergeFunctionsLegacyPassPass(PassRegistry&);
void initializeMergeICmpsLegacyPassPass(PassRegistry &);
void initializeMergedLoadStoreMotionLegacyPassPass(PassRegistry&);
void initializeMetaRenamerPass(PassRegistry&);
void initializeModuleDebugInfoLegacyPrinterPass(PassRegistry &);
void initializeModuleMemProfilerLegacyPassPass(PassRegistry &);
void initializeModuleSummaryIndexWrapperPassPass(PassRegistry&);
void initializeModuloScheduleTestPass(PassRegistry&);
void initializeMustExecutePrinterPass(PassRegistry&);
void initializeMustBeExecutedContextPrinterPass(PassRegistry&);
void initializeNaryReassociateLegacyPassPass(PassRegistry&);
void initializeNewGVNLegacyPassPass(PassRegistry&);
void initializeObjCARCAAWrapperPassPass(PassRegistry&);
void initializeObjCARCAPElimPass(PassRegistry&);
void initializeObjCARCContractLegacyPassPass(PassRegistry &);
void initializeObjCARCExpandPass(PassRegistry&);
void initializeObjCARCOptLegacyPassPass(PassRegistry &);
void initializeOptimizationRemarkEmitterWrapperPassPass(PassRegistry&);
void initializeOptimizePHIsPass(PassRegistry&);
void initializePAEvalPass(PassRegistry&);
void initializePEIPass(PassRegistry&);
#if INTEL_CUSTOMIZATION
void initializePGOIndirectCallPromotionLegacyPassPass(PassRegistry&);
void initializePGOInstrumentationGenLegacyPassPass(PassRegistry&);
void initializePGOInstrumentationUseLegacyPassPass(PassRegistry&);
void initializePGOInstrumentationGenCreateVarLegacyPassPass(PassRegistry&);
#endif // INTEL_CUSTOMIZATION
void initializePHIEliminationPass(PassRegistry&);
void initializePartialInlinerLegacyPassPass(PassRegistry&);
void initializePartiallyInlineLibCallsLegacyPassPass(PassRegistry&);
void initializePatchableFunctionPass(PassRegistry&);
void initializePeepholeOptimizerPass(PassRegistry&);
void initializePhiValuesWrapperPassPass(PassRegistry&);
void initializePhysicalRegisterUsageInfoPass(PassRegistry&);
void initializePlaceBackedgeSafepointsImplPass(PassRegistry&);
void initializePlaceSafepointsPass(PassRegistry&);
void initializePostDomOnlyPrinterWrapperPassPass(PassRegistry &);
void initializePostDomOnlyViewerWrapperPassPass(PassRegistry &);
void initializePostDomPrinterWrapperPassPass(PassRegistry &);
void initializePostDomViewerWrapperPassPass(PassRegistry &);
void initializePostDominatorTreeWrapperPassPass(PassRegistry&);
void initializePostMachineSchedulerPass(PassRegistry&);
void initializePostOrderFunctionAttrsLegacyPassPass(PassRegistry&);
void initializePostRAHazardRecognizerPass(PassRegistry&);
void initializePostRAMachineSinkingPass(PassRegistry&);
void initializePostRASchedulerPass(PassRegistry&);
void initializePreISelIntrinsicLoweringLegacyPassPass(PassRegistry&);
void initializePredicateInfoPrinterLegacyPassPass(PassRegistry&);
void initializePrintFunctionPassWrapperPass(PassRegistry&);
void initializePrintModulePassWrapperPass(PassRegistry&);
void initializeProcessImplicitDefsPass(PassRegistry&);
void initializeProfileSummaryInfoWrapperPassPass(PassRegistry&);
void initializePromoteLegacyPassPass(PassRegistry&);
void initializePruneEHPass(PassRegistry&);
void initializeRABasicPass(PassRegistry&);
void initializePseudoProbeInserterPass(PassRegistry &);
#if INTEL_CUSTOMIZATION
#if INTEL_FEATURE_MARKERCOUNT
void initializePseudoMarkerCountInserterPass(PassRegistry &);
#endif // INTEL_FEATURE_MARKERCOUNT
#endif // INTEL_CUSTOMIZATION
void initializeRAGreedyPass(PassRegistry&);
void initializeReachingDefAnalysisPass(PassRegistry&);
void initializeReassociateLegacyPassPass(PassRegistry&);
void initializeRedundantDbgInstEliminationPass(PassRegistry&);
void initializeRegAllocEvictionAdvisorAnalysisPass(PassRegistry &);
void initializeRegAllocFastPass(PassRegistry&);
void initializeRegAllocScoringPass(PassRegistry &);
void initializeRegBankSelectPass(PassRegistry&);
void initializeRegToMemLegacyPass(PassRegistry&);
void initializeRegUsageInfoCollectorPass(PassRegistry&);
void initializeRegUsageInfoPropagationPass(PassRegistry&);
void initializeRegionInfoPassPass(PassRegistry&);
void initializeRegionOnlyPrinterPass(PassRegistry&);
void initializeRegionOnlyViewerPass(PassRegistry&);
void initializeRegionPrinterPass(PassRegistry&);
void initializeRegionViewerPass(PassRegistry&);
void initializeRegisterCoalescerPass(PassRegistry&);
void initializeRemoveRedundantDebugValuesPass(PassRegistry&);
void initializeRenameIndependentSubregsPass(PassRegistry&);
void initializeReplaceWithVeclibLegacyPass(PassRegistry &);
void initializeResetMachineFunctionPass(PassRegistry&);
void initializeReversePostOrderFunctionAttrsLegacyPassPass(PassRegistry&);
void initializeRewriteStatepointsForGCLegacyPassPass(PassRegistry &);
void initializeRewriteSymbolsLegacyPassPass(PassRegistry&);
void initializeSCCPLegacyPassPass(PassRegistry&);
void initializeSCEVAAWrapperPassPass(PassRegistry&);
void initializeSLPVectorizerPass(PassRegistry&);
void initializeSROALegacyPassPass(PassRegistry&);
void initializeSROALegacyCGSCCAdaptorPassPass(PassRegistry &); // INTEL
void initializeSafeStackLegacyPassPass(PassRegistry&);
void initializeSafepointIRVerifierPass(PassRegistry&);
void initializeSelectOptimizePass(PassRegistry &);
void initializeScalarEvolutionWrapperPassPass(PassRegistry&);
void initializeScalarizeMaskedMemIntrinLegacyPassPass(PassRegistry &);
void initializeScalarizerLegacyPassPass(PassRegistry&);
void initializeScavengerTestPass(PassRegistry&);
void initializeScopedNoAliasAAWrapperPassPass(PassRegistry&);
void initializeSeparateConstOffsetFromGEPLegacyPassPass(PassRegistry &);
void initializeShadowStackGCLoweringPass(PassRegistry&);
void initializeShrinkWrapPass(PassRegistry&);
void initializeSimpleInlinerPass(PassRegistry&);
void initializeSimpleLoopUnswitchLegacyPassPass(PassRegistry&);
void initializeSingleLoopExtractorPass(PassRegistry&);
void initializeSinkingLegacyPassPass(PassRegistry&);
void initializeSjLjEHPreparePass(PassRegistry&);
void initializeSlotIndexesPass(PassRegistry&);
void initializeSpeculativeExecutionLegacyPassPass(PassRegistry&);
void initializeSpillPlacementPass(PassRegistry&);
void initializeStackColoringPass(PassRegistry&);
void initializeStackMapLivenessPass(PassRegistry&);
void initializeStackProtectorPass(PassRegistry&);
void initializeStackSafetyGlobalInfoWrapperPassPass(PassRegistry &);
void initializeStackSafetyInfoWrapperPassPass(PassRegistry &);
void initializeStackSlotColoringPass(PassRegistry&);
void initializeStraightLineStrengthReduceLegacyPassPass(PassRegistry &);
void initializeStripDeadDebugInfoPass(PassRegistry&);
void initializeStripDeadPrototypesLegacyPassPass(PassRegistry&);
void initializeStripDebugDeclarePass(PassRegistry&);
void initializeStripDebugMachineModulePass(PassRegistry &);
void initializeStripGCRelocatesLegacyPass(PassRegistry &);
void initializeStripNonDebugSymbolsPass(PassRegistry&);
void initializeStripNonLineTableDebugLegacyPassPass(PassRegistry &);
void initializeStripSymbolsPass(PassRegistry&);
void initializeStructurizeCFGLegacyPassPass(PassRegistry &);
void initializeSYCLLowerWGScopeLegacyPassPass(PassRegistry &);
void initializeSYCLLowerESIMDLegacyPassPass(PassRegistry &);
void initializeSYCLLowerInvokeSimdLegacyPassPass(PassRegistry &);
void initializeSYCLMutatePrintfAddrspaceLegacyPassPass(PassRegistry &);
void initializeSPIRITTAnnotationsLegacyPassPass(PassRegistry &);
void initializeESIMDLowerLoadStorePass(PassRegistry &);
void initializeESIMDLowerVecArgLegacyPassPass(PassRegistry &);
void initializeESIMDVerifierPass(PassRegistry &);
void initializeSYCLLowerWGLocalMemoryLegacyPass(PassRegistry &);
void initializeTailCallElimPass(PassRegistry&);
void initializeTailDuplicatePass(PassRegistry&);
void initializeTargetLibraryInfoWrapperPassPass(PassRegistry&);
void initializeTargetPassConfigPass(PassRegistry&);
void initializeTargetTransformInfoWrapperPassPass(PassRegistry&);
void initializeThreadSanitizerLegacyPassPass(PassRegistry&); // INTEL
void initializeTLSVariableHoistLegacyPassPass(PassRegistry &);
void initializeTwoAddressInstructionPassPass(PassRegistry&);
void initializeTypeBasedAAWrapperPassPass(PassRegistry&);
void initializeTypePromotionPass(PassRegistry&);
void initializeUnifyFunctionExitNodesLegacyPassPass(PassRegistry &);
void initializeUnifyLoopExitsLegacyPassPass(PassRegistry &);
void initializeUnpackMachineBundlesPass(PassRegistry&);
void initializeUnreachableBlockElimLegacyPassPass(PassRegistry&);
void initializeUnreachableMachineBlockElimPass(PassRegistry&);
void initializeVectorCombineLegacyPassPass(PassRegistry&);
void initializeVerifierLegacyPassPass(PassRegistry&);
void initializeVirtRegMapPass(PassRegistry&);
void initializeVirtRegRewriterPass(PassRegistry&);
void initializeWarnMissedTransformationsLegacyPass(PassRegistry &);
void initializeWasmEHPreparePass(PassRegistry&);
void initializeWinEHPreparePass(PassRegistry&);
void initializeWriteBitcodePassPass(PassRegistry&);
void initializeWriteThinLTOBitcodePass(PassRegistry&);
void initializeXRayInstrumentationPass(PassRegistry&);
#if INTEL_CUSTOMIZATION
// Pass for alias analysis for STL templates
void initializeStdContainerAAWrapperPassPass(PassRegistry &);
// Pass for alias metadata propagation
void initializeStdContainerOptLegacyPassPass(PassRegistry &);
// Pass for TBAA metadata propagation
void initializeTbaaMDPropagationLegacyPassPass(PassRegistry &);
// Pass for removing fakeload intrinisics
void initializeCleanupFakeLoadsLegacyPassPass(PassRegistry &);
#if INTEL_CUSTOMIZATION
#if INTEL_FEATURE_SW_ADVANCED
// Pass for function recognition
void initializeFunctionRecognizerLegacyPassPass(PassRegistry &);
#endif // INTEL_FEATURE_SW_ADVANCED
#endif // INTEL_CUSTOMIZATION
// Pass for handling '#pragma vector aligned'.
void initializeHandlePragmaVectorAlignedLegacyPassPass(PassRegistry &);
// Pass for indirect call conversion using points-to info
void initializeIndirectCallConvLegacyPassPass(PassRegistry &);
// Pass for SnodeInfo analysis
void initializeSNodeAnalysisPass(PassRegistry &);
// Pass for register promotion for non escaped block scope global variables.
void initializeNonLTOGlobalOptLegacyPassPass(PassRegistry &);
// Pass for math call optimization.
void initializeMapIntrinToImlPass(PassRegistry&);
// Pass for indicating loopopt based throttling.
void initializeLoopOptMarkerLegacyPassPass(PassRegistry&);
// Pass to store the opt level.
void initializeXmainOptLevelWrapperPassPass(PassRegistry&);
void initializeOptReportOptionsPassPass(PassRegistry &);
// Pass for removing region directives.
void initializeRemoveRegionDirectivesLegacyPassPass(PassRegistry &);
void initializeOptReportEmitterLegacyPassPass(PassRegistry&);
// Pass for loop carried CSE
void initializeLoopCarriedCSELegacyPass(PassRegistry&);
// Pass for transforming __fpga_reg builtin representation
void initializeTransformFPGARegPass(PassRegistry &);
#if INTEL_CUSTOMIZATION
#if INTEL_FEATURE_SW_ADVANCED
// Qsort recognition
void initializeQsortRecognizerLegacyPassPass(PassRegistry&);
#endif // INTEL_FEATURE_SW_ADVANCED
#endif // INTEL_CUSTOMIZATION
// Multiversioning and inline marking for tiled functions
void initializeTileMVInlMarkerLegacyPassPass(PassRegistry&);
// Mark callsites for aggressive inlining
void initializeAggInlinerLegacyPassPass(PassRegistry&);
// Hoist base addresses of Dope Vector
void initializeDopeVectorHoistWrapperPass(PassRegistry&);
// Converting annotation attribute to function attribute
void initializeParseAnnotateAttributesLegacyPass(PassRegistry&);
// HIR Passes
void initializeHIRRegionIdentificationWrapperPassPass(PassRegistry&);
void initializeHIRSCCFormationWrapperPassPass(PassRegistry&);
void initializeHIRFrameworkWrapperPassPass(PassRegistry&);
void initializeHIROptReportEmitterWrapperPassPass(PassRegistry&);
void initializeHIRDDAnalysisWrapperPassPass(PassRegistry&);
void initializeHIRLoopLocalityWrapperPassPass(PassRegistry&);
void initializeHIRLoopResourceWrapperPassPass(PassRegistry&);
void initializeHIRSafeReductionAnalysisWrapperPassPass(PassRegistry&);
void initializeHIRSparseArrayReductionAnalysisWrapperPassPass(PassRegistry&);
void initializeHIRArraySectionAnalysisWrapperPassPass(PassRegistry&);
void initializeHIRLoopStatisticsWrapperPassPass(PassRegistry&);
void initializeHIRSSADeconstructionLegacyPassPass(PassRegistry&);
void initializeHIRTempCleanupLegacyPassPass(PassRegistry&);
void initializeHIRParVecAnalysisWrapperPassPass(PassRegistry &);
void initializeHIRParDirInsertPass(PassRegistry&);
void initializeHIRVecDirInsertPass(PassRegistry&);
void initializeHIRPrinterPass(llvm::PassRegistry&);
void initializeHIRPreVecCompleteUnrollLegacyPassPass(PassRegistry&);
void initializeHIRPostVecCompleteUnrollLegacyPassPass(PassRegistry&);
void initializeHIRLoopInterchangeLegacyPassPass(PassRegistry&);
void initializeHIRLoopBlockingLegacyPassPass(PassRegistry&);
void initializeHIRPragmaLoopBlockingLegacyPassPass(PassRegistry&);
void initializeHIRGenerateMKLCallLegacyPassPass(PassRegistry&);
void initializeHIRLoopDistributionForMemRecLegacyPassPass(PassRegistry&);
void initializeHIRLoopDistributionForLoopNestLegacyPassPass(PassRegistry&);
void initializeHIRLoopRematerializeLegacyPassPass(PassRegistry&);
void initializeHIRLoopRerollLegacyPassPass(PassRegistry&);
void initializeHIRGeneralUnrollLegacyPassPass(PassRegistry&);
void initializeHIRUnrollAndJamLegacyPassPass(PassRegistry&);
void initializeHIROptPredicateLegacyPassPass(PassRegistry&);
void initializeHIRRuntimeDDLegacyPassPass(PassRegistry&);
void initializeHIRLoopReversalLegacyPassPass(PassRegistry&);
void initializeHIRLMMLegacyPassPass(PassRegistry&);
void initializeHIRLoopCollapseLegacyPassPass(PassRegistry&);
void initializeHIRPMSymbolicTripCountCompleteUnrollLegacyPassPass(PassRegistry&);
void initializeHIRScalarReplArrayLegacyPassPass(PassRegistry&);
void initializeHIRDummyTransformationPass(PassRegistry&);
void initializeHIRCodeGenWrapperPassPass(PassRegistry&);
void initializeHIROptVarPredicateLegacyPassPass(PassRegistry&);
void initializeHIRIdiomRecognitionLegacyPassPass(PassRegistry&);
void initializeHIRMVForConstUBLegacyPassPass(PassRegistry&);
void initializeHIRMVForVariableStrideLegacyPassPass(PassRegistry&);
void initializeHIRLoopConcatenationLegacyPassPass(PassRegistry&);
void initializeHIRArrayTransposeLegacyPassPass(PassRegistry&);
void initializeHIRAosToSoaLegacyPassPass(PassRegistry&);
void initializeHIRLoopFusionLegacyPassPass(PassRegistry&);
void initializeHIRDeadStoreEliminationLegacyPassPass(PassRegistry&);
void initializeHIRLastValueComputationLegacyPassPass(PassRegistry&);
void initializeHIRPropagateCastedIVLegacyPassPass(PassRegistry&);
void initializeHIRMultiExitLoopRerollLegacyPassPass(PassRegistry&);
void initializeHIRRecognizeParLoopPass(PassRegistry&);
void initializeHIRMinMaxRecognitionLegacyPassPass(PassRegistry &);
void initializeHIRIdentityMatrixIdiomRecognitionLegacyPassPass(PassRegistry&);
void initializeHIRPrefetchingLegacyPassPass(PassRegistry&);
void initializeHIRSinkingForPerfectLoopnestLegacyPassPass(PassRegistry&);
void initializeHIRUndoSinkingForPerfectLoopnestLegacyPassPass(PassRegistry&);
void initializeHIRConditionalTempSinkingLegacyPassPass(PassRegistry&);
void initializeHIRMemoryReductionSinkingLegacyPassPass(PassRegistry&);
void initializeHIRRowWiseMVLegacyPassPass(PassRegistry &);
void initializeHIRConditionalLoadStoreMotionLegacyPassPass(PassRegistry &);
void initializeHIRNontemporalMarkingLegacyPassPass(PassRegistry &);
void initializeHIRStoreResultIntoTempArrayLegacyPassPass(PassRegistry&);
void initializeHIRSumWindowReuseLegacyPassPass(PassRegistry &);
void initializeHIRNonZeroSinkingForPerfectLoopnestLegacyPassPass(PassRegistry&);
void initializeHIRIdentityMatrixSubstitutionLegacyPassPass(PassRegistry&);
void initializeHIRArrayScalarizationTestLauncherLegacyPassPass(PassRegistry &);
void initializeHIRIfReversalLegacyPassPass(PassRegistry &);
#if INTEL_FEATURE_SW_ADVANCED
void initializeHIRInterLoopBlockingLegacyPassPass(PassRegistry &);
void initializeHIRCrossLoopArrayContractionLegacyPassPass(PassRegistry&);
#endif // INTEL_FEATURE_SW_ADVANCED
// VPO Vectorizer Passes
void initializeAVRGeneratePass(PassRegistry&);
void initializeAVRGenerateHIRPass(PassRegistry&);
void initializeAVRDecomposeHIRPass(PassRegistry&);
void initializeVPOPredicatorPass(PassRegistry&);
void initializeVPOPredicatorHIRPass(PassRegistry&);
void initializeVecClonePass(PassRegistry&);
void initializeAvrDefUsePass(PassRegistry&);
void initializeAvrDefUseHIRPass(PassRegistry&);
void initializeAvrCFGPass(PassRegistry&);
void initializeAvrCFGHIRPass(PassRegistry&);
void initializeSIMDLaneEvolutionPass(PassRegistry&);
void initializeSIMDLaneEvolutionHIRPass(PassRegistry&);
void initializeVPODriverPass(PassRegistry&);
void initializeVPODriverHIRPass(PassRegistry&);
void initializeVPODirectiveCleanupPass(PassRegistry&);
void initializeVectorGraphInfoPass(PassRegistry&);
void initializeVectorGraphPredicatorPass(PassRegistry&);
void initializeWholeProgramWrapperPassPass(PassRegistry&);
void initializeMultiVersioningWrapperPass(PassRegistry&);
// VPO VPlan Pass for pragma omp ordered simd
void initializeVPlanPragmaOmpOrderedSimdExtractPass(PassRegistry &);
// VPO VPlan Pass for pragma omp simd if
void initializeVPlanPragmaOmpSimdIfPass(PassRegistry &);
// VPO VPlan Vectorizer Pass  --  TODO: VEC to COLLAB
void initializeVPlanDriverPass(PassRegistry&);
// VPO VPlan Vectorizer HIR Pass
void initializeVPlanDriverHIRPass(PassRegistry&);
void initializeVPlanFunctionVectorizerLegacyPassPass(PassRegistry&);
// OpenCL Passes
void initializeFMASplitterLegacyPassPass(PassRegistry&);
// Pass for dynamic_cast calls optimization
void initializeOptimizeDynamicCastsWrapperPass(PassRegistry&);
void initializeMachineOptReportEmitterPass(PassRegistry&);
// DPCPP Kernel Transformation passes
void initializeAddFastMathLegacyPass(PassRegistry &);
void initializeAddFunctionAttrsLegacyPass(PassRegistry &);
void initializeAddImplicitArgsLegacyPass(PassRegistry &);
void initializeAddNTAttrLegacyPass(PassRegistry &);
void initializeAddTLSGlobalsLegacyPass(PassRegistry &);
void initializeAutorunReplicatorLegacyPass(PassRegistry &);
void initializeBarrierInFunctionLegacyPass(PassRegistry &);
void initializeBuiltinCallToInstLegacyPass(PassRegistry &);
void initializeBuiltinImportLegacyPass(PassRegistry &);
void initializeBuiltinLICMLegacyPass(PassRegistry &);
void initializeBuiltinLibInfoAnalysisLegacyPass(PassRegistry &);
void initializeChannelPipeTransformationLegacyPass(PassRegistry &);
void initializeCleanupWrappedKernelLegacyPass(PassRegistry &);
void initializeCoerceTypesLegacyPass(PassRegistry &);
void initializeCoerceWin64TypesLegacyPass(PassRegistry &);
void initializeCreateSimdVariantPropagationLegacyPass(PassRegistry &);
void initializeDataPerBarrierWrapperPass(PassRegistry &);
void initializeDataPerValueWrapperPass(PassRegistry &);
void initializeDetectRecursionLegacyPass(PassRegistry &);
void initializeDPCPPAliasAnalysisLegacyPass(PassRegistry &);
void initializeDPCPPExternalAliasAnalysisLegacyPass(PassRegistry &);
void initializeDPCPPEqualizerLegacyPass(PassRegistry &);
void initializeDPCPPKernelAnalysisLegacyPass(PassRegistry &);
void initializeDPCPPKernelPostVecPass(PassRegistry&);
void initializeDPCPPKernelVecCloneLegacyPass(PassRegistry &);
void initializeDPCPPKernelWGLoopCreatorLegacyPass(PassRegistry &);
void initializeDPCPPPreprocessSPIRVFriendlyIRLegacyPass(PassRegistry &);
void initializeDPCPPRewritePipesLegacyPass(PassRegistry &);
void initializeDeduceMaxWGDimLegacyPass(PassRegistry &);
void initializeDuplicateCalledKernelsLegacyPass(PassRegistry &);
void initializeExternalizeGlobalVariablesLegacyPass(PassRegistry &);
void initializeGroupBuiltinLegacyPass(PassRegistry &);
void initializeHandleVPlanMaskLegacyPass(PassRegistry &);
void initializeImplicitArgsAnalysisLegacyPass(PassRegistry &);
void initializeImplicitGIDLegacyPass(PassRegistry &);
void initializeIndirectCallLoweringLegacyPass(PassRegistry &);
void initializeInferArgumentAliasLegacyPass(PassRegistry &);
void initializeInfiniteLoopCreatorLegacyPass(PassRegistry &);
void initializeInstToFuncCallLegacyPass(PassRegistry &);
void initializeInternalizeGlobalVariablesLegacyPass(PassRegistry &);
void initializeInternalizeNonKernelFuncLegacyPass(PassRegistry &);
void initializeKernelBarrierLegacyPass(PassRegistry &);
void initializeLinearIdResolverLegacyPass(PassRegistry &);
void initializeLocalBufferAnalysisLegacyPass(PassRegistry &);
void initializeLocalBuffersLegacyPass(PassRegistry &);
void initializeLoopStridedCodeMotionLegacyPass(PassRegistry &);
void initializeLoopWIAnalysisLegacyPass(PassRegistry &);
void initializeOptimizeIDivAndIRemLegacyPass(PassRegistry &);
void initializePatchCallbackArgsLegacyPass(PassRegistry &);
void initializePhiCanonicalizationLegacyPass(PassRegistry &);
void initializePipeIOTransformationLegacyPass(PassRegistry &);
void initializePipeOrderingLegacyPass(PassRegistry &);
void initializePipeSupportLegacyPass(PassRegistry &);
void initializePrepareKernelArgsLegacyPass(PassRegistry &);
void initializePreventDivCrashesLegacyPass(PassRegistry &);
void initializeProfilingInfoLegacyPass(PassRegistry &);
void initializeReduceCrossBarrierValuesLegacyPass(PassRegistry &);
void initializeRedundantPhiNodeLegacyPass(PassRegistry &);
void initializeRelaxedMathLegacyPass(PassRegistry &);
void initializeRemoveAtExitLegacyPass(PassRegistry &);
void initializeRemoveDuplicatedBarrierLegacyPass(PassRegistry &);
void initializeReplaceScalarWithMaskLegacyPass(PassRegistry &);
void initializeReqdSubGroupSizeLegacyPass(PassRegistry &);
void initializeResolveMatrixFillLegacyPass(PassRegistry &);
void initializeResolveMatrixLayoutLegacyPass(PassRegistry &);
void initializeResolveMatrixWISliceLegacyPass(PassRegistry &);
void initializeResolveSubGroupWICallLegacyPass(PassRegistry &);
void initializeResolveVarTIDCallLegacyPass(PassRegistry &);
void initializeResolveWICallLegacyPass(PassRegistry &);
void initializeSetPreferVectorWidthLegacyPass(PassRegistry &);
void initializeSetVectorizationFactorLegacyPass(PassRegistry &);
void initializeSGBarrierPropagateLegacyPass(PassRegistry &);
void initializeSGBarrierSimplifyLegacyPass(PassRegistry &);
void initializeSGBuiltinLegacyPass(PassRegistry &);
void initializeSGLoopConstructLegacyPass(PassRegistry &);
void initializeSGSizeAnalysisLegacyPass(PassRegistry &);
void initializeSGSizeCollectorLegacyPass(PassRegistry &);
void initializeSGSizeCollectorIndirectLegacyPass(PassRegistry &);
void initializeSGValueWidenLegacyPass(PassRegistry &);
void initializeSinCosFoldLegacyPass(PassRegistry &);
void initializeSoaAllocaAnalysisLegacyPass(PassRegistry &);
void initializeSplitBBonBarrierLegacyPass(PassRegistry &);
void initializeTaskSeqAsyncHandlingLegacyPass(PassRegistry &);
void initializeUpdateCallAttrsLegacyPass(PassRegistry &);
void initializeVectorVariantFillInLegacyPass(PassRegistry &);
void initializeVectorVariantLoweringLegacyPass(PassRegistry &);
void initializeVFAnalysisLegacyPass(PassRegistry &);
void initializeVectorKernelEliminationLegacyPass(PassRegistry &);
void initializeVectorizationDimensionAnalysisLegacyPass(PassRegistry &);
void initializeWGLoopBoundariesLegacyPass(PassRegistry &);
void initializeWeightedInstCountAnalysisLegacyPass(PassRegistry &);
void initializeWorkItemAnalysisLegacyPass(PassRegistry &);
void initializeWIRelatedValueWrapperPass(PassRegistry &);
// Add/Sub reassociation pass
void initializeAddSubReassociateLegacyPassPass(PassRegistry&);
// Forced CMOV generation pass
void initializeForcedCMOVGenerationLegacyPassPass(PassRegistry&);
void initializeRAReportEmitterPass(PassRegistry&);
void initializeVPOParoptOptimizeDataSharingPass(PassRegistry&);
#if INTEL_FEATURE_SW_ADVANCED
void initializeNontemporalStoreWrapperPassPass(PassRegistry&);
#endif // INTEL_FEATURE_SW_ADVANCED
void initializeVPOParoptSharedPrivatizationPass(PassRegistry&);
void initializeVPOParoptTargetInlinePass(PassRegistry&);
void initializeVPOParoptApplyConfigPass(PassRegistry&);
// Transform sin and cos to sinpi, cospi, or sincospi pass
void initializeTransformSinAndCosCallsLegacyPassPass(PassRegistry&);
// Add attributes to loops
void initializeIntelLoopAttrsWrapperPass(PassRegistry&);
// Add math function declarations
void initializeIntelMathLibrariesDeclarationWrapperPass(PassRegistry&);
// Simplifed dead argument elimination with IPO analysis
void initializeIntelIPODeadArgEliminationWrapperPass(PassRegistry &);
#endif // INTEL_CUSTOMIZATION

#if INTEL_COLLAB
// VPO WRegion Passes
void initializeWRegionCollectionWrapperPassPass(PassRegistry&);
void initializeWRegionInfoWrapperPassPass(PassRegistry&);
void initializeWRegionInfoAnalysisPass(PassRegistry&);
// VPO Utility Passes
void initializeVPOCFGRestructuringPass(PassRegistry&);
void initializeVPOCFGSimplifyPass(PassRegistry&);
// VPO Paropt Loop Collapse Pass
void initializeVPOParoptLoopCollapsePass(PassRegistry&);
// VPO Paropt Loop Transform Pass
void initializeVPOParoptLoopTransformPass(PassRegistry&);
// VPO Paropt Prepare Pass
void initializeVPOParoptPreparePass(PassRegistry&);
// VPO Pass to restore operands renamed by VPO Paropt Prepare pass
void initializeVPORestoreOperandsPass(PassRegistry &);
// VPO Parallelizer Pass
void initializeVPOParoptPass(PassRegistry&);
// VPO Tpv Transformation
void initializeVPOParoptTpvPass(PassRegistry&);
// VPO Paropt LowerSimd Pass
void initializeVPOParoptLowerSimdPass(PassRegistry&);

#endif // INTEL_COLLAB

#if INTEL_CUSTOMIZATION
void initializeLoadCoalescingLegacyPassPass(PassRegistry &);
void initializeMathLibraryFunctionsReplacementLegacyPassPass(PassRegistry &);
void initializeVPOParoptConfigWrapperPass(PassRegistry &);
void initializeIntelVTableFixupLegacyPassPass(PassRegistry &);
void initializeAutoCPUCloneLegacyPassPass(PassRegistry&);
#endif // INTEL_CUSTOMIZATION

} // end namespace llvm

#endif // LLVM_INITIALIZEPASSES_H
