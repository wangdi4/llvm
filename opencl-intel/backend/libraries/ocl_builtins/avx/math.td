// vim:ts=2:sw=2:et:

//////////////////////////////////////////////////////////////////////////////////////////////
// AVX math implementation
//////////////////////////////////////////////////////////////////////////////////////////////

// Redefine "special care" types inherited from SSE4.2 to make ocl-tblgen use them instead
list<OclType> math_svml_vF_expand = [v16f64];
list<OclType> math_svml_vFpvI32_expand = [v16f64];
list<OclType> math_svml_vFpvF_expand = [v16f64];
list<OclType> math_svml_vF32_expand = [v3f32];
list<OclType> math_svml_vF32vF32_expand = [v3f32, v16f32];
// get back to the generic implementation of FRM built-ins for AVX and its inheritors
list<OclType> math_svml_vFRM_expand = [];


//////////////////////////////////////////////////////////////////////////////////////////////
//                Float f32 section
//////////////////////////////////////////////////////////////////////////////////////////////

// ceil float8
OclBuiltinImpl ceil_v8f32_impl = OclBuiltinImpl<ceil, [v8f32], 0, [{
  return _mm256_ceil_ps($Arg0VarName);
  }]>;

// floor float8
OclBuiltinImpl floor_v8f32_impl = OclBuiltinImpl<floor, [v8f32], 0, [{
  return _mm256_floor_ps($Arg0VarName);
  }]>;

// trunc
OclBuiltinImpl trunc_v8f32_impl = OclBuiltinImpl<trunc, [v8f32], 0, [{
  return _mm256_round_ps($Arg0VarName, _MM_FROUND_TO_ZERO);
  }]>;

// round
OclBuiltinImpl round_v8f32_impl = OclBuiltinImpl<round, [v8f32], 0, [{
  const int mth_signMask = 0x7FFFFFFF;
  float8 xAbs = fabs($Arg0VarName);
  int8 xSign = as_int8($Arg0VarName) & ~mth_signMask;
  float8 xAbsRounded = _mm256_round_ps(xAbs, _MM_FROUND_TO_NEAREST_INT);
  float8 xDiff = xAbs - xAbsRounded;
  xDiff = as_float8(xDiff == 0.5f);
  xDiff = as_float8(as_int8(xDiff) & as_int(1.0f));
  $Arg0VarName = xAbsRounded + xDiff;
  $Arg0VarName = as_float8(as_int8($Arg0VarName) | xSign);
  return $Arg0VarName;
  }]>;

// rint
OclBuiltinImpl rint_v8f32_impl = OclBuiltinImpl<rint, [v8f32], 0, [{
  return _mm256_round_ps($Arg0VarName, _MM_FROUND_TO_NEAREST_INT);
  }]>;

// rsqrt
OclBuiltinImpl rsqrt_v8f32_impl = OclBuiltinImpl<rsqrt, [v8f32], 0, [{
  const float8 temp = _mm256_sqrt_ps($Arg0VarName);
  return 1.0f/temp;
  }]>;

// intel_maskmove()
// non OpenCL builtin. Specialization for AVX
// this builtin
OclBuiltinImpl intel_movemask_v8i32 = OclBuiltinImpl<intel_movemask, [v8i32], 0, [{
    return as_int(_mm256_movemask_ps((__m256)$Arg0VarName));
  }]>;

// sqrt
// leave only implementaion for float4. others sizes are promoted and expanded to float4 version
OclBuiltinImpl sqrt_v8f32_impl = OclBuiltinImpl<sqrt, [v8f32], 0, [{
    return _mm256_sqrt_ps($Arg0VarName);
  }]>;

// native_divide
// leave only implementaion for float4. others sizes are promoted and expanded to float4 version
OclBuiltinImpl native_divide_v8f32_impl = OclBuiltinImpl<native_divide, [v8f32], 0, [{
    $ReturnType $ReturnVarName = _mm256_rcp_ps($Arg1VarName);
    return $Arg0VarName * $ReturnVarName;
  }]>;

// native_recip
// leave only implementaion for float4. others sizes are promoted and expanded to float4 version
OclBuiltinImpl native_recip_v8f32_impl = OclBuiltinImpl<native_recip, [v8f32], 0, [{
    return _mm256_rcp_ps($Arg0VarName);
  }]>;

// native_rsqrt
// leave only implementaion for float4. others sizes are promoted and expanded to float4 version
OclBuiltinImpl native_rsqrt_v8f32_impl = OclBuiltinImpl<native_rsqrt, [v8f32], 0, [{
    return _mm256_rsqrt_ps($Arg0VarName);
  }]>;

// native_sqrt
// leave only implementaion for float4. others sizes are promoted and expanded to float4 version
OclBuiltinImpl native_sqrt_v8f32_impl = OclBuiltinImpl<native_sqrt, [v8f32], 0, [{
    return _mm256_sqrt_ps($Arg0VarName);
  }]>;

//////////////////////////////////////////////////////////////////////////////////////////////
//                Double f64 section
//////////////////////////////////////////////////////////////////////////////////////////////
// ceil
OclBuiltinImpl ceil_v4f64_impl = OclBuiltinImpl<ceil, [v4f64], 0, [{
    return _mm256_ceil_pd($Arg0VarName);
  }]>;

// floor
OclBuiltinImpl floor_v4f64_impl = OclBuiltinImpl<floor, [v4f64], 0, [{
    return _mm256_floor_pd($Arg0VarName);
  }]>;

// trunc
OclBuiltinImpl trunc_v4f64_impl = OclBuiltinImpl<trunc, [v4f64], 0, [{
    return _mm256_round_pd($Arg0VarName, _MM_FROUND_TO_ZERO);
  }]>;

// round
OclBuiltinImpl round_v4f64_impl = OclBuiltinImpl<round, [v4f64], 0, [{
  const long mth_signMask = 0x7FFFFFFFFFFFFFFF;
  double4 xAbs = fabs($Arg0VarName);
  long4 xSign = as_long4($Arg0VarName) & ~mth_signMask;
  double4 xAbsRounded = _mm256_round_pd(xAbs, _MM_FROUND_TO_NEAREST_INT);
  double4 xDiff = xAbs - xAbsRounded;
  xDiff = as_double4(xDiff == 0.5);
  xDiff = as_double4(as_long4(xDiff) & as_long(1.0));
  $Arg0VarName = xAbsRounded + xDiff;
  $Arg0VarName = as_double4(as_long4($Arg0VarName) | xSign);
  return $Arg0VarName;
  }]>;

// rint
OclBuiltinImpl rint_v4f64_impl = OclBuiltinImpl<rint, [v4f64], 0, [{
    return _mm256_round_pd($Arg0VarName, _MM_FROUND_TO_NEAREST_INT);
  }]>;

// intel_maskmove()
// non OpenCL builtin. Specialization for AVX
// this builtin
OclBuiltinImpl intel_movemask_v4i64 = OclBuiltinImpl<intel_movemask, [v4i64], 0, [{
    return as_int(_mm256_movemask_pd((__m256d)$Arg0VarName));
  }]>;

// sqrt
// leave only implementaion to double2. others sizes are calling double2 version
OclBuiltinImpl sqrt_v4f64_impl = OclBuiltinImpl<sqrt, [v4f64], 0, [{
    return _mm256_sqrt_pd($Arg0VarName);
  }]>;


//////////////////////////////////////////////////////////////////////////////////////////////
//         v16f32 and v8f64 svml calls for AVX and higher
//////////////////////////////////////////////////////////////////////////////////////////////
// NOTE ssse3 and sse42 .dll (or .so) doesn't include v16f32 and v8f64,
// but avx and avx2 includes these functions, so we can call them for these
// architectures

list<OclType> avx_types = [v16f32, v8f64];

// acos
OclBuiltinImpl acos_svml = OclBuiltinImpl<acos, avx_types, 0, [{
    return __ocl_svml_$Target_acos$SVMLSuffix($Arg0VarName);
  }]>;

// acosh
OclBuiltinImpl acosh_svml = OclBuiltinImpl<acosh, avx_types, 0, [{
    return __ocl_svml_$Target_acosh$SVMLSuffix($Arg0VarName);
  }]>;

// acospi
OclBuiltinImpl acospi_svml = OclBuiltinImpl<acospi, avx_types, 0, [{
    return __ocl_svml_$Target_acospi$SVMLSuffix($Arg0VarName);
  }]>;

// asin
OclBuiltinImpl asin_svml = OclBuiltinImpl<asin, avx_types, 0, [{
    return __ocl_svml_$Target_asin$SVMLSuffix($Arg0VarName);
  }]>;

// asinh
OclBuiltinImpl asinh_svml = OclBuiltinImpl<asinh, avx_types, 0, [{
    return __ocl_svml_$Target_asinh$SVMLSuffix($Arg0VarName);
  }]>;

// asinpi
OclBuiltinImpl asinpi_svml = OclBuiltinImpl<asinpi, avx_types, 0, [{
    return __ocl_svml_$Target_asinpi$SVMLSuffix($Arg0VarName);
  }]>;

// atan
OclBuiltinImpl atan_svml = OclBuiltinImpl<atan, avx_types, 0, [{
    return __ocl_svml_$Target_atan$SVMLSuffix($Arg0VarName);
  }]>;

// atan2
OclBuiltinImpl atan2_svml = OclBuiltinImpl<atan2, avx_types, 0, [{
    return __ocl_svml_$Target_atan2$SVMLSuffix($Arg0VarName, $Arg1VarName);
  }]>;

// atanh
OclBuiltinImpl atanh_svml = OclBuiltinImpl<atanh, avx_types, 0, [{
    return __ocl_svml_$Target_atanh$SVMLSuffix($Arg0VarName);
  }]>;

// atanpi
OclBuiltinImpl atanpi_svml = OclBuiltinImpl<atanpi, avx_types, 0, [{
    return __ocl_svml_$Target_atanpi$SVMLSuffix($Arg0VarName);
  }]>;

// atan2pi
OclBuiltinImpl atan2pi_svml = OclBuiltinImpl<atan2pi, avx_types, 0, [{
    return __ocl_svml_$Target_atan2pi$SVMLSuffix($Arg0VarName, $Arg1VarName);
  }]>;

// cbrt
OclBuiltinImpl cbrt_svml = OclBuiltinImpl<cbrt, avx_types, 0, [{
    return __ocl_svml_$Target_cbrt$SVMLSuffix($Arg0VarName);
  }]>;

// cos
OclBuiltinImpl cos_svml = OclBuiltinImpl<cos, avx_types, 0, [{
    return __ocl_svml_$Target_cos$SVMLSuffix($Arg0VarName);
  }]>;

// cosh
OclBuiltinImpl cosh_svml = OclBuiltinImpl<cosh, avx_types, 0, [{
    return __ocl_svml_$Target_cosh$SVMLSuffix($Arg0VarName);
  }]>;

// cospi
OclBuiltinImpl cospi_svml = OclBuiltinImpl<cospi, avx_types, 0, [{
    return __ocl_svml_$Target_cospi$SVMLSuffix($Arg0VarName);
  }]>;

// erfc
OclBuiltinImpl erfc_svml = OclBuiltinImpl<erfc, avx_types, 0, [{
    return __ocl_svml_$Target_erfc$SVMLSuffix($Arg0VarName);
  }]>;

// erf
OclBuiltinImpl erf_svml = OclBuiltinImpl<erf, avx_types, 0, [{
    return __ocl_svml_$Target_erf$SVMLSuffix($Arg0VarName);
  }]>;

// exp
OclBuiltinImpl exp_svml = OclBuiltinImpl<exp, avx_types, 0, [{
    return __ocl_svml_$Target_exp$SVMLSuffix($Arg0VarName);
  }]>;

// exp2
OclBuiltinImpl exp2_svml = OclBuiltinImpl<exp2, avx_types, 0, [{
    return __ocl_svml_$Target_exp2$SVMLSuffix($Arg0VarName);
  }]>;

// exp10
OclBuiltinImpl exp10_svml = OclBuiltinImpl<exp10, avx_types, 0, [{
    return __ocl_svml_$Target_exp10$SVMLSuffix($Arg0VarName);
  }]>;

// expm1
OclBuiltinImpl expm1_svml = OclBuiltinImpl<expm1, avx_types, 0, [{
    return __ocl_svml_$Target_expm1$SVMLSuffix($Arg0VarName);
  }]>;

// fmod
OclBuiltinImpl fmod_svml = OclBuiltinImpl<fmod, avx_types, 0, [{
    return __ocl_svml_$Target_fmod$SVMLSuffix($Arg0VarName, $Arg1VarName);
  }]>;

// lgamma
OclBuiltinImpl lgamma_svml = OclBuiltinImpl<lgamma, avx_types, 0, [{
    return __ocl_svml_$Target_lgamma$SVMLSuffix($Arg0VarName);
  }]>;

// log
OclBuiltinImpl log_svml = OclBuiltinImpl<log, avx_types, 0, [{
    return __ocl_svml_$Target_log$SVMLSuffix($Arg0VarName);
  }]>;

// log2
OclBuiltinImpl log2_svml = OclBuiltinImpl<log2, avx_types, 0, [{
    return __ocl_svml_$Target_log2$SVMLSuffix($Arg0VarName);
  }]>;

// log10
OclBuiltinImpl log10_svml = OclBuiltinImpl<log10, avx_types, 0, [{
    return __ocl_svml_$Target_log10$SVMLSuffix($Arg0VarName);
  }]>;

// log1p
OclBuiltinImpl log1p_svml = OclBuiltinImpl<log1p, avx_types, 0, [{
    return __ocl_svml_$Target_log1p$SVMLSuffix($Arg0VarName);
  }]>;

// logb
OclBuiltinImpl logb_svml = OclBuiltinImpl<logb, avx_types, 0, [{
    return __ocl_svml_$Target_logb$SVMLSuffix($Arg0VarName);
  }]>;

// sin
OclBuiltinImpl sin_svml = OclBuiltinImpl<sin, avx_types, 0, [{
    return __ocl_svml_$Target_sin$SVMLSuffix($Arg0VarName);
  }]>;

// sinh
OclBuiltinImpl sinh_svml = OclBuiltinImpl<sinh, avx_types, 0, [{
    return __ocl_svml_$Target_sinh$SVMLSuffix($Arg0VarName);
  }]>;

// sinpi
OclBuiltinImpl sinpi_svml = OclBuiltinImpl<sinpi, avx_types, 0, [{
    return __ocl_svml_$Target_sinpi$SVMLSuffix($Arg0VarName);
  }]>;

// tan
OclBuiltinImpl tan_svml = OclBuiltinImpl<tan, avx_types, 0, [{
    return __ocl_svml_$Target_tan$SVMLSuffix($Arg0VarName);
  }]>;

// tanh
OclBuiltinImpl tanh_svml = OclBuiltinImpl<tanh, avx_types, 0, [{
    return __ocl_svml_$Target_tanh$SVMLSuffix($Arg0VarName);
  }]>;

// tanpi
OclBuiltinImpl tanpi_svml = OclBuiltinImpl<tanpi, avx_types, 0, [{
    return __ocl_svml_$Target_tanpi$SVMLSuffix($Arg0VarName);
  }]>;

//////////////////////////////////////////////////////////////////////////////////////////////
//                SVML ABI hacks for AVX
//  We inherit hacks from SSE42
//////////////////////////////////////////////////////////////////////////////////////////////
