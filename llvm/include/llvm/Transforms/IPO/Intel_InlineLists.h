//===---------------- Intel_InlineLists.h - [No]Inline Lists  -------------===//
//
// Copyright (C) 2017 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
//
// This pass assignes attributes to the call sites that appear in inline and
// noinline lists.
//
//===----------------------------------------------------------------------===//
//

#ifndef LLVM_TRANSFORMS_IPO_INTEL_INLINELISTS_H
#define LLVM_TRANSFORMS_IPO_INTEL_INLINELISTS_H

#include "llvm/IR/Module.h"
#include "llvm/IR/PassManager.h"

namespace llvm {

class InlineListsPass : public PassInfoMixin<InlineListsPass> {
public:
  PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM);
  static bool isRequired() { return true; }
};
} // namespace llvm

#endif // LLVM_TRANSFORMS_IPO_INTEL_INLINELISTS_H
