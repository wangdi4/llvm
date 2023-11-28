//===== DeleteFieldOP.h - Delete field with support for opaque pointers =====//
//
// Copyright (C) 2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===---------------------------------------------------------------------===//
// This file declares the DTrans delete field optimization pass for with
// support for IR using either opaque or non-opaque pointers.
//===---------------------------------------------------------------------===//

#if !INTEL_FEATURE_SW_DTRANS
#error DeleteFieldOP.h include in an non-INTEL_FEATURE_SW_DTRANS build.
#endif

#ifndef INTEL_DTRANS_TRANSFORMS_DELETEFIELDOP_H
#define INTEL_DTRANS_TRANSFORMS_DELETEFIELDOP_H

#include "llvm/IR/PassManager.h"

namespace llvm {
class Function;
class Module;
class TargetLibraryInfo;
class WholeProgramInfo;

namespace dtransOP {
class DTransSafetyInfo;

class DeleteFieldOPPass : public PassInfoMixin<DeleteFieldOPPass> {
public:
  using GetTLIFn = std::function<const TargetLibraryInfo &(const Function &)>;

  PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM);

  bool runImpl(Module &M, DTransSafetyInfo *DTInfo, WholeProgramInfo &WPInfo,
               GetTLIFn GetTLI);
};

} // namespace dtransOP

} // namespace llvm

#endif // INTEL_DTRANS_TRANSFORMS_DELETEFIELDOP_H
