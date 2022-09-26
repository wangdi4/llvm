//===-- Scalar.cpp --------------------------------------------------------===//
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
// This file implements common infrastructure for libLLVMScalarOpts.a, which
// implements several scalar transformations over the LLVM intermediate
// representation, including the C bindings for that library.
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/Scalar.h"
#include "llvm-c/Initialization.h"
#include "llvm-c/Transforms/Scalar.h"
#include "llvm/Analysis/BasicAliasAnalysis.h"
#include "llvm/Analysis/Intel_StdContainerAA.h"  // INTEL
#include "llvm/Analysis/ScopedNoAliasAA.h"
#include "llvm/Analysis/TypeBasedAliasAnalysis.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Verifier.h"
#include "llvm/InitializePasses.h"
#include "llvm/Transforms/Scalar/GVN.h"
#include "llvm/Transforms/Scalar/Scalarizer.h"
#include "llvm/Transforms/Utils/UnifyFunctionExitNodes.h"
#if INTEL_CUSTOMIZATION
#if INTEL_FEATURE_CSA
#include "Intel_CSA/CSAIRPasses.h"
#endif  // INTEL_FEATURE_CSA
#endif  // INTEL_CUSTOMIZATION

using namespace llvm;

/// initializeScalarOptsPasses - Initialize all passes linked into the
/// ScalarOpts library.
void llvm::initializeScalarOpts(PassRegistry &Registry) {
  initializeADCELegacyPassPass(Registry);
  initializeAnnotationRemarksLegacyPass(Registry);
  initializeBDCELegacyPassPass(Registry);
  initializeAlignmentFromAssumptionsPass(Registry);
  initializeCallSiteSplittingLegacyPassPass(Registry);
  initializeConstantHoistingLegacyPassPass(Registry);
  initializeConvertGEPToSubscriptIntrinsicLegacyPassPass(Registry); // INTEL
  initializeConstraintEliminationPass(Registry);
  initializeCorrelatedValuePropagationPass(Registry);
  initializeDCELegacyPassPass(Registry);
  initializeDivRemPairsLegacyPassPass(Registry);
  initializeScalarizerLegacyPassPass(Registry);
  initializeDSELegacyPassPass(Registry);
  initializeGuardWideningLegacyPassPass(Registry);
  initializeLoopGuardWideningLegacyPassPass(Registry);
  initializeGVNLegacyPassPass(Registry);
  initializeNewGVNLegacyPassPass(Registry);
  initializeEarlyCSELegacyPassPass(Registry);
  initializeEarlyCSEMemSSALegacyPassPass(Registry);
  initializeMakeGuardsExplicitLegacyPassPass(Registry);
  initializeGVNHoistLegacyPassPass(Registry);
  initializeGVNSinkLegacyPassPass(Registry);
  initializeFlattenCFGLegacyPassPass(Registry);
  initializeIRCELegacyPassPass(Registry);
  initializeIndVarSimplifyLegacyPassPass(Registry);
  initializeInferAddressSpacesPass(Registry);
  initializeInstSimplifyLegacyPassPass(Registry);
  initializeJumpThreadingPass(Registry);
  initializeDFAJumpThreadingLegacyPassPass(Registry);
  initializeLegacyLICMPassPass(Registry);
  initializeLegacyLoopSinkPassPass(Registry);
  initializeLoopFuseLegacyPass(Registry);
  initializeLoopDataPrefetchLegacyPassPass(Registry);
  initializeLoopDeletionLegacyPassPass(Registry);
  initializeLoopAccessLegacyAnalysisPass(Registry);
  initializeLoopInstSimplifyLegacyPassPass(Registry);
  initializeLoopInterchangeLegacyPassPass(Registry);
  initializeLoopFlattenLegacyPassPass(Registry);
  initializeLoopPredicationLegacyPassPass(Registry);
  initializeLoopRotateLegacyPassPass(Registry);
  initializeLoopStrengthReducePass(Registry);
  initializeLoopRerollLegacyPassPass(Registry);
  initializeLoopUnrollPass(Registry);
  initializeLoopUnrollAndJamPass(Registry);
#if INTEL_CUSTOMIZATION
  initializeLoopUnswitchPass(Registry);
#endif // INTEL_CUSTOMIZATION
  initializeWarnMissedTransformationsLegacyPass(Registry);
  initializeLoopVersioningLICMLegacyPassPass(Registry);
  initializeLoopIdiomRecognizeLegacyPassPass(Registry);
  initializeLowerAtomicLegacyPassPass(Registry);
  initializeLowerConstantIntrinsicsPass(Registry);
  initializeLowerExpectIntrinsicPass(Registry);
  initializeLowerGuardIntrinsicLegacyPassPass(Registry);
  initializeLowerMatrixIntrinsicsLegacyPassPass(Registry);
  initializeLowerMatrixIntrinsicsMinimalLegacyPassPass(Registry);
  initializeLowerWidenableConditionLegacyPassPass(Registry);
  initializeLowerSubscriptIntrinsicLegacyPassPass(Registry); // INTEL
  initializeMemCpyOptLegacyPassPass(Registry);
  initializeMergeICmpsLegacyPassPass(Registry);
  initializeMergedLoadStoreMotionLegacyPassPass(Registry);
  initializeNaryReassociateLegacyPassPass(Registry);
  initializePartiallyInlineLibCallsLegacyPassPass(Registry);
  initializeReassociateLegacyPassPass(Registry);
  initializeRedundantDbgInstEliminationPass(Registry);
  initializeRegToMemLegacyPass(Registry);
  initializeRewriteStatepointsForGCLegacyPassPass(Registry);
  initializeScalarizeMaskedMemIntrinLegacyPassPass(Registry);
  initializeSCCPLegacyPassPass(Registry);
  initializeSROALegacyPassPass(Registry);
  initializeSROALegacyCGSCCAdaptorPassPass(Registry); // INTEL
  initializeCFGSimplifyPassPass(Registry);
  initializeStructurizeCFGLegacyPassPass(Registry);
  initializeSimpleLoopUnswitchLegacyPassPass(Registry);
  initializeSinkingLegacyPassPass(Registry);
  initializeTailCallElimPass(Registry);
  initializeTLSVariableHoistLegacyPassPass(Registry);
  initializeSeparateConstOffsetFromGEPLegacyPassPass(Registry);
  initializeSpeculativeExecutionLegacyPassPass(Registry);
  initializeStraightLineStrengthReduceLegacyPassPass(Registry);
  initializePlaceBackedgeSafepointsImplPass(Registry);
  initializePlaceSafepointsPass(Registry);
  initializeFloat2IntLegacyPassPass(Registry);
  initializeLoopDistributeLegacyPass(Registry);
#if INTEL_CUSTOMIZATION
  initializeDopeVectorHoistWrapperPass(Registry);
  initializeNonLTOGlobalOptLegacyPassPass(Registry);
#if INTEL_FEATURE_SW_ADVANCED
  initializeFunctionRecognizerLegacyPassPass(Registry);
  initializeNontemporalStoreWrapperPassPass(Registry);
#endif // INTEL_FEATURE_SW_ADVANCED
  initializeIndirectCallConvLegacyPassPass(Registry);
  initializeStdContainerOptLegacyPassPass(Registry);
  initializeTbaaMDPropagationLegacyPassPass(Registry);
  initializeCleanupFakeLoadsLegacyPassPass(Registry);
  initializeMultiVersioningWrapperPass(Registry);
  initializeLoopOptMarkerLegacyPassPass(Registry);
  initializeOptReportEmitterLegacyPassPass(Registry);
  initializeRemoveRegionDirectivesLegacyPassPass(Registry);
  initializeTransformFPGARegPass(Registry);
  initializeAddSubReassociateLegacyPassPass(Registry);
  initializeForcedCMOVGenerationLegacyPassPass(Registry);
  initializeTransformSinAndCosCallsLegacyPassPass(Registry);
  initializeHandlePragmaVectorAlignedLegacyPassPass(Registry);
  initializeIntelLoopAttrsWrapperPass(Registry);
#if INTEL_FEATURE_CSA
  initializeCSAScalarPasses(Registry);
#endif // INTEL_FEATURE_CSA
  initializeLoopCarriedCSELegacyPass(Registry);
#endif // INTEL_CUSTOMIZATION
  initializeLoopLoadEliminationPass(Registry);
  initializeLoopSimplifyCFGLegacyPassPass(Registry);
  initializeLoopVersioningLegacyPassPass(Registry);
  initializeIVSplitLegacyPassPass(Registry); // INTEL
}

void LLVMAddLoopSimplifyCFGPass(LLVMPassManagerRef PM) {
  unwrap(PM)->add(createLoopSimplifyCFGPass());
}

void LLVMInitializeScalarOpts(LLVMPassRegistryRef R) {
  initializeScalarOpts(*unwrap(R));
}

void LLVMAddAggressiveDCEPass(LLVMPassManagerRef PM) {
  unwrap(PM)->add(createAggressiveDCEPass());
}

void LLVMAddDCEPass(LLVMPassManagerRef PM) {
  unwrap(PM)->add(createDeadCodeEliminationPass());
}

void LLVMAddBitTrackingDCEPass(LLVMPassManagerRef PM) {
  unwrap(PM)->add(createBitTrackingDCEPass());
}

void LLVMAddAlignmentFromAssumptionsPass(LLVMPassManagerRef PM) {
  unwrap(PM)->add(createAlignmentFromAssumptionsPass());
}

void LLVMAddCFGSimplificationPass(LLVMPassManagerRef PM) {
  unwrap(PM)->add(createCFGSimplificationPass());
}

void LLVMAddDeadStoreEliminationPass(LLVMPassManagerRef PM) {
  unwrap(PM)->add(createDeadStoreEliminationPass());
}

void LLVMAddScalarizerPass(LLVMPassManagerRef PM) {
  unwrap(PM)->add(createScalarizerPass());
}

void LLVMAddGVNPass(LLVMPassManagerRef PM) {
  unwrap(PM)->add(createGVNPass());
}

void LLVMAddNewGVNPass(LLVMPassManagerRef PM) {
  unwrap(PM)->add(createNewGVNPass());
}

void LLVMAddMergedLoadStoreMotionPass(LLVMPassManagerRef PM) {
  unwrap(PM)->add(createMergedLoadStoreMotionPass());
}

void LLVMAddIndVarSimplifyPass(LLVMPassManagerRef PM) {
  unwrap(PM)->add(createIndVarSimplifyPass());
}

void LLVMAddInstructionSimplifyPass(LLVMPassManagerRef PM) {
  unwrap(PM)->add(createInstSimplifyLegacyPass());
}

void LLVMAddJumpThreadingPass(LLVMPassManagerRef PM) {
  unwrap(PM)->add(createJumpThreadingPass());
}

void LLVMAddLoopSinkPass(LLVMPassManagerRef PM) {
  unwrap(PM)->add(createLoopSinkPass());
}

void LLVMAddLICMPass(LLVMPassManagerRef PM) {
  unwrap(PM)->add(createLICMPass());
}

void LLVMAddLoopDeletionPass(LLVMPassManagerRef PM) {
  unwrap(PM)->add(createLoopDeletionPass());
}

void LLVMAddLoopFlattenPass(LLVMPassManagerRef PM) {
  unwrap(PM)->add(createLoopFlattenPass());
}

void LLVMAddLoopIdiomPass(LLVMPassManagerRef PM) {
  unwrap(PM)->add(createLoopIdiomPass());
}

void LLVMAddLoopRotatePass(LLVMPassManagerRef PM) {
  unwrap(PM)->add(createLoopRotatePass());
}

void LLVMAddLoopRerollPass(LLVMPassManagerRef PM) {
  unwrap(PM)->add(createLoopRerollPass());
}

void LLVMAddLoopUnrollPass(LLVMPassManagerRef PM) {
  unwrap(PM)->add(createLoopUnrollPass());
}

void LLVMAddLoopUnrollAndJamPass(LLVMPassManagerRef PM) {
  unwrap(PM)->add(createLoopUnrollAndJamPass());
}

#if INTEL_CUSTOMIZATION
void LLVMAddLoopUnswitchPass(LLVMPassManagerRef PM) {
  unwrap(PM)->add(createLoopUnswitchPass());
}
#endif // INTEL_CUSTOMIZATION

void LLVMAddLowerAtomicPass(LLVMPassManagerRef PM) {
  unwrap(PM)->add(createLowerAtomicPass());
}

void LLVMAddMemCpyOptPass(LLVMPassManagerRef PM) {
  unwrap(PM)->add(createMemCpyOptPass());
}

void LLVMAddPartiallyInlineLibCallsPass(LLVMPassManagerRef PM) {
  unwrap(PM)->add(createPartiallyInlineLibCallsPass());
}

void LLVMAddReassociatePass(LLVMPassManagerRef PM) {
  unwrap(PM)->add(createReassociatePass());
}

#if INTEL_CUSTOMIZATION
void LLVMAddAddSubReassociatePass(LLVMPassManagerRef PM) {
  unwrap(PM)->add(createAddSubReassociatePass());
}
#endif // INTEL_CUSTOMIZATION

#if INTEL_CUSTOMIZATION
void LLVMAddForcedCMOVGenerationPass(LLVMPassManagerRef PM) {
  unwrap(PM)->add(createForcedCMOVGenerationPass());
}
#endif // INTEL_CUSTOMIZATION

#if INTEL_CUSTOMIZATION
void LLVMAddTransformSinAndCosCallsPass(LLVMPassManagerRef PM) {
  unwrap(PM)->add(createTransformSinAndCosCallsPass());
}
#endif // INTEL_CUSTOMIZATION

void LLVMAddSCCPPass(LLVMPassManagerRef PM) {
  unwrap(PM)->add(createSCCPPass());
}

void LLVMAddScalarReplAggregatesPass(LLVMPassManagerRef PM) {
  unwrap(PM)->add(createSROAPass());
}

void LLVMAddScalarReplAggregatesPassSSA(LLVMPassManagerRef PM) {
  unwrap(PM)->add(createSROAPass());
}

void LLVMAddScalarReplAggregatesPassWithThreshold(LLVMPassManagerRef PM,
                                                  int Threshold) {
  unwrap(PM)->add(createSROAPass());
}

void LLVMAddSimplifyLibCallsPass(LLVMPassManagerRef PM) {
  // NOTE: The simplify-libcalls pass has been removed.
}

void LLVMAddTailCallEliminationPass(LLVMPassManagerRef PM) {
  unwrap(PM)->add(createTailCallEliminationPass());
}

void LLVMAddDemoteMemoryToRegisterPass(LLVMPassManagerRef PM) {
  unwrap(PM)->add(createDemoteRegisterToMemoryPass());
}

void LLVMAddVerifierPass(LLVMPassManagerRef PM) {
  unwrap(PM)->add(createVerifierPass());
}

void LLVMAddCorrelatedValuePropagationPass(LLVMPassManagerRef PM) {
  unwrap(PM)->add(createCorrelatedValuePropagationPass());
}

void LLVMAddEarlyCSEPass(LLVMPassManagerRef PM) {
  unwrap(PM)->add(createEarlyCSEPass(false/*=UseMemorySSA*/));
}

void LLVMAddEarlyCSEMemSSAPass(LLVMPassManagerRef PM) {
  unwrap(PM)->add(createEarlyCSEPass(true/*=UseMemorySSA*/));
}

void LLVMAddGVNHoistLegacyPass(LLVMPassManagerRef PM) {
  unwrap(PM)->add(createGVNHoistPass());
}

void LLVMAddTypeBasedAliasAnalysisPass(LLVMPassManagerRef PM) {
  unwrap(PM)->add(createTypeBasedAAWrapperPass());
}

void LLVMAddScopedNoAliasAAPass(LLVMPassManagerRef PM) {
  unwrap(PM)->add(createScopedNoAliasAAWrapperPass());
}

#if INTEL_CUSTOMIZATION
void LLVMAddStdContainerAAPass(LLVMPassManagerRef PM) {
  unwrap(PM)->add(createStdContainerAAWrapperPass());
}

void LLVMAddStdContainerOptPass(LLVMPassManagerRef PM) {
  unwrap(PM)->add(createStdContainerOptPass());
}
#endif // INTEL_CUSTOMIZATION

void LLVMAddBasicAliasAnalysisPass(LLVMPassManagerRef PM) {
  unwrap(PM)->add(createBasicAAWrapperPass());
}

void LLVMAddLowerConstantIntrinsicsPass(LLVMPassManagerRef PM) {
  unwrap(PM)->add(createLowerConstantIntrinsicsPass());
}

void LLVMAddLowerExpectIntrinsicPass(LLVMPassManagerRef PM) {
  unwrap(PM)->add(createLowerExpectIntrinsicPass());
}

#if INTEL_CUSTOMIZATION
void LLVMAddLowerSubscriptIntrinsicPass(LLVMPassManagerRef PM) {
  unwrap(PM)->add(createLowerSubscriptIntrinsicLegacyPass());
}

void LLVMAddConvertGEPToSubscriptIntrinsicPass(LLVMPassManagerRef PM) {
  unwrap(PM)->add(createConvertGEPToSubscriptIntrinsicLegacyPass());
}
#endif // INTEL_CUSTOMIZATION

void LLVMAddUnifyFunctionExitNodesPass(LLVMPassManagerRef PM) {
  unwrap(PM)->add(createUnifyFunctionExitNodesPass());
}
