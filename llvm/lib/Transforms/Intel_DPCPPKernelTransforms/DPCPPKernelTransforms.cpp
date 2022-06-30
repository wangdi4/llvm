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
  initializeBuiltinLICMLegacyPass(Registry);
  initializeBuiltinLibInfoAnalysisLegacyPass(Registry);
  initializeChannelPipeTransformationLegacyPass(Registry);
  initializeCleanupWrappedKernelLegacyPass(Registry);
  initializeCoerceTypesLegacyPass(Registry);
  initializeCoerceWin64TypesLegacyPass(Registry);
  initializeCreateSimdVariantPropagationLegacyPass(Registry);
  initializeDataPerBarrierWrapperPass(Registry);
  initializeDataPerValueWrapperPass(Registry);
  initializeDeduceMaxWGDimLegacyPass(Registry);
  initializeDetectRecursionLegacyPass(Registry);
  initializeDPCPPAliasAnalysisLegacyPass(Registry);
  initializeDPCPPExternalAliasAnalysisLegacyPass(Registry);
  initializeDPCPPEqualizerLegacyPass(Registry);
  initializeDPCPPKernelAnalysisLegacyPass(Registry);
  initializeDPCPPKernelPostVecPass(Registry);
  initializeDPCPPKernelVecCloneLegacyPass(Registry);
  initializeDPCPPKernelWGLoopCreatorLegacyPass(Registry);
  initializeDPCPPPreprocessSPIRVFriendlyIRLegacyPass(Registry);
  initializeDPCPPRewritePipesLegacyPass(Registry);
  initializeDuplicateCalledKernelsLegacyPass(Registry);
  initializeExternalizeGlobalVariablesLegacyPass(Registry);
  initializeGroupBuiltinLegacyPass(Registry);
  initializeHandleVPlanMaskLegacyPass(Registry);
  initializeImplicitArgsAnalysisLegacyPass(Registry);
  initializeImplicitGIDLegacyPass(Registry);
  initializeIndirectCallLoweringLegacyPass(Registry);
  initializeInferArgumentAliasLegacyPass(Registry);
  initializeInfiniteLoopCreatorLegacyPass(Registry);
  initializeInstToFuncCallLegacyPass(Registry);
  initializeInternalizeGlobalVariablesLegacyPass(Registry);
  initializeInternalizeNonKernelFuncLegacyPass(Registry);
  initializeKernelBarrierLegacyPass(Registry);
  initializeLinearIdResolverLegacyPass(Registry);
  initializeLocalBufferAnalysisLegacyPass(Registry);
  initializeLocalBuffersLegacyPass(Registry);
  initializeLoopStridedCodeMotionLegacyPass(Registry);
  initializeLoopWIAnalysisLegacyPass(Registry);
  initializeOptimizeIDivAndIRemLegacyPass(Registry);
  initializePatchCallbackArgsLegacyPass(Registry);
  initializePhiCanonicalizationLegacyPass(Registry);
  initializePipeIOTransformationLegacyPass(Registry);
  initializePipeOrderingLegacyPass(Registry);
  initializePipeSupportLegacyPass(Registry);
  initializePrepareKernelArgsLegacyPass(Registry);
  initializePreventDivCrashesLegacyPass(Registry);
  initializeProfilingInfoLegacyPass(Registry);
  initializeReduceCrossBarrierValuesLegacyPass(Registry);
  initializeRedundantPhiNodeLegacyPass(Registry);
  initializeRelaxedMathLegacyPass(Registry);
  initializeRemoveAtExitLegacyPass(Registry);
  initializeRemoveDuplicatedBarrierLegacyPass(Registry);
  initializeReplaceScalarWithMaskLegacyPass(Registry);
  initializeReqdSubGroupSizeLegacyPass(Registry);
  initializeResolveMatrixFillLegacyPass(Registry);
  initializeResolveMatrixLayoutLegacyPass(Registry);
  initializeResolveMatrixWISliceLegacyPass(Registry);
  initializeResolveSubGroupWICallLegacyPass(Registry);
  initializeResolveVarTIDCallLegacyPass(Registry);
  initializeResolveWICallLegacyPass(Registry);
  initializeSetPreferVectorWidthLegacyPass(Registry);
  initializeSetVectorizationFactorLegacyPass(Registry);
  initializeSGBarrierPropagateLegacyPass(Registry);
  initializeSGBarrierSimplifyLegacyPass(Registry);
  initializeSGBuiltinLegacyPass(Registry);
  initializeSGLoopConstructLegacyPass(Registry);
  initializeSGSizeAnalysisLegacyPass(Registry);
  initializeSGSizeCollectorIndirectLegacyPass(Registry);
  initializeSGSizeCollectorLegacyPass(Registry);
  initializeSGValueWidenLegacyPass(Registry);
  initializeSinCosFoldLegacyPass(Registry);
  initializeSoaAllocaAnalysisLegacyPass(Registry);
  initializeSplitBBonBarrierLegacyPass(Registry);
  initializeTaskSeqAsyncHandlingLegacyPass(Registry);
  initializeUpdateCallAttrsLegacyPass(Registry);
  initializeVectorizationDimensionAnalysisLegacyPass(Registry);
  initializeVectorKernelEliminationLegacyPass(Registry);
  initializeVectorVariantFillInLegacyPass(Registry);
  initializeVectorVariantLoweringLegacyPass(Registry);
  initializeVFAnalysisLegacyPass(Registry);
  initializeWGLoopBoundariesLegacyPass(Registry);
  initializeWIRelatedValueWrapperPass(Registry);
  initializeWeightedInstCountAnalysisLegacyPass(Registry);
  initializeWorkItemAnalysisLegacyPass(Registry);
}

