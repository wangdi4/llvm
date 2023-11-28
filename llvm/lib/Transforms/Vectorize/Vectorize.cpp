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

#include "llvm/IR/LegacyPassManager.h" // INTEL
#include "llvm/InitializePasses.h"
#include "llvm/PassRegistry.h"

using namespace llvm;

/// Initialize all passes linked into the Vectorization library.
void llvm::initializeVectorization(PassRegistry &Registry) {
  initializeLoadStoreVectorizerLegacyPassPass(Registry);
#if INTEL_CUSTOMIZATION
  initializeLoadCoalescingLegacyPassPass(Registry);
  initializeMathLibraryFunctionsReplacementLegacyPassPass(Registry);
  initializeVPlanPragmaOmpOrderedSimdExtractPass(Registry);
  initializeVPlanPragmaOmpSimdIfPass(Registry);
  initializeVPlanFunctionVectorizerLegacyPassPass(Registry);
#endif // INTEL_CUSTOMIZATION
}
