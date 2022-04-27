// Copyright 2010-2022 Intel Corporation.
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
    intel::initializeScalarizeFunctionPass(Registry);
    intel::initializeSimplifyGEPPass(Registry);
    intel::initializePacketizeFunctionPass(Registry);
    intel::initializeX86ResolverPass(Registry);
    intel::initializeAVX512ResolverPass(Registry);
    intel::initializeOCLBuiltinPreVectorizationPassPass(Registry);
    intel::initializeSpecialCaseBuiltinResolverPass(Registry);
    intel::initializeOCLBuiltinPreVectorizationPassPass(Registry);
    intel::initializeCLBuiltinLICMPass(Registry);
    intel::initializeCLStreamSamplerPass(Registry);
    intel::initializeRemoveDuplicationBarrierPass(Registry);

    intel::initializeResolveVariableTIDCallPass(Registry);
    intel::initializeReplaceScalarWithMaskPass(Registry);
    intel::initializePreventDivCrashesPass(Registry);
    intel::initializeExternalizeGlobalVariablesPass(Registry);
    intel::initializeInternalizeGlobalVariablesPass(Registry);
    intel::initializeOclFunctionAttrsPass(Registry);
    intel::initializeBuiltinLibInfoPass(Registry);
    intel::initializeRelaxedPassPass(Registry);
    intel::initializePrefetchPass(Registry);
    intel::initializeSubGroupAdaptationPass(Registry);
    intel::initializeRemovePrefetchPass(Registry);
    intel::initializeDetectRecursionPass(Registry);
    intel::initializeDebugInfoPassPass(Registry);
    intel::initializeSmartGVNPass(Registry);
    intel::initializeOCLAliasAnalysisPass(Registry);
    intel::initializeChannelPipeTransformationPass(Registry);
    intel::initializePipeIOTransformationPass(Registry);
    intel::initializePipeSupportPass(Registry);
    intel::initializePipeOrderingPass(Registry);
    intel::initializeInfiniteLoopCreatorPass(Registry);
    intel::initializeAutorunReplicatorPass(Registry);
    intel::initializeStripIntelIPPass(Registry);
    intel::initializeOCLReqdSubGroupSizePass(Registry);
    intel::initializeChannelsUsageAnalysisPass(Registry);
    intel::initializeKernelSubGroupInfoPass(Registry);
    intel::initializeSYCLPipesHackPass(Registry);
    intel::initializePatchCallbackArgsPass(Registry);
    intel::initializeWeightedInstCounterPass(Registry);
    intel::initializeScalarizeFunctionPass(Registry);
    intel::initializeRemoveAtExitPass(Registry);
    intel::initializeChooseVectorizationDimensionPass(Registry);
    intel::initializeVectorKernelDiscardPass(Registry);
    intel::initializeSetPreferVectorWidthPass(Registry);
    intel::initializeUndefExternalFuncsPass(Registry);
}
#endif //INITIALIZE_OCL_PASSES_H
