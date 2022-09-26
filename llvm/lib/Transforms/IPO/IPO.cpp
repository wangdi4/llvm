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
#include "llvm/Transforms/IPO/Intel_InlineLists.h" // INTEL
#include "llvm/Transforms/IPO/Intel_InlineReportEmitter.h" // INTEL
#include "llvm/Transforms/IPO/Intel_InlineReportSetup.h" // INTEL

using namespace llvm;

void llvm::initializeIPO(PassRegistry &Registry) {
  initializeOpenMPOptCGSCCLegacyPassPass(Registry);
  initializeAnnotation2MetadataLegacyPass(Registry);
  initializeCalledValuePropagationLegacyPassPass(Registry);
  initializeConstantMergeLegacyPassPass(Registry);
  initializeCrossDSOCFIPass(Registry);
  initializeDAEPass(Registry);
  initializeDAHPass(Registry);
  initializeDAESYCLPass(Registry);
  initializeForceFunctionAttrsLegacyPassPass(Registry);
  initializeFunctionSpecializationLegacyPassPass(Registry);
  initializeGlobalDCELegacyPassPass(Registry);
  initializeGlobalOptLegacyPassPass(Registry);
  initializeGlobalSplitPass(Registry);
  initializeHotColdSplittingLegacyPassPass(Registry);
  initializeIROutlinerLegacyPassPass(Registry);
  initializeAlwaysInlinerLegacyPassPass(Registry);
  initializeInlineListsPass(Registry); // INTEL
  initializeInlineReportEmitterPass(Registry); // INTEL
  initializeInlineReportSetupPass(Registry); // INTEL
  initializeSimpleInlinerPass(Registry);
  initializeInferFunctionAttrsLegacyPassPass(Registry);
  initializeInternalizeLegacyPassPass(Registry);
  initializeLoopExtractorLegacyPassPass(Registry);
  initializeBlockExtractorLegacyPassPass(Registry);
  initializeSingleLoopExtractorPass(Registry);
  initializeMergeFunctionsLegacyPassPass(Registry);
  initializePartialInlinerLegacyPassPass(Registry);
  initializeAttributorLegacyPassPass(Registry);
  initializeAttributorCGSCCLegacyPassPass(Registry);
  initializePostOrderFunctionAttrsLegacyPassPass(Registry);
  initializeReversePostOrderFunctionAttrsLegacyPassPass(Registry);
  initializePruneEHPass(Registry);
  initializeIPSCCPLegacyPassPass(Registry);
  initializeStripDeadPrototypesLegacyPassPass(Registry);
  initializeStripSymbolsPass(Registry);
  initializeStripDebugDeclarePass(Registry);
  initializeStripDeadDebugInfoPass(Registry);
  initializeStripNonDebugSymbolsPass(Registry);
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

void LLVMAddCalledValuePropagationPass(LLVMPassManagerRef PM) {
  unwrap(PM)->add(createCalledValuePropagationPass());
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

void LLVMAddFunctionInliningPass(LLVMPassManagerRef PM) {
  unwrap(PM)->add(createFunctionInliningPass());
}

void LLVMAddAlwaysInlinerPass(LLVMPassManagerRef PM) {
  unwrap(PM)->add(llvm::createAlwaysInlinerLegacyPass());
}

void LLVMAddGlobalDCEPass(LLVMPassManagerRef PM) {
  unwrap(PM)->add(createGlobalDCEPass());
}

void LLVMAddGlobalOptimizerPass(LLVMPassManagerRef PM) {
  unwrap(PM)->add(createGlobalOptimizerPass());
}

void LLVMAddPruneEHPass(LLVMPassManagerRef PM) {
  unwrap(PM)->add(createPruneEHPass());
}

void LLVMAddIPSCCPPass(LLVMPassManagerRef PM) {
  unwrap(PM)->add(createIPSCCPPass());
}

void LLVMAddMergeFunctionsPass(LLVMPassManagerRef PM) {
  unwrap(PM)->add(createMergeFunctionsPass());
}

void LLVMAddInternalizePass(LLVMPassManagerRef PM, unsigned AllButMain) {
  auto PreserveMain = [=](const GlobalValue &GV) {
#if INTEL_CUSTOMIZATION
    if (isa<Function>(GV) && cast<Function>(GV).hasMetadata("llvm.acd.clone"))
      return AllButMain && GV.getName().startswith("main.");
#endif // INTEL_CUSTOMIZATION
    return AllButMain && GV.getName() == "main";
  };
  unwrap(PM)->add(createInternalizePass(PreserveMain));
}

void LLVMAddInternalizePassWithMustPreservePredicate(
    LLVMPassManagerRef PM,
    void *Context,
    LLVMBool (*Pred)(LLVMValueRef, void *)) {
  unwrap(PM)->add(createInternalizePass([=](const GlobalValue &GV) {
    return Pred(wrap(&GV), Context) == 0 ? false : true;
  }));
}

void LLVMAddStripDeadPrototypesPass(LLVMPassManagerRef PM) {
  unwrap(PM)->add(createStripDeadPrototypesPass());
}

void LLVMAddStripSymbolsPass(LLVMPassManagerRef PM) {
  unwrap(PM)->add(createStripSymbolsPass());
}
