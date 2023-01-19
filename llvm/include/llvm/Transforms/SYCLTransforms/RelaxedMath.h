//===- RelaxedMath.h - Relaxed math builtin substitution --------*- C++ -*-===//
//
// Copyright (C) 2022 Intel Corporation
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
//===---------------------------------------------------------------------===//
//
// This pass replace math builtin with its relaxed version if OpenCL
// -cl-fast-relaxed-math option is present.
// E.g.
//   call <16 x float> @_Z3sinDv16_f(<16 x float> %0)
// is replaced with
//   call <16 x float> @_Z6sin_rmDv16_f(<16 x float> %0)
//
//===---------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_SYCLTRANSFORMS_RELAXEDMATH_H
#define LLVM_TRANSFORMS_SYCLTRANSFORMS_RELAXEDMATH_H

#include "llvm/IR/PassManager.h"

namespace llvm {

class RelaxedMathPass : public PassInfoMixin<RelaxedMathPass> {
public:
  PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM);
};

} // namespace llvm

#endif // LLVM_TRANSFORMS_SYCLTRANSFORMS_RELAXEDMATH_H
