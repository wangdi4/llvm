// vim:ts=2:sw=2:et:

let Types = [v1f32, v2f32, v3f32, v4f32, v8f32, v16f32, v1f64, v2f64, v3f64, v4f64, v8f64, v16f64],
    Attrs = [SVMLCC, CONSTFUNC],
    IsDeclOnly = 1, NeedForwardDecl = 1 in {
def svml_acos       : OclBuiltin<"_ocl_svml_$Target_acos$SVMLSuffix",       (outs gentype:$ret), (ins gentype:$x), [], []>;
def svml_acosh      : OclBuiltin<"_ocl_svml_$Target_acosh$SVMLSuffix",      (outs gentype:$ret), (ins gentype:$x), [], []>;
def svml_acospi     : OclBuiltin<"_ocl_svml_$Target_acospi$SVMLSuffix",     (outs gentype:$ret), (ins gentype:$x), [], []>;
def svml_asin       : OclBuiltin<"_ocl_svml_$Target_asin$SVMLSuffix",       (outs gentype:$ret), (ins gentype:$x), [], []>;
def svml_asinh      : OclBuiltin<"_ocl_svml_$Target_asinh$SVMLSuffix",      (outs gentype:$ret), (ins gentype:$x), [], []>;
def svml_asinpi     : OclBuiltin<"_ocl_svml_$Target_asinpi$SVMLSuffix",     (outs gentype:$ret), (ins gentype:$x), [], []>;
def svml_atan       : OclBuiltin<"_ocl_svml_$Target_atan$SVMLSuffix",       (outs gentype:$ret), (ins gentype:$x), [], []>;
def svml_atan2      : OclBuiltin<"_ocl_svml_$Target_atan2$SVMLSuffix",      (outs gentype:$ret), (ins gentype:$x, gentype:$y), [], []>;
def svml_atanh      : OclBuiltin<"_ocl_svml_$Target_atanh$SVMLSuffix",      (outs gentype:$ret), (ins gentype:$x), [], []>;
def svml_atanpi     : OclBuiltin<"_ocl_svml_$Target_atanpi$SVMLSuffix",     (outs gentype:$ret), (ins gentype:$x), [], []>;
def svml_atan2pi    : OclBuiltin<"_ocl_svml_$Target_atan2pi$SVMLSuffix",    (outs gentype:$ret), (ins gentype:$x, gentype:$y), [], []>;
def svml_cbrt       : OclBuiltin<"_ocl_svml_$Target_cbrt$SVMLSuffix",       (outs gentype:$ret), (ins gentype:$x), [], []>;
def svml_ceil       : OclBuiltin<"_ocl_svml_$Target_ceil$SVMLSuffix",       (outs gentype:$ret), (ins gentype:$x), [], []>;
def svml_copysign   : OclBuiltin<"_ocl_svml_$Target_copysign$SVMLSuffix",   (outs gentype:$ret), (ins gentype:$x, gentype:$y), [], []>;
def svml_cos        : OclBuiltin<"_ocl_svml_$Target_cos$SVMLSuffix",        (outs gentype:$ret), (ins gentype:$x), [], []>;
def svml_cosh       : OclBuiltin<"_ocl_svml_$Target_cosh$SVMLSuffix",       (outs gentype:$ret), (ins gentype:$x), [], []>;
def svml_cospi      : OclBuiltin<"_ocl_svml_$Target_cospi$SVMLSuffix",      (outs gentype:$ret), (ins gentype:$x), [], []>;
def svml_erfc       : OclBuiltin<"_ocl_svml_$Target_erfc$SVMLSuffix",       (outs gentype:$ret), (ins gentype:$x), [], []>;
def svml_erf        : OclBuiltin<"_ocl_svml_$Target_erf$SVMLSuffix",        (outs gentype:$ret), (ins gentype:$x), [], []>;
def svml_exp        : OclBuiltin<"_ocl_svml_$Target_exp$SVMLSuffix",        (outs gentype:$ret), (ins gentype:$x), [], []>;
def svml_exp2       : OclBuiltin<"_ocl_svml_$Target_exp2$SVMLSuffix",       (outs gentype:$ret), (ins gentype:$x), [], []>;
def svml_exp10      : OclBuiltin<"_ocl_svml_$Target_exp10$SVMLSuffix",      (outs gentype:$ret), (ins gentype:$x), [], []>;
def svml_expm1      : OclBuiltin<"_ocl_svml_$Target_expm1$SVMLSuffix",      (outs gentype:$ret), (ins gentype:$x), [], []>;
def svml_fabs       : OclBuiltin<"_ocl_svml_$Target_fabs$SVMLSuffix",       (outs gentype:$ret), (ins gentype:$x), [], []>;
def svml_fdim       : OclBuiltin<"_ocl_svml_$Target_fdim$SVMLSuffix",       (outs gentype:$ret), (ins gentype:$x), [], []>;
def svml_floor      : OclBuiltin<"_ocl_svml_$Target_floor$SVMLSuffix",      (outs gentype:$ret), (ins gentype:$x), [], []>;
def svml_fma        : OclBuiltin<"_ocl_svml_$Target_fma$SVMLSuffix",        (outs gentype:$ret), (ins gentype:$a, gentype:$b, gentype:$c), [], []>;
def svml_fmax       : OclBuiltin<"_ocl_svml_$Target_fmax$SVMLSuffix",       (outs gentype:$ret), (ins gentype:$x, gentype:$y), [], []>;
def svml_fmin       : OclBuiltin<"_ocl_svml_$Target_fmin$SVMLSuffix",       (outs gentype:$ret), (ins gentype:$x, gentype:$y), [], []>;
def svml_fmod       : OclBuiltin<"_ocl_svml_$Target_fmod$SVMLSuffix",       (outs gentype:$ret), (ins gentype:$x, gentype:$y), [], []>;
def svml_fract      : OclBuiltin<"_ocl_svml_$Target_fract$SVMLSuffix",      (outs gentype:$ret), (ins gentype:$x, pgentype:$iptr), [], []>, ExceptAttr<[CONSTFUNC]>;
def svml_frexp      : OclBuiltin<"_ocl_svml_$Target_frexp$SVMLSuffix",      (outs gentype:$ret), (ins gentype:$x, pi32gentype:$iptr), [], []>, ExceptAttr<[CONSTFUNC]>;
def svml_hypot      : OclBuiltin<"_ocl_svml_$Target_hypot$SVMLSuffix",      (outs gentype:$ret), (ins gentype:$x, gentype:$y), [], []>;
def svml_ilogb      : OclBuiltin<"_ocl_svml_$Target_ilogb$SVMLSuffix",      (outs i32gentype:$ret), (ins gentype:$x), [], []>;
def svml_ldexp      : OclBuiltin<"_ocl_svml_$Target_ldexp$SVMLSuffix",      (outs gentype:$ret), (ins gentype:$x, i32gentype:$k), [], []>;
def svml_lgamma     : OclBuiltin<"_ocl_svml_$Target_lgamma$SVMLSuffix",     (outs gentype:$ret), (ins gentype:$x), [], []>;
def svml_lgamma_r   : OclBuiltin<"_ocl_svml_$Target_lgamma_r$SVMLSuffix",   (outs gentype:$ret), (ins gentype:$x, pi32gentype:$iptr), [], []>, ExceptAttr<[CONSTFUNC]>;
def svml_log        : OclBuiltin<"_ocl_svml_$Target_log$SVMLSuffix",        (outs gentype:$ret), (ins gentype:$x), [], []>;
def svml_log2       : OclBuiltin<"_ocl_svml_$Target_log2$SVMLSuffix",       (outs gentype:$ret), (ins gentype:$x), [], []>;
def svml_log10      : OclBuiltin<"_ocl_svml_$Target_log10$SVMLSuffix",      (outs gentype:$ret), (ins gentype:$x), [], []>;
def svml_log1p      : OclBuiltin<"_ocl_svml_$Target_log1p$SVMLSuffix",      (outs gentype:$ret), (ins gentype:$x), [], []>;
def svml_logb       : OclBuiltin<"_ocl_svml_$Target_logb$SVMLSuffix",       (outs gentype:$ret), (ins gentype:$x), [], []>;
def svml_mad        : OclBuiltin<"_ocl_svml_$Target_mad$SVMLSuffix",        (outs gentype:$ret), (ins gentype:$a, gentype:$b, gentype:$c), [], []>;
def svml_maxmag     : OclBuiltin<"_ocl_svml_$Target_maxmag$SVMLSuffix",     (outs gentype:$ret), (ins gentype:$x, gentype:$y), [], []>;
def svml_minmag     : OclBuiltin<"_ocl_svml_$Target_minmag$SVMLSuffix",     (outs gentype:$ret), (ins gentype:$x, gentype:$y), [], []>;
def svml_modf       : OclBuiltin<"_ocl_svml_$Target_modf$SVMLSuffix",       (outs gentype:$ret), (ins gentype:$x, pgentype:$y), [], []>, ExceptAttr<[CONSTFUNC]>;
def svml_nan        : OclBuiltin<"_ocl_svml_$Target_nan$SVMLSuffix",        (outs gentype:$ret), (ins ugentype:$x), [], []>;
def svml_nextafter  : OclBuiltin<"_ocl_svml_$Target_nextafter$SVMLSuffix",  (outs gentype:$ret), (ins gentype:$x, gentype:$y), [], []>;
def svml_pow        : OclBuiltin<"_ocl_svml_$Target_pow$SVMLSuffix",        (outs gentype:$ret), (ins gentype:$x, gentype:$y), [], []>;
def svml_pown       : OclBuiltin<"_ocl_svml_$Target_pown$SVMLSuffix",       (outs gentype:$ret), (ins gentype:$x, i32gentype:$y), [], []>;
def svml_powr       : OclBuiltin<"_ocl_svml_$Target_powr$SVMLSuffix",       (outs gentype:$ret), (ins gentype:$x, gentype:$y), [], []>;
def svml_remainder  : OclBuiltin<"_ocl_svml_$Target_remainder$SVMLSuffix",  (outs gentype:$ret), (ins gentype:$x, gentype:$y), [], []>;
def svml_remquo     : OclBuiltin<"_ocl_svml_$Target_remquo$SVMLSuffix",     (outs gentype:$ret), (ins gentype:$x, gentype:$y, pi32gentype:$quo), [], []>, ExceptAttr<[CONSTFUNC]>;
def svml_rint       : OclBuiltin<"_ocl_svml_$Target_rint$SVMLSuffix",       (outs gentype:$ret), (ins gentype:$x), [], []>;
def svml_rootn      : OclBuiltin<"_ocl_svml_$Target_rootn$SVMLSuffix",      (outs gentype:$ret), (ins gentype:$x, i32gentype:$y), [], []>;
def svml_round      : OclBuiltin<"_ocl_svml_$Target_round$SVMLSuffix",      (outs gentype:$ret), (ins gentype:$x), [], []>;
def svml_rsqrt      : OclBuiltin<"_ocl_svml_$Target_rsqrt$SVMLSuffix",      (outs gentype:$ret), (ins gentype:$x), [], []>;
def svml_sin        : OclBuiltin<"_ocl_svml_$Target_sin$SVMLSuffix",        (outs gentype:$ret), (ins gentype:$x), [], []>;
def svml_sincos     : OclBuiltin<"_ocl_svml_$Target_sincos$SVMLSuffix",     (outs gentype:$ret), (ins gentype:$x, pgentype:$cosval), [], []>, ExceptAttr<[CONSTFUNC]>;
def svml_sinh       : OclBuiltin<"_ocl_svml_$Target_sinh$SVMLSuffix",       (outs gentype:$ret), (ins gentype:$x), [], []>;
def svml_sinpi      : OclBuiltin<"_ocl_svml_$Target_sinpi$SVMLSuffix",      (outs gentype:$ret), (ins gentype:$x), [], []>;
def svml_sqrt       : OclBuiltin<"_ocl_svml_$Target_sqrt$SVMLSuffix",       (outs gentype:$ret), (ins gentype:$x), [], []>;
def svml_tan        : OclBuiltin<"_ocl_svml_$Target_tan$SVMLSuffix",        (outs gentype:$ret), (ins gentype:$x), [], []>;
def svml_tanh       : OclBuiltin<"_ocl_svml_$Target_tanh$SVMLSuffix",       (outs gentype:$ret), (ins gentype:$x), [], []>;
def svml_tanpi      : OclBuiltin<"_ocl_svml_$Target_tanpi$SVMLSuffix",      (outs gentype:$ret), (ins gentype:$x), [], []>;
def svml_tgamma     : OclBuiltin<"_ocl_svml_$Target_tgamma$SVMLSuffix",     (outs gentype:$ret), (ins gentype:$x), [], []>;
def svml_trunc      : OclBuiltin<"_ocl_svml_$Target_trunc$SVMLSuffix",      (outs gentype:$ret), (ins gentype:$x), [], []>;
}

let Types = [v1f32, v2f32, v3f32, v4f32, v8f32, v16f32],
    Attrs = [SVMLCC, CONSTFUNC],
    IsDeclOnly = 1, NeedForwardDecl = 1 in {
def svml_cos_native     : OclBuiltin<"_ocl_svml_$Target_cos$SVMLSuffix_native",     (outs gentype:$ret), (ins gentype:$x), [], []>;
def svml_divide_native  : OclBuiltin<"_ocl_svml_$Target_divide$SVMLSuffix_native",  (outs gentype:$ret), (ins gentype:$x, gentype:$y), [], []>;
def svml_exp_native     : OclBuiltin<"_ocl_svml_$Target_exp$SVMLSuffix_native",     (outs gentype:$ret), (ins gentype:$x), [], []>;
def svml_exp2_native    : OclBuiltin<"_ocl_svml_$Target_exp2$SVMLSuffix_native",    (outs gentype:$ret), (ins gentype:$x), [], []>;
def svml_exp10_native   : OclBuiltin<"_ocl_svml_$Target_exp10$SVMLSuffix_native",   (outs gentype:$ret), (ins gentype:$x), [], []>;
def svml_log_native     : OclBuiltin<"_ocl_svml_$Target_log$SVMLSuffix_native",     (outs gentype:$ret), (ins gentype:$x), [], []>;
def svml_log2_native    : OclBuiltin<"_ocl_svml_$Target_log2$SVMLSuffix_native",    (outs gentype:$ret), (ins gentype:$x), [], []>;
def svml_log10_native   : OclBuiltin<"_ocl_svml_$Target_log10$SVMLSuffix_native",   (outs gentype:$ret), (ins gentype:$x), [], []>;
def svml_powr_native    : OclBuiltin<"_ocl_svml_$Target_powr$SVMLSuffix_native",    (outs gentype:$ret), (ins gentype:$x, gentype:$y), [], []>;
def svml_recip_native   : OclBuiltin<"_ocl_svml_$Target_recip$SVMLSuffix_native",   (outs gentype:$ret), (ins gentype:$x), [], []>;
def svml_rsqrt_native   : OclBuiltin<"_ocl_svml_$Target_rsqrt$SVMLSuffix_native",   (outs gentype:$ret), (ins gentype:$x), [], []>;
def svml_sin_native     : OclBuiltin<"_ocl_svml_$Target_sin$SVMLSuffix_native",     (outs gentype:$ret), (ins gentype:$x), [], []>;
def svml_sqrt_native    : OclBuiltin<"_ocl_svml_$Target_sqrt$SVMLSuffix_native",    (outs gentype:$ret), (ins gentype:$x), [], []>;
def svml_tan_native     : OclBuiltin<"_ocl_svml_$Target_tan$SVMLSuffix_native",     (outs gentype:$ret), (ins gentype:$x), [], []>;
}
