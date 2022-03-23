//==--- InstToFuncCallPass.h - Replaces inst with func call - C++ -*--------==//
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
//===----------------------------------------------------------------------===//
//
// This pass replaces LLVM IR instruction with function call, e.g.
//   * For performance: replace vector convert function with builtin that calls
//     svml functions.
//   * For functionality: replace 'fdiv fast' with svml function on AVX/SSE42 to
//     meet accuracy requirement of OpenCL/SYCL CTS.
//
// ===--------------------------------------------------------------------=== //

#ifndef LLVM_TRANSFORMS_INTEL_DPCPPKERNELTRANSFORMS_INSTTOFUNCCALL_H
#define LLVM_TRANSFORMS_INTEL_DPCPPKERNELTRANSFORMS_INSTTOFUNCCALL_H

#include "llvm/IR/Intel_VectorVariant.h"
#include "llvm/IR/PassManager.h"

namespace llvm {

class InstToFuncCallPass : public PassInfoMixin<InstToFuncCallPass> {
public:
  InstToFuncCallPass(VectorVariant::ISAClass ISA = VectorVariant::XMM)
      : ISA(ISA) {}

  PreservedAnalyses run(Module &M, ModuleAnalysisManager &);

private:
  VectorVariant::ISAClass ISA;
};

} // namespace llvm

#endif // LLVM_TRANSFORMS_INTEL_DPCPPKERNELTRANSFORMS_INSTTOFUNCCALL_H
