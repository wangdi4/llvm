//===- llvm/LinkAllPasses.h ------------ Reference All Passes ---*- C++ -*-===//
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
#include "llvm/Analysis/CallPrinter.h"
#include "llvm/Analysis/DomPrinter.h"
#include "llvm/Analysis/GlobalsModRef.h"
#include "llvm/Analysis/IntervalPartition.h"
#include "llvm/Analysis/Passes.h"
#include "llvm/Analysis/PostDominators.h"
#include "llvm/Analysis/RegionPass.h"
#include "llvm/Analysis/RegionPrinter.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/ScalarEvolutionAliasAnalysis.h"
#include "llvm/Analysis/ScopedNoAliasAA.h"
#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/Analysis/TypeBasedAliasAnalysis.h"
#include "llvm/CodeGen/Passes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRPrintingPasses.h"
#include "llvm/SYCLLowerIR/ESIMD/ESIMDVerifier.h"
#include "llvm/SYCLLowerIR/ESIMD/LowerESIMD.h"
#include "llvm/SYCLLowerIR/LowerInvokeSimd.h"
#include "llvm/SYCLLowerIR/LowerWGLocalMemory.h"
#include "llvm/SYCLLowerIR/LowerWGScope.h"
#include "llvm/SYCLLowerIR/MutatePrintfAddrspace.h"
#include "llvm/Support/Valgrind.h"
#include "llvm/Transforms/IPO.h"
#include "llvm/Transforms/IPO/AlwaysInliner.h"
#include "llvm/Transforms/InstCombine/InstCombine.h"
#include "llvm/Transforms/Instrumentation.h"
#include "llvm/Transforms/Instrumentation/SPIRITTAnnotations.h"
#include "llvm/Transforms/ObjCARC.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Scalar/GVN.h"
#include "llvm/Transforms/Scalar/Scalarizer.h"
#include "llvm/Transforms/Utils.h"
#include "llvm/Transforms/Utils/SymbolRewriter.h"
#include "llvm/Transforms/Utils/UnifyFunctionExitNodes.h"
#include "llvm/Transforms/Vectorize.h"

#if INTEL_CUSTOMIZATION
#include "llvm/Analysis/Intel_ArrayUseAnalysis.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Passes.h"
#include "llvm/Analysis/Intel_OptReport/OptReportOptionsPass.h"
#include "llvm/Analysis/Intel_StdContainerAA.h"
#include "llvm/Transforms/IPO/Intel_InlineReportEmitter.h"
#include "llvm/Transforms/IPO/Intel_InlineReportSetup.h"
#include "llvm/Transforms/Intel_LoopTransforms/Passes.h"
#include "llvm/Transforms/Intel_MapIntrinToIml/MapIntrinToIml.h"
#include "llvm/Transforms/Utils/Intel_VecClone.h"
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
#if INTEL_CUSTOMIZATION
      (void) llvm::createNonLTOGlobalOptimizerPass();
      (void) llvm::createTbaaMDPropagationLegacyPass();
      (void) llvm::createCleanupFakeLoadsPass();
      (void) llvm::createHandlePragmaVectorAlignedPass();
      (void) llvm::createStdContainerOptPass();
      (void)llvm::createStdContainerAAWrapperPass();
      (void) llvm::createOptReportOptionsPass();
      (void) llvm::createOptReportEmitterLegacyPass();
      (void) llvm::createLowerSubscriptIntrinsicLegacyPass();
      (void) llvm::createConvertGEPToSubscriptIntrinsicLegacyPass();
      (void) llvm::createForcedCMOVGenerationPass();
      (void) llvm::createTransformFPGARegPass();
      (void) llvm::createTransformSinAndCosCallsPass();
#endif // INTEL_CUSTOMIZATION
      (void) llvm::createBasicAAWrapperPass();
      (void) llvm::createSCEVAAWrapperPass();
      (void) llvm::createTypeBasedAAWrapperPass();
      (void) llvm::createScopedNoAliasAAWrapperPass();
      (void) llvm::createBreakCriticalEdgesPass();
      (void) llvm::createCallGraphDOTPrinterPass();
      (void) llvm::createCallGraphViewerPass();
      (void) llvm::createCFGSimplificationPass();
      (void) llvm::createStructurizeCFGPass();
      (void) llvm::createCostModelAnalysisPass();
      (void) llvm::createDeadArgEliminationPass();
      (void) llvm::createDeadArgEliminationSYCLPass();
      (void) llvm::createDeadCodeEliminationPass();
      (void) llvm::createDependenceAnalysisWrapperPass();
#if INTEL_CUSTOMIZATION
      (void) llvm::createGCOVProfilerPass();
#endif // INTEL_CUSTOMIZATION
      (void) llvm::createDomOnlyPrinterWrapperPassPass();
      (void) llvm::createDomPrinterWrapperPassPass();
      (void) llvm::createDomOnlyViewerWrapperPassPass();
      (void) llvm::createDomViewerWrapperPassPass();
      (void) llvm::createAlwaysInlinerLegacyPass();
      (void) llvm::createGlobalsAAWrapperPass();
      (void) llvm::createGuardWideningPass();
      (void) llvm::createLoopGuardWideningPass();
      (void) llvm::createInstSimplifyLegacyPass();
      (void) llvm::createInstructionCombiningPass();
      (void) llvm::createJMCInstrumenterPass();
      (void) llvm::createKCFIPass();
      (void) llvm::createLCSSAPass();
      (void) llvm::createLICMPass();
      (void) llvm::createLoopSinkPass();
      (void) llvm::createLazyValueInfoPass();
      (void) llvm::createLoopExtractorPass();
      (void) llvm::createLoopPredicationPass();
      (void) llvm::createLoopSimplifyPass();
      (void) llvm::createLoopSimplifyCFGPass();
      (void) llvm::createLoopStrengthReducePass();
      (void) llvm::createLoopUnrollPass();
#if INTEL_CUSTOMIZATION
      (void) llvm::createLoopUnswitchPass();
#endif // INTEL_CUSTOMIZATION
      (void) llvm::createLoopRotatePass();
      (void) llvm::createLowerConstantIntrinsicsPass();
      (void) llvm::createLowerGlobalDtorsLegacyPass();
      (void) llvm::createLowerInvokePass();
      (void) llvm::createLowerSwitchPass();
      (void) llvm::createNaryReassociatePass();
#if INTEL_CUSTOMIZATION
      (void) llvm::createObjCARCAAWrapperPass();
#endif // INTEL_CUSTOMIZATION
      (void) llvm::createObjCARCContractPass();
      (void) llvm::createPromoteMemoryToRegisterPass();
      (void) llvm::createDemoteRegisterToMemoryPass();
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
      (void) llvm::createSafeStackPass();
      (void) llvm::createSROAPass();
      (void) llvm::createSingleLoopExtractorPass();
      (void) llvm::createTailCallEliminationPass();
      (void)llvm::createTLSVariableHoistPass();
      (void) llvm::createIVSplitLegacyPass(); // INTEL
      (void) llvm::createUnifyFunctionExitNodesPass();
      (void) llvm::createInstCountPass();
      (void) llvm::createConstantHoistingPass();
      (void) llvm::createCodeGenPreparePass();
      (void) llvm::createEarlyCSEPass();
      (void) llvm::createMergedLoadStoreMotionPass();
      (void) llvm::createGVNPass();
      (void) llvm::createPostDomTree();
      (void) llvm::createMergeICmpsLegacyPass();
      (void) llvm::createExpandLargeDivRemPass();
      (void) llvm::createExpandMemCmpPass();
      (void) llvm::createExpandVectorPredicationPass();
#if INTEL_CUSTOMIZATION
      (void)llvm::createSYCLLowerWGScopePass();
      (void)llvm::createSYCLLowerESIMDPass();
      (void)llvm::createESIMDLowerLoadStorePass();
      (void)llvm::createESIMDVerifierPass();
      (void)llvm::createSPIRITTAnnotationsLegacyPass();
      (void)llvm::createSYCLLowerWGLocalMemoryLegacyPass();
      (void)llvm::createESIMDVerifierPass();
      (void)llvm::createSYCLLowerInvokeSimdPass();
#endif // INTEL_CUSTOMIZATION
      std::string buf;
      llvm::raw_string_ostream os(buf);
      (void) llvm::createPrintModulePass(os);
      (void) llvm::createPrintFunctionPass(os);
      (void) llvm::createSinkingPass();
      (void) llvm::createLowerAtomicPass();
      (void) llvm::createLoadStoreVectorizerPass();
#if INTEL_CUSTOMIZATION
      (void) llvm::createVPlanPragmaOmpOrderedSimdExtractPass();
      (void) llvm::createVPlanFunctionVectorizerPass();
#endif // INTEL_CUSTOMIZATION
      (void) llvm::createPartiallyInlineLibCallsPass();
      (void) llvm::createScalarizerPass();
      (void) llvm::createSeparateConstOffsetFromGEPPass();
      (void) llvm::createSpeculativeExecutionPass();
      (void) llvm::createSpeculativeExecutionIfHasBranchDivergencePass();
      (void) llvm::createStraightLineStrengthReducePass();
      (void)llvm::createScalarizeMaskedMemIntrinLegacyPass();
      (void) llvm::createHardwareLoopsLegacyPass();
      (void) llvm::createUnifyLoopExitsPass();
      (void) llvm::createFixIrreduciblePass();
      (void)llvm::createSelectOptimizePass();

      (void)new llvm::IntervalPartition();
      (void)new llvm::ScalarEvolutionWrapperPass();
      llvm::Function::Create(nullptr, llvm::GlobalValue::ExternalLinkage)->viewCFGOnly();
      llvm::RGPassManager RGM;
      llvm::TargetLibraryInfoImpl TLII;
      llvm::TargetLibraryInfo TLI(TLII);
      llvm::AliasAnalysis AA(TLI);
      llvm::BatchAAResults BAA(AA);
      llvm::AliasSetTracker X(BAA);
      X.add(nullptr, llvm::LocationSize::beforeOrAfterPointer(),
            llvm::AAMDNodes()); // for -print-alias-sets
      (void) llvm::AreStatisticsEnabled();
      (void) llvm::sys::RunningOnValgrind();

#if INTEL_CUSTOMIZATION
      (void) llvm::createLoadCoalescingPass();
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
      (void)llvm::createHIRTempCleanupPass();
      (void) llvm::createHIROptPredicatePass();
      (void) llvm::createHIROptVarPredicatePass();
      (void) llvm::createHIRUnrollAndJamPass();
      (void) llvm::createHIRParDirInsertPass();
      (void)llvm::createHIRVecDirInsertPass();
      (void) llvm::createHIRPMSymbolicTripCountCompleteUnrollLegacyPass();
      (void)llvm::createHIRScalarReplArrayPass();
      (void)llvm::createHIRPropagateCastedIVPass();
      (void) llvm::createHIRPrefetchingPass();
      (void) llvm::createHIRSinkingForPerfectLoopnestPass();
      (void)llvm::createHIRUndoSinkingForPerfectLoopnestPass();
      (void) llvm::createHIRNontemporalMarkingPass();
      (void) llvm::createHIRStoreResultIntoTempArrayPass();
      (void) llvm::createHIRSumWindowReusePass();
      (void) llvm::createHIRNonZeroSinkingForPerfectLoopnestPass();

      // Optimize math calls
      (void) llvm::createMapIntrinToImlPass();

      // VPO WRegion Passes
      (void) llvm::createWRegionCollectionWrapperPassPass();
      (void) llvm::createWRegionInfoWrapperPassPass();

      // VPO Vectorizer Passes
      (void) llvm::createVPODirectiveCleanupPass();
      (void) llvm::createVecClonePass();

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
