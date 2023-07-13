//===-- Vectorize.h - Vectorization Transformations -------------*- C++ -*-===//
// INTEL_CUSTOMIZATION
//
// INTEL CONFIDENTIAL
//
// Modifications, Copyright (C) 2021-2023 Intel Corporation
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
// This header file defines prototypes for accessor functions that expose passes
// in the Vectorize transformations library.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_VECTORIZE_H
#define LLVM_TRANSFORMS_VECTORIZE_H

#include <functional> // INTEL

namespace llvm {
class Pass;
class Function; // INTEL

using FatalErrorHandlerTy = std::function<void (llvm::Function *F)>;  // INTEL

//===----------------------------------------------------------------------===//
//
// LoadStoreVectorizer - Create vector loads and stores, but leave scalar
// operations.
//
Pass *createLoadStoreVectorizerPass();

#if INTEL_CUSTOMIZATION

//===----------------------------------------------------------------------===//
//
// LoadCoalescing - Combine consecutive loads into wider loads and emit shuffles
// operations.
//
Pass *createLoadCoalescingPass();

//===----------------------------------------------------------------------===//
//
// MathFunctionReplacement - Replace known math operations with optimized
// library function calls.
//
Pass *createMathLibraryFunctionsReplacementPass(); // remove in OCL commit
Pass *createMathLibraryFunctionsReplacementPass(bool isOCL);

Pass *createVPlanFunctionVectorizerPass();

//===----------------------------------------------------------------------===//
//
// Support for pragma omp ordered simd.
//
Pass *createVPlanPragmaOmpOrderedSimdExtractPass();

//===----------------------------------------------------------------------===//
//
// Support for pragma omp simd if.
//
Pass *createVPlanPragmaOmpSimdIfPass();

#endif // INTEL_CUSTOMIZATION
} // End llvm namespace

#endif
