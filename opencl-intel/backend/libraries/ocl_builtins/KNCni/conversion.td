// vim:ts=2:sw=2:et:

// Conversions optimized for MIC

OclBuiltinImpl mask_mic_cvt_f64_f32 = OclBuiltinImpl<mask_convert_f64, [v8f32], 0, [{
    float16 n$Arg1VarName; n$Arg1VarName.lo = $Arg1VarName;
    return as_$ReturnType(_mm512_mask_cvtpslo_pd(_mm512_undefined_pd(), $Arg0VarName, n$Arg1VarName));
  }]>;
OclBuiltinImpl mic_cvt_f64_f32 = OclBuiltinImpl<convert_f64, [v8f32], 0, [{
    float16 n$Arg0VarName; n$Arg0VarName.lo = $Arg0VarName;
    return as_$ReturnType(_mm512_cvtpslo_pd(n$Arg0VarName));
  }]>;
OclBuiltinImpl mic_cvt_f64_promote = OclBuiltinImpl<convert_f64, [v1f32, v2f32, v3f32, v4f32], 0, [{
    float8 n$Arg0VarName; n$Arg0VarName$Pattern = $Arg0VarName;
    double8 $ReturnVarName = mask_convert_double8($Mask, n$Arg0VarName);
    return $ReturnVarName$Pattern;
  }]>;
OclBuiltinImpl mic_cvt_f64_expand = OclBuiltinImpl<convert_f64, [v16f32], 0, [{
    $ReturnType $ReturnVarName;
    $ReturnVarName$ExpandLoPattern = convert_double8($Arg0VarName$ExpandLoPattern);
    $ReturnVarName$ExpandHiPattern = convert_double8($Arg0VarName$ExpandHiPattern);
    return $ReturnVarName;
  }]>;

OclBuiltinImpl mic_rte_cvt_f64_f32 = OclBuiltinImpl<convert_f64_rte,
                                                    [v1f32, v2f32, v3f32, v4f32, v8f32, v16f32], 0, [{
    return convert_$ReturnType($Arg0VarName);
  }]>;

OclBuiltinImpl mic_rtz_cvt_f64_f32 = OclBuiltinImpl<convert_f64_rtz,
                                                    [v1f32, v2f32, v3f32, v4f32, v8f32, v16f32], 0, [{
    return convert_$ReturnType($Arg0VarName);
  }]>;

OclBuiltinImpl mic_rtp_cvt_f64_f32 = OclBuiltinImpl<convert_f64_rtp,
                                                    [v1f32, v2f32, v3f32, v4f32, v8f32, v16f32], 0, [{
    return convert_$ReturnType($Arg0VarName);
  }]>;

OclBuiltinImpl mic_rtn_cvt_f64_f32 = OclBuiltinImpl<convert_f64_rtn,
                                                    [v1f32, v2f32, v3f32, v4f32, v8f32, v16f32], 0, [{
    return convert_$ReturnType($Arg0VarName);
  }]>;

OclBuiltinImpl mic_rte_cvt_f32_f64 = OclBuiltinImpl<convert_f32_rte,
                                                    [v1f64, v2f64, v3f64, v4f64, v8f64, v16f64], 0, [{
    return as_$ReturnType(
            trunc_$Arg0Type_$ReturnType($Arg0VarName));
  }]>;

OclBuiltinImpl mic_cvt_f32_f64 = OclBuiltinImpl<convert_f32,
                                                    [v1f64, v2f64, v3f64, v4f64, v8f64, v16f64], 0, [{
    return as_$ReturnType(
            trunc_$Arg0Type_$ReturnType($Arg0VarName));
  }]>;

OclBuiltinImpl cvt_f32_i8i16 = OclBuiltinImpl<convert_f32,
                                              [ v1i8,  v2i8,  v4i8,  v8i8,
                                               v1i16, v2i16, v4i16, v8i16,
                                               v1i32, v2i32, v4i32, v8i32], 0, [{
    return __ocl_svml_$Target_cvti64tofp$rtn$SVMLSuffix(
            as_$Arg0i64gentype(
              sext_$Arg0Symigentype_$Arg0Symi64gentype(
                as_$Arg0igentype($Arg0VarName))));
  }]>;

OclBuiltinImpl cvt_f32_u8u16 = OclBuiltinImpl<convert_f32,
                                              [ v1u8,  v2u8,  v4u8,  v8u8,
                                               v1u16, v2u16, v4u16, v8u16,
                                               v1u32, v2u32, v4u32, v8u32], 0, [{
    return __ocl_svml_$Target_cvtu64tofp$rtn$SVMLSuffix(
            as_$Arg0u64gentype(
              zext_$Arg0Symigentype_$Arg0Symi64gentype(
                as_$Arg0igentype($Arg0VarName))));
  }]>;

OclBuiltinImpl rte_cvt_f32_i8i16 = OclBuiltinImpl<convert_f32_rte,
                                              [ v1i8,  v2i8,  v4i8,  v8i8,
                                               v1i16, v2i16, v4i16, v8i16,
                                               v1i32, v2i32, v4i32, v8i32], 0, [{
    return __ocl_svml_$Target_cvti64tofp$rtn$SVMLSuffix(
            as_$Arg0i64gentype(
              sext_$Arg0Symigentype_$Arg0Symi64gentype(
                as_$Arg0igentype($Arg0VarName))));
  }]>;

OclBuiltinImpl rte_cvt_f32_u8u16 = OclBuiltinImpl<convert_f32_rte,
                                              [ v1u8,  v2u8,  v4u8,  v8u8,
                                               v1u16, v2u16, v4u16, v8u16,
                                               v1u32, v2u32, v4u32, v8u32], 0, [{
    return __ocl_svml_$Target_cvtu64tofp$rtn$SVMLSuffix(
            as_$Arg0u64gentype(
              zext_$Arg0Symigentype_$Arg0Symi64gentype(
                as_$Arg0igentype($Arg0VarName))));
  }]>;

OclBuiltinImpl rtz_cvt_f32_i8i16 = OclBuiltinImpl<convert_f32_rtz,
                                              [ v1i8,  v2i8,  v4i8,  v8i8,
                                               v1i16, v2i16, v4i16, v8i16,
                                               v1i32, v2i32, v4i32, v8i32], 0, [{
    return __ocl_svml_$Target_cvti64tofp$rtz$SVMLSuffix(
            as_$Arg0i64gentype(
              sext_$Arg0Symigentype_$Arg0Symi64gentype(
                as_$Arg0igentype($Arg0VarName))));
  }]>;

OclBuiltinImpl rtz_cvt_f32_u8u16 = OclBuiltinImpl<convert_f32_rtz,
                                              [ v1u8,  v2u8,  v4u8,  v8u8,
                                               v1u16, v2u16, v4u16, v8u16,
                                               v1u32, v2u32, v4u32, v8u32], 0, [{
    return __ocl_svml_$Target_cvtu64tofp$rtz$SVMLSuffix(
            as_$Arg0u64gentype(
              zext_$Arg0Symigentype_$Arg0Symi64gentype(
                as_$Arg0igentype($Arg0VarName))));
  }]>;

OclBuiltinImpl rtp_cvt_f32_i8i16 = OclBuiltinImpl<convert_f32_rtp,
                                              [ v1i8,  v2i8,  v4i8,  v8i8,
                                               v1i16, v2i16, v4i16, v8i16,
                                               v1i32, v2i32, v4i32, v8i32], 0, [{
    return __ocl_svml_$Target_cvti64tofp$up$SVMLSuffix(
            as_$Arg0i64gentype(
              sext_$Arg0Symigentype_$Arg0Symi64gentype(
                as_$Arg0igentype($Arg0VarName))));
  }]>;

OclBuiltinImpl rtp_cvt_f32_u8u16 = OclBuiltinImpl<convert_f32_rtp,
                                              [ v1u8,  v2u8,  v4u8,  v8u8,
                                               v1u16, v2u16, v4u16, v8u16,
                                               v1u32, v2u32, v4u32, v8u32], 0, [{
    return __ocl_svml_$Target_cvtu64tofp$up$SVMLSuffix(
            as_$Arg0u64gentype(
              zext_$Arg0Symigentype_$Arg0Symi64gentype(
                as_$Arg0igentype($Arg0VarName))));
  }]>;

OclBuiltinImpl rtn_cvt_f32_i8i16 = OclBuiltinImpl<convert_f32_rtn,
                                              [ v1i8,  v2i8,  v4i8,  v8i8,
                                               v1i16, v2i16, v4i16, v8i16,
                                               v1i32, v2i32, v4i32, v8i32], 0, [{
    return __ocl_svml_$Target_cvti64tofp$down$SVMLSuffix(
            as_$Arg0i64gentype(
              sext_$Arg0Symigentype_$Arg0Symi64gentype(
                as_$Arg0igentype($Arg0VarName))));
  }]>;

OclBuiltinImpl rtn_cvt_f32_u8u16 = OclBuiltinImpl<convert_f32_rtn,
                                              [ v1u8,  v2u8,  v4u8,  v8u8,
                                               v1u16, v2u16, v4u16, v8u16,
                                               v1u32, v2u32, v4u32, v8u32], 0, [{
    return __ocl_svml_$Target_cvtu64tofp$down$SVMLSuffix(
            as_$Arg0u64gentype(
              zext_$Arg0Symigentype_$Arg0Symi64gentype(
                as_$Arg0igentype($Arg0VarName))));
  }]>;

// Workaround for convert_char3(float3 x) functions.

OclBuiltinImpl fp2int_i8_f32  = OclBuiltinImpl<convert_i8, [v3f32], 0, [{
    float4 convertIn = (float4)0;
    convertIn.s012 = $Arg0VarName;
    char4 convertOut = convert_char4(convertIn);
    return convertOut.s012;
  }]>;

OclBuiltinImpl fp2int_i8_f32_rte  = OclBuiltinImpl<convert_i8_rte, [v3f32], 0, [{
    float4 convertIn = (float4)0;
    convertIn.s012 = $Arg0VarName;
    char4 convertOut = convert_char4_rte(convertIn);
    return convertOut.s012;
  }]>;

OclBuiltinImpl fp2int_i8_f32_rtz  = OclBuiltinImpl<convert_i8_rtz, [v3f32], 0, [{
    float4 convertIn = (float4)0;
    convertIn.s012 = $Arg0VarName;
    char4 convertOut = convert_char4_rtz(convertIn);
    return convertOut.s012;
  }]>;

OclBuiltinImpl fp2int_i8_f32_rtp  = OclBuiltinImpl<convert_i8_rtp, [v3f32], 0, [{
    float4 convertIn = (float4)0;
    convertIn.s012 = $Arg0VarName;
    char4 convertOut = convert_char4_rtp(convertIn);
    return convertOut.s012;
  }]>;

OclBuiltinImpl fp2int_i8_f32_rtn  = OclBuiltinImpl<convert_i8_rtn, [v3f32], 0, [{
    float4 convertIn = (float4)0;
    convertIn.s012 = $Arg0VarName;
    char4 convertOut = convert_char4_rtn(convertIn);
    return convertOut.s012;
  }]>;

// Workaround for convert_uchar3(float3 x) functions.

OclBuiltinImpl fp2int_u8_f32  = OclBuiltinImpl<convert_u8, [v3f32], 0, [{
    float4 convertIn = (float4)0;
    convertIn.s012 = $Arg0VarName;
    uchar4 convertOut = convert_uchar4(convertIn);
    return convertOut.s012;
  }]>;

OclBuiltinImpl fp2int_u8_f32_rte  = OclBuiltinImpl<convert_u8_rte, [v3f32], 0, [{
    float4 convertIn = (float4)0;
    convertIn.s012 = $Arg0VarName;
    uchar4 convertOut = convert_uchar4_rte(convertIn);
    return convertOut.s012;
  }]>;

OclBuiltinImpl fp2int_u8_f32_rtz  = OclBuiltinImpl<convert_u8_rtz, [v3f32], 0, [{
    float4 convertIn = (float4)0;
    convertIn.s012 = $Arg0VarName;
    uchar4 convertOut = convert_uchar4_rtz(convertIn);
    return convertOut.s012;
  }]>;

OclBuiltinImpl fp2int_u8_f32_rtp  = OclBuiltinImpl<convert_u8_rtp, [v3f32], 0, [{
    float4 convertIn = (float4)0;
    convertIn.s012 = $Arg0VarName;
    uchar4 convertOut = convert_uchar4_rtp(convertIn);
    return convertOut.s012;
  }]>;

OclBuiltinImpl fp2int_u8_f32_rtn  = OclBuiltinImpl<convert_u8_rtn, [v3f32], 0, [{
    float4 convertIn = (float4)0;
    convertIn.s012 = $Arg0VarName;
    uchar4 convertOut = convert_uchar4_rtn(convertIn);
    return convertOut.s012;
  }]>;

// Workaround for convert_char3(double3 x) functions.

OclBuiltinImpl fp2int_i8_f64  = OclBuiltinImpl<convert_i8, [v3f64], 0, [{
    double4 convertIn = (double4)0;
    convertIn.s012 = $Arg0VarName;
    char4 convertOut = convert_char4(convertIn);
    return convertOut.s012;
  }]>;

OclBuiltinImpl fp2int_i8_f64_rte  = OclBuiltinImpl<convert_i8_rte, [v3f64], 0, [{
    double4 convertIn = (double4)0;
    convertIn.s012 = $Arg0VarName;
    char4 convertOut = convert_char4_rte(convertIn);
    return convertOut.s012;
  }]>;

OclBuiltinImpl fp2int_i8_f64_rtz  = OclBuiltinImpl<convert_i8_rtz, [v3f64], 0, [{
    double4 convertIn = (double4)0;
    convertIn.s012 = $Arg0VarName;
    char4 convertOut = convert_char4_rtz(convertIn);
    return convertOut.s012;
  }]>;

OclBuiltinImpl fp2int_i8_f64_rtp  = OclBuiltinImpl<convert_i8_rtp, [v3f64], 0, [{
    double4 convertIn = (double4)0;
    convertIn.s012 = $Arg0VarName;
    char4 convertOut = convert_char4_rtp(convertIn);
    return convertOut.s012;
  }]>;

OclBuiltinImpl fp2int_i8_f64_rtn  = OclBuiltinImpl<convert_i8_rtn, [v3f64], 0, [{
    double4 convertIn = (double4)0;
    convertIn.s012 = $Arg0VarName;
    char4 convertOut = convert_char4_rtn(convertIn);
    return convertOut.s012;
  }]>;

// Workaround for convert_uchar3(double3 x) functions.

OclBuiltinImpl fp2int_u8_f64  = OclBuiltinImpl<convert_u8, [v3f64], 0, [{
    double4 convertIn = (double4)0;
    convertIn.s012 = $Arg0VarName;
    uchar4 convertOut = convert_uchar4(convertIn);
    return convertOut.s012;
  }]>;

OclBuiltinImpl fp2int_u8_f64_rte  = OclBuiltinImpl<convert_u8_rte, [v3f64], 0, [{
    double4 convertIn = (double4)0;
    convertIn.s012 = $Arg0VarName;
    uchar4 convertOut = convert_uchar4_rte(convertIn);
    return convertOut.s012;
  }]>;

OclBuiltinImpl fp2int_u8_f64_rtz  = OclBuiltinImpl<convert_u8_rtz, [v3f64], 0, [{
    double4 convertIn = (double4)0;
    convertIn.s012 = $Arg0VarName;
    uchar4 convertOut = convert_uchar4_rtz(convertIn);
    return convertOut.s012;
  }]>;

OclBuiltinImpl fp2int_u8_f64_rtp  = OclBuiltinImpl<convert_u8_rtp, [v3f64], 0, [{
    double4 convertIn = (double4)0;
    convertIn.s012 = $Arg0VarName;
    uchar4 convertOut = convert_uchar4_rtp(convertIn);
    return convertOut.s012;
  }]>;

OclBuiltinImpl fp2int_u8_f64_rtn  = OclBuiltinImpl<convert_u8_rtn, [v3f64], 0, [{
    double4 convertIn = (double4)0;
    convertIn.s012 = $Arg0VarName;
    uchar4 convertOut = convert_uchar4_rtn(convertIn);
    return convertOut.s012;
  }]>;

// Workaround for convert_short3(float3 x) functions.

OclBuiltinImpl fp2int_i16_f32  = OclBuiltinImpl<convert_i16, [v3f32], 0, [{
    float4 convertIn = (float4)0;
    convertIn.s012 = $Arg0VarName;
    short4 convertOut = convert_short4(convertIn);
    return convertOut.s012;
  }]>;

OclBuiltinImpl fp2int_i16_f32_rte  = OclBuiltinImpl<convert_i16_rte, [v3f32], 0, [{
    float4 convertIn = (float4)0;
    convertIn.s012 = $Arg0VarName;
    short4 convertOut = convert_short4_rte(convertIn);
    return convertOut.s012;
  }]>;

OclBuiltinImpl fp2int_i16_f32_rtz  = OclBuiltinImpl<convert_i16_rtz, [v3f32], 0, [{
    float4 convertIn = (float4)0;
    convertIn.s012 = $Arg0VarName;
    short4 convertOut = convert_short4_rtz(convertIn);
    return convertOut.s012;
  }]>;

OclBuiltinImpl fp2int_i16_f32_rtp  = OclBuiltinImpl<convert_i16_rtp, [v3f32], 0, [{
    float4 convertIn = (float4)0;
    convertIn.s012 = $Arg0VarName;
    short4 convertOut = convert_short4_rtp(convertIn);
    return convertOut.s012;
  }]>;

OclBuiltinImpl fp2int_i16_f32_rtn  = OclBuiltinImpl<convert_i16_rtn, [v3f32], 0, [{
    float4 convertIn = (float4)0;
    convertIn.s012 = $Arg0VarName;
    short4 convertOut = convert_short4_rtn(convertIn);
    return convertOut.s012;
  }]>;

// Workaround for convert_ushort3(float3 x) functions.

OclBuiltinImpl fp2int_u16_f32  = OclBuiltinImpl<convert_u16, [v3f32], 0, [{
    float4 convertIn = (float4)0;
    convertIn.s012 = $Arg0VarName;
    ushort4 convertOut = convert_ushort4(convertIn);
    return convertOut.s012;
  }]>;

OclBuiltinImpl fp2int_u16_f32_rte  = OclBuiltinImpl<convert_u16_rte, [v3f32], 0, [{
    float4 convertIn = (float4)0;
    convertIn.s012 = $Arg0VarName;
    ushort4 convertOut = convert_ushort4_rte(convertIn);
    return convertOut.s012;
  }]>;

OclBuiltinImpl fp2int_u16_f32_rtz  = OclBuiltinImpl<convert_u16_rtz, [v3f32], 0, [{
    float4 convertIn = (float4)0;
    convertIn.s012 = $Arg0VarName;
    ushort4 convertOut = convert_ushort4_rtz(convertIn);
    return convertOut.s012;
  }]>;

OclBuiltinImpl fp2int_u16_f32_rtp  = OclBuiltinImpl<convert_u16_rtp, [v3f32], 0, [{
    float4 convertIn = (float4)0;
    convertIn.s012 = $Arg0VarName;
    ushort4 convertOut = convert_ushort4_rtp(convertIn);
    return convertOut.s012;
  }]>;

OclBuiltinImpl fp2int_u16_f32_rtn  = OclBuiltinImpl<convert_u16_rtn, [v3f32], 0, [{
    float4 convertIn = (float4)0;
    convertIn.s012 = $Arg0VarName;
    ushort4 convertOut = convert_ushort4_rtn(convertIn);
    return convertOut.s012;
  }]>;

// Workaround for convert_short3(double3 x) functions.

OclBuiltinImpl fp2int_i16_f64  = OclBuiltinImpl<convert_i16, [v3f64], 0, [{
    double4 convertIn = (double4)0;
    convertIn.s012 = $Arg0VarName;
    short4 convertOut = convert_short4(convertIn);
    return convertOut.s012;
  }]>;

OclBuiltinImpl fp2int_i16_f64_rte  = OclBuiltinImpl<convert_i16_rte, [v3f64], 0, [{
    double4 convertIn = (double4)0;
    convertIn.s012 = $Arg0VarName;
    short4 convertOut = convert_short4_rte(convertIn);
    return convertOut.s012;
  }]>;

OclBuiltinImpl fp2int_i16_f64_rtz  = OclBuiltinImpl<convert_i16_rtz, [v3f64], 0, [{
    double4 convertIn = (double4)0;
    convertIn.s012 = $Arg0VarName;
    short4 convertOut = convert_short4_rtz(convertIn);
    return convertOut.s012;
  }]>;

OclBuiltinImpl fp2int_i16_f64_rtp  = OclBuiltinImpl<convert_i16_rtp, [v3f64], 0, [{
    double4 convertIn = (double4)0;
    convertIn.s012 = $Arg0VarName;
    short4 convertOut = convert_short4_rtp(convertIn);
    return convertOut.s012;
  }]>;

OclBuiltinImpl fp2int_i16_f64_rtn  = OclBuiltinImpl<convert_i16_rtn, [v3f64], 0, [{
    double4 convertIn = (double4)0;
    convertIn.s012 = $Arg0VarName;
    short4 convertOut = convert_short4_rtn(convertIn);
    return convertOut.s012;
  }]>;

// Workaround for convert_ushort3(double3 x) functions.

OclBuiltinImpl fp2int_u16_f64  = OclBuiltinImpl<convert_u16, [v3f64], 0, [{
    double4 convertIn = (double4)0;
    convertIn.s012 = $Arg0VarName;
    ushort4 convertOut = convert_ushort4(convertIn);
    return convertOut.s012;
  }]>;

OclBuiltinImpl fp2int_u16_f64_rte  = OclBuiltinImpl<convert_u16_rte, [v3f64], 0, [{
    double4 convertIn = (double4)0;
    convertIn.s012 = $Arg0VarName;
    ushort4 convertOut = convert_ushort4_rte(convertIn);
    return convertOut.s012;
  }]>;

OclBuiltinImpl fp2int_u16_f64_rtz  = OclBuiltinImpl<convert_u16_rtz, [v3f64], 0, [{
    double4 convertIn = (double4)0;
    convertIn.s012 = $Arg0VarName;
    ushort4 convertOut = convert_ushort4_rtz(convertIn);
    return convertOut.s012;
  }]>;

OclBuiltinImpl fp2int_u16_f64_rtp  = OclBuiltinImpl<convert_u16_rtp, [v3f64], 0, [{
    double4 convertIn = (double4)0;
    convertIn.s012 = $Arg0VarName;
    ushort4 convertOut = convert_ushort4_rtp(convertIn);
    return convertOut.s012;
  }]>;

OclBuiltinImpl fp2int_u16_f64_rtn  = OclBuiltinImpl<convert_u16_rtn, [v3f64], 0, [{
    double4 convertIn = (double4)0;
    convertIn.s012 = $Arg0VarName;
    ushort4 convertOut = convert_ushort4_rtn(convertIn);
    return convertOut.s012;
  }]>;

// Workaround for convert_int3(float3 x) functions.

OclBuiltinImpl fp2int_i32_f32  = OclBuiltinImpl<convert_i32, [v3f32], 0, [{
    float4 convertIn = (float4)0;
    convertIn.s012 = $Arg0VarName;
    int4 convertOut = convert_int4(convertIn);
    return convertOut.s012;
  }]>;

OclBuiltinImpl fp2int_i32_f32_rte  = OclBuiltinImpl<convert_i32_rte, [v3f32], 0, [{
    float4 convertIn = (float4)0;
    convertIn.s012 = $Arg0VarName;
    int4 convertOut = convert_int4_rte(convertIn);
    return convertOut.s012;
  }]>;

OclBuiltinImpl fp2int_i32_f32_rtz  = OclBuiltinImpl<convert_i32_rtz, [v3f32], 0, [{
    float4 convertIn = (float4)0;
    convertIn.s012 = $Arg0VarName;
    int4 convertOut = convert_int4_rtz(convertIn);
    return convertOut.s012;
  }]>;

OclBuiltinImpl fp2int_i32_f32_rtp  = OclBuiltinImpl<convert_i32_rtp, [v3f32], 0, [{
    float4 convertIn = (float4)0;
    convertIn.s012 = $Arg0VarName;
    int4 convertOut = convert_int4_rtp(convertIn);
    return convertOut.s012;
  }]>;

OclBuiltinImpl fp2int_i32_f32_rtn  = OclBuiltinImpl<convert_i32_rtn, [v3f32], 0, [{
    float4 convertIn = (float4)0;
    convertIn.s012 = $Arg0VarName;
    int4 convertOut = convert_int4_rtn(convertIn);
    return convertOut.s012;
  }]>;

// Workaround for convert_uint3(float3 x) functions.

OclBuiltinImpl fp2int_u32_f32  = OclBuiltinImpl<convert_u32, [v3f32], 0, [{
    float4 convertIn = (float4)0;
    convertIn.s012 = $Arg0VarName;
    uint4 convertOut = convert_uint4(convertIn);
    return convertOut.s012;
  }]>;

OclBuiltinImpl fp2int_u32_f32_rte  = OclBuiltinImpl<convert_u32_rte, [v3f32], 0, [{
    float4 convertIn = (float4)0;
    convertIn.s012 = $Arg0VarName;
    uint4 convertOut = convert_uint4_rte(convertIn);
    return convertOut.s012;
  }]>;

OclBuiltinImpl fp2int_u32_f32_rtz  = OclBuiltinImpl<convert_u32_rtz, [v3f32], 0, [{
    float4 convertIn = (float4)0;
    convertIn.s012 = $Arg0VarName;
    uint4 convertOut = convert_uint4_rtz(convertIn);
    return convertOut.s012;
  }]>;

OclBuiltinImpl fp2int_u32_f32_rtp  = OclBuiltinImpl<convert_u32_rtp, [v3f32], 0, [{
    float4 convertIn = (float4)0;
    convertIn.s012 = $Arg0VarName;
    uint4 convertOut = convert_uint4_rtp(convertIn);
    return convertOut.s012;
  }]>;

OclBuiltinImpl fp2int_u32_f32_rtn  = OclBuiltinImpl<convert_u32_rtn, [v3f32], 0, [{
    float4 convertIn = (float4)0;
    convertIn.s012 = $Arg0VarName;
    uint4 convertOut = convert_uint4_rtn(convertIn);
    return convertOut.s012;
  }]>;

// Workaround for convert_int3(double3 x) functions.

OclBuiltinImpl fp2int_i32_f64  = OclBuiltinImpl<convert_i32, [v3f64], 0, [{
    double4 convertIn = (double4)0;
    convertIn.s012 = $Arg0VarName;
    int4 convertOut = convert_int4(convertIn);
    return convertOut.s012;
  }]>;

OclBuiltinImpl fp2int_i32_f64_rte  = OclBuiltinImpl<convert_i32_rte, [v3f64], 0, [{
    double4 convertIn = (double4)0;
    convertIn.s012 = $Arg0VarName;
    int4 convertOut = convert_int4_rte(convertIn);
    return convertOut.s012;
  }]>;

OclBuiltinImpl fp2int_i32_f64_rtz  = OclBuiltinImpl<convert_i32_rtz, [v3f64], 0, [{
    double4 convertIn = (double4)0;
    convertIn.s012 = $Arg0VarName;
    int4 convertOut = convert_int4_rtz(convertIn);
    return convertOut.s012;
  }]>;

OclBuiltinImpl fp2int_i32_f64_rtp  = OclBuiltinImpl<convert_i32_rtp, [v3f64], 0, [{
    double4 convertIn = (double4)0;
    convertIn.s012 = $Arg0VarName;
    int4 convertOut = convert_int4_rtp(convertIn);
    return convertOut.s012;
  }]>;

OclBuiltinImpl fp2int_i32_f64_rtn  = OclBuiltinImpl<convert_i32_rtn, [v3f64], 0, [{
    double4 convertIn = (double4)0;
    convertIn.s012 = $Arg0VarName;
    int4 convertOut = convert_int4_rtn(convertIn);
    return convertOut.s012;
  }]>;

// Workaround for convert_uint3(double3 x) functions.

OclBuiltinImpl fp2int_u32_f64  = OclBuiltinImpl<convert_u32, [v3f64], 0, [{
    double4 convertIn = (double4)0;
    convertIn.s012 = $Arg0VarName;
    uint4 convertOut = convert_uint4(convertIn);
    return convertOut.s012;
  }]>;

OclBuiltinImpl fp2int_u32_f64_rte  = OclBuiltinImpl<convert_u32_rte, [v3f64], 0, [{
    double4 convertIn = (double4)0;
    convertIn.s012 = $Arg0VarName;
    uint4 convertOut = convert_uint4_rte(convertIn);
    return convertOut.s012;
  }]>;

OclBuiltinImpl fp2int_u32_f64_rtz  = OclBuiltinImpl<convert_u32_rtz, [v3f64], 0, [{
    double4 convertIn = (double4)0;
    convertIn.s012 = $Arg0VarName;
    uint4 convertOut = convert_uint4_rtz(convertIn);
    return convertOut.s012;
  }]>;

OclBuiltinImpl fp2int_u32_f64_rtp  = OclBuiltinImpl<convert_u32_rtp, [v3f64], 0, [{
    double4 convertIn = (double4)0;
    convertIn.s012 = $Arg0VarName;
    uint4 convertOut = convert_uint4_rtp(convertIn);
    return convertOut.s012;
  }]>;

OclBuiltinImpl fp2int_u32_f64_rtn  = OclBuiltinImpl<convert_u32_rtn, [v3f64], 0, [{
    double4 convertIn = (double4)0;
    convertIn.s012 = $Arg0VarName;
    uint4 convertOut = convert_uint4_rtn(convertIn);
    return convertOut.s012;
  }]>;

// Workaround for convert_long3(float3 x) functions.

OclBuiltinImpl fp2int_i64_f32  = OclBuiltinImpl<convert_i64, [v3f32], 0, [{
    float4 convertIn = (float4)0;
    convertIn.s012 = $Arg0VarName;
    long4 convertOut = convert_long4(convertIn);
    return convertOut.s012;
  }]>;

OclBuiltinImpl fp2int_i64_f32_rte  = OclBuiltinImpl<convert_i64_rte, [v3f32], 0, [{
    float4 convertIn = (float4)0;
    convertIn.s012 = $Arg0VarName;
    long4 convertOut = convert_long4_rte(convertIn);
    return convertOut.s012;
  }]>;

OclBuiltinImpl fp2int_i64_f32_rtz  = OclBuiltinImpl<convert_i64_rtz, [v3f32], 0, [{
    float4 convertIn = (float4)0;
    convertIn.s012 = $Arg0VarName;
    long4 convertOut = convert_long4_rtz(convertIn);
    return convertOut.s012;
  }]>;

OclBuiltinImpl fp2int_i64_f32_rtp  = OclBuiltinImpl<convert_i64_rtp, [v3f32], 0, [{
    float4 convertIn = (float4)0;
    convertIn.s012 = $Arg0VarName;
    long4 convertOut = convert_long4_rtp(convertIn);
    return convertOut.s012;
  }]>;

OclBuiltinImpl fp2int_i64_f32_rtn  = OclBuiltinImpl<convert_i64_rtn, [v3f32], 0, [{
    float4 convertIn = (float4)0;
    convertIn.s012 = $Arg0VarName;
    long4 convertOut = convert_long4_rtn(convertIn);
    return convertOut.s012;
  }]>;

// Workaround for convert_ulong3(float3 x) functions.

OclBuiltinImpl fp2int_u64_f32  = OclBuiltinImpl<convert_u64, [v3f32], 0, [{
    float4 convertIn = (float4)0;
    convertIn.s012 = $Arg0VarName;
    ulong4 convertOut = convert_ulong4(convertIn);
    return convertOut.s012;
  }]>;

OclBuiltinImpl fp2int_u64_f32_rte  = OclBuiltinImpl<convert_u64_rte, [v3f32], 0, [{
    float4 convertIn = (float4)0;
    convertIn.s012 = $Arg0VarName;
    ulong4 convertOut = convert_ulong4_rte(convertIn);
    return convertOut.s012;
  }]>;

OclBuiltinImpl fp2int_u64_f32_rtz  = OclBuiltinImpl<convert_u64_rtz, [v3f32], 0, [{
    float4 convertIn = (float4)0;
    convertIn.s012 = $Arg0VarName;
    ulong4 convertOut = convert_ulong4_rtz(convertIn);
    return convertOut.s012;
  }]>;

OclBuiltinImpl fp2int_u64_f32_rtp  = OclBuiltinImpl<convert_u64_rtp, [v3f32], 0, [{
    float4 convertIn = (float4)0;
    convertIn.s012 = $Arg0VarName;
    ulong4 convertOut = convert_ulong4_rtp(convertIn);
    return convertOut.s012;
  }]>;

OclBuiltinImpl fp2int_u64_f32_rtn  = OclBuiltinImpl<convert_u64_rtn, [v3f32], 0, [{
    float4 convertIn = (float4)0;
    convertIn.s012 = $Arg0VarName;
    ulong4 convertOut = convert_ulong4_rtn(convertIn);
    return convertOut.s012;
  }]>;

// Workaround for convert_long3(double3 x) functions.

OclBuiltinImpl fp2int_i64_f64  = OclBuiltinImpl<convert_i64, [v3f64], 0, [{
    double4 convertIn = (double4)0;
    convertIn.s012 = $Arg0VarName;
    long4 convertOut = convert_long4(convertIn);
    return convertOut.s012;
  }]>;

OclBuiltinImpl fp2int_i64_f64_rte  = OclBuiltinImpl<convert_i64_rte, [v3f64], 0, [{
    double4 convertIn = (double4)0;
    convertIn.s012 = $Arg0VarName;
    long4 convertOut = convert_long4_rte(convertIn);
    return convertOut.s012;
  }]>;

OclBuiltinImpl fp2int_i64_f64_rtz  = OclBuiltinImpl<convert_i64_rtz, [v3f64], 0, [{
    double4 convertIn = (double4)0;
    convertIn.s012 = $Arg0VarName;
    long4 convertOut = convert_long4_rtz(convertIn);
    return convertOut.s012;
  }]>;

OclBuiltinImpl fp2int_i64_f64_rtp  = OclBuiltinImpl<convert_i64_rtp, [v3f64], 0, [{
    double4 convertIn = (double4)0;
    convertIn.s012 = $Arg0VarName;
    long4 convertOut = convert_long4_rtp(convertIn);
    return convertOut.s012;
  }]>;

OclBuiltinImpl fp2int_i64_f64_rtn  = OclBuiltinImpl<convert_i64_rtn, [v3f64], 0, [{
    double4 convertIn = (double4)0;
    convertIn.s012 = $Arg0VarName;
    long4 convertOut = convert_long4_rtn(convertIn);
    return convertOut.s012;
  }]>;

// Workaround for convert_ulong3(double3 x) functions.

OclBuiltinImpl fp2int_u64_f64  = OclBuiltinImpl<convert_u64, [v3f64], 0, [{
    double4 convertIn = (double4)0;
    convertIn.s012 = $Arg0VarName;
    ulong4 convertOut = convert_ulong4(convertIn);
    return convertOut.s012;
  }]>;

OclBuiltinImpl fp2int_u64_f64_rte  = OclBuiltinImpl<convert_u64_rte, [v3f64], 0, [{
    double4 convertIn = (double4)0;
    convertIn.s012 = $Arg0VarName;
    ulong4 convertOut = convert_ulong4_rte(convertIn);
    return convertOut.s012;
  }]>;

OclBuiltinImpl fp2int_u64_f64_rtz  = OclBuiltinImpl<convert_u64_rtz, [v3f64], 0, [{
    double4 convertIn = (double4)0;
    convertIn.s012 = $Arg0VarName;
    ulong4 convertOut = convert_ulong4_rtz(convertIn);
    return convertOut.s012;
  }]>;

OclBuiltinImpl fp2int_u64_f64_rtp  = OclBuiltinImpl<convert_u64_rtp, [v3f64], 0, [{
    double4 convertIn = (double4)0;
    convertIn.s012 = $Arg0VarName;
    ulong4 convertOut = convert_ulong4_rtp(convertIn);
    return convertOut.s012;
  }]>;

OclBuiltinImpl fp2int_u64_f64_rtn  = OclBuiltinImpl<convert_u64_rtn, [v3f64], 0, [{
    double4 convertIn = (double4)0;
    convertIn.s012 = $Arg0VarName;
    ulong4 convertOut = convert_ulong4_rtn(convertIn);
    return convertOut.s012;
  }]>;

// Workaround for convert_float3() functions.
OclBuiltinImpl cvt_f32_promote = OclBuiltinImpl<convert_f32,
                                          [v3i8, v3u8, v3i16, v3u16, v3i32, v3u32], 0, [{
    $NativeArg0Type x_reg; x_reg$Pattern = $Arg0VarName;
    $NativeReturnType result = convert_float16(x_reg);
    return result.s012;
  }]>;

OclBuiltinImpl cvt_f32_64_promote = OclBuiltinImpl<convert_f32, [v3f64, v3i64, v3u64], 0, [{
    $NativeArg0Type x_reg; x_reg$Pattern = $Arg0VarName;
    float8 result = convert_float8(x_reg);
    return result.s012;
  }]>;

OclBuiltinImpl cvt_f32_rte_promote = OclBuiltinImpl<convert_f32_rte,
                                          [v3i8, v3u8, v3i16, v3u16, v3i32, v3u32], 0, [{
    $NativeArg0Type x_reg; x_reg$Pattern = $Arg0VarName;
    $NativeReturnType result = convert_float16_rte(x_reg);
    return result.s012;
  }]>;

OclBuiltinImpl cvt_f32_64_rte_promote = OclBuiltinImpl<convert_f32_rte, [v3f64, v3i64, v3u64], 0, [{
    $NativeArg0Type x_reg; x_reg$Pattern = $Arg0VarName;
    float8 result = convert_float8_rte(x_reg);
    return result.s012;
  }]>;

OclBuiltinImpl cvt_f32_rtz_promote = OclBuiltinImpl<convert_f32_rtz,
                                          [v3i8, v3u8, v3i16, v3u16, v3i32, v3u32], 0, [{
    $NativeArg0Type x_reg; x_reg$Pattern = $Arg0VarName;
    $NativeReturnType result = convert_float16_rtz(x_reg);
    return result.s012;
  }]>;

OclBuiltinImpl cvt_f32_64_rtz_promote = OclBuiltinImpl<convert_f32_rtz, [v3f64, v3i64, v3u64], 0, [{
    $NativeArg0Type x_reg; x_reg$Pattern = $Arg0VarName;
    float8 result = convert_float8_rtz(x_reg);
    return result.s012;
  }]>;

OclBuiltinImpl cvt_f32_rtp_promote = OclBuiltinImpl<convert_f32_rtp,
                                          [v3i8, v3u8, v3i16, v3u16, v3i32, v3u32], 0, [{
    $NativeArg0Type x_reg; x_reg$Pattern = $Arg0VarName;
    $NativeReturnType result = convert_float16_rtp(x_reg);
    return result.s012;
  }]>;

OclBuiltinImpl cvt_f32_64_rtp_promote = OclBuiltinImpl<convert_f32_rtp, [v3f64, v3i64, v3u64], 0, [{
    $NativeArg0Type x_reg; x_reg$Pattern = $Arg0VarName;
    float8 result = convert_float8_rtp(x_reg);
    return result.s012;
  }]>;

OclBuiltinImpl cvt_f32_rtn_promote = OclBuiltinImpl<convert_f32_rtn,
                                          [v3i8, v3u8, v3i16, v3u16, v3i32, v3u32], 0, [{
    $NativeArg0Type x_reg; x_reg$Pattern = $Arg0VarName;
    $NativeReturnType result = convert_float16_rtn(x_reg);
    return result.s012;
  }]>;

OclBuiltinImpl cvt_f32_64_rtn_promote = OclBuiltinImpl<convert_f32_rtn, [v3f64, v3i64, v3u64], 0, [{
    $NativeArg0Type x_reg; x_reg$Pattern = $Arg0VarName;
    float8 result = convert_float8_rtn(x_reg);
    return result.s012;
  }]>;

// Workaround for convert_double3() functions.
OclBuiltinImpl cvt_f64_promote = OclBuiltinImpl<convert_f64,
                                          [v3i8, v3u8, v3i16, v3u16, v3i32, v3u32, v3i64, v3u64, v3f32], 0, [{
    $NativeArg0Type x_reg; x_reg$Pattern = $Arg0VarName;
    double8 result = convert_double8(x_reg.s01234567);
    return result.s012;
  }]>;

OclBuiltinImpl cvt_f64_rte_promote = OclBuiltinImpl<convert_f64_rte,
                                          [v3i8, v3u8, v3i16, v3u16, v3i32, v3u32, v3i64, v3u64, v3f32], 0, [{
    $NativeArg0Type x_reg; x_reg$Pattern = $Arg0VarName;
    double8 result = convert_double8_rte(x_reg.s01234567);
    return result.s012;
  }]>;

OclBuiltinImpl cvt_f64_rtz_promote = OclBuiltinImpl<convert_f64_rtz,
                                          [v3i8, v3u8, v3i16, v3u16, v3i32, v3u32, v3i64, v3u64, v3f32], 0, [{
    $NativeArg0Type x_reg; x_reg$Pattern = $Arg0VarName;
    $NativeReturnType result = convert_double8_rtz(x_reg.s01234567);
    return result.s012;
  }]>;

OclBuiltinImpl cvt_f64_rtp_promote = OclBuiltinImpl<convert_f64_rtp,
                                          [v3i8, v3u8, v3i16, v3u16, v3i32, v3u32, v3i64, v3u64, v3f32], 0, [{
    $NativeArg0Type x_reg; x_reg$Pattern = $Arg0VarName;
    $NativeReturnType result = convert_double8_rtp(x_reg.s01234567);
    return result.s012;
  }]>;

OclBuiltinImpl cvt_f64_rtn_promote = OclBuiltinImpl<convert_f64_rtn,
                                          [v3i8, v3u8, v3i16, v3u16, v3i32, v3u32, v3i64, v3u64, v3f32], 0, [{
    $NativeArg0Type x_reg; x_reg$Pattern = $Arg0VarName;
    $NativeReturnType result = convert_double8_rtn(x_reg.s01234567);
    return result.s012;
  }]>;

