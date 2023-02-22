//===-- Vectorize.cpp -----------------------------------------------------===//
// INTEL_CUSTOMIZATION
//
// INTEL CONFIDENTIAL
//
// Modifications, Copyright (C) 2021 Intel Corporation
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you ("License"). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.
//
// end INTEL_CUSTOMIZATION
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements common infrastructure for libLLVMVectorizeOpts.a, which
// implements several vectorization transformations over the LLVM intermediate
// representation, including the C bindings for that library.
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/Vectorize.h"
#include "llvm-c/Initialization.h"
#include "llvm-c/Transforms/Vectorize.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/InitializePasses.h"
#include "llvm/PassRegistry.h"

using namespace llvm;

/// Initialize all passes linked into the Vectorization library.
void llvm::initializeVectorization(PassRegistry &Registry) {
  initializeLoopVectorizePass(Registry);
  initializeSLPVectorizerPass(Registry);
  initializeLoadStoreVectorizerLegacyPassPass(Registry);
#if INTEL_CUSTOMIZATION
  initializeLoadCoalescingLegacyPassPass(Registry);
  initializeMathLibraryFunctionsReplacementLegacyPassPass(Registry);
  initializeVPlanPragmaOmpOrderedSimdExtractPass(Registry);
  initializeVPlanPragmaOmpSimdIfPass(Registry);
  initializeVPlanDriverPass(Registry);
  initializeVPlanDriverHIRPass(Registry);
  initializeVPlanFunctionVectorizerLegacyPassPass(Registry);
#endif
  initializeVectorCombineLegacyPassPass(Registry);
}

void LLVMInitializeVectorization(LLVMPassRegistryRef R) {
  initializeVectorization(*unwrap(R));
}

void LLVMAddLoopVectorizePass(LLVMPassManagerRef PM) {
  unwrap(PM)->add(createLoopVectorizePass());
}

void LLVMAddSLPVectorizePass(LLVMPassManagerRef PM) {
  unwrap(PM)->add(createSLPVectorizerPass());
}

#if INTEL_CUSTOMIZATION
void LLVMAddLoadCoalescingPass(LLVMPassManagerRef PM) {
  unwrap(PM)->add(createLoadCoalescingPass());
}

void LLVMAddMathLibraryFunctionReplacementPass(LLVMPassManagerRef PM) {
  unwrap(PM)->add(createMathLibraryFunctionsReplacementPass());
}

void LLVMAddVPlanPragmaOmpOrderedSimdExtractPass(LLVMPassManagerRef PM) {
  unwrap(PM)->add(createVPlanPragmaOmpOrderedSimdExtractPass());
}

void LLVMAddVPlanPragmaOmpSimdIfPass(LLVMPassManagerRef PM) {
  unwrap(PM)->add(createVPlanPragmaOmpSimdIfPass());
}

void LLVMAddVPlanDriverPass(LLVMPassManagerRef PM) {
  unwrap(PM)->add(createVPlanDriverPass());
}

void LLVMAddVPlanDriverHIRPass(LLVMPassManagerRef PM) {
  unwrap(PM)->add(createVPlanDriverHIRPass(false));
}
#endif
