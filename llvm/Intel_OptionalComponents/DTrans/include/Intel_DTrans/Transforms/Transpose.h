//===---------------- Transpose.h - DTransTransposePass -------------------===//
//
// Copyright (C) 2019 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
//
// This file declares the DTrans Transpose optimization for Fortran
// multi-dimensional arrays.
//
//===----------------------------------------------------------------------===//

#if !INTEL_FEATURE_SW_DTRANS
#error Transpose.h include in an non-INTEL_FEATURE_SW_DTRANS build.
#endif

#ifndef INTEL_DTRANS_TRANSFORMS_TRANSPOSE_H
#define INTEL_DTRANS_TRANSFORMS_TRANSPOSE_H

#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/PassManager.h"

namespace llvm {

class LoopInfo;

namespace dtrans {

// Lambda function to collect the LoopInfo for a given function
using TransposeLoopInfoFuncType = function_ref<LoopInfo &(Function &)>;
using TransposeTLIType =
    std::function<const TargetLibraryInfo &(Function &)>;

class TransposePass : public PassInfoMixin<dtrans::TransposePass> {
public:
  PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM);

  bool runImpl(Module &M, TransposeLoopInfoFuncType GetLI,
               dtrans::TransposeTLIType GetTLI);
};

} // end namespace dtrans

} // end namespace llvm

#endif // INTEL_DTRANS_TRANSFORMS_TRANSPOSE_H
