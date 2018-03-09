//===---------- Intel_LoopOptMarker.h - LoopOpt Marker ----------*- C++ -*-===//
//
// Copyright (C) 2017 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_SCALAR_LOOPOPTMARKER_H
#define LLVM_TRANSFORMS_SCALAR_LOOPOPTMARKER_H

#include "llvm/IR/Function.h"
#include "llvm/IR/PassManager.h"

namespace llvm {

class LoopOptMarkerPass : public PassInfoMixin<LoopOptMarkerPass> {
public:
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &);
};

}

#endif //LLVM_TRANSFORMS_SCALAR_LOOPOPTMARKER_H
