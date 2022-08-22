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

// --- sub_group_shuffle builtins ---
//
// 
// https://registry.khronos.org/OpenCL/extensions/intel/cl_intel_subgroups.html
// https://registry.khronos.org/OpenCL/specs/3.0-unified/html/OpenCL_Ext.html#cl_khr_subgroup_shuffle
//

defvar sub_group_types = [v1f16];

// Prototypes
let Attrs = [OVERLOADABLE], VectorAttrs = [KERNEL_CALL_ONCE], NeedForwardDecl = true in {
  // scalar
  let Types = [v1f16] in {
    def sub_group_shuffle_vf1 : OclBuiltin<"intel_sub_group_shuffle", (outs gentype:$ret), (ins gentype:$data, v1u32: $sub_group_local_id)>;
    def sub_group_shuffle_xor_vf1 : OclBuiltin<"intel_sub_group_shuffle_xor", (outs gentype:$ret), (ins gentype:$data, v1u32: $sub_group_local_id)>;
    def sub_group_shuffle_up_vf1 : OclBuiltin<"intel_sub_group_shuffle_up", (outs gentype:$ret), (ins gentype:$data, gentype:$current, v1u32: $sub_group_local_id)>;
    def sub_group_shuffle_down_vf1 : OclBuiltin<"intel_sub_group_shuffle_down", (outs gentype:$ret), (ins gentype:$data, gentype:$next, v1u32: $sub_group_local_id)>;

    def khr_sub_group_shuffle_up_vf1 : OclBuiltin<"sub_group_shuffle_up", (outs gentype:$ret), (ins gentype:$data, v1u32: $delta)>;
    def khr_sub_group_shuffle_down_vf1 : OclBuiltin<"sub_group_shuffle_down", (outs gentype:$ret), (ins gentype:$data, v1u32: $delta)>;
  }
  // vectorized
  foreach vf = [4, 8, 16, 32, 64] in {
    defvar mask_type = !cast<OclType>("v" # vf # "u32");
    let Types = ExpandTypesByVFAndFlatten<sub_group_types, [vf]>.Tout in {
      def sub_group_shuffle_vf # vf : OclBuiltin<"intel_sub_group_shuffle", (outs gentype:$ret), (ins gentype:$data, mask_type: $sub_group_local_id, mask_type:$vec_mask)>;
      def sub_group_shuffle_xor_vf # vf : OclBuiltin<"intel_sub_group_shuffle_xor", (outs gentype:$ret), (ins gentype:$data, mask_type: $value, mask_type:$vec_mask)>;
      def sub_group_shuffle_up_vf # vf : OclBuiltin<"intel_sub_group_shuffle_up", (outs gentype:$ret), (ins gentype:$data, gentype:$current, mask_type: $value, mask_type:$vec_mask)>;
      def sub_group_shuffle_down_vf # vf : OclBuiltin<"intel_sub_group_shuffle_down", (outs gentype:$ret), (ins gentype:$data, gentype:$next, mask_type: $value, mask_type:$vec_mask)>;

      def khr_sub_group_shuffle_up_vf # vf : OclBuiltin<"sub_group_shuffle_up", (outs gentype:$ret), (ins gentype:$data, mask_type: $delta, mask_type:$vec_mask)>;
      def khr_sub_group_shuffle_down_vf # vf : OclBuiltin<"sub_group_shuffle_down", (outs gentype:$ret), (ins gentype:$data, mask_type: $delta, mask_type:$vec_mask)>;
    }
  }

  // Because half type shuffle builtins are implemented via short type builtins.
  // So just declare short builtins here, when all builtin are migrated here,
  // these declarations can be deleted.
  foreach vf = [4, 8, 16, 32, 64] in {
    defvar mask_type = !cast<OclType>("v" # vf # "u32");
    let Types = ExpandTypesByVFAndFlatten<[v1i16], [vf]>.Tout in {
      def sub_group_shuffle_short_vf # vf : OclBuiltin<"intel_sub_group_shuffle", (outs gentype:$ret), (ins gentype:$data, mask_type: $sub_group_local_id, mask_type:$vec_mask)>;
      def sub_group_shuffle_xor_short_vf # vf : OclBuiltin<"intel_sub_group_shuffle_xor", (outs gentype:$ret), (ins gentype:$data, mask_type: $value, mask_type:$vec_mask)>;
      def sub_group_shuffle_up_short_vf # vf : OclBuiltin<"intel_sub_group_shuffle_up", (outs gentype:$ret), (ins gentype:$previous, gentype:$current, mask_type: $value, mask_type:$vec_mask)>;
      def sub_group_shuffle_down_short_vf # vf : OclBuiltin<"intel_sub_group_shuffle_down", (outs gentype:$ret), (ins gentype:$current, gentype:$next, mask_type: $value, mask_type:$vec_mask)>;
      
      def khr_sub_group_shuffle_up_short_vf # vf : OclBuiltin<"sub_group_shuffle_up", (outs gentype:$ret), (ins gentype:$data, mask_type: $delta, mask_type:$vec_mask)>;
      def khr_sub_group_shuffle_down_short_vf # vf : OclBuiltin<"sub_group_shuffle_down", (outs gentype:$ret), (ins gentype:$data, mask_type: $delta, mask_type:$vec_mask)>;
    }
  }
}
// Implementations
// scalar
foreach bi = [sub_group_shuffle_vf1, sub_group_shuffle_xor_vf1,
              sub_group_shuffle_up_vf1, khr_sub_group_shuffle_up_vf1,
              sub_group_shuffle_down_vf1, khr_sub_group_shuffle_down_vf1] in {
  def : OclBuiltinImpl<bi, sub_group_types, false, [{
    return $Arg0VarName;
  }]>, SHARED;
}

// vectorized
defvar intel_sub_group_shuffle_f16_code = [{
    short$VecLength temp = __builtin_astype($Arg0VarName, short$VecLength);
    temp = $func(temp, $Arg1VarName, $Arg2VarName);
    $Arg0VarName = __builtin_astype(temp, $Arg0Type);
    return $Arg0VarName;
}];

defvar intel_sub_group_shuffle_up_down_f16_code = [{
    short$VecLength temp1 = __builtin_astype($Arg0VarName, short$VecLength);
    short$VecLength temp2 = __builtin_astype($Arg1VarName, short$VecLength);
    temp1 = $func(temp1, temp2, $Arg2VarName, $Arg3VarName);
    $Arg0VarName = __builtin_astype(temp1, $Arg0Type);
    return $Arg0VarName;
}];

foreach vf = [4, 8, 16, 32, 64] in {
  defvar builtin = !cast<OclBuiltin>("sub_group_shuffle_vf" # vf);
  def : OclBuiltinImpl<builtin, builtin.Types, /*isDeclOnly*/false, intel_sub_group_shuffle_f16_code, [["$func", "intel_sub_group_shuffle"]]>, SHARED;

  defvar builtin_xor = !cast<OclBuiltin>("sub_group_shuffle_xor_vf" # vf);
  def : OclBuiltinImpl<builtin_xor, builtin_xor.Types, /*isDeclOnly*/false, intel_sub_group_shuffle_f16_code, [["$func", "intel_sub_group_shuffle_xor"]]>, SHARED;

  defvar builtin_up = !cast<OclBuiltin>("sub_group_shuffle_up_vf" # vf);
  def : OclBuiltinImpl<builtin_up, builtin_up.Types, /*isDeclOnly*/false, intel_sub_group_shuffle_up_down_f16_code, [["$func", "intel_sub_group_shuffle_up"]]>, SHARED;

  defvar builtin_down = !cast<OclBuiltin>("sub_group_shuffle_down_vf" # vf);
  def : OclBuiltinImpl<builtin_down, builtin_down.Types, /*isDeclOnly*/false, intel_sub_group_shuffle_up_down_f16_code, [["$func", "intel_sub_group_shuffle_down"]]>, SHARED;

  defvar builtin_khr_up = !cast<OclBuiltin>("khr_sub_group_shuffle_up_vf" # vf);
  def : OclBuiltinImpl<builtin_khr_up, builtin_khr_up.Types, /*isDeclOnly*/false, intel_sub_group_shuffle_f16_code, [["$func", "sub_group_shuffle_up"]]>, SHARED;

  defvar builtin_khr_down = !cast<OclBuiltin>("khr_sub_group_shuffle_down_vf" # vf);
  def : OclBuiltinImpl<builtin_khr_down, builtin_khr_down.Types, /*isDeclOnly*/false, intel_sub_group_shuffle_f16_code, [["$func", "sub_group_shuffle_down"]]>, SHARED;
}

// VectInfos
foreach suffix = ["", "_xor", "_up", "_down"] in {
  let Types = [v1f16] in {
    def : VectInfo<[], !cast<OclBuiltin>("sub_group_shuffle"#suffix#"_vf1"), !cast<OclBuiltin>("sub_group_shuffle"#suffix#"_vf4"), 
                      !cast<OclBuiltin>("sub_group_shuffle"#suffix#"_vf8"), !cast<OclBuiltin>("sub_group_shuffle"#suffix#"_vf16"), 
                      !cast<OclBuiltin>("sub_group_shuffle"#suffix#"_vf32"), !cast<OclBuiltin>("sub_group_shuffle"#suffix#"_vf64")>;
  }
}
foreach suffix = ["_up", "_down"] in {
  let Types = [v1f16] in {
    def : VectInfo<[], !cast<OclBuiltin>("khr_sub_group_shuffle"#suffix#"_vf1"), !cast<OclBuiltin>("khr_sub_group_shuffle"#suffix#"_vf4"), 
                      !cast<OclBuiltin>("khr_sub_group_shuffle"#suffix#"_vf8"), !cast<OclBuiltin>("khr_sub_group_shuffle"#suffix#"_vf16"), 
                      !cast<OclBuiltin>("khr_sub_group_shuffle"#suffix#"_vf32"), !cast<OclBuiltin>("khr_sub_group_shuffle"#suffix#"_vf64")>;
  }
}

// AliasMap
foreach vf = [1, 4, 8, 16, 32, 64] in {
  defvar types = WidenType<v1f16, vf>.Tout;
  def : AliasMap<[["sub_group_shuffle_vf" #vf, "sub_group_shuffle"], 
                  ["sub_group_shuffle_xor_vf" #vf, "sub_group_shuffle_xor"]], [types]>;
}
