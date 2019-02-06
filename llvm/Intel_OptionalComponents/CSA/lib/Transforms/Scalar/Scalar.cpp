//===- Scalar.cpp - CSA scalar passes ---------------------------*- C++ -*-===//
//
// Copyright (C) 2018 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
///===---------------------------------------------------------------------===//
/// \file
///
/// This file implements common infrastructure for libLLVMIntel_CSAScalarOpt
/// library.
///
///===---------------------------------------------------------------------===//

#include "Intel_CSA/CSAIRPasses.h"

using namespace llvm;

void llvm::initializeCSAScalarPasses(PassRegistry &Registry) {
  initializeLoopSPMDizationPass(Registry);
  initializeCSALowerParallelIntrinsicsWrapperPass(Registry);
  initializeCSAGraphSplitterPass(Registry);
}
