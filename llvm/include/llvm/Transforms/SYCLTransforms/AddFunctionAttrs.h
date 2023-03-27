//===- AddFunctionAttrs.h - Add function attributes -------------*- C++ -*-===//
//
// Copyright (C) 2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_SYCLTRANSFORMS_ADD_FUNCTION_ATTRS_H
#define LLVM_TRANSFORMS_SYCLTRANSFORMS_ADD_FUNCTION_ATTRS_H

#include "llvm/IR/PassManager.h"

namespace llvm {

/// AddFunctionAttrs pass marks synchronize built-ins of DPC++/OpenCL language
/// and all functions that calls them direct or indirect, in order to prevent
/// LLVM Standard passes or vectorizer from breaking their semantic.
///
/// For instance, it propogates convergent, kernel-convergent-call,
/// kernel-call-once and noduplicate attributes to all synchronize built-ins.
class AddFunctionAttrsPass : public PassInfoMixin<AddFunctionAttrsPass> {
public:
  PreservedAnalyses run(Module &M, ModuleAnalysisManager &MAM);

  bool runImpl(Module &M);
};

} // namespace llvm
#endif // LLVM_TRANSFORMS_SYCLTRANSFORMS_ADD_FUNCTION_ATTRS_H
