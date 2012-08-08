// vim:ts=2:sw=2:et:

// Conversions with saturation optimized for MIC

// Workaround for convert_char3_sat(float3 x) functions.
// convert_i8_sat

OclBuiltinImpl fp2int_i8_f32_sat = OclBuiltinImpl<convert_i8_sat, [v3f32], 0, [{
    float4 convertIn = (float4)0;
    convertIn.s012 = $Arg0VarName;
    char4 convertOut = convert_char4_sat(convertIn);
    return convertOut.s012;
  }]>;

OclBuiltinImpl fp2int_i8_f32_sat_rte = OclBuiltinImpl<convert_i8_sat_rte, [v3f32], 0, [{
    float4 convertIn = (float4)0;
    convertIn.s012 = $Arg0VarName;
    char4 convertOut = convert_char4_sat_rte(convertIn);
    return convertOut.s012;
  }]>;

OclBuiltinImpl fp2int_i8_f32_sat_rtz = OclBuiltinImpl<convert_i8_sat_rtz, [v3f32], 0, [{
    float4 convertIn = (float4)0;
    convertIn.s012 = $Arg0VarName;
    char4 convertOut = convert_char4_sat_rtz(convertIn);
    return convertOut.s012;
  }]>;

OclBuiltinImpl fp2int_i8_f32_sat_rtn = OclBuiltinImpl<convert_i8_sat_rtn, [v3f32], 0, [{
    float4 convertIn = (float4)0;
    convertIn.s012 = $Arg0VarName;
    char4 convertOut = convert_char4_sat_rtn(convertIn);
    return convertOut.s012;
  }]>;

OclBuiltinImpl fp2int_i8_f32_sat_rtp = OclBuiltinImpl<convert_i8_sat_rtp, [v3f32], 0, [{
    float4 convertIn = (float4)0;
    convertIn.s012 = $Arg0VarName;
    char4 convertOut = convert_char4_sat_rtp(convertIn);
    return convertOut.s012;
  }]>;

// convert_u8_sat

OclBuiltinImpl fp2int_u8_f32_sat = OclBuiltinImpl<convert_u8_sat, [v3f32], 0, [{
    float4 convertIn = (float4)0;
    convertIn.s012 = $Arg0VarName;
    uchar4 convertOut = convert_uchar4_sat(convertIn);
    return convertOut.s012;
  }]>;

OclBuiltinImpl fp2int_u8_f32_sat_rte = OclBuiltinImpl<convert_u8_sat_rte, [v3f32], 0, [{
    float4 convertIn = (float4)0;
    convertIn.s012 = $Arg0VarName;
    uchar4 convertOut = convert_uchar4_sat_rte(convertIn);
    return convertOut.s012;
  }]>;

OclBuiltinImpl fp2int_u8_f32_sat_rtz = OclBuiltinImpl<convert_u8_sat_rtz, [v3f32], 0, [{
    float4 convertIn = (float4)0;
    convertIn.s012 = $Arg0VarName;
    uchar4 convertOut = convert_uchar4_sat_rtz(convertIn);
    return convertOut.s012;
  }]>;

OclBuiltinImpl fp2int_u8_f32_sat_rtn = OclBuiltinImpl<convert_u8_sat_rtn, [v3f32], 0, [{
    float4 convertIn = (float4)0;
    convertIn.s012 = $Arg0VarName;
    uchar4 convertOut = convert_uchar4_sat_rtn(convertIn);
    return convertOut.s012;
  }]>;

OclBuiltinImpl fp2int_u8_f32_sat_rtp = OclBuiltinImpl<convert_u8_sat_rtp, [v3f32], 0, [{
    float4 convertIn = (float4)0;
    convertIn.s012 = $Arg0VarName;
    uchar4 convertOut = convert_uchar4_sat_rtp(convertIn);
    return convertOut.s012;
  }]>;

// Workaround for convert_char3(double3 x) functions.
// convert_i8_sat

OclBuiltinImpl fp2int_i8_f64_sat = OclBuiltinImpl<convert_i8_sat, [v3f64], 0, [{
    double4 convertIn = (double4)0;
    convertIn.s012 = $Arg0VarName;
    char4 convertOut = convert_char4_sat(convertIn);
    return convertOut.s012;
  }]>;

OclBuiltinImpl fp2int_i8_f64_sat_rte = OclBuiltinImpl<convert_i8_sat_rte, [v3f64], 0, [{
    double4 convertIn = (double4)0;
    convertIn.s012 = $Arg0VarName;
    char4 convertOut = convert_char4_sat_rte(convertIn);
    return convertOut.s012;
  }]>;

OclBuiltinImpl fp2int_i8_f64_sat_rtz = OclBuiltinImpl<convert_i8_sat_rtz, [v3f64], 0, [{
    double4 convertIn = (double4)0;
    convertIn.s012 = $Arg0VarName;
    char4 convertOut = convert_char4_sat_rtz(convertIn);
    return convertOut.s012;
  }]>;

OclBuiltinImpl fp2int_i8_f64_sat_rtn = OclBuiltinImpl<convert_i8_sat_rtn, [v3f64], 0, [{
    double4 convertIn = (double4)0;
    convertIn.s012 = $Arg0VarName;
    char4 convertOut = convert_char4_sat_rtn(convertIn);
    return convertOut.s012;
  }]>;

OclBuiltinImpl fp2int_i8_f64_sat_rtp = OclBuiltinImpl<convert_i8_sat_rtp, [v3f64], 0, [{
    double4 convertIn = (double4)0;
    convertIn.s012 = $Arg0VarName;
    char4 convertOut = convert_char4_sat_rtp(convertIn);
    return convertOut.s012;
  }]>;

// convert_u8_sat

OclBuiltinImpl fp2int_u8_f64_sat = OclBuiltinImpl<convert_u8_sat, [v3f64], 0, [{
    double4 convertIn = (double4)0;
    convertIn.s012 = $Arg0VarName;
    uchar4 convertOut = convert_uchar4_sat(convertIn);
    return convertOut.s012;
  }]>;

OclBuiltinImpl fp2int_u8_f64_sat_rte = OclBuiltinImpl<convert_u8_sat_rte, [v3f64], 0, [{
    double4 convertIn = (double4)0;
    convertIn.s012 = $Arg0VarName;
    uchar4 convertOut = convert_uchar4_sat_rte(convertIn);
    return convertOut.s012;
  }]>;

OclBuiltinImpl fp2int_u8_f64_sat_rtz = OclBuiltinImpl<convert_u8_sat_rtz, [v3f64], 0, [{
    double4 convertIn = (double4)0;
    convertIn.s012 = $Arg0VarName;
    uchar4 convertOut = convert_uchar4_sat_rtz(convertIn);
    return convertOut.s012;
  }]>;

OclBuiltinImpl fp2int_u8_f64_sat_rtn = OclBuiltinImpl<convert_u8_sat_rtn, [v3f64], 0, [{
    double4 convertIn = (double4)0;
    convertIn.s012 = $Arg0VarName;
    uchar4 convertOut = convert_uchar4_sat_rtn(convertIn);
    return convertOut.s012;
  }]>;

OclBuiltinImpl fp2int_u8_f64_sat_rtp = OclBuiltinImpl<convert_u8_sat_rtp, [v3f64], 0, [{
    double4 convertIn = (double4)0;
    convertIn.s012 = $Arg0VarName;
    uchar4 convertOut = convert_uchar4_sat_rtp(convertIn);
    return convertOut.s012;
  }]>;

// convert_i16_sat

OclBuiltinImpl fp2int_i16_f32_sat = OclBuiltinImpl<convert_i16_sat, [v3f32], 0, [{
    float4 convertIn = (float4)0;
    convertIn.s012 = $Arg0VarName;
    short4 convertOut = convert_short4_sat(convertIn);
    return convertOut.s012;
  }]>;

OclBuiltinImpl fp2int_i16_f32_sat_rte = OclBuiltinImpl<convert_i16_sat_rte, [v3f32], 0, [{
    float4 convertIn = (float4)0;
    convertIn.s012 = $Arg0VarName;
    short4 convertOut = convert_short4_sat_rte(convertIn);
    return convertOut.s012;
  }]>;

OclBuiltinImpl fp2int_i16_f32_sat_rtz = OclBuiltinImpl<convert_i16_sat_rtz, [v3f32], 0, [{
    float4 convertIn = (float4)0;
    convertIn.s012 = $Arg0VarName;
    short4 convertOut = convert_short4_sat_rtz(convertIn);
    return convertOut.s012;
  }]>;

OclBuiltinImpl fp2int_i16_f32_sat_rtn = OclBuiltinImpl<convert_i16_sat_rtn, [v3f32], 0, [{
    float4 convertIn = (float4)0;
    convertIn.s012 = $Arg0VarName;
    short4 convertOut = convert_short4_sat_rtn(convertIn);
    return convertOut.s012;
  }]>;

OclBuiltinImpl fp2int_i16_f32_sat_rtp = OclBuiltinImpl<convert_i16_sat_rtp, [v3f32], 0, [{
    float4 convertIn = (float4)0;
    convertIn.s012 = $Arg0VarName;
    short4 convertOut = convert_short4_sat_rtp(convertIn);
    return convertOut.s012;
  }]>;

// convert_u16_sat

OclBuiltinImpl fp2int_u16_f32_sat = OclBuiltinImpl<convert_u16_sat, [v3f32], 0, [{
    float4 convertIn = (float4)0;
    convertIn.s012 = $Arg0VarName;
    ushort4 convertOut = convert_ushort4_sat(convertIn);
    return convertOut.s012;
  }]>;

OclBuiltinImpl fp2int_u16_f32_sat_rte = OclBuiltinImpl<convert_u16_sat_rte, [v3f32], 0, [{
    float4 convertIn = (float4)0;
    convertIn.s012 = $Arg0VarName;
    ushort4 convertOut = convert_ushort4_sat_rte(convertIn);
    return convertOut.s012;
  }]>;

OclBuiltinImpl fp2int_u16_f32_sat_rtz = OclBuiltinImpl<convert_u16_sat_rtz, [v3f32], 0, [{
    float4 convertIn = (float4)0;
    convertIn.s012 = $Arg0VarName;
    ushort4 convertOut = convert_ushort4_sat_rtz(convertIn);
    return convertOut.s012;
  }]>;

OclBuiltinImpl fp2int_u16_f32_sat_rtn = OclBuiltinImpl<convert_u16_sat_rtn, [v3f32], 0, [{
    float4 convertIn = (float4)0;
    convertIn.s012 = $Arg0VarName;
    ushort4 convertOut = convert_ushort4_sat_rtn(convertIn);
    return convertOut.s012;
  }]>;

OclBuiltinImpl fp2int_u16_f32_sat_rtp = OclBuiltinImpl<convert_u16_sat_rtp, [v3f32], 0, [{
    float4 convertIn = (float4)0;
    convertIn.s012 = $Arg0VarName;
    ushort4 convertOut = convert_ushort4_sat_rtp(convertIn);
    return convertOut.s012;
  }]>;

// convert_i16_sat

OclBuiltinImpl fp2int_i16_f64_sat = OclBuiltinImpl<convert_i16_sat, [v3f64], 0, [{
    double4 convertIn = (double4)0;
    convertIn.s012 = $Arg0VarName;
    short4 convertOut = convert_short4_sat(convertIn);
    return convertOut.s012;
  }]>;

OclBuiltinImpl fp2int_i16_f64_sat_rte = OclBuiltinImpl<convert_i16_sat_rte, [v3f64], 0, [{
    double4 convertIn = (double4)0;
    convertIn.s012 = $Arg0VarName;
    short4 convertOut = convert_short4_sat_rte(convertIn);
    return convertOut.s012;
  }]>;

OclBuiltinImpl fp2int_i16_f64_sat_rtz = OclBuiltinImpl<convert_i16_sat_rtz, [v3f64], 0, [{
    double4 convertIn = (double4)0;
    convertIn.s012 = $Arg0VarName;
    short4 convertOut = convert_short4_sat_rtz(convertIn);
    return convertOut.s012;
  }]>;

OclBuiltinImpl fp2int_i16_f64_sat_rtn = OclBuiltinImpl<convert_i16_sat_rtn, [v3f64], 0, [{
    double4 convertIn = (double4)0;
    convertIn.s012 = $Arg0VarName;
    short4 convertOut = convert_short4_sat_rtn(convertIn);
    return convertOut.s012;
  }]>;

OclBuiltinImpl fp2int_i16_f64_sat_rtp = OclBuiltinImpl<convert_i16_sat_rtp, [v3f64], 0, [{
    double4 convertIn = (double4)0;
    convertIn.s012 = $Arg0VarName;
    short4 convertOut = convert_short4_sat_rtp(convertIn);
    return convertOut.s012;
  }]>;

// convert_u16_sat

OclBuiltinImpl fp2int_u16_f64_sat = OclBuiltinImpl<convert_u16_sat, [v3f64], 0, [{
    double4 convertIn = (double4)0;
    convertIn.s012 = $Arg0VarName;
    ushort4 convertOut = convert_ushort4_sat(convertIn);
    return convertOut.s012;
  }]>;

OclBuiltinImpl fp2int_u16_f64_sat_rte = OclBuiltinImpl<convert_u16_sat_rte, [v3f64], 0, [{
    double4 convertIn = (double4)0;
    convertIn.s012 = $Arg0VarName;
    ushort4 convertOut = convert_ushort4_sat_rte(convertIn);
    return convertOut.s012;
  }]>;

OclBuiltinImpl fp2int_u16_f64_sat_rtz = OclBuiltinImpl<convert_u16_sat_rtz, [v3f64], 0, [{
    double4 convertIn = (double4)0;
    convertIn.s012 = $Arg0VarName;
    ushort4 convertOut = convert_ushort4_sat_rtz(convertIn);
    return convertOut.s012;
  }]>;

OclBuiltinImpl fp2int_u16_f64_sat_rtn = OclBuiltinImpl<convert_u16_sat_rtn, [v3f64], 0, [{
    double4 convertIn = (double4)0;
    convertIn.s012 = $Arg0VarName;
    ushort4 convertOut = convert_ushort4_sat_rtn(convertIn);
    return convertOut.s012;
  }]>;

OclBuiltinImpl fp2int_u16_f64_sat_rtp = OclBuiltinImpl<convert_u16_sat_rtp, [v3f64], 0, [{
    double4 convertIn = (double4)0;
    convertIn.s012 = $Arg0VarName;
    ushort4 convertOut = convert_ushort4_sat_rtp(convertIn);
    return convertOut.s012;
  }]>;

// Workaround for convert_int3_sat(float3 x) functions.
// convert_i32_sat

OclBuiltinImpl fp2int_i32_f32_sat = OclBuiltinImpl<convert_i32_sat, [v3f32], 0, [{
    float4 convertIn = (float4)0;
    convertIn.s012 = $Arg0VarName;
    int4 convertOut = convert_int4_sat(convertIn);
    return convertOut.s012;
  }]>;

OclBuiltinImpl fp2int_i32_f32_sat_rte = OclBuiltinImpl<convert_i32_sat_rte, [v3f32], 0, [{
    float4 convertIn = (float4)0;
    convertIn.s012 = $Arg0VarName;
    int4 convertOut = convert_int4_sat_rte(convertIn);
    return convertOut.s012;
  }]>;

OclBuiltinImpl fp2int_i32_f32_sat_rtz = OclBuiltinImpl<convert_i32_sat_rtz, [v3f32], 0, [{
    float4 convertIn = (float4)0;
    convertIn.s012 = $Arg0VarName;
    int4 convertOut = convert_int4_sat_rtz(convertIn);
    return convertOut.s012;
  }]>;

OclBuiltinImpl fp2int_i32_f32_sat_rtn = OclBuiltinImpl<convert_i32_sat_rtn, [v3f32], 0, [{
    float4 convertIn = (float4)0;
    convertIn.s012 = $Arg0VarName;
    int4 convertOut = convert_int4_sat_rtn(convertIn);
    return convertOut.s012;
  }]>;

OclBuiltinImpl fp2int_i32_f32_sat_rtp = OclBuiltinImpl<convert_i32_sat_rtp, [v3f32], 0, [{
    float4 convertIn = (float4)0;
    convertIn.s012 = $Arg0VarName;
    int4 convertOut = convert_int4_sat_rtp(convertIn);
    return convertOut.s012;
  }]>;

// convert_u32_sat

OclBuiltinImpl fp2int_u32_f32_sat = OclBuiltinImpl<convert_u32_sat, [v3f32], 0, [{
    float4 convertIn = (float4)0;
    convertIn.s012 = $Arg0VarName;
    uint4 convertOut = convert_uint4_sat(convertIn);
    return convertOut.s012;
  }]>;

OclBuiltinImpl fp2int_u32_f32_sat_rte = OclBuiltinImpl<convert_u32_sat_rte, [v3f32], 0, [{
    float4 convertIn = (float4)0;
    convertIn.s012 = $Arg0VarName;
    uint4 convertOut = convert_uint4_sat_rte(convertIn);
    return convertOut.s012;
  }]>;

OclBuiltinImpl fp2int_u32_f32_sat_rtz = OclBuiltinImpl<convert_u32_sat_rtz, [v3f32], 0, [{
    float4 convertIn = (float4)0;
    convertIn.s012 = $Arg0VarName;
    uint4 convertOut = convert_uint4_sat_rtz(convertIn);
    return convertOut.s012;
  }]>;

OclBuiltinImpl fp2int_u32_f32_sat_rtn = OclBuiltinImpl<convert_u32_sat_rtn, [v3f32], 0, [{
    float4 convertIn = (float4)0;
    convertIn.s012 = $Arg0VarName;
    uint4 convertOut = convert_uint4_sat_rtn(convertIn);
    return convertOut.s012;
  }]>;

OclBuiltinImpl fp2int_u32_f32_sat_rtp = OclBuiltinImpl<convert_u32_sat_rtp, [v3f32], 0, [{
    float4 convertIn = (float4)0;
    convertIn.s012 = $Arg0VarName;
    uint4 convertOut = convert_uint4_sat_rtp(convertIn);
    return convertOut.s012;
  }]>;

// Workaround for convert_int3(double3 x) functions.
// convert_i32_sat

OclBuiltinImpl fp2int_i32_f64_sat = OclBuiltinImpl<convert_i32_sat, [v3f64], 0, [{
    double4 convertIn = (double4)0;
    convertIn.s012 = $Arg0VarName;
    int4 convertOut = convert_int4_sat(convertIn);
    return convertOut.s012;
  }]>;

OclBuiltinImpl fp2int_i32_f64_sat_rte = OclBuiltinImpl<convert_i32_sat_rte, [v3f64], 0, [{
    double4 convertIn = (double4)0;
    convertIn.s012 = $Arg0VarName;
    int4 convertOut = convert_int4_sat_rte(convertIn);
    return convertOut.s012;
  }]>;

OclBuiltinImpl fp2int_i32_f64_sat_rtz = OclBuiltinImpl<convert_i32_sat_rtz, [v3f64], 0, [{
    double4 convertIn = (double4)0;
    convertIn.s012 = $Arg0VarName;
    int4 convertOut = convert_int4_sat_rtz(convertIn);
    return convertOut.s012;
  }]>;

OclBuiltinImpl fp2int_i32_f64_sat_rtn = OclBuiltinImpl<convert_i32_sat_rtn, [v3f64], 0, [{
    double4 convertIn = (double4)0;
    convertIn.s012 = $Arg0VarName;
    int4 convertOut = convert_int4_sat_rtn(convertIn);
    return convertOut.s012;
  }]>;

OclBuiltinImpl fp2int_i32_f64_sat_rtp = OclBuiltinImpl<convert_i32_sat_rtp, [v3f64], 0, [{
    double4 convertIn = (double4)0;
    convertIn.s012 = $Arg0VarName;
    int4 convertOut = convert_int4_sat_rtp(convertIn);
    return convertOut.s012;
  }]>;

// convert_u32_sat

OclBuiltinImpl fp2int_u32_f64_sat = OclBuiltinImpl<convert_u32_sat, [v3f64], 0, [{
    double4 convertIn = (double4)0;
    convertIn.s012 = $Arg0VarName;
    uint4 convertOut = convert_uint4_sat(convertIn);
    return convertOut.s012;
  }]>;

OclBuiltinImpl fp2int_u32_f64_sat_rte = OclBuiltinImpl<convert_u32_sat_rte, [v3f64], 0, [{
    double4 convertIn = (double4)0;
    convertIn.s012 = $Arg0VarName;
    uint4 convertOut = convert_uint4_sat_rte(convertIn);
    return convertOut.s012;
  }]>;

OclBuiltinImpl fp2int_u32_f64_sat_rtz = OclBuiltinImpl<convert_u32_sat_rtz, [v3f64], 0, [{
    double4 convertIn = (double4)0;
    convertIn.s012 = $Arg0VarName;
    uint4 convertOut = convert_uint4_sat_rtz(convertIn);
    return convertOut.s012;
  }]>;

OclBuiltinImpl fp2int_u32_f64_sat_rtn = OclBuiltinImpl<convert_u32_sat_rtn, [v3f64], 0, [{
    double4 convertIn = (double4)0;
    convertIn.s012 = $Arg0VarName;
    uint4 convertOut = convert_uint4_sat_rtn(convertIn);
    return convertOut.s012;
  }]>;

OclBuiltinImpl fp2int_u32_f64_sat_rtp = OclBuiltinImpl<convert_u32_sat_rtp, [v3f64], 0, [{
    double4 convertIn = (double4)0;
    convertIn.s012 = $Arg0VarName;
    uint4 convertOut = convert_uint4_sat_rtp(convertIn);
    return convertOut.s012;
  }]>;

// Workaround for convert_long3_sat(float3 x) functions.
// convert_i64_sat

OclBuiltinImpl fp2int_i64_f32_sat = OclBuiltinImpl<convert_i64_sat, [v3f32], 0, [{
    float4 convertIn = (float4)0;
    convertIn.s012 = $Arg0VarName;
    long4 convertOut = convert_long4_sat(convertIn);
    return convertOut.s012;
  }]>;

OclBuiltinImpl fp2int_i64_f32_sat_rte = OclBuiltinImpl<convert_i64_sat_rte, [v3f32], 0, [{
    float4 convertIn = (float4)0;
    convertIn.s012 = $Arg0VarName;
    long4 convertOut = convert_long4_sat_rte(convertIn);
    return convertOut.s012;
  }]>;

OclBuiltinImpl fp2int_i64_f32_sat_rtz = OclBuiltinImpl<convert_i64_sat_rtz, [v3f32], 0, [{
    float4 convertIn = (float4)0;
    convertIn.s012 = $Arg0VarName;
    long4 convertOut = convert_long4_sat_rtz(convertIn);
    return convertOut.s012;
  }]>;

OclBuiltinImpl fp2int_i64_f32_sat_rtn = OclBuiltinImpl<convert_i64_sat_rtn, [v3f32], 0, [{
    float4 convertIn = (float4)0;
    convertIn.s012 = $Arg0VarName;
    long4 convertOut = convert_long4_sat_rtn(convertIn);
    return convertOut.s012;
  }]>;

OclBuiltinImpl fp2int_i64_f32_sat_rtp = OclBuiltinImpl<convert_i64_sat_rtp, [v3f32], 0, [{
    float4 convertIn = (float4)0;
    convertIn.s012 = $Arg0VarName;
    long4 convertOut = convert_long4_sat_rtp(convertIn);
    return convertOut.s012;
  }]>;

// convert_u64_sat

OclBuiltinImpl fp2int_u64_f32_sat = OclBuiltinImpl<convert_u64_sat, [v3f32], 0, [{
    float4 convertIn = (float4)0;
    convertIn.s012 = $Arg0VarName;
    ulong4 convertOut = convert_ulong4_sat(convertIn);
    return convertOut.s012;
  }]>;

OclBuiltinImpl fp2int_u64_f32_sat_rte = OclBuiltinImpl<convert_u64_sat_rte, [v3f32], 0, [{
    float4 convertIn = (float4)0;
    convertIn.s012 = $Arg0VarName;
    ulong4 convertOut = convert_ulong4_sat_rte(convertIn);
    return convertOut.s012;
  }]>;

OclBuiltinImpl fp2int_u64_f32_sat_rtz = OclBuiltinImpl<convert_u64_sat_rtz, [v3f32], 0, [{
    float4 convertIn = (float4)0;
    convertIn.s012 = $Arg0VarName;
    ulong4 convertOut = convert_ulong4_sat_rtz(convertIn);
    return convertOut.s012;
  }]>;

OclBuiltinImpl fp2int_u64_f32_sat_rtn = OclBuiltinImpl<convert_u64_sat_rtn, [v3f32], 0, [{
    float4 convertIn = (float4)0;
    convertIn.s012 = $Arg0VarName;
    ulong4 convertOut = convert_ulong4_sat_rtn(convertIn);
    return convertOut.s012;
  }]>;

OclBuiltinImpl fp2int_u64_f32_sat_rtp = OclBuiltinImpl<convert_u64_sat_rtp, [v3f32], 0, [{
    float4 convertIn = (float4)0;
    convertIn.s012 = $Arg0VarName;
    ulong4 convertOut = convert_ulong4_sat_rtp(convertIn);
    return convertOut.s012;
  }]>;

// Workaround for convert_long3(double3 x) functions.
// convert_i64_sat

OclBuiltinImpl fp2int_i64_f64_sat = OclBuiltinImpl<convert_i64_sat, [v3f64], 0, [{
    double4 convertIn = (double4)0;
    convertIn.s012 = $Arg0VarName;
    long4 convertOut = convert_long4_sat(convertIn);
    return convertOut.s012;
  }]>;

OclBuiltinImpl fp2int_i64_f64_sat_rte = OclBuiltinImpl<convert_i64_sat_rte, [v3f64], 0, [{
    double4 convertIn = (double4)0;
    convertIn.s012 = $Arg0VarName;
    long4 convertOut = convert_long4_sat_rte(convertIn);
    return convertOut.s012;
  }]>;

OclBuiltinImpl fp2int_i64_f64_sat_rtz = OclBuiltinImpl<convert_i64_sat_rtz, [v3f64], 0, [{
    double4 convertIn = (double4)0;
    convertIn.s012 = $Arg0VarName;
    long4 convertOut = convert_long4_sat_rtz(convertIn);
    return convertOut.s012;
  }]>;

OclBuiltinImpl fp2int_i64_f64_sat_rtn = OclBuiltinImpl<convert_i64_sat_rtn, [v3f64], 0, [{
    double4 convertIn = (double4)0;
    convertIn.s012 = $Arg0VarName;
    long4 convertOut = convert_long4_sat_rtn(convertIn);
    return convertOut.s012;
  }]>;

OclBuiltinImpl fp2int_i64_f64_sat_rtp = OclBuiltinImpl<convert_i64_sat_rtp, [v3f64], 0, [{
    double4 convertIn = (double4)0;
    convertIn.s012 = $Arg0VarName;
    long4 convertOut = convert_long4_sat_rtp(convertIn);
    return convertOut.s012;
  }]>;

// convert_u64_sat

OclBuiltinImpl fp2int_u64_f64_sat = OclBuiltinImpl<convert_u64_sat, [v3f64], 0, [{
    double4 convertIn = (double4)0;
    convertIn.s012 = $Arg0VarName;
    ulong4 convertOut = convert_ulong4_sat(convertIn);
    return convertOut.s012;
  }]>;

OclBuiltinImpl fp2int_u64_f64_sat_rte = OclBuiltinImpl<convert_u64_sat_rte, [v3f64], 0, [{
    double4 convertIn = (double4)0;
    convertIn.s012 = $Arg0VarName;
    ulong4 convertOut = convert_ulong4_sat_rte(convertIn);
    return convertOut.s012;
  }]>;

OclBuiltinImpl fp2int_u64_f64_sat_rtz = OclBuiltinImpl<convert_u64_sat_rtz, [v3f64], 0, [{
    double4 convertIn = (double4)0;
    convertIn.s012 = $Arg0VarName;
    ulong4 convertOut = convert_ulong4_sat_rtz(convertIn);
    return convertOut.s012;
  }]>;

OclBuiltinImpl fp2int_u64_f64_sat_rtn = OclBuiltinImpl<convert_u64_sat_rtn, [v3f64], 0, [{
    double4 convertIn = (double4)0;
    convertIn.s012 = $Arg0VarName;
    ulong4 convertOut = convert_ulong4_sat_rtn(convertIn);
    return convertOut.s012;
  }]>;

OclBuiltinImpl fp2int_u64_f64_sat_rtp = OclBuiltinImpl<convert_u64_sat_rtp, [v3f64], 0, [{
    double4 convertIn = (double4)0;
    convertIn.s012 = $Arg0VarName;
    ulong4 convertOut = convert_ulong4_sat_rtp(convertIn);
    return convertOut.s012;
  }]>;

