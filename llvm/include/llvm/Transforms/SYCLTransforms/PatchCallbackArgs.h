//===- PatchCallbackArgs.h - Resolve builtin call to callbacks -*- C++ -*--===//
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

#ifndef LLVM_TRANSFORMS_SYCLTRANSFORMS_PATCH_CALLBACK_ARGS_H
#define LLVM_TRANSFORMS_SYCLTRANSFORMS_PATCH_CALLBACK_ARGS_H

#include "llvm/IR/PassManager.h"
#include "llvm/Transforms/SYCLTransforms/ImplicitArgsAnalysis.h"

namespace llvm {

class PatchCallbackArgsPass : public PassInfoMixin<PatchCallbackArgsPass> {
  using ValuePair = std::pair<Value *, Value *>;
  using FuncToValuePair = DenseMap<Function *, ValuePair>;

public:
  PreservedAnalyses run(Module &M, ModuleAnalysisManager &MAM);

  bool runImpl(Module &M, ImplicitArgsInfo *IAInfo);

  static bool isRequired() { return true; }

private:
  FuncToValuePair FuncToImplicitArgs;
};

} // namespace

#endif
