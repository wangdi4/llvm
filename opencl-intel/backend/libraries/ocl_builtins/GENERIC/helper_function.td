//
let Types = [v1i16],
    Attrs = [INTERNAL_INLINE, OVERLOADABLE],
    NeedForwardDecl = 1 in {
  def __ocl_HalfToFloat   : OclBuiltin<"__ocl_HalfToFloat",   (outs v1f32:$ret), (ins gentype:$param), [], [], 0, 0>;
}

let Types = [v8i16],
    Attrs = [INTERNAL_INLINE, OVERLOADABLE],
    NeedForwardDecl = 1 in {
  def __ocl_Half4ToFloat4   : OclBuiltin<"__ocl_Half4ToFloat4",   (outs v4f32:$ret), (ins gentype:$xmm0), [], [], 0, 0>;
  def __ocl_Half8ToFloat8   : OclBuiltin<"__ocl_Half8ToFloat8",   (outs v8f32:$ret), (ins gentype:$xmm0), [], [], 0, 0>;
}

let Types = [v16i16],
    Attrs = [INTERNAL_INLINE, OVERLOADABLE],
    NeedForwardDecl = 1 in {
  def __ocl_Half16ToFloat16   : OclBuiltin<"__ocl_Half16ToFloat16",   (outs v16f32:$ret), (ins gentype:$xmm0), [], [], 0, 0>;
}

let Types = [v4f32],
    Attrs = [INTERNAL_INLINE, OVERLOADABLE],
    NeedForwardDecl = 1 in {
  def __ocl_float2half_rte   : OclBuiltin<"__ocl_float2half_rte",   (outs v4f32:$ret), (ins gentype:$param), [], [], 0, 0>;
  def __ocl_float2half_rtz   : OclBuiltin<"__ocl_float2half_rtz",   (outs v4f32:$ret), (ins gentype:$param), [], [], 0, 0>;
  def __ocl_float2half_rtp   : OclBuiltin<"__ocl_float2half_rtp",   (outs v4f32:$ret), (ins gentype:$param), [], [], 0, 0>;
  def __ocl_float2half_rtn   : OclBuiltin<"__ocl_float2half_rtn",   (outs v4f32:$ret), (ins gentype:$param), [], [], 0, 0>;
  def __ocl_float2half   : OclBuiltin<"__ocl_float2half",   (outs v4f32:$ret), (ins gentype:$param), [], [], 0, 0>;
}

let Types = [v8f32],
    Attrs = [INTERNAL_INLINE, OVERLOADABLE],
    NeedForwardDecl = 1 in {
  def __ocl_float2half8_rte   : OclBuiltin<"__ocl_float2half8_rte",   (outs v4f32:$ret), (ins gentype:$param), [], [], 0, 0>;
  def __ocl_float2half8_rtz   : OclBuiltin<"__ocl_float2half8_rtz",   (outs v4f32:$ret), (ins gentype:$param), [], [], 0, 0>;
  def __ocl_float2half8_rtn   : OclBuiltin<"__ocl_float2half8_rtn",   (outs v4f32:$ret), (ins gentype:$param), [], [], 0, 0>;
  def __ocl_float2half8_rtp   : OclBuiltin<"__ocl_float2half8_rtp",   (outs v4f32:$ret), (ins gentype:$param), [], [], 0, 0>;
  def __ocl_float2half8   : OclBuiltin<"__ocl_float2half8",   (outs v4f32:$ret), (ins gentype:$param), [], [], 0, 0>;
}

let Types = [v16f32],
    Attrs = [INTERNAL_INLINE, OVERLOADABLE],
    NeedForwardDecl = 1 in {
  def __ocl_float2half16_rte   : OclBuiltin<"__ocl_float2half16_rte",   (outs v8f32:$ret), (ins gentype:$param), [], [], 0, 0>;
  def __ocl_float2half16_rtz   : OclBuiltin<"__ocl_float2half16_rtz",   (outs v8f32:$ret), (ins gentype:$param), [], [], 0, 0>;
  def __ocl_float2half16_rtn   : OclBuiltin<"__ocl_float2half16_rtn",   (outs v8f32:$ret), (ins gentype:$param), [], [], 0, 0>;
  def __ocl_float2half16_rtp   : OclBuiltin<"__ocl_float2half16_rtp",   (outs v8f32:$ret), (ins gentype:$param), [], [], 0, 0>;
  def __ocl_float2half16       : OclBuiltin<"__ocl_float2half16",       (outs v8f32:$ret), (ins gentype:$param), [], [], 0, 0>;
}

let Types = [v2f64],
    Attrs = [INTERNAL_INLINE, OVERLOADABLE],
    NeedForwardDecl = 1 in {
  def __ocl_double2ToHalf2_rtp   : OclBuiltin<"__ocl_double2ToHalf2_rtp",   (outs v4f32:$ret), (ins gentype:$param), [], [], 0, 0>;
  def __ocl_double2ToHalf2_rtn   : OclBuiltin<"__ocl_double2ToHalf2_rtn",   (outs v4f32:$ret), (ins gentype:$param), [], [], 0, 0>;
  def __ocl_double2ToHalf2_rtz   : OclBuiltin<"__ocl_double2ToHalf2_rtz",   (outs v4f32:$ret), (ins gentype:$param), [], [], 0, 0>;
  def __ocl_double2ToHalf2_rte   : OclBuiltin<"__ocl_double2ToHalf2_rte",   (outs v4f32:$ret), (ins gentype:$param), [], [], 0, 0>;
  def __ocl_double2ToHalf2   : OclBuiltin<"__ocl_double2ToHalf2",   (outs v4f32:$ret), (ins gentype:$param), [], [], 0, 0>;
}

let Types = [v4f64],
    Attrs = [INTERNAL_INLINE, OVERLOADABLE],
    NeedForwardDecl = 1 in {
  def __ocl_double4ToHalf4_rte   : OclBuiltin<"__ocl_double4ToHalf4_rte",   (outs v4f32:$ret), (ins gentype:$param), [], [], 0, 0>;
  def __ocl_double4ToHalf4_rtp   : OclBuiltin<"__ocl_double4ToHalf4_rtp",   (outs v4f32:$ret), (ins gentype:$param), [], [], 0, 0>;
  def __ocl_double4ToHalf4_rtn   : OclBuiltin<"__ocl_double4ToHalf4_rtn",   (outs v4f32:$ret), (ins gentype:$param), [], [], 0, 0>;
  def __ocl_double4ToHalf4_rtz   : OclBuiltin<"__ocl_double4ToHalf4_rtz",   (outs v4f32:$ret), (ins gentype:$param), [], [], 0, 0>;
  def __ocl_double4ToHalf4   : OclBuiltin<"__ocl_double4ToHalf4",   (outs v4f32:$ret), (ins gentype:$param), [], [], 0, 0>;
}

let Types = [v8f64],
    Attrs = [INTERNAL_INLINE, OVERLOADABLE],
    NeedForwardDecl = 1 in {
  def __ocl_double8ToHalf8_rte   : OclBuiltin<"__ocl_double8ToHalf8_rte",   (outs v4f32:$ret), (ins gentype:$param), [], [], 0, 0>;
  def __ocl_double8ToHalf8_rtp   : OclBuiltin<"__ocl_double8ToHalf8_rtp",   (outs v4f32:$ret), (ins gentype:$param), [], [], 0, 0>;
  def __ocl_double8ToHalf8_rtn   : OclBuiltin<"__ocl_double8ToHalf8_rtn",   (outs v4f32:$ret), (ins gentype:$param), [], [], 0, 0>;
  def __ocl_double8ToHalf8_rtz   : OclBuiltin<"__ocl_double8ToHalf8_rtz",   (outs v4f32:$ret), (ins gentype:$param), [], [], 0, 0>;
  def __ocl_double8ToHalf8   : OclBuiltin<"__ocl_double8ToHalf8",   (outs v4f32:$ret), (ins gentype:$param), [], [], 0, 0>;
}
// ****************************************************************************
//                                 transpose
// ****************************************************************************

let Types = [v4i32, v4f32],
    Attrs = [INTERNAL_INLINE, OVERLOADABLE],
    NeedForwardDecl = 1 in {
  def __ocl_load_transpose4x4_avx2_helper   : OclBuiltin<"__ocl_load_transpose4x4_avx2_helper", (outs), (ins v8gentype:$In0, v8gentype:$In1, pgentype:$Out0, pgentype:$Out1, pgentype:$Out2, pgentype:$Out3), [], [], 0, 0>, PRIVATE;
  def __ocl_transpose_store4x4_avx2_helper  : OclBuiltin<"__ocl_transpose_store4x4_avx2_helper", (outs), (ins v8gentype:$In0, v8gentype:$In1, pv8gentype:$Out0, pv8gentype:$Out1), [], [], 0, 0>, PRIVATE;

  def __ocl_common_transpose4x8_avx2_helper  : OclBuiltin<"__ocl_common_transpose4x8_avx2_helper", (outs), (ins v8gentype:$In0, v8gentype:$In1, v8gentype:$In2, v8gentype:$In3, pv8gentype:$Out0, pv8gentype:$Out1, pv8gentype:$Out2, pv8gentype:$Out3),  [], [], 0, 0>, PRIVATE;
}

let Types = [v4i8, v4i16],
    Attrs = [INTERNAL_INLINE, OVERLOADABLE],
    NeedForwardDecl = 1 in {
  def __ocl_load_transpose4x4_helper   : OclBuiltin<"__ocl_load_transpose4x4_helper", (outs), (ins v16gentype:$In, pgentype:$Out0, pgentype:$Out1, pgentype:$Out2, pgentype:$Out3), [], [], 0, 0>, PRIVATE;
  def __ocl_transpose_store4x4_helper  : OclBuiltin<"__ocl_transpose_store4x4_helper", (outs), (ins pv16gentype:$Out, gentype:$In0, gentype:$In2, gentype:$In3, gentype:$In4), [], [], 0, 0>, PRIVATE;

  def __ocl_load_transpose4x8_helper   : OclBuiltin<"__ocl_load_transpose4x8_helper", (outs), (ins v16gentype:$In0, v16gentype:$In1, pv8gentype:$Out0, pv8gentype:$Out1, pv8gentype:$Out2, pv8gentype:$Out3), [], [], 0, 0>, PRIVATE;
  def __ocl_transpose_store4x8_helper  : OclBuiltin<"__ocl_transpose_store4x8_helper", (outs), (ins pv16gentype:$Out0, pv16gentype:$Out1, v8gentype:$In0, v8gentype:$In1, v8gentype:$In2, v8gentype:$In3), [], [], 0, 0>, PRIVATE;
}

let Types = [v4f32, v4i32, v4i8, v4i16],
    Attrs = [INTERNAL_INLINE, OVERLOADABLE],
    NeedForwardDecl = 1 in {
  def __ocl_transpose4x4  : OclBuiltin<"__ocl_transpose4x4", (outs), (ins gentype:$xyzw0, gentype:$xyzw1, gentype:$xyzw2, gentype:$xyzw3, pgentype:$xOut, pgentype:$yOut, pgentype:$zOut, pgentype:$wOut ), [], [], 0, 0>, PRIVATE;
  def __ocl_transpose4x8  : OclBuiltin<"__ocl_transpose4x8", (outs), (ins gentype:$xyzw0, gentype:$xyzw1, gentype:$xyzw2, gentype:$xyzw3, gentype:$xyzw4, gentype:$xyzw5, gentype:$xyzw6, gentype:$xyzw7, pv8gentype:$xOut, pv8gentype:$yOut, pv8gentype:$zOut, pv8gentype:$wOut  ),  [], [], 0, 0>, PRIVATE;
  def __ocl_transpose8x4  : OclBuiltin<"__ocl_transpose8x4", (outs), (ins v8gentype:$xOut, v8gentype:$yOut, v8gentype:$zOut, v8gentype:$wOut, pgentype:$xyzw0, pgentype:$xyzw1, pgentype:$xyzw2, pgentype:$xyzw3, pgentype:$xyzw4, pgentype:$xyzw5, pgentype:$xyzw6, pgentype:$xyzw7 ), [], [], 0, 0>, PRIVATE;
}

let Types = [v4i8, v4i16],
    Attrs = [INTERNAL_INLINE, OVERLOADABLE],
    NeedForwardDecl = 1 in {
  def __ocl_transpose4x16  : OclBuiltin<"__ocl_transpose4x16", (outs), (ins gentype:$xyzw0, gentype:$xyzw1, gentype:$xyzw2, gentype:$xyzw3, gentype:$xyzw4, gentype:$xyzw5, gentype:$xyzw6, gentype:$xyzw7, gentype:$xyzw8, gentype:$xyzw9, gentype:$xyzw10, gentype:$xyzw11, gentype:$xyzw12, gentype:$xyzw13, gentype:$xyzw14, gentype:$xyzw15, pv16gentype:$xOut, pv16gentype:$yOut, pv16gentype:$zOut, pv16gentype:$wOut  ),  [], [], 0, 0>, PRIVATE;
  def __ocl_transpose16x4  : OclBuiltin<"__ocl_transpose16x4", (outs), (ins v16gentype:$xOut, v16gentype:$yOut, v16gentype:$zOut, v16gentype:$wOut, pgentype:$xyzw0, pgentype:$xyzw1, pgentype:$xyzw2, pgentype:$xyzw3, pgentype:$xyzw4, pgentype:$xyzw5, pgentype:$xyzw6, pgentype:$xyzw7, pgentype:$xyzw8, pgentype:$xyzw9, pgentype:$xyzw10, pgentype:$xyzw11, pgentype:$xyzw12, pgentype:$xyzw13, pgentype:$xyzw14, pgentype:$xyzw15 ), [], [], 0, 0>, PRIVATE;
}

let Types = [v2i32, v2f32],
    Attrs = [INTERNAL_INLINE, OVERLOADABLE],
    NeedForwardDecl = 1 in {
  def __ocl_transpose2x8  : OclBuiltin<"__ocl_transpose2x8", (outs), (ins gentype:$xyzw0, gentype:$xyzw1, gentype:$xyzw2, gentype:$xyzw3, gentype:$xyzw4, gentype:$xyzw5, gentype:$xyzw6, gentype:$xyzw7, pv8gentype:$xOut, pv8gentype:$yOut ),  [], [], 0, 0>, PRIVATE;
}

let Types = [v4i8, v4i32, v4f32],
    Attrs = [INLINE] in {
  let NeedForwardDecl = 1 in {
    def __ocl_load_transpose4x4            : OclBuiltin<"__ocl_load_transpose_$Arg0BaseType_4x4",           (outs), (ins pgentype:$pLoadAdd, pgentype:$xOut, pgentype:$yOut, pgentype:$zOut, pgentype:$wOut), [], [], 0, 0>, PRIVATE;
    def __ocl_transpose_store4x4           : OclBuiltin<"__ocl_transpose_store_$Arg0BaseType_4x4",          (outs), (ins pgentype:$pStoreAdd, gentype:$xIn, gentype:$yIn, gentype:$zIn, gentype:$wIn ), [], [], 0, 0>, PRIVATE;
    def __ocl_gather_transpose4x4          : OclBuiltin<"__ocl_gather_transpose_$Arg0BaseType_4x4",         (outs), (ins pgentype:$pLoadAdd0, pgentype:$pLoadAdd1, pgentype:$pLoadAdd2, pgentype:$pLoadAdd3, pgentype:$xOut, pgentype:$yOut, pgentype:$zOut, pgentype:$wOut ), [], [], 0, 0>, PRIVATE;
    def __ocl_transpose_scatter4x4         : OclBuiltin<"__ocl_transpose_scatter_$Arg0BaseType_4x4",        (outs), (ins pgentype:$pStoreAdd0, pgentype:$pStoreAdd1, pgentype:$pStoreAdd2, pgentype:$pStoreAdd3, gentype:$xIn, gentype:$yIn, gentype:$zIn, gentype:$wIn ), [], [], 0, 0>, PRIVATE;
  }
  def __ocl_masked_load_transpose4x4     : OclBuiltin<"__ocl_masked_load_transpose_$Arg0BaseType_4x4",    (outs), (ins pgentype:$pLoadAdd, pgentype:$xOut, pgentype:$yOut, pgentype:$zOut, pgentype:$wOut, v4i32:$mask ), [], [], 0, 0>, PRIVATE;
  def __ocl_masked_transpose_store4x4    : OclBuiltin<"__ocl_masked_transpose_store_$Arg0BaseType_4x4",   (outs), (ins pgentype:$pStoreAdd, gentype:$xIn, gentype:$yIn, gentype:$zIn, gentype:$wIn, v4i32:$mask ), [], [], 0, 0>, PRIVATE;
  def __ocl_masked_gather_transpose4x4   : OclBuiltin<"__ocl_masked_gather_transpose_$Arg0BaseType_4x4",  (outs), (ins pgentype:$pLoadAdd0, pgentype:$pLoadAdd1, pgentype:$pLoadAdd2, pgentype:$pLoadAdd3, pgentype:$xOut, pgentype:$yOut, pgentype:$zOut, pgentype:$wOut, v4i32:$mask ), [], [], 0, 0>, PRIVATE;
  def __ocl_masked_transpose_scatter4x4  : OclBuiltin<"__ocl_masked_transpose_scatter_$Arg0BaseType_4x4", (outs), (ins pgentype:$pStoreAdd0, pgentype:$pStoreAdd1, pgentype:$pStoreAdd2, pgentype:$pStoreAdd3, gentype:$xIn, gentype:$yIn, gentype:$zIn, gentype:$wIn, v4i32:$mask ), [], [], 0, 0>, PRIVATE;
}


let Types = [v4i8, v4i16, v4i32, v4f32],
    Attrs = [INLINE] in {
  let NeedForwardDecl = 1 in {
    def __ocl_load_transpose4x8            : OclBuiltin<"__ocl_load_transpose_$Arg0BaseType_4x8",           (outs), (ins pgentype:$pLoadAdd, pv8gentype:$xOut, pv8gentype:$yOut, pv8gentype:$zOut, pv8gentype:$wOut), [], [], 0, 0>, PRIVATE;
    def __ocl_transpose_store4x8           : OclBuiltin<"__ocl_transpose_store_$Arg0BaseType_4x8",          (outs), (ins pgentype:$pStoreAdd, v8gentype:$xIn, v8gentype:$yIn, v8gentype:$zIn, v8gentype:$wIn ), [], [], 0, 0>, PRIVATE;
    def __ocl_gather_transpose4x8          : OclBuiltin<"__ocl_gather_transpose_$Arg0BaseType_4x8",         (outs), (ins pgentype:$pLoadAdd0, pgentype:$pLoadAdd1, pgentype:$pLoadAdd2, pgentype:$pLoadAdd3, pgentype:$pLoadAdd4, pgentype:$pLoadAdd5, pgentype:$pLoadAdd6, pgentype:$pLoadAdd7, pv8gentype:$xOut, pv8gentype:$yOut, pv8gentype:$zOut, pv8gentype:$wOut), [], [], 0, 0>, PRIVATE;
    def __ocl_transpose_scatter4x8         : OclBuiltin<"__ocl_transpose_scatter_$Arg0BaseType_4x8",        (outs), (ins pgentype:$pStoreAdd0, pgentype:$pStoreAdd1, pgentype:$pStoreAdd2, pgentype:$pStoreAdd3, pgentype:$pStoreAdd4, pgentype:$pStoreAdd5, pgentype:$pStoreAdd6, pgentype:$pStoreAdd7, v8gentype:$xIn, v8gentype:$yIn, v8gentype:$zIn, v8gentype:$wIn), [], [], 0, 0>, PRIVATE;
  }
  def __ocl_masked_load_transpose4x8     : OclBuiltin<"__ocl_masked_load_transpose_$Arg0BaseType_4x8",    (outs), (ins pgentype:$pLoadAdd, pv8gentype:$xOut, pv8gentype:$yOut, pv8gentype:$zOut, pv8gentype:$wOut, v8i32:$mask), [], [], 0, 0>, PRIVATE;
  def __ocl_masked_transpose_store4x8    : OclBuiltin<"__ocl_masked_transpose_store_$Arg0BaseType_4x8",   (outs), (ins pgentype:$pStoreAdd, v8gentype:$xIn, v8gentype:$yIn, v8gentype:$zIn, v8gentype:$wIn, v8i32:$mask), [], [], 0, 0>, PRIVATE;
  def __ocl_masked_gather_transpose4x8   : OclBuiltin<"__ocl_masked_gather_transpose_$Arg0BaseType_4x8",  (outs), (ins pgentype:$pLoadAdd0, pgentype:$pLoadAdd1, pgentype:$pLoadAdd2, pgentype:$pLoadAdd3, pgentype:$pLoadAdd4, pgentype:$pLoadAdd5, pgentype:$pLoadAdd6, pgentype:$pLoadAdd7, pv8gentype:$xOut, pv8gentype:$yOut, pv8gentype:$zOut, pv8gentype:$wOut, v8i32:$mask), [], [], 0, 0>, PRIVATE;
  def __ocl_masked_transpose_scatter4x8  : OclBuiltin<"__ocl_masked_transpose_scatter_$Arg0BaseType_4x8", (outs), (ins pgentype:$pStoreAdd0, pgentype:$pStoreAdd1, pgentype:$pStoreAdd2, pgentype:$pStoreAdd3, pgentype:$pStoreAdd4, pgentype:$pStoreAdd5, pgentype:$pStoreAdd6, pgentype:$pStoreAdd7, v8gentype:$xIn, v8gentype:$yIn, v8gentype:$zIn, v8gentype:$wIn, v8i32:$mask ), [], [], 0, 0>, PRIVATE;
}

let Types = [v4i8, v4i16, v2i32, v2f32],
    Attrs = [INLINE] in {
  let NeedForwardDecl = 1 in {
    def __ocl_load_transpose2x8            : OclBuiltin<"__ocl_load_transpose_$Arg0BaseType_2x8",           (outs), (ins pgentype:$pLoadAdd, pv8gentype:$xOut, pv8gentype:$yOut), [], [], 0, 0>, PRIVATE;
  }
  def __ocl_masked_load_transpose2x8     : OclBuiltin<"__ocl_masked_load_transpose_$Arg0BaseType_2x8",    (outs), (ins pgentype:$pLoadAdd, pv8gentype:$xOut, pv8gentype:$yOut, v8i32:$mask), [], [], 0, 0>, PRIVATE;
}

let Types = [v4i8, v4i16, v4i32, v4f32],
    Attrs = [INLINE] in {
  let NeedForwardDecl = 1 in {
    def __ocl_load_transpose4x16           : OclBuiltin<"__ocl_load_transpose_$Arg0BaseType_4x16",          (outs), (ins pgentype:$pLoadAdd, pv16gentype:$xOut, pv16gentype:$yOut, pv16gentype:$zOut, pv16gentype:$wOut), [], [], 0, 0>, PRIVATE;
    def __ocl_gather_transpose4x16         : OclBuiltin<"__ocl_gather_transpose_$Arg0BaseType_4x16",        (outs), (ins pgentype:$pLoadAdd0, pgentype:$pLoadAdd1, pgentype:$pLoadAdd2, pgentype:$pLoadAdd3, pgentype:$pLoadAdd4, pgentype:$pLoadAdd5, pgentype:$pLoadAdd6, pgentype:$pLoadAdd7, pgentype:$pLoadAdd8, pgentype:$pLoadAdd9, pgentype:$pLoadAdd10, pgentype:$pLoadAdd11, pgentype:$pLoadAdd12, pgentype:$pLoadAdd13, pgentype:$pLoadAdd14, pgentype:$pLoadAdd15, pv16gentype:$xOut, pv16gentype:$yOut, pv16gentype:$zOut, pv16gentype:$wOut), [], [], 0, 0>, PRIVATE;
    def __ocl_masked_load_transpose4x16    : OclBuiltin<"__ocl_masked_load_transpose_$Arg0BaseType_4x16",   (outs), (ins pgentype:$pLoadAdd, pv16gentype:$xOut, pv16gentype:$yOut, pv16gentype:$zOut, pv16gentype:$wOut, v1u16:$mask), [], [], 0, 0>, PRIVATE;
    def __ocl_transpose_scatter4x16        : OclBuiltin<"__ocl_transpose_scatter_$Arg0BaseType_4x16",       (outs), (ins pgentype:$pStoreAdd0, pgentype:$pStoreAdd1, pgentype:$pStoreAdd2, pgentype:$pStoreAdd3, pgentype:$pStoreAdd4, pgentype:$pStoreAdd5, pgentype:$pStoreAdd6, pgentype:$pStoreAdd7, pgentype:$pStoreAdd8, pgentype:$pStoreAdd9, pgentype:$pStoreAdd10, pgentype:$pStoreAdd11, pgentype:$pStoreAdd12, pgentype:$pStoreAdd13, pgentype:$pStoreAdd14, pgentype:$pStoreAdd15, v16gentype:$xIn, v16gentype:$yIn, v16gentype:$zIn, v16gentype:$wIn), [], [], 0, 0>, PRIVATE;
  }
  def __ocl_masked_gather_transpose4x16  : OclBuiltin<"__ocl_masked_gather_transpose_$Arg0BaseType_4x16",  (outs), (ins pgentype:$pLoadAdd0, pgentype:$pLoadAdd1, pgentype:$pLoadAdd2, pgentype:$pLoadAdd3, pgentype:$pLoadAdd4, pgentype:$pLoadAdd5, pgentype:$pLoadAdd6, pgentype:$pLoadAdd7, pgentype:$pLoadAdd8, pgentype:$pLoadAdd9, pgentype:$pLoadAdd10, pgentype:$pLoadAdd11, pgentype:$pLoadAdd12, pgentype:$pLoadAdd13, pgentype:$pLoadAdd14, pgentype:$pLoadAdd15, pv16gentype:$xOut, pv16gentype:$yOut, pv16gentype:$zOut, pv16gentype:$wOut, v16i32:$mask), [], [], 0, 0>, PRIVATE;
  def __ocl_masked_transpose_scatter4x16 : OclBuiltin<"__ocl_masked_transpose_scatter_$Arg0BaseType_4x16", (outs), (ins pgentype:$pStoreAdd0, pgentype:$pStoreAdd1, pgentype:$pStoreAdd2, pgentype:$pStoreAdd3, pgentype:$pStoreAdd4, pgentype:$pStoreAdd5, pgentype:$pStoreAdd6, pgentype:$pStoreAdd7, pgentype:$pStoreAdd8, pgentype:$pStoreAdd9, pgentype:$pStoreAdd10, pgentype:$pStoreAdd11, pgentype:$pStoreAdd12, pgentype:$pStoreAdd13, pgentype:$pStoreAdd14, pgentype:$pStoreAdd15, v16gentype:$xIn, v16gentype:$yIn, v16gentype:$zIn, v16gentype:$wIn, v16i32:$mask), [], [], 0, 0>, PRIVATE;
}

let Attrs = [OVERLOADABLE],
  NeedForwardDecl = 1 in {
    let Types = [v32i64, v32u64,
                 v32i32, v32u32, v32f32,
                 v32i16, v32u16,
                 v32i8,  v32u8] in {
      def __ocl_shuffle_transpose_8x4 : OclBuiltin<"__ocl_shuffle_transpose_$Arg0BaseType_8x4", (outs gentype:$ret), (ins gentype:$src), [], [], 0, 0>, PRIVATE;
      def __ocl_shuffle_transpose_4x8 : OclBuiltin<"__ocl_shuffle_transpose_$Arg0BaseType_4x8", (outs gentype:$ret), (ins gentype:$src), [], [], 0, 0>, PRIVATE;
    }

    let Types = [v64i64, v64u64,
                 v64i32, v64u32, v64f32,
                 v64i16, v64u16,
                 v64i8,  v64u8] in {
      def __ocl_shuffle_transpose_4x16 : OclBuiltin<"__ocl_shuffle_transpose_$Arg0BaseType_4x16", (outs gentype:$ret), (ins gentype:$src), [], [], 0, 0>, PRIVATE;
      def __ocl_shuffle_transpose_16x4 : OclBuiltin<"__ocl_shuffle_transpose_$Arg0BaseType_16x4", (outs gentype:$ret), (ins gentype:$src), [], [], 0, 0>, PRIVATE;
      def __ocl_shuffle_transpose_8x8  : OclBuiltin<"__ocl_shuffle_transpose_$Arg0BaseType_8x8",  (outs gentype:$ret), (ins gentype:$src), [], [], 0, 0>, PRIVATE;
    }

    let Types = [v128i64, v128u64,
                 v128i32, v128u32, v128f32,
                 v128i16, v128u16,
                 v128i8,  v128u8] in {
      def __ocl_shuffle_transpose_8x16 : OclBuiltin<"__ocl_shuffle_transpose_$Arg0BaseType_8x16", (outs gentype:$ret), (ins gentype:$src), [], [], 0, 0>, PRIVATE;
      def __ocl_shuffle_transpose_16x8 : OclBuiltin<"__ocl_shuffle_transpose_$Arg0BaseType_16x8", (outs gentype:$ret), (ins gentype:$src), [], [], 0, 0>, PRIVATE;
    }

    let Types = [v256i64, v256u64,
                 v256i32, v256u32, v256f32,
                 v256i16, v256u16,
                 v256i8,  v256u8] in {
      def __ocl_shuffle_transpose_16x16 : OclBuiltin<"__ocl_shuffle_transpose_$Arg0BaseType_16x16", (outs gentype:$ret), (ins gentype:$src), [], [], 0, 0>, PRIVATE;
    }
  }

let Attrs = [OVERLOADABLE],
  NeedForwardDecl = 1 in {
    let Types = [v16i32, v32i32, v64i32, v128i32, v256i32] in {
      def __ocl_extend_mask_v16 : OclBuiltin<"__ocl_extend_mask_to_$VecLength", (outs gentype:$ret), (ins v16i32:$mask), [], [], 0, 0>, PRIVATE;
    }

    let Types = [v8i32, v16i32, v32i32, v64i32, v128i32] in {
      def __ocl_extend_mask_v8 : OclBuiltin<"__ocl_extend_mask_to_$VecLength", (outs gentype:$ret), (ins v8i32:$mask), [], [], 0, 0>, PRIVATE;
    }
  }

// ****************************************************************************
//                                 masked_load_store
// ****************************************************************************
let Types = [v4f32, v4i32, v4i16, v4i8] in {
let NeedForwardDecl = 1 in {
def __ocl_masked_load4x4   : OclBuiltin<"__ocl_masked_load_$Arg0BaseType_4x4",   (outs), (ins pgentype:$pLoadAdd, pgentype:$pLoadedValues, v4i32:$mask), [], [], 0, 0>, PRIVATE;
def __ocl_masked_load4x8   : OclBuiltin<"__ocl_masked_load_$Arg0BaseType_4x8",   (outs), (ins pgentype:$pLoadAdd, pgentype:$pLoadedValues, v8i32:$mask), [], [], 0, 0>, PRIVATE;
def __ocl_masked_store4x4   : OclBuiltin<"__ocl_masked_store_$Arg0BaseType_4x4",   (outs), (ins pgentype:$pStoreAdd, pgentype:$pValuesToStore, v4i32:$mask), [], [], 0, 0>, PRIVATE;
def __ocl_masked_store4x8   : OclBuiltin<"__ocl_masked_store_$Arg0BaseType_4x8",   (outs), (ins pgentype:$pStoreAdd, pgentype:$pValuesToStore, v8i32:$mask), [], [], 0, 0>, PRIVATE;
}
}
let Types = [v4i32, v4i64, v4f32, v4f64] in {
let NeedForwardDecl = 1 in {
def __ocl_masked_load4   : OclBuiltin<"__ocl_masked_load_$ReturnType",   (outs gentype:$ret), (ins pgentype:$pLoadAdd, v4i32:$mask), [], [], 0, 0>, PRIVATE;
def __ocl_masked_store4   : OclBuiltin<"__ocl_masked_store_$Arg1Type",   (outs), (ins pgentype:$pStoreAdd, gentype:$data, v4i32:$mask), [], [], 0, 0>, PRIVATE;
}
}

let Types = [v8i32, v8i64, v8f32, v8f64], NeedForwardDecl = 1 in {
def __ocl_masked_load8   : OclBuiltin<"__ocl_masked_load_$ReturnType",   (outs gentype:$ret), (ins pgentype:$pLoadAdd, v8i32:$mask), [], [], 0, 0>, PRIVATE;
def __ocl_masked_store8   : OclBuiltin<"__ocl_masked_store_$Arg1Type",   (outs), (ins pgentype:$pStoreAdd, gentype:$data, v8i32:$mask),[], [], 0, 0>, PRIVATE;
}


let Types = [v8i64, v8f64], NeedForwardDecl = 1 in {
def __ocl_masked_load8_knl   : OclBuiltin<"__ocl_imasked_load_$ReturnType",   (outs gentype:$ret), (ins pgentype:$pLoadAdd, v1u8:$mask), [], [], 0, 0>, PRIVATE;
def __ocl_masked_store8_knl   : OclBuiltin<"__ocl_imasked_store_$Arg1Type",   (outs), (ins pgentype:$pStoreAdd, gentype:$data, v1u8:$mask),[], [], 0, 0>, PRIVATE;
}


let Types = [v16i32, v16i64, v16f32, v16f64], NeedForwardDecl = 1 in {
def __ocl_masked_load16_knl   : OclBuiltin<"__ocl_imasked_load_$ReturnType",   (outs gentype:$ret), (ins pgentype:$pLoadAdd, v1u16:$mask), [], [], 0, 0>, PRIVATE;
def __ocl_masked_store16_knl   : OclBuiltin<"__ocl_imasked_store_$Arg1Type",   (outs), (ins pgentype:$pStoreAdd, gentype:$data, v1u16:$mask),[], [], 0, 0>, PRIVATE;
}

let Types = [v2f32, v2i32] in {
let NeedForwardDecl = 1 in {
def __ocl_masked_load2x8   : OclBuiltin<"__ocl_masked_load_$Arg0BaseType_2x8",   (outs), (ins pgentype:$pLoadAdd, pgentype:$pLoadedValues, v8i32:$mask), [], [], 0, 0>, PRIVATE;
}
}


// ****************************************************************************
//               __ocl_helper_shuffle/__ocl_helper_shuffle2
// These helper functions are only declarations (no implementation needed)
// * Used by transpose built-ins with immediate mask
// * Replaced by ShufleCallToInst pass from function call to ShuffleVectorInst
// ****************************************************************************
let Types = [ v2i8,  v4i8,  v8i8,  v16i8,
              v2u8,  v4u8,  v8u8,  v16u8,
             v2i16, v4i16, v8i16, v16i16,
             v2u16, v4u16, v8u16, v16u16,
             v2i32, v4i32, v8i32, v16i32,
             v2u32, v4u32, v8u32, v16u32,
             v2i64, v4i64, v8i64, v16i64,
             v2u64, v4u64, v8u64, v16u64,
             v2f32, v4f32, v8f32, v16f32,
             v2f64, v4f64, v8f64, v16f64],
    Attrs = [CONSTFUNC, OVERLOADABLE],
    NeedForwardDecl = 1 in {
  def helper_shuffle_2   : OclBuiltin<"__ocl_helper_shuffle",   (outs gentype2:$ret), (ins gentype:$x, ugentype2:$mask), [], [], 0, 0>;
  def helper_shuffle_4   : OclBuiltin<"__ocl_helper_shuffle",   (outs gentype4:$ret), (ins gentype:$x, ugentype4:$mask), [], [], 0, 0>;
  def helper_shuffle_8   : OclBuiltin<"__ocl_helper_shuffle",   (outs gentype8:$ret), (ins gentype:$x, ugentype8:$mask), [], [], 0, 0>;
  def helper_shuffle_16  : OclBuiltin<"__ocl_helper_shuffle",   (outs gentype16:$ret), (ins gentype:$x, ugentype16:$mask), [], [], 0, 0>;
  def helper_shuffle2_2  : OclBuiltin<"__ocl_helper_shuffle2",  (outs gentype2:$ret), (ins gentype:$x, gentype:$y, ugentype2:$mask), [], [], 0, 0>;
  def helper_shuffle2_4  : OclBuiltin<"__ocl_helper_shuffle2",  (outs gentype4:$ret), (ins gentype:$x, gentype:$y, ugentype4:$mask), [], [], 0, 0>;
  def helper_shuffle2_8  : OclBuiltin<"__ocl_helper_shuffle2",  (outs gentype8:$ret), (ins gentype:$x, gentype:$y, ugentype8:$mask), [], [], 0, 0>;
  def helper_shuffle2_16 : OclBuiltin<"__ocl_helper_shuffle2",  (outs gentype16:$ret), (ins gentype:$x, gentype:$y, ugentype16:$mask), [], [], 0, 0>;
}

let Types = [v16i8, v16i32, v16f32], NeedForwardDecl = 1 in {
def __ocl_helper_shuffle_pairs :
  OclBuiltin<"__ocl_helper_shuffle_pairs_$Arg1Type", (outs gentype:$ret),
   (ins gentype:$arg0, gentype:$arg1, gentype:$arg2, gentype:$arg3, v16i32:$mask),
  [], [], 0, 0>;
}
