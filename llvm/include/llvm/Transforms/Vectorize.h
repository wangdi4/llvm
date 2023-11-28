//===-- Vectorize.h - Vectorization Transformations -------------*- C++ -*-===//
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
// This header file defines prototypes for accessor functions that expose passes
// in the Vectorize transformations library.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_VECTORIZE_H
#define LLVM_TRANSFORMS_VECTORIZE_H

#include <functional> // INTEL

namespace llvm {
class Pass;
#if INTEL_CUSTOMIZATION
class Function;

namespace vpo {
// Enum indicating vectorization error kind.
enum class VecErrorKind {
  // Means that a loop in the function was not vectorized by any reason, the
  // code was not modified by vectorizer. May be useful in some cases in OCL
  // pipeline.
  Bailout,
  // Means that by some reason vectorizer generated a wrong code and compilation
  // should be aborted. That can happen due to different reasons. E.g. an
  // indirect call in SYCL that is made using user-defined vector function
  // pointer does not have the needed vector variant.
  Fatal,
};

using VecErrorHandlerTy =
    std::function<void(llvm::Function *F, VecErrorKind K)>;
} // namespace vpo
#endif // INTEL_CUSTOMIZATION
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
