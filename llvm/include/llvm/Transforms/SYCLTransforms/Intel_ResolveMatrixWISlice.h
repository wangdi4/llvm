//==----------------- Intel_ResolveMatrixWISlice.h - C++ -*----------------==//
//
// Copyright (C) 2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //

#ifndef LLVM_TRANSFORMS_SYCLTRANSFORMS_RESOLVE_MATRIX_WI_SLICE_LENGTH_H
#define LLVM_TRANSFORMS_SYCLTRANSFORMS_RESOLVE_MATRIX_WI_SLICE_LENGTH_H

#include "llvm/IR/PassManager.h"

namespace llvm {

// clang-format off
/// # Pass description
/// Resolve matrix WI slicing intrinsics to backend internal builtins.
/// 1.
///   call i64 @llvm.experimental.matrix.wi.slice.length.v144i32(<144 x i32> %mat, i32 12, i32 12, metadata !"matrix.rowmajor", metadata !"scope.subgroup", metadata !"matrix.use.unnecessary")
/// -->
///   call i64 @get_sub_group_slice_length.(i32 144)
/// 2.
///   call i32 @llvm.experimental.matrix.wi.slice.extractelement.v144i32.i64(<144 x i32> %mat, i32 12, i32 12, i64 %element.index, metadata !"matrix.rowmajor", metadata !"scope.subgroup", metadata !"matrix.use.unnecessary")
/// -->
///   %id = call i64 @get_sub_group_rowslice_id.v144i32.i64(<144 x i32> %mat, i32 12, i32 12, i64 %element.index)
///   %elem = call i32 @sub_group_rowslice_extractelement.i32(i64 %id)
/// 3.
///   call <144 x i32> @llvm.experimental.matrix.wi.slice.insertelement.v144i32.i64(<144 x i32> %mat, i32 12, i32 12, i32 %val, i64 %element.index, metadata !"matrix.rowmajor", metadata !"scope.subgroup", metadata !"matrix.use.unnecessary")
/// -->
///   %id = call i64 @get_sub_group_rowslice_id.v144i32.i64(<144 x i32> %mat, i32 12, i32 12, i64 %element.index)
///   call void @sub_group_rowslice_insertelement.i32(i64 %id, i32 %val)
///   %mat.update = call <144 x i32> @sub_group_insert_rowslice_to_matrix.v144i32(i64 %id)
///
/// # Mapping matrix elements to workitems
/// ## Terms: slice
/// SLICE(WI): the matrix elements owned by a single workitem.
/// The elements are mapped to each workitem by spreading each subgroup across
/// the matrix in row-major order.
/// e.g. Given a 4-by-5 subgroup-scoped matrix, and assuming the subgroup size
/// is 4.
/// The matrix elements are mapped to each workitem:
///        --------------------------
///        | W0 | W1 | W2 | W3 | W0 |
///        | W1 | W2 | W3 | W0 | W1 |
///        | W2 | W3 | W0 | W1 | W2 |
///        | W3 | W0 | W1 | W2 | W3 |
///        --------------------------
/// Workitem #0 owns the SLICE(WI0) = [Mat[0,0], Mat[0,4], Mat[1,3], Mat[2,2], Mat[3,1]]
/// (stride == SG_size if we view the matrix in row-major order)
/// The length of a workitem's slice is specified by the internal builtin:
/// @get_sub_group_slice_length.()
///
/// ## Terms: rowslice
/// ROWSLICE(i): {SLICE(WI)[i] | for each WI in the subgroup}
/// In the example above, there're 5 rowslices:
/// ROWSLICE(0) = [Mat[0,0], Mat[0,1], Mat[0,2], Mat[0,3]]
/// ROWSLICE(1) = [Mat[0,4], Mat[1,0], Mat[1,1], Mat[1,2]]
/// ROWSLICE(2) = [Mat[1,3], Mat[1,4], Mat[2,0], Mat[2,1]]
/// ROWSLICE(3) = [Mat[2,2], Mat[2,3], Mat[2,4], Mat[3,0]]
/// ROWSLICE(4) = [Mat[3,1], Mat[3,2], Mat[3,3], Mat[3,4]]
/// The length of a rowslice, is equal to the subgroup size.
///
/// # Input intrinsic semantics
/// - i32 @llvm.experimental.matrix.wi.slice.extractelement.v144i32.i64(<144 x i32> %mat, i32 12, i32 12, i64 %element.index, metadata !"matrix.rowmajor", metadata !"scope.subgroup", metadata !"matrix.use.unnecessary")
///     The argument %element.index represents the element index in the slice
///     owned by the current workitem.
///     e.g.
///     For workitem #0 in the example above, %element.index == 1 will point
///     to SLICE(WI0)[1] == Mat[0,4].
/// - Similar for insertelement
///
/// # Output builtin semantics
/// - i64 @get_sub_group_slice_length.(i32 immarg %total.element.count)
///     Will be resolved as a constant in later passes (after subgroup size is known).
///     FORMULA:
///       @get_sub_group_slice_length.(i32 %total.element.count) == ceil(%total.element.count / @get_max_sub_group_size())
///     NOTE: This still represents the length of the original slice.
/// - i64 @get_sub_group_rowslice_id.<MatrixTypeMangle>.<IndexTypeMangle>(<144 x i32> %mat, i32 12, i32 12, i64 %element.index)
///     This internal builtin is used to track which logical rowslice we're
///     trying to perform the extract/insert operations on.
///     Logically the return value refers to the ROWSLICE(%element.index),
///     but we don't simply replace it now to allow vectorizer widen the
///     extracting and inserting operations automatically.
///     ATTRIBUTES:
///     - "kernel-uniform-call": all arguments are uniform in the subgroup
///     scope.
///     - "opencl-vec-uniform-return": the return value is uniform in the
///     subgroup scope.
/// - <ElementType> @sub_group_rowslice_extractelement.<ElementTypeMangle>(i64 %rowslice.id)
///     Extracts the element owned by the current workitem, in the rowslice
///     specified by %rowslice.id (which must be the return value of
///     @get_sub_group_rowslice_id).
///     We'll generate "vector-variants" attribute for this builtin dynamically
///     in later passes, so that we can extract the whole rowslice with one call
///     in the vectorized context.
///     ATTRIBUTES:
///     - "kernel-call-once": serializing or VF pumping is not allowed.
/// - void @sub_group_rowslice_insertelement.<ElementTypeMangle>(i64 %rowslice.id, i32 %element)
///     Inserts the %element into the element owned by the current workitem, in
///     the rowslice specified by %rowslice.id (which must be the return value
///     of @get_sub_group_rowslice_id).
///     We'll generate "vector-variants" attribute for this builtin dynamically
///     in later passes, so that we can insert the whole rowslice with one call
///     in the vectorized context.
///     NOTE: With this call alone, we haven't actually inserted the rowslice
///     into the matrix yet. The actual updating operation is delegated to the
///     following builtin: @sub_group_insert_rowslice_to_matrix.
///     ATTRIBUTES:
///     - "kernel-call-once": serializing or VF pumping is not allowed.
/// - <MatrixType> @sub_group_insert_rowslice_to_matrix(i64 %rowslice.id)
///     Performs the actual inserting operation and returns the updated matrix.
///     We expect exactly one other use of %rowslice.id to be the
///     @sub_group_rowslice_insertelement call, so that we can obtain the
///     concrete data that needs to be inserted.
///     This is a uniform call in vector context since we only wants to insert
///     to the matrix by rowslice -- happening just once per subgroup is enough.
///     ATTRIBUTES:
///     - "kernel-uniform-call": all arguments are uniform in the subgroup
///     scope.
///     - "opencl-vec-uniform-return": the return value is uniform in the
///     subgroup scope.
// clang-format on
class ResolveMatrixWISlicePass
    : public PassInfoMixin<ResolveMatrixWISlicePass> {
public:
  PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM);

  // Glue for old PM.
  bool runImpl(Module &M);

  static bool isRequired() { return true; }
};
} // namespace llvm

#endif // LLVM_TRANSFORMS_SYCLTRANSFORMS_RESOLVE_MATRIX_WI_SLICE_LENGTH_H
