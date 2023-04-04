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

// e.g.
// TruncOrZextCodeGen<"%val", i32, i64>.Tout == "%val.scale = zext i32 %val to i64"
// TruncOrZextCodeGen<"%val", i64, i32>.Tout == "%val.scale = trunc i64 %val to i32"
// TruncOrZextCodeGen<"%val", i32, i32>.Tout == ""
// TruncOrZextCodeGen<"%val", v4i32, v4i64>.Tout == "%val.scale = zext <4 x i32> %val to <4 x i64>"
// TruncOrZextCodeGen<"%val", v4i64, v4i32>.Tout == "%val.scale = trunc <4 x i64> %val to <4 x i32>"
// TruncOrZextCodeGen<"%val", v4i64, v4i64>.Tout == ""
class TruncOrZextCodeGen<string value_name, Type src_type, Type dst_type> {
  bit _is_noop = !eq(src_type, dst_type);
  bit _is_vector_type = !isa<VectorType>(src_type);
  assert !or(
    !and(_is_vector_type, !isa<VectorType>(dst_type), !eq(!cast<VectorType>(src_type).Len, !cast<VectorType>(dst_type).Len)),
    !and(!isa<IntType>(src_type), !isa<IntType>(dst_type))
    ), "Cannot trunc/zext " # src_type # " to " # dst_type;
  IntType _src_elem_type = !cast<IntType>(!if(_is_vector_type, !cast<VectorType>(src_type).ElementType, src_type));
  IntType _dst_elem_type = !cast<IntType>(!if(_is_vector_type, !cast<VectorType>(dst_type).ElementType, dst_type));
  int _src_type_bitwidth = _src_elem_type.BitWidth;
  int _dst_type_bitwidth = _dst_elem_type.BitWidth;
  string _op = !if(!gt(_src_type_bitwidth, _dst_type_bitwidth), "trunc", "zext");

  string ValueName = !if(_is_noop, value_name, value_name # ".scale");
  string Tout = !if(_is_noop, "",
    ValueName # " = " # _op # " " # src_type.Name # " " # value_name # " to " # dst_type.Name);
}

// Forward declarations for llvm intrinsics
let EmitMangledName = false in {
  foreach type = [
    i4, i8, i16, i32, i64, // _get_sub_group_leader
    v4i128, v8i128, v16i128, v32i128, v64i128 // sub_group_ballot_find_lsb
    ] in {
    def llvm_cttz_ # type : LLDeclare<"llvm.cttz." # type, [type, i1], type>;
  }
  foreach type = [v4i4, v8i8, v16i16, v32i32, v64i64] in { // sub_group_ballot_find_msb
    def llvm_ctlz_ # type : LLDeclare<"llvm.ctlz." # type, [type, i1], type>;
  }
}

// _get_sub_group_leader implementations
// uint _get_sub_group_leader(gentype mask)
// Semantics: get the local id of the first active item in the subgroup.
// This is achieved by finding the first non-zero mask element.
// Implementation:
// Truncate mask to <VF x i1> booleans, then bitcast to an i{VF} integer.
// Since x86 is little-endian, the element zero of <VF x i1> would be put in
// the least significant bit of i{VF}.
// So we can obtain the index by counting the trailing zeros in the i{VF}
// integer.
let EmitMangledName = true,
    FuncAttrs = ["memory(none)", "willreturn", "nounwind"] in {
  defvar builtin = "_get_sub_group_leader";
  defvar ret_type = i32;
foreach vf = [4, 8, 16, 32, 64] in {
  defvar llvm_cttz = !cast<LLDeclare>("llvm_cttz_i" # vf);
  foreach type = [i8, i16, i32, i64] in {
    defvar mask_type = !cast<VectorType>("v" # vf # type);
    foreach is_signed = [true, false] in {
      defvar mask_mangle = MangleVectorType<mask_type, is_signed>.ret;

      defvar trunc_or_zext_expr = !cond(!lt(vf, 32): "%r = zext i" # vf # " %trail.zero to i32",
                                        !eq(vf, 32): "",
                                        true: "%r = trunc i" # vf # " %trail.zero to i32");
      defvar ret_var = !if(!eq(vf, 32), "%trail.zero", "%r");
      defm builtin # vf # type # is_signed : LLDefine<builtin, [Value<mask_type, "mask">], ret_type, [{
        %to.bool = trunc {Args[0]} to <{VF} x i1>
        %to.int = bitcast <{VF} x i1> %to.bool to i{VF}
        ; count trailing zeros (x86 is little-endian)
        %trail.zero = call i{VF} @{INTRINSIC_NAME}(i{VF} %to.int, i1 true) ; don't expect all-zero mask
        {TRUNC_OR_ZEXT}
        ret i32 {RET_VAR}
      }], [Macro<"VF", !cast<string>(vf)>, Macro<"INTRINSIC_NAME", llvm_cttz.Name>, Macro<"TRUNC_OR_ZEXT", trunc_or_zext_expr>, Macro<"RET_VAR", ret_var>], mask_mangle>;
    } // foreach is_signed
  } // foreach type
} // foreach vf
}

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

// sub_group_ballot_find_lsb vector implementations
// scalar:
//   uint sub_group_ballot_find_lsb(uint4 value)
// vf=16:
//   uint16 sub_group_ballot_find_lsb(uint64 value, uint16 vec_mask)
let EmitMangledName = true,
    FuncAttrs = ["memory(none)", "willreturn", "nounwind"] in {
  defvar builtin = "sub_group_ballot_find_lsb";
foreach vf = [4, 8, 16, 32, 64] in {
  defvar ret_type = !cast<VectorType>("v" # vf # "i32");
  defvar value_type = !cast<VectorType>("v" # !mul(vf, 4) # "i32");
  defvar mask_type = ret_type;

  defvar value_mangle = MangleVectorType<value_type, /*signed*/false>.ret;
  defvar mask_mangle = MangleVectorType<mask_type, /*signed*/false>.ret;

  defm builtin # vf : LLDefine<builtin, [Value<value_type, "value">, Value<mask_type, "vec_mask">], ret_type, [{
    ; cast each uint4 to i128
    %to.int = bitcast {Args[0]} to <{VF} x i128>
    %trail.zero = call <{VF} x i128> @llvm.cttz.v{VF}i128(<{VF} x i128> %to.int, i1 true) ; don't expect zero
    %trunc = trunc <{VF} x i128> %trail.zero to <{VF} x i32>
    ret <{VF} x i32> %trunc
  }], [Macro<"VF", !cast<string>(vf)>], value_mangle # mask_mangle>;
} // foreach vf
}

// sub_group_ballot_find_msb vector implementations
// scalar:
//   uint sub_group_ballot_find_msb(uint4 value)
// vf=16:
//   uint16 sub_group_ballot_find_msb(uint64 value, uint16 vec_mask)
let EmitMangledName = true,
    FuncAttrs = ["memory(none)", "willreturn", "nounwind"] in {
  defvar builtin = "sub_group_ballot_find_msb";
foreach vf = [4, 8, 16, 32, 64] in {
  defvar ret_type = !cast<VectorType>("v" # vf # "i32");
  defvar value_type = !cast<VectorType>("v" # !mul(vf, 4) # "i32");
  defvar mask_type = ret_type;

  defvar value_mangle = MangleVectorType<value_type, /*signed*/false>.ret;
  defvar mask_mangle = MangleVectorType<mask_type, /*signed*/false>.ret;

  defvar trunc_or_zext = TruncOrZextCodeGen<"%count.zero", !cast<VectorType>("v" # vf # "i" # vf), !cast<VectorType>("v" # vf # "i32")>;
  defvar maximum_sglids = JoinIndices<!listsplat(!sub(vf, 1), vf), "i32">.Tout;
  defvar cal_offset = "%ret = sub nuw nsw <" # vf # " x i32> " # maximum_sglids # ", " # trunc_or_zext.ValueName;

  defm builtin # vf : LLDefine<builtin, [Value<value_type, "value">, Value<mask_type, "vec_mask">], ret_type, [{
    ; cast each uint4 to i128
    %to.int = bitcast {Args[0]} to <{VF} x i128>
    ; trunc i128 to match subgroup size
    %trunc = trunc <{VF} x i128> %to.int to <{VF} x i{VF}>
    %count.zero = call <{VF} x i{VF}> @llvm.ctlz.v{VF}i{VF}(<{VF} x i{VF}> %trunc, i1 true) ; don't expect zero
    {TRUNC_OR_ZEXT}
    ; subtract count.zero from max sg lid
    {CALCULATE_OFFSET}
    ret <{VF} x i32> %ret
  }], [Macro<"VF", !cast<string>(vf)>, Macro<"TRUNC_OR_ZEXT", trunc_or_zext.Tout>, Macro<"CALCULATE_OFFSET", cal_offset>], value_mangle # mask_mangle>;
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
