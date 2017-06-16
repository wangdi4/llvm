// vim:ts=2:sw=2:et:

// NOTE
// this file contains built-ins shared between all CPU architectures,
// if these functions should be promoted or expanded from basic implementation
// which can be found in GENERIC/math_shared.td

// include Promote1_v3_to_v4, Promote2_v3_to_v4 codes
include "ssse3/code_promote.td"

// vXfX = f(vXfX)
list<OclType> math_svml_vF_promote = [v3f32, v3f64];
list<OclType> math_svml_vF_expand = [v16f64];
list<OclType> math_svml_vFvF_promote = [v3f32, v3f64];
list<OclType> math_svml_vFvF_expand = [v16f64];
list<OclType> math_svml_vFvI32_expand = [v2f32, v2f64, v16f64];
list<OclType> math_svml_vFvI32_promote = [v3f32, v3f64];
list<OclType> math_svml_vF32_expand = [v3f32];
list<OclType> math_svml_vF32vF32_expand = [v3f32];
list<OclType> math_svml_vF64_expand = [v16f64];
list<OclType> math_svml_vF_expand_native = [v16f64];

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

// copysign
// leave only implementaion for float4. float, float2, float3 are promoted to float4
OclBuiltinImpl copysign_f32_promote = OclBuiltinImpl<copysign, [v1f32, v2f32, v3f32], 0, Promote2>;

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

// fabs
// leave only implementaion for float4. float, float2, float3 are promoted to float4
OclBuiltinImpl fabs_f32_promote = OclBuiltinImpl<fabs, [v1f32, v2f32, v3f32], 0, Promote1>;

// fdim
// leave only implementaion for float4. float, float2, float3 are promoted to float4
OclBuiltinImpl fdim_f32_promote = OclBuiltinImpl<fdim, [v1f32, v2f32, v3f32], 0, Promote2>;

OclBuiltinImpl floor_f32_promote = OclBuiltinImpl<floor, [v1f32, v2f32, v3f32], 0, Promote1>;
OclBuiltinImpl floor_f64_promote = OclBuiltinImpl<floor, [v1f64], 0, Promote1>;
OclBuiltinImpl floor_f64_expand = OclBuiltinImpl<floor, [v3f64, v16f64], 0, Expand1>;

// fmin
// leave only implementaion for float4. float, float2, float3 are promoted to float4
OclBuiltinImpl fmin_f32_promote = OclBuiltinImpl<fmin, [v1f32, v2f32, v3f32], 0, Promote2>;

// fmax
// leave only implementaion for float4. float, float2, float3 are promoted to float4
OclBuiltinImpl fmax_f32_promote = OclBuiltinImpl<fmax, [v1f32, v2f32, v3f32], 0, Promote2>;

// fmod f(vXfX, vXfX)
OclBuiltinImpl fmod_svml_hack_expand = OclBuiltinImpl<fmod, math_svml_vFvF_expand, 0, Expand2>;
OclBuiltinImpl fmod_svml_hack_promote = OclBuiltinImpl<fmod, math_svml_vFvF_promote, 0, Promote2_v3_to_v4>;


// hypot inlined implementation using NewtonRaphson sqrt and native_rsqrt
// native_rsqrt is assumed to have low precision and be quick
// on CPU float native_rsqrt (floatX) is fast mm_rcp_ps intrinsic (5 clocks)
// on corner cases calls SVML
code hypot_impl_nr = [{
    // check NaN or Inf
    $Arg0igentype isNaNOrInf = (as_$Arg0igentype($Arg0VarName) & $ReturnBaseType_const_expMask) == $ReturnBaseType_const_expMask;
    $Arg0VarName = fabs($Arg0VarName);
    $Arg1VarName = fabs($Arg1VarName);
    $Arg0Type res = $Arg0VarName*$Arg0VarName + $Arg1VarName*$Arg1VarName;
    // !!! native_rsqrt() MUST call fast _mm_rcp_ps() or _mm256_rcp_ps()
    // mm_rcp_ps() has 11 bits precision
    $Arg0Type x_0 = native_rsqrt(res);
    const $Arg0Type oneHalf = 0.5;
    const $Arg0Type three = 3.0;
    res = oneHalf * res * x_0 * (three - res * x_0 * x_0);
    $Arg0Type lmax = max($Arg0VarName, $Arg1VarName);
    // check corner cases and call SVML in that case
    if (intel_movemask( lmax > $ReturnBaseType_const_tooBig | lmax < $ReturnBaseType_const_tooSmall | isNaNOrInf ))
      res = __ocl_svml_$Target_hypot$SVMLSuffix($Arg0VarName, $Arg1VarName);
    return res;
  }];

// hypot
// float, float2, float4, float8 version uses inline NewtonRaphson
OclBuiltinImpl hypot_v148f32 = OclBuiltinImpl<hypot, [v1f32, v2f32, v4f32, v8f32], 0, hypot_impl_nr>;

// hypot f(vXfX, vXfX)
OclBuiltinImpl hypot_svml_expand = OclBuiltinImpl<hypot, math_svml_vFvF_expand, 0, Expand2>;

// [LLVM 3.6 UPGRADE] This is a quick fix for
// CORC-208 - [Nightly] Pass Spir1.2_math_brute_force - math_kernel3.hypot_float3 did not validate
//
// The root cause is UB in hypot_impl_nr hypot(float4, float4) in case when
// float3 arguments get promoted to float4 arguments filling 4th component with undef value causing
// hypot(float3, float3) producing not bit-to-bit equal results between runs on the same input.
//
// According to SVML documentation performance-tolerant values for
// hypotf and hypot are in range [-10000.0, 10000.0] for both arguments.
// Choosing zero to avoid an additional constant load.
OclBuiltinImpl hypot_svml_promote = OclBuiltinImpl<hypot, math_svml_vFvF_promote, 0, Promote2_tolerant_v3_to_v4,
                     ["$TolerantValue0"->"0.0", "$TolerantValue1"->"0.0"]>;

// ilogb
// float3 are promoted to float4
OclBuiltinImpl ilogb_f32_promote = OclBuiltinImpl<ilogb, [v3f32], 0, Promote1>;
// ilogb(v2f64) is not efficient. calls v1f64. this is hack
OclBuiltinImpl ilogb_svml_expand = OclBuiltinImpl<ilogb, [v2f64, v16f64], 0, Expand1>;
OclBuiltinImpl ilogb_svml_promote = OclBuiltinImpl<ilogb, [v3f64], 0, Promote1_v3_to_v4>;

// ldexp f(vXfX, vXI32)
OclBuiltinImpl ldexp_svml_expand = OclBuiltinImpl<ldexp, math_svml_vFvI32_expand, 0, Expand2>;
OclBuiltinImpl ldexp_svml_promote = OclBuiltinImpl<ldexp, math_svml_vFvI32_promote, 0, Promote2_v3_to_v4>;

// lgamma f(vXfX)
OclBuiltinImpl lgamma_svml_expand = OclBuiltinImpl<lgamma, math_svml_vF_expand, 0, Expand1>;
OclBuiltinImpl lgamma_svml_promote = OclBuiltinImpl<lgamma, math_svml_vF_promote, 0, Promote1_v3_to_v4>;

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

// minmag
// leave only implementaion for float4. float, float2, float3 are promoted to float4
OclBuiltinImpl minmag_f32_promote = OclBuiltinImpl<minmag, [v1f32, v2f32, v3f32], 0, Promote2>;

// maxmag
// leave only implementaion for float4. float, float2, float3 are promoted to float4
OclBuiltinImpl maxmag_f32_promote = OclBuiltinImpl<maxmag, [v1f32, v2f32, v3f32], 0, Promote2>;


// nan
// float3 are promoted to float4
OclBuiltinImpl nan_f32_promote = OclBuiltinImpl<nan, [v1f32, v2f32, v3f32], 0, Promote1>;
// leave only implementaion for double2. double are promoted to double2
OclBuiltinImpl nan_f64_promote = OclBuiltinImpl<nan, [v1f64], 0, Promote1>;

// nextafter  f(vXfX, vXfX)
OclBuiltinImpl nextafter_svml_expand = OclBuiltinImpl<nextafter, [v16f64], 0, Expand2>;
OclBuiltinImpl nextafter_svml_promote = OclBuiltinImpl<nextafter, math_svml_vFvF_promote, 0, Promote2_v3_to_v4>;

// pow f(vXfX, vXfX)
OclBuiltinImpl pow_svml_hack_expand = OclBuiltinImpl<pow, [v16f64], 0, Expand2>;
OclBuiltinImpl pow_svml_hack_promote = OclBuiltinImpl<pow, math_svml_vFvF_promote, 0, Promote2_v3_to_v4>;

// pown f(vXfX, vXI32X)
OclBuiltinImpl pown_svml_hack_expand = OclBuiltinImpl<pown, math_svml_vFvI32_expand, 0, Expand2>;
OclBuiltinImpl pown_svml_hack_promote = OclBuiltinImpl<pown, math_svml_vFvI32_promote, 0, Promote2_v3_to_v4>;

// pown f(vXfX, vXfX)
OclBuiltinImpl powr_svml_expand = OclBuiltinImpl<powr, [v16f64], 0, Expand2>;
OclBuiltinImpl powr_svml_promote = OclBuiltinImpl<powr, math_svml_vFvF_promote, 0, Promote2_v3_to_v4>;

// remainder f(vXfX, vXfX)
OclBuiltinImpl remainder_svml_expand = OclBuiltinImpl<remainder, [v16f64], 0, Expand2>;
OclBuiltinImpl remainder_svml_promote = OclBuiltinImpl<remainder, math_svml_vFvF_promote, 0, Promote2_v3_to_v4>;

// rint f(vXfX)
OclBuiltinImpl rint_svml_expand = OclBuiltinImpl<rint, [v3f64, v16f64], 0, Expand1>;
OclBuiltinImpl rint_svml_promote = OclBuiltinImpl<rint, [v1f32, v2f32, v3f32, v1f64], 0, Promote1>;

// rint f(vXfX)
OclBuiltinImpl round_svml_expand = OclBuiltinImpl<round, [v3f64, v16f64], 0, Expand1>;
OclBuiltinImpl round_svml_promote = OclBuiltinImpl<round, [v1f32, v2f32, v3f32, v1f64], 0, Promote1>;

// rootn f(vXfX, vI32X)
OclBuiltinImpl rootn_svml_expand = OclBuiltinImpl<rootn, math_svml_vFvI32_expand, 0, Expand2>;
OclBuiltinImpl rootn_svml_promote = OclBuiltinImpl<rootn, math_svml_vFvI32_promote, 0, Promote2_v3_to_v4>;

// rsqrt f(vXfX)
// rsqrt
OclBuiltinImpl rsqrt_f32_impl = OclBuiltinImpl<rsqrt, [v4f32], 0, [{
    const float4 temp = _mm_sqrt_ps($Arg0VarName);
    return 1.0f/temp;
  }]>;
// leave only implementaion for float4. others sizes are promoted and expanded to float4 version
OclBuiltinImpl rsqrt_f32_promote = OclBuiltinImpl<rsqrt, [v1f32, v2f32, v3f32], 0, Promote1>;
// reason SVML's rsqrt for doubles works better than sequence 1.0/_mm_256_sqrt(double)
OclBuiltinImpl rsqrt_svml_v64_expand = OclBuiltinImpl<rsqrt, [v16f64], 0, Expand1>;
OclBuiltinImpl rsqrt_svml_v64_promote = OclBuiltinImpl<rsqrt, [v3f64], 0, Promote1_v3_to_v4>;

// sin f(vXfX)
OclBuiltinImpl sin_svml_expand = OclBuiltinImpl<sin, math_svml_vF_expand, 0, Expand1>;
OclBuiltinImpl sin_svml_promote = OclBuiltinImpl<sin, math_svml_vF_promote, 0, Promote1_v3_to_v4>;

// sin f(vXfX)
OclBuiltinImpl sinh_svml_expand = OclBuiltinImpl<sinh, math_svml_vF_expand, 0, Expand1>;
OclBuiltinImpl sinh_svml_promote = OclBuiltinImpl<sinh, math_svml_vF_promote, 0, Promote1_v3_to_v4>;

// sinpi f(vXfX)
OclBuiltinImpl sinpi_svml_expand = OclBuiltinImpl<sinpi, math_svml_vF_expand, 0, Expand1>;
OclBuiltinImpl sinpi_svml_promote = OclBuiltinImpl<sinpi, math_svml_vF_promote, 0, Promote1_v3_to_v4>;

// sqrt
// leave only implementaion for float4. others sizes are promoted and expanded to float4 version
OclBuiltinImpl sqrt_v4f32_impl = OclBuiltinImpl<sqrt, [v4f32], 0, [{
    return _mm_sqrt_ps($Arg0VarName);
  }]>;
OclBuiltinImpl sqrt_f32_promote = OclBuiltinImpl<sqrt, [v1f32, v2f32, v3f32], 0, Promote1>;
OclBuiltinImpl sqrt_f64_impl = OclBuiltinImpl<sqrt, [v2f64], 0, [{
    return _mm_sqrt_pd($Arg0VarName);
  }]>;
OclBuiltinImpl sqrt_f64_promote = OclBuiltinImpl<sqrt, [v1f64], 0, Promote1>;
OclBuiltinImpl sqrt_f64_expand = OclBuiltinImpl<sqrt, [v3f64, v16f64], 0, Expand1>;

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

// trunc
OclBuiltinImpl trunc_f32_promote = OclBuiltinImpl<trunc, [v3f32], 0, Promote1>;
OclBuiltinImpl trunc_f64_promote = OclBuiltinImpl<trunc, [v1f64], 0, Promote1>;
OclBuiltinImpl trunc_f64_expand = OclBuiltinImpl<trunc, [v3f64, v16f64], 0, Expand1>;

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

///////////////////////////////////////////////////////////////////////////////////////
// half_ functions
// half_divide f(vXfX, vXfX)
OclBuiltinImpl half_divide_svml_hack_promote = OclBuiltinImpl<half_divide, math_svml_vF32vF32_expand, 0, Promote2_v3_to_v4>;

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

// native_divide
// leave only implementaion for float4. others sizes are promoted and expanded to float4 version
OclBuiltinImpl native_divide_v4f32_impl = OclBuiltinImpl<native_divide, [v4f32], 0, [{
    $ReturnType $ReturnVarName = _mm_rcp_ps($Arg1VarName);
    return $Arg0VarName * $ReturnVarName;
  }]>;
OclBuiltinImpl native_divide_f32_promote = OclBuiltinImpl<native_divide, [v1f32, v2f32, v3f32], 0, Promote2>;

// native_recip
// leave only implementaion for float4. others sizes are promoted and expanded to float4 version
OclBuiltinImpl native_recip_v4f32_impl = OclBuiltinImpl<native_recip, [v4f32], 0, [{
    return _mm_rcp_ps($Arg0VarName);
  }]>;
OclBuiltinImpl native_recip_f32_promote = OclBuiltinImpl<native_recip, [v2f32, v3f32], 0, Promote1>;

OclBuiltinImpl native_rsqrt_v4f32_impl = OclBuiltinImpl<native_rsqrt, [v4f32], 0, [{
    return _mm_rsqrt_ps($Arg0VarName);
  }]>;
OclBuiltinImpl native_rsqrt_f32_promote = OclBuiltinImpl<native_rsqrt, [v2f32, v3f32], 0, Promote1>;

// native_sqrt
// leave only implementaion for float4. others sizes are promoted and expanded to float4 version
OclBuiltinImpl native_sqrt_v4f32_impl = OclBuiltinImpl<native_sqrt, [v4f32], 0, [{
    return _mm_sqrt_ps($Arg0VarName);
  }]>;
OclBuiltinImpl native_sqrt_f32_promote = OclBuiltinImpl<native_sqrt, [v2f32, v3f32], 0, Promote1>;

// native_sin f(vXfX)
OclBuiltinImpl native_sin_svml_expand = OclBuiltinImpl<native_sin, math_svml_vF_expand_native, 0, Expand1>;
OclBuiltinImpl native_sin_svml_promote = OclBuiltinImpl<native_sin, math_svml_vF_promote, 0, Promote1_v3_to_v4>;

// native_sinpi f(vXfX)
OclBuiltinImpl native_sinpi_svml_expand = OclBuiltinImpl<native_sinpi, math_svml_vF_expand_native, 0, Expand1>;
OclBuiltinImpl native_sinpi_svml_promote = OclBuiltinImpl<native_sinpi, math_svml_vF_promote, 0, Promote1_v3_to_v4>;

// native_tan f(vXfX)
OclBuiltinImpl native_tan_svml_expand = OclBuiltinImpl<native_tan, math_svml_vF_expand_native, 0, Expand1>;
OclBuiltinImpl native_tan_svml_promote = OclBuiltinImpl<native_tan, math_svml_vF_promote, 0, Promote1_v3_to_v4>;

// native_tanpi f(vXfX)
OclBuiltinImpl native_tanpi_svml_expand = OclBuiltinImpl<native_tanpi, math_svml_vF_expand_native, 0, Expand1>;
OclBuiltinImpl native_tanpi_svml_promote = OclBuiltinImpl<native_tanpi, math_svml_vF_promote, 0, Promote1_v3_to_v4>;

// native_cos f(vXfX)
OclBuiltinImpl native_cos_svml_expand = OclBuiltinImpl<native_cos, math_svml_vF_expand_native, 0, Expand1>;
OclBuiltinImpl native_cos_svml_promote = OclBuiltinImpl<native_cos, math_svml_vF_promote, 0, Promote1_v3_to_v4>;

// native_cospi f(vXfX)
OclBuiltinImpl native_cospi_svml_expand = OclBuiltinImpl<native_cospi, math_svml_vF_expand_native, 0, Expand1>;
OclBuiltinImpl native_cospi_svml_promote = OclBuiltinImpl<native_cospi, math_svml_vF_promote, 0, Promote1_v3_to_v4>;

// native_exp f(vXfX)
OclBuiltinImpl native_exp_svml_expand = OclBuiltinImpl<native_exp, math_svml_vF_expand_native, 0, Expand1>;
OclBuiltinImpl native_exp_svml_promote = OclBuiltinImpl<native_exp, math_svml_vF_promote, 0, Promote1_v3_to_v4>;

// native_exp2 f(vXfX)
OclBuiltinImpl native_exp2_svml_expand = OclBuiltinImpl<native_exp2, math_svml_vF_expand_native, 0, Expand1>;
OclBuiltinImpl native_exp2_svml_promote = OclBuiltinImpl<native_exp2, math_svml_vF_promote, 0, Promote1_v3_to_v4>;

// native_exp10 f(vXfX)
OclBuiltinImpl native_exp10_svml_expand = OclBuiltinImpl<native_exp10, math_svml_vF_expand_native, 0, Expand1>;
OclBuiltinImpl native_exp10_svml_promote = OclBuiltinImpl<native_exp10, math_svml_vF_promote, 0, Promote1_v3_to_v4>;

// native_expm1 f(vXfX)
OclBuiltinImpl native_expm1_svml_expand = OclBuiltinImpl<native_expm1, math_svml_vF_expand_native, 0, Expand1>;
OclBuiltinImpl native_expm1_svml_promote = OclBuiltinImpl<native_expm1, math_svml_vF_promote, 0, Promote1_v3_to_v4>;

// native_log f(vXfX)
OclBuiltinImpl native_log_svml_expand = OclBuiltinImpl<native_log, math_svml_vF_expand_native, 0, Expand1>;
OclBuiltinImpl native_log_svml_promote = OclBuiltinImpl<native_log, math_svml_vF_promote, 0, Promote1_v3_to_v4>;

// native_log2 f(vXfX)
OclBuiltinImpl native_log2_svml_expand = OclBuiltinImpl<native_log2, math_svml_vF_expand_native, 0, Expand1>;
OclBuiltinImpl native_log2_svml_promote = OclBuiltinImpl<native_log2, math_svml_vF_promote, 0, Promote1_v3_to_v4>;

// native_log10 f(vXfX)
OclBuiltinImpl native_log10_svml_expand = OclBuiltinImpl<native_log10, math_svml_vF_expand_native, 0, Expand1>;
OclBuiltinImpl native_log10_svml_promote = OclBuiltinImpl<native_log10, math_svml_vF_promote, 0, Promote1_v3_to_v4>;

// native_powr f(vXfX, vXfX)
OclBuiltinImpl native_powr_svml_expand = OclBuiltinImpl<native_powr, math_svml_vF_expand_native, 0, Expand2>;
OclBuiltinImpl native_powr_svml_promote = OclBuiltinImpl<native_powr, math_svml_vFvF_promote, 0, Promote2_v3_to_v4>;

// native_acos f(vXfX)
OclBuiltinImpl native_acos_svml_expand = OclBuiltinImpl<native_acos, math_svml_vF_expand_native, 0, Expand1>;
OclBuiltinImpl native_acos_svml_promote = OclBuiltinImpl<native_acos, math_svml_vF_promote, 0, Promote1_v3_to_v4>;

// native_acosh f(vXfX)
OclBuiltinImpl native_acosh_svml_expand = OclBuiltinImpl<native_acosh, math_svml_vF_expand_native, 0, Expand1>;
OclBuiltinImpl native_acosh_svml_promote = OclBuiltinImpl<native_acosh, math_svml_vF_promote, 0, Promote1_v3_to_v4>;

// native_acospi f(vXfX)
OclBuiltinImpl native_acospi_svml_expand = OclBuiltinImpl<native_acospi, math_svml_vF_expand_native, 0, Expand1>;
OclBuiltinImpl native_acospi_svml_promote = OclBuiltinImpl<native_acospi, math_svml_vF_promote, 0, Promote1_v3_to_v4>;

// native_asin f(vXfX)
OclBuiltinImpl native_asin_svml_expand = OclBuiltinImpl<native_asin, math_svml_vF_expand_native, 0, Expand1>;
OclBuiltinImpl native_asin_svml_promote = OclBuiltinImpl<native_asin, math_svml_vF_promote, 0, Promote1_v3_to_v4>;

// native_asinh f(vXfX)
OclBuiltinImpl native_asinh_svml_expand = OclBuiltinImpl<native_asinh, math_svml_vF_expand_native, 0, Expand1>;
OclBuiltinImpl native_asinh_svml_promote = OclBuiltinImpl<native_asinh, math_svml_vF_promote, 0, Promote1_v3_to_v4>;

// native_asinpi f(vXfX)
OclBuiltinImpl native_asinpi_svml_expand = OclBuiltinImpl<native_asinpi, math_svml_vF_expand_native, 0, Expand1>;
OclBuiltinImpl native_asinpi_svml_promote = OclBuiltinImpl<native_asinpi, math_svml_vF_promote, 0, Promote1_v3_to_v4>;

// native_atan f(vXfX)
OclBuiltinImpl native_atan_svml_expand = OclBuiltinImpl<native_atan, math_svml_vF_expand_native, 0, Expand1>;
OclBuiltinImpl native_atan_svml_promote = OclBuiltinImpl<native_atan, math_svml_vF_promote, 0, Promote1_v3_to_v4>;

// native_atan2 f(vXfX, vXfX)
OclBuiltinImpl native_atan2_svml_hack_expand = OclBuiltinImpl<native_atan2, math_svml_vF_expand_native, 0, Expand2>;
OclBuiltinImpl native_atan2_svml_hack_promote = OclBuiltinImpl<native_atan2, math_svml_vFvF_promote, 0, Promote2_v3_to_v4>;

// native_atan f(vXfX)
OclBuiltinImpl native_atan_svml_expand = OclBuiltinImpl<native_atan, math_svml_vF_expand_native, 0, Expand1>;
OclBuiltinImpl native_atan_svml_promote = OclBuiltinImpl<native_atan, math_svml_vF_promote, 0, Promote1_v3_to_v4>;

// native_atanh f(vXfX)
OclBuiltinImpl native_atanh_svml_expand = OclBuiltinImpl<native_atanh, math_svml_vF_expand_native, 0, Expand1>;
OclBuiltinImpl native_atanh_svml_promote = OclBuiltinImpl<native_atanh, math_svml_vF_promote, 0, Promote1_v3_to_v4>;

// native_atanpi f(vXfX)
OclBuiltinImpl native_atanpi_svml_expand = OclBuiltinImpl<native_atanpi, math_svml_vF_expand_native, 0, Expand1>;
OclBuiltinImpl native_atanpi_svml_promote = OclBuiltinImpl<native_atanpi, math_svml_vF_promote, 0, Promote1_v3_to_v4>;

// native_atan2pi f(vXfX, vXfX)
OclBuiltinImpl native_atan2pi_svml_hack_expand = OclBuiltinImpl<native_atan2pi, math_svml_vF_expand_native, 0, Expand2>;
OclBuiltinImpl native_atan2pi_svml_hack_promote = OclBuiltinImpl<native_atan2pi, math_svml_vFvF_promote, 0, Promote2_v3_to_v4>;

// native_cbrt f(vXfX)
OclBuiltinImpl native_cbrt_svml_expand = OclBuiltinImpl<native_cbrt, math_svml_vF_expand_native, 0, Expand1>;
OclBuiltinImpl native_cbrt_svml_promote = OclBuiltinImpl<native_cbrt, math_svml_vF_promote, 0, Promote1_v3_to_v4>;

// native_pow f(vXfX)
OclBuiltinImpl native_pow_svml_hack_expand = OclBuiltinImpl<native_pow, math_svml_vF_expand_native, 0, Expand2>;
OclBuiltinImpl native_pow_svml_hack_promote = OclBuiltinImpl<native_pow, math_svml_vFvF_promote, 0, Promote2_v3_to_v4>;

// native_erfc f(vXfX)
OclBuiltinImpl native_erfc_svml_expand = OclBuiltinImpl<native_erfc, math_svml_vF_expand_native, 0, Expand1>;
OclBuiltinImpl native_erfc_svml_promote = OclBuiltinImpl<native_erfc, math_svml_vF_promote, 0, Promote1_v3_to_v4>;

// native_erf f(vXfX)
OclBuiltinImpl native_erf_svml_expand = OclBuiltinImpl<native_erf, math_svml_vF_expand_native, 0, Expand1>;
OclBuiltinImpl native_erf_svml_promote = OclBuiltinImpl<native_erf, math_svml_vF_promote, 0, Promote1_v3_to_v4>;

// native_log1p f(vXfX)
OclBuiltinImpl native_log1p_svml_expand = OclBuiltinImpl<native_log1p, math_svml_vF_expand_native, 0, Expand1>;
OclBuiltinImpl native_log1p_svml_promote = OclBuiltinImpl<native_log1p, math_svml_vF_promote, 0, Promote1_v3_to_v4>;

// native_log1p f(vXfX, vXfX)
// only doubles. floats are implemented in code
OclBuiltinImpl native_divide_svml_hack_expand = OclBuiltinImpl<native_divide, math_svml_vF_expand_native, 0, Expand2>;
OclBuiltinImpl native_divide_svml_hack_promote = OclBuiltinImpl<native_divide, [v3f64], 0, Promote2_v3_to_v4>;

// native_recip f(vXfX, vXfX)
// only doubles. floats are implemented natively
OclBuiltinImpl native_recip_svml_expand = OclBuiltinImpl<native_recip, math_svml_vF_expand_native, 0, Expand1>;
OclBuiltinImpl native_recip_svml_promote = OclBuiltinImpl<native_recip, [v3f64], 0, Promote1_v3_to_v4>;

// native_rsqrt f(vXfX)
// only doubles. floats are implemented natively
OclBuiltinImpl native_rsqrt_svml_expand = OclBuiltinImpl<native_rsqrt, math_svml_vF_expand_native, 0, Expand1>;
OclBuiltinImpl native_rsqrt_svml_promote = OclBuiltinImpl<native_rsqrt, [v3f64], 0, Promote1_v3_to_v4>;

// native_rcbrt f(vXfX)
OclBuiltinImpl native_rcbrt_svml_expand = OclBuiltinImpl<native_rcbrt, math_svml_vF_expand_native, 0, Expand1>;
OclBuiltinImpl native_rcbrt_svml_promote = OclBuiltinImpl<native_rcbrt, [v3f64], 0, Promote1_v3_to_v4>;

// native_rootn f(vXfX)
OclBuiltinImpl native_rootn_svml_expand = OclBuiltinImpl<native_rootn, math_svml_vF_expand_native, 0, Expand2>;
OclBuiltinImpl native_rootn_svml_promote = OclBuiltinImpl<native_rootn, [v3f64], 0, Promote2_v3_to_v4>;

// native_sinh f(vXfX)
OclBuiltinImpl native_sinh_svml_expand = OclBuiltinImpl<native_sinh, math_svml_vF_expand_native, 0, Expand1>;
OclBuiltinImpl native_sinh_svml_promote = OclBuiltinImpl<native_sinh, [v3f64], 0, Promote1_v3_to_v4>;

// native_cosh f(vXfX)
OclBuiltinImpl native_cosh_svml_expand = OclBuiltinImpl<native_cosh, math_svml_vF_expand_native, 0, Expand1>;
OclBuiltinImpl native_cosh_svml_promote = OclBuiltinImpl<native_cosh, [v3f64], 0, Promote1_v3_to_v4>;

// native_pown f(vXfX)
OclBuiltinImpl native_pown_svml_expand = OclBuiltinImpl<native_pown, math_svml_vF_expand_native, 0, Expand2>;
OclBuiltinImpl native_pown_svml_promote = OclBuiltinImpl<native_pown, [v3f64], 0, Promote2_v3_to_v4>;

// native_powr f(vXfX)
OclBuiltinImpl native_powr_svml_expand = OclBuiltinImpl<native_powr, math_svml_vF_expand_native, 0, Expand2>;
OclBuiltinImpl native_powr_svml_promote = OclBuiltinImpl<native_powr, [v3f64], 0, Promote2_v3_to_v4>;

// native_tanh f(vXfX)
OclBuiltinImpl native_tanh_svml_expand = OclBuiltinImpl<native_tanh, math_svml_vF_expand_native, 0, Expand1>;
OclBuiltinImpl native_tanh_svml_promote = OclBuiltinImpl<native_tanh, [v3f64], 0, Promote1_v3_to_v4>;
