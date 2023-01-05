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

include "GENERIC/ll_generation/definition.td"

// sub_group_ballot vector implementations
// scalar:
//   uint4 sub_group_ballot(int predicate)
// vf=16:
//   uint64 sub_group_ballot(int16 predicate, uint16 vec_mask)
let EmitMangledName = true,
    FuncAttrs = ["memory(none)", "willreturn", "nounwind"] in {
  defvar builtin = "sub_group_ballot";
foreach vf = [4, 8, 16, 32, 64] in {
  defvar ret_type = !cast<VectorType>("v" # !mul(vf, 4) # "i32");
  defvar predicate_type = !cast<VectorType>("v" # vf # "i32");
  defvar mask_type = predicate_type;

  defvar predicate_mangle = MangleVectorType<predicate_type, /*signed*/true>.ret;
  defvar mask_mangle = MangleVectorType<mask_type, /*signed*/false>.ret;

  // e.g. vf = 4
  // ext_index = [0, 1, 2, 3, 4, 4, ..., 4] ; of length 128
  defvar ext_index = Range<0, vf>.Tout # !listsplat(vf, !sub(128, vf));
  // broadcast_index = [0, 1, 2, 3, 0, 1, 2, 3, ..., 0, 1, 2, 3] ; of length 4*vf
  defvar broadcast_index = !foldl([]<int>, !listsplat(Range<0, 4>.Tout, vf), acc, inner_list, acc # inner_list);
  defm builtin # vf : LLDefine<builtin, [Value<predicate_type, "predicate">, Value<mask_type, "vec_mask">], ret_type, [{
    %mask = and {Args[0]}, {Args[1].name}
    %to.bool = icmp ne {Args[0].type} %mask, zeroinitializer
    ; Extend <VF x i1> to 128 bits to fit in uint4
    %ext.128bit = shufflevector <{VF} x i1> %to.bool, <{VF} x i1> zeroinitializer, <128 x i32> {EXT_INDEX}
    %int4 = bitcast <128 x i1> %ext.128bit to <4 x i32>
    %broadcast = shufflevector <4 x i32> %int4, <4 x i32> poison, {RetType} {BROADCAST_INDEX}
    ret {RetType} %broadcast
  }], [Macro<"VF", !cast<string>(vf)>, Macro<"EXT_INDEX", JoinIndices<ext_index>.Tout>, Macro<"BROADCAST_INDEX", JoinIndices<broadcast_index>.Tout>], predicate_mangle # mask_mangle>;
} // foreach vf
}

// sub_group_reduce_add vector implementations of half type
// e.g. vf=16: sub_group_reduce_add(int16 src, uint16 vec_mask)
let EmitMangledName = true,
    FuncAttrs = ["memory(none)", "willreturn", "nounwind"] in {
  defvar reduce_add_builtin = "sub_group_reduce_add";
  defvar reduce_add_builtin_non_uniform = "sub_group_non_uniform_reduce_add";
  defvar reduce_add_ir_code =
    [{
      %usrc = bitcast <{VF} x half> %src to <{VF} x i16>
      %uvec_mask = trunc <{VF} x i32> %vec_mask to <{VF} x i16>
      %umask_src = and <{VF} x i16> %usrc, %uvec_mask
      %mask_src = bitcast <{VF} x i16> %umask_src to <{VF} x half>
      %sum = call half @llvm.vector.reduce.fadd.v{VF}f16(half 0xH0000, <{VF} x half> %mask_src)
      %vsum = insertelement <{VF} x half> poison, half %sum, i64 0
      %result = shufflevector <{VF} x half> %vsum, <{VF} x half> poison, <{VF} x i32> zeroinitializer
      ret {RetType} %result
    }];

  foreach vf = [4, 8, 16, 32, 64] in {
    defvar ret_type = !cast<VectorType>("v" # vf # "f16");
    defvar src_type = !cast<VectorType>("v" # vf # "f16");
    defvar mask_type = !cast<VectorType>("v" # vf # "i32");

    defvar src_mangle = MangleVectorType<src_type, /*signed*/true>.ret;
    defvar mask_mangle = MangleVectorType<mask_type, /*signed*/false>.ret;

    defvar func_name = "llvm.vector.reduce.fadd.v" # vf #f16;
    defvar start_type = !cast<FloatType>("f16");
    defvar value_type = !cast<VectorType>("v" # vf # "f16");
    defvar llvm_ret_type = start_type;
    let EmitMangledName = false in
      def func_name # vf : LLDeclare<func_name, [start_type, value_type], llvm_ret_type, "">;

    defm reduce_add_builtin # vf : LLDefine<reduce_add_builtin, [Value<src_type, "src">, Value<mask_type, "vec_mask">], ret_type,
      reduce_add_ir_code, [Macro<"VF", !cast<string>(vf)>], src_mangle # mask_mangle>;
    defm reduce_add_builtin_non_uniform # vf : LLDefine<reduce_add_builtin_non_uniform, [Value<src_type, "src">, Value<mask_type, "vec_mask">], ret_type,
      reduce_add_ir_code, [Macro<"VF", !cast<string>(vf)>], src_mangle # mask_mangle>;
  }
}
