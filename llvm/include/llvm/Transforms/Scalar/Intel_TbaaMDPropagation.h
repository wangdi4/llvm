//===- TbaaMDPropagation.h - TBAA MD Propagation Pass -------*- C++ -*-===//
//
// Copyright (C) 2015-2016 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
/// \file
/// This file provides the interface for LLVM's TBAA propagation pass
/// which recovers the tbaa for the return pointer dereferences.
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_SCALAR_TBAAMDPROPAGATION_H
#define LLVM_TRANSFORMS_SCALAR_TBAAMDPROPAGATION_H

#include "llvm/IR/Function.h"
#include "llvm/IR/PassManager.h"

namespace llvm {

/// Performs the Pass to recover the tbaa for the return pointer
/// dereference.
class TbaaMDPropagationPass : public PassInfoMixin<TbaaMDPropagationPass> {
public:
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &AM);
};
/// Removes extra intel.fakeload intrinsics after all inlining is finished.
class CleanupFakeLoadsPass : public PassInfoMixin<CleanupFakeLoadsPass> {
public:
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &AM);
};
} // end namespace llvm

#endif // LLVM_TRANSFORMS_SCALAR_TBAAMDPROPAGATION_H
