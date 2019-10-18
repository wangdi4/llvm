//===-- Intel_LoopTransforms.cpp-------------------------------------------===//
//
// Copyright (C) 2015-2019 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//

#include "llvm/InitializePasses.h"
#include "llvm/PassRegistry.h"

using namespace llvm;

void llvm::initializeIntel_LoopTransforms(PassRegistry &Registry) {
  initializeHIRSSADeconstructionLegacyPassPass(Registry);
  initializeHIRTempCleanupLegacyPassPass(Registry);
  initializeHIRSymbolicTripCountCompleteUnrollLegacyPassPass(Registry);
  initializeHIRLoopInterchangeLegacyPassPass(Registry);
  initializeHIROptPredicateLegacyPassPass(Registry);
  initializeHIRPreVecCompleteUnrollLegacyPassPass(Registry);
  initializeHIRPostVecCompleteUnrollLegacyPassPass(Registry);
  initializeHIRGeneralUnrollLegacyPassPass(Registry);
  initializeHIRUnrollAndJamLegacyPassPass(Registry);
  initializeHIRDummyTransformationPass(Registry);
  initializeHIRLoopReversalLegacyPassPass(Registry);
  initializeHIRLMMLegacyPassPass(Registry);
  initializeHIRLoopCollapseLegacyPassPass(Registry);
  initializeHIRRuntimeDDLegacyPassPass(Registry);
  initializeHIRScalarReplArrayLegacyPassPass(Registry);
  initializeHIRLoopBlockingLegacyPassPass(Registry);
  initializeHIRGenerateMKLCallLegacyPassPass(Registry);
  initializeHIRLoopDistributionForMemRecLegacyPassPass(Registry);
  initializeHIRLoopDistributionForLoopNestLegacyPassPass(Registry);
  initializeHIRLoopRematerializeLegacyPassPass(Registry);
  initializeHIRLoopRerollLegacyPassPass(Registry);
  initializeHIROptReportEmitterWrapperPassPass(Registry);
  initializeHIRCodeGenWrapperPassPass(Registry);
  initializeHIRParDirInsertPass(Registry);
  initializeHIRVecDirInsertPass(Registry);
  initializeHIROptVarPredicateLegacyPassPass(Registry);
  initializeHIRIdiomRecognitionLegacyPassPass(Registry);
  initializeHIRMVForConstUBLegacyPassPass(Registry);
  initializeHIRLoopConcatenationLegacyPassPass(Registry);
  initializeHIRArrayTransposeLegacyPassPass(Registry);
  initializeHIRAosToSoaLegacyPassPass(Registry);
  initializeHIRLoopFusionLegacyPassPass(Registry);
  initializeHIRDeadStoreEliminationLegacyPassPass(Registry);
  initializeHIRLastValueComputationLegacyPassPass(Registry);
  initializeHIRPropagateCastedIVLegacyPassPass(Registry);
  initializeHIRMultiExitLoopRerollLegacyPassPass(Registry);
  initializeHIRRecognizeParLoopPass(Registry);
  initializeHIRIdentityMatrixIdiomRecognitionLegacyPassPass(Registry);
  initializeHIRPrefetchingLegacyPassPass(Registry);
  initializeHIRSinkingForPerfectLoopnestLegacyPassPass(Registry);
  initializeHIRUndoSinkingForPerfectLoopnestLegacyPassPass(Registry);
  initializeHIRConditionalTempSinkingLegacyPassPass(Registry);
  initializeHIRMemoryReductionSinkingLegacyPassPass(Registry);
}
