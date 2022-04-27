//==-------------------- ResolveMatrixLayout.h -  C++ -*-------------------==//
//
// Copyright (C) 2022 Intel Corporation
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you ("License"). Unless the License provides otherwise, you may
// not use, modify, copy, publish, distribute, disclose or transmit this
// software or the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.
// ===--------------------------------------------------------------------=== //

#ifndef LLVM_TRANSFORMS_INTEL_DPCPP_KERNEL_TRANSFORMS_RESOLVE_MATRIX_LAYOUT_H
#define LLVM_TRANSFORMS_INTEL_DPCPP_KERNEL_TRANSFORMS_RESOLVE_MATRIX_LAYOUT_H

#include "llvm/IR/PassManager.h"

namespace llvm {

// clang-format off
/// # Pass description
/// Resolve matrix layout of matrix load intrinsics.
/// Transform
/// %res = call <8 x i8> @llvm.experimental.matrix.load.v8i8.p4i8(
///   i32* addressspace(4) %ptr, i64 stride, i1 false, i32 4, i32 2,
///   metadata !"matrix.packed_b", metadata !"matrix.rowmajor", metadata
///   !"scope.subgroup")
/// into
/// %alloc = alloca [i8 x 8]
/// %ptr2 = bitcast %alloc to i8*
/// matrix_layout_transform_rowmajor_to_vnni(ptr, %ptr2, rows, cols, stride)
/// %res = call <8 x i8> @llvm.experimental.matrix.load.v8i8.p4i8(
///   i8% ptr2, i64 stride, i1 false, i32 4, i32 2,
///   metadata !"matrix.packed_b", metadata !"matrix.packed_b", metadata
///   !"scope.subgroup")
// clang-format on
class ResolveMatrixLayoutPass
    : public PassInfoMixin<ResolveMatrixLayoutPass> {
public:
  PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM);

  // Glue for old PM.
  bool runImpl(Module &M);
};
} // namespace llvm

#endif // LLVM_TRANSFORMS_INTEL_DPCPP_KERNEL_TRANSFORMS_RESOLVE_MATRIX_LAYOUT_H
