OclBuiltinImpl __ocl_Half16ToFloat16A2 = OclBuiltinImpl<__ocl_Half16ToFloat16, [v16i16], 0, [{
    return _mm512_cvtph_ps(__builtin_astype($Arg0VarName, __m256i));
  }]>;

OclBuiltinImpl __ocl_float2half16_A2 = OclBuiltinImpl<__ocl_float2half16, [v16f32], 0, [{
    return as_$ReturnType(_mm512_cvtps_ph($Arg0VarName, _MM_FROUND_TO_NEAREST_INT));
  }]>;

OclBuiltinImpl __ocl_float2half16_rteA2 = OclBuiltinImpl<__ocl_float2half16_rte,
[v16f32], 0, [{
    return as_$ReturnType(_mm512_cvtps_ph($Arg0VarName, _MM_FROUND_TO_NEAREST_INT));
  }]>;

OclBuiltinImpl __ocl_float2half16_rtzA2 = OclBuiltinImpl<__ocl_float2half16_rtz, [v16f32], 0, [{
    return as_$ReturnType(_mm512_cvtps_ph($Arg0VarName, _MM_FROUND_TO_ZERO));
  }]>;

OclBuiltinImpl __ocl_float2half16_rtnA2 = OclBuiltinImpl<__ocl_float2half16_rtn, [v16f32], 0, [{
    return as_$ReturnType(_mm512_cvtps_ph($Arg0VarName, _MM_FROUND_TO_NEG_INF));
  }]>;

OclBuiltinImpl __ocl_float2half16_rtpA2 = OclBuiltinImpl<__ocl_float2half16_rtp, [v16f32], 0, [{
    return as_$ReturnType(_mm512_cvtps_ph($Arg0VarName, _MM_FROUND_TO_POS_INF));
  }]>;

OclBuiltinImpl __ocl_double2half8_rteA2 = OclBuiltinImpl<__ocl_double8ToHalf8_rte, [v8f64], 0, [{
    float8 f8 = _mm512_cvt_roundpd_ps($Arg0VarName,
                                      _MM_FROUND_NO_EXC | _MM_FROUND_TO_NEAREST_INT);
    return as_$ReturnType(_mm256_cvtps_ph(f8, _MM_FROUND_TO_NEAREST_INT));
  }]>;

OclBuiltinImpl __ocl_double2half8_rtpA2 = OclBuiltinImpl<__ocl_double8ToHalf8_rtp, [v8f64], 0, [{
    float8 f8 = _mm512_cvt_roundpd_ps($Arg0VarName,
                                      _MM_FROUND_NO_EXC | _MM_FROUND_TO_POS_INF);
    return as_$ReturnType(_mm256_cvtps_ph(f8, _MM_FROUND_TO_POS_INF));
  }]>;

OclBuiltinImpl __ocl_double2half8_rtnA2 = OclBuiltinImpl<__ocl_double8ToHalf8_rtn, [v8f64], 0, [{
    float8 f8 = _mm512_cvt_roundpd_ps($Arg0VarName,
                                      _MM_FROUND_NO_EXC | _MM_FROUND_TO_NEG_INF);
    return as_$ReturnType(_mm256_cvtps_ph(f8, _MM_FROUND_TO_NEG_INF));
  }]>;

OclBuiltinImpl __ocl_double2half8_rtzA2 = OclBuiltinImpl<__ocl_double8ToHalf8_rtz, [v8f64], 0, [{
    float8 f8 = _mm512_cvt_roundpd_ps($Arg0VarName,
                                      _MM_FROUND_NO_EXC | _MM_FROUND_TO_ZERO);
    return as_$ReturnType(_mm256_cvtps_ph(f8, _MM_FROUND_TO_ZERO));
  }]>;
  
code Storef_half16 = [{
    float8 res = __ocl_float2half16($Arg0VarName);
    vstore16(*(short16*)&res, $Arg1VarName, ($Arg2igentype)$Arg2VarName);
  }];

 code Stored_half8 = [{
    $Arg2VarName = $Arg2VarName + ($Arg1VarName*8);
    float4 f4 = __ocl_double8ToHalf8_rte($Arg0VarName);
    _mm_storeu_ps((float*)$Arg2VarName, f4);
  }];

code Stored_half16 = [{
    $Arg2VarName = $Arg2VarName + ($Arg1VarName*16);
    float4 t1 = __ocl_double8ToHalf8_rte($Arg0VarName.lo);
    float4 t2 = __ocl_double8ToHalf8_rte($Arg0VarName.hi);
    _mm_storeu_ps((float*)$Arg2VarName, t1);
    $Arg2VarName = $Arg2VarName + 8;
    _mm_storeu_ps((float*)$Arg2VarName, t2);
  }];

code Stored_half_rte16 = [{
    $Arg2VarName = $Arg2VarName + ($Arg1VarName*16);
    float4 t1 = __ocl_double8ToHalf8_rte($Arg0VarName.lo);
    float4 t2 = __ocl_double8ToHalf8_rte($Arg0VarName.hi);
    _mm_storeu_ps((float*)$Arg2VarName, t1);
    $Arg2VarName = $Arg2VarName + 8;
    _mm_storeu_ps((float*)$Arg2VarName, t2);
  }];

code Stored_half_rtp16 = [{
    $Arg2VarName = $Arg2VarName + ($Arg1VarName*16);
    float4 t1 = __ocl_double8ToHalf8_rtp($Arg0VarName.lo);
    float4 t2 = __ocl_double8ToHalf8_rtp($Arg0VarName.hi);
    _mm_storeu_ps((float*)$Arg2VarName, t1);
    $Arg2VarName = $Arg2VarName + 8;
    _mm_storeu_ps((float*)$Arg2VarName, t2);
  }];

code Stored_half_rtn16 = [{
    $Arg2VarName = $Arg2VarName + ($Arg1VarName*16);
    float4 t1 = __ocl_double8ToHalf8_rtn($Arg0VarName.lo);
    float4 t2 = __ocl_double8ToHalf8_rtn($Arg0VarName.hi);
    _mm_storeu_ps((float*)$Arg2VarName, t1);
    $Arg2VarName = $Arg2VarName + 8;
    _mm_storeu_ps((float*)$Arg2VarName, t2);
  }];

code Stored_half_rtz16 = [{
    $Arg2VarName = $Arg2VarName + ($Arg1VarName*16);
    float4 t1 = __ocl_double8ToHalf8_rtz($Arg0VarName.lo);
    float4 t2 = __ocl_double8ToHalf8_rtz($Arg0VarName.hi);
    _mm_storeu_ps((float*)$Arg2VarName, t1);
    $Arg2VarName = $Arg2VarName + 8;
    _mm_storeu_ps((float*)$Arg2VarName, t2);
  }];

code Storef_half_rte16 = [{
    float8 res = __ocl_float2half16_rte($Arg0VarName);
    vstore16(*(short16*)&res, $Arg1VarName, ($Arg2igentype)$Arg2VarName);
  }];

code Storef_half_rtp16 = [{
    float8 res = __ocl_float2half16_rtp($Arg0VarName);
    vstore16(*(short16*)&res, $Arg1VarName, ($Arg2igentype)$Arg2VarName);
  }];

code Storef_half_rtn16 = [{
    float8 res = __ocl_float2half16_rtn($Arg0VarName);
    vstore16(*(short16*)&res, $Arg1VarName, ($Arg2igentype)$Arg2VarName);
  }];

code Storef_half_rtz16 = [{
    float8 res = __ocl_float2half16_rtz($Arg0VarName);
    vstore16(*(short16*)&res, $Arg1VarName, ($Arg2igentype)$Arg2VarName);
  }];

code Stored_half_rtp8 = [{
    $Arg2VarName = $Arg2VarName + ($Arg1VarName*8);
    float8 f8 = _mm512_cvt_roundpd_ps($Arg0VarName,
                                      _MM_FROUND_NO_EXC | _MM_FROUND_TO_POS_INF);
    float4 f4 = as_float4(_mm256_cvtps_ph(f8, _MM_FROUND_TO_POS_INF));
    _mm_storeu_ps((float*)$Arg2VarName, f4);
  }];

code Stored_half_rtn8 = [{
    $Arg2VarName = $Arg2VarName + ($Arg1VarName*8);
    float8 f8 = _mm512_cvt_roundpd_ps($Arg0VarName,
                                      _MM_FROUND_NO_EXC | _MM_FROUND_TO_NEG_INF);
    float4 f4 = as_float4(_mm256_cvtps_ph(f8, _MM_FROUND_TO_NEG_INF));
    _mm_storeu_ps((float*)$Arg2VarName, f4);
  }];

code Stored_half_rte8 = [{
    $Arg2VarName = $Arg2VarName + ($Arg1VarName*8);
    float8 f8 = _mm512_cvt_roundpd_ps($Arg0VarName,
                                      _MM_FROUND_NO_EXC | _MM_FROUND_TO_NEAREST_INT);
    float4 f4 = as_float4(_mm256_cvtps_ph(f8, _MM_FROUND_TO_NEAREST_INT));
    _mm_storeu_ps((float*)$Arg2VarName, f4);
  }];

code Stored_half_rtz8 = [{
    $Arg2VarName = $Arg2VarName + ($Arg1VarName*8);
    float8 f8 = _mm512_cvt_roundpd_ps($Arg0VarName,
                                      _MM_FROUND_NO_EXC | _MM_FROUND_TO_ZERO);
    float4 f4 = as_float4(_mm256_cvtps_ph(f8, _MM_FROUND_TO_ZERO));
    _mm_storeu_ps((float*)$Arg2VarName, f4);
  }];

code Load_half8 = [{
    short8 tmp = *((short8 *)($Arg1VarName + 8*$Arg0VarName));
    return __ocl_Half8ToFloat8(tmp);
  }];

code Load_half16 = [{
    short16 tmp = *((short16 *)($Arg1VarName + 16*$Arg0VarName));
    return __ocl_Half16ToFloat16(tmp);
  }];
