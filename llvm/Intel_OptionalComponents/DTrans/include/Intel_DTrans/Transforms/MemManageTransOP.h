//===----------- MemManageTransOP.h - DTransMemManageTransOPPass ----------===//
//
// Copyright (C) 2022 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
//
// This file declares the Initial Memory Management Transformation pass for
// opaque pointers.
//
//===----------------------------------------------------------------------===//

#ifndef INTEL_DTRANS_TRANSFORMS_MEMMANAGETRANSOP_H
#define INTEL_DTRANS_TRANSFORMS_MEMMANAGETRANSOP_H

#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/PassManager.h"

namespace llvm {

class WholeProgramInfo;

namespace dtransOP {

/// Pass to perform Initial Memory Management Transformation.
class MemManageTransOPPass : public PassInfoMixin<MemManageTransOPPass> {
public:
  using MemTLITy = std::function<const TargetLibraryInfo &(const Function &)>;

  PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM);

  bool runImpl(Module &M, WholeProgramInfo &WPInfo, MemTLITy GetTLI);
};
} // namespace dtransOP

} // namespace llvm

#endif // INTEL_DTRANS_TRANSFORMS_MEMMANAGETRANSOP_H
