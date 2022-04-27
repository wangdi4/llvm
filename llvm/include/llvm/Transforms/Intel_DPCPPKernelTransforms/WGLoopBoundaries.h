//===- WGLoopBoundaries.h - Compute workgroup loop boundaries ---*- C++ -*-===//
//
// Copyright (C) 2022 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_INTEL_DPCPPKERNELTRANSFORMS_WGLOOPBOUNDARIES_H
#define LLVM_TRANSFORMS_INTEL_DPCPPKERNELTRANSFORMS_WGLOOPBOUNDARIES_H

#include "llvm/IR/PassManager.h"

namespace llvm {

/// WGLoopBoundariesPass checks if a kernel contains branches that have 2
/// properties:
/// 1. They are uniform across all work items or they can be replaced by setting
///    a boundary on the workgroup loop (e.g. if get_global_id(0) < 5).
/// 2. If a certain work item takes one side of the branch then it has no side
///    effect instructions (load, store etc...)
/// A common scenario where this properties apply are kernels of the form:
///   __kernel void foo(int4 dim, ...) {
///     if (get_global_id(0) >= dim.x || get_global_id(1) >= dim.y) return;
///     kernel code...
///
/// If early exit is found then this pass removes the branch, so only the kernel
/// code remains, and calculate the new loop size to be used by loop generator.
/// Finally it creates a function which returns the initial global ids and
/// workgroup loop sizes in LLVM array (even if no early exits found) and
/// uniform early exit (0 - skip the kernel).
/// This array contains 2 entries per Jit dimension + 1 for uniform early exit.
///
/// The former example will result with
///   __kernel void foo(int4 dim, ...) {
///     kernel code...
///
/// In case 3 dimensions, we will get following workgroup boundaries function:
/// [7 x size_t] __kernel WG.boundaries.foo(int4 dim, ...) {
///   size_t dim0_lower = get_base_gid(0);
///   size_t dim0_upper = min(get_base_gid(0) + get_local_size(0), dim.x);
///   size_t dim0_size = dim0_upper - dim0_lower;
///   size_t dim1_lower = get_base_gid(1);
///   size_t dim1_upper = min(get_base_gid(1) + get_local_size(1), dim.x);
///   size_t dim1_size = dim1_upper - dim1_lower;
///   size_t dim2_lower = get_base_gid(2);
///   size_t dim2_size = get_local_size(2);
///   size_t uni = (0 < dim0_size) && (0 < dim_size)
///   return [uni, dim0_lower, dim0_size, dim1_lower, dim1_size, dim2_lower,
///           dim2_size];
/// }
///
/// In case 1 dimension, we will get following workgroup boundaries function:
/// [3 x size_t] __kernel WG.boundaries.foo(int4 dim, ...) {
///   size_t dim0_lower = get_base_gid(0);
///   size_t dim0_upper = min(get_base_gid(0) + get_local_size(0), dim.x);
///   size_t dim0_size = dim0_upper - dim0_lower;
///   size_t uni = (0 < dim0_size) && (get_global_gid(0) < dim.y);
///   return [uni, dim0_lower, dim0_size];
/// }
///
/// This pass and loop generator agree on the indices of the array using
/// interface implemented in WGBoundDecoder class.
class WGLoopBoundariesPass : public PassInfoMixin<WGLoopBoundariesPass> {
public:
  PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM);
};

} // namespace llvm

#endif // LLVM_TRANSFORMS_INTEL_DPCPPKERNELTRANSFORMS_WGLOOPBOUNDARIES_H
