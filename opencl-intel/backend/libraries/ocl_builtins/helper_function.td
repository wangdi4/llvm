// 
let Types = [v1f16],
    Attrs = [INTERNAL_INLINE, OVERLOADABLE] in {
let NeedForwardDecl = 1 in {
def __ocl_HalfToFloat   : OclBuiltin<"__ocl_HalfToFloat",   (outs v1f32:$ret), (ins gentype:$param), [], []>;
}
}
let Types = [v8i16],
    Attrs = [INTERNAL_INLINE, OVERLOADABLE] in {
let NeedForwardDecl = 1 in {
def __ocl_Half4ToFloat4   : OclBuiltin<"__ocl_Half4ToFloat4",   (outs v4f32:$ret), (ins gentype:$xmm0), [], []>;
def __ocl_Half8ToFloat8   : OclBuiltin<"__ocl_Half8ToFloat8",   (outs v8f32:$ret), (ins gentype:$xmm0), [], []>;
}
}
let Types = [v4f32],
    Attrs = [INTERNAL_INLINE, OVERLOADABLE] in {
let NeedForwardDecl = 1 in {	
def __ocl_float2half_rte   : OclBuiltin<"__ocl_float2half_rte",   (outs v4f32:$ret), (ins gentype:$param), [], []>;
def __ocl_float2half_rtz   : OclBuiltin<"__ocl_float2half_rtz",   (outs v4f32:$ret), (ins gentype:$param), [], []>;
def __ocl_float2half_rtp   : OclBuiltin<"__ocl_float2half_rtp",   (outs v4f32:$ret), (ins gentype:$param), [], []>;
def __ocl_float2half_rtn   : OclBuiltin<"__ocl_float2half_rtn",   (outs v4f32:$ret), (ins gentype:$param), [], []>;
def __ocl_float2half   : OclBuiltin<"__ocl_float2half",   (outs v4f32:$ret), (ins gentype:$param), [], []>;
}
}
let Types = [v8f32],
    Attrs = [INTERNAL_INLINE, OVERLOADABLE] in {
let NeedForwardDecl = 1 in {
def __ocl_float2half8_rte   : OclBuiltin<"__ocl_float2half8_rte",   (outs v4f32:$ret), (ins gentype:$param), [], []>;
def __ocl_float2half8_rtz   : OclBuiltin<"__ocl_float2half8_rtz",   (outs v4f32:$ret), (ins gentype:$param), [], []>;
def __ocl_float2half8_rtn   : OclBuiltin<"__ocl_float2half8_rtn",   (outs v4f32:$ret), (ins gentype:$param), [], []>;
def __ocl_float2half8_rtp   : OclBuiltin<"__ocl_float2half8_rtp",   (outs v4f32:$ret), (ins gentype:$param), [], []>;
def __ocl_float2half8   : OclBuiltin<"__ocl_float2half8",   (outs v4f32:$ret), (ins gentype:$param), [], []>;
}
}
let Types = [v2f64],
    Attrs = [INTERNAL_INLINE, OVERLOADABLE] in {
let NeedForwardDecl = 1 in {
def __ocl_double2ToHalf2_rtp   : OclBuiltin<"__ocl_double2ToHalf2_rtp",   (outs v4f32:$ret), (ins gentype:$param), [], []>;
def __ocl_double2ToHalf2_rtn   : OclBuiltin<"__ocl_double2ToHalf2_rtn",   (outs v4f32:$ret), (ins gentype:$param), [], []>;
def __ocl_double2ToHalf2_rtz   : OclBuiltin<"__ocl_double2ToHalf2_rtz",   (outs v4f32:$ret), (ins gentype:$param), [], []>;
def __ocl_double2ToHalf2_rte   : OclBuiltin<"__ocl_double2ToHalf2_rte",   (outs v4f32:$ret), (ins gentype:$param), [], []>;
def __ocl_double2ToHalf2   : OclBuiltin<"__ocl_double2ToHalf2",   (outs v4f32:$ret), (ins gentype:$param), [], []>;
}
}
let Types = [v4f64],
    Attrs = [INTERNAL_INLINE, OVERLOADABLE] in {
let NeedForwardDecl = 1 in {
def __ocl_double4ToHalf4_rte   : OclBuiltin<"__ocl_double4ToHalf4_rte",   (outs v4f32:$ret), (ins gentype:$param), [], []>;
def __ocl_double4ToHalf4_rtp   : OclBuiltin<"__ocl_double4ToHalf4_rtp",   (outs v4f32:$ret), (ins gentype:$param), [], []>;
def __ocl_double4ToHalf4_rtn   : OclBuiltin<"__ocl_double4ToHalf4_rtn",   (outs v4f32:$ret), (ins gentype:$param), [], []>;
def __ocl_double4ToHalf4_rtz   : OclBuiltin<"__ocl_double4ToHalf4_rtz",   (outs v4f32:$ret), (ins gentype:$param), [], []>;
def __ocl_double4ToHalf4   : OclBuiltin<"__ocl_double4ToHalf4",   (outs v4f32:$ret), (ins gentype:$param), [], []>;
}
}