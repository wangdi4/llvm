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
    union { double8 d; long8 ix; } x, j;
    float8 f;
    short8 h;
    x.d = $Arg0VarName;
    // The key point  is to set the 29 bit to 1 if any of bits 28:0 are 1
    j.ix = x.ix + 0x1fffffff;
    j.ix &= 0x20000000;
    x.ix |= j.ix;
    x.ix &= ~0x1fffffff;
    f = convert_float8_rte(x.d);
    vstore_half8_rte(f, 0, (__private half*)&h);
    return as_float4(h);
//    float8 f8 = _mm512_cvt_roundpd_ps($Arg0VarName,
//                                      _MM_FROUND_NO_EXC | _MM_FROUND_TO_NEAREST_INT);
//    return as_$ReturnType(_mm256_cvtps_ph(f8, _MM_FROUND_TO_NEAREST_INT));
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
  
code Stored_half8 = [{
    $Arg2VarName = $Arg2VarName + ($Arg1VarName*8);
    float4 f4 = __ocl_double8ToHalf8_rte($Arg0VarName);
    _mm_storeu_ps((__private float*)(float*)$Arg2VarName, f4);
  }];

code Stored_half16 = [{
    $Arg2VarName = $Arg2VarName + ($Arg1VarName*16);
    float8 f8;
    f8.lo = __ocl_double8ToHalf8_rte($Arg0VarName.lo);
    f8.hi = __ocl_double8ToHalf8_rte($Arg0VarName.hi);
    _mm256_storeu_ps((float*)$Arg2VarName, f8);
  }];

code Stored_half_rte16 = [{
    $Arg2VarName = $Arg2VarName + ($Arg1VarName*16);
    float8 f8;
    f8.lo = __ocl_double8ToHalf8_rte($Arg0VarName.lo);
    f8.hi = __ocl_double8ToHalf8_rte($Arg0VarName.hi);
    _mm256_storeu_ps((float*)$Arg2VarName, f8);
  }];

code Stored_half_rtp16 = [{
    $Arg2VarName = $Arg2VarName + ($Arg1VarName*16);
    float8 f8;
    f8.lo = __ocl_double8ToHalf8_rtp($Arg0VarName.lo);
    f8.hi = __ocl_double8ToHalf8_rtp($Arg0VarName.hi);
    _mm256_storeu_ps((float*)$Arg2VarName, f8);
  }];

code Stored_half_rtn16 = [{
    $Arg2VarName = $Arg2VarName + ($Arg1VarName*16);
    float8 f8;
    f8.lo = __ocl_double8ToHalf8_rtn($Arg0VarName.lo);
    f8.hi = __ocl_double8ToHalf8_rtn($Arg0VarName.hi);
    _mm256_storeu_ps((float*)$Arg2VarName, f8);
  }];

code Stored_half_rtz16 = [{
    $Arg2VarName = $Arg2VarName + ($Arg1VarName*16);
    float8 f8;
    f8.lo = __ocl_double8ToHalf8_rtz($Arg0VarName.lo);
    f8.hi = __ocl_double8ToHalf8_rtz($Arg0VarName.hi);
    _mm256_storeu_ps((float*)$Arg2VarName, f8);
  }];

code Storef_half16 = [{
    $Arg2VarName = $Arg2VarName + ($Arg1VarName*16);
    float8 f8 = __ocl_float2half16($Arg0VarName);
    _mm256_storeu_ps((float*)$Arg2VarName, f8);
  }];

code Storef_half8 = [{
    $Arg2VarName = $Arg2VarName + ($Arg1VarName*8);
    float4 f4 = __ocl_float2half8($Arg0VarName);
    _mm_storeu_ps((__private float*)(float*)$Arg2VarName, f4);
  }];

code Storef_half_rte8 = [{
    $Arg2VarName = $Arg2VarName + ($Arg1VarName*8);
    float4 f4 = __ocl_float2half8_rte($Arg0VarName);
    _mm_storeu_ps((__private float*)(float*)$Arg2VarName, f4);
  }];

code Storef_half_rtp8 = [{
    $Arg2VarName = $Arg2VarName + ($Arg1VarName*8);
    float4 f4 = __ocl_float2half8_rtp($Arg0VarName);
    _mm_storeu_ps((__private float*)(float*)$Arg2VarName, f4);
  }];

code Storef_half_rtn8 = [{
    $Arg2VarName = $Arg2VarName + ($Arg1VarName*8);
    float4 f4 = __ocl_float2half8_rtn($Arg0VarName);
    _mm_storeu_ps((__private float*)(float*)$Arg2VarName, f4);
  }];

code Storef_half_rtz8 = [{
    $Arg2VarName = $Arg2VarName + ($Arg1VarName*8);
    float4 f4 = __ocl_float2half8_rtz($Arg0VarName);
    _mm_storeu_ps((__private float*)(float*)$Arg2VarName, f4);
  }];

code Storef_half_rte16 = [{
    $Arg2VarName = $Arg2VarName + ($Arg1VarName*16);
    float8 f8 = __ocl_float2half16_rte($Arg0VarName);
    _mm256_storeu_ps((float*)$Arg2VarName, f8);
  }];

code Storef_half_rtp16 = [{
    $Arg2VarName = $Arg2VarName + ($Arg1VarName*16);
    float8 f8 = __ocl_float2half16_rtp($Arg0VarName);
    _mm256_storeu_ps((float*)$Arg2VarName, f8);
  }];

code Storef_half_rtn16 = [{
    $Arg2VarName = $Arg2VarName + ($Arg1VarName*16);
    float8 f8 = __ocl_float2half16_rtn($Arg0VarName);
    _mm256_storeu_ps((float*)$Arg2VarName, f8);
  }];

code Storef_half_rtz16 = [{
    $Arg2VarName = $Arg2VarName + ($Arg1VarName*16);
    float8 f8 = __ocl_float2half16_rtz($Arg0VarName);
    _mm256_storeu_ps((float*)$Arg2VarName, f8);
  }];

code Stored_half_rtp8 = [{
    $Arg2VarName = $Arg2VarName + ($Arg1VarName*8);
    float8 f8 = _mm512_cvt_roundpd_ps($Arg0VarName,
                                      _MM_FROUND_NO_EXC | _MM_FROUND_TO_POS_INF);
    float4 f4 = as_float4(_mm256_cvtps_ph(f8, _MM_FROUND_TO_POS_INF));
    _mm_storeu_ps((__private float*)(float*)$Arg2VarName, f4);
  }];

code Stored_half_rtn8 = [{
    $Arg2VarName = $Arg2VarName + ($Arg1VarName*8);
    float8 f8 = _mm512_cvt_roundpd_ps($Arg0VarName,
                                      _MM_FROUND_NO_EXC | _MM_FROUND_TO_NEG_INF);
    float4 f4 = as_float4(_mm256_cvtps_ph(f8, _MM_FROUND_TO_NEG_INF));
    _mm_storeu_ps((__private float*)(float*)$Arg2VarName, f4);
  }];

code Stored_half_rte8 = [{
    $Arg2VarName = $Arg2VarName + ($Arg1VarName*8);
    float4 f4 = __ocl_double8ToHalf8_rte($Arg0VarName);
    _mm_storeu_ps((__private float*)(float*)$Arg2VarName, f4);
  }];

code Stored_half_rtz8 = [{
    $Arg2VarName = $Arg2VarName + ($Arg1VarName*8);
    float8 f8 = _mm512_cvt_roundpd_ps($Arg0VarName,
                                      _MM_FROUND_NO_EXC | _MM_FROUND_TO_ZERO);
    float4 f4 = as_float4(_mm256_cvtps_ph(f8, _MM_FROUND_TO_ZERO));
    _mm_storeu_ps((__private float*)(float*)$Arg2VarName, f4);
  }];

// Aligned store
code Storead_half8 = [{
    $Arg2VarName = $Arg2VarName + ($Arg1VarName*8);
    float4 f4 = __ocl_double8ToHalf8_rte($Arg0VarName);
    _mm_store_ps((float*)$Arg2VarName, f4);
  }];

code Storead_half_rte8 = [{
    $Arg2VarName = $Arg2VarName + ($Arg1VarName*8);
    float4 f4 = __ocl_double8ToHalf8_rte($Arg0VarName);
    _mm_store_ps((float*)$Arg2VarName, f4);
  }];

code Storead_half_rtn8 = [{
    $Arg2VarName = $Arg2VarName + ($Arg1VarName*8);
    float4 f4 = __ocl_double8ToHalf8_rtn($Arg0VarName);
    _mm_store_ps((float*)$Arg2VarName, f4);
  }];

code Storead_half_rtp8 = [{
    $Arg2VarName = $Arg2VarName + ($Arg1VarName*8);
    float4 f4 = __ocl_double8ToHalf8_rtp($Arg0VarName);
    _mm_store_ps((float*)$Arg2VarName, f4);
  }];

code Storead_half_rtz8 = [{
    $Arg2VarName = $Arg2VarName + ($Arg1VarName*8);
    float4 f4 = __ocl_double8ToHalf8_rtz($Arg0VarName);
    _mm_store_ps((float*)$Arg2VarName, f4);
  }];

code Storead_half16 = [{
    $Arg2VarName = $Arg2VarName + ($Arg1VarName*16);
    float8 f8;
    f8.lo = __ocl_double8ToHalf8_rte($Arg0VarName.lo);
    f8.hi = __ocl_double8ToHalf8_rte($Arg0VarName.hi);
    _mm256_store_ps((float*)$Arg2VarName, f8);
  }];

code Storead_half_rte16 = [{
    $Arg2VarName = $Arg2VarName + ($Arg1VarName*16);
    float8 f8;
    f8.lo = __ocl_double8ToHalf8_rte($Arg0VarName.lo);
    f8.hi = __ocl_double8ToHalf8_rte($Arg0VarName.hi);
    _mm256_store_ps((float*)$Arg2VarName, f8);
  }];

code Storead_half_rtn16 = [{
    $Arg2VarName = $Arg2VarName + ($Arg1VarName*16);
    float8 f8;
    f8.lo = __ocl_double8ToHalf8_rtn($Arg0VarName.lo);
    f8.hi = __ocl_double8ToHalf8_rtn($Arg0VarName.hi);
    _mm256_store_ps((float*)$Arg2VarName, f8);
  }];

code Storead_half_rtp16 = [{
    $Arg2VarName = $Arg2VarName + ($Arg1VarName*16);
    float8 f8;
    f8.lo = __ocl_double8ToHalf8_rtp($Arg0VarName.lo);
    f8.hi = __ocl_double8ToHalf8_rtp($Arg0VarName.hi);
    _mm256_store_ps((float*)$Arg2VarName, f8);
  }];

code Storead_half_rtz16 = [{
    $Arg2VarName = $Arg2VarName + ($Arg1VarName*16);
    float8 f8;
    f8.lo = __ocl_double8ToHalf8_rtz($Arg0VarName.lo);
    f8.hi = __ocl_double8ToHalf8_rtz($Arg0VarName.hi);
    _mm256_store_ps((float*)$Arg2VarName, f8);
  }];

code Load_half8 = [{
    const short8 inp = vload8($Arg0VarName, ($Arg1igentype)$Arg1VarName);
    return __ocl_Half8ToFloat8(inp);
  }];

code Load_half16 = [{
    float16 res;
    const short16 inp = vload16($Arg0VarName, ($Arg1igentype)$Arg1VarName);
    res.lo = __ocl_Half8ToFloat8(inp.lo);
    res.hi = __ocl_Half8ToFloat8(inp.hi);
    return res;
  }];
