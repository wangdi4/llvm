// INTEL CONFIDENTIAL
//
// Copyright 2010-2020 Intel Corporation.
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you (License). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.
//
#ifndef INITIALIZE_OCL_PASSES_H
#define INITIALIZE_OCL_PASSES_H

#include "InitializePasses.h"

static void initializeOCLPasses(llvm::PassRegistry &Registry) {
    intel::initializePhiCanonPass(Registry);
    intel::initializePredicatorPass(Registry);
    intel::initializeWIAnalysisPass(Registry);
    intel::initializeScalarizeFunctionPass(Registry);
    intel::initializeSimplifyGEPPass(Registry);
    intel::initializePacketizeFunctionPass(Registry);
    intel::initializeX86ResolverPass(Registry);
    intel::initializeAVX512ResolverPass(Registry);
    intel::initializeOCLBuiltinPreVectorizationPassPass(Registry);
    intel::initializeSpecialCaseBuiltinResolverPass(Registry);
    intel::initializeOCLBuiltinPreVectorizationPassPass(Registry);
    intel::initializeCLBuiltinLICMPass(Registry);
    intel::initializeCLWGLoopCreatorPass(Registry);
    intel::initializeCLWGLoopBoundariesPass(Registry);
    intel::initializeCLStreamSamplerPass(Registry);
    intel::initializeCleanupWrappedKernelsPass(Registry);
    intel::initializeKernelAnalysisPass(Registry);
    intel::initializeIRInjectModulePass(Registry);
    intel::initializenameByInstTypePass(Registry);
    intel::initializeDuplicateCalledKernelsPass(Registry);
    intel::initializeRedundantPhiNodePass(Registry);
    intel::initializeGroupBuiltinPass(Registry);
    intel::initializeBarrierInFunctionPass(Registry);
    intel::initializeRemoveDuplicationBarrierPass(Registry);
    intel::initializeSplitBBonBarrierPass(Registry);
    intel::initializeBarrierPass(Registry);
    intel::initializeWIRelatedValuePass(Registry);
    intel::initializeDataPerBarrierPass(Registry);
    intel::initializeDataPerValuePass(Registry);
    intel::initializeReplaceScalarWithMaskPass(Registry);
    intel::initializePreventDivCrashesPass(Registry);
    intel::initializeBuiltinCallToInstPass(Registry);
    intel::initializeInstToFuncCallPass(Registry);
    intel::initializeInternalizeNonKernelFuncPass(Registry);
    intel::initializeInternalizeGlobalVariablesPass(Registry);
    intel::initializeAddImplicitArgsPass(Registry);
    intel::initializeOclFunctionAttrsPass(Registry);
    intel::initializeOclSyncFunctionAttrsPass(Registry);
    intel::initializeBuiltinLibInfoPass(Registry);
    intel::initializeLocalBuffAnalysisPass(Registry);
    intel::initializeLocalBuffersWrapperPass(Registry);
    intel::initializeLocalBuffersWithDebugWrapperPass(Registry);
    intel::initializeLoopStridedCodeMotionPass(Registry);
    intel::initializeRelaxedPassPass(Registry);
    intel::initializeShiftZeroUpperBitsPass(Registry);
    intel::initializePrefetchPass(Registry);
    intel::initializeBIImportPass(Registry);
    intel::initializeHandleVPlanMaskPass(Registry);
    intel::initializeGenericAddressStaticResolutionPass(Registry);
    intel::initializeGenericAddressDynamicResolutionPass(Registry);
    intel::initializeLLVMEqualizerPass(Registry);
    intel::initializeSubGroupAdaptationPass(Registry);
    intel::initializeLinearIdResolverPass(Registry);
    intel::initializePrepareKernelArgsPass(Registry);
    intel::initializeRemovePrefetchPass(Registry);
    intel::initializeResolveWICallPass(Registry);
    intel::initializeResolveSubGroupWICallPass(Registry);
    intel::initializeResolveBlockToStaticCallPass(Registry);
    intel::initializeDetectRecursionPass(Registry);
    intel::initializeDebugInfoPassPass(Registry);
    intel::initializeSmartGVNPass(Registry);
    intel::initializeDeduceMaxWGDimPass(Registry);
    intel::initializeRenderscriptVectorizerPass(Registry);
    intel::initializeSinCosFoldPass(Registry);
    intel::initializeOCLAliasAnalysisPass(Registry);
    intel::initializePrintfArgumentsPromotionPass(Registry);
    intel::initializeChannelPipeTransformationPass(Registry);
    intel::initializePipeIOTransformationPass(Registry);
    intel::initializePipeSupportPass(Registry);
    intel::initializePipeOrderingPass(Registry);
    intel::initializeInfiniteLoopCreatorPass(Registry);
    intel::initializeAutorunReplicatorPass(Registry);
    intel::initializeImplicitGlobalIdPassPass(Registry);
    intel::initializeImplicitArgsAnalysisPass(Registry);
    intel::initializeStripIntelIPPass(Registry);
    intel::initializeOCLReqdSubGroupSizePass(Registry);
    intel::initializeOCLVecClonePass(Registry);
    intel::initializeOCLPostVectPass(Registry);
    intel::initializeChannelsUsageAnalysisPass(Registry);
    intel::initializeKernelSubGroupInfoPass(Registry);
    intel::initializeSYCLPipesHackPass(Registry);
    intel::initializePatchCallbackArgsPass(Registry);
    intel::initializeAddTLSGlobalsPass(Registry);
    intel::initializeCoerceTypesPass(Registry);
    intel::initializeWeightedInstCounterPass(Registry);
    intel::initializeScalarizeFunctionPass(Registry);
    intel::initializeRemoveAtExitPass(Registry);
}
#endif //INITIALIZE_OCL_PASSES_H
