// vim:ts=2:sw=2:et:


// acos
OclBuiltinImpl acos_svml = OclBuiltinImpl<acos, acos.Types, 0, [{
    return __ocl_svml_$Target_acos$SVMLSuffix($Arg0VarName);
  }]>;

// acosh
OclBuiltinImpl acosh_svml = OclBuiltinImpl<acosh, acosh.Types, 0, [{
    return __ocl_svml_$Target_acosh$SVMLSuffix($Arg0VarName);
  }]>;

// acospi
OclBuiltinImpl acospi_svml = OclBuiltinImpl<acospi, acospi.Types, 0, [{
    return __ocl_svml_$Target_acospi$SVMLSuffix($Arg0VarName);
  }]>;

// asin
OclBuiltinImpl asin_svml = OclBuiltinImpl<asin, asin.Types, 0, [{
    return __ocl_svml_$Target_asin$SVMLSuffix($Arg0VarName);
  }]>;

// asinh
OclBuiltinImpl asinh_svml = OclBuiltinImpl<asinh, asinh.Types, 0, [{
    return __ocl_svml_$Target_asinh$SVMLSuffix($Arg0VarName);
  }]>;

// asinpi
OclBuiltinImpl asinpi_svml = OclBuiltinImpl<asinpi, asinpi.Types, 0, [{
    return __ocl_svml_$Target_asinpi$SVMLSuffix($Arg0VarName);
  }]>;

// atan
OclBuiltinImpl atan_svml = OclBuiltinImpl<atan, atan.Types, 0, [{
    return __ocl_svml_$Target_atan$SVMLSuffix($Arg0VarName);
  }]>;

// atan2
OclBuiltinImpl atan2_svml = OclBuiltinImpl<atan2, atan2.Types, 0, [{
    return __ocl_svml_$Target_atan2$SVMLSuffix($Arg0VarName, $Arg1VarName);
  }]>;

// atanh
OclBuiltinImpl atanh_svml = OclBuiltinImpl<atanh, atanh.Types, 0, [{
    return __ocl_svml_$Target_atanh$SVMLSuffix($Arg0VarName);
  }]>;

// atanpi
OclBuiltinImpl atanpi_svml = OclBuiltinImpl<atanpi, atanpi.Types, 0, [{
    return __ocl_svml_$Target_atanpi$SVMLSuffix($Arg0VarName);
  }]>;

// atan2pi
OclBuiltinImpl atan2pi_svml = OclBuiltinImpl<atan2pi, atan2pi.Types, 0, [{
    return __ocl_svml_$Target_atan2pi$SVMLSuffix($Arg0VarName, $Arg1VarName);
  }]>;

// cbrt
OclBuiltinImpl cbrt_svml = OclBuiltinImpl<cbrt, cbrt.Types, 0, [{
    return __ocl_svml_$Target_cbrt$SVMLSuffix($Arg0VarName);
  }]>;

// ceil
OclBuiltinImpl ceil_svml = OclBuiltinImpl<ceil, ceil.Types, 0, [{
    return __ocl_svml_$Target_ceil$SVMLSuffix($Arg0VarName);
  }]>;

// copysign
OclBuiltinImpl copysign_svml = OclBuiltinImpl<copysign, copysign.Types, 0, [{
    return __ocl_svml_$Target_copysign$SVMLSuffix($Arg0VarName, $Arg1VarName);
  }]>;

// cos
OclBuiltinImpl cos_svml = OclBuiltinImpl<cos, cos.Types, 0, [{
    return __ocl_svml_$Target_cos$SVMLSuffix($Arg0VarName);
  }]>;

// cosh
OclBuiltinImpl cosh_svml = OclBuiltinImpl<cosh, cosh.Types, 0, [{
    return __ocl_svml_$Target_cosh$SVMLSuffix($Arg0VarName);
  }]>;

// cospi
OclBuiltinImpl cospi_svml = OclBuiltinImpl<cospi, cospi.Types, 0, [{
    return __ocl_svml_$Target_cospi$SVMLSuffix($Arg0VarName);
  }]>;

// erfc
OclBuiltinImpl erfc_svml = OclBuiltinImpl<erfc, erfc.Types, 0, [{
    return __ocl_svml_$Target_erfc$SVMLSuffix($Arg0VarName);
  }]>;

// erf
OclBuiltinImpl erf_svml = OclBuiltinImpl<erf, erf.Types, 0, [{
    return __ocl_svml_$Target_erf$SVMLSuffix($Arg0VarName);
  }]>;

// exp
OclBuiltinImpl exp_svml = OclBuiltinImpl<exp, exp.Types, 0, [{
    return __ocl_svml_$Target_exp$SVMLSuffix($Arg0VarName);
  }]>;

// exp2
OclBuiltinImpl exp2_svml = OclBuiltinImpl<exp2, exp2.Types, 0, [{
    return __ocl_svml_$Target_exp2$SVMLSuffix($Arg0VarName);
  }]>;

// exp10
OclBuiltinImpl exp10_svml = OclBuiltinImpl<exp10, exp10.Types, 0, [{
    return __ocl_svml_$Target_exp10$SVMLSuffix($Arg0VarName);
  }]>;

// expm1
OclBuiltinImpl expm1_svml = OclBuiltinImpl<expm1, expm1.Types, 0, [{
    return __ocl_svml_$Target_expm1$SVMLSuffix($Arg0VarName);
  }]>;

// fabs
OclBuiltinImpl fabs_svml = OclBuiltinImpl<fabs, fabs.Types, 0, [{
    return __ocl_svml_$Target_fabs$SVMLSuffix($Arg0VarName);
  }]>;

// fdim
OclBuiltinImpl fdim_svml = OclBuiltinImpl<fdim, fdim.Types, 0, [{
    return __ocl_svml_$Target_fdim$SVMLSuffix($Arg0VarName, $Arg1VarName);
  }]>;

// floor
OclBuiltinImpl floor_svml = OclBuiltinImpl<floor, floor.Types, 0, [{
    return __ocl_svml_$Target_floor$SVMLSuffix($Arg0VarName);
  }]>;

// fma
OclBuiltinImpl fma_svml = OclBuiltinImpl<fma, fma.Types, 0, [{
    return __ocl_svml_$Target_fma$SVMLSuffix($Arg0VarName, $Arg1VarName, $Arg2VarName);
  }]>;

// fmax
OclBuiltinImpl fmax_svml = OclBuiltinImpl<fmax, fmax.Types, 0, [{
    return __ocl_svml_$Target_fmax$SVMLSuffix($Arg0VarName, $Arg1VarName);
  }]>;

// fmin
OclBuiltinImpl fmin_svml = OclBuiltinImpl<fmin, fmin.Types, 0, [{
    return __ocl_svml_$Target_fmin$SVMLSuffix($Arg0VarName, $Arg1VarName);
  }]>;

// fmod
OclBuiltinImpl fmod_svml = OclBuiltinImpl<fmod, fmod.Types, 0, [{
    return __ocl_svml_$Target_fmod$SVMLSuffix($Arg0VarName, $Arg1VarName);
  }]>;

// fract
code fract_svml = [{
    return __ocl_svml_$Target_fract$SVMLSuffix($Arg0VarName, ($Arg1NoASType)$Arg1VarName);
  }];
OclBuiltinImpl fract_pas_svml = OclBuiltinImpl<fract_pas, fract_pas.Types, 0, fract_svml>;
OclBuiltinImpl fract_las_svml = OclBuiltinImpl<fract_las, fract_las.Types, 0, fract_svml>;
OclBuiltinImpl fract_gas_svml = OclBuiltinImpl<fract_gas, fract_gas.Types, 0, fract_svml>;

// frexp
code frexp_svml = [{
    return __ocl_svml_$Target_frexp$SVMLSuffix($Arg0VarName, ($Arg1NoASType)$Arg1VarName);
  }];
OclBuiltinImpl frexp_pas_svml = OclBuiltinImpl<frexp_pas, frexp_pas.Types, 0, frexp_svml>;
OclBuiltinImpl frexp_las_svml = OclBuiltinImpl<frexp_las, frexp_las.Types, 0, frexp_svml>;
OclBuiltinImpl frexp_gas_svml = OclBuiltinImpl<frexp_gas, frexp_gas.Types, 0, frexp_svml>;

// hypot
OclBuiltinImpl hypot_svml = OclBuiltinImpl<hypot, hypot.Types, 0, [{
    return __ocl_svml_$Target_hypot$SVMLSuffix($Arg0VarName, $Arg1VarName);
  }]>;

// ilogb
OclBuiltinImpl ilogb_svml = OclBuiltinImpl<ilogb, ilogb.Types, 0, [{
    return __ocl_svml_$Target_ilogb$SVMLSuffix($Arg0VarName);
  }]>;

// ldexp
OclBuiltinImpl ldexp_svml = OclBuiltinImpl<ldexp, ldexp.Types, 0, [{
    return __ocl_svml_$Target_ldexp$SVMLSuffix($Arg0VarName, $Arg1VarName);
  }]>;

// lgamma
OclBuiltinImpl lgamma_svml = OclBuiltinImpl<lgamma, lgamma.Types, 0, [{
    return __ocl_svml_$Target_lgamma$SVMLSuffix($Arg0VarName);
  }]>;

// lgamma_r
code lgamma_r_svml = [{
    return __ocl_svml_$Target_lgamma_r$SVMLSuffix($Arg0VarName, ($Arg1NoASType)$Arg1VarName);
  }];
OclBuiltinImpl lgamma_r_pas_svml = OclBuiltinImpl<lgamma_r_pas, lgamma_r_pas.Types, 0, lgamma_r_svml>;
OclBuiltinImpl lgamma_r_las_svml = OclBuiltinImpl<lgamma_r_las, lgamma_r_las.Types, 0, lgamma_r_svml>;
OclBuiltinImpl lgamma_r_gas_svml = OclBuiltinImpl<lgamma_r_gas, lgamma_r_gas.Types, 0, lgamma_r_svml>;

// log
OclBuiltinImpl log_svml = OclBuiltinImpl<log, log.Types, 0, [{
    return __ocl_svml_$Target_log$SVMLSuffix($Arg0VarName);
  }]>;

// log2
OclBuiltinImpl log2_svml = OclBuiltinImpl<log2, log2.Types, 0, [{
    return __ocl_svml_$Target_log2$SVMLSuffix($Arg0VarName);
  }]>;

// log10
OclBuiltinImpl log10_svml = OclBuiltinImpl<log10, log10.Types, 0, [{
    return __ocl_svml_$Target_log10$SVMLSuffix($Arg0VarName);
  }]>;

// log1p
OclBuiltinImpl log1p_svml = OclBuiltinImpl<log1p, log1p.Types, 0, [{
    return __ocl_svml_$Target_log1p$SVMLSuffix($Arg0VarName);
  }]>;

// logb
OclBuiltinImpl logb_svml = OclBuiltinImpl<logb, logb.Types, 0, [{
    return __ocl_svml_$Target_logb$SVMLSuffix($Arg0VarName);
  }]>;

// mad
OclBuiltinImpl mad_svml = OclBuiltinImpl<mad, mad.Types, 0, [{
    return __ocl_svml_$Target_mad$SVMLSuffix($Arg0VarName, $Arg1VarName, $Arg2VarName);
  }]>;

// maxmag
OclBuiltinImpl maxmag_svml = OclBuiltinImpl<maxmag, maxmag.Types, 0, [{
    return __ocl_svml_$Target_maxmag$SVMLSuffix($Arg0VarName, $Arg1VarName);
  }]>;

// minmag
OclBuiltinImpl minmag_svml = OclBuiltinImpl<minmag, minmag.Types, 0, [{
    return __ocl_svml_$Target_minmag$SVMLSuffix($Arg0VarName, $Arg1VarName);
  }]>;

// modf
code modf_svml = [{
    return __ocl_svml_$Target_modf$SVMLSuffix($Arg0VarName, ($Arg1NoASType)$Arg1VarName);
  }];
OclBuiltinImpl modf_pas_svml = OclBuiltinImpl<modf_pas, modf_pas.Types, 0, modf_svml>;
OclBuiltinImpl modf_las_svml = OclBuiltinImpl<modf_las, modf_las.Types, 0, modf_svml>;
OclBuiltinImpl modf_gas_svml = OclBuiltinImpl<modf_gas, modf_gas.Types, 0, modf_svml>;

// nan
OclBuiltinImpl nan_svml = OclBuiltinImpl<nan, nan.Types, 0, [{
    return __ocl_svml_$Target_nan$SVMLSuffix($Arg0VarName);
  }]>;

// nextafter
OclBuiltinImpl nextafter_svml = OclBuiltinImpl<nextafter, nextafter.Types, 0, [{
    return __ocl_svml_$Target_nextafter$SVMLSuffix($Arg0VarName, $Arg1VarName);
  }]>;

// pow
OclBuiltinImpl pow_svml = OclBuiltinImpl<pow, pow.Types, 0, [{
    return __ocl_svml_$Target_pow$SVMLSuffix($Arg0VarName, $Arg1VarName);
  }]>;

// pown
OclBuiltinImpl pown_svml = OclBuiltinImpl<pown, pown.Types, 0, [{
    return __ocl_svml_$Target_pown$SVMLSuffix($Arg0VarName, $Arg1VarName);
  }]>;

// powr
OclBuiltinImpl powr_svml = OclBuiltinImpl<powr, powr.Types, 0, [{
    return __ocl_svml_$Target_powr$SVMLSuffix($Arg0VarName, $Arg1VarName);
  }]>;

// remainder
OclBuiltinImpl remainder_svml = OclBuiltinImpl<remainder, remainder.Types, 0, [{
    return __ocl_svml_$Target_remainder$SVMLSuffix($Arg0VarName, $Arg1VarName);
  }]>;

// remquo
code remquo_svml = [{
    return __ocl_svml_$Target_remquo$SVMLSuffix($Arg0VarName, $Arg1VarName, ($Arg2NoASType)$Arg2VarName);
  }];
OclBuiltinImpl remquo_pas_svml = OclBuiltinImpl<remquo_pas, remquo_pas.Types, 0, remquo_svml>;
OclBuiltinImpl remquo_las_svml = OclBuiltinImpl<remquo_las, remquo_las.Types, 0, remquo_svml>;
OclBuiltinImpl remquo_gas_svml = OclBuiltinImpl<remquo_gas, remquo_gas.Types, 0, remquo_svml>;

// rint
OclBuiltinImpl rint_svml = OclBuiltinImpl<rint, rint.Types, 0, [{
    return __ocl_svml_$Target_rint$SVMLSuffix($Arg0VarName);
  }]>;

// rootn
OclBuiltinImpl rootn_svml = OclBuiltinImpl<rootn, rootn.Types, 0, [{
    return __ocl_svml_$Target_rootn$SVMLSuffix($Arg0VarName, $Arg1VarName);
  }]>;

// round
OclBuiltinImpl round_svml = OclBuiltinImpl<round, round.Types, 0, [{
    return __ocl_svml_$Target_round$SVMLSuffix($Arg0VarName);
  }]>;

// rsqrt
OclBuiltinImpl rsqrt_svml = OclBuiltinImpl<rsqrt, rsqrt.Types, 0, [{
    return __ocl_svml_$Target_rsqrt$SVMLSuffix($Arg0VarName);
  }]>;

// sin
OclBuiltinImpl sin_svml = OclBuiltinImpl<sin, sin.Types, 0, [{
    return __ocl_svml_$Target_sin$SVMLSuffix($Arg0VarName);
  }]>;

// sincos
code sincos_svml = [{
    return __ocl_svml_$Target_sincos$SVMLSuffix($Arg0VarName, ($Arg1NoASType)$Arg1VarName);
  }];
OclBuiltinImpl sincos_pas_svml = OclBuiltinImpl<sincos_pas, sincos_pas.Types, 0, sincos_svml>;
OclBuiltinImpl sincos_las_svml = OclBuiltinImpl<sincos_las, sincos_las.Types, 0, sincos_svml>;
OclBuiltinImpl sincos_gas_svml = OclBuiltinImpl<sincos_gas, sincos_gas.Types, 0, sincos_svml>;

// sinh
OclBuiltinImpl sinh_svml = OclBuiltinImpl<sinh, sinh.Types, 0, [{
    return __ocl_svml_$Target_sinh$SVMLSuffix($Arg0VarName);
  }]>;

// sinpi
OclBuiltinImpl sinpi_svml = OclBuiltinImpl<sinpi, sinpi.Types, 0, [{
    return __ocl_svml_$Target_sinpi$SVMLSuffix($Arg0VarName);
  }]>;

// sqrt
OclBuiltinImpl sqrt_svml = OclBuiltinImpl<sqrt, sqrt.Types, 0, [{
    return __ocl_svml_$Target_sqrt$SVMLSuffix($Arg0VarName);
  }]>;

// tan
OclBuiltinImpl tan_svml = OclBuiltinImpl<tan, tan.Types, 0, [{
    return __ocl_svml_$Target_tan$SVMLSuffix($Arg0VarName);
  }]>;

// tanh
OclBuiltinImpl tanh_svml = OclBuiltinImpl<tanh, tanh.Types, 0, [{
    return __ocl_svml_$Target_tanh$SVMLSuffix($Arg0VarName);
  }]>;

// tanpi
OclBuiltinImpl tanpi_svml = OclBuiltinImpl<tanpi, tanpi.Types, 0, [{
    return __ocl_svml_$Target_tanpi$SVMLSuffix($Arg0VarName);
  }]>;

// tgamma
OclBuiltinImpl tgamma_svml = OclBuiltinImpl<tgamma, tgamma.Types, 0, [{
    return __ocl_svml_$Target_tgamma$SVMLSuffix($Arg0VarName);
  }]>;

// trunc
OclBuiltinImpl trunc_svml = OclBuiltinImpl<trunc, trunc.Types, 0, [{
    return __ocl_svml_$Target_trunc$SVMLSuffix($Arg0VarName);
  }]>;

// half_cos
OclBuiltinImpl half_cos_svml = OclBuiltinImpl<half_cos, half_cos.Types, 0, [{
    return __ocl_svml_$Target_cos$SVMLSuffix_native($Arg0VarName);
  }]>;

// half_divide
OclBuiltinImpl half_divide_svml = OclBuiltinImpl<half_divide, half_divide.Types, 0, [{
    return __ocl_svml_$Target_div$SVMLSuffix_native($Arg0VarName, $Arg1VarName);
  }]>;

// half_exp
OclBuiltinImpl half_exp_svml = OclBuiltinImpl<half_exp, half_exp.Types, 0, [{
    return __ocl_svml_$Target_exp$SVMLSuffix_native($Arg0VarName);
  }]>;

// half_exp2
OclBuiltinImpl half_exp2_svml = OclBuiltinImpl<half_exp2, half_exp2.Types, 0, [{
    return __ocl_svml_$Target_exp2$SVMLSuffix_native($Arg0VarName);
  }]>;

// half_exp10
OclBuiltinImpl half_exp10_svml = OclBuiltinImpl<half_exp10, half_exp10.Types, 0, [{
    return __ocl_svml_$Target_exp10$SVMLSuffix_native($Arg0VarName);
  }]>;

// half_log
OclBuiltinImpl half_log_svml = OclBuiltinImpl<half_log, half_log.Types, 0, [{
    return __ocl_svml_$Target_log$SVMLSuffix_native($Arg0VarName);
  }]>;

// half_log2
OclBuiltinImpl half_log2_svml = OclBuiltinImpl<half_log2, half_log2.Types, 0, [{
    return __ocl_svml_$Target_log2$SVMLSuffix_native($Arg0VarName);
  }]>;

// half_log10
OclBuiltinImpl half_log10_svml = OclBuiltinImpl<half_log10, half_log10.Types, 0, [{
    return __ocl_svml_$Target_log10$SVMLSuffix_native($Arg0VarName);
  }]>;

// half_powr
OclBuiltinImpl half_powr_svml = OclBuiltinImpl<half_powr, half_powr.Types, 0, [{
    return __ocl_svml_$Target_powr$SVMLSuffix_native($Arg0VarName, $Arg1VarName);
  }]>;

// half_recip
OclBuiltinImpl half_recip_svml = OclBuiltinImpl<half_recip, half_recip.Types, 0, [{
    return __ocl_svml_$Target_inv$SVMLSuffix_native($Arg0VarName);
  }]>;

// half_rsqrt
OclBuiltinImpl half_rsqrt_svml = OclBuiltinImpl<half_rsqrt, half_rsqrt.Types, 0, [{
    return __ocl_svml_$Target_rsqrt$SVMLSuffix_native($Arg0VarName);
  }]>;

// half_sin
OclBuiltinImpl half_sin_svml = OclBuiltinImpl<half_sin, half_sin.Types, 0, [{
    return __ocl_svml_$Target_sin$SVMLSuffix_native($Arg0VarName);
  }]>;

// half_sqrt
OclBuiltinImpl half_sqrt_svml = OclBuiltinImpl<half_sqrt, half_sqrt.Types, 0, [{
    return __ocl_svml_$Target_sqrt$SVMLSuffix_native($Arg0VarName);
  }]>;

// half_tan
OclBuiltinImpl half_tan_svml = OclBuiltinImpl<half_tan, half_tan.Types, 0, [{
    return __ocl_svml_$Target_tan$SVMLSuffix_native($Arg0VarName);
  }]>;

// native_cos
OclBuiltinImpl native_cos_svml = OclBuiltinImpl<native_cos, native_cos.Types, 0, [{
    return __ocl_svml_$Target_cos$SVMLSuffix_native($Arg0VarName);
  }]>;

// native_divide
OclBuiltinImpl native_divide_svml = OclBuiltinImpl<native_divide, native_divide.Types, 0, [{
    return __ocl_svml_$Target_div$SVMLSuffix_native($Arg0VarName, $Arg1VarName);
  }]>;

// native_exp
OclBuiltinImpl native_exp_svml = OclBuiltinImpl<native_exp, native_exp.Types, 0, [{
    return __ocl_svml_$Target_exp$SVMLSuffix_native($Arg0VarName);
  }]>;

// native_exp2
OclBuiltinImpl native_exp2_svml = OclBuiltinImpl<native_exp2, native_exp2.Types, 0, [{
    return __ocl_svml_$Target_exp2$SVMLSuffix_native($Arg0VarName);
  }]>;

// native_exp10
OclBuiltinImpl native_exp10_svml = OclBuiltinImpl<native_exp10, native_exp10.Types, 0, [{
    return __ocl_svml_$Target_exp10$SVMLSuffix_native($Arg0VarName);
  }]>;

// native_log
OclBuiltinImpl native_log_svml = OclBuiltinImpl<native_log, native_log.Types, 0, [{
    return __ocl_svml_$Target_log$SVMLSuffix_native($Arg0VarName);
  }]>;

// native_log2
OclBuiltinImpl native_log2_svml = OclBuiltinImpl<native_log2, native_log2.Types, 0, [{
    return __ocl_svml_$Target_log2$SVMLSuffix_native($Arg0VarName);
  }]>;

// native_log10
OclBuiltinImpl native_log10_svml = OclBuiltinImpl<native_log10, native_log10.Types, 0, [{
    return __ocl_svml_$Target_log10$SVMLSuffix_native($Arg0VarName);
  }]>;

// native_powr
OclBuiltinImpl native_powr_svml = OclBuiltinImpl<native_powr, native_powr.Types, 0, [{
    return __ocl_svml_$Target_powr$SVMLSuffix_native($Arg0VarName, $Arg1VarName);
  }]>;

// native_recip
OclBuiltinImpl native_recip_svml = OclBuiltinImpl<native_recip, native_recip.Types, 0, [{
    return __ocl_svml_$Target_inv$SVMLSuffix_native($Arg0VarName);
  }]>;

// native_rsqrt
OclBuiltinImpl native_rsqrt_svml = OclBuiltinImpl<native_rsqrt, native_rsqrt.Types, 0, [{
    return __ocl_svml_$Target_rsqrt$SVMLSuffix_native($Arg0VarName);
  }]>;

// native_sin
OclBuiltinImpl native_sin_svml = OclBuiltinImpl<native_sin, native_sin.Types, 0, [{
    return __ocl_svml_$Target_sin$SVMLSuffix_native($Arg0VarName);
  }]>;

// native_sqrt
OclBuiltinImpl native_sqrt_svml = OclBuiltinImpl<native_sqrt, native_sqrt.Types, 0, [{
    return __ocl_svml_$Target_sqrt$SVMLSuffix_native($Arg0VarName);
  }]>;

// native_tan
OclBuiltinImpl native_tan_svml = OclBuiltinImpl<native_tan, native_tan.Types, 0, [{
    return __ocl_svml_$Target_tan$SVMLSuffix_native($Arg0VarName);
  }]>;

// native_acos
OclBuiltinImpl native_acos_svml = OclBuiltinImpl<native_acos, native_acos.Types, 0, [{
    return __ocl_svml_$Target_acos$SVMLSuffix_native($Arg0VarName);
  }]>;

// native_acosh
OclBuiltinImpl native_acosh_svml = OclBuiltinImpl<native_acosh, native_acosh.Types, 0, [{
    return __ocl_svml_$Target_acosh$SVMLSuffix_native($Arg0VarName);
  }]>;

// native_acospi
OclBuiltinImpl native_acospi_svml = OclBuiltinImpl<native_acospi, native_acospi.Types, 0, [{
    return __ocl_svml_$Target_acospi$SVMLSuffix_native($Arg0VarName);
  }]>;

// native_asin
OclBuiltinImpl native_asin_svml = OclBuiltinImpl<native_asin, native_asin.Types, 0, [{
    return __ocl_svml_$Target_asin$SVMLSuffix_native($Arg0VarName);
  }]>;

// native_asinh
OclBuiltinImpl native_asinh_svml = OclBuiltinImpl<native_asinh, native_asinh.Types, 0, [{
    return __ocl_svml_$Target_asinh$SVMLSuffix_native($Arg0VarName);
  }]>;

// native_asinpi
OclBuiltinImpl native_asinpi_svml = OclBuiltinImpl<native_asinpi, native_asinpi.Types, 0, [{
    return __ocl_svml_$Target_asinpi$SVMLSuffix_native($Arg0VarName);
  }]>;

// native_atan
OclBuiltinImpl native_atan_svml = OclBuiltinImpl<native_atan, native_atan.Types, 0, [{
    return __ocl_svml_$Target_atan$SVMLSuffix_native($Arg0VarName);
  }]>;

// native_atan2
OclBuiltinImpl native_atan2_svml = OclBuiltinImpl<native_atan2, native_atan2.Types, 0, [{
    return __ocl_svml_$Target_atan2$SVMLSuffix_native($Arg0VarName, $Arg1VarName);
  }]>;

// native_atanh
OclBuiltinImpl native_atanh_svml = OclBuiltinImpl<native_atanh, native_atanh.Types, 0, [{
    return __ocl_svml_$Target_atanh$SVMLSuffix_native($Arg0VarName);
  }]>;

// native_atanpi
OclBuiltinImpl native_atanpi_svml = OclBuiltinImpl<native_atanpi, native_atanpi.Types, 0, [{
    return __ocl_svml_$Target_atanpi$SVMLSuffix_native($Arg0VarName);
  }]>;

// native_atan2pi
OclBuiltinImpl native_atan2pi_svml = OclBuiltinImpl<native_atan2pi, native_atan2pi.Types, 0, [{
    return __ocl_svml_$Target_atan2pi$SVMLSuffix_native($Arg0VarName, $Arg0VarName);
  }]>;

// native_cbrt
OclBuiltinImpl native_cbrt_svml = OclBuiltinImpl<native_cbrt, native_cbrt.Types, 0, [{
    return __ocl_svml_$Target_cbrt$SVMLSuffix_native($Arg0VarName);
  }]>;

// native_cosh
OclBuiltinImpl native_cosh_svml = OclBuiltinImpl<native_cosh, native_cosh.Types, 0, [{
    return __ocl_svml_$Target_cosh$SVMLSuffix_native($Arg0VarName);
  }]>;

// native_cospi
OclBuiltinImpl native_cospi_svml = OclBuiltinImpl<native_cospi, native_cospi.Types, 0, [{
    return __ocl_svml_$Target_cospi$SVMLSuffix_native($Arg0VarName);
  }]>;

// native_erfc
OclBuiltinImpl native_erfc_svml = OclBuiltinImpl<native_erfc, native_erfc.Types, 0, [{
    return __ocl_svml_$Target_erfc$SVMLSuffix_native($Arg0VarName);
  }]>;

// native_erf
OclBuiltinImpl native_erf_svml = OclBuiltinImpl<native_erf, native_erf.Types, 0, [{
    return __ocl_svml_$Target_erf$SVMLSuffix_native($Arg0VarName);
  }]>;

// native_expm1
OclBuiltinImpl native_expm1_svml = OclBuiltinImpl<native_expm1, native_expm1.Types, 0, [{
    return __ocl_svml_$Target_expm1$SVMLSuffix_native($Arg0VarName);
  }]>;

// native_hypot
OclBuiltinImpl native_hypot_svml = OclBuiltinImpl<native_hypot, native_hypot.Types, 0, [{
    return __ocl_svml_$Target_hypot$SVMLSuffix_native($Arg0VarName, $Arg1VarName);
  }]>;

// native_log1p
OclBuiltinImpl native_log1p_svml = OclBuiltinImpl<native_log1p, native_log1p.Types, 0, [{
    return __ocl_svml_$Target_log1p$SVMLSuffix_native($Arg0VarName);
  }]>;

// native_pow
OclBuiltinImpl native_pow_svml = OclBuiltinImpl<native_pow, native_pow.Types, 0, [{
    return __ocl_svml_$Target_pow$SVMLSuffix_native($Arg0VarName, $Arg1VarName);
  }]>;

// native_pown
OclBuiltinImpl native_pown_svml = OclBuiltinImpl<native_pown, native_pown.Types, 0, [{
    return __ocl_svml_$Target_pown$SVMLSuffix_native($Arg0VarName, $Arg1VarName);
  }]>;

// native_rcbrt
OclBuiltinImpl native_rcbrt_svml = OclBuiltinImpl<native_rcbrt, native_rcbrt.Types, 0, [{
    return __ocl_svml_$Target_rcbrt$SVMLSuffix_native($Arg0VarName);
  }]>;

// native_rootn
OclBuiltinImpl native_rootn_svml = OclBuiltinImpl<native_rootn, native_rootn.Types, 0, [{
    return __ocl_svml_$Target_rootn$SVMLSuffix_native($Arg0VarName, $Arg1VarName);
  }]>;

// native_sincos
code native_sincos_svml = [{
    return __ocl_svml_$Target_sincos$SVMLSuffix_native($Arg0VarName, ($Arg1NoASType)$Arg1VarName);
  }];
OclBuiltinImpl native_sincos_pas_svml = OclBuiltinImpl<native_sincos_pas, native_sincos_pas.Types, 0, native_sincos_svml>;
OclBuiltinImpl native_sincos_las_svml = OclBuiltinImpl<native_sincos_las, native_sincos_las.Types, 0, native_sincos_svml>;
OclBuiltinImpl native_sincos_gas_svml = OclBuiltinImpl<native_sincos_gas, native_sincos_gas.Types, 0, native_sincos_svml>;

// native_sinh
OclBuiltinImpl native_sinh_svml = OclBuiltinImpl<native_sinh, native_sinh.Types, 0, [{
    return __ocl_svml_$Target_sinh$SVMLSuffix_native($Arg0VarName);
  }]>;

// native_sinpi
OclBuiltinImpl native_sinpi_svml = OclBuiltinImpl<native_sinpi, native_sinpi.Types, 0, [{
    return __ocl_svml_$Target_sinpi$SVMLSuffix_native($Arg0VarName);
  }]>;

// native_tanh
OclBuiltinImpl native_tanh_svml = OclBuiltinImpl<native_tanh, native_tanh.Types, 0, [{
    return __ocl_svml_$Target_tanh$SVMLSuffix_native($Arg0VarName);
  }]>;

// native_tanpi
OclBuiltinImpl native_tanpi_svml = OclBuiltinImpl<native_tanpi, native_tanpi.Types, 0, [{
    return __ocl_svml_$Target_tanpi$SVMLSuffix_native($Arg0VarName);
  }]>;
