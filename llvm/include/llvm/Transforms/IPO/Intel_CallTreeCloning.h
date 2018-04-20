//===------------ Intel_CallTreeCloning.h - Call tree cloning -*-----------===//
//
// Copyright (C) 2018 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//

#ifndef LLVM_TRANSFORMS_IPO_INTEL_CALLTREECLONING_H
#define LLVM_TRANSFORMS_IPO_INTEL_CALLTREECLONING_H

#include "llvm/IR/Module.h"
#include "llvm/IR/PassManager.h"

namespace llvm {

/// \brief New Pass Manager interface to the call tree cloning implemented by
/// \c CallTreeCloningImpl.
class CallTreeCloningPass : public PassInfoMixin<CallTreeCloningPass> {
public:
  PreservedAnalyses run(Module &M, ModuleAnalysisManager &MAM);
};

} // namespace llvm
#endif // LLVM_TRANSFORMS_IPO_INTEL_CALLTREECLONING_H