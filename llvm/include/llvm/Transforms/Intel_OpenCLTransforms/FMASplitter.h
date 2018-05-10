//==----- FMASplitter.h - Pass to split FMA intrinsic --------*- C++ -*-----==//
//
// Copyright (C) 2018 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //
#ifndef INTEL_OPENCL_TRANSFORMS_FMASPLITTER_H
#define INTEL_OPENCL_TRANSFORMS_FMASPLITTER_H

#include "llvm/IR/PassManager.h"

namespace llvm {
class FMASplitterPass : public PassInfoMixin<FMASplitterPass> {
public:
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &AM);
};
}

#endif // INTEL_OPENCL_TRANSFORMS_FMASPLITTER_H
