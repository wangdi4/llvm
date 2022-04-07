include "GENERIC/ll_generation/definition.td"

// sub_group_ballot vector implementations
// scalar:
//   uint4 sub_group_ballot(int predicate)
// vf=16:
//   uint64 sub_group_ballot(int16 predicate, uint16 vec_mask)
let EmitMangledName = true,
    FuncAttrs = ["readnone", "willreturn", "nounwind"] in {
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
