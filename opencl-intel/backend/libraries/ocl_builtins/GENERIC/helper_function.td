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

// ****************************************************************************
//                                 transpose
// ****************************************************************************


let Types = [v4f32, v4i32, v4i8, v4i16],
    Attrs = [INTERNAL_INLINE, OVERLOADABLE],
    NeedForwardDecl = 1 in {
  def __ocl_transpose4x4  : OclBuiltin<"__ocl_transpose4x4", (outs), (ins gentype:$xyzw0, gentype:$xyzw1, gentype:$xyzw2, gentype:$xyzw3, pgentype:$xOut, pgentype:$yOut, pgentype:$zOut, pgentype:$wOut ), [], [], 0, 0>, PRIVATE;
  def __ocl_transpose4x8  : OclBuiltin<"__ocl_transpose4x8", (outs), (ins gentype:$xyzw0, gentype:$xyzw1, gentype:$xyzw2, gentype:$xyzw3, gentype:$xyzw4, gentype:$xyzw5, gentype:$xyzw6, gentype:$xyzw7, pv8gentype:$xOut, pv8gentype:$yOut, pv8gentype:$zOut, pv8gentype:$wOut  ),  [], [], 0, 0>, PRIVATE;
  def __ocl_transpose8x4  : OclBuiltin<"__ocl_transpose8x4", (outs), (ins v8gentype:$xOut, v8gentype:$yOut, v8gentype:$zOut, v8gentype:$wOut, pgentype:$xyzw0, pgentype:$xyzw1, pgentype:$xyzw2, pgentype:$xyzw3, pgentype:$xyzw4, pgentype:$xyzw5, pgentype:$xyzw6, pgentype:$xyzw7 ), [], [], 0, 0>, PRIVATE;
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

// ****************************************************************************
//                                 masked_load_store
// ****************************************************************************
let Types = [v4f32, v4i32, v4i16, v4i8],
    Attrs = [INLINE],
    NeedForwardDecl = 1 in {
  def __ocl_masked_load4x4   : OclBuiltin<"__ocl_masked_load_$Arg0BaseType_4x4",   (outs), (ins pgentype:$pLoadAdd, pgentype:$pLoadedValues, v4i32:$mask), [], [], 0, 0>, PRIVATE;
  def __ocl_masked_load4x8   : OclBuiltin<"__ocl_masked_load_$Arg0BaseType_4x8",   (outs), (ins pgentype:$pLoadAdd, pgentype:$pLoadedValues, v8i32:$mask), [], [], 0, 0>, PRIVATE;
  def __ocl_masked_store4x4   : OclBuiltin<"__ocl_masked_store_$Arg0BaseType_4x4",   (outs), (ins pgentype:$pStoreAdd, pgentype:$pValuesToStore, v4i32:$mask), [], [], 0, 0>, PRIVATE;
  def __ocl_masked_store4x8   : OclBuiltin<"__ocl_masked_store_$Arg0BaseType_4x8",   (outs), (ins pgentype:$pStoreAdd, pgentype:$pValuesToStore, v8i32:$mask), [], [], 0, 0>, PRIVATE;
}

let Types = [v4i32, v4i64, v4f32, v4f64],
    Attrs = [INLINE],
    NeedForwardDecl = 1 in {
  def __ocl_masked_load4   : OclBuiltin<"__ocl_masked_load_$ReturnType",   (outs gentype:$ret), (ins pgentype:$pLoadAdd, v4i32:$mask), [], [], 0, 0>, PRIVATE;
  def __ocl_masked_store4   : OclBuiltin<"__ocl_masked_store_$Arg1Type",   (outs), (ins pgentype:$pStoreAdd, gentype:$data, v4i32:$mask), [], [], 0, 0>, PRIVATE;
}

let Types = [v8i32, v8i64, v8f32, v8f64],
    Attrs = [INLINE],
    NeedForwardDecl = 1 in {
  def __ocl_masked_load8   : OclBuiltin<"__ocl_masked_load_$ReturnType",   (outs gentype:$ret), (ins pgentype:$pLoadAdd, v8i32:$mask), [], [], 0, 0>, PRIVATE;
  def __ocl_masked_store8   : OclBuiltin<"__ocl_masked_store_$Arg1Type",   (outs), (ins pgentype:$pStoreAdd, gentype:$data, v8i32:$mask),[], [], 0, 0>, PRIVATE;
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