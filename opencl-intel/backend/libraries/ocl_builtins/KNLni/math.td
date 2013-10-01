//////////////////////////////////////////////////////////////////////////////////////////////
// KNL math implementation
//////////////////////////////////////////////////////////////////////////////////////////////

list<OclType> math_svml_vF_expand = [v16f64];
list<OclType> math_svml_vFpvI32_expand = [v16f64];
list<OclType> math_svml_vFpvF_expand = [v16f64];
list<OclType> math_svml_vF32_expand = [v3f32];
list<OclType> math_svml_vF32vF32_expand = [v3f32];
list<OclType> math_svml_vF64_expand = [v16f64];
list<OclType> math_svml_vFvFvF_expand = [v16f64];
list<OclType> math_svml_vFvF_expand = [v16f64];
list<OclType> math_svml_vFvFpvI32_expand = [v16f64];
list<OclType> math_svml_vFvI32_expand = [v16f64];


// rsqrt
OclBuiltinImpl rsqrt_v16f32_impl = OclBuiltinImpl<rsqrt, [v16f32], 0, [{
    const float16 temp = _mm512_sqrt_ps($Arg0VarName);
    return 1.0f/temp;
  }]>;

OclBuiltinImpl rsqrt_v8f64_impl = OclBuiltinImpl<rsqrt, [v8f64], 0, [{
    return _mm512_rsqrt14_pd($Arg0VarName);
  }]>;

// sqrt
OclBuiltinImpl sqrt_v8f64_impl = OclBuiltinImpl<sqrt, [v8f64], 0, [{
    return _mm512_sqrt_pd($Arg0VarName);
  }]>;

OclBuiltinImpl sqrt_v16f32_impl = OclBuiltinImpl<sqrt, [v16f32], 0, [{
    return _mm512_sqrt_ps($Arg0VarName);
  }]>;

OclBuiltinImpl native_sqrt_v8f64_impl = OclBuiltinImpl<native_sqrt, [v8f64], 0, [{
    return __ocl_svml_b3_sqrt8_native($Arg0VarName);
  }]>;

OclBuiltinImpl native_sqrt_v16f32_impl = OclBuiltinImpl<native_sqrt, [v16f32], 0, [{
    return __ocl_svml_b3_sqrtf16_native($Arg0VarName);
  }]>;

