let Types = [ v1i8,  v2i8,  v3i8,  v4i8,  v8i8,  v16i8,
              v1u8,  v2u8,  v3u8,  v4u8,  v8u8,  v16u8,
             v1i16, v2i16, v3i16, v4i16, v8i16, v16i16,
             v1u16, v2u16, v3u16, v4u16, v8u16, v16u16,
             v1i32, v2i32, v3i32, v4i32, v8i32, v16i32,
             v1u32, v2u32, v3u32, v4u32, v8u32, v16u32,
             v1i64, v2i64, v3i64, v4i64, v8i64, v16i64,
             v1u64, v2u64, v3u64, v4u64, v8u64, v16u64],
    Attrs = [CONSTFUNC, OVERLOADABLE] in {
let NeedForwardDecl = 1 in {
def ctz       : OclBuiltin<"ctz",       (outs gentype:$ret), (ins gentype:$x), [], []>;
}
}

// native relaxed math built-ins OpenCL 2.0 rev 08 Table 7.2, single-precision only
let Types = [v1f32, v2f32, v3f32, v4f32, v8f32, v16f32],
    Attrs = [CONSTFUNC, OVERLOADABLE],
    NeedForwardDecl = 1 in {
def native_cos_rm    : OclBuiltin<"native_cos_rm",    (outs gentype:$ret), (ins gentype:$x), [], []>;
//def native_divide_rm : OclBuiltin<"native_divide_rm", (outs gentype:$ret), (ins gentype:$x, gentype:$y), [], []>;
def native_exp_rm    : OclBuiltin<"native_exp_rm",    (outs gentype:$ret), (ins gentype:$x), [], []>;
def native_exp2_rm   : OclBuiltin<"native_exp2_rm",   (outs gentype:$ret), (ins gentype:$x), [], []>;
def native_exp10_rm  : OclBuiltin<"native_exp10_rm",  (outs gentype:$ret), (ins gentype:$x), [], []>;
def native_log_rm    : OclBuiltin<"native_log_rm",    (outs gentype:$ret), (ins gentype:$x), [], []>;
def native_log2_rm   : OclBuiltin<"native_log2_rm",   (outs gentype:$ret), (ins gentype:$x), [], []>;
def native_pow_rm   : OclBuiltin<"native_pow_rm",   (outs gentype:$ret), (ins gentype:$x, gentype:$y), [], []>;
def native_sin_rm    : OclBuiltin<"native_sin_rm",    (outs gentype:$ret), (ins gentype:$x), [], []>;
def native_tan_rm    : OclBuiltin<"native_tan_rm",    (outs gentype:$ret), (ins gentype:$x), [], []>;
def native_sincos_pas_rm    : OclBuiltin<"native_sincos_rm",    (outs gentype:$ret), (ins gentype:$x, pgentype:$cosval), [], []>, ExceptAttr<[CONSTFUNC]>, PRIVATE;
def native_sincos_las_rm    : OclBuiltin<"native_sincos_rm",    (outs gentype:$ret), (ins gentype:$x, pgentype:$cosval), [], []>, ExceptAttr<[CONSTFUNC]>, LOCAL;
def native_sincos_gas_rm    : OclBuiltin<"native_sincos_rm",    (outs gentype:$ret), (ins gentype:$x, pgentype:$cosval), [], []>, ExceptAttr<[CONSTFUNC]>, GLOBAL;
}