//==----- DPCPPKernelTransforms.cpp - passes initialization -*- C++ -*------==//
//
// Copyright (C) 2020-2022 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //

#include "llvm/InitializePasses.h"

using namespace llvm;

void llvm::initializeIntel_DPCPPKernelTransforms(PassRegistry &Registry) {
  initializeAddFastMathLegacyPass(Registry);
  initializeAddFunctionAttrsLegacyPass(Registry);
  initializeAddImplicitArgsLegacyPass(Registry);
  initializeAddNTAttrLegacyPass(Registry);
  initializeAddTLSGlobalsLegacyPass(Registry);
  initializeAutorunReplicatorLegacyPass(Registry);
  initializeBarrierInFunctionLegacyPass(Registry);
  initializeBuiltinCallToInstLegacyPass(Registry);
  initializeBuiltinImportLegacyPass(Registry);
  initializeBuiltinLibInfoAnalysisLegacyPass(Registry);
  initializeCleanupWrappedKernelLegacyPass(Registry);
  initializeCoerceTypesLegacyPass(Registry);
  initializeCoerceWin64TypesLegacyPass(Registry);
  initializeCreateSimdVariantPropagationLegacyPass(Registry);
  initializeDataPerBarrierWrapperPass(Registry);
  initializeDataPerValueWrapperPass(Registry);
  initializeDPCPPEqualizerLegacyPass(Registry);
  initializeDPCPPKernelAnalysisLegacyPass(Registry);
  initializeDPCPPKernelPostVecPass(Registry);
  initializeDPCPPKernelVecCloneLegacyPass(Registry);
  initializeDPCPPKernelWGLoopCreatorLegacyPass(Registry);
  initializeDPCPPPreprocessSPIRVFriendlyIRLegacyPass(Registry);
  initializeDeduceMaxWGDimLegacyPass(Registry);
  initializeDuplicateCalledKernelsLegacyPass(Registry);
  initializeExternalizeGlobalVariablesLegacyPass(Registry);
  initializeGroupBuiltinLegacyPass(Registry);
  initializeHandleVPlanMaskLegacyPass(Registry);
  initializeImplicitArgsAnalysisLegacyPass(Registry);
  initializeImplicitGIDLegacyPass(Registry);
  initializeIndirectCallLoweringLegacyPass(Registry);
  initializeInstToFuncCallLegacyPass(Registry);
  initializeInternalizeNonKernelFuncLegacyPass(Registry);
  initializeKernelBarrierLegacyPass(Registry);
  initializeLinearIdResolverLegacyPass(Registry);
  initializeLocalBufferAnalysisLegacyPass(Registry);
  initializeLocalBuffersLegacyPass(Registry);
  initializeLoopStridedCodeMotionLegacyPass(Registry);
  initializeLoopWIAnalysisLegacyPass(Registry);
  initializePhiCanonicalizationLegacyPass(Registry);
  initializePrepareKernelArgsLegacyPass(Registry);
  initializeReduceCrossBarrierValuesLegacyPass(Registry);
  initializeRedundantPhiNodeLegacyPass(Registry);
  initializeResolveMatrixWISliceLegacyPass(Registry);
  initializeResolveSubGroupWICallLegacyPass(Registry);
  initializeResolveMatrixFillLegacyPass(Registry);
  initializeResolveMatrixLayoutLegacyPass(Registry);
  initializeResolveWICallLegacyPass(Registry);
  initializeSetVectorizationFactorLegacyPass(Registry);
  initializeSGBarrierPropagateLegacyPass(Registry);
  initializeSGBarrierSimplifyLegacyPass(Registry);
  initializeSGBuiltinLegacyPass(Registry);
  initializeSGLoopConstructLegacyPass(Registry);
  initializeSGSizeAnalysisLegacyPass(Registry);
  initializeSGSizeCollectorLegacyPass(Registry);
  initializeSGSizeCollectorIndirectLegacyPass(Registry);
  initializeSGValueWidenLegacyPass(Registry);
  initializeSinCosFoldLegacyPass(Registry);
  initializeTaskSeqAsyncHandlingLegacyPass(Registry);
  initializeSoaAllocaAnalysisLegacyPass(Registry);
  initializeSplitBBonBarrierLegacyPass(Registry);
  initializeUpdateCallAttrsLegacyPass(Registry);
  initializeVectorVariantFillInLegacyPass(Registry);
  initializeVectorVariantLoweringLegacyPass(Registry);
  initializeVFAnalysisLegacyPass(Registry);
  initializeVectorizationDimensionAnalysisLegacyPass(Registry);
  initializeWGLoopBoundariesLegacyPass(Registry);
  initializeWorkItemAnalysisLegacyPass(Registry);
  initializeWIRelatedValueWrapperPass(Registry);
}

