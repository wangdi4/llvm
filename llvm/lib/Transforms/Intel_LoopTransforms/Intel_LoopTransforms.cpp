//===-- Intel_LoopTransforms.cpp-------------------------------------------===//
//
// Copyright (C) 2015-2018 Intel Corporation. All rights reserved.
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
  initializeHIRTempCleanupPass(Registry);
  initializeHIRSymbolicTripCountCompleteUnrollLegacyPassPass(Registry);
  initializeHIRLoopInterchangeLegacyPassPass(Registry);
  initializeHIROptPredicatePass(Registry);
  initializeHIRPreVecCompleteUnrollPass(Registry);
  initializeHIRPostVecCompleteUnrollPass(Registry);
  initializeHIRGeneralUnrollPass(Registry);
  initializeHIRUnrollAndJamLegacyPassPass(Registry);
  initializeHIRDummyTransformationPass(Registry);
  initializeHIRLoopReversalPass(Registry);
  initializeHIRLMMPass(Registry);
  initializeHIRLoopCollapsePass(Registry);
  initializeHIRRuntimeDDPass(Registry);
  initializeHIRScalarReplArrayPass(Registry);
  initializeHIRLoopDistributionForMemRecPass(Registry);
  initializeHIRLoopDistributionForLoopNestPass(Registry);
  initializeHIROptReportEmitterWrapperPassPass(Registry);
  initializeHIRCodeGenWrapperPassPass(Registry);
  initializeHIRParDirInsertPass(Registry);
  initializeHIRVecDirInsertPass(Registry);
  initializeHIROptVarPredicatePass(Registry);
  initializeHIRIdiomRecognitionPass(Registry);
  initializeHIRMVForConstUBPass(Registry);
  initializeHIRLoopConcatenationLegacyPassPass(Registry);
  initializeHIRArrayTransposePass(Registry);
  initializeHIRLoopFusionPass(Registry);
  initializeHIRDeadStoreEliminationPass(Registry);
}
