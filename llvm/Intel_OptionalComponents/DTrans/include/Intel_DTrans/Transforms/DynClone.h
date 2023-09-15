//===--------------- DynClone.h - DTransDynClonePass ------------===//
//
// Copyright (C) 2018 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
//
// This file declares the DTrans Dynamic Cloning optimization pass.
//
//===----------------------------------------------------------------------===//

#ifndef INTEL_DTRANS_TRANSFORMS_DYNCLONE_H
#define INTEL_DTRANS_TRANSFORMS_DYNCLONE_H

#include "llvm/Analysis/LoopInfo.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/PassManager.h"

namespace llvm {

class TargetLibraryInfo;
class WholeProgramInfo;

namespace dtrans {

using LoopInfoFuncType = std::function<LoopInfo &(Function &)>;
using DynGetTLITy = std::function<const TargetLibraryInfo &(Function &)>;

} // namespace dtrans

namespace dtransOP {

class DTransSafetyInfo;

using LoopInfoFuncType = std::function<LoopInfo &(Function &)>;
using DynGetTLITy = std::function<const TargetLibraryInfo &(Function &)>;

class DynClonePass : public PassInfoMixin<DynClonePass> {
public:
  PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM);

  bool runImpl(Module &M, DTransSafetyInfo &DTInfo, DynGetTLITy GetTLI,
               WholeProgramInfo &WPInfo, LoopInfoFuncType &GetLI);
};

} // namespace dtransOP

} // namespace llvm

#endif // INTEL_DTRANS_TRANSFORMS_DYNCLONE_H
