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
#include "llvm/Analysis/BasicAliasAnalysis.h"
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
#endif // INTEL_FEATURE_CSA
#include "llvm/Analysis/Intel_StdContainerAA.h"
#endif // INTEL_CUSTOMIZATION

using namespace llvm;

/// initializeScalarOptsPasses - Initialize all passes linked into the
/// ScalarOpts library.
void llvm::initializeScalarOpts(PassRegistry &Registry) {
  initializeADCELegacyPassPass(Registry);
  initializeBDCELegacyPassPass(Registry);
  initializeAlignmentFromAssumptionsPass(Registry);
  initializeCallSiteSplittingLegacyPassPass(Registry);
  initializeConstantHoistingLegacyPassPass(Registry);
  initializeConvertGEPToSubscriptIntrinsicLegacyPassPass(Registry); // INTEL
  initializeCorrelatedValuePropagationPass(Registry);
  initializeDCELegacyPassPass(Registry);
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
  initializeLoopDataPrefetchLegacyPassPass(Registry);
  initializeLoopAccessLegacyAnalysisPass(Registry);
  initializeLoopInstSimplifyLegacyPassPass(Registry);
  initializeLoopPredicationLegacyPassPass(Registry);
  initializeLoopRotateLegacyPassPass(Registry);
  initializeLoopStrengthReducePass(Registry);
  initializeLoopUnrollPass(Registry);
#if INTEL_CUSTOMIZATION
  initializeLoopUnswitchPass(Registry);
#endif // INTEL_CUSTOMIZATION
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
  initializePlaceBackedgeSafepointsLegacyPassPass(Registry);
  initializePlaceSafepointsLegacyPassPass(Registry);
  initializeFloat2IntLegacyPassPass(Registry);
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
  initializeLoopSimplifyCFGLegacyPassPass(Registry);
  initializeIVSplitLegacyPassPass(Registry); // INTEL
}

void LLVMInitializeScalarOpts(LLVMPassRegistryRef R) {
  initializeScalarOpts(*unwrap(R));
}

#if INTEL_CUSTOMIZATION
void LLVMAddLoopUnswitchPass(LLVMPassManagerRef PM) {
  unwrap(PM)->add(createLoopUnswitchPass());
}
#endif // INTEL_CUSTOMIZATION

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

#if INTEL_CUSTOMIZATION
void LLVMAddStdContainerAAPass(LLVMPassManagerRef PM) {
  unwrap(PM)->add(createStdContainerAAWrapperPass());
}
#endif // INTEL_CUSTOMIZATION

#if INTEL_CUSTOMIZATION
void LLVMAddLowerSubscriptIntrinsicPass(LLVMPassManagerRef PM) {
  unwrap(PM)->add(createLowerSubscriptIntrinsicLegacyPass());
}

void LLVMAddConvertGEPToSubscriptIntrinsicPass(LLVMPassManagerRef PM) {
  unwrap(PM)->add(createConvertGEPToSubscriptIntrinsicLegacyPass());
}
#endif // INTEL_CUSTOMIZATION
