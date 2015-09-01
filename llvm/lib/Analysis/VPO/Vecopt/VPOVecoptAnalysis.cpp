//===----------------------------------------------------------------------===//
//
//   Copyright (C) 2015 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation. and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//   Source file:
//   ------------
//   VPOVecoptAnalysis.cpp -- Vecopt Analysis Passes initializers.
//
//===----------------------------------------------------------------------===//

#include "llvm/InitializePasses.h"
#include "llvm/PassRegistry.h"

using namespace llvm;

void llvm::initializeVPOVecoptAnalysis(PassRegistry &Registry) {
  initializeIdentifyVectorCandidatesPass(Registry);
  initializeAVRGeneratePass(Registry);
}
