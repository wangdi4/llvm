OclBuiltinImpl __ocl_Half16ToFloat16A2 = OclBuiltinImpl<__ocl_Half16ToFloat16, [v16i16], 0, [{
    return _mm512_cvtph_ps(__builtin_astype($Arg0VarName, __m256i));
  }]>;

OclBuiltinImpl __ocl_float2half16_rteA2 = OclBuiltinImpl<__ocl_float2half16_rte, [v16f32], 0, [{
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

code Load_half16 = [{
    float16 res;
    const short16 inp = vload16($Arg0VarName, ($Arg1igentype)$Arg1VarName);
    return __ocl_Half16ToFloat16(inp);
  }];
  
code Storef_half16 = [{
    float8 res = __ocl_float2half16($Arg0VarName);
    vstore16(*(short16*)&res, $Arg1VarName, ($Arg2igentype)$Arg2VarName);
  }];

