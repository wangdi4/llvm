//==---- PipeOrdering.h - Insert barriers to pipe-containing loops -- C++ -==//
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
// ===--------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_SYCLTRANSFORMS_PIPE_ORDERING_H
#define LLVM_TRANSFORMS_SYCLTRANSFORMS_PIPE_ORDERING_H

#include "llvm/IR/PassManager.h"

namespace llvm {

class LoopInfo;

/// Add barriers for kernels with loop and pipe built-ins.
/// When channels exist in the body of a loop with multiple work-items, a
/// loop body has an implicit work-group barrier. This implies that loop
/// iteration 0 of each work-item in a work-group executes before iteration
/// 1 of each work-item in a work-group, and so on.
class PipeOrderingPass : public PassInfoMixin<PipeOrderingPass> {
public:
  PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM);

  bool runImpl(Module &M, function_ref<LoopInfo &(Function &)> GetLI);

  static bool isRequired() { return true; }
};

} // namespace llvm

#endif // LLVM_TRANSFORMS_SYCLTRANSFORMS_PIPE_ORDERING_H
