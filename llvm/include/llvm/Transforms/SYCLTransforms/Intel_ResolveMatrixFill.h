//==--- Intel_ResolveMatrixFill.h - Resolve matrix fill intrinsics -- C++ -*==//
//
// Copyright (C) 2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //

#ifndef LLVM_TRANSFORMS_SYCLTRANSFORMS_RESOLVE_MATRIX_FILL_H
#define LLVM_TRANSFORMS_SYCLTRANSFORMS_RESOLVE_MATRIX_FILL_H

#include "llvm/IR/PassManager.h"

namespace llvm {

/// If the fill data is not zero, resolve @llvm.experimental.matrix.fill
/// intrinsic to:
/// - A @llvm.experimental.matrix.fill call with zero data followed by a loop of
/// @llvm.experimental.wi.slice.insertelement calls to actually fill the matrix
/// with desired data.

// clang-format off
/// Example 1:
/// call <15 x i32> @llvm.experimental.matrix.fill.v15i32.i32(i32 0, i32 3, i32 5, metadata !"matrix.rowmajor", metadata !"scope.subgroup", metadata !"matrix.use.unnecessary")
/// --> we keep it as is, because CodeGen will handle the zero-filling itself.
///
/// Example 2:
/// %mat = call <15 x i32> @llvm.experimental.matrix.fill.v15i32.i32(i32 %data, i32 3, i32 5, metadata !"matrix.rowmajor", metadata !"scope.subgroup", metadata !"matrix.use.unnecessary")
/// call void @foo(<15 x i32> %mat)
/// -->
/// entry:
///   %mat.init = <15 x i32> @llvm.experimental.matrix.fill.v15i32.i32(i32 0, i32 3, i32 5, metadata !"matrix.rowmajor", metadata !"scope.subgroup", metadata !"matrix.use.unnecessary")
///   %length = call i32 @llvm.experimental.matrix.wi.slice.length.v15i32(<15 x i32> %mat.init, i32 3, i32 5, metadata !"matrix.rowmajor", metadata !"scope.subgroup", metadata !"matrix.use.unnecessary")
///   br label %matrix.fill.slice.loop.header
/// matrix.fill.slice.loop.header:
///   %i = phi i32 [ 0, %entry ], [ %i.inc, %matrix.fill.slice.loop ]
///   %mat = phi <15 x i32> [ %mat.init, %entry ], [ %mat.update, %matrix.fill.slice.loop ]
///   %c = icmp lt i32 %i, %length
///   br i1 %c, label %matrix.fill.slice.loop, label %matrix.fill.slice.loop.end
/// matrix.fill.slice.loop:
///   %mat.update = call <15 x i32> @llvm.experimental.wi.slice.insertelement.v15i32.i32(<15 x i32> %mat, i32 3, i32 5, i32 %data, i32 %i, metadata !"matrix.rowmajor", metadata !"scope.subgroup", metadata !"matrix.use.unnecessary")
///   %i.inc = add i32 %i, 1
///   br label %matrix.fill.slice.loop.header
/// matrix.fill.slice.loop.end:
///   call void @foo(<15 x i32> %mat)
// clang-format on

class ResolveMatrixFillPass : public PassInfoMixin<ResolveMatrixFillPass> {
public:
  PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM);

  // Glue for old PM.
  bool runImpl(Module &M);

  static bool isRequired() { return true; }
};

} // namespace llvm

#endif // LLVM_TRANSFORMS_SYCLTRANSFORMS_RESOLVE_MATRIX_FILL_H
