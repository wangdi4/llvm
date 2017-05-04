//===- llvm/InitializePasses.h -------- Initialize All Passes ---*- C++ -*-===//
//
//                      The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
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
void initializeGlobalISel(PassRegistry &Registry);

/// Initialize all passes linked into the CodeGen library.
void initializeTarget(PassRegistry&);

#if INTEL_CUSTOMIZATION
/// initializeIntel_LoopAnalysis - Initialize all passes linked into the
/// Intel_LoopAnalysis library.
void initializeIntel_LoopAnalysis(PassRegistry&);

/// initializeIntel_LoopTransforms - Initialize all passes linked into the
/// Intel_LoopTransforms library.
void initializeIntel_LoopTransforms(PassRegistry&);

// initializeIntel_VPOAnaylsis - Initialize all passes linked into the 
// Intel_VPOAnalysis library
void initializeIntel_VPOAnalysis(PassRegistry&);

// initializeIntel_VPOTransforms - Initialize all passes linked into the
// Intel_VPOTransforms library
void initializeIntel_VPOTransforms(PassRegistry&);
#endif // INTEL_CUSTOMIZATION

void initializeAAEvalLegacyPassPass(PassRegistry&);
void initializeAAResultsWrapperPassPass(PassRegistry &);
void initializeADCELegacyPassPass(PassRegistry&);
void initializeAddDiscriminatorsLegacyPassPass(PassRegistry&);
void initializeAddressSanitizerModulePass(PassRegistry&);
void initializeAddressSanitizerPass(PassRegistry&);
void initializeAggInlAALegacyPassPass(PassRegistry&); // INTEL
void initializeAliasSetPrinterPass(PassRegistry&);
void initializeAlignmentFromAssumptionsPass(PassRegistry&);
void initializeAlwaysInlinerLegacyPassPass(PassRegistry&);
void initializeAndersensAAWrapperPassPass(PassRegistry&); // INTEL
void initializeArgPromotionPass(PassRegistry&);
void initializeAssumptionCacheTrackerPass(PassRegistry &);
void initializeAtomicExpandPass(PassRegistry&);
void initializeBBVectorizePass(PassRegistry&);
void initializeBDCELegacyPassPass(PassRegistry &);
void initializeBarrierNoopPass(PassRegistry&);
void initializeBasicAAWrapperPassPass(PassRegistry&);
void initializeBlockExtractorPassPass(PassRegistry&);
void initializeBlockFrequencyInfoWrapperPassPass(PassRegistry&);
void initializeBoundsCheckingPass(PassRegistry&);
void initializeBranchFolderPassPass(PassRegistry&);
void initializeBranchProbabilityInfoWrapperPassPass(PassRegistry&);
void initializeBranchRelaxationPass(PassRegistry&);
void initializeBreakCriticalEdgesPass(PassRegistry&);
void initializeCFGOnlyViewerLegacyPassPass(PassRegistry&);
void initializeCFGPrinterLegacyPassPass(PassRegistry&);
void initializeCFGOnlyPrinterLegacyPassPass(PassRegistry&);
void initializeCFGSimplifyPassPass(PassRegistry&);
void initializeCFGViewerLegacyPassPass(PassRegistry&);
void initializeCFLAndersAAWrapperPassPass(PassRegistry&);
void initializeCFLSteensAAWrapperPassPass(PassRegistry&);
void initializeCallGraphDOTPrinterPass(PassRegistry&);
void initializeCallGraphPrinterLegacyPassPass(PassRegistry&);
void initializeCallGraphViewerPass(PassRegistry&);
void initializeCallGraphWrapperPassPass(PassRegistry &);
void initializeCodeGenPreparePass(PassRegistry&);
void initializeCountingFunctionInserterPass(PassRegistry&);
void initializeConstantHoistingLegacyPassPass(PassRegistry&);
void initializeConstantMergeLegacyPassPass(PassRegistry &);
void initializeConstantPropagationPass(PassRegistry&);
void initializeCorrelatedValuePropagationPass(PassRegistry&);
void initializeCostModelAnalysisPass(PassRegistry&);
void initializeCrossDSOCFIPass(PassRegistry&);
void initializeDAEPass(PassRegistry&);
void initializeDAHPass(PassRegistry&);
void initializeDCELegacyPassPass(PassRegistry&);
void initializeDSELegacyPassPass(PassRegistry&);
void initializeDataFlowSanitizerPass(PassRegistry&);
void initializeDeadInstEliminationPass(PassRegistry&);
void initializeDeadMachineInstructionElimPass(PassRegistry&);
void initializeDelinearizationPass(PassRegistry &);
void initializeDemandedBitsWrapperPassPass(PassRegistry&);
void initializeDependenceAnalysisPass(PassRegistry&);
void initializeDependenceAnalysisWrapperPassPass(PassRegistry&);
void initializeDetectDeadLanesPass(PassRegistry&);
void initializeDivergenceAnalysisPass(PassRegistry&);
void initializeDomOnlyPrinterPass(PassRegistry&);
void initializeDomOnlyViewerPass(PassRegistry&);
void initializeDomPrinterPass(PassRegistry&);
void initializeDomViewerPass(PassRegistry&);
void initializeDominanceFrontierWrapperPassPass(PassRegistry&);
void initializeDominatorTreeWrapperPassPass(PassRegistry&);
void initializeDwarfEHPreparePass(PassRegistry&);
void initializeEarlyCSELegacyPassPass(PassRegistry &);
void initializeEarlyCSEMemSSALegacyPassPass(PassRegistry &);
void initializeEarlyIfConverterPass(PassRegistry&);
void initializeEdgeBundlesPass(PassRegistry&);
void initializeEfficiencySanitizerPass(PassRegistry&);
void initializeEliminateAvailableExternallyLegacyPassPass(PassRegistry &);
void initializeRAGreedyPass(PassRegistry&);
void initializeGVNHoistLegacyPassPass(PassRegistry &);
void initializeExpandISelPseudosPass(PassRegistry&);
void initializeExpandPostRAPass(PassRegistry&);
void initializeExternalAAWrapperPassPass(PassRegistry&);
void initializeFEntryInserterPass(PassRegistry &);
void initializeFinalizeMachineBundlesPass(PassRegistry&);
void initializeFlattenCFGPassPass(PassRegistry&);
void initializeFloat2IntLegacyPassPass(PassRegistry&);
void initializeForceFunctionAttrsLegacyPassPass(PassRegistry&);
void initializeForwardControlFlowIntegrityPass(PassRegistry&);
void initializeFuncletLayoutPass(PassRegistry &);
void initializeFunctionImportLegacyPassPass(PassRegistry &);
void initializeGCMachineCodeAnalysisPass(PassRegistry&);
void initializeGCModuleInfoPass(PassRegistry&);
void initializeGCOVProfilerLegacyPassPass(PassRegistry&);
void initializeGVNLegacyPassPass(PassRegistry&);
void initializeGlobalDCELegacyPassPass(PassRegistry&);
void initializeGlobalMergePass(PassRegistry&);
void initializeGlobalOptLegacyPassPass(PassRegistry&);
void initializeGlobalSplitPass(PassRegistry&);
void initializeGlobalsAAWrapperPassPass(PassRegistry&);
void initializeGuardWideningLegacyPassPass(PassRegistry&);
void initializeIPCPPass(PassRegistry&);
void initializeIPCloningLegacyPassPass(PassRegistry&);             // INTEL
void initializeIPSCCPLegacyPassPass(PassRegistry &);
void initializeIRTranslatorPass(PassRegistry &);
void initializeIVUsersWrapperPassPass(PassRegistry&);
void initializeIfConverterPass(PassRegistry&);
void initializeImplicitNullChecksPass(PassRegistry&);
void initializeIndVarSimplifyLegacyPassPass(PassRegistry&);
void initializeInductiveRangeCheckEliminationPass(PassRegistry&);
void initializeInferAddressSpacesPass(PassRegistry&);
void initializeInferFunctionAttrsLegacyPassPass(PassRegistry&);
void initializeInlineAggressiveWrapperPassPass(PassRegistry&); // INTEL
void initializeInlineCostAnalysisPass(PassRegistry&);
void initializeInstCountPass(PassRegistry&);
void initializeInstNamerPass(PassRegistry&);
void initializeInstSimplifierPass(PassRegistry&);
void initializeInstrProfilingLegacyPassPass(PassRegistry &);
void initializeInstructionCombiningPassPass(PassRegistry&);
void initializeInstructionSelectPass(PassRegistry &);
void initializeInterleavedAccessPass(PassRegistry &);
void initializeInternalizeLegacyPassPass(PassRegistry&);
void initializeIntervalPartitionPass(PassRegistry&);
void initializeJumpThreadingPass(PassRegistry&);
void initializeLCSSAWrapperPassPass(PassRegistry&);
void initializeLCSSAVerificationPassPass(PassRegistry&);
void initializeLegacyLICMPassPass(PassRegistry&);
void initializeLegacyLoopSinkPassPass(PassRegistry&);
void initializeLazyBranchProbabilityInfoPassPass(PassRegistry&);
void initializeLazyMachineBlockFrequencyInfoPassPass(PassRegistry&);
void initializeLazyBlockFrequencyInfoPassPass(PassRegistry&);
void initializeLazyValueInfoWrapperPassPass(PassRegistry&);
void initializeLegalizerPass(PassRegistry&);
void initializeLibCallsShrinkWrapLegacyPassPass(PassRegistry&);
void initializeLintPass(PassRegistry&);
void initializeLiveDebugValuesPass(PassRegistry&);
void initializeLiveDebugVariablesPass(PassRegistry&);
void initializeLiveIntervalsPass(PassRegistry&);
void initializeLiveRegMatrixPass(PassRegistry&);
void initializeLiveStacksPass(PassRegistry&);
void initializeLiveVariablesPass(PassRegistry&);
void initializeLoadCombinePass(PassRegistry&);
void initializeLoaderPassPass(PassRegistry&);
void initializeLoadStoreVectorizerPass(PassRegistry&);
void initializeLocalStackSlotPassPass(PassRegistry&);
void initializeLoopAccessLegacyAnalysisPass(PassRegistry&);
void initializeLoopDataPrefetchLegacyPassPass(PassRegistry &);
void initializeLoopDeletionLegacyPassPass(PassRegistry&);
void initializeLoopDistributeLegacyPass(PassRegistry&);
void initializeLoopExtractorPass(PassRegistry&);
void initializeLoopIdiomRecognizeLegacyPassPass(PassRegistry&);
void initializeLoopInfoWrapperPassPass(PassRegistry&);
void initializeLoopInstSimplifyLegacyPassPass(PassRegistry&);
void initializeLoopInterchangePass(PassRegistry &);
void initializeLoopLoadEliminationPass(PassRegistry&);
void initializeLoopPassPass(PassRegistry&);
void initializeLoopPredicationLegacyPassPass(PassRegistry&);
void initializeLoopRerollPass(PassRegistry&);
void initializeLoopRotateLegacyPassPass(PassRegistry&);
void initializeLoopSimplifyCFGLegacyPassPass(PassRegistry&);
void initializeLoopSimplifyPass(PassRegistry&);
void initializeLoopStrengthReducePass(PassRegistry&);
void initializeLoopUnrollPass(PassRegistry&);
void initializeLoopUnswitchPass(PassRegistry&);
void initializeLoopVectorizePass(PassRegistry&);
void initializeLoopVersioningLICMPass(PassRegistry&);
void initializeLoopVersioningPassPass(PassRegistry &);
void initializeLowerAtomicLegacyPassPass(PassRegistry &);
void initializeLowerEmuTLSPass(PassRegistry&);
void initializeLowerExpectIntrinsicPass(PassRegistry&);
void initializeLowerGuardIntrinsicLegacyPassPass(PassRegistry&);
void initializeLowerIntrinsicsPass(PassRegistry&);
void initializeLowerInvokeLegacyPassPass(PassRegistry&);
void initializeLowerSwitchPass(PassRegistry&);
void initializeLowerTypeTestsPass(PassRegistry&);
void initializeMIRPrintingPassPass(PassRegistry&);
void initializeMachineBlockFrequencyInfoPass(PassRegistry&);
void initializeMachineBlockPlacementPass(PassRegistry&);
void initializeMachineBlockPlacementStatsPass(PassRegistry&);
void initializeMachineBranchProbabilityInfoPass(PassRegistry&);
void initializeMachineCSEPass(PassRegistry&);
void initializeMachineCombinerPass(PassRegistry &);
void initializeMachineCopyPropagationPass(PassRegistry&);
void initializeMachineDominanceFrontierPass(PassRegistry&);
void initializeMachineDominatorTreePass(PassRegistry&);
void initializeMachineFunctionPrinterPassPass(PassRegistry&);
void initializeMachineLICMPass(PassRegistry&);
void initializeMachineLoopInfoPass(PassRegistry&);
void initializeMachineModuleInfoPass(PassRegistry&);
void initializeMachineOptimizationRemarkEmitterPassPass(PassRegistry&);
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
void initializeMemorySSAWrapperPassPass(PassRegistry&);
void initializeMemorySSAPrinterLegacyPassPass(PassRegistry &);
void initializeMemorySanitizerPass(PassRegistry&);
void initializeMergeFunctionsPass(PassRegistry&);
void initializeMergedLoadStoreMotionLegacyPassPass(PassRegistry &);
void initializeMetaRenamerPass(PassRegistry&);
void initializeModuleDebugInfoPrinterPass(PassRegistry&);
void initializeModuleSummaryIndexWrapperPassPass(PassRegistry &);
void initializeNameAnonGlobalLegacyPassPass(PassRegistry &);
void initializeNaryReassociateLegacyPassPass(PassRegistry &);
void initializeNewGVNPass(PassRegistry&);
void initializeObjCARCAAWrapperPassPass(PassRegistry&);
void initializeObjCARCAPElimPass(PassRegistry&);
void initializeObjCARCContractPass(PassRegistry&);
void initializeObjCARCExpandPass(PassRegistry&);
void initializeObjCARCOptPass(PassRegistry&);
void initializeOptimizationRemarkEmitterWrapperPassPass(PassRegistry&);
void initializeOptimizePHIsPass(PassRegistry&);
void initializePAEvalPass(PassRegistry &);
void initializePEIPass(PassRegistry&);
void initializePGOIndirectCallPromotionLegacyPassPass(PassRegistry&);
void initializePGOInstrumentationGenLegacyPassPass(PassRegistry&);
void initializePGOInstrumentationUseLegacyPassPass(PassRegistry&);
void initializePHIEliminationPass(PassRegistry&);
void initializePhysicalRegisterUsageInfoPass(PassRegistry &);
void initializePartialInlinerLegacyPassPass(PassRegistry &);
void initializePartiallyInlineLibCallsLegacyPassPass(PassRegistry &);
void initializePatchableFunctionPass(PassRegistry &);
void initializePeepholeOptimizerPass(PassRegistry&);
void initializePlaceBackedgeSafepointsImplPass(PassRegistry&);
void initializePlaceSafepointsPass(PassRegistry&);
void initializePostDomOnlyPrinterPass(PassRegistry&);
void initializePostDomOnlyViewerPass(PassRegistry&);
void initializePostDomPrinterPass(PassRegistry&);
void initializePostDomViewerPass(PassRegistry&);
void initializePostDominatorTreeWrapperPassPass(PassRegistry&);
void initializePostMachineSchedulerPass(PassRegistry&);
void initializePostOrderFunctionAttrsLegacyPassPass(PassRegistry&);
void initializePostRAHazardRecognizerPass(PassRegistry&);
void initializePostRASchedulerPass(PassRegistry&);
void initializePredicateInfoPrinterLegacyPassPass(PassRegistry &);
void initializePreISelIntrinsicLoweringLegacyPassPass(PassRegistry&);
void initializePrintBasicBlockPassPass(PassRegistry&);
void initializePrintFunctionPassWrapperPass(PassRegistry&);
void initializePrintModulePassWrapperPass(PassRegistry&);
void initializeProcessImplicitDefsPass(PassRegistry&);
void initializeProfileSummaryInfoWrapperPassPass(PassRegistry &);
void initializePromoteLegacyPassPass(PassRegistry &);
void initializePruneEHPass(PassRegistry&);
void initializeReassociateLegacyPassPass(PassRegistry&);
void initializeRegBankSelectPass(PassRegistry &);
void initializeRegToMemPass(PassRegistry&);
void initializeRegionInfoPassPass(PassRegistry&);
void initializeRegionOnlyPrinterPass(PassRegistry&);
void initializeRegionOnlyViewerPass(PassRegistry&);
void initializeRegionPrinterPass(PassRegistry&);
void initializeRegionViewerPass(PassRegistry&);
void initializeRegisterCoalescerPass(PassRegistry&);
void initializeStripGCRelocatesPass(PassRegistry&);
void initializeRenameIndependentSubregsPass(PassRegistry&);
void initializeResetMachineFunctionPass(PassRegistry &);
void initializeReversePostOrderFunctionAttrsLegacyPassPass(PassRegistry&);
void initializeRewriteStatepointsForGCPass(PassRegistry&);
void initializeRewriteSymbolsLegacyPassPass(PassRegistry&);
void initializeSCCPLegacyPassPass(PassRegistry &);
void initializeSCEVAAWrapperPassPass(PassRegistry&);
void initializeSLPVectorizerPass(PassRegistry&);
void initializeSROALegacyPassPass(PassRegistry&);
void initializeSafeStackPass(PassRegistry&);
void initializeSampleProfileLoaderLegacyPassPass(PassRegistry&);
void initializeSanitizerCoverageModulePass(PassRegistry&);
void initializeScalarEvolutionWrapperPassPass(PassRegistry&);
void initializeScalarizerPass(PassRegistry&);
void initializeScopedNoAliasAAWrapperPassPass(PassRegistry&);
void initializeSeparateConstOffsetFromGEPPass(PassRegistry &);
void initializeShadowStackGCLoweringPass(PassRegistry&);
void initializeShrinkWrapPass(PassRegistry &);
void initializeSimpleInlinerPass(PassRegistry&);
void initializeSingleLoopExtractorPass(PassRegistry&);
void initializeSinkingLegacyPassPass(PassRegistry&);
void initializeSjLjEHPreparePass(PassRegistry&);
void initializeSlotIndexesPass(PassRegistry&);
void initializeSpeculativeExecutionLegacyPassPass(PassRegistry&);
void initializeSpillPlacementPass(PassRegistry&);
void initializeStackColoringPass(PassRegistry&);
void initializeStackMapLivenessPass(PassRegistry&);
void initializeStackProtectorPass(PassRegistry&);
void initializeStackSlotColoringPass(PassRegistry&);
void initializeStraightLineStrengthReducePass(PassRegistry &);
void initializeStripDeadDebugInfoPass(PassRegistry&);
void initializeStripDeadPrototypesLegacyPassPass(PassRegistry&);
void initializeStripDebugDeclarePass(PassRegistry&);
void initializeStripNonDebugSymbolsPass(PassRegistry&);
void initializeStripNonLineTableDebugInfoPass(PassRegistry&);
void initializeStripSymbolsPass(PassRegistry&);
void initializeStructurizeCFGPass(PassRegistry&);
void initializeTailCallElimPass(PassRegistry&);
void initializeTailDuplicatePassPass(PassRegistry&);
void initializeTargetLibraryInfoWrapperPassPass(PassRegistry &);
void initializeTargetPassConfigPass(PassRegistry&);
void initializeTargetTransformInfoWrapperPassPass(PassRegistry &);
void initializeThreadSanitizerPass(PassRegistry&);
void initializeTwoAddressInstructionPassPass(PassRegistry&);
void initializeTypeBasedAAWrapperPassPass(PassRegistry&);
void initializeUnifyFunctionExitNodesPass(PassRegistry&);
void initializeUnpackMachineBundlesPass(PassRegistry&);
void initializeUnreachableBlockElimLegacyPassPass(PassRegistry&);
void initializeUnreachableMachineBlockElimPass(PassRegistry&);
void initializeVerifierLegacyPassPass(PassRegistry&);
void initializeVirtRegMapPass(PassRegistry&);
void initializeVirtRegRewriterPass(PassRegistry&);
void initializeWholeProgramDevirtPass(PassRegistry &);
void initializeWinEHPreparePass(PassRegistry&);
void initializeWriteBitcodePassPass(PassRegistry &);
void initializeWriteThinLTOBitcodePass(PassRegistry &);
void initializeXRayInstrumentationPass(PassRegistry &);
#if INTEL_CUSTOMIZATION
// Pass for alias analysis for STL templates
void initializeStdContainerAAWrapperPassPass(PassRegistry &);
// Pass for alias metadata propagation
void initializeStdContainerOptPass(PassRegistry &);
// Pass for TBAA metadata propagation
void initializeTbaaMDPropagationPass(PassRegistry &);
// Pass for indirect call conversion using points-to info
void initializeIndirectCallConvPass(PassRegistry &);
// Pass for SnodeInfo analysis
void initializeSNodeAnalysisPass(PassRegistry &);
// Pass for register promotion for non escaped block scope global variables.
void initializeNonLTOGlobalOptPass(PassRegistry &);
// Pass for math call optimization.
void initializeMapIntrinToImlPass(PassRegistry&);
// Pass for indicating loopopt based throttling.
void initializeLoopOptMarkerPass(PassRegistry&);
// HIR Passes
void initializeHIRRegionIdentificationPass(PassRegistry&);
void initializeHIRSCCFormationPass(PassRegistry&);
void initializeHIRScalarSymbaseAssignmentPass(PassRegistry&);
void initializeHIRCreationPass(PassRegistry&);
void initializeHIRCleanupPass(PassRegistry&);
void initializeHIRLoopFormationPass(PassRegistry&);
void initializeHIRParserPass(PassRegistry&);
void initializeHIRSymbaseAssignmentPass(PassRegistry&);
void initializeHIRFrameworkPass(PassRegistry&);
void initializeHIRDDAnalysisPass(PassRegistry&);
void initializeHIRLocalityAnalysisPass(PassRegistry&);
void initializeHIRLoopResourcePass(PassRegistry&);
void initializeHIRSafeReductionAnalysisPass(PassRegistry&);
void initializeHIRLoopStatisticsPass(PassRegistry&);
void initializeHIRSSADeconstructionPass(PassRegistry&);
void initializeHIRTempCleanupPass(PassRegistry&);
void initializeHIRParVecAnalysisPass(PassRegistry&);
void initializeHIRParDirInsertPass(PassRegistry&);
void initializeHIRVecDirInsertPass(PassRegistry&);
void initializeHIRVectVLSAnalysisPass(PassRegistry&);
void initializeHIRPrinterPass(llvm::PassRegistry&);
void initializeHIRPreVecCompleteUnrollPass(PassRegistry&);
void initializeHIRPostVecCompleteUnrollPass(PassRegistry&);
void initializeHIRLoopInterchangePass(PassRegistry&);
void initializeHIRLoopDistributionForMemRecPass(PassRegistry&);
void initializeHIRLoopDistributionForLoopNestPass(PassRegistry&);
void initializeHIRGeneralUnrollPass(PassRegistry&);
void initializeHIRUnrollAndJamPass(PassRegistry&);
void initializeHIROptPredicatePass(PassRegistry&);
void initializeHIRRuntimeDDPass(PassRegistry&);
void initializeHIRLoopReversalPass(PassRegistry&);
void initializeHIRLMMPass(PassRegistry&);
void initializeHIRScalarReplArrayPass(PassRegistry&);
void initializeHIRDummyTransformationPass(PassRegistry&);
void initializeHIRCodeGenPass(PassRegistry&);
void initializeHIROptVarPredicatePass(PassRegistry&);
void initializeHIRIdiomRecognitionPass(PassRegistry&);
void initializeHIRMVForConstUBPass(PassRegistry&);
// VPO WRegion Passes
void initializeWRegionCollectionPass(PassRegistry&);
void initializeWRegionInfoPass(PassRegistry&);
void initializeWRegionInfoAnalysisPass(PassRegistry&);
// VPO Utility Pass
void initializeVPOCFGRestructuringPass(PassRegistry&);
// VPO Paropt Prepare Pass
void initializeVPOParoptPreparePass(PassRegistry&);
// VPO Parallelizer Pass
void initializeVPOParoptPass(PassRegistry&);
// VPO Tpv Transformation
void initializeVPOParoptTpvPass(PassRegistry&);
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
#endif // INTEL_CUSTOMIZATION
void initializeLoopDistributePass(PassRegistry&);
void initializeSjLjEHPreparePass(PassRegistry&);
void initializeDemandedBitsWrapperPassPass(PassRegistry&);
void initializeFuncletLayoutPass(PassRegistry &);
void initializeLoopLoadEliminationPass(PassRegistry&);
void initializeFunctionImportPassPass(PassRegistry &);
void initializeLoopVersioningPassPass(PassRegistry &);
void initializeWholeProgramWrapperPassPass(PassRegistry&); // INTEL
void initializeWholeProgramDevirtPass(PassRegistry &);
void initializePatchableFunctionPass(PassRegistry &);
}

#endif
