//===-------------- ResolveTypes.h - DTransResolveTypesPass  --------------===//
//
// Copyright (C) 2018-2019 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
//
// This file declares the DTrans ResolveTypes pass.
//
//===----------------------------------------------------------------------===//

#if !INTEL_INCLUDE_DTRANS
#error ResolveTypes.h include in an non-INTEL_INCLUDE_DTRANS build.
#endif

#ifndef INTEL_DTRANS_TRANSFORMS_RESOLVETYPES_H
#define INTEL_DTRANS_TRANSFORMS_RESOLVETYPES_H

#include "llvm/IR/Module.h"
#include "llvm/IR/PassManager.h"

namespace llvm {

class TargetLibraryInfo;
class WholeProgramInfo;

namespace dtrans {

/// Pass to resolve name collisions between identical types.
class ResolveTypesPass : public PassInfoMixin<dtrans::ResolveTypesPass> {
public:
  PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM);

  // This is used to share the core implementation with the legacy pass.
  bool
  runImpl(Module &M,
          std::function<const TargetLibraryInfo &(const Function &)> GetTLI,
          WholeProgramInfo &WPInfo);
};

} // namespace dtrans

ModulePass *createDTransResolveTypesWrapperPass();

} // namespace llvm

#endif // INTEL_DTRANS_TRANSFORMS_RESOLVETYPES_H
