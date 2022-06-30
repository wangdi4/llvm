// vim:ts=2:sw=2:et:

//////////////////////////////////////////////////////////////////////////////////////////////
// AVX math implementation
//////////////////////////////////////////////////////////////////////////////////////////////

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
OclBuiltinImpl rint_v4f64_impl = OclBuiltinImpl<rint, [v4f64], 0, [{
  return _mm256_round_pd($Arg0VarName, _MM_FROUND_TO_NEAREST_INT);
  }]>;
OclBuiltinImpl rint_v8f32_impl = OclBuiltinImpl<rint, [v8f32], 0, [{
  return _mm256_round_ps($Arg0VarName, _MM_FROUND_TO_NEAREST_INT);
  }]>;

// rsqrt
/*
OclBuiltinImpl rsqrt_v8f32_impl = OclBuiltinImpl<rsqrt, [v8f32], 0, [{
  const float8 temp = _mm256_sqrt_ps($Arg0VarName);
  return 1.0f/temp;
  }]>;
*/

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
//                SVML ABI hacks for AVX
//  We inherit hacks from SSE42
//////////////////////////////////////////////////////////////////////////////////////////////
