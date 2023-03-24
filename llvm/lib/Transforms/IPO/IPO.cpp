//===-- IPO.cpp -----------------------------------------------------------===//
// INTEL_CUSTOMIZATION
//
// INTEL CONFIDENTIAL
//
// Modifications, Copyright (C) 2021-2023 Intel Corporation
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
//
// This file implements the common infrastructure (including C bindings) for
// libLLVMIPO.a, which implements several transformations over the LLVM
// intermediate representation.
//
//===----------------------------------------------------------------------===//

#include "llvm-c/Initialization.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/InitializePasses.h"
#include "llvm/Transforms/IPO.h"
#include "llvm/Transforms/IPO/AlwaysInliner.h"
#include "llvm/Transforms/IPO/FunctionAttrs.h"

#if INTEL_CUSTOMIZATION
#include "llvm/Transforms/IPO/Intel_InlineReportEmitter.h"
#include "llvm/Transforms/IPO/Intel_InlineReportSetup.h"
#endif // INTEL_CUSTOMIZATION

using namespace llvm;

void llvm::initializeIPO(PassRegistry &Registry) {
  initializeDAEPass(Registry);
  initializeDAHPass(Registry);
  initializeDAESYCLPass(Registry);
  initializeAlwaysInlinerLegacyPassPass(Registry);
  initializeInlineReportEmitterPass(Registry); // INTEL
  initializeInlineReportSetupPass(Registry); // INTEL
  initializeLoopExtractorLegacyPassPass(Registry);
  initializeSingleLoopExtractorPass(Registry);
  initializeBarrierNoopPass(Registry);
}

void LLVMInitializeIPO(LLVMPassRegistryRef R) {
  initializeIPO(*unwrap(R));
}

#if INTEL_CUSTOMIZATION
void LLVMAddInlineReportSetupPass(LLVMPassManagerRef PM) {
  unwrap(PM)->add(createInlineReportSetupPass());
}
void LLVMAddInlineReportEmitterPass(LLVMPassManagerRef PM) {
  unwrap(PM)->add(createInlineReportEmitterPass());
}
#endif  // INTEL_CUSTOMIZATION
