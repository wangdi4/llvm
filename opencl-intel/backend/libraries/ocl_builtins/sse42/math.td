// vim:ts=2:sw=2:et:


//////////////////////////////////////////////////////////////////////////////////////////////
//                Common f32 and f64 section
//////////////////////////////////////////////////////////////////////////////////////////////

// half_recip
// route call to native. Specific to CPU(not-MIC) implementation. 
// Assuming native_ version gives needed accuracy
OclBuiltinImpl half_recip_impl = OclBuiltinImpl<half_recip, half_recip.Types, 0, [{
    return native_recip($Arg0VarName);
  }]>;

// half_rsqrt
// route call to native. Specific to CPU(not-MIC) implementation. 
// Assuming native_ version gives needed accuracy
OclBuiltinImpl half_rsqrt_impl = OclBuiltinImpl<half_rsqrt, half_rsqrt.Types, 0, [{
    return native_rsqrt($Arg0VarName);
  }]>;

// half_sqrt
// route call to native. Specific to CPU(not-MIC) implementation. 
// Assuming native_ version gives needed accuracy
OclBuiltinImpl half_sqrt_impl = OclBuiltinImpl<half_sqrt, half_sqrt.Types, 0, [{
    return native_sqrt($Arg0VarName);
  }]>;

//////////////////////////////////////////////////////////////////////////////////////////////
//                Float f32 section
//////////////////////////////////////////////////////////////////////////////////////////////
// fabs
// leave only implementaion for float4. float, float2, float3 are promoted to float4
OclBuiltinImpl fabs_f32_promote = OclBuiltinImpl<fabs, [v1f32, v2f32, v3f32], 0, Promote1>;

// fmin
// leave only implementaion for float4. float, float2, float3 are promoted to float4
OclBuiltinImpl fmin_f32_promote = OclBuiltinImpl<fmin, [v1f32, v2f32, v3f32], 0, Promote2>;

// fmax
// leave only implementaion for float4. float, float2, float3 are promoted to float4
OclBuiltinImpl fmax_f32_promote = OclBuiltinImpl<fmax, [v1f32, v2f32, v3f32], 0, Promote2>;

// minmag
// leave only implementaion for float4. float, float2, float3 are promoted to float4
OclBuiltinImpl minmag_f32_promote = OclBuiltinImpl<minmag, [v1f32, v2f32, v3f32], 0, Promote2>;

// maxmag
// leave only implementaion for float4. float, float2, float3 are promoted to float4
OclBuiltinImpl maxmag_f32_promote = OclBuiltinImpl<maxmag, [v1f32, v2f32, v3f32], 0, Promote2>;

// ceil
OclBuiltinImpl ceil_f32_impl = OclBuiltinImpl<ceil, [v4f32], 0, [{
	return _mm_ceil_ps($Arg0VarName);
  }]>;
// leave only implementaion for float4. others sizes are promoted and expanded to float4 version
OclBuiltinImpl ceil_f32_promote = OclBuiltinImpl<ceil, [v1f32, v2f32, v3f32], 0, Promote1>;
OclBuiltinImpl ceil_f32_expand = OclBuiltinImpl<ceil, [v8f32, v16f32], 0, Expand1>;

// floor
OclBuiltinImpl floor_f32_impl = OclBuiltinImpl<floor, [v4f32], 0, [{
	return _mm_floor_ps($Arg0VarName);
  }]>;
// leave only implementaion for float4. others sizes are promoted and expanded to float4 version
OclBuiltinImpl floor_f32_promote = OclBuiltinImpl<floor, [v1f32, v2f32, v3f32], 0, Promote1>;
OclBuiltinImpl floor_f32_expand = OclBuiltinImpl<floor, [v8f32, v16f32], 0, Expand1>;

// trunc
OclBuiltinImpl trunc_f32_impl = OclBuiltinImpl<trunc, [v4f32], 0, [{
	return _mm_round_ps($Arg0VarName, _MM_FROUND_TO_ZERO);
  }]>;
// leave only implementaion for float4. others sizes are promoted and expanded to float4 version
OclBuiltinImpl trunc_f32_promote = OclBuiltinImpl<trunc, [v1f32, v2f32, v3f32], 0, Promote1>;
OclBuiltinImpl trunc_f32_expand = OclBuiltinImpl<trunc, [v8f32, v16f32], 0, Expand1>;

// round
OclBuiltinImpl round_f32_impl = OclBuiltinImpl<round, [v4f32], 0, [{
	const int mth_signMask = 0x7FFFFFFF;
	float4 xAbs = fabs($Arg0VarName);
	int4 xSign = as_int4($Arg0VarName) & ~mth_signMask;
	float4 xAbsRounded = _mm_round_ps(xAbs, _MM_FROUND_TO_NEAREST_INT);
	float4 xDiff = xAbs - xAbsRounded;
	xDiff = as_float4(xDiff == 0.5f);
	xDiff = as_float4(as_int4(xDiff) & as_int(1.0f));
	$Arg0VarName = xAbsRounded + xDiff;
	$Arg0VarName = as_float4(as_int4($Arg0VarName) | xSign);
	return $Arg0VarName;
  }]>;
// leave only implementaion for float4. others sizes are promoted and expanded to float4 version
OclBuiltinImpl round_f32_promote = OclBuiltinImpl<round, [v1f32, v2f32, v3f32], 0, Promote1>;
OclBuiltinImpl round_f32_expand = OclBuiltinImpl<round, [v8f32, v16f32], 0, Expand1>;

// rint
OclBuiltinImpl rint_f32_impl = OclBuiltinImpl<rint, [v4f32], 0, [{
	return _mm_round_ps($Arg0VarName, _MM_FROUND_TO_NEAREST_INT);
  }]>;
// leave only implementaion for float4. others sizes are promoted and expanded to float4 version
OclBuiltinImpl rint_f32_promote = OclBuiltinImpl<rint, [v1f32, v2f32, v3f32], 0, Promote1>;
OclBuiltinImpl rint_f32_expand = OclBuiltinImpl<rint, [v8f32, v16f32], 0, Expand1>;

// rsqrt
OclBuiltinImpl rsqrt_f32_impl = OclBuiltinImpl<rsqrt, [v4f32], 0, [{
	const float4 temp = _mm_sqrt_ps($Arg0VarName);
	return 1.0f/temp;
  }]>;
// leave only implementaion for float4. others sizes are promoted and expanded to float4 version
OclBuiltinImpl rsqrt_f32_promote = OclBuiltinImpl<rsqrt, [v1f32, v2f32, v3f32], 0, Promote1>;
OclBuiltinImpl rsqrt_f32_expand = OclBuiltinImpl<rsqrt, [v8f32, v16f32], 0, Expand1>;

// copysign
// leave only implementaion for float4. float, float2, float3 are promoted to float4
OclBuiltinImpl copysign_f32_promote = OclBuiltinImpl<copysign, [v1f32, v2f32, v3f32], 0, Promote2>;

// fdim
// leave only implementaion for float4. float, float2, float3 are promoted to float4
OclBuiltinImpl fdim_f32_promote = OclBuiltinImpl<fdim, [v1f32, v2f32, v3f32], 0, Promote2>;

// ilogb
// leave only implementaion for float4. float, float2, float3 are promoted to float4
OclBuiltinImpl ilogb_f32_promote = OclBuiltinImpl<ilogb, [v1f32, v2f32, v3f32], 0, Promote1>;

// nan
// leave only implementaion for float4. float, float2, float3 are promoted to float4
OclBuiltinImpl nan_f32_promote = OclBuiltinImpl<nan, [v1f32, v2f32, v3f32], 0, Promote1>;

// intel_maskmove() 
// non OpenCL builtin. Specialization for SSE42
// this builtin
OclBuiltinImpl intel_movemask_v4i32 = OclBuiltinImpl<intel_movemask, [v4i32], 0, [{
    return as_int(_mm_movemask_ps((__m128)$Arg0VarName));
    }]>;

//fract
// leave only implementaion for float4. float, float2, float3 are promoted to float4
OclBuiltinImpl fract_pas_f32_promote = OclBuiltinImpl<fract_pas, [v1f32, v2f32, v3f32], 0, Promote_fract>;
OclBuiltinImpl fract_las_f32_promote = OclBuiltinImpl<fract_las, [v1f32, v2f32, v3f32], 0, Promote_fract>;
OclBuiltinImpl fract_gas_f32_promote = OclBuiltinImpl<fract_gas, [v1f32, v2f32, v3f32], 0, Promote_fract>;

// sqrt
// leave only implementaion for float4. others sizes are promoted and expanded to float4 version
OclBuiltinImpl sqrt_v4f32_impl = OclBuiltinImpl<sqrt, [v4f32], 0, [{
    return _mm_sqrt_ps($Arg0VarName);
  }]>;
OclBuiltinImpl sqrt_f32_promote = OclBuiltinImpl<sqrt, [v1f32, v2f32, v3f32], 0, Promote1>;
OclBuiltinImpl sqrt_f32_expand = OclBuiltinImpl<sqrt, [v8f32, v16f32], 0, Expand1>;

// native_divide
// leave only implementaion for float4. others sizes are promoted and expanded to float4 version
OclBuiltinImpl native_divide_v4f32_impl = OclBuiltinImpl<native_divide, [v4f32], 0, [{
    $ReturnType $ReturnVarName = _mm_rcp_ps($Arg1VarName);
    return $Arg0VarName * $ReturnVarName;
  }]>;
OclBuiltinImpl native_divide_f32_promote = OclBuiltinImpl<native_divide, [v1f32, v2f32, v3f32], 0, Promote2>;
OclBuiltinImpl native_divide_f32_expand = OclBuiltinImpl<native_divide, [v8f32, v16f32], 0, Expand2>;

// native_recip
// leave only implementaion for float4. others sizes are promoted and expanded to float4 version
OclBuiltinImpl native_recip_v4f32_impl = OclBuiltinImpl<native_recip, [v4f32], 0, [{
    return _mm_rcp_ps($Arg0VarName);
  }]>;
OclBuiltinImpl native_recip_f32_promote = OclBuiltinImpl<native_recip, [v1f32, v2f32, v3f32], 0, Promote1>;
OclBuiltinImpl native_recip_f32_expand = OclBuiltinImpl<native_recip, [v8f32, v16f32], 0, Expand1>;

// native_rsqrt
// leave only implementaion for float4. others sizes are promoted and expanded to float4 version
OclBuiltinImpl native_rsqrt_v4f32_impl = OclBuiltinImpl<native_rsqrt, [v4f32], 0, [{
    return _mm_rsqrt_ps($Arg0VarName);
  }]>;
OclBuiltinImpl native_rsqrt_f32_promote = OclBuiltinImpl<native_rsqrt, [v1f32, v2f32, v3f32], 0, Promote1>;
OclBuiltinImpl native_rsqrt_f32_expand = OclBuiltinImpl<native_rsqrt, [v8f32, v16f32], 0, Expand1>;

// native_sqrt
// leave only implementaion for float4. others sizes are promoted and expanded to float4 version
OclBuiltinImpl native_sqrt_v4f32_impl = OclBuiltinImpl<native_sqrt, [v4f32], 0, [{
    return _mm_sqrt_ps($Arg0VarName);
  }]>;
OclBuiltinImpl native_sqrt_f32_promote = OclBuiltinImpl<native_sqrt, [v1f32, v2f32, v3f32], 0, Promote1>;
OclBuiltinImpl native_sqrt_f32_expand = OclBuiltinImpl<native_sqrt, [v8f32, v16f32], 0, Expand1>;

//////////////////////////////////////////////////////////////////////////////////////////////
//                Double f64 section
//////////////////////////////////////////////////////////////////////////////////////////////
// ceil 
// leave only implementaion to double2. others sizes are calling double2 version
OclBuiltinImpl ceil_f64_impl = OclBuiltinImpl<ceil, [v2f64], 0, [{
	return _mm_ceil_pd($Arg0VarName);
  }]>;
OclBuiltinImpl ceil_f64_promote = OclBuiltinImpl<ceil, [v1f64], 0, Promote1>;
OclBuiltinImpl ceil_f64_expand = OclBuiltinImpl<ceil, [v3f64, v4f64, v8f64, v16f64], 0, Expand1>;

// floor 
// leave only implementaion to double2. others sizes are calling double2 version
OclBuiltinImpl floor_f64_impl = OclBuiltinImpl<floor, [v2f64], 0, [{
	return _mm_floor_pd($Arg0VarName);
  }]>;
OclBuiltinImpl floor_f64_promote = OclBuiltinImpl<floor, [v1f64], 0, Promote1>;
OclBuiltinImpl floor_f64_expand = OclBuiltinImpl<floor, [v3f64, v4f64, v8f64, v16f64], 0, Expand1>;

// trunc 
// leave only implementaion to double2. others sizes are calling double2 version
OclBuiltinImpl trunc_f64_impl = OclBuiltinImpl<trunc, [v2f64], 0, [{
	return _mm_round_pd($Arg0VarName, _MM_FROUND_TO_ZERO);
  }]>;
OclBuiltinImpl trunc_f64_promote = OclBuiltinImpl<trunc, [v1f64], 0, Promote1>;
OclBuiltinImpl trunc_f64_expand = OclBuiltinImpl<trunc, [v3f64, v4f64, v8f64, v16f64], 0, Expand1>;

// round 
// leave only implementaion to double2. others sizes are calling double2 version
OclBuiltinImpl round_f64_impl = OclBuiltinImpl<round, [v2f64], 0, [{
	const long mth_signMask = 0x7FFFFFFFFFFFFFFF;
	double2 xAbs = fabs($Arg0VarName);
	long2 xSign = as_long2($Arg0VarName) & ~mth_signMask;
	double2 xAbsRounded = _mm_round_pd(xAbs, _MM_FROUND_TO_NEAREST_INT);
	double2 xDiff = xAbs - xAbsRounded;
	xDiff = as_double2(xDiff == 0.5);
	xDiff = as_double2(as_long2(xDiff) & as_long(1.0));
	$Arg0VarName = xAbsRounded + xDiff;
	$Arg0VarName = as_double2(as_long2($Arg0VarName) | xSign);
	return $Arg0VarName;
  }]>;
OclBuiltinImpl round_f64_promote = OclBuiltinImpl<round, [v1f64], 0, Promote1>;
OclBuiltinImpl round_f64_expand = OclBuiltinImpl<round, [v3f64, v4f64, v8f64, v16f64], 0, Expand1>;

// rint 
// leave only implementaion to double2. others sizes are calling double2 version
OclBuiltinImpl rint_f64_impl = OclBuiltinImpl<rint, [v2f64], 0, [{
	return _mm_round_pd($Arg0VarName, _MM_FROUND_TO_NEAREST_INT);
  }]>;
OclBuiltinImpl rint_f64_promote = OclBuiltinImpl<rint, [v1f64], 0, Promote1>;
OclBuiltinImpl rint_f64_expand = OclBuiltinImpl<rint, [v3f64, v4f64, v8f64, v16f64], 0, Expand1>;


// sqrt 
// leave only implementaion to double2. others sizes are calling double2 version
OclBuiltinImpl sqrt_f64_impl = OclBuiltinImpl<sqrt, [v2f64], 0, [{
	return _mm_sqrt_pd($Arg0VarName);
  }]>;
OclBuiltinImpl sqrt_f64_promote = OclBuiltinImpl<sqrt, [v1f64], 0, Promote1>;
OclBuiltinImpl sqrt_f64_expand = OclBuiltinImpl<sqrt, [v3f64, v4f64, v8f64, v16f64], 0, Expand1>;

// nan
// leave only implementaion for double2. double are promoted to double2
OclBuiltinImpl nan_f64_promote = OclBuiltinImpl<nan, [v1f64], 0, Promote1>;

// native_rsqrt 
// _mm_rsqrt_pd does not exist. Use SVML

//////////////////////////////////////////////////////////////////////////////////////////////
//                SVML ABI hacks for SSE42
// Problem is with ABI to SVML. We cannot pass some argument types. Example: double16
// We have to route call to lower datatype SVML function.
// What you see below is series of dirty hacks to substitute calls to failing ABI SVML functions
// with functions which are legal to call.
// float3, double3 are promoted to call float4, double4 versions
// float16, sometimes float8, double16 are expanded to lower versions of SVML
// The whole code section below should be gone when ABI to SVML is fixed. 
// Filed ticket CSSD100014450: ABI for SVML does not work for all supported types
//////////////////////////////////////////////////////////////////////////////////////////////
  
  // promote f(v3) to v4. common code for float3 and double3
  code Promote1_v3_to_v4 = [{
    $Arg0BaseType#4 n$Arg0VarName;
	n$Arg0VarName.s012 = $Arg0VarName;
    return $Func(n$Arg0VarName).s012;
  }];
  
  // promote f(v3, v3) to v4. common code for float3 and double3
  code Promote2_v3_to_v4 = [{
    $Arg0BaseType#4 n$Arg0VarName;
	n$Arg0VarName.s012 = $Arg0VarName;
    $Arg1BaseType#4  n$Arg1VarName;
	n$Arg1VarName.s012 = $Arg1VarName;
    $Arg0BaseType#4 $ReturnVarName = $Func(n$Arg0VarName, n$Arg1VarName);
    return $ReturnVarName.s012;
  }];
  
  // promote f(v3, v3, v3) to v4. common code for float3 and double3
  code Promote3_v3_to_v4 = [{
    $Arg0BaseType#4 n$Arg0VarName; 
	n$Arg0VarName.s012 = $Arg0VarName;
    $Arg1BaseType#4 n$Arg1VarName;
	n$Arg1VarName.s012 = $Arg1VarName;
    $Arg2BaseType#4 n$Arg2VarName; 
	n$Arg2VarName.s012 = $Arg2VarName;
    $Arg0BaseType#4 $ReturnVarName = $Func(n$Arg0VarName, n$Arg1VarName,n$Arg2VarName);
    return $ReturnVarName.s012;
  }];

  // promote f(v3, pv3) to v4. common code for float3 and double3
  code Promote_2Ptr_v3_to_v4 = [{
    $Arg0BaseType#4 n$Arg0VarName;
    $Arg1BaseType#4 p$Arg1VarName;
	n$Arg0VarName.s012 = $Arg0VarName;
    $Arg0BaseType#4 $ReturnVarName = $Func(n$Arg0VarName, &p$Arg1VarName);
    *$Arg1VarName = p$Arg1VarName.s012;
    return $ReturnVarName.s012;
  }];

  // promote f(v3, v3, pv3) to v4. common code for float3 and double3
  code Promote_remquo_v3_to_v4 = [{
    $Arg0BaseType#4 n$Arg0VarName;
    $Arg1BaseType#4 n$Arg1VarName;
    $Arg2BaseType#4 p$Arg2VarName;
	n$Arg0VarName.s012 = $Arg0VarName;
    n$Arg1VarName.s012 = $Arg1VarName;
    $Arg0BaseType#4 $ReturnVarName = $Func(n$Arg0VarName, n$Arg1VarName, &p$Arg2VarName);
    *$Arg2VarName = p$Arg2VarName.s012;
    return $ReturnVarName.s012;
  }];

// vXfX = f(vXfX)
list<OclType> math_svml_vF_expand = [v16f64];
list<OclType> math_svml_vF_promote = [v3f32, v3f64];
list<OclType> math_svml_vF32_expand = [v3f32];

// vXfX = f(vXfX, vXfX)
list<OclType> math_svml_vFvF_expand = [v16f32, v8f64, v16f64];
list<OclType> math_svml_vFvF_promote = [v3f32, v3f64];
list<OclType> math_svml_vF32vF32_expand = [v3f32, v16f32];
list<OclType> math_svml_vF32vF32_promote = [v3f32];

// vXfX = f(vXfX, vXfX, vXfX)
list<OclType> math_svml_vFvFvF_expand = [v8f32, v16f32, v4f64, v8f64, v16f64];
list<OclType> math_svml_vFvFvF_promote = [v3f32, v3f64];

// vXfX = f(vXfX, pvXfX)
list<OclType> math_svml_vFpvF_expand = [v8f64, v16f64];
list<OclType> math_svml_vFpvF_promote = [v3f32, v3f64];

// vXfX = f(vXfX, pvXi32)
list<OclType> math_svml_vFpvI32_expand = [v16f64];
list<OclType> math_svml_vFpvI32_promote = [v3f32, v3f64];
// vXfX = f(vXfX, vXi32) 
list<OclType> math_svml_vFvI32_expand = [v2f32, v16f32, v2f64, v8f64, v16f64];
list<OclType> math_svml_vFvI32_promote = [v3f32, v3f64];
// vXfX = f(vXfX, vXfX, pvXi32) 
list<OclType> math_svml_vFvFpvI32_expand = [v16f32, v8f64, v16f64];
list<OclType> math_svml_vFvFpvI32_promote = [v3f32, v3f64];

// acos f(vXfX)
OclBuiltinImpl acos_svml_expand = OclBuiltinImpl<acos, math_svml_vF_expand, 0, Expand1>;
OclBuiltinImpl acos_svml_promote = OclBuiltinImpl<acos, math_svml_vF_promote, 0, Promote1_v3_to_v4>;

// acosh f(vXfX)
OclBuiltinImpl acosh_svml_expand = OclBuiltinImpl<acosh, math_svml_vF_expand, 0, Expand1>;
OclBuiltinImpl acosh_svml_promote = OclBuiltinImpl<acosh, math_svml_vF_promote, 0, Promote1_v3_to_v4>;

// acospi f(vXfX)
OclBuiltinImpl acospi_svml_expand = OclBuiltinImpl<acospi, math_svml_vF_expand, 0, Expand1>;
OclBuiltinImpl acospi_svml_promote = OclBuiltinImpl<acospi, math_svml_vF_promote, 0, Promote1_v3_to_v4>;

// asin f(vXfX)
OclBuiltinImpl asin_svml_expand = OclBuiltinImpl<asin, math_svml_vF_expand, 0, Expand1>;
OclBuiltinImpl asin_svml_promote = OclBuiltinImpl<asin, math_svml_vF_promote, 0, Promote1_v3_to_v4>;

// asinh f(vXfX)
OclBuiltinImpl asinh_svml_expand = OclBuiltinImpl<asinh, math_svml_vF_expand, 0, Expand1>;
OclBuiltinImpl asinh_svml_promote = OclBuiltinImpl<asinh, math_svml_vF_promote, 0, Promote1_v3_to_v4>;

// asinpi f(vXfX)
OclBuiltinImpl asinpi_svml_expand = OclBuiltinImpl<asinpi, math_svml_vF_expand, 0, Expand1>;
OclBuiltinImpl asinpi_svml_promote = OclBuiltinImpl<asinpi, math_svml_vF_promote, 0, Promote1_v3_to_v4>;

// atan f(vXfX)
OclBuiltinImpl atan_svml_expand = OclBuiltinImpl<atan, math_svml_vF_expand, 0, Expand1>;
OclBuiltinImpl atan_svml_promote = OclBuiltinImpl<atan, math_svml_vF_promote, 0, Promote1_v3_to_v4 >;

// atan2 f(vXfX, vXfX)
OclBuiltinImpl atan2_svml_expand = OclBuiltinImpl<atan2, math_svml_vFvF_expand, 0, Expand2>;
OclBuiltinImpl atan2_svml_promote = OclBuiltinImpl<atan2, math_svml_vFvF_promote, 0, Promote2_v3_to_v4>;

// atanh f(vXfX)
OclBuiltinImpl atanh_svml_expand = OclBuiltinImpl<atanh, math_svml_vF_expand, 0, Expand1>;
OclBuiltinImpl atanh_svml_promote = OclBuiltinImpl<atanh, math_svml_vF_promote, 0, Promote1_v3_to_v4>;

// atanpi f(vXfX)
OclBuiltinImpl atanpi_svml_expand = OclBuiltinImpl<atanpi, math_svml_vF_expand, 0, Expand1>;
OclBuiltinImpl atanpi_svml_promote = OclBuiltinImpl<atanpi, math_svml_vF_promote, 0, Promote1_v3_to_v4>;

// atan2pi f(vXfX, vXfX)
OclBuiltinImpl atan2pi_svml_hack_expand = OclBuiltinImpl<atan2pi, math_svml_vFvF_expand, 0, Expand2>;
OclBuiltinImpl atan2pi_svml_hack_promote = OclBuiltinImpl<atan2pi, math_svml_vFvF_promote, 0, Promote2_v3_to_v4>;

// cbrt f(vXfX)
OclBuiltinImpl cbrt_svml_expand = OclBuiltinImpl<cbrt, math_svml_vF_expand, 0, Expand1>;
OclBuiltinImpl cbrt_svml_promote = OclBuiltinImpl<cbrt, math_svml_vF_promote, 0, Promote1_v3_to_v4>;

// cos f(vXfX)
OclBuiltinImpl cos_svml_expand = OclBuiltinImpl<cos, math_svml_vF_expand, 0, Expand1>;
OclBuiltinImpl cos_svml_promote = OclBuiltinImpl<cos, math_svml_vF_promote, 0, Promote1_v3_to_v4>;

// cosh f(vXfX)
OclBuiltinImpl cosh_svml_expand = OclBuiltinImpl<cosh, math_svml_vF_expand, 0, Expand1>;
OclBuiltinImpl cosh_svml_promote = OclBuiltinImpl<cosh, math_svml_vF_promote, 0, Promote1_v3_to_v4>;

// cospi f(vXfX)
OclBuiltinImpl cospi_svml_expand = OclBuiltinImpl<cospi, math_svml_vF_expand, 0, Expand1>;
OclBuiltinImpl cospi_svml_promote = OclBuiltinImpl<cospi, math_svml_vF_promote, 0, Promote1_v3_to_v4>;

// erfc f(vXfX)
OclBuiltinImpl erfc_svml_expand = OclBuiltinImpl<erfc, math_svml_vF_expand, 0, Expand1>;
OclBuiltinImpl erfc_svml_promote = OclBuiltinImpl<erfc, math_svml_vF_promote, 0, Promote1_v3_to_v4>;

// erf f(vXfX)
OclBuiltinImpl erf_svml_expand = OclBuiltinImpl<erf, math_svml_vF_expand, 0, Expand1>;
OclBuiltinImpl erf_svml_promote = OclBuiltinImpl<erf, math_svml_vF_promote, 0, Promote1_v3_to_v4>;

// exp f(vXfX)
OclBuiltinImpl exp_svml_expand = OclBuiltinImpl<exp, math_svml_vF_expand, 0, Expand1>;
OclBuiltinImpl exp_svml_promote = OclBuiltinImpl<exp, math_svml_vF_promote, 0, Promote1_v3_to_v4>;

// exp2 f(vXfX)
OclBuiltinImpl exp2_svml_expand = OclBuiltinImpl<exp2, math_svml_vF_expand, 0, Expand1>;
OclBuiltinImpl exp2_svml_promote = OclBuiltinImpl<exp2, math_svml_vF_promote, 0, Promote1_v3_to_v4>;

// exp10 f(vXfX)
OclBuiltinImpl exp10_svml_expand = OclBuiltinImpl<exp10, math_svml_vF_expand, 0, Expand1>;
OclBuiltinImpl exp10_svml_promote = OclBuiltinImpl<exp10, math_svml_vF_promote, 0, Promote1_v3_to_v4>;

// expm1 f(vXfX)
OclBuiltinImpl expm1_svml_expand = OclBuiltinImpl<expm1, math_svml_vF_expand, 0, Expand1>;
OclBuiltinImpl expm1_svml_promote = OclBuiltinImpl<expm1, math_svml_vF_promote, 0, Promote1_v3_to_v4>;

// fma  f(vXfX, vXfX, vXfX)
OclBuiltinImpl fma_svml_expand = OclBuiltinImpl<fma, math_svml_vFvFvF_expand, 0, Expand3>;
OclBuiltinImpl fma_svml_promote = OclBuiltinImpl<fma, math_svml_vFvFvF_promote, 0, Promote3_v3_to_v4>;

// fmod f(vXfX, vXfX)
OclBuiltinImpl fmod_svml_hack_expand = OclBuiltinImpl<fmod, math_svml_vFvF_expand, 0, Expand2>;
OclBuiltinImpl fmod_svml_hack_promote = OclBuiltinImpl<fmod, math_svml_vFvF_promote, 0, Promote2_v3_to_v4>;

// fract f(vXfX, pvXfX)
OclBuiltinImpl fract_pas_f64_expand = OclBuiltinImpl<fract_pas, math_svml_vFpvF_expand, 0, Expand_2Ptr>;
OclBuiltinImpl fract_pas_f64_promote = OclBuiltinImpl<fract_pas, math_svml_vFpvF_promote, 0, Promote_2Ptr_v3_to_v4>;

OclBuiltinImpl fract_las_f64_expand = OclBuiltinImpl<fract_las, math_svml_vFpvF_expand, 0, Expand_2Ptr>;
OclBuiltinImpl fract_las_f64_promote = OclBuiltinImpl<fract_las, math_svml_vFpvF_promote, 0, Promote_2Ptr_v3_to_v4>;

OclBuiltinImpl fract_gas_f64_expand = OclBuiltinImpl<fract_gas, math_svml_vFpvF_expand, 0, Expand_2Ptr>;
OclBuiltinImpl fract_gas_f64_promote = OclBuiltinImpl<fract_gas, math_svml_vFpvF_promote, 0, Promote_2Ptr_v3_to_v4>;

// frexp f(vXfX, pvXI32)
OclBuiltinImpl frexp_pas_expand = OclBuiltinImpl<frexp_pas, math_svml_vFpvI32_expand, 0, Expand_frexp_lgamma >;
OclBuiltinImpl frexp_pas_promote = OclBuiltinImpl<frexp_pas, math_svml_vFpvI32_promote, 0, Promote_2Ptr_v3_to_v4 >;

OclBuiltinImpl frexp_gas_expand = OclBuiltinImpl<frexp_gas, math_svml_vFpvI32_expand, 0, Expand_frexp_lgamma >;
OclBuiltinImpl frexp_gas_promote = OclBuiltinImpl<frexp_gas, math_svml_vFpvI32_promote, 0, Promote_2Ptr_v3_to_v4 >;

OclBuiltinImpl frexp_las_expand = OclBuiltinImpl<frexp_las, math_svml_vFpvI32_expand, 0, Expand_frexp_lgamma >;
OclBuiltinImpl frexp_las_promote = OclBuiltinImpl<frexp_las, math_svml_vFpvI32_promote, 0, Promote_2Ptr_v3_to_v4 >;

// hypot f(vXfX, vXfX)
OclBuiltinImpl hypot_svml_hack_expand = OclBuiltinImpl<hypot, math_svml_vFvF_expand, 0, Expand2>;
OclBuiltinImpl hypot_svml_hack_promote = OclBuiltinImpl<hypot, math_svml_vFvF_promote, 0, Promote2_v3_to_v4>;

// ilogb vXI32 = f(vXfX)
// ilogb float is implemented natively. 
// ilogb(v2f64) is not efficient. calls v1f64. this is hack
OclBuiltinImpl ilogb_svml_expand = OclBuiltinImpl<ilogb, [v2f64, v16f64], 0, Expand1>;
OclBuiltinImpl ilogb_svml_promote = OclBuiltinImpl<ilogb, [v3f64], 0, Promote1_v3_to_v4>;

// ldexp f(vXfX, vXI32)
OclBuiltinImpl ldexp_svml_expand = OclBuiltinImpl<ldexp, math_svml_vFvI32_expand, 0, Expand2>;
OclBuiltinImpl ldexp_svml_promote = OclBuiltinImpl<ldexp, math_svml_vFvI32_promote, 0, Promote2_v3_to_v4>;

// lgamma f(vXfX)
OclBuiltinImpl lgamma_svml_expand = OclBuiltinImpl<lgamma, math_svml_vF_expand, 0, Expand1>;
OclBuiltinImpl lgamma_svml_promote = OclBuiltinImpl<lgamma, math_svml_vF_promote, 0, Promote1_v3_to_v4>;

// lgamma f(vXfX, pvXI32)
OclBuiltinImpl lgamma_r_pas_expand = OclBuiltinImpl<lgamma_r_pas, math_svml_vFpvI32_expand, 0, Expand_frexp_lgamma >;
OclBuiltinImpl lgamma_r_pas_promote = OclBuiltinImpl<lgamma_r_pas, math_svml_vFpvI32_promote, 0, Promote_2Ptr_v3_to_v4 >;

OclBuiltinImpl lgamma_r_gas_expand = OclBuiltinImpl<lgamma_r_gas, math_svml_vFpvI32_expand, 0, Expand_frexp_lgamma >;
OclBuiltinImpl lgamma_r_gas_promote = OclBuiltinImpl<lgamma_r_gas, math_svml_vFpvI32_promote, 0, Promote_2Ptr_v3_to_v4 >;

OclBuiltinImpl lgamma_r_las_expand = OclBuiltinImpl<lgamma_r_las, math_svml_vFpvI32_expand, 0, Expand_frexp_lgamma >;
OclBuiltinImpl lgamma_r_las_promote = OclBuiltinImpl<lgamma_r_las, math_svml_vFpvI32_promote, 0, Promote_2Ptr_v3_to_v4 >;

// log f(vXfX)
OclBuiltinImpl log_svml_expand = OclBuiltinImpl<log, math_svml_vF_expand, 0, Expand1>;
OclBuiltinImpl log_svml_promote = OclBuiltinImpl<log, math_svml_vF_promote, 0, Promote1_v3_to_v4>;

// log2 f(vXfX)
OclBuiltinImpl log2_svml_expand = OclBuiltinImpl<log2, math_svml_vF_expand, 0, Expand1>;
OclBuiltinImpl log2_svml_promote = OclBuiltinImpl<log2, math_svml_vF_promote, 0, Promote1_v3_to_v4>;

// log10 f(vXfX)
OclBuiltinImpl log10_svml_expand = OclBuiltinImpl<log10, math_svml_vF_expand, 0, Expand1>;
OclBuiltinImpl log10_svml_promote = OclBuiltinImpl<log10, math_svml_vF_promote, 0, Promote1_v3_to_v4>;

// log1p f(vXfX)
OclBuiltinImpl log1p_svml_expand = OclBuiltinImpl<log1p, math_svml_vF_expand, 0, Expand1>;
OclBuiltinImpl log1p_svml_promote = OclBuiltinImpl<log1p, math_svml_vF_promote, 0, Promote1_v3_to_v4>;

// logb f(vXfX)
OclBuiltinImpl logb_svml_expand = OclBuiltinImpl<logb, math_svml_vF_expand, 0, Expand1>;
OclBuiltinImpl logb_svml_promote = OclBuiltinImpl<logb, math_svml_vF_promote, 0, Promote1_v3_to_v4>;

// modf f(vXfX, pvXfX)
OclBuiltinImpl modf_pas_f64_expand = OclBuiltinImpl<modf_pas, math_svml_vFpvF_expand, 0, Expand_2Ptr>;
OclBuiltinImpl modf_pas_f64_promote = OclBuiltinImpl<modf_pas, math_svml_vFpvF_promote, 0, Promote_2Ptr_v3_to_v4>;

OclBuiltinImpl modf_las_f64_expand = OclBuiltinImpl<modf_las, math_svml_vFpvF_expand, 0, Expand_2Ptr>;
OclBuiltinImpl modf_las_f64_promote = OclBuiltinImpl<modf_las, math_svml_vFpvF_promote, 0, Promote_2Ptr_v3_to_v4>;

OclBuiltinImpl modf_gas_f64_expand = OclBuiltinImpl<modf_gas, math_svml_vFpvF_expand, 0, Expand_2Ptr>;
OclBuiltinImpl modf_gas_f64_promote = OclBuiltinImpl<modf_gas, math_svml_vFpvF_promote, 0, Promote_2Ptr_v3_to_v4>;

// nextafter  f(vXfX, vXfX)
OclBuiltinImpl nextafter_svml_expand = OclBuiltinImpl<nextafter, math_svml_vFvF_expand, 0, Expand2>;
OclBuiltinImpl nextafter_svml_promote = OclBuiltinImpl<nextafter, math_svml_vFvF_promote, 0, Promote2_v3_to_v4>;

// pow f(vXfX, vXfX)
OclBuiltinImpl pow_svml_hack_expand = OclBuiltinImpl<pow, math_svml_vFvF_expand, 0, Expand2>;
OclBuiltinImpl pow_svml_hack_promote = OclBuiltinImpl<pow, math_svml_vFvF_promote, 0, Promote2_v3_to_v4>;

// pown f(vXfX, vXI32X)
OclBuiltinImpl pown_svml_hack_expand = OclBuiltinImpl<pown, math_svml_vFvI32_expand, 0, Expand2>;
OclBuiltinImpl pown_svml_hack_promote = OclBuiltinImpl<pown, math_svml_vFvI32_promote, 0, Promote2_v3_to_v4>;

// pown f(vXfX, vXfX)
OclBuiltinImpl powr_svml_expand = OclBuiltinImpl<powr, math_svml_vFvF_expand, 0, Expand2>;
OclBuiltinImpl powr_svml_promote = OclBuiltinImpl<powr, math_svml_vFvF_promote, 0, Promote2_v3_to_v4>;

// remainder f(vXfX, vXfX)
OclBuiltinImpl remainder_svml_expand = OclBuiltinImpl<remainder, math_svml_vFvF_expand, 0, Expand2>;
OclBuiltinImpl remainder_svml_promote = OclBuiltinImpl<remainder, math_svml_vFvF_promote, 0, Promote2_v3_to_v4>;

// remquo f(vXfX, vXfX, pvI32X)
OclBuiltinImpl remquo_pas_expand = OclBuiltinImpl<remquo_pas, math_svml_vFvFpvI32_expand, 0, Expand_remquo >;
OclBuiltinImpl remquo_pas_promote = OclBuiltinImpl<remquo_pas, math_svml_vFvFpvI32_promote, 0, Promote_remquo_v3_to_v4 >;

OclBuiltinImpl remquo_gas_expand = OclBuiltinImpl<remquo_gas, math_svml_vFvFpvI32_expand, 0, Expand_remquo >;
OclBuiltinImpl remquo_gas_promote = OclBuiltinImpl<remquo_gas, math_svml_vFvFpvI32_promote, 0, Promote_remquo_v3_to_v4 >;

OclBuiltinImpl remquo_las_expand = OclBuiltinImpl<remquo_las, math_svml_vFvFpvI32_expand, 0, Expand_remquo >;
OclBuiltinImpl remquo_las_promote = OclBuiltinImpl<remquo_las, math_svml_vFvFpvI32_promote, 0, Promote_remquo_v3_to_v4 >;

// rootn f(vXfX, vI32X)
OclBuiltinImpl rootn_svml_expand = OclBuiltinImpl<rootn, math_svml_vFvI32_expand, 0, Expand2>;
OclBuiltinImpl rootn_svml_promote = OclBuiltinImpl<rootn, math_svml_vFvI32_promote, 0, Promote2_v3_to_v4>;

// rsqrt f(vXfX)
// only doubles. floats are implemented natively
// reason SVML's rsqrt for doubles works better than sequence 1.0/_mm_256_sqrt(double)
OclBuiltinImpl rsqrt_svml_v64_expand = OclBuiltinImpl<rsqrt, [v16f64], 0, Expand1>;
OclBuiltinImpl rsqrt_svml_v64_promote = OclBuiltinImpl<rsqrt, [v3f64], 0, Promote1_v3_to_v4>;

// sin f(vXfX)
OclBuiltinImpl sin_svml_expand = OclBuiltinImpl<sin, math_svml_vF_expand, 0, Expand1>;
OclBuiltinImpl sin_svml_promote = OclBuiltinImpl<sin, math_svml_vF_promote, 0, Promote1_v3_to_v4>;

// sincos
OclBuiltinImpl sincos_pas_f64_expand = OclBuiltinImpl<sincos_pas, math_svml_vFpvF_expand, 0, Expand_2Ptr>;
OclBuiltinImpl sincos_pas_f64_promote = OclBuiltinImpl<sincos_pas, math_svml_vFpvF_promote, 0, Promote_2Ptr_v3_to_v4>;

OclBuiltinImpl sincos_las_f64_expand = OclBuiltinImpl<sincos_las, math_svml_vFpvF_expand, 0, Expand_2Ptr>;
OclBuiltinImpl sincos_las_f64_promote = OclBuiltinImpl<sincos_las, math_svml_vFpvF_promote, 0, Promote_2Ptr_v3_to_v4>;

OclBuiltinImpl sincos_gas_f64_expand = OclBuiltinImpl<sincos_gas, math_svml_vFpvF_expand, 0, Expand_2Ptr>;
OclBuiltinImpl sincos_gas_f64_promote = OclBuiltinImpl<sincos_gas, math_svml_vFpvF_promote, 0, Promote_2Ptr_v3_to_v4>;

// sin f(vXfX)
OclBuiltinImpl sinh_svml_expand = OclBuiltinImpl<sinh, math_svml_vF_expand, 0, Expand1>;
OclBuiltinImpl sinh_svml_promote = OclBuiltinImpl<sinh, math_svml_vF_promote, 0, Promote1_v3_to_v4>;

// sinpi f(vXfX)
OclBuiltinImpl sinpi_svml_expand = OclBuiltinImpl<sinpi, math_svml_vF_expand, 0, Expand1>;
OclBuiltinImpl sinpi_svml_promote = OclBuiltinImpl<sinpi, math_svml_vF_promote, 0, Promote1_v3_to_v4>;

// tan f(vXfX)
OclBuiltinImpl tan_svml_expand = OclBuiltinImpl<tan, math_svml_vF_expand, 0, Expand1>;
OclBuiltinImpl tan_svml_promote = OclBuiltinImpl<tan, math_svml_vF_promote, 0, Promote1_v3_to_v4>;

// tanh f(vXfX)
OclBuiltinImpl tanh_svml_expand = OclBuiltinImpl<tanh, math_svml_vF_expand, 0, Expand1>;
OclBuiltinImpl tanh_svml_promote = OclBuiltinImpl<tanh, math_svml_vF_promote, 0, Promote1_v3_to_v4>;

// tanpi f(vXfX)
OclBuiltinImpl tanpi_svml_expand = OclBuiltinImpl<tanpi, math_svml_vF_expand, 0, Expand1>;
OclBuiltinImpl tanpi_svml_promote = OclBuiltinImpl<tanpi, math_svml_vF_promote, 0, Promote1_v3_to_v4>;

// tgamma f(vXfX)
OclBuiltinImpl tgamma_svml_expand = OclBuiltinImpl<tgamma, math_svml_vF_expand, 0, Expand1>;
OclBuiltinImpl tgamma_svml_promote = OclBuiltinImpl<tgamma, math_svml_vF_promote, 0, Promote1_v3_to_v4>;
///////////////////////////////////////////////////////////////////////////////////////
// half_ functions
// half_divide f(vXfX, vXfX)
OclBuiltinImpl half_divide_svml_hack_expand = OclBuiltinImpl<half_divide, math_svml_vF32vF32_expand, 0, Expand2>;
OclBuiltinImpl half_divide_svml_hack_promote = OclBuiltinImpl<half_divide, math_svml_vF32vF32_promote, 0, Promote2_v3_to_v4>;

// half_cos f(vXfX)
OclBuiltinImpl half_cos_svml_expand = OclBuiltinImpl<half_cos, math_svml_vF32_expand, 0, Expand1>;

// half_exp f(vXfX)
OclBuiltinImpl half_exp_svml_expand = OclBuiltinImpl<half_exp, math_svml_vF32_expand, 0, Expand1>;

// half_exp2 f(vXfX)
OclBuiltinImpl half_exp2_svml_expand = OclBuiltinImpl<half_exp2, math_svml_vF32_expand, 0, Expand1>;

// half_exp10 f(vXfX)
OclBuiltinImpl half_exp10_svml_expand = OclBuiltinImpl<half_exp10, math_svml_vF32_expand, 0, Expand1>;

// half_log f(vXfX)
OclBuiltinImpl half_log_svml_expand = OclBuiltinImpl<half_log, math_svml_vF32_expand, 0, Expand1>;

// half_log2 f(vXfX)
OclBuiltinImpl half_log2_svml_expand = OclBuiltinImpl<half_log2, math_svml_vF32_expand, 0, Expand1>;

// half_log10 f(vXfX)
OclBuiltinImpl half_log10_svml_expand = OclBuiltinImpl<half_log10, math_svml_vF32_expand, 0, Expand1>;

// half_powr f(vXfX, vXfX)
OclBuiltinImpl half_powr_svml_expand = OclBuiltinImpl<half_powr, math_svml_vF32vF32_expand, 0, Expand2>;
OclBuiltinImpl half_powr_svml_promote = OclBuiltinImpl<half_powr, math_svml_vF32vF32_expand, 0, Promote2_v3_to_v4>;

// half_sin f(vXfX)
OclBuiltinImpl half_sin_svml_expand = OclBuiltinImpl<half_sin, math_svml_vF32_expand, 0, Expand1>;

// half_tan f(vXfX)
OclBuiltinImpl half_tan_svml_expand = OclBuiltinImpl<half_tan, math_svml_vF32_expand, 0, Expand1>;

///////////////////////////////////////////////////////////////////////////////////////
// native

// native_sin f(vXfX)
OclBuiltinImpl native_sin_svml_expand = OclBuiltinImpl<native_sin, math_svml_vF_expand, 0, Expand1>;
OclBuiltinImpl native_sin_svml_promote = OclBuiltinImpl<native_sin, math_svml_vF_promote, 0, Promote1_v3_to_v4>;

// native_sinpi f(vXfX)
OclBuiltinImpl native_sinpi_svml_expand = OclBuiltinImpl<native_sinpi, math_svml_vF_expand, 0, Expand1>;
OclBuiltinImpl native_sinpi_svml_promote = OclBuiltinImpl<native_sinpi, math_svml_vF_promote, 0, Promote1_v3_to_v4>;

// native_tan f(vXfX)
OclBuiltinImpl native_tan_svml_expand = OclBuiltinImpl<native_tan, math_svml_vF_expand, 0, Expand1>;
OclBuiltinImpl native_tan_svml_promote = OclBuiltinImpl<native_tan, math_svml_vF_promote, 0, Promote1_v3_to_v4>;

// native_tanpi f(vXfX)
OclBuiltinImpl native_tanpi_svml_expand = OclBuiltinImpl<native_tanpi, math_svml_vF_expand, 0, Expand1>;
OclBuiltinImpl native_tanpi_svml_promote = OclBuiltinImpl<native_tanpi, math_svml_vF_promote, 0, Promote1_v3_to_v4>;

// native_cos f(vXfX)
OclBuiltinImpl native_cos_svml_expand = OclBuiltinImpl<native_cos, math_svml_vF_expand, 0, Expand1>;
OclBuiltinImpl native_cos_svml_promote = OclBuiltinImpl<native_cos, math_svml_vF_promote, 0, Promote1_v3_to_v4>;

// native_cospi f(vXfX)
OclBuiltinImpl native_cospi_svml_expand = OclBuiltinImpl<native_cospi, math_svml_vF_expand, 0, Expand1>;
OclBuiltinImpl native_cospi_svml_promote = OclBuiltinImpl<native_cospi, math_svml_vF_promote, 0, Promote1_v3_to_v4>;

// native_exp f(vXfX)
OclBuiltinImpl native_exp_svml_expand = OclBuiltinImpl<native_exp, math_svml_vF_expand, 0, Expand1>;
OclBuiltinImpl native_exp_svml_promote = OclBuiltinImpl<native_exp, math_svml_vF_promote, 0, Promote1_v3_to_v4>;

// native_exp2 f(vXfX)
OclBuiltinImpl native_exp2_svml_expand = OclBuiltinImpl<native_exp2, math_svml_vF_expand, 0, Expand1>;
OclBuiltinImpl native_exp2_svml_promote = OclBuiltinImpl<native_exp2, math_svml_vF_promote, 0, Promote1_v3_to_v4>;

// native_exp10 f(vXfX)
OclBuiltinImpl native_exp10_svml_expand = OclBuiltinImpl<native_exp10, math_svml_vF_expand, 0, Expand1>;
OclBuiltinImpl native_exp10_svml_promote = OclBuiltinImpl<native_exp10, math_svml_vF_promote, 0, Promote1_v3_to_v4>;

// native_expm1 f(vXfX)
OclBuiltinImpl native_expm1_svml_expand = OclBuiltinImpl<native_expm1, math_svml_vF_expand, 0, Expand1>;
OclBuiltinImpl native_expm1_svml_promote = OclBuiltinImpl<native_expm1, math_svml_vF_promote, 0, Promote1_v3_to_v4>;

// native_log f(vXfX)
OclBuiltinImpl native_log_svml_expand = OclBuiltinImpl<native_log, math_svml_vF_expand, 0, Expand1>;
OclBuiltinImpl native_log_svml_promote = OclBuiltinImpl<native_log, math_svml_vF_promote, 0, Promote1_v3_to_v4>;

// native_log2 f(vXfX)
OclBuiltinImpl native_log2_svml_expand = OclBuiltinImpl<native_log2, math_svml_vF_expand, 0, Expand1>;
OclBuiltinImpl native_log2_svml_promote = OclBuiltinImpl<native_log2, math_svml_vF_promote, 0, Promote1_v3_to_v4>;

// native_log10 f(vXfX)
OclBuiltinImpl native_log10_svml_expand = OclBuiltinImpl<native_log10, math_svml_vF_expand, 0, Expand1>;
OclBuiltinImpl native_log10_svml_promote = OclBuiltinImpl<native_log10, math_svml_vF_promote, 0, Promote1_v3_to_v4>;

// native_powr f(vXfX, vXfX)
OclBuiltinImpl native_powr_svml_expand = OclBuiltinImpl<native_powr, math_svml_vFvF_expand, 0, Expand2>;
OclBuiltinImpl native_powr_svml_promote = OclBuiltinImpl<native_powr, math_svml_vFvF_promote, 0, Promote2_v3_to_v4>;

// native_acos f(vXfX)
OclBuiltinImpl native_acos_svml_expand = OclBuiltinImpl<native_acos, math_svml_vF_expand, 0, Expand1>;
OclBuiltinImpl native_acos_svml_promote = OclBuiltinImpl<native_acos, math_svml_vF_promote, 0, Promote1_v3_to_v4>;

// native_acosh f(vXfX)
OclBuiltinImpl native_acosh_svml_expand = OclBuiltinImpl<native_acosh, math_svml_vF_expand, 0, Expand1>;
OclBuiltinImpl native_acosh_svml_promote = OclBuiltinImpl<native_acosh, math_svml_vF_promote, 0, Promote1_v3_to_v4>;

// native_acospi f(vXfX)
OclBuiltinImpl native_acospi_svml_expand = OclBuiltinImpl<native_acospi, math_svml_vF_expand, 0, Expand1>;
OclBuiltinImpl native_acospi_svml_promote = OclBuiltinImpl<native_acospi, math_svml_vF_promote, 0, Promote1_v3_to_v4>;

// native_asin f(vXfX)
OclBuiltinImpl native_asin_svml_expand = OclBuiltinImpl<native_asin, math_svml_vF_expand, 0, Expand1>;
OclBuiltinImpl native_asin_svml_promote = OclBuiltinImpl<native_asin, math_svml_vF_promote, 0, Promote1_v3_to_v4>;

// native_asinh f(vXfX)
OclBuiltinImpl native_asinh_svml_expand = OclBuiltinImpl<native_asinh, math_svml_vF_expand, 0, Expand1>;
OclBuiltinImpl native_asinh_svml_promote = OclBuiltinImpl<native_asinh, math_svml_vF_promote, 0, Promote1_v3_to_v4>;

// native_asinpi f(vXfX)
OclBuiltinImpl native_asinpi_svml_expand = OclBuiltinImpl<native_asinpi, math_svml_vF_expand, 0, Expand1>;
OclBuiltinImpl native_asinpi_svml_promote = OclBuiltinImpl<native_asinpi, math_svml_vF_promote, 0, Promote1_v3_to_v4>;

// native_atan f(vXfX)
OclBuiltinImpl native_atan_svml_expand = OclBuiltinImpl<native_atan, math_svml_vF_expand, 0, Expand1>;
OclBuiltinImpl native_atan_svml_promote = OclBuiltinImpl<native_atan, math_svml_vF_promote, 0, Promote1_v3_to_v4>;

// native_atan2 f(vXfX, vXfX)
OclBuiltinImpl native_atan2_svml_hack_expand = OclBuiltinImpl<native_atan2, math_svml_vFvF_expand, 0, Expand2>;
OclBuiltinImpl native_atan2_svml_hack_promote = OclBuiltinImpl<native_atan2, math_svml_vFvF_promote, 0, Promote2_v3_to_v4>;

// native_atan f(vXfX)
OclBuiltinImpl native_atan_svml_expand = OclBuiltinImpl<native_atan, math_svml_vF_expand, 0, Expand1>;
OclBuiltinImpl native_atan_svml_promote = OclBuiltinImpl<native_atan, math_svml_vF_promote, 0, Promote1_v3_to_v4>;

// native_atanh f(vXfX)
OclBuiltinImpl native_atanh_svml_expand = OclBuiltinImpl<native_atanh, math_svml_vF_expand, 0, Expand1>;
OclBuiltinImpl native_atanh_svml_promote = OclBuiltinImpl<native_atanh, math_svml_vF_promote, 0, Promote1_v3_to_v4>;

// native_atanpi f(vXfX)
OclBuiltinImpl native_atanpi_svml_expand = OclBuiltinImpl<native_atanpi, math_svml_vF_expand, 0, Expand1>;
OclBuiltinImpl native_atanpi_svml_promote = OclBuiltinImpl<native_atanpi, math_svml_vF_promote, 0, Promote1_v3_to_v4>;

// native_atan2pi f(vXfX, vXfX)
OclBuiltinImpl native_atan2pi_svml_hack_expand = OclBuiltinImpl<native_atan2pi, math_svml_vFvF_expand, 0, Expand2>;
OclBuiltinImpl native_atan2pi_svml_hack_promote = OclBuiltinImpl<native_atan2pi, math_svml_vFvF_promote, 0, Promote2_v3_to_v4>;

// native_cbrt f(vXfX)
OclBuiltinImpl native_cbrt_svml_expand = OclBuiltinImpl<native_cbrt, math_svml_vF_expand, 0, Expand1>;
OclBuiltinImpl native_cbrt_svml_promote = OclBuiltinImpl<native_cbrt, math_svml_vF_promote, 0, Promote1_v3_to_v4>;

// native_pow f(vXfX)
OclBuiltinImpl native_pow_svml_hack_expand = OclBuiltinImpl<native_pow, math_svml_vFvF_expand, 0, Expand2>;
OclBuiltinImpl native_pow_svml_hack_promote = OclBuiltinImpl<native_pow, math_svml_vFvF_promote, 0, Promote2_v3_to_v4>;

// native_erfc f(vXfX)
OclBuiltinImpl native_erfc_svml_expand = OclBuiltinImpl<native_erfc, math_svml_vF_expand, 0, Expand1>;
OclBuiltinImpl native_erfc_svml_promote = OclBuiltinImpl<native_erfc, math_svml_vF_promote, 0, Promote1_v3_to_v4>;

// native_erf f(vXfX)
OclBuiltinImpl native_erf_svml_expand = OclBuiltinImpl<native_erf, math_svml_vF_expand, 0, Expand1>;
OclBuiltinImpl native_erf_svml_promote = OclBuiltinImpl<native_erf, math_svml_vF_promote, 0, Promote1_v3_to_v4>;

// native_log1p f(vXfX)
OclBuiltinImpl native_log1p_svml_expand = OclBuiltinImpl<native_log1p, math_svml_vF_expand, 0, Expand1>;
OclBuiltinImpl native_log1p_svml_promote = OclBuiltinImpl<native_log1p, math_svml_vF_promote, 0, Promote1_v3_to_v4>;

// native_log1p f(vXfX, vXfX)
// only doubles. floats are implemented in code
OclBuiltinImpl native_divide_svml_hack_expand = OclBuiltinImpl<native_divide, [v8f64, v16f64], 0, Expand2>;
OclBuiltinImpl native_divide_svml_hack_promote = OclBuiltinImpl<native_divide, [v3f64], 0, Promote2_v3_to_v4>;

// native_recip f(vXfX, vXfX)
// only doubles. floats are implemented natively
OclBuiltinImpl native_recip_svml_expand = OclBuiltinImpl<native_recip, [v16f64], 0, Expand1>;
OclBuiltinImpl native_recip_svml_promote = OclBuiltinImpl<native_recip, [v3f64], 0, Promote1_v3_to_v4>;

// native_rsqrt f(vXfX)
// only doubles. floats are implemented natively
OclBuiltinImpl native_rsqrt_svml_expand = OclBuiltinImpl<native_rsqrt, [v16f64], 0, Expand1>;
OclBuiltinImpl native_rsqrt_svml_promote = OclBuiltinImpl<native_rsqrt, [v3f64], 0, Promote1_v3_to_v4>;