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

#include "llvm/IR/LegacyPassManager.h" // INTEL
#include "llvm/InitializePasses.h"

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
  initializeConstantHoistingLegacyPassPass(Registry);
  initializeConvertGEPToSubscriptIntrinsicLegacyPassPass(Registry); // INTEL
  initializeDCELegacyPassPass(Registry);
  initializeScalarizerLegacyPassPass(Registry);
  initializeGuardWideningLegacyPassPass(Registry);
  initializeLoopGuardWideningLegacyPassPass(Registry);
  initializeGVNLegacyPassPass(Registry);
  initializeEarlyCSELegacyPassPass(Registry);
  initializeEarlyCSEMemSSALegacyPassPass(Registry);
  initializeMakeGuardsExplicitLegacyPassPass(Registry);
  initializeFlattenCFGLegacyPassPass(Registry);
  initializeInferAddressSpacesPass(Registry);
  initializeInstSimplifyLegacyPassPass(Registry);
  initializeLegacyLICMPassPass(Registry);
  initializeLegacyLoopSinkPassPass(Registry);
  initializeLoopDataPrefetchLegacyPassPass(Registry);
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
  initializeLowerWidenableConditionLegacyPassPass(Registry);
  initializeLowerSubscriptIntrinsicLegacyPassPass(Registry); // INTEL
  initializeMergeICmpsLegacyPassPass(Registry);
  initializeMergedLoadStoreMotionLegacyPassPass(Registry);
  initializeNaryReassociateLegacyPassPass(Registry);
  initializePartiallyInlineLibCallsLegacyPassPass(Registry);
  initializeReassociateLegacyPassPass(Registry);
  initializeRedundantDbgInstEliminationPass(Registry);
  initializeRegToMemLegacyPass(Registry);
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
#if INTEL_CUSTOMIZATION
  initializeNonLTOGlobalOptLegacyPassPass(Registry);
#if INTEL_FEATURE_SW_ADVANCED
  initializeNontemporalStoreWrapperPassPass(Registry);
#endif // INTEL_FEATURE_SW_ADVANCED
  initializeStdContainerOptLegacyPassPass(Registry);
  initializeTbaaMDPropagationLegacyPassPass(Registry);
  initializeCleanupFakeLoadsLegacyPassPass(Registry);
  initializeLoopOptMarkerLegacyPassPass(Registry);
  initializeOptReportEmitterLegacyPassPass(Registry);
  initializeTransformFPGARegPass(Registry);
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
