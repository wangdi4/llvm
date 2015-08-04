OclBuiltinImpl __ocl_HalfToFloatA2 = OclBuiltinImpl<__ocl_HalfToFloat, [v1i16], 0, [{
    short8 x;
    x.s0 = $Arg0VarName;
    return __ocl_Half4ToFloat4(x).s0;
  }]>;

OclBuiltinImpl __ocl_Half4ToFloat4A2 = OclBuiltinImpl<__ocl_Half4ToFloat4, [v8i16], 0, [{
    return _mm_cvtph_ps(__builtin_astype($Arg0VarName, __m128i));
  }]>;

OclBuiltinImpl __ocl_Half8ToFloat8A2 = OclBuiltinImpl<__ocl_Half8ToFloat8, [v8i16], 0, [{
    return _mm256_cvtph_ps(__builtin_astype($Arg0VarName, __m128i));
  }]>;

OclBuiltinImpl __ocl_float2half_rteA2 = OclBuiltinImpl<__ocl_float2half_rte, [v4f32], 0, [{
    return as_$ReturnType(_mm_cvtps_ph($Arg0VarName, _MM_FROUND_TO_NEAREST_INT));
  }]>;

OclBuiltinImpl __ocl_float2half_rtzA2 = OclBuiltinImpl<__ocl_float2half_rtz, [v4f32], 0, [{
    return as_$ReturnType(_mm_cvtps_ph($Arg0VarName, _MM_FROUND_TO_ZERO));
  }]>;

OclBuiltinImpl __ocl_float2half_rtpA2 = OclBuiltinImpl<__ocl_float2half_rtp, [v4f32], 0, [{
    return as_$ReturnType(_mm_cvtps_ph($Arg0VarName, _MM_FROUND_TO_POS_INF));
  }]>;

OclBuiltinImpl __ocl_float2half_rtnA2 = OclBuiltinImpl<__ocl_float2half_rtn, [v4f32], 0, [{
    return as_$ReturnType(_mm_cvtps_ph($Arg0VarName, _MM_FROUND_TO_NEG_INF));
  }]>;

OclBuiltinImpl __ocl_float2half8_rteA2 = OclBuiltinImpl<__ocl_float2half8_rte, [v8f32], 0, [{
    return as_$ReturnType(_mm256_cvtps_ph($Arg0VarName, _MM_FROUND_TO_NEAREST_INT));
  }]>;

OclBuiltinImpl __ocl_float2half8_rtzA2 = OclBuiltinImpl<__ocl_float2half8_rtz, [v8f32], 0, [{
    return as_$ReturnType(_mm256_cvtps_ph($Arg0VarName, _MM_FROUND_TO_ZERO));
  }]>;

OclBuiltinImpl __ocl_float2half8_rtnA2 = OclBuiltinImpl<__ocl_float2half8_rtn, [v8f32], 0, [{
    return as_$ReturnType(_mm256_cvtps_ph($Arg0VarName, _MM_FROUND_TO_NEG_INF));
  }]>;

OclBuiltinImpl __ocl_float2half8_rtpA2 = OclBuiltinImpl<__ocl_float2half8_rtp, [v8f32], 0, [{
    return as_$ReturnType(_mm256_cvtps_ph($Arg0VarName, _MM_FROUND_TO_POS_INF));
  }]>;

OclBuiltinImpl __ocl_double2ToHalf2_rteA2 = OclBuiltinImpl<__ocl_double2ToHalf2_rte, [v2f64], 0, [{
    union { double2 d; long2 ix; } x,j;
    float2 f;
    float4 res;
    short2 h;
    x.d = $Arg0VarName;
    // The key point  is to set the 29 bit to 1 if any of bits 28:0 are 1
    j.ix = x.ix + 0x1fffffff;
    j.ix &= 0x20000000;
    x.ix |= j.ix;
    x.ix &= ~0x1fffffff;
    f = convert_float2_rte(x.d);
    vstore_half2_rte(f, 0, (__private half*)&h);
    res.s0 = as_float(h);
    return res;
  }]>;

OclBuiltinImpl __ocl_double4ToHalf4_rteA2 = OclBuiltinImpl<__ocl_double4ToHalf4_rte, [v4f64], 0, [{
    union { double4 d; long4 ix; } x,j;
    float4 f,res;
    short4 h;
    x.d = $Arg0VarName;
    // The key point  is to set the 29 bit to 1 if any of bits 28:0 are 1
    j.ix = x.ix + 0x1fffffff;
    j.ix &= 0x20000000;
    x.ix |= j.ix;
    x.ix &= ~0x1fffffff;
    f = convert_float4_rte(x.d);
    vstore_half4_rte(f, 0, (__private half*)&h);
    res.s01 = as_float2(h);
    return res;
  }]>;
