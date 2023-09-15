//===------------------ Intel_PaddedPointerPropagation.h  -----------------===//
//
// Copyright (C) 2018 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
//
// This file provides declaration methods and classes for support of the
// propagation of the padded pointer property across the program call graph and
// an interface to mark a Value as having a padding and an interface to query to
// check if a Value points to padded memory
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_INTEL_PADDEDPTRPROP_H
#define LLVM_TRANSFORMS_INTEL_PADDEDPTRPROP_H

#include "llvm/IR/Module.h"
#include "llvm/IR/PassManager.h"
#include "llvm/IR/IntrinsicInst.h"

namespace llvm {

namespace dtransOP {

/// Pass for new pass manager for padding property propagation.
class PaddedPtrPropOPPass : public PassInfoMixin<PaddedPtrPropOPPass> {
public:
  PreservedAnalyses run(Module &M, ModuleAnalysisManager &);
};

} // namespace dtransOP

/// A routine to query if a Value points to padded memory
int getPaddingForValue(Value *V);

/// A routine to insert padding markup for a Value
void insertPaddedMarkUp(Value *V, int Padding);

/// A routine for padding markup removal
void removePaddedMarkUp(IntrinsicInst *I);

} // namespace llvm

#endif // LLVM_TRANSFORMS_INTEL_PADDEDPTRPROP_H
