// vim:ts=2:sw=2:et:



//////////////////////////////////////////////////////////////////////////////////////////////
//                Float f32 section
//////////////////////////////////////////////////////////////////////////////////////////////

OclBuiltinImpl ceil_f32_promote = OclBuiltinImpl<ceil, [v3f32], 0, Promote1>;
OclBuiltinImpl ceil_f32_expand = OclBuiltinImpl<ceil, [v8f32, v16f32], 0, Expand1>;

OclBuiltinImpl trunc_f32_expand = OclBuiltinImpl<trunc, [v8f32], 0, Expand1>;

// rsqrt
// only doubles. floats are implemented natively
OclBuiltinImpl rsqrt_svml_v64_expand = OclBuiltinImpl<rsqrt, [v8f64], 0, Expand1>;
OclBuiltinImpl rsqrt_f32_expand = OclBuiltinImpl<rsqrt, [v8f32, v16f32], 0, Expand1>;


// intel_maskmove()
// non OpenCL builtin. Specialization for SSE42
// this builtin
code Expand_movemask = [{
    return intel_movemask($Arg0VarName$ExpandLoPattern) | intel_movemask($Arg0VarName$ExpandHiPattern);
  }];

OclBuiltinImpl intel_movemask_v4i32 = OclBuiltinImpl<intel_movemask, [v4i32], 0, [{
    return as_int(_mm_movemask_ps((__m128)$Arg0VarName));
  }]>;

OclBuiltinImpl intel_movemask_i32_expand = OclBuiltinImpl<intel_movemask, [v8i32, v16i32], 0, Expand_movemask>;

//fract
// float3 are promoted to float4
OclBuiltinImpl fract_pas_f32_promote = OclBuiltinImpl<fract_pas, [v3f32], 0, Promote_fract>;
OclBuiltinImpl fract_las_f32_promote = OclBuiltinImpl<fract_las, [v3f32], 0, Promote_fract>;
OclBuiltinImpl fract_gas_f32_promote = OclBuiltinImpl<fract_gas, [v3f32], 0, Promote_fract>;

// sqrt
OclBuiltinImpl sqrt_f32_expand = OclBuiltinImpl<sqrt, [v8f32, v16f32], 0, Expand1>;


//////////////////////////////////////////////////////////////////////////////////////////////
//                Double f64 section
//////////////////////////////////////////////////////////////////////////////////////////////
OclBuiltinImpl ceil_f64_promote = OclBuiltinImpl<ceil, [v1f64], 0, Promote1>;
OclBuiltinImpl ceil_f64_expand = OclBuiltinImpl<ceil, [v3f64, v4f64, v8f64, v16f64], 0, Expand1>;
OclBuiltinImpl trunc_f64_expand = OclBuiltinImpl<trunc, [v4f64], 0, Expand1>;
// sqrt
OclBuiltinImpl sqrt_f64_expand = OclBuiltinImpl<sqrt, [v4f64, v8f64], 0, Expand1>;


// intel_maskmove()
// non OpenCL builtin. Specialization for SSE42
// this builtin
OclBuiltinImpl intel_movemask_v2i64 = OclBuiltinImpl<intel_movemask, [v2i64], 0, [{
    return as_int(_mm_movemask_pd((__m128d)$Arg0VarName));
  }]>;

OclBuiltinImpl intel_movemask_i64_expand = OclBuiltinImpl<intel_movemask, [v4i64, v8i64, v16i64], 0, Expand_movemask>;

include "ssse3/code_promote.td"

// vXfX = f(vXfX)
list<OclType> math_svml_vF_expand = [v16f64, v8f64, v16f32];
list<OclType> math_svml_vF64_expand = [v16f64, v8f64];
list<OclType> math_svml_vF_promote = [v3f32, v3f64];
list<OclType> math_svml_vFRM_expand = [v16f32];

// vXfX = f(vXfX, vXfX)
list<OclType> math_svml_vFvF_expand = [v16f32, v8f64, v16f64];
list<OclType> math_svml_vFvF_promote = [v3f32, v3f64];
list<OclType> math_svml_vF32vF32_promote = [v3f32];

// vXfX = f(vXfX, vXfX, vXfX)
list<OclType> math_svml_vFvFvF_expand = [v8f32, v16f32, v4f64, v8f64, v16f64];
list<OclType> math_svml_vFvFvF_promote = [v3f32, v3f64];

// vXfX = f(vXfX, pvXfX)
list<OclType> math_svml_vFpvF_expand = [v8f64, v16f64, v16f32];
list<OclType> math_svml_vFpvF_promote = [v3f32, v3f64];

// vXfX = f(vXfX, pvXi32)
list<OclType> math_svml_vFpvI32_expand = [v16f64, v8f64, v16f32];
// vXfX = f(vXfX, vXi32)
list<OclType> math_svml_vFvI32_expand = [v2f32, v16f32, v2f64, v8f64, v16f64];
list<OclType> math_svml_vFvI32_promote = [v3f32, v3f64];
// vXfX = f(vXfX, vXfX, pvXi32)
list<OclType> math_svml_vFvFpvI32_expand = [v16f32, v8f64, v16f64];
list<OclType> math_svml_vFvFpvI32_promote = [v3f32, v3f64];

// vXfX = f(vXfX)
list<OclType> math_svml_vF_expand_shared = [v16f32, v8f64];
// vXfX = f(vXfX, vXfX)
list<OclType> math_svml_vFvF_expand_shared = [v16f32, v8f64];
list<OclType> math_svml_vFvI32_expand_shared = [v16f32, v8f64];

// acos f(vXfX)
OclBuiltinImpl acos_svml_expand = OclBuiltinImpl<acos, math_svml_vF_expand_shared, 0, Expand1>;

// acosh f(vXfX)
OclBuiltinImpl acosh_svml_expand = OclBuiltinImpl<acosh, math_svml_vF_expand_shared, 0, Expand1>;

// acospi f(vXfX)
OclBuiltinImpl acospi_svml_expand = OclBuiltinImpl<acospi, math_svml_vF_expand_shared, 0, Expand1>;

// asin f(vXfX)
OclBuiltinImpl asin_svml_expand = OclBuiltinImpl<asin, math_svml_vF_expand_shared, 0, Expand1>;

// asinh f(vXfX)
OclBuiltinImpl asinh_svml_expand = OclBuiltinImpl<asinh, math_svml_vF_expand_shared, 0, Expand1>;

// asinpi f(vXfX)
OclBuiltinImpl asinpi_svml_expand = OclBuiltinImpl<asinpi, math_svml_vF_expand_shared, 0, Expand1>;

// atan f(vXfX)
OclBuiltinImpl atan_svml_expand = OclBuiltinImpl<atan, math_svml_vF_expand_shared, 0, Expand1>;

// atan2 f(vXfX, vXfX)
OclBuiltinImpl atan2_svml_expand = OclBuiltinImpl<atan2, math_svml_vFvF_expand_shared, 0, Expand2>;

// atanh f(vXfX)
OclBuiltinImpl atanh_svml_expand = OclBuiltinImpl<atanh, math_svml_vF_expand_shared, 0, Expand1>;

// atanpi f(vXfX)
OclBuiltinImpl atanpi_svml_expand = OclBuiltinImpl<atanpi, math_svml_vF_expand_shared, 0, Expand1>;

// atan2pi f(vXfX, vXfX)
OclBuiltinImpl atan2pi_svml_hack_expand = OclBuiltinImpl<atan2pi, math_svml_vFvF_expand_shared, 0, Expand2>;

// cbrt f(vXfX)
OclBuiltinImpl cbrt_svml_expand = OclBuiltinImpl<cbrt, math_svml_vF_expand_shared, 0, Expand1>;

// cos f(vXfX)
OclBuiltinImpl cos_svml_expand = OclBuiltinImpl<cos, math_svml_vF_expand_shared, 0, Expand1>;

// cosh f(vXfX)
OclBuiltinImpl cosh_svml_expand = OclBuiltinImpl<cosh, math_svml_vF_expand_shared, 0, Expand1>;

// cospi f(vXfX)
OclBuiltinImpl cospi_svml_expand = OclBuiltinImpl<cospi, math_svml_vF_expand_shared, 0, Expand1>;

// erfc f(vXfX)
OclBuiltinImpl erfc_svml_expand = OclBuiltinImpl<erfc, math_svml_vF_expand_shared, 0, Expand1>;

// erf f(vXfX)
OclBuiltinImpl erf_svml_expand = OclBuiltinImpl<erf, math_svml_vF_expand_shared, 0, Expand1>;

// exp f(vXfX)
OclBuiltinImpl exp_svml_expand = OclBuiltinImpl<exp, math_svml_vF_expand_shared, 0, Expand1>;

// exp2 f(vXfX)
OclBuiltinImpl exp2_svml_expand = OclBuiltinImpl<exp2, math_svml_vF_expand_shared, 0, Expand1>;

// exp10 f(vXfX)
OclBuiltinImpl exp10_svml_expand = OclBuiltinImpl<exp10, math_svml_vF_expand_shared, 0, Expand1>;

// expm1 f(vXfX)
OclBuiltinImpl expm1_svml_expand = OclBuiltinImpl<expm1, math_svml_vF_expand_shared, 0, Expand1>;

// fma  f(vXfX, vXfX, vXfX)
OclBuiltinImpl fma_svml_expand = OclBuiltinImpl<fma, math_svml_vFvFvF_expand, 0, Expand3>;
OclBuiltinImpl fma_svml_promote = OclBuiltinImpl<fma, math_svml_vFvFvF_promote, 0, Promote3_v3_to_v4>;

// fmod f(vXfX, vXfX)
OclBuiltinImpl fmod_svml_hack_expand = OclBuiltinImpl<fmod, math_svml_vFvF_expand_shared, 0, Expand2>;

// fract f(vXfX, pvXfX)
OclBuiltinImpl fract_pas_f64_expand = OclBuiltinImpl<fract_pas, math_svml_vFpvF_expand, 0, Expand_2Ptr>;
OclBuiltinImpl fract_pas_f64_promote = OclBuiltinImpl<fract_pas, math_svml_vFpvF_promote, 0, Promote_2Ptr_v3_to_v4>;

OclBuiltinImpl fract_las_f64_expand = OclBuiltinImpl<fract_las, math_svml_vFpvF_expand, 0, Expand_2Ptr>;
OclBuiltinImpl fract_las_f64_promote = OclBuiltinImpl<fract_las, math_svml_vFpvF_promote, 0, Promote_2Ptr_v3_to_v4>;

OclBuiltinImpl fract_gas_f64_expand = OclBuiltinImpl<fract_gas, math_svml_vFpvF_expand, 0, Expand_2Ptr>;
OclBuiltinImpl fract_gas_f64_promote = OclBuiltinImpl<fract_gas, math_svml_vFpvF_promote, 0, Promote_2Ptr_v3_to_v4>;

// frexp f(vXfX, pvXI32)
OclBuiltinImpl frexp_pas_expand = OclBuiltinImpl<frexp_pas, math_svml_vFpvI32_expand, 0, Expand_frexp_lgamma >;
OclBuiltinImpl frexp_pas_promote = OclBuiltinImpl<frexp_pas, [v3f64], 0, Promote_2Ptr_v3_to_v4 >;

OclBuiltinImpl frexp_gas_expand = OclBuiltinImpl<frexp_gas, math_svml_vFpvI32_expand, 0, Expand_frexp_lgamma >;
OclBuiltinImpl frexp_gas_promote = OclBuiltinImpl<frexp_gas, [v3f64], 0, Promote_2Ptr_v3_to_v4 >;

OclBuiltinImpl frexp_las_expand = OclBuiltinImpl<frexp_las, math_svml_vFpvI32_expand, 0, Expand_frexp_lgamma >;
OclBuiltinImpl frexp_las_promote = OclBuiltinImpl<frexp_las, [v3f64], 0, Promote_2Ptr_v3_to_v4 >;

// lgamma f(vXfX)
OclBuiltinImpl lgamma_svml_expand = OclBuiltinImpl<lgamma, math_svml_vF_expand_shared, 0, Expand1>;

// lgamma f(vXfX, pvXI32)
OclBuiltinImpl lgamma_r_pas_expand = OclBuiltinImpl<lgamma_r_pas, math_svml_vFpvI32_expand, 0, Expand_frexp_lgamma >;
OclBuiltinImpl lgamma_r_pas_promote = OclBuiltinImpl<lgamma_r_pas, [v3f64], 0, Promote_2Ptr_v3_to_v4 >;

OclBuiltinImpl lgamma_r_gas_expand = OclBuiltinImpl<lgamma_r_gas, math_svml_vFpvI32_expand, 0, Expand_frexp_lgamma >;
OclBuiltinImpl lgamma_r_gas_promote = OclBuiltinImpl<lgamma_r_gas, [v3f64], 0, Promote_2Ptr_v3_to_v4 >;

OclBuiltinImpl lgamma_r_las_expand = OclBuiltinImpl<lgamma_r_las, math_svml_vFpvI32_expand, 0, Expand_frexp_lgamma >;
OclBuiltinImpl lgamma_r_las_promote = OclBuiltinImpl<lgamma_r_las, [v3f64], 0, Promote_2Ptr_v3_to_v4 >;

// log f(vXfX)
OclBuiltinImpl log_svml_expand = OclBuiltinImpl<log, math_svml_vF_expand_shared, 0, Expand1>;

// log2 f(vXfX)
OclBuiltinImpl log2_svml_expand = OclBuiltinImpl<log2, math_svml_vF_expand_shared, 0, Expand1>;

// log10 f(vXfX)
OclBuiltinImpl log10_svml_expand = OclBuiltinImpl<log10, math_svml_vF_expand_shared, 0, Expand1>;

// log1p f(vXfX)
OclBuiltinImpl log1p_svml_expand = OclBuiltinImpl<log1p, math_svml_vF_expand_shared, 0, Expand1>;

// logb f(vXfX)
OclBuiltinImpl logb_svml_expand = OclBuiltinImpl<logb, math_svml_vF_expand_shared, 0, Expand1>;

// modf f(vXfX, pvXfX)
OclBuiltinImpl modf_pas_f64_expand = OclBuiltinImpl<modf_pas, math_svml_vFpvF_expand, 0, Expand_2Ptr>;
OclBuiltinImpl modf_pas_f64_promote = OclBuiltinImpl<modf_pas, math_svml_vFpvF_promote, 0, Promote_2Ptr_v3_to_v4>;

OclBuiltinImpl modf_las_f64_expand = OclBuiltinImpl<modf_las, math_svml_vFpvF_expand, 0, Expand_2Ptr>;
OclBuiltinImpl modf_las_f64_promote = OclBuiltinImpl<modf_las, math_svml_vFpvF_promote, 0, Promote_2Ptr_v3_to_v4>;

OclBuiltinImpl modf_gas_f64_expand = OclBuiltinImpl<modf_gas, math_svml_vFpvF_expand, 0, Expand_2Ptr>;
OclBuiltinImpl modf_gas_f64_promote = OclBuiltinImpl<modf_gas, math_svml_vFpvF_promote, 0, Promote_2Ptr_v3_to_v4>;

// remquo f(vXfX, vXfX, pvI32X)
OclBuiltinImpl remquo_pas_expand = OclBuiltinImpl<remquo_pas, math_svml_vFvFpvI32_expand, 0, Expand_remquo >;
OclBuiltinImpl remquo_pas_promote = OclBuiltinImpl<remquo_pas, math_svml_vFvFpvI32_promote, 0, Promote_remquo_v3_to_v4 >;

OclBuiltinImpl remquo_gas_expand = OclBuiltinImpl<remquo_gas, math_svml_vFvFpvI32_expand, 0, Expand_remquo >;
OclBuiltinImpl remquo_gas_promote = OclBuiltinImpl<remquo_gas, math_svml_vFvFpvI32_promote, 0, Promote_remquo_v3_to_v4 >;

OclBuiltinImpl remquo_las_expand = OclBuiltinImpl<remquo_las, math_svml_vFvFpvI32_expand, 0, Expand_remquo >;
OclBuiltinImpl remquo_las_promote = OclBuiltinImpl<remquo_las, math_svml_vFvFpvI32_promote, 0, Promote_remquo_v3_to_v4 >;

// sin f(vXfX)
OclBuiltinImpl sin_svml_expand = OclBuiltinImpl<sin, math_svml_vF_expand_shared, 0, Expand1>;

// sincos
OclBuiltinImpl sincos_pas_f64_expand = OclBuiltinImpl<sincos_pas, math_svml_vFpvF_expand, 0, Expand_2Ptr>;
OclBuiltinImpl sincos_pas_f64_promote = OclBuiltinImpl<sincos_pas, math_svml_vFpvF_promote, 0, Promote_2Ptr_v3_to_v4>;

OclBuiltinImpl sincos_las_f64_expand = OclBuiltinImpl<sincos_las, math_svml_vFpvF_expand, 0, Expand_2Ptr>;
OclBuiltinImpl sincos_las_f64_promote = OclBuiltinImpl<sincos_las, math_svml_vFpvF_promote, 0, Promote_2Ptr_v3_to_v4>;

OclBuiltinImpl sincos_gas_f64_expand = OclBuiltinImpl<sincos_gas, math_svml_vFpvF_expand, 0, Expand_2Ptr>;
OclBuiltinImpl sincos_gas_f64_promote = OclBuiltinImpl<sincos_gas, math_svml_vFpvF_promote, 0, Promote_2Ptr_v3_to_v4>;

// sin f(vXfX)
OclBuiltinImpl sinh_svml_expand = OclBuiltinImpl<sinh, math_svml_vF_expand_shared, 0, Expand1>;

// sinpi f(vXfX)
OclBuiltinImpl sinpi_svml_expand = OclBuiltinImpl<sinpi, math_svml_vF_expand_shared, 0, Expand1>;

// tan f(vXfX)
OclBuiltinImpl tan_svml_expand = OclBuiltinImpl<tan, math_svml_vF_expand_shared, 0, Expand1>;

// tanh f(vXfX)
OclBuiltinImpl tanh_svml_expand = OclBuiltinImpl<tanh, math_svml_vF_expand_shared, 0, Expand1>;

// tanpi f(vXfX)
OclBuiltinImpl tanpi_svml_expand = OclBuiltinImpl<tanpi, math_svml_vF_expand_shared, 0, Expand1>;

///////////////////////////////////////////////////////////////////////////////////////
// native
// native_sincos_pas f(vXfX)
OclBuiltinImpl native_sincos_pas_svml_expand = OclBuiltinImpl<native_sincos_pas, math_svml_vF_expand, 0, Expand_2Ptr>;
OclBuiltinImpl native_sincos_pas_svml_promote = OclBuiltinImpl<native_sincos_pas, [v3f64], 0, Promote_2Ptr_v3_to_v4>;

// native_sincos_las f(vXfX)
OclBuiltinImpl native_sincos_las_svml_expand = OclBuiltinImpl<native_sincos_las, math_svml_vF_expand, 0, Expand_2Ptr>;
OclBuiltinImpl native_sincos_las_svml_promote = OclBuiltinImpl<native_sincos_las, [v3f64], 0, Promote_2Ptr_v3_to_v4>;

// native_sincos_gas f(vXfX)
OclBuiltinImpl native_sincos_gas_svml_expand = OclBuiltinImpl<native_sincos_gas, math_svml_vF_expand, 0, Expand_2Ptr>;
OclBuiltinImpl native_sincos_gas_svml_promote = OclBuiltinImpl<native_sincos_gas, [v3f64], 0, Promote_2Ptr_v3_to_v4>;



// OpenCL 2.0 -cl-fast-relax-math functions
// cos_rm
OclBuiltinImpl cos_svml_rm_expand = OclBuiltinImpl<cos_rm, math_svml_vFRM_expand, 0, Expand1>;

// exp_rm
OclBuiltinImpl exp_svml_rm_expand = OclBuiltinImpl<exp_rm, math_svml_vFRM_expand, 0, Expand1>;

// exp2_rm
OclBuiltinImpl exp2_svml_rm_expand = OclBuiltinImpl<exp2_rm, math_svml_vFRM_expand, 0, Expand1>;

// exp10_rm
OclBuiltinImpl exp10_svml_rm_expand = OclBuiltinImpl<exp10_rm, math_svml_vFRM_expand, 0, Expand1>;

// log_rm
OclBuiltinImpl log_svml_rm_expand = OclBuiltinImpl<log_rm, math_svml_vFRM_expand, 0, Expand1>;

// log2_rm
OclBuiltinImpl log2_svml_rm_expand = OclBuiltinImpl<log2_rm, math_svml_vFRM_expand, 0, Expand1>;

// pow_rm
OclBuiltinImpl pow_svml_rm_expand = OclBuiltinImpl<pow_rm, math_svml_vFRM_expand, 0, Expand2>;

// sin_rm
OclBuiltinImpl sin_svml_rm_expand = OclBuiltinImpl<sin_rm, math_svml_vFRM_expand, 0, Expand1>;

// sincos_rm
OclBuiltinImpl sincos_pas_svml_rm_expand = OclBuiltinImpl<sincos_pas_rm, math_svml_vFRM_expand, 0, Expand_2Ptr>;
OclBuiltinImpl sincos_las_svml_rm_expand = OclBuiltinImpl<sincos_las_rm, math_svml_vFRM_expand, 0, Expand_2Ptr>;
OclBuiltinImpl sincos_gas_svml_rm_expand = OclBuiltinImpl<sincos_gas_rm, math_svml_vFRM_expand, 0, Expand_2Ptr>;

// tan_rm
OclBuiltinImpl tan_svml_rm_expand = OclBuiltinImpl<tan_rm, math_svml_vFRM_expand, 0, Expand1>;
