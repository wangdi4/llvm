//===-- Intel_LoopTransforms.cpp-------------------------------------------===//
//
// Copyright (C) 2015 Intel Corporation. All rights reserved.
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
  initializeHIRPMSymbolicTripCountCompleteUnrollLegacyPassPass(Registry);
  initializeHIRLoopInterchangeLegacyPassPass(Registry);
  initializeHIROptPredicateLegacyPassPass(Registry);
  initializeHIRUnrollAndJamLegacyPassPass(Registry);
  initializeHIRLoopReversalLegacyPassPass(Registry);
  initializeHIRLoopCollapseLegacyPassPass(Registry);
  initializeHIRRuntimeDDLegacyPassPass(Registry);
  initializeHIRScalarReplArrayLegacyPassPass(Registry);
  initializeHIRLoopBlockingLegacyPassPass(Registry);
  initializeHIRPragmaLoopBlockingLegacyPassPass(Registry);
  initializeHIRLoopDistributionForMemRecLegacyPassPass(Registry);
  initializeHIRLoopDistributionForLoopNestLegacyPassPass(Registry);
  initializeHIRLoopRematerializeLegacyPassPass(Registry);
  initializeHIRLoopRerollLegacyPassPass(Registry);
  initializeHIROptReportEmitterWrapperPassPass(Registry);
  initializeHIRParDirInsertPass(Registry);
  initializeHIRVecDirInsertPass(Registry);
  initializeHIROptVarPredicateLegacyPassPass(Registry);
  initializeHIRMVForConstUBLegacyPassPass(Registry);
  initializeHIRMVForVariableStrideLegacyPassPass(Registry);
  initializeHIRLoopConcatenationLegacyPassPass(Registry);
  initializeHIRLoopFusionLegacyPassPass(Registry);
  initializeHIRLastValueComputationLegacyPassPass(Registry);
  initializeHIRPropagateCastedIVLegacyPassPass(Registry);
  initializeHIRMultiExitLoopRerollLegacyPassPass(Registry);
  initializeHIRRecognizeParLoopPass(Registry);
  initializeHIRMinMaxRecognitionLegacyPassPass(Registry);
  initializeHIRPrefetchingLegacyPassPass(Registry);
  initializeHIRSinkingForPerfectLoopnestLegacyPassPass(Registry);
  initializeHIRUndoSinkingForPerfectLoopnestLegacyPassPass(Registry);
  initializeHIRMemoryReductionSinkingLegacyPassPass(Registry);
  initializeHIRNontemporalMarkingLegacyPassPass(Registry);
  initializeHIRStoreResultIntoTempArrayLegacyPassPass(Registry);
  initializeHIRSumWindowReuseLegacyPassPass(Registry);
  initializeHIRNonZeroSinkingForPerfectLoopnestLegacyPassPass(Registry);
}
