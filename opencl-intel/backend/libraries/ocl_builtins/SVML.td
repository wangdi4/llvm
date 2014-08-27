// vim:ts=2:sw=2:et:

let Types = [v1f32, v2f32, v3f32, v4f32, v8f32, v16f32,
             v1f64, v2f64, v3f64, v4f64, v8f64, v16f64],
    Attrs = [SVMLCC, CONSTFUNC],
    IsDeclOnly = 1, NeedForwardDecl = 1 in {
def svml_acos       : OclBuiltin<"__ocl_svml_$Target_acos$SVMLSuffix",       (outs gentype:$ret), (ins gentype:$x), [], []>;
def svml_acosh      : OclBuiltin<"__ocl_svml_$Target_acosh$SVMLSuffix",      (outs gentype:$ret), (ins gentype:$x), [], []>;
def svml_acospi     : OclBuiltin<"__ocl_svml_$Target_acospi$SVMLSuffix",     (outs gentype:$ret), (ins gentype:$x), [], []>;
def svml_asin       : OclBuiltin<"__ocl_svml_$Target_asin$SVMLSuffix",       (outs gentype:$ret), (ins gentype:$x), [], []>;
def svml_asinh      : OclBuiltin<"__ocl_svml_$Target_asinh$SVMLSuffix",      (outs gentype:$ret), (ins gentype:$x), [], []>;
def svml_asinpi     : OclBuiltin<"__ocl_svml_$Target_asinpi$SVMLSuffix",     (outs gentype:$ret), (ins gentype:$x), [], []>;
def svml_atan       : OclBuiltin<"__ocl_svml_$Target_atan$SVMLSuffix",       (outs gentype:$ret), (ins gentype:$x), [], []>;
def svml_atan2      : OclBuiltin<"__ocl_svml_$Target_atan2$SVMLSuffix",      (outs gentype:$ret), (ins gentype:$x, gentype:$y), [], []>;
def svml_atanh      : OclBuiltin<"__ocl_svml_$Target_atanh$SVMLSuffix",      (outs gentype:$ret), (ins gentype:$x), [], []>;
def svml_atanpi     : OclBuiltin<"__ocl_svml_$Target_atanpi$SVMLSuffix",     (outs gentype:$ret), (ins gentype:$x), [], []>;
def svml_atan2pi    : OclBuiltin<"__ocl_svml_$Target_atan2pi$SVMLSuffix",    (outs gentype:$ret), (ins gentype:$x, gentype:$y), [], []>;
def svml_cbrt       : OclBuiltin<"__ocl_svml_$Target_cbrt$SVMLSuffix",       (outs gentype:$ret), (ins gentype:$x), [], []>;
def svml_ceil       : OclBuiltin<"__ocl_svml_$Target_ceil$SVMLSuffix",       (outs gentype:$ret), (ins gentype:$x), [], []>;
def svml_copysign   : OclBuiltin<"__ocl_svml_$Target_copysign$SVMLSuffix",   (outs gentype:$ret), (ins gentype:$x, gentype:$y), [], []>;
def svml_cos        : OclBuiltin<"__ocl_svml_$Target_cos$SVMLSuffix",        (outs gentype:$ret), (ins gentype:$x), [], []>;
def svml_cosh       : OclBuiltin<"__ocl_svml_$Target_cosh$SVMLSuffix",       (outs gentype:$ret), (ins gentype:$x), [], []>;
def svml_cospi      : OclBuiltin<"__ocl_svml_$Target_cospi$SVMLSuffix",      (outs gentype:$ret), (ins gentype:$x), [], []>;
def svml_div        : OclBuiltin<"__ocl_svml_$Target_div$SVMLSuffix",        (outs gentype:$ret), (ins gentype:$x, gentype:$y), [], []>;
def svml_erfc       : OclBuiltin<"__ocl_svml_$Target_erfc$SVMLSuffix",       (outs gentype:$ret), (ins gentype:$x), [], []>;
def svml_erf        : OclBuiltin<"__ocl_svml_$Target_erf$SVMLSuffix",        (outs gentype:$ret), (ins gentype:$x), [], []>;
def svml_exp        : OclBuiltin<"__ocl_svml_$Target_exp$SVMLSuffix",        (outs gentype:$ret), (ins gentype:$x), [], []>;
def svml_exp2       : OclBuiltin<"__ocl_svml_$Target_exp2$SVMLSuffix",       (outs gentype:$ret), (ins gentype:$x), [], []>;
def svml_exp10      : OclBuiltin<"__ocl_svml_$Target_exp10$SVMLSuffix",      (outs gentype:$ret), (ins gentype:$x), [], []>;
def svml_expm1      : OclBuiltin<"__ocl_svml_$Target_expm1$SVMLSuffix",      (outs gentype:$ret), (ins gentype:$x), [], []>;
def svml_fabs       : OclBuiltin<"__ocl_svml_$Target_fabs$SVMLSuffix",       (outs gentype:$ret), (ins gentype:$x), [], []>;
def svml_fdim       : OclBuiltin<"__ocl_svml_$Target_fdim$SVMLSuffix",       (outs gentype:$ret), (ins gentype:$x, gentype:$y), [], []>;
def svml_floor      : OclBuiltin<"__ocl_svml_$Target_floor$SVMLSuffix",      (outs gentype:$ret), (ins gentype:$x), [], []>;
def svml_fma        : OclBuiltin<"__ocl_svml_$Target_fma$SVMLSuffix",        (outs gentype:$ret), (ins gentype:$a, gentype:$b, gentype:$c), [], []>;
def svml_fmax       : OclBuiltin<"__ocl_svml_$Target_fmax$SVMLSuffix",       (outs gentype:$ret), (ins gentype:$x, gentype:$y), [], []>;
def svml_fmin       : OclBuiltin<"__ocl_svml_$Target_fmin$SVMLSuffix",       (outs gentype:$ret), (ins gentype:$x, gentype:$y), [], []>;
def svml_fmod       : OclBuiltin<"__ocl_svml_$Target_fmod$SVMLSuffix",       (outs gentype:$ret), (ins gentype:$x, gentype:$y), [], []>;
def svml_fract      : OclBuiltin<"__ocl_svml_$Target_fract$SVMLSuffix",      (outs gentype:$ret), (ins gentype:$x, pgentype:$iptr), [], []>, ExceptAttr<[CONSTFUNC]>;
def svml_frexp      : OclBuiltin<"__ocl_svml_$Target_frexp$SVMLSuffix",      (outs gentype:$ret), (ins gentype:$x, pi32gentype:$iptr), [], []>, ExceptAttr<[CONSTFUNC]>;
def svml_hypot      : OclBuiltin<"__ocl_svml_$Target_hypot$SVMLSuffix",      (outs gentype:$ret), (ins gentype:$x, gentype:$y), [], []>;
def svml_ilogb      : OclBuiltin<"__ocl_svml_$Target_ilogb$SVMLSuffix",      (outs i32gentype:$ret), (ins gentype:$x), [], []>;
def svml_ldexp      : OclBuiltin<"__ocl_svml_$Target_ldexp$SVMLSuffix",      (outs gentype:$ret), (ins gentype:$x, i32gentype:$k), [], []>;
def svml_lgamma     : OclBuiltin<"__ocl_svml_$Target_lgamma$SVMLSuffix",     (outs gentype:$ret), (ins gentype:$x), [], []>;
def svml_lgamma_r   : OclBuiltin<"__ocl_svml_$Target_lgammar$SVMLSuffix",    (outs gentype:$ret), (ins gentype:$x, pi32gentype:$iptr), [], []>, ExceptAttr<[CONSTFUNC]>;
def svml_log        : OclBuiltin<"__ocl_svml_$Target_log$SVMLSuffix",        (outs gentype:$ret), (ins gentype:$x), [], []>;
def svml_log2       : OclBuiltin<"__ocl_svml_$Target_log2$SVMLSuffix",       (outs gentype:$ret), (ins gentype:$x), [], []>;
def svml_log10      : OclBuiltin<"__ocl_svml_$Target_log10$SVMLSuffix",      (outs gentype:$ret), (ins gentype:$x), [], []>;
def svml_log1p      : OclBuiltin<"__ocl_svml_$Target_log1p$SVMLSuffix",      (outs gentype:$ret), (ins gentype:$x), [], []>;
def svml_logb       : OclBuiltin<"__ocl_svml_$Target_logb$SVMLSuffix",       (outs gentype:$ret), (ins gentype:$x), [], []>;
def svml_mad        : OclBuiltin<"__ocl_svml_$Target_mad$SVMLSuffix",        (outs gentype:$ret), (ins gentype:$a, gentype:$b, gentype:$c), [], []>;
def svml_maxmag     : OclBuiltin<"__ocl_svml_$Target_maxmag$SVMLSuffix",     (outs gentype:$ret), (ins gentype:$x, gentype:$y), [], []>;
def svml_minmag     : OclBuiltin<"__ocl_svml_$Target_minmag$SVMLSuffix",     (outs gentype:$ret), (ins gentype:$x, gentype:$y), [], []>;
def svml_modf       : OclBuiltin<"__ocl_svml_$Target_modf$SVMLSuffix",       (outs gentype:$ret), (ins gentype:$x, pgentype:$y), [], []>, ExceptAttr<[CONSTFUNC]>;
def svml_nan        : OclBuiltin<"__ocl_svml_$Target_nan$SVMLSuffix",        (outs gentype:$ret), (ins ugentype:$x), [], []>;
def svml_nextafter  : OclBuiltin<"__ocl_svml_$Target_nextafter$SVMLSuffix",  (outs gentype:$ret), (ins gentype:$x, gentype:$y), [], []>;
def svml_pow        : OclBuiltin<"__ocl_svml_$Target_pow$SVMLSuffix",        (outs gentype:$ret), (ins gentype:$x, gentype:$y), [], []>;
def svml_pown       : OclBuiltin<"__ocl_svml_$Target_pown$SVMLSuffix",       (outs gentype:$ret), (ins gentype:$x, i32gentype:$y), [], []>;
def svml_powr       : OclBuiltin<"__ocl_svml_$Target_powr$SVMLSuffix",       (outs gentype:$ret), (ins gentype:$x, gentype:$y), [], []>;
def svml_remainder  : OclBuiltin<"__ocl_svml_$Target_remainder$SVMLSuffix",  (outs gentype:$ret), (ins gentype:$x, gentype:$y), [], []>;
def svml_remquo     : OclBuiltin<"__ocl_svml_$Target_remquo$SVMLSuffix",     (outs gentype:$ret), (ins gentype:$x, gentype:$y, pi32gentype:$quo), [], []>, ExceptAttr<[CONSTFUNC]>;
def svml_rint       : OclBuiltin<"__ocl_svml_$Target_rint$SVMLSuffix",       (outs gentype:$ret), (ins gentype:$x), [], []>;
def svml_rootn      : OclBuiltin<"__ocl_svml_$Target_rootn$SVMLSuffix",      (outs gentype:$ret), (ins gentype:$x, i32gentype:$y), [], []>;
def svml_round      : OclBuiltin<"__ocl_svml_$Target_round$SVMLSuffix",      (outs gentype:$ret), (ins gentype:$x), [], []>;
def svml_rsqrt      : OclBuiltin<"__ocl_svml_$Target_rsqrt$SVMLSuffix",      (outs gentype:$ret), (ins gentype:$x), [], []>;
def svml_sin        : OclBuiltin<"__ocl_svml_$Target_sin$SVMLSuffix",        (outs gentype:$ret), (ins gentype:$x), [], []>;
def svml_sincos     : OclBuiltin<"__ocl_svml_$Target_sincos$SVMLSuffix",     (outs gentype:$ret), (ins gentype:$x, pgentype:$cosval), [], []>, ExceptAttr<[CONSTFUNC]>;
def svml_sinh       : OclBuiltin<"__ocl_svml_$Target_sinh$SVMLSuffix",       (outs gentype:$ret), (ins gentype:$x), [], []>;
def svml_sinpi      : OclBuiltin<"__ocl_svml_$Target_sinpi$SVMLSuffix",      (outs gentype:$ret), (ins gentype:$x), [], []>;
def svml_sqrt       : OclBuiltin<"__ocl_svml_$Target_sqrt$SVMLSuffix",       (outs gentype:$ret), (ins gentype:$x), [], []>;
def svml_tan        : OclBuiltin<"__ocl_svml_$Target_tan$SVMLSuffix",        (outs gentype:$ret), (ins gentype:$x), [], []>;
def svml_tanh       : OclBuiltin<"__ocl_svml_$Target_tanh$SVMLSuffix",       (outs gentype:$ret), (ins gentype:$x), [], []>;
def svml_tanpi      : OclBuiltin<"__ocl_svml_$Target_tanpi$SVMLSuffix",      (outs gentype:$ret), (ins gentype:$x), [], []>;
def svml_tgamma     : OclBuiltin<"__ocl_svml_$Target_tgamma$SVMLSuffix",     (outs gentype:$ret), (ins gentype:$x), [], []>;
def svml_trunc      : OclBuiltin<"__ocl_svml_$Target_trunc$SVMLSuffix",      (outs gentype:$ret), (ins gentype:$x), [], []>;
}

let Types = [v1f32, v2f32, v3f32, v4f32, v8f32, v16f32,
             v1f64, v2f64, v3f64, v4f64, v8f64, v16f64],
    Attrs = [SVMLCC, CONSTFUNC],
    IsDeclOnly = 1, NeedForwardDecl = 1 in {
def svml_acos_native    : OclBuiltin<"__ocl_svml_$Target_acos$SVMLSuffix_native",     (outs gentype:$ret), (ins gentype:$x), [], []>;
def svml_acosh_native   : OclBuiltin<"__ocl_svml_$Target_acosh$SVMLSuffix_native",    (outs gentype:$ret), (ins gentype:$x), [], []>;
def svml_acospi_native  : OclBuiltin<"__ocl_svml_$Target_acospi$SVMLSuffix_native",   (outs gentype:$ret), (ins gentype:$x), [], []>;
def svml_asin_native    : OclBuiltin<"__ocl_svml_$Target_asin$SVMLSuffix_native",     (outs gentype:$ret), (ins gentype:$x), [], []>;
def svml_asinh_native   : OclBuiltin<"__ocl_svml_$Target_asinh$SVMLSuffix_native",    (outs gentype:$ret), (ins gentype:$x), [], []>;
def svml_asinpi_native  : OclBuiltin<"__ocl_svml_$Target_asinpi$SVMLSuffix_native",   (outs gentype:$ret), (ins gentype:$x), [], []>;
def svml_atan_native    : OclBuiltin<"__ocl_svml_$Target_atan$SVMLSuffix_native",     (outs gentype:$ret), (ins gentype:$x), [], []>;
def svml_atan2_native   : OclBuiltin<"__ocl_svml_$Target_atan2$SVMLSuffix_native",    (outs gentype:$ret), (ins gentype:$x, gentype:$y), [], []>;
def svml_atanh_native   : OclBuiltin<"__ocl_svml_$Target_atanh$SVMLSuffix_native",    (outs gentype:$ret), (ins gentype:$x), [], []>;
def svml_atanpi_native  : OclBuiltin<"__ocl_svml_$Target_atanpi$SVMLSuffix_native",   (outs gentype:$ret), (ins gentype:$x), [], []>;
def svml_atan2pi_native : OclBuiltin<"__ocl_svml_$Target_atan2pi$SVMLSuffix_native",  (outs gentype:$ret), (ins gentype:$x, gentype:$y), [], []>;
def svml_cbrt_native    : OclBuiltin<"__ocl_svml_$Target_cbrt$SVMLSuffix_native",     (outs gentype:$ret), (ins gentype:$x), [], []>;
def svml_cos_native     : OclBuiltin<"__ocl_svml_$Target_cos$SVMLSuffix_native",      (outs gentype:$ret), (ins gentype:$x), [], []>;
def svml_cosh_native    : OclBuiltin<"__ocl_svml_$Target_cosh$SVMLSuffix_native",     (outs gentype:$ret), (ins gentype:$x), [], []>;
def svml_cospi_native   : OclBuiltin<"__ocl_svml_$Target_cospi$SVMLSuffix_native",    (outs gentype:$ret), (ins gentype:$x), [], []>;
def svml_div_native     : OclBuiltin<"__ocl_svml_$Target_div$SVMLSuffix_native",      (outs gentype:$ret), (ins gentype:$x, gentype:$y), [], []>;
def svml_erfc_native    : OclBuiltin<"__ocl_svml_$Target_erfc$SVMLSuffix_native",     (outs gentype:$ret), (ins gentype:$x), [], []>;
def svml_erf_native     : OclBuiltin<"__ocl_svml_$Target_erf$SVMLSuffix_native",      (outs gentype:$ret), (ins gentype:$x), [], []>;
def svml_exp_native     : OclBuiltin<"__ocl_svml_$Target_exp$SVMLSuffix_native",      (outs gentype:$ret), (ins gentype:$x), [], []>;
def svml_exp2_native    : OclBuiltin<"__ocl_svml_$Target_exp2$SVMLSuffix_native",     (outs gentype:$ret), (ins gentype:$x), [], []>;
def svml_exp10_native   : OclBuiltin<"__ocl_svml_$Target_exp10$SVMLSuffix_native",    (outs gentype:$ret), (ins gentype:$x), [], []>;
def svml_expm1_native   : OclBuiltin<"__ocl_svml_$Target_expm1$SVMLSuffix_native",    (outs gentype:$ret), (ins gentype:$x), [], []>;
def svml_hypot_native   : OclBuiltin<"__ocl_svml_$Target_hypot$SVMLSuffix_native",    (outs gentype:$ret), (ins gentype:$x, gentype:$y), [], []>;
def svml_inv_native     : OclBuiltin<"__ocl_svml_$Target_inv$SVMLSuffix_native",      (outs gentype:$ret), (ins gentype:$x), [], []>;
def svml_log_native     : OclBuiltin<"__ocl_svml_$Target_log$SVMLSuffix_native",      (outs gentype:$ret), (ins gentype:$x), [], []>;
def svml_log2_native    : OclBuiltin<"__ocl_svml_$Target_log2$SVMLSuffix_native",     (outs gentype:$ret), (ins gentype:$x), [], []>;
def svml_log10_native   : OclBuiltin<"__ocl_svml_$Target_log10$SVMLSuffix_native",    (outs gentype:$ret), (ins gentype:$x), [], []>;
def svml_log1p_native   : OclBuiltin<"__ocl_svml_$Target_log1p$SVMLSuffix_native",    (outs gentype:$ret), (ins gentype:$x), [], []>;
def svml_pow_native     : OclBuiltin<"__ocl_svml_$Target_pow$SVMLSuffix_native",      (outs gentype:$ret), (ins gentype:$x, gentype:$y), [], []>;
def svml_pown_native    : OclBuiltin<"__ocl_svml_$Target_pown$SVMLSuffix_native",     (outs gentype:$ret), (ins gentype:$x, i32gentype:$y), [], []>;
def svml_powr_native    : OclBuiltin<"__ocl_svml_$Target_powr$SVMLSuffix_native",     (outs gentype:$ret), (ins gentype:$x, gentype:$y), [], []>;
def svml_rcbrt_native   : OclBuiltin<"__ocl_svml_$Target_rcbrt$SVMLSuffix_native",    (outs gentype:$ret), (ins gentype:$x), [], []>;
def svml_rootn_native   : OclBuiltin<"__ocl_svml_$Target_rootn$SVMLSuffix_native",    (outs gentype:$ret), (ins gentype:$x, i32gentype:$y), [], []>;
def svml_rsqrt_native   : OclBuiltin<"__ocl_svml_$Target_rsqrt$SVMLSuffix_native",    (outs gentype:$ret), (ins gentype:$x), [], []>;
def svml_sincos_native  : OclBuiltin<"__ocl_svml_$Target_sincos$SVMLSuffix_native",   (outs gentype:$ret), (ins gentype:$x, pgentype:$cosval), [], []>, ExceptAttr<[CONSTFUNC]>;
def svml_sin_native     : OclBuiltin<"__ocl_svml_$Target_sin$SVMLSuffix_native",      (outs gentype:$ret), (ins gentype:$x), [], []>;
def svml_sinh_native    : OclBuiltin<"__ocl_svml_$Target_sinh$SVMLSuffix_native",     (outs gentype:$ret), (ins gentype:$x), [], []>;
def svml_sinpi_native   : OclBuiltin<"__ocl_svml_$Target_sinpi$SVMLSuffix_native",    (outs gentype:$ret), (ins gentype:$x), [], []>;
def svml_sqrt_native    : OclBuiltin<"__ocl_svml_$Target_sqrt$SVMLSuffix_native",     (outs gentype:$ret), (ins gentype:$x), [], []>;
def svml_tan_native     : OclBuiltin<"__ocl_svml_$Target_tan$SVMLSuffix_native",      (outs gentype:$ret), (ins gentype:$x), [], []>;
def svml_tanh_native    : OclBuiltin<"__ocl_svml_$Target_tanh$SVMLSuffix_native",     (outs gentype:$ret), (ins gentype:$x), [], []>;
def svml_tanpi_native   : OclBuiltin<"__ocl_svml_$Target_tanpi$SVMLSuffix_native",    (outs gentype:$ret), (ins gentype:$x), [], []>;
}

let Types = [v1f32, v2f32, v3f32, v4f32, v8f32, v16f32],
    Attrs = [SVMLCC, CONSTFUNC],
    IsDeclOnly = 1, NeedForwardDecl = 1 in {
def svml_cos_half     : OclBuiltin<"__ocl_svml_$Target_cos$SVMLSuffix_half",      (outs gentype:$ret), (ins gentype:$x), [], []>;
def svml_div_half     : OclBuiltin<"__ocl_svml_$Target_div$SVMLSuffix_half",      (outs gentype:$ret), (ins gentype:$x, gentype:$y), [], []>;
def svml_exp_half     : OclBuiltin<"__ocl_svml_$Target_exp$SVMLSuffix_half",      (outs gentype:$ret), (ins gentype:$x), [], []>;
def svml_exp2_half    : OclBuiltin<"__ocl_svml_$Target_exp2$SVMLSuffix_half",     (outs gentype:$ret), (ins gentype:$x), [], []>;
def svml_exp10_half   : OclBuiltin<"__ocl_svml_$Target_exp10$SVMLSuffix_half",    (outs gentype:$ret), (ins gentype:$x), [], []>;
def svml_log_half     : OclBuiltin<"__ocl_svml_$Target_log$SVMLSuffix_half",      (outs gentype:$ret), (ins gentype:$x), [], []>;
def svml_log2_half    : OclBuiltin<"__ocl_svml_$Target_log2$SVMLSuffix_half",     (outs gentype:$ret), (ins gentype:$x), [], []>;
def svml_log10_half   : OclBuiltin<"__ocl_svml_$Target_log10$SVMLSuffix_half",    (outs gentype:$ret), (ins gentype:$x), [], []>;
def svml_powr_half    : OclBuiltin<"__ocl_svml_$Target_powr$SVMLSuffix_half",     (outs gentype:$ret), (ins gentype:$x, gentype:$y), [], []>;
def svml_recip_half     : OclBuiltin<"__ocl_svml_$Target_inv$SVMLSuffix_half",      (outs gentype:$ret), (ins gentype:$x), [], []>;
def svml_rsqrt_half   : OclBuiltin<"__ocl_svml_$Target_rsqrt$SVMLSuffix_half",    (outs gentype:$ret), (ins gentype:$x), [], []>;
def svml_sin_half     : OclBuiltin<"__ocl_svml_$Target_sin$SVMLSuffix_half",      (outs gentype:$ret), (ins gentype:$x), [], []>;
def svml_sqrt_half    : OclBuiltin<"__ocl_svml_$Target_sqrt$SVMLSuffix_half",     (outs gentype:$ret), (ins gentype:$x), [], []>;
def svml_tan_half     : OclBuiltin<"__ocl_svml_$Target_tan$SVMLSuffix_half",      (outs gentype:$ret), (ins gentype:$x), [], []>;
}

let Types = [v1f64, v2f64, v3f64, v4f64, v8f64, v16f64],
    Attrs = [SVMLCC, CONSTFUNC],
    IsDeclOnly = 1, NeedForwardDecl = 1 in {
def svml_cvtfp64tofp32_rte  : OclBuiltin<"__ocl_svml_$Target_cvtfp64tofp32$rtn$SVMLSuffix",  (outs f32gentype:$ret), (ins gentype:$x), [], []>;
def svml_cvtfp64tofp32_rtz  : OclBuiltin<"__ocl_svml_$Target_cvtfp64tofp32$rtz$SVMLSuffix",  (outs f32gentype:$ret), (ins gentype:$x), [], []>;
def svml_cvtfp64tofp32_rtp  : OclBuiltin<"__ocl_svml_$Target_cvtfp64tofp32$up$SVMLSuffix",   (outs f32gentype:$ret), (ins gentype:$x), [], []>;
def svml_cvtfp64tofp32_rtn  : OclBuiltin<"__ocl_svml_$Target_cvtfp64tofp32$down$SVMLSuffix", (outs f32gentype:$ret), (ins gentype:$x), [], []>;
}

let Types = [v1f32, v2f32, v3f32, v4f32, v8f32, v16f32,
             v1f64, v2f64, v3f64, v4f64, v8f64, v16f64],
    Attrs = [SVMLCC, CONSTFUNC],
    IsDeclOnly = 1, NeedForwardDecl = 1 in {
def svml_cvtfptoi32_rte       : OclBuiltin<"__ocl_svml_$Target_cvtfptoi32$rtnnosat$SVMLSuffix",  (outs i32gentype:$ret), (ins gentype:$x), [], []>;
def svml_cvtfptoi32_rtz       : OclBuiltin<"__ocl_svml_$Target_cvtfptoi32$rtznosat$SVMLSuffix",  (outs i32gentype:$ret), (ins gentype:$x), [], []>;
def svml_cvtfptoi32_rtp       : OclBuiltin<"__ocl_svml_$Target_cvtfptoi32$upnosat$SVMLSuffix",   (outs i32gentype:$ret), (ins gentype:$x), [], []>;
def svml_cvtfptoi32_rtn       : OclBuiltin<"__ocl_svml_$Target_cvtfptoi32$downnosat$SVMLSuffix", (outs i32gentype:$ret), (ins gentype:$x), [], []>;
def svml_cvtfptoi32_sat_rte   : OclBuiltin<"__ocl_svml_$Target_cvtfptoi32$rtnsat$SVMLSuffix",    (outs i32gentype:$ret), (ins gentype:$x), [], []>;
def svml_cvtfptoi32_sat_rtz   : OclBuiltin<"__ocl_svml_$Target_cvtfptoi32$rtzsat$SVMLSuffix",    (outs i32gentype:$ret), (ins gentype:$x), [], []>;
def svml_cvtfptoi32_sat_rtp   : OclBuiltin<"__ocl_svml_$Target_cvtfptoi32$upsat$SVMLSuffix",     (outs i32gentype:$ret), (ins gentype:$x), [], []>;
def svml_cvtfptoi32_sat_rtn   : OclBuiltin<"__ocl_svml_$Target_cvtfptoi32$downsat$SVMLSuffix",   (outs i32gentype:$ret), (ins gentype:$x), [], []>;
def svml_cvtfptou32_rte       : OclBuiltin<"__ocl_svml_$Target_cvtfptou32$rtnnosat$SVMLSuffix",  (outs u32gentype:$ret), (ins gentype:$x), [], []>;
def svml_cvtfptou32_rtz       : OclBuiltin<"__ocl_svml_$Target_cvtfptou32$rtznosat$SVMLSuffix",  (outs u32gentype:$ret), (ins gentype:$x), [], []>;
def svml_cvtfptou32_rtp       : OclBuiltin<"__ocl_svml_$Target_cvtfptou32$upnosat$SVMLSuffix",   (outs u32gentype:$ret), (ins gentype:$x), [], []>;
def svml_cvtfptou32_rtn       : OclBuiltin<"__ocl_svml_$Target_cvtfptou32$downnosat$SVMLSuffix", (outs u32gentype:$ret), (ins gentype:$x), [], []>;
def svml_cvtfptou32_sat_rte   : OclBuiltin<"__ocl_svml_$Target_cvtfptou32$rtnsat$SVMLSuffix",    (outs u32gentype:$ret), (ins gentype:$x), [], []>;
def svml_cvtfptou32_sat_rtz   : OclBuiltin<"__ocl_svml_$Target_cvtfptou32$rtzsat$SVMLSuffix",    (outs u32gentype:$ret), (ins gentype:$x), [], []>;
def svml_cvtfptou32_sat_rtp   : OclBuiltin<"__ocl_svml_$Target_cvtfptou32$upsat$SVMLSuffix",     (outs u32gentype:$ret), (ins gentype:$x), [], []>;
def svml_cvtfptou32_sat_rtn   : OclBuiltin<"__ocl_svml_$Target_cvtfptou32$downsat$SVMLSuffix",   (outs u32gentype:$ret), (ins gentype:$x), [], []>;
def svml_cvtfptoi64_rte       : OclBuiltin<"__ocl_svml_$Target_cvtfptoi64$rtnnosat$SVMLSuffix",  (outs i64gentype:$ret), (ins gentype:$x), [], []>, Except<[v1f32, v1f64]>;
def svml_cvtfptoi64_rtz       : OclBuiltin<"__ocl_svml_$Target_cvtfptoi64$rtznosat$SVMLSuffix",  (outs i64gentype:$ret), (ins gentype:$x), [], []>, Except<[v1f32, v1f64]>;
def svml_cvtfptoi64_rtp       : OclBuiltin<"__ocl_svml_$Target_cvtfptoi64$upnosat$SVMLSuffix",   (outs i64gentype:$ret), (ins gentype:$x), [], []>, Except<[v1f32, v1f64]>;
def svml_cvtfptoi64_rtn       : OclBuiltin<"__ocl_svml_$Target_cvtfptoi64$downnosat$SVMLSuffix", (outs i64gentype:$ret), (ins gentype:$x), [], []>, Except<[v1f32, v1f64]>;
def svml_cvtfptoi64_sat_rte   : OclBuiltin<"__ocl_svml_$Target_cvtfptoi64$rtnsat$SVMLSuffix",    (outs i64gentype:$ret), (ins gentype:$x), [], []>, Except<[v1f32, v1f64]>;
def svml_cvtfptoi64_sat_rtz   : OclBuiltin<"__ocl_svml_$Target_cvtfptoi64$rtzsat$SVMLSuffix",    (outs i64gentype:$ret), (ins gentype:$x), [], []>, Except<[v1f32, v1f64]>;
def svml_cvtfptoi64_sat_rtp   : OclBuiltin<"__ocl_svml_$Target_cvtfptoi64$upsat$SVMLSuffix",     (outs i64gentype:$ret), (ins gentype:$x), [], []>, Except<[v1f32, v1f64]>;
def svml_cvtfptoi64_sat_rtn   : OclBuiltin<"__ocl_svml_$Target_cvtfptoi64$downsat$SVMLSuffix",   (outs i64gentype:$ret), (ins gentype:$x), [], []>, Except<[v1f32, v1f64]>;
def svml_cvtfptou64_rte       : OclBuiltin<"__ocl_svml_$Target_cvtfptou64$rtnnosat$SVMLSuffix",  (outs u64gentype:$ret), (ins gentype:$x), [], []>, Except<[v1f32, v1f64]>;
def svml_cvtfptou64_rtz       : OclBuiltin<"__ocl_svml_$Target_cvtfptou64$rtznosat$SVMLSuffix",  (outs u64gentype:$ret), (ins gentype:$x), [], []>, Except<[v1f32, v1f64]>;
def svml_cvtfptou64_rtp       : OclBuiltin<"__ocl_svml_$Target_cvtfptou64$upnosat$SVMLSuffix",   (outs u64gentype:$ret), (ins gentype:$x), [], []>, Except<[v1f32, v1f64]>;
def svml_cvtfptou64_rtn       : OclBuiltin<"__ocl_svml_$Target_cvtfptou64$downnosat$SVMLSuffix", (outs u64gentype:$ret), (ins gentype:$x), [], []>, Except<[v1f32, v1f64]>;
def svml_cvtfptou64_sat_rte   : OclBuiltin<"__ocl_svml_$Target_cvtfptou64$rtnsat$SVMLSuffix",    (outs u64gentype:$ret), (ins gentype:$x), [], []>, Except<[v1f32, v1f64]>;
def svml_cvtfptou64_sat_rtz   : OclBuiltin<"__ocl_svml_$Target_cvtfptou64$rtzsat$SVMLSuffix",    (outs u64gentype:$ret), (ins gentype:$x), [], []>, Except<[v1f32, v1f64]>;
def svml_cvtfptou64_sat_rtp   : OclBuiltin<"__ocl_svml_$Target_cvtfptou64$upsat$SVMLSuffix",     (outs u64gentype:$ret), (ins gentype:$x), [], []>, Except<[v1f32, v1f64]>;
def svml_cvtfptou64_sat_rtn   : OclBuiltin<"__ocl_svml_$Target_cvtfptou64$downsat$SVMLSuffix",   (outs u64gentype:$ret), (ins gentype:$x), [], []>, Except<[v1f32, v1f64]>;
}

let Types = [v1f32, v2f32, v3f32, v4f32, v8f32, v16f32,
             v1f64, v2f64, v3f64, v4f64, v8f64, v16f64],
    Attrs = [SVMLCC, CONSTFUNC],
    IsDeclOnly = 1, NeedForwardDecl = 1 in {
def svml_cvti32tofp_rte : OclBuiltin<"__ocl_svml_$Target_cvti32tofp$rtn$SVMLSuffix",   (outs gentype:$ret), (ins i32gentype:$x), [], []>;
def svml_cvti32tofp_rtz : OclBuiltin<"__ocl_svml_$Target_cvti32tofp$rtz$SVMLSuffix",   (outs gentype:$ret), (ins i32gentype:$x), [], []>;
def svml_cvti32tofp_rtp : OclBuiltin<"__ocl_svml_$Target_cvti32tofp$up$SVMLSuffix",    (outs gentype:$ret), (ins i32gentype:$x), [], []>;
def svml_cvti32tofp_rtn : OclBuiltin<"__ocl_svml_$Target_cvti32tofp$down$SVMLSuffix",  (outs gentype:$ret), (ins i32gentype:$x), [], []>;
def svml_cvtu32tofp_rte : OclBuiltin<"__ocl_svml_$Target_cvtu32tofp$rtn$SVMLSuffix",   (outs gentype:$ret), (ins u32gentype:$x), [], []>;
def svml_cvtu32tofp_rtz : OclBuiltin<"__ocl_svml_$Target_cvtu32tofp$rtz$SVMLSuffix",   (outs gentype:$ret), (ins u32gentype:$x), [], []>;
def svml_cvtu32tofp_rtp : OclBuiltin<"__ocl_svml_$Target_cvtu32tofp$up$SVMLSuffix",    (outs gentype:$ret), (ins u32gentype:$x), [], []>;
def svml_cvtu32tofp_rtn : OclBuiltin<"__ocl_svml_$Target_cvtu32tofp$down$SVMLSuffix",  (outs gentype:$ret), (ins u32gentype:$x), [], []>;
def svml_cvti64tofp_rte : OclBuiltin<"__ocl_svml_$Target_cvti64tofp$rtn$SVMLSuffix",   (outs gentype:$ret), (ins i64gentype:$x), [], []>, Except<[v1f32, v1f64]>;
def svml_cvti64tofp_rtz : OclBuiltin<"__ocl_svml_$Target_cvti64tofp$rtz$SVMLSuffix",   (outs gentype:$ret), (ins i64gentype:$x), [], []>, Except<[v1f32, v1f64]>;
def svml_cvti64tofp_rtp : OclBuiltin<"__ocl_svml_$Target_cvti64tofp$up$SVMLSuffix",    (outs gentype:$ret), (ins i64gentype:$x), [], []>, Except<[v1f32, v1f64]>;
def svml_cvti64tofp_rtn : OclBuiltin<"__ocl_svml_$Target_cvti64tofp$down$SVMLSuffix",  (outs gentype:$ret), (ins i64gentype:$x), [], []>, Except<[v1f32, v1f64]>;
def svml_cvtu64tofp_rte : OclBuiltin<"__ocl_svml_$Target_cvtu64tofp$rtn$SVMLSuffix",   (outs gentype:$ret), (ins u64gentype:$x), [], []>, Except<[v1f32, v1f64]>;
def svml_cvtu64tofp_rtz : OclBuiltin<"__ocl_svml_$Target_cvtu64tofp$rtz$SVMLSuffix",   (outs gentype:$ret), (ins u64gentype:$x), [], []>, Except<[v1f32, v1f64]>;
def svml_cvtu64tofp_rtp : OclBuiltin<"__ocl_svml_$Target_cvtu64tofp$up$SVMLSuffix",    (outs gentype:$ret), (ins u64gentype:$x), [], []>, Except<[v1f32, v1f64]>;
def svml_cvtu64tofp_rtn : OclBuiltin<"__ocl_svml_$Target_cvtu64tofp$down$SVMLSuffix",  (outs gentype:$ret), (ins u64gentype:$x), [], []>, Except<[v1f32, v1f64]>;
}

// All conversion functions from/to scalar 64-bit integer values do not use SVML calling convention.
let Types = [v1f32, v1f64],
    Attrs = [CONSTFUNC],
    IsDeclOnly = 1, NeedForwardDecl = 1 in {
def svml_cvtfptoi64v1_rte       : OclBuiltin<"__ocl_svml_$Target_cvtfptoi64$rtnnosat$SVMLSuffix",  (outs i64gentype:$ret), (ins gentype:$x), [], []>;
def svml_cvtfptoi64v1_rtz       : OclBuiltin<"__ocl_svml_$Target_cvtfptoi64$rtznosat$SVMLSuffix",  (outs i64gentype:$ret), (ins gentype:$x), [], []>;
def svml_cvtfptoi64v1_rtp       : OclBuiltin<"__ocl_svml_$Target_cvtfptoi64$upnosat$SVMLSuffix",   (outs i64gentype:$ret), (ins gentype:$x), [], []>;
def svml_cvtfptoi64v1_rtn       : OclBuiltin<"__ocl_svml_$Target_cvtfptoi64$downnosat$SVMLSuffix", (outs i64gentype:$ret), (ins gentype:$x), [], []>;
def svml_cvtfptoi64v1_sat_rte   : OclBuiltin<"__ocl_svml_$Target_cvtfptoi64$rtnsat$SVMLSuffix",    (outs i64gentype:$ret), (ins gentype:$x), [], []>;
def svml_cvtfptoi64v1_sat_rtz   : OclBuiltin<"__ocl_svml_$Target_cvtfptoi64$rtzsat$SVMLSuffix",    (outs i64gentype:$ret), (ins gentype:$x), [], []>;
def svml_cvtfptoi64v1_sat_rtp   : OclBuiltin<"__ocl_svml_$Target_cvtfptoi64$upsat$SVMLSuffix",     (outs i64gentype:$ret), (ins gentype:$x), [], []>;
def svml_cvtfptoi64v1_sat_rtn   : OclBuiltin<"__ocl_svml_$Target_cvtfptoi64$downsat$SVMLSuffix",   (outs i64gentype:$ret), (ins gentype:$x), [], []>;
def svml_cvtfptou64v1_rte       : OclBuiltin<"__ocl_svml_$Target_cvtfptou64$rtnnosat$SVMLSuffix",  (outs u64gentype:$ret), (ins gentype:$x), [], []>;
def svml_cvtfptou64v1_rtz       : OclBuiltin<"__ocl_svml_$Target_cvtfptou64$rtznosat$SVMLSuffix",  (outs u64gentype:$ret), (ins gentype:$x), [], []>;
def svml_cvtfptou64v1_rtp       : OclBuiltin<"__ocl_svml_$Target_cvtfptou64$upnosat$SVMLSuffix",   (outs u64gentype:$ret), (ins gentype:$x), [], []>;
def svml_cvtfptou64v1_rtn       : OclBuiltin<"__ocl_svml_$Target_cvtfptou64$downnosat$SVMLSuffix", (outs u64gentype:$ret), (ins gentype:$x), [], []>;
def svml_cvtfptou64v1_sat_rte   : OclBuiltin<"__ocl_svml_$Target_cvtfptou64$rtnsat$SVMLSuffix",    (outs u64gentype:$ret), (ins gentype:$x), [], []>;
def svml_cvtfptou64v1_sat_rtz   : OclBuiltin<"__ocl_svml_$Target_cvtfptou64$rtzsat$SVMLSuffix",    (outs u64gentype:$ret), (ins gentype:$x), [], []>;
def svml_cvtfptou64v1_sat_rtp   : OclBuiltin<"__ocl_svml_$Target_cvtfptou64$upsat$SVMLSuffix",     (outs u64gentype:$ret), (ins gentype:$x), [], []>;
def svml_cvtfptou64v1_sat_rtn   : OclBuiltin<"__ocl_svml_$Target_cvtfptou64$downsat$SVMLSuffix",   (outs u64gentype:$ret), (ins gentype:$x), [], []>;
}

let Types = [v1f32, v1f64],
    Attrs = [CONSTFUNC],
    IsDeclOnly = 1, NeedForwardDecl = 1 in {
def svml_cvti64v1tofp_rte : OclBuiltin<"__ocl_svml_$Target_cvti64tofp$rtn$SVMLSuffix",   (outs gentype:$ret), (ins i64gentype:$x), [], []>;
def svml_cvti64v1tofp_rtz : OclBuiltin<"__ocl_svml_$Target_cvti64tofp$rtz$SVMLSuffix",   (outs gentype:$ret), (ins i64gentype:$x), [], []>;
def svml_cvti64v1tofp_rtp : OclBuiltin<"__ocl_svml_$Target_cvti64tofp$up$SVMLSuffix",    (outs gentype:$ret), (ins i64gentype:$x), [], []>;
def svml_cvti64v1tofp_rtn : OclBuiltin<"__ocl_svml_$Target_cvti64tofp$down$SVMLSuffix",  (outs gentype:$ret), (ins i64gentype:$x), [], []>;
def svml_cvtu64v1tofp_rte : OclBuiltin<"__ocl_svml_$Target_cvtu64tofp$rtn$SVMLSuffix",   (outs gentype:$ret), (ins u64gentype:$x), [], []>;
def svml_cvtu64v1tofp_rtz : OclBuiltin<"__ocl_svml_$Target_cvtu64tofp$rtz$SVMLSuffix",   (outs gentype:$ret), (ins u64gentype:$x), [], []>;
def svml_cvtu64v1tofp_rtp : OclBuiltin<"__ocl_svml_$Target_cvtu64tofp$up$SVMLSuffix",    (outs gentype:$ret), (ins u64gentype:$x), [], []>;
def svml_cvtu64v1tofp_rtn : OclBuiltin<"__ocl_svml_$Target_cvtu64tofp$down$SVMLSuffix",  (outs gentype:$ret), (ins u64gentype:$x), [], []>;
}

let Types = [v1i32, v2i32, v3i32, v4i32, v8i32, v16i32],
    Attrs = [CONSTFUNC],
    IsDeclOnly = 1, NeedForwardDecl = 1 in {
def svml_idiv          : OclBuiltin<"__ocl_svml_$Target_idiv",      (outs gentype:$ret), (ins gentype:$x, gentype:$y), [], [], 0, 1>;
def svml_irem          : OclBuiltin<"__ocl_svml_$Target_irem",      (outs gentype:$ret), (ins gentype:$x, gentype:$y), [], [], 0, 1>;

}
let Types = [v1u32, v2u32, v3u32, v4u32, v8u32, v16u32],
    Attrs = [CONSTFUNC],
    IsDeclOnly = 1, NeedForwardDecl = 1 in {
def svml_udiv          : OclBuiltin<"__ocl_svml_$Target_udiv",      (outs gentype:$ret), (ins gentype:$x, gentype:$y), [], [], 0, 1>;
def svml_urem          : OclBuiltin<"__ocl_svml_$Target_urem",      (outs gentype:$ret), (ins gentype:$x, gentype:$y), [], [], 0, 1>;
}

