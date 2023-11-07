//===-- Analysis.cpp ------------------------------------------------------===//
// INTEL_CUSTOMIZATION
//
// INTEL CONFIDENTIAL
//
// Modifications, Copyright (C) 2021 Intel Corporation
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you ("License"). Unless the License provides otherwise, you may
// not use, modify, copy, publish, distribute, disclose or transmit this
// software or the related documents without Intel's prior written permission.
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

#include "llvm-c/Analysis.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Verifier.h"
#include "llvm/InitializePasses.h"
#include "llvm/PassRegistry.h"
#include "llvm/Support/raw_ostream.h"
#include <cstring>

using namespace llvm;

/// initializeAnalysis - Initialize all passes linked into the Analysis library.
void llvm::initializeAnalysis(PassRegistry &Registry) {
<<<<<<< HEAD
  initializeAAEvalLegacyPassPass(Registry);
  initializeArrayUseWrapperPassPass(Registry);  // INTEL
=======
>>>>>>> 02230e12d77a19e4e21ad55ce9738faf84a95903
  initializeBasicAAWrapperPassPass(Registry);
  initializeBlockFrequencyInfoWrapperPassPass(Registry);
  initializeBranchProbabilityInfoWrapperPassPass(Registry);
  initializeCallGraphWrapperPassPass(Registry);
  initializeCallGraphDOTPrinterPass(Registry);
  initializeCallGraphPrinterLegacyPassPass(Registry);
  initializeCallGraphViewerPass(Registry);
  initializeCostModelAnalysisPass(Registry);
  initializeCFGViewerLegacyPassPass(Registry);
  initializeCFGPrinterLegacyPassPass(Registry);
  initializeCFGOnlyViewerLegacyPassPass(Registry);
  initializeCFGOnlyPrinterLegacyPassPass(Registry);
  initializeCycleInfoWrapperPassPass(Registry);
  initializeDependenceAnalysisWrapperPassPass(Registry);
  initializeDelinearizationPass(Registry);
#if INTEL_CUSTOMIZATION
  initializeDemandedBitsWrapperPassPass(Registry);
#endif // INTEL_CUSTOMIZATION
  initializeDominanceFrontierWrapperPassPass(Registry);
  initializeDomViewerWrapperPassPass(Registry);
  initializeDomPrinterWrapperPassPass(Registry);
  initializeDomOnlyViewerWrapperPassPass(Registry);
  initializePostDomViewerWrapperPassPass(Registry);
  initializeDomOnlyPrinterWrapperPassPass(Registry);
  initializePostDomPrinterWrapperPassPass(Registry);
  initializePostDomOnlyViewerWrapperPassPass(Registry);
  initializePostDomOnlyPrinterWrapperPassPass(Registry);
  initializeAAResultsWrapperPassPass(Registry);
  initializeGlobalsAAWrapperPassPass(Registry);
  initializeIVUsersWrapperPassPass(Registry);
  initializeInstCountLegacyPassPass(Registry);
  initializeIntervalPartitionPass(Registry);
  initializeIRSimilarityIdentifierWrapperPassPass(Registry);
  initializeLazyBranchProbabilityInfoPassPass(Registry);
  initializeLazyBlockFrequencyInfoPassPass(Registry);
  initializeLazyValueInfoWrapperPassPass(Registry);
  initializeLazyValueInfoPrinterPass(Registry);
  initializeLoopInfoWrapperPassPass(Registry);
  initializeMemoryDependenceWrapperPassPass(Registry);
#if INTEL_CUSTOMIZATION
  initializeSNodeAnalysisPass(Registry);
  initializeStdContainerAAWrapperPassPass(Registry);
#endif // INTEL_CUSTOMIZATION
  initializeModuleSummaryIndexWrapperPassPass(Registry);
  initializeOptimizationRemarkEmitterWrapperPassPass(Registry);
  initializePhiValuesWrapperPassPass(Registry);
  initializePostDominatorTreeWrapperPassPass(Registry);
  initializeRegionInfoPassPass(Registry);
  initializeRegionViewerPass(Registry);
  initializeRegionPrinterPass(Registry);
  initializeRegionOnlyViewerPass(Registry);
  initializeRegionOnlyPrinterPass(Registry);
  initializeSCEVAAWrapperPassPass(Registry);
  initializeScalarEvolutionWrapperPassPass(Registry);
  initializeStackSafetyGlobalInfoWrapperPassPass(Registry);
  initializeStackSafetyInfoWrapperPassPass(Registry);
  initializeTargetTransformInfoWrapperPassPass(Registry);
  initializeTypeBasedAAWrapperPassPass(Registry);
  initializeScopedNoAliasAAWrapperPassPass(Registry);
  initializeLCSSAVerificationPassPass(Registry);
  initializeMemorySSAWrapperPassPass(Registry);
}

LLVMBool LLVMVerifyModule(LLVMModuleRef M, LLVMVerifierFailureAction Action,
                          char **OutMessages) {
  raw_ostream *DebugOS = Action != LLVMReturnStatusAction ? &errs() : nullptr;
  std::string Messages;
  raw_string_ostream MsgsOS(Messages);

  LLVMBool Result = verifyModule(*unwrap(M), OutMessages ? &MsgsOS : DebugOS);

  // Duplicate the output to stderr.
  if (DebugOS && OutMessages)
    *DebugOS << MsgsOS.str();

  if (Action == LLVMAbortProcessAction && Result)
    report_fatal_error("Broken module found, compilation aborted!");

  if (OutMessages)
    *OutMessages = strdup(MsgsOS.str().c_str());

  return Result;
}

LLVMBool LLVMVerifyFunction(LLVMValueRef Fn, LLVMVerifierFailureAction Action) {
  LLVMBool Result = verifyFunction(
      *unwrap<Function>(Fn), Action != LLVMReturnStatusAction ? &errs()
                                                              : nullptr);

  if (Action == LLVMAbortProcessAction && Result)
    report_fatal_error("Broken function found, compilation aborted!");

  return Result;
}

void LLVMViewFunctionCFG(LLVMValueRef Fn) {
  Function *F = unwrap<Function>(Fn);
  F->viewCFG();
}

void LLVMViewFunctionCFGOnly(LLVMValueRef Fn) {
  Function *F = unwrap<Function>(Fn);
  F->viewCFGOnly();
}
