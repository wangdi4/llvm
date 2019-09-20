//===-- Intel_LoopAnalysis.cpp --------------------------------------------===//
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

void llvm::initializeIntel_LoopAnalysis(PassRegistry &Registry) {
  initializeHIRRegionIdentificationWrapperPassPass(Registry);
  initializeHIRSCCFormationWrapperPassPass(Registry);
  initializeHIRFrameworkWrapperPassPass(Registry);
  initializeHIRDDAnalysisWrapperPassPass(Registry);
  initializeHIRLoopLocalityWrapperPassPass(Registry);
  initializeHIRLoopResourceWrapperPassPass(Registry);
  initializeHIRLoopStatisticsWrapperPassPass(Registry);
  initializeHIRSafeReductionAnalysisWrapperPassPass(Registry);
  initializeHIRSparseArrayReductionAnalysisWrapperPassPass(Registry);
  initializeHIRParVecAnalysisWrapperPassPass(Registry);
}
