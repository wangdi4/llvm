//===------------------ CSAIRPasses.h - CSA IR Passes -----------*- C++ -*-===//
//
// Copyright (C) 2018 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
//
// This file declares IR Passes used for CSA target
//
//===----------------------------------------------------------------------===//

#if !INTEL_FEATURE_CSA
#error CSAIRPasses.h include in an non-INTEL_FEATURE_CSA build.
#endif  // INTEL_FEATURE_CSA

#ifndef INTEL_CSA_CSAIRPASSES_H
#define INTEL_CSA_CSAIRPASSES_H

#include "llvm/PassRegistry.h"
#include "llvm/Pass.h"

namespace llvm {

void initializeCSAScalarPasses(PassRegistry&);
void initializeLoopSPMDizationPass(PassRegistry&);
void initializeCSALowerParallelIntrinsicsWrapperPass(PassRegistry&);

//
// LoopSPMDization - This pass is a simple loop SPMDization pass.
//
Pass *createLoopSPMDizationPass();

//
// CSALowerParallelIntrinsics - This pass transforms CSA parallel
// intrinsics calls into LLVM metadata.
//
Pass *createCSALowerParallelIntrinsicsWrapperPass();
} // namespace llvm

#endif  // INTEL_CSA_CSAIRPASSES_H
