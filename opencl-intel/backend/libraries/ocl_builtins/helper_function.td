// 
let Types = [v1f16],
    Attrs = [OVERLOADABLE] in {
let NeedForwardDecl = 1 in {
def HalfToFloat   : OclBuiltin<"HalfToFloat",   (outs v1f32:$ret), (ins gentype:$param), [], []>;
}
}
let Types = [v8i16],
    Attrs = [OVERLOADABLE] in {
let NeedForwardDecl = 1 in {
def Half4ToFloat4   : OclBuiltin<"Half4ToFloat4",   (outs v4f32:$ret), (ins gentype:$xmm0), [], []>;
def Half8ToFloat8   : OclBuiltin<"Half8ToFloat8",   (outs v8f32:$ret), (ins gentype:$xmm0), [], []>;
}
}
let Types = [v4f32],
    Attrs = [OVERLOADABLE] in {
let NeedForwardDecl = 1 in {	
def _ocl_float2half_rte   : OclBuiltin<"_ocl_float2half_rte",   (outs v4f32:$ret), (ins gentype:$param), [], []>;
def _ocl_float2half_rtz   : OclBuiltin<"_ocl_float2half_rtz",   (outs v4f32:$ret), (ins gentype:$param), [], []>;
def _ocl_float2half_rtp   : OclBuiltin<"_ocl_float2half_rtp",   (outs v4f32:$ret), (ins gentype:$param), [], []>;
def _ocl_float2half_rtn   : OclBuiltin<"_ocl_float2half_rtn",   (outs v4f32:$ret), (ins gentype:$param), [], []>;
def _ocl_float2half   : OclBuiltin<"_ocl_float2half",   (outs v4f32:$ret), (ins gentype:$param), [], []>;
}
}
let Types = [v8f32],
    Attrs = [OVERLOADABLE] in {
let NeedForwardDecl = 1 in {
def _ocl_float2half8_rte   : OclBuiltin<"_ocl_float2half8_rte",   (outs v4f32:$ret), (ins gentype:$param), [], []>;
def _ocl_float2half8_rtz   : OclBuiltin<"_ocl_float2half8_rtz",   (outs v4f32:$ret), (ins gentype:$param), [], []>;
def _ocl_float2half8_rtn   : OclBuiltin<"_ocl_float2half8_rtn",   (outs v4f32:$ret), (ins gentype:$param), [], []>;
def _ocl_float2half8_rtp   : OclBuiltin<"_ocl_float2half8_rtp",   (outs v4f32:$ret), (ins gentype:$param), [], []>;
def _ocl_float2half8   : OclBuiltin<"_ocl_float2half8",   (outs v4f32:$ret), (ins gentype:$param), [], []>;
}
}
let Types = [v2f64],
    Attrs = [OVERLOADABLE] in {
let NeedForwardDecl = 1 in {
def _ocl_double2ToHalf2_rtp   : OclBuiltin<"_ocl_double2ToHalf2_rtp",   (outs v4f32:$ret), (ins gentype:$param), [], []>;
def _ocl_double2ToHalf2_rtn   : OclBuiltin<"_ocl_double2ToHalf2_rtn",   (outs v4f32:$ret), (ins gentype:$param), [], []>;
def _ocl_double2ToHalf2_rtz   : OclBuiltin<"_ocl_double2ToHalf2_rtz",   (outs v4f32:$ret), (ins gentype:$param), [], []>;
def _ocl_double2ToHalf2_rte   : OclBuiltin<"_ocl_double2ToHalf2_rte",   (outs v4f32:$ret), (ins gentype:$param), [], []>;
def _ocl_double2ToHalf2   : OclBuiltin<"_ocl_double2ToHalf2",   (outs v4f32:$ret), (ins gentype:$param), [], []>;
}
}
let Types = [v4f64],
    Attrs = [OVERLOADABLE] in {
let NeedForwardDecl = 1 in {
def _ocl_double4ToHalf4_rte   : OclBuiltin<"_ocl_double4ToHalf4_rte",   (outs v4f32:$ret), (ins gentype:$param), [], []>;
def _ocl_double4ToHalf4_rtp   : OclBuiltin<"_ocl_double4ToHalf4_rtp",   (outs v4f32:$ret), (ins gentype:$param), [], []>;
def _ocl_double4ToHalf4_rtn   : OclBuiltin<"_ocl_double4ToHalf4_rtn",   (outs v4f32:$ret), (ins gentype:$param), [], []>;
def _ocl_double4ToHalf4_rtz   : OclBuiltin<"_ocl_double4ToHalf4_rtz",   (outs v4f32:$ret), (ins gentype:$param), [], []>;
def _ocl_double4ToHalf4   : OclBuiltin<"_ocl_double4ToHalf4",   (outs v4f32:$ret), (ins gentype:$param), [], []>;
}
}