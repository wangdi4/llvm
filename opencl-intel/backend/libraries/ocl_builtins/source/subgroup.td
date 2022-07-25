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
  def : OclBuiltinImpl<bi, bi.Types, /*isDeclOnly*/false, ret_zero_code>, SHARED;
}
foreach bi = dummy_no_ret_builtins in {
  def : OclBuiltinImpl<bi, bi.Types, /*isDeclOnly*/false, ret_void_code>, SHARED;
}

// VectInfos (not needed for dummy subgroup builtins)


// --- cl_khr_subgroup_ballot ---
//
// https://www.khronos.org/registry/OpenCL/specs/3.0-unified/html/OpenCL_Ext.html#cl_khr_subgroup_ballot
//
// For the sub_group_non_uniform_broadcast and sub_group_broadcast_first
// functions, the generic type name gentype may be one of the supported
// built-in scalar data types char, uchar, short, ushort, int, uint, long,
// ulong, float, double (if double precision is supported), or half (if half
// precision is supported).
//
// For the sub_group_non_uniform_broadcast function, the generic type name
// gentype may additionally be one of the supported built-in vector data
// types charn, ucharn, shortn, ushortn, intn, uintn, longn, ulongn, floatn,
// doublen (if double precision is supported), or halfn (if half precision is
// supported).
//
// TODO: implement
// sub_group_non_uniform_broadcast
// sub_group_broadcast_first
// sub_group_inverse_ballot
// sub_group_ballot_bit_extract
// sub_group_ballot_bit_count
// sub_group_ballot_inclusive_scan
// sub_group_ballot_exclusive_scan
// sub_group_ballot_find_lsb
// sub_group_ballot_find_msb
// get_sub_group_eq_mask
// get_sub_group_ge_mask
// get_sub_group_gt_mask
// get_sub_group_le_mask
// get_sub_group_lt_mask
//
// Prototypes
let Attrs = [OVERLOADABLE], VectorAttrs = [KERNEL_CALL_ONCE] in {
  def sub_group_ballot : OclBuiltin<"sub_group_ballot", (outs v4u32:$ret), (ins gentype:$predicate), [v1i32]>;
  foreach vf = [4, 8, 16, 32, 64] in {
    defvar mask_type = !cast<OclType>("v" # vf # "u32");
    def sub_group_ballot_vf # vf : OclBuiltin<"sub_group_ballot", (outs WidenType<v4u32, vf>.Tout:$ret), (ins gentype:$predicate, mask_type:$vec_mask), [WidenType<v1i32, vf>.Tout]>;
  }
}

// Implementations
def : OclBuiltinImpl<sub_group_ballot, sub_group_ballot.Types, false, ret_zero_code>, SHARED;
foreach vf = [4, 8, 16, 32, 64] in {
  defvar builtin = !cast<OclBuiltin>("sub_group_ballot_vf" # vf);
  // Implementation is written in LLVM IR (shared/ll_subgroup_impl.td).
  def : OclBuiltinImpl<builtin, builtin.Types, /*IsDeclOnly*/true, [{}]>, SHARED;
} // foreach vf

// VectInfos
let Types = [v1i32] in {
def : VectInfo<[], sub_group_ballot, sub_group_ballot_vf4, sub_group_ballot_vf8, sub_group_ballot_vf16, sub_group_ballot_vf32, sub_group_ballot_vf64>;
}
