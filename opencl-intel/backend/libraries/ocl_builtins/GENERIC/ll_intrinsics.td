// vim:ts=2:sw=2:et:
// integer conversion intrinsics in LLVM IR

// sext
let Types = [ v1i8,  v2i8,  v3i8,  v4i8,  v8i8,  v16i8,
             v1i16, v2i16, v3i16, v4i16, v8i16, v16i16,
             v1i32, v2i32, v3i32, v4i32, v8i32, v16i32],
    IsDeclOnly = 1, NeedForwardDecl = 1 in {
def sext_i16 : OclBuiltin<"__ocl_sext_$Arg0Symgentype_$ReturnSymgentype", (outs i16gentype:$ret), (ins gentype:$x), [], [], 0, 0>,
               Except<[v1i16, v2i16, v3i16, v4i16, v8i16, v16i16,
                       v1i32, v2i32, v3i32, v4i32, v8i32, v16i32]>;
def sext_i32 : OclBuiltin<"__ocl_sext_$Arg0Symgentype_$ReturnSymgentype", (outs i32gentype:$ret), (ins gentype:$x), [], [], 0, 0>,
               Except<[v1i32, v2i32, v3i32, v4i32, v8i32, v16i32]>;
def sext_i64 : OclBuiltin<"__ocl_sext_$Arg0Symgentype_$ReturnSymgentype", (outs i64gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
}

// zext
let Types = [ v1i8,  v2i8,  v3i8,  v4i8,  v8i8,  v16i8,
             v1i16, v2i16, v3i16, v4i16, v8i16, v16i16,
             v1i32, v2i32, v3i32, v4i32, v8i32, v16i32],
    IsDeclOnly = 1, NeedForwardDecl = 1 in {
def zext_i16 : OclBuiltin<"__ocl_zext_$Arg0Symgentype_$ReturnSymgentype", (outs i16gentype:$ret), (ins gentype:$x), [], [], 0, 0>,
               Except<[v1i16, v2i16, v3i16, v4i16, v8i16, v16i16,
                       v1i32, v2i32, v3i32, v4i32, v8i32, v16i32]>;
def zext_i32 : OclBuiltin<"__ocl_zext_$Arg0Symgentype_$ReturnSymgentype", (outs i32gentype:$ret), (ins gentype:$x), [], [], 0, 0>,
               Except<[v1i32, v2i32, v3i32, v4i32, v8i32, v16i32]>;
def zext_i64 : OclBuiltin<"__ocl_zext_$Arg0Symgentype_$ReturnSymgentype", (outs i64gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
}

// trunc
let Types = [v1i16, v2i16, v3i16, v4i16, v8i16, v16i16,
             v1i32, v2i32, v3i32, v4i32, v8i32, v16i32,
             v1i64, v2i64, v3i64, v4i64, v8i64, v16i64],
    IsDeclOnly = 1, NeedForwardDecl = 1 in {
def trunc_i8  : OclBuiltin<"__ocl_trunc_$Arg0Symgentype_$ReturnSymgentype", (outs  i8gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def trunc_i16 : OclBuiltin<"__ocl_trunc_$Arg0Symgentype_$ReturnSymgentype", (outs i16gentype:$ret), (ins gentype:$x), [], [], 0, 0>,
                Except<[v1i16, v2i16, v3i16, v4i16, v8i16, v16i16]>;
def trunc_i32 : OclBuiltin<"__ocl_trunc_$Arg0Symgentype_$ReturnSymgentype", (outs i32gentype:$ret), (ins gentype:$x), [], [], 0, 0>,
                Except<[v1i16, v2i16, v3i16, v4i16, v8i16, v16i16,
                        v1i32, v2i32, v3i32, v4i32, v8i32, v16i32]>;
}

let Types = [v1f64, v2f64, v3f64, v4f64, v8f64, v16f64],
    IsDeclOnly = 1, NeedForwardDecl = 1 in {
def trunc_f32 : OclBuiltin<"__ocl_trunc_$Arg0Type_$ReturnType", (outs f32gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
}

let Types = [v16i32, v8i64],
    IsDeclOnly = 1, NeedForwardDecl = 1 in {
def __ocl_select : OclBuiltin<"__ocl_select_$Arg0Symgentype", (outs gentype:$ret), (ins gentype:$x, gentype:$y, gentype:$c), [], [], 0, 0>;
}

let Types = [ v1u8,  v2u8,  v4u8,  v8u8,  v16u8,
             v1u16, v2u16, v4u16, v8u16, v16u16,
             v1u32, v2u32, v4u32, v8u32, v16u32,
             v1u64, v2u64, v4u64, v8u64, v16u64],
    IsDeclOnly = 1, NeedForwardDecl = 1 in {
def helper_clz : OclBuiltin<"__ocl_helper_clz_$Arg0Symgentype", (outs gentype:$x), (ins gentype:$x), [], [], 0, 0>;
def helper_ctz : OclBuiltin<"__ocl_helper_ctz_$Arg0Symgentype", (outs gentype:$x), (ins gentype:$x), [], [], 0, 0>;
}

let Types = [v1u16], IsDeclOnly = 1, NeedForwardDecl = 1 in {
  def _ocl_mask_expand : OclBuiltin<"__ocl_expand_mask_4x16", (outs),
  (ins gentype:$in, pgentype:$out0, pgentype:$out1, pgentype:$out2, pgentype:$out3), [], [], 0, 0>;
}
