//===-- IPO.cpp -----------------------------------------------------------===//
// INTEL_CUSTOMIZATION
//
// INTEL CONFIDENTIAL
//
// Modifications, Copyright (C) 2021-2022 Intel Corporation
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
// This file implements the common infrastructure (including C bindings) for
// libLLVMIPO.a, which implements several transformations over the LLVM
// intermediate representation.
//
//===----------------------------------------------------------------------===//

#include "llvm-c/Transforms/IPO.h"
#include "llvm-c/Initialization.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/InitializePasses.h"
#include "llvm/Transforms/IPO.h"
#include "llvm/Transforms/IPO/AlwaysInliner.h"
#include "llvm/Transforms/IPO/FunctionAttrs.h"

#if INTEL_CUSTOMIZATION
#include "llvm/Transforms/IPO/Intel_InlineLists.h"
#include "llvm/Transforms/IPO/Intel_InlineReportEmitter.h"
#include "llvm/Transforms/IPO/Intel_InlineReportSetup.h"
#endif // INTEL_CUSTOMIZATION

using namespace llvm;

void llvm::initializeIPO(PassRegistry &Registry) {
  initializeAnnotation2MetadataLegacyPass(Registry);
  initializeConstantMergeLegacyPassPass(Registry);
  initializeDAEPass(Registry);
  initializeDAHPass(Registry);
  initializeDAESYCLPass(Registry);
  initializeForceFunctionAttrsLegacyPassPass(Registry);
  initializeGlobalDCELegacyPassPass(Registry);
  initializeAlwaysInlinerLegacyPassPass(Registry);
  initializeInlineListsPass(Registry); // INTEL
  initializeInlineReportEmitterPass(Registry); // INTEL
  initializeInlineReportSetupPass(Registry); // INTEL
  initializeInferFunctionAttrsLegacyPassPass(Registry);
  initializeLoopExtractorLegacyPassPass(Registry);
  initializeSingleLoopExtractorPass(Registry);
  initializeAttributorLegacyPassPass(Registry);
  initializeAttributorCGSCCLegacyPassPass(Registry);
  initializePostOrderFunctionAttrsLegacyPassPass(Registry);
  initializeBarrierNoopPass(Registry);
  initializeEliminateAvailableExternallyLegacyPassPass(Registry);
#if INTEL_CUSTOMIZATION
#if INTEL_FEATURE_SW_ADVANCED
  initializeIPCloningLegacyPassPass(Registry);
#endif // INTEL_FEATURE_SW_ADVANCED
  initializeCallTreeCloningLegacyPassPass(Registry);
  initializeIntelAdvancedFastCallWrapperPassPass(Registry);
  initializeDopeVectorConstPropLegacyPassPass(Registry);
  initializeIntelArgumentAlignmentLegacyPassPass(Registry);
#if INTEL_FEATURE_SW_ADVANCED
  initializeQsortRecognizerLegacyPassPass(Registry);
#endif // INTEL_FEATURE_SW_ADVANCED
  initializeAggInlinerLegacyPassPass(Registry);
#if INTEL_FEATURE_SW_DTRANS
  initializeIntelFoldWPIntrinsicLegacyPassPass(Registry);
#endif // INTEL_FEATURE_SW_DTRANS
  initializeIPArrayTransposeLegacyPassPass(Registry);
  initializeArgNoAliasPropPass(Registry);
  initializeIntelVTableFixupLegacyPassPass(Registry);
  initializeIntelMathLibrariesDeclarationWrapperPass(Registry);
  initializeIntelIPODeadArgEliminationWrapperPass(Registry);
#if INTEL_FEATURE_SW_ADVANCED
  initializeTileMVInlMarkerLegacyPassPass(Registry);
  initializeIntelPartialInlineLegacyPassPass(Registry);
  initializeIntelIPOPrefetchWrapperPassPass(Registry);
  initializeIPPredOptLegacyPassPass(Registry);
  initializeDeadArrayOpsEliminationLegacyPassPass(Registry);
#endif // INTEL_FEATURE_SW_ADVANCED
#endif // INTEL_CUSTOMIZATION
}

void LLVMInitializeIPO(LLVMPassRegistryRef R) {
  initializeIPO(*unwrap(R));
}

void LLVMAddConstantMergePass(LLVMPassManagerRef PM) {
  unwrap(PM)->add(createConstantMergePass());
}

void LLVMAddDeadArgEliminationPass(LLVMPassManagerRef PM) {
  unwrap(PM)->add(createDeadArgEliminationPass());
}

#if INTEL_CUSTOMIZATION
void LLVMAddInlineListsPass(LLVMPassManagerRef PM) {
  unwrap(PM)->add(createInlineListsPass());
}
void LLVMAddInlineReportSetupPass(LLVMPassManagerRef PM) {
  unwrap(PM)->add(createInlineReportSetupPass());
}
void LLVMAddInlineReportEmitterPass(LLVMPassManagerRef PM) {
  unwrap(PM)->add(createInlineReportEmitterPass());
}
#endif  // INTEL_CUSTOMIZATION

void LLVMAddFunctionAttrsPass(LLVMPassManagerRef PM) {
  unwrap(PM)->add(createPostOrderFunctionAttrsLegacyPass());
}

void LLVMAddAlwaysInlinerPass(LLVMPassManagerRef PM) {
  unwrap(PM)->add(llvm::createAlwaysInlinerLegacyPass());
}

void LLVMAddGlobalDCEPass(LLVMPassManagerRef PM) {
  unwrap(PM)->add(createGlobalDCEPass());
}
