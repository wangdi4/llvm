//////////////////////////////////////////////////////////////////////////////////////////////
// KNL math implementation
//////////////////////////////////////////////////////////////////////////////////////////////


// rsqrt

OclBuiltinImpl native_rsqrt_v1f32_impl = OclBuiltinImpl<native_rsqrt, [v1f32], 0, [{
    float4 tmp;
    tmp.s0 = $Arg0VarName;
    return as_float4(_mm_rsqrt28_round_ss(tmp, tmp, _MM_FROUND_NO_EXC)).s0;
  }]>;

OclBuiltinImpl native_rsqrt_v1f64_impl = OclBuiltinImpl<native_rsqrt, [v1f64], 0, [{
    double2 tmp;
    tmp.s0 = $Arg0VarName;
    return as_double2(_mm_rsqrt28_round_sd(tmp, tmp, _MM_FROUND_NO_EXC)).s0;
  }]>;

OclBuiltinImpl native_rsqrt_v16f32_impl = OclBuiltinImpl<native_rsqrt, [v16f32], 0, [{
    return _mm512_rsqrt28_round_ps($Arg0VarName, _MM_FROUND_NO_EXC);
  }]>;

OclBuiltinImpl native_rsqrt_v8f64_impl = OclBuiltinImpl<native_rsqrt, [v8f64], 0, [{
    return _mm512_rsqrt28_round_pd($Arg0VarName, _MM_FROUND_NO_EXC);
  }]>;

// native_recip
OclBuiltinImpl native_recip_v1f32_impl = OclBuiltinImpl<native_recip, [v1f32], 0, [{
    float4 tmp;
    tmp.s0 = $Arg0VarName;
    return as_float4(_mm_rcp28_round_ss(tmp, tmp, _MM_FROUND_NO_EXC)).s0;
  }]>;

OclBuiltinImpl native_recip_v1f64_impl = OclBuiltinImpl<native_recip, [v1f64], 0, [{
    double2 tmp;
    tmp.s0 = $Arg0VarName;
    return as_double2(_mm_rcp28_round_sd(tmp, tmp, _MM_FROUND_NO_EXC)).s0;
  }]>;

OclBuiltinImpl native_recip_v16f32_impl = OclBuiltinImpl<native_recip, [v16f32], 0, [{
    return _mm512_rcp28_round_ps($Arg0VarName, _MM_FROUND_NO_EXC);
  }]>;

OclBuiltinImpl native_recip_v8f64_impl = OclBuiltinImpl<native_recip, [v8f64], 0, [{
    return _mm512_rcp28_round_pd($Arg0VarName, _MM_FROUND_NO_EXC);
  }]>;

// sqrt
OclBuiltinImpl native_sqrt_v8f64_impl = OclBuiltinImpl<native_sqrt, [v8f64], 0, [{
    return _mm512_rcp28_round_pd(native_rsqrt($Arg0VarName), _MM_FROUND_NO_EXC);
  }]>;

OclBuiltinImpl native_sqrt_v1f32_impl = OclBuiltinImpl<native_sqrt, [v1f32], 0, [{
    float4 tmp;
    tmp.s0 = $Arg0VarName;
    tmp = native_rsqrt(tmp);
    return as_float4(_mm_rcp28_round_ss(tmp, tmp, _MM_FROUND_NO_EXC)).s0;
  }]>;

OclBuiltinImpl native_sqrt_v1f64_impl = OclBuiltinImpl<native_sqrt, [v1f64], 0, [{
    double2 tmp;
    tmp.s0 = $Arg0VarName;
    tmp = native_rsqrt(tmp);
    return as_double2(_mm_rcp28_round_sd(tmp, tmp, _MM_FROUND_NO_EXC)).s0;
  }]>;

