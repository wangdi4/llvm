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

OclBuiltinImpl mic_rtz_cvt_f32_f64 = OclBuiltinImpl<convert_f32_rtz,
                                                    [v1f64, v2f64, v3f64, v4f64, v8f64, v16f64], 0, [{
    return convert_$ReturnType($Arg0VarName);
  }]>;

OclBuiltinImpl mic_rtp_cvt_f32_f64 = OclBuiltinImpl<convert_f32_rtp,
                                                    [v1f64, v2f64, v3f64, v4f64, v8f64, v16f64], 0, [{
    return convert_$ReturnType($Arg0VarName);
  }]>;

OclBuiltinImpl mic_rtn_cvt_f32_f64 = OclBuiltinImpl<convert_f32_rtn,
                                                    [v1f64, v2f64, v3f64, v4f64, v8f64, v16f64], 0, [{
    return convert_$ReturnType($Arg0VarName);
  }]>;

OclBuiltinImpl cvt_f32_i8i16 = OclBuiltinImpl<convert_f32,
                                              [ v1i8,  v2i8,  v3i8,  v4i8,  v8i8,
                                               v1i16, v2i16, v3i16, v4i16, v8i16,
                                               v1i32, v2i32, v3i32, v4i32, v8i32], 0, [{
    return __ocl_svml_$Target_cvti64tofp$rtn$SVMLSuffix(
            as_$Arg0i64gentype(
              sext_$Arg0Symigentype_$Arg0Symi64gentype(
                as_$Arg0igentype($Arg0VarName))));
  }]>;

OclBuiltinImpl cvt_f32_u8u16 = OclBuiltinImpl<convert_f32,
                                              [ v1u8,  v2u8,  v3u8,  v4u8,  v8u8,
                                               v1u16, v2u16, v3u16, v4u16, v8u16,
                                               v1u32, v2u32, v3u32, v4u32, v8u32], 0, [{
    return __ocl_svml_$Target_cvtu64tofp$rtn$SVMLSuffix(
            as_$Arg0u64gentype(
              zext_$Arg0Symigentype_$Arg0Symi64gentype(
                as_$Arg0igentype($Arg0VarName))));
  }]>;

