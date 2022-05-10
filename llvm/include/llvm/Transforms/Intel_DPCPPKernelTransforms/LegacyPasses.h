//==- LegacyPasses.h - Legacy constructors for DPCPP kernel transforms -----==//
//
// Copyright (C) 2021-2022 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //
#ifndef LLVM_TRANSFORMS_INTEL_DPCPP_KERNEL_TRANSFORMS_LEGACY_PASSES_H
#define LLVM_TRANSFORMS_INTEL_DPCPP_KERNEL_TRANSFORMS_LEGACY_PASSES_H

#include "llvm/ADT/StringSet.h"
#include "llvm/IR/Intel_VectorVariant.h" // for VectorVariant::ISAClass

namespace llvm {

class FunctionPass;
class ImmutablePass;
class LoopPass;
class ModulePass;
class Pass;
class StringRef;
template <typename T, unsigned N> class SmallVector;

FunctionPass *createAddFastMathLegacyPass();
FunctionPass *createAddNTAttrLegacyPass();
FunctionPass *createBuiltinCallToInstLegacyPass();
FunctionPass *createPhiCanonicalizationLegacyPass();
FunctionPass *createRedundantPhiNodeLegacyPass();
FunctionPass *createSinCosFoldLegacyPass();
FunctionPass *createSoaAllocaAnalysisLegacyPass();
FunctionPass *createWorkItemAnalysisLegacyPass(unsigned VectorizeDim = 0);
ImmutablePass *
createBuiltinLibInfoAnalysisLegacyPass(ArrayRef<Module *> BuiltinModules = {});
ImmutablePass *createDPCPPAliasAnalysisLegacyPass();
ImmutablePass *createDPCPPExternalAliasAnalysisLegacyPass();
LoopPass *createLoopStridedCodeMotionLegacyPass();
LoopPass *createLoopWIAnalysisLegacyPass();
ModulePass *createAddFunctionAttrsLegacyPass();
ModulePass *createAddImplicitArgsLegacyPass();
ModulePass *createAddTLSGlobalsLegacyPass();
ModulePass *createAutorunReplicatorLegacyPass();
ModulePass *createBarrierInFunctionLegacyPass();
ModulePass *createBuiltinImportLegacyPass(
    const SmallVector<Module *, 2> &BuiltinModules = SmallVector<Module *, 2>(),
    StringRef CPUPrefix = "");
ModulePass *createCleanupWrappedKernelLegacyPass();
ModulePass *createCoerceTypesLegacyPass();
ModulePass *createCoerceWin64TypesLegacyPass();
ModulePass *createCreateSimdVariantPropagationLegacyPass();
ModulePass *createDataPerBarrierWrapperPass();
ModulePass *createDataPerValueWrapperPass();
ModulePass *createDeduceMaxWGDimLegacyPass();
ModulePass *
createDPCPPEqualizerLegacyPass(ArrayRef<Module *> BuiltinModules = {});
ModulePass *createDPCPPKernelAnalysisLegacyPass();
ModulePass *createDPCPPKernelPostVecPass();
ModulePass *createDPCPPKernelVecClonePass(
    ArrayRef<std::tuple<const char *, const char *, const char *>> VectInfos =
        {},
    VectorVariant::ISAClass ISA = VectorVariant::XMM, bool IsOCL = false);
ModulePass *createDPCPPKernelWGLoopCreatorLegacyPass();
ModulePass *createDPCPPPreprocessSPIRVFriendlyIRLegacyPass();
ModulePass *createDPCPPRewritePipesLegacyPass();
ModulePass *createDuplicateCalledKernelsLegacyPass();
ModulePass *createExternalizeGlobalVariablesLegacyPass();
ModulePass *
createGroupBuiltinLegacyPass(ArrayRef<Module *> BuiltinModules = {});
ModulePass *
createHandleVPlanMaskLegacyPass(const StringSet<> *VPlanMaskedFuncs);
ModulePass *createImplicitArgsAnalysisLegacyPass();
ModulePass *createImplicitGIDLegacyPass(bool HandleBarrier = true);
ModulePass *createIndirectCallLoweringLegacyPass();
ModulePass *createInstToFuncCallLegacyPass(
    VectorVariant::ISAClass ISA = VectorVariant::XMM);
ModulePass *createInternalizeGlobalVariablesLegacyPass();
ModulePass *createInternalizeNonKernelFuncLegacyPass();
ModulePass *createKernelBarrierLegacyPass(bool isNativeDebug,
                                          bool useTLSGlobals);
ModulePass *createLinearIdResolverPass();
ModulePass *createLocalBufferAnalysisLegacyPass();
ModulePass *createLocalBuffersLegacyPass(bool UseTLSGlobals);
ModulePass *createPipeOrderingLegacyPass();
ModulePass *createPrepareKernelArgsLegacyPass(bool UseTLSGlobals);
ModulePass *createProfilingInfoLegacyPass();
ModulePass *createReduceCrossBarrierValuesLegacyPass();
ModulePass *createRemoveAtExitLegacyPass();
ModulePass *createResolveMatrixFillLegacyPass();
ModulePass *createResolveMatrixLayoutLegacyPass();
ModulePass *createResolveMatrixWISliceLegacyPass();
ModulePass *createResolveSubGroupWICallLegacyPass(
    const SmallVector<Module *, 2> &BuiltinModules = SmallVector<Module *, 2>(),
    bool ResolveSGBarrier = true);
ModulePass *createResolveVarTIDCallLegacyPass();
ModulePass *createResolveWICallLegacyPass(bool IsUniformWGSize,
                                          bool UseTLSGlobals);
ModulePass *createSetVectorizationFactorLegacyPass(
    VectorVariant::ISAClass ISA = VectorVariant::XMM);
ModulePass *createSGBarrierPropagateLegacyPass();
ModulePass *createSGBarrierSimplifyLegacyPass();
ModulePass *createSGBuiltinLegacyPass(
    ArrayRef<std::tuple<const char *, const char *, const char *>> VectInfos =
        {});
ModulePass *createSGLoopConstructLegacyPass();
ModulePass *createSGSizeAnalysisLegacyPass();
ModulePass *createSGSizeCollectorIndirectLegacyPass(VectorVariant::ISAClass);
ModulePass *createSGSizeCollectorLegacyPass(VectorVariant::ISAClass);
ModulePass *createSGValueWidenLegacyPass();
ModulePass *createSplitBBonBarrierLegacyPass();
ModulePass *createTaskSeqAsyncHandlingLegacyPass();
ModulePass *createUpdateCallAttrsLegacyPass();
ModulePass *createVectorizationDimensionAnalysisLegacyPass();
ModulePass *createVectorVariantFillInLegacyPass();
ModulePass *createVectorVariantLoweringLegacyPass(VectorVariant::ISAClass);
ModulePass *createVFAnalysisLegacyPass();
ModulePass *createWGLoopBoundariesLegacyPass();
ModulePass *createWIRelatedValueWrapperPass();
Pass *createParseAnnotateAttributesPass();
} // namespace llvm

#endif // LLVM_TRANSFORMS_INTEL_DPCPP_KERNEL_TRANSFORMS_LEGACY_PASSES_H
