//===--------------------- HIRRowWiseMV.h ----------------------*- C++-*---===//
//
// Copyright (C) 2019 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
/// \file
/// Exposes the HIR row-wise multiversioning pass.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_HIRROWWISEMVPASS_H
#define LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_HIRROWWISEMVPASS_H

#include "llvm/Transforms/Intel_LoopTransforms/HIRTransformPass.h"

namespace llvm {

namespace loopopt {

/// An HIR pass which performs row-wise multiversioning.
class HIRRowWiseMVPass : public HIRPassInfoMixin<HIRRowWiseMVPass> {
public:
  static constexpr auto PassName = "hir-rowwise-mv";
  PreservedAnalyses runImpl(Function &, FunctionAnalysisManager &,
                            HIRFramework &);
};

} // namespace loopopt

} // namespace llvm

#endif // LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_HIRROWWISEMVPASS_H
