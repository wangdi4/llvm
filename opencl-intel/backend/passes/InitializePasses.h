// INTEL CONFIDENTIAL
//
// Copyright 2012-2018 Intel Corporation.
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

//===----------------------------------------------------------------------===//
//
// This file contains the declarations for the pass initialization routines
// for the OCL project.
//
//===----------------------------------------------------------------------===//

#ifndef OCL_INITIALIZEPASSES_H
#define OCL_INITIALIZEPASSES_H

namespace llvm {
    class PassRegistry;
}

namespace intel {
void initializePhiCanonPass(llvm::PassRegistry&);
void initializePredicatorPass(llvm::PassRegistry&);
void initializeWIAnalysisPass(llvm::PassRegistry&);
void initializeOCLBranchProbabilityPass(llvm::PassRegistry&);
void initializeScalarizeFunctionPass(llvm::PassRegistry&);
void initializeSimplifyGEPPass(llvm::PassRegistry&);
void initializePacketizeFunctionPass(llvm::PassRegistry&);
void initializeX86ResolverPass(llvm::PassRegistry&);
void initializeAVX512ResolverPass(llvm::PassRegistry&);
void initializeOCLBuiltinPreVectorizationPassPass(llvm::PassRegistry&);
void initializeSpecialCaseBuiltinResolverPass(llvm::PassRegistry&);
void initializeCLWGLoopBoundariesPass(llvm::PassRegistry&);
void initializeCLWGLoopCreatorPass(llvm::PassRegistry&);
void initializeCLStreamSamplerPass(llvm::PassRegistry&);
void initializeCleanupWrappedKernelsPass(llvm::PassRegistry&);
void initializeKernelAnalysisPass(llvm::PassRegistry&);
void initializeIRInjectModulePass(llvm::PassRegistry&);
void initializenameByInstTypePass(llvm::PassRegistry&);
void initializeDuplicateCalledKernelsPass(llvm::PassRegistry&);
void initializeRedundantPhiNodePass(llvm::PassRegistry&);
void initializeGroupBuiltinPass(llvm::PassRegistry&);
void initializeBarrierInFunctionPass(llvm::PassRegistry&);
void initializeRemoveDuplicationBarrierPass(llvm::PassRegistry&);
void initializeSplitBBonBarrierPass(llvm::PassRegistry&);
void initializeBarrierPass(llvm::PassRegistry&);
void initializeWIRelatedValuePass(llvm::PassRegistry&);
void initializeSinCosFoldPass(llvm::PassRegistry&);
void initializeDataPerBarrierPass(llvm::PassRegistry&);
void initializeDataPerValuePass(llvm::PassRegistry&);
void initializePreventDivCrashesPass(llvm::PassRegistry&);
void initializeBuiltinCallToInstPass(llvm::PassRegistry&);
void initializeInstToFuncCallPass(llvm::PassRegistry&);
void initializeInternalizeNonKernelFuncPass(llvm::PassRegistry&);
void initializeInternalizeGlobalVariablesPass(llvm::PassRegistry&);
void initializeAddImplicitArgsPass(llvm::PassRegistry&);
void initializeOclFunctionAttrsPass(llvm::PassRegistry&);
void initializeOclSyncFunctionAttrsPass(llvm::PassRegistry&);
void initializeBuiltinLibInfoPass(llvm::PassRegistry&);
void initializeLocalBuffersWrapperPass(llvm::PassRegistry&);
void initializeLocalBuffersWithDebugWrapperPass(llvm::PassRegistry&);
void initializeRelaxedPassPass(llvm::PassRegistry&);
void initializeShiftZeroUpperBitsPass(llvm::PassRegistry&);
void initializePrefetchPass(llvm::PassRegistry&);
void initializeLoopWIAnalysisPass(llvm::PassRegistry&);
void initializeSoaAllocaAnalysisPass(llvm::PassRegistry&);
void initializePostDominanceFrontierPass(llvm::PassRegistry&);
void initializeLocalBuffAnalysisPass(llvm::PassRegistry&);
void initializeBIImportPass(llvm::PassRegistry&);
void initializeGenericAddressStaticResolutionPass(llvm::PassRegistry&);
void initializeGenericAddressDynamicResolutionPass(llvm::PassRegistry&);
void initializeLLVMEqualizerPass(llvm::PassRegistry&);
void initializeSubGroupAdaptationPass(llvm::PassRegistry&);
void initializeLinearIdResolverPass(llvm::PassRegistry&);
void initializePrepareKernelArgsPass(llvm::PassRegistry&);
void initializeResolveWICallPass(llvm::PassRegistry&);
void initializeResolveSubGroupWICallPass(llvm::PassRegistry&);
void initializeResolveBlockToStaticCallPass(llvm::PassRegistry&);
void initializeDetectRecursionPass(llvm::PassRegistry&);
void initializeDebugInfoPassPass(llvm::PassRegistry&);
void initializeImplicitArgsAnalysisPass(llvm::PassRegistry&);
void initializeSmartGVNPass(llvm::PassRegistry&);
void initializeDeduceMaxWGDimPass(llvm::PassRegistry&);
void initializeRenderscriptVectorizerPass(llvm::PassRegistry&);
void initializeOCLAliasAnalysisPass(llvm::PassRegistry&);
void initializePrintfArgumentsPromotionPass(llvm::PassRegistry&);
void initializeChannelPipeTransformationPass(llvm::PassRegistry&);
void initializePipeIOTransformationPass(llvm::PassRegistry&);
void initializePipeSupportPass(llvm::PassRegistry&);
void initializePipeOrderingPass(llvm::PassRegistry&);
void initializeInfiniteLoopCreatorPass(llvm::PassRegistry&);
void initializeAutorunReplicatorPass(llvm::PassRegistry&);
void initializeImplicitGlobalIdPassPass(llvm::PassRegistry&);
void initializeStripIntelIPPass(llvm::PassRegistry&);
void initializeOCLReqdSubGroupSizePass(llvm::PassRegistry&);
void initializeOCLVecClonePass(llvm::PassRegistry&);
void initializeOCLPostVectPass(llvm::PassRegistry&);
void initializeChannelsUsageAnalysisPass(llvm::PassRegistry&);
void initializeKernelSubGroupInfoPass(llvm::PassRegistry&);
void initializeSYCLPipesHackPass(llvm::PassRegistry&);
void initializePatchCallbackArgsPass(llvm::PassRegistry &);
void initializeAddTLSGlobalsPass(llvm::PassRegistry &);
void initializeCoerceTypesPass(llvm::PassRegistry &);
void initializeWeightedInstCounterPass(llvm::PassRegistry &);
void initializeScalarizeFunctionPass(llvm::PassRegistry &);
}

#endif
