//////////////////////////////////////////////////////////////////////////////////////////////
// AVX-512F math implementation
//////////////////////////////////////////////////////////////////////////////////////////////

// sqrt
OclBuiltinImpl sqrt_v8f64_impl = OclBuiltinImpl<sqrt, [v8f64], 0, [{
    return _mm512_sqrt_pd($Arg0VarName);
  }]>;

OclBuiltinImpl sqrt_v16f32_impl = OclBuiltinImpl<sqrt, [v16f32], 0, [{
    return _mm512_sqrt_ps($Arg0VarName);
  }]>;

// native_recip
OclBuiltinImpl native_recip_v16f32_impl = OclBuiltinImpl<native_recip, [v16f32], 0, [{
    return _mm512_rcp14_ps($Arg0VarName);
  }]>;

// native_rsqrt
OclBuiltinImpl native_rsqrt_v16f32_impl = OclBuiltinImpl<native_rsqrt, [v16f32], 0, [{
    return _mm512_rsqrt14_ps($Arg0VarName);
  }]>;

// native_divide
OclBuiltinImpl native_divide_v16f32_impl = OclBuiltinImpl<native_divide, [v16f32], 0, [{
    $ReturnType $ReturnVarName = _mm512_rcp14_ps($Arg1VarName);
    return $Arg0VarName * $ReturnVarName;
  }]>;

// ceil float16
OclBuiltinImpl ceil_v16f32_impl = OclBuiltinImpl<ceil, [v16f32], 0, [{
  return _mm512_ceil_ps($Arg0VarName);
  }]>;

// ceil double8
OclBuiltinImpl ceil_v8f64_impl = OclBuiltinImpl<ceil, [v8f64], 0, [{
  return _mm512_ceil_pd($Arg0VarName);
  }]>;

// floor float16
OclBuiltinImpl floor_v16f32_impl = OclBuiltinImpl<floor, [v16f32], 0, [{
  return _mm512_floor_ps($Arg0VarName);
  }]>;

// floor double8
OclBuiltinImpl floor_v8f64_impl = OclBuiltinImpl<floor, [v8f64], 0, [{
  return _mm512_floor_pd($Arg0VarName);
  }]>;

// fma float16
OclBuiltinImpl fma_v16f32_impl = OclBuiltinImpl<fma, [v16f32], 0, [{
    return _mm512_fmadd_ps($Arg0VarName, $Arg1VarName, $Arg2VarName);
  }]>;

// fma double8
OclBuiltinImpl fma_v8f64_impl = OclBuiltinImpl<fma, [v8f64], 0, [{
    return _mm512_fmadd_pd($Arg0VarName, $Arg1VarName, $Arg2VarName);
  }]>;

// SVML calls

list<OclType> math_FP512_types = [v8f64, v16f32];

// acos
OclBuiltinImpl acos_svml = OclBuiltinImpl<acos, math_FP512_types, 0, [{
    return __ocl_svml_$Target_acos$SVMLSuffix($Arg0VarName);
  }]>;

// acosh
OclBuiltinImpl acosh_svml = OclBuiltinImpl<acosh, math_FP512_types, 0, [{
    return __ocl_svml_$Target_acosh$SVMLSuffix($Arg0VarName);
  }]>;

// acospi
OclBuiltinImpl acospi_svml = OclBuiltinImpl<acospi, math_FP512_types, 0, [{
    return __ocl_svml_$Target_acospi$SVMLSuffix($Arg0VarName);
  }]>;

// asin
OclBuiltinImpl asin_svml = OclBuiltinImpl<asin, math_FP512_types, 0, [{
    return __ocl_svml_$Target_asin$SVMLSuffix($Arg0VarName);
  }]>;

// asinh
OclBuiltinImpl asinh_svml = OclBuiltinImpl<asinh, math_FP512_types, 0, [{
    return __ocl_svml_$Target_asinh$SVMLSuffix($Arg0VarName);
  }]>;

// asinpi
OclBuiltinImpl asinpi_svml = OclBuiltinImpl<asinpi, math_FP512_types, 0, [{
    return __ocl_svml_$Target_asinpi$SVMLSuffix($Arg0VarName);
  }]>;

// atan
OclBuiltinImpl atan_svml = OclBuiltinImpl<atan, math_FP512_types, 0, [{
    return __ocl_svml_$Target_atan$SVMLSuffix($Arg0VarName);
  }]>;

// atan2
OclBuiltinImpl atan2_svml = OclBuiltinImpl<atan2, math_FP512_types, 0, [{
    return __ocl_svml_$Target_atan2$SVMLSuffix($Arg0VarName, $Arg1VarName);
  }]>;

// atanh
OclBuiltinImpl atanh_svml = OclBuiltinImpl<atanh, math_FP512_types, 0, [{
    return __ocl_svml_$Target_atanh$SVMLSuffix($Arg0VarName);
  }]>;

// atanpi
OclBuiltinImpl atanpi_svml = OclBuiltinImpl<atanpi, math_FP512_types, 0, [{
    return __ocl_svml_$Target_atanpi$SVMLSuffix($Arg0VarName);
  }]>;

// atan2pi
OclBuiltinImpl atan2pi_svml = OclBuiltinImpl<atan2pi, math_FP512_types, 0, [{
    return __ocl_svml_$Target_atan2pi$SVMLSuffix($Arg0VarName, $Arg1VarName);
  }]>;

// cbrt
OclBuiltinImpl cbrt_svml = OclBuiltinImpl<cbrt, math_FP512_types, 0, [{
    return __ocl_svml_$Target_cbrt$SVMLSuffix($Arg0VarName);
  }]>;

// cos
OclBuiltinImpl cos_svml = OclBuiltinImpl<cos, math_FP512_types, 0, [{
    return __ocl_svml_$Target_cos$SVMLSuffix($Arg0VarName);
  }]>;

// cosh
OclBuiltinImpl cosh_svml = OclBuiltinImpl<cosh, math_FP512_types, 0, [{
    return __ocl_svml_$Target_cosh$SVMLSuffix($Arg0VarName);
  }]>;

// cospi
OclBuiltinImpl cospi_svml = OclBuiltinImpl<cospi, math_FP512_types, 0, [{
    return __ocl_svml_$Target_cospi$SVMLSuffix($Arg0VarName);
  }]>;

// erfc
OclBuiltinImpl erfc_svml = OclBuiltinImpl<erfc, math_FP512_types, 0, [{
    return __ocl_svml_$Target_erfc$SVMLSuffix($Arg0VarName);
  }]>;

// erf
OclBuiltinImpl erf_svml = OclBuiltinImpl<erf, math_FP512_types, 0, [{
    return __ocl_svml_$Target_erf$SVMLSuffix($Arg0VarName);
  }]>;

// exp
OclBuiltinImpl exp_svml = OclBuiltinImpl<exp, math_FP512_types, 0, [{
    return __ocl_svml_$Target_exp$SVMLSuffix($Arg0VarName);
  }]>;

// exp2
OclBuiltinImpl exp2_svml = OclBuiltinImpl<exp2, math_FP512_types, 0, [{
    return __ocl_svml_$Target_exp2$SVMLSuffix($Arg0VarName);
  }]>;

// exp10
OclBuiltinImpl exp10_svml = OclBuiltinImpl<exp10, math_FP512_types, 0, [{
    return __ocl_svml_$Target_exp10$SVMLSuffix($Arg0VarName);
  }]>;

// expm1
OclBuiltinImpl expm1_svml = OclBuiltinImpl<expm1, math_FP512_types, 0, [{
    return __ocl_svml_$Target_expm1$SVMLSuffix($Arg0VarName);
  }]>;

// fmod
OclBuiltinImpl fmod_svml = OclBuiltinImpl<fmod, math_FP512_types, 0, [{
    return __ocl_svml_$Target_fmod$SVMLSuffix($Arg0VarName, $Arg1VarName);
  }]>;

// hypot
// NOTE: there is fast inlined implementation of hypot using newton-raphson and quick native_rsqrt function
OclBuiltinImpl hypot_svml = OclBuiltinImpl<hypot, math_FP512_types, 0, [{
    return __ocl_svml_$Target_hypot$SVMLSuffix($Arg0VarName, $Arg1VarName);
  }]>;

// ilogb double
OclBuiltinImpl ilogb_svml_f64 = OclBuiltinImpl<ilogb, [v8f64],  0, [{
    return __ocl_svml_$Target_ilogb$SVMLSuffix($Arg0VarName);
  }]>;

// ldexp
OclBuiltinImpl ldexp_svml = OclBuiltinImpl<ldexp, math_FP512_types, 0, [{
    return __ocl_svml_$Target_ldexp$SVMLSuffix($Arg0VarName, $Arg1VarName);
  }]>;

// lgamma
OclBuiltinImpl lgamma_svml = OclBuiltinImpl<lgamma, math_FP512_types, 0, [{
    return __ocl_svml_$Target_lgamma$SVMLSuffix($Arg0VarName);
  }]>;

// log
OclBuiltinImpl log_svml = OclBuiltinImpl<log, math_FP512_types, 0, [{
    return __ocl_svml_$Target_log$SVMLSuffix($Arg0VarName);
  }]>;

// log2
OclBuiltinImpl log2_svml = OclBuiltinImpl<log2, math_FP512_types, 0, [{
    return __ocl_svml_$Target_log2$SVMLSuffix($Arg0VarName);
  }]>;

// log10
OclBuiltinImpl log10_svml = OclBuiltinImpl<log10, math_FP512_types, 0, [{
    return __ocl_svml_$Target_log10$SVMLSuffix($Arg0VarName);
  }]>;

// log1p
OclBuiltinImpl log1p_svml = OclBuiltinImpl<log1p, math_FP512_types, 0, [{
    return __ocl_svml_$Target_log1p$SVMLSuffix($Arg0VarName);
  }]>;

// logb
OclBuiltinImpl logb_svml = OclBuiltinImpl<logb, math_FP512_types, 0, [{
    return __ocl_svml_$Target_logb$SVMLSuffix($Arg0VarName);
  }]>;

// nextafter
OclBuiltinImpl nextafter_svml = OclBuiltinImpl<nextafter, math_FP512_types, 0, [{
    return __ocl_svml_$Target_nextafter$SVMLSuffix($Arg0VarName, $Arg1VarName);
  }]>;

// pow
OclBuiltinImpl pow_svml = OclBuiltinImpl<pow, math_FP512_types, 0, [{
    return __ocl_svml_$Target_pow$SVMLSuffix($Arg0VarName, $Arg1VarName);
  }]>;

// pown types is a special case, math_FP512_types are not used
// pown
OclBuiltinImpl pown_svml = OclBuiltinImpl<pown, math_FP512_types, 0, [{
    return __ocl_svml_$Target_pown$SVMLSuffix($Arg0VarName, $Arg1VarName);
  }]>;

// powr
OclBuiltinImpl powr_svml = OclBuiltinImpl<powr, math_FP512_types, 0, [{
    return __ocl_svml_$Target_powr$SVMLSuffix($Arg0VarName, $Arg1VarName);
  }]>;

// remainder
OclBuiltinImpl remainder_svml = OclBuiltinImpl<remainder, math_FP512_types, 0, [{
    return __ocl_svml_$Target_remainder$SVMLSuffix($Arg0VarName, $Arg1VarName);
  }]>;

// remquo
code remquo_svml = [{
    return __ocl_svml_$Target_remquo$SVMLSuffix($Arg0VarName, $Arg1VarName, ($Arg2NoASType)$Arg2VarName);
  }];
OclBuiltinImpl remquo_pas_svml = OclBuiltinImpl<remquo_pas, math_FP512_types, 0, remquo_svml>;
OclBuiltinImpl remquo_las_svml = OclBuiltinImpl<remquo_las, math_FP512_types, 0, remquo_svml>;
OclBuiltinImpl remquo_gas_svml = OclBuiltinImpl<remquo_gas, math_FP512_types, 0, remquo_svml>;

// rint
OclBuiltinImpl rint_svml = OclBuiltinImpl<rint, math_FP512_types, 0, [{
    return __ocl_svml_$Target_rint$SVMLSuffix($Arg0VarName);
  }]>;

OclBuiltinImpl rootn_svml = OclBuiltinImpl<rootn, math_FP512_types, 0, [{
    return __ocl_svml_$Target_rootn$SVMLSuffix($Arg0VarName, $Arg1VarName);
  }]>;

// round
OclBuiltinImpl round_svml = OclBuiltinImpl<round, math_FP512_types, 0, [{
    return __ocl_svml_$Target_round$SVMLSuffix($Arg0VarName);
  }]>;

// rsqrt
OclBuiltinImpl rsqrt_svml = OclBuiltinImpl<rsqrt, math_FP512_types, 0, [{
    return __ocl_svml_$Target_rsqrt$SVMLSuffix($Arg0VarName);
  }]>;

// sin
OclBuiltinImpl sin_svml = OclBuiltinImpl<sin, math_FP512_types, 0, [{
    return __ocl_svml_$Target_sin$SVMLSuffix($Arg0VarName);
  }]>;

// sinh
OclBuiltinImpl sinh_svml = OclBuiltinImpl<sinh, math_FP512_types, 0, [{
    return __ocl_svml_$Target_sinh$SVMLSuffix($Arg0VarName);
  }]>;

// sinpi
OclBuiltinImpl sinpi_svml = OclBuiltinImpl<sinpi, math_FP512_types, 0, [{
    return __ocl_svml_$Target_sinpi$SVMLSuffix($Arg0VarName);
  }]>;

// tan
OclBuiltinImpl tan_svml = OclBuiltinImpl<tan, math_FP512_types, 0, [{
    return __ocl_svml_$Target_tan$SVMLSuffix($Arg0VarName);
  }]>;

// tanh
OclBuiltinImpl tanh_svml = OclBuiltinImpl<tanh, math_FP512_types, 0, [{
    return __ocl_svml_$Target_tanh$SVMLSuffix($Arg0VarName);
  }]>;

// tanpi
OclBuiltinImpl tanpi_svml = OclBuiltinImpl<tanpi, math_FP512_types, 0, [{
    return __ocl_svml_$Target_tanpi$SVMLSuffix($Arg0VarName);
  }]>;

// tgamma
OclBuiltinImpl tgamma_svml = OclBuiltinImpl<tgamma, math_FP512_types, 0, [{
    return __ocl_svml_$Target_tgamma$SVMLSuffix($Arg0VarName);
  }]>;

// TODO: calls to trunc svml functions can be replaced with avx512f intrinsic
// after upgrade to llvm supported it to avoid redundant calls
/* trunc float16
OclBuiltinImpl trunc_v16f32_impl = OclBuiltinImpl<trunc, [v16f32], 0, [{
  // trancate, round to nearest smallest magnitude integer
    return _mm512_roundscale_round_ps($Arg0VarName, 0x3, _MM_FROUND_TO_ZERO);
  }]>;

// trunc double
OclBuiltinImpl trunc_v8f64_impl = OclBuiltinImpl<trunc, [v8f64], 0, [{
  // trancate, round to nearest smallest magnitude integer
    return _mm512_roundscale_round_pd($Arg0VarName, 0x3, _MM_FROUND_TO_ZERO);
  }]>;
*/
// trunc
OclBuiltinImpl trunc_svml = OclBuiltinImpl<trunc, math_FP512_types, 0, [{
    return __ocl_svml_$Target_trunc$SVMLSuffix($Arg0VarName);
  }]>;

// half_cos
OclBuiltinImpl half_cos_svml = OclBuiltinImpl<half_cos, [v16f32], 0, [{
    return __ocl_svml_$Target_cos$SVMLSuffix_half($Arg0VarName);
  }]>;

// half_divide
OclBuiltinImpl half_divide_svml = OclBuiltinImpl<half_divide, [v16f32], 0, [{
    return __ocl_svml_$Target_div$SVMLSuffix_half($Arg0VarName, $Arg1VarName);
  }]>;

// half_exp
OclBuiltinImpl half_exp_svml = OclBuiltinImpl<half_exp, [v16f32], 0, [{
    return __ocl_svml_$Target_exp$SVMLSuffix_half($Arg0VarName);
  }]>;

// half_exp2
OclBuiltinImpl half_exp2_svml = OclBuiltinImpl<half_exp2, [v16f32], 0, [{
    return __ocl_svml_$Target_exp2$SVMLSuffix_half($Arg0VarName);
  }]>;

// half_exp10
OclBuiltinImpl half_exp10_svml = OclBuiltinImpl<half_exp10, [v16f32], 0, [{
    return __ocl_svml_$Target_exp10$SVMLSuffix_half($Arg0VarName);
  }]>;

// half_log
OclBuiltinImpl half_log_svml = OclBuiltinImpl<half_log, [v16f32], 0, [{
    return __ocl_svml_$Target_log$SVMLSuffix_half($Arg0VarName);
  }]>;

// half_log2
OclBuiltinImpl half_log2_svml = OclBuiltinImpl<half_log2, [v16f32], 0, [{
    return __ocl_svml_$Target_log2$SVMLSuffix_half($Arg0VarName);
  }]>;

// half_log10
OclBuiltinImpl half_log10_svml = OclBuiltinImpl<half_log10, [v16f32], 0, [{
    return __ocl_svml_$Target_log10$SVMLSuffix_half($Arg0VarName);
  }]>;

// half_powr
OclBuiltinImpl half_powr_svml = OclBuiltinImpl<half_powr, [v16f32], 0, [{
    return __ocl_svml_$Target_powr$SVMLSuffix_half($Arg0VarName, $Arg1VarName);
  }]>;

// half_sin
OclBuiltinImpl half_sin_svml = OclBuiltinImpl<half_sin, [v16f32], 0, [{
    return __ocl_svml_$Target_sin$SVMLSuffix_half($Arg0VarName);
  }]>;

// half_tan
OclBuiltinImpl half_tan_svml = OclBuiltinImpl<half_tan, [v16f32], 0, [{
    return __ocl_svml_$Target_tan$SVMLSuffix_half($Arg0VarName);
  }]>;


// native_cos
OclBuiltinImpl native_cos_svml = OclBuiltinImpl<native_cos, math_FP512_types, 0, [{
    return __ocl_svml_$Target_cos$SVMLSuffix_native($Arg0VarName);
  }]>;

// native_exp
OclBuiltinImpl native_exp_svml = OclBuiltinImpl<native_exp, math_FP512_types, 0, [{
    return __ocl_svml_$Target_exp$SVMLSuffix_native($Arg0VarName);
  }]>;

// native_exp2
OclBuiltinImpl native_exp2_svml = OclBuiltinImpl<native_exp2, math_FP512_types, 0, [{
    return __ocl_svml_$Target_exp2$SVMLSuffix_native($Arg0VarName);
  }]>;

// native_exp10
OclBuiltinImpl native_exp10_svml = OclBuiltinImpl<native_exp10, math_FP512_types, 0, [{
    return __ocl_svml_$Target_exp10$SVMLSuffix_native($Arg0VarName);
  }]>;

// native_log
OclBuiltinImpl native_log_svml = OclBuiltinImpl<native_log, math_FP512_types, 0, [{
    return __ocl_svml_$Target_log$SVMLSuffix_native($Arg0VarName);
  }]>;

// native_log2
OclBuiltinImpl native_log2_svml = OclBuiltinImpl<native_log2, math_FP512_types, 0, [{
    return __ocl_svml_$Target_log2$SVMLSuffix_native($Arg0VarName);
  }]>;

// native_log10
OclBuiltinImpl native_log10_svml = OclBuiltinImpl<native_log10, math_FP512_types, 0, [{
    return __ocl_svml_$Target_log10$SVMLSuffix_native($Arg0VarName);
  }]>;

// native_powr
OclBuiltinImpl native_powr_svml = OclBuiltinImpl<native_powr, math_FP512_types, 0, [{
    return __ocl_svml_$Target_powr$SVMLSuffix_native($Arg0VarName, $Arg1VarName);
  }]>;

// native_sin
OclBuiltinImpl native_sin_svml = OclBuiltinImpl<native_sin, math_FP512_types, 0, [{
    return __ocl_svml_$Target_sin$SVMLSuffix_native($Arg0VarName);
  }]>;

// native_tan
OclBuiltinImpl native_tan_svml = OclBuiltinImpl<native_tan, math_FP512_types, 0, [{
    return __ocl_svml_$Target_tan$SVMLSuffix_native($Arg0VarName);
  }]>;

// native_acos
OclBuiltinImpl native_acos_svml = OclBuiltinImpl<native_acos, math_FP512_types, 0, [{
    return __ocl_svml_$Target_acos$SVMLSuffix_native($Arg0VarName);
  }]>;

// native_acosh
OclBuiltinImpl native_acosh_svml = OclBuiltinImpl<native_acosh, math_FP512_types, 0, [{
    return __ocl_svml_$Target_acosh$SVMLSuffix_native($Arg0VarName);
  }]>;

// native_acospi
OclBuiltinImpl native_acospi_svml = OclBuiltinImpl<native_acospi, math_FP512_types, 0, [{
    return __ocl_svml_$Target_acospi$SVMLSuffix_native($Arg0VarName);
  }]>;

// native_asin
OclBuiltinImpl native_asin_svml = OclBuiltinImpl<native_asin, math_FP512_types, 0, [{
    return __ocl_svml_$Target_asin$SVMLSuffix_native($Arg0VarName);
  }]>;

// native_asinh
OclBuiltinImpl native_asinh_svml = OclBuiltinImpl<native_asinh, math_FP512_types, 0, [{
    return __ocl_svml_$Target_asinh$SVMLSuffix_native($Arg0VarName);
  }]>;

// native_asinpi
OclBuiltinImpl native_asinpi_svml = OclBuiltinImpl<native_asinpi, math_FP512_types, 0, [{
    return __ocl_svml_$Target_asinpi$SVMLSuffix_native($Arg0VarName);
  }]>;

// native_atan
OclBuiltinImpl native_atan_svml = OclBuiltinImpl<native_atan, math_FP512_types, 0, [{
    return __ocl_svml_$Target_atan$SVMLSuffix_native($Arg0VarName);
  }]>;

// native_atan2
OclBuiltinImpl native_atan2_svml = OclBuiltinImpl<native_atan2, math_FP512_types, 0, [{
    return __ocl_svml_$Target_atan2$SVMLSuffix_native($Arg0VarName, $Arg1VarName);
  }]>;

// native_atanh
OclBuiltinImpl native_atanh_svml = OclBuiltinImpl<native_atanh, math_FP512_types, 0, [{
    return __ocl_svml_$Target_atanh$SVMLSuffix_native($Arg0VarName);
  }]>;

// native_atanpi
OclBuiltinImpl native_atanpi_svml = OclBuiltinImpl<native_atanpi, math_FP512_types, 0, [{
    return __ocl_svml_$Target_atanpi$SVMLSuffix_native($Arg0VarName);
  }]>;

// native_atan2pi
OclBuiltinImpl native_atan2pi_svml = OclBuiltinImpl<native_atan2pi, math_FP512_types, 0, [{
    return __ocl_svml_$Target_atan2pi$SVMLSuffix_native($Arg0VarName, $Arg0VarName);
  }]>;

// native_cbrt
OclBuiltinImpl native_cbrt_svml = OclBuiltinImpl<native_cbrt, math_FP512_types, 0, [{
    return __ocl_svml_$Target_cbrt$SVMLSuffix_native($Arg0VarName);
  }]>;

// native_cosh
OclBuiltinImpl native_cosh_svml = OclBuiltinImpl<native_cosh, math_FP512_types, 0, [{
    return __ocl_svml_$Target_cosh$SVMLSuffix_native($Arg0VarName);
  }]>;

// native_cospi
OclBuiltinImpl native_cospi_svml = OclBuiltinImpl<native_cospi, math_FP512_types, 0, [{
    return __ocl_svml_$Target_cospi$SVMLSuffix_native($Arg0VarName);
  }]>;

// native_erfc
OclBuiltinImpl native_erfc_svml = OclBuiltinImpl<native_erfc, math_FP512_types, 0, [{
    return __ocl_svml_$Target_erfc$SVMLSuffix_native($Arg0VarName);
  }]>;

// native_erf
OclBuiltinImpl native_erf_svml = OclBuiltinImpl<native_erf, math_FP512_types, 0, [{
    return __ocl_svml_$Target_erf$SVMLSuffix_native($Arg0VarName);
  }]>;

// native_expm1
OclBuiltinImpl native_expm1_svml = OclBuiltinImpl<native_expm1, math_FP512_types, 0, [{
    return __ocl_svml_$Target_expm1$SVMLSuffix_native($Arg0VarName);
  }]>;

// native_log1p
OclBuiltinImpl native_log1p_svml = OclBuiltinImpl<native_log1p, math_FP512_types, 0, [{
    return __ocl_svml_$Target_log1p$SVMLSuffix_native($Arg0VarName);
  }]>;

// native_pow
OclBuiltinImpl native_pow_svml = OclBuiltinImpl<native_pow, math_FP512_types, 0, [{
    return __ocl_svml_$Target_pow$SVMLSuffix_native($Arg0VarName, $Arg1VarName);
  }]>;

// native_pown
OclBuiltinImpl native_pown_svml = OclBuiltinImpl<native_pown, math_FP512_types, 0, [{
    return __ocl_svml_$Target_pown$SVMLSuffix_native($Arg0VarName, $Arg1VarName);
  }]>;

// native_rcbrt
OclBuiltinImpl native_rcbrt_svml = OclBuiltinImpl<native_rcbrt, math_FP512_types, 0, [{
    return __ocl_svml_$Target_rcbrt$SVMLSuffix_native($Arg0VarName);
  }]>;

// native_rootn
OclBuiltinImpl knl_native_rootn_svml = OclBuiltinImpl<native_rootn, math_FP512_types, 0, [{
    return __ocl_svml_$Target_rootn$SVMLSuffix_native($Arg0VarName, $Arg1VarName);
  }]>;

// native_sinh
OclBuiltinImpl native_sinh_svml = OclBuiltinImpl<native_sinh, math_FP512_types, 0, [{
    return __ocl_svml_$Target_sinh$SVMLSuffix_native($Arg0VarName);
  }]>;

// native_sinpi
OclBuiltinImpl native_sinpi_svml = OclBuiltinImpl<native_sinpi, math_FP512_types, 0, [{
    return __ocl_svml_$Target_sinpi$SVMLSuffix_native($Arg0VarName);
  }]>;

// native_tanh
OclBuiltinImpl native_tanh_svml = OclBuiltinImpl<native_tanh, math_FP512_types, 0, [{
    return __ocl_svml_$Target_tanh$SVMLSuffix_native($Arg0VarName);
  }]>;

// native_tanpi
OclBuiltinImpl native_tanpi_svml = OclBuiltinImpl<native_tanpi, math_FP512_types, 0, [{
    return __ocl_svml_$Target_tanpi$SVMLSuffix_native($Arg0VarName);
  }]>;

// native sqrt
OclBuiltinImpl native_sqrt_svml = OclBuiltinImpl<native_sqrt, [v16f32, v8f64], 0, [{
    return __ocl_svml_$Target_sqrt$SVMLSuffix_native($Arg0VarName);
  }]>;

// native divide double8
OclBuiltinImpl native_divide_svml = OclBuiltinImpl<native_divide, [v8f64], 0, [{
    return __ocl_svml_$Target_div$VecLength($Arg0VarName, $Arg1VarName);
  }]>;

// native recip double8
OclBuiltinImpl native_recip_svml = OclBuiltinImpl<native_recip, [v8f64], 0, [{
    return __ocl_svml_$Target_inv$VecLength($Arg0VarName);
  }]>;

// native rsqrt double8
OclBuiltinImpl native_rsqrt_svml = OclBuiltinImpl<native_rsqrt, [v8f64], 0, [{
    return __ocl_svml_$Target_rsqrt$VecLength($Arg0VarName);
  }]>;

// integer remainder
OclBuiltinImpl irem_svml = OclBuiltinImpl<irem, [v16i32], 0, [{
    return __ocl_svml_$Target_irem$VecLength($Arg0VarName, $Arg1VarName);
  }]>;

// unsigned integer remainder
OclBuiltinImpl urem_svml = OclBuiltinImpl<urem, [v16u32], 0, [{
    return __ocl_svml_$Target_urem$VecLength($Arg0VarName, $Arg1VarName);
  }]>;

// integer division
OclBuiltinImpl idiv_svml = OclBuiltinImpl<idiv, [v16i32], 0, [{
    return __ocl_svml_$Target_idiv$VecLength($Arg0VarName, $Arg1VarName);
  }]>;

// unsigned integer division
OclBuiltinImpl udiv_svml = OclBuiltinImpl<udiv, [v16u32], 0, [{
    return __ocl_svml_$Target_udiv$VecLength($Arg0VarName, $Arg1VarName);
  }]>;
