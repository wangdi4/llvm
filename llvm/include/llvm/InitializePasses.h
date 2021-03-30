//===- llvm/InitializePasses.h - Initialize All Passes ----------*- C++ -*-===//
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

/// Initialize all passes linked into the Coroutines library.
void initializeCoroutines(PassRegistry&);

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
void initializeModuleAddressSanitizerLegacyPassPass(PassRegistry &);
void initializeASanGlobalsMetadataWrapperPassPass(PassRegistry &);
void initializeAddressSanitizerLegacyPassPass(PassRegistry &);
void initializeAggressiveInstCombinerLegacyPassPass(PassRegistry&);
void initializeAliasSetPrinterPass(PassRegistry&);
void initializeAlignmentFromAssumptionsPass(PassRegistry&);
void initializeAlwaysInlinerLegacyPassPass(PassRegistry&);
void initializeAndersensAAWrapperPassPass(PassRegistry&); // INTEL
void initializeAssumeSimplifyPassLegacyPassPass(PassRegistry &);
void initializeAssumeBuilderPassLegacyPassPass(PassRegistry &);
void initializeAnnotation2MetadataLegacyPass(PassRegistry &);
void initializeAnnotationRemarksLegacyPass(PassRegistry &);
void initializeOpenMPOptLegacyPassPass(PassRegistry &);
void initializeArgNoAliasPropPass(PassRegistry &); // INTEL
void initializeArgPromotionPass(PassRegistry&);
void initializeArrayUseWrapperPassPass(PassRegistry&); // INTEL
void initializeAssumptionCacheTrackerPass(PassRegistry&);
void initializeAtomicExpandPass(PassRegistry&);
void initializeAttributorLegacyPassPass(PassRegistry&);
void initializeAttributorCGSCCLegacyPassPass(PassRegistry &);
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
void initializeCanonicalizeAliasesLegacyPassPass(PassRegistry &);
void initializeCanonicalizeFreezeInLoopsPass(PassRegistry &);
void initializeCFGOnlyPrinterLegacyPassPass(PassRegistry&);
void initializeCFGOnlyViewerLegacyPassPass(PassRegistry&);
void initializeCFGPrinterLegacyPassPass(PassRegistry&);
void initializeCFGSimplifyPassPass(PassRegistry&);
void initializeCFGuardPass(PassRegistry&);
void initializeCFGuardLongjmpPass(PassRegistry&);
void initializeCFGViewerLegacyPassPass(PassRegistry&);
void initializeCFIInstrInserterPass(PassRegistry&);
void initializeCFLAndersAAWrapperPassPass(PassRegistry&);
void initializeCFLSteensAAWrapperPassPass(PassRegistry&);
void initializeCGProfileLegacyPassPass(PassRegistry &);
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
void initializeControlHeightReductionLegacyPassPass(PassRegistry&);
void initializeConvertGEPToSubscriptIntrinsicLegacyPassPass(PassRegistry &); // INTEL
void initializeCorrelatedValuePropagationPass(PassRegistry&);
void initializeCostModelAnalysisPass(PassRegistry&);
void initializeCrossDSOCFIPass(PassRegistry&);
void initializeDAEPass(PassRegistry&);
void initializeDAHPass(PassRegistry&);
void initializeDAESYCLPass(PassRegistry&);
void initializeDCELegacyPassPass(PassRegistry&);
void initializeDSELegacyPassPass(PassRegistry&);
void initializeDataFlowSanitizerLegacyPassPass(PassRegistry &);
void initializeDeadArrayOpsEliminationLegacyPassPass(PassRegistry&); // INTEL
void initializeDeadMachineInstructionElimPass(PassRegistry&);
void initializeDebugifyMachineModulePass(PassRegistry &);
void initializeDelinearizationPass(PassRegistry&);
void initializeDemandedBitsWrapperPassPass(PassRegistry&);
void initializeDependenceAnalysisPass(PassRegistry&);
void initializeDependenceAnalysisWrapperPassPass(PassRegistry&);
void initializeDopeVectorConstPropLegacyPassPass(PassRegistry&);   // INTEL
void initializeDetectDeadLanesPass(PassRegistry&);
void initializeDivRemPairsLegacyPassPass(PassRegistry&);
void initializeDomOnlyPrinterPass(PassRegistry&);
void initializeDomOnlyViewerPass(PassRegistry&);
void initializeDomPrinterPass(PassRegistry&);
void initializeDomViewerPass(PassRegistry&);
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
void initializeEliminateAvailableExternallyLegacyPassPass(PassRegistry&);
void initializeEntryExitInstrumenterPass(PassRegistry&);
void initializeExpandMemCmpPassPass(PassRegistry&);
void initializeExpandPostRAPass(PassRegistry&);
void initializeExpandReductionsPass(PassRegistry&);
void initializeMakeGuardsExplicitLegacyPassPass(PassRegistry&);
void initializeExternalAAWrapperPassPass(PassRegistry&);
void initializeFEntryInserterPass(PassRegistry&);
void initializeFinalizeISelPass(PassRegistry&);
void initializeFinalizeMachineBundlesPass(PassRegistry&);
void initializeFixIrreduciblePass(PassRegistry &);
void initializeFixupStatepointCallerSavedPass(PassRegistry&);
void initializeFlattenCFGPassPass(PassRegistry&);
void initializeFloat128ExpandPass(PassRegistry&); // INTEL
void initializeFloat2IntLegacyPassPass(PassRegistry&);
void initializeForceFunctionAttrsLegacyPassPass(PassRegistry&);
void initializeForwardControlFlowIntegrityPass(PassRegistry&);
void initializeFuncletLayoutPass(PassRegistry&);
void initializeFunctionImportLegacyPassPass(PassRegistry&);
void initializeFunctionSplittingWrapperPass(PassRegistry&);        // INTEL
void initializeGCMachineCodeAnalysisPass(PassRegistry&);
void initializeGCModuleInfoPass(PassRegistry&);
void initializeGCOVProfilerLegacyPassPass(PassRegistry&);
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
void initializeMemProfilerLegacyPassPass(PassRegistry &);
void initializeHotColdSplittingLegacyPassPass(PassRegistry&);
void initializeHWAddressSanitizerLegacyPassPass(PassRegistry &);
void initializeIPSCCPLegacyPassPass(PassRegistry&);
void initializeIRCELegacyPassPass(PassRegistry&);
void initializeIROutlinerLegacyPassPass(PassRegistry&);
void initializeIRSimilarityIdentifierWrapperPassPass(PassRegistry&);
void initializeIRTranslatorPass(PassRegistry&);
void initializeIPArrayTransposeLegacyPassPass(PassRegistry &); // INTEL
void initializeIPCloningLegacyPassPass(PassRegistry&);             // INTEL
void initializeCallTreeCloningLegacyPassPass(PassRegistry &);      // INTEL
void initializeIVUsersWrapperPassPass(PassRegistry&);
void initializeIfConverterPass(PassRegistry&);
void initializeImmutableModuleSummaryIndexWrapperPassPass(PassRegistry&);
void initializeImplicitNullChecksPass(PassRegistry&);
void initializeIndVarSimplifyLegacyPassPass(PassRegistry&);
void initializeIndirectBrExpandPassPass(PassRegistry&);
#if INTEL_COLLAB
void initializeInferAddressSpacesLegacyPassPass(PassRegistry &);
#else // INTEL_COLLAB
void initializeInferAddressSpacesPass(PassRegistry&);
#endif // INTEL_COLLAB
void initializeInferFunctionAttrsLegacyPassPass(PassRegistry&);
void initializeInjectTLIMappingsLegacyPass(PassRegistry &);
void initializeInlineAggressiveWrapperPassPass(PassRegistry&); // INTEL
void initializeInlineCostAnalysisPass(PassRegistry&);
void initializeInstCountLegacyPassPass(PassRegistry &);
void initializeInstNamerPass(PassRegistry&);
void initializeInstSimplifyLegacyPassPass(PassRegistry &);
void initializeInstrProfilingLegacyPassPass(PassRegistry&);
void initializeInstrOrderFileLegacyPassPass(PassRegistry&);
void initializeInstructionCombiningPassPass(PassRegistry&);
void initializeInstructionSelectPass(PassRegistry&);
void initializeIntelAdvancedFastCallWrapperPassPass(PassRegistry &); // INTEL
void initializeIntelIPOPrefetchWrapperPassPass(PassRegistry &); // INTEL
void initializeIntelPartialInlineLegacyPassPass(PassRegistry &); // INTEL
void initializeIntelArgumentAlignmentLegacyPassPass(PassRegistry &); // INTEL
void initializeIntelFoldWPIntrinsicLegacyPassPass(PassRegistry &); // INTEL
void initializeInterleavedAccessPass(PassRegistry&);
void initializeInterleavedLoadCombinePass(PassRegistry &);
void initializeInternalizeLegacyPassPass(PassRegistry&);
void initializeIntervalPartitionPass(PassRegistry&);
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
void initializeLiveVariablesPass(PassRegistry&);
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
void initializeLowerGuardIntrinsicLegacyPassPass(PassRegistry&);
void initializeLowerWidenableConditionLegacyPassPass(PassRegistry&);
void initializeLowerSubscriptIntrinsicLegacyPassPass(PassRegistry&); // INTEL
void initializeLowerIntrinsicsPass(PassRegistry&);
void initializeLowerInvokeLegacyPassPass(PassRegistry&);
void initializeLowerSwitchLegacyPassPass(PassRegistry &);
void initializeLowerTypeTestsPass(PassRegistry&);
void initializeLowerMatrixIntrinsicsLegacyPassPass(PassRegistry &);
void initializeLowerMatrixIntrinsicsMinimalLegacyPassPass(PassRegistry &);
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
void initializeMemorySanitizerLegacyPassPass(PassRegistry&);
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
void initializeNameAnonGlobalLegacyPassPass(PassRegistry&);
void initializeUniqueInternalLinkageNamesLegacyPassPass(PassRegistry &);
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
void initializePGOIndirectCallPromotionLegacyPassPass(PassRegistry&);
void initializePGOInstrumentationGenLegacyPassPass(PassRegistry&);
void initializePGOInstrumentationUseLegacyPassPass(PassRegistry&);
void initializePGOInstrumentationGenCreateVarLegacyPassPass(PassRegistry&);
void initializePGOMemOPSizeOptLegacyPassPass(PassRegistry&);
void initializePHIEliminationPass(PassRegistry&);
void initializePartialInlinerLegacyPassPass(PassRegistry&);
void initializePartiallyInlineLibCallsLegacyPassPass(PassRegistry&);
void initializePatchableFunctionPass(PassRegistry&);
void initializePeepholeOptimizerPass(PassRegistry&);
void initializePhiValuesWrapperPassPass(PassRegistry&);
void initializePhysicalRegisterUsageInfoPass(PassRegistry&);
void initializePlaceBackedgeSafepointsImplPass(PassRegistry&);
void initializePlaceSafepointsPass(PassRegistry&);
void initializePostDomOnlyPrinterPass(PassRegistry&);
void initializePostDomOnlyViewerPass(PassRegistry&);
void initializePostDomPrinterPass(PassRegistry&);
void initializePostDomViewerPass(PassRegistry&);
void initializePostDominatorTreeWrapperPassPass(PassRegistry&);
void initializePostInlineEntryExitInstrumenterPass(PassRegistry&);
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
void initializeRAGreedyPass(PassRegistry&);
void initializeReachingDefAnalysisPass(PassRegistry&);
void initializeReassociateLegacyPassPass(PassRegistry&);
void initializeRedundantDbgInstEliminationPass(PassRegistry&);
void initializeRegAllocFastPass(PassRegistry&);
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
void initializeRenameIndependentSubregsPass(PassRegistry&);
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
void initializeSampleProfileLoaderLegacyPassPass(PassRegistry&);
void initializeModuleSanitizerCoverageLegacyPassPass(PassRegistry &);
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
void initializeESIMDLowerLoadStorePass(PassRegistry &);
void initializeESIMDLowerVecArgLegacyPassPass(PassRegistry &);
void initializeTailCallElimPass(PassRegistry&);
void initializeTailDuplicatePass(PassRegistry&);
void initializeTargetLibraryInfoWrapperPassPass(PassRegistry&);
void initializeTargetPassConfigPass(PassRegistry&);
void initializeTargetTransformInfoWrapperPassPass(PassRegistry&);
void initializeThreadSanitizerLegacyPassPass(PassRegistry&);
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
void initializeWholeProgramDevirtPass(PassRegistry&);
void initializeWinEHPreparePass(PassRegistry&);
void initializeWriteBitcodePassPass(PassRegistry&);
void initializeWriteThinLTOBitcodePass(PassRegistry&);
void initializeXRayInstrumentationPass(PassRegistry&);
#if INTEL_CUSTOMIZATION
// Pass for alias analysis for STL templates
void initializeStdContainerAAWrapperPassPass(PassRegistry &);
// Pass for alias metadata propagation
void initializeStdContainerOptPass(PassRegistry &);
// Pass for TBAA metadata propagation
void initializeTbaaMDPropagationLegacyPassPass(PassRegistry &);
// Pass for removing fakeload intrinisics
void initializeCleanupFakeLoadsLegacyPassPass(PassRegistry &);
// Pass for function recognition
void initializeFunctionRecognizerLegacyPassPass(PassRegistry &);
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
void initializeLoopOptReportEmitterLegacyPassPass(PassRegistry&);
// Pass for loop carried CSE
void initializeLoopCarriedCSELegacyPass(PassRegistry&);
// Pass for transforming __fpga_reg builtin representation
void initializeTransformFPGARegPass(PassRegistry &);
// Qsort recognition
void initializeQsortRecognizerLegacyPassPass(PassRegistry&);
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
void initializeHIRInterLoopBlockingLegacyPassPass(PassRegistry&);
void initializeHIRLoopFusionLegacyPassPass(PassRegistry&);
void initializeHIRDeadStoreEliminationLegacyPassPass(PassRegistry&);
void initializeHIRLastValueComputationLegacyPassPass(PassRegistry&);
void initializeHIRPropagateCastedIVLegacyPassPass(PassRegistry&);
void initializeHIRMultiExitLoopRerollLegacyPassPass(PassRegistry&);
void initializeHIRRecognizeParLoopPass(PassRegistry&);
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
void initializeHIRCrossLoopArrayContractionLegacyPassPass(PassRegistry&);
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
// VPO VPlan Vectorizer Pass  --  TODO: VEC to COLLAB
void initializeVPlanDriverPass(PassRegistry&);
// VPO VPlan Vectorizer HIR Pass
void initializeVPlanDriverHIRPass(PassRegistry&);
void initializeVPlanFunctionVectorizerLegacyPassPass(PassRegistry&);
// OpenCL Passes
void initializeFMASplitterLegacyPassPass(PassRegistry&);
// Pass for dynamic_cast calls optimization
void initializeOptimizeDynamicCastsWrapperPass(PassRegistry&);
void initializeMachineLoopOptReportEmitterPass(PassRegistry&);
// DPCPP Kernel Transformation passes
void initializeDPCPPEqualizerLegacyPass(PassRegistry &);
void initializeDPCPPKernelVecClonePass(PassRegistry&);
void initializeDPCPPKernelPostVecPass(PassRegistry&);
void initializeDPCPPKernelWGLoopCreatorLegacyPassPass(PassRegistry&);
void initializeDPCPPKernelAnalysisPass(PassRegistry&);
void initializePhiCanonicalizationPass(PassRegistry&);
void initializeRedundantPhiNodePass(PassRegistry&);
void initializeSplitBBonBarrierPass(PassRegistry&);
void initializeWIRelatedValuePass(PassRegistry&);
void initializeDataPerBarrierPass(PassRegistry&);
void initializeDataPerValuePass(PassRegistry&);
void initializeKernelBarrierPass(PassRegistry&);
void initializeBarrierInFunctionPass(PassRegistry&);
void initializePostBarrierPass(PassRegistry&);
// Add/Sub reassociation pass
void initializeAddSubReassociateLegacyPassPass(PassRegistry&);
// Forced CMOV generation pass
void initializeForcedCMOVGenerationLegacyPassPass(PassRegistry&);
void initializeRAReportEmitterPass(PassRegistry&);
void initializeVPOParoptOptimizeDataSharingPass(PassRegistry&);
void initializeNontemporalStoreWrapperPassPass(PassRegistry&);
void initializeVPOParoptSharedPrivatizationPass(PassRegistry&);
// Transform sin and cos to sinpi, cospi, or sincospi pass
void initializeTransformSinAndCosCallsLegacyPassPass(PassRegistry&);
#endif // INTEL_CUSTOMIZATION

#if INTEL_COLLAB
// VPO WRegion Passes
void initializeWRegionCollectionWrapperPassPass(PassRegistry&);
void initializeWRegionInfoWrapperPassPass(PassRegistry&);
void initializeWRegionInfoAnalysisPass(PassRegistry&);
// VPO Utility Pass
void initializeVPOCFGRestructuringPass(PassRegistry&);
// VPO Paropt Loop Collapse Pass
void initializeVPOParoptLoopCollapsePass(PassRegistry&);
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
#endif // INTEL_CUSTOMIZATION

} // end namespace llvm

#endif // LLVM_INITIALIZEPASSES_H
