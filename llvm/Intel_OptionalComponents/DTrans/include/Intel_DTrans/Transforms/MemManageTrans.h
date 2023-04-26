//===--------------- MemManageTrans.h - DTransMemManageTransPass ----------===//
//
// Copyright (C) 2021-2023 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
//
// This file declares the Initial Memory Management Transformation pass.
//
//===----------------------------------------------------------------------===//

#ifndef INTEL_DTRANS_TRANSFORMS_MEMMANAGETRANS_H
#define INTEL_DTRANS_TRANSFORMS_MEMMANAGETRANS_H

#include "Intel_DTrans/Analysis/DTransAnalysis.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/PassManager.h"

#include "Intel_DTrans/Transforms/MemManageInfoImpl.h"

namespace llvm {

namespace dtrans {

using MemTLITy = std::function<const TargetLibraryInfo &(const Function &)>;

/// Pass to perform Initial Memory Management Transformation.
class MemManageTransPass : public PassInfoMixin<MemManageTransPass> {
public:
  PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM);

  bool runImpl(Module &M, DTransAnalysisInfo &Info, WholeProgramInfo &WPInfo,
               MemTLITy GetTLI);
};

} // namespace dtrans

} // namespace llvm

#endif // INTEL_DTRANS_TRANSFORMS_MEMMANAGETRANS_H
