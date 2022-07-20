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

include "GENERIC/code_helpers.td"
include "GENERIC/opencl-builtin.td"
include "source/core/targets.td"

// --- Dummy subgroup builtins ---
//
// ResolveSubGroupWICall doesn't resolve these builtin when they are contained
// in functions that are NOT called by the kernel.
// So we need to provide fake implementations to avoid "unimplemented" error.
//
// Prototypes
let Types = [v1i32], Attrs = [OVERLOADABLE] in {
defset list<OclBuiltin> dummy_builtins = {
  def get_sub_group_size : OclBuiltin<"get_sub_group_size", (outs uint:$ret), (ins void)>;
  def get_max_sub_group_size : OclBuiltin<"get_max_sub_group_size", (outs uint:$ret), (ins void)>;
  def get_num_sub_groups : OclBuiltin<"get_num_sub_groups", (outs uint:$ret), (ins void)>;
  def get_enqueued_num_sub_groups : OclBuiltin<"get_enqueued_num_sub_groups", (outs uint:$ret), (ins void)>;
  def get_sub_group_id : OclBuiltin<"get_sub_group_id", (outs uint:$ret), (ins void)>;
  def get_sub_group_local_id : OclBuiltin<"get_sub_group_local_id", (outs uint:$ret), (ins void)>;
} // defset dummy_builtins
defset list<OclBuiltin> dummy_no_ret_builtins = {
  def sub_group_barrier : OclBuiltin<"sub_group_barrier", (outs void:$ret), (ins cl_mem_fence_flags:$fence)>;
  def sub_group_barrier_with_scope : OclBuiltin<"sub_group_barrier", (outs void:$ret), (ins cl_mem_fence_flags:$fence, memory_scope:$scope)>;
} // defset dummy_no_ret_builtins
}

// Implementations
foreach bi = dummy_builtins in {
  def : OclBuiltinImpl<bi, bi.Types, /*isDeclOnly*/false, ret_zero_code>, SSE;
}
foreach bi = dummy_no_ret_builtins in {
  def : OclBuiltinImpl<bi, bi.Types, /*isDeclOnly*/false, ret_void_code>, SSE;
}

// VectInfos (not needed for dummy subgroup builtins)
