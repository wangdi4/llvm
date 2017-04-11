//===-- Intel_LoopTransforms.cpp-------------------------------------------===//
//
// Copyright (C) 2015-2016 Intel Corporation. All rights reserved.
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
  initializeHIRSSADeconstructionPass(Registry);
  initializeHIRTempCleanupPass(Registry);
  initializeHIRLoopInterchangePass(Registry);
  initializeHIROptPredicatePass(Registry);
  initializeHIRPreVecCompleteUnrollPass(Registry);
  initializeHIRPostVecCompleteUnrollPass(Registry);
  initializeHIRGeneralUnrollPass(Registry);
  initializeHIRUnrollAndJamPass(Registry);
  initializeHIRDummyTransformationPass(Registry);
  initializeHIRLoopReversalPass(Registry);
  initializeHIRLMMPass(Registry);
  initializeHIRRuntimeDDPass(Registry);
  initializeHIRScalarReplArrayPass(Registry);
  initializeHIRLoopDistributionForMemRecPass(Registry);
  initializeHIRLoopDistributionForLoopNestPass(Registry);
  initializeHIRCodeGenPass(Registry);
  initializeVPODriverHIRPass(Registry);
  initializeHIRParDirInsertPass(Registry);
  initializeHIRVecDirInsertPass(Registry);
  initializeHIROptVarPredicatePass(Registry);
  initializeHIRIdiomRecognitionPass(Registry);
  initializeHIRMVForConstUBPass(Registry);
}
