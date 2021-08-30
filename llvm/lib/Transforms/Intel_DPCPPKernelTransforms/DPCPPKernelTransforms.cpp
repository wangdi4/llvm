//==----- DPCPPKernelTransforms.cpp - passes initialization -*- C++ -*------==//
//
// Copyright (C) 2020-2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //

#include "llvm/InitializePasses.h"

using namespace llvm;

void llvm::initializeIntel_DPCPPKernelTransforms(PassRegistry &Registry) {
  initializeAddFunctionAttrsLegacyPass(Registry);
  initializeAddImplicitArgsLegacyPass(Registry);
  initializeBarrierInFunctionLegacyPass(Registry);
  initializeBuiltinCallToInstLegacyPass(Registry);
  initializeBuiltinImportLegacyPass(Registry);
  initializeCleanupWrappedKernelLegacyPass(Registry);
  initializeCreateSimdVariantPropagationLegacyPass(Registry);
  initializeDataPerBarrierWrapperPass(Registry);
  initializeDataPerValueWrapperPass(Registry);
  initializeDPCPPEqualizerLegacyPass(Registry);
  initializeDPCPPKernelAnalysisLegacyPass(Registry);
  initializeDPCPPKernelPostVecPass(Registry);
  initializeDPCPPKernelVecCloneLegacyPass(Registry);
  initializeDPCPPKernelWGLoopCreatorLegacyPass(Registry);
  initializeDPCPPPreprocessSPIRVFriendlyIRLegacyPass(Registry);
  initializeDuplicateCalledKernelsLegacyPass(Registry);
  initializeGroupBuiltinLegacyPass(Registry);
  initializeImplicitArgsAnalysisLegacyPass(Registry);
  initializeInternalizeNonKernelFuncLegacyPass(Registry);
  initializeKernelBarrierLegacyPass(Registry);
  initializeLinearIdResolverLegacyPass(Registry);
  initializeLocalBufferAnalysisLegacyPass(Registry);
  initializeLocalBuffersLegacyPass(Registry);
  initializePhiCanonicalizationLegacyPass(Registry);
  initializePrepareKernelArgsLegacyPass(Registry);
  initializeRedundantPhiNodeLegacyPass(Registry);
  initializeResolveWICallLegacyPass(Registry);
  initializeSetVectorizationFactorLegacyPass(Registry);
  initializeSGSizeCollectorLegacyPass(Registry);
  initializeSGSizeCollectorIndirectLegacyPass(Registry);
  initializeSplitBBonBarrierLegacyPass(Registry);
  initializeUpdateCallAttrsLegacyPass(Registry);
  initializeVectorVariantFillInLegacyPass(Registry);
  initializeVectorVariantLoweringLegacyPass(Registry);
  initializeWIRelatedValueWrapperPass(Registry);
}

