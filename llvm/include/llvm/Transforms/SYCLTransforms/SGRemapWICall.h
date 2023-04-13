//==-------- SGRemapWICall.h - Remap work-item function calls ------- C++ -==//
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
//
// ===--------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_SYCLTRANSFORMS_REMAP_WI_CALL_H
#define LLVM_TRANSFORMS_SYCLTRANSFORMS_REMAP_WI_CALL_H

#include "llvm/IR/PassManager.h"
#include "llvm/Transforms/SYCLTransforms/Utils/CompilationUtils.h"

namespace llvm {

/// Remap work-item calls for subgroup construction.
///
/// When end user enqueues a kernel with a specific NDRange configuration, OCL
/// CPU runtime may choose a different NDRange to launch the kernel in order to
/// support different subgroup construction modes. In such cases, we use this
/// pass to transform WI calls so that their return values are recovered to
/// produce the correct value for the user-specific NDRange.
///
/// Currently two different NDRange mapping modes are supported: linearize and
/// dimension-swapping.
///
/// Example of linearize:
/// User enqueues the kernel with global sizes (8,4,2) and local sizes (4, 2,
/// 1), and runtime launches the kernel with global sizes (64,1,1) and local
/// sizes (8,1,1) instead, i.e., the dimensions are flatten (or linearized) into
/// the 0th dimension.
/// Please note that linearization is done within each original work-group. This
/// is why we translate all get_global_id calls to the combination of
/// (get_local_id, get_group_id, get_local_size) first.
///
/// Example of dimension-swapping:
/// User enqueues the kernel with global sizes (8,4,2), and runtime launches the
/// kernel with global sizes (2,4,8) instead, i.e., the 0th dimension is swapped
/// with the 2nd dimension.
///
/// Term definition - "user." variant:
/// The "user." variant of a WI function effectively represents the semantics
/// for the user-specific NDRange, and it will be resolved by the ResolveWICall
/// pass.
/// For example, "user.get_global_size" is the "user." variant of
/// "get_global_size".
///
/// The following WI calls will be remapped according to the formula:
/// Linearize mode:
///     - get_global_id(i32 dim)
///       --> get_local_id(dim) + get_group_id(dim) * get_local_size(dim)
///       Note: get_local_id, get_group_id and get_local_size will be processed
///       later.
///     - get_local_id(i32 dim)
///       --> get_local_id(0) / Prod{get_local_size(i) for i in [0, dim)}
///           % get_local_size(dim)
///     - get_group_id(i32 dim)
///       --> get_group_id(0) / Prod{get_num_groups(i) for i in [0, dim)}
///           % get_num_groups(dim)
/// Dimension-swapping mode:
///     - get_global_id(i32 dim)
///       --> get_global_id(0) if dim == SwapWithZeroDim
///       --> get_global_id(SwapWithZeroDim) if dim == 0
///     - get_local_id(i32 dim)
///       --> get_local_id(0) if dim == SwapWithZeroDim
///       --> get_local_id(SwapWithZeroDim) if dim == 0
///     - get_group_id(i32 dim)
///       --> get_group_id(0) if dim == SwapWithZeroDim
///       --> get_group_id(SwapWithZeroDim) if dim == 0
///
/// Finally, this pass replaces all the following WI functions with their
/// "user." variants respectively:
/// - get_global_size
/// - get_local_size
/// - get_enqueued_local_size
/// - get_num_groups
///
/// When this pass finishes processing, "user.get_global_size",
/// "user.get_local_size", "user.get_enqueued_local_size", "user.get_num_groups"
/// may be generated. And ResolveWICall will resolve them to user-specific
/// NDRange sizes.
///
/// FIXME:
/// 1. Respect kernel metadata (or attribute) that comes from FE.
/// 2. Will have to clone subroutines containing affected WI calls for kernels
///    requiring different remapping modes.
class SGRemapWICallPass : public PassInfoMixin<SGRemapWICallPass> {
public:
  SGRemapWICallPass(
      SubGroupConstructionMode RemapMode = SubGroupConstructionMode::X)
      : RemapMode(RemapMode) {}

  PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM);

  static bool isRequired() { return true; }

private:
  SubGroupConstructionMode RemapMode;
};

} // namespace llvm

#endif // LLVM_TRANSFORMS_SYCLTRANSFORMS_REMAP_WI_CALL_H
