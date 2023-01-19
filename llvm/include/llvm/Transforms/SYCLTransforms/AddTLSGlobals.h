//===- AddTLSGlobals.h - AddTLSGlobals pass C++ -*-------------------------===//
//
// Copyright (C) 2022 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_SYCLTRANSFORMS_ADD_TLS_GLOBALS_H
#define LLVM_TRANSFORMS_SYCLTRANSFORMS_ADD_TLS_GLOBALS_H

#include "llvm/IR/PassManager.h"

namespace llvm {
class AddTLSGlobalsPass : public PassInfoMixin<AddTLSGlobalsPass> {
public:
  PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM);

  static bool isRequired() { return true; }
};

} // namespace llvm

#endif // LLVM_TRANSFORMS_SYCLTRANSFORMS_ADD_TLS_GLOBALS_H
