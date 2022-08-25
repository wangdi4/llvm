//===- llvm/LinkAllPasses.h ------------ Reference All Passes ---*- C++ -*-===//
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
// This header file pulls in all transformation and analysis passes for tools
// like opt and bugpoint that need this functionality.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LINKALLPASSES_H
#define LLVM_LINKALLPASSES_H

#include "llvm/ADT/Statistic.h"
#include "llvm/Analysis/AliasAnalysisEvaluator.h"
#include "llvm/Analysis/AliasSetTracker.h"
#include "llvm/Analysis/BasicAliasAnalysis.h"
#include "llvm/Analysis/CFLAndersAliasAnalysis.h"
#include "llvm/Analysis/CFLSteensAliasAnalysis.h"
#include "llvm/Analysis/CallPrinter.h"
#include "llvm/Analysis/DomPrinter.h"
#include "llvm/Analysis/GlobalsModRef.h"
#include "llvm/Analysis/IntervalPartition.h"
#include "llvm/Analysis/Lint.h"
#include "llvm/Analysis/Passes.h"
#include "llvm/Analysis/PostDominators.h"
#include "llvm/Analysis/RegionPass.h"
#include "llvm/Analysis/RegionPrinter.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/ScalarEvolutionAliasAnalysis.h"
#include "llvm/Analysis/Intel_Andersens.h"  // INTEL
#include "llvm/Analysis/Intel_ArrayUseAnalysis.h"  // INTEL
#include "llvm/Analysis/Intel_LoopAnalysis/Passes.h" // INTEL - HIR
#include "llvm/Analysis/Intel_StdContainerAA.h"  // INTEL
#include "llvm/Analysis/Intel_XmainOptLevelPass.h" // INTEL
#include "llvm/Analysis/Intel_OptReport/OptReportOptionsPass.h" // INTEL
#include "llvm/Analysis/ScopedNoAliasAA.h"
#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/Analysis/TypeBasedAliasAnalysis.h"
#include "llvm/CodeGen/Passes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRPrintingPasses.h"
#include "llvm/SYCLLowerIR/ESIMD/ESIMDVerifier.h"
#include "llvm/SYCLLowerIR/ESIMD/LowerESIMD.h"
#include "llvm/SYCLLowerIR/LowerWGLocalMemory.h"
#include "llvm/SYCLLowerIR/LowerWGScope.h"
#include "llvm/SYCLLowerIR/LowerInvokeSimd.h"
#include "llvm/SYCLLowerIR/MutatePrintfAddrspace.h"
#include "llvm/Support/Valgrind.h"
#include "llvm/Transforms/AggressiveInstCombine/AggressiveInstCombine.h"
#include "llvm/Transforms/IPO.h"
#include "llvm/Transforms/IPO/AlwaysInliner.h"
#include "llvm/Transforms/IPO/Attributor.h"
#include "llvm/Transforms/IPO/FunctionAttrs.h"
#include "llvm/Transforms/IPO/Intel_AdvancedFastCall.h" // INTEL
#if INTEL_CUSTOMIZATION
#if INTEL_FEATURE_SW_ADVANCED
#include "llvm/Transforms/IPO/Intel_IPOPrefetch.h"
#endif // INTEL_FEATURE_SW_ADVANCED
#endif // INTEL_CUSTOMIZATION
#include "llvm/Transforms/IPO/Intel_AutoCPUClone.h" // INTEL
#include "llvm/Transforms/IPO/Intel_InlineLists.h" // INTEL
#include "llvm/Transforms/IPO/Intel_InlineReportEmitter.h" // INTEL
#include "llvm/Transforms/IPO/Intel_InlineReportSetup.h" // INTEL
#include "llvm/Transforms/IPO/Intel_OptimizeDynamicCasts.h" // INTEL
#include "llvm/Transforms/InstCombine/InstCombine.h"
#include "llvm/Transforms/Instrumentation.h"
#include "llvm/Transforms/Instrumentation/BoundsChecking.h"
#include "llvm/Transforms/Instrumentation/SPIRITTAnnotations.h"
#include "llvm/Transforms/ObjCARC.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Scalar/GVN.h"
#include "llvm/Transforms/Scalar/InstSimplifyPass.h"
#include "llvm/Transforms/Scalar/Scalarizer.h"
#include "llvm/Transforms/Utils.h"
#include "llvm/Transforms/Utils/SymbolRewriter.h"
#include "llvm/Transforms/Utils/UnifyFunctionExitNodes.h"
#include "llvm/Transforms/Vectorize.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/LegacyPasses.h"  // INTEL
#include "llvm/Transforms/Intel_LoopTransforms/Passes.h"         // INTEL - HIR
#include "llvm/Transforms/Intel_MapIntrinToIml/MapIntrinToIml.h" // INTEL
#include "llvm/Transforms/Utils/Intel_VecClone.h"                // INTEL

#if INTEL_CUSTOMIZATION
#if INTEL_FEATURE_SW_DTRANS
#include "Intel_DTrans/DTransCommon.h"
#endif // INTEL_FEATURE_SW_DTRANS
#if INTEL_FEATURE_CSA
#include "Intel_CSA/CSAIRPasses.h"
#endif  // INTEL_FEATURE_CSA
#endif // INTEL_CUSTOMIZATION

#if INTEL_COLLAB
#include "llvm/Analysis/VPO/WRegionInfo/WRegionPasses.h"
#include "llvm/Transforms/VPO/VPOPasses.h"
#endif // INTEL_COLLAB

#include <cstdlib>

namespace {
  struct ForcePassLinking {
    ForcePassLinking() {
      // We must reference the passes in such a way that compilers will not
      // delete it all as dead code, even with whole program optimization,
      // yet is effectively a NO-OP. As the compiler isn't smart enough
      // to know that getenv() never returns -1, this will do the job.
      // This is so that globals in the translation units where these functions
      // are defined are forced to be initialized, populating various
      // registries.
      if (std::getenv("bar") != (char*) -1)
        return;

      (void) llvm::createAAEvalPass();
      (void) llvm::createAggressiveDCEPass();
      (void) llvm::createAggressiveInstCombinerPass();
      (void) llvm::createBitTrackingDCEPass();
      (void)llvm::createOpenMPOptCGSCCLegacyPass();
      (void) llvm::createAlignmentFromAssumptionsPass();
#if INTEL_CUSTOMIZATION
      (void) llvm::createAndersensAAWrapperPass();
#if INTEL_FEATURE_SW_DTRANS
      (void) llvm::createDTransPasses();
#endif // INTEL_FEATURE_SW_DTRANS
      (void) llvm::createNonLTOGlobalOptimizerPass();
      (void) llvm::createTbaaMDPropagationLegacyPass();
      (void) llvm::createCleanupFakeLoadsPass();
      (void) llvm::createHandlePragmaVectorAlignedPass();
      (void) llvm::createStdContainerOptPass();
      (void) llvm::createStdContainerAAWrapperPass();
      (void) llvm::createInlineListsPass();
      (void) llvm::createInlineReportSetupPass();
      (void) llvm::createInlineReportEmitterPass();
      (void) llvm::createIntelAdvancedFastCallWrapperPass();
#if INTEL_FEATURE_SW_ADVANCED
      (void) llvm::createIntelIPOPrefetchWrapperPass();
#endif // INTEL_FEATURE_SW_ADVANCED
      (void) llvm::createXmainOptLevelWrapperPass();
      (void) llvm::createOptReportOptionsPass();
      (void) llvm::createRemoveRegionDirectivesLegacyPass();
      (void) llvm::createOptReportEmitterLegacyPass();
      (void) llvm::createLowerSubscriptIntrinsicLegacyPass();
      (void) llvm::createConvertGEPToSubscriptIntrinsicLegacyPass();
      (void) llvm::createCallTreeCloningPass();
      (void) llvm::createAddSubReassociatePass();
      (void) llvm::createForcedCMOVGenerationPass();
      (void) llvm::createTransformFPGARegPass();
      (void) llvm::createTransformSinAndCosCallsPass();
#endif // INTEL_CUSTOMIZATION
      (void) llvm::createBasicAAWrapperPass();
      (void) llvm::createSCEVAAWrapperPass();
      (void) llvm::createTypeBasedAAWrapperPass();
      (void) llvm::createScopedNoAliasAAWrapperPass();
      (void) llvm::createBoundsCheckingLegacyPass();
      (void) llvm::createBreakCriticalEdgesPass();
      (void) llvm::createCallGraphDOTPrinterPass();
      (void) llvm::createCallGraphViewerPass();
      (void) llvm::createCFGSimplificationPass();
      (void) llvm::createCFLAndersAAWrapperPass();
      (void) llvm::createCFLSteensAAWrapperPass();
      (void) llvm::createStructurizeCFGPass();
      (void) llvm::createLibCallsShrinkWrapPass();
      (void) llvm::createCalledValuePropagationPass();
      (void) llvm::createConstantMergePass();
      (void) llvm::createCostModelAnalysisPass();
      (void) llvm::createDeadArgEliminationPass();
      (void) llvm::createDeadArgEliminationSYCLPass();
      (void) llvm::createDeadCodeEliminationPass();
      (void) llvm::createDeadStoreEliminationPass();
      (void) llvm::createDependenceAnalysisWrapperPass();
#if INTEL_CUSTOMIZATION
      (void) llvm::createGCOVProfilerPass();
      (void) llvm::createPGOInstrumentationGenLegacyPass();
      (void) llvm::createPGOInstrumentationUseLegacyPass();
      (void) llvm::createPGOInstrumentationGenCreateVarLegacyPass();
      (void) llvm::createPGOIndirectCallPromotionLegacyPass();
#endif // INTEL_CUSTOMIZATION
      (void) llvm::createDomOnlyPrinterWrapperPassPass();
      (void) llvm::createDomPrinterWrapperPassPass();
      (void) llvm::createDomOnlyViewerWrapperPassPass();
      (void) llvm::createDomViewerWrapperPassPass();
      (void) llvm::createFunctionInliningPass();
#if INTEL_CUSTOMIZATION
#if INTEL_FEATURE_SW_ADVANCED
      (void) llvm::createFunctionRecognizerLegacyPass();
#endif // INTEL_FEATURE_SW_ADVANCED
#endif // INTEL_CUSTOMIZATION
      (void) llvm::createAlwaysInlinerLegacyPass();
      (void) llvm::createGlobalDCEPass();
      (void) llvm::createGlobalOptimizerPass();
      (void) llvm::createGlobalsAAWrapperPass();
      (void) llvm::createGuardWideningPass();
      (void) llvm::createLoopGuardWideningPass();
      (void) llvm::createIPSCCPPass();
      (void) llvm::createIndirectCallConvLegacyPass(); // INTEL
      (void) llvm::createInductiveRangeCheckEliminationPass();
      (void) llvm::createIndVarSimplifyPass();
      (void) llvm::createInstSimplifyLegacyPass();
      (void) llvm::createInstructionCombiningPass();
      (void) llvm::createInternalizePass();
      (void) llvm::createJMCInstrumenterPass();
      (void) llvm::createLCSSAPass();
      (void) llvm::createLegacyDivergenceAnalysisPass();
      (void) llvm::createLICMPass();
      (void) llvm::createLoopSinkPass();
      (void) llvm::createLazyValueInfoPass();
      (void) llvm::createLoopExtractorPass();
      (void) llvm::createLoopInterchangePass();
      (void) llvm::createLoopFlattenPass();
      (void) llvm::createLoopPredicationPass();
      (void) llvm::createLoopSimplifyPass();
      (void) llvm::createLoopSimplifyCFGPass();
      (void) llvm::createLoopStrengthReducePass();
      (void) llvm::createLoopRerollPass();
      (void) llvm::createLoopUnrollPass();
      (void) llvm::createLoopUnrollAndJamPass();
#if INTEL_CUSTOMIZATION
      (void) llvm::createLoopUnswitchPass();
#endif // INTEL_CUSTOMIZATION
      (void) llvm::createLoopVersioningLICMPass();
      (void) llvm::createLoopIdiomPass();
      (void) llvm::createLoopRotatePass();
      (void) llvm::createLowerConstantIntrinsicsPass();
      (void) llvm::createLowerExpectIntrinsicPass();
      (void) llvm::createLowerGlobalDtorsLegacyPass();
      (void) llvm::createLowerInvokePass();
      (void) llvm::createLowerSwitchPass();
      (void) llvm::createNaryReassociatePass();
      (void) llvm::createObjCARCAAWrapperPass();
      (void) llvm::createObjCARCAPElimPass();
      (void) llvm::createObjCARCExpandPass();
      (void) llvm::createObjCARCContractPass();
      (void) llvm::createObjCARCOptPass();
      (void) llvm::createPAEvalPass();
      (void) llvm::createPromoteMemoryToRegisterPass();
      (void) llvm::createDemoteRegisterToMemoryPass();
      (void) llvm::createPruneEHPass();
      (void)llvm::createPostDomOnlyPrinterWrapperPassPass();
      (void)llvm::createPostDomPrinterWrapperPassPass();
      (void)llvm::createPostDomOnlyViewerWrapperPassPass();
      (void)llvm::createPostDomViewerWrapperPassPass();
      (void) llvm::createReassociatePass();
      (void) llvm::createRedundantDbgInstEliminationPass();
      (void) llvm::createRegionInfoPass();
      (void) llvm::createRegionOnlyPrinterPass();
      (void) llvm::createRegionOnlyViewerPass();
      (void) llvm::createRegionPrinterPass();
      (void) llvm::createRegionViewerPass();
      (void) llvm::createSCCPPass();
      (void) llvm::createSafeStackPass();
      (void) llvm::createSROAPass();
      (void) llvm::createSingleLoopExtractorPass();
      (void) llvm::createStripSymbolsPass();
      (void) llvm::createStripNonDebugSymbolsPass();
      (void) llvm::createStripDeadDebugInfoPass();
      (void) llvm::createStripDeadPrototypesPass();
      (void) llvm::createTailCallEliminationPass();
      (void)llvm::createTLSVariableHoistPass();
      (void) llvm::createJumpThreadingPass();
      (void) llvm::createIVSplitLegacyPass(); // INTEL
      (void) llvm::createDFAJumpThreadingPass();
      (void) llvm::createUnifyFunctionExitNodesPass();
      (void) llvm::createInstCountPass();
      (void) llvm::createConstantHoistingPass();
      (void) llvm::createCodeGenPreparePass();
      (void) llvm::createEarlyCSEPass();
      (void) llvm::createGVNHoistPass();
      (void) llvm::createMergedLoadStoreMotionPass();
      (void) llvm::createGVNPass();
      (void) llvm::createNewGVNPass();
      (void) llvm::createMemCpyOptPass();
      (void) llvm::createLoopDeletionPass();
      (void) llvm::createPostDomTree();
      (void) llvm::createInstructionNamerPass();
      (void) llvm::createMetaRenamerPass();
      (void) llvm::createAttributorLegacyPass();
      (void) llvm::createAttributorCGSCCLegacyPass();
      (void) llvm::createPostOrderFunctionAttrsLegacyPass();
      (void) llvm::createReversePostOrderFunctionAttrsPass();
      (void) llvm::createMergeFunctionsPass();
      (void) llvm::createMergeICmpsLegacyPass();
      (void) llvm::createExpandMemCmpPass();
      (void) llvm::createExpandVectorPredicationPass();
      (void)llvm::createSYCLLowerWGScopePass();
      (void)llvm::createSYCLLowerESIMDPass();
      (void)llvm::createESIMDLowerLoadStorePass();
      (void)llvm::createESIMDLowerVecArgPass();
      (void)llvm::createESIMDVerifierPass();
      (void)llvm::createSPIRITTAnnotationsLegacyPass();
      (void)llvm::createSYCLLowerWGLocalMemoryLegacyPass();
      (void)llvm::createESIMDVerifierPass();
      (void)llvm::createSYCLLowerInvokeSimdPass();
      std::string buf;
      llvm::raw_string_ostream os(buf);
      (void) llvm::createPrintModulePass(os);
      (void) llvm::createPrintFunctionPass(os);
      (void) llvm::createModuleDebugInfoPrinterPass();
      (void) llvm::createPartialInliningPass();
      (void) llvm::createLintLegacyPassPass();
      (void) llvm::createSinkingPass();
      (void) llvm::createLowerAtomicPass();
      (void) llvm::createCorrelatedValuePropagationPass();
      (void) llvm::createMemDepPrinter();
      (void) llvm::createLoopVectorizePass();
      (void) llvm::createSLPVectorizerPass();
      (void) llvm::createLoadStoreVectorizerPass();
#if INTEL_CUSTOMIZATION
      (void) llvm::createVPlanPragmaOmpOrderedSimdExtractPass();
      (void) llvm::createVPlanDriverPass();
      (void) llvm::createVPlanDriverHIRPass(false);
      (void) llvm::createVPlanFunctionVectorizerPass();
#endif // INTEL_CUSTOMIZATION
      (void) llvm::createVectorCombinePass();
      (void) llvm::createPartiallyInlineLibCallsPass();
      (void) llvm::createScalarizerPass();
      (void) llvm::createSeparateConstOffsetFromGEPPass();
      (void) llvm::createSpeculativeExecutionPass();
      (void) llvm::createSpeculativeExecutionIfHasBranchDivergencePass();
      (void) llvm::createRewriteSymbolsPass();
      (void) llvm::createStraightLineStrengthReducePass();
      (void) llvm::createMemDerefPrinter();
      (void) llvm::createMustExecutePrinter();
      (void) llvm::createMustBeExecutedContextPrinter();
      (void) llvm::createFloat2IntPass();
      (void) llvm::createEliminateAvailableExternallyPass();
      (void)llvm::createScalarizeMaskedMemIntrinLegacyPass();
      (void) llvm::createWarnMissedTransformationsPass();
      (void) llvm::createHardwareLoopsPass();
      (void) llvm::createInjectTLIMappingsLegacyPass();
      (void) llvm::createUnifyLoopExitsPass();
      (void) llvm::createFixIrreduciblePass();
      (void)llvm::createFunctionSpecializationPass();
      (void)llvm::createSelectOptimizePass();

      (void)new llvm::IntervalPartition();
      (void)new llvm::ScalarEvolutionWrapperPass();
      llvm::Function::Create(nullptr, llvm::GlobalValue::ExternalLinkage)->viewCFGOnly();
      llvm::RGPassManager RGM;
      llvm::TargetLibraryInfoImpl TLII;
      llvm::TargetLibraryInfo TLI(TLII);
      llvm::AliasAnalysis AA(TLI);
      llvm::AliasSetTracker X(AA);
      X.add(nullptr, llvm::LocationSize::beforeOrAfterPointer(),
            llvm::AAMDNodes()); // for -print-alias-sets
      (void) llvm::AreStatisticsEnabled();
      (void) llvm::sys::RunningOnValgrind();

#if INTEL_CUSTOMIZATION
      (void)llvm::createLoadCoalescingPass();
      (void)llvm::createMathLibraryFunctionsReplacementPass();
      (void) llvm::createSNodeAnalysisPass();
      (void) llvm::createLoopOptMarkerLegacyPass();
      (void) llvm::createArrayUseWrapperPass();
#if INTEL_FEATURE_SW_ADVANCED
      (void) llvm::createNontemporalStoreWrapperPass();
#endif // INTEL_FEATURE_SW_ADVANCED
      // HIR passes
      (void) llvm::createHIRRegionIdentificationWrapperPass();
      (void) llvm::createHIRSCCFormationWrapperPass();
      (void) llvm::createHIRFrameworkWrapperPass();
      (void) llvm::createHIROptReportEmitterWrapperPass();
      (void) llvm::createHIRDDAnalysisPass();
      (void) llvm::createHIRLocalityAnalysisPass();
      (void) llvm::createHIRLoopResourceWrapperPass();
      (void) llvm::createHIRLoopStatisticsWrapperPass();
      (void) llvm::createHIRParVecAnalysisPass();
      (void) llvm::createHIRSafeReductionAnalysisPass();
      (void) llvm::createHIRSparseArrayReductionAnalysisPass();
      (void) llvm::createHIRSSADeconstructionLegacyPass();
      (void) llvm::createHIRTempCleanupPass();
      (void) llvm::createHIRLoopInterchangePass();
      (void) llvm::createHIRLoopBlockingPass();
      (void) llvm::createHIRPragmaLoopBlockingPass();
      (void) llvm::createHIRGenerateMKLCallPass();
      (void) llvm::createHIROptPredicatePass();
      (void) llvm::createHIROptVarPredicatePass();
      (void) llvm::createHIRGeneralUnrollPass();
      (void) llvm::createHIRUnrollAndJamPass();
      (void) llvm::createHIRPreVecCompleteUnrollPass();
      (void) llvm::createHIRPostVecCompleteUnrollPass();
      (void) llvm::createHIRParDirInsertPass();
      (void) llvm::createHIRVecDirInsertPass();
      (void) llvm::createHIRLoopDistributionForMemRecPass();
      (void) llvm::createHIRLoopDistributionForLoopNestPass();
      (void) llvm::createHIRLoopRematerializePass();
      (void) llvm::createHIRLoopRerollPass();
      (void) llvm::createHIRLoopReversalPass();
      (void) llvm::createHIRIfReversalPass();
      (void) llvm::createHIRLMMPass();
      (void) llvm::createHIRLoopCollapsePass();
      (void) llvm::createHIRPMSymbolicTripCountCompleteUnrollLegacyPass();
      (void) llvm::createHIRScalarReplArrayPass();
      (void) llvm::createHIRIdiomRecognitionPass();
      (void) llvm::createHIRMVForConstUBPass();
      (void) llvm::createHIRMVForVariableStridePass();
      (void) llvm::createHIRLoopConcatenationPass();
      (void) llvm::createHIRArrayTransposePass();
      (void) llvm::createHIRAosToSoaPass();
#if INTEL_FEATURE_SW_ADVANCED
      (void) llvm::createHIRInterLoopBlockingPass();
#endif // INTEL_FEATURE_SW_ADVANCED
      (void) llvm::createHIRLoopFusionPass();
      (void) llvm::createHIRDummyTransformationPass();
      (void) llvm::createHIRCodeGenWrapperPass();
      (void) llvm::createHIRDeadStoreEliminationPass();
      (void) llvm::createHIRLastValueComputationPass();
      (void) llvm::createHIRPropagateCastedIVPass();
      (void) llvm::createHIRMultiExitLoopRerollPass();
      (void) llvm::createHIRMinMaxRecognitionPass();
      (void) llvm::createHIRIdentityMatrixIdiomRecognitionPass();
      (void) llvm::createHIRPrefetchingPass();
      (void) llvm::createHIRSinkingForPerfectLoopnestPass();
      (void) llvm::createHIRUndoSinkingForPerfectLoopnestPass();
      (void) llvm::createHIRConditionalTempSinkingPass();
      (void) llvm::createHIRMemoryReductionSinkingPass();
      (void) llvm::createHIRRowWiseMVPass();
      (void) llvm::createHIRConditionalLoadStoreMotionPass();
      (void) llvm::createHIRNontemporalMarkingPass();
      (void) llvm::createHIRStoreResultIntoTempArrayPass();
      (void) llvm::createHIRSumWindowReusePass();
      (void) llvm::createHIRNonZeroSinkingForPerfectLoopnestPass();
      (void) llvm::createHIRIdentityMatrixSubstitutionPass();
      (void) llvm::createHIRArrayScalarizationTestLauncherPass();

      // DPCPP Kernel Transformations
      (void)llvm::createAddFunctionAttrsLegacyPass();
      (void)llvm::createBuiltinImportLegacyPass();
      (void)llvm::createBuiltinLICMLegacyPass();
      (void)llvm::createBuiltinLibInfoAnalysisLegacyPass();
      (void)llvm::createCoerceWin64TypesLegacyPass();
      (void)llvm::createDPCPPAliasAnalysisLegacyPass();
      (void)llvm::createDPCPPExternalAliasAnalysisLegacyPass();
      (void)llvm::createDPCPPEqualizerLegacyPass();
      (void)llvm::createDPCPPKernelVecClonePass();
      (void)llvm::createDPCPPKernelPostVecPass();
      (void)llvm::createDPCPPKernelWGLoopCreatorLegacyPass();
      (void)llvm::createDPCPPKernelAnalysisLegacyPass();
      (void)llvm::createDPCPPPreprocessSPIRVFriendlyIRLegacyPass();
      (void)llvm::createDPCPPRewritePipesLegacyPass();
      (void)llvm::createDeduceMaxWGDimLegacyPass();
      (void)llvm::createDetectRecursionLegacyPass();
      (void)llvm::createDuplicateCalledKernelsLegacyPass();
      (void)llvm::createExternalizeGlobalVariablesLegacyPass();
      (void)llvm::createPhiCanonicalizationLegacyPass();
      (void)llvm::createRedundantPhiNodeLegacyPass();
      (void)llvm::createGroupBuiltinLegacyPass();
      (void)llvm::createSetVectorizationFactorLegacyPass();
      (void)llvm::createSplitBBonBarrierLegacyPass();
      (void)llvm::createWIRelatedValueWrapperPass();
      (void)llvm::createDataPerBarrierWrapperPass();
      (void)llvm::createDataPerValueWrapperPass();
      (void)llvm::createKernelBarrierLegacyPass(false, false);
      (void)llvm::createBarrierInFunctionLegacyPass();
      (void)llvm::createImplicitArgsAnalysisLegacyPass();
      (void)llvm::createImplicitGIDLegacyPass();
      (void)llvm::createInferArgumentAliasLegacyPass();
      (void)llvm::createInfiniteLoopCreatorLegacyPass();
      (void)llvm::createInstToFuncCallLegacyPass();
      (void)llvm::createInternalizeGlobalVariablesLegacyPass();
      (void)llvm::createInternalizeNonKernelFuncLegacyPass();
      (void)llvm::createLocalBufferAnalysisLegacyPass();
      (void)llvm::createLocalBuffersLegacyPass(false);
      (void)llvm::createLoopStridedCodeMotionLegacyPass();
      (void)llvm::createLoopWIAnalysisLegacyPass();
      (void)llvm::createAddFastMathLegacyPass();
      (void)llvm::createAddImplicitArgsLegacyPass();
      (void)llvm::createAddNTAttrLegacyPass();
      (void)llvm::createAddTLSGlobalsLegacyPass();
      (void)llvm::createAutorunReplicatorLegacyPass();
      (void)llvm::createOptimizeIDivAndIRemLegacyPass();
      (void)llvm::createRelaxedMathLegacyPass();
      (void)llvm::createRemoveAtExitLegacyPass();
      (void)llvm::createRemoveDuplicatedBarrierLegacyPass(false);
      (void)llvm::createReplaceScalarWithMaskLegacyPass();
      (void)llvm::createReqdSubGroupSizeLegacyPass();
      (void)llvm::createResolveMatrixFillLegacyPass();
      (void)llvm::createResolveMatrixLayoutLegacyPass();
      (void)llvm::createResolveMatrixWISliceLegacyPass();
      (void)llvm::createResolveSubGroupWICallLegacyPass();
      (void)llvm::createResolveVarTIDCallLegacyPass();
      (void)llvm::createResolveWICallLegacyPass(false, false);
      (void)llvm::createSetPreferVectorWidthLegacyPass();
      (void)llvm::createSGBarrierPropagateLegacyPass();
      (void)llvm::createSGBarrierSimplifyLegacyPass();
      (void)llvm::createSGBuiltinLegacyPass();
      (void)llvm::createSGLoopConstructLegacyPass();
      (void)llvm::createSGSizeAnalysisLegacyPass();
      (void)llvm::createSGValueWidenLegacyPass();
      (void)llvm::createSinCosFoldLegacyPass();
      (void)llvm::createSoaAllocaAnalysisLegacyPass();
      (void)llvm::createPatchCallbackArgsLegacyPass(false);
      (void)llvm::createPipeIOTransformationLegacyPass();
      (void)llvm::createPipeOrderingLegacyPass();
      (void)llvm::createPipeSupportLegacyPass();
      (void)llvm::createPrepareKernelArgsLegacyPass(false);
      (void)llvm::createPreventDivCrashesLegacyPass();
      (void)llvm::createProfilingInfoLegacyPass();
      (void)llvm::createChannelPipeTransformationLegacyPass();
      (void)llvm::createCleanupWrappedKernelLegacyPass();
      (void)llvm::createCoerceTypesLegacyPass();
      (void)llvm::createVFAnalysisLegacyPass();
      (void)llvm::createHandleVPlanMaskLegacyPass(nullptr);
      (void)llvm::createVectorVariantFillInLegacyPass();
      (void)llvm::createVectorVariantLoweringLegacyPass(llvm::VFISAKind::SSE);
      (void)llvm::createSGSizeCollectorLegacyPass(llvm::VFISAKind::SSE);
      (void)llvm::createSGSizeCollectorIndirectLegacyPass(llvm::VFISAKind::SSE);
      (void)llvm::createTaskSeqAsyncHandlingLegacyPass();
      (void)llvm::createUpdateCallAttrsLegacyPass();
      (void)llvm::createIndirectCallLoweringLegacyPass();
      (void)llvm::createCreateSimdVariantPropagationLegacyPass();
      (void)llvm::createLinearIdResolverPass();
      (void)llvm::createVectorKernelEliminationLegacyPass();
      (void)llvm::createVectorizationDimensionAnalysisLegacyPass();
      (void)llvm::createWGLoopBoundariesLegacyPass();
      (void)llvm::createWeightedInstCountAnalysisLegacyPass();
      (void)llvm::createWorkItemAnalysisLegacyPass();

      // Optimize math calls
      (void) llvm::createMapIntrinToImlPass();

      // VPO WRegion Passes
      (void) llvm::createWRegionCollectionWrapperPassPass();
      (void) llvm::createWRegionInfoWrapperPassPass();

      // VPO Vectorizer Passes
      (void) llvm::createVPODirectiveCleanupPass();
      (void) llvm::createVecClonePass();

      // dynamic_cast calls optimization pass.
      (void) llvm::createOptimizeDynamicCastsWrapperPass();

  #if INTEL_FEATURE_CSA
      // Various CSA passes.
      (void) llvm::createLoopSPMDizationPass();
      (void) llvm::createCSALowerParallelIntrinsicsWrapperPass();
      (void) llvm::createCSAGraphSplitterPass();
  #endif  // INTEL_FEATURE_CSA
      (void) llvm::createVPOParoptOptimizeDataSharingPass();
      (void) llvm::createVPOParoptSharedPrivatizationPass();
      (void) llvm::createVPOParoptTargetInlinePass();
      (void) llvm::createVPOParoptApplyConfigPass();
      (void) llvm::createIntelVTableFixupPass();
      (void) llvm::createAutoCPUCloneLegacyPass();
  #endif // INTEL_CUSTOMIZATION

  #if INTEL_COLLAB
      // VPO Paropt Loop Collapse Pass
      (void) llvm::createVPOParoptLoopCollapsePass();

      // VPO Paropt Loop Transform Pass
      (void) llvm::createVPOParoptLoopTransformPass();

      // VPO Paropt Prepare Passes
      (void) llvm::createVPOParoptPreparePass();

      // VPO Pass to restore clause opreands renamed by the Prepare pass.
      (void)llvm::createVPORestoreOperandsPass();

      // VPO Pass to rename clause opreands during the Prepare pass.
      (void)llvm::createVPORenameOperandsPass();

      // VPO Paropt Guard Memory Motion Pass
      (void) llvm::createVPOParoptGuardMemoryMotionPass();

      // VPO Parallelizer Passes
      (void) llvm::createVPOParoptPass();

      // VPO Thread Private Transformation
      (void) llvm::createVPOParoptTpvPass();
  #endif // INTEL_COLLAB
    }
  } ForcePassLinking; // Force link by creating a global definition.
}

#endif

