//===- llvm/InitializePasses.h - Initialize All Passes ----------*- C++ -*-===//
// INTEL_CUSTOMIZATION
//
// INTEL CONFIDENTIAL
//
// Modifications, Copyright (C) 2021 Intel Corporation
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you ("License"). Unless the License provides otherwise, you may
// not use, modify, copy, publish, distribute, disclose or transmit this
// software or the related documents without Intel's prior written permission.
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

/// Initialize all passes linked into the Core library.
void initializeCore(PassRegistry&);

/// Initialize all passes linked into the TransformUtils library.
void initializeTransformUtils(PassRegistry&);

/// Initialize all passes linked into the ScalarOpts library.
void initializeScalarOpts(PassRegistry&);

/// Initialize all passes linked into the Vectorize library.
void initializeVectorization(PassRegistry&);

/// Initialize all passes linked into the InstCombine library.
void initializeInstCombine(PassRegistry&);

/// Initialize all passes linked into the IPO library.
void initializeIPO(PassRegistry&);

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
void initializeIntel_LoopTransforms(PassRegistry &);
#endif // INTEL_CUSTOMIZATION

#if INTEL_COLLAB
// initializeVPOAnaylsis - Initialize all passes linked into the
// VPOAnalysis library
void initializeVPOAnalysis(PassRegistry&);

// initializeVPOTransforms - Initialize all passes linked into the
// VPOTransforms library
void initializeVPOTransforms(PassRegistry&);
#endif // INTEL_COLLAB

void initializeAAResultsWrapperPassPass(PassRegistry&);
void initializeAlwaysInlinerLegacyPassPass(PassRegistry&);
void initializeAssignmentTrackingAnalysisPass(PassRegistry &);
void initializeAssumeBuilderPassLegacyPassPass(PassRegistry &);
void initializeArgNoAliasPropPass(PassRegistry &); // INTEL
void initializeArrayUseWrapperPassPass(PassRegistry&); // INTEL
void initializeAssumptionCacheTrackerPass(PassRegistry&);
void initializeAtomicExpandPass(PassRegistry&);
void initializeBasicBlockPathCloningPass(PassRegistry &);
void initializeBasicBlockSectionsProfileReaderPass(PassRegistry &);
void initializeBasicBlockSectionsPass(PassRegistry &);
void initializeBarrierNoopPass(PassRegistry&);
void initializeBasicAAWrapperPassPass(PassRegistry&);
void initializeBlockFrequencyInfoWrapperPassPass(PassRegistry&);
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
void initializeCallBrPreparePass(PassRegistry &);
void initializeCallGraphDOTPrinterPass(PassRegistry&);
void initializeCallGraphPrinterLegacyPassPass(PassRegistry&);
void initializeCallGraphViewerPass(PassRegistry&);
void initializeCallGraphWrapperPassPass(PassRegistry&);
void initializeCheckDebugMachineModulePass(PassRegistry &);
void initializeCodeGenPreparePass(PassRegistry&);
void initializeComplexDeinterleavingLegacyPassPass(PassRegistry&);
void initializeConstantHoistingLegacyPassPass(PassRegistry&);
void initializeConvertGEPToSubscriptIntrinsicLegacyPassPass(PassRegistry &); // INTEL
void initializeGCOVProfilerLegacyPassPass(PassRegistry&); // INTEL
void initializeCostModelAnalysisPass(PassRegistry&);
void initializeCycleInfoWrapperPassPass(PassRegistry &);
void initializeDAEPass(PassRegistry&);
void initializeDAHPass(PassRegistry&);
void initializeDAESYCLPass(PassRegistry&);
void initializeDCELegacyPassPass(PassRegistry&);
#if INTEL_CUSTOMIZATION
#if INTEL_FEATURE_SW_ADVANCED
void initializeDeadArrayOpsEliminationLegacyPassPass(PassRegistry&);
#endif // INTEL_FEATURE_SW_ADVANCED
#endif // INTEL_CUSTOMIZATION
void initializeDeadMachineInstructionElimPass(PassRegistry&);
void initializeDebugifyMachineModulePass(PassRegistry &);
void initializeDelinearizationPass(PassRegistry&);
#if INTEL_CUSTOMIZATION
void initializeDemandedBitsWrapperPassPass(PassRegistry&);
#endif // INTEL_CUSTOMIZATION
void initializeDependenceAnalysisWrapperPassPass(PassRegistry&);
void initializeDopeVectorConstPropLegacyPassPass(PassRegistry&);   // INTEL
void initializeDetectDeadLanesPass(PassRegistry&);
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
void initializeExpandComplexPass(PassRegistry&); // INTEL
void initializeExpandLargeFpConvertLegacyPassPass(PassRegistry&);
void initializeExpandLargeDivRemLegacyPassPass(PassRegistry&);
void initializeExpandMemCmpPassPass(PassRegistry&);
void initializeExpandPostRAPass(PassRegistry&);
void initializeExpandReductionsPass(PassRegistry&);
void initializeExpandVectorPredicationPass(PassRegistry &);
void initializeMakeGuardsExplicitLegacyPassPass(PassRegistry&);
void initializeExternalAAWrapperPassPass(PassRegistry&);
void initializeFEntryInserterPass(PassRegistry&);
void initializeFPBuiltinFnSelectionLegacyPassPass(PassRegistry &);
void initializeFinalizeISelPass(PassRegistry&);
void initializeFinalizeMachineBundlesPass(PassRegistry&);
void initializeFixIrreduciblePass(PassRegistry &);
void initializeFixupStatepointCallerSavedPass(PassRegistry&);
void initializeFlattenCFGLegacyPassPass(PassRegistry &);
void initializeFloat128ExpandPass(PassRegistry&); // INTEL
void initializeFoldLoadsToGatherPass(PassRegistry&); // INTEL
void initializeFuncletLayoutPass(PassRegistry&);
void initializeFunctionSplittingWrapperPass(PassRegistry&);        // INTEL
void initializeGCEmptyBasicBlocksPass(PassRegistry &);
void initializeGCMachineCodeAnalysisPass(PassRegistry&);
void initializeGCModuleInfoPass(PassRegistry&);
void initializeGVNLegacyPassPass(PassRegistry&);
void initializeGlobalMergePass(PassRegistry&);
void initializeGlobalsAAWrapperPassPass(PassRegistry&);
void initializeGuardWideningLegacyPassPass(PassRegistry&);
void initializeHardwareLoopsLegacyPass(PassRegistry&);
void initializeMIRProfileLoaderPassPass(PassRegistry &);
void initializeIRSimilarityIdentifierWrapperPassPass(PassRegistry&);
void initializeIRTranslatorPass(PassRegistry&);
void initializeIVUsersWrapperPassPass(PassRegistry&);
void initializeIfConverterPass(PassRegistry&);
void initializeImmutableModuleSummaryIndexWrapperPassPass(PassRegistry&);
void initializeImplicitNullChecksPass(PassRegistry&);
void initializeIndirectBrExpandPassPass(PassRegistry&);
void initializeInferAddressSpacesPass(PassRegistry&);
void initializeInlineAggressiveWrapperPassPass(PassRegistry&); // INTEL
void initializeInstCountLegacyPassPass(PassRegistry &);
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
void initializeIntervalPartitionPass(PassRegistry&);
void initializeJMCInstrumenterPass(PassRegistry&);
void initializeIVSplitLegacyPassPass(PassRegistry&); // INTEL
void initializeKCFIPass(PassRegistry &);
void initializeLCSSAVerificationPassPass(PassRegistry&);
void initializeLCSSAWrapperPassPass(PassRegistry&);
void initializeLazyBlockFrequencyInfoPassPass(PassRegistry&);
void initializeLazyBranchProbabilityInfoPassPass(PassRegistry&);
void initializeLazyMachineBlockFrequencyInfoPassPass(PassRegistry&);
void initializeLazyValueInfoPrinterPass(PassRegistry&);
void initializeLazyValueInfoWrapperPassPass(PassRegistry&);
void initializeLegacyLICMPassPass(PassRegistry&);
void initializeLegacyLoopSinkPassPass(PassRegistry&);
void initializeLegalizerPass(PassRegistry&);
void initializeMemorySanitizerLegacyPassPass(PassRegistry&); // INTEL
void initializeGISelCSEAnalysisWrapperPassPass(PassRegistry &);
void initializeGISelKnownBitsAnalysisPass(PassRegistry &);
void initializeLiveDebugValuesPass(PassRegistry&);
void initializeLiveDebugVariablesPass(PassRegistry&);
void initializeLiveIntervalsPass(PassRegistry&);
void initializeLiveRangeShrinkPass(PassRegistry&);
void initializeLiveRegMatrixPass(PassRegistry&);
void initializeLiveStacksPass(PassRegistry&);
void initializeLiveVariablesPass(PassRegistry &);
void initializeLoadStoreOptPass(PassRegistry &);
void initializeLoadStoreVectorizerLegacyPassPass(PassRegistry&);
void initializeLocalStackSlotPassPass(PassRegistry&);
void initializeLocalizerPass(PassRegistry&);
void initializeLoopDataPrefetchLegacyPassPass(PassRegistry&);
void initializeLoopExtractorLegacyPassPass(PassRegistry &);
void initializeLoopGuardWideningLegacyPassPass(PassRegistry&);
void initializeLoopInfoWrapperPassPass(PassRegistry&);
void initializeLoopInstSimplifyLegacyPassPass(PassRegistry&);
void initializeLoopPassPass(PassRegistry&);
void initializeLoopPredicationLegacyPassPass(PassRegistry&);
void initializeLoopRotateLegacyPassPass(PassRegistry&);
void initializeLoopSimplifyCFGLegacyPassPass(PassRegistry&);
void initializeLoopSimplifyPass(PassRegistry&);
void initializeLoopStrengthReducePass(PassRegistry&);
void initializeLoopUnrollPass(PassRegistry&);
#ifdef INTEL_CUSTOMIZATION
void initializeLoopUnswitchPass(PassRegistry&);
#endif // INTEL_CUSTOMIZATION
void initializeLowerAtomicLegacyPassPass(PassRegistry&);
void initializeLowerConstantIntrinsicsPass(PassRegistry&);
void initializeLowerEmuTLSPass(PassRegistry&);
void initializeLowerGlobalDtorsLegacyPassPass(PassRegistry &);
void initializeLowerWidenableConditionLegacyPassPass(PassRegistry&);
void initializeLowerSubscriptIntrinsicLegacyPassPass(PassRegistry&); // INTEL
void initializeLowerIntrinsicsPass(PassRegistry&);
void initializeLowerInvokeLegacyPassPass(PassRegistry&);
void initializeLowerSwitchLegacyPassPass(PassRegistry &);
void initializeKCFIPass(PassRegistry &);
void initializeMIRAddFSDiscriminatorsPass(PassRegistry &);
void initializeMIRCanonicalizerPass(PassRegistry &);
void initializeMIRNamerPass(PassRegistry &);
void initializeMIRPrintingPassPass(PassRegistry&);
void initializeMachineBlockFrequencyInfoPass(PassRegistry&);
void initializeMachineBlockPlacementPass(PassRegistry&);
void initializeMachineBlockPlacementStatsPass(PassRegistry&);
void initializeMachineBranchProbabilityInfoPass(PassRegistry&);
void initializeMachineCFGPrinterPass(PassRegistry &);
void initializeMachineCSEPass(PassRegistry&);
void initializeMachineCombinerPass(PassRegistry&);
void initializeMachineCopyPropagationPass(PassRegistry&);
void initializeMachineCycleInfoPrinterPassPass(PassRegistry &);
void initializeMachineCycleInfoWrapperPassPass(PassRegistry &);
void initializeMachineDominanceFrontierPass(PassRegistry&);
void initializeMachineDominatorTreePass(PassRegistry&);
void initializeMachineFunctionPrinterPassPass(PassRegistry&);
void initializeMachineFunctionSplitterPass(PassRegistry &);
void initializeMachineLateInstrsCleanupPass(PassRegistry&);
void initializeMachineLICMPass(PassRegistry&);
void initializeMachineLoopInfoPass(PassRegistry&);
void initializeMachineModuleInfoWrapperPassPass(PassRegistry &);
void initializeMachineOptimizationRemarkEmitterPassPass(PassRegistry&);
void initializeMachineOutlinerPass(PassRegistry&);
void initializeMachinePipelinerPass(PassRegistry&);
void initializeMachinePostDominatorTreePass(PassRegistry&);
void initializeMachineRegionInfoPassPass(PassRegistry&);
void initializeMachineSanitizerBinaryMetadataPass(PassRegistry &);
void initializeMachineSchedulerPass(PassRegistry&);
void initializeMachineSinkingPass(PassRegistry&);
void initializeMachineTraceMetricsPass(PassRegistry&);
void initializeMachineUniformityInfoPrinterPassPass(PassRegistry &);
void initializeMachineUniformityAnalysisPassPass(PassRegistry &);
void initializeMachineVerifierPassPass(PassRegistry&);
void initializeMemoryDependenceWrapperPassPass(PassRegistry&);
void initializeMemorySSAWrapperPassPass(PassRegistry&);
void initializeMergeICmpsLegacyPassPass(PassRegistry &);
void initializeMergedLoadStoreMotionLegacyPassPass(PassRegistry&);
void initializeModuleSummaryIndexWrapperPassPass(PassRegistry&);
void initializeModuloScheduleTestPass(PassRegistry&);
void initializeNaryReassociateLegacyPassPass(PassRegistry&);
#if INTEL_CUSTOMIZATION
void initializeObjCARCAAWrapperPassPass(PassRegistry&);
void initializeObjCARCAPElimPass(PassRegistry&);
#endif // INTEL_CUSTOMIZATION
void initializeObjCARCContractLegacyPassPass(PassRegistry &);
#if INTEL_CUSTOMIZATION
void initializeObjCARCExpandPass(PassRegistry&);
void initializeObjCARCOptLegacyPassPass(PassRegistry &);
#endif // INTEL_CUSTOMIZATION
void initializeOptimizationRemarkEmitterWrapperPassPass(PassRegistry&);
void initializeOptimizePHIsPass(PassRegistry&);
void initializePEIPass(PassRegistry&);
void initializePHIEliminationPass(PassRegistry&);
void initializePartiallyInlineLibCallsLegacyPassPass(PassRegistry&);
void initializePatchableFunctionPass(PassRegistry&);
void initializePeepholeOptimizerPass(PassRegistry&);
void initializePhiValuesWrapperPassPass(PassRegistry&);
void initializePhysicalRegisterUsageInfoPass(PassRegistry&);
void initializePlaceBackedgeSafepointsLegacyPassPass(PassRegistry &);
void initializePostDomOnlyPrinterWrapperPassPass(PassRegistry &);
void initializePostDomOnlyViewerWrapperPassPass(PassRegistry &);
void initializePostDomPrinterWrapperPassPass(PassRegistry &);
void initializePostDomViewerWrapperPassPass(PassRegistry &);
void initializePostDominatorTreeWrapperPassPass(PassRegistry&);
void initializePostMachineSchedulerPass(PassRegistry&);
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
void initializeRegAllocPriorityAdvisorAnalysisPass(PassRegistry &);
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
void initializeSCEVAAWrapperPassPass(PassRegistry&);
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
void initializeSimpleLoopUnswitchLegacyPassPass(PassRegistry&);
void initializeSingleLoopExtractorPass(PassRegistry&);
void initializeSinkingLegacyPassPass(PassRegistry&);
void initializeSjLjEHPreparePass(PassRegistry&);
void initializeSlotIndexesPass(PassRegistry&);
void initializeSpeculativeExecutionLegacyPassPass(PassRegistry&);
void initializeSpillPlacementPass(PassRegistry&);
void initializeStackColoringPass(PassRegistry&);
void initializeStackFrameLayoutAnalysisPassPass(PassRegistry &);
void initializeStackMapLivenessPass(PassRegistry&);
void initializeStackProtectorPass(PassRegistry&);
void initializeStackSafetyGlobalInfoWrapperPassPass(PassRegistry &);
void initializeStackSafetyInfoWrapperPassPass(PassRegistry &);
void initializeStackSlotColoringPass(PassRegistry&);
void initializeStraightLineStrengthReduceLegacyPassPass(PassRegistry &);
void initializeStripDebugMachineModulePass(PassRegistry &);
void initializeStripGCRelocatesLegacyPass(PassRegistry &);
void initializeStructurizeCFGLegacyPassPass(PassRegistry &);
void initializeSYCLLowerWGScopeLegacyPassPass(PassRegistry &);
void initializeSYCLLowerESIMDLegacyPassPass(PassRegistry &);
void initializeSYCLLowerInvokeSimdLegacyPassPass(PassRegistry &);
void initializeSYCLMutatePrintfAddrspaceLegacyPassPass(PassRegistry &);
void initializeSPIRITTAnnotationsLegacyPassPass(PassRegistry &);
void initializeESIMDLowerLoadStorePass(PassRegistry &);
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
void initializeTypePromotionLegacyPass(PassRegistry&);
void initializeUniformityInfoWrapperPassPass(PassRegistry &);
void initializeUnifyFunctionExitNodesLegacyPassPass(PassRegistry &);
void initializeUnifyLoopExitsLegacyPassPass(PassRegistry &);
void initializeUnpackMachineBundlesPass(PassRegistry&);
void initializeUnreachableBlockElimLegacyPassPass(PassRegistry&);
void initializeUnreachableMachineBlockElimPass(PassRegistry&);
void initializeVerifierLegacyPassPass(PassRegistry&);
void initializeVirtRegMapPass(PassRegistry&);
void initializeVirtRegRewriterPass(PassRegistry&);
void initializeWasmEHPreparePass(PassRegistry&);
void initializeWinEHPreparePass(PassRegistry&);
void initializeWriteBitcodePassPass(PassRegistry&);
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
// Pass for handling '#pragma vector aligned'.
void initializeHandlePragmaVectorAlignedLegacyPassPass(PassRegistry &);
// Pass for SnodeInfo analysis
void initializeSNodeAnalysisPass(PassRegistry &);
// Pass for register promotion for non escaped block scope global variables.
void initializeNonLTOGlobalOptLegacyPassPass(PassRegistry &);
// Pass for math call optimization.
void initializeMapIntrinToImlPass(PassRegistry&);
// Pass for indicating loopopt based throttling.
void initializeLoopOptMarkerLegacyPassPass(PassRegistry&);
// Pass to store the opt level.
void initializeOptReportOptionsPassPass(PassRegistry &);
void initializeOptReportEmitterLegacyPassPass(PassRegistry&);
// Pass for loop carried CSE
void initializeLoopCarriedCSELegacyPass(PassRegistry&);
// Pass for transforming __fpga_reg builtin representation
void initializeTransformFPGARegPass(PassRegistry &);
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
void initializeVectorGraphPredicatorPass(PassRegistry &);
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
void initializeMachineOptReportEmitterPass(PassRegistry &);
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
// VPO Pass to rename operands as part of VPO Paropt Prepare pass
void initializeVPORenameOperandsPass(PassRegistry &);
// VPO Pass to guard memory motion of clause variables.
void initializeVPOParoptGuardMemoryMotionPass(PassRegistry &);
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
#endif // INTEL_CUSTOMIZATION

} // end namespace llvm

#endif // LLVM_INITIALIZEPASSES_H
