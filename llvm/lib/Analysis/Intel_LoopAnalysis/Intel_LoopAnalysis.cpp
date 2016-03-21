//===-- Intel_LoopAnalysis.cpp --------------------------------------------===//
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

void llvm::initializeIntel_LoopAnalysis(PassRegistry &Registry) {
  initializeRegionIdentificationPass(Registry);
  initializeSCCFormationPass(Registry);
  initializeScalarSymbaseAssignmentPass(Registry);
  initializeHIRCreationPass(Registry);
  initializeHIRCleanupPass(Registry);
  initializeLoopFormationPass(Registry);
  initializeHIRParserPass(Registry);
  initializeSymbaseAssignmentPass(Registry);
  initializeHIRFrameworkPass(Registry);
  initializeDDAnalysisPass(Registry);
  initializeHIRLocalityAnalysisPass(Registry);
  initializeHIRVectVLSAnalysisPass(Registry);
}
