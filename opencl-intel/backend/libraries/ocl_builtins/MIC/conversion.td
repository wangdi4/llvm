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
